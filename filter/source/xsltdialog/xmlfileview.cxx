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
#include <vcl/textview.hxx>
#include <vcl/scrbar.hxx>
#include <tools/stream.hxx>
#include <tools/time.hxx>
#include <osl/file.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/htmltokn.h>
#include <vcl/txtattr.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
