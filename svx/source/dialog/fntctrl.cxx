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

#include <sfx2/viewsh.hxx>      // SfxViewShell
#include <sfx2/printer.hxx>     // Printer
#include <vcl/builder.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <unicode/uchar.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>

#include <vector>
#include <deque>
#include <svtools/colorcfg.hxx>
#include <svtools/sampletext.hxx>

#include <svx/fntctrl.hxx>
#include <svx/dialogs.hrc>
#include <svx/sdasitm.hxx>

// Item set includes
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/stritem.hxx>                  // SfxStringItem
#include <svl/languageoptions.hxx>

#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/editids.hrc>      // #define SID_ATTR_*
#include <editeng/fontitem.hxx>     // SvxFontItem
#include <editeng/postitem.hxx>     // SvxPostureItem
#include <editeng/udlnitem.hxx>     // SvxUnderlineItem
#include <editeng/crsditem.hxx>     // SvxCrossedOutItem
#include <editeng/cntritem.hxx>     // SvxContourItem
#include <editeng/wghtitem.hxx>     // SvxWeightItem
#include <editeng/fhgtitem.hxx>     // SvxFontHeightItem
#include <editeng/shdditem.hxx>     // SvxShadowedItem
#include <editeng/escpitem.hxx>     // SvxEscapementItem
#include <editeng/wrlmitem.hxx>     // SvxWordLineModeItem
#include <editeng/cmapitem.hxx>     // SvxWordLineModeItem
#include <editeng/kernitem.hxx>     // SvxKerningItem
#include <editeng/brshitem.hxx>     // SvxBrushItem
#include <editeng/colritem.hxx>     // SvxColorItem
#include <editeng/emphitem.hxx>     // SvxEmphasisMarkItem
#include <editeng/charreliefitem.hxx>       // SvxCharReliefItem
#include <editeng/twolinesitem.hxx>         // SvxTwoLinesItem
#include <editeng/charscaleitem.hxx>        // SvxCharScaleWidthItem
#include <editeng/langitem.hxx>     // SvxLanguageItem

// define ----------------------------------------------------------------

//TODO: remove this and calculate off the actual size of text, not
//an arbitrary number of characters
#define TEXT_WIDTH 80
#define ISITEMSET rSet.GetItemState(nWhich)>=SFX_ITEM_DEFAULT

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
    void calcFontHeightAnyAscent(OutputDevice* _pWin,const Font& _rFont,long& _nHeight,long& _nAscent)
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
    sal_Bool                        bDelPrinter;

    Reference < XBreakIterator >    xBreak;
    std::vector<sal_uIntPtr>        aTextWidth;
    std::deque<xub_StrLen>         aScriptChg;
    std::vector<sal_uInt16>         aScriptType;
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
    sal_uInt16                      nFontWidthScale;

    sal_Bool                        bSelection      : 1,
                                    bGetSelection   : 1,
                                    bUseResText     : 1,
                                    bPreviewBackgroundToCharacter : 1,
                                    bTwoLines       : 1,
                                    bUseFontNameAsText : 1,
                                    bTextInited     : 1;

    bool m_bCJKEnabled;
    bool m_bCTLEnabled;


public:
    inline FontPrevWin_Impl() :
        pPrinter( NULL ), bDelPrinter( sal_False ),
        pColor( NULL ), pBackColor( 0 ), nAscent( 0 ),
        cStartBracket( 0 ), cEndBracket( 0 ), nFontWidthScale( 100 ),
        bSelection( sal_False ), bGetSelection( sal_False ), bUseResText( sal_False ),
        bPreviewBackgroundToCharacter( sal_False ), bTwoLines( sal_False ),
        bUseFontNameAsText( sal_False ), bTextInited( sal_False )
    {
        SvtLanguageOptions aLanguageOptions;
        m_bCJKEnabled = aLanguageOptions.IsAnyEnabled();
        m_bCTLEnabled = aLanguageOptions.IsCTLFontEnabled();

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
    Size                CalcTextSize( OutputDevice* pWin, OutputDevice* pPrt, const SvxFont &rFont );
    void                DrawPrev( OutputDevice* pWin, Printer* pPrt, Point &rPt, const SvxFont &rFont );

    sal_Bool            SetFontWidthScale( sal_uInt16 nScaleInPercent );
    inline void         Invalidate100PercentFontWidth();
    inline sal_Bool     Is100PercentFontWidthValid() const;
    void                ScaleFontWidth( const OutputDevice& rOutDev );
                            // scales rNonCJKFont and aCJKFont depending on nFontWidthScale and
                            //  sets the 100%-Font-Widths
};

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

/*
 * evalutates the scripttypes of the actual string.
 * Afterwards the positions of script change are notified in aScriptChg,
 * the scripttypes in aScriptType.
 * The aTextWidth array will be filled with zero.
 */
void FontPrevWin_Impl::CheckScript()
{
    assert(aText.Len()); // must have a preview text here!
    if (aText == aScriptText)
    {
        return; // already initialized
    }

    aScriptText = aText;

    aScriptChg.clear();
    aScriptType.clear();
    aTextWidth.clear();

    if( !xBreak.is() )
    {
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        xBreak = Reference< XBreakIterator >(xMSF->createInstance(
                ::rtl::OUString("com.sun.star.i18n.BreakIterator") ),UNO_QUERY);
    }
    assert(xBreak.is()); // no can do without breakiter
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
            aScriptType.push_back( nScript );
            aTextWidth.push_back( 0 );

            if( nChg < aText.Len() )
                nScript = xBreak->getScriptType( aText, nChg );
            else
                break;
        } while( sal_True );
    }
}

/*
 * Size FontPrevWin_Impl::CalcTextSize(..)
 * fills the aTextWidth array with the text width of every part
 * of the actual string without a script change inside.
 * For Latin parts the given rFont will be used,
 * for Asian parts the aCJKFont.
 * The returned size contains the whole string.
 * The member nAscent is calculated to the maximal ascent of all used fonts.
 */

Size FontPrevWin_Impl::CalcTextSize( OutputDevice* pWin, OutputDevice* _pPrinter,
    const SvxFont &rFont )
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
        const SvxFont& rFnt = (nScript==com::sun::star::i18n::ScriptType::ASIAN) ? aCJKFont : ((nScript==com::sun::star::i18n::ScriptType::COMPLEX) ? aCTLFont : rFont);
        sal_uIntPtr nWidth = rFnt.GetTxtSize( _pPrinter, aText, nStart, nEnd-nStart ).
                       Width();
        if (nIdx >= aTextWidth.size())
            break;
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

/*
 * void FontPrevWin_Impl::DrawPrev(..)
 * calls SvxFont::DrawPrev(..) for every part of the string without a script
 * change inside, for Asian parts the aCJKFont will be used, otherwise the
 * given rFont.
 */

void FontPrevWin_Impl::DrawPrev( OutputDevice* pWin, Printer* _pPrinter,
    Point &rPt, const SvxFont &rFont )
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
        const SvxFont& rFnt = (nScript==com::sun::star::i18n::ScriptType::ASIAN) ? aCJKFont : ((nScript==com::sun::star::i18n::ScriptType::COMPLEX) ? aCTLFont : rFont);
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

void SvxFontPrevWindow::Init()
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
}

SvxFontPrevWindow::SvxFontPrevWindow( Window* pParent, const ResId& rId ) :
    Window( pParent, rId )
{
    m_aInitialSize = GetSizePixel();
    Init();
}

SvxFontPrevWindow::SvxFontPrevWindow(Window* pParent) :
    Window(pParent)
{
    Init();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxFontPrevWindow(Window *pParent, VclBuilder::stringmap &)
{
    SvxFontPrevWindow *pWindow = new SvxFontPrevWindow(pParent);
    return pWindow;
}

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
    Invalidate();
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

void SvxFontPrevWindow::SetFont( const SvxFont& rNormalOutFont, const SvxFont& rCJKOutFont, const SvxFont& rCTLFont )
{
    setFont( rNormalOutFont, pImpl->aFont );
    setFont( rCJKOutFont, pImpl->aCJKFont );
    setFont( rCTLFont, pImpl->aCTLFont );


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
    const SvxFont& rFont = pImpl->aFont;
    const SvxFont& rCJKFont = pImpl->aCJKFont;
    const SvxFont& rCTLFont = pImpl->aCTLFont;

    if ( !IsEnabled() )
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size aLogSize( GetOutputSize() );

        Rectangle aRect( Point( 0, 0 ), aLogSize );
        SetLineColor();
        SetFillColor( rStyleSettings.GetWindowColor() );
        DrawRect( aRect );
    }
    else
    {
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
                using namespace com::sun::star::i18n::ScriptType;

                //If we're showing multiple sample texts, then they're all
                //sample texts. If only showing Latin, continue to use
                //the fontname as the preview
                if ((pImpl->m_bCJKEnabled) || (pImpl->m_bCTLEnabled))
                    pImpl->aText = makeRepresentativeTextForFont(LATIN, rFont);
                else
                    pImpl->aText = rFont.GetName();

                if (pImpl->m_bCJKEnabled)
                {
                    if (pImpl->aText.Len())
                        pImpl->aText.AppendAscii("   ");
                    pImpl->aText += makeRepresentativeTextForFont(ASIAN, rCJKFont);

                }
                if (pImpl->m_bCTLEnabled)
                {
                    if (pImpl->aText.Len())
                        pImpl->aText.AppendAscii("   ");
                    pImpl->aText += makeRepresentativeTextForFont(COMPLEX, rCTLFont);
                }
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

        if (IsTwoLines())
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
                rtl::OUString sBracket(pImpl->cStartBracket);
                nStartBracketWidth = rFont.GetTxtSize( pPrinter, sBracket ).Width();
            }
            if(pImpl->cEndBracket)
            {
                rtl::OUString sBracket(pImpl->cEndBracket);
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
                rtl::OUString sBracket(pImpl->cStartBracket);
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
                rtl::OUString sBracket(pImpl->cEndBracket);
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
}

sal_Bool SvxFontPrevWindow::IsTwoLines() const
{
    return pImpl->bTwoLines;
}

void SvxFontPrevWindow::SetTwoLines(sal_Bool bSet)
{
    pImpl->bTwoLines = bSet;
}

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

// -----------------------------------------------------------------------

static bool GetWhich (const SfxItemSet &rSet, sal_uInt16 nSlot, sal_uInt16 &rWhich)
{
    rWhich = rSet.GetPool()->GetWhich( nSlot, sal_True );
    return rSet.GetItemState(rWhich) >= SFX_ITEM_DEFAULT;
}

static void SetPrevFont( const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont )
{
    sal_uInt16 nWhich;
    if (GetWhich ( rSet, nSlot, nWhich ) )
    {
        const SvxFontItem& rFontItem = ( SvxFontItem& ) rSet.Get( nWhich );
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetName( rFontItem.GetFamilyName() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
        rFont.SetStyleName( rFontItem.GetStyleName() );
    }
}

static void SetPrevFontStyle( const SfxItemSet& rSet, sal_uInt16 nPosture, sal_uInt16 nWeight, SvxFont& rFont )
{
    sal_uInt16 nWhich;
    if( GetWhich( rSet, nPosture, nWhich ) )
    {
        const SvxPostureItem& rItem = ( const SvxPostureItem& ) rSet.Get( nWhich );
        rFont.SetItalic( ( FontItalic ) rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }

    if( GetWhich( rSet, nWeight, nWhich ) )
    {
        const SvxWeightItem& rItem = ( const SvxWeightItem& ) rSet.Get( nWhich );
        rFont.SetWeight( ( FontWeight ) rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }
}

void SvxFontPrevWindow::SetFontSize( const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont )
{
    sal_uInt16 nWhich;
    long nH;
    if( GetWhich( rSet, nSlot, nWhich ) )
    {
        nH = LogicToLogic( ( ( SvxFontHeightItem& ) rSet.Get( nWhich ) ).GetHeight(),
                            ( MapUnit ) rSet.GetPool()->GetMetric( nWhich ),
                            MAP_TWIP );
    }
    else
        nH = 240;   // as default 12pt

    rFont.SetSize( Size( 0, nH ) );
}

void SvxFontPrevWindow::SetFontLang(const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont)
{
    sal_uInt16 nWhich;
    LanguageType nLang;
    if( GetWhich( rSet, nSlot, nWhich ) )
        nLang = static_cast<const SvxLanguageItem&>(rSet.Get(nWhich)).GetLanguage();
    else
        nLang = LANGUAGE_NONE;
    rFont.SetLanguage(nLang);
}

static void SetPrevFontEscapement(SvxFont& _rFont, sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
{
    _rFont.SetPropr( nProp );
    _rFont.SetProprRel( nEscProp );
    _rFont.SetEscapement( nEsc );
}

void SvxFontPrevWindow::SetFromItemSet( const SfxItemSet &rSet,
                    bool bPreviewBackgroundToCharacter )
{
    sal_uInt16 nWhich;
    SvxFont& rFont = GetFont();
    SvxFont& rCJKFont = GetCJKFont();
    SvxFont& rCTLFont = GetCTLFont();

    // Preview string
    if( GetWhich( rSet, SID_CHAR_DLG_PREVIEW_STRING, nWhich ) )
    {
        const SfxStringItem& rItem = ( SfxStringItem& ) rSet.Get( nWhich );
        ::rtl::OUString aString = rItem.GetValue();
        if( !aString.isEmpty() )
            SetPreviewText( aString );
        else
            SetFontNameAsPreviewText();
    }

    // Underline
    FontUnderline eUnderline;
    if( GetWhich( rSet, SID_ATTR_CHAR_UNDERLINE, nWhich ) )
    {
        const SvxUnderlineItem& rItem = ( SvxUnderlineItem& ) rSet.Get( nWhich );
        eUnderline = ( FontUnderline ) rItem.GetValue();
        SetTextLineColor( rItem.GetColor() );
    }
    else
        eUnderline = UNDERLINE_NONE;

    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );

    // Overline
    FontUnderline eOverline;
    if( GetWhich( rSet, SID_ATTR_CHAR_OVERLINE, nWhich ) )
    {
        const SvxOverlineItem& rItem = ( SvxOverlineItem& ) rSet.Get( nWhich );
        eOverline = ( FontUnderline ) rItem.GetValue();
        SetOverlineColor( rItem.GetColor() );
    }
    else
        eOverline = UNDERLINE_NONE;

    rFont.SetOverline( eOverline );
    rCJKFont.SetOverline( eOverline );
    rCTLFont.SetOverline( eOverline );

    //  Strikeout
    FontStrikeout eStrikeout;
    if( GetWhich( rSet, SID_ATTR_CHAR_STRIKEOUT, nWhich ) )
    {
        const SvxCrossedOutItem& rItem = ( SvxCrossedOutItem& ) rSet.Get( nWhich );
        eStrikeout = ( FontStrikeout ) rItem.GetValue();
    }
    else
        eStrikeout = STRIKEOUT_NONE;

    rFont.SetStrikeout( eStrikeout );
    rCJKFont.SetStrikeout( eStrikeout );
    rCTLFont.SetStrikeout( eStrikeout );

    // WordLineMode
    if( GetWhich( rSet, SID_ATTR_CHAR_WORDLINEMODE, nWhich ) )
    {
        const SvxWordLineModeItem& rItem = ( SvxWordLineModeItem& ) rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
        rCJKFont.SetWordLineMode( rItem.GetValue() );
        rCTLFont.SetWordLineMode( rItem.GetValue() );
    }

    // Emphasis
    if( GetWhich( rSet, SID_ATTR_CHAR_EMPHASISMARK, nWhich ) )
    {
        const SvxEmphasisMarkItem& rItem = ( SvxEmphasisMarkItem& ) rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );
    }

    // Relief
    if( GetWhich( rSet, SID_ATTR_CHAR_RELIEF, nWhich ) )
    {
        const SvxCharReliefItem& rItem = ( SvxCharReliefItem& ) rSet.Get( nWhich );
        FontRelief eFontRelief = ( FontRelief ) rItem.GetValue();
        rFont.SetRelief( eFontRelief );
        rCJKFont.SetRelief( eFontRelief );
        rCTLFont.SetRelief( eFontRelief );
    }

    // Effects
    if( GetWhich( rSet, SID_ATTR_CHAR_CASEMAP, nWhich ) )
    {
        const SvxCaseMapItem& rItem = ( SvxCaseMapItem& ) rSet.Get( nWhich );
        SvxCaseMap eCaseMap = ( SvxCaseMap ) rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
        rCJKFont.SetCaseMap( eCaseMap );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( eCaseMap == SVX_CASEMAP_KAPITAELCHEN ? SVX_CASEMAP_NOT_MAPPED : eCaseMap );
    }

    // Outline
    if( GetWhich( rSet, SID_ATTR_CHAR_CONTOUR, nWhich ) )
    {
        const SvxContourItem& rItem = ( SvxContourItem& ) rSet.Get( nWhich );
        sal_Bool bOutline = rItem.GetValue();
        rFont.SetOutline( bOutline );
        rCJKFont.SetOutline( bOutline );
        rCTLFont.SetOutline( bOutline );
    }

    // Shadow
    if( GetWhich( rSet, SID_ATTR_CHAR_SHADOWED, nWhich ) )
    {
        const SvxShadowedItem& rItem = ( SvxShadowedItem& ) rSet.Get( nWhich );
        sal_Bool bShadow = rItem.GetValue();
        rFont.SetShadow( bShadow );
        rCJKFont.SetShadow( bShadow );
        rCTLFont.SetShadow( bShadow );
    }

    // Background
    sal_Bool bTransparent;
    if( GetWhich( rSet, bPreviewBackgroundToCharacter ? SID_ATTR_BRUSH : SID_ATTR_BRUSH_CHAR, nWhich ) )
    {
         const SvxBrushItem& rBrush = ( SvxBrushItem& ) rSet.Get( nWhich );
         const Color& rColor = rBrush.GetColor();
         bTransparent = rColor.GetTransparency() > 0;
         rFont.SetFillColor( rColor );
         rCJKFont.SetFillColor( rColor );
         rCTLFont.SetFillColor( rColor );
    }
    else
        bTransparent = TRUE;

    rFont.SetTransparent( bTransparent );
    rCJKFont.SetTransparent( bTransparent );
    rCTLFont.SetTransparent( bTransparent );

    Color aBackCol( COL_TRANSPARENT );
    if( !bPreviewBackgroundToCharacter )
    {
        if( GetWhich( rSet, SID_ATTR_BRUSH, nWhich ) )
        {
            const SvxBrushItem& rBrush = ( SvxBrushItem& ) rSet.Get( nWhich );
            if( GPOS_NONE == rBrush.GetGraphicPos() )
                aBackCol = rBrush.GetColor();
        }
    }
    SetBackColor( aBackCol );

    // Font
    SetPrevFont( rSet, SID_ATTR_CHAR_FONT, rFont );
    SetPrevFont( rSet, SID_ATTR_CHAR_CJK_FONT, rCJKFont );
    SetPrevFont( rSet, SID_ATTR_CHAR_CTL_FONT, rCTLFont );

    // Style
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_POSTURE, SID_ATTR_CHAR_WEIGHT, rFont );
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_CJK_POSTURE, SID_ATTR_CHAR_CJK_WEIGHT, rCJKFont );
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_CTL_POSTURE, SID_ATTR_CHAR_CTL_WEIGHT, rCTLFont );

    // Size
    SetFontSize( rSet, SID_ATTR_CHAR_FONTHEIGHT, rFont );
    SetFontSize( rSet, SID_ATTR_CHAR_CJK_FONTHEIGHT, rCJKFont );
    SetFontSize( rSet, SID_ATTR_CHAR_CTL_FONTHEIGHT, rCTLFont );

    // Language
    SetFontLang( rSet, SID_ATTR_CHAR_LANGUAGE, rFont );
    SetFontLang( rSet, SID_ATTR_CHAR_CJK_LANGUAGE, rCJKFont );
    SetFontLang( rSet, SID_ATTR_CHAR_CTL_LANGUAGE, rCTLFont );

    // Color
    if( GetWhich( rSet, SID_ATTR_CHAR_COLOR, nWhich ) )
    {
        const SvxColorItem& rItem = ( SvxColorItem& ) rSet.Get( nWhich );
        Color aCol( rItem.GetValue() );
        rFont.SetColor( aCol );

        rCJKFont.SetColor( aCol );
        rCTLFont.SetColor( aCol );

        AutoCorrectFontColor(); // handle color COL_AUTO
    }

    // Kerning
    if( GetWhich( rSet, SID_ATTR_CHAR_KERNING, nWhich ) )
    {
        const SvxKerningItem& rItem = ( SvxKerningItem& ) rSet.Get( nWhich );
        short nKern = ( short )
                        LogicToLogic( rItem.GetValue(), ( MapUnit ) rSet.GetPool()->GetMetric( nWhich ), MAP_TWIP );
        rFont.SetFixKerning( nKern );
        rCJKFont.SetFixKerning( nKern );
        rCTLFont.SetFixKerning( nKern );
    }

    // Escapement
    const sal_uInt8 nProp = 100;
    short nEsc;
    sal_uInt8 nEscProp;
    if( GetWhich( rSet, SID_ATTR_CHAR_ESCAPEMENT, nWhich ) )
    {
        const SvxEscapementItem& rItem = ( SvxEscapementItem& ) rSet.Get( nWhich );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();

        if( nEsc == DFLT_ESC_AUTO_SUPER )
            nEsc = DFLT_ESC_SUPER;
        else if( nEsc == DFLT_ESC_AUTO_SUB )
            nEsc = DFLT_ESC_SUB;
    }
    else
    {
        nEsc  = 0;
        nEscProp = 100;
    }
    SetPrevFontEscapement( rFont, nProp, nEscProp, nEsc );
    SetPrevFontEscapement( rCJKFont, nProp, nEscProp, nEsc );
    SetPrevFontEscapement( rCTLFont, nProp, nEscProp, nEsc );

    // Font width scale
    if( GetWhich( rSet, SID_ATTR_CHAR_SCALEWIDTH, nWhich ) )
    {
        const SvxCharScaleWidthItem&rItem = ( SvxCharScaleWidthItem& ) rSet.Get( nWhich );
        SetFontWidthScale( rItem.GetValue() );
    }

    Invalidate();
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::Init( const SfxItemSet& rSet )
{
    SvxFont&        rFont = GetFont();
    SvxFont&        rCJKFont = GetCJKFont();
    SvxFont&        rCTLFont = GetCTLFont();

    initFont(rFont);
    initFont(rCJKFont);
    initFont(rCTLFont);
    InitSettings( sal_True, sal_True );

    sal_uInt16 nWhich;
    nWhich = rSet.GetPool()->GetWhich( SID_CHAR_DLG_PREVIEW_STRING );
    if( ISITEMSET )
    {
        const SfxStringItem& rItem = ( SfxStringItem& ) rSet.Get( nWhich );
        ::rtl::OUString aString = rItem.GetValue();
        if( !aString.isEmpty() )
            SetPreviewText( aString );
        else
            SetFontNameAsPreviewText();
    }

    // Underline
    FontUnderline eUnderline;
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_UNDERLINE );
    if( ISITEMSET )
    {
        const SvxUnderlineItem& rItem = ( SvxUnderlineItem& ) rSet.Get( nWhich );
        eUnderline = ( FontUnderline ) rItem.GetValue();
        SetTextLineColor( rItem.GetColor() );
    }
    else
        eUnderline = UNDERLINE_NONE;

    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );

    // Overline
    FontUnderline eOverline;
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_OVERLINE );
    if( ISITEMSET )
    {
        const SvxOverlineItem& rItem = ( SvxOverlineItem& ) rSet.Get( nWhich );
        eOverline = ( FontUnderline ) rItem.GetValue();
        SetOverlineColor( rItem.GetColor() );
    }
    else
        eOverline = UNDERLINE_NONE;

    rFont.SetOverline( eOverline );
    rCJKFont.SetOverline( eOverline );
    rCTLFont.SetOverline( eOverline );

    //  Strikeout
    FontStrikeout eStrikeout;
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    if( ISITEMSET )
    {
        const SvxCrossedOutItem& rItem = ( SvxCrossedOutItem& ) rSet.Get( nWhich );
        eStrikeout = ( FontStrikeout ) rItem.GetValue();
    }
    else
        eStrikeout = STRIKEOUT_NONE;

    rFont.SetStrikeout( eStrikeout );
    rCJKFont.SetStrikeout( eStrikeout );
    rCTLFont.SetStrikeout( eStrikeout );

    // WordLineMode
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    if( ISITEMSET )
    {
        const SvxWordLineModeItem& rItem = ( SvxWordLineModeItem& ) rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
        rCJKFont.SetWordLineMode( rItem.GetValue() );
        rCTLFont.SetWordLineMode( rItem.GetValue() );
    }

    // Emphasis
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    if( ISITEMSET )
    {
        const SvxEmphasisMarkItem& rItem = ( SvxEmphasisMarkItem& ) rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );
    }

    // Relief
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_RELIEF );
    if( ISITEMSET )
    {
        const SvxCharReliefItem& rItem = ( SvxCharReliefItem& ) rSet.Get( nWhich );
        FontRelief eFontRelief = ( FontRelief ) rItem.GetValue();
        rFont.SetRelief( eFontRelief );
        rCJKFont.SetRelief( eFontRelief );
        rCTLFont.SetRelief( eFontRelief );
    }

    // Effects
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_CASEMAP );
    if( ISITEMSET )
    {
        const SvxCaseMapItem& rItem = ( SvxCaseMapItem& ) rSet.Get( nWhich );
        SvxCaseMap eCaseMap = ( SvxCaseMap ) rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
        rCJKFont.SetCaseMap( eCaseMap );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( eCaseMap == SVX_CASEMAP_KAPITAELCHEN ? SVX_CASEMAP_NOT_MAPPED : eCaseMap );
    }

    // Outline
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_CONTOUR );
    if( ISITEMSET )
    {
        const SvxContourItem& rItem = ( SvxContourItem& ) rSet.Get( nWhich );
        sal_Bool bOutline = rItem.GetValue();
        rFont.SetOutline( bOutline );
        rCJKFont.SetOutline( bOutline );
        rCTLFont.SetOutline( bOutline );
    }

    // Shadow
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_SHADOWED );
    if( ISITEMSET )
    {
        const SvxShadowedItem& rItem = ( SvxShadowedItem& ) rSet.Get( nWhich );
        sal_Bool bShadow = rItem.GetValue();
        rFont.SetShadow( bShadow );
        rCJKFont.SetShadow( bShadow );
        rCTLFont.SetShadow( bShadow );
    }

    // Background
    sal_Bool bTransparent;
    nWhich = rSet.GetPool()->GetWhich( pImpl->bPreviewBackgroundToCharacter ? SID_ATTR_BRUSH : SID_ATTR_BRUSH_CHAR );
    if( ISITEMSET )
    {
         const SvxBrushItem& rBrush = ( SvxBrushItem& ) rSet.Get( nWhich );
         const Color& rColor = rBrush.GetColor();
         bTransparent = rColor.GetTransparency() > 0;
         rFont.SetFillColor( rColor );
         rCJKFont.SetFillColor( rColor );
         rCTLFont.SetFillColor( rColor );
    }
    else
        bTransparent = sal_True;

    rFont.SetTransparent( bTransparent );
    rCJKFont.SetTransparent( bTransparent );
    rCTLFont.SetTransparent( bTransparent );

    Color aBackCol( COL_TRANSPARENT );
    if( !pImpl->bPreviewBackgroundToCharacter )
    {
        nWhich = rSet.GetPool()->GetWhich( SID_ATTR_BRUSH );
        if( ISITEMSET )
        {
            const SvxBrushItem& rBrush = ( SvxBrushItem& ) rSet.Get( nWhich );
            if( GPOS_NONE == rBrush.GetGraphicPos() )
                aBackCol = rBrush.GetColor();
        }
    }
    SetBackColor( aBackCol );

    // Font
    SetFont( rSet, SID_ATTR_CHAR_FONT, rFont );
    SetFont( rSet, SID_ATTR_CHAR_CJK_FONT, rCJKFont );
    SetFont( rSet, SID_ATTR_CHAR_CTL_FONT, rCTLFont );

    // Style
    SetFontStyle( rSet, SID_ATTR_CHAR_POSTURE, SID_ATTR_CHAR_WEIGHT, rFont );
    SetFontStyle( rSet, SID_ATTR_CHAR_CJK_POSTURE, SID_ATTR_CHAR_CJK_WEIGHT, rCJKFont );
    SetFontStyle( rSet, SID_ATTR_CHAR_CTL_POSTURE, SID_ATTR_CHAR_CTL_WEIGHT, rCTLFont );

    // Size
    SetFontSize( rSet, SID_ATTR_CHAR_FONTHEIGHT, rFont );
    SetFontSize( rSet, SID_ATTR_CHAR_CJK_FONTHEIGHT, rCJKFont );
    SetFontSize( rSet, SID_ATTR_CHAR_CTL_FONTHEIGHT, rCTLFont );

    // Language
    SetFontLang( rSet, SID_ATTR_CHAR_LANGUAGE, rFont );
    SetFontLang( rSet, SID_ATTR_CHAR_CJK_LANGUAGE, rCJKFont );
    SetFontLang( rSet, SID_ATTR_CHAR_CTL_LANGUAGE, rCTLFont );

    // Color
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_COLOR );
    if( ISITEMSET )
    {
        const SvxColorItem& rItem = ( SvxColorItem& ) rSet.Get( nWhich );
        Color aCol( rItem.GetValue() );
        rFont.SetColor( aCol );
        rCJKFont.SetColor( aCol );
        rCTLFont.SetColor( aCol );

        AutoCorrectFontColor(); // handle color COL_AUTO
    }

    // Kerning
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_KERNING );
    if( ISITEMSET )
    {
        const SvxKerningItem& rItem = ( SvxKerningItem& ) rSet.Get( nWhich );
        short nKern = ( short )
                        LogicToLogic( rItem.GetValue(), ( MapUnit ) rSet.GetPool()->GetMetric( nWhich ), MAP_TWIP );
        rFont.SetFixKerning( nKern );
        rCJKFont.SetFixKerning( nKern );
        rCTLFont.SetFixKerning( nKern );
    }

    // Escapement
    nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    const sal_uInt8 nProp = 100;
    short nEsc;
    sal_uInt8 nEscProp;
    if( ISITEMSET )
    {
        const SvxEscapementItem& rItem = ( SvxEscapementItem& ) rSet.Get( nWhich );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();

        if( nEsc == DFLT_ESC_AUTO_SUPER )
            nEsc = DFLT_ESC_SUPER;
        else if( nEsc == DFLT_ESC_AUTO_SUB )
            nEsc = DFLT_ESC_SUB;
    }
    else
    {
        nEsc  = 0;
        nEscProp = 100;
    }

    SetFontEscapement( nProp, nEscProp, nEsc );

    // Font width scale
    SetFontWidthScale( rSet );

    Invalidate();
}

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFont( const SfxItemSet& rSet, sal_uInt16 nSlot, SvxFont& rFont )
{
    sal_uInt16 nWhich = rSet.GetPool()->GetWhich( nSlot );
    if( ISITEMSET )
    {
        const SvxFontItem& rFontItem = (const SvxFontItem&)rSet.Get(nWhich);
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetName( rFontItem.GetFamilyName() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
        rFont.SetStyleName( rFontItem.GetStyleName() );
    }
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFontStyle( const SfxItemSet& rSet, sal_uInt16 nPosture, sal_uInt16 nWeight, SvxFont& rFont )
{
    sal_uInt16 nWhich = rSet.GetPool()->GetWhich( nPosture );
    if( ISITEMSET )
    {
        const SvxPostureItem& rItem = ( SvxPostureItem& ) rSet.Get( nWhich );
        rFont.SetItalic( ( FontItalic ) rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }

    nWhich = rSet.GetPool()->GetWhich( nWeight );
    if( ISITEMSET )
    {
        SvxWeightItem& rItem = ( SvxWeightItem& ) rSet.Get( nWhich );
        rFont.SetWeight( ( FontWeight ) rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }
}

// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFontWidthScale( const SfxItemSet& rSet )
{
    sal_uInt16  nWhich = rSet.GetPool()->GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if( ISITEMSET )
    {
        const SvxCharScaleWidthItem&    rItem = ( SvxCharScaleWidthItem& ) rSet.Get( nWhich );

        SetFontWidthScale( rItem.GetValue() );
    }
}

// -----------------------------------------------------------------------
namespace
{
    // -----------------------------------------------------------------------
    void setFontEscapement(SvxFont& _rFont,sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
    {
        _rFont.SetPropr( nProp );
        _rFont.SetProprRel( nEscProp );
        _rFont.SetEscapement( nEsc );
    }
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
}
// -----------------------------------------------------------------------

void SvxFontPrevWindow::SetFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
{
    setFontEscapement(GetFont(),nProp,nEscProp,nEsc);
    setFontEscapement(GetCJKFont(),nProp,nEscProp,nEsc);
    setFontEscapement(GetCTLFont(),nProp,nEscProp,nEsc);
    Invalidate();
}

Size SvxFontPrevWindow::GetOptimalSize(WindowSizeType eType) const
{
    if (eType == WINDOWSIZE_MAXIMUM)
        return Window::GetOptimalSize(eType);

    return m_aInitialSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
