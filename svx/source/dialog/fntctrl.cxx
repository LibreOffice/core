/*************************************************************************
 *
 *  $RCSfile: fntctrl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ama $ $Date: 2001-07-19 07:42:36 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_XUB_STRLEN
#include <svtools/svstdarr.hxx>
#endif

#pragma hdrstop

#include "fntctrl.hxx"
#include "dialogs.hrc"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;

// class FontPrevWin_Impl -----------------------------------------------

class FontPrevWin_Impl
{
    friend class SvxFontPrevWindow;
    Reference < XBreakIterator > xBreak;
    SvULongs aTextWidth;
    SvXub_StrLens aScriptChg;
    SvUShorts aScriptType;
    SvxFont aCJKFont;
    String  aText;
    String  aScriptText;
    Color*  pColor;
    Color*  pBackColor;
    long nAscent;
    sal_Unicode         cStartBracket;
    sal_Unicode         cEndBracket;
    BOOL    bSelection      : 1,
            bGetSelection   : 1,
            bUseResText     : 1,
            bTwoLines       : 1;
    void _CheckScript();
public:
    FontPrevWin_Impl() :
        cStartBracket(0), cEndBracket(0), pColor( NULL ), pBackColor( 0 ),
        bSelection( FALSE ), bGetSelection( FALSE ), bUseResText( FALSE ),
        bTwoLines( FALSE )  {}
    void CheckScript() { if( aText != aScriptText ) _CheckScript(); }
    Size CalcTextSize( OutputDevice* pWin, OutputDevice* pPrt, SvxFont &rFont );
    void DrawPrev( OutputDevice* pWin, Printer* pPrt, Point &rPt,
                   SvxFont &rFont );
};

// class FontPrevWin_Impl -----------------------------------------------

/*-----------------19.7.2001 08:44------------------
 * void FontPrevWin_Impl::_CheckScript()
 * evalutates the scripttypes of the actual string.
 * Afterwards the positions of script change are notified in aScriptChg,
 * the scripttypes in aScriptType.
 * The aTextWidth array will be filled with zero.
 * --------------------------------------------------*/

void FontPrevWin_Impl::_CheckScript()
{
    aScriptText = aText;
    USHORT nCnt = aScriptChg.Count();
    if( nCnt )
    {
        aScriptChg.Remove( 0, nCnt );
        aScriptType.Remove( 0, nCnt );
        aTextWidth.Remove( 0, nCnt );
        nCnt = 0;
    }
    if( !xBreak.is() )
    {
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        Reference < XInterface > xI = xMSF->createInstance(
            ::rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) );
        if ( xI.is() )
        {
            Any x = xI->queryInterface( ::getCppuType((const Reference< XBreakIterator >*)0) );
            x >>= xBreak;
        }
    }
    if( xBreak.is() )
    {
        USHORT nScript = xBreak->getScriptType( aText, 0 );
        USHORT nChg = 0;
        if( ScriptType::WEAK == nScript )
        {
            nChg = (xub_StrLen)xBreak->endOfScript( aText, nChg, nScript );
            if( nChg < aText.Len() )
                nScript = xBreak->getScriptType( aText, nChg );
            else
                nScript = ScriptType::LATIN;
        }

        do
        {
            nChg = (xub_StrLen)xBreak->endOfScript( aText, nChg, nScript );
            aScriptChg.Insert( nChg, nCnt );
            aScriptType.Insert( nScript, nCnt );
            aTextWidth.Insert( ULONG(0), nCnt++ );

            if( nChg < aText.Len() )
                nScript = xBreak->getScriptType( aText, nChg );
            else
                break;
        } while( TRUE );
    }
}

/*-----------------19.7.2001 08:48------------------
 * Size FontPrevWin_Impl::CalcTextSize(..)
 * fills the aTextWidth array with the text width of every part
 * of the actual string without a script change inside.
 * For Latin parts the given rFont will be used,
 * for Asian parts the aCJKFont.
 * The returned size contains the whole string.
 * The member nAscent is calculated to the maximal ascent of all used fonts.
 * --------------------------------------------------*/

Size FontPrevWin_Impl::CalcTextSize( OutputDevice* pWin, OutputDevice* pPrinter,
    SvxFont &rFont )
{
    USHORT nScript;
    USHORT nIdx = 0;
    xub_StrLen nStart = 0;
    xub_StrLen nEnd;
    USHORT nCnt = aScriptChg.Count();
    if( nCnt )
    {
        nEnd = aScriptChg[ nIdx ];
        nScript = aScriptType[ nIdx ];
    }
    else
    {
        nEnd = aText.Len();
        nScript = ScriptType::LATIN;
    }
    long nTxtWidth = 0;
    long nCJKHeight = 0;
    long nHeight = 0;
    nAscent = 0;
    long nCJKAscent = 0;
    do
    {
        SvxFont& rFnt = (nScript==ScriptType::ASIAN) ? aCJKFont : rFont;
        ULONG nWidth = rFnt.GetTxtSize( pPrinter, aText, nStart, nEnd-nStart ).
                       Width();
        aTextWidth[ nIdx++ ] = nWidth;
        nTxtWidth += nWidth;
        if( nScript==ScriptType::ASIAN )
        {
            if( !nCJKHeight )
            {
                pWin->SetFont( aCJKFont );
                FontMetric aMetric( pWin->GetFontMetric() );
                nCJKHeight = aMetric.GetLineHeight();
                nCJKAscent = aMetric.GetAscent();
            }
        }
        else
        {
            if( !nHeight )
            {
                pWin->SetFont( rFont );
                FontMetric aMetric( pWin->GetFontMetric() );
                nHeight = aMetric.GetLineHeight();
                nAscent = aMetric.GetAscent();
            }
        }
        if( nEnd < aText.Len() && nIdx < nCnt )
        {
            nStart = nEnd;
            nEnd = aScriptChg[ nIdx ];
            nScript = aScriptType[ nIdx ];
        }
        else
            break;
    }
    while( TRUE );
    nHeight -= nAscent;
    nCJKHeight -= nCJKAscent;
    if( nHeight < nCJKHeight )
        nHeight = nCJKHeight;
    if( nAscent < nCJKAscent )
        nAscent = nCJKAscent;
    nHeight += nAscent;

    Size aTxtSize( nTxtWidth, nHeight );
    return aTxtSize;
}

/*-----------------19.7.2001 08:54------------------
 * void FontPrevWin_Impl::DrawPrev(..)
 * calls SvxFont::DrawPrev(..) for every part of the string without a script
 * change inside, for Asian parts the aCJKFont will be used, otherwise the
 * given rFont.
 * --------------------------------------------------*/

void FontPrevWin_Impl::DrawPrev( OutputDevice* pWin, Printer* pPrinter,
    Point &rPt, SvxFont &rFont )
{
    Font aOldFont = pPrinter->GetFont();
    USHORT nScript;
    USHORT nIdx = 0;
    xub_StrLen nStart = 0;
    xub_StrLen nEnd;
    USHORT nCnt = aScriptChg.Count();
    if( nCnt )
    {
        nEnd = aScriptChg[ nIdx ];
        nScript = aScriptType[ nIdx ];
    }
    else
    {
        nEnd = aText.Len();
        nScript = ScriptType::LATIN;
    }
    do
    {
        SvxFont& rFnt = (nScript==ScriptType::ASIAN) ? aCJKFont : rFont;
        pPrinter->SetFont( rFnt );

        rFnt.DrawPrev( pWin, pPrinter, rPt, aText, nStart, nEnd - nStart );

        rPt.X() += aTextWidth[ nIdx++ ];
        if( nEnd < aText.Len() && nIdx < nCnt )
        {
            nStart = nEnd;
            nEnd = aScriptChg[ nIdx ];
            nScript = aScriptType[ nIdx ];
        }
        else
            break;
    }
    while( TRUE );
    pPrinter->SetFont( aOldFont );
}

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
    pImpl->aCJKFont.SetTransparent(TRUE);
    aFont.SetAlign(ALIGN_BASELINE);
    pImpl->aCJKFont.SetAlign(ALIGN_BASELINE);
    InitSettings( TRUE, TRUE );
    SetBorderStyle( WINDOW_BORDER_MONO );
}

// -----------------------------------------------------------------------

SvxFontPrevWindow::~SvxFontPrevWindow()
{
    delete pImpl->pColor;
    delete pImpl->pBackColor;
    delete pImpl;

    if ( bDelPrinter )
        delete pPrinter;
}

// -----------------------------------------------------------------------

SvxFont& SvxFontPrevWindow::GetCJKFont()
{
    return pImpl->aCJKFont;
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

void SvxFontPrevWindow::SetBackColor(const Color &rColor)
{
    delete pImpl->pBackColor;
    pImpl->pBackColor = new Color( rColor );
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

    pImpl->CheckScript();
    Size aTxtSize = pImpl->CalcTextSize( this, pPrinter, aFont );

    const Size aLogSize( GetOutputSize() );

    long nX = aLogSize.Width()  / 2 - aTxtSize.Width() / 2;
    long nY = aLogSize.Height() / 2 - aTxtSize.Height() / 2;

    if ( nY + pImpl->nAscent > aLogSize.Height() )
        nY = aLogSize.Height() - pImpl->nAscent;

    if ( pImpl->pBackColor )
    {
        Rectangle aRect( Point( 0, 0 ), aLogSize );
        Color aLineCol = GetLineColor();
        Color aFillCol = GetFillColor();
        SetLineColor();
        SetFillColor( *pImpl->pBackColor );
        DrawRect( aRect );
        SetLineColor( aLineCol );
        SetFillColor( aFillCol );
    }
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

    long nStdAscent = pImpl->nAscent;
    nY += nStdAscent;

    if(pImpl->bTwoLines)
    {
        SvxFont aSmallFont(aFont);
        Size aSize( aSmallFont.GetSize() );
        aSize.Height() = ( aSize.Height() * 3 ) / 5;
        aSize.Width() = ( aSize.Width() * 3 ) / 5;
        aSmallFont.SetSize( aSize );
        aSize = pImpl->aCJKFont.GetSize();
        {
            Size aTmpSize;
            aTmpSize.Height() = ( aSize.Height() * 3 ) / 5;
            aTmpSize.Width() = ( aSize.Width() * 3 ) / 5;
            pImpl->aCJKFont.SetSize( aTmpSize );
        }

        long nStartBracketWidth = 0;
        long nEndBracketWidth = 0;
        long nTextWidth = 0;
        if(pImpl->cStartBracket)
        {
            String sBracket(pImpl->cStartBracket);
            nStartBracketWidth = aFont.GetTxtSize( pPrinter, sBracket ).Width();
        }
        if(pImpl->cEndBracket)
        {
            String sBracket(pImpl->cEndBracket);
            nEndBracketWidth = aFont.GetTxtSize( pPrinter, sBracket ).Width();
        }
        nTextWidth = pImpl->CalcTextSize( this, pPrinter, aSmallFont ).Width();
        long nResultWidth = nStartBracketWidth;
        nResultWidth += nEndBracketWidth;
        nResultWidth += nTextWidth;

        long nX = (aLogSize.Width() - nResultWidth) / 2;
        DrawLine( Point( 0,  nY ), Point( nX, nY ) );
        DrawLine( Point( nX + nResultWidth, nY ), Point( aLogSize.Width(), nY ) );

        long nSmallAscent = pImpl->nAscent;
        long nOffset = (nStdAscent - nSmallAscent ) / 2;

        if(pImpl->cStartBracket)
        {
            String sBracket(pImpl->cStartBracket);
            aFont.DrawPrev( this, pPrinter, Point( nX, nY - nOffset - 4), sBracket );
            nX += nStartBracketWidth;
        }

        pImpl->DrawPrev( this, pPrinter, Point( nX, nY - nSmallAscent - 2 ), aSmallFont );
        pImpl->DrawPrev( this, pPrinter, Point( nX, nY ), aSmallFont );

        nX += nTextWidth;
        if(pImpl->cEndBracket)
        {
            String sBracket(pImpl->cEndBracket);
            aFont.DrawPrev( this, pPrinter, Point( nX + 1, nY - nOffset - 4), sBracket );
        }
        pImpl->aCJKFont.SetSize( aSize );
    }
    else
    {
        DrawLine( Point( 0,  nY ), Point( nX, nY ) );
        DrawLine( Point( nX + aTxtSize.Width(), nY ), Point( aLogSize.Width(), nY ) );
        pImpl->DrawPrev( this, pPrinter, Point( nX, nY ), aFont );
    }
}
/* -----------------------------04.12.00 16:26--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SvxFontPrevWindow::IsTwoLines() const
{
    return pImpl->bTwoLines;
}
/* -----------------------------04.12.00 16:26--------------------------------

 ---------------------------------------------------------------------------*/
void SvxFontPrevWindow::SetTwoLines(BOOL bSet)
{
    pImpl->bTwoLines = bSet;}

/* -----------------------------04.12.00 16:26--------------------------------

 ---------------------------------------------------------------------------*/
void SvxFontPrevWindow::SetBrackets(sal_Unicode cStart, sal_Unicode cEnd)
{
    pImpl->cStartBracket = cStart;
    pImpl->cEndBracket = cEnd;
}

