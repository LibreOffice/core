/*************************************************************************
 *
 *  $RCSfile: htmlsupp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

BOOL HTMLParser::ParseScriptOptions( String& rLangString,
                                     HTMLScriptLanguage& rLang,
                                     String& rSrc,
                                     String& rLibrary,
                                     String& rModule )
{
    const HTMLOptions *pOptions = GetOptions();

    rLangString.Erase();
    rLang = HTML_SL_JAVASCRIPT;
    rSrc.Erase();
    rLibrary.Erase();
    rModule.Erase();

    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[ --i ];
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
            rSrc = INetURLObject::RelToAbs( pOption->GetString() );
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
    sal_Unicode c;
    while( rString.Len() &&
           ( ' '==(c=rString.GetChar(0UL)) || '\t'==c || '\r'==c || '\n'==c ) )
        rString.Erase( 0UL, 1UL );

    while( rString.Len() &&
           ( ' '==(c=rString.GetChar( rString.Len()-1UL))
           || '\t'==c || '\r'==c || '\n'==c ) )
        rString.Erase( rString.Len()-1UL );


    // SGML-Kommentare entfernen
    if( rString.Len() >= 4UL &&
        rString.CompareToAscii( "<!--", 4UL ) == COMPARE_EQUAL )
    {
        sal_uInt32 nPos = 3UL;
        if( bFull )
        {
            // die gesamte Zeile !
            nPos = 4UL;
            while( nPos < rString.Len() &&
                ( ( c = rString.GetChar( nPos )) != '\r' && c != '\n' ) )
                ++nPos;
            if( c == '\r' && nPos+1UL < rString.Len() &&
                '\n' == rString.GetChar( nPos+1UL ))
                ++nPos;
            else if( c != '\n' )
                nPos = 3UL;
        }
        rString.Erase( 0UL, ++nPos );
    }

    if( rString.Len() >=3UL &&
        rString.Copy(rString.Len()-3UL).CompareToAscii("-->")
            == COMPARE_EQUAL )
    {
        rString.Erase( rString.Len()-3UL );
        if( bFull )
        {
            // auch noch ein "//" oder "'" und ggf CR/LF davor
            rString.EraseTrailingChars();
            sal_uInt32 nDel = 0UL, nLen = rString.Len();
            if( nLen >= 2UL &&
                rString.Copy(nLen-2UL).CompareToAscii("//") == COMPARE_EQUAL )
            {
                nDel = 2UL;
            }
            else if( nLen && '\'' == rString.GetChar(nLen-1UL) )
            {
                nDel = 1UL;
            }
            if( nDel && nLen >= nDel+1UL )
            {
                c = rString.GetChar( nLen-(nDel+1UL) );
                if( '\r'==c || '\n'==c )
                {
                    nDel++;
                    if( '\n'==c && nLen >= nDel+1UL &&
                        '\r'==rString.GetChar( nLen-(nDel+1UL) ) )
                        nDel++;
                }
            }
            rString.Erase( nLen-nDel );
        }
    }
}

