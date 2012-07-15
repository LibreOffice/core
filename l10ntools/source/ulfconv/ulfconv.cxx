/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sal/alloca.h>
#include <sal/macros.h>

#include <rtl/ustring.hxx>

#include <map>
#include <string>

/*****************************************************************************
 * typedefs
 *****************************************************************************/

typedef std::map< const std::string, rtl_TextEncoding > EncodingMap;

struct _pair {
    const char *key;
    rtl_TextEncoding value;
};

static int _pair_compare (const char *key, const _pair *pair);
static const _pair* _pair_search (const char *key, const _pair *base, unsigned int member );


const _pair _ms_encoding_list[] = {
    { "0",       RTL_TEXTENCODING_UTF8        },
    { "1250",    RTL_TEXTENCODING_MS_1250     },
    { "1251",    RTL_TEXTENCODING_MS_1251     },
    { "1252",    RTL_TEXTENCODING_MS_1252     },
    { "1253",    RTL_TEXTENCODING_MS_1253     },
    { "1254",    RTL_TEXTENCODING_MS_1254     },
    { "1255",    RTL_TEXTENCODING_MS_1255     },
    { "1256",    RTL_TEXTENCODING_MS_1256     },
    { "1257",    RTL_TEXTENCODING_MS_1257     },
    { "1258",    RTL_TEXTENCODING_MS_1258     },
    { "874",     RTL_TEXTENCODING_MS_874      },
    { "932",     RTL_TEXTENCODING_MS_932      },
    { "936",     RTL_TEXTENCODING_MS_936      },
    { "949",     RTL_TEXTENCODING_MS_949      },
    { "950",     RTL_TEXTENCODING_MS_950      }
};


/*****************************************************************************
 * fgets that work with unix line ends on Windows
 *****************************************************************************/

char * my_fgets(char *s, int n, FILE *fp)
{
    int i;
    for( i=0; i < n-1; i++ )
    {
        int c = getc(fp);

        if( c == EOF )
            break;

        s[i] = (char) c;

        if( s[i] == '\n' )
        {
            i++;
            break;
        }
    }

    if( i>0 )
    {
        s[i] = '\0';
        return s;
    }
    else
    {
        return NULL;
    }
}

/*****************************************************************************
 * compare function for binary search
 *****************************************************************************/

static int
_pair_compare (const char *key, const _pair *pair)
{
    int result = rtl_str_compareIgnoreAsciiCase( key, pair->key );
    return result;
}

/*****************************************************************************
 * binary search on encoding tables
 *****************************************************************************/

static const _pair*
_pair_search (const char *key, const _pair *base, unsigned int member )
{
    unsigned int lower = 0;
    unsigned int upper = member;
    unsigned int current;
    int comparison;

    /* check for validity of input */
    if ( (key == NULL) || (base == NULL) || (member == 0) )
        return NULL;

    /* binary search */
    while ( lower < upper )
    {
        current = (lower + upper) / 2;
        comparison = _pair_compare( key, base + current );
        if (comparison < 0)
            upper = current;
        else
        if (comparison > 0)
            lower = current + 1;
        else
            return base + current;
    }

    return NULL;
}


/************************************************************************
 * read_encoding_table
 ************************************************************************/

void read_encoding_table(char * file, EncodingMap& aEncodingMap)
{
    FILE * fp = fopen(file, "r");
    if ( ! fp  ) {
        fprintf(stderr, "ulfconv: %s %s\n", file, strerror(errno));
        exit(2);
    }

    char buffer[512];
    while ( NULL != my_fgets(buffer, sizeof(buffer), fp) ) {

        // strip comment lines
        if ( buffer[0] == '#' )
            continue;

        // find end of language string
        char * cp;
        for ( cp = buffer; ! isspace(*cp); cp++ )
            ;
        *cp = '\0';

        // find start of codepage string
        for ( ++cp; isspace(*cp); ++cp )
            ;
        char * codepage = cp;

        // find end of codepage string
        for ( ++cp; ! isspace(*cp); ++cp )
            ;
        *cp = '\0';

        // find the correct mapping for codepage
        const unsigned int members = SAL_N_ELEMENTS( _ms_encoding_list );
        const _pair *encoding = _pair_search( codepage, _ms_encoding_list, members );

        if ( encoding != NULL ) {
            const std::string language(buffer);
            aEncodingMap.insert( EncodingMap::value_type(language, encoding->value) );
        }
    }

    fclose(fp);
}

/************************************************************************
 * print_legacy_mixed
 ************************************************************************/

void print_legacy_mixed(
    FILE * ostream,
    const rtl::OUString& aString,
    const std::string& language,
    EncodingMap& aEncodingMap)
{
    EncodingMap::iterator iter = aEncodingMap.find(language);

    if ( iter != aEncodingMap.end() ) {
        fputs(OUStringToOString(aString, iter->second).getStr(), ostream);
    } else {
        fprintf(stderr, "ulfconv: WARNING: no legacy encoding found for %s\n", language.c_str());
    }
}

/************************************************************************
 * print_java_style
 ************************************************************************/

void print_java_style(FILE * ostream, const rtl::OUString& aString)
{
    int imax = aString.getLength();
    for (int i = 0; i < imax; i++) {
        sal_Unicode uc = aString[i];
        if ( uc < 128 ) {
            fprintf(ostream, "%c", (char) uc);
        } else {
            fprintf(ostream, "\\u%2.2x%2.2x", uc >> 8, uc & 0xFF );
        }
    }
}

/************************************************************************
 * main
 ************************************************************************/

int main( int argc, char * const argv[] )
{
    EncodingMap aEncodingMap;

    FILE *istream = stdin;
    FILE *ostream = stdout;

    char *outfile = NULL;

    int errflg = 0;
    int argi;

    for( argi=1; argi < argc; argi++ )
    {
        if( argv[argi][0] == '-' && argv[argi][2] == '\0' )
        {
            switch(argv[argi][1]) {
            case 'o':
                if (argi+1 >= argc || argv[argi+1][0] == '-')
                {
                    fprintf(stderr, "Option -%c requires an operand\n", argv[argi][1]);
                    errflg++;
                    break;
                }

                ++argi;
                outfile = argv[argi];
                break;
            case 't':
                if (argi+1 >= argc || argv[argi+1][0] == '-')
                {
                    fprintf(stderr, "Option -%c requires an operand\n", argv[argi][1]);
                    errflg++;
                    break;
                }

                read_encoding_table(argv[++argi], aEncodingMap);
                break;
            default:
                fprintf(stderr, "Unrecognized option: -%c\n", argv[argi][1]);
                errflg++;
            }
        }
        else
        {
            break;
        }
    }

    if (errflg) {
      fprintf(stderr, "Usage: ulfconv [-o <output file>] [-t <encoding table>] [<ulf file>]\n");
      exit(2);
    }

    /* assign input file to stdin */
    if ( argi < argc )
    {
        istream = fopen(argv[argi], "r");
        if ( istream  == NULL ) {
            fprintf(stderr, "ulfconv: %s : %s\n", argv[argi], strerror(errno));
            exit(2);
        }
    }

    /* open output file if any */
    if ( outfile )
    {
        ostream = fopen(outfile, "w");
        if ( ostream == NULL ) {
            fprintf(stderr, "ulfconv: %s : %s\n", outfile, strerror(errno));
            fclose(istream);
            exit(2);
        }
    }

    /* read line by line from stdin */
    char buffer[65536];
    while ( NULL != fgets(buffer, sizeof(buffer), istream) ) {

        /* only handle lines containing " = " */
        char * cp = strstr(buffer, " = \"");
        if ( cp ) {
            rtl::OUString aString;

            /* find end of lang string */
            int n;
            for ( n=0; ! isspace(buffer[n]); n++ )
                ;

            std::string line = buffer;
            std::string lang(line, 0, n);

            cp += 4;
            rtl_string2UString( &aString.pData, cp, strrchr(cp, '\"') - cp,
                RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS );

            fprintf(ostream, "%s = \"", lang.c_str());

            if ( aEncodingMap.empty() ) {
                print_java_style(ostream, aString);
            } else {
                print_legacy_mixed(ostream, aString, lang, aEncodingMap);
            }

            fprintf(ostream, "\"\n");


        } else {
            fputs(buffer, ostream);
        }
    }

    fclose(ostream);
    fclose(istream);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
