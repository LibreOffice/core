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

#include "def.h"
char *hash_lookup( char *symbol, struct symhash *symbols );
void hash_undefine( char *symbol, struct symhash *symbols );
int gobble( register struct filepointer *filep, struct inclist *file,
    struct inclist *file_red, struct symhash *symbols );
int deftype ( register char *line, register struct filepointer *filep,
    register struct inclist *file_red, register struct inclist *file,
    int parse_it, struct symhash *symbols);
int zero_value(register char *exp, register struct filepointer *filep,
    register struct inclist *file_red, register struct symhash *symbols);

extern char *directives[];
extern struct symhash *maininclist;

int find_includes(struct filepointer *filep, struct inclist *file, struct inclist *file_red, int recursion, boolean failOK, struct IncludesCollection* incCollection, struct symhash *symbols)
{
    register char   *line;
    register int    type;
    boolean recfailOK;

    while ((line = get_line(filep))) {
        switch(type = deftype(line, filep, file_red, file, TRUE, symbols)) {
        case IF:
        doif:
            type = find_includes(filep, file,
                file_red, recursion+1, failOK, incCollection, symbols);
            while ((type == ELIF) || (type == ELIFFALSE) ||
                   (type == ELIFGUESSFALSE))
                type = gobble(filep, file, file_red, symbols);
            if (type == ELSE)
                gobble(filep, file, file_red, symbols);
            break;
        case IFFALSE:
        case IFGUESSFALSE:
            doiffalse:
            if (type == IFGUESSFALSE || type == ELIFGUESSFALSE)
                recfailOK = TRUE;
            else
                recfailOK = failOK;
            type = gobble(filep, file, file_red, symbols);
            if (type == ELSE)
                find_includes(filep, file,
                      file_red, recursion+1, recfailOK, incCollection, symbols);
            else if (type == ELIF)
                goto doif;
            else if ((type == ELIFFALSE) || (type == ELIFGUESSFALSE))
                goto doiffalse;
            break;
        case IFDEF:
        case IFNDEF:
            if ((type == IFDEF && hash_lookup(line, symbols))
             || (type == IFNDEF && !hash_lookup(line, symbols))) {
                debug(1,(type == IFNDEF ?
                    "line %d: %s !def'd in %s via %s%s\n" : "",
                    filep->f_line, line,
                    file->i_file, file_red->i_file, ": doit"));
                type = find_includes(filep, file,
                    file_red, recursion+1, failOK, incCollection, symbols);
                while (type == ELIF || type == ELIFFALSE || type == ELIFGUESSFALSE)
                    type = gobble(filep, file, file_red, symbols);
                if (type == ELSE)
                    gobble(filep, file, file_red, symbols);
            }
            else {
                debug(1,(type == IFDEF ?
                    "line %d: %s !def'd in %s via %s%s\n" : "",
                    filep->f_line, line,
                    file->i_file, file_red->i_file, ": gobble"));
                type = gobble(filep, file, file_red, symbols);
                if (type == ELSE)
                    find_includes(filep, file,
                        file_red, recursion + 1, failOK, incCollection, symbols);
                else if (type == ELIF)
                        goto doif;
                else if (type == ELIFFALSE || type == ELIFGUESSFALSE)
                        goto doiffalse;
            }
            break;
        case ELSE:
        case ELIFFALSE:
        case ELIFGUESSFALSE:
        case ELIF:
            if (!recursion)
                gobble(filep, file, file_red, symbols);
        case ENDIF:
            if (recursion)
                return(type);
        case DEFINE:
            define(line, &symbols);
            break;
        case UNDEF:
            if (!*line) {
                warning("%s, line %d: incomplete undef == \"%s\"\n",
                file_red->i_file, filep->f_line, line);
                break;
            }
            hash_undefine(line, symbols);
            break;
        case INCLUDE:
            add_include(filep, file, file_red, line, FALSE, failOK, incCollection, symbols);
            break;
        case INCLUDEDOT:
            add_include(filep, file, file_red, line, TRUE, failOK, incCollection, symbols);
            break;
        case ERROR:
                warning("%s: %d: %s\n", file_red->i_file,
                 filep->f_line, line);
                break;

        case PRAGMA:
        case IDENT:
        case SCCS:
        case EJECT:
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
    return(-1);
}

int gobble(filep, file, file_red, symbols)
    register struct filepointer *filep;
    struct inclist      *file, *file_red;
    struct symhash      *symbols;
{
    register char   *line;
    register int    type;

    while ((line = get_line(filep))) {
        switch(type = deftype(line, filep, file_red, file, FALSE, symbols)) {
        case IF:
        case IFFALSE:
        case IFGUESSFALSE:
        case IFDEF:
        case IFNDEF:
            type = gobble(filep, file, file_red, symbols);
            while ((type == ELIF) || (type == ELIFFALSE) ||
                   (type == ELIFGUESSFALSE))
                type = gobble(filep, file, file_red, symbols);
            if (type == ELSE)
                    (void)gobble(filep, file, file_red, symbols);
            break;
        case ELSE:
        case ENDIF:
            debug(0,("%s, line %d: #%s\n",
                file->i_file, filep->f_line,
                directives[type]));
            return(type);
        case DEFINE:
        case UNDEF:
        case INCLUDE:
        case INCLUDEDOT:
        case PRAGMA:
        case ERROR:
        case IDENT:
        case SCCS:
        case EJECT:
            break;
        case ELIF:
        case ELIFFALSE:
        case ELIFGUESSFALSE:
            return(type);
        case -1:
            warning("%s, line %d: unknown directive == \"%s\"\n",
                file_red->i_file, filep->f_line, line);
            break;
        }
    }
    return(-1);
}

/*
 * Decide what type of # directive this line is.
 */
int deftype (line, filep, file_red, file, parse_it, symbols)
    register char   *line;
    register struct filepointer *filep;
    register struct inclist *file_red, *file;
    int parse_it;
    struct symhash  *symbols;
{
    register char   *p;
    char    *directive, savechar;
    register int    ret;

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
        ret = zero_value(p, filep, file_red, symbols);
        if (ret != IF)
        {
        debug(0,("false...\n"));
        if (ret == IFFALSE)
            return(ELIFFALSE);
        else
            return(ELIFGUESSFALSE);
        }
        else
        {
        debug(0,("true...\n"));
        return(ELIF);
        }
    }

    if (ret < 0 || ! parse_it)
        return(ret);

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
        ret = zero_value(p, filep, file_red, symbols);
        debug(0,("%s, line %d: %s #if %s\n",
             file->i_file, filep->f_line, ret?"false":"true", p));
        break;
    case IFDEF:
    case IFNDEF:
        debug(0,("%s, line %d: #%s %s\n",
            file->i_file, filep->f_line, directives[ret], p));
    case UNDEF:
        /*
         * separate the name of a single symbol.
         */
        while (isalnum(*p) || *p == '_')
            *line++ = *p++;
        *line = '\0';
        break;
    case INCLUDE:
        debug(2,("%s, line %d: #include %s\n",
            file->i_file, filep->f_line, p));

        /* Support ANSI macro substitution */
        {
            char *sym = hash_lookup(p, symbols);
            while (sym)
            {
                p = sym;
                debug(3,("%s : #includes SYMBOL %s\n",
                            file->i_incstring,
                            sym));
                /* mark file as having included a 'soft include' */
                file->i_included_sym = TRUE;
                sym = hash_lookup(p, symbols);
            }
        }

        /*
         * Separate the name of the include file.
         */
        while (*p && *p != '"' && *p != '<')
            p++;
        if (! *p)
            return(-2);
        if (*p++ == '"') {
            ret = INCLUDEDOT;
            while (*p && *p != '"')
                *line++ = *p++;
        } else
            while (*p && *p != '>')
                *line++ = *p++;
        *line = '\0';
        break;
    case DEFINE:
        /*
         * copy the definition back to the beginning of the line.
         */
        memmove (line, p, strlen(p));
        break;
    case ELSE:
    case ENDIF:
    case ELIF:
    case PRAGMA:
    case ERROR:
    case IDENT:
    case SCCS:
    case EJECT:
        debug(0,("%s, line %d: #%s\n",
            file->i_file, filep->f_line, directives[ret]));
        /*
         * nothing to do.
         */
        break;
    }
    return(ret);
}

/*
 * HACK! - so that we do not have to introduce 'symbols' in each cppsetup.c
 * function...  It's safe, functions from cppsetup.c don't return here.
 */
struct symhash *global_symbols = NULL;

char * isdefined( symbol )
    register char *symbol;
{
    return hash_lookup( symbol, global_symbols );
}

/*
 * Return type based on if the #if expression evaluates to 0
 */
int zero_value(exp, filep, file_red, symbols)
    register char   *exp;
    register struct filepointer *filep;
    register struct inclist *file_red;
    register struct symhash *symbols;
{
    global_symbols = symbols; /* HACK! see above */
    if (cppsetup(exp, filep, file_red))
        return(IFFALSE);
    else
        return(IF);
}

void define( def, symbols )
    char            *def;
    struct symhash **symbols;
{
    char *val;

    /* Separate symbol name and its value */
    val = def;
    while (isalnum(*val) || *val == '_')
        val++;
    if (*val)
        *val++ = '\0';
    while (*val == ' ' || *val == '\t')
        val++;

    if (!*val)
        val = "1";
    hash_define( def, val, symbols );
}

static int hash( str )
    register char *str;
{
    /* Hash (Kernighan and Ritchie) */
    register unsigned int hashval = 0;

    for ( ; *str; str++ )
    {
        hashval = ( hashval * SYMHASHSEED ) + ( *str );
    }

    return hashval & ( SYMHASHMEMBERS - 1 );
}

struct symhash *hash_copy( symbols )
    struct symhash *symbols;
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

void hash_free( symbols )
    struct symhash *symbols;
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

void hash_define( name, val, symbols )
    char            *name, *val;
    struct symhash **symbols;
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

char *hash_lookup( symbol, symbols )
    char           *symbol;
    struct symhash *symbols;
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

void hash_undefine( symbol, symbols )
    char           *symbol;
    struct symhash *symbols;
{
    int hashval;
    struct pair *it;

    if ( !symbols )
        return;

    hashval = hash( symbol );
    it = symbols->s_pairs[ hashval ];

    /* Replace/insert the symbol */
    if ( it == NULL )
        return;
    else if ( strcmp( it->p_name, symbol ) == 0 )
    {
        if ( it->p_next )
        {
            struct pair *tmp;
            it->p_name = it->p_next->p_name;
            it->p_value = it->p_next->p_value;
            tmp = it->p_next->p_next;
            free( it->p_next );
            it->p_next = tmp;
        }
        else
        {
            free( it );
            symbols->s_pairs[ hashval ] = NULL;
        }
    }
    else
    {
        while ( it->p_next && ( strcmp( it->p_next->p_name, symbol ) != 0 ) )
        {
            it = it->p_next;
        }
        if ( it->p_next )
        {
            struct pair *tmp = it->p_next;
            it->p_next = it->p_next->p_next;
            free( tmp );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
