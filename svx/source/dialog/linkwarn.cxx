/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Novell, Inc.
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
#include "precompiled_svx.hxx"

#include "linkwarn.hrc"
#include "svx/linkwarn.hxx"
#include "svx/dialogs.hrc"
#include "svx/dialmgr.hxx"
#include <vcl/msgbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <svtools/miscopt.hxx>

SvxLinkWarningDialog::SvxLinkWarningDialog( Window* pParent, const String& _rFileName ) :
    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_LINK_WARNING ) ),
    m_aQueryImage       ( this, SVX_RES( FI_QUERY ) ),
    m_aInfoText         ( this, SVX_RES( FT_INFOTEXT ) ),
    m_aLinkGraphicBtn   ( this, SVX_RES( PB_OK ) ),
    m_aEmbedGraphicBtn  ( this, SVX_RES( PB_NO ) ),
    m_aOptionLine       ( this, SVX_RES( FL_OPTION ) ),
    m_aWarningOnBox     ( this, SVX_RES( CB_WARNING_OFF ) )
{
    FreeResource();

    // set questionmark image
    m_aQueryImage.SetImage( QueryBox::GetStandardImage() );

    // replace filename
    String sInfoText = m_aInfoText.GetText();
    sInfoText.SearchAndReplaceAll(
        UniString::CreateFromAscii(
            RTL_CONSTASCII_STRINGPARAM( "%FILENAME" ) ), _rFileName );
    m_aInfoText.SetText( sInfoText );

    // load state of "warning on" checkbox from misc options
    SvtMiscOptions aMiscOpt;
    m_aWarningOnBox.Check( aMiscOpt.ShowLinkWarningDialog() == sal_True );
    if( aMiscOpt.IsShowLinkWarningDialogReadOnly() )
        m_aWarningOnBox.Disable();

    // set focus to Cancel button
    m_aEmbedGraphicBtn.GrabFocus();

    // calculate and set the size of the dialog and its controls
    InitSize();
}

// -----------------------------------------------------------------------

SvxLinkWarningDialog::~SvxLinkWarningDialog()
{
    // save value of "warning off" checkbox, if necessary
    SvtMiscOptions aMiscOpt;
    sal_Bool bChecked = m_aWarningOnBox.IsChecked();
    if ( aMiscOpt.ShowLinkWarningDialog() != bChecked )
        aMiscOpt.SetShowLinkWarningDialog( bChecked );
}

// -----------------------------------------------------------------------

void SvxLinkWarningDialog::InitSize()
{
    // text of checkbox to wide -> add new line
    long nTxtW = m_aWarningOnBox.GetCtrlTextWidth( m_aWarningOnBox.GetText() ) + IMPL_EXTRA_BUTTON_WIDTH;
    long nCtrlW = m_aWarningOnBox.GetSizePixel().Width();
    if ( nTxtW >= nCtrlW )
    {
        long nTextHeight = m_aWarningOnBox.GetTextHeight();
        Size aNewSize = m_aWarningOnBox.GetSizePixel();
        aNewSize.Height() += nTextHeight;
        m_aWarningOnBox.SetSizePixel( aNewSize );
        aNewSize = GetSizePixel();
        aNewSize.Height() += nTextHeight;
        SetSizePixel( aNewSize );
    }

    // align the size of the information text control (FixedText) to its content
    Size aMinSize = m_aInfoText.CalcMinimumSize( m_aInfoText.GetSizePixel().Width() );
    long nTxtH = aMinSize.Height();
    long nCtrlH = m_aInfoText.GetSizePixel().Height();
    long nDelta = ( nCtrlH - nTxtH );
    Size aNewSize = m_aInfoText.GetSizePixel();
    aNewSize.Height() -= nDelta;
    m_aInfoText.SetSizePixel( aNewSize );

    // new position for the succeeding windows
    Window* pWins[] =
    {
        &m_aLinkGraphicBtn, &m_aEmbedGraphicBtn, &m_aOptionLine, &m_aWarningOnBox
    };
    Window** pCurrent = pWins;
    for ( sal_uInt32 i = 0; i < sizeof(pWins) / sizeof(*pWins); ++i, ++pCurrent )
    {
        Point aNewPos = (*pCurrent)->GetPosPixel();
        aNewPos.Y() -= nDelta;
        (*pCurrent)->SetPosPixel( aNewPos );
    }

    // new size of the dialog
    aNewSize = GetSizePixel();
    aNewSize.Height() -= nDelta;
    SetSizePixel( aNewSize );

    // recalculate the size and position of the buttons
    nTxtW = m_aLinkGraphicBtn.GetCtrlTextWidth( m_aLinkGraphicBtn.GetText() );
    long nTemp = m_aEmbedGraphicBtn.GetCtrlTextWidth( m_aEmbedGraphicBtn.GetText() );
    if ( nTemp > nTxtW )
        nTxtW = nTemp;
    nTxtW += IMPL_EXTRA_BUTTON_WIDTH;
    Size a3Size = LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
    Point aPos = m_aLinkGraphicBtn.GetPosPixel();
    aPos.X() = ( aNewSize.Width() - (2*nTxtW) - a3Size.Width() ) / 2;
    long nDefX = m_aWarningOnBox.GetPosPixel().X();
    if ( nDefX < aPos.X() )
        aPos.X() = nDefX;
    aNewSize = m_aLinkGraphicBtn.GetSizePixel();
    aNewSize.Width() = nTxtW;
    m_aLinkGraphicBtn.SetPosSizePixel( aPos, aNewSize );
    aPos.X() += nTxtW + a3Size.Width();
    m_aEmbedGraphicBtn.SetPosSizePixel( aPos, aNewSize );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
