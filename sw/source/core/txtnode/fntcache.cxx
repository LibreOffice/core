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

#include <memory>
#include <sal/config.h>

#include <cstdlib>

#include <i18nlangtag/mslangid.hxx>
#include <vcl/outdev.hxx>
#include <vcl/print.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/lazydelete.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <breakit.hxx>
#include <paintfrm.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <fntcache.hxx>
#include <IDocumentSettingAccess.hxx>
#include <swfont.hxx>
#include <wrong.hxx>
#include <dbg_lay.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <scriptinfo.hxx>
#include <editeng/brushitem.hxx>
#include <swmodule.hxx>
#include <accessibilityoptions.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <doc.hxx>
#include <editeng/fhgtitem.hxx>
#include <vcl/glyphitem.hxx>
#include <vcl/vcllayout.hxx>
#include <docsh.hxx>
#include <strings.hrc>
#include <fntcap.hxx>

using namespace ::com::sun::star;

// global variables declared in fntcache.hxx
// FontCache is created in txtinit.cxx TextInit_ and deleted in TextFinit
SwFntCache *pFntCache = nullptr;
// last Font set by ChgFntCache
SwFntObj *pLastFont = nullptr;
// "MagicNumber" used to identify Fonts
sal_uInt8* mnFontCacheIdCounter = nullptr;

static constexpr Color gWaveCol(COL_GRAY);

long SwFntObj::nPixWidth;
MapMode* SwFntObj::pPixMap = nullptr;
static vcl::DeleteOnDeinit< VclPtr<OutputDevice> > s_pFntObjPixOut( new VclPtr<OutputDevice> );

namespace
{

long EvalGridWidthAdd( const SwTextGridItem *const pGrid, const SwDrawTextInfo &rInf )
{
    SwDocShell* pDocShell = rInf.GetShell()->GetDoc()->GetDocShell();
    SfxStyleSheetBasePool* pBasePool = pDocShell->GetStyleSheetPool();

    SfxStyleSheetBase* pStyle = pBasePool->Find(SwResId(STR_POOLCOLL_STANDARD), SfxStyleFamily::Para);
    SfxItemSet& aTmpSet = pStyle->GetItemSet();
    const SvxFontHeightItem &aDefaultFontItem = aTmpSet.Get(RES_CHRATR_CJK_FONTSIZE);

    const SwDoc* pDoc = rInf.GetShell()->GetDoc();
    const sal_uInt16 nGridWidth = GetGridWidth(*pGrid, *pDoc);
    const sal_uInt32 nFontHeight = aDefaultFontItem.GetHeight();
    const long nGridWidthAdd = nGridWidth > nFontHeight ? nGridWidth - nFontHeight : 0;
    if( SwFontScript::Latin == rInf.GetFont()->GetActual() )
        return nGridWidthAdd / 2;

    return nGridWidthAdd;
}

/**
 * Pre-calculates glyph items for the rendered subset of rKey's text, assuming
 * outdev state does not change between the outdev calls.
 */
SalLayoutGlyphs* lcl_CreateLayout(const SwTextGlyphsKey& rKey, SalLayoutGlyphs& rTextGlyphs)
{
    // Use pre-calculated result.
    if (rTextGlyphs.IsValid())
        return &rTextGlyphs;

    if (rKey.m_nIndex >= rKey.m_aText.getLength())
        // Same as in OutputDevice::GetTextArray().
        return nullptr;

    // Calculate glyph items.
    std::unique_ptr<SalLayout> pLayout
        = rKey.m_pOutputDevice->ImplLayout(rKey.m_aText, rKey.m_nIndex, rKey.m_nLength, Point(0, 0), 0,
                                         nullptr, SalLayoutFlags::GlyphItemsOnly);
    if (!pLayout)
        return nullptr;

    const SalLayoutGlyphs* pGlyphs = pLayout->GetGlyphs();
    if (!pGlyphs)
        return nullptr;

    // Remember the calculation result.
    rTextGlyphs = *pGlyphs;

    return &rTextGlyphs;
}
}

bool operator<(const SwTextGlyphsKey& l, const SwTextGlyphsKey& r)
{
    if (l.m_pOutputDevice.get() < r.m_pOutputDevice.get())
        return true;
    if (l.m_pOutputDevice.get() > r.m_pOutputDevice.get())
        return false;
    if (l.m_nIndex < r.m_nIndex)
        return true;
    if (l.m_nIndex > r.m_nIndex)
        return false;
    if (l.m_nLength < r.m_nLength)
        return true;
    if (l.m_nLength > r.m_nLength)
        return false;

    // Comparing strings is expensive, so compare them:
    // - only at the end of this function
    // - only once
    // - only the relevant substring (if the index/length is not out of bounds)
    sal_Int32 nRet = 0;
    if (l.m_nLength < 0 || l.m_nIndex < 0 || l.m_nIndex + l.m_nLength > l.m_aText.getLength())
        nRet = l.m_aText.compareTo(r.m_aText);
    else
        nRet = memcmp(l.m_aText.getStr() + l.m_nIndex, r.m_aText.getStr() + r.m_nIndex,
                      l.m_nLength * sizeof(sal_Unicode));
    if (nRet < 0)
        return true;
    if (nRet > 0)
        return false;

    return false;
};

void SwFntCache::Flush( )
{
    if ( pLastFont )
    {
        pLastFont->Unlock();
        pLastFont = nullptr;
    }
    SwCache::Flush( );
}

SwFntObj::SwFntObj(const SwSubFont &rFont, const void* nFontCacheId, SwViewShell const *pSh)
    : SwCacheObj(nFontCacheId)
    , m_aFont(rFont)
    , m_pScrFont(nullptr)
    , m_pPrtFont(&m_aFont)
    , m_pPrinter(nullptr)
    , m_nGuessedLeading(USHRT_MAX)
    , m_nExtLeading(USHRT_MAX)
    , m_nScrAscent(0)
    , m_nPrtAscent(USHRT_MAX)
    , m_nScrHeight(0)
    , m_nPrtHeight(USHRT_MAX)
    , m_nPropWidth(rFont.GetPropWidth())
{
    m_nZoom = pSh ? pSh->GetViewOptions()->GetZoom() : USHRT_MAX;
    m_bSymbol = RTL_TEXTENCODING_SYMBOL == m_aFont.GetCharSet();
    m_bPaintBlank = ( LINESTYLE_NONE != m_aFont.GetUnderline()
                 || LINESTYLE_NONE != m_aFont.GetOverline()
                 || STRIKEOUT_NONE != m_aFont.GetStrikeout() )
                 && !m_aFont.IsWordLineMode();
    m_aFont.SetLanguage(rFont.GetLanguage());
}

SwFntObj::~SwFntObj()
{
    if ( m_pScrFont != m_pPrtFont )
        delete m_pScrFont;
    if ( m_pPrtFont != &m_aFont )
        delete m_pPrtFont;
}

void SwFntObj::CreatePrtFont( const OutputDevice& rPrt )
{
    if ( m_nPropWidth == 100 || m_pPrinter == &rPrt )
        return;

    if( m_pScrFont != m_pPrtFont )
        delete m_pScrFont;
    if( m_pPrtFont != &m_aFont )
        delete m_pPrtFont;

    const vcl::Font aOldFnt( rPrt.GetFont() );
    const_cast<OutputDevice&>(rPrt).SetFont( m_aFont );
    const FontMetric aWinMet( rPrt.GetFontMetric() );
    const_cast<OutputDevice&>(rPrt).SetFont( aOldFnt );
    auto nWidth = ( aWinMet.GetFontSize().Width() * m_nPropWidth ) / 100;

    if( !nWidth )
        ++nWidth;
    m_pPrtFont = new vcl::Font( m_aFont );
    m_pPrtFont->SetFontSize( Size( nWidth, m_aFont.GetFontSize().Height() ) );
    m_pScrFont = nullptr;

}

/*
 *  returns whether we have to adjust the output font to resemble
 *  the formatting font
 *
 *  _Not_ necessary if
 *
 *  1. RefDef == OutDev (text formatting, online layout...)
 *  2. PDF export from online layout
 *  3. Prospect/PagePreview printing
 */
static bool lcl_IsFontAdjustNecessary( const vcl::RenderContext& rOutDev,
                                const vcl::RenderContext& rRefDev )
{
    return &rRefDev != &rOutDev &&
           OUTDEV_WINDOW != rRefDev.GetOutDevType() &&
           ( OUTDEV_PRINTER != rRefDev.GetOutDevType() ||
             OUTDEV_PRINTER != rOutDev.GetOutDevType() );
}

struct CalcLinePosData
{
    SwDrawTextInfo& rInf;
    vcl::Font& rFont;
    TextFrameIndex const nCnt;
    const bool bSwitchH2V;
    const bool bSwitchH2VLRBT;
    const bool bSwitchL2R;
    long const nHalfSpace;
    long* const pKernArray;
    const bool bBidiPor;

    CalcLinePosData( SwDrawTextInfo& _rInf, vcl::Font& _rFont,
          TextFrameIndex const _nCnt, const bool _bSwitchH2V, const bool _bSwitchH2VLRBT, const bool _bSwitchL2R,
                      long _nHalfSpace, long* _pKernArray, const bool _bBidiPor) :
        rInf( _rInf ),
        rFont( _rFont ),
        nCnt( _nCnt ),
        bSwitchH2V( _bSwitchH2V ),
        bSwitchH2VLRBT( _bSwitchH2VLRBT ),
        bSwitchL2R( _bSwitchL2R ),
        nHalfSpace( _nHalfSpace ),
        pKernArray( _pKernArray ),
        bBidiPor( _bBidiPor )
    {
    }
};

// Computes the start and end position of an underline. This function is called
// from the DrawText-method (for underlining misspelled words or smarttag terms).
static void lcl_calcLinePos( const CalcLinePosData &rData,
    Point &rStart, Point &rEnd, TextFrameIndex const nStart, TextFrameIndex const nWrLen)
{
    long nBlank = 0;
    const TextFrameIndex nEnd = nStart + nWrLen;
    const long nTmpSpaceAdd = rData.rInf.GetSpace() / SPACING_PRECISION_FACTOR;

    if ( nEnd < rData.nCnt
       && CH_BLANK == rData.rInf.GetText()[sal_Int32(rData.rInf.GetIdx() + nEnd)] )
    {
        if (nEnd + TextFrameIndex(1) == rData.nCnt)
            nBlank -= nTmpSpaceAdd;
        else
            nBlank -= rData.nHalfSpace;
    }

    // determine start, end and length of wave line
    sal_Int32 nKernStart = nStart ? rData.pKernArray[sal_Int32(nStart) - 1] : 0;
    sal_Int32 nKernEnd = rData.pKernArray[sal_Int32(nEnd) - 1];

    const sal_uInt16 nDir = rData.bBidiPor ? 1800
                                           : UnMapDirection(rData.rFont.GetOrientation(),
                                                            rData.bSwitchH2V, rData.bSwitchH2VLRBT);

    switch ( nDir )
    {
    case 0 :
        rStart.AdjustX(nKernStart );
        rEnd.setX( nBlank + rData.rInf.GetPos().X() + nKernEnd );
        rEnd.setY( rData.rInf.GetPos().Y() );
        break;
    case 900 :
        rStart.AdjustY( -nKernStart );
        rEnd.setX( rData.rInf.GetPos().X() );
        rEnd.setY( nBlank + rData.rInf.GetPos().Y() - nKernEnd );
        break;
    case 1800 :
        rStart.AdjustX( -nKernStart );
        rEnd.setX( rData.rInf.GetPos().X() - nKernEnd - nBlank );
        rEnd.setY( rData.rInf.GetPos().Y() );
        break;
    case 2700 :
        rStart.AdjustY(nKernStart );
        rEnd.setX( rData.rInf.GetPos().X() );
        rEnd.setY( nBlank + rData.rInf.GetPos().Y() + nKernEnd );
        break;
    }

    if ( rData.bSwitchL2R )
    {
        rData.rInf.GetFrame()->SwitchLTRtoRTL( rStart );
        rData.rInf.GetFrame()->SwitchLTRtoRTL( rEnd );
    }

    if ( rData.bSwitchH2V )
    {
        rData.rInf.GetFrame()->SwitchHorizontalToVertical( rStart );
        rData.rInf.GetFrame()->SwitchHorizontalToVertical( rEnd );
    }
}

// Returns the Ascent of the Font on the given output device;
// it may be necessary to create the screen font first.
sal_uInt16 SwFntObj::GetFontAscent( const SwViewShell *pSh, const OutputDevice& rOut )
{
    sal_uInt16 nRet = 0;
    const OutputDevice& rRefDev = pSh ? pSh->GetRefDev() : rOut;

    if ( pSh && lcl_IsFontAdjustNecessary( rOut, rRefDev ) )
    {
        CreateScrFont( *pSh, rOut );
        OSL_ENSURE( USHRT_MAX != m_nScrAscent, "nScrAscent is going berzerk" );
        nRet = m_nScrAscent;
    }
    else
    {
        if (m_nPrtAscent == USHRT_MAX) // printer ascent unknown?
        {
            CreatePrtFont( rOut );
            const vcl::Font aOldFnt( rRefDev.GetFont() );
            const_cast<OutputDevice&>(rRefDev).SetFont( *m_pPrtFont );
            const FontMetric aOutMet( rRefDev.GetFontMetric() );
            m_nPrtAscent = static_cast<sal_uInt16>(aOutMet.GetAscent());
            const_cast<OutputDevice&>(rRefDev).SetFont( aOldFnt );
        }

        nRet = m_nPrtAscent;
    }

#if !defined(MACOSX) // #i89844# extleading is below the line for Mac
    // TODO: move extleading below the line for all platforms too
    nRet += GetFontLeading( pSh, rRefDev );
#endif

    OSL_ENSURE( USHRT_MAX != nRet, "GetFontAscent returned USHRT_MAX" );
    return nRet;
}

// Returns the height of the Font on the given output device;
// it may be necessary to create the screen font first.
sal_uInt16 SwFntObj::GetFontHeight( const SwViewShell* pSh, const OutputDevice& rOut )
{
    sal_uInt16 nRet = 0;
    const OutputDevice& rRefDev = pSh ? pSh->GetRefDev() : rOut;

    if ( pSh && lcl_IsFontAdjustNecessary( rOut, rRefDev ) )
    {
        CreateScrFont( *pSh, rOut );
        OSL_ENSURE( USHRT_MAX != m_nScrHeight, "nScrHeight is going berzerk" );
        nRet = m_nScrHeight + GetFontLeading( pSh, rRefDev );
    }
    else
    {
        if (m_nPrtHeight == USHRT_MAX) // printer height unknown?
        {
            CreatePrtFont( rOut );
            const vcl::Font aOldFnt( rRefDev.GetFont() );
            const_cast<OutputDevice&>(rRefDev).SetFont( *m_pPrtFont );
            m_nPrtHeight = static_cast<sal_uInt16>(rRefDev.GetTextHeight());

#if OSL_DEBUG_LEVEL > 0
            // Check if vcl did not change the meaning of GetTextHeight
            const FontMetric aOutMet( rRefDev.GetFontMetric() );
            long nTmpPrtHeight = static_cast<sal_uInt16>(aOutMet.GetAscent()) + aOutMet.GetDescent();
            // #i106098#: do not compare with == here due to rounding error
            OSL_ENSURE( std::abs(nTmpPrtHeight - m_nPrtHeight) < 3,
                    "GetTextHeight != Ascent + Descent" );
#endif

            const_cast<OutputDevice&>(rRefDev).SetFont( aOldFnt );
        }

        nRet = m_nPrtHeight + GetFontLeading( pSh, rRefDev );
    }

    OSL_ENSURE( USHRT_MAX != nRet, "GetFontHeight returned USHRT_MAX" );
    return nRet;
}

sal_uInt16 SwFntObj::GetFontLeading( const SwViewShell *pSh, const OutputDevice& rOut )
{
    sal_uInt16 nRet = 0;

    if ( pSh )
    {
        if ( USHRT_MAX == m_nGuessedLeading || USHRT_MAX == m_nExtLeading )
        {
            SolarMutexGuard aGuard;

            const vcl::Font aOldFnt( rOut.GetFont() );
            const_cast<OutputDevice&>(rOut).SetFont( *m_pPrtFont );
            const FontMetric aMet( rOut.GetFontMetric() );
            const_cast<OutputDevice&>(rOut).SetFont( aOldFnt );
            m_bSymbol = RTL_TEXTENCODING_SYMBOL == aMet.GetCharSet();
            GuessLeading( *pSh, aMet );
            m_nExtLeading = static_cast<sal_uInt16>(aMet.GetExternalLeading());
            /* HACK: FIXME There is something wrong with Writer's bullet rendering, causing lines
               with bullets to be higher than they should be. I think this is because
               Writer uses font's external leading incorrect, as the vertical distance
               added to every line instead of only a distance between multiple lines,
               which means a single bullet has external leading added even though it
               shouldn't, but frankly this is just an educated guess rather than understanding
               Writer's layout (heh).
               Symbol font in some documents is 'StarSymbol; Arial Unicode MS', and Windows
               machines often do not have StarSymbol, falling back to Arial Unicode MS, which
               has unusually high external leading. So just reset external leading for fonts
               which are used to bullets, as those should not be used on multiple lines anyway,
               so in correct rendering external leading should be irrelevant anyway.
               Interestingly enough, bSymbol is false for 'StarSymbol; Arial Unicode MS', so
               also check explicitly.
            */
            if( m_bSymbol || IsStarSymbol( m_pPrtFont->GetFamilyName()))
                m_nExtLeading = 0;
        }

        const IDocumentSettingAccess& rIDSA = pSh->getIDocumentSettingAccess();
        const bool bBrowse = ( pSh->GetWin() &&
                               pSh->GetViewOptions()->getBrowseMode() &&
                              !pSh->GetViewOptions()->IsPrtFormat() );

        if ( !bBrowse && rIDSA.get(DocumentSettingId::ADD_EXT_LEADING) )
            nRet = m_nExtLeading;
        else
            nRet = m_nGuessedLeading;
    }

    OSL_ENSURE( USHRT_MAX != nRet, "GetFontLeading returned USHRT_MAX" );
    return nRet;
}

//  pOut is the output device, not the reference device
void SwFntObj::CreateScrFont( const SwViewShell& rSh, const OutputDevice& rOut )
{
    if ( m_pScrFont )
        return;

    // any changes to the output device are reset at the end of the function
    OutputDevice* pOut = const_cast<OutputDevice*>(&rOut);

    // Save old font
    vcl::Font aOldOutFont( pOut->GetFont() );

    m_nScrHeight = USHRT_MAX;

    // Condition for output font / refdev font adjustment
    OutputDevice* pPrt = &rSh.GetRefDev();

    if( !rSh.GetWin() ||
        !rSh.GetViewOptions()->getBrowseMode() ||
         rSh.GetViewOptions()->IsPrtFormat() )
    {
        // After CreatePrtFont m_pPrtFont is the font which is actually used
        // by the reference device
        CreatePrtFont( *pPrt );
        m_pPrinter = pPrt;

        // save old reference device font
        vcl::Font aOldPrtFnt( pPrt->GetFont() );

        // set the font used at the reference device at the reference device
        // and the output device
        pPrt->SetFont( *m_pPrtFont );
        pOut->SetFont( *m_pPrtFont );

        // This should be the default for pScrFont.
        m_pScrFont = m_pPrtFont;

        FontMetric aMet = pPrt->GetFontMetric( );
        // Don't lose "faked" properties of the logical font that don't truly
        // exist in the physical font metrics which vcl which fake up for us
        aMet.SetWeight(m_pScrFont->GetWeight());
        aMet.SetItalic(m_pScrFont->GetItalic());

        m_bSymbol = RTL_TEXTENCODING_SYMBOL == aMet.GetCharSet();

        if ( USHRT_MAX == m_nGuessedLeading )
            GuessLeading( rSh, aMet );

        if ( USHRT_MAX == m_nExtLeading )
            m_nExtLeading = static_cast<sal_uInt16>(aMet.GetExternalLeading());

        // reset the original reference device font
        pPrt->SetFont( aOldPrtFnt );
    }
    else
    {
        m_bSymbol = RTL_TEXTENCODING_SYMBOL == m_aFont.GetCharSet();
        if ( m_nGuessedLeading == USHRT_MAX )
            m_nGuessedLeading = 0;

        // no external leading in browse mode
        if ( m_nExtLeading == USHRT_MAX )
            m_nExtLeading = 0;

        m_pScrFont = m_pPrtFont;
    }

    // check zoom factor, e.g. because of PrtOle2 during export
    {
        // In case the zoom factor of the output device differs from the
        // one in the ViewOptions, this Font must not be cached,
        // hence set zoom factor to an invalid value
        long nTmp;
        if( pOut->GetMapMode().GetScaleX().IsValid() &&
            pOut->GetMapMode().GetScaleY().IsValid() &&
            pOut->GetMapMode().GetScaleX() == pOut->GetMapMode().GetScaleY() )
        {
            nTmp = long(100 * pOut->GetMapMode().GetScaleX());
        }
        else
            nTmp = 0;
        if( nTmp != m_nZoom )
            m_nZoom = USHRT_MAX - 1;
    }

    m_nScrAscent = static_cast<sal_uInt16>(pOut->GetFontMetric().GetAscent());
    if ( USHRT_MAX == m_nScrHeight )
        m_nScrHeight = static_cast<sal_uInt16>(pOut->GetTextHeight());

    // reset original output device font
    pOut->SetFont( aOldOutFont );
}

void SwFntObj::GuessLeading( const SwViewShell&
#if defined(_WIN32)
                             rSh
#endif
                             , const FontMetric& rMet )
{
    // If leading >= 5, this seems to be enough leading.
    // Nothing has to be done.
    if ( rMet.GetInternalLeading() >= 5 )
    {
        m_nGuessedLeading = 0;
        return;
    }

#if defined(_WIN32)
    OutputDevice *pWin = rSh.GetWin() ?
                         rSh.GetWin() :
                         Application::GetDefaultDevice();
    if ( pWin )
    {
        MapMode aTmpMap( MapUnit::MapTwip );
        MapMode aOldMap = pWin->GetMapMode( );
        pWin->SetMapMode( aTmpMap );
        const vcl::Font aOldFnt( pWin->GetFont() );
        pWin->SetFont( *m_pPrtFont );
        const FontMetric aWinMet( pWin->GetFontMetric() );
        const sal_uInt16 nWinHeight = sal_uInt16( aWinMet.GetFontSize().Height() );
        if( m_pPrtFont->GetFamilyName().indexOf( aWinMet.GetFamilyName() ) != -1 )
        {
            // If the Leading on the Window is also 0, then it has to stay
            // that way (see also StarMath).
            long nTmpLeading = aWinMet.GetInternalLeading();
            if( nTmpLeading <= 0 )
            {
                pWin->SetFont( rMet );
                nTmpLeading = pWin->GetFontMetric().GetInternalLeading();
                if( nTmpLeading < 0 )
                    m_nGuessedLeading = 0;
                else
                    m_nGuessedLeading = sal_uInt16(nTmpLeading);
            }
            else
            {
                m_nGuessedLeading = sal_uInt16(nTmpLeading);
                // Manta-Hack #50153#:
                // Wer beim Leading luegt, luegt moeglicherweise auch beim
                // Ascent/Descent, deshalb wird hier ggf. der Font ein wenig
                // tiefergelegt, ohne dabei seine Hoehe zu aendern.
                // (above original comment preserved for cultural reasons)
                // Those who lie about their Leading, may lie about their
                // Ascent/Descent as well, hence the Font will be lowered a
                // little without changing its height.
                long nDiff = std::min( rMet.GetDescent() - aWinMet.GetDescent(),
                    aWinMet.GetAscent() - rMet.GetAscent() - nTmpLeading );
                if( nDiff > 0 )
                {
                    OSL_ENSURE( m_nPrtAscent < USHRT_MAX, "GuessLeading: PrtAscent-Fault" );
                    if ( m_nPrtAscent < USHRT_MAX )
                        m_nPrtAscent = m_nPrtAscent + static_cast<sal_uInt16>(( 2 * nDiff ) / 5);
                }
            }
        }
        else
        {
            // If all else fails, take 15% of the height, as empirically
            // determined by CL
            m_nGuessedLeading = (nWinHeight * 15) / 100;
        }
        pWin->SetFont( aOldFnt );
        pWin->SetMapMode( aOldMap );
    }
    else
        m_nGuessedLeading = 0;
#else
    m_nGuessedLeading = 0;
#endif
}

// Set the font at the given output device; for screens it may be
// necessary to do some adjustment first.
void SwFntObj::SetDevFont( const SwViewShell *pSh, OutputDevice& rOut )
{
    const OutputDevice& rRefDev = pSh ? pSh->GetRefDev() : rOut;

    if ( pSh && lcl_IsFontAdjustNecessary( rOut, rRefDev ) )
    {
        CreateScrFont( *pSh, rOut );
        if( !GetScrFont()->IsSameInstance( rOut.GetFont() ) )
            rOut.SetFont( *m_pScrFont );
        if( m_pPrinter && ( !m_pPrtFont->IsSameInstance( m_pPrinter->GetFont() ) ) )
            m_pPrinter->SetFont( *m_pPrtFont );
    }
    else
    {
        CreatePrtFont( rOut );
        if( !m_pPrtFont->IsSameInstance( rOut.GetFont() ) )
            rOut.SetFont( *m_pPrtFont );
    }

    // Here, we actually do not need the leading values, but by calling
    // GetFontLeading() we assure that the values are calculated for later use.
    GetFontLeading( pSh, rRefDev );
}

#define WRONG_SHOW_MIN 5

/*
 * Output text:
 *      on screen              => DrawTextArray
 *      on printer, !Kerning   => DrawText
 *      on printer + Kerning   => DrawStretchText
 */
static sal_uInt8 lcl_WhichPunctuation( sal_Unicode cChar )
{
    if ( ( cChar < 0x3001 || cChar > 0x3002 ) &&
            ( cChar < 0x3008 || cChar > 0x3011 ) &&
            ( cChar < 0x3014 || cChar > 0x301F ) &&
              0xFF62 != cChar && 0xFF63 != cChar )
        // no punctuation
        return SwScriptInfo::NONE;
    else if ( 0x3001 == cChar || 0x3002 == cChar ||
              0x3009 == cChar || 0x300B == cChar ||
              0x300D == cChar || 0x300F == cChar ||
              0x3011 == cChar || 0x3015 == cChar ||
              0x3017 == cChar || 0x3019 == cChar ||
              0x301B == cChar || 0x301E == cChar ||
              0x301F == cChar || 0xFF63 == cChar )
        // right punctuation
        return SwScriptInfo::SPECIAL_RIGHT;

    return SwScriptInfo::SPECIAL_LEFT;
}

static bool lcl_IsMonoSpaceFont( const vcl::RenderContext& rOut )
{
    const long nWidth1 = rOut.GetTextWidth( OUString( u'\x3008' ) );
    const long nWidth2 = rOut.GetTextWidth( OUString( u'\x307C' ) );
    return nWidth1 == nWidth2;
}

static bool lcl_IsFullstopCentered( const vcl::RenderContext& rOut )
{
    const FontMetric aMetric( rOut.GetFontMetric() );
    return aMetric.IsFullstopCentered() ;
}

/* This helper structure (SwForbidden) contains the already marked parts of the string
    to avoid double lines (e.g grammar + spell check error) */

typedef std::vector<std::pair<TextFrameIndex, TextFrameIndex>> SwForbidden;

static void lcl_DrawLineForWrongListData(
    SwForbidden &rForbidden,
    const SwDrawTextInfo    &rInf,
    sw::WrongListIterator *pWList,
    const CalcLinePosData   &rCalcLinePosData,
    const Size              &rPrtFontSize )
{
    if (!pWList) return;

    TextFrameIndex nStart = rInf.GetIdx();
    TextFrameIndex nWrLen = rInf.GetLen();

    // check if respective data is available in the current text range
    if (!pWList->Check( nStart, nWrLen ))
    {
        return;
    }

    long nHght = rInf.GetOut().LogicToPixel( rPrtFontSize ).Height();

    // Draw wavy lines for spell and grammar errors only if font is large enough.
    // Lines for smart tags will always be drawn.
    if (pWList != rInf.GetSmartTags() && WRONG_SHOW_MIN >= nHght)
    {
        return;
    }

    SwForbidden::iterator pIter = rForbidden.begin();
    if (rInf.GetOut().GetConnectMetaFile())
        rInf.GetOut().Push();

    const Color aCol( rInf.GetOut().GetLineColor() );

    // iterate over all ranges stored in the respective SwWrongList
    do
    {
        nStart -= rInf.GetIdx();

        const TextFrameIndex nEnd = nStart + nWrLen;
        TextFrameIndex nNext = nStart;
        while( nNext < nEnd )
        {
            while( pIter != rForbidden.end() && pIter->second <= nNext )
                ++pIter;

            const TextFrameIndex nNextStart = nNext;
            TextFrameIndex nNextEnd = nEnd;

            if( pIter == rForbidden.end() || nNextEnd <= pIter->first )
            {
                // No overlapping mark up found
                rForbidden.insert(pIter, std::make_pair(nNextStart, nNextEnd));
                pIter = rForbidden.begin();
                nNext = nEnd;
            }
            else
            {
                nNext = pIter->second;
                if( nNextStart < pIter->first )
                {
                    nNextEnd = pIter->first;
                    pIter->first = nNextStart;
                }
                else
                    continue;
            }
            // determine line pos
            Point aStart( rInf.GetPos() );
            Point aEnd;
            lcl_calcLinePos( rCalcLinePosData, aStart, aEnd, nNextStart, nNextEnd - nNextStart );

            SwWrongArea const*const wrongArea = pWList->GetWrongElement(nNextStart + rInf.GetIdx());
            if (wrongArea != nullptr)
            {
                if (WRONGAREA_DASHED == wrongArea->mLineType)
                {
                    rInf.GetOut().SetLineColor( wrongArea->mColor );

                    aStart.AdjustY(30 );
                    aEnd.AdjustY(30 );

                    LineInfo aLineInfo( LineStyle::Dash );
                    aLineInfo.SetDistance( 40 );
                    aLineInfo.SetDashLen( 1 );
                    aLineInfo.SetDashCount(1);

                    rInf.GetOut().DrawLine( aStart, aEnd, aLineInfo );
                }
                else if (WRONGAREA_WAVE == wrongArea->mLineType)
                {
                    rInf.GetOut().SetLineColor( wrongArea->mColor );

                    rInf.GetOut().DrawWaveLine( aStart, aEnd );
                }
            }
        }

        nStart = nEnd + rInf.GetIdx();
        nWrLen = rInf.GetIdx() + rInf.GetLen() - nStart;
    }
    while (nWrLen && pWList->Check( nStart, nWrLen ));

    rInf.GetOut().SetLineColor( aCol );

    if (rInf.GetOut().GetConnectMetaFile())
        rInf.GetOut().Pop();
}

void SwFntObj::DrawText( SwDrawTextInfo &rInf )
{
    OSL_ENSURE( rInf.GetShell(), "SwFntObj::DrawText without shell" );

    OutputDevice& rRefDev = rInf.GetShell()->GetRefDev();
    OutputDevice* pWin = rInf.GetShell()->GetWin();

    // true if pOut is the printer and the printer has been used for formatting
    const bool bPrt = OUTDEV_PRINTER == rInf.GetOut().GetOutDevType() &&
                      OUTDEV_PRINTER == rRefDev.GetOutDevType();
    const bool bBrowse = ( pWin &&
                           rInf.GetShell()->GetViewOptions()->getBrowseMode() &&
                          !rInf.GetShell()->GetViewOptions()->IsPrtFormat() &&
                          !rInf.GetBullet() &&
                           ( rInf.GetSpace() || !rInf.GetKern() ) &&
                          !rInf.GetWrong() &&
                          !rInf.GetGrammarCheck() &&
                          !rInf.GetSmartTags() &&
                          !rInf.GetGreyWave() );

    // bDirectPrint indicates that we can enter the branch which calls
    // the DrawText functions instead of calling the DrawTextArray functions
    const bool bDirectPrint = bPrt || bBrowse;

    // Condition for output font / refdev font adjustment
    const bool bUseScrFont =
        lcl_IsFontAdjustNecessary( rInf.GetOut(), rRefDev );

    vcl::Font* pTmpFont = bUseScrFont ? m_pScrFont : m_pPrtFont;

    //  bDirectPrint and bUseScrFont should have these values:

    //  Outdev / RefDef  | Printer | VirtPrinter | Window

    //  Printer          | 1 - 0   | 0 - 1       | -

    //  VirtPrinter/PDF  | 0 - 1   | 0 - 1       | -

    //  Window/VirtWindow| 0 - 1   | 0 - 1       | 1 - 0

    // Exception: During painting of a Writer OLE object, we do not have
    // a window. Therefore bUseSrcFont is always 0 in this case.

#if OSL_DEBUG_LEVEL > 0

    const bool bNoAdjust = bPrt ||
            (  pWin &&
               rInf.GetShell()->GetViewOptions()->getBrowseMode() &&
              !rInf.GetShell()->GetViewOptions()->IsPrtFormat() );

    if ( OUTDEV_PRINTER == rInf.GetOut().GetOutDevType() )
    {
        // Printer output
        if ( OUTDEV_PRINTER == rRefDev.GetOutDevType() )
        {
            OSL_ENSURE( bNoAdjust && !bUseScrFont, "Outdev Check failed" );
        }
        else if ( rRefDev.IsVirtual() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else
        {
            OSL_FAIL( "Outdev Check failed" );
        }
    }
    else if ( rInf.GetOut().IsVirtual() && ! pWin )
    {
        // PDF export
        if ( OUTDEV_PRINTER == rRefDev.GetOutDevType() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else if ( rRefDev.IsVirtual() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else
        {
            OSL_FAIL( "Outdev Check failed" );
        }
    }
    else if ( OUTDEV_WINDOW == rInf.GetOut().GetOutDevType() ||
               ( rInf.GetOut().IsVirtual() && pWin ) )
    {
        // Window or virtual window
        if ( OUTDEV_PRINTER == rRefDev.GetOutDevType() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else if ( rRefDev.IsVirtual() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else if ( OUTDEV_WINDOW == rRefDev.GetOutDevType() )
        {
            OSL_ENSURE( bNoAdjust && !bUseScrFont, "Outdev Check failed" );
        }
        else
        {
            OSL_FAIL( "Outdev Check failed" );
        }
    }
    else
    {
            OSL_FAIL( "Outdev Check failed" );
    }

#endif

    // robust: better use the printer font instead of using no font at all
    OSL_ENSURE( pTmpFont, "No screen or printer font?" );
    if ( ! pTmpFont )
        pTmpFont = m_pPrtFont;

    // HACK: LINESTYLE_WAVE must not be abused any more, hence the grey wave
    // line of the ExtendedAttributeSets will appear in the font color first

    const bool bSwitchH2V = rInf.GetFrame() && rInf.GetFrame()->IsVertical();
    const bool bSwitchH2VLRBT = rInf.GetFrame() && rInf.GetFrame()->IsVertLRBT();
    const bool bSwitchL2R = rInf.GetFrame() && rInf.GetFrame()->IsRightToLeft() &&
                            ! rInf.IsIgnoreFrameRTL();
    const ComplexTextLayoutFlags nMode = rInf.GetOut().GetLayoutMode();
    const bool bBidiPor = ( bSwitchL2R !=
                            ( ComplexTextLayoutFlags::Default != ( ComplexTextLayoutFlags::BiDiRtl & nMode ) ) );

    // be sure to have the correct layout mode at the printer
    if ( m_pPrinter )
    {
        m_pPrinter->SetLayoutMode( rInf.GetOut().GetLayoutMode() );
        m_pPrinter->SetDigitLanguage( rInf.GetOut().GetDigitLanguage() );
    }

    Point aTextOriginPos( rInf.GetPos() );
    if( !bPrt )
    {
        if( rInf.GetpOut() != *s_pFntObjPixOut.get() || rInf.GetOut().GetMapMode() != *pPixMap )
        {
            *pPixMap = rInf.GetOut().GetMapMode();
            (*s_pFntObjPixOut.get()) = rInf.GetpOut();
            Size aTmp( 1, 1 );
            nPixWidth = rInf.GetOut().PixelToLogic( aTmp ).Width();
        }

        aTextOriginPos.AdjustX(rInf.GetFrame()->IsRightToLeft() ? 0 : nPixWidth );
    }

    Color aOldColor( pTmpFont->GetColor() );
    bool bChgColor = rInf.ApplyAutoColor( pTmpFont );
    if( !pTmpFont->IsSameInstance( rInf.GetOut().GetFont() ) )
        rInf.GetOut().SetFont( *pTmpFont );
    if ( bChgColor )
        pTmpFont->SetColor( aOldColor );

    if (TextFrameIndex(COMPLETE_STRING) == rInf.GetLen())
        rInf.SetLen( TextFrameIndex(rInf.GetText().getLength()) );

    // ASIAN LINE AND CHARACTER GRID MODE START

    if ( rInf.GetFrame() && rInf.SnapToGrid() && rInf.GetFont() &&
         SwFontScript::CJK == rInf.GetFont()->GetActual() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));

        // ASIAN LINE AND CHARACTER GRID MODE: Do we want to snap asian characters to the grid?
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars())
        {
            //for textgrid refactor
            //const sal_uInt16 nGridWidth = pGrid->GetBaseHeight();
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GetGridWidth(*pGrid, *pDoc);

            // kerning array - gives the absolute position of end of each character
            std::unique_ptr<long[]> pKernArray(new long[sal_Int32(rInf.GetLen())]);

            if ( m_pPrinter )
                m_pPrinter->GetTextArray( rInf.GetText(), pKernArray.get(),
                            sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
            else
                rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                            sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));

            // Change the average width per character to an appropriate grid width
            // basically get the ratio of the avg width to the grid unit width, then
            // multiple this ratio to give the new avg width - which in this case
            // gives a new grid width unit size

            long nAvgWidthPerChar = pKernArray[sal_Int32(rInf.GetLen()) - 1] / sal_Int32(rInf.GetLen());

            const sal_uLong nRatioAvgWidthCharToGridWidth = nAvgWidthPerChar ?
                                ( nAvgWidthPerChar - 1 ) / nGridWidth + 1:
                                1;

            nAvgWidthPerChar = nRatioAvgWidthCharToGridWidth * nGridWidth;

            // the absolute end position of the first character is also its width
            long nCharWidth = pKernArray[ 0 ];
            sal_uLong nHalfWidth = nAvgWidthPerChar / 2;

            long nNextFix=0;

            // we work out the start position (origin) of the first character,
            // and we set the next "fix" offset to half the width of the char.
            // The exceptions are for punctuation characters that are not centered
            // so in these cases we just add half a regular "average" character width
            // to the first characters actual width to allow the next character to
            // be centred automatically
            // If the character is "special right", then the offset is correct already
            // so the fix offset is as normal - half the average character width

            sal_Unicode cChar = rInf.GetText()[ sal_Int32(rInf.GetIdx()) ];
            sal_uInt8 nType = lcl_WhichPunctuation( cChar );
            switch ( nType )
            {
            // centre character
            case SwScriptInfo::NONE :
                aTextOriginPos.AdjustX(( nAvgWidthPerChar - nCharWidth ) / 2 );
                nNextFix = nCharWidth / 2;
                break;
            case SwScriptInfo::SPECIAL_RIGHT :
                nNextFix = nHalfWidth;
                break;
            // punctuation
            default:
                aTextOriginPos.AdjustX(nAvgWidthPerChar - nCharWidth );
                nNextFix = nCharWidth - nHalfWidth;
            }

            // calculate offsets
            for (sal_Int32 j = 1; j < sal_Int32(rInf.GetLen()); ++j)
            {
                long nCurrentCharWidth = pKernArray[ j ] - pKernArray[ j - 1 ];
                nNextFix += nAvgWidthPerChar;

                // almost the same as getting the offset for the first character:
                // punctuation characters are not centered, so just add half an
                // average character width minus the characters actual char width
                // to get the offset into the centre of the next character

                cChar = rInf.GetText()[ sal_Int32(rInf.GetIdx()) + j ];
                nType = lcl_WhichPunctuation( cChar );
                switch ( nType )
                {
                case SwScriptInfo::NONE :
                    pKernArray[ j - 1 ] = nNextFix - ( nCurrentCharWidth / 2 );
                    break;
                case SwScriptInfo::SPECIAL_RIGHT :
                    pKernArray[ j - 1 ] = nNextFix - nHalfWidth;
                    break;
                default:
                    pKernArray[ j - 1 ] = nNextFix + nHalfWidth - nCurrentCharWidth;
                }
            }

            // the layout engine requires the total width of the output
            pKernArray[sal_Int32(rInf.GetLen()) - 1] = rInf.GetWidth() -
                                              aTextOriginPos.X() + rInf.GetPos().X() ;

            if ( bSwitchH2V )
                rInf.GetFrame()->SwitchHorizontalToVertical( aTextOriginPos );

            rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                pKernArray.get(), sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));

            return;
        }
    }

    // For text grid refactor
    // ASIAN LINE AND CHARACTER GRID MODE START: not snap to characters

    if ( rInf.GetFrame() && rInf.SnapToGrid() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));

        // ASIAN LINE AND CHARACTER GRID MODE - do not snap to characters
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && !pGrid->IsSnapToChars() )
        {
            const long nGridWidthAdd = EvalGridWidthAdd( pGrid, rInf );

            std::unique_ptr<long[]> pKernArray(new long[sal_Int32(rInf.GetLen())]);

            if ( m_pPrinter )
                m_pPrinter->GetTextArray( rInf.GetText(), pKernArray.get(),
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
            else
                rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
            if ( bSwitchH2V )
                rInf.GetFrame()->SwitchHorizontalToVertical( aTextOriginPos );
            if ( rInf.GetSpace() || rInf.GetKanaComp())
            {
                long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
                if ( rInf.GetFont() && rInf.GetLen() )
                {
                    bool bSpecialJust = false;
                    const SwScriptInfo* pSI = rInf.GetScriptInfo();
                    const SwFontScript nActual = rInf.GetFont()->GetActual();
                    ///Kana Compression
                    if( SwFontScript::CJK == nActual && rInf.GetKanaComp() &&
                        pSI && pSI->CountCompChg() &&
                        lcl_IsMonoSpaceFont( *(rInf.GetpOut()) ) )
                    {
                        pSI->Compress( pKernArray.get(), rInf.GetIdx(), rInf.GetLen(),
                            rInf.GetKanaComp(), static_cast<sal_uInt16>(m_aFont.GetFontSize().Height()), lcl_IsFullstopCentered( rInf.GetOut() ) , &aTextOriginPos );
                        bSpecialJust = true;
                    }
                    ///Asian Justification
                    if ( ( SwFontScript::CJK == nActual || SwFontScript::Latin == nActual ) && nSpaceAdd )
                    {
                        LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CJK );
                        if (!MsLangId::isKorean(aLang))
                        {
                            long nSpaceSum = nSpaceAdd;
                            for (sal_Int32 nI = 0; nI < sal_Int32(rInf.GetLen()); ++nI)
                            {
                                pKernArray[ nI ] += nSpaceSum;
                                nSpaceSum += nSpaceAdd;
                            }
                            bSpecialJust = true;
                            nSpaceAdd = 0;
                        }
                    }
                    long nGridAddSum = nGridWidthAdd;
                    for (sal_Int32 i = 0; i < sal_Int32(rInf.GetLen()); i++, nGridAddSum += nGridWidthAdd )
                    {
                        pKernArray[i] += nGridAddSum;
                    }
                    long nKernSum = rInf.GetKern();
                    if ( bSpecialJust || rInf.GetKern() )
                    {
                        for (sal_Int32 i = 0; i < sal_Int32(rInf.GetLen()); i++, nKernSum += rInf.GetKern())
                        {
                            if (CH_BLANK == rInf.GetText()[sal_Int32(rInf.GetIdx())+i])
                                nKernSum += nSpaceAdd;
                            pKernArray[i] += nKernSum;
                        }
                        ///With through/uderstr. Grouped style requires a blank at the end
                        ///of a text edition special measures:
                        if( m_bPaintBlank && rInf.GetLen() && (CH_BLANK ==
                            rInf.GetText()[sal_Int32(rInf.GetIdx() + rInf.GetLen()) - 1]))
                        {
                            ///If it concerns a singular, underlined space acts,
                            ///we must spend two:
                            if (TextFrameIndex(1) == rInf.GetLen())
                            {
                                pKernArray[0] = rInf.GetWidth() + nSpaceAdd;
                                rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                                    pKernArray.get(), sal_Int32(rInf.GetIdx()), 1);
                            }
                            else
                            {
                                pKernArray[sal_Int32(rInf.GetLen()) - 2] += nSpaceAdd;
                                rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                                    pKernArray.get(), sal_Int32(rInf.GetIdx()),
                                    sal_Int32(rInf.GetLen()));
                            }
                        }
                        else
                        {
                            rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                                pKernArray.get(), sal_Int32(rInf.GetIdx()),
                                sal_Int32(rInf.GetLen()));
                        }
                    }
                    else
                    {
                        Point aTmpPos( aTextOriginPos );
                        sal_Int32 i;
                        sal_Int32 j = 0;
                        long nSpaceSum = 0;
                        for (i = 0; i < sal_Int32(rInf.GetLen()); i++ )
                        {
                            if( CH_BLANK == rInf.GetText()[ sal_Int32(rInf.GetIdx()) + i ] )
                            {
                                nSpaceSum += nSpaceAdd;
                                if( j < i)
                                    rInf.GetOut().DrawTextArray( aTmpPos, rInf.GetText(),
                                    pKernArray.get() + j,
                                    sal_Int32(rInf.GetIdx()) + j, i - j );
                                j = i + 1;
                                pKernArray[i] = pKernArray[i] + nSpaceSum;
                                aTmpPos.setX( aTextOriginPos.X() + pKernArray[ i ] + nKernSum );
                            }
                        }
                        if( j < i )
                            rInf.GetOut().DrawTextArray( aTmpPos, rInf.GetText(),
                                pKernArray.get() + j,
                                sal_Int32(rInf.GetIdx()) + j, i - j );
                    }
                }
            }
            else
            {
                //long nKernAdd = rInf.GetKern();
                long nKernAdd = 0;
                long nGridAddSum = nGridWidthAdd + nKernAdd;
                for (sal_Int32 i = 0; i < sal_Int32(rInf.GetLen());
                     i++, nGridAddSum += nGridWidthAdd + nKernAdd)
                {
                    pKernArray[i] += nGridAddSum;
                }
                rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                    pKernArray.get(), sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
            }
            return;
        }
    }

    // DIRECT PAINTING WITHOUT SCREEN ADJUSTMENT

    if ( bDirectPrint )
    {
        const Fraction aTmp( 1, 1 );
        bool bStretch = rInf.GetWidth() && (rInf.GetLen() > TextFrameIndex(1)) && bPrt
                        && ( aTmp != rInf.GetOut().GetMapMode().GetScaleX() );

        if ( bSwitchL2R )
            rInf.GetFrame()->SwitchLTRtoRTL( aTextOriginPos );

        if ( bSwitchH2V )
            rInf.GetFrame()->SwitchHorizontalToVertical( aTextOriginPos );

        // In the good old days we used to have a simple DrawText if the
        // output device is the printer. Now we need a DrawTextArray if
        // 1. KanaCompression is enabled
        // 2. Justified alignment
        // Simple kerning is handled by DrawStretchText
        if( rInf.GetSpace() || rInf.GetKanaComp() )
        {
            std::unique_ptr<long[]> pKernArray(new long[sal_Int32(rInf.GetLen())]);
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                           sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));

            if( bStretch )
            {
                sal_Int32 nZwi = sal_Int32(rInf.GetLen()) - 1;
                long nDiff = rInf.GetWidth() - pKernArray[ nZwi ]
                             - sal_Int32(rInf.GetLen()) * rInf.GetKern();
                long nRest = nDiff % nZwi;
                long nAdd;
                if( nRest < 0 )
                {
                    nAdd = -1;
                    nRest += nZwi;
                }
                else
                {
                    nAdd = +1;
                    nRest = nZwi - nRest;
                }
                nDiff /= nZwi;
                long nSum = nDiff;
                for( sal_Int32 i = 0; i < nZwi; )
                {
                    pKernArray[ i ] += nSum;
                    if( ++i == nRest )
                        nDiff += nAdd;
                    nSum += nDiff;
                }
            }

            // Modify Array for special justifications

            long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
            bool bSpecialJust = false;

            if ( rInf.GetFont() && rInf.GetLen() )
            {
                const SwScriptInfo* pSI = rInf.GetScriptInfo();
                const SwFontScript nActual = rInf.GetFont()->GetActual();

                // Kana Compression
                if ( SwFontScript::CJK == nActual && rInf.GetKanaComp() &&
                     pSI && pSI->CountCompChg() &&
                     lcl_IsMonoSpaceFont( rInf.GetOut() ) )
                {
                    pSI->Compress( pKernArray.get(), rInf.GetIdx(), rInf.GetLen(),
                                   rInf.GetKanaComp(),
                                   static_cast<sal_uInt16>(m_aFont.GetFontSize().Height()), lcl_IsFullstopCentered( rInf.GetOut() ), &aTextOriginPos );
                    bSpecialJust = true;
                }

                // Asian Justification
                if ( SwFontScript::CJK == nActual && nSpaceAdd )
                {
                    LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CJK );

                    if (!MsLangId::isKorean(aLang))
                    {
                        SwScriptInfo::CJKJustify( rInf.GetText(), pKernArray.get(), nullptr,
                                rInf.GetIdx(), rInf.GetLen(), aLang, nSpaceAdd, rInf.IsSpaceStop() );

                        bSpecialJust = true;
                        nSpaceAdd = 0;
                    }
                }

                // Kashida Justification
                if ( SwFontScript::CTL == nActual && nSpaceAdd )
                {
                    if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() )
                        && rInf.GetOut().GetMinKashida() )
                    {
                        if ( pSI && pSI->CountKashida() &&
                            pSI->KashidaJustify( pKernArray.get(), nullptr, rInf.GetIdx(),
                                                 rInf.GetLen(), nSpaceAdd ) != -1 )
                        {
                            bSpecialJust = true;
                            nSpaceAdd = 0;
                        }
                    }
                }

                // Thai Justification
                if ( SwFontScript::CTL == nActual && nSpaceAdd )
                {
                    LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CTL );

                    if ( LANGUAGE_THAI == aLang )
                    {
                        // Use rInf.GetSpace() because it has more precision than
                        // nSpaceAdd:
                        SwScriptInfo::ThaiJustify( rInf.GetText(), pKernArray.get(), nullptr,
                                                   rInf.GetIdx(), rInf.GetLen(),
                                                   rInf.GetNumberOfBlanks(),
                                                   rInf.GetSpace() );

                        // adding space to blanks is already done
                        bSpecialJust = true;
                        nSpaceAdd = 0;
                    }
                }
            }

            long nKernSum = rInf.GetKern();

            if ( bStretch || m_bPaintBlank || rInf.GetKern() || bSpecialJust )
            {
                for (sal_Int32 i = 0; i < sal_Int32(rInf.GetLen()); i++,
                     nKernSum += rInf.GetKern() )
                {
                    if (CH_BLANK == rInf.GetText()[sal_Int32(rInf.GetIdx()) + i])
                        nKernSum += nSpaceAdd;
                    pKernArray[i] += nKernSum;
                }

                // In case of underlined/strike-through justified text
                // a blank at the end requires special handling:
                if( m_bPaintBlank && rInf.GetLen() && ( CH_BLANK ==
                    rInf.GetText()[sal_Int32(rInf.GetIdx() + rInf.GetLen())-1]))
                {
                    // If it is a single underlined space, output 2 spaces:
                    if (TextFrameIndex(1) == rInf.GetLen())
                    {
                        pKernArray[0] = rInf.GetWidth() + nSpaceAdd;

                        rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                             pKernArray.get(), sal_Int32(rInf.GetIdx()), 1 );
                    }
                    else
                    {
                        pKernArray[ sal_Int32(rInf.GetLen()) - 2 ] += nSpaceAdd;
                        rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                            pKernArray.get(), sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
                    }
                }
                else
                    rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                            pKernArray.get(), sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
            }
            else
            {
                Point aTmpPos( aTextOriginPos );
                sal_Int32 j = 0;
                sal_Int32 i;
                for( i = 0; i < sal_Int32(rInf.GetLen()); i++ )
                {
                    if (CH_BLANK == rInf.GetText()[sal_Int32(rInf.GetIdx()) + i])
                    {
                        nKernSum += nSpaceAdd;
                        if( j < i )
                            rInf.GetOut().DrawText( aTmpPos, rInf.GetText(),
                                        sal_Int32(rInf.GetIdx()) + j, i - j);
                        j = i + 1;
                        SwTwips nAdd = pKernArray[ i ] + nKernSum;
                        if ( ( ComplexTextLayoutFlags::BiDiStrong | ComplexTextLayoutFlags::BiDiRtl ) == nMode )
                            nAdd *= -1;
                        aTmpPos.setX( aTextOriginPos.X() + nAdd );
                    }
                }
                if( j < i )
                    rInf.GetOut().DrawText( aTmpPos, rInf.GetText(),
                                sal_Int32(rInf.GetIdx()) + j, i - j);
            }
        }
        else if( bStretch )
        {
            long nTmpWidth = rInf.GetWidth();
            if( rInf.GetKern() && rInf.GetLen() && nTmpWidth > rInf.GetKern() )
                nTmpWidth -= rInf.GetKern();
            rInf.GetOut().DrawStretchText( aTextOriginPos, nTmpWidth,
                       rInf.GetText(),
                       sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
        }
        else if( rInf.GetKern() )
        {
            const long nTmpWidth = GetTextSize( rInf ).Width();

            const Color aSaveColor( pTmpFont->GetColor() );
            const bool bColorChanged = rInf.ApplyAutoColor( pTmpFont );

            if( bColorChanged )
            {
                if( !pTmpFont->IsSameInstance( rInf.GetOut().GetFont() ) )
                    rInf.GetOut().SetFont( *pTmpFont );
                pTmpFont->SetColor( aSaveColor );
            }

            rInf.GetOut().DrawStretchText( aTextOriginPos, nTmpWidth,
                    rInf.GetText(),
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
        }
        else
            rInf.GetOut().DrawText( aTextOriginPos, rInf.GetText(),
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
    }

    // PAINTING WITH FORMATTING DEVICE/SCREEN ADJUSTMENT

    else
    {
        const OUString* pStr = &rInf.GetText();

        OUString aStr;
        OUString aBulletOverlay;
        bool bBullet = rInf.GetBullet();
        if( m_bSymbol )
            bBullet = false;
        std::unique_ptr<long[]> pKernArray(new long[sal_Int32(rInf.GetLen())]);
        CreateScrFont( *rInf.GetShell(), rInf.GetOut() );
        long nScrPos;

        // get screen array
        std::unique_ptr<long[]> pScrArray(new long[sal_Int32(rInf.GetLen())]);
        SwTextGlyphsKey aGlyphsKey{ &rInf.GetOut(), rInf.GetText(), sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()) };
        SalLayoutGlyphs* pGlyphs = lcl_CreateLayout(aGlyphsKey, m_aTextGlyphs[aGlyphsKey]);
        rInf.GetOut().GetTextArray( rInf.GetText(), pScrArray.get(),
                        sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()), nullptr, pGlyphs);

        // OLE: no printer available
        // OSL_ENSURE( pPrinter, "DrawText needs pPrinter" )
        if ( m_pPrinter )
        {
            // pTmpFont has already been set as current font for rInf.GetOut()
            if ( m_pPrinter.get() != rInf.GetpOut() || pTmpFont != m_pPrtFont )
            {
                if( !m_pPrtFont->IsSameInstance( m_pPrinter->GetFont() ) )
                    m_pPrinter->SetFont( *m_pPrtFont );
            }
            aGlyphsKey = SwTextGlyphsKey{ m_pPrinter, rInf.GetText(), sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()) };
            pGlyphs = lcl_CreateLayout(aGlyphsKey, m_aTextGlyphs[aGlyphsKey]);
            m_pPrinter->GetTextArray(rInf.GetText(), pKernArray.get(),
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()), nullptr, pGlyphs);
        }
        else
        {
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
        }

        // Modify Printer and ScreenArrays for special justifications

        long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
        bool bNoHalfSpace = false;

        if ( rInf.GetFont() && rInf.GetLen() )
        {
            const SwFontScript nActual = rInf.GetFont()->GetActual();
            const SwScriptInfo* pSI = rInf.GetScriptInfo();

            // Kana Compression
            if ( SwFontScript::CJK == nActual && rInf.GetKanaComp() &&
                 pSI && pSI->CountCompChg() &&
                 lcl_IsMonoSpaceFont( rInf.GetOut() ) )
            {
                Point aTmpPos( aTextOriginPos );
                pSI->Compress( pScrArray.get(), rInf.GetIdx(), rInf.GetLen(),
                               rInf.GetKanaComp(),
                               static_cast<sal_uInt16>(m_aFont.GetFontSize().Height()), lcl_IsFullstopCentered( rInf.GetOut() ), &aTmpPos );
                pSI->Compress( pKernArray.get(), rInf.GetIdx(), rInf.GetLen(),
                               rInf.GetKanaComp(),
                               static_cast<sal_uInt16>(m_aFont.GetFontSize().Height()), lcl_IsFullstopCentered( rInf.GetOut() ), &aTextOriginPos );
            }

            // Asian Justification
            if ( SwFontScript::CJK == nActual && nSpaceAdd )
            {
                LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CJK );

                if (!MsLangId::isKorean(aLang))
                {
                    SwScriptInfo::CJKJustify( rInf.GetText(), pKernArray.get(), pScrArray.get(),
                            rInf.GetIdx(), rInf.GetLen(), aLang, nSpaceAdd, rInf.IsSpaceStop() );

                    nSpaceAdd = 0;
                }
            }

            // Kashida Justification
            if ( SwFontScript::CTL == nActual && nSpaceAdd )
            {
                if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() )
                    && rInf.GetOut().GetMinKashida() )
                {
                    if ( pSI && pSI->CountKashida() &&
                         pSI->KashidaJustify( pKernArray.get(), pScrArray.get(), rInf.GetIdx(),
                                              rInf.GetLen(), nSpaceAdd ) != -1 )
                        nSpaceAdd = 0;
                    else
                        bNoHalfSpace = true;
                }
            }

            // Thai Justification
            if ( SwFontScript::CTL == nActual && nSpaceAdd )
            {
                LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CTL );

                if ( LANGUAGE_THAI == aLang )
                {
                    SwScriptInfo::ThaiJustify( rInf.GetText(), pKernArray.get(),
                                               pScrArray.get(), rInf.GetIdx(),
                                               rInf.GetLen(),
                                               rInf.GetNumberOfBlanks(),
                                               rInf.GetSpace() );

                    // adding space to blanks is already done
                    nSpaceAdd = 0;
                }
            }
        }

        nScrPos = pScrArray[ 0 ];

        if( bBullet )
        {
            // !!! HACK !!!
            // The Arabic layout engine requires some context of the string
            // which should be painted.
            sal_Int32 nCopyStart = sal_Int32(rInf.GetIdx());
            if ( nCopyStart )
                --nCopyStart;

            sal_Int32 nCopyLen = sal_Int32(rInf.GetLen());
            if ( nCopyStart + nCopyLen < rInf.GetText().getLength() )
                ++nCopyLen;

            aStr = rInf.GetText().copy( nCopyStart, nCopyLen );
            pStr = &aStr;

            aBulletOverlay = rInf.GetText().copy( nCopyStart, nCopyLen );

            for( sal_Int32 i = 0; i < aBulletOverlay.getLength(); ++i )
                if( CH_BLANK == aBulletOverlay[ i ] )
                {
                    /* fdo#72488 Hack: try to see if the space is zero width
                     * and don't bother with inserting a bullet in this case.
                     */
                    if ((i + nCopyStart + 1 >= sal_Int32(rInf.GetLen())) ||
                        pKernArray[i + nCopyStart] != pKernArray[ i + nCopyStart + 1])
                    {
                        aBulletOverlay = aBulletOverlay.replaceAt(i, 1, OUString(CH_BULLET));
                    }
                    else
                    {
                        aBulletOverlay = aBulletOverlay.replaceAt(i, 1, OUString(CH_BLANK));
                    }
                }
                else
                {
                    aBulletOverlay = aBulletOverlay.replaceAt(i, 1, OUString(CH_BLANK));
                }
        }

        TextFrameIndex nCnt(rInf.GetText().getLength());
        if ( nCnt < rInf.GetIdx() )
            assert(false); // layout bug, not handled below
        else
            nCnt = nCnt - rInf.GetIdx();
        nCnt = std::min(nCnt, rInf.GetLen());
        long nKernSum = rInf.GetKern();
        sal_Unicode cChPrev = rInf.GetText()[sal_Int32(rInf.GetIdx())];

        // In case of a single underlined space in justified text,
        // have to output 2 spaces:
        if ((nCnt == TextFrameIndex(1)) && rInf.GetSpace() && (cChPrev == CH_BLANK))
        {
            pKernArray[0] = rInf.GetWidth() +
                            rInf.GetKern() +
                          ( rInf.GetSpace() / SPACING_PRECISION_FACTOR );

            if ( bSwitchL2R )
                rInf.GetFrame()->SwitchLTRtoRTL( aTextOriginPos );

            if ( bSwitchH2V )
                rInf.GetFrame()->SwitchHorizontalToVertical( aTextOriginPos );

            rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                         pKernArray.get(), sal_Int32(rInf.GetIdx()), 1 );
            if( bBullet )
                rInf.GetOut().DrawTextArray( aTextOriginPos, *pStr, pKernArray.get(),
                                             rInf.GetIdx() ? 1 : 0, 1 );
        }
        else
        {
            sal_Unicode nCh;

            // In case of Pair Kerning the printer influence on the positioning
            // grows
            const int nMul = m_pPrtFont->GetKerning() != FontKerning::NONE ? 1 : 3;
            const int nDiv = nMul+1;

            // nSpaceSum contains the sum of the intermediate space distributed
            // among Spaces by the Justification.
            // The Spaces themselves will be positioned in the middle of the
            // intermediate space, hence the nSpace/2.
            // In case of word-by-word underlining they have to be positioned
            // at the beginning of the intermediate space, so that the space
            // is not underlined.
            // A Space at the beginning or end of the text must be positioned
            // before (resp. after) the whole intermediate space, otherwise
            // the underline/strike-through would have gaps.
            long nSpaceSum = 0;
            // in word line mode and for Arabic, we disable the half space trick:
            const long nHalfSpace = m_pPrtFont->IsWordLineMode() || bNoHalfSpace ? 0 : nSpaceAdd / 2;
            const long nOtherHalf = nSpaceAdd - nHalfSpace;
            if ( nSpaceAdd && ( cChPrev == CH_BLANK ) )
                nSpaceSum = nHalfSpace;
            for (sal_Int32 i = 1; i < sal_Int32(nCnt); ++i, nKernSum += rInf.GetKern())
            {
                nCh = rInf.GetText()[sal_Int32(rInf.GetIdx()) + i];

                OSL_ENSURE( pScrArray, "Where is the screen array?" );
                long nScr;
                nScr = pScrArray[ i ] - pScrArray[ i - 1 ];

                // If there is an (ex-)Space before us, position optimally,
                // i.e., our right margin to the 100% printer position;
                // if we _are_ an ex-Space, position us left-aligned to the
                // printer position.
                if ( nCh == CH_BLANK )
                {
                    nScrPos = pKernArray[i-1] + nScr;

                    if ( cChPrev == CH_BLANK )
                        nSpaceSum += nOtherHalf;
                    if (i + 1 == sal_Int32(nCnt))
                        nSpaceSum += nSpaceAdd;
                    else
                        nSpaceSum += nHalfSpace;
                }
                else
                {
                    if ( cChPrev == CH_BLANK )
                    {
                        nScrPos = pKernArray[i-1] + nScr;
                        // no Pixel is lost:
                        nSpaceSum += nOtherHalf;
                    }
                    else if ( cChPrev == '-' )
                        nScrPos = pKernArray[i-1] + nScr;
                    else
                    {
                        nScrPos += nScr;
                        nScrPos = ( nMul * nScrPos + pKernArray[i] ) / nDiv;
                    }
                }
                cChPrev = nCh;
                pKernArray[i-1] = nScrPos - nScr + nKernSum + nSpaceSum;
                // In word line mode and for Arabic, we disabled the half space trick. If a portion
                // ends with a blank, the full nSpaceAdd value has been added to the character in
                // front of the blank. This leads to painting artifacts, therefore we remove the
                // nSpaceAdd value again:
                if ((bNoHalfSpace || m_pPrtFont->IsWordLineMode()) && i+1 == sal_Int32(nCnt) && nCh == CH_BLANK)
                    pKernArray[i-1] = pKernArray[i-1] - nSpaceAdd;
            }

            // the layout engine requires the total width of the output
            pKernArray[sal_Int32(rInf.GetLen()) - 1] += nKernSum + nSpaceSum;

            if( rInf.GetGreyWave() )
            {
                if( rInf.GetLen() )
                {
                    long nHght = rInf.GetOut().LogicToPixel(
                                    m_pPrtFont->GetFontSize() ).Height();
                    if( WRONG_SHOW_MIN < nHght )
                    {
                        if ( rInf.GetOut().GetConnectMetaFile() )
                            rInf.GetOut().Push();

                        Color aCol( rInf.GetOut().GetLineColor() );
                        bool bColSave = aCol != gWaveCol;
                        if ( bColSave )
                            rInf.GetOut().SetLineColor( gWaveCol );

                        Point aEnd;
                        long nKernVal = pKernArray[sal_Int32(rInf.GetLen()) - 1];

                        const sal_uInt16 nDir = bBidiPor
                                                    ? 1800
                                                    : UnMapDirection(GetFont().GetOrientation(),
                                                                     bSwitchH2V, bSwitchH2VLRBT);

                        switch ( nDir )
                        {
                        case 0 :
                            aEnd.setX( rInf.GetPos().X() + nKernVal );
                            aEnd.setY( rInf.GetPos().Y() );
                            break;
                        case 900 :
                            aEnd.setX( rInf.GetPos().X() );
                            aEnd.setY( rInf.GetPos().Y() - nKernVal );
                            break;
                        case 1800 :
                            aEnd.setX( rInf.GetPos().X() - nKernVal );
                            aEnd.setY( rInf.GetPos().Y() );
                            break;
                        case 2700 :
                            aEnd.setX( rInf.GetPos().X() );
                            aEnd.setY( rInf.GetPos().Y() + nKernVal );
                            break;
                        }

                        Point aCurrPos( rInf.GetPos() );

                        if ( bSwitchL2R )
                        {
                            rInf.GetFrame()->SwitchLTRtoRTL( aCurrPos );
                            rInf.GetFrame()->SwitchLTRtoRTL( aEnd );
                        }

                        if ( bSwitchH2V )
                        {
                            rInf.GetFrame()->SwitchHorizontalToVertical( aCurrPos );
                            rInf.GetFrame()->SwitchHorizontalToVertical( aEnd );
                        }
                        rInf.GetOut().DrawWaveLine( aCurrPos, aEnd );

                        if ( bColSave )
                            rInf.GetOut().SetLineColor( aCol );

                        if ( rInf.GetOut().GetConnectMetaFile() )
                            rInf.GetOut().Pop();
                    }
                }
            }
            else if( !m_bSymbol && rInf.GetLen() )
            {
                // anything to do?
                if (rInf.GetWrong() || rInf.GetGrammarCheck() || rInf.GetSmartTags())
                {
                    CalcLinePosData aCalcLinePosData(rInf, GetFont(), nCnt, bSwitchH2V,
                                                     bSwitchH2VLRBT, bSwitchL2R, nHalfSpace,
                                                     pKernArray.get(), bBidiPor);

                    SwForbidden aForbidden;
                    // draw line for smart tag data
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetSmartTags(), aCalcLinePosData, Size() );
                    // draw wave line for spell check errors
                    // draw them BEFORE the grammar check lines to 'override' the latter in case of conflict.
                    // reason: some grammar errors can only be found if spelling errors are fixed,
                    // therefore we don't want the user to miss a spelling error.
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetWrong(), aCalcLinePosData, m_pPrtFont->GetFontSize() );
                    // draw wave line for grammar check errors
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetGrammarCheck(), aCalcLinePosData, m_pPrtFont->GetFontSize() );
                }
            }

            sal_Int32 nLen = sal_Int32(rInf.GetLen());

            if( nLen > 0 )
            {

                if ( bSwitchL2R )
                    rInf.GetFrame()->SwitchLTRtoRTL( aTextOriginPos );

                if ( bSwitchH2V )
                    rInf.GetFrame()->SwitchHorizontalToVertical( aTextOriginPos );

                // If we paint bullets instead of spaces, we use a copy of
                // the paragraph string. For the layout engine, the copy
                // of the string has to be an environment of the range which
                // is painted
                sal_Int32 nTmpIdx = bBullet
                            ? (rInf.GetIdx() ? 1 : 0)
                            : sal_Int32(rInf.GetIdx());
                aGlyphsKey = SwTextGlyphsKey{ &rInf.GetOut(), *pStr, nTmpIdx, nLen };
                pGlyphs = lcl_CreateLayout(aGlyphsKey, m_aTextGlyphs[aGlyphsKey]);
                rInf.GetOut().DrawTextArray( aTextOriginPos, *pStr, pKernArray.get(),
                                             nTmpIdx , nLen, SalLayoutFlags::NONE, pGlyphs );
                if (bBullet)
                {
                    rInf.GetOut().Push();
                    Color aPreviousColor = pTmpFont->GetColor();

                    FontLineStyle aPreviousUnderline = pTmpFont->GetUnderline();
                    FontLineStyle aPreviousOverline = pTmpFont->GetOverline();
                    FontStrikeout aPreviousStrikeout = pTmpFont->GetStrikeout();

                    pTmpFont->SetColor( NON_PRINTING_CHARACTER_COLOR );
                    pTmpFont->SetUnderline(LINESTYLE_NONE);
                    pTmpFont->SetOverline(LINESTYLE_NONE);
                    pTmpFont->SetStrikeout(STRIKEOUT_NONE);
                    rInf.GetOut().SetFont( *pTmpFont );
                    long nShift = rInf.GetOut( ).GetFontMetric( ).GetBulletOffset( );
                    if ( nShift )
                    {
                        long nAdd = 0;

                        if (aBulletOverlay.getLength() > nTmpIdx &&
                            aBulletOverlay[ nTmpIdx ] == CH_BULLET )
                        {
                            if (bSwitchH2V)
                                aTextOriginPos.AdjustY(nShift ) ;
                            else
                                aTextOriginPos.AdjustX(nShift ) ;
                            nAdd = nShift ;
                        }
                        for( sal_Int32 i = 1 ; i < nLen ; ++i )
                        {
                            if ( aBulletOverlay[ i + nTmpIdx ] == CH_BULLET )
                                pKernArray [ i - 1 ] += nShift ;
                            if ( nAdd )
                                pKernArray [ i - 1 ] -= nAdd;
                        }
                    }
                    rInf.GetOut().DrawTextArray( aTextOriginPos, aBulletOverlay, pKernArray.get(),
                                                 nTmpIdx , nLen );
                    pTmpFont->SetColor( aPreviousColor );

                    pTmpFont->SetUnderline(aPreviousUnderline);
                    pTmpFont->SetOverline(aPreviousOverline);
                    pTmpFont->SetStrikeout(aPreviousStrikeout);
                    rInf.GetOut().Pop();
                }
            }
        }
    }
}

Size SwFntObj::GetTextSize( SwDrawTextInfo& rInf )
{
    Size aTextSize;
    const TextFrameIndex nLn = (TextFrameIndex(COMPLETE_STRING) != rInf.GetLen())
        ? rInf.GetLen()
        : TextFrameIndex(rInf.GetText().getLength());

    // be sure to have the correct layout mode at the printer
    if ( m_pPrinter )
    {
        m_pPrinter->SetLayoutMode( rInf.GetOut().GetLayoutMode() );
        m_pPrinter->SetDigitLanguage( rInf.GetOut().GetDigitLanguage() );
    }

    if ( rInf.GetFrame() && nLn && rInf.SnapToGrid() && rInf.GetFont() &&
         SwFontScript::CJK == rInf.GetFont()->GetActual() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars() )
        {
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GetGridWidth(*pGrid, *pDoc);

            OutputDevice* pOutDev;

            if ( m_pPrinter )
            {
                if( !m_pPrtFont->IsSameInstance( m_pPrinter->GetFont() ) )
                    m_pPrinter->SetFont(*m_pPrtFont);
                pOutDev = m_pPrinter;
            }
            else
                pOutDev = rInf.GetpOut();

            aTextSize.setWidth( pOutDev->GetTextWidth(rInf.GetText(),
                        sal_Int32(rInf.GetIdx()), sal_Int32(nLn)) );

            OSL_ENSURE( !rInf.GetShell() ||
                    ( USHRT_MAX != GetGuessedLeading() && USHRT_MAX != GetExternalLeading() ),
                "Leading values should be already calculated" );
            aTextSize.setHeight( pOutDev->GetTextHeight() +
                                GetFontLeading( rInf.GetShell(), rInf.GetOut() ) );

            long nAvgWidthPerChar = aTextSize.Width() / sal_Int32(nLn);

            const sal_uLong i = nAvgWidthPerChar ?
                            ( nAvgWidthPerChar - 1 ) / nGridWidth + 1:
                            1;

            aTextSize.setWidth(i * nGridWidth * sal_Int32(nLn));
            rInf.SetKanaDiff( 0 );
            return aTextSize;
        }
    }

    //for textgrid refactor
    if ( rInf.GetFrame() && nLn && rInf.SnapToGrid() && rInf.GetFont() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && !pGrid->IsSnapToChars() )
        {
            const long nGridWidthAdd = EvalGridWidthAdd( pGrid, rInf );
            OutputDevice* pOutDev;
            if ( m_pPrinter )
            {
                if( !m_pPrtFont->IsSameInstance( m_pPrinter->GetFont() ) )
                    m_pPrinter->SetFont(*m_pPrtFont);
                pOutDev = m_pPrinter;
            }
            else
                pOutDev = rInf.GetpOut();
            aTextSize.setWidth(pOutDev->GetTextWidth(rInf.GetText(),
                        sal_Int32(rInf.GetIdx()), sal_Int32(nLn)));
            aTextSize.setHeight( pOutDev->GetTextHeight() +
                                GetFontLeading( rInf.GetShell(), rInf.GetOut() ) );
            aTextSize.AdjustWidth(sal_Int32(nLn) * nGridWidthAdd);
            //if ( rInf.GetKern() && nLn )
            //    aTextSize.Width() += ( nLn ) * long( rInf.GetKern() );

            rInf.SetKanaDiff( 0 );
            return aTextSize;
        }
    }

    const bool bCompress = rInf.GetKanaComp() && nLn &&
                           rInf.GetFont() &&
                           SwFontScript::CJK == rInf.GetFont()->GetActual() &&
                           rInf.GetScriptInfo() &&
                           rInf.GetScriptInfo()->CountCompChg() &&
                           lcl_IsMonoSpaceFont( rInf.GetOut() );

    OSL_ENSURE( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
            CountCompChg()), "Compression without info" );

    // This is the part used e.g., for cursor travelling
    // See condition for DrawText or DrawTextArray (bDirectPrint)
    if ( m_pPrinter && m_pPrinter.get() != rInf.GetpOut() )
    {
        if( !m_pPrtFont->IsSameInstance( m_pPrinter->GetFont() ) )
            m_pPrinter->SetFont(*m_pPrtFont);
        aTextSize.setWidth( m_pPrinter->GetTextWidth( rInf.GetText(),
                               sal_Int32(rInf.GetIdx()), sal_Int32(nLn)));
        aTextSize.setHeight( m_pPrinter->GetTextHeight() );
        std::unique_ptr<long[]> pKernArray(new long[sal_Int32(nLn)]);
        CreateScrFont( *rInf.GetShell(), rInf.GetOut() );
        if( !GetScrFont()->IsSameInstance( rInf.GetOut().GetFont() ) )
            rInf.GetOut().SetFont( *m_pScrFont );
        long nScrPos;

        m_pPrinter->GetTextArray(rInf.GetText(), pKernArray.get(),
                sal_Int32(rInf.GetIdx()), sal_Int32(nLn));
        if( bCompress )
            rInf.SetKanaDiff( rInf.GetScriptInfo()->Compress( pKernArray.get(),
                rInf.GetIdx(), nLn, rInf.GetKanaComp(),
                static_cast<sal_uInt16>(m_aFont.GetFontSize().Height()) ,lcl_IsFullstopCentered( rInf.GetOut() ) ) );
        else
            rInf.SetKanaDiff( 0 );

        if ( rInf.GetKanaDiff() )
            nScrPos = pKernArray[ sal_Int32(nLn) - 1 ];
        else
        {
            std::unique_ptr<long[]> pScrArray(new long[sal_Int32(rInf.GetLen())]);
            rInf.GetOut().GetTextArray( rInf.GetText(), pScrArray.get(),
                        sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
            nScrPos = pScrArray[ 0 ];
            TextFrameIndex nCnt(rInf.GetText().getLength());
            if ( nCnt < rInf.GetIdx() )
                nCnt = TextFrameIndex(0); // assert???
            else
                nCnt = nCnt - rInf.GetIdx();
            nCnt = std::min(nCnt, nLn);
            sal_Unicode nChPrev = rInf.GetText()[ sal_Int32(rInf.GetIdx()) ];

            sal_Unicode nCh;

            // In case of Pair Kerning the printer influence on the positioning
            // grows
            const int nMul = m_pPrtFont->GetKerning() != FontKerning::NONE ? 1 : 3;
            const int nDiv = nMul+1;
            for (sal_Int32 i = 1; i < sal_Int32(nCnt); i++)
            {
                nCh = rInf.GetText()[ sal_Int32(rInf.GetIdx()) + i ];
                long nScr;
                nScr = pScrArray[ i ] - pScrArray[ i - 1 ];
                if ( nCh == CH_BLANK )
                    nScrPos = pKernArray[i-1]+nScr;
                else
                {
                    if ( nChPrev == CH_BLANK || nChPrev == '-' )
                        nScrPos = pKernArray[i-1]+nScr;
                    else
                    {
                        nScrPos += nScr;
                        nScrPos = ( nMul * nScrPos + pKernArray[i] ) / nDiv;
                    }
                }
                nChPrev = nCh;
                pKernArray[i-1] = nScrPos - nScr;
            }
        }

        pKernArray.reset();
        aTextSize.setWidth( nScrPos );
    }
    else
    {
        if( !m_pPrtFont->IsSameInstance( rInf.GetOut().GetFont() ) )
            rInf.GetOut().SetFont( *m_pPrtFont );
        if( bCompress )
        {
            std::unique_ptr<long[]> pKernArray( new long[sal_Int32(nLn)] );
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                                sal_Int32(rInf.GetIdx()), sal_Int32(nLn));
            rInf.SetKanaDiff( rInf.GetScriptInfo()->Compress( pKernArray.get(),
                rInf.GetIdx(), nLn, rInf.GetKanaComp(),
                static_cast<sal_uInt16>(m_aFont.GetFontSize().Height()) ,lcl_IsFullstopCentered( rInf.GetOut() ) ) );
            aTextSize.setWidth( pKernArray[sal_Int32(nLn) - 1] );
        }
        else
        {
            SwTextGlyphsKey aGlyphsKey{ &rInf.GetOut(), rInf.GetText(), sal_Int32(rInf.GetIdx()), sal_Int32(nLn) };
            SalLayoutGlyphs* pGlyphs = lcl_CreateLayout(aGlyphsKey, m_aTextGlyphs[aGlyphsKey]);
            aTextSize.setWidth( rInf.GetOut().GetTextWidth( rInf.GetText(),
                                   sal_Int32(rInf.GetIdx()), sal_Int32(nLn),
                                                           rInf.GetVclCache(), pGlyphs) );
            rInf.SetKanaDiff( 0 );
        }

        aTextSize.setHeight( rInf.GetOut().GetTextHeight() );
    }

    if ( rInf.GetKern() && nLn )
        aTextSize.AdjustWidth((sal_Int32(nLn) - 1) * rInf.GetKern());

    OSL_ENSURE( !rInf.GetShell() ||
            ( USHRT_MAX != GetGuessedLeading() && USHRT_MAX != GetExternalLeading() ),
              "Leading values should be already calculated" );
    aTextSize.AdjustHeight(GetFontLeading( rInf.GetShell(), rInf.GetOut() ) );
    return aTextSize;
}

TextFrameIndex SwFntObj::GetCursorOfst(SwDrawTextInfo &rInf)
{
    long nSpaceAdd =       rInf.GetSpace() / SPACING_PRECISION_FACTOR;
    const long nSperren = -rInf.GetSperren() / SPACING_PRECISION_FACTOR;
    long nKern = rInf.GetKern();

    if( 0 != nSperren )
        nKern -= nSperren;

    std::unique_ptr<long[]> pKernArray(new long[sal_Int32(rInf.GetLen())]);

    // be sure to have the correct layout mode at the printer
    if ( m_pPrinter )
    {
        m_pPrinter->SetLayoutMode( rInf.GetOut().GetLayoutMode() );
        m_pPrinter->SetDigitLanguage( rInf.GetOut().GetDigitLanguage() );
        SwTextGlyphsKey aGlyphsKey{ m_pPrinter, rInf.GetText(), sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()) };
        SalLayoutGlyphs* pGlyphs = lcl_CreateLayout(aGlyphsKey, m_aTextGlyphs[aGlyphsKey]);
        m_pPrinter->GetTextArray( rInf.GetText(), pKernArray.get(),
                sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()), nullptr, pGlyphs);
    }
    else
        rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));

    const SwScriptInfo* pSI = rInf.GetScriptInfo();
    if ( rInf.GetFont() && rInf.GetLen() )
    {
        const SwFontScript nActual = rInf.GetFont()->GetActual();

        // Kana Compression
        if ( SwFontScript::CJK == nActual && rInf.GetKanaComp() &&
             pSI && pSI->CountCompChg() &&
             lcl_IsMonoSpaceFont( rInf.GetOut() ) )
        {
            pSI->Compress( pKernArray.get(), rInf.GetIdx(), rInf.GetLen(),
                           rInf.GetKanaComp(),
                           static_cast<sal_uInt16>(m_aFont.GetFontSize().Height()),
                           lcl_IsFullstopCentered( rInf.GetOut() ) );
        }

        // Asian Justification
        if ( SwFontScript::CJK == rInf.GetFont()->GetActual() )
        {
            LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CJK );

            if (!MsLangId::isKorean(aLang))
            {
                SwScriptInfo::CJKJustify( rInf.GetText(), pKernArray.get(), nullptr,
                        rInf.GetIdx(), rInf.GetLen(), aLang, nSpaceAdd, rInf.IsSpaceStop() );

                nSpaceAdd = 0;
            }

        }

        // Kashida Justification
        if ( SwFontScript::CTL == nActual && rInf.GetSpace() )
        {
            if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() )
                && rInf.GetOut().GetMinKashida() )
            {
                if ( pSI && pSI->CountKashida() &&
                    pSI->KashidaJustify( pKernArray.get(), nullptr, rInf.GetIdx(), rInf.GetLen(),
                                         nSpaceAdd ) != -1 )
                    nSpaceAdd = 0;
            }
        }

        // Thai Justification
        if ( SwFontScript::CTL == nActual && nSpaceAdd )
        {
            LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CTL );

            if ( LANGUAGE_THAI == aLang )
            {
                SwScriptInfo::ThaiJustify( rInf.GetText(), pKernArray.get(), nullptr,
                                           rInf.GetIdx(), rInf.GetLen(),
                                           rInf.GetNumberOfBlanks(),
                                           rInf.GetSpace() );

                // adding space to blanks is already done
                nSpaceAdd = 0;
            }
        }
    }

    long nLeft = 0;
    long nRight = 0;
    TextFrameIndex nCnt(0);
    long nSpaceSum = 0;
    long nKernSum = 0;

    if ( rInf.GetFrame() && rInf.GetLen() && rInf.SnapToGrid() &&
         rInf.GetFont() && SwFontScript::CJK == rInf.GetFont()->GetActual() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars() )
        {
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GetGridWidth(*pGrid, *pDoc);

            long nAvgWidthPerChar = pKernArray[sal_Int32(rInf.GetLen()) - 1] / sal_Int32(rInf.GetLen());

            sal_uLong i = nAvgWidthPerChar ?
                      ( nAvgWidthPerChar - 1 ) / nGridWidth + 1:
                      1;

            nAvgWidthPerChar = i * nGridWidth;

// stupid CLANG
            nCnt = TextFrameIndex(rInf.GetOfst() / nAvgWidthPerChar);
            if (2 * (rInf.GetOfst() - sal_Int32(nCnt) * nAvgWidthPerChar) > nAvgWidthPerChar)
                ++nCnt;

            return nCnt;
        }
    }

    //for textgrid refactor
    if ( rInf.GetFrame() && rInf.GetLen() && rInf.SnapToGrid() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && !pGrid->IsSnapToChars() )
        {

            const long nGridWidthAdd = EvalGridWidthAdd( pGrid, rInf );

            for (TextFrameIndex j = TextFrameIndex(0); j < rInf.GetLen(); j++)
            {
                long nScr = pKernArray[sal_Int32(j)] + (nSpaceAdd + nGridWidthAdd) * (sal_Int32(j) + 1);
                if( nScr >= rInf.GetOfst())
                {
                    nCnt = j;
                    break;
                }
            }
            return nCnt;
        }
    }

    sal_Int32 nDone = 0;
    TextFrameIndex nIdx = rInf.GetIdx();
    TextFrameIndex nLastIdx = nIdx;
    const TextFrameIndex nEnd = rInf.GetIdx() + rInf.GetLen();

    // #i105901#
    // skip character cells for all script types
    LanguageType aLang = rInf.GetFont()->GetLanguage();

    while ( ( nRight < long( rInf.GetOfst() ) ) && ( nIdx < nEnd ) )
    {
        if (nSpaceAdd && CH_BLANK == rInf.GetText()[ sal_Int32(nIdx)])
            nSpaceSum += nSpaceAdd;

        // go to next character (cell).
        nLastIdx = nIdx;

        nIdx = TextFrameIndex(g_pBreakIt->GetBreakIter()->nextCharacters(
                    rInf.GetText(), sal_Int32(nIdx),
                    g_pBreakIt->GetLocale( aLang ),
                    i18n::CharacterIteratorMode::SKIPCELL, 1, nDone));
        if ( nIdx <= nLastIdx )
            break;

        nLeft = nRight;
        nRight = pKernArray[sal_Int32(nIdx - rInf.GetIdx()) - 1] + nKernSum + nSpaceSum;

        nKernSum += nKern;
    }

    // step back if position is before the middle of the character
    // or if we do not want to go to the next character
    if ( nIdx > rInf.GetIdx() &&
         ( rInf.IsPosMatchesBounds() ||
           ( ( nRight > long( rInf.GetOfst() ) ) &&
             ( nRight - rInf.GetOfst() > rInf.GetOfst() - nLeft ) ) ) )
        nCnt = nLastIdx - rInf.GetIdx(); // first half
    else
        nCnt = nIdx - rInf.GetIdx(); // second half

    if ( pSI )
        rInf.SetCursorBidiLevel( pSI->DirType( nLastIdx ) );

    return nCnt;
}

SwFntAccess::SwFntAccess( const void* & rnFontCacheId,
                sal_uInt16 &rIndex, const void *pOwn, SwViewShell const *pSh,
                bool bCheck ) :
  SwCacheAccess( *pFntCache, rnFontCacheId, rIndex ),
  m_pShell( pSh )
{
    // the used ctor of SwCacheAccess searches for rnFontCacheId+rIndex in the cache
    if ( IsAvail() )
    {
        // fast case: known Font (rnFontCacheId), no need to check printer and zoom
        if ( !bCheck )
            return;

        // Font is known, but has to be checked
    }
    else
    {   // Font not known, must be searched
        bCheck = false;
    }

    {
        OutputDevice* pOut = nullptr;
        sal_uInt16 nZoom = USHRT_MAX;

        // Get the reference device
        if ( pSh )
        {
            pOut = &pSh->GetRefDev();
            nZoom = pSh->GetViewOptions()->GetZoom();
        }

        SwFntObj *pFntObj;
        if ( bCheck )
        {
            pFntObj = Get();
            if ( ( pFntObj->GetZoom( ) == nZoom ) &&
                 ( pFntObj->m_pPrinter == pOut ) &&
                   pFntObj->GetPropWidth() ==
                        static_cast<SwSubFont const *>(pOwn)->GetPropWidth() )
            {
                return; // result of Check: Drucker+Zoom okay.
            }
            pFntObj->Unlock(); // forget this object, printer/zoom differs
            m_pObj = nullptr;
        }

        // Search by font comparison, quite expensive!
        // Look for same font and same printer
        pFntObj = pFntCache->First();
        while ( pFntObj && !( pFntObj->m_aFont == *static_cast<vcl::Font const *>(pOwn) &&
                              pFntObj->GetZoom() == nZoom &&
                              pFntObj->GetPropWidth() ==
                              static_cast<SwSubFont const *>(pOwn)->GetPropWidth() &&
                              ( !pFntObj->m_pPrinter || pFntObj->m_pPrinter == pOut ) ) )
            pFntObj = SwFntCache::Next( pFntObj );

        if( pFntObj && pFntObj->m_pPrinter.get() != pOut )
        {
            // found one without printer, let's see if there is one with
            // the same printer as well
            SwFntObj *pTmpObj = pFntObj;
            while( pTmpObj && !( pTmpObj->m_aFont == *static_cast<vcl::Font const *>(pOwn) &&
                   pTmpObj->GetZoom()==nZoom && pTmpObj->m_pPrinter==pOut &&
                   pTmpObj->GetPropWidth() ==
                        static_cast<SwSubFont const *>(pOwn)->GetPropWidth() ) )
                pTmpObj = SwFntCache::Next( pTmpObj );
            if( pTmpObj )
                pFntObj = pTmpObj;
        }

        if ( !pFntObj ) // Font has not been found, create one
        {
            // Have to create new Object, hence Owner must be a SwFont, later
            // the Owner will be the "MagicNumber"
            SwCacheAccess::m_pOwner = pOwn;
            pFntObj = Get(); // will create via NewObj() and lock
            OSL_ENSURE(pFntObj, "No Font, no Fun.");
        }
        else  // Font has been found, so we lock it.
        {
            pFntObj->Lock();
            if (pFntObj->m_pPrinter.get() != pOut) // if no printer is known by now
            {
                OSL_ENSURE( !pFntObj->m_pPrinter, "SwFntAccess: Printer Changed" );
                pFntObj->CreatePrtFont( *pOut );
                pFntObj->m_pPrinter = pOut;
                pFntObj->m_pScrFont = nullptr;
                pFntObj->m_nGuessedLeading = USHRT_MAX;
                pFntObj->m_nExtLeading = USHRT_MAX;
                pFntObj->m_nPrtAscent = USHRT_MAX;
                pFntObj->m_nPrtHeight = USHRT_MAX;
            }
            m_pObj = pFntObj;
        }

        // no matter if new or found, now the Owner of the Object is a
        // MagicNumber, and will be given to the SwFont, as well as the Index
        // for later direct access
        rnFontCacheId = reinterpret_cast<void*>(reinterpret_cast<sal_IntPtr>(pFntObj->GetOwner()));
        SwCacheAccess::m_pOwner = pFntObj->GetOwner();
        rIndex = pFntObj->GetCachePos();
    }
}

SwCacheObj *SwFntAccess::NewObj( )
{
    // a new Font, a new "MagicNumber".
    return new SwFntObj( *static_cast<SwSubFont const *>(m_pOwner), ++mnFontCacheIdCounter, m_pShell );
}

TextFrameIndex SwFont::GetTextBreak(SwDrawTextInfo const & rInf, long nTextWidth)
{
    ChgFnt( rInf.GetShell(), rInf.GetOut() );

    const bool bCompress = rInf.GetKanaComp() && rInf.GetLen() &&
                           SwFontScript::CJK == GetActual() &&
                           rInf.GetScriptInfo() &&
                           rInf.GetScriptInfo()->CountCompChg() &&
                           lcl_IsMonoSpaceFont( rInf.GetOut() );

    OSL_ENSURE( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
            CountCompChg()), "Compression without info" );

    TextFrameIndex nTextBreak(0);
    long nKern = 0;

    TextFrameIndex nLn = rInf.GetLen() == TextFrameIndex(COMPLETE_STRING)
        ? TextFrameIndex(rInf.GetText().getLength()) : rInf.GetLen();

    if ( rInf.GetFrame() && nLn && rInf.SnapToGrid() &&
         rInf.GetFont() && SwFontScript::CJK == rInf.GetFont()->GetActual() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars() )
        {
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GetGridWidth(*pGrid, *pDoc);

            std::unique_ptr<long[]> pKernArray(new long[sal_Int32(rInf.GetLen())]);
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));

            long nAvgWidthPerChar = pKernArray[sal_Int32(rInf.GetLen()) - 1] / sal_Int32(rInf.GetLen());

            const sal_uLong i = nAvgWidthPerChar ?
                            ( nAvgWidthPerChar - 1 ) / nGridWidth + 1:
                            1;

            nAvgWidthPerChar = i * nGridWidth;
            long nCurrPos = nAvgWidthPerChar;

            while( nTextBreak < rInf.GetLen() && nTextWidth >= nCurrPos )
            {
                nCurrPos += nAvgWidthPerChar;
                ++nTextBreak;
            }

            return nTextBreak + rInf.GetIdx();
        }
    }

    //for text grid enhancement
    if ( rInf.GetFrame() && nLn && rInf.SnapToGrid() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && !pGrid->IsSnapToChars() )
        {
            const long nGridWidthAdd = EvalGridWidthAdd( pGrid, rInf );

            std::unique_ptr<long[]> pKernArray(new long[sal_Int32(rInf.GetLen())] );
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                        sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
            long nCurrPos = pKernArray[sal_Int32(nTextBreak)] + nGridWidthAdd;
            while (++nTextBreak < rInf.GetLen() && nTextWidth >= nCurrPos)
            {
                nCurrPos = pKernArray[sal_Int32(nTextBreak)] + nGridWidthAdd * (sal_Int32(nTextBreak) + 1);
            }
            return nTextBreak + rInf.GetIdx();
        }
    }

    if( m_aSub[m_nActual].IsCapital() && nLn )
    {
        nTextBreak = GetCapitalBreak( rInf.GetShell(), rInf.GetpOut(),
            rInf.GetScriptInfo(), rInf.GetText(), nTextWidth, rInf.GetIdx(),
            nLn );
    }
    else
    {
        nKern = CheckKerning();

        const OUString* pTmpText;
        OUString aTmpText;
        TextFrameIndex nTmpIdx;
        TextFrameIndex nTmpLen;
        bool bTextReplaced = false;

        if ( !m_aSub[m_nActual].IsCaseMap() )
        {
            pTmpText = &rInf.GetText();
            nTmpIdx = rInf.GetIdx();
            nTmpLen = nLn;
        }
        else
        {
            const OUString aSnippet(rInf.GetText().copy(sal_Int32(rInf.GetIdx()), sal_Int32(nLn)));
            aTmpText = m_aSub[m_nActual].CalcCaseMap( aSnippet );
            const bool bTitle = SvxCaseMap::Capitalize == m_aSub[m_nActual].GetCaseMap();

            // Uaaaaahhhh!!! In title case mode, we would get wrong results
            if ( bTitle && nLn )
            {
                // check if rInf.GetIdx() is begin of word
                if ( !g_pBreakIt->GetBreakIter()->isBeginWord(
                     rInf.GetText(), sal_Int32(rInf.GetIdx()),
                     g_pBreakIt->GetLocale( m_aSub[m_nActual].GetLanguage() ),
                     i18n::WordType::ANYWORD_IGNOREWHITESPACES ) )
                {
                    // In this case, the beginning of aTmpText is wrong.
                    OUString aSnippetTmp(aSnippet.copy(0, 1));
                    aSnippetTmp = m_aSub[m_nActual].CalcCaseMap( aSnippetTmp );
                    aTmpText = aTmpText.replaceAt( 0, aSnippetTmp.getLength(), OUString(aSnippet[0]) );
                }
            }

            pTmpText = &aTmpText;
            nTmpIdx = TextFrameIndex(0);
            nTmpLen = TextFrameIndex(aTmpText.getLength());
            bTextReplaced = true;
        }

        if( rInf.GetHyphPos() ) {
            sal_Int32 nHyphPos = sal_Int32(*rInf.GetHyphPos());
            nTextBreak = TextFrameIndex(rInf.GetOut().GetTextBreak(
                             *pTmpText, nTextWidth,
                             u'-', nHyphPos,
                             sal_Int32(nTmpIdx), sal_Int32(nTmpLen),
                             nKern, rInf.GetVclCache()));
            *rInf.GetHyphPos() = TextFrameIndex((nHyphPos == -1) ? COMPLETE_STRING : nHyphPos);
        }
        else
        {
            SwFntAccess aFntAccess(m_aSub[m_nActual].m_nFontCacheId, m_aSub[m_nActual].m_nFontIndex,
                                   &m_aSub[m_nActual], rInf.GetShell());
            SwTextGlyphsKey aGlyphsKey{ &rInf.GetOut(), *pTmpText, sal_Int32(nTmpIdx), sal_Int32(nTmpLen) };
            SalLayoutGlyphs* pGlyphs
                = lcl_CreateLayout(aGlyphsKey, aFntAccess.Get()->GetTextGlyphs()[aGlyphsKey]);
            nTextBreak = TextFrameIndex(rInf.GetOut().GetTextBreak(
                             *pTmpText, nTextWidth,
                             sal_Int32(nTmpIdx), sal_Int32(nTmpLen),
                             nKern, rInf.GetVclCache(), pGlyphs));
        }

        if (bTextReplaced && sal_Int32(nTextBreak) != -1)
        {
            if ( nTmpLen != nLn )
                nTextBreak = sw_CalcCaseMap( *this, rInf.GetText(),
                                             rInf.GetIdx(), nLn, nTextBreak );
            else
                nTextBreak = nTextBreak + rInf.GetIdx();
        }
    }

    TextFrameIndex nTextBreak2 = sal_Int32(nTextBreak) == -1
            ? TextFrameIndex(COMPLETE_STRING)
            : nTextBreak;

    if ( ! bCompress )
        return nTextBreak2;

    nTextBreak2 = nTextBreak2 - rInf.GetIdx();

    if( nTextBreak2 < nLn )
    {
        if( !nTextBreak2 && nLn )
            nLn = TextFrameIndex(1);
        else if (nLn > nTextBreak2 + nTextBreak2)
            nLn = nTextBreak2 + nTextBreak2;
        std::unique_ptr<long[]> pKernArray( new long[sal_Int32(nLn)] );
        rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray.get(),
                                    sal_Int32(rInf.GetIdx()), sal_Int32(nLn));
        if( rInf.GetScriptInfo()->Compress( pKernArray.get(), rInf.GetIdx(), nLn,
                            rInf.GetKanaComp(), static_cast<sal_uInt16>(GetHeight( m_nActual )),
                            lcl_IsFullstopCentered( rInf.GetOut() ) ) )
        {
            long nKernAdd = nKern;
            TextFrameIndex const nTmpBreak = nTextBreak2;
            if( nKern && nTextBreak2 )
                nKern *= sal_Int32(nTextBreak2) - 1;
            while (nTextBreak2 < nLn && nTextWidth >= pKernArray[sal_Int32(nTextBreak2)] + nKern)
            {
                nKern += nKernAdd;
                ++nTextBreak2;
            }
            if( rInf.GetHyphPos() )
                *rInf.GetHyphPos() += nTextBreak2 - nTmpBreak; // It's not perfect
        }
    }
    nTextBreak2 = nTextBreak2 + rInf.GetIdx();

    return nTextBreak2;
}

bool SwDrawTextInfo::ApplyAutoColor( vcl::Font* pFont )
{
    const vcl::Font& rFnt = pFont ? *pFont : GetOut().GetFont();
    Color nNewColor = COL_BLACK;
    bool bChgFntColor = false;
    bool bChgLineColor = false;

    if (GetShell() && !GetShell()->GetWin() && GetShell()->GetViewOptions()->IsBlackFont())
    {
        if ( COL_BLACK != rFnt.GetColor() )
            bChgFntColor = true;

        if ( (COL_BLACK != GetOut().GetLineColor()) ||
             (COL_BLACK != GetOut().GetOverlineColor()) )
            bChgLineColor = true;
    }
    else
    {
        // FontColor has to be changed if:
        // 1. FontColor = AUTO or 2. IsAlwaysAutoColor is set
        // LineColor has to be changed if:
        // 1. IsAlwaysAutoColor is set

        bChgLineColor = GetShell() && GetShell()->GetWin() &&
                GetShell()->GetAccessibilityOptions()->IsAlwaysAutoColor();

        bChgFntColor = COL_AUTO == rFnt.GetColor() || bChgLineColor;

        if ( bChgFntColor )
        {
            // check if current background has a user defined setting
            const Color* pCol = GetFont() ? GetFont()->GetBackColor() : nullptr;
            Color aColor;
            if( ! pCol || COL_TRANSPARENT == *pCol )
            {
                const SvxBrushItem* pItem;
                SwRect aOrigBackRect;
                drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

                /// OD 21.08.2002
                ///     consider, that [GetBackgroundBrush(...)] can set <pCol>
                ///     - see implementation in /core/layout/paintfrm.cxx
                /// OD 21.08.2002 #99657#
                ///     There is a user defined setting for the background, if there
                ///     is a background brush and its color is *not* "no fill"/"auto fill".
                if( GetFrame()->GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigBackRect, false, /*bConsiderTextBox=*/true ) )
                {
                    if (aFillAttributes.get() && aFillAttributes->isUsed())
                    {
                        // First see if fill attributes provide a color.
                        aColor = Color(aFillAttributes->getAverageColor(aGlobalRetoucheColor.getBColor()));
                        pCol = &aColor;
                    }

                    // If not, then fall back to the old brush item.
                    if ( !pCol )
                    {
                        pCol = &pItem->GetColor();
                    }

                    /// OD 30.08.2002 #99657#
                    /// determined color <pCol> can be <COL_TRANSPARENT>. Thus, check it.
                    if ( *pCol == COL_TRANSPARENT)
                        pCol = nullptr;
                }
                else
                    pCol = nullptr;
            }

            // no user defined color at paragraph or font background
            if ( ! pCol )
                pCol = &aGlobalRetoucheColor;

            if( GetShell() && GetShell()->GetWin() )
            {
                // here we determine the preferred window text color for painting
                const SwViewOption* pViewOption = GetShell()->GetViewOptions();
                if(pViewOption->IsPagePreview() &&
                        !SW_MOD()->GetAccessibilityOptions().GetIsForPagePreviews())
                    nNewColor = COL_BLACK;
                else
                    // we take the font color from the appearance page
                    nNewColor = SwViewOption::GetFontColor();
            }

            // change painting color depending of dark/bright background
            Color aTmpColor( nNewColor );
            if ( pCol->IsDark() && aTmpColor.IsDark() )
                nNewColor = COL_WHITE;
            else if ( pCol->IsBright() && aTmpColor.IsBright() )
                nNewColor = COL_BLACK;
        }
    }

    if ( bChgFntColor || bChgLineColor )
    {
        Color aNewColor( nNewColor );

        if ( bChgFntColor )
        {
            if ( pFont && aNewColor != pFont->GetColor() )
            {
                // only set the new color at the font passed as argument
                pFont->SetColor( aNewColor );
            }
            else if ( aNewColor != GetOut().GetFont().GetColor() )
            {
                // set new font with new color at output device
                vcl::Font aFont( rFnt );
                aFont.SetColor( aNewColor );
                GetOut().SetFont( aFont );
            }
        }

        // the underline and overline colors have to be set separately
        if ( bChgLineColor )
        {
            // get current font color or color set at output device
            aNewColor = pFont ? pFont->GetColor() : GetOut().GetFont().GetColor();
            if ( aNewColor != GetOut().GetLineColor() )
                GetOut().SetLineColor( aNewColor );
            if ( aNewColor != GetOut().GetOverlineColor() )
                GetOut().SetOverlineColor( aNewColor );
        }

        return true;
    }

    return false;
}

void SwClearFntCacheTextGlyphs()
{
    for (SwFntObj* pFntObj = pFntCache->First(); pFntObj; pFntObj = SwFntCache::Next(pFntObj))
        pFntObj->GetTextGlyphs().clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
