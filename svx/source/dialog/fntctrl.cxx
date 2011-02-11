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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <sfx2/viewsh.hxx>      // SfxViewShell
#include <sfx2/printer.hxx>     // Printer
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <unicode/uchar.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_XUB_STRLEN
#include <svl/svstdarr.hxx>
#endif
#include <svtools/colorcfg.hxx>

#include <svx/fntctrl.hxx>
#include <svx/dialogs.hrc>
#define TEXT_WIDTH  20

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using ::com::sun::star::i18n::XBreakIterator;

// -----------------------------------------------------------------------
// small helper functions to set fonts
// -----------------------------------------------------------------------
namespace
{
    void scaleFontWidth(Font& _rFont,const OutputDevice& rOutDev,long& _n100PercentFont)
    {
        _rFont.SetWidth( 0 );
        _n100PercentFont = rOutDev.GetFontMetric( _rFont ).GetWidth();
    }
    // -----------------------------------------------------------------------
    void initFont(Font& _rFont)
    {
        _rFont.SetTransparent(sal_True);
        _rFont.SetAlign(ALIGN_BASELINE);
    }
    // -----------------------------------------------------------------------
    void setFontSize(Font& _rFont)
    {
        Size aSize( _rFont.GetSize() );
        aSize.Height() = ( aSize.Height() * 3 ) / 5;
        aSize.Width() = ( aSize.Width() * 3 ) / 5;
        _rFont.SetSize( aSize );
    }
    // -----------------------------------------------------------------------
    void calcFontHeightAnyAscent(OutputDevice* _pWin,Font& _rFont,long& _nHeight,long& _nAscent)
    {
        if ( !_nHeight )
        {
            _pWin->SetFont( _rFont );
            FontMetric aMetric( _pWin->GetFontMetric() );
            _nHeight = aMetric.GetLineHeight();
            _nAscent = aMetric.GetAscent();
        }
    }
    // -----------------------------------------------------------------------
    void setFont( const SvxFont& rNewFont, SvxFont& rImplFont )
    {
        rImplFont = rNewFont;
        rImplFont.SetTransparent( sal_True );
        rImplFont.SetAlign( ALIGN_BASELINE );
    }

}
// -----------------------------------------------------------------------


// class FontPrevWin_Impl -----------------------------------------------

class FontPrevWin_Impl
{
    friend class SvxFontPrevWindow;

    SvxFont                         aFont;
    Printer*                        pPrinter;
    sal_Bool                            bDelPrinter;

    Reference < XBreakIterator >    xBreak;
    SvULongs                        aTextWidth;
    SvXub_StrLens                   aScriptChg;
    SvUShorts                       aScriptType;
    SvxFont                         aCJKFont;
    SvxFont                         aCTLFont;
    String                          aText;
    String                          aScriptText;
    Color*                          pColor;
    Color*                          pBackColor;
    long                            nAscent;
    sal_Unicode                     cStartBracket;
    sal_Unicode                     cEndBracket;

    long                            n100PercentFontWidth;       // initial -1 -> not set yet
    long                            n100PercentFontWidthCJK;
    long                            n100PercentFontWidthCTL;
    sal_uInt16                          nFontWidthScale;

    sal_Bool                            bSelection      : 1,
                                    bGetSelection   : 1,
                                    bUseResText     : 1,
                                    bTwoLines       : 1,
                                    bIsCJKUI        : 1,
                                    bIsCTLUI        : 1,
                                    bUseFontNameAsText : 1,
                                    bTextInited     : 1;

    void                _CheckScript();
public:
    inline FontPrevWin_Impl() :
        pPrinter( NULL ), bDelPrinter( sal_False ),
        pColor( NULL ), pBackColor( 0 ),
        cStartBracket( 0 ), cEndBracket( 0 ), nFontWidthScale( 100 ),
        bSelection( sal_False ), bGetSelection( sal_False ), bUseResText( sal_False ),
        bTwoLines( sal_False ),
        bIsCJKUI( sal_False ), bIsCTLUI( sal_False ),
        bUseFontNameAsText( sal_False ), bTextInited( sal_False )
        {
            Invalidate100PercentFontWidth();
        }

    inline ~FontPrevWin_Impl()
    {
        delete pColor;
        delete pBackColor;
        if( bDelPrinter )
            delete pPrinter;
    }

    void                CheckScript();
    Size                CalcTextSize( OutputDevice* pWin, OutputDevice* pPrt, SvxFont &rFont );
    void                DrawPrev( OutputDevice* pWin, Printer* pPrt, Point &rPt, SvxFont &rFont );

    sal_Bool                SetFontWidthScale( sal_uInt16 nScaleInPercent );
    inline void         Invalidate100PercentFontWidth();
    inline sal_Bool         Is100PercentFontWidthValid() const;
    void                ScaleFontWidth( const OutputDevice& rOutDev );
                            // scales rNonCJKFont and aCJKFont depending on nFontWidthScale and
                            //  sets the 100%-Font-Widths
};

void FontPrevWin_Impl::CheckScript()
{
    if( aText != aScriptText )
        _CheckScript();
}

inline void FontPrevWin_Impl::Invalidate100PercentFontWidth()
{
    n100PercentFontWidth = n100PercentFontWidthCJK = n100PercentFontWidthCTL = -1;
}

inline sal_Bool FontPrevWin_Impl::Is100PercentFontWidthValid() const
{
    DBG_ASSERT( ( n100PercentFontWidth == -1 && n100PercentFontWidthCJK == -1 ) ||
                ( n100PercentFontWidth != -1 && n100PercentFontWidthCJK != -1 ) ||
                ( n100PercentFontWidth == -1 && n100PercentFontWidthCTL == -1 ) ||
                ( n100PercentFontWidth != -1 && n100PercentFontWidthCTL != -1 ),
                "*FontPrevWin_Impl::Is100PercentFontWidthValid(): 100PercentFontWidth's not synchronous" );
    return n100PercentFontWidth != -1;
}

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
    size_t nCnt = aScriptChg.size();
    if( nCnt )
    {
        aScriptChg.clear();
        aScriptType.Remove( 0, nCnt );
        aTextWidth.Remove( 0, nCnt );
        nCnt = 0;
    }
    if( !xBreak.is() )
    {
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        xBreak = Reference< XBreakIterator >(xMSF->createInstance(
                ::rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) ),UNO_QUERY);
    }
    if( xBreak.is() )
    {
        sal_uInt16 nScript = xBreak->getScriptType( aText, 0 );
        sal_uInt16 nChg = 0;
        if( com::sun::star::i18n::ScriptType::WEAK == nScript )
        {
            nChg = (xub_StrLen)xBreak->endOfScript( aText, nChg, nScript );
            if( nChg < aText.Len() )
                nScript = xBreak->getScriptType( aText, nChg );
            else
                nScript = com::sun::star::i18n::ScriptType::LATIN;
        }

        do
        {
            nChg = (xub_StrLen)xBreak->endOfScript( aText, nChg, nScript );
            if (nChg < aText.Len() && nChg > 0 &&
                (com::sun::star::i18n::ScriptType::WEAK ==
                 xBreak->getScriptType(aText, nChg - 1)))
            {
                int8_t nType = u_charType(aText.GetChar(nChg) );
                if (nType == U_NON_SPACING_MARK || nType == U_ENCLOSING_MARK ||
                    nType == U_COMBINING_SPACING_MARK )
                {
                    aScriptChg.push_back( nChg - 1 );
                }
                else
                {
                    aScriptChg.push_back( nChg );
                }
            }
            else
            {
                aScriptChg.push_back( nChg );
            }
            aScriptType.Insert( nScript, nCnt );
            aTextWidth.Insert( sal_uIntPtr(0), nCnt++ );

            if( nChg < aText.Len() )
                nScript = xBreak->getScriptType( aText, nChg );
            else
                break;
        } while( sal_True );
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

Size FontPrevWin_Impl::CalcTextSize( OutputDevice* pWin, OutputDevice* _pPrinter,
    SvxFont &rFont )
{
    sal_uInt16 nScript;
    sal_uInt16 nIdx = 0;
    xub_StrLen nStart = 0;
    xub_StrLen nEnd;
    size_t nCnt = aScriptChg.size();
    if( nCnt )
    {
        nEnd = aScriptChg[ nIdx ];
        nScript = aScriptType[ nIdx ];
    }
    else
    {
        nEnd = aText.Len();
        nScript = com::sun::star::i18n::ScriptType::LATIN;
    }
    long nTxtWidth = 0;
    long nCJKHeight = 0;
    long nCTLHeight = 0;
    long nHeight = 0;
    nAscent = 0;
    long nCJKAscent = 0;
    long nCTLAscent = 0;
    do
    {
        SvxFont& rFnt = (nScript==com::sun::star::i18n::ScriptType::ASIAN) ? aCJKFont : ((nScript==com::sun::star::i18n::ScriptType::COMPLEX) ? aCTLFont : rFont);
        sal_uIntPtr nWidth = rFnt.GetTxtSize( _pPrinter, aText, nStart, nEnd-nStart ).
                       Width();
        aTextWidth[ nIdx++ ] = nWidth;
        nTxtWidth += nWidth;
        switch(nScript)
        {
            case com::sun::star::i18n::ScriptType::ASIAN:
                calcFontHeightAnyAscent(pWin,aCJKFont,nCJKHeight,nCJKAscent);
                break;
            case com::sun::star::i18n::ScriptType::COMPLEX:
                calcFontHeightAnyAscent(pWin,aCTLFont,nCTLHeight,nCTLAscent);
                break;
            default:
                calcFontHeightAnyAscent(pWin,rFont,nHeight,nAscent);
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
    while( sal_True );
    nHeight -= nAscent;
    nCJKHeight -= nCJKAscent;
    nCTLHeight -= nCTLAscent;
    if( nHeight < nCJKHeight )
        nHeight = nCJKHeight;
    if( nAscent < nCJKAscent )
        nAscent = nCJKAscent;
    if( nHeight < nCTLHeight )
        nHeight = nCTLHeight;
    if( nAscent < nCTLAscent )
        nAscent = nCTLAscent;
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

void FontPrevWin_Impl::DrawPrev( OutputDevice* pWin, Printer* _pPrinter,
    Point &rPt, SvxFont &rFont )
{
    Font aOldFont = _pPrinter->GetFont();
    sal_uInt16 nScript;
    sal_uInt16 nIdx = 0;
    xub_StrLen nStart = 0;
    xub_StrLen nEnd;
    size_t nCnt = aScriptChg.size();
    if( nCnt )
    {
        nEnd = aScriptChg[ nIdx ];
        nScript = aScriptType[ nIdx ];
    }
    else
    {
        nEnd = aText.Len();
        nScript = com::sun::star::i18n::ScriptType::LATIN;
    }
    do
    {
        SvxFont& rFnt = (nScript==com::sun::star::i18n::ScriptType::ASIAN) ? aCJKFont : ((nScript==com::sun::star::i18n::ScriptType::COMPLEX) ? aCTLFont : rFont);
        _pPrinter->SetFont( rFnt );

        rFnt.DrawPrev( pWin, _pPrinter, rPt, aText, nStart, nEnd - nStart );

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
    while( sal_True );
    _pPrinter->SetFont( aOldFont );
}

// -----------------------------------------------------------------------

sal_Bool FontPrevWin_Impl::SetFontWidthScale( sal_uInt16 nScale )
{
    if( nFontWidthScale != nScale )
    {
        nFontWidthScale = nScale;
        return sal_True;
    }

    return sal_False;
}


// -----------------------------------------------------------------------

void FontPrevWin_Impl::ScaleFontWidth( const OutputDevice& rOutDev )
{
    if( !Is100PercentFontWidthValid() )
    {
        scaleFontWidth(aFont,rOutDev,n100PercentFontWidth);
        scaleFontWidth(aCJKFont,rOutDev,n100PercentFontWidthCJK);
        scaleFontWidth(aCTLFont,rOutDev,n100PercentFontWidthCTL);
    }

    aFont.SetWidth( n100PercentFontWidth * nFontWidthScale / 100 );
    aCJKFont.SetWidth( n100PercentFontWidthCJK * nFontWidthScale / 100 );
    aCTLFont.SetWidth( n100PercentFontWidthCTL * nFontWidthScale / 100 );
}

// class SvxFontPrevWindow -----------------------------------------------

void SvxFontPrevWindow::InitSettings( sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if ( bForeground )
    {
        svtools::ColorConfig aColorConfig;
        Color aTextColor( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );

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

    Window     ( pParent, rId )
{
    pImpl = new FontPrevWin_Impl;
    SfxViewShell* pSh = SfxViewShell::Current();

    if ( pSh )
        pImpl->pPrinter = pSh->GetPrinter();

    if ( !pImpl->pPrinter )
    {
        pImpl->pPrinter = new Printer;
        pImpl->bDelPrinter = sal_True;
    }
    SetMapMode( MapMode( MAP_TWIP ) );
    initFont(pImpl->aFont);
    initFont(pImpl->aCJKFont);
    initFont(pImpl->aCTLFont);
    InitSettings( sal_True, sal_True );
    SetBorderStyle( WINDOW_BORDER_MONO );

    LanguageType eLanguage = Application::GetSettings().GetUILanguage();
    switch( eLanguage )
    {
        case LANGUAGE_CHINESE:
        case LANGUAGE_JAPANESE:
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
        case LANGUAGE_CHINESE_SIMPLIFIED:
        case LANGUAGE_CHINESE_HONGKONG:
        case LANGUAGE_CHINESE_SINGAPORE:
        case LANGUAGE_CHINESE_MACAU:
        case LANGUAGE_CHINESE_TRADITIONAL:
            pImpl->bIsCJKUI = sal_True;
            break;
        // TODO: CTL Locale
        //  pImpl->bIsCTLUI = sal_True;
        //  break;
        default:
            pImpl->bIsCJKUI = pImpl->bIsCTLUI = sal_False;
            break;
    }
}

// -----------------------------------------------------------------------

SvxFontPrevWindow::~SvxFontPrevWindow()
{
    delete pImpl;
}

// -----------------------------------------------------------------------
SvxFont& SvxFontPrevWindow::GetCTLFont()
{
    return pImpl->aCTLFont;
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
        InitSettings( sal_True, sal_False );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
        InitSettings( sal_False, sal_True );

    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitSettings( sal_True, sal_True );
    else
        Window::DataChanged( rDCEvt );
}

SvxFont& SvxFontPrevWindow::GetFont()
{
    pImpl->Invalidate100PercentFontWidth();     // because the user might change the size
    return pImpl->aFont;
}

const SvxFont& SvxFontPrevWindow::GetFont() const
{
    return pImpl->aFont;
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetPreviewText( const ::rtl::OUString& rString )
{
    pImpl->aText = rString;
    pImpl->bTextInited = sal_True;
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFontNameAsPreviewText()
{
    pImpl->bUseFontNameAsText = sal_True;
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFont( const SvxFont& rOutFont )
{
    setFont( rOutFont, pImpl->aFont );

    pImpl->Invalidate100PercentFontWidth();
    Invalidate();
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFont( const SvxFont& rNormalOutFont, const SvxFont& rCJKOutFont, const SvxFont& rCTLFont )
{
    setFont( rNormalOutFont, pImpl->aFont );
    setFont( rCJKOutFont, pImpl->aCJKFont );
    setFont( rCTLFont, pImpl->aCTLFont );


    pImpl->Invalidate100PercentFontWidth();
    Invalidate();
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetCJKFont( const SvxFont &rCJKOutFont )
{
    setFont( rCJKOutFont, pImpl->aCJKFont );

    pImpl->Invalidate100PercentFontWidth();
    Invalidate();
}
// -----------------------------------------------------------------------------
void SvxFontPrevWindow::SetCTLFont( const SvxFont &rCTLOutFont )
{
    setFont( rCTLOutFont, pImpl->aCTLFont );

    pImpl->Invalidate100PercentFontWidth();
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

void SvxFontPrevWindow::ResetColor()
{
    delete pImpl->pColor;
    pImpl->pColor = 0;
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

void SvxFontPrevWindow::UseResourceText( sal_Bool bUse )
{
    pImpl->bUseResText = bUse;
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::Paint( const Rectangle& )
{
    Printer* pPrinter = pImpl->pPrinter;
    SvxFont& rFont = pImpl->aFont;
    SvxFont& rCJKFont = pImpl->aCJKFont;
    // TODO: SvxFont& rCTLFont = pImpl->aCTLFont;

    if ( pImpl->bUseResText )
        pImpl->aText = GetText();
    else if ( !pImpl->bSelection && !pImpl->bTextInited )
    {
        SfxViewShell* pSh = SfxViewShell::Current();

        if ( pSh && !pImpl->bGetSelection && !pImpl->bUseFontNameAsText )
        {
            pImpl->aText = pSh->GetSelectionText();
            pImpl->bGetSelection = sal_True;
            pImpl->bSelection = pImpl->aText.Len() != 0;

        }

        if ( !pImpl->bSelection || pImpl->bUseFontNameAsText )
        {
            pImpl->aText = rFont.GetName();
            if( pImpl->bIsCJKUI )
                pImpl->aText += rCJKFont.GetName();
            //TODO bIsCTLUI
        }

        if ( !pImpl->aText.Len() )
            pImpl->aText = GetText();

        // remove line feeds and carriage returns from string
        bool bNotEmpty = false;
        for ( xub_StrLen i = 0; i < pImpl->aText.Len(); ++i )
        {
            if ( 0xa == pImpl->aText.GetChar( i ) ||
                 0xd == pImpl->aText.GetChar( i ) )
                 pImpl->aText.SetChar( i, ' ' );
            else
                bNotEmpty = true;
        }
        if ( !bNotEmpty )
            pImpl->aText = GetText();

        if ( pImpl->aText.Len() > (TEXT_WIDTH-1) )
            pImpl->aText.Erase( pImpl->aText.Search( sal_Unicode( ' ' ), TEXT_WIDTH ) );
    }

    // calculate text width scaling
    pImpl->ScaleFontWidth( *this/*, rFont*/ );

    pImpl->CheckScript();
    Size aTxtSize = pImpl->CalcTextSize( this, pPrinter, rFont );

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
        SvxFont aSmallFont( rFont );
        Size aOldSize = pImpl->aCJKFont.GetSize();
        setFontSize(aSmallFont);
        setFontSize(pImpl->aCJKFont);

        long nStartBracketWidth = 0;
        long nEndBracketWidth = 0;
        long nTextWidth = 0;
        if(pImpl->cStartBracket)
        {
            String sBracket(pImpl->cStartBracket);
            nStartBracketWidth = rFont.GetTxtSize( pPrinter, sBracket ).Width();
        }
        if(pImpl->cEndBracket)
        {
            String sBracket(pImpl->cEndBracket);
            nEndBracketWidth = rFont.GetTxtSize( pPrinter, sBracket ).Width();
        }
        nTextWidth = pImpl->CalcTextSize( this, pPrinter, aSmallFont ).Width();
        long nResultWidth = nStartBracketWidth;
        nResultWidth += nEndBracketWidth;
        nResultWidth += nTextWidth;

        long _nX = (aLogSize.Width() - nResultWidth) / 2;
        DrawLine( Point( 0,  nY ), Point( _nX, nY ) );
        DrawLine( Point( _nX + nResultWidth, nY ), Point( aLogSize.Width(), nY ) );

        long nSmallAscent = pImpl->nAscent;
        long nOffset = (nStdAscent - nSmallAscent ) / 2;

        if(pImpl->cStartBracket)
        {
            String sBracket(pImpl->cStartBracket);
            rFont.DrawPrev( this, pPrinter, Point( _nX, nY - nOffset - 4), sBracket );
            _nX += nStartBracketWidth;
        }

        Point aTmpPoint1( _nX, nY - nSmallAscent - 2 );
        Point aTmpPoint2( _nX, nY );
        pImpl->DrawPrev( this, pPrinter, aTmpPoint1, aSmallFont );
        pImpl->DrawPrev( this, pPrinter, aTmpPoint2, aSmallFont );

        _nX += nTextWidth;
        if(pImpl->cEndBracket)
        {
            Point aTmpPoint( _nX + 1, nY - nOffset - 4);
            String sBracket(pImpl->cEndBracket);
            rFont.DrawPrev( this, pPrinter, aTmpPoint, sBracket );
        }
        pImpl->aCJKFont.SetSize( aOldSize );
    }
    else
    {
        Color aLineCol = GetLineColor();

        SetLineColor( rFont.GetColor() );
        DrawLine( Point( 0,  nY ), Point( nX, nY ) );
        DrawLine( Point( nX + aTxtSize.Width(), nY ), Point( aLogSize.Width(), nY ) );

        SetLineColor( aLineCol );

        Point aTmpPoint( nX, nY );
        pImpl->DrawPrev( this, pPrinter, aTmpPoint, rFont );
    }
}
/* -----------------------------04.12.00 16:26--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SvxFontPrevWindow::IsTwoLines() const
{
    return pImpl->bTwoLines;
}
/* -----------------------------04.12.00 16:26--------------------------------

 ---------------------------------------------------------------------------*/
void SvxFontPrevWindow::SetTwoLines(sal_Bool bSet)
{
    pImpl->bTwoLines = bSet;}

/* -----------------------------04.12.00 16:26--------------------------------

 ---------------------------------------------------------------------------*/
void SvxFontPrevWindow::SetBrackets(sal_Unicode cStart, sal_Unicode cEnd)
{
    pImpl->cStartBracket = cStart;
    pImpl->cEndBracket = cEnd;
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFontWidthScale( sal_uInt16 n )
{
    if( pImpl->SetFontWidthScale( n ) )
        Invalidate();
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::AutoCorrectFontColor( void )
{
    Color   aFontColor( GetTextColor() );

    if( COL_AUTO == pImpl->aFont.GetColor().GetColor() )
        pImpl->aFont.SetColor( aFontColor );

    if( COL_AUTO == pImpl->aCJKFont.GetColor().GetColor() )
        pImpl->aCJKFont.SetColor( aFontColor );

    if( COL_AUTO == pImpl->aCTLFont.GetColor().GetColor() )
        pImpl->aCTLFont.SetColor( aFontColor );
}
