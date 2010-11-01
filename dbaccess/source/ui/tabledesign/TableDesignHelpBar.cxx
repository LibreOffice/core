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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "TableDesignHelpBar.hxx"
#include <tools/debug.hxx>
#include <svtools/svmedit.hxx>
#include "dbaccess_helpid.hrc"
#include <memory>
using namespace dbaui;
#define STANDARD_MARGIN                 6
//==================================================================
// class OTableDesignHelpBar
//==================================================================
DBG_NAME(OTableDesignHelpBar)
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
OTableDesignHelpBar::~OTableDesignHelpBar()
{
    DBG_DTOR(OTableDesignHelpBar,NULL);
    ::std::auto_ptr<Window> aTemp(m_pTextWin);
    m_pTextWin = NULL;
}

//------------------------------------------------------------------------------
void OTableDesignHelpBar::SetHelpText( const String& rText )
{
    DBG_CHKTHIS(OTableDesignHelpBar,NULL);
    if(m_pTextWin)
        m_pTextWin->SetText( rText );
    Invalidate();
}

//------------------------------------------------------------------------------
void OTableDesignHelpBar::Resize()
{
    DBG_CHKTHIS(OTableDesignHelpBar,NULL);
    //////////////////////////////////////////////////////////////////////
    // Abmessungen parent window
    Size aOutputSize( GetOutputSizePixel() );

    //////////////////////////////////////////////////////////////////////
    // TextWin anpassen
    if(m_pTextWin)
        m_pTextWin->SetPosSizePixel( Point(STANDARD_MARGIN+1, STANDARD_MARGIN+1),
            Size(aOutputSize.Width()-(2*STANDARD_MARGIN)-2,
                 aOutputSize.Height()-(2*STANDARD_MARGIN)-2) );

}

//------------------------------------------------------------------------------
long OTableDesignHelpBar::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == EVENT_LOSEFOCUS)
        SetHelpText(String());
    return TabPage::PreNotify(rNEvt);
}
// -----------------------------------------------------------------------------
sal_Bool OTableDesignHelpBar::isCopyAllowed()
{
    return m_pTextWin && m_pTextWin->GetSelected().Len();
}
// -----------------------------------------------------------------------------
sal_Bool OTableDesignHelpBar::isCutAllowed()
{
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OTableDesignHelpBar::isPasteAllowed()
{
    return sal_False;
}
// -----------------------------------------------------------------------------
void OTableDesignHelpBar::cut()
{
}
// -----------------------------------------------------------------------------
void OTableDesignHelpBar::copy()
{
    if ( m_pTextWin )
        m_pTextWin->Copy();
}
// -----------------------------------------------------------------------------
void OTableDesignHelpBar::paste()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
