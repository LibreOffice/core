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

#include <TableDesignHelpBar.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#include <helpids.h>
#include <memory>
using namespace dbaui;
#define STANDARD_MARGIN                 6
// class OTableDesignHelpBar
OTableDesignHelpBar::OTableDesignHelpBar( vcl::Window* pParent ) :
     TabPage( pParent, WB_3DLOOK )
{
    m_pTextWin = VclPtr<MultiLineEdit>::Create( this, WB_VSCROLL | WB_LEFT | WB_BORDER | WB_NOTABSTOP | WB_READONLY);
    m_pTextWin->SetHelpId(HID_TABLE_DESIGN_HELP_WINDOW);
    m_pTextWin->SetReadOnly();
    m_pTextWin->SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );
    m_pTextWin->Show();
}

OTableDesignHelpBar::~OTableDesignHelpBar()
{
    disposeOnce();
}

void OTableDesignHelpBar::dispose()
{
    m_pTextWin.disposeAndClear();
    TabPage::dispose();
}

void OTableDesignHelpBar::SetHelpText( const OUString& rText )
{
    if(m_pTextWin)
        m_pTextWin->SetText( rText );
    Invalidate();
}

void OTableDesignHelpBar::Resize()
{
    // parent window dimensions
    Size aOutputSize( GetOutputSizePixel() );

    // adapt the TextWin
    if(m_pTextWin)
        m_pTextWin->SetPosSizePixel( Point(STANDARD_MARGIN+1, STANDARD_MARGIN+1),
            Size(aOutputSize.Width()-(2*STANDARD_MARGIN)-2,
                 aOutputSize.Height()-(2*STANDARD_MARGIN)-2) );

}

bool OTableDesignHelpBar::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS)
        SetHelpText(OUString());
    return TabPage::PreNotify(rNEvt);
}

bool OTableDesignHelpBar::isCopyAllowed()
{
    return m_pTextWin && !m_pTextWin->GetSelected().isEmpty();
}

bool OTableDesignHelpBar::isCutAllowed()
{
    return false;
}

bool OTableDesignHelpBar::isPasteAllowed()
{
    return false;
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
