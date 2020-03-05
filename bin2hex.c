/*
 * Copyright (c) 2020 Brian Callahan <bcallah@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
bin2hex(FILE *in, FILE *out)
{
	int c, sum;
	uint8_t record[16], i, size;
	uint16_t addr = 0;

rec:
	memset(record, 0, sizeof record);
	i = 0;
	sum = 0;
	do {
		if ((c = fgetc(in)) == EOF)
			break;

		record[i] = (uint8_t) c;
		sum += c;
	} while (++i < sizeof record);
	sum = sum + i + ((uint8_t) (addr >> 8)) + ((uint8_t) addr);

	if (i > 0) {
		fprintf(out, ":%02X%04X%02X", i, addr, 0x00);
		for (size = 0; size < i; ++size)
			fprintf(out, "%02X", record[size]);
		fprintf(out, "%02X\n", (unsigned char) (-sum));

		if (c != EOF) {
			addr += 0x10;
			goto rec;
		}
	}

	fprintf(out, ":%02X%04X%02X%02X\n", 0x00, 0x0000, 0x01, 0xFF);
}

int
main(int argc, char *argv[])
{
	FILE *fp, *fq;

	if (pledge("stdio rpath wpath cpath", NULL) == -1)
		err(1, "pledge");

	if (argc != 3) {
		(void) fprintf(stderr, "usage: %s infile outfile\n",
			       getprogname());
		exit(1);
	}

	if ((fp = fopen(argv[1], "r")) == NULL)
		err(1, "fopen");

	if ((fq = fopen(argv[2], "w+")) == NULL)
		err(1, "fopen");

	if (pledge("stdio", NULL) == -1)
		err(1, "pledge");

	bin2hex(fp, fq);

	(void) fclose(fq);
	(void) fclose(fp);

	return 0;
}
