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

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <comphelper/oslfile2streamwrap.hxx>

#include <rtl/tencinfo.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svtools/textview.hxx>
#include <vcl/scrbar.hxx>
#include <tools/stream.hxx>
#include <tools/time.hxx>
#include <osl/file.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/htmltokn.h>
#include <svtools/txtattr.hxx>

#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "xmlfileview.hxx"
#include "xmlfileview.hrc"
#include "xmlfilterhelpids.hrc"

#include <deque>

using namespace osl;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::xml;
using namespace com::sun::star::xml::sax;

using ::rtl::OUString;


#define MAX_SYNTAX_HIGHLIGHT 20
#define MAX_HIGHLIGHTTIME 200
#define SYNTAX_HIGHLIGHT_TIMEOUT 200


struct SwTextPortion
{
    sal_uInt16 nLine;
    sal_uInt16 nStart, nEnd;
    svtools::ColorConfigEntry eType;
};

typedef std::deque<SwTextPortion> SwTextPortions;

void TextViewOutWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    switch( rDCEvt.GetType() )
    {
    case DATACHANGED_SETTINGS:
        // den Settings abgefragt werden.
        if( rDCEvt.GetFlags() & SETTINGS_STYLE )
        {
            const Color &rCol = GetSettings().GetStyleSettings().GetWindowColor();
            SetBackground( rCol );
            Font aFont( pTextView->GetTextEngine()->GetFont() );
            aFont.SetFillColor( rCol );
            pTextView->GetTextEngine()->SetFont( aFont );
        }
        break;
    }
}

void TextViewOutWin::MouseMove( const MouseEvent &rEvt )
{
    if ( pTextView )
        pTextView->MouseMove( rEvt );
}

void TextViewOutWin::MouseButtonUp( const MouseEvent &rEvt )
{
    if ( pTextView )
        pTextView->MouseButtonUp( rEvt );
}

void TextViewOutWin::MouseButtonDown( const MouseEvent &rEvt )
{
    GrabFocus();
    if ( pTextView )
        pTextView->MouseButtonDown( rEvt );
}

void TextViewOutWin::Command( const CommandEvent& rCEvt )
{
    switch(rCEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        break;

        default:
            if ( pTextView )
                pTextView->Command( rCEvt );
            else
                Window::Command(rCEvt);
    }
}

void TextViewOutWin::KeyInput( const KeyEvent& rKEvt )
{
    if(!TextEngine::DoesKeyChangeText( rKEvt ))
        pTextView->KeyInput( rKEvt );
}

void  TextViewOutWin::Paint( const Rectangle& rRect )
{
    pTextView->Paint( rRect );
}

///////////////////////////////////////////////////////////////////////

void lcl_Highlight(const String& rSource, SwTextPortions& aPortionList)
{
    const sal_Unicode cOpenBracket = '<';
    const sal_Unicode cCloseBracket= '>';
    const sal_Unicode cSlash        = '/';
    const sal_Unicode cExclamation = '!';
    const sal_Unicode cMinus        = '-';
    const sal_Unicode cSpace        = ' ';
    const sal_Unicode cTab          = 0x09;
    const sal_Unicode cLF          = 0x0a;
    const sal_Unicode cCR          = 0x0d;


    const sal_uInt16 nStrLen = rSource.Len();
    sal_uInt16 nInsert = 0;         // Number of inserted Portions
    sal_uInt16 nActPos = 0;         // Position, at the '<' was found
    sal_uInt16 nOffset = 0;         // Offset of nActPos for '<'
    sal_uInt16 nPortStart = USHRT_MAX;  // For the TextPortion
    sal_uInt16 nPortEnd  =  0;  //
    SwTextPortion aText;
    while(nActPos < nStrLen)
    {
        svtools::ColorConfigEntry eFoundType = svtools::HTMLUNKNOWN;
        if(rSource.GetChar(nActPos) == cOpenBracket && nActPos < nStrLen - 2 )
        {
            // 'leere' Portion einfuegen
            if(nPortEnd < nActPos - 1 )
            {
                aText.nLine = 0;
                // am Anfang nicht verschieben
                aText.nStart = nPortEnd;
                if(nInsert)
                    aText.nStart += 1;
                aText.nEnd = nActPos - 1;
                aText.eType = svtools::HTMLUNKNOWN;
                aPortionList.push_back( aText );
                nInsert++;
            }
            sal_Unicode cFollowFirst = rSource.GetChar((xub_StrLen)(nActPos + 1));
            sal_Unicode cFollowNext = rSource.GetChar((xub_StrLen)(nActPos + 2));
            if(cExclamation == cFollowFirst)
            {
                // "<!" SGML oder Kommentar
                if(cMinus == cFollowNext &&
                    nActPos < nStrLen - 3 && cMinus == rSource.GetChar((xub_StrLen)(nActPos + 3)))
                {
                    eFoundType = svtools::HTMLCOMMENT;
                }
                else
                    eFoundType = svtools::HTMLSGML;
                nPortStart = nActPos;
                nPortEnd = nActPos + 1;
            }
            else if(cSlash == cFollowFirst)
            {
                // "</" Slash ignorieren
                nPortStart = nActPos;
                nActPos++;
                nOffset++;
            }
            if(svtools::HTMLUNKNOWN == eFoundType)
            {
                //jetzt koennte hier ein keyword folgen
                sal_uInt16 nSrchPos = nActPos;
                while(++nSrchPos < nStrLen - 1)
                {
                    sal_Unicode cNext = rSource.GetChar(nSrchPos);
                    if( cNext == cSpace ||
                        cNext == cTab   ||
                        cNext == cLF    ||
                        cNext == cCR)
                        break;
                    else if(cNext == cCloseBracket)
                    {
                        break;
                    }
                }
                if(nSrchPos > nActPos + 1)
                {
                    //irgend ein String wurde gefunden
                    String sToken = rSource.Copy(nActPos + 1, nSrchPos - nActPos - 1 );
                    sToken.ToUpperAscii();
                    {
                        //Token gefunden
                        eFoundType = svtools::HTMLKEYWORD;
                        nPortEnd = nSrchPos;
                        nPortStart = nActPos;
                    }
                }
            }
            // jetzt muss noch '>' gesucht werden
            if(svtools::HTMLUNKNOWN != eFoundType)
            {
                sal_Bool bFound = sal_False;
                for(sal_uInt16 i = nPortEnd; i < nStrLen; i++)
                    if(cCloseBracket == rSource.GetChar(i))
                    {
                        bFound = sal_True;
                        nPortEnd = i;
                        break;
                    }
                if(!bFound && (eFoundType == svtools::HTMLCOMMENT))
                {
                    // Kommentar ohne Ende in dieser Zeile
                    bFound  = sal_True;
                    nPortEnd = nStrLen - 1;
                }

                if(bFound ||(eFoundType == svtools::HTMLCOMMENT))
                {
                    SwTextPortion aText2;
                    aText2.nLine = 0;
                    aText2.nStart = nPortStart + 1;
                    aText2.nEnd = nPortEnd;
                    aText2.eType = eFoundType;
                    aPortionList.push_back( aText2 );
                    nInsert++;
                    eFoundType = svtools::HTMLUNKNOWN;
                }

            }
        }
        nActPos++;
    }
    if(nInsert && nPortEnd < nActPos - 1)
    {
        aText.nLine = 0;
        aText.nStart = nPortEnd + 1;
        aText.nEnd = nActPos - 1;
        aText.eType = svtools::HTMLUNKNOWN;
        aPortionList.push_back( aText );
        nInsert++;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
