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

#include <sal/config.h>

#include <i18nlangtag/mslangid.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/outdev.hxx>
#include <vcl/kernarray.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/glyphitemcache.hxx>
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
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <tgrditem.hxx>
#include <scriptinfo.hxx>
#include <editeng/brushitem.hxx>
#include <accessibilityoptions.hxx>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <doc.hxx>
#include <editeng/fhgtitem.hxx>
#include <vcl/glyphitem.hxx>
#include <vcl/vcllayout.hxx>
#include <docsh.hxx>
#include <strings.hrc>
#include <fntcap.hxx>
#include <vcl/outdev/ScopedStates.hxx>
#include <o3tl/hash_combine.hxx>
#include <cstdint>
#include <memory>
#include "justify.hxx"

using namespace ::com::sun::star;

// global variables declared in fntcache.hxx
// FontCache is created in txtinit.cxx TextInit_ and deleted in TextFinit
SwFntCache *pFntCache = nullptr;
// last Font set by ChgFntCache
SwFntObj *pLastFont = nullptr;

constexpr Color gWaveCol(COL_GRAY);

tools::Long SwFntObj::s_nPixWidth;
MapMode* SwFntObj::s_pPixMap = nullptr;
static vcl::DeleteOnDeinit< VclPtr<OutputDevice> > s_pFntObjPixOut {};

void SwFntCache::Flush( )
{
    if ( pLastFont )
    {
        pLastFont->Unlock();
        pLastFont = nullptr;
    }
    SwCache::Flush( );
}

SwFntObj::SwFntObj(const SwSubFont &rFont, std::uintptr_t nFontCacheId, SwViewShell const *pSh)
    : SwCacheObj(reinterpret_cast<void *>(nFontCacheId))
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
    , m_nScrHangingBaseline(0)
    , m_nPrtHangingBaseline(0)
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

namespace {

struct CalcLinePosData
{
    SwDrawTextInfo& rInf;
    vcl::Font& rFont;
    TextFrameIndex nCnt;
    const bool bSwitchH2V;
    const bool bSwitchH2VLRBT;
    const bool bSwitchL2R;
    tools::Long nHalfSpace;
    KernArray& rKernArray;
    const bool bBidiPor;

    CalcLinePosData( SwDrawTextInfo& _rInf, vcl::Font& _rFont,
          TextFrameIndex const _nCnt, const bool _bSwitchH2V, const bool _bSwitchH2VLRBT, const bool _bSwitchL2R,
                      tools::Long _nHalfSpace, KernArray& _rKernArray, const bool _bBidiPor) :
        rInf( _rInf ),
        rFont( _rFont ),
        nCnt( _nCnt ),
        bSwitchH2V( _bSwitchH2V ),
        bSwitchH2VLRBT( _bSwitchH2VLRBT ),
        bSwitchL2R( _bSwitchL2R ),
        nHalfSpace( _nHalfSpace ),
        rKernArray( _rKernArray ),
        bBidiPor( _bBidiPor )
    {
    }
};

}

// Computes the start and end position of an underline. This function is called
// from the DrawText-method (for underlining misspelled words or smarttag terms).
static void lcl_calcLinePos( const CalcLinePosData &rData,
    Point &rStart, Point &rEnd, TextFrameIndex const nStart, TextFrameIndex const nWrLen)
{
    tools::Long nBlank = 0;
    const TextFrameIndex nEnd = nStart + nWrLen;
    const tools::Long nTmpSpaceAdd = rData.rInf.GetSpace() / SPACING_PRECISION_FACTOR;

    if ( nEnd < rData.nCnt
       && CH_BLANK == rData.rInf.GetText()[sal_Int32(rData.rInf.GetIdx() + nEnd)] )
    {
        if (nEnd + TextFrameIndex(1) == rData.nCnt)
            nBlank -= nTmpSpaceAdd;
        else
            nBlank -= rData.nHalfSpace;
    }

    // determine start, end and length of wave line
    sal_Int32 nKernStart = nStart ? rData.rKernArray[sal_Int32(nStart) - 1] : 0;
    sal_Int32 nKernEnd = rData.rKernArray[sal_Int32(nEnd) - 1];

    const Degree10 nDir = rData.bBidiPor ? 1800_deg10
                                           : UnMapDirection(rData.rFont.GetOrientation(),
                                                            rData.bSwitchH2V, rData.bSwitchH2VLRBT);

    switch ( nDir.get() )
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

    // tdf#151968
    // if start < end, OutputDevice::DrawWaveLine() will think it is a rotated
    // line, so we swap nStart and nEnd to avoid this.
    if ( rData.bBidiPor )
        std::swap(rStart, rEnd);

    if ( rData.bSwitchL2R )
    {
        rData.rInf.GetFrame()->SwitchLTRtoRTL( rStart );
        rData.rInf.GetFrame()->SwitchLTRtoRTL( rEnd );

        // tdf#151968
        // We need to do this here as well for LTR text in a RTL paragraph.
        std::swap(rStart, rEnd);
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
            m_nPrtAscent = o3tl::narrowing<sal_uInt16>(aOutMet.GetAscent());
            m_nPrtHangingBaseline = o3tl::narrowing<sal_uInt16>(aOutMet.GetHangingBaseline());
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
            m_nPrtHeight = o3tl::narrowing<sal_uInt16>(rRefDev.GetTextHeight());

#if OSL_DEBUG_LEVEL > 0
            // Check if vcl did not change the meaning of GetTextHeight
            const FontMetric aOutMet( rRefDev.GetFontMetric() );
            tools::Long nTmpPrtHeight = o3tl::narrowing<sal_uInt16>(aOutMet.GetAscent()) + aOutMet.GetDescent();
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
            m_nExtLeading = o3tl::narrowing<sal_uInt16>(aMet.GetExternalLeading());
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
            if( m_bSymbol || IsOpenSymbol( m_pPrtFont->GetFamilyName()))
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

sal_uInt16 SwFntObj::GetFontHangingBaseline( const SwViewShell* pSh, const OutputDevice& rOut )
{
    sal_uInt16 nRet = 0;
    const OutputDevice& rRefDev = pSh ? pSh->GetRefDev() : rOut;

    GetFontAscent(pSh, rOut);

    if ( pSh && lcl_IsFontAdjustNecessary( rOut, rRefDev ) )
        nRet = m_nScrHangingBaseline;
    else
        nRet = m_nPrtHangingBaseline;

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
            m_nExtLeading = o3tl::narrowing<sal_uInt16>(aMet.GetExternalLeading());

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

    m_nScrAscent = o3tl::narrowing<sal_uInt16>(pOut->GetFontMetric().GetAscent());
    m_nScrHangingBaseline =  o3tl::narrowing<sal_uInt16>(pOut->GetFontMetric().GetHangingBaseline());
    if ( USHRT_MAX == m_nScrHeight )
        m_nScrHeight = o3tl::narrowing<sal_uInt16>(pOut->GetTextHeight());

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
                         rSh.GetWin()->GetOutDev() :
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
            sal_Int32 nTmpLeading = aWinMet.GetInternalLeading();
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
                sal_Int32 nDiff = std::min( rMet.GetDescent() - aWinMet.GetDescent(),
                    aWinMet.GetAscent() - rMet.GetAscent() - nTmpLeading );
                if( nDiff > 0 )
                {
                    OSL_ENSURE( m_nPrtAscent < USHRT_MAX, "GuessLeading: PrtAscent-Fault" );
                    if ( m_nPrtAscent < USHRT_MAX )
                        m_nPrtAscent = m_nPrtAscent + o3tl::narrowing<sal_uInt16>(( 2 * nDiff ) / 5);
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
static bool lcl_IsMonoSpaceFont( const vcl::RenderContext& rOut )
{
    const tools::Long nWidth1 = rOut.GetTextWidth( OUString( u'\x3008' ) );
    const tools::Long nWidth2 = rOut.GetTextWidth( OUString( u'\x307C' ) );
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

    tools::Long nHght = rInf.GetOut().LogicToPixel( rPrtFontSize ).Height();

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
                const SwViewShell* pShell = rInf.GetShell();
                sal_uInt16 nZoom = pShell ? round(pShell->GetViewOptions()->GetZoom()/100) : 1;
                if (WRONGAREA_WAVE == wrongArea->mLineType)
                {
                    vcl::ScopedAntialiasing a(rInf.GetOut(), true);
                    rInf.GetOut().SetLineColor( wrongArea->mColor );
                    rInf.GetOut().DrawWaveLine( aStart, aEnd, 1 + nZoom, 3 + nZoom );
                }
                else if (WRONGAREA_BOLDWAVE == wrongArea->mLineType)
                {
                    vcl::ScopedAntialiasing a(rInf.GetOut(), true);
                    rInf.GetOut().SetLineColor( wrongArea->mColor );
                    rInf.GetOut().DrawWaveLine( aStart, aEnd, 2 + nZoom, 4 + nZoom  );
                }
                else if (WRONGAREA_BOLD == wrongArea->mLineType)
                {
                    rInf.GetOut().SetLineColor( wrongArea->mColor );

                    aStart.AdjustY(30 );
                    aEnd.AdjustY(30 );

                    LineInfo aLineInfo( LineStyle::Solid, 26 );

                    rInf.GetOut().DrawLine( aStart, aEnd, aLineInfo );
                }
                else if (WRONGAREA_DASHED == wrongArea->mLineType)
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

static void GetTextArray(const OutputDevice& rDevice, const OUString& rStr, KernArray& rDXAry,
                         sal_Int32 nIndex, sal_Int32 nLen, bool bCaret = false,
                         const vcl::text::TextLayoutCache* layoutCache = nullptr)
{
    const SalLayoutGlyphs* pLayoutCache = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(&rDevice, rStr, nIndex, nLen,
        0, layoutCache);
    rDevice.GetTextArray(rStr, &rDXAry, nIndex, nLen, bCaret, layoutCache, pLayoutCache);
}

static void GetTextArray(const OutputDevice& rOutputDevice, const SwDrawTextInfo& rInf, KernArray& rDXAry,
                         bool bCaret = false)
{
    return GetTextArray(rOutputDevice, rInf.GetText(), rDXAry, rInf.GetIdx().get(), rInf.GetLen().get(),
                        bCaret, rInf.GetVclCache());
}

static void GetTextArray(const OutputDevice& rOutputDevice, const SwDrawTextInfo& rInf, KernArray& rDXAry,
                         sal_Int32 nLen, bool bCaret = false)
{
    // Substring is fine.
    assert( nLen <= rInf.GetLen().get());
    return GetTextArray(rOutputDevice, rInf.GetText(), rDXAry, rInf.GetIdx().get(), nLen, bCaret, rInf.GetVclCache());
}

void SwFntObj::DrawText( SwDrawTextInfo &rInf )
{
    OSL_ENSURE( rInf.GetShell(), "SwFntObj::DrawText without shell" );

    OutputDevice& rRefDev = rInf.GetShell()->GetRefDev();
    vcl::Window* pWin = rInf.GetShell()->GetWin();

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
    const vcl::text::ComplexTextLayoutFlags nMode = rInf.GetOut().GetLayoutMode();
    const bool bBidiPor = ( bSwitchL2R !=
                            ( vcl::text::ComplexTextLayoutFlags::Default != ( vcl::text::ComplexTextLayoutFlags::BiDiRtl & nMode ) ) );

    // be sure to have the correct layout mode at the printer
    if ( m_pPrinter )
    {
        m_pPrinter->SetLayoutMode( rInf.GetOut().GetLayoutMode() );
        m_pPrinter->SetDigitLanguage( rInf.GetOut().GetDigitLanguage() );
    }

    Point aTextOriginPos( rInf.GetPos() );
    if( !bPrt )
    {
        if( rInf.GetpOut() != *s_pFntObjPixOut.get() || rInf.GetOut().GetMapMode() != *s_pPixMap )
        {
            *s_pPixMap = rInf.GetOut().GetMapMode();
            (*s_pFntObjPixOut.get()) = rInf.GetpOut();
            Size aTmp( 1, 1 );
            s_nPixWidth = rInf.GetOut().PixelToLogic( aTmp ).Width();
        }

        aTextOriginPos.AdjustX(rInf.GetFrame()->IsRightToLeft() ? 0 : s_nPixWidth );
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

        // ASIAN LINE AND CHARACTER GRID MODE
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() )
        {
            //for textgrid refactor
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const tools::Long nGridWidth = GetGridWidth(*pGrid, *pDoc);
            tools::Long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;

            // kerning array - gives the absolute position of end of each character
            KernArray aKernArray;

            if ( m_pPrinter )
                GetTextArray(*m_pPrinter, rInf, aKernArray);
            else
                GetTextArray(rInf.GetOut(), rInf, aKernArray);

            tools::Long nDelta = 0;

            if (pGrid->IsSnapToChars())
            {
                nDelta = sw::Justify::SnapToGrid(aKernArray, rInf.GetText(), sal_Int32(rInf.GetIdx()),
                                      sal_Int32(rInf.GetLen()), nGridWidth, false);
            }
            else
            {
                sw::Justify::SnapToGridEdge(aKernArray, sal_Int32(rInf.GetLen()), nGridWidth,
                        nSpaceAdd, rInf.GetKern());
            }

            if (nDelta)
                aTextOriginPos.AdjustX(nDelta);

            if ( bSwitchH2V )
                rInf.GetFrame()->SwitchHorizontalToVertical( aTextOriginPos );

            rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                aKernArray, {}, sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));

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
            KernArray aKernArray;
            GetTextArray(rInf.GetOut(), rInf, aKernArray);
            std::vector<sal_Bool> aKashidaArray;

            if( bStretch )
            {
                sal_Int32 nZwi = sal_Int32(rInf.GetLen()) - 1;
                tools::Long nDiff = rInf.GetWidth() - aKernArray[ nZwi ]
                             - sal_Int32(rInf.GetLen()) * rInf.GetKern();
                tools::Long nRest = nDiff % nZwi;
                tools::Long nAdd;
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
                tools::Long nSum = nDiff;
                for( sal_Int32 i = 0; i < nZwi; )
                {
                    aKernArray.adjust(i, nSum);
                    if( ++i == nRest )
                        nDiff += nAdd;
                    nSum += nDiff;
                }
            }

            // Modify Array for special justifications

            tools::Long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
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
                    pSI->Compress( aKernArray, rInf.GetIdx(), rInf.GetLen(),
                                   rInf.GetKanaComp(),
                                   o3tl::narrowing<sal_uInt16>(m_aFont.GetFontSize().Height()), lcl_IsFullstopCentered( rInf.GetOut() ), &aTextOriginPos );
                    bSpecialJust = true;
                }

                // Asian Justification
                if ( SwFontScript::CJK == nActual && nSpaceAdd )
                {
                    LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CJK );

                    if (!MsLangId::isKorean(aLang))
                    {
                        SwScriptInfo::CJKJustify( rInf.GetText(), aKernArray,
                                rInf.GetIdx(), rInf.GetLen(), aLang, nSpaceAdd, rInf.IsSpaceStop() );

                        bSpecialJust = true;
                        nSpaceAdd = 0;
                    }
                }

                // Kashida Justification
                if ( SwFontScript::CTL == nActual && nSpaceAdd )
                {
                    if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
                    {
                        aKashidaArray.resize(aKernArray.size(), false);
                        if ( pSI && pSI->CountKashida() &&
                            pSI->KashidaJustify( &aKernArray, aKashidaArray.data(), rInf.GetIdx(),
                                                 rInf.GetLen(), nSpaceAdd ) != -1 )
                        {
                            bSpecialJust = true;
                            nSpaceAdd = 0;
                        }
                        else
                            aKashidaArray.clear();
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
                        SwScriptInfo::ThaiJustify( rInf.GetText(), &aKernArray,
                                                   rInf.GetIdx(), rInf.GetLen(),
                                                   rInf.GetNumberOfBlanks(),
                                                   rInf.GetSpace() );

                        // adding space to blanks is already done
                        bSpecialJust = true;
                        nSpaceAdd = 0;
                    }
                }
            }

            tools::Long nKernSum = rInf.GetKern();

            if ( bStretch || m_bPaintBlank || rInf.GetKern() || bSpecialJust )
            {
                for (sal_Int32 i = 0; i < sal_Int32(rInf.GetLen()); i++,
                     nKernSum += rInf.GetKern() )
                {
                    if (CH_BLANK == rInf.GetText()[sal_Int32(rInf.GetIdx()) + i])
                        nKernSum += nSpaceAdd;
                    aKernArray.adjust(i, nKernSum);
                }

                // In case of underlined/strike-through justified text
                // a blank at the end requires special handling:
                if( m_bPaintBlank && rInf.GetLen() && ( CH_BLANK ==
                    rInf.GetText()[sal_Int32(rInf.GetIdx() + rInf.GetLen())-1]))
                {
                    // If it is a single underlined space, output 2 spaces:
                    if (TextFrameIndex(1) == rInf.GetLen())
                    {
                        aKernArray.set(0, rInf.GetWidth() + nSpaceAdd);

                        rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                             aKernArray, aKashidaArray, sal_Int32(rInf.GetIdx()), 1 );
                    }
                    else
                    {
                        sal_Int32 nIndex(sal_Int32(rInf.GetLen()) - 2);
                        aKernArray.adjust(nIndex, nSpaceAdd);
                        rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                            aKernArray, aKashidaArray, sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
                    }
                }
                else
                    rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                            aKernArray, aKashidaArray, sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
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
                        SwTwips nAdd = aKernArray[ i ] + nKernSum;
                        if ( ( vcl::text::ComplexTextLayoutFlags::BiDiStrong | vcl::text::ComplexTextLayoutFlags::BiDiRtl ) == nMode )
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
            tools::Long nTmpWidth = rInf.GetWidth();
            if( rInf.GetKern() && rInf.GetLen() && nTmpWidth > rInf.GetKern() )
                nTmpWidth -= rInf.GetKern();
            rInf.GetOut().DrawStretchText( aTextOriginPos, nTmpWidth,
                       rInf.GetText(),
                       sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));
        }
        else if( rInf.GetKern() )
        {
            const tools::Long nTmpWidth = GetTextSize( rInf ).Width();

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
        OUString aBulletOverlay;
        bool bBullet = rInf.GetBullet();
        if( m_bSymbol )
            bBullet = false;
        CreateScrFont( *rInf.GetShell(), rInf.GetOut() );

        VclPtr<OutputDevice> xFormattingDevice;

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
            xFormattingDevice = m_pPrinter;
        }
        else
        {
            xFormattingDevice = &rInf.GetOut();
        }

        //tdf#152094 see if we can retain a subpixel factor
        int nSubPixels = 1;
        MapMode aMapMode(xFormattingDevice->GetMapMode());
        if (aMapMode.IsSimple() && aMapMode.GetMapUnit() == MapUnit::MapTwip)
        {
            if (xFormattingDevice->GetDPIX() == xFormattingDevice->GetDPIY())
            {
                int nRatio = xFormattingDevice->GetDPIX() / 1440;
                if (nRatio * 1440 == xFormattingDevice->GetDPIX())
                    nSubPixels = nRatio;
            }
        }
        KernArray aKernArray(nSubPixels);
        GetTextArray(*xFormattingDevice, rInf, aKernArray);

        std::vector<sal_Bool> aKashidaArray;

        // Modify Printer and ScreenArrays for special justifications

        tools::Long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
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
                pSI->Compress( aKernArray, rInf.GetIdx(), rInf.GetLen(),
                               rInf.GetKanaComp(),
                               o3tl::narrowing<sal_uInt16>(m_aFont.GetFontSize().Height()), lcl_IsFullstopCentered( rInf.GetOut() ), &aTextOriginPos );
            }

            // Asian Justification
            if ( SwFontScript::CJK == nActual && nSpaceAdd )
            {
                LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CJK );

                if (!MsLangId::isKorean(aLang))
                {
                    SwScriptInfo::CJKJustify( rInf.GetText(), aKernArray,
                            rInf.GetIdx(), rInf.GetLen(), aLang, nSpaceAdd, rInf.IsSpaceStop() );

                    nSpaceAdd = 0;
                }
            }

            // Kashida Justification
            if ( SwFontScript::CTL == nActual && nSpaceAdd )
            {
                if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
                {
                    aKashidaArray.resize(aKernArray.size(), false);
                    if ( pSI && pSI->CountKashida() &&
                         pSI->KashidaJustify( &aKernArray, aKashidaArray.data(), rInf.GetIdx(),
                                              rInf.GetLen(), nSpaceAdd ) != -1 )
                        nSpaceAdd = 0;
                    else
                    {
                        aKashidaArray.clear();
                        bNoHalfSpace = true;
                    }
                }
            }

            // Thai Justification
            if ( SwFontScript::CTL == nActual && nSpaceAdd )
            {
                LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CTL );

                if ( LANGUAGE_THAI == aLang )
                {
                    SwScriptInfo::ThaiJustify( rInf.GetText(), &aKernArray,
                                               rInf.GetIdx(),
                                               rInf.GetLen(),
                                               rInf.GetNumberOfBlanks(),
                                               rInf.GetSpace() );

                    // adding space to blanks is already done
                    nSpaceAdd = 0;
                }
            }
        }

        if( bBullet )
        {
            // Copy the substring that will be painted, and replace spaces with
            // bullets, and everything else with space.
            sal_Int32 nCopyStart = sal_Int32(rInf.GetIdx());
            sal_Int32 nCopyLen = sal_Int32(rInf.GetLen());

            aBulletOverlay = rInf.GetText().copy( nCopyStart, nCopyLen );

            for( sal_Int32 i = 0; i < aBulletOverlay.getLength(); ++i )
            {
                const sal_Unicode replaceChar = aBulletOverlay[ i ];
                if( CH_BLANK == replaceChar || CH_EN_SPACE == replaceChar )
                {
                    /* fdo#72488 Hack: try to see if the space is zero width
                     * and don't bother with inserting a bullet in this case.
                     */
                    if ((i + nCopyStart + 1 >= sal_Int32(rInf.GetLen())) ||
                        aKernArray[i + nCopyStart] != aKernArray[ i + nCopyStart + 1])
                    {
                        aBulletOverlay = aBulletOverlay.replaceAt(i, 1, rtl::OUStringChar(CH_BLANK == replaceChar ? CH_BULLET : CH_DEGREE));
                    }
                    else
                    {
                        aBulletOverlay = aBulletOverlay.replaceAt(i, 1, rtl::OUStringChar(replaceChar));
                    }
                }
                else
                {
                    aBulletOverlay = aBulletOverlay.replaceAt(i, 1, rtl::OUStringChar(CH_BLANK));
                }
            }
        }

        TextFrameIndex nCnt(rInf.GetText().getLength());
        if ( nCnt < rInf.GetIdx() )
            assert(false); // layout bug, not handled below
        else
            nCnt = nCnt - rInf.GetIdx();
        nCnt = std::min(nCnt, rInf.GetLen());
        sal_Unicode cChPrev = rInf.GetText()[sal_Int32(rInf.GetIdx())];

        // In case of a single underlined space in justified text,
        // have to output 2 spaces:
        if ((nCnt == TextFrameIndex(1)) && rInf.GetSpace() && (cChPrev == CH_BLANK))
        {
            aKernArray.set(0, rInf.GetWidth() +
                            rInf.GetKern() +
                           (rInf.GetSpace() / SPACING_PRECISION_FACTOR));

            if ( bSwitchL2R )
                rInf.GetFrame()->SwitchLTRtoRTL( aTextOriginPos );

            if ( bSwitchH2V )
                rInf.GetFrame()->SwitchHorizontalToVertical( aTextOriginPos );

            rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(),
                         aKernArray, aKashidaArray, sal_Int32(rInf.GetIdx()), 1 );
            if( bBullet )
                rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(), aKernArray, {},
                                             sal_Int32(rInf.GetIdx()), 1 );
        }
        else
        {
            if (m_pPrtFont->IsWordLineMode())
                bNoHalfSpace = true;

            sw::Justify::SpaceDistribution(aKernArray, rInf.GetText(), sal_Int32(rInf.GetIdx()),
                    sal_Int32(nCnt), nSpaceAdd, rInf.GetKern(), bNoHalfSpace);

            if( rInf.GetGreyWave() )
            {
                if( rInf.GetLen() )
                {
                    tools::Long nHght = rInf.GetOut().LogicToPixel(
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
                        tools::Long nKernVal = aKernArray[sal_Int32(rInf.GetLen()) - 1];

                        const Degree10 nDir = bBidiPor
                                                    ? 1800_deg10
                                                    : UnMapDirection(GetFont().GetOrientation(),
                                                                     bSwitchH2V, bSwitchH2VLRBT);

                        switch ( nDir.get() )
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
                        {
                            vcl::ScopedAntialiasing a(rInf.GetOut(), true);
                            rInf.GetOut().DrawWaveLine( aCurrPos, aEnd );
                        }
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
                    const tools::Long nHalfSpace = bNoHalfSpace ? 0 : nSpaceAdd / 2;
                    CalcLinePosData aCalcLinePosData(rInf, GetFont(), nCnt, bSwitchH2V,
                                                     bSwitchH2VLRBT, bSwitchL2R, nHalfSpace,
                                                     aKernArray, bBidiPor);

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

                sal_Int32 nIdx = sal_Int32(rInf.GetIdx());
                const SalLayoutGlyphs* pGlyphs = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(&rInf.GetOut(),
                     rInf.GetText(), nIdx, nLen);
                rInf.GetOut().DrawTextArray( aTextOriginPos, rInf.GetText(), aKernArray, aKashidaArray,
                                             nIdx, nLen, SalLayoutFlags::NONE, pGlyphs );
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
                    tools::Long nShift = rInf.GetOut( ).GetFontMetric( ).GetBulletOffset( );
                    if ( nShift )
                    {
                        tools::Long nAdd = 0;

                        if ( aBulletOverlay[ 0 ] == CH_BULLET )
                        {
                            if (bSwitchH2V)
                                aTextOriginPos.AdjustY(nShift ) ;
                            else
                                aTextOriginPos.AdjustX(nShift ) ;
                            nAdd = nShift ;
                        }
                        for( sal_Int32 i = 1 ; i < nLen ; ++i )
                        {
                            if ( aBulletOverlay[ i ] == CH_BULLET )
                                aKernArray.adjust(i - 1, nShift);
                            if ( nAdd )
                                aKernArray.adjust(i - 1, -nAdd);
                        }
                    }
                    rInf.GetOut().DrawTextArray( aTextOriginPos, aBulletOverlay, aKernArray,
                                                 {}, 0, nLen );
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

    const TextFrameIndex nMsrLn = (TextFrameIndex(COMPLETE_STRING) != rInf.GetMeasureLen())
        ? rInf.GetMeasureLen()
        : nLn;

    // If the measure length is different from the length, then we are
    // measuring substring width for caret positioning, see SetMeasureLength()
    // use in TextCursor::GetCharRect_().
    bool bCaret(nMsrLn != nLn);

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
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() )
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

            KernArray aKernArray;
            GetTextArray(*pOutDev, rInf, aKernArray, sal_Int32(nLn), bCaret);
            if (pGrid->IsSnapToChars())
            {
                sw::Justify::SnapToGrid(aKernArray, rInf.GetText(), sal_Int32(rInf.GetIdx()),
                                  sal_Int32(rInf.GetLen()), nGridWidth, true);
            }
            else
            {
                // use 0 to calculate raw width without rInf.GetSpace().
                sw::Justify::SnapToGridEdge(aKernArray, sal_Int32(rInf.GetLen()), nGridWidth, 0,
                        rInf.GetKern());
            }

            aTextSize.setWidth(aKernArray[sal_Int32(nMsrLn) - 1]);
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
    KernArray aKernArray;
    if ( m_pPrinter && m_pPrinter.get() != rInf.GetpOut() )
    {
        if( !m_pPrtFont->IsSameInstance( m_pPrinter->GetFont() ) )
            m_pPrinter->SetFont(*m_pPrtFont);
        aTextSize.setHeight( m_pPrinter->GetTextHeight() );

        CreateScrFont( *rInf.GetShell(), rInf.GetOut() );
        if( !GetScrFont()->IsSameInstance( rInf.GetOut().GetFont() ) )
            rInf.GetOut().SetFont( *m_pScrFont );

        GetTextArray(*m_pPrinter, rInf.GetText(), aKernArray,
                     sal_Int32(rInf.GetIdx()), sal_Int32(nLn), bCaret);
    }
    else
    {
        if( !m_pPrtFont->IsSameInstance( rInf.GetOut().GetFont() ) )
            rInf.GetOut().SetFont( *m_pPrtFont );
        aTextSize.setHeight( rInf.GetOut().GetTextHeight() );

        GetTextArray(rInf.GetOut(), rInf, aKernArray, nLn.get(), bCaret);
    }

    if (bCompress)
    {
        rInf.SetKanaDiff(rInf.GetScriptInfo()->Compress(aKernArray, rInf.GetIdx(), nLn, rInf.GetKanaComp(),
            o3tl::narrowing<sal_uInt16>(m_aFont.GetFontSize().Height()), lcl_IsFullstopCentered(rInf.GetOut())));
    }
    else
        rInf.SetKanaDiff( 0 );

    if (nMsrLn)
    {
        aTextSize.setWidth(aKernArray[sal_Int32(nMsrLn) - 1]);

        // Note that we can't simply use sal_Int(nMsrLn) - 1 as nSpaceCount
        // because a glyph may be made up of more than one characters.
        sal_Int32 nSpaceCount = 0;
        tools::Long nOldValue = aKernArray[0];

        for(sal_Int32 i = 1; i < sal_Int32(nMsrLn); ++i)
        {
            if (nOldValue != aKernArray[i])
            {
                ++nSpaceCount;
                nOldValue = aKernArray[i];
            }
        }

        if (rInf.GetKern())
            aTextSize.AdjustWidth(nSpaceCount * rInf.GetKern());
    }

    OSL_ENSURE( !rInf.GetShell() ||
            ( USHRT_MAX != GetGuessedLeading() && USHRT_MAX != GetExternalLeading() ),
              "Leading values should be already calculated" );
    aTextSize.AdjustHeight(GetFontLeading( rInf.GetShell(), rInf.GetOut() ) );
    return aTextSize;
}

TextFrameIndex SwFntObj::GetModelPositionForViewPoint(SwDrawTextInfo &rInf)
{
    tools::Long nSpaceAdd =       rInf.GetSpace() / SPACING_PRECISION_FACTOR;
    const tools::Long nCharacterSpacing = -rInf.GetCharacterSpacing() / SPACING_PRECISION_FACTOR;
    tools::Long nKern = rInf.GetKern();

    if( 0 != nCharacterSpacing )
        nKern -= nCharacterSpacing;

    KernArray aKernArray;

    // be sure to have the correct layout mode at the printer
    if ( m_pPrinter )
    {
        m_pPrinter->SetLayoutMode( rInf.GetOut().GetLayoutMode() );
        m_pPrinter->SetDigitLanguage( rInf.GetOut().GetDigitLanguage() );
        GetTextArray(*m_pPrinter, rInf, aKernArray);
    }
    else
    {
        GetTextArray(rInf.GetOut(), rInf, aKernArray);
    }

    if ( rInf.GetFrame() && rInf.GetLen() && rInf.SnapToGrid() &&
         rInf.GetFont() && SwFontScript::CJK == rInf.GetFont()->GetActual() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() )
        {
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GetGridWidth(*pGrid, *pDoc);

            if (pGrid->IsSnapToChars())
            {
                sw::Justify::SnapToGrid(aKernArray, rInf.GetText(), sal_Int32(rInf.GetIdx()),
                                      sal_Int32(rInf.GetLen()), nGridWidth, true);
            }
            else
            {
                sw::Justify::SnapToGridEdge(aKernArray, sal_Int32(rInf.GetLen()), nGridWidth,
                        nSpaceAdd, rInf.GetKern());
            }

            return  TextFrameIndex(sw::Justify::GetModelPosition(aKernArray, sal_Int32(rInf.GetLen()),
                        rInf.GetOffset()));
        }
    }

    const SwScriptInfo* pSI = rInf.GetScriptInfo();
    if ( rInf.GetFont() && rInf.GetLen() )
    {
        const SwFontScript nActual = rInf.GetFont()->GetActual();

        // Kana Compression
        if ( SwFontScript::CJK == nActual && rInf.GetKanaComp() &&
             pSI && pSI->CountCompChg() &&
             lcl_IsMonoSpaceFont( rInf.GetOut() ) )
        {
            pSI->Compress( aKernArray, rInf.GetIdx(), rInf.GetLen(),
                           rInf.GetKanaComp(),
                           o3tl::narrowing<sal_uInt16>(m_aFont.GetFontSize().Height()),
                           lcl_IsFullstopCentered( rInf.GetOut() ) );
        }

        // Asian Justification
        if ( SwFontScript::CJK == rInf.GetFont()->GetActual() )
        {
            LanguageType aLang = rInf.GetFont()->GetLanguage( SwFontScript::CJK );

            if (!MsLangId::isKorean(aLang))
            {
                SwScriptInfo::CJKJustify( rInf.GetText(), aKernArray,
                        rInf.GetIdx(), rInf.GetLen(), aLang, nSpaceAdd, rInf.IsSpaceStop() );

                nSpaceAdd = 0;
            }

        }

        // Kashida Justification
        if ( SwFontScript::CTL == nActual && rInf.GetSpace() )
        {
            if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
            {
                if ( pSI && pSI->CountKashida() &&
                    pSI->KashidaJustify( &aKernArray, nullptr, rInf.GetIdx(), rInf.GetLen(),
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
                SwScriptInfo::ThaiJustify( rInf.GetText(), &aKernArray,
                                           rInf.GetIdx(), rInf.GetLen(),
                                           rInf.GetNumberOfBlanks(),
                                           rInf.GetSpace() );

                // adding space to blanks is already done
                nSpaceAdd = 0;
            }
        }
    }

    tools::Long nLeft = 0;
    tools::Long nRight = 0;
    TextFrameIndex nCnt(0);
    tools::Long nSpaceSum = 0;
    tools::Long nKernSum = 0;

    sal_Int32 nDone = 0;
    TextFrameIndex nIdx = rInf.GetIdx();
    TextFrameIndex nLastIdx = nIdx;
    const TextFrameIndex nEnd = rInf.GetIdx() + rInf.GetLen();

    // #i105901#
    // skip character cells for all script types
    LanguageType aLang = rInf.GetFont()->GetLanguage();

    while ( ( nRight < tools::Long( rInf.GetOffset() ) ) && ( nIdx < nEnd ) )
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

        // the next character might be outside the layout range (e.g tdf124116-1.odt)
        if (nIdx > nEnd)
            nIdx = nEnd;

        nLeft = nRight;
        nRight = aKernArray[sal_Int32(nIdx - rInf.GetIdx()) - 1] + nKernSum + nSpaceSum;

        nKernSum += nKern;
    }

    // step back if position is before the middle of the character
    // or if we do not want to go to the next character
    if ( nIdx > rInf.GetIdx() &&
         ( rInf.IsPosMatchesBounds() ||
           ( ( nRight > tools::Long( rInf.GetOffset() ) ) &&
             ( nRight - rInf.GetOffset() > rInf.GetOffset() - nLeft ) ) ) )
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
    if ( m_pObj )
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
    // "MagicNumber" used to identify Fonts
    static std::uintptr_t fontCacheIdCounter = 0;
    // a new Font, a new "MagicNumber".
    return new SwFntObj( *static_cast<SwSubFont const *>(m_pOwner), ++fontCacheIdCounter, m_pShell );
}

TextFrameIndex SwFont::GetTextBreak(SwDrawTextInfo const & rInf, tools::Long nTextWidth)
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
    tools::Long nKern = 0;

    TextFrameIndex nLn = rInf.GetLen() == TextFrameIndex(COMPLETE_STRING)
        ? TextFrameIndex(rInf.GetText().getLength()) : rInf.GetLen();

    if ( rInf.GetFrame() && nLn && rInf.SnapToGrid() &&
         rInf.GetFont() && SwFontScript::CJK == rInf.GetFont()->GetActual() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetFrame()->FindPageFrame()));
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() )
        {
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GetGridWidth(*pGrid, *pDoc);

            KernArray aKernArray;
            GetTextArray( rInf.GetOut(), rInf.GetText(), aKernArray,
                    sal_Int32(rInf.GetIdx()), sal_Int32(rInf.GetLen()));

            if (pGrid->IsSnapToChars())
            {
                sw::Justify::SnapToGrid(aKernArray, rInf.GetText(), sal_Int32(rInf.GetIdx()),
                                  sal_Int32(rInf.GetLen()), nGridWidth, true);
            }
            else
            {
                // use 0 to calculate raw width without rInf.GetSpace().
                sw::Justify::SnapToGridEdge(aKernArray, sal_Int32(rInf.GetLen()), nGridWidth,
                        0, rInf.GetKern());
            }

            while(nTextBreak < rInf.GetLen() && aKernArray[sal_Int32(nTextBreak)] <= nTextWidth)
                ++nTextBreak;

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
                    aTmpText = aTmpText.replaceAt( 0, aSnippetTmp.getLength(), rtl::OUStringChar(aSnippet[0]) );
                }
            }

            pTmpText = &aTmpText;
            nTmpIdx = TextFrameIndex(0);
            nTmpLen = TextFrameIndex(aTmpText.getLength());
            bTextReplaced = true;
        }

        if( rInf.GetHyphPos() ) {
            sal_Int32 nHyphPos = sal_Int32(*rInf.GetHyphPos());
            const SalLayoutGlyphs* pGlyphs = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(
                &rInf.GetOut(), *pTmpText, nTmpIdx.get(), nTmpLen.get(), 0, rInf.GetVclCache());
            nTextBreak = TextFrameIndex(rInf.GetOut().GetTextBreak(
                             *pTmpText, nTextWidth,
                             u'-', nHyphPos,
                             sal_Int32(nTmpIdx), sal_Int32(nTmpLen),
                             nKern, rInf.GetVclCache(), pGlyphs));
            *rInf.GetHyphPos() = TextFrameIndex((nHyphPos == -1) ? COMPLETE_STRING : nHyphPos);
        }
        else
        {
            SwFntAccess aFntAccess(m_aSub[m_nActual].m_nFontCacheId, m_aSub[m_nActual].m_nFontIndex,
                                   &m_aSub[m_nActual], rInf.GetShell());
            const SalLayoutGlyphs* pGlyphs = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(&rInf.GetOut(),
                *pTmpText, nTmpIdx.get(), nTmpLen.get(), 0, rInf.GetVclCache());
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

    // tdf112290 tdf136588 Break the line correctly only if there is an image inline,
    // and the image wider than the line...
    if (GetCaseMap() == SvxCaseMap::SmallCaps && TextFrameIndex(COMPLETE_STRING) == nTextBreak2 &&
        ! bCompress && nTextWidth == 0)
        // If nTextWidth == 0 means the line is full, we have to break it
        nTextBreak2 = TextFrameIndex(1);

    if ( ! bCompress )
        return nTextBreak2;

    nTextBreak2 = nTextBreak2 - rInf.GetIdx();

    if( nTextBreak2 < nLn )
    {
        if( !nTextBreak2 && nLn )
            nLn = TextFrameIndex(1);
        else if (nLn > nTextBreak2 + nTextBreak2)
            nLn = nTextBreak2 + nTextBreak2;
        KernArray aKernArray;
        GetTextArray( rInf.GetOut(), rInf.GetText(), aKernArray,
                                    sal_Int32(rInf.GetIdx()), sal_Int32(nLn));
        if( rInf.GetScriptInfo()->Compress( aKernArray, rInf.GetIdx(), nLn,
                            rInf.GetKanaComp(), o3tl::narrowing<sal_uInt16>(GetHeight( m_nActual )),
                            lcl_IsFullstopCentered( rInf.GetOut() ) ) )
        {
            tools::Long nKernAdd = nKern;
            TextFrameIndex const nTmpBreak = nTextBreak2;
            if( nKern && nTextBreak2 )
                nKern *= sal_Int32(nTextBreak2) - 1;
            while (nTextBreak2 < nLn && nTextWidth >= aKernArray[sal_Int32(nTextBreak2)] + nKern)
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

    const SwViewShell *pVSh = GetShell();
    const bool bOutputToWindow(pVSh && (pVSh->GetWin() || pVSh->isOutputToWindow()));

    if (pVSh && !bOutputToWindow && pVSh->GetViewOptions()->IsBlackFont())
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

        bChgLineColor = pVSh && bOutputToWindow &&
                pVSh->GetAccessibilityOptions()->IsAlwaysAutoColor();

        bChgFntColor = COL_AUTO == rFnt.GetColor() || bChgLineColor;

        if ( bChgFntColor )
        {
            // check if current background has a user defined setting
            std::optional<Color> pCol;
            if (GetFont())
                pCol = GetFont()->GetBackColor();
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
                    if (aFillAttributes && aFillAttributes->isUsed())
                    {
                        // First see if fill attributes provide a color.
                        pCol = Color(aFillAttributes->getAverageColor(aGlobalRetoucheColor.getBColor()));
                    }

                    // If not, then fall back to the old brush item.
                    if ( !pCol )
                    {
                        pCol = pItem->GetColor();
                    }

                    /// OD 30.08.2002 #99657#
                    /// determined color <pCol> can be <COL_TRANSPARENT>. Thus, check it.
                    if ( *pCol == COL_TRANSPARENT)
                        pCol.reset();
                }
                else
                    pCol.reset();
            }

            // no user defined color at paragraph or font background
            if ( ! pCol )
                pCol = aGlobalRetoucheColor;

            if (pVSh && bOutputToWindow)
            {
                // here we determine the preferred window text color for painting
                const SwViewOption* pViewOption = pVSh->GetViewOptions();
                if(pViewOption->IsPagePreview() &&
                        !officecfg::Office::Common::Accessibility::IsForPagePreviews::get())
                    nNewColor = COL_BLACK;
                else
                    // we take the font color from the appearance page
                    nNewColor = pViewOption->GetFontColor();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
