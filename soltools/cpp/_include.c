#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#if (defined(_WIN32) || defined(_MSDOS) || defined(__IBMC__))
#   include <io.h>
#else
#   include <unistd.h>
#endif

#ifdef __hpux
#   define _HPUX_SOURCE
#endif
#ifdef _MSC_VER
#   define _POSIX_
#endif
#ifdef __IBMC__
#   include <fcntl.h>
#   define PATH_MAX _MAX_PATH
#endif
#include <limits.h>

#include "cpp.h"

Includelist includelist[NINCLUDE];
Wraplist wraplist[NINCLUDE];

void
    doinclude(Tokenrow * trp, int depth, int import)
{
    char fname[PATH_MAX], iname[PATH_MAX];
    Includelist *ip;
    int angled, len, fd, i;

    trp->tp += 1;
    if (trp->tp >= trp->lp)
        goto syntax;
    if (trp->tp->type != STRING && trp->tp->type != LT)
    {
        len = trp->tp - trp->bp;
        expandrow(trp, "<include>");
        trp->tp = trp->bp + len;
    }
    if (trp->tp->type == STRING)
    {
        len = trp->tp->len - 2;
        if (len > sizeof(fname) - 1)
            len = sizeof(fname) - 1;
        strncpy(fname, (char *) trp->tp->t + 1, len);
        angled = 0;
    }
    else
    {
        if (trp->tp->type == LT)
        {
            len = 0;
            trp->tp++;
            while (trp->tp->type != GT)
            {
                if (trp->tp > trp->lp || len + trp->tp->len + 2 >= sizeof(fname))
                    goto syntax;
                strncpy(fname + len, (char *) trp->tp->t, trp->tp->len);
                len += trp->tp->len;
                trp->tp++;
            }
            angled = 1;
        }
        else
            goto syntax;
    }
    trp->tp += 2;
    if (trp->tp < trp->lp || len == 0)
        goto syntax;
    fname[len] = '\0';
    if (fname[0] == '/')
    {
        fd = open(fname, O_RDONLY);
        strcpy(iname, fname);
    }
    else
    {
        for (fd = -1, i = (depth < 0) ? (NINCLUDE - 1) : (depth - 1); i >= 0; i--)
        {
            ip = &includelist[i];
            if (ip->file == NULL || ip->deleted || (angled && ip->always == 0))
                continue;
            if (strlen(fname) + strlen(ip->file) + 2 > sizeof(iname))
                continue;
            strcpy(iname, ip->file);
            strcat(iname, "/");
            strcat(iname, fname);
            if ((fd = open(iname, O_RDONLY)) >= 0)
                break;
        }
    }

    if (fd >= 0)
    {
        if (++incdepth > NINC )
            error(FATAL, "#%s too deeply nested", import ? "import" : "include");
        if (Xflag)
            genimport(fname, angled, iname, import);
         if (Iflag)
            error(INFO, "Open %s file [%s]", import ? "import" : "include", iname );

        for (i = NINCLUDE - 1; i >= 0; i--)
        {
            if ((wraplist[i].file != NULL) &&
                (strncmp(wraplist[i].file, iname, strlen(wraplist[i].file)) == 0))
                break;
        }

        setsource((char *) newstring((uchar *) iname, strlen(iname), 0), i, fd, NULL, (i >= 0) ? 1 : 0);

        if (!Pflag)
            genline();
    }
    else
    {
        trp->tp = trp->bp + 2;
        error(ERROR, "Could not find %s file %r", import ? "import" : "include", trp);
    }
    return;
syntax:
    error(ERROR, "Syntax error in #%s", import ? "import" : "include");
    return;
}

/*
 * Generate a line directive for cursource
 */
void
    genline(void)
{
    static Token ta = {UNCLASS};
    static Tokenrow tr = {&ta, &ta, &ta + 1, 1};
    uchar *p;

    ta.t = p = (uchar *) outptr;
    strcpy((char *) p, "#line ");
    p += sizeof("#line ") - 1;
    p = (uchar *) outnum((char *) p, cursource->line);
    *p++ = ' ';
    *p++ = '"';
    if (cursource->filename[0] != '/' && wd[0])
    {
        strcpy((char *) p, wd);
        p += strlen(wd);
        *p++ = '/';
    }
    strcpy((char *) p, cursource->filename);
    p += strlen((char *) p);
    *p++ = '"';
    *p++ = '\n';
    ta.len = (char *) p - outptr;
    outptr = (char *) p;
    tr.tp = tr.bp;
    puttokens(&tr);
}

/*
 * Generate a pragma import/include directive
 */
void
    genimport(char *fname, int angled, char *iname, int import)
{
    static Token ta = {UNCLASS};
    static Tokenrow tr = {&ta, &ta, &ta + 1, 1};
    uchar *p;

    ta.t = p = (uchar *) outptr;

    if (import)
        strcpy((char *) p, "#pragma import");
    else
        strcpy((char *) p, "#pragma include");

    p += strlen((char *) p);

    *p++ = '(';

    *p++ = angled ? '<' : '"';
    strcpy((char *) p, fname);
    p += strlen(fname);
    *p++ = angled ? '>' : '"';

    *p++ = ',';

    *p++ = '"';
    strcpy((char *) p, iname);
    p += strlen(iname);
    *p++ = '"';

    *p++ = ')';
    *p++ = '\n';

    ta.len = (char *) p - outptr;
    outptr = (char *) p;
    tr.tp = tr.bp;
    puttokens(&tr);
}

/*
 * Generate a extern C directive
 */
void
    genwrap(int end)
{
    static Token ta = {UNCLASS};
    static Tokenrow tr = {&ta, &ta, &ta + 1, 1};
    uchar *p;

    if (Cplusplus)
    {
        ta.t = p = (uchar *) outptr;

        if (! end)
            strcpy((char *) p, "extern \"C\" {");
        else
            strcpy((char *) p, "}");

        p += strlen((char *) p);

        *p++ = '\n';

        ta.len = (char *) p - outptr;
        outptr = (char *) p;
        tr.tp = tr.bp;
        puttokens(&tr);
    }
}

