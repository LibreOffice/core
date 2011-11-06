/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <ctype.h>
#include <stdio.h>
#include <tools/urlobj.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>
#endif

#include <svtools/parhtml.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>

/*  */

// Tabellen zum Umwandeln von Options-Werten in Strings

static HTMLOptionEnum __READONLY_DATA aScriptLangOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_LG_starbasic, HTML_SL_STARBASIC   },
    { OOO_STRING_SVTOOLS_HTML_LG_javascript,    HTML_SL_JAVASCRIPT  },
    { OOO_STRING_SVTOOLS_HTML_LG_javascript11,HTML_SL_JAVASCRIPT    },
    { OOO_STRING_SVTOOLS_HTML_LG_livescript,    HTML_SL_JAVASCRIPT  },
//  { OOO_STRING_SVTOOLS_HTML_LG_unused_javascript, HTML_SL_UNUSEDJS },
//  { OOO_STRING_SVTOOLS_HTML_LG_vbscript,  HTML_SL_VBSCRIPT    },
//  { OOO_STRING_SVTOOLS_HTML_LG_starone,       HTML_SL_STARONE     },
    { 0,                    0                   }
};

sal_Bool HTMLParser::ParseScriptOptions( String& rLangString, const String& rBaseURL,
                                     HTMLScriptLanguage& rLang,
                                     String& rSrc,
                                     String& rLibrary,
                                     String& rModule )
{
    const HTMLOptions *pScriptOptions = GetOptions();

    rLangString.Erase();
    rLang = HTML_SL_JAVASCRIPT;
    rSrc.Erase();
    rLibrary.Erase();
    rModule.Erase();

    for( sal_uInt16 i = pScriptOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pScriptOptions)[ --i ];
        switch( pOption->GetToken() )
        {
        case HTML_O_LANGUAGE:
            {
                rLangString = pOption->GetString();
                sal_uInt16 nLang;
                if( pOption->GetEnum( nLang, aScriptLangOptEnums ) )
                    rLang = (HTMLScriptLanguage)nLang;
                else
                    rLang = HTML_SL_UNKNOWN;
            }
            break;

        case HTML_O_SRC:
            rSrc = INetURLObject::GetAbsURL( rBaseURL, pOption->GetString() );
            break;
        case HTML_O_SDLIBRARY:
            rLibrary = pOption->GetString();
            break;

        case HTML_O_SDMODULE:
            rModule = pOption->GetString();
            break;
        }
    }

    return sal_True;
}

void HTMLParser::RemoveSGMLComment( String &rString, sal_Bool bFull )
{
    sal_Unicode c = 0;
    while( rString.Len() &&
           ( ' '==(c=rString.GetChar(0)) || '\t'==c || '\r'==c || '\n'==c ) )
        rString.Erase( 0, 1 );

    while( rString.Len() &&
           ( ' '==(c=rString.GetChar( rString.Len()-1))
           || '\t'==c || '\r'==c || '\n'==c ) )
        rString.Erase( rString.Len()-1 );


    // SGML-Kommentare entfernen
    if( rString.Len() >= 4 &&
        rString.CompareToAscii( "<!--", 4 ) == COMPARE_EQUAL )
    {
        xub_StrLen nPos = 3;
        if( bFull )
        {
            // die gesamte Zeile !
            nPos = 4;
            while( nPos < rString.Len() &&
                ( ( c = rString.GetChar( nPos )) != '\r' && c != '\n' ) )
                ++nPos;
            if( c == '\r' && nPos+1 < rString.Len() &&
                '\n' == rString.GetChar( nPos+1 ))
                ++nPos;
            else if( c != '\n' )
                nPos = 3;
        }
        rString.Erase( 0, ++nPos );
    }

    if( rString.Len() >=3 &&
        rString.Copy(rString.Len()-3).CompareToAscii("-->")
            == COMPARE_EQUAL )
    {
        rString.Erase( rString.Len()-3 );
        if( bFull )
        {
            // auch noch ein "//" oder "'" und ggf CR/LF davor
            rString.EraseTrailingChars();
            xub_StrLen nDel = 0, nLen = rString.Len();
            if( nLen >= 2 &&
                rString.Copy(nLen-2).CompareToAscii("//") == COMPARE_EQUAL )
            {
                nDel = 2;
            }
            else if( nLen && '\'' == rString.GetChar(nLen-1) )
            {
                nDel = 1;
            }
            if( nDel && nLen >= nDel+1 )
            {
                c = rString.GetChar( nLen-(nDel+1) );
                if( '\r'==c || '\n'==c )
                {
                    nDel++;
                    if( '\n'==c && nLen >= nDel+1 &&
                        '\r'==rString.GetChar( nLen-(nDel+1) ) )
                        nDel++;
                }
            }
            rString.Erase( nLen-nDel );
        }
    }
}

