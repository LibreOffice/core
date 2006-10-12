/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlsupp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 15:27:55 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <ctype.h>
#include <stdio.h>
#include <tools/urlobj.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svstdarr.hxx>
#endif

#include "parhtml.hxx"
#include "htmltokn.h"
#include "htmlkywd.hxx"

/*  */

// Tabellen zum Umwandeln von Options-Werten in Strings

static HTMLOptionEnum __READONLY_DATA aScriptLangOptEnums[] =
{
    { sHTML_LG_starbasic,   HTML_SL_STARBASIC   },
    { sHTML_LG_javascript,  HTML_SL_JAVASCRIPT  },
    { sHTML_LG_javascript11,HTML_SL_JAVASCRIPT  },
    { sHTML_LG_livescript,  HTML_SL_JAVASCRIPT  },
//  { sHTML_LG_unused_javascript, HTML_SL_UNUSEDJS },
//  { sHTML_LG_vbscript,    HTML_SL_VBSCRIPT    },
//  { sHTML_LG_starone,     HTML_SL_STARONE     },
    { 0,                    0                   }
};

BOOL HTMLParser::ParseScriptOptions( String& rLangString, const String& rBaseURL,
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

    for( USHORT i = pScriptOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pScriptOptions)[ --i ];
        switch( pOption->GetToken() )
        {
        case HTML_O_LANGUAGE:
            {
                rLangString = pOption->GetString();
                USHORT nLang;
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

    return TRUE;
}

void HTMLParser::RemoveSGMLComment( String &rString, BOOL bFull )
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

