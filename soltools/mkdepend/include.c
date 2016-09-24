/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* $XConsortium: include.c,v 1.17 94/12/05 19:33:08 gildea Exp $ */
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

void remove_dotdot( char * );
int isdot( char * );
int isdotdot( char * );
int issymbolic(char * dir, char * component);
int exists_path(struct IncludesCollection*, char*);


extern struct inclist inclist[ MAXFILES ],
                      *inclistp;
extern char *includedirs[ ];
extern char *notdotdot[ ];
extern boolean show_where_not;
extern boolean warn_multiple;

struct inclist *inc_path(char *file, char *include, boolean dot, struct IncludesCollection *incCollection)
{
    static char path[ BUFSIZ ];
    char   **pp, *p;
    struct inclist *ip;
    struct stat st;
    boolean found = FALSE;
    (void)dot;

    /*
     * Check all previously found include files for a path that
     * has already been expanded.
     */
    for (ip = inclist; ip->i_file; ip++)
        if ((strcmp(ip->i_incstring, include) == 0) && !ip->i_included_sym)
        {
          found = TRUE;
          break;
        }

    /*
     * If the path was surrounded by "" or is an absolute path,
     * then check the exact path provided.
     */
// FIXME: creates duplicates in the dependency files if absolute paths are
// given, which certainly is not the intended behavior. Also it slows down
// makedepend performance considerably.
//  if (!found && (dot || *include == '/')) {
//
//      if ((exists_path(incCollection, include)) && stat(include, &st) == 0 && !( st.st_mode & S_IFDIR)) {
//          ip = newinclude(include, include);
//          found = TRUE;
//      }
//      else if (show_where_not)
//          warning1("\tnot in %s\n", include);
//  }

    /*
     * See if this include file is in the directory of the
     * file being compiled.
     */
    if (!found) {
        for (p=file+strlen(file); p>file; p--)
            if (*p == '/')
                break;
        if (p == file)
        {
            if(strlen(include) >= BUFSIZ )
            {
                fatalerr("include filename too long \"%s\"\n", include);
            }
            else
            {
                strcpy(path, include);
            }
        }
        else
        {
            int partial = (p - file);
            size_t inc_len = strlen(include);
            if(inc_len + partial >= BUFSIZ )
            {
                fatalerr("include filename too long \"%s\"\n", include);
            }
            else
            {
                memcpy(path, file, partial);
                memcpy(path + partial, include, inc_len);
                path[partial + inc_len] = 0;
            }
        }
        remove_dotdot(path);
        if ((exists_path(incCollection, path)) && stat(path, &st) == 0 && !( st.st_mode & S_IFDIR)) {
            ip = newinclude(path, include);
            found = TRUE;
        }
        else if (show_where_not)
            warning1("\tnot in %s\n", path);
    }

    /*
     * Check the include directories specified. (standard include dir
     * should be at the end.)
     */
    if (!found)
        for (pp = includedirs; *pp; pp++) {
            sprintf(path, "%s/%s", *pp, include);
            remove_dotdot(path);
            if ((exists_path(incCollection, path)) && stat(path, &st) == 0 && !(st.st_mode & S_IFDIR)) {
                ip = newinclude(path, include);
                found = TRUE;
                break;
            }
            else if (show_where_not)
                warning1("\tnot in %s\n", path);
        }

    if (!found)
        ip = NULL;
    return ip;
}

int exists_path(struct IncludesCollection *incCollection, char *path)
{
    convert_slashes(path);
    return call_IncludesCollection_exists(incCollection, path);
}

/*
 * Occasionally, pathnames are created that look like .../x/../y
 * Any of the 'x/..' sequences within the name can be eliminated.
 * (but only if 'x' is not a symbolic link!!)
 */
void remove_dotdot(char *path)
{
    char    *end, *from, *to, **cp;
    char    *components[ MAXFILES ],
            newpath[ BUFSIZ ];
    boolean     component_copied;

    /*
     * slice path up into components.
     */
    to = newpath;
    if (*path == '/')
        *to++ = '/';
    *to = '\0';
    cp = components;
    for (from=end=path; *end; end++)
        if (*end == '/') {
            while (*end == '/')
                *end++ = '\0';
            if (*from)
                *cp++ = from;
            from = end;
        }
    *cp++ = from;
    *cp = NULL;

    /*
     * Recursively remove all 'x/..' component pairs.
     */
    cp = components;
    while(*cp) {
        if (!isdot(*cp) && !isdotdot(*cp) && isdotdot(*(cp+1))
            && !issymbolic(newpath, *cp))
        {
            char **fp = cp + 2;
            char **tp = cp;

            do {
              *tp++ = *fp; /* move all the pointers down */
            } while (*fp++);
            if (cp != components)
            cp--;   /* go back and check for nested ".." */
        } else {
            cp++;
        }
    }
    /*
     * Concatenate the remaining path elements.
     */
    cp = components;
    component_copied = FALSE;
    while(*cp) {
        if (component_copied)
            *to++ = '/';
        component_copied = TRUE;
        for (from = *cp; *from; )
            *to++ = *from++;
        *to = '\0';
        cp++;
    }
    *to++ = '\0';

    /*
     * copy the reconstituted path back to our pointer.
     */
    strcpy(path, newpath);
}

int isdot(char *p)
{
    if(p && p[0] == '.' && p[1] == '\0')
        return TRUE;
    return FALSE;
}

int isdotdot(char *p)
{
    if(p && p[0] == '.' && p[1] == '.' && p[2] == '\0')
        return TRUE;
    return FALSE;
}

int issymbolic(char *dir, char *component)
{
#ifdef S_IFLNK
    struct stat st;
    char    buf[ BUFSIZ ], **pp;

    sprintf(buf, "%s%s%s", dir, *dir ? "/" : "", component);
    for (pp=notdotdot; *pp; pp++)
        if (strcmp(*pp, buf) == 0)
            return TRUE;
    if (lstat(buf, &st) == 0
    && (st.st_mode & S_IFMT) == S_IFLNK) {
        *pp++ = copy(buf);
        if (pp >= &notdotdot[ MAXDIRS ])
            fatalerr("out of .. dirs, increase MAXDIRS\n");
        return TRUE;
    }
#else
    (void)dir; (void)component;
#endif
    return FALSE;
}

/*
 * Add an include file to the list of those included by 'file'.
 */
struct inclist *newinclude(char *newfile, char *incstring)
{
    struct inclist *ip;

    /*
     * First, put this file on the global list of include files.
     */
    ip = inclistp++;
    if (inclistp == inclist + MAXFILES - 1)
        fatalerr("out of space: increase MAXFILES\n");
    ip->i_file = copy(newfile);
    ip->i_included_sym = FALSE;
    if (incstring == NULL)
        ip->i_incstring = ip->i_file;
    else
        ip->i_incstring = copy(incstring);

    return ip;
}

void included_by(struct inclist *ip, struct inclist *newfile)
{
    int i;

    if (ip == NULL)
        return;
    /*
     * Put this include file (newfile) on the list of files included
     * by 'file'.  If 'file' is NULL, then it is not an include
     * file itself (i.e. was probably mentioned on the command line).
     * If it is already on the list, don't stick it on again.
     */
    if (ip->i_list == NULL)
        ip->i_list = (struct inclist **)
            malloc(sizeof(struct inclist *) * ++ip->i_listlen);
    else {
        for (i=0; i<ip->i_listlen; i++)
            if (ip->i_list[ i ] == newfile) {
                i = (int)strlen(newfile->i_file);
                if (!ip->i_included_sym &&
                !(i > 2 &&
                  newfile->i_file[i-1] == 'c' &&
                  newfile->i_file[i-2] == '.'))
                {
                /* only complain if ip has */
                /* no #include SYMBOL lines  */
                /* and is not a .c file */
                if (warn_multiple)
                {
                    warning("%s includes %s more than once!\n",
                        ip->i_file, newfile->i_file);
                    warning1("Already have\n");
                    for (i=0; i<ip->i_listlen; i++)
                        warning1("\t%s\n", ip->i_list[i]->i_file);
                }
                }
                return;
            }
        ip->i_list = (struct inclist **) realloc(ip->i_list,
            sizeof(struct inclist *) * ++ip->i_listlen);
    }
    ip->i_list[ ip->i_listlen-1 ] = newfile;
}

void inc_clean (void)
{
    struct inclist *ip;

    for (ip = inclist; ip < inclistp; ip++) {
        ip->i_marked = FALSE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
