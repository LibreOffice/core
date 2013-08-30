/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* $XConsortium: main.c,v 1.84 94/11/30 16:10:44 kaleb Exp $ */
/* $XFree86: xc/config/makedepend/main.c,v 3.4 1995/07/15 14:53:49 dawes Exp $ */
/*

Copyright (c) 1993, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#if defined(FREEBSD) || defined(MACOSX)
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <io.h>
#endif

#ifdef _MSC_VER     /* Define ssize_t */

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifdef  _WIN64
typedef __int64    ssize_t;
#else
typedef _W64 int   ssize_t;
#endif

#endif

#include "def.h"
#include <string.h>
#ifdef hpux
#define sigvec sigvector
#endif /* hpux */

#ifdef X_POSIX_C_SOURCE
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <signal.h>
#undef _POSIX_C_SOURCE
#else
#if defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#include <signal.h>
#else
#define _POSIX_SOURCE
#include <signal.h>
#undef _POSIX_SOURCE
#endif
#endif

#include <stdarg.h>

#ifdef MINIX
#define USE_CHMOD   1
#endif

#ifdef DEBUG
int _debugmask;
#endif

char *ProgramName;

#define OBJSUFFIX ".obj"
#define INCLUDEDIR "."

char    *directives[] = {
    "if",
    "ifdef",
    "ifndef",
    "else",
    "endif",
    "define",
    "undef",
    "include",
    "line",
    "pragma",
    "error",
    "ident",
    "sccs",
    "elif",
    "eject",
    NULL
};

#define MAKEDEPEND
#include "imakemdep.h"  /* from config sources */
#undef MAKEDEPEND

/*******   function declarations ********/
/*******   added by -Wall project *******/
void redirect(char * line, char * makefile );

struct  inclist inclist[ MAXFILES ],
        *inclistp = inclist;

struct symhash *maininclist = NULL;

char    *filelist[ MAXFILES ];
char    *includedirs[ MAXDIRS + 1 ];
char    *notdotdot[ MAXDIRS ];
char    *objprefix = "";
char    *objsuffix = OBJSUFFIX;
char    *startat = "# DO NOT DELETE";
int width = 78;
boolean append = FALSE;
boolean printed = FALSE;
boolean verbose = FALSE;
boolean show_where_not = FALSE;
boolean warn_multiple = FALSE;  /* Warn on multiple includes of same file */

static
#ifdef SIGNALRETURNSINT
int
#else
void
#endif
catch (int sig)
{
    fflush (stdout);
    fatalerr ("got signal %d\n", sig);
}

#if (defined(i386) && defined(SYSV)) || defined(WIN32)
#define USGISH
#endif

#ifndef USGISH
#ifndef _POSIX_SOURCE
#define sigaction sigvec
#define sa_handler sv_handler
#define sa_mask sv_mask
#define sa_flags sv_flags
#endif
struct sigaction sig_act;
#endif /* USGISH */

boolean native_win_slashes = FALSE;

int main(argc, argv)
    int argc;
    char    **argv;
{
    register char   **fp = filelist;
    register char   **incp = includedirs;
    register char   *p;
    register struct inclist *ip;
    char    *makefile = NULL;
    struct filepointer  *filecontent;
    struct pair *psymp = predefs;
    char *endmarker = NULL;
    char *defincdir = NULL;
    struct IncludesCollection* incCollection;

    ProgramName = argv[0];

    while (psymp->p_name)
    {
        hash_define(psymp->p_name, psymp->p_value, &maininclist);
        psymp++;
    }
    if (argc == 2 && argv[1][0] == '@') {
        struct stat ast;
        int afd;
        char *args;
        char **nargv;
        int nargc;
        char quotechar = '\0';

        nargc = 1;
        if ((afd = open(argv[1]+1, O_RDONLY)) < 0)
            fatalerr("cannot open \"%s\"\n", argv[1]+1);
        fstat(afd, &ast);
        args = (char *)malloc(ast.st_size + 1);
        if ((ast.st_size = read(afd, args, (size_t) ast.st_size)) < 0)
            fatalerr("failed to read %s\n", argv[1]+1);
        args[ast.st_size] = '\0';
        close(afd);
        for (p = args; *p; p++) {
        if (quotechar) {
            if (quotechar == '\\'
                || (*p == quotechar && p[-1] != '\\'))
                quotechar = '\0';
            continue;
        }
        switch (*p) {
        case '\\':
        case '"':
        case '\'':
            quotechar = *p;
            break;
        case ' ':
        case '\n':
            *p = '\0';
            if (p > args && p[-1])
                nargc++;
            break;
        }
        }
        if (p[-1])
            nargc++;
        nargv = (char **)malloc(nargc * sizeof(char *));
        nargv[0] = argv[0];
        argc = 1;
        for (p = args; argc < nargc; p += strlen(p) + 1)
            if (*p) nargv[argc++] = p;
                argv = nargv;
    }
    for(argc--, argv++; argc; argc--, argv++) {
            /* if looking for endmarker then check before parsing */
        if (endmarker && strcmp (endmarker, *argv) == 0) {
            endmarker = NULL;
            continue;
        }
        if (**argv != '-') {
            /* treat +thing as an option for C++ */
            if (endmarker && **argv == '+')
                continue;
            *fp++ = argv[0];
            continue;
        }
        switch(argv[0][1]) {
        case '-':
            endmarker = &argv[0][2];
            if (endmarker[0] == '\0') endmarker = "--";
            break;
        case 'D':
            if (argv[0][2] == '\0') {
                argv++;
                argc--;
            }
            for (p=argv[0] + 2; *p ; p++)
                if (*p == '=') {
                    *p = ' ';
                    break;
                }
            define(argv[0] + 2, &maininclist);
            break;
        case 'I':
            if (incp >= includedirs + MAXDIRS)
                fatalerr("Too many -I flags.\n");
            *incp++ = argv[0]+2;
            if (**(incp-1) == '\0') {
                *(incp-1) = *(++argv);
                argc--;
            }
            break;
        case 'Y':
            defincdir = argv[0]+2;
            break;
        /* do not use if endmarker processing */
        case 'a':
            if (endmarker) break;
            append = TRUE;
            break;
        case 'w':
            if (endmarker) break;
            if (argv[0][2] == '\0') {
                argv++;
                argc--;
                width = atoi(argv[0]);
            } else
                width = atoi(argv[0]+2);
            break;
        case 'n':
            // Use "-n" switch to generate dependencies with windows-native slash style
            native_win_slashes = TRUE;
            break;
        case 'o':
            if (endmarker) break;
            if (argv[0][2] == '\0') {
                argv++;
                argc--;
                objsuffix = argv[0];
            } else
                objsuffix = argv[0]+2;
            break;
        case 'p':
            if (endmarker) break;
            if (argv[0][2] == '\0') {
                argv++;
                argc--;
                objprefix = argv[0];
            } else
                objprefix = argv[0]+2;
            break;
        case 'v':
            if (endmarker) break;
            verbose = TRUE;
#ifdef DEBUG
            if (argv[0][2])
                _debugmask = atoi(argv[0]+2);
#endif
            break;
        case 's':
            if (endmarker) break;
            startat = argv[0]+2;
            if (*startat == '\0') {
                startat = *(++argv);
                argc--;
            }
            if (*startat != '#')
                fatalerr("-s flag's value should start %s\n",
                    "with '#'.");
            break;
        case 'f':
            if (endmarker) break;
            makefile = argv[0]+2;
            if (*makefile == '\0') {
                makefile = *(++argv);
                argc--;
            }
            break;

        case 'm':
            warn_multiple = TRUE;
            break;

        /* Ignore -O, -g so we can just pass ${CFLAGS} to
           makedepend
         */
        case 'O':
        case 'g':
            break;
        default:
            if (endmarker) break;
            warning("ignoring option %s\n", argv[0]);
        }
    }

    convert_slashes(objprefix);
    objprefix = append_slash(objprefix);

    if (!defincdir) {
#ifdef PREINCDIR
        if (incp >= includedirs + MAXDIRS)
        fatalerr("Too many -I flags.\n");
        *incp++ = PREINCDIR;
#endif
        if (incp >= includedirs + MAXDIRS)
            fatalerr("Too many -I flags.\n");
        *incp++ = INCLUDEDIR;
#ifdef POSTINCDIR
        if (incp >= includedirs + MAXDIRS)
            fatalerr("Too many -I flags.\n");
        *incp++ = POSTINCDIR;
#endif
    } else if (*defincdir) {
        if (incp >= includedirs + MAXDIRS)
            fatalerr("Too many -I flags.\n");
        *incp++ = defincdir;
    }

    redirect(startat, makefile);

    /*
     * catch signals.
     */
#ifdef USGISH
/*  should really reset SIGINT to SIG_IGN if it was.  */
#ifdef SIGHUP
    signal (SIGHUP, catch);
#endif
    signal (SIGINT, catch);
#ifdef SIGQUIT
    signal (SIGQUIT, catch);
#endif
    signal (SIGILL, catch);
#ifdef SIGBUS
    signal (SIGBUS, catch);
#endif
    signal (SIGSEGV, catch);
#ifdef SIGSYS
    signal (SIGSYS, catch);
#endif
    signal (SIGFPE, catch);
#else
    sig_act.sa_handler = catch;
#ifdef _POSIX_SOURCE
    sigemptyset(&sig_act.sa_mask);
    sigaddset(&sig_act.sa_mask, SIGINT);
    sigaddset(&sig_act.sa_mask, SIGQUIT);
#ifdef SIGBUS
    sigaddset(&sig_act.sa_mask, SIGBUS);
#endif
    sigaddset(&sig_act.sa_mask, SIGILL);
    sigaddset(&sig_act.sa_mask, SIGSEGV);
    sigaddset(&sig_act.sa_mask, SIGHUP);
    sigaddset(&sig_act.sa_mask, SIGPIPE);
#ifdef SIGSYS
    sigaddset(&sig_act.sa_mask, SIGSYS);
#endif
#else
    sig_act.sa_mask = ((1<<(SIGINT -1))
               |(1<<(SIGQUIT-1))
#ifdef SIGBUS
               |(1<<(SIGBUS-1))
#endif
               |(1<<(SIGILL-1))
               |(1<<(SIGSEGV-1))
               |(1<<(SIGHUP-1))
               |(1<<(SIGPIPE-1))
#ifdef SIGSYS
               |(1<<(SIGSYS-1))
#endif
               );
#endif /* _POSIX_SOURCE */
    sig_act.sa_flags = 0;
    sigaction(SIGHUP, &sig_act, (struct sigaction *)0);
    sigaction(SIGINT, &sig_act, (struct sigaction *)0);
    sigaction(SIGQUIT, &sig_act, (struct sigaction *)0);
    sigaction(SIGILL, &sig_act, (struct sigaction *)0);
#ifdef SIGBUS
    sigaction(SIGBUS, &sig_act, (struct sigaction *)0);
#endif
    sigaction(SIGSEGV, &sig_act, (struct sigaction *)0);
#ifdef SIGSYS
    sigaction(SIGSYS, &sig_act, (struct sigaction *)0);
#endif
#endif /* USGISH */

    /*
     * now peruse through the list of files.
     */
    incCollection = create_IncludesCollection();

    for(fp=filelist; *fp; fp++) {
        struct symhash *includes;
        filecontent = getfile(*fp);
        ip = newinclude(*fp, (char *)NULL);

        includes = hash_copy( maininclist );
        find_includes(filecontent, ip, ip, 0, FALSE, incCollection, includes);
        hash_free( includes );

        freefile(filecontent);
        recursive_pr_include(ip, ip->i_file, base_name(*fp));
        if (printed)
            fwrite("\n\n", 2, 1, stdout);
        recursive_pr_dummy(ip, ip->i_file);
        inc_clean();
    }
    if (printed)
        printf("\n");

    delete_IncludesCollection(incCollection);

    exit(0);
}

struct filepointer *getfile(file)
    char    *file;
{
    register int    fd;
    struct filepointer  *content;
    struct stat st;
    off_t       size_backup;
    ssize_t     bytes_read;
    unsigned    malloc_size;

    content = (struct filepointer *)malloc(sizeof(struct filepointer));
    if ((fd = open(file, O_RDONLY)) < 0) {
        warning("makedepend:  Cannot open file \"%s\"\n", file);
        content->f_p = content->f_base = content->f_end = (char *)malloc(1);
        *content->f_p = '\0';
        return(content);
    }
    fstat(fd, &st);

    size_backup = st.st_size;
    malloc_size = size_backup;
    /* Since off_t usually is larger than unsigned, need to test for
     * truncation.
     */
    if ( (off_t)malloc_size != size_backup )
    {
        close( fd );
        warning("makedepend:  File \"%s\" is too large.\n", file);
        content->f_p = content->f_base = content->f_end = (char *)malloc(1);
        *content->f_p = '\0';
        return(content);
    }

    content->f_base = (char *)malloc(malloc_size+1);
    if (content->f_base == NULL)
        fatalerr("makedepend:  Cannot allocate memory to process file \"%s\"\n", file);
    if ((bytes_read = read(fd, content->f_base, malloc_size)) < 0)
        if ( st.st_mode & S_IFREG )
            fatalerr("makedepend:  Failed to read file \"%s\"\n", file);

    close(fd);
    content->f_len = bytes_read+1;
    content->f_p = content->f_base;
    content->f_end = content->f_base + bytes_read;
    *content->f_end = '\0';
    content->f_line = 0;
    return(content);
}

void freefile(fp)
    struct filepointer  *fp;
{
    free(fp->f_base);
    free(fp);
}

char *copy(str)
    register char   *str;
{
    register char   *p = (char *)malloc(strlen(str) + 1);

    strcpy(p, str);
    return(p);
}

int match(str, list)
    register char   *str, **list;
{
    register int    i;

    for (i=0; *list; i++, list++)
        if (strcmp(str, *list) == 0)
            return(i);
    return(-1);
}

/*
 * Get the next line.  We only return lines beginning with '#' since that
 * is all this program is ever interested in.
 */
char *get_line(filep)
    register struct filepointer *filep;
{
    register char   *p, /* walking pointer */
            *eof,   /* end of file pointer */
            *bol;   /* beginning of line pointer */
    register int    lineno; /* line number */

    p = filep->f_p;
    eof = filep->f_end;
    if (p >= eof)
        return((char *)NULL);
    lineno = filep->f_line;

    for(bol = p--; ++p < eof; ) {
        if (*p == '/' && *(p+1) == '*') { /* consume comments */
            *p++ = ' ', *p++ = ' ';
            while (*p) {
                if (*p == '*' && *(p+1) == '/') {
                    *p++ = ' ', *p = ' ';
                    break;
                }
                else if (*p == '\n')
                    lineno++;
                *p++ = ' ';
            }
            continue;
        }
        else if (*p == '/' && *(p+1) == '/') { /* consume comments */
            *p++ = ' ', *p++ = ' ';
            while (*p && *p != '\n')
                *p++ = ' ';
            if ( *p == '\n' )
                p--;
            lineno++;
            continue;
        }
        else if (*p == '\\') {
            if (*(p+1) == '\n') {
                *p = ' ';
                *(p+1) = ' ';
                lineno++;
            }
        }
        else if (*p == '\n') {
            lineno++;
            if (*bol == '#') {
                register char *cp;

                *p++ = '\0';
                /* punt lines with just # (yacc generated) */
                for (cp = bol+1;
                     *cp && (*cp == ' ' || *cp == '\t'); cp++);
                if (*cp) goto done;
            }
            bol = p+1;
        }
    }
    if (*bol != '#')
        bol = NULL;
done:
    filep->f_p = p;
    filep->f_line = lineno;
    return(bol);
}

/*
 * Strip the file name down to what we want to see in the Makefile.
 * It will have objprefix and objsuffix around it.
 */
char *base_name(file)
    register char   *file;
{
    register char   *p;

    file = copy(file);
    for(p=file+strlen(file); p>file && *p != '.'; p--) ;

    if (*p == '.')
        *p = '\0';

    while (p > file) {
        if ( *p == '/' ||  *p == '\\') {
            file = p + 1;
            break;
        };
        p--;
    };
    return(file);
}

#if defined(USG) && !defined(CRAY) && !defined(SVR4)
int rename (from, to)
    char *from, *to;
{
    (void) unlink (to);
    if (link (from, to) == 0) {
    unlink (from);
    return 0;
    } else {
    return -1;
    }
}
#endif /* USGISH */

void redirect(line, makefile)
    char    *line,
        *makefile;
{
    FILE    *fdout;
    fdout = freopen(makefile, "wb", stdout); // binary mode please
    if (fdout == NULL)
        fatalerr("cannot open \"%s\"\n", makefile);
    (void) line;

    // don't need any of that nonsense
#if 0
    struct stat st;
    FILE    *fdin, *fdout;
    char    backup[ BUFSIZ ],
        buf[ BUFSIZ ];
    boolean found = FALSE;
    int len;

    /*
     * if makefile is "-" then let it pour onto stdout.
     */
    if (makefile && *makefile == '-' && *(makefile+1) == '\0')
        return;

    /*
     * use a default makefile is not specified.
     */
    if (!makefile) {
        if (stat("Makefile", &st) == 0)
            makefile = "Makefile";
        else if (stat("makefile", &st) == 0)
            makefile = "makefile";
        else
            fatalerr("[mM]akefile is not present\n");
    }
    else
        stat(makefile, &st);
    if ((fdin = fopen(makefile, "r")) == NULL)
        fatalerr("cannot open \"%s\"\n", makefile);
    sprintf(backup, "%s.bak", makefile);
    unlink(backup);
#if defined(WIN32)
    fclose(fdin);
#endif
    if (rename(makefile, backup) < 0)
        fatalerr("cannot rename %s to %s\n", makefile, backup);
#if defined(WIN32)
    if ((fdin = fopen(backup, "r")) == NULL)
        fatalerr("cannot open \"%s\"\n", backup);
#endif
    if ((fdout = freopen(makefile, "w", stdout)) == NULL)
        fatalerr("cannot open \"%s\"\n", backup);
    len = strlen(line);
    while (!found && fgets(buf, BUFSIZ, fdin)) {
        if (*buf == '#' && strncmp(line, buf, len) == 0)
            found = TRUE;
        fputs(buf, fdout);
    }
    if (!found) {
        if (verbose)
            warning("Adding new delimiting line \"%s\" and dependencies...\n",
                line);
        puts(line); /* same as fputs(fdout); but with newline */
    } else if (append) {
        while (fgets(buf, BUFSIZ, fdin)) {
        fputs(buf, fdout);
        }
    }
    fflush(fdout);
#if defined(USGISH) || defined(USE_CHMOD)
    chmod(makefile, st.st_mode);
#else
    fchmod(fileno(fdout), st.st_mode);
#endif /* USGISH */
    fclose(fdin);
#endif
}

void fatalerr(char *msg, ...)
{
    va_list args;
    fprintf(stderr, "%s: error:  ", ProgramName);
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    exit (1);
}

void warning(char *msg, ...)
{
#ifdef DEBUG_MKDEPEND
    va_list args;
    fprintf(stderr, "%s: warning:  ", ProgramName);
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
#else
    (void)msg;
#endif /* DEBUG_MKDEPEND */
}

void warning1(char *msg, ...)
{
#ifdef DEBUG_MKDEPEND
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
#else
    (void)msg;
#endif /* DEBUG_MKDEPEND */
}

void convert_slashes(path)
    char* path;
{
#if defined (WNT)
    /*
     * Convert backslashes to slashes
     */
    char *ptr;
    if (native_win_slashes) {
        for (ptr = (char*)path; *ptr; ++ptr)
            if (*ptr == '/')
                *ptr = '\\';
    } else {
        for (ptr = (char*)path; *ptr; ++ptr)
            if (*ptr == '\\')
                *ptr = '/';
    };
#else
    (void)path;
#endif
}

char* append_slash(path)
    char* path;
{
    char *new_string;
    if ((path[strlen(path) - 1] == '/') || (path[strlen(path) - 1] == '\\')) {
        new_string = path;
    } else {
        new_string = (char*)malloc(sizeof(char) * (strlen(path) + 2));
        strcpy(new_string, path);
        if (native_win_slashes)
            strcat(new_string, "\\");
        else
            strcat(new_string, "/");
    };
    return new_string;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
