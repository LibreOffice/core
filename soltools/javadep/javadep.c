/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

/* All Java Virtual Machine Specs are from
 * "The Java Virtual Machine Specification", T. Lindholm, F. Yellin
 * (JVMS)
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#if defined(UNX) || defined(OS2)
#include <unistd.h>
#include <netinet/in.h>     /* ntohl(), ntohs() */
#elif defined(WNT)
#include <io.h>
#define access      _access
#define vsnprintf   _vsnprintf
#define CDECL       _cdecl
#define F_OK        00
#define PATH_MAX    _MAX_PATH
#define ntohl(x)    ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
                        (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define ntohs(x)    ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#endif

#if defined(OS2)
#define CDECL
#endif

/* max. length of line in response file */
#define RES_FILE_BUF    65536

struct file {
    char *pname;
    FILE *pfs;
};

struct growable {
    int     ncur;
    int     nmax;
    char    **parray;
};

typedef struct file     file_t;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

struct utf8 {
    uint16  nlen;
    void    *pdata;
};

typedef struct utf8     utf8_t;

/* The contents of the Constant_pool is described in JVMS p. 93
 */
enum {
    CONSTANT_Class              = 7,
    CONSTANT_Fieldref           = 9,
    CONSTANT_Methodref          = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_String             = 8,
    CONSTANT_Integer            = 3,
    CONSTANT_Float              = 4,
    CONSTANT_Long               = 5,
    CONSTANT_Double             = 6,
    CONSTANT_NameAndType        = 12,
    CONSTANT_Utf8               = 1
};

enum { NGROW_INIT = 10, NGROW = 2 };

static char     *pprogname  = "javadep";
static char     csep        = ';';
#if defined (UNX) || defined(OS2)
#define CDECL
static char     cpathsep    = '/';
#elif defined (WNT) || defined(OS2)
static char     cpathsep    = '\\';
#endif
static FILE     *pfsout     = NULL;
static char     *pout_file  = NULL;


/* prototypes */
uint8   read_uint8(const file_t *pfile);
uint16  read_uint16(const file_t *pfile);
uint32  read_uint32(const file_t *pfile);
void    skip_bytes(const file_t *pfile, const size_t nnum);
char    *escape_slash(const char *pstr);
int     is_inner(const char *pstr);
void    print_dependencies(const struct growable *pdep,
                           const char* pclass_file);
void    process_class_file(const char *pfilenamem,
                           const struct growable *pfilt);
char    *utf8tolatin1(const utf8_t a_utf8);
void    *xmalloc(size_t size);
void    *xcalloc(size_t nmemb, size_t size);
void    *xrealloc(void *ptr, size_t size);
void    grow_if_needed (struct growable *pgrow);
int     append_to_growable(struct growable *, char *);
struct growable *allocate_growable(void);
void    free_growable(struct growable *pgrowvoid);
void    create_filters(struct growable *pfilt, const struct growable *pinc);
void    usage(void);
void    err_quit(const char *, ...);
void    silent_quit(void);

#ifdef WNT
/* poor man's getopt() */
int     simple_getopt(char *pargv[], const char *poptstring);
char    *optarg = NULL;
int     optind  = 1;
int     optopt  = 0;
int     opterr  = 0;
#endif

uint8
read_uint8(const file_t *pfile)
{
    /* read a byte from classfile */
    int nread;
    uint8 ndata;
    nread = fread(&ndata, sizeof(uint8), 1, pfile->pfs);
    if ( !nread ) {
        fclose(pfile->pfs);
        err_quit("%s: truncated class file", pfile->pname);
    }
    return ndata;
}

uint16
read_uint16(const file_t *pfile)
{
    /* read a short from classfile and convert it to host format */
    int nread;
    uint16 ndata;
    nread = fread(&ndata, sizeof(uint16), 1, pfile->pfs);
    if ( !nread ) {
        fclose(pfile->pfs);
        err_quit("%s: truncated class file", pfile->pname);
    }
    ndata = ntohs(ndata);
    return ndata;
}

uint32
read_uint32(const file_t *pfile)
{
    /* read an int from classfile and convert it to host format */
    int nread;
    uint32 ndata;
    nread = fread(&ndata, sizeof(uint32), 1, pfile->pfs);
    if ( !nread ) {
        fclose(pfile->pfs);
        err_quit("%s: truncated class file", pfile->pname);
    }
    ndata = ntohl(ndata);
    return ndata;
}

utf8_t
read_utf8(const file_t *pfile)
{
    /* Read a java utf-8-string with uint16 length prependend
     * from class file. Returns utf8 struct
     * with fresh allocated datablock,
     * caller is responsible for freeing.
     * Data is still in network byteorder
     */

    utf8_t  a_utf8;
    int     nread;

    a_utf8.pdata = NULL;

    a_utf8.nlen = read_uint16(pfile);
    if (a_utf8.nlen > 0) {
        a_utf8.pdata = xmalloc(a_utf8.nlen*sizeof(char));
        nread = fread(a_utf8.pdata, a_utf8.nlen*sizeof(char), 1, pfile->pfs);
        if ( !nread ) {
            fclose(pfile->pfs);
            err_quit("%s: truncated class file", pfile->pname);
        }
    }

    return a_utf8;
}

char *utf8tolatin1(const utf8_t a_utf8)
{
    /* function returns fresh allocated zero terminated string,
     * caller is responsible for freeing
     */

    /* JVMS p. 101: the null byte is encoded using a two byte format,
     * Java Virtual Machine Utf8 strings differ in this respect from
     * standard UTF-8 strings
     */

    /* Multibyte data is in network byte order */

    char *p;
    char *pp;
    char *pstr;

    pstr = pp = xmalloc((a_utf8.nlen+1) * sizeof(char));

    for ( p = (char*)a_utf8.pdata;
          p <  (char*)a_utf8.pdata+a_utf8.nlen;
          p++ ) {
        if ( *p & 0x80 ) {
            err_quit("sorry, real UTF8 decoding not yet implemented\n");
        } else {
            *pp++ = *p;
        }
    }
    *pp = '\0';

    return pstr;
}


void
skip_bytes(const file_t *pfile, const size_t nnumber)
{
    /* skip a nnumber of bytes in classfile */
    if ( fseek(pfile->pfs, nnumber, SEEK_CUR) == -1 )
         err_quit("%s: %s", pfile->pname, strerror(errno));
}

void
add_to_dependencies(struct growable *pdep,
                    const struct growable *pfilt,
                    char *pdepstr,
                    const char *pclass_file)
{
    /* create dependencies */
    int i;
    int nlen_filt, nlen_str, nlen_pdepstr;
    char *pstr, *ptrunc;
    char path[PATH_MAX+1];
    char cnp_class_file[PATH_MAX+1];
    char cnp_str[PATH_MAX+1];

    nlen_pdepstr = strlen(pdepstr);
    pstr = xmalloc((nlen_pdepstr+6+1)*sizeof(char));
    memcpy(pstr, pdepstr, nlen_pdepstr+1);
    strncat(pstr, ".class", 6);

    if ( pfilt->ncur == 0 ) { /* no filters */
        if ( access(pstr, F_OK) == 0 ) {
            append_to_growable(pdep, strdup(pstr));
        }
    } else {
        nlen_str    = strlen(pstr);
        for ( i = 0; i < pfilt->ncur; i++ ) {
            nlen_filt   = strlen(pfilt->parray[i]);
            if ( nlen_filt + 1 + nlen_str > PATH_MAX )
                err_quit("path to long");
            memcpy(path, pfilt->parray[i], nlen_filt);
            path[nlen_filt]     = '/';
            memcpy( path+nlen_filt+1, pstr, nlen_str+1);

            if ( access(path, F_OK) != 0 ) {
                free(pstr);
                pstr = NULL;
                return; /* path doesn't represent a real file, don't bother */
            }

            /* get the canonical path */
#if defined (UNX) || defined(OS2)
            if ( !(realpath(pclass_file, cnp_class_file)
                && realpath(path, cnp_str) ) ) {
                err_quit("can't get the canonical path");
            }
#else
            if ( !(_fullpath(cnp_class_file, pclass_file, sizeof(cnp_class_file))
                && _fullpath(cnp_str, path, sizeof(cnp_str)) ) ) {
                err_quit("can't get the canonical path");
            }
#endif

            /* truncate so that only the package prefix remains */
            ptrunc = strrchr(cnp_str, cpathsep);
            *ptrunc = '\0';
            ptrunc = strrchr(cnp_class_file, cpathsep);
            *ptrunc = '\0';

            if ( !strcmp(cnp_str, cnp_class_file) ) {
                free(pstr);
                pstr = NULL;
                return; /* identical, don't bother with this one */
            }

            append_to_growable(pdep, strdup(path));
        }
    }
    free(pstr);
    return;
}

char *
escape_slash(const char *pstr)
{
    /* returns a fresh allocated string with all cpathsep escaped exchanged
     * with "$/"
     *
     * caller is responsible for freeing
     */

    const char *pp = pstr;
    char *p, *pnp;
    char *pnew_str;
    int nlen_pnp, nlen_pp;
    int i = 0;

    while ( (p=strchr(pp, cpathsep)) != NULL ) {
        ++i;
        pp = ++p;
    }

    nlen_pnp = strlen(pstr) + i;
    pnp = pnew_str = xmalloc((nlen_pnp+1) * sizeof(char));

    pp = pstr;

    if ( i > 0 ) {
        while ( (p=strchr(pp, cpathsep)) != NULL ) {
            memcpy(pnp, pp, p-pp);
            pnp += p-pp;
            *pnp++ = '$';
            *pnp++ = '/';
            pp = ++p;
        }
    }
    nlen_pp = strlen(pp);
    memcpy(pnp, pp, nlen_pp+1);

    return pnew_str;
}


void
print_dependencies(const struct growable *pdep, const char* pclass_file)
{
    char *pstr;
    int i;

    pstr = escape_slash(pclass_file);
    fprintf(pfsout, "%s:", pstr);
    free(pstr);

    for( i=0; i<pdep->ncur; ++i) {
        fprintf(pfsout, "  \\\n");
        pstr=escape_slash(pdep->parray[i]);
        fprintf(pfsout, "\t%s", pstr);
        free(pstr);
    }

    fprintf(pfsout,"\n\n");
    return;
}

int
is_inner(const char *pstr)
{
    /* return true if character '$' is found in classname */

    /*
     * note that a '$' in a classname is not an exact indicator
     * for an inner class. Java identifier may legally contain
     * this chararcter, and so may classnames. In the context
     * of javadep this doesn't matter since the makefile system
     * can't cope with classfiles with '$'s in the filename
     * anyway.
     *
     */

    if ( strchr(pstr, '$') != NULL )
        return 1;

    return 0;
}

void
process_class_file(const char *pfilename, const struct growable *pfilt)
{
    /* read class file and extract object information
     * java class files are in bigendian data format
     * (JVMS, p. 83)
     */
    int     i;
    uint32  nmagic;
    uint16  nminor, nmajor;
    uint16  ncnt;
    uint16  nclass_cnt;
    utf8_t* pc_pool;
    uint16* pc_class;
    file_t  file;

    struct growable *pdepen;

    file.pname = (char*)pfilename;

    file.pfs = fopen(file.pname,"rb");
    if ( !file.pfs )
        silent_quit();

    nmagic = read_uint32(&file);

    if ( nmagic != 0xCAFEBABE ) {
        fclose(file.pfs);
        err_quit("%s: invalid magic", file.pname);
    }

    nminor = read_uint16(&file);
    nmajor = read_uint16(&file);

    /* get number of entries in constant pool */
    ncnt = read_uint16(&file);

#ifdef DEBUG
    printf("Magic: %p\n", (void*)nmagic);
    printf("Major %d, Minor %d\n", nmajor, nminor);
    printf("Const_pool_count %d\n", ncnt);
#endif

    /* There can be ncount entries in the constant_pool table
     * so at most ncount-1 of them can be of type CONSTANT_Class
     * (at leat one CONSTANT_Utf8 entry must exist).
     * Usually way less CONSTANT_Class entries exists, of course
     */

    pc_pool = xcalloc(ncnt,sizeof(utf8_t));
    pc_class = xmalloc((ncnt-1)*sizeof(uint16));

    /* pc_pool[0] is reserved to the java virtual machine and does
     * not exist in the class file
     */

    nclass_cnt = 0;

    for (i = 1; i < ncnt; i++) {
        uint8   ntag;
        uint16  nindex;
        utf8_t  a_utf8;

        ntag = read_uint8(&file);

        /* we are only interested in CONSTANT_Class entries and
         * Utf8 string entries, because they might belong to
         * CONSTANT_Class entries
         */
        switch(ntag) {
            case CONSTANT_Class:
                nindex = read_uint16(&file);
                pc_class[nclass_cnt++] = nindex;
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
                skip_bytes(&file, 4L);
                break;
            case CONSTANT_String:
                skip_bytes(&file, 2L);
                break;
            case CONSTANT_Integer:
            case CONSTANT_Float:
                skip_bytes(&file, 4L);
                break;
            case CONSTANT_Long:
            case CONSTANT_Double:
                skip_bytes(&file, 8L);
                /* Long and Doubles take 2(!)
                 * entries in constant_pool_table
                 */
                i++;
                break;
            case CONSTANT_NameAndType:
                skip_bytes(&file, 4L);
                break;
            case CONSTANT_Utf8:
                a_utf8 = read_utf8(&file);
                pc_pool[i] = a_utf8;
                break;
            default:
                /* Unknown Constant_pool entry, this means we are
                 * in trouble
                 */
                err_quit("corrupted class file\n");
                break;

        }
    }

    fclose(file.pfs);

    pdepen = allocate_growable();

    for (i = 0; i < nclass_cnt; i++) {
        char *pstr, *ptmpstr;
        pstr = ptmpstr = utf8tolatin1(pc_pool[pc_class[i]]);
        /* we are not interested in inner classes */
        if ( is_inner(pstr) ) {
            free(pstr);
            pstr = NULL;
            continue;
        }
        /* strip off evt. array indicators */
        if ( *ptmpstr == '[' ) {
            while ( *ptmpstr == '[' )
                ptmpstr++;
            /* we only interested in obj. arrays, which are marked with 'L' */
            if ( *ptmpstr == 'L' ) {
                char *p = pstr;
                pstr = strdup(++ptmpstr);
                /* remove final ';' from object array name */
                pstr[strlen(pstr)-1] = '\0';
                free(p);
            } else {
                free(pstr);
                pstr = NULL;
            }
        }

        if (pstr) {
            add_to_dependencies(pdepen, pfilt, pstr, file.pname);
            free(pstr);
        }
    }

    print_dependencies(pdepen, file.pname);
    free_growable(pdepen);
    pdepen = NULL;

    for (i = 0; i < ncnt; i++)
        free(pc_pool[i].pdata);

    free(pc_class);
    free(pc_pool);
}

void *
xmalloc(size_t size)
{
    void *ptr;

    ptr = malloc(size);

    if ( !ptr )
        err_quit("out of memory");

    return ptr;
}


void *
xcalloc(size_t nmemb, size_t size)
{
    void *ptr;

    ptr = calloc(nmemb, size);

    if ( !ptr )
        err_quit("out of memory");

    return ptr;
}

void *
xrealloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);

    if ( !ptr )
        err_quit("out of memory");

    return ptr;
}

void
err_quit(const char* fmt, ...)
{
    /* No dependency file must be generated for any error condition,
     * just print message and exit.
     */
    va_list args;
    char buffer[PATH_MAX];

    va_start(args, fmt);

    if ( pprogname )
        fprintf(stderr, "%s: ", pprogname);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    fputs(buffer, stderr);
    fputc('\n', stderr);

    va_end(args);

    /* clean up */
    if ( pfsout && pfsout != stdout ) {
        fclose(pfsout);
        unlink(pout_file);
    }
    exit(1);
}

void
silent_quit()
{
    /* In some cases we should just do a silent exit */

    /* clean up */
    if ( pfsout && pfsout != stdout ) {
        fclose(pfsout);
        unlink(pout_file);
    }
    exit(0);
}

int append_to_growable(struct growable *pgrow, char *pstr)
{
    /* append an element pstr to pgrow,
     * return new number of elements
     */
    grow_if_needed(pgrow);
    pgrow->parray[pgrow->ncur++] = pstr;
    return pgrow->ncur;
}

void
grow_if_needed(struct growable *pgrow)
{
    /* grow growable arrays */

    if ( pgrow->ncur >= pgrow->nmax ) {
        pgrow->parray = xrealloc(pgrow->parray,
                           (NGROW*pgrow->nmax)*sizeof(char*));
        pgrow->nmax *= NGROW;
    }
    return;
}

struct growable *allocate_growable(void)
{
    /* allocate an growable array,
     * initialize with NGROW_INIT elements
     */

    struct growable *pgrow;

    pgrow = xmalloc(sizeof(struct growable));
    pgrow->parray = xmalloc(NGROW_INIT*sizeof(char *));
    pgrow->nmax = NGROW_INIT;
    pgrow->ncur = 0;
    return pgrow;
}

void
free_growable(struct growable *pgrow)
{
    int i;
    for( i = 0; i < pgrow->ncur; i++ )
        free(pgrow->parray[i]);
    free(pgrow->parray);
    free(pgrow);
}

void
create_filters(struct growable *pfilt, const struct growable *pinc)
{
    char *p, *pp, *pstr;
    int i, nlen, nlen_pstr;
    /* break up includes into filter list */
    for ( i = 0; i < pinc->ncur; i++ ) {
        pp = pinc->parray[i];

        while ( (p = strchr(pp, csep)) != NULL) {
            nlen = p - pp;
            pstr = xmalloc((nlen+1)*sizeof(char*));
            memcpy(pstr, pp, nlen);
            pstr[nlen] = '\0';
            append_to_growable(pfilt, pstr);
            pp = p + 1;
        }
        nlen_pstr = strlen(pp);
        pstr = xmalloc((nlen_pstr+1)*sizeof(char*));
        memcpy(pstr, pp, nlen_pstr+1);
        append_to_growable(pfilt, pstr);
    }

}

void
usage()
{
    fprintf(stderr,
            "usage: %s [-i|-I includepath ...  -s|-S seperator "
            "-o|-O outpath -v|-V -h|-H] <file> ....\n",
            pprogname);
}

#ifdef WNT
/* my very simple minded implementation of getopt()
 * it's to sad that getopt() is not available everywhere
 * note: this is not a full POSIX conforming getopt()
 */
int simple_getopt(char *pargv[], const char *poptstring)
{
    char *parg = pargv[optind];

    /* skip all response file arguments */
    if ( parg ) {
        while ( *parg == '@' )
            parg = pargv[++optind];

        if ( parg[0] == '-' && parg[1] != '\0' ) {
            char *popt;
            int c = parg[1];
            if ( (popt = strchr(poptstring, c)) == NULL ) {
               optopt = c;
                if ( opterr )
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return '?';
            }
            if ( *(++popt) == ':') {
                if ( parg[2] != '\0' ) {
                    optarg = ++parg;
                } else {
                    optarg = pargv[++optind];
                }
            } else {
                optarg = NULL;
            }
            ++optind;
            return c;
        }
    }
    return -1;
}
#endif

int CDECL
main(int argc, char *argv[])
{
    int     bv_flag = 0;
    struct  growable *presp, *pincs, *pfilters;
    int     c, i, nall_argc;
    char    **pall_argv;

    presp = allocate_growable();

    /* FIXME: cleanup the option parsing */
    /* search for response file, read it */
    for ( i = 1; i < argc; i++ ) {
        char *parg = argv[i];
        char buffer[RES_FILE_BUF];

        if ( *parg == '@' ) {
            FILE *pfile = fopen(++parg, "r");
            if ( !pfile )
                err_quit("%s: %s", parg, strerror(errno));
            while ( !feof(pfile) ) {
                char *p, *token;

                if ( fgets(buffer, RES_FILE_BUF, pfile) ) {;
                    p = buffer;
                    while ( (token = strtok(p, " \t\n")) != NULL ) {
                        p = NULL;
                        append_to_growable(presp, strdup(token));
                    }
                }
            }
            fclose(pfile);
        }
    }

    /* copy all arguments incl. response file in one array
     * for parsing with getopt
     */
    nall_argc = argc + presp->ncur;
    pall_argv = xmalloc((nall_argc+1)*sizeof(char *));
    memcpy(pall_argv, argv, argc*sizeof(char *));
    memcpy(pall_argv+argc, presp->parray, presp->ncur*sizeof(char *));
    *(pall_argv+argc+presp->ncur) = '\0'; /* terminate */

    opterr = 0;
    pincs = allocate_growable();

#ifdef WNT
    while( (c = simple_getopt(pall_argv, ":i:I:s:S:o:OhHvV")) != -1 ) {
#else
    while( (c = getopt(nall_argc, pall_argv, ":i:I:s:S:o:OhHvV")) != -1 ) {
#endif
        switch(c) {
            case 'i':
            case 'I':
                append_to_growable(pincs, strdup(optarg));
                break;
            case 's':
            case 'S':
                csep = optarg[0];
                break;
            case 'o':
            case 'O':
                pout_file = optarg;
                break;
            case 'h':
            case 'H':
                usage();
                return 0;
                break;
            case 'v':
            case 'V':
                bv_flag = 1;
                break;
            case '?':
                if (isprint (optopt))
                    fprintf (stderr,
                             "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                             "Unknown option character `\\x%x'.\n",
                             optopt);
                usage();
                return 1;
                break;
            case ':':
                fprintf(stderr, "Missing parameter.\n");
                usage();
                return 1;
                break;
            default:
                usage();
                return 1;
                break;
        }
    }

    pfilters = allocate_growable();
    create_filters(pfilters, pincs);
    free_growable(pincs);
    pincs = NULL;

    if ( pout_file ) {
        pfsout = fopen(pout_file, "w");
        if ( !pfsout )
            err_quit("%s: %s", pout_file, strerror(errno));
    } else {
        pfsout = stdout;
    }

    /* the remaining arguments are either class file
     * names or response files, ignore response file
     * since they have already been included
     */
    for ( i = optind; i < nall_argc; i++ ) {
        char *parg = pall_argv[i];
        if ( *parg != '@' ) {
            process_class_file(parg, pfilters);
            if ( pfsout != stdout ) {
                if ( bv_flag )
                    printf("Processed %s ...\n", parg);
            }
        }
    }

    free_growable(pfilters);
    pfilters = NULL;
    free(pall_argv);
    pall_argv = NULL;
    free_growable(presp);
    presp = NULL;

    fclose(pfsout);
    exit(0);
}

