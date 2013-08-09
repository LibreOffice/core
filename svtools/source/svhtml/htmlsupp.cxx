/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <ctype.h>
#include <stdio.h>
#include <comphelper/string.hxx>
#include <svtools/parhtml.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <tools/urlobj.hxx>

// Table for converting option values into strings
static HTMLOptionEnum const aScriptLangOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_LG_starbasic, HTML_SL_STARBASIC   },
    { OOO_STRING_SVTOOLS_HTML_LG_javascript,    HTML_SL_JAVASCRIPT  },
    { OOO_STRING_SVTOOLS_HTML_LG_javascript11,HTML_SL_JAVASCRIPT    },
    { OOO_STRING_SVTOOLS_HTML_LG_livescript,    HTML_SL_JAVASCRIPT  },
    { 0,                    0                   }
};

bool HTMLParser::ParseScriptOptions( OUString& rLangString, const OUString& rBaseURL,
                                     HTMLScriptLanguage& rLang,
                                     OUString& rSrc,
                                     OUString& rLibrary,
                                     OUString& rModule )
{
    const HTMLOptions& aScriptOptions = GetOptions();

    rLangString = "";
    rLang = HTML_SL_JAVASCRIPT;
    rSrc = "";
    rLibrary = "";
    rModule = "";

    for( size_t i = aScriptOptions.size(); i; )
    {
        const HTMLOption& aOption = aScriptOptions[--i];
        switch( aOption.GetToken() )
        {
        case HTML_O_LANGUAGE:
            {
                rLangString = aOption.GetString();
                sal_uInt16 nLang;
                if( aOption.GetEnum( nLang, aScriptLangOptEnums ) )
                    rLang = (HTMLScriptLanguage)nLang;
                else
                    rLang = HTML_SL_UNKNOWN;
            }
            break;

        case HTML_O_SRC:
            rSrc = INetURLObject::GetAbsURL( rBaseURL, aOption.GetString() );
            break;
        case HTML_O_SDLIBRARY:
            rLibrary = aOption.GetString();
            break;

        case HTML_O_SDMODULE:
            rModule = aOption.GetString();
            break;
        }
    }

    return true;
}

void HTMLParser::RemoveSGMLComment( OUString &rString, sal_Bool bFull )
{
    sal_Unicode c = 0;
    while( !rString.isEmpty() &&
           ( ' '==(c=rString[0]) || '\t'==c || '\r'==c || '\n'==c ) )
        rString = rString.copy( 1, rString.getLength() - 1 );

    while( !rString.isEmpty() &&
           ( ' '==(c=rString[rString.getLength()-1])
           || '\t'==c || '\r'==c || '\n'==c ) )
        rString = rString.copy( 0, rString.getLength()-1 );


    // remove SGML comments
    if( rString.getLength() >= 4 &&
        rString.startsWith( "<!--" ) )
    {
        sal_Int32 nPos = 3;
        if( bFull )
        {
            // the whole line
            nPos = 4;
            while( nPos < rString.getLength() &&
                ( ( c = rString[nPos] ) != '\r' && c != '\n' ) )
                ++nPos;
            if( c == '\r' && nPos+1 < rString.getLength() &&
                '\n' == rString[nPos+1] )
                ++nPos;
            else if( c != '\n' )
                nPos = 3;
        }
        ++nPos;
        rString = rString.copy( nPos, rString.getLength() - nPos );
    }

    if( rString.getLength() >=3 &&
        rString.endsWith("-->") )
    {
        rString = rString.copy( 0, rString.getLength()-3 );
        if( bFull )
        {
            // "//" or "'", maybe preceding CR/LF
            rString = comphelper::string::stripEnd(rString, ' ');
            sal_Int32 nDel = 0, nLen = rString.getLength();
            if( nLen >= 2 &&
                rString.endsWith("//") )
            {
                nDel = 2;
            }
            else if( nLen && '\'' == rString[nLen-1] )
            {
                nDel = 1;
            }
            if( nDel && nLen >= nDel+1 )
            {
                c = rString[nLen-(nDel+1)];
                if( '\r'==c || '\n'==c )
                {
                    nDel++;
                    if( '\n'==c && nLen >= nDel+1 &&
                        '\r'==rString[nLen-(nDel+1)] )
                        nDel++;
                }
            }
            rString = rString.copy( 0, nLen-nDel );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
