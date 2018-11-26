/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* $XConsortium: parse.c,v 1.30 94/04/17 20:10:38 gildea Exp $ */
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

#include <ctype.h>

#include "def.h"
static char *hash_lookup( char *symbol, struct symhash *symbols );
static int gobble( struct filepointer *filep, struct inclist *file,
    struct inclist *file_red, struct symhash *symbols );
static int deftype ( char *line, struct filepointer *filep, struct inclist *file,
    int parse_it, struct symhash *symbols);
static int zero_value(char const *exp, struct symhash *symbols);

extern struct symhash *maininclist;

int find_includes(struct filepointer *filep, struct inclist *file, struct inclist *file_red, int recursion, boolean failOK, struct IncludesCollection* incCollection, struct symhash *symbols)
{
    char   *line;
    int    type;

    while ((line = get_line(filep))) {
        type = deftype(line, filep, file, TRUE, symbols);
        switch(type) {
        case IF:
        doif:
            type = find_includes(filep, file,
                file_red, recursion+1, failOK, incCollection, symbols);
            while ((type == ELIF) || (type == ELIFFALSE) ||
                   (type == ELIFGUESSFALSE))
                type = gobble(filep, file, file_red, symbols);
            break;
        case IFFALSE:
            doiffalse:
            type = gobble(filep, file, file_red, symbols);
            if (type == ELIF)
                goto doif;
            else if ((type == ELIFFALSE) || (type == ELIFGUESSFALSE))
                goto doiffalse;
            break;
        case ELIFFALSE:
        case ELIFGUESSFALSE:
        case ELIF:
            if (!recursion)
                gobble(filep, file, file_red, symbols);
            if (recursion)
                return type;
            define(line, &symbols);
            break;
        case ERROR:
                warning("%s: %d: %s\n", file_red->i_file,
                 filep->f_line, line);
                break;

        case -1:
            warning("%s", file_red->i_file);
            if (file_red != file)
                warning1(" (reading %s)", file->i_file);
            warning1(", line %d: unknown directive == \"%s\"\n",
                 filep->f_line, line);
            break;
        case -2:
            warning("%s", file_red->i_file);
            if (file_red != file)
                warning1(" (reading %s)", file->i_file);
            warning1(", line %d: incomplete include == \"%s\"\n",
                 filep->f_line, line);
            break;
        }
    }
    // coverity[leaked_storage] - on purpose
    return -1;
}

int gobble(struct filepointer *filep,
           struct inclist *file,
           struct inclist *file_red,
           struct symhash *symbols)
{
    char   *line;
    int    type;

    while ((line = get_line(filep))) {
        type = deftype(line, filep, file, FALSE, symbols);
        switch(type) {
        case IF:
        case IFFALSE:
            type = gobble(filep, file, file_red, symbols);
            while ((type == ELIF) || (type == ELIFFALSE) ||
                   (type == ELIFGUESSFALSE))
                type = gobble(filep, file, file_red, symbols);
            break;
        case ERROR:
            break;
        case ELIF:
        case ELIFFALSE:
        case ELIFGUESSFALSE:
            return type;
        case -1:
            warning("%s, line %d: unknown directive == \"%s\"\n",
                file_red->i_file, filep->f_line, line);
            break;
        }
    }
    return -1;
}

/*
 * Decide what type of # directive this line is.
 */
int deftype (char *line, struct filepointer *filep, struct inclist * file,
            int parse_it, struct symhash *symbols)
{
    char   *p;
    char    *directive, savechar;
    int    ret;
    (void)file; // used in DEBUG mode
    (void)filep;
    /*
     * Parse the directive...
     */
    directive=line+1;
    while (*directive == ' ' || *directive == '\t')
        directive++;

    p = directive;
    while (*p >= 'a' && *p <= 'z')
        p++;
    savechar = *p;
    *p = '\0';
    ret = match(directive, directives);
    *p = savechar;

    /* If we don't recognize this compiler directive or we happen to just
     * be gobbling up text while waiting for an #endif or #elif or #else
     * in the case of an #elif we must check the zero_value and return an
     * ELIF or an ELIFFALSE.
     */

    if (ret == ELIF && !parse_it)
    {
        while (*p == ' ' || *p == '\t')
            p++;
        /*
         * parse an expression.
         */
        debug(0,("%s, line %d: #elif %s ",
           file->i_file, filep->f_line, p));
        ret = zero_value(p, symbols);
        if (ret != IF)
        {
          debug(0,("false...\n"));
          if (ret == IFFALSE)
              return ELIFFALSE;
          else
              return ELIFGUESSFALSE;
          }
        else
        {
          debug(0,("true...\n"));
          return ELIF;
        }
    }

    if (ret < 0 || ! parse_it)
        return ret;

    /*
     * now decide how to parse the directive, and do it.
     */
    while (*p == ' ' || *p == '\t')
        p++;
    switch (ret) {
    case IF:
        /*
         * parse an expression.
         */
        ret = zero_value(p, symbols);
        debug(0,("%s, line %d: %s #if %s\n",
             file->i_file, filep->f_line, ret?"false":"true", p));
        break;
    case ELIF:
    case ERROR:
        debug(0,("%s, line %d: #%s\n",
            file->i_file, filep->f_line, directives[ret]));
        /*
         * nothing to do.
         */
        break;
    }
    return ret;
}

/*
 * HACK! - so that we do not have to introduce 'symbols' in each cppsetup.c
 * function...  It's safe, functions from cppsetup.c don't return here.
 */
static struct symhash *global_symbols = NULL;

char * isdefined( char *symbol )
{
    return hash_lookup( symbol, global_symbols );
}

/*
 * Return type based on if the #if expression evaluates to 0
 */
int zero_value(char const *exp, struct symhash *symbols)
{
    global_symbols = symbols; /* HACK! see above */
    if (cppsetup(exp))
        return IFFALSE;
    else
        return IF;
}

void define( char *def, struct symhash **symbols )
{
    char *val;

    /* Separate symbol name and its value */
    val = def;
    while (isalnum((unsigned char)*val) || *val == '_')
        val++;
    if (*val)
        *val++ = '\0';
    while (*val == ' ' || *val == '\t')
        val++;

    if (!*val)
        val = "1";
    hash_define( def, val, symbols );
}

static int hash( char *str )
{
    /* Hash (Kernighan and Ritchie) */
    unsigned int hashval = 0;

    for ( ; *str; str++ )
    {
        hashval = ( hashval * SYMHASHSEED ) + ( *str );
    }

    return hashval & ( SYMHASHMEMBERS - 1 );
}

struct symhash *hash_copy( struct symhash *symbols )
{
    int i;
    struct symhash *newsym;
    if ( !symbols )
        return NULL;

    newsym = (struct symhash *) malloc( sizeof( struct symhash ) );

    for ( i = 0; i < SYMHASHMEMBERS; ++i )
    {
        if ( !symbols->s_pairs[ i ] )
            newsym->s_pairs[ i ] = NULL;
        else
        {
            struct pair *it = symbols->s_pairs[ i ];
            struct pair *nw = newsym->s_pairs[ i ] = (struct pair*) malloc( sizeof( struct pair ) );
            nw->p_name = it->p_name;
            nw->p_value = it->p_value;
            nw->p_next = NULL;

            while ( it->p_next )
            {
                nw->p_next = (struct pair*) malloc( sizeof( struct pair ) );
                it = it->p_next;
                nw = nw->p_next;
                nw->p_name = it->p_name;
                nw->p_value = it->p_value;
                nw->p_next = NULL;
            }
        }
    }
    return newsym;
}

void hash_free( struct symhash *symbols )
{
    int i;

    if ( !symbols )
        return;

    for ( i = 0; i < SYMHASHMEMBERS; ++i )
    {
        struct pair *it = symbols->s_pairs[ i ];
        struct pair *next;
        while ( it )
        {
            next = it->p_next;
            free( it );
            it = next;
        }
    }
    free( symbols->s_pairs );
}

void hash_define( char *name, char const *val, struct symhash **symbols )
{
    int hashval;
    struct pair *it;

    if ( !symbols )
        return;

    /* Make space if it's needed */
    if ( *symbols == NULL )
    {
        int i;

        *symbols = (struct symhash *) malloc( sizeof( struct symhash ) );
        if ( *symbols == NULL )
            fatalerr( "malloc()/realloc() failure in insert_defn()\n" );

        for ( i = 0; i < SYMHASHMEMBERS; ++i )
            (*symbols)->s_pairs[i] = NULL;
    }

    hashval = hash( name );
    it = (*symbols)->s_pairs[ hashval ];

    /* Replace/insert the symbol */
    if ( it == NULL )
    {
        it = (*symbols)->s_pairs[ hashval ] = (struct pair*) malloc( sizeof( struct pair ) );
        it->p_name = copy( name );
        it->p_value = copy( val );
        it->p_next = NULL;
    }
    else if ( strcmp( it->p_name, name ) == 0 )
    {
        it->p_value = copy( val );
    }
    else
    {
        while ( it->p_next && ( strcmp( it->p_next->p_name, name ) != 0 ) )
        {
            it = it->p_next;
        }
        if ( it->p_next )
            it->p_next->p_name = copy( name );
        else
        {
            it->p_next = (struct pair*) malloc( sizeof( struct pair ) );
            it->p_next->p_name = copy( name );
            it->p_next->p_value = copy( val );
            it->p_next->p_next = NULL;
        }
    }
}

char *hash_lookup( char *symbol, struct symhash *symbols )
{
    struct pair *it;

    if ( !symbols )
        return NULL;

    it = symbols->s_pairs[ hash( symbol ) ];

    while ( it && ( strcmp( it->p_name, symbol ) != 0 ) )
    {
        it = it->p_next;
    }
    if ( it )
        return it->p_value;

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
