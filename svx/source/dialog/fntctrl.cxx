/*************************************************************************
 *
 *  $RCSfile: fntctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>      // SfxViewShell
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>     // SfxPrinter
#endif
#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#pragma hdrstop

#include "fntctrl.hxx"
#include "dialogs.hrc"

// struct FontPrevWin_Impl -----------------------------------------------

struct FontPrevWin_Impl
{
    BOOL    bSelection      : 1,
            bGetSelection   : 1,
            bUseResText     : 1;
    Color*  pColor;
    String  aText;

    FontPrevWin_Impl() :
        bSelection( FALSE ), bGetSelection( FALSE ), bUseResText( FALSE ), pColor( NULL ) {}
};

// class SvxFontPrevWindow -----------------------------------------------

void SvxFontPrevWindow::InitSettings( BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        Color aTextColor = rStyleSettings.GetWindowTextColor();

        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetWindowColor() );
    }
    Invalidate();
}

// -----------------------------------------------------------------------

SvxFontPrevWindow::SvxFontPrevWindow( Window* pParent, const ResId& rId ) :

    Window     ( pParent, rId ),
    pPrinter   ( NULL ),
    bDelPrinter( FALSE )

{
    pImpl = new FontPrevWin_Impl;
    SfxViewShell* pSh = SfxViewShell::Current();

    if ( pSh )
        pPrinter = pSh->GetPrinter();

    if ( !pPrinter )
    {
        pPrinter = new Printer;
        bDelPrinter = TRUE;
    }
    SetMapMode( MapMode( MAP_TWIP ) );
    aFont.SetTransparent(TRUE);
    aFont.SetAlign(ALIGN_BASELINE);
    InitSettings( TRUE, TRUE );
}

// -----------------------------------------------------------------------

SvxFontPrevWindow::~SvxFontPrevWindow()
{
    delete pImpl->pColor;
    delete pImpl;

    if ( bDelPrinter )
        delete pPrinter;
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
        InitSettings( TRUE, FALSE );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( FALSE, TRUE );

    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( TRUE, TRUE );
    else
        Window::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFont(const SvxFont &rOutFont)
{
    aFont = rOutFont;
    aFont.SetTransparent(TRUE);
    aFont.SetAlign(ALIGN_BASELINE);
    Invalidate();
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetColor(const Color &rColor)
{
    delete pImpl->pColor;
    pImpl->pColor = new Color( rColor );
    Invalidate();
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::UseResourceText( BOOL bUse )
{
    pImpl->bUseResText = bUse;
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::Paint( const Rectangle& rRect )
{
    if ( pImpl->bUseResText )
        pImpl->aText = GetText();
    else if ( !pImpl->bSelection )
    {
        SfxViewShell* pSh = SfxViewShell::Current();

        if ( pSh && !pImpl->bGetSelection )
        {
            pImpl->aText = pSh->GetSelectionText();
            pImpl->bGetSelection = TRUE;
            pImpl->bSelection = pImpl->aText.Len() != 0;
        }

        if ( !pImpl->bSelection )
            pImpl->aText = aFont.GetName();

        if ( !pImpl->aText.Len() )
            pImpl->aText = GetText();

        if ( pImpl->aText.Len() > 15 )
            pImpl->aText.Erase( pImpl->aText.Search( sal_Unicode( ' ' ), 16 ) );
    }
    Window::SetFont(aFont);
    Font aOldFont = pPrinter->GetFont();
    pPrinter->SetFont( aFont );
    Size aTxtSize( aFont.GetTxtSize( pPrinter, pImpl->aText ) );
    pPrinter->SetFont( aOldFont );
    const Size aLogSize( GetOutputSize() );
    FontMetric aMetric(GetFontMetric());
    aTxtSize.Height() = aMetric.GetLineHeight();

    long nX = aLogSize.Width()  / 2 - aTxtSize.Width() / 2;
    long nY = aLogSize.Height() / 2 - aTxtSize.Height() / 2;

    if ( nY + aMetric.GetAscent() > aLogSize.Height() )
        nY = aLogSize.Height() - aMetric.GetAscent();

    if ( pImpl->pColor )
    {
        Rectangle aRect( Point( nX, nY ), aTxtSize );
        Color aLineCol = GetLineColor();
        Color aFillCol = GetFillColor();
        SetLineColor();
        SetFillColor( *pImpl->pColor );
        DrawRect( aRect );
        SetLineColor( aLineCol );
        SetFillColor( aFillCol );
    }

    nY += aMetric.GetAscent();
    DrawLine( Point( 0,  nY ), Point( nX, nY ) );
    DrawLine( Point( nX + aTxtSize.Width(), nY ), Point( aLogSize.Width(), nY ) );
    aFont.DrawPrev( this, pPrinter, Point( nX, nY ), pImpl->aText );
}


