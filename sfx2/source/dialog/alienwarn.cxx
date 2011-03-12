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
#include "precompiled_sfx2.hxx"

#include <sal/macros.h>
#include "alienwarn.hxx"
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxuno.hxx>

#include "alienwarn.hrc"
#include "dialog.hrc"
#include <vcl/msgbox.hxx>
#include <unotools/saveopt.hxx>

// class SfxAlienWarningDialog -------------------------------------------

SfxAlienWarningDialog::SfxAlienWarningDialog( Window* pParent, const String& _rFormatName ) :

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

    // replace formatname
    String sInfoText = m_aInfoText.GetText();
    sInfoText.SearchAndReplaceAll( DEFINE_CONST_UNICODE("%FORMATNAME"), _rFormatName );
    m_aInfoText.SetText( sInfoText );

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
    // if the button text is too wide, then broaden the button
    long nTxtW = m_aMoreInfoBtn.GetCtrlTextWidth( m_aMoreInfoBtn.GetText() );
    long nCtrlW = m_aMoreInfoBtn.GetSizePixel().Width();
    if ( nTxtW >= nCtrlW )
    {
        long nDelta = nTxtW - nCtrlW;
        nDelta += IMPL_EXTRA_BUTTON_WIDTH;
        Point aNextPoint = m_aKeepCurrentBtn.GetPosPixel();
        aNextPoint.X() += m_aKeepCurrentBtn.GetSizePixel().Width();
        Point aNewPoint = m_aMoreInfoBtn.GetPosPixel();
        aNewPoint.X() -= nDelta;
        if ( aNextPoint.X() >= aNewPoint.X() )
        {
            long nSpace = aNextPoint.X() - aNewPoint.X();
            nSpace += 2;
            nDelta -= nSpace;
            aNewPoint.X() += nSpace;
        }
        Size aNewSize = m_aMoreInfoBtn.GetSizePixel();
        aNewSize.Width() += nDelta;
        m_aMoreInfoBtn.SetPosSizePixel( aNewPoint, aNewSize );
    }

    // text of checkbox to wide -> add new line
    nTxtW = m_aWarningOnBox.GetCtrlTextWidth( m_aWarningOnBox.GetText() ) + IMPL_EXTRA_BUTTON_WIDTH;
    nCtrlW = m_aWarningOnBox.GetSizePixel().Width();
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
        &m_aSaveODFBtn, &m_aKeepCurrentBtn, &m_aMoreInfoBtn, &m_aOptionLine, &m_aWarningOnBox
    };
    Window** pCurrent = pWins;
    for ( sal_uInt32 i = 0; i < SAL_N_ELEMENTS( pWins ); ++i, ++pCurrent )
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
    m_aMoreInfoBtn.Hide();
    nTxtW = m_aKeepCurrentBtn.GetCtrlTextWidth( m_aKeepCurrentBtn.GetText() );
    long nTemp = m_aSaveODFBtn.GetCtrlTextWidth( m_aSaveODFBtn.GetText() );
    if ( nTemp > nTxtW )
        nTxtW = nTemp;
    nTxtW += IMPL_EXTRA_BUTTON_WIDTH;
    Size a3Size = LogicToPixel( Size( 3, 3 ), MAP_APPFONT );
    Point aPos = m_aKeepCurrentBtn.GetPosPixel();
    aPos.X() = ( aNewSize.Width() - (2*nTxtW) - a3Size.Width() ) / 2;
    long nDefX = m_aWarningOnBox.GetPosPixel().X();
    if ( nDefX < aPos.X() )
        aPos.X() = nDefX;
    aNewSize = m_aKeepCurrentBtn.GetSizePixel();
    aNewSize.Width() = nTxtW;
    m_aKeepCurrentBtn.SetPosSizePixel( aPos, aNewSize );
    aPos.X() += nTxtW + a3Size.Width();
    m_aSaveODFBtn.SetPosSizePixel( aPos, aNewSize );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
