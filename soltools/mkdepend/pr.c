/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* $XConsortium: pr.c,v 1.17 94/04/17 20:10:38 gildea Exp $ */
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

#include "def.h"
#include <string.h>
static size_t pr( struct inclist *ip, char *file,char *base);

extern int  width;

void add_include(struct filepointer *filep, struct inclist *file, struct inclist *file_red, char *include, boolean dot, boolean failOK, struct IncludesCollection* incCollection, struct symhash *symbols)
{
    struct inclist *newfile;
    struct filepointer *content;

    /*
     * First decide what the pathname of this include file really is.
     */
    newfile = inc_path(file->i_file, include, dot, incCollection);
    if (newfile == NULL) {
        if (failOK)
            return;
        if (file != file_red)
            warning("%s (reading %s, line %d): ",
                file_red->i_file, file->i_file, filep->f_line);
        else
            warning("%s, line %d: ", file->i_file, filep->f_line);
        warning1("cannot find include file \"%s\"\n", include);
        show_where_not = TRUE;
        newfile = inc_path(file->i_file, include, dot, incCollection);
        show_where_not = FALSE;
    }

    if (!newfile)
        return;

    /* Only add new dependency files if they don't have "/usr/include" in them. */
    if (!(newfile->i_file && strstr(newfile->i_file, "/usr/"))) {
        included_by(file, newfile);
    }

    if (!newfile->i_searched) {
        newfile->i_searched = TRUE;
        content = getfile(newfile->i_file);
        find_includes(content, newfile, file_red, 0, failOK, incCollection, symbols);
        freefile(content);
    }
}

static void pr_dummy(struct inclist const *ip)
{
    fwrite(ip->i_file, strlen(ip->i_file), 1, stdout);
    fwrite(" :\n\n", 4, 1, stdout);
}

void recursive_pr_dummy(struct inclist *head, char *file)
{
    int    i;

    if (head->i_marked == 2)
        return;
    head->i_marked = 2; // it's a large boolean...
    if (head->i_file != file)
        pr_dummy(head);
    for (i=0; i<head->i_listlen; i++)
        recursive_pr_dummy(head->i_list[ i ], file);
}


void recursive_pr_include(struct inclist *head, char *file, char *base)
{
    int    i;

    if (head->i_marked)
        return;
    head->i_marked = TRUE;
    if (head->i_file != file)
        pr(head, file, base);
    for (i=0; i<head->i_listlen; i++)
        recursive_pr_include(head->i_list[ i ], file, base);
}

size_t pr(struct inclist *ip, char *file, char *base)
{
    size_t ret;
    static char *lastfile;
    int    len, i;
    char    buf[ BUFSIZ ];

    printed = TRUE;
    len = (int)strlen(ip->i_file)+4;
    if (file != lastfile) {
        lastfile = file;
        sprintf(buf, "\n%s%s%s: \\\n %s", objprefix, base, objsuffix,
            ip->i_file);
        len = (int)strlen(buf);
    }
    else {
        buf[0] = ' ';
        buf[1] = '\\';
        buf[2] = '\n';
        buf[3] = ' ';
        strcpy(buf+4, ip->i_file);
    }
    ret = fwrite(buf, len, 1, stdout);

    /*
     * If verbose is set, then print out what this file includes.
     */
    if (! verbose || ip->i_list == NULL || ip->i_notified)
        return ret;
    ip->i_notified = TRUE;
    lastfile = NULL;
    printf("\n# %s includes:", ip->i_file);
    for (i=0; i<ip->i_listlen; i++)
        printf("\n#\t%s", ip->i_list[ i ]->i_incstring);
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
