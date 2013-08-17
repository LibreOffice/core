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

#include "TableDesignHelpBar.hxx"
#include <tools/debug.hxx>
#include <svtools/svmedit.hxx>
#include "dbaccess_helpid.hrc"
#include <memory>
using namespace dbaui;
#define STANDARD_MARGIN                 6
// class OTableDesignHelpBar
DBG_NAME(OTableDesignHelpBar)
OTableDesignHelpBar::OTableDesignHelpBar( Window* pParent ) :
     TabPage( pParent, WB_3DLOOK )
{
    DBG_CTOR(OTableDesignHelpBar,NULL);
    m_pTextWin = new MultiLineEdit( this, WB_VSCROLL | WB_LEFT | WB_BORDER | WB_NOTABSTOP | WB_READONLY);
    m_pTextWin->SetHelpId(HID_TABLE_DESIGN_HELP_WINDOW);
    m_pTextWin->SetReadOnly();
    m_pTextWin->SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );
    m_pTextWin->Show();
}

OTableDesignHelpBar::~OTableDesignHelpBar()
{
    DBG_DTOR(OTableDesignHelpBar,NULL);
    ::std::auto_ptr<Window> aTemp(m_pTextWin);
    m_pTextWin = NULL;
}

void OTableDesignHelpBar::SetHelpText( const String& rText )
{
    DBG_CHKTHIS(OTableDesignHelpBar,NULL);
    if(m_pTextWin)
        m_pTextWin->SetText( rText );
    Invalidate();
}

void OTableDesignHelpBar::Resize()
{
    DBG_CHKTHIS(OTableDesignHelpBar,NULL);
    // Abmessungen parent window
    Size aOutputSize( GetOutputSizePixel() );

    // TextWin anpassen
    if(m_pTextWin)
        m_pTextWin->SetPosSizePixel( Point(STANDARD_MARGIN+1, STANDARD_MARGIN+1),
            Size(aOutputSize.Width()-(2*STANDARD_MARGIN)-2,
                 aOutputSize.Height()-(2*STANDARD_MARGIN)-2) );

}

long OTableDesignHelpBar::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == EVENT_LOSEFOCUS)
        SetHelpText(String());
    return TabPage::PreNotify(rNEvt);
}

sal_Bool OTableDesignHelpBar::isCopyAllowed()
{
    return m_pTextWin && !m_pTextWin->GetSelected().isEmpty();
}

sal_Bool OTableDesignHelpBar::isCutAllowed()
{
    return sal_False;
}

sal_Bool OTableDesignHelpBar::isPasteAllowed()
{
    return sal_False;
}

void OTableDesignHelpBar::cut()
{
}

void OTableDesignHelpBar::copy()
{
    if ( m_pTextWin )
        m_pTextWin->Copy();
}

void OTableDesignHelpBar::paste()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
