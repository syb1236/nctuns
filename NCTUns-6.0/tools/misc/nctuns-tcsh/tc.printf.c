/* $Header: /src/pub/tcsh/tc.printf.c,v 3.27 2005/01/05 16:06:15 christos Exp $ */
/*
 * tc.printf.c: A public-domain, minimal printf/sprintf routine that prints
 *	       through the putchar() routine.  Feel free to use for
 *	       anything...  -- 7/17/87 Paul Placeway
 */
/*-
 * Copyright (c) 1980, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "sh.h"

RCSID("$Id: tc.printf.c,v 3.27 2005/01/05 16:06:15 christos Exp $")

#ifdef lint
#undef va_arg
#define va_arg(a, b) (a ? (b) 0 : (b) 0)
#endif

#define INF	32766		/* should be bigger than any field to print */

static char buf[128];
static char snil[] = "(nil)";

static	void	xaddchar	__P((int));
static	void	doprnt		__P((void (*) __P((int)), const char *, va_list));

static void
doprnt(addchar, sfmt, ap)
    void    (*addchar) __P((int));
    const char   *sfmt;
    va_list ap;
{
    char *bp;
    const char *f;
#ifdef SHORT_STRINGS
    Char *Bp;
#endif /* SHORT_STRINGS */
#ifdef HAVE_LONG_LONG
    long long l;
    unsigned long long u;
#else
    long l;
    unsigned long u;
#endif
    int i;
    int fmt;
    unsigned char pad = ' ';
    int     flush_left = 0, f_width = 0, prec = INF, hash = 0, do_long = 0;
    int     sign = 0;
    int     attributes = 0;


    f = sfmt;
    for (; *f; f++) {
	if (*f != '%') {	/* then just out the char */
	    (*addchar) ((int) (((unsigned char)*f) | attributes));
	}
	else {
	    f++;		/* skip the % */

	    if (*f == '-') {	/* minus: flush left */
		flush_left = 1;
		f++;
	    }

	    if (*f == '0' || *f == '.') {
		/* padding with 0 rather than blank */
		pad = '0';
		f++;
	    }
	    if (*f == '*') {	/* field width */
		f_width = va_arg(ap, int);
		f++;
	    }
	    else if (isdigit((unsigned char) *f)) {
		f_width = atoi(f);
		while (isdigit((unsigned char) *f))
		    f++;	/* skip the digits */
	    }

	    if (*f == '.') {	/* precision */
		f++;
		if (*f == '*') {
		    prec = va_arg(ap, int);
		    f++;
		}
		else if (isdigit((unsigned char) *f)) {
		    prec = atoi(f);
		    while (isdigit((unsigned char) *f))
			f++;	/* skip the digits */
		}
	    }

	    if (*f == '#') {	/* alternate form */
		hash = 1;
		f++;
	    }

	    if (*f == 'l') {	/* long format */
		do_long++;
		f++;
		if (*f == 'l') {
		    do_long++;
		    f++;
		}
	    }

	    fmt = (unsigned char) *f;
	    if (fmt != 'S' && fmt != 'Q' && isupper(fmt)) {
		do_long = 1;
		fmt = tolower(fmt);
	    }
	    bp = buf;
	    switch (fmt) {	/* do the format */
	    case 'd':
		switch (do_long) {
		case 0:
		    l = (long) (va_arg(ap, int));
		    break;
		case 1:
#ifndef HAVE_LONG_LONG
		default:
#endif
		    l = va_arg(ap, long);
		    break;
#ifdef HAVE_LONG_LONG
		default:
		    l = va_arg(ap, long long);
		    break;
#endif
		}

		if (l < 0) {
		    sign = 1;
		    l = -l;
		}
		do {
		    *bp++ = (char) (l % 10) + '0';
		} while ((l /= 10) > 0);
		if (sign)
		    *bp++ = '-';
		f_width = f_width - (int) (bp - buf);
		if (!flush_left)
		    while (f_width-- > 0) 
			(*addchar) ((int) (pad | attributes));
		for (bp--; bp >= buf; bp--) 
		    (*addchar) ((int) (((unsigned char) *bp) | attributes));
		if (flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (' ' | attributes));
		break;

	    case 'p':
		do_long = 1;
		hash = 1;
		fmt = 'x';
		/*FALLTHROUGH*/
	    case 'o':
	    case 'x':
	    case 'u':
		switch (do_long) {
		case 0:
		    u = (unsigned long) (va_arg(ap, unsigned int));
		    break;
		case 1:
#ifndef HAVE_LONG_LONG
		default:
#endif
		    u = va_arg(ap, unsigned long);
		    break;
#ifdef HAVE_LONG_LONG
		default:
		    u = va_arg(ap, unsigned long long);
		    break;
#endif
		}
		if (fmt == 'u') {	/* unsigned decimal */
		    do {
			*bp++ = (char) (u % 10) + '0';
		    } while ((u /= 10) > 0);
		}
		else if (fmt == 'o') {	/* octal */
		    do {
			*bp++ = (char) (u % 8) + '0';
		    } while ((u /= 8) > 0);
		    if (hash)
			*bp++ = '0';
		}
		else if (fmt == 'x') {	/* hex */
		    do {
			i = (int) (u % 16);
			if (i < 10)
			    *bp++ = i + '0';
			else
			    *bp++ = i - 10 + 'a';
		    } while ((u /= 16) > 0);
		    if (hash) {
			*bp++ = 'x';
			*bp++ = '0';
		    }
		}
		i = f_width - (int) (bp - buf);
		if (!flush_left)
		    while (i-- > 0)
			(*addchar) ((int) (pad | attributes));
		for (bp--; bp >= buf; bp--)
		    (*addchar) ((int) (((unsigned char) *bp) | attributes));
		if (flush_left)
		    while (i-- > 0)
			(*addchar) ((int) (' ' | attributes));
		break;


	    case 'c':
		i = va_arg(ap, int);
		(*addchar) ((int) (i | attributes));
		break;

	    case 'S':
	    case 'Q':
#ifdef SHORT_STRINGS
		Bp = va_arg(ap, Char *);
		if (!Bp) {
		    bp = NULL;
		    goto lcase_s;
	        }
		f_width = f_width - Strlen(Bp);
		if (!flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (pad | attributes));
		for (i = 0; *Bp && i < prec; i++) {
		    char cbuf[MB_LEN_MAX];
		    size_t pos, len;

		    if (fmt == 'Q' && *Bp & QUOTE)
			(*addchar) ((int) ('\\' | attributes));
		    len = one_wctomb(cbuf, *Bp & CHAR);
		    for (pos = 0; pos < len; pos++)
			(*addchar) ((int) ((unsigned char)cbuf[pos]
					   | attributes | (*Bp & ATTRIBUTES)));
		    Bp++;
		}
		if (flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (' ' | attributes));
		break;
#endif /* SHORT_STRINGS */

	    case 's':
	    case 'q':
		bp = va_arg(ap, char *);
lcase_s:
		if (!bp)
		    bp = snil;
		f_width = f_width - strlen((char *) bp);
		if (!flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (pad | attributes));
		for (i = 0; *bp && i < prec; i++) {
		    if (fmt == 'q' && *bp & QUOTE)
			(*addchar) ((int) ('\\' | attributes));
		    (*addchar) ((int) (((unsigned char) *bp & TRIM) |
				   	attributes));
		    bp++;
		}
		if (flush_left)
		    while (f_width-- > 0)
			(*addchar) ((int) (' ' | attributes));
		break;

	    case 'a':
		attributes = va_arg(ap, int);
		break;

	    case '%':
		(*addchar) ((int) ('%' | attributes));
		break;

	    default:
		break;
	    }
	    flush_left = 0, f_width = 0, prec = INF, hash = 0, do_long = 0;
	    sign = 0;
	    pad = ' ';
	}
    }
}


static char *xstring, *xestring;
static void
xaddchar(c)
    int     c;
{
    if (xestring == xstring)
	*xstring = '\0';
    else
	*xstring++ = (char) c;
}


pret_t
/*VARARGS*/
#ifdef PROTOTYPES
xsnprintf(char *str, size_t size, const char *fmt, ...)
#else
xsnprintf(va_alist)
    va_dcl
#endif
{
    va_list va;
#ifdef PROTOTYPES
    va_start(va, fmt);
#else
    char *str, *fmt;
    size_t size;

    va_start(va);
    str = va_arg(va, char *);
    size = va_arg(va, size_t);
    fmt = va_arg(va, char *);
#endif

    xstring = str;
    xestring = str + size - 1;
    doprnt(xaddchar, fmt, va);
    va_end(va);
    *xstring++ = '\0';
#ifdef PURIFY
    return 1;
#endif
}

pret_t
/*VARARGS*/
#ifdef PROTOTYPES
xprintf(const char *fmt, ...)
#else
xprintf(va_alist)
    va_dcl
#endif
{
    va_list va;
#ifdef PROTOTYPES
    va_start(va, fmt);
#else
    char   *fmt;

    va_start(va);
    fmt = va_arg(va, char *);
#endif
    doprnt(xputchar, fmt, va);
    va_end(va);
#ifdef PURIFY
    return 1;
#endif
}


pret_t
xvprintf(fmt, va)
    const char   *fmt;
    va_list va;
{
    doprnt(xputchar, fmt, va);
#ifdef PURIFY
    return 1;
#endif
}

pret_t
xvsnprintf(str, size, fmt, va)
    char   *str;
    size_t size;
    const char   *fmt;
    va_list va;
{
    xstring = str;
    xestring = str + size - 1;
    doprnt(xaddchar, fmt, va);
    *xstring++ = '\0';
#ifdef PURIFY
    return 1;
#endif
}



#ifdef PURIFY
/* Purify uses (some of..) the following functions to output memory-use
 * debugging info.  Given all the messing with file descriptors that
 * tcsh does, the easiest way I could think of to get it (Purify) to
 * print anything was by replacing some standard functions with
 * ones that do tcsh output directly - see dumb hook in doreaddirs()
 * (sh.dir.c) -sg
 */
#ifndef FILE
#define FILE int
#endif
int 
#ifdef PROTOTYPES
fprintf(FILE *fp, const char* fmt, ...)
#else
fprintf(va_alist)
    va_dcl
#endif
{
    va_list va;
#ifdef PROTOTYPES
    va_start(va, fmt);
#else
    FILE *fp;
    const char   *fmt;

    va_start(va);
    fp = va_arg(va, FILE *);
    fmt = va_arg(va, const char *);
#endif
    doprnt(xputchar, fmt, va);
    va_end(va);
    return 1;
}

int 
vfprintf(fp, fmt, va)
    FILE *fp;
    const char   *fmt;
    va_list va;
{
    doprnt(xputchar, fmt, va);
    return 1;
}

#endif	/* PURIFY */
