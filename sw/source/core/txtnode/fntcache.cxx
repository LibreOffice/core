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

#include <i18nlangtag/mslangid.hxx>
#include <vcl/outdev.hxx>
#include <vcl/print.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metric.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <breakit.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <fntcache.hxx>
#include <IDocumentSettingAccess.hxx>
#include <swfont.hxx>       // CH_BLANK + CH_BULLET
#include <wrong.hxx>
#include "dbg_lay.hxx"
#include <txtfrm.hxx>       // SwTxtFrm
#include <pagefrm.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <scriptinfo.hxx>
#include <editeng/brushitem.hxx>
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include <accessibilityoptions.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <doc.hxx>
#include <editeng/fhgtitem.hxx>
#include <docsh.hxx>
#include <poolfmt.hrc>

using namespace ::com::sun::star;

// global variables declared in fntcache.hxx
// FontCache is created in txtinit.cxx _TextInit and deleted in _TextFinit
SwFntCache *pFntCache = NULL;
// last Font set by ChgFntCache
SwFntObj *pLastFont = NULL;
// "MagicNumber" used to identify Fonts
sal_uInt8* pMagicNo = NULL;

Color *pWaveCol = 0;

long SwFntObj::nPixWidth;
MapMode* SwFntObj::pPixMap = NULL;
OutputDevice* SwFntObj::pPixOut = NULL;

extern sal_uInt16 UnMapDirection( sal_uInt16 nDir, const sal_Bool bVertFormat );
sal_uInt16 GetDefaultFontHeight( SwDrawTextInfo &rInf )
{
    SwDocShell* pDocShell = rInf.GetShell()->GetDoc()->GetDocShell();
    SfxStyleSheetBasePool* pBasePool = pDocShell->GetStyleSheetPool();

    OUString sString(SW_RESSTR(STR_POOLCOLL_STANDARD));

    SfxStyleSheetBase* pStyle = pBasePool->Find(sString, (SfxStyleFamily)SFX_STYLE_FAMILY_PARA);
    SfxItemSet& aTmpSet = pStyle->GetItemSet();
    SvxFontHeightItem &aDefaultFontItem = (SvxFontHeightItem&)aTmpSet.Get(RES_CHRATR_CJK_FONTSIZE);
    return (sal_uInt16)aDefaultFontItem.GetHeight();
}



/*************************************************************************
|*
|*  SwFntCache::Flush()
|*
|*************************************************************************/

void SwFntCache::Flush( )
{
    if ( pLastFont )
    {
        pLastFont->Unlock();
        pLastFont = NULL;
    }
    SwCache::Flush( );
}

/*************************************************************************
|*
|*  SwFntObj::SwFntObj(), ~SwFntObj()
|*
|*************************************************************************/

SwFntObj::SwFntObj( const SwSubFont &rFont, const void *pOwn, ViewShell *pSh ) :
    SwCacheObj( (void*)pOwn ),
    aFont( rFont ),
    pScrFont( NULL ),
    pPrtFont( &aFont ),
    pPrinter( NULL ),
    nPropWidth( rFont.GetPropWidth() )
{
    nZoom = pSh ? pSh->GetViewOptions()->GetZoom() : USHRT_MAX;
    nGuessedLeading = USHRT_MAX;
    nExtLeading = USHRT_MAX;
    nPrtAscent = USHRT_MAX;
    nPrtHeight = USHRT_MAX;
    bSymbol = RTL_TEXTENCODING_SYMBOL == aFont.GetCharSet();
    bPaintBlank = ( UNDERLINE_NONE != aFont.GetUnderline()
                 || UNDERLINE_NONE != aFont.GetOverline()
                 || STRIKEOUT_NONE != aFont.GetStrikeout() )
                 && !aFont.IsWordLineMode();
    aFont.SetLanguage(rFont.GetLanguage());
}

SwFntObj::~SwFntObj()
{
    if ( pScrFont != pPrtFont )
        delete pScrFont;
    if ( pPrtFont != &aFont )
        delete pPrtFont;
}

void SwFntObj::CreatePrtFont( const OutputDevice& rPrt )
{
    if ( nPropWidth != 100 && pPrinter != &rPrt )
    {
        if( pScrFont != pPrtFont )
            delete pScrFont;
        if( pPrtFont != &aFont )
            delete pPrtFont;

        const Font aOldFnt( rPrt.GetFont() );
        ((OutputDevice&)rPrt).SetFont( aFont );
        const FontMetric aWinMet( rPrt.GetFontMetric() );
        ((OutputDevice&)rPrt).SetFont( aOldFnt );
        long nWidth = ( aWinMet.GetSize().Width() * nPropWidth ) / 100;

        if( !nWidth )
            ++nWidth;
        pPrtFont = new Font( aFont );
        pPrtFont->SetSize( Size( nWidth, aFont.GetSize().Height() ) );
        pScrFont = NULL;
    }
}

/*************************************************************************
 *
 *  bool lcl_IsFontAdjustNecessary( rOutDev, rRefDev )
 *
 *  returns whether we have to adjust the output font to resemble
 *  the formatting font
 *
 *  _Not_ necessary if
 *
 *  1. RefDef == OutDev (text formatting, online layout...)
 *  2. PDF export from online layout
 *  3. Prospect/PagePreview pringing
 *
 *************************************************************************/

static bool lcl_IsFontAdjustNecessary( const OutputDevice& rOutDev,
                                const OutputDevice& rRefDev )
{
    return &rRefDev != &rOutDev &&
           OUTDEV_WINDOW != rRefDev.GetOutDevType() &&
           ( OUTDEV_PRINTER != rRefDev.GetOutDevType() ||
             OUTDEV_PRINTER != rOutDev.GetOutDevType() );
}

struct CalcLinePosData
{
    SwDrawTextInfo& rInf;
    Font& rFont;
    xub_StrLen nCnt;
    const sal_Bool bSwitchH2V;
    const sal_Bool bSwitchL2R;
    long nHalfSpace;
    sal_Int32* pKernArray;
    const sal_Bool bBidiPor;

    CalcLinePosData( SwDrawTextInfo& _rInf, Font& _rFont,
                      xub_StrLen _nCnt, const sal_Bool _bSwitchH2V, const sal_Bool _bSwitchL2R,
                      long _nHalfSpace, sal_Int32* _pKernArray, const sal_Bool _bBidiPor) :
        rInf( _rInf ),
        rFont( _rFont ),
        nCnt( _nCnt ),
        bSwitchH2V( _bSwitchH2V ),
        bSwitchL2R( _bSwitchL2R ),
        nHalfSpace( _nHalfSpace ),
        pKernArray( _pKernArray ),
        bBidiPor( _bBidiPor )
    {
    }
};

/** Function: lcl_calcLinePos

   Computes the start and end position of an underline. This function is called
   from the DrawText-method (for underlining misspelled words or smarttag terms).
*/

static void lcl_calcLinePos( const CalcLinePosData &rData,
    Point &rStart, Point &rEnd, xub_StrLen nStart, xub_StrLen nWrLen )
{
   long nBlank = 0;
   const xub_StrLen nEnd = nStart + nWrLen;
   const long nTmpSpaceAdd = rData.rInf.GetSpace() / SPACING_PRECISION_FACTOR;

   if ( nEnd < rData.nCnt
       && CH_BLANK == rData.rInf.GetText()[ rData.rInf.GetIdx() + nEnd ] )
   {
       if( nEnd + 1 == rData.nCnt )
           nBlank -= nTmpSpaceAdd;
       else
           nBlank -= rData.nHalfSpace;
   }

   // determine start, end and length of wave line
   sal_Int32 nKernStart = nStart ? rData.pKernArray[ sal_uInt16( nStart - 1 ) ] : 0;
   sal_Int32 nKernEnd = rData.pKernArray[ sal_uInt16( nEnd - 1 ) ];

   sal_uInt16 nDir = rData.bBidiPor ? 1800 :
       UnMapDirection( rData.rFont.GetOrientation(), rData.bSwitchH2V );

   switch ( nDir )
   {
   case 0 :
       rStart.X() += nKernStart;
       rEnd.X() = nBlank + rData.rInf.GetPos().X() + nKernEnd;
       rEnd.Y() = rData.rInf.GetPos().Y();
       break;
   case 900 :
       rStart.Y() -= nKernStart;
       rEnd.X() = rData.rInf.GetPos().X();
       rEnd.Y() = nBlank + rData.rInf.GetPos().Y() - nKernEnd;
       break;
   case 1800 :
       rStart.X() -= nKernStart;
       rEnd.X() = rData.rInf.GetPos().X() - nKernEnd - nBlank;
       rEnd.Y() = rData.rInf.GetPos().Y();
       break;
   case 2700 :
       rStart.Y() += nKernStart;
       rEnd.X() = rData.rInf.GetPos().X();
       rEnd.Y() = nBlank + rData.rInf.GetPos().Y() + nKernEnd;
       break;
   }

   if ( rData.bSwitchL2R )
   {
       rData.rInf.GetFrm()->SwitchLTRtoRTL( rStart );
       rData.rInf.GetFrm()->SwitchLTRtoRTL( rEnd );
   }

   if ( rData.bSwitchH2V )
   {
       rData.rInf.GetFrm()->SwitchHorizontalToVertical( rStart );
       rData.rInf.GetFrm()->SwitchHorizontalToVertical( rEnd );
   }
}

/*************************************************************************
 *
 *  sal_uInt16 SwFntObj::GetFontAscent( const OutputDevice& rOut )
 *
 * Returns the Ascent of the Font on the given output device;
 * it may be necessary to create the screen font first.
 *************************************************************************/

sal_uInt16 SwFntObj::GetFontAscent( const ViewShell *pSh, const OutputDevice& rOut )
{
    sal_uInt16 nRet = 0;
    const OutputDevice& rRefDev = pSh ? pSh->GetRefDev() : rOut;

    if ( pSh && lcl_IsFontAdjustNecessary( rOut, rRefDev ) )
    {
        CreateScrFont( *pSh, rOut );
        OSL_ENSURE( USHRT_MAX != nScrAscent, "nScrAscent is going berzerk" );
        nRet = nScrAscent;
    }
    else
    {
        if (nPrtAscent == USHRT_MAX) // printer ascent unknown?
        {
            CreatePrtFont( rOut );
            const Font aOldFnt( rRefDev.GetFont() );
            ((OutputDevice&)rRefDev).SetFont( *pPrtFont );
            const FontMetric aOutMet( rRefDev.GetFontMetric() );
            nPrtAscent = (sal_uInt16) aOutMet.GetAscent();
            ( (OutputDevice&)rRefDev).SetFont( aOldFnt );
        }

        nRet = nPrtAscent;
    }

#if !defined(MACOSX) // #i89844# extleading is below the line for Mac
    // TODO: move extleading below the line for all platforms too
    nRet += GetFontLeading( pSh, rRefDev );
#endif

    OSL_ENSURE( USHRT_MAX != nRet, "GetFontAscent returned USHRT_MAX" );
    return nRet;
}

/*************************************************************************
 *
 *  sal_uInt16 SwFntObj::GetFontHeight( const OutputDevice* pOut )
 *
 * Returns the height of the Font on the given output device;
 * it may be necessary to create the screen font first.
 *************************************************************************/

sal_uInt16 SwFntObj::GetFontHeight( const ViewShell* pSh, const OutputDevice& rOut )
{
    sal_uInt16 nRet = 0;
    const OutputDevice& rRefDev = pSh ? pSh->GetRefDev() : rOut;

    if ( pSh && lcl_IsFontAdjustNecessary( rOut, rRefDev ) )
    {
        CreateScrFont( *pSh, rOut );
        OSL_ENSURE( USHRT_MAX != nScrHeight, "nScrHeight is going berzerk" );
        nRet = nScrHeight + GetFontLeading( pSh, rRefDev );
    }
    else
    {
        if (nPrtHeight == USHRT_MAX) // printer height unknown?
        {
            CreatePrtFont( rOut );
            const Font aOldFnt( rRefDev.GetFont() );
            ((OutputDevice&)rRefDev).SetFont( *pPrtFont );
            nPrtHeight = static_cast<sal_uInt16>(rRefDev.GetTextHeight());

#if OSL_DEBUG_LEVEL > 0
            // Check if vcl did not change the meading of GetTextHeight
            const FontMetric aOutMet( rRefDev.GetFontMetric() );
            long nTmpPrtHeight = (sal_uInt16)aOutMet.GetAscent() + aOutMet.GetDescent();
            (void) nTmpPrtHeight;
            // #i106098#: do not compare with == here due to rounding error
            OSL_ENSURE( abs(nTmpPrtHeight - nPrtHeight) < 3,
                    "GetTextHeight != Ascent + Descent" );
#endif

            ((OutputDevice&)rRefDev).SetFont( aOldFnt );
        }

        nRet = nPrtHeight + GetFontLeading( pSh, rRefDev );
    }

    OSL_ENSURE( USHRT_MAX != nRet, "GetFontHeight returned USHRT_MAX" );
    return nRet;
}

sal_uInt16 SwFntObj::GetFontLeading( const ViewShell *pSh, const OutputDevice& rOut )
{
    sal_uInt16 nRet = 0;

    if ( pSh )
    {
        if ( USHRT_MAX == nGuessedLeading || USHRT_MAX == nExtLeading )
        {
            SolarMutexGuard aGuard;

            const Font aOldFnt( rOut.GetFont() );
            ((OutputDevice&)rOut).SetFont( *pPrtFont );
            const FontMetric aMet( rOut.GetFontMetric() );
            ((OutputDevice&)rOut).SetFont( aOldFnt );
            bSymbol = RTL_TEXTENCODING_SYMBOL == aMet.GetCharSet();
            GuessLeading( *pSh, aMet );
            nExtLeading = static_cast<sal_uInt16>(aMet.GetExtLeading());
            /* HACK: There is something wrong with Writer's bullet rendering, causing lines
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
            if( bSymbol || IsStarSymbol( pPrtFont->GetName()))
                nExtLeading = 0;
        }

        const IDocumentSettingAccess& rIDSA = *pSh->getIDocumentSettingAccess();
        const bool bBrowse = ( pSh->GetWin() &&
                               pSh->GetViewOptions()->getBrowseMode() &&
                              !pSh->GetViewOptions()->IsPrtFormat() );

        if ( !bBrowse && rIDSA.get(IDocumentSettingAccess::ADD_EXT_LEADING) )
            nRet = nExtLeading;
        else
            nRet = nGuessedLeading;
    }

    OSL_ENSURE( USHRT_MAX != nRet, "GetFontLeading returned USHRT_MAX" );
    return nRet;
}


/*************************************************************************
 *
 *  SwFntObj::CreateScrFont( const ViewShell& rSh, const OutputDevice& rOut )
 *
 *  pOut is the output device, not the reference device
 *
 *************************************************************************/

void SwFntObj::CreateScrFont( const ViewShell& rSh, const OutputDevice& rOut )
{
    if ( pScrFont )
        return;

    // any changes to the output device are reset at the end of the function
    OutputDevice* pOut = (OutputDevice*)&rOut;

    // Save old font
    Font aOldOutFont( pOut->GetFont() );

    nScrHeight = USHRT_MAX;

    // Condition for output font / refdev font adjustment
    OutputDevice* pPrt = &rSh.GetRefDev();

    if( !rSh.GetWin() ||
        !rSh.GetViewOptions()->getBrowseMode() ||
         rSh.GetViewOptions()->IsPrtFormat() )
    {
        // After CreatePrtFont pPrtFont is the font which is actually used
        // by the reference device
        CreatePrtFont( *pPrt );
        pPrinter = pPrt;

        // save old reference device font
        Font aOldPrtFnt( pPrt->GetFont() );

        // set the font used at the reference device at the reference device
        // and the output device
        pPrt->SetFont( *pPrtFont );
        pOut->SetFont( *pPrtFont );

        // This should be the default for pScrFont.
        pScrFont = pPrtFont;

        FontMetric aMet = pPrt->GetFontMetric( );
        //Don't loose "faked" properties of the logical font that don't truly
        //exist in the physical font metrics which vcl which fake up for us
        aMet.SetWeight(pScrFont->GetWeight());
        aMet.SetItalic(pScrFont->GetItalic());

        bSymbol = RTL_TEXTENCODING_SYMBOL == aMet.GetCharSet();

        if ( USHRT_MAX == nGuessedLeading )
            GuessLeading( rSh, aMet );

        if ( USHRT_MAX == nExtLeading )
            nExtLeading = static_cast<sal_uInt16>(aMet.GetExtLeading());

        // reset the original reference device font
        pPrt->SetFont( aOldPrtFnt );
    }
    else
    {
        bSymbol = RTL_TEXTENCODING_SYMBOL == aFont.GetCharSet();
        if ( nGuessedLeading == USHRT_MAX )
            nGuessedLeading = 0;

        // no external leading in browse mode
        if ( nExtLeading == USHRT_MAX )
            nExtLeading = 0;

        pScrFont = pPrtFont;
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
            nTmp = ( 100 * pOut->GetMapMode().GetScaleX().GetNumerator() ) /
                     pOut->GetMapMode().GetScaleX().GetDenominator();
        }
        else
            nTmp = 0;
        if( nTmp != nZoom )
            nZoom = USHRT_MAX - 1;
    }

    nScrAscent = (sal_uInt16)pOut->GetFontMetric().GetAscent();
    if ( USHRT_MAX == nScrHeight )
        nScrHeight = (sal_uInt16)pOut->GetTextHeight();

    // reset original output device font
    pOut->SetFont( aOldOutFont );
}


void SwFntObj::GuessLeading( const ViewShell&
#if defined(WNT)
                             rSh
#endif
                             , const FontMetric& rMet )
{
    // If leading >= 5, this seems to be enough leading.
    // Nothing has to be done.
    if ( rMet.GetIntLeading() >= 5 )
    {
        nGuessedLeading = 0;
        return;
    }

#if defined(WNT)
    OutputDevice *pWin = rSh.GetWin() ?
                         rSh.GetWin() :
                         GetpApp()->GetDefaultDevice();
    if ( pWin )
    {
        MapMode aTmpMap( MAP_TWIP );
        MapMode aOldMap = pWin->GetMapMode( );
        pWin->SetMapMode( aTmpMap );
        const Font aOldFnt( pWin->GetFont() );
        pWin->SetFont( *pPrtFont );
        const FontMetric aWinMet( pWin->GetFontMetric() );
        const sal_uInt16 nWinHeight = sal_uInt16( aWinMet.GetSize().Height() );
        if( pPrtFont->GetName().indexOf( aWinMet.GetName() ) != -1 )
        {
            // If the Leading on the Window is also 0, then it has to stay
            // that way (see also StarMath).
            long nTmpLeading = (long)aWinMet.GetIntLeading();
            if( nTmpLeading <= 0 )
            {
                pWin->SetFont( rMet );
                nTmpLeading = (long)pWin->GetFontMetric().GetIntLeading();
                if( nTmpLeading < 0 )
                    nGuessedLeading = 0;
                else
                    nGuessedLeading = sal_uInt16(nTmpLeading);
            }
            else
            {
                nGuessedLeading = sal_uInt16(nTmpLeading);
                // Manta-Hack #50153#:
                // Wer beim Leading luegt, luegt moeglicherweise auch beim
                // Ascent/Descent, deshalb wird hier ggf. der Font ein wenig
                // tiefergelegt, ohne dabei seine Hoehe zu aendern.
                // (above original comment preserved for cultural reasons)
                // Those who lie about their Leading, may lie about their
                // Ascent/Descent as well, hence the Font will be lowered a
                // litte without changing its height.
                long nDiff = std::min( rMet.GetDescent() - aWinMet.GetDescent(),
                    aWinMet.GetAscent() - rMet.GetAscent() - nTmpLeading );
                if( nDiff > 0 )
                {
                    OSL_ENSURE( nPrtAscent < USHRT_MAX, "GuessLeading: PrtAscent-Fault" );
                    if ( nPrtAscent < USHRT_MAX )
                        nPrtAscent = nPrtAscent + (sal_uInt16)(( 2 * nDiff ) / 5);
                }
            }
        }
        else
        {
            // If all else fails, take 15% of the height, as emprically
            // determined by CL
            nGuessedLeading = (nWinHeight * 15) / 100;
        }
        pWin->SetFont( aOldFnt );
        pWin->SetMapMode( aOldMap );
    }
    else
#endif
        nGuessedLeading = 0;
}

/*************************************************************************
 *
 *  void SwFntObj::SetDeviceFont( const OutputDevice *pOut ),
 *
 * Set the font at the given output device; for screens it may be
 * necessary to do some adjustment first.
 *
 *************************************************************************/

void SwFntObj::SetDevFont( const ViewShell *pSh, OutputDevice& rOut )
{
    const OutputDevice& rRefDev = pSh ? pSh->GetRefDev() : rOut;

    if ( pSh && lcl_IsFontAdjustNecessary( rOut, rRefDev ) )
    {
        CreateScrFont( *pSh, rOut );
        if( !GetScrFont()->IsSameInstance( rOut.GetFont() ) )
            rOut.SetFont( *pScrFont );
        if( pPrinter && ( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) ) )
            pPrinter->SetFont( *pPrtFont );
    }
    else
    {
        CreatePrtFont( rOut );
        if( !pPrtFont->IsSameInstance( rOut.GetFont() ) )
            rOut.SetFont( *pPrtFont );
    }

    // Here, we actually do not need the leading values, but by calling
    // GetFontLeading() we assure that the values are calculated for later use.
    GetFontLeading( pSh, rRefDev );
}

#define WRONG_SHOW_MIN 5
#define WRONG_SHOW_SMALL 11
#define WRONG_SHOW_MEDIUM 15

/*************************************************************************
 *
 * void SwFntObj::DrawText( ... )
 *
 * Output text:
 *      on screen              => DrawTextArray
 *      on printer, !Kerning   => DrawText
 *      on printer + Kerning   => DrawStretchText
 *
 *************************************************************************/

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

static bool lcl_IsMonoSpaceFont( const OutputDevice& rOut )
{
    const OUString aStr1( sal_Unicode( 0x3008 ) );
    const OUString aStr2( sal_Unicode( 0x307C ) );
    const long nWidth1 = rOut.GetTextWidth( aStr1 );
    const long nWidth2 = rOut.GetTextWidth( aStr2 );
    return nWidth1 == nWidth2;
}

/* This helper structure (SwForbidden) contains the already marked parts of the string
    to avoid double lines (e.g grammar + spell check error) */

typedef std::vector< std::pair< xub_StrLen, xub_StrLen > > SwForbidden;

static void lcl_DrawLineForWrongListData(
    SwForbidden &rForbidden,
    const SwDrawTextInfo    &rInf,
    const SwWrongList       *pWList,
    const CalcLinePosData   &rCalcLinePosData,
    const Size              &rPrtFontSize )
{
    if (!pWList) return;

    xub_StrLen nStart = rInf.GetIdx();
    xub_StrLen nWrLen = rInf.GetLen();

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
        nStart = nStart - rInf.GetIdx();

        const xub_StrLen nEnd = nStart + nWrLen;
        xub_StrLen nNext = nStart;
        while( nNext < nEnd )
        {
            while( pIter != rForbidden.end() && pIter->second <= nNext )
                ++pIter;

            xub_StrLen nNextStart = nNext;
            xub_StrLen nNextEnd = nEnd;

            if( pIter == rForbidden.end() || nNextEnd <= pIter->first )
            {
                // No overlapping mark up found
                std::pair< xub_StrLen, xub_StrLen > aNew;
                aNew.first = nNextStart;
                aNew.second = nNextEnd;
                rForbidden.insert( pIter, aNew );
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


            sal_uInt16 wrongPos = pWList->GetWrongPos(nNextStart + rInf.GetIdx());

            const SwWrongArea* wrongArea = pWList->GetElement(wrongPos);

            if (wrongArea != 0)
            {
                if (WRONGAREA_DASHED == wrongArea->mLineType)
                {
                    rInf.GetOut().SetLineColor( wrongArea->mColor );

                    aStart.Y() +=30;
                    aEnd.Y() +=30;

                    LineInfo aLineInfo( LINE_DASH );
                    aLineInfo.SetDistance( 40 );
                    aLineInfo.SetDashLen( 1 );
                    aLineInfo.SetDashCount(1);

                    rInf.GetOut().DrawLine( aStart, aEnd, aLineInfo );
                }
                else if (WRONGAREA_WAVE == wrongArea->mLineType)
                {
                    rInf.GetOut().SetLineColor( wrongArea->mColor );

                    // get wavy line type to use
                    sal_uInt16 nWave =
                        WRONG_SHOW_MEDIUM < nHght ? WAVE_NORMAL :
                        ( WRONG_SHOW_SMALL < nHght ? WAVE_SMALL : WAVE_FLAT );

                    rInf.GetOut().DrawWaveLine( aStart, aEnd, nWave );
                }
                else if (WRONGAREA_WAVE_NORMAL == wrongArea->mLineType)
                {
                    rInf.GetOut().SetLineColor( wrongArea->mColor );

                    rInf.GetOut().DrawWaveLine( aStart, aEnd, WAVE_NORMAL);
                }

                else if (WRONGAREA_WAVE_SMALL == wrongArea->mLineType)
                {
                    rInf.GetOut().SetLineColor( wrongArea->mColor );

                    rInf.GetOut().DrawWaveLine( aStart, aEnd, WAVE_SMALL);
                }
                else if (WRONGAREA_WAVE_FLAT == wrongArea->mLineType)
                {
                    rInf.GetOut().SetLineColor( wrongArea->mColor );

                    rInf.GetOut().DrawWaveLine( aStart, aEnd, WAVE_FLAT);
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

    Font* pTmpFont = bUseScrFont ? pScrFont : pPrtFont;

    //
    //  bDirectPrint and bUseScrFont should have these values:
    //
    //  Outdev / RefDef  | Printer | VirtPrinter | Window
    // ----------------------------------------------------
    //  Printer          | 1 - 0   | 0 - 1       | -
    // ----------------------------------------------------
    //  VirtPrinter/PDF  | 0 - 1   | 0 - 1       | -
    // ----------------------------------------------------
    //  Window/VirtWindow| 0 - 1   | 0 - 1       | 1 - 0
    //
    // Exception: During painting of a Writer OLE object, we do not have
    // a window. Therefore bUseSrcFont is always 0 in this case.
    //

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
        else if ( OUTDEV_VIRDEV == rRefDev.GetOutDevType() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else
        {
            OSL_FAIL( "Outdev Check failed" );
        }
    }
    else if ( OUTDEV_VIRDEV == rInf.GetOut().GetOutDevType() && ! pWin )
    {
        // PDF export
        if ( OUTDEV_PRINTER == rRefDev.GetOutDevType() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else if ( OUTDEV_VIRDEV == rRefDev.GetOutDevType() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else
        {
            OSL_FAIL( "Outdev Check failed" );
        }
    }
    else if ( OUTDEV_WINDOW == rInf.GetOut().GetOutDevType() ||
               ( OUTDEV_VIRDEV == rInf.GetOut().GetOutDevType() && pWin ) )
    {
        // Window or virtual window
        if ( OUTDEV_PRINTER == rRefDev.GetOutDevType() )
        {
            OSL_ENSURE( !bNoAdjust && bUseScrFont, "Outdev Check failed" );
        }
        else if ( OUTDEV_VIRDEV == rRefDev.GetOutDevType() )
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
        pTmpFont = pPrtFont;

    // HACK: UNDERLINE_WAVE must not be abused any more, hence the grey wave
    // line of the ExtendedAttributeSets will appear in the font color first

    const sal_Bool bSwitchH2V = rInf.GetFrm() && rInf.GetFrm()->IsVertical();
    const sal_Bool bSwitchL2R = rInf.GetFrm() && rInf.GetFrm()->IsRightToLeft() &&
                            ! rInf.IsIgnoreFrmRTL();
    const sal_uLong nMode = rInf.GetOut().GetLayoutMode();
    const sal_Bool bBidiPor = ( bSwitchL2R !=
                            ( 0 != ( TEXT_LAYOUT_BIDI_RTL & nMode ) ) );

    // be sure to have the correct layout mode at the printer
    if ( pPrinter )
    {
        pPrinter->SetLayoutMode( rInf.GetOut().GetLayoutMode() );
        pPrinter->SetDigitLanguage( rInf.GetOut().GetDigitLanguage() );
    }

    Point aPos( rInf.GetPos() );
    if( !bPrt )
    {
        if( rInf.GetpOut() != pPixOut || rInf.GetOut().GetMapMode() != *pPixMap )
        {
            *pPixMap = rInf.GetOut().GetMapMode();
            pPixOut = rInf.GetpOut();
            Size aTmp( 1, 1 );
            nPixWidth = rInf.GetOut().PixelToLogic( aTmp ).Width();
        }

        aPos.X() += rInf.GetFrm()->IsRightToLeft() ? 0 : nPixWidth;
    }

    Color aOldColor( pTmpFont->GetColor() );
    sal_Bool bChgColor = rInf.ApplyAutoColor( pTmpFont );
    if( !pTmpFont->IsSameInstance( rInf.GetOut().GetFont() ) )
        rInf.GetOut().SetFont( *pTmpFont );
    if ( bChgColor )
        pTmpFont->SetColor( aOldColor );

    if ( STRING_LEN == rInf.GetLen() )
        rInf.SetLen( rInf.GetText().getLength() );


    //
    // ASIAN LINE AND CHARACTER GRID MODE START: snap to characters
    //

    if ( rInf.GetFrm() && rInf.SnapToGrid() && rInf.GetFont() &&
         SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars())
        {
            //for textgrid refactor
            //const sal_uInt16 nGridWidth = pGrid->GetBaseHeight();
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GETGRIDWIDTH(pGrid, pDoc);
            sal_Int32* pKernArray = new sal_Int32[rInf.GetLen()];

            if ( pPrinter )
                pPrinter->GetTextArray( rInf.GetText(), pKernArray,
                                        rInf.GetIdx(), rInf.GetLen() );
            else
                rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                            rInf.GetIdx(), rInf.GetLen() );

            long nWidthPerChar = pKernArray[ rInf.GetLen() - 1 ] / rInf.GetLen();

            const sal_uLong i = nWidthPerChar ?
                                ( nWidthPerChar - 1 ) / nGridWidth + 1:
                                1;

            nWidthPerChar = i * nGridWidth;

            // position of first character, we take the printer position
            long nCharWidth = pKernArray[ 0 ];
            sal_uLong nHalfWidth = nWidthPerChar / 2;

            long nNextFix;

            // punctuation characters are not centered
            sal_Unicode cChar = rInf.GetText()[ rInf.GetIdx() ];
            sal_uInt8 nType = lcl_WhichPunctuation( cChar );
            switch ( nType )
            {
            case SwScriptInfo::NONE :
                aPos.X() += ( nWidthPerChar - nCharWidth ) / 2;
                nNextFix = nCharWidth / 2;
                break;
            case SwScriptInfo::SPECIAL_RIGHT :
                nNextFix = nHalfWidth;
                break;
            default:
                aPos.X() += nWidthPerChar - nCharWidth;
                nNextFix = nCharWidth - nHalfWidth;
            }

            // calculate offsets
            for ( xub_StrLen j = 1; j < rInf.GetLen(); ++j )
            {
                long nScr = pKernArray[ j ] - pKernArray[ j - 1 ];
                nNextFix += nWidthPerChar;

                // punctuation characters are not centered
                cChar = rInf.GetText()[ rInf.GetIdx() + j ];
                nType = lcl_WhichPunctuation( cChar );
                switch ( nType )
                {
                case SwScriptInfo::NONE :
                    pKernArray[ j - 1 ] = nNextFix - ( nScr / 2 );
                    break;
                case SwScriptInfo::SPECIAL_RIGHT :
                    pKernArray[ j - 1 ] = nNextFix - nHalfWidth;
                    break;
                default:
                    pKernArray[ j - 1 ] = nNextFix + nHalfWidth - nScr;
                }
            }

            // the layout engine requires the total width of the output
            pKernArray[ rInf.GetLen() - 1 ] = rInf.GetWidth() -
                                              aPos.X() + rInf.GetPos().X() ;

            if ( bSwitchH2V )
                rInf.GetFrm()->SwitchHorizontalToVertical( aPos );

            rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                pKernArray, rInf.GetIdx(), rInf.GetLen() );

            delete[] pKernArray;
            return;
        }
    }

    // For text grid refactor
    // ASIAN LINE AND CHARACTER GRID MODE START: not snap to characters
    //
    if ( rInf.GetFrm() && rInf.SnapToGrid() && rInf.GetFont() &&
         SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )

        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && !pGrid->IsSnapToChars() )
        {
            const sal_uInt16  nDefaultFontHeight = GetDefaultFontHeight( rInf );

            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            long nGridWidthAdd = GETGRIDWIDTH(pGrid, pDoc);
            if( SW_LATIN == rInf.GetFont()->GetActual() )
                nGridWidthAdd = ( nGridWidthAdd - nDefaultFontHeight ) / 2;
            else
                nGridWidthAdd = nGridWidthAdd - nDefaultFontHeight;

            sal_Int32*  pKernArray = new sal_Int32[rInf.GetLen()];

            if ( pPrinter )
                pPrinter->GetTextArray( rInf.GetText(), pKernArray,
                rInf.GetIdx(), rInf.GetLen() );
            else
                rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                rInf.GetIdx(), rInf.GetLen() );
            if ( bSwitchH2V )
                rInf.GetFrm()->SwitchHorizontalToVertical( aPos );
            if ( rInf.GetSpace() || rInf.GetKanaComp())
            {
                long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
                if ( rInf.GetFont() && rInf.GetLen() )
                {
                    bool bSpecialJust = false;
                    const SwScriptInfo* pSI = rInf.GetScriptInfo();
                    const sal_uInt8 nActual = rInf.GetFont()->GetActual();
                    ///Kana Compression
                    if( SW_CJK == nActual && rInf.GetKanaComp() &&
                        pSI && pSI->CountCompChg() &&
                        lcl_IsMonoSpaceFont( *(rInf.GetpOut()) ) )
                    {
                        pSI->Compress( pKernArray,rInf.GetIdx(), rInf.GetLen(),
                            rInf.GetKanaComp(), (sal_uInt16)aFont.GetSize().Height(),&aPos );
                        bSpecialJust = true;
                    }
                    ///Asian Justification
                    if ( ( SW_CJK == nActual || SW_LATIN == nActual ) && nSpaceAdd )
                    {
                        LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CJK );
                        if (!MsLangId::isKorean(aLang))
                        {
                            long nSpaceSum = nSpaceAdd;
                            for ( sal_uInt16 nI = 0; nI < rInf.GetLen(); ++nI )
                            {
                                pKernArray[ nI ] += nSpaceSum;
                                nSpaceSum += nSpaceAdd;
                            }
                            bSpecialJust = true;
                            nSpaceAdd = 0;
                        }
                    }
                    long nGridAddSum = nGridWidthAdd;
                    for(xub_StrLen i = 0; i < rInf.GetLen(); i++,nGridAddSum += nGridWidthAdd )
                    {
                        pKernArray[i] += nGridAddSum;
                    }
                    long nKernSum = rInf.GetKern();
                    if ( bSpecialJust || rInf.GetKern() )
                    {
                        for( xub_StrLen i = 0; i < rInf.GetLen(); i++, nKernSum += rInf.GetKern() )
                        {
                            if ( CH_BLANK == rInf.GetText()[ rInf.GetIdx()+i ] )
                                nKernSum += nSpaceAdd;
                            pKernArray[i] += nKernSum;
                        }
                        ///With through/uderstr. Grouped style requires a blank at the end
                        ///of a text edition special measures:
                        if( bPaintBlank && rInf.GetLen() && (CH_BLANK ==
                            rInf.GetText()[ rInf.GetIdx() + rInf.GetLen() - 1 ] ) )
                        {
                            ///If it concerns a singular, underlined space acts,
                            ///we must spend two:
                            if( 1 == rInf.GetLen() )
                            {
                                pKernArray[0] = rInf.GetWidth() + nSpaceAdd;
                                rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                                    pKernArray, rInf.GetIdx(), 1 );
                            }
                            else
                            {
                                pKernArray[ rInf.GetLen() - 2] += nSpaceAdd;
                                rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                                    pKernArray, rInf.GetIdx(), rInf.GetLen() );
                            }
                        }
                        else
                        {
                            rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                                pKernArray, rInf.GetIdx(), rInf.GetLen() );
                        }
                    }
                    else
                    {
                        Point aTmpPos( aPos );
                        xub_StrLen i;
                        xub_StrLen j = 0;
                        long nSpaceSum = 0;
                        for( i = 0; i < rInf.GetLen(); i++ )
                        {
                            if( CH_BLANK == rInf.GetText()[ rInf.GetIdx() + i ] )
                            {
                                nSpaceSum += nSpaceAdd;
                                if( j < i)
                                    rInf.GetOut().DrawText( aTmpPos, rInf.GetText(),
                                    rInf.GetIdx() + j, i - j );
                                j = i + 1;
                                pKernArray[i] = pKernArray[i] + nSpaceSum;
                                aTmpPos.X() = aPos.X() + pKernArray[ i ] + nKernSum;
                            }
                        }
                        if( j < i )
                            rInf.GetOut().DrawText( aTmpPos, rInf.GetText(),
                            rInf.GetIdx() +j , i - j );
                    }
                }
            }
            else
            {
                //long nKernAdd = rInf.GetKern();
        long nKernAdd = 0;
                long nGridAddSum = nGridWidthAdd + nKernAdd;
                for(xub_StrLen i = 0; i < rInf.GetLen(); i++,nGridAddSum += nGridWidthAdd + nKernAdd )
                {
                    pKernArray[i] += nGridAddSum;
                }
                rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                    pKernArray, rInf.GetIdx(), rInf.GetLen() );
            }
            delete[] pKernArray;
            return;
        }
    }

    //
    // DIRECT PAINTING WITHOUT SCREEN ADJUSTMENT
    //

    if ( bDirectPrint )
    {
        const Fraction aTmp( 1, 1 );
        bool bStretch = rInf.GetWidth() && ( rInf.GetLen() > 1 ) && bPrt
                        && ( aTmp != rInf.GetOut().GetMapMode().GetScaleX() );

        if ( bSwitchL2R )
            rInf.GetFrm()->SwitchLTRtoRTL( aPos );

        if ( bSwitchH2V )
            rInf.GetFrm()->SwitchHorizontalToVertical( aPos );

        // In the good old days we used to have a simple DrawText if the
        // output device is the printer. Now we need a DrawTextArray if
        // 1. KanaCompression is enabled
        // 2. Justified alignment
        // Simple kerning is handled by DrawStretchText
        if( rInf.GetSpace() || rInf.GetKanaComp() )
        {
            sal_Int32 *pKernArray = new sal_Int32[ rInf.GetLen() ];
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                       rInf.GetIdx(), rInf.GetLen() );

            if( bStretch )
            {
                xub_StrLen nZwi = rInf.GetLen() - 1;
                long nDiff = rInf.GetWidth() - pKernArray[ nZwi ]
                             - rInf.GetLen() * rInf.GetKern();
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
                for( xub_StrLen i = 0; i < nZwi; )
                {
                    pKernArray[ i ] += nSum;
                    if( ++i == nRest )
                        nDiff += nAdd;
                    nSum += nDiff;
                }
            }

            //
            // Modify Array for special justifications
            //
            long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
            bool bSpecialJust = false;

            if ( rInf.GetFont() && rInf.GetLen() )
            {
                const SwScriptInfo* pSI = rInf.GetScriptInfo();
                const sal_uInt8 nActual = rInf.GetFont()->GetActual();

                // Kana Compression
                if ( SW_CJK == nActual && rInf.GetKanaComp() &&
                     pSI && pSI->CountCompChg() &&
                     lcl_IsMonoSpaceFont( rInf.GetOut() ) )
                {
                    pSI->Compress( pKernArray, rInf.GetIdx(), rInf.GetLen(),
                                   rInf.GetKanaComp(),
                                   (sal_uInt16)aFont.GetSize().Height(), &aPos );
                    bSpecialJust = true;
                }

                // Asian Justification
                if ( SW_CJK == nActual && nSpaceAdd )
                {
                    LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CJK );

                    if (!MsLangId::isKorean(aLang))
                    {
                        long nSpaceSum = nSpaceAdd;
                        for ( sal_uInt16 nI = 0; nI < rInf.GetLen(); ++nI )
                        {
                            pKernArray[ nI ] += nSpaceSum;
                            nSpaceSum += nSpaceAdd;
                        }

                        bSpecialJust = true;
                        nSpaceAdd = 0;
                    }
                }

                // Kashida Justification
                if ( SW_CTL == nActual && nSpaceAdd )
                {
                    if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
                    {
                        if ( pSI && pSI->CountKashida() &&
                            pSI->KashidaJustify( pKernArray, 0, rInf.GetIdx(),
                                                 rInf.GetLen(), nSpaceAdd ) != STRING_LEN )
                        {
                            bSpecialJust = true;
                            nSpaceAdd = 0;
                        }
                    }
                }

                // Thai Justification
                if ( SW_CTL == nActual && nSpaceAdd )
                {
                    LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CTL );

                    if ( LANGUAGE_THAI == aLang )
                    {
                        // Use rInf.GetSpace() because it has more precision than
                        // nSpaceAdd:
                        SwScriptInfo::ThaiJustify( rInf.GetText(), pKernArray, 0,
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

            if ( bStretch || bPaintBlank || rInf.GetKern() || bSpecialJust )
            {
                for( xub_StrLen i = 0; i < rInf.GetLen(); i++,
                     nKernSum += rInf.GetKern() )
                {
                    if ( CH_BLANK == rInf.GetText()[ rInf.GetIdx()+i ] )
                        nKernSum += nSpaceAdd;
                    pKernArray[i] += nKernSum;
                }

                // In case of underlined/strike-through justified text
                // a blank at the end requires special handling:
                if( bPaintBlank && rInf.GetLen() && ( CH_BLANK ==
                    rInf.GetText()[ rInf.GetIdx()+rInf.GetLen()-1 ] ) )
                {
                    // If it is a single underlined space, output 2 spaces:
                    if( 1 == rInf.GetLen() )
                    {
                           pKernArray[0] = rInf.GetWidth() + nSpaceAdd;

                        rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                                                     pKernArray, rInf.GetIdx(), 1 );
                    }
                    else
                    {
                        pKernArray[ rInf.GetLen() - 2 ] += nSpaceAdd;
                        rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                            pKernArray, rInf.GetIdx(), rInf.GetLen() );
                    }
                }
                else
                    rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                                                 pKernArray, rInf.GetIdx(), rInf.GetLen() );
            }
            else
            {
                Point aTmpPos( aPos );
                xub_StrLen j = 0;
                xub_StrLen i;
                for( i = 0; i < rInf.GetLen(); i++ )
                {
                    if( CH_BLANK == rInf.GetText()[ rInf.GetIdx()+i ] )
                    {
                        nKernSum += nSpaceAdd;
                        if( j < i )
                            rInf.GetOut().DrawText( aTmpPos, rInf.GetText(),
                                                rInf.GetIdx() + j, i - j );
                        j = i + 1;
                        SwTwips nAdd = pKernArray[ i ] + nKernSum;
                        if ( ( TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_BIDI_RTL ) == nMode )
                            nAdd *= -1;
                        aTmpPos.X() = aPos.X() + nAdd;
                    }
                }
                if( j < i )
                    rInf.GetOut().DrawText( aTmpPos, rInf.GetText(),
                                            rInf.GetIdx() + j, i - j );
            }
            delete[] pKernArray;
        }
        else if( bStretch )
        {
            long nTmpWidth = rInf.GetWidth();
            if( rInf.GetKern() && rInf.GetLen() && nTmpWidth > rInf.GetKern() )
                nTmpWidth -= rInf.GetKern();
            rInf.GetOut().DrawStretchText( aPos, nTmpWidth,
                                           rInf.GetText(), rInf.GetIdx(), rInf.GetLen() );
        }
        else if( rInf.GetKern() )
        {
            const long nTmpWidth = GetTextSize( rInf ).Width();

            const Color aSaveColor( pTmpFont->GetColor() );
            const sal_Bool bColorChanged = rInf.ApplyAutoColor( pTmpFont );

            if( bColorChanged )
            {
                if( !pTmpFont->IsSameInstance( rInf.GetOut().GetFont() ) )
                    rInf.GetOut().SetFont( *pTmpFont );
                pTmpFont->SetColor( aSaveColor );
            }

            rInf.GetOut().DrawStretchText( aPos, (sal_uInt16)nTmpWidth,
                                           rInf.GetText(), rInf.GetIdx(), rInf.GetLen() );
        }
        else
            rInf.GetOut().DrawText( aPos, rInf.GetText(),
                                    rInf.GetIdx(), rInf.GetLen() );
    }

    //
    // PAINTING WITH FORMATTING DEVICE/SCREEN ADJUSTMENT
    //

    else
    {
        const OUString* pStr = &rInf.GetText();
        OUString aStr;
        sal_Bool bBullet = rInf.GetBullet();
        if( bSymbol )
            bBullet = sal_False;
        sal_Int32 *pKernArray = new sal_Int32[ rInf.GetLen() ];
        CreateScrFont( *rInf.GetShell(), rInf.GetOut() );
        long nScrPos;

        // get screen array
        sal_Int32* pScrArray = new sal_Int32[ rInf.GetLen() ];
        rInf.GetOut().GetTextArray( rInf.GetText(), pScrArray,
                                    rInf.GetIdx(), rInf.GetLen() );

        // OLE: no printer available
        // OSL_ENSURE( pPrinter, "DrawText needs pPrinter" )
        if ( pPrinter )
        {
            // pTmpFont has already been set as current font for rInf.GetOut()
            if ( pPrinter != rInf.GetpOut() || pTmpFont != pPrtFont )
            {
                if( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) )
                    pPrinter->SetFont( *pPrtFont );
            }
            pPrinter->GetTextArray( rInf.GetText(), pKernArray, rInf.GetIdx(),
                                    rInf.GetLen() );
        }
        else
        {
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                        rInf.GetIdx(), rInf.GetLen() );
        }

        //
        // Modify Printer and ScreenArrays for special justifications
        //
        long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
        bool bNoHalfSpace = false;

        if ( rInf.GetFont() && rInf.GetLen() )
        {
            const sal_uInt8 nActual = rInf.GetFont()->GetActual();
            const SwScriptInfo* pSI = rInf.GetScriptInfo();

            // Kana Compression
            if ( SW_CJK == nActual && rInf.GetKanaComp() &&
                 pSI && pSI->CountCompChg() &&
                 lcl_IsMonoSpaceFont( rInf.GetOut() ) )
            {
                Point aTmpPos( aPos );
                pSI->Compress( pScrArray, rInf.GetIdx(), rInf.GetLen(),
                               rInf.GetKanaComp(),
                               (sal_uInt16)aFont.GetSize().Height(), &aTmpPos );
                pSI->Compress( pKernArray, rInf.GetIdx(), rInf.GetLen(),
                               rInf.GetKanaComp(),
                               (sal_uInt16)aFont.GetSize().Height(), &aPos );
            }

            // Asian Justification
            if ( SW_CJK == nActual && nSpaceAdd )
            {
                LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CJK );

                if (!MsLangId::isKorean(aLang))
                {
                    long nSpaceSum = nSpaceAdd;
                    for ( sal_uInt16 nI = 0; nI < rInf.GetLen(); ++nI )
                    {
                        pKernArray[ nI ] += nSpaceSum;
                        pScrArray[ nI ] += nSpaceSum;
                        nSpaceSum += nSpaceAdd;
                    }

                    nSpaceAdd = 0;
                }
            }

            // Kashida Justification
            if ( SW_CTL == nActual && nSpaceAdd )
            {
                if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
                {
                    if ( pSI && pSI->CountKashida() &&
                         pSI->KashidaJustify( pKernArray, pScrArray, rInf.GetIdx(),
                                              rInf.GetLen(), nSpaceAdd ) != STRING_LEN )
                        nSpaceAdd = 0;
                    else
                        bNoHalfSpace = true;
                }
            }

            // Thai Justification
            if ( SW_CTL == nActual && nSpaceAdd )
            {
                LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CTL );

                if ( LANGUAGE_THAI == aLang )
                {
                    SwScriptInfo::ThaiJustify( rInf.GetText(), pKernArray,
                                               pScrArray, rInf.GetIdx(),
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
            xub_StrLen nCopyStart = rInf.GetIdx();
            if ( nCopyStart )
                --nCopyStart;

            xub_StrLen nCopyLen = rInf.GetLen();
            if ( nCopyStart + nCopyLen < rInf.GetText().getLength() )
                ++nCopyLen;

            aStr = rInf.GetText().copy( nCopyStart, nCopyLen );
            pStr = &aStr;

            for( sal_Int32 i = 0; i < aStr.getLength(); ++i )
                if( CH_BLANK == aStr[ i ] )
                    aStr = aStr.replaceAt(i, 1, OUString(CH_BULLET));
        }

        xub_StrLen nCnt = rInf.GetText().getLength();
        if ( nCnt < rInf.GetIdx() )
            nCnt = 0;
        else
            nCnt = nCnt - rInf.GetIdx();
        nCnt = std::min( nCnt, rInf.GetLen() );
        long nKernSum = rInf.GetKern();
        sal_Unicode cChPrev = rInf.GetText()[ rInf.GetIdx() ];

        // In case of a single underlined space in justified text,
        // have to output 2 spaces:
        if ( ( nCnt == 1 ) && rInf.GetSpace() && ( cChPrev == CH_BLANK ) )
        {
            pKernArray[0] = rInf.GetWidth() +
                            rInf.GetKern() +
                          ( rInf.GetSpace() / SPACING_PRECISION_FACTOR );

            if ( bSwitchL2R )
                rInf.GetFrm()->SwitchLTRtoRTL( aPos );

            if ( bSwitchH2V )
                rInf.GetFrm()->SwitchHorizontalToVertical( aPos );

            rInf.GetOut().DrawTextArray( aPos, rInf.GetText(),
                                         pKernArray, rInf.GetIdx(), 1 );
            if( bBullet )
                rInf.GetOut().DrawTextArray( aPos, *pStr, pKernArray,
                                             rInf.GetIdx() ? 1 : 0, 1 );
        }
        else
        {
            sal_Unicode nCh;

            // In case of Pair Kerning the printer influence on the positioning
            // grows
            sal_uInt16 nMul = 3;

            if ( pPrtFont->GetKerning() )
                nMul = 1;

            const sal_uInt16 nDiv = nMul+1;

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
            const long nHalfSpace = pPrtFont->IsWordLineMode() || bNoHalfSpace ? 0 : nSpaceAdd / 2;
            const long nOtherHalf = nSpaceAdd - nHalfSpace;
            if ( nSpaceAdd && ( cChPrev == CH_BLANK ) )
                nSpaceSum = nHalfSpace;
            for ( xub_StrLen i=1; i<nCnt; ++i,nKernSum += rInf.GetKern() )
            {
                nCh = rInf.GetText()[ rInf.GetIdx() + i ];

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
                    if ( i + 1 == nCnt )
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
                if ( (bNoHalfSpace || pPrtFont->IsWordLineMode()) && i+1 == nCnt && nCh == CH_BLANK )
                    pKernArray[i-1] = pKernArray[i-1] - nSpaceAdd;
            }

            // the layout engine requires the total width of the output
            pKernArray[ rInf.GetLen() - 1 ] += nKernSum + nSpaceSum;

            if( rInf.GetGreyWave() )
            {
                if( rInf.GetLen() )
                {
                    long nHght = rInf.GetOut().LogicToPixel(
                                    pPrtFont->GetSize() ).Height();
                    if( WRONG_SHOW_MIN < nHght )
                    {
                        if ( rInf.GetOut().GetConnectMetaFile() )
                            rInf.GetOut().Push();

                        sal_uInt16 nWave =
                            WRONG_SHOW_MEDIUM < nHght ? WAVE_NORMAL :
                            ( WRONG_SHOW_SMALL < nHght ? WAVE_SMALL :
                            WAVE_FLAT );
                        Color aCol( rInf.GetOut().GetLineColor() );
                        bool bColSave = aCol != *pWaveCol;
                        if ( bColSave )
                            rInf.GetOut().SetLineColor( *pWaveCol );

                        Point aEnd;
                        long nKernVal = pKernArray[ sal_uInt16( rInf.GetLen() - 1 ) ];

                        sal_uInt16 nDir = bBidiPor ?
                                        1800 :
                                        UnMapDirection(
                                            GetFont()->GetOrientation(),
                                            bSwitchH2V );

                        switch ( nDir )
                        {
                        case 0 :
                            aEnd.X() = rInf.GetPos().X() + nKernVal;
                            aEnd.Y() = rInf.GetPos().Y();
                            break;
                        case 900 :
                            aEnd.X() = rInf.GetPos().X();
                            aEnd.Y() = rInf.GetPos().Y() - nKernVal;
                            break;
                        case 1800 :
                            aEnd.X() = rInf.GetPos().X() - nKernVal;
                            aEnd.Y() = rInf.GetPos().Y();
                            break;
                        case 2700 :
                            aEnd.X() = rInf.GetPos().X();
                            aEnd.Y() = rInf.GetPos().Y() + nKernVal;
                            break;
                        }

                        Point aCurrPos( rInf.GetPos() );

                        if ( bSwitchL2R )
                        {
                            rInf.GetFrm()->SwitchLTRtoRTL( aCurrPos );
                            rInf.GetFrm()->SwitchLTRtoRTL( aEnd );
                        }

                        if ( bSwitchH2V )
                        {
                            rInf.GetFrm()->SwitchHorizontalToVertical( aCurrPos );
                            rInf.GetFrm()->SwitchHorizontalToVertical( aEnd );
                        }
                        rInf.GetOut().DrawWaveLine( aCurrPos, aEnd, nWave );

                        if ( bColSave )
                            rInf.GetOut().SetLineColor( aCol );

                        if ( rInf.GetOut().GetConnectMetaFile() )
                            rInf.GetOut().Pop();
                    }
                }
            }
            else if( !bSymbol && rInf.GetLen() )
            {
                // anything to do?
                if (rInf.GetWrong() || rInf.GetGrammarCheck() || rInf.GetSmartTags())
                {
                    CalcLinePosData aCalcLinePosData(rInf, *GetFont(),
                            nCnt, bSwitchH2V, bSwitchL2R,
                            nHalfSpace, pKernArray, bBidiPor);

                    SwForbidden aForbidden;
                    // draw line for smart tag data
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetSmartTags(), aCalcLinePosData, Size() );
                    // draw wave line for spell check errors
                    // draw them BEFORE the grammar check lines to 'override' the latter in case of conflict.
                    // reason: some grammar errors can only be found if spelling errors are fixed,
                    // therefore we don't want the user to miss a spelling error.
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetWrong(), aCalcLinePosData, pPrtFont->GetSize() );
                    // draw wave line for grammar check errors
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetGrammarCheck(), aCalcLinePosData, pPrtFont->GetSize() );
                }
            }

            xub_StrLen nOffs = 0;
            xub_StrLen nLen = rInf.GetLen();

            if( nOffs < nLen )
            {
                // If we paint bullets instead of spaces, we use a copy of
                // the paragraph string. For the layout engine, the copy
                // of the string has to be an environment of the range which
                // is painted
                xub_StrLen nTmpIdx = bBullet ?
                                              ( rInf.GetIdx() ? 1 : 0 ) :
                                              rInf.GetIdx();

                if ( bSwitchL2R )
                    rInf.GetFrm()->SwitchLTRtoRTL( aPos );

                if ( bSwitchH2V )
                    rInf.GetFrm()->SwitchHorizontalToVertical( aPos );

                rInf.GetOut().DrawTextArray( aPos, *pStr, pKernArray + nOffs,
                                             nTmpIdx + nOffs , nLen - nOffs );
            }
        }
        delete[] pScrArray;
        delete[] pKernArray;
    }
}

Size SwFntObj::GetTextSize( SwDrawTextInfo& rInf )
{
    Size aTxtSize;
    const xub_StrLen nLn = ( STRING_LEN != rInf.GetLen() ) ? rInf.GetLen() :
                           rInf.GetText().getLength();

    // be sure to have the correct layout mode at the printer
    if ( pPrinter )
    {
        pPrinter->SetLayoutMode( rInf.GetOut().GetLayoutMode() );
        pPrinter->SetDigitLanguage( rInf.GetOut().GetDigitLanguage() );
    }

    if ( rInf.GetFrm() && nLn && rInf.SnapToGrid() && rInf.GetFont() &&
         SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars() )
        {
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GETGRIDWIDTH(pGrid, pDoc);

            OutputDevice* pOutDev;

            if ( pPrinter )
            {
                if( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) )
                    pPrinter->SetFont(*pPrtFont);
                pOutDev = pPrinter;
            }
            else
                pOutDev = rInf.GetpOut();

            aTxtSize.Width() =
                    pOutDev->GetTextWidth( rInf.GetText(), rInf.GetIdx(), nLn );

            OSL_ENSURE( !rInf.GetShell() ||
                    ( USHRT_MAX != GetGuessedLeading() && USHRT_MAX != GetExtLeading() ),
                "Leading values should be already calculated" );
            aTxtSize.Height() = pOutDev->GetTextHeight() +
                                GetFontLeading( rInf.GetShell(), rInf.GetOut() );

            long nWidthPerChar = aTxtSize.Width() / nLn;

            const sal_uLong i = nWidthPerChar ?
                            ( nWidthPerChar - 1 ) / nGridWidth + 1:
                            1;

            aTxtSize.Width() = i * nGridWidth * nLn;
            rInf.SetKanaDiff( 0 );
            return aTxtSize;
        }
    }

    //for textgrid refactor
    if ( rInf.GetFrm() && nLn && rInf.SnapToGrid() && rInf.GetFont() &&
         SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && !pGrid->IsSnapToChars() )
        {
            const sal_uInt16 nDefaultFontHeight = GetDefaultFontHeight( rInf );

            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            long nGridWidthAdd = GETGRIDWIDTH(pGrid, pDoc);
            if( SW_LATIN == rInf.GetFont()->GetActual() )
                nGridWidthAdd = ( nGridWidthAdd - nDefaultFontHeight ) / 2;
            else
                nGridWidthAdd = nGridWidthAdd - nDefaultFontHeight;
            OutputDevice* pOutDev;
            if ( pPrinter )
            {
                if( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) )
                    pPrinter->SetFont(*pPrtFont);
                pOutDev = pPrinter;
            }
            else
                pOutDev = rInf.GetpOut();
            aTxtSize.Width() = pOutDev->GetTextWidth( rInf.GetText(), rInf.GetIdx(), nLn );
            aTxtSize.Height() = pOutDev->GetTextHeight() +
                                GetFontLeading( rInf.GetShell(), rInf.GetOut() );
            aTxtSize.Width() += (nLn) * long( nGridWidthAdd );
            //if ( rInf.GetKern() && nLn )
            //    aTxtSize.Width() += ( nLn ) * long( rInf.GetKern() );

            rInf.SetKanaDiff( 0 );
            return aTxtSize;
        }
    }

    const bool bCompress = rInf.GetKanaComp() && nLn &&
                           rInf.GetFont() &&
                           SW_CJK == rInf.GetFont()->GetActual() &&
                           rInf.GetScriptInfo() &&
                           rInf.GetScriptInfo()->CountCompChg() &&
                           lcl_IsMonoSpaceFont( rInf.GetOut() );

    OSL_ENSURE( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
            CountCompChg()), "Compression without info" );

    // This is the part used e.g., for cursor travelling
    // See condition for DrawText or DrawTextArray (bDirectPrint)
    if ( pPrinter && pPrinter != rInf.GetpOut() )
    {
        if( !pPrtFont->IsSameInstance( pPrinter->GetFont() ) )
            pPrinter->SetFont(*pPrtFont);
        aTxtSize.Width() = pPrinter->GetTextWidth( rInf.GetText(),
                                                   rInf.GetIdx(), nLn );
        aTxtSize.Height() = pPrinter->GetTextHeight();
        sal_Int32 *pKernArray = new sal_Int32[nLn];
        CreateScrFont( *rInf.GetShell(), rInf.GetOut() );
        if( !GetScrFont()->IsSameInstance( rInf.GetOut().GetFont() ) )
            rInf.GetOut().SetFont( *pScrFont );
        long nScrPos;

        pPrinter->GetTextArray( rInf.GetText(), pKernArray, rInf.GetIdx(),nLn );
        if( bCompress )
            rInf.SetKanaDiff( rInf.GetScriptInfo()->Compress( pKernArray,
                rInf.GetIdx(), nLn, rInf.GetKanaComp(),
                (sal_uInt16)aFont.GetSize().Height() ) );
        else
            rInf.SetKanaDiff( 0 );

        if ( rInf.GetKanaDiff() )
            nScrPos = pKernArray[ nLn - 1 ];
        else
        {
            sal_Int32* pScrArray = new sal_Int32[ rInf.GetLen() ];
            rInf.GetOut().GetTextArray( rInf.GetText(), pScrArray,
                                        rInf.GetIdx(), rInf.GetLen() );
            nScrPos = pScrArray[ 0 ];
            xub_StrLen nCnt = rInf.GetText().getLength();
            if ( nCnt < rInf.GetIdx() )
                nCnt=0;
            else
                nCnt = nCnt - rInf.GetIdx();
            nCnt = std::min (nCnt, nLn);
            sal_Unicode nChPrev = rInf.GetText()[ rInf.GetIdx() ];

            sal_Unicode nCh;

            // In case of Pair Kerning the printer influence on the positioning
            // grows
            sal_uInt16 nMul = 3;
            if ( pPrtFont->GetKerning() )
                nMul = 1;
            const sal_uInt16 nDiv = nMul+1;
            for( xub_StrLen i=1; i<nCnt; i++ )
            {
                nCh = rInf.GetText()[ rInf.GetIdx() + i ];
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
            delete[] pScrArray;
        }

        delete[] pKernArray;
        aTxtSize.Width() = nScrPos;
    }
    else
    {
        if( !pPrtFont->IsSameInstance( rInf.GetOut().GetFont() ) )
            rInf.GetOut().SetFont( *pPrtFont );
        if( bCompress )
        {
            sal_Int32 *pKernArray = new sal_Int32[nLn];
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                        rInf.GetIdx(), nLn );
            rInf.SetKanaDiff( rInf.GetScriptInfo()->Compress( pKernArray,
                rInf.GetIdx(), nLn, rInf.GetKanaComp(),
                (sal_uInt16) aFont.GetSize().Height() ) );
            aTxtSize.Width() = pKernArray[ nLn - 1 ];
            delete[] pKernArray;
        }
        else
        {
            aTxtSize.Width() = rInf.GetOut().GetTextWidth( rInf.GetText(),
                                                           rInf.GetIdx(), nLn );
            rInf.SetKanaDiff( 0 );
        }

        aTxtSize.Height() = rInf.GetOut().GetTextHeight();
    }

    if ( rInf.GetKern() && nLn )
        aTxtSize.Width() += ( nLn - 1 ) * long( rInf.GetKern() );

    OSL_ENSURE( !rInf.GetShell() ||
            ( USHRT_MAX != GetGuessedLeading() && USHRT_MAX != GetExtLeading() ),
              "Leading values should be already calculated" );
    aTxtSize.Height() += GetFontLeading( rInf.GetShell(), rInf.GetOut() );
    return aTxtSize;
}


xub_StrLen SwFntObj::GetCrsrOfst( SwDrawTextInfo &rInf )
{
    long nSpaceAdd =       rInf.GetSpace() / SPACING_PRECISION_FACTOR;
    const long nSperren = -rInf.GetSperren() / SPACING_PRECISION_FACTOR;
    long nKern = rInf.GetKern();

    if( 0 != nSperren )
        nKern -= nSperren;

    sal_Int32 *pKernArray = new sal_Int32[ rInf.GetLen() ];

    // be sure to have the correct layout mode at the printer
    if ( pPrinter )
    {
        pPrinter->SetLayoutMode( rInf.GetOut().GetLayoutMode() );
        pPrinter->SetDigitLanguage( rInf.GetOut().GetDigitLanguage() );
        pPrinter->GetTextArray( rInf.GetText(), pKernArray,
                                rInf.GetIdx(), rInf.GetLen() );
    }
    else
        rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                    rInf.GetIdx(), rInf.GetLen() );

    const SwScriptInfo* pSI = rInf.GetScriptInfo();
    if ( rInf.GetFont() && rInf.GetLen() )
    {
        const sal_uInt8 nActual = rInf.GetFont()->GetActual();

        // Kana Compression
        if ( SW_CJK == nActual && rInf.GetKanaComp() &&
             pSI && pSI->CountCompChg() &&
             lcl_IsMonoSpaceFont( rInf.GetOut() ) )
        {
            pSI->Compress( pKernArray, rInf.GetIdx(), rInf.GetLen(),
                           rInf.GetKanaComp(),
                           (sal_uInt16) aFont.GetSize().Height() );
        }

        // Asian Justification
        if ( SW_CJK == rInf.GetFont()->GetActual() )
        {
            LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CJK );

            if (!MsLangId::isKorean(aLang))
            {
                long nSpaceSum = nSpaceAdd;
                for ( sal_uInt16 nI = 0; nI < rInf.GetLen(); ++nI )
                {
                    pKernArray[ nI ] += nSpaceSum;
                    nSpaceSum += nSpaceAdd;
                }

                nSpaceAdd = 0;
            }

        }

        // Kashida Justification
        if ( SW_CTL == nActual && rInf.GetSpace() )
        {
            if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
            {
                if ( pSI && pSI->CountKashida() &&
                    pSI->KashidaJustify( pKernArray, 0, rInf.GetIdx(), rInf.GetLen(),
                                         nSpaceAdd ) != STRING_LEN )
                    nSpaceAdd = 0;
            }
        }

        // Thai Justification
        if ( SW_CTL == nActual && nSpaceAdd )
        {
            LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CTL );

            if ( LANGUAGE_THAI == aLang )
            {
                SwScriptInfo::ThaiJustify( rInf.GetText(), pKernArray, 0,
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
    xub_StrLen nCnt = 0;
    long nSpaceSum = 0;
    long nKernSum = 0;

    if ( rInf.GetFrm() && rInf.GetLen() && rInf.SnapToGrid() &&
         rInf.GetFont() && SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars() )
        {
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GETGRIDWIDTH(pGrid, pDoc);

            long nWidthPerChar = pKernArray[ rInf.GetLen() - 1 ] / rInf.GetLen();

            sal_uLong i = nWidthPerChar ?
                      ( nWidthPerChar - 1 ) / nGridWidth + 1:
                      1;

            nWidthPerChar = i * nGridWidth;

            nCnt = (sal_uInt16)(rInf.GetOfst() / nWidthPerChar);
            if ( 2 * ( rInf.GetOfst() - nCnt * nWidthPerChar ) > nWidthPerChar )
                ++nCnt;

            delete[] pKernArray;
            return nCnt;
        }
    }

    //for textgrid refactor
    if ( rInf.GetFrm() && rInf.GetLen() && rInf.SnapToGrid() &&
         rInf.GetFont() && SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && !pGrid->IsSnapToChars() )
        {

            const sal_uInt16 nDefaultFontHeight = GetDefaultFontHeight( rInf );

            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            long nGridWidthAdd = GETGRIDWIDTH(pGrid, pDoc);
            if( SW_LATIN == rInf.GetFont()->GetActual() )
                nGridWidthAdd = ( nGridWidthAdd - nDefaultFontHeight ) / 2;
            else
                nGridWidthAdd = nGridWidthAdd - nDefaultFontHeight;

            for(xub_StrLen j = 0; j < rInf.GetLen(); j++)
            {
                long nScr = pKernArray[ j ] + ( nSpaceAdd + nGridWidthAdd  ) * ( j + 1 );
                if( nScr >= rInf.GetOfst())
                {
                    nCnt = j;
                    break;
                }
            }
            delete[] pKernArray;
            return nCnt;
        }
    }

    sal_uInt16 nItrMode = i18n::CharacterIteratorMode::SKIPCELL;
    sal_Int32 nDone = 0;
    LanguageType aLang = LANGUAGE_NONE;
    bool bSkipCharacterCells = false;
    xub_StrLen nIdx = rInf.GetIdx();
    xub_StrLen nLastIdx = nIdx;
    const xub_StrLen nEnd = rInf.GetIdx() + rInf.GetLen();

    // #i105901#
    // skip character cells for all script types
    if ( g_pBreakIt->GetBreakIter().is() )
    {
        aLang = rInf.GetFont()->GetLanguage();
        bSkipCharacterCells = true;
    }

    while ( ( nRight < long( rInf.GetOfst() ) ) && ( nIdx < nEnd ) )
    {
        if ( nSpaceAdd && CH_BLANK == rInf.GetText()[ nIdx ] )
            nSpaceSum += nSpaceAdd;

        // go to next character (cell).
        nLastIdx = nIdx;

        if ( bSkipCharacterCells )
        {
            nIdx = (xub_StrLen)g_pBreakIt->GetBreakIter()->nextCharacters( rInf.GetText(),
                        nIdx, g_pBreakIt->GetLocale( aLang ), nItrMode, 1, nDone );
            if ( nIdx <= nLastIdx )
                break;
        }
        else
            ++nIdx;

        nLeft = nRight;
        nRight = pKernArray[ nIdx - rInf.GetIdx() - 1 ] + nKernSum + nSpaceSum;

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

    delete[] pKernArray;
    return nCnt;
}


/*************************************************************************
|*
|*  SwFntAccess::SwFntAccess()
|*
|*************************************************************************/

SwFntAccess::SwFntAccess( const void* &rMagic,
                sal_uInt16 &rIndex, const void *pOwn, ViewShell *pSh,
                sal_Bool bCheck ) :
  SwCacheAccess( *pFntCache, rMagic, rIndex ),
  pShell( pSh )
{
    // the used ctor of SwCacheAccess searches for rMagic+rIndex in the cache
    if ( IsAvail() )
    {
        // fast case: known Font (rMagic), no need to check printer and zoom
        if ( !bCheck )
            return;

        // Font is known, but has to be checked
    }
    else
    {   // Font not known, must be searched
        bCheck = sal_False;
    }


    {
        OutputDevice* pOut = 0;
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
                 ( pFntObj->pPrinter == pOut ) &&
                   pFntObj->GetPropWidth() ==
                        ((SwSubFont*)pOwn)->GetPropWidth() )
            {
                return; // result of Check: Drucker+Zoom okay.
            }
            pFntObj->Unlock(); // forget this object, printer/zoom differs
            pObj = NULL;
        }

        // Search by font comparison, quite expensive!
        // Look for same font and same printer
        pFntObj = pFntCache->First();
        while ( pFntObj && !( pFntObj->aFont == *(Font *)pOwn &&
                              pFntObj->GetZoom() == nZoom &&
                              pFntObj->GetPropWidth() ==
                              ((SwSubFont*)pOwn)->GetPropWidth() &&
                              ( !pFntObj->pPrinter || pFntObj->pPrinter == pOut ) ) )
            pFntObj = pFntCache->Next( pFntObj );

        if( pFntObj && pFntObj->pPrinter != pOut )
        {
            // found one without printer, let's see if there is one with
            // the same printer as well
            SwFntObj *pTmpObj = pFntObj;
            while( pTmpObj && !( pTmpObj->aFont == *(Font *)pOwn &&
                   pTmpObj->GetZoom()==nZoom && pTmpObj->pPrinter==pOut &&
                   pTmpObj->GetPropWidth() ==
                        ((SwSubFont*)pOwn)->GetPropWidth() ) )
                pTmpObj = pFntCache->Next( pTmpObj );
            if( pTmpObj )
                pFntObj = pTmpObj;
        }

        if ( !pFntObj ) // Font has not been found, create one
        {
            // Have to create new Object, hence Owner must be a SwFont, later
            // the Owner will be the "MagicNumber"
            SwCacheAccess::pOwner = pOwn;
            pFntObj = Get(); // will create via NewObj() and lock
            OSL_ENSURE(pFntObj, "No Font, no Fun.");
        }
        else  // Font has been found, so we lock it.
        {
            pFntObj->Lock();
            if (pFntObj->pPrinter != pOut) // if no printer is known by now
            {
                OSL_ENSURE( !pFntObj->pPrinter, "SwFntAccess: Printer Changed" );
                pFntObj->CreatePrtFont( *pOut );
                pFntObj->pPrinter = pOut;
                pFntObj->pScrFont = NULL;
                pFntObj->nGuessedLeading = USHRT_MAX;
                pFntObj->nExtLeading = USHRT_MAX;
                pFntObj->nPrtAscent = USHRT_MAX;
                pFntObj->nPrtHeight = USHRT_MAX;
            }
            pObj = pFntObj;
        }

        // no matter if new or found, now the Owner of the Object is a
        // MagicNumber, and will be given to the SwFont, as well as the Index
        // for later direct access
        rMagic = pFntObj->GetOwner();
        SwCacheAccess::pOwner = rMagic;
        rIndex = pFntObj->GetCachePos();
    }
}

SwCacheObj *SwFntAccess::NewObj( )
{
    // a new Font, a new "MagicNumber".
    return new SwFntObj( *(SwSubFont *)pOwner, ++pMagicNo, pShell );
}

extern xub_StrLen sw_CalcCaseMap( const SwFont& rFnt,
                                   const OUString& rOrigString,
                                   xub_StrLen nOfst,
                                   xub_StrLen nLen,
                                   xub_StrLen nIdx );

xub_StrLen SwFont::GetTxtBreak( SwDrawTextInfo& rInf, long nTextWidth )
{
    ChgFnt( rInf.GetShell(), rInf.GetOut() );

    const bool bCompress = rInf.GetKanaComp() && rInf.GetLen() &&
                           SW_CJK == GetActual() &&
                           rInf.GetScriptInfo() &&
                           rInf.GetScriptInfo()->CountCompChg() &&
                           lcl_IsMonoSpaceFont( rInf.GetOut() );

    OSL_ENSURE( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
            CountCompChg()), "Compression without info" );

    sal_uInt16 nTxtBreak = 0;
    long nKern = 0;

    sal_uInt16 nLn = ( rInf.GetLen() == STRING_LEN ? rInf.GetText().getLength()
                                               : rInf.GetLen() );

    if ( rInf.GetFrm() && nLn && rInf.SnapToGrid() &&
         rInf.GetFont() && SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars() )
        {
            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            const sal_uInt16 nGridWidth = GETGRIDWIDTH(pGrid, pDoc);

            sal_Int32* pKernArray = new sal_Int32[rInf.GetLen()];
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                        rInf.GetIdx(), rInf.GetLen() );

            long nWidthPerChar = pKernArray[ rInf.GetLen() - 1 ] / rInf.GetLen();

            const sal_uLong i = nWidthPerChar ?
                            ( nWidthPerChar - 1 ) / nGridWidth + 1:
                            1;

            nWidthPerChar = i * nGridWidth;
            long nCurrPos = nWidthPerChar;

            while( nTxtBreak < rInf.GetLen() && nTextWidth >= nCurrPos )
            {
                nCurrPos += nWidthPerChar;
                ++nTxtBreak;
            }

            delete[] pKernArray;
            return nTxtBreak + rInf.GetIdx();
        }
    }

    //for text grid enhancement
    if ( rInf.GetFrm() && nLn && rInf.SnapToGrid() && rInf.GetFont() &&
         SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && !pGrid->IsSnapToChars() )
        {
            const sal_uInt16 nDefaultFontHeight = GetDefaultFontHeight( rInf );

            const SwDoc* pDoc = rInf.GetShell()->GetDoc();
            long nGridWidthAdd = GETGRIDWIDTH(pGrid, pDoc);
            if( SW_LATIN == rInf.GetFont()->GetActual() )
                nGridWidthAdd = ( nGridWidthAdd - nDefaultFontHeight ) / 2 ;
            else
                nGridWidthAdd = nGridWidthAdd - nDefaultFontHeight;

            sal_Int32* pKernArray = new sal_Int32[rInf.GetLen()];
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                            rInf.GetIdx(), rInf.GetLen() );
            long nCurrPos = pKernArray[nTxtBreak] + nGridWidthAdd;
            while( nTxtBreak < rInf.GetLen() && nTextWidth >= nCurrPos)
            {
                nTxtBreak++;
                nCurrPos = pKernArray[nTxtBreak] + nGridWidthAdd * ( nTxtBreak + 1 );
            }
            delete[] pKernArray;
            return nTxtBreak + rInf.GetIdx();
        }
    }

    if( aSub[nActual].IsCapital() && nLn )
    {
        nTxtBreak = GetCapitalBreak( rInf.GetShell(), rInf.GetpOut(),
            rInf.GetScriptInfo(), rInf.GetText(), nTextWidth, rInf.GetIdx(),
            nLn );
    }
    else
    {
        nKern = CheckKerning();

        const OUString* pTmpText;
        OUString aTmpText;
        xub_StrLen nTmpIdx;
        xub_StrLen nTmpLen;
        bool bTextReplaced = false;

        if ( !aSub[nActual].IsCaseMap() )
        {
            pTmpText = &rInf.GetText();
            nTmpIdx = rInf.GetIdx();
            nTmpLen = nLn;
        }
        else
        {
            const OUString aSnippet(rInf.GetText().copy(rInf.GetIdx(), nLn));
            aTmpText = aSub[nActual].CalcCaseMap( aSnippet );
            const bool bTitle = SVX_CASEMAP_TITEL == aSub[nActual].GetCaseMap() &&
                                g_pBreakIt->GetBreakIter().is();

            // Uaaaaahhhh!!! In title case mode, we would get wrong results
            if ( bTitle && nLn )
            {
                // check if rInf.GetIdx() is begin of word
                if ( !g_pBreakIt->GetBreakIter()->isBeginWord(
                     rInf.GetText(), rInf.GetIdx(),
                     g_pBreakIt->GetLocale( aSub[nActual].GetLanguage() ),
                     i18n::WordType::ANYWORD_IGNOREWHITESPACES ) )
                {
                    // In this case, the beginning of aTmpText is wrong.
                    OUString aSnippetTmp(aSnippet.copy(0, 1));
                    aSnippetTmp = aSub[nActual].CalcCaseMap( aSnippetTmp );
                    aTmpText = aTmpText.replaceAt( 0, aSnippetTmp.getLength(), OUString(aSnippet[0]) );
                }
            }

            pTmpText = &aTmpText;
            nTmpIdx = 0;
            nTmpLen = aTmpText.getLength();
            bTextReplaced = true;
        }

        OUString sTmpText(*pTmpText); // only needed until *pTmpText is OUString
        sal_Int32 nTmpIdx2 = nTmpIdx;  // ditto
        sal_Int32 nTmpLen2 = nTmpLen;  // ditto
        if( rInf.GetHyphPos() ) {
            sal_Int32 nHyphPos = *rInf.GetHyphPos();
            nTxtBreak = rInf.GetOut().GetTextBreak( sTmpText, nTextWidth,
                             static_cast<sal_Unicode>('-'), nHyphPos,
                             nTmpIdx2, nTmpLen2, nKern );
            *rInf.GetHyphPos() = (nHyphPos == -1)
                ? STRING_LEN : static_cast<xub_StrLen>(nHyphPos);
        }
        else
            nTxtBreak = rInf.GetOut().GetTextBreak( sTmpText, nTextWidth,
                                                    nTmpIdx2, nTmpLen2, nKern );

        nTmpIdx = nTmpIdx2;     // ditto
        nTmpLen = nTmpLen2;     // ditto
        if ( bTextReplaced && STRING_LEN != nTxtBreak )
        {
            if ( nTmpLen != nLn )
                nTxtBreak = sw_CalcCaseMap( *this, rInf.GetText(),
                                             rInf.GetIdx(), nLn, nTxtBreak );
            else
                nTxtBreak = nTxtBreak + rInf.GetIdx();
        }
    }

    if ( ! bCompress )
        return nTxtBreak;

    nTxtBreak = nTxtBreak - rInf.GetIdx();

    if( nTxtBreak < nLn )
    {
        if( !nTxtBreak && nLn )
            nLn = 1;
        else if( nLn > 2 * nTxtBreak )
            nLn = 2 * nTxtBreak;
        sal_Int32 *pKernArray = new sal_Int32[ nLn ];
        rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                    rInf.GetIdx(), nLn );
        if( rInf.GetScriptInfo()->Compress( pKernArray, rInf.GetIdx(), nLn,
                            rInf.GetKanaComp(), (sal_uInt16)GetHeight( nActual ) ) )
        {
            long nKernAdd = nKern;
            xub_StrLen nTmpBreak = nTxtBreak;
            if( nKern && nTxtBreak )
                nKern *= nTxtBreak - 1;
            while( nTxtBreak<nLn && nTextWidth >= pKernArray[nTxtBreak] +nKern )
            {
                nKern += nKernAdd;
                ++nTxtBreak;
            }
            if( rInf.GetHyphPos() )
                *rInf.GetHyphPos() += nTxtBreak - nTmpBreak; // It's not perfect
        }
        delete[] pKernArray;
    }
    nTxtBreak = nTxtBreak + rInf.GetIdx();

    return nTxtBreak;
}

extern Color aGlobalRetoucheColor;

sal_Bool SwDrawTextInfo::ApplyAutoColor( Font* pFont )
{
    const Font& rFnt = pFont ? *pFont : GetOut().GetFont();
    bool bPrt = GetShell() && ! GetShell()->GetWin();
    ColorData nNewColor = COL_BLACK;
    bool bChgFntColor = false;
    bool bChgLineColor = false;

    if( bPrt && GetShell() && GetShell()->GetViewOptions()->IsBlackFont() )
    {
        if ( COL_BLACK != rFnt.GetColor().GetColor() )
            bChgFntColor = true;

        if ( (COL_BLACK != GetOut().GetLineColor().GetColor()) ||
             (COL_BLACK != GetOut().GetOverlineColor().GetColor()) )
            bChgLineColor = true;
    }
    else
    {
        // FontColor has to be changed if:
        // 1. FontColor = AUTO or 2. IsAlwaysAutoColor is set
        // LineColor has to be changed if:
        // 1. IsAlwaysAutoColor is set

        bChgLineColor = ! bPrt && GetShell() &&
                GetShell()->GetAccessibilityOptions()->IsAlwaysAutoColor();

        bChgFntColor = COL_AUTO == rFnt.GetColor().GetColor() || bChgLineColor;

        if ( bChgFntColor )
        {
            // check if current background has a user defined setting
            const Color* pCol = GetFont() ? GetFont()->GetBackColor() : NULL;
            if( ! pCol || COL_TRANSPARENT == pCol->GetColor() )
            {
                const SvxBrushItem* pItem;
                const XFillStyleItem* pFillStyleItem;
                const XFillGradientItem* pFillGradientItem;
                SwRect aOrigBackRect;

                /// OD 21.08.2002
                ///     consider, that [GetBackgroundBrush(...)] can set <pCol>
                ///     - see implementation in /core/layout/paintfrm.cxx
                /// OD 21.08.2002 #99657#
                ///     There is a user defined setting for the background, if there
                ///     is a background brush and its color is *not* "no fill"/"auto fill".
                if( GetFrm()->GetBackgroundBrush( pItem, pFillStyleItem, pFillGradientItem, pCol, aOrigBackRect, sal_False ) )
                {
                    if ( !pCol )
                    {
                        pCol = &pItem->GetColor();
                    }

                    /// OD 30.08.2002 #99657#
                    /// determined color <pCol> can be <COL_TRANSPARENT>. Thus, check it.
                    if ( pCol->GetColor() == COL_TRANSPARENT)
                        pCol = NULL;
                }
                else
                    pCol = NULL;
            }

            // no user defined color at paragraph or font background
            if ( ! pCol )
                pCol = &aGlobalRetoucheColor;

            if( GetShell() && GetShell()->GetWin() )
            {
                // here we determine the prefered window text color for painting
                const SwViewOption* pViewOption = GetShell()->GetViewOptions();
                if(pViewOption->IsPagePreview() &&
                        !SW_MOD()->GetAccessibilityOptions().GetIsForPagePreviews())
                    nNewColor = COL_BLACK;
                else
                    // we take the font color from the appearance page
                    nNewColor = SwViewOption::GetFontColor().GetColor();
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
                Font aFont( rFnt );
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

        return sal_True;
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
