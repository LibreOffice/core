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


#include <sal/macros.h>
#include "alienwarn.hxx"
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxuno.hxx>

#include "alienwarn.hrc"
#include "dialog.hrc"
#include <vcl/msgbox.hxx>
#include <unotools/saveopt.hxx>

// class SfxAlienWarningDialog -------------------------------------------

SfxAlienWarningDialog::SfxAlienWarningDialog( Window* pParent, const OUString& _rFormatName ) :

    SfxModalDialog( pParent, SfxResId( RID_DLG_ALIEN_WARNING ) ),

    m_aKeepCurrentBtn   ( this, SfxResId( PB_NO ) ),
    m_aSaveODFBtn       ( this, SfxResId( PB_YES ) ),
    m_aMoreInfoBtn      ( this, SfxResId( PB_MOREINFO ) ),
    m_aOptionLine       ( this, SfxResId( FL_OPTION ) ),
    m_aWarningOnBox     ( this, SfxResId( CB_WARNING_OFF ) ),
    m_aQueryImage       ( this, SfxResId( FI_QUERY ) ),
    m_aInfoText         ( this, SfxResId( FT_INFOTEXT ) )

{
    FreeResource();

    // set questionmark image
    m_aQueryImage.SetImage( QueryBox::GetStandardImage() );

    // replace formatname (text)
    OUString sInfoText = m_aInfoText.GetText();
    sInfoText = sInfoText.replaceAll( "%FORMATNAME", _rFormatName );
    m_aInfoText.SetText( sInfoText );

    // replace formatname (button)
    sInfoText = m_aKeepCurrentBtn.GetText();
    sInfoText = sInfoText.replaceAll( "%FORMATNAME", _rFormatName );
    m_aKeepCurrentBtn.SetText( sInfoText );

    // load value of "warning on" checkbox from save options
    m_aWarningOnBox.Check( SvtSaveOptions().IsWarnAlienFormat() == sal_True );

    // set focus to "Keep Current Format" button
    m_aKeepCurrentBtn.GrabFocus();

    // pb: #i43989# we have no online help for this dialog at the moment
    // -> hide the "more info" button
    //m_aMoreInfoBtn.Hide();

    // calculate and set the size of the dialog and its controls
    InitSize();
}

// -----------------------------------------------------------------------

SfxAlienWarningDialog::~SfxAlienWarningDialog()
{
    // save value of "warning off" checkbox, if necessary
    SvtSaveOptions aSaveOpt;
    sal_Bool bChecked = m_aWarningOnBox.IsChecked();
    if ( aSaveOpt.IsWarnAlienFormat() != bChecked )
        aSaveOpt.SetWarnAlienFormat( bChecked );
}

// -----------------------------------------------------------------------

void SfxAlienWarningDialog::InitSize()
{
    const long nExtraButtonWidth = LogicToPixel( Size(IMPL_EXTRA_BUTTON_WIDTH,1), MapMode(MAP_APPFONT) ).getWidth();
    const long nAwCol2 = LogicToPixel( Size(AW_COL_2,1), MapMode(MAP_APPFONT) ).getWidth();
    long nTxtW, nCtrlW;

    // layout calculations should be re-done, when More Info button is enabled
    m_aMoreInfoBtn.Hide();

    // recalculate the size and position of the buttons
    nTxtW = m_aKeepCurrentBtn.GetCtrlTextWidth( m_aKeepCurrentBtn.GetText() );
    nTxtW += nExtraButtonWidth;
    Size aNewSize = m_aKeepCurrentBtn.GetSizePixel();
    aNewSize.Width() = nTxtW;
    m_aKeepCurrentBtn.SetSizePixel( aNewSize );
    Point aPos = m_aSaveODFBtn.GetPosPixel();
    aPos.X() = nAwCol2 + nTxtW + nExtraButtonWidth;
    m_aSaveODFBtn.SetPosPixel( aPos );
    nTxtW = m_aSaveODFBtn.GetCtrlTextWidth( m_aSaveODFBtn.GetText() );
    nTxtW += nExtraButtonWidth;
    aNewSize = m_aSaveODFBtn.GetSizePixel();
    aNewSize.Width() = nTxtW;
    m_aSaveODFBtn.SetSizePixel( aNewSize );
    long nBtnsWidthSize = m_aKeepCurrentBtn.GetSizePixel().Width() + m_aSaveODFBtn.GetSizePixel().Width() + nAwCol2 + 2*nExtraButtonWidth;

    // resize + text of checkbox too wide -> add new line
    aNewSize = m_aWarningOnBox.GetSizePixel();
    aNewSize.Width() = nBtnsWidthSize - 2*nExtraButtonWidth;
    m_aWarningOnBox.SetSizePixel( aNewSize );
    nTxtW = m_aWarningOnBox.GetCtrlTextWidth( m_aWarningOnBox.GetText() );
    nCtrlW = m_aWarningOnBox.GetSizePixel().Width();
    if ( nTxtW >= nCtrlW )
    {
        long nTextHeight = m_aWarningOnBox.GetTextHeight();
        aNewSize.Height() += nTextHeight;
        m_aWarningOnBox.SetSizePixel( aNewSize );
        aNewSize = GetSizePixel();
        aNewSize.Height() += nTextHeight;
        SetSizePixel( aNewSize );
    }

    // resize + align the size of the information text control (FixedText) to its content
    aNewSize = m_aInfoText.GetSizePixel();
    aNewSize.Width() = nBtnsWidthSize - 2*nExtraButtonWidth;
    m_aInfoText.SetSizePixel( aNewSize );
    Size aMinSize = m_aInfoText.CalcMinimumSize( m_aInfoText.GetSizePixel().Width() );
    long nTxtH = aMinSize.Height();
    long nCtrlH = m_aInfoText.GetSizePixel().Height();
    long nDelta = ( nCtrlH - nTxtH );
    aNewSize.Height() -= nDelta;
    m_aInfoText.SetSizePixel( aNewSize );


    // new position for the succeeding windows
    Window* pWins[] =
    {
        &m_aSaveODFBtn, &m_aKeepCurrentBtn, &m_aMoreInfoBtn, &m_aOptionLine, &m_aWarningOnBox
    };
    Window** pCurrent = pWins;
    for ( sal_uInt32 i = 0; i < sizeof( pWins ) / sizeof( pWins[ 0 ] ); ++i, ++pCurrent )
    {
        Point aNewPos = (*pCurrent)->GetPosPixel();
        aNewPos.Y() -= nDelta;
        (*pCurrent)->SetPosPixel( aNewPos );
    }

    // new size of the dialog
    aNewSize = GetSizePixel();
    aNewSize.Height() -= nDelta;
    aNewSize.Width() = nBtnsWidthSize;
    SetSizePixel( aNewSize );

    // resize the FixedLine
    aNewSize = m_aOptionLine.GetSizePixel();
    aNewSize.Width() = GetSizePixel().Width();
    m_aOptionLine.SetSizePixel( aNewSize );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
