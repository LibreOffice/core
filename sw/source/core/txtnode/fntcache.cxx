/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <swfont.hxx>
#include <wrong.hxx>
#include "dbg_lay.hxx"
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
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



SwFntCache *pFntCache = NULL;

SwFntObj *pLastFont = NULL;

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

SwFntObj::SwFntObj(const SwSubFont &rFont, const void *pOwn, SwViewShell *pSh)
    : SwCacheObj((void*)pOwn)
    , aFont(rFont)
    , pScrFont(NULL)
    , pPrtFont(&aFont)
    , pPrinter(NULL)
    , nGuessedLeading(USHRT_MAX)
    , nExtLeading(USHRT_MAX)
    , nScrAscent(0)
    , nPrtAscent(USHRT_MAX)
    , nScrHeight(0)
    , nPrtHeight(USHRT_MAX)
    , nPropWidth(rFont.GetPropWidth())
{
    nZoom = pSh ? pSh->GetViewOptions()->GetZoom() : USHRT_MAX;
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
    sal_Int32 nCnt;
    const sal_Bool bSwitchH2V;
    const sal_Bool bSwitchL2R;
    long nHalfSpace;
    sal_Int32* pKernArray;
    const sal_Bool bBidiPor;

    CalcLinePosData( SwDrawTextInfo& _rInf, Font& _rFont,
                      sal_Int32 _nCnt, const sal_Bool _bSwitchH2V, const sal_Bool _bSwitchL2R,
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
    Point &rStart, Point &rEnd, sal_Int32 nStart, sal_Int32 nWrLen )
{
   long nBlank = 0;
   const sal_Int32 nEnd = nStart + nWrLen;
   const long nTmpSpaceAdd = rData.rInf.GetSpace() / SPACING_PRECISION_FACTOR;

   if ( nEnd < rData.nCnt
       && CH_BLANK == rData.rInf.GetText()[ rData.rInf.GetIdx() + nEnd ] )
   {
       if( nEnd + 1 == rData.nCnt )
           nBlank -= nTmpSpaceAdd;
       else
           nBlank -= rData.nHalfSpace;
   }

   
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

sal_uInt16 SwFntObj::GetFontAscent( const SwViewShell *pSh, const OutputDevice& rOut )
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
        if (nPrtAscent == USHRT_MAX) 
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

#if !defined(MACOSX) 
    
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

sal_uInt16 SwFntObj::GetFontHeight( const SwViewShell* pSh, const OutputDevice& rOut )
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
        if (nPrtHeight == USHRT_MAX) 
        {
            CreatePrtFont( rOut );
            const Font aOldFnt( rRefDev.GetFont() );
            ((OutputDevice&)rRefDev).SetFont( *pPrtFont );
            nPrtHeight = static_cast<sal_uInt16>(rRefDev.GetTextHeight());

#if OSL_DEBUG_LEVEL > 0
            
            const FontMetric aOutMet( rRefDev.GetFontMetric() );
            long nTmpPrtHeight = (sal_uInt16)aOutMet.GetAscent() + aOutMet.GetDescent();
            (void) nTmpPrtHeight;
            
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

sal_uInt16 SwFntObj::GetFontLeading( const SwViewShell *pSh, const OutputDevice& rOut )
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
 *  SwFntObj::CreateScrFont( const SwViewShell& rSh, const OutputDevice& rOut )
 *
 *  pOut is the output device, not the reference device
 *
 *************************************************************************/

void SwFntObj::CreateScrFont( const SwViewShell& rSh, const OutputDevice& rOut )
{
    if ( pScrFont )
        return;

    
    OutputDevice* pOut = (OutputDevice*)&rOut;

    
    Font aOldOutFont( pOut->GetFont() );

    nScrHeight = USHRT_MAX;

    
    OutputDevice* pPrt = &rSh.GetRefDev();

    if( !rSh.GetWin() ||
        !rSh.GetViewOptions()->getBrowseMode() ||
         rSh.GetViewOptions()->IsPrtFormat() )
    {
        
        
        CreatePrtFont( *pPrt );
        pPrinter = pPrt;

        
        Font aOldPrtFnt( pPrt->GetFont() );

        
        
        pPrt->SetFont( *pPrtFont );
        pOut->SetFont( *pPrtFont );

        
        pScrFont = pPrtFont;

        FontMetric aMet = pPrt->GetFontMetric( );
        
        
        aMet.SetWeight(pScrFont->GetWeight());
        aMet.SetItalic(pScrFont->GetItalic());

        bSymbol = RTL_TEXTENCODING_SYMBOL == aMet.GetCharSet();

        if ( USHRT_MAX == nGuessedLeading )
            GuessLeading( rSh, aMet );

        if ( USHRT_MAX == nExtLeading )
            nExtLeading = static_cast<sal_uInt16>(aMet.GetExtLeading());

        
        pPrt->SetFont( aOldPrtFnt );
    }
    else
    {
        bSymbol = RTL_TEXTENCODING_SYMBOL == aFont.GetCharSet();
        if ( nGuessedLeading == USHRT_MAX )
            nGuessedLeading = 0;

        
        if ( nExtLeading == USHRT_MAX )
            nExtLeading = 0;

        pScrFont = pPrtFont;
    }

    
    {
        
        
        
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

    
    pOut->SetFont( aOldOutFont );
}


void SwFntObj::GuessLeading( const SwViewShell&
#if defined(WNT)
                             rSh
#endif
                             , const FontMetric& rMet )
{
    
    
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

void SwFntObj::SetDevFont( const SwViewShell *pSh, OutputDevice& rOut )
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
        
        return SwScriptInfo::NONE;
    else if ( 0x3001 == cChar || 0x3002 == cChar ||
              0x3009 == cChar || 0x300B == cChar ||
              0x300D == cChar || 0x300F == cChar ||
              0x3011 == cChar || 0x3015 == cChar ||
              0x3017 == cChar || 0x3019 == cChar ||
              0x301B == cChar || 0x301E == cChar ||
              0x301F == cChar || 0xFF63 == cChar )
        
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

typedef std::vector< std::pair< sal_Int32, sal_Int32 > > SwForbidden;

static void lcl_DrawLineForWrongListData(
    SwForbidden &rForbidden,
    const SwDrawTextInfo    &rInf,
    const SwWrongList       *pWList,
    const CalcLinePosData   &rCalcLinePosData,
    const Size              &rPrtFontSize )
{
    if (!pWList) return;

    sal_Int32 nStart = rInf.GetIdx();
    sal_Int32 nWrLen = rInf.GetLen();

    
    if (!pWList->Check( nStart, nWrLen ))
    {
        return;
    }

    long nHght = rInf.GetOut().LogicToPixel( rPrtFontSize ).Height();

    
    
    if (pWList != rInf.GetSmartTags() && WRONG_SHOW_MIN >= nHght)
    {
        return;
    }

    SwForbidden::iterator pIter = rForbidden.begin();
    if (rInf.GetOut().GetConnectMetaFile())
        rInf.GetOut().Push();

    const Color aCol( rInf.GetOut().GetLineColor() );

    
    do
    {
        nStart -= rInf.GetIdx();

        const sal_Int32 nEnd = nStart + nWrLen;
        sal_Int32 nNext = nStart;
        while( nNext < nEnd )
        {
            while( pIter != rForbidden.end() && pIter->second <= nNext )
                ++pIter;

            const sal_Int32 nNextStart = nNext;
            sal_Int32 nNextEnd = nEnd;

            if( pIter == rForbidden.end() || nNextEnd <= pIter->first )
            {
                
                std::pair< sal_Int32, sal_Int32 > aNew;
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

    
    
    const bool bDirectPrint = bPrt || bBrowse;

    
    const bool bUseScrFont =
        lcl_IsFontAdjustNecessary( rInf.GetOut(), rRefDev );

    Font* pTmpFont = bUseScrFont ? pScrFont : pPrtFont;

    //
    
    //
    
    
    
    
    
    
    
    //
    
    
    //

#if OSL_DEBUG_LEVEL > 0

    const bool bNoAdjust = bPrt ||
            (  pWin &&
               rInf.GetShell()->GetViewOptions()->getBrowseMode() &&
              !rInf.GetShell()->GetViewOptions()->IsPrtFormat() );

    if ( OUTDEV_PRINTER == rInf.GetOut().GetOutDevType() )
    {
        
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

    
    OSL_ENSURE( pTmpFont, "No screen or printer font?" );
    if ( ! pTmpFont )
        pTmpFont = pPrtFont;

    
    

    const sal_Bool bSwitchH2V = rInf.GetFrm() && rInf.GetFrm()->IsVertical();
    const bool bSwitchL2R = rInf.GetFrm() && rInf.GetFrm()->IsRightToLeft() &&
                            ! rInf.IsIgnoreFrmRTL();
    const sal_uLong nMode = rInf.GetOut().GetLayoutMode();
    const sal_Bool bBidiPor = ( bSwitchL2R !=
                            ( 0 != ( TEXT_LAYOUT_BIDI_RTL & nMode ) ) );

    
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

    if ( COMPLETE_STRING == rInf.GetLen() )
        rInf.SetLen( rInf.GetText().getLength() );


    //
    
    //

    if ( rInf.GetFrm() && rInf.SnapToGrid() && rInf.GetFont() &&
         SW_CJK == rInf.GetFont()->GetActual() )
    {
        GETGRID( rInf.GetFrm()->FindPageFrm() )
        if ( pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars())
        {
            
            
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

            
            long nCharWidth = pKernArray[ 0 ];
            sal_uLong nHalfWidth = nWidthPerChar / 2;

            long nNextFix;

            
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

            
            for( sal_Int32 j = 1; j < rInf.GetLen(); ++j )
            {
                long nScr = pKernArray[ j ] - pKernArray[ j - 1 ];
                nNextFix += nWidthPerChar;

                
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
                    
                    if( SW_CJK == nActual && rInf.GetKanaComp() &&
                        pSI && pSI->CountCompChg() &&
                        lcl_IsMonoSpaceFont( *(rInf.GetpOut()) ) )
                    {
                        pSI->Compress( pKernArray,rInf.GetIdx(), rInf.GetLen(),
                            rInf.GetKanaComp(), (sal_uInt16)aFont.GetSize().Height(),&aPos );
                        bSpecialJust = true;
                    }
                    
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
                    for(sal_Int32 i = 0; i < rInf.GetLen(); i++, nGridAddSum += nGridWidthAdd )
                    {
                        pKernArray[i] += nGridAddSum;
                    }
                    long nKernSum = rInf.GetKern();
                    if ( bSpecialJust || rInf.GetKern() )
                    {
                        for( sal_Int32 i = 0; i < rInf.GetLen(); i++, nKernSum += rInf.GetKern() )
                        {
                            if ( CH_BLANK == rInf.GetText()[ rInf.GetIdx()+i ] )
                                nKernSum += nSpaceAdd;
                            pKernArray[i] += nKernSum;
                        }
                        
                        
                        if( bPaintBlank && rInf.GetLen() && (CH_BLANK ==
                            rInf.GetText()[ rInf.GetIdx() + rInf.GetLen() - 1 ] ) )
                        {
                            
                            
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
                        sal_Int32 i;
                        sal_Int32 j = 0;
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
                
        long nKernAdd = 0;
                long nGridAddSum = nGridWidthAdd + nKernAdd;
                for(sal_Int32 i = 0; i < rInf.GetLen(); i++,nGridAddSum += nGridWidthAdd + nKernAdd )
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

        
        
        
        
        
        if( rInf.GetSpace() || rInf.GetKanaComp() )
        {
            sal_Int32 *pKernArray = new sal_Int32[ rInf.GetLen() ];
            rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                       rInf.GetIdx(), rInf.GetLen() );

            if( bStretch )
            {
                sal_Int32 nZwi = rInf.GetLen() - 1;
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
                for( sal_Int32 i = 0; i < nZwi; )
                {
                    pKernArray[ i ] += nSum;
                    if( ++i == nRest )
                        nDiff += nAdd;
                    nSum += nDiff;
                }
            }

            //
            
            //
            long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
            bool bSpecialJust = false;

            if ( rInf.GetFont() && rInf.GetLen() )
            {
                const SwScriptInfo* pSI = rInf.GetScriptInfo();
                const sal_uInt8 nActual = rInf.GetFont()->GetActual();

                
                if ( SW_CJK == nActual && rInf.GetKanaComp() &&
                     pSI && pSI->CountCompChg() &&
                     lcl_IsMonoSpaceFont( rInf.GetOut() ) )
                {
                    pSI->Compress( pKernArray, rInf.GetIdx(), rInf.GetLen(),
                                   rInf.GetKanaComp(),
                                   (sal_uInt16)aFont.GetSize().Height(), &aPos );
                    bSpecialJust = true;
                }

                
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

                
                if ( SW_CTL == nActual && nSpaceAdd )
                {
                    if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
                    {
                        if ( pSI && pSI->CountKashida() &&
                            pSI->KashidaJustify( pKernArray, 0, rInf.GetIdx(),
                                                 rInf.GetLen(), nSpaceAdd ) != -1 )
                        {
                            bSpecialJust = true;
                            nSpaceAdd = 0;
                        }
                    }
                }

                
                if ( SW_CTL == nActual && nSpaceAdd )
                {
                    LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CTL );

                    if ( LANGUAGE_THAI == aLang )
                    {
                        
                        
                        SwScriptInfo::ThaiJustify( rInf.GetText(), pKernArray, 0,
                                                   rInf.GetIdx(), rInf.GetLen(),
                                                   rInf.GetNumberOfBlanks(),
                                                   rInf.GetSpace() );

                        
                        bSpecialJust = true;
                        nSpaceAdd = 0;
                    }
                }
            }

            long nKernSum = rInf.GetKern();

            if ( bStretch || bPaintBlank || rInf.GetKern() || bSpecialJust )
            {
                for( sal_Int32 i = 0; i < rInf.GetLen(); i++,
                     nKernSum += rInf.GetKern() )
                {
                    if ( CH_BLANK == rInf.GetText()[ rInf.GetIdx()+i ] )
                        nKernSum += nSpaceAdd;
                    pKernArray[i] += nKernSum;
                }

                
                
                if( bPaintBlank && rInf.GetLen() && ( CH_BLANK ==
                    rInf.GetText()[ rInf.GetIdx()+rInf.GetLen()-1 ] ) )
                {
                    
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
                sal_Int32 j = 0;
                sal_Int32 i;
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
    
    //

    else
    {
        const OUString* pStr = &rInf.GetText();
        OUString aStr;
        OUString aBulletOverlay;
        sal_Bool bBullet = rInf.GetBullet();
        if( bSymbol )
            bBullet = sal_False;
        sal_Int32 *pKernArray = new sal_Int32[ rInf.GetLen() ];
        CreateScrFont( *rInf.GetShell(), rInf.GetOut() );
        long nScrPos;

        
        sal_Int32* pScrArray = new sal_Int32[ rInf.GetLen() ];
        rInf.GetOut().GetTextArray( rInf.GetText(), pScrArray,
                                    rInf.GetIdx(), rInf.GetLen() );

        
        
        if ( pPrinter )
        {
            
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
        
        //
        long nSpaceAdd = rInf.GetSpace() / SPACING_PRECISION_FACTOR;
        bool bNoHalfSpace = false;

        if ( rInf.GetFont() && rInf.GetLen() )
        {
            const sal_uInt8 nActual = rInf.GetFont()->GetActual();
            const SwScriptInfo* pSI = rInf.GetScriptInfo();

            
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

            
            if ( SW_CTL == nActual && nSpaceAdd )
            {
                if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
                {
                    if ( pSI && pSI->CountKashida() &&
                         pSI->KashidaJustify( pKernArray, pScrArray, rInf.GetIdx(),
                                              rInf.GetLen(), nSpaceAdd ) != -1 )
                        nSpaceAdd = 0;
                    else
                        bNoHalfSpace = true;
                }
            }

            
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

                    
                    nSpaceAdd = 0;
                }
            }
        }

        nScrPos = pScrArray[ 0 ];

        if( bBullet )
        {
            
            
            
            sal_Int32 nCopyStart = rInf.GetIdx();
            if ( nCopyStart )
                --nCopyStart;

            sal_Int32 nCopyLen = rInf.GetLen();
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
                    if ((i + nCopyStart + 1 >= rInf.GetLen()) ||
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

        sal_Int32 nCnt = rInf.GetText().getLength();
        if ( nCnt < rInf.GetIdx() )
            nCnt = 0;
        else
            nCnt = nCnt - rInf.GetIdx();
        nCnt = std::min<sal_Int32>( nCnt, rInf.GetLen() );
        long nKernSum = rInf.GetKern();
        sal_Unicode cChPrev = rInf.GetText()[ rInf.GetIdx() ];

        
        
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

            
            
            sal_uInt16 nMul = 3;

            if ( pPrtFont->GetKerning() )
                nMul = 1;

            const sal_uInt16 nDiv = nMul+1;

            
            
            
            
            
            
            
            
            
            
            long nSpaceSum = 0;
            
            const long nHalfSpace = pPrtFont->IsWordLineMode() || bNoHalfSpace ? 0 : nSpaceAdd / 2;
            const long nOtherHalf = nSpaceAdd - nHalfSpace;
            if ( nSpaceAdd && ( cChPrev == CH_BLANK ) )
                nSpaceSum = nHalfSpace;
            for( sal_Int32 i=1; i<nCnt; ++i, nKernSum += rInf.GetKern() )
            {
                nCh = rInf.GetText()[ rInf.GetIdx() + i ];

                OSL_ENSURE( pScrArray, "Where is the screen array?" );
                long nScr;
                nScr = pScrArray[ i ] - pScrArray[ i - 1 ];

                
                
                
                
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
                
                
                
                
                if ( (bNoHalfSpace || pPrtFont->IsWordLineMode()) && i+1 == nCnt && nCh == CH_BLANK )
                    pKernArray[i-1] = pKernArray[i-1] - nSpaceAdd;
            }

            
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
                
                if (rInf.GetWrong() || rInf.GetGrammarCheck() || rInf.GetSmartTags())
                {
                    CalcLinePosData aCalcLinePosData(rInf, *GetFont(),
                            nCnt, bSwitchH2V, bSwitchL2R,
                            nHalfSpace, pKernArray, bBidiPor);

                    SwForbidden aForbidden;
                    
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetSmartTags(), aCalcLinePosData, Size() );
                    
                    
                    
                    
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetWrong(), aCalcLinePosData, pPrtFont->GetSize() );
                    
                    lcl_DrawLineForWrongListData( aForbidden, rInf, rInf.GetGrammarCheck(), aCalcLinePosData, pPrtFont->GetSize() );
                }
            }

            sal_Int32 nOffs = 0;
            sal_Int32 nLen = rInf.GetLen();

            if( nOffs < nLen )
            {
                
                
                
                
                sal_Int32 nTmpIdx = bBullet ?
                                              ( rInf.GetIdx() ? 1 : 0 ) :
                                              rInf.GetIdx();

                if ( bSwitchL2R )
                    rInf.GetFrm()->SwitchLTRtoRTL( aPos );

                if ( bSwitchH2V )
                    rInf.GetFrm()->SwitchHorizontalToVertical( aPos );

                rInf.GetOut().DrawTextArray( aPos, *pStr, pKernArray + nOffs,
                                             nTmpIdx + nOffs , nLen - nOffs );
                if (bBullet)
                {
                    rInf.GetOut().Push();
                    Color aPreviousColor = pTmpFont->GetColor();

                    FontUnderline aPreviousUnderline = pTmpFont->GetUnderline();
                    FontUnderline aPreviousOverline = pTmpFont->GetOverline();
                    FontStrikeout aPreviousStrikeout = pTmpFont->GetStrikeout();

                    pTmpFont->SetColor( Color(NON_PRINTING_CHARACTER_COLOR) );
                    pTmpFont->SetUnderline(UNDERLINE_NONE);
                    pTmpFont->SetOverline(UNDERLINE_NONE);
                    pTmpFont->SetStrikeout(STRIKEOUT_NONE);
                    rInf.GetOut().SetFont( *pTmpFont );
                    rInf.GetOut().DrawTextArray( aPos, aBulletOverlay, pKernArray + nOffs,
                                                 nTmpIdx + nOffs , nLen - nOffs );
                    pTmpFont->SetColor( aPreviousColor );

                    pTmpFont->SetUnderline(aPreviousUnderline);
                    pTmpFont->SetOverline(aPreviousOverline);
                    pTmpFont->SetStrikeout(aPreviousStrikeout);
                    rInf.GetOut().Pop();
                }
            }
        }
        delete[] pScrArray;
        delete[] pKernArray;
    }
}

Size SwFntObj::GetTextSize( SwDrawTextInfo& rInf )
{
    Size aTxtSize;
    const sal_Int32 nLn = ( COMPLETE_STRING != rInf.GetLen() ) ? rInf.GetLen() :
                           rInf.GetText().getLength();

    
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
            sal_Int32 nCnt = rInf.GetText().getLength();
            if ( nCnt < rInf.GetIdx() )
                nCnt=0;
            else
                nCnt = nCnt - rInf.GetIdx();
            nCnt = std::min<sal_Int32>(nCnt, nLn);
            sal_Unicode nChPrev = rInf.GetText()[ rInf.GetIdx() ];

            sal_Unicode nCh;

            
            
            sal_uInt16 nMul = 3;
            if ( pPrtFont->GetKerning() )
                nMul = 1;
            const sal_uInt16 nDiv = nMul+1;
            for( sal_Int32 i = 1; i<nCnt; i++ )
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


sal_Int32 SwFntObj::GetCrsrOfst( SwDrawTextInfo &rInf )
{
    long nSpaceAdd =       rInf.GetSpace() / SPACING_PRECISION_FACTOR;
    const long nSperren = -rInf.GetSperren() / SPACING_PRECISION_FACTOR;
    long nKern = rInf.GetKern();

    if( 0 != nSperren )
        nKern -= nSperren;

    sal_Int32 *pKernArray = new sal_Int32[ rInf.GetLen() ];

    
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

        
        if ( SW_CJK == nActual && rInf.GetKanaComp() &&
             pSI && pSI->CountCompChg() &&
             lcl_IsMonoSpaceFont( rInf.GetOut() ) )
        {
            pSI->Compress( pKernArray, rInf.GetIdx(), rInf.GetLen(),
                           rInf.GetKanaComp(),
                           (sal_uInt16) aFont.GetSize().Height() );
        }

        
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

        
        if ( SW_CTL == nActual && rInf.GetSpace() )
        {
            if ( SwScriptInfo::IsArabicText( rInf.GetText(), rInf.GetIdx(), rInf.GetLen() ) )
            {
                if ( pSI && pSI->CountKashida() &&
                    pSI->KashidaJustify( pKernArray, 0, rInf.GetIdx(), rInf.GetLen(),
                                         nSpaceAdd ) != -1 )
                    nSpaceAdd = 0;
            }
        }

        
        if ( SW_CTL == nActual && nSpaceAdd )
        {
            LanguageType aLang = rInf.GetFont()->GetLanguage( SW_CTL );

            if ( LANGUAGE_THAI == aLang )
            {
                SwScriptInfo::ThaiJustify( rInf.GetText(), pKernArray, 0,
                                           rInf.GetIdx(), rInf.GetLen(),
                                           rInf.GetNumberOfBlanks(),
                                           rInf.GetSpace() );

                
                nSpaceAdd = 0;
            }
        }
    }

    long nLeft = 0;
    long nRight = 0;
    sal_Int32 nCnt = 0;
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

            for(sal_Int32 j = 0; j < rInf.GetLen(); j++)
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
    sal_Int32 nIdx = rInf.GetIdx();
    sal_Int32 nLastIdx = nIdx;
    const sal_Int32 nEnd = rInf.GetIdx() + rInf.GetLen();

    
    
    if ( g_pBreakIt->GetBreakIter().is() )
    {
        aLang = rInf.GetFont()->GetLanguage();
        bSkipCharacterCells = true;
    }

    while ( ( nRight < long( rInf.GetOfst() ) ) && ( nIdx < nEnd ) )
    {
        if ( nSpaceAdd && CH_BLANK == rInf.GetText()[ nIdx ] )
            nSpaceSum += nSpaceAdd;

        
        nLastIdx = nIdx;

        if ( bSkipCharacterCells )
        {
            nIdx = g_pBreakIt->GetBreakIter()->nextCharacters( rInf.GetText(),
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

    
    
    if ( nIdx > rInf.GetIdx() &&
         ( rInf.IsPosMatchesBounds() ||
           ( ( nRight > long( rInf.GetOfst() ) ) &&
             ( nRight - rInf.GetOfst() > rInf.GetOfst() - nLeft ) ) ) )
        nCnt = nLastIdx - rInf.GetIdx(); 
    else
        nCnt = nIdx - rInf.GetIdx(); 

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
                sal_uInt16 &rIndex, const void *pOwn, SwViewShell *pSh,
                sal_Bool bCheck ) :
  SwCacheAccess( *pFntCache, rMagic, rIndex ),
  pShell( pSh )
{
    
    if ( IsAvail() )
    {
        
        if ( !bCheck )
            return;

        
    }
    else
    {   
        bCheck = sal_False;
    }


    {
        OutputDevice* pOut = 0;
        sal_uInt16 nZoom = USHRT_MAX;

        
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
                return; 
            }
            pFntObj->Unlock(); 
            pObj = NULL;
        }

        
        
        pFntObj = pFntCache->First();
        while ( pFntObj && !( pFntObj->aFont == *(Font *)pOwn &&
                              pFntObj->GetZoom() == nZoom &&
                              pFntObj->GetPropWidth() ==
                              ((SwSubFont*)pOwn)->GetPropWidth() &&
                              ( !pFntObj->pPrinter || pFntObj->pPrinter == pOut ) ) )
            pFntObj = pFntCache->Next( pFntObj );

        if( pFntObj && pFntObj->pPrinter != pOut )
        {
            
            
            SwFntObj *pTmpObj = pFntObj;
            while( pTmpObj && !( pTmpObj->aFont == *(Font *)pOwn &&
                   pTmpObj->GetZoom()==nZoom && pTmpObj->pPrinter==pOut &&
                   pTmpObj->GetPropWidth() ==
                        ((SwSubFont*)pOwn)->GetPropWidth() ) )
                pTmpObj = pFntCache->Next( pTmpObj );
            if( pTmpObj )
                pFntObj = pTmpObj;
        }

        if ( !pFntObj ) 
        {
            
            
            SwCacheAccess::pOwner = pOwn;
            pFntObj = Get(); 
            OSL_ENSURE(pFntObj, "No Font, no Fun.");
        }
        else  
        {
            pFntObj->Lock();
            if (pFntObj->pPrinter != pOut) 
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

        
        
        
        rMagic = pFntObj->GetOwner();
        SwCacheAccess::pOwner = rMagic;
        rIndex = pFntObj->GetCachePos();
    }
}

SwCacheObj *SwFntAccess::NewObj( )
{
    
    return new SwFntObj( *(SwSubFont *)pOwner, ++pMagicNo, pShell );
}

extern sal_Int32 sw_CalcCaseMap( const SwFont& rFnt,
                                   const OUString& rOrigString,
                                   sal_Int32 nOfst,
                                   sal_Int32 nLen,
                                   sal_Int32 nIdx );

sal_Int32 SwFont::GetTxtBreak( SwDrawTextInfo& rInf, long nTextWidth )
{
    ChgFnt( rInf.GetShell(), rInf.GetOut() );

    const bool bCompress = rInf.GetKanaComp() && rInf.GetLen() &&
                           SW_CJK == GetActual() &&
                           rInf.GetScriptInfo() &&
                           rInf.GetScriptInfo()->CountCompChg() &&
                           lcl_IsMonoSpaceFont( rInf.GetOut() );

    OSL_ENSURE( !bCompress || ( rInf.GetScriptInfo() && rInf.GetScriptInfo()->
            CountCompChg()), "Compression without info" );

    sal_Int32 nTxtBreak = 0;
    long nKern = 0;

    sal_uInt16 nLn = ( rInf.GetLen() == COMPLETE_STRING ? rInf.GetText().getLength()
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
        sal_Int32 nTmpIdx;
        sal_Int32 nTmpLen;
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

            
            if ( bTitle && nLn )
            {
                
                if ( !g_pBreakIt->GetBreakIter()->isBeginWord(
                     rInf.GetText(), rInf.GetIdx(),
                     g_pBreakIt->GetLocale( aSub[nActual].GetLanguage() ),
                     i18n::WordType::ANYWORD_IGNOREWHITESPACES ) )
                {
                    
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

        if( rInf.GetHyphPos() ) {
            sal_Int32 nHyphPos = *rInf.GetHyphPos();
            nTxtBreak = rInf.GetOut().GetTextBreak( *pTmpText, nTextWidth,
                             static_cast<sal_Unicode>('-'), nHyphPos,
                             nTmpIdx, nTmpLen, nKern );
            *rInf.GetHyphPos() = (nHyphPos == -1) ? COMPLETE_STRING : nHyphPos;
        }
        else
            nTxtBreak = rInf.GetOut().GetTextBreak( *pTmpText, nTextWidth,
                                                    nTmpIdx, nTmpLen, nKern );

        if ( bTextReplaced && nTxtBreak != -1 )
        {
            if ( nTmpLen != nLn )
                nTxtBreak = sw_CalcCaseMap( *this, rInf.GetText(),
                                             rInf.GetIdx(), nLn, nTxtBreak );
            else
                nTxtBreak = nTxtBreak + rInf.GetIdx();
        }
    }

    sal_Int32 nTxtBreak2 = nTxtBreak == -1 ? COMPLETE_STRING : nTxtBreak;

    if ( ! bCompress )
        return nTxtBreak2;

    nTxtBreak2 = nTxtBreak2 - rInf.GetIdx();

    if( nTxtBreak2 < nLn )
    {
        if( !nTxtBreak2 && nLn )
            nLn = 1;
        else if( nLn > 2 * nTxtBreak2 )
            nLn = 2 * nTxtBreak2;
        sal_Int32 *pKernArray = new sal_Int32[ nLn ];
        rInf.GetOut().GetTextArray( rInf.GetText(), pKernArray,
                                    rInf.GetIdx(), nLn );
        if( rInf.GetScriptInfo()->Compress( pKernArray, rInf.GetIdx(), nLn,
                            rInf.GetKanaComp(), (sal_uInt16)GetHeight( nActual ) ) )
        {
            long nKernAdd = nKern;
            sal_Int32 nTmpBreak = nTxtBreak2;
            if( nKern && nTxtBreak2 )
                nKern *= nTxtBreak2 - 1;
            while( nTxtBreak2<nLn && nTextWidth >= pKernArray[nTxtBreak2] +nKern )
            {
                nKern += nKernAdd;
                ++nTxtBreak2;
            }
            if( rInf.GetHyphPos() )
                *rInf.GetHyphPos() += nTxtBreak2 - nTmpBreak; 
        }
        delete[] pKernArray;
    }
    nTxtBreak2 = nTxtBreak2 + rInf.GetIdx();

    return nTxtBreak2;
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
        
        
        
        

        bChgLineColor = ! bPrt && GetShell() &&
                GetShell()->GetAccessibilityOptions()->IsAlwaysAutoColor();

        bChgFntColor = COL_AUTO == rFnt.GetColor().GetColor() || bChgLineColor;

        if ( bChgFntColor )
        {
            
            const Color* pCol = GetFont() ? GetFont()->GetBackColor() : NULL;
            if( ! pCol || COL_TRANSPARENT == pCol->GetColor() )
            {
                const SvxBrushItem* pItem;
                const XFillStyleItem* pFillStyleItem;
                const XFillGradientItem* pFillGradientItem;
                SwRect aOrigBackRect;

                
                
                
                
                
                
                if( GetFrm()->GetBackgroundBrush( pItem, pFillStyleItem, pFillGradientItem, pCol, aOrigBackRect, sal_False ) )
                {
                    if ( !pCol )
                    {
                        pCol = &pItem->GetColor();
                    }

                    
                    
                    if ( pCol->GetColor() == COL_TRANSPARENT)
                        pCol = NULL;
                }
                else
                    pCol = NULL;
            }

            
            if ( ! pCol )
                pCol = &aGlobalRetoucheColor;

            if( GetShell() && GetShell()->GetWin() )
            {
                
                const SwViewOption* pViewOption = GetShell()->GetViewOptions();
                if(pViewOption->IsPagePreview() &&
                        !SW_MOD()->GetAccessibilityOptions().GetIsForPagePreviews())
                    nNewColor = COL_BLACK;
                else
                    
                    nNewColor = SwViewOption::GetFontColor().GetColor();
            }

            
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
                
                pFont->SetColor( aNewColor );
            }
            else if ( aNewColor != GetOut().GetFont().GetColor() )
            {
                
                Font aFont( rFnt );
                aFont.SetColor( aNewColor );
                GetOut().SetFont( aFont );
            }
        }

        
        if ( bChgLineColor )
        {
            
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
