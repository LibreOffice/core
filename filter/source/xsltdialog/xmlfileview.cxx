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
