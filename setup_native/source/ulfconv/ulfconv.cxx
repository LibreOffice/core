/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ulfconv.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:30:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#if !(defined NETBSD || defined FREEBSD || defined MACOSX || WNT)
#include <alloca.h>
#endif

#ifdef WNT
#include <malloc.h>
#define alloca _alloca
#endif

#include <rtl/ustring.hxx>

#include <map>
#include <string>

/*****************************************************************************/
/* typedefs
/*****************************************************************************/

typedef std::map< const std::string, rtl_TextEncoding > EncodingMap;

typedef struct {
    const char  *key;
    const rtl_TextEncoding value;
} _pair;

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


/*****************************************************************************/
/* fgets that work with unix line ends on Windows
/*****************************************************************************/

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

/*****************************************************************************/
/* compare function for binary search
/*****************************************************************************/

static int
_pair_compare (const char *key, const _pair *pair)
{
    int result = rtl_str_compareIgnoreAsciiCase( key, pair->key );
    return result;
}

/*****************************************************************************/
/* binary search on encoding tables
/*****************************************************************************/

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
    int nlang = 0;

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
        const unsigned int members = sizeof( _ms_encoding_list ) / sizeof( _pair );
        const _pair *encoding = _pair_search( codepage, _ms_encoding_list, members );

        if ( encoding != NULL ) {
            const std::string language(buffer);
            aEncodingMap.insert( EncodingMap::value_type(language, encoding->value) );
        }
    }
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
                    fprintf(stderr, "Option -%c requires an operand\n", argv[argi]);
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
                fprintf(stderr, "Unrecognized option: -%c\n", argv[argi]);
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
            fprintf(stderr, "ulfconv: %s : %s\n", argv[argi], strerror(errno));
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
            fprintf(ostream, buffer);
        }
    }
}
