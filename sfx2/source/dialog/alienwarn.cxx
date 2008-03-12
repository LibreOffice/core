/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: alienwarn.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:33:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "alienwarn.hxx"
#include "sfxresid.hxx"
#include <sfx2/sfxuno.hxx>

#include "alienwarn.hrc"
#include "dialog.hrc"

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

// class SfxAlienWarningDialog -------------------------------------------

SfxAlienWarningDialog::SfxAlienWarningDialog( Window* pParent, const String& _rFormatName ) :

    SfxModalDialog( pParent, SfxResId( RID_DLG_ALIEN_WARNING ) ),

    m_aQueryImage       ( this, SfxResId( FI_QUERY ) ),
    m_aInfoText         ( this, SfxResId( FT_INFOTEXT ) ),
    m_aKeepCurrentBtn   ( this, SfxResId( PB_NO ) ),
    m_aSaveODFBtn       ( this, SfxResId( PB_YES ) ),
    m_aMoreInfoBtn      ( this, SfxResId( PB_MOREINFO ) ),
    m_aOptionLine       ( this, SfxResId( FL_OPTION ) ),
    m_aWarningOnBox     ( this, SfxResId( CB_WARNING_OFF ) )

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
    for ( sal_uInt32 i = 0; i < sizeof( pWins ) / sizeof( pWins[ 0 ] ); ++i, ++pCurrent )
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

