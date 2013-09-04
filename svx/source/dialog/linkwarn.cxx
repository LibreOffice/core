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

#include "linkwarn.hrc"
#include "svx/linkwarn.hxx"
#include "svx/dialogs.hrc"
#include "svx/dialmgr.hxx"
#include <vcl/msgbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <svtools/miscopt.hxx>

SvxLinkWarningDialog::SvxLinkWarningDialog( Window* pParent, const OUString& _rFileName ) :
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
    OUString sInfoText = m_aInfoText.GetText();
    sInfoText = sInfoText.replaceAll("%FILENAME", _rFileName);
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
    for ( sal_uInt32 i = 0; i < SAL_N_ELEMENTS(pWins); ++i, ++pCurrent )
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
