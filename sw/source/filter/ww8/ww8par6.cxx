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

#include <stdlib.h>
#include <svl/itemiter.hxx>
#include <rtl/tencinfo.h>


#include <hintids.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/pgrditem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <filter/msfilter/sprmids.hxx>
#include <fmtpdsc.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <pagedesc.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <fchrfmt.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <fmtftntx.hxx>
#include <frmatr.hxx>
#include <section.hxx>
#include <lineinfo.hxx>
#include <fmtline.hxx>
#include <txatbase.hxx>
#include <fmtflcnt.hxx>
#include <fmtclbl.hxx>
#include <tgrditem.hxx>
#include <hfspacingitem.hxx>
#include <swtable.hxx>
#include <fltini.hxx>
#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include "ww8scan.hxx"
#include "ww8par2.hxx"
#include "ww8graf.hxx"


#include <fmtwrapinfluenceonobjpos.hxx>

using namespace sw::util;
using namespace sw::types;
using namespace ::com::sun::star;
using namespace nsHdFtFlags;





#define MM_250 1417             
#define MM_200 1134             


static sal_uInt8 lcl_ReadBorders(bool bVer67, WW8_BRC* brc, WW8PLCFx_Cp_FKP* pPap,
    const WW8RStyle* pSty = 0, const WW8PLCFx_SEPX* pSep = 0);


ColorData SwWW8ImplReader::GetCol(sal_uInt8 nIco)
{
    static const ColorData eSwWW8ColA[] =
    {
        COL_AUTO, COL_BLACK, COL_LIGHTBLUE, COL_LIGHTCYAN, COL_LIGHTGREEN,
        COL_LIGHTMAGENTA, COL_LIGHTRED, COL_YELLOW, COL_WHITE, COL_BLUE,
        COL_CYAN, COL_GREEN, COL_MAGENTA, COL_RED, COL_BROWN, COL_GRAY,
        COL_LIGHTGRAY
    };

    return eSwWW8ColA[nIco];
}

inline sal_uInt32 MSRoundTweak(sal_uInt32 x)
{
    return x;
}

/***************************************************************************
#  Seiten - Attribute, die nicht ueber die Attribut-Verwaltung, sondern
#   ueber ...->HasSprm abgearbeitet werden
#   ( ausser OLST, dass weiterhin ein normales Attribut ist )
#**************************************************************************/

static short ReadSprm( const WW8PLCFx_SEPX* pSep, sal_uInt16 nId, short nDefaultVal )
{
    const sal_uInt8* pS = pSep->HasSprm( nId );          
    short nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    return nVal;
}

static sal_uInt16 ReadUSprm( const WW8PLCFx_SEPX* pSep, sal_uInt16 nId, short nDefaultVal )
{
    const sal_uInt8* pS = pSep->HasSprm( nId );          
    sal_uInt16 nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    return nVal;
}

static sal_uInt8 ReadBSprm( const WW8PLCFx_SEPX* pSep, sal_uInt16 nId, sal_uInt8 nDefaultVal )
{
    const sal_uInt8* pS = pSep->HasSprm( nId );          
    sal_uInt8 nVal = pS ? *pS : nDefaultVal;
    return nVal;
}

void wwSection::SetDirection()
{
    
    switch (maSep.wTextFlow)
    {
        default:
            OSL_ENSURE(!this, "Unknown layout type");
        case 0:
            meDir=FRMDIR_HORI_LEFT_TOP;
            break;
        case 1:
            meDir=FRMDIR_VERT_TOP_RIGHT;
            break;
        case 2:
            
            
            
            
            meDir=FRMDIR_VERT_TOP_RIGHT;
            break;
        case 3:
            
            meDir=FRMDIR_VERT_TOP_RIGHT;
            break;
        case 4:
            
            meDir=FRMDIR_HORI_LEFT_TOP;
            break;
    }

    sal_uInt8 nRTLPgn = maSep.fBiDi;
    if ((meDir == FRMDIR_HORI_LEFT_TOP) && nRTLPgn)
        meDir = FRMDIR_HORI_RIGHT_TOP;
}

bool wwSection::IsVertical() const
{
    if (meDir == FRMDIR_VERT_TOP_RIGHT || meDir == FRMDIR_VERT_TOP_LEFT)
        return true;
    return false;
}

/*
  This is something of festering mapping, I'm open to better ways of doing it,
  but primarily the grid in writer is different to that in word. In writer the
  grid elements are squares with ruby rows inbetween. While in word there is no
  ruby stuff, and the elements are rectangles. By misusing the ruby row I can
  handle distortions in one direction, but its all a bit of a mess:
*/
void SwWW8ImplReader::SetDocumentGrid(SwFrmFmt &rFmt, const wwSection &rSection)
{
    if (bVer67)
        return;

    rFmt.SetFmtAttr(SvxFrameDirectionItem(rSection.meDir, RES_FRAMEDIR));

    SwTwips nTextareaHeight = rFmt.GetFrmSize().GetHeight();
    const SvxULSpaceItem &rUL = ItemGet<SvxULSpaceItem>(rFmt, RES_UL_SPACE);
    nTextareaHeight -= rUL.GetUpper();
    nTextareaHeight -= rUL.GetLower();

    SwTwips nTextareaWidth = rFmt.GetFrmSize().GetWidth();
    const SvxLRSpaceItem &rLR = ItemGet<SvxLRSpaceItem>(rFmt, RES_LR_SPACE);
    nTextareaWidth -= rLR.GetLeft();
    nTextareaWidth -= rLR.GetRight();

    if (rSection.IsVertical())
        std::swap(nTextareaHeight, nTextareaWidth);

    SwTextGridItem aGrid;
    aGrid.SetDisplayGrid(false);
    aGrid.SetPrintGrid(false);
    SwTextGrid eType=GRID_NONE;

    switch (rSection.maSep.clm)
    {
        case 0:
            eType = GRID_NONE;
            break;
        default:
            OSL_ENSURE(!this, "Unknown grid type");
        case 3:
            eType = GRID_LINES_CHARS;
            aGrid.SetSnapToChars(sal_True);
            break;
        case 1:
            eType = GRID_LINES_CHARS;
            aGrid.SetSnapToChars(sal_False);
            break;
        case 2:
            eType = GRID_LINES_ONLY;
            break;
    }

    aGrid.SetGridType(eType);

    
    
    if (eType != GRID_NONE)
        rDoc.set(IDocumentSettingAccess::ADD_EXT_LEADING, false);

    
    sal_Bool bSquaredMode = sal_False;
    rDoc.SetDefaultPageMode( bSquaredMode );
    aGrid.SetSquaredMode( bSquaredMode );

    
    sal_uInt32 nCharWidth=240;
    for (sal_uInt16 nI = 0; nI < pStyles->GetCount(); ++nI)
    {
        if (vColl[nI].bValid && vColl[nI].pFmt &&
            vColl[nI].GetWWStyleId() == 0)
        {
            nCharWidth = ItemGet<SvxFontHeightItem>(*(vColl[nI].pFmt),
                RES_CHRATR_CJK_FONTSIZE).GetHeight();
            break;
        }
    }

    
    if (rSection.maSep.dxtCharSpace)
    {
        sal_uInt32 nCharSpace = rSection.maSep.dxtCharSpace;
        
        sal_Int32 nMain = (nCharSpace & 0xFFFFF000);
        nMain/=0x1000;
        nCharWidth += nMain*20;

        int nFraction = (nCharSpace & 0x00000FFF);
        nFraction = (nFraction*20)/0xFFF;
        nCharWidth += nFraction;
    }

    aGrid.SetBaseWidth( writer_cast<sal_uInt16>(nCharWidth));

    
    sal_Int32 nLinePitch = rSection.maSep.dyaLinePitch;
    if (nLinePitch >= 1 && nLinePitch <= 31680)
    {
        aGrid.SetLines(writer_cast<sal_uInt16>(nTextareaHeight/nLinePitch));
        aGrid.SetBaseHeight(writer_cast<sal_uInt16>(nLinePitch));
    }

    sal_Int32 nRubyHeight = 0;
    aGrid.SetRubyHeight(writer_cast<sal_uInt16>(nRubyHeight));

    rFmt.SetFmtAttr(aGrid);
}

void SwWW8ImplReader::Read_ParaBiDi(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_FRAMEDIR);
    else
    {
        SvxFrameDirection eDir =
            *pData ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP;
        NewAttr(SvxFrameDirectionItem(eDir, RES_FRAMEDIR));
    }
}

bool wwSectionManager::SetCols(SwFrmFmt &rFmt, const wwSection &rSection,
    sal_uInt32 nNettoWidth) const
{
    
    const sal_Int16 nCols = rSection.NoCols();

    if (nCols < 2)          
        return false;

    SwFmtCol aCol;                      

    
    sal_Int32 nColSpace = rSection.StandardColSeparation();

    const SEPr& rSep = rSection.maSep;

    
    if (rSep.fLBetween)
    {
        aCol.SetLineAdj(COLADJ_TOP);      
        aCol.SetLineHeight(100);
        aCol.SetLineColor(Color(COL_BLACK));
        aCol.SetLineWidth(1);
    }

    aCol.Init(nCols, writer_cast<sal_uInt16>(nColSpace),
        writer_cast<sal_uInt16>(nNettoWidth));

    
    if (!rSep.fEvenlySpaced)
    {
        aCol._SetOrtho(false);
        const sal_uInt16 maxIdx = SAL_N_ELEMENTS(rSep.rgdxaColumnWidthSpacing);
        for (sal_uInt16 i = 0, nIdx = 1; i < nCols && nIdx < maxIdx; i++, nIdx+=2 )
        {
            SwColumn* pCol = &aCol.GetColumns()[i];
            const sal_Int32 nLeft = rSep.rgdxaColumnWidthSpacing[nIdx-1]/2;
            const sal_Int32 nRight = rSep.rgdxaColumnWidthSpacing[nIdx+1]/2;
            const sal_Int32 nWishWidth = rSep.rgdxaColumnWidthSpacing[nIdx]
                + nLeft + nRight;
            pCol->SetWishWidth(writer_cast<sal_uInt16>(nWishWidth));
            pCol->SetLeft(writer_cast<sal_uInt16>(nLeft));
            pCol->SetRight(writer_cast<sal_uInt16>(nRight));
        }
        aCol.SetWishWidth(writer_cast<sal_uInt16>(nNettoWidth));
    }
    rFmt.SetFmtAttr(aCol);
    return true;
}

void wwSectionManager::SetLeftRight(wwSection &rSection)
{
    
    sal_uInt32 nWWLe = MSRoundTweak(rSection.maSep.dxaLeft);
    sal_uInt32 nWWRi = MSRoundTweak(rSection.maSep.dxaRight);
    sal_uInt32 nWWGu = rSection.maSep.dzaGutter;

    /*
    fRTLGutter is set if the gutter is on the right, the gutter is otherwise
    placed on the left unless the global dop options are to put it on top, that
    case is handled in GetPageULData.
    */
    if (rSection.maSep.fRTLGutter)
        nWWRi += nWWGu;
    else if (!mrReader.pWDop->iGutterPos)
        nWWLe += nWWGu;

    
    if ((rSection.nPgWidth - nWWLe - nWWRi) < MINLAY)
    {
        /*
        There are some label templates which are "broken", they specify
        margins which make no sense e.g. Left 16.10cm, Right 16.10cm. So the
        space left between the margins is less than 0 In word the left margin
        is honoured and if the right margin would be past the left margin is
        left at the left margin position.

        Now this will work fine for importing, layout and exporting, *but* the
        page layout dialog has a hardcoded minimum page width of 0.5cm so it
        will report a different value than what is actually being used. i.e.
        it will add up the values to give a wider page than is actually being
        used.
        */
        nWWRi = rSection.nPgWidth - nWWLe - MINLAY;
    }

    rSection.nPgLeft = nWWLe;
    rSection.nPgRight = nWWRi;
}

void wwSectionManager::SetPage(SwPageDesc &rInPageDesc, SwFrmFmt &rFmt,
    const wwSection &rSection, bool bIgnoreCols) const
{
    
    rInPageDesc.SetLandscape(rSection.IsLandScape());

    
    SwFmtFrmSize aSz( rFmt.GetFrmSize() );
    aSz.SetWidth(rSection.GetPageWidth());
    aSz.SetHeight(SvxPaperInfo::GetSloppyPaperDimension(rSection.GetPageHeight()));
    rFmt.SetFmtAttr(aSz);

    rFmt.SetFmtAttr(
        SvxLRSpaceItem(rSection.GetPageLeft(), rSection.GetPageRight(), 0, 0, RES_LR_SPACE));

    if (!bIgnoreCols)
        SetCols(rFmt, rSection, rSection.GetTextAreaWidth());
}

static sal_uInt16 lcl_MakeSafeNegativeSpacing(sal_uInt16 nIn)
{
    if (nIn > SHRT_MAX)
        nIn = 0;
    return nIn;
}

void SwWW8ImplReader::SetPageBorder(SwFrmFmt &rFmt, const wwSection &rSection) const
{
    if (!IsBorder(rSection.brc))
        return;

    SfxItemSet aSet(rFmt.GetAttrSet());
    short aSizeArray[5]={0};
    SetFlyBordersShadow(aSet, rSection.brc, &aSizeArray[0]);
    SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(aSet, RES_LR_SPACE));
    SvxULSpaceItem aUL(ItemGet<SvxULSpaceItem>(aSet, RES_UL_SPACE));

    SvxBoxItem aBox(ItemGet<SvxBoxItem>(aSet, RES_BOX));
    short aOriginalBottomMargin = aBox.GetDistance(BOX_LINE_BOTTOM);

    if (rSection.maSep.pgbOffsetFrom == 1)
    {
        sal_uInt16 nDist;
        if (aBox.GetLeft())
        {
            nDist = aBox.GetDistance(BOX_LINE_LEFT);
            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aLR.GetLeft() - nDist)), BOX_LINE_LEFT);
            aSizeArray[WW8_LEFT] =
                aSizeArray[WW8_LEFT] - nDist + aBox.GetDistance(BOX_LINE_LEFT);
        }

        if (aBox.GetRight())
        {
            nDist = aBox.GetDistance(BOX_LINE_RIGHT);
            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aLR.GetRight() - nDist)), BOX_LINE_RIGHT);
            aSizeArray[WW8_RIGHT] =
                aSizeArray[WW8_RIGHT] - nDist + aBox.GetDistance(BOX_LINE_RIGHT);
        }

        if (aBox.GetTop())
        {
            nDist = aBox.GetDistance(BOX_LINE_TOP);
            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aUL.GetUpper() - nDist)), BOX_LINE_TOP);
            aSizeArray[WW8_TOP] =
                aSizeArray[WW8_TOP] - nDist + aBox.GetDistance(BOX_LINE_TOP);
        }

        if (aBox.GetBottom())
        {
            nDist = aBox.GetDistance(BOX_LINE_BOTTOM);
            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aUL.GetLower() - nDist)), BOX_LINE_BOTTOM);
            aSizeArray[WW8_BOT] =
                aSizeArray[WW8_BOT] - nDist + aBox.GetDistance(BOX_LINE_BOTTOM);
        }

        aSet.Put(aBox);
    }

    if (aBox.GetLeft())
        aLR.SetLeft(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aLR.GetLeft() - aSizeArray[WW8_LEFT])));
    if (aBox.GetRight())
        aLR.SetRight(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aLR.GetRight() - aSizeArray[WW8_RIGHT])));
    if (aBox.GetTop())
        aUL.SetUpper(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aUL.GetUpper() - aSizeArray[WW8_TOP])));
    if (aBox.GetBottom())
    {
        
        
        
        if(aUL.GetLower() != 0)
            aUL.SetLower(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aUL.GetLower() - aSizeArray[WW8_BOT])));
        else
            aUL.SetLower(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aOriginalBottomMargin - aSizeArray[WW8_BOT])));
    }

    aSet.Put(aLR);
    aSet.Put(aUL);
    rFmt.SetFmtAttr(aSet);
}

void wwSectionManager::GetPageULData(const wwSection &rSection,
    wwSectionManager::wwULSpaceData& rData) const
{
    sal_Int32 nWWUp = rSection.maSep.dyaTop;
    sal_Int32 nWWLo = rSection.maSep.dyaBottom;
    sal_uInt32 nWWHTop = rSection.maSep.dyaHdrTop;
    sal_uInt32 nWWFBot = rSection.maSep.dyaHdrBottom;

    /*
    If there is gutter in 97+ and the dop says put it on top then get the
    gutter distance and set it to the top margin. When we are "two pages
    in one" the gutter is put at the top of odd pages, and bottom of
    even pages, something we cannot do. So we will put it on top of all
    pages, that way the pages are at least the right size.
    */
    if (
         (!mrReader.bVer67 && mrReader.pWDop->iGutterPos &&
         rSection.maSep.fRTLGutter)
       )
    {
        nWWUp += rSection.maSep.dzaGutter;
    }

    rData.bHasHeader = (rSection.maSep.grpfIhdt &
        (WW8_HEADER_EVEN | WW8_HEADER_ODD | WW8_HEADER_FIRST)) != 0;

    if( rData.bHasHeader )
    {
        rData.nSwUp  = nWWHTop;             
        
        
        if ( nWWUp > 0 &&
             static_cast<sal_uInt32>(abs(nWWUp)) >= nWWHTop )
            rData.nSwHLo = nWWUp - nWWHTop;
        else
            rData.nSwHLo = 0;

        
        
        if (rData.nSwHLo < sal::static_int_cast< sal_uInt32 >(cMinHdFtHeight))
            rData.nSwHLo = sal::static_int_cast< sal_uInt32 >(cMinHdFtHeight);
    }
    else 
        rData.nSwUp = std::abs(nWWUp);

    rData.bHasFooter = (rSection.maSep.grpfIhdt &
        (WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST)) != 0;

    if( rData.bHasFooter )
    {
        rData.nSwLo = nWWFBot;              
        
        if ( nWWLo > 0 &&
             static_cast<sal_uInt32>(abs(nWWLo)) >= nWWFBot )
            rData.nSwFUp = nWWLo - nWWFBot;
        else
            rData.nSwFUp = 0;

        
        
        if (rData.nSwFUp < sal::static_int_cast< sal_uInt32 >(cMinHdFtHeight))
            rData.nSwFUp = sal::static_int_cast< sal_uInt32 >(cMinHdFtHeight);
    }
    else 
        rData.nSwLo = std::abs(nWWLo);
}

void wwSectionManager::SetPageULSpaceItems(SwFrmFmt &rFmt,
    wwSectionManager::wwULSpaceData& rData, const wwSection &rSection) const
{
    if (rData.bHasHeader)               
    {
        
        if (SwFrmFmt* pHdFmt = (SwFrmFmt*)rFmt.GetHeader().GetHeaderFmt())
        {
            SvxULSpaceItem aHdUL(pHdFmt->GetULSpace());
            if (!rSection.IsFixedHeightHeader())    
            {
                pHdFmt->SetFmtAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwHLo));
                
                
                aHdUL.SetLower( writer_cast<sal_uInt16>(rData.nSwHLo - cMinHdFtHeight) );
                pHdFmt->SetFmtAttr(SwHeaderAndFooterEatSpacingItem(
                    RES_HEADER_FOOTER_EAT_SPACING, true));
            }
            else
            {
                
                const SwTwips nHdLowerSpace( std::abs(rSection.maSep.dyaTop) - rData.nSwUp - rData.nSwHLo );
                pHdFmt->SetFmtAttr(SwFmtFrmSize(ATT_FIX_SIZE, 0, rData.nSwHLo + nHdLowerSpace));
                aHdUL.SetLower( static_cast< sal_uInt16 >(nHdLowerSpace) );
                pHdFmt->SetFmtAttr(SwHeaderAndFooterEatSpacingItem(
                    RES_HEADER_FOOTER_EAT_SPACING, false));
            }
            pHdFmt->SetFmtAttr(aHdUL);
        }
    }

    if (rData.bHasFooter)               
    {
        if (SwFrmFmt* pFtFmt = (SwFrmFmt*)rFmt.GetFooter().GetFooterFmt())
        {
            SvxULSpaceItem aFtUL(pFtFmt->GetULSpace());
            if (!rSection.IsFixedHeightFooter())    
            {
                pFtFmt->SetFmtAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwFUp));
                
                
                aFtUL.SetUpper( writer_cast<sal_uInt16>(rData.nSwFUp - cMinHdFtHeight) );
                pFtFmt->SetFmtAttr(SwHeaderAndFooterEatSpacingItem(
                    RES_HEADER_FOOTER_EAT_SPACING, true));
            }
            else
            {
                
                const SwTwips nFtUpperSpace( std::abs(rSection.maSep.dyaBottom) - rData.nSwLo - rData.nSwFUp );
                pFtFmt->SetFmtAttr(SwFmtFrmSize(ATT_FIX_SIZE, 0, rData.nSwFUp + nFtUpperSpace));
                aFtUL.SetUpper( static_cast< sal_uInt16 >(nFtUpperSpace) );
                pFtFmt->SetFmtAttr(SwHeaderAndFooterEatSpacingItem(
                    RES_HEADER_FOOTER_EAT_SPACING, false));
            }
            pFtFmt->SetFmtAttr(aFtUL);
        }
    }

    SvxULSpaceItem aUL(writer_cast<sal_uInt16>(rData.nSwUp),
        writer_cast<sal_uInt16>(rData.nSwLo), RES_UL_SPACE);
    rFmt.SetFmtAttr(aUL);
}

SwSectionFmt *wwSectionManager::InsertSection(
    SwPaM& rMyPaM, wwSection &rSection)
{
    SwSectionData aSection( CONTENT_SECTION,
            mrReader.rDoc.GetUniqueSectionName() );

    SfxItemSet aSet( mrReader.rDoc.GetAttrPool(), aFrmFmtSetRange );

    bool nRTLPgn = !maSegments.empty() && maSegments.back().IsBiDi();
    aSet.Put(SvxFrameDirectionItem(
        nRTLPgn ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));

    if (2 == mrReader.pWDop->fpc)
        aSet.Put( SwFmtFtnAtTxtEnd(FTNEND_ATTXTEND));
    if (0 == mrReader.pWDop->epc)
        aSet.Put( SwFmtEndAtTxtEnd(FTNEND_ATTXTEND));

    aSection.SetProtectFlag(SectionIsProtected(rSection));

    rSection.mpSection =
        mrReader.rDoc.InsertSwSection( rMyPaM, aSection, 0, & aSet );
    OSL_ENSURE(rSection.mpSection, "section not inserted!");
    if (!rSection.mpSection)
        return 0;

    SwPageDesc *pPage = 0;
    mySegrIter aEnd = maSegments.rend();
    for (mySegrIter aIter = maSegments.rbegin(); aIter != aEnd; ++aIter)
    {
        if (0 != (pPage = aIter->mpPage))
            break;
    }

    OSL_ENSURE(pPage, "no page outside this section!");

    if (!pPage)
        pPage = &mrReader.rDoc.GetPageDesc(0);

    if (!pPage)
        return 0;

    SwSectionFmt *pFmt = rSection.mpSection->GetFmt();
    OSL_ENSURE(pFmt, "impossible");
    if (!pFmt)
        return 0;

    SwFrmFmt& rFmt = pPage->GetMaster();
    const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
    long nPageLeft  = rLR.GetLeft();
    long nPageRight = rLR.GetRight();
    long nSectionLeft = rSection.GetPageLeft() - nPageLeft;
    long nSectionRight = rSection.GetPageRight() - nPageRight;
    if ((nSectionLeft != 0) || (nSectionRight != 0))
    {
        SvxLRSpaceItem aLR(nSectionLeft, nSectionRight, 0, 0, RES_LR_SPACE);
        pFmt->SetFmtAttr(aLR);
    }

    SetCols(*pFmt, rSection, rSection.GetTextAreaWidth());
    return pFmt;
}

void SwWW8ImplReader::HandleLineNumbering(const wwSection &rSection)
{
    
    if (mbNewDoc && rSection.maSep.nLnnMod)
    {
        
        bool bRestartLnNumPerSection = (1 == rSection.maSep.lnc);

        if (bNoLnNumYet)
        {
            SwLineNumberInfo aInfo( rDoc.GetLineNumberInfo() );

            aInfo.SetPaintLineNumbers(true);

            aInfo.SetRestartEachPage(rSection.maSep.lnc == 0);

            aInfo.SetPosFromLeft(writer_cast<sal_uInt16>(rSection.maSep.dxaLnn));

            
            aInfo.SetCountBy(rSection.maSep.nLnnMod);

            
            aInfo.SetCountBlankLines(true);
            aInfo.SetCountInFlys(false);
            aInfo.SetPos( LINENUMBER_POS_LEFT );
            SvxNumberType aNumType; 
            aInfo.SetNumType( aNumType );

            rDoc.SetLineNumberInfo( aInfo );
            bNoLnNumYet = false;
        }

        if (
            (0 < rSection.maSep.lnnMin) ||
            (bRestartLnNumPerSection && !bNoLnNumYet)
           )
        {
            SwFmtLineNumber aLN;
            if (const SwFmtLineNumber* pLN
                = (const SwFmtLineNumber*)GetFmtAttr(RES_LINENUMBER))
            {
                aLN.SetCountLines( pLN->IsCount() );
            }
            aLN.SetStartValue(1 + rSection.maSep.lnnMin);
            NewAttr(aLN);
            pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_LINENUMBER);
        }
        bNoLnNumYet = false;
    }
}

wwSection::wwSection(const SwPosition &rPos) : maStart(rPos.nNode),
    mpSection(0), mpPage(0), meDir(FRMDIR_HORI_LEFT_TOP),
    nPgWidth(SvxPaperInfo::GetPaperSize(PAPER_A4).Width()),
    nPgLeft(MM_250), nPgRight(MM_250), mnBorders(0), mbHasFootnote(false)
{
}

void wwSectionManager::SetNumberingType(const wwSection &rNewSection,
    SwPageDesc &rPageDesc) const
{
    
    static const SvxExtNumType aNumTyp[5] =
    {
        SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N
    };

    SvxNumberType aType;
    aType.SetNumberingType( static_cast< sal_Int16 >(aNumTyp[rNewSection.maSep.nfcPgn]) );
    rPageDesc.SetNumType(aType);
}






void wwSectionManager::CreateSep(const long nTxtPos, bool /*bMustHaveBreak*/)
{
    /*
    #i1909# section/page breaks should not occur in tables or subpage
    elements like frames. Word itself ignores them in this case. The bug is
    more likely that this filter created such documents in the past!
    */
    if (mrReader.nInTable || mrReader.bTxbxFlySection || mrReader.InLocalApo())
        return;

    WW8PLCFx_SEPX* pSep = mrReader.pPlcxMan->GetSepPLCF();
    OSL_ENSURE(pSep, "impossible!");
    if (!pSep)
        return;

    ww::WordVersion eVer = mrReader.GetFib().GetFIBVersion();

    
    
    WW8PLCFspecial* pWkb = mrReader.pPlcxMan->GetWkbPLCF();
    if (pWkb && pWkb->SeekPosExact(nTxtPos) &&
            pWkb->Where() == nTxtPos)
    {
        void* pData;
        WW8_CP nTest;
        pWkb->Get(nTest, pData);
        OUString sSectionName = mrReader.aLinkStringMap[SVBT16ToShort( ((WW8_WKB*)pData)->nLinkId) ];
        sSectionName = mrReader.ConvertFFileName(sSectionName);
        SwSectionData aSection(FILE_LINK_SECTION, sSectionName);
        aSection.SetLinkFileName( sSectionName );
        aSection.SetProtectFlag(true);
        
        mrReader.rDoc.InsertSwSection(*mrReader.pPaM, aSection, 0, 0, false);
    }

    wwSection aLastSection(*mrReader.pPaM->GetPoint());
    if (!maSegments.empty())
        aLastSection = maSegments.back();

    
    sal_uInt16 nLIdx = ( ( mrReader.pWwFib->lid & 0xff ) == 0x9 ) ? 1 : 0;

    
    wwSection aNewSection(*mrReader.pPaM->GetPoint());

    static const sal_uInt16 aVer2Ids0[] =
    {
        /*sprmSBkc*/           117,
        /*sprmSFTitlePage*/    118,
        /*sprmSNfcPgn*/        122,
        /*sprmSCcolumns*/      119,
        /*sprmSDxaColumns*/    120,
        /*sprmSLBetween*/      133
    };

    static const sal_uInt16 aVer67Ids0[] =
    {
        /*sprmSBkc*/           142,
        /*sprmSFTitlePage*/    143,
        /*sprmSNfcPgn*/        147,
        /*sprmSCcolumns*/      144,
        /*sprmSDxaColumns*/    145,
        /*sprmSLBetween*/      158
    };

    static const sal_uInt16 aVer8Ids0[] =
    {
        /*sprmSBkc*/           0x3009,
        /*sprmSFTitlePage*/    0x300A,
        /*sprmSNfcPgn*/        0x300E,
        /*sprmSCcolumns*/      0x500B,
        /*sprmSDxaColumns*/    0x900C,
        /*sprmSLBetween*/      0x3019
    };

    const sal_uInt16* pIds = eVer <= ww::eWW2 ? aVer2Ids0 : eVer <= ww::eWW7 ? aVer67Ids0 : aVer8Ids0;

    if (!maSegments.empty())
    {
        
        
        
        
        
        
        if (const sal_uInt8* pSprmBkc = pSep->HasSprm(pIds[0]))
            aNewSection.maSep.bkc = *pSprmBkc;
    }

    
    aNewSection.maSep.fTitlePage =
        (0 != ReadBSprm( pSep, pIds[1], 0 ));

    
    aNewSection.maSep.nfcPgn = ReadBSprm( pSep, pIds[2], 0 );
    if (aNewSection.maSep.nfcPgn > 4)
        aNewSection.maSep.nfcPgn = 0;

    aNewSection.maSep.fUnlocked = eVer > ww::eWW2 ? ReadBSprm(pSep, (eVer <= ww::eWW7 ? 139 : 0x3006), 0 ) : 0;

    
    aNewSection.maSep.fBiDi = eVer >= ww::eWW8 ? ReadBSprm(pSep, 0x3228, 0) : 0;

    aNewSection.maSep.ccolM1 = ReadSprm(pSep, pIds[3], 0 );

    
    aNewSection.maSep.dxaColumns = ReadUSprm( pSep, pIds[4], 708 );

    
    aNewSection.maSep.fLBetween = ReadBSprm(pSep, pIds[5], 0 );

    if (eVer >= ww::eWW6)
    {
        
        aNewSection.maSep.fEvenlySpaced =
            sal_uInt8(ReadBSprm(pSep, (eVer <= ww::eWW7 ? 138 : 0x3005), 1) ? true : false);

        const sal_uInt8 numrgda = SAL_N_ELEMENTS(aNewSection.maSep.rgdxaColumnWidthSpacing);
        if (aNewSection.maSep.ccolM1 > 0 && !aNewSection.maSep.fEvenlySpaced)
        {
            aNewSection.maSep.rgdxaColumnWidthSpacing[0] = 0;
            int nCols = aNewSection.maSep.ccolM1 + 1;
            int nIdx = 0;
            for (int i = 0; i < nCols; ++i)
            {
                
                const sal_uInt8* pSW = pSep->HasSprm( (eVer <= ww::eWW7 ? 136 : 0xF203), sal_uInt8( i ) );

                OSL_ENSURE( pSW, "+Sprm 136 (bzw. 0xF203) (ColWidth) fehlt" );
                sal_uInt16 nWidth = pSW ? SVBT16ToShort(pSW + 1) : 1440;

                if (++nIdx < numrgda)
                    aNewSection.maSep.rgdxaColumnWidthSpacing[nIdx] = nWidth;

                if (i < nCols-1)
                {
                    
                    const sal_uInt8* pSD = pSep->HasSprm( (eVer <= ww::eWW7 ? 137 : 0xF204), sal_uInt8( i ) );

                    OSL_ENSURE( pSD, "+Sprm 137 (bzw. 0xF204) (Colspacing) fehlt" );
                    if( pSD )
                    {
                        nWidth = SVBT16ToShort(pSD + 1);
                        if (++nIdx < numrgda)
                            aNewSection.maSep.rgdxaColumnWidthSpacing[nIdx] = nWidth;
                    }
                }
            }
        }
    }

    static const sal_uInt16 aVer2Ids1[] =
    {
        /*sprmSBOrientation*/   137,
        /*sprmSXaPage*/         139,
        /*sprmSYaPage*/         140,
        /*sprmSDxaLeft*/        141,
        /*sprmSDxaRight*/       142,
        /*sprmSDzaGutter*/      145,
        /*sprmSFPgnRestart*/    125,
        /*sprmSPgnStart*/       136,
        /*sprmSDmBinFirst*/     115,
        /*sprmSDmBinOther*/     116
    };

    static const sal_uInt16 aVer67Ids1[] =
    {
        /*sprmSBOrientation*/   162,
        /*sprmSXaPage*/         164,
        /*sprmSYaPage*/         165,
        /*sprmSDxaLeft*/        166,
        /*sprmSDxaRight*/       167,
        /*sprmSDzaGutter*/      170,
        /*sprmSFPgnRestart*/    150,
        /*sprmSPgnStart*/       161,
        /*sprmSDmBinFirst*/     140,
        /*sprmSDmBinOther*/     141
    };

    static const sal_uInt16 aVer8Ids1[] =
    {
        /*sprmSBOrientation*/   0x301d,
        /*sprmSXaPage*/         0xB01F,
        /*sprmSYaPage*/         0xB020,
        /*sprmSDxaLeft*/        0xB021,
        /*sprmSDxaRight*/       0xB022,
        /*sprmSDzaGutter*/      0xB025,
        /*sprmSFPgnRestart*/    0x3011,
        /*sprmSPgnStart*/       0x501C,
           /*sprmSDmBinFirst*/     0x5007,
        /*sprmSDmBinOther*/     0x5008
    };

    pIds = eVer <= ww::eWW2 ? aVer2Ids1 : eVer <= ww::eWW7 ? aVer67Ids1 : aVer8Ids1;

                                            
    aNewSection.maSep.dmOrientPage = ReadBSprm(pSep, pIds[0], 0);

    
    aNewSection.maSep.xaPage = ReadUSprm(pSep, pIds[1], lLetterWidth);
    aNewSection.nPgWidth = SvxPaperInfo::GetSloppyPaperDimension(aNewSection.maSep.xaPage);

    aNewSection.maSep.yaPage = ReadUSprm(pSep, pIds[2], lLetterHeight);

    
    static const sal_uInt16 nLef[] = { MM_250, 1800 };
    static const sal_uInt16 nRig[] = { MM_250, 1800 };

    aNewSection.maSep.dxaLeft = ReadUSprm( pSep, pIds[3], nLef[nLIdx]);
    aNewSection.maSep.dxaRight = ReadUSprm( pSep, pIds[4], nRig[nLIdx]);

    
    
    
    
    if(mrReader.pWDop->doptypography.f2on1 &&
            aNewSection.maSep.dmOrientPage == 2)
        std::swap(aNewSection.maSep.dxaLeft, aNewSection.maSep.dxaRight);

    aNewSection.maSep.dzaGutter = ReadUSprm( pSep, pIds[5], 0);

    aNewSection.maSep.fRTLGutter = static_cast< sal_uInt8 >(eVer >= ww::eWW8 ? ReadUSprm( pSep, 0x322A, 0 ) : 0);

    
    aNewSection.maSep.fPgnRestart = ReadBSprm(pSep, pIds[6], 0);

    aNewSection.maSep.pgnStart = ReadBSprm( pSep, pIds[7], 0 );

    if (eVer >= ww::eWW6)
    {
        if (const sal_uInt8* p = pSep->HasSprm( (eVer <= ww::eWW7 ? 132 : 0x3001) ))
            aNewSection.maSep.iHeadingPgn = *p;

        if (const sal_uInt8* p = pSep->HasSprm( (eVer <= ww::eWW7 ? 131 : 0x3000) ))
            aNewSection.maSep.cnsPgn = *p;
    }

    if(const sal_uInt8* pSprmSDmBinFirst = pSep->HasSprm( pIds[8] ))
        aNewSection.maSep.dmBinFirst = *pSprmSDmBinFirst;

    if (const sal_uInt8* pSprmSDmBinOther = pSep->HasSprm( pIds[9] ))
        aNewSection.maSep.dmBinOther = *pSprmSDmBinOther;

    static const sal_uInt16 nTop[] = { MM_250, 1440 };
    static const sal_uInt16 nBot[] = { MM_200, 1440 };

    static const sal_uInt16 aVer2Ids2[] =
    {
        /*sprmSDyaTop*/         143,
        /*sprmSDyaBottom*/      144,
        /*sprmSDyaHdrTop*/      131,
        /*sprmSDyaHdrBottom*/   132,
        /*sprmSNLnnMod*/        129,
        /*sprmSLnc*/            127,
        /*sprmSDxaLnn*/         130,
        /*sprmSLnnMin*/         135
    };

    static const sal_uInt16 aVer67Ids2[] =
    {
        /*sprmSDyaTop*/         168,
        /*sprmSDyaBottom*/      169,
        /*sprmSDyaHdrTop*/      156,
        /*sprmSDyaHdrBottom*/   157,
        /*sprmSNLnnMod*/        154,
        /*sprmSLnc*/            152,
        /*sprmSDxaLnn*/         155,
        /*sprmSLnnMin*/         160
    };
    static const sal_uInt16 aVer8Ids2[] =
    {
        /*sprmSDyaTop*/         0x9023,
        /*sprmSDyaBottom*/      0x9024,
        /*sprmSDyaHdrTop*/      0xB017,
        /*sprmSDyaHdrBottom*/   0xB018,
        /*sprmSNLnnMod*/        0x5015,
        /*sprmSLnc*/            0x3013,
        /*sprmSDxaLnn*/         0x9016,
        /*sprmSLnnMin*/         0x501B
    };

    pIds = eVer <= ww::eWW2 ? aVer2Ids2 : eVer <= ww::eWW7 ? aVer67Ids2 : aVer8Ids2;

    aNewSection.maSep.dyaTop = ReadSprm( pSep, pIds[0], nTop[nLIdx] );
    aNewSection.maSep.dyaBottom = ReadSprm( pSep, pIds[1], nBot[nLIdx] );
    aNewSection.maSep.dyaHdrTop = ReadUSprm( pSep, pIds[2], 720 );
    aNewSection.maSep.dyaHdrBottom = ReadUSprm( pSep, pIds[3], 720 );

    if (eVer >= ww::eWW8)
    {
        aNewSection.maSep.wTextFlow = ReadUSprm(pSep, 0x5033, 0);
        aNewSection.maSep.clm = ReadUSprm( pSep, 0x5032, 0 );
        aNewSection.maSep.dyaLinePitch = ReadUSprm(pSep, 0x9031, 360);
        if (const sal_uInt8* pS = pSep->HasSprm(0x7030))
            aNewSection.maSep.dxtCharSpace = SVBT32ToUInt32(pS);

        
        sal_uInt16 pgbProp = ReadSprm( pSep, 0x522F, 0 );
        aNewSection.maSep.pgbApplyTo = pgbProp & 0x0007;
        aNewSection.maSep.pgbPageDepth = (pgbProp & 0x0018) >> 3;
        aNewSection.maSep.pgbOffsetFrom = (pgbProp & 0x00E0) >> 5;

        aNewSection.mnBorders =
            ::lcl_ReadBorders(eVer <= ww::eWW7, aNewSection.brc, 0, 0, pSep);
    }

    
    if (const sal_uInt8* pSprmSNLnnMod = pSep->HasSprm( pIds[4] ))
        aNewSection.maSep.nLnnMod = *pSprmSNLnnMod;

    if (const sal_uInt8* pSprmSLnc = pSep->HasSprm( pIds[5] ))
        aNewSection.maSep.lnc = *pSprmSLnc;

    if (const sal_uInt8* pSprmSDxaLnn = pSep->HasSprm( pIds[6] ))
        aNewSection.maSep.dxaLnn = SVBT16ToShort( pSprmSDxaLnn );

    if (const sal_uInt8* pSprmSLnnMin = pSep->HasSprm( pIds[7] ))
        aNewSection.maSep.lnnMin = *pSprmSLnnMin;

    if (eVer <= ww::eWW7)
        aNewSection.maSep.grpfIhdt = ReadBSprm(pSep, eVer <= ww::eWW2 ? 128 : 153, 0);
    else if (mrReader.pHdFt)
    {
        aNewSection.maSep.grpfIhdt = WW8_HEADER_ODD | WW8_FOOTER_ODD
            | WW8_HEADER_FIRST | WW8_FOOTER_FIRST;

        
        
        
        
        

        if (mrReader.pWDop->fFacingPages)
            aNewSection.maSep.grpfIhdt |= WW8_HEADER_EVEN | WW8_FOOTER_EVEN;

        
        
        for (int nI = 0, nMask = 1; nI < 6; ++nI, nMask <<= 1)
        {
            if (aNewSection.maSep.grpfIhdt & nMask)
            {
                WW8_CP nStart;
                long nLen;
                mrReader.pHdFt->GetTextPosExact( static_cast< short >(nI + ( maSegments.size() + 1) * 6), nStart, nLen);
                
                
                if (!nLen)
                {
                    if (
                        maSegments.empty() ||
                        !(maSegments.back().maSep.grpfIhdt & nMask)
                       )
                    {
                        aNewSection.maSep.grpfIhdt &= ~nMask;
                    }
                }
            }
        }
    }

    SetLeftRight(aNewSection);
    

    if (eVer >= ww::eWW8)
        aNewSection.SetDirection();

    mrReader.HandleLineNumbering(aNewSection);
    maSegments.push_back(aNewSection);
}

void SwWW8ImplReader::CopyPageDescHdFt(const SwPageDesc* pOrgPageDesc,
    SwPageDesc* pNewPageDesc, sal_uInt8 nCode )
{
    
    if( nCode & WW8_HEADER_ODD )
    {
        rDoc.CopyHeader(pOrgPageDesc->GetMaster(),
                        pNewPageDesc->GetMaster() );
    }
    
    if( nCode & WW8_FOOTER_ODD )
    {
        rDoc.CopyFooter(pOrgPageDesc->GetMaster(),
                        pNewPageDesc->GetMaster());
    }
    
    if( nCode & WW8_HEADER_EVEN )
    {
        rDoc.CopyHeader(pOrgPageDesc->GetLeft(),
                        pNewPageDesc->GetLeft());
    }
    
    if( nCode & WW8_FOOTER_EVEN )
    {
        rDoc.CopyFooter(pOrgPageDesc->GetLeft(),
                        pNewPageDesc->GetLeft());
    }
    
    if( nCode & WW8_HEADER_FIRST )
    {
        rDoc.CopyHeader(pOrgPageDesc->GetFirstMaster(),
                        pNewPageDesc->GetFirstMaster());
    }
    
    if( nCode & WW8_FOOTER_FIRST )
    {
        rDoc.CopyFooter(pOrgPageDesc->GetFirstMaster(),
                        pNewPageDesc->GetFirstMaster());
    }
}





static bool _SetWW8_BRC(bool bVer67, WW8_BRC& rVar, const sal_uInt8* pS)
{
    if( pS )
    {
        if( bVer67 )
            memcpy( rVar.aBits1, pS, sizeof( SVBT16 ) );
        else
            rVar = *((WW8_BRC*)pS);
    }

    return 0 != pS;
}

static sal_uInt8 lcl_ReadBorders(bool bVer67, WW8_BRC* brc, WW8PLCFx_Cp_FKP* pPap,
    const WW8RStyle* pSty, const WW8PLCFx_SEPX* pSep)
{




    sal_uInt8 nBorder = 0;
    if( pSep )
    {
        if( !bVer67 )
        {
             sal_uInt8* pSprm[4];

            
             if( pSep->Find4Sprms(  0x702B,   0x702C,   0x702D,   0x702E,
                                    pSprm[0], pSprm[1], pSprm[2], pSprm[3] ) )
             {
                for( int i = 0; i < 4; ++i )
                    nBorder |= int(_SetWW8_BRC( bVer67, brc[ i ], pSprm[ i ] ))<<i;
             }
        }
    }
    else
    {

        static const sal_uInt16 aVer67Ids[5] = { 38, 39, 40, 41, 42 };

        static const sal_uInt16 aVer8Ids[5] =
                { 0x6424, 0x6425, 0x6426, 0x6427, 0x6428 };

        const sal_uInt16* pIds = bVer67 ? aVer67Ids : aVer8Ids;

        if( pPap )
        {
            for( int i = 0; i < 5; ++i, ++pIds )
                nBorder |= int(_SetWW8_BRC( bVer67, brc[ i ], pPap->HasSprm( *pIds )))<<i;
        }
        else if( pSty )
        {
            for( int i = 0; i < 5; ++i, ++pIds )
                nBorder |= int(_SetWW8_BRC( bVer67, brc[ i ], pSty->HasParaSprm( *pIds )))<<i;
        }
        else {
            OSL_ENSURE( pSty || pPap, "WW8PLCFx_Cp_FKP and WW8RStyle "
                               "and WW8PLCFx_SEPX is 0" );
        }
    }

    return nBorder;
}

void GetLineIndex(SvxBoxItem &rBox, short nLineThickness, short nSpace, sal_uInt8 nCol, short nIdx,
    sal_uInt16 nOOIndex, sal_uInt16 nWWIndex, short *pSize=0)
{
    ::editeng::SvxBorderStyle const eStyle(
            ::editeng::ConvertBorderStyleFromWord(nIdx));

    ::editeng::SvxBorderLine aLine;
    aLine.SetBorderLineStyle( eStyle );
    double const fConverted( (table::BorderLineStyle::NONE == eStyle) ? 0.0 :
        ::editeng::ConvertBorderWidthFromWord(eStyle, nLineThickness, nIdx));
    aLine.SetWidth(fConverted);

    
    if (nCol == 0)
        nCol = 1;

    aLine.SetColor(SwWW8ImplReader::GetCol(nCol));

    if (pSize)
        pSize[nWWIndex] = fConverted + nSpace;

    rBox.SetLine(&aLine, nOOIndex);
    rBox.SetDistance(nSpace, nOOIndex);

}

void Set1Border(bool bVer67, SvxBoxItem &rBox, const WW8_BRC& rBor,
    sal_uInt16 nOOIndex, sal_uInt16 nWWIndex, short *pSize, const bool bIgnoreSpace)
{
    sal_uInt8 nCol;
    short nSpace, nIdx;
    short nLineThickness = rBor.DetermineBorderProperties(bVer67,&nSpace,&nCol,
        &nIdx);

    GetLineIndex(rBox, nLineThickness, bIgnoreSpace ? 0 : nSpace, nCol, nIdx, nOOIndex, nWWIndex, pSize );

}

static bool lcl_IsBorder(bool bVer67, const WW8_BRC* pbrc, bool bChkBtwn = false)
{
    if( bVer67  )
        return ( pbrc[WW8_TOP  ].aBits1[0] & 0x18 ) ||  
               ( pbrc[WW8_LEFT ].aBits1[0] & 0x18 ) ||
               ( pbrc[WW8_BOT  ].aBits1[0] & 0x18 ) ||
               ( pbrc[WW8_RIGHT].aBits1[0] & 0x18 ) ||
               ( bChkBtwn && ( pbrc[WW8_BETW ].aBits1[0] )) ||
               
               ( (pbrc[WW8_TOP  ].aBits1[0] & 0x07)+1 > 6) ||
               ( (pbrc[WW8_LEFT ].aBits1[0] & 0x07)+1 > 6) ||
               ( (pbrc[WW8_BOT  ].aBits1[0] & 0x07)+1 > 6) ||
               ( (pbrc[WW8_RIGHT].aBits1[0] & 0x07)+1 > 6) ||
               ( bChkBtwn && ( (pbrc[WW8_BETW ].aBits1[0] & 0x07)+1 > 6))
               ;
                
                
    else
        return pbrc[WW8_TOP  ].aBits1[1] ||         
               pbrc[WW8_LEFT ].aBits1[1] ||
               pbrc[WW8_BOT  ].aBits1[1] ||
               pbrc[WW8_RIGHT].aBits1[1] ||
               (bChkBtwn && pbrc[WW8_BETW ].aBits1[1]);
}

bool SwWW8ImplReader::IsBorder(const WW8_BRC* pbrc, bool bChkBtwn) const
{
    return lcl_IsBorder(bVer67, pbrc, bChkBtwn);
}

bool WW8_BRC::IsEmpty(bool bVer67) const
{
    return (IsBlank() || IsZeroed(bVer67));
}

bool WW8_BRC::IsBlank() const
{
    return (aBits1[0] == 0xff && aBits1[1] == 0xff);
}

bool WW8_BRC::IsZeroed(bool bVer67) const
{
    return (!(bVer67 ? (aBits1[0] & 0x001f) : aBits1[1]));
}

bool SwWW8ImplReader::SetBorder(SvxBoxItem& rBox, const WW8_BRC* pbrc,
    short *pSizeArray, sal_uInt8 nSetBorders) const
{
    bool bChange = false;
    static const sal_uInt16 aIdArr[ 10 ] =
    {
        WW8_TOP,    BOX_LINE_TOP,
        WW8_LEFT,   BOX_LINE_LEFT,
        WW8_RIGHT,  BOX_LINE_RIGHT,
        WW8_BOT,    BOX_LINE_BOTTOM,
        WW8_BETW,   BOX_LINE_BOTTOM
    };

    for( int i = 0, nEnd = 8; i < nEnd; i += 2 )
    {
        
        const WW8_BRC& rB = pbrc[ aIdArr[ i ] ];
        if( !rB.IsEmpty(bVer67))
        {
            Set1Border(bVer67, rBox, rB, aIdArr[i+1], aIdArr[i], pSizeArray, false);
            bChange = true;
        }
        else if ( nSetBorders & (1 << aIdArr[i]) )
        {
            /*
            ##826##, ##653##

            If a style has borders set,and the para attributes attempt remove
            the borders, then this is perfectably acceptable, so we shouldn't
            ignore this blank entry

            nSetBorders has a bit set for each location that a sprm set a
            border, so with a sprm set, but no border, then disable the
            appropriate border
            */
            rBox.SetLine( 0, aIdArr[ i+1 ] );
        }
    }
    return bChange;
}


bool SwWW8ImplReader::SetShadow(SvxShadowItem& rShadow, const short *pSizeArray,
    const WW8_BRC& aRightBrc) const
{
    bool bRet = (
                ( bVer67 ? (aRightBrc.aBits1[ 0 ] & 0x20 )
                         : (aRightBrc.aBits2[ 1 ] & 0x20 ) )
                && (pSizeArray && pSizeArray[WW8_RIGHT])
                );
    if (bRet)
    {
        rShadow.SetColor(Color(COL_BLACK));
    
        short nVal = aRightBrc.DetermineBorderProperties(bVer67);
    
        if (nVal < 0x10)
            nVal = 0x10;
        rShadow.SetWidth(nVal);
        rShadow.SetLocation(SVX_SHADOW_BOTTOMRIGHT);
        bRet = true;
    }
    return bRet;
}

void SwWW8ImplReader::GetBorderDistance(const WW8_BRC* pbrc,
    Rectangle& rInnerDist) const
{
    
    if (bVer67)
    {
        rInnerDist = Rectangle(((pbrc[ 1 ].aBits1[1] >> 3) & 0x1f) * 20,
                               ((pbrc[ 0 ].aBits1[1] >> 3) & 0x1f) * 20,
                               ((pbrc[ 3 ].aBits1[1] >> 3) & 0x1f) * 20,
                               ((pbrc[ 2 ].aBits1[1] >> 3) & 0x1f) * 20 );
    }
    else
    {
        rInnerDist = Rectangle( (pbrc[ 1 ].aBits2[1]       & 0x1f) * 20,
                                (pbrc[ 0 ].aBits2[1]       & 0x1f) * 20,
                                (pbrc[ 3 ].aBits2[1]       & 0x1f) * 20,
                                (pbrc[ 2 ].aBits2[1]       & 0x1f) * 20 );
    }
}


bool SwWW8ImplReader::SetFlyBordersShadow(SfxItemSet& rFlySet,
    const WW8_BRC *pbrc, short *pSizeArray) const
{
    bool bShadowed = false;
    if (IsBorder(pbrc))
    {
        SvxBoxItem aBox( RES_BOX );
        SetBorder(aBox, pbrc, pSizeArray);

        rFlySet.Put( aBox );

        
        SvxShadowItem aShadow( RES_SHADOW );
        if( SetShadow( aShadow, pSizeArray, pbrc[WW8_RIGHT] ))
        {
            bShadowed = true;
            rFlySet.Put( aShadow );
        }
    }
    return bShadowed;
}




                            
#define MAX_BORDER_SIZE 210         
#define MAX_EMPTY_BORDER 10         

static void FlySecur1(short& rSize, const bool bBorder)
{
    short nMin = MINFLY +
        (bBorder ? MAX_BORDER_SIZE : MAX_EMPTY_BORDER);

    if ( rSize < nMin )
        rSize = nMin;
}

inline bool SetValSprm( sal_Int16* pVar, WW8PLCFx_Cp_FKP* pPap, sal_uInt16 nId )
{
    const sal_uInt8* pS = pPap->HasSprm( nId );
    if( pS )
        *pVar = (sal_Int16)SVBT16ToShort( pS );
    return ( pS != 0 );
}

inline bool SetValSprm( sal_Int16* pVar, const WW8RStyle* pStyle, sal_uInt16 nId )
{
    const sal_uInt8* pS = pStyle->HasParaSprm( nId );
    if( pS )
        *pVar = (sal_Int16)SVBT16ToShort( pS );
    return ( pS != 0 );
}

/*
#i1930 revealed that sprm 0x360D as used in tables can affect the frame
around the table. Its full structure is not fully understood as yet.
*/
void WW8FlyPara::ApplyTabPos(const WW8_TablePos *pTabPos)
{
    if (pTabPos)
    {
        nSp26 = pTabPos->nSp26;
        nSp27 = pTabPos->nSp27;
        nSp29 = pTabPos->nSp29;
        nLeMgn = pTabPos->nLeMgn;
        nRiMgn = pTabPos->nRiMgn;
        nUpMgn = pTabPos->nUpMgn;
        nLoMgn = pTabPos->nLoMgn;
        nSp37 = pTabPos->nSp37;
    }
}

WW8FlyPara::WW8FlyPara(bool bIsVer67, const WW8FlyPara* pSrc /* = 0 */)
{
    if ( pSrc )
        memcpy( this, pSrc, sizeof( WW8FlyPara ) ); 
    else
    {
        memset( this, 0, sizeof( WW8FlyPara ) );    
        nSp37 = 2;                                  
    }
    bVer67 = bIsVer67;
}

bool WW8FlyPara::operator==(const WW8FlyPara& rSrc) const
{
    /*
     Compare the parts that word seems to compare for equivalence.
     Interestingly being autoheight or absolute height (the & 0x7fff) doesn't
     matter to word
    */
    return
       (
         (nSp26 == rSrc.nSp26) &&
         (nSp27 == rSrc.nSp27) &&
         ((nSp45 & 0x7fff) == (rSrc.nSp45 & 0x7fff)) &&
         (nSp28 == rSrc.nSp28) &&
         (nLeMgn == rSrc.nLeMgn) &&
         (nRiMgn == rSrc.nRiMgn) &&
         (nUpMgn == rSrc.nUpMgn) &&
         (nLoMgn == rSrc.nLoMgn) &&
         (nSp29 == rSrc.nSp29) &&
         (nSp37 == rSrc.nSp37)
       );
}


void WW8FlyPara::Read(sal_uInt8 nOrigSp29, WW8PLCFx_Cp_FKP* pPap)
{
    const sal_uInt8* pS = 0;
    if( bVer67 )
    {
        SetValSprm( &nSp26, pPap, 26 ); 
        
        mbVertSet |= SetValSprm( &nSp27, pPap, 27 );    
        SetValSprm( &nSp45, pPap, 45 ); 
        SetValSprm( &nSp28, pPap, 28 ); 
        SetValSprm( &nLeMgn, pPap, 49 ); 
        SetValSprm( &nRiMgn, pPap, 49 ); 
        SetValSprm( &nUpMgn, pPap, 48 ); 
        SetValSprm( &nLoMgn, pPap, 48 ); 

        pS = pPap->HasSprm( 37 );                       
        if( pS )
            nSp37 = *pS;
    }
    else
    {
        SetValSprm( &nSp26, pPap, NS_sprm::LN_PDxaAbs ); 
        
        mbVertSet |= SetValSprm( &nSp27, pPap, NS_sprm::LN_PDyaAbs );    
        SetValSprm( &nSp45, pPap, NS_sprm::LN_PWHeightAbs ); 
        SetValSprm( &nSp28, pPap, NS_sprm::LN_PDxaWidth ); 
        SetValSprm( &nLeMgn, pPap, NS_sprm::LN_PDxaFromText );    
        SetValSprm( &nRiMgn, pPap, NS_sprm::LN_PDxaFromText );    
        SetValSprm( &nUpMgn, pPap, NS_sprm::LN_PDyaFromText );    
        SetValSprm( &nLoMgn, pPap, NS_sprm::LN_PDyaFromText );    

        pS = pPap->HasSprm( NS_sprm::LN_PWr );                               
        if( pS )
            nSp37 = *pS;
    }

    if( ::lcl_ReadBorders( bVer67, brc, pPap ))     
        bBorderLines = ::lcl_IsBorder( bVer67, brc );

    /*
     #i8798#
     Appears that with no dyaAbs set then the actual vert anchoring set is
     ignored and we remain relative to text, so if that is the case we are 0
     from para anchor, so we update the frame to have explicitly this type of
     anchoring
    */
    if (!mbVertSet)
        nSp29 = (nOrigSp29 & 0xCF) | 0x20;
    else
        nSp29 = nOrigSp29;
}

void WW8FlyPara::ReadFull(sal_uInt8 nOrigSp29, SwWW8ImplReader* pIo)
{
    WW8PLCFMan* pPlcxMan = pIo->pPlcxMan;
    WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();

    Read(nOrigSp29, pPap);    

    do{             
        if( nSp45 != 0 /* || nSp28 != 0 */ )
            break;                      
        if( pIo->pWwFib->fComplex )
            break;                      
                                        
        SvStream* pIoStrm = pIo->pStrm;
        sal_uLong nPos = pIoStrm->Tell();
        WW8PLCFxSave1 aSave;
        pPlcxMan->GetPap()->Save( aSave );
        bGrafApo = false;

        do{             
            sal_uInt8 nTxt[2];

            if (!checkRead(*pIoStrm, nTxt, 2)) 
                break;

            if( nTxt[0] != 0x01 || nTxt[1] != 0x0d )
                break;                              

            pPap->advance();                        

            
            
            const sal_uInt8* pS = pPap->HasSprm( bVer67 ? 29 : 0x261B );

            
            if (!pS)
            {
                bGrafApo = true;
                break;                              
            }

            ww::WordVersion eVer = pIo->GetFib().GetFIBVersion();
            WW8FlyPara *pNowStyleApo=0;
            sal_uInt16 nColl = pPap->GetIstd();
            ww::sti eSti = eVer < ww::eWW6 ? ww::GetCanonicalStiFromStc( static_cast< sal_uInt8 >(nColl) ) : static_cast<ww::sti>(nColl);
            while (eSti != ww::stiNil && nColl < pIo->vColl.size() && 0 == (pNowStyleApo = pIo->vColl[nColl].pWWFly))
            {
                nColl = pIo->vColl[nColl].nBase;
                eSti = eVer < ww::eWW6 ? ww::GetCanonicalStiFromStc( static_cast< sal_uInt8 >(nColl) ) : static_cast<ww::sti>(nColl);
            }

            WW8FlyPara aF(bVer67, pNowStyleApo);
                                                
            aF.Read( *pS, pPap );               
            if( !( aF == *this ) )              
                bGrafApo = true;                
                                                
        }
        while( false );                             

        pPlcxMan->GetPap()->Restore( aSave );
        pIoStrm->Seek( nPos );
    }while( false );                                    
}



void WW8FlyPara::Read(sal_uInt8 nOrigSp29, WW8RStyle* pStyle)
{
    const sal_uInt8* pS = 0;
    if (bVer67)
    {
        SetValSprm( &nSp26, pStyle, 26 );   
        
        mbVertSet |= SetValSprm(&nSp27, pStyle, 27);    
        SetValSprm( &nSp45, pStyle, 45 );   
        SetValSprm( &nSp28, pStyle, 28 );   
        SetValSprm( &nLeMgn,    pStyle, 49 );   
        SetValSprm( &nRiMgn,    pStyle, 49 );   
        SetValSprm( &nUpMgn,    pStyle, 48 );   
        SetValSprm( &nLoMgn,    pStyle, 48 );   

        pS = pStyle->HasParaSprm( 37 );             
        if( pS )
            nSp37 = *pS;
    }
    else
    {
        SetValSprm( &nSp26, pStyle, 0x8418 );   
        
        mbVertSet |= SetValSprm(&nSp27, pStyle, 0x8419);    
        SetValSprm( &nSp45, pStyle, 0x442B );   
        SetValSprm( &nSp28, pStyle, 0x841A );   
        SetValSprm( &nLeMgn, pStyle, 0x842F );  
        SetValSprm( &nRiMgn, pStyle, 0x842F );  
        SetValSprm( &nUpMgn, pStyle, 0x842E );  
        SetValSprm( &nLoMgn, pStyle, 0x842E );  

        pS = pStyle->HasParaSprm( 0x2423 );             
        if( pS )
            nSp37 = *pS;
    }

    if (::lcl_ReadBorders(bVer67, brc, 0, pStyle))      
        bBorderLines = ::lcl_IsBorder(bVer67, brc);

    /*
     #i8798#
     Appears that with no dyaAbs set then the actual vert anchoring set is
     ignored and we remain relative to text, so if that is the case we are 0
     from para anchor, so we update the frame to have explicitly this type of
     anchoring
    */
    if (!mbVertSet)
        nSp29 = (nOrigSp29 & 0xCF) | 0x20;
    else
        nSp29 = nOrigSp29;
}

bool WW8FlyPara::IsEmpty() const
{
    WW8FlyPara aEmpty(bVer67);
    /*
     wr of 0 like 2 appears to me to be equivalent for checking here. See
     #107103# if wrong, so given that the empty is 2, if we are 0 then set
     empty to 0 to make 0 equiv to 2 for empty checking
    */
    OSL_ENSURE(aEmpty.nSp37 == 2, "this is not what we expect for nSp37");
    if (this->nSp37 == 0)
        aEmpty.nSp37 = 0;
    if (aEmpty == *this)
        return true;
    return false;
}


WW8SwFlyPara::WW8SwFlyPara( SwPaM& rPaM,
                            SwWW8ImplReader& rIo,
                            WW8FlyPara& rWW,
                            const sal_uInt32 nWWPgTop,
                            const sal_uInt32 nPgLeft,
                            const sal_uInt32 nPgWidth,
                            const sal_Int32 nIniFlyDx,
                            const sal_Int32 nIniFlyDy )
{
    (void) rPaM;
    (void) nPgLeft;

    memset( this, 0, sizeof( WW8SwFlyPara ) );  
    nNewNettoWidth = MINFLY;                    

    eSurround = ( rWW.nSp37 > 1 ) ? SURROUND_IDEAL : SURROUND_NONE;
    
    const bool bIsTable = rIo.pPlcxMan->HasParaSprm(0x2416);
    if (  bIsTable && rWW.nSp37 == 2 )
        eSurround = SURROUND_PARALLEL;

    /*
     #95905#, #83307# seems to have gone away now, so reenable parallel
     wrapping support for frames in headers/footers. I don't know if we truly
     have an explictly specified behaviour for these circumstances.
    */

    nHeight = rWW.nSp45;
    if( nHeight & 0x8000 )
    {
        nHeight &= 0x7fff;
        eHeightFix = ATT_MIN_SIZE;
    }
    else
        eHeightFix = ATT_FIX_SIZE;

    if( nHeight <= MINFLY )
    {                           
        eHeightFix = ATT_MIN_SIZE;
        nHeight = MINFLY;
    }

    nWidth = nNettoWidth = rWW.nSp28;
    if( nWidth <= 10 )                              
    {
        bAutoWidth = true;
        nWidth = nNettoWidth =
            msword_cast<sal_Int16>((nPgWidth ? nPgWidth : 2268)); 
    }
    if( nWidth <= MINFLY )
        nWidth = nNettoWidth = MINFLY;              

    eVAlign = text::VertOrientation::NONE;                            
    eHAlign = text::HoriOrientation::NONE;
    nYPos = 0;
    nXPos = 0;

    nRiMgn = rWW.nRiMgn;
    nLeMgn = rWW.nLeMgn;
    nLoMgn = rWW.nLoMgn;
    nUpMgn = rWW.nUpMgn;

    /*
    See issue #i9178# for the 9 anchoring options, and make sure they stay
    working if you modify the anchoring logic here.
    */

    
    
    
    

    
    nYBind = (( rWW.nSp29 & 0x30 ) >> 4);
    
    
    eAnchor = FLY_AT_PARA;

    switch (nYBind)
    {
        case 0:     
            eVRel = text::RelOrientation::PAGE_PRINT_AREA;
            break;
        case 1:     
            eVRel = text::RelOrientation::PAGE_FRAME;
            break;
        default:    
            eVRel = text::RelOrientation::FRAME;
            break;
    }


    switch( rWW.nSp27 )             
    {
        case -4:
            eVAlign = text::VertOrientation::TOP;
            if (nYBind < 2)
                nUpMgn = 0;
            break;  
        case -8:
            eVAlign = text::VertOrientation::CENTER;
            break;  
        case -12:
            eVAlign = text::VertOrientation::BOTTOM;
            if (nYBind < 2)
                nLoMgn = 0;
            break;  
        default:
            nYPos = rWW.nSp27 + (short)nIniFlyDy;
            break;  
    }

    switch( rWW.nSp26 )                 
    {
        case 0:
            eHAlign = text::HoriOrientation::LEFT;
            nLeMgn = 0;
            break;  
        case -4:
            eHAlign = text::HoriOrientation::CENTER;
            break;  
        case -8:
            eHAlign = text::HoriOrientation::RIGHT;
            nRiMgn = 0;
            break;  
        case -12:
            eHAlign = text::HoriOrientation::LEFT;
            bToggelPos = true;
            break;  
        case -16:
            eHAlign = text::HoriOrientation::RIGHT;
            bToggelPos = true;
            break;  
        default:
            nXPos = rWW.nSp26 + (short)nIniFlyDx;
            break;  
    }

    nXBind = ( rWW.nSp29 & 0xc0 ) >> 6;

    switch (nXBind)           
    {
        case 0:     
            eHRel = text::RelOrientation::FRAME;
            break;
        case 1:     
            eHRel = text::RelOrientation::PAGE_PRINT_AREA;
            break;
        default:    
            eHRel = text::RelOrientation::PAGE_FRAME;
            break;
    }

    
    
    
    
    
    
    
    
    
    sal_Int16 nLeBorderMgn( 0L );
    if ( !bAutoWidth )
    {
        sal_Int16 nTemp = rWW.brc[WW8_LEFT].DetermineBorderProperties(rWW.bVer67,
            &nLeBorderMgn);
        nLeBorderMgn = nLeBorderMgn + nTemp;
    }
    
    sal_Int16 nRiBorderMgn( 0L );
    if ( !bAutoWidth )
    {
        sal_Int16 nTemp = rWW.brc[WW8_RIGHT].DetermineBorderProperties(rWW.bVer67,
            &nRiBorderMgn);
        nRiBorderMgn = nRiBorderMgn + nTemp;
    }
    if ( !bAutoWidth && eHAlign == text::HoriOrientation::LEFT && eHRel == text::RelOrientation::PAGE_FRAME )
    {
        
        
        
        eHAlign = text::HoriOrientation::NONE;
        eHRel = text::RelOrientation::PAGE_PRINT_AREA;
        nXPos = -nWidth - (2*nLeBorderMgn) - rWW.nRiMgn;
        
        nLeMgn = rWW.nLeMgn;
    }
    else if ( !bAutoWidth && eHAlign == text::HoriOrientation::RIGHT && eHRel == text::RelOrientation::PAGE_FRAME )
    {
        
        
        
        eHAlign = text::HoriOrientation::NONE;
        eHRel = text::RelOrientation::PAGE_RIGHT;
        nXPos = ( nRiBorderMgn - nLeBorderMgn ) + rWW.nLeMgn;
        
        nRiMgn = rWW.nRiMgn;
    }
    else if ( !bAutoWidth && eHAlign == text::HoriOrientation::LEFT && eHRel == text::RelOrientation::PAGE_PRINT_AREA )
    {
        
        
        eHAlign = text::HoriOrientation::NONE;
        eHRel = text::RelOrientation::PAGE_PRINT_AREA;
        nXPos = -nLeBorderMgn;
        
        nLeMgn = rWW.nLeMgn;
    }
    else if ( !bAutoWidth && eHAlign == text::HoriOrientation::RIGHT && eHRel == text::RelOrientation::PAGE_PRINT_AREA )
    {
        
        
        eHAlign = text::HoriOrientation::NONE;
        eHRel = text::RelOrientation::PAGE_RIGHT;
        nXPos = -nWidth - nLeBorderMgn;
        
        nRiMgn = rWW.nRiMgn;
    }
    else if (rWW.bBorderLines)
    {
        /*
        #i582#
        Word has a curious bug where the offset stored do not take into
        account the internal distance from the corner both
        */
        sal_Int16 nLeLMgn = 0;
        sal_Int16 nTemp = rWW.brc[WW8_LEFT].DetermineBorderProperties(rWW.bVer67,
            &nLeLMgn);
        nLeLMgn = nLeLMgn + nTemp;

        if (nLeLMgn)
        {
            if (eHAlign == text::HoriOrientation::LEFT)
                eHAlign = text::HoriOrientation::NONE;
            nXPos = nXPos - nLeLMgn;
        }
    }

    
    if ( eVAlign == text::VertOrientation::NONE && eVRel == text::RelOrientation::PAGE_PRINT_AREA )
    {
        
        
        eVRel = text::RelOrientation::PAGE_FRAME;
        nYPos = static_cast< sal_Int16 >( nYPos + nWWPgTop );
    }

    FlySecur1( nWidth, rWW.bBorderLines );          
    FlySecur1( nHeight, rWW.bBorderLines );

}





void WW8SwFlyPara::BoxUpWidth( long nInWidth )
{
    if( bAutoWidth && nInWidth > nNewNettoWidth )
        nNewNettoWidth = nInWidth;
};





WW8FlySet::WW8FlySet(SwWW8ImplReader& rReader, const WW8FlyPara* pFW,
    const WW8SwFlyPara* pFS, bool bGraf)
    : SfxItemSet(rReader.rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
{
    if (!rReader.mbNewDoc)
        Reader::ResetFrmFmtAttrs(*this);    
                                            
    Put(SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));

/*Below can all go when we have from left in rtl mode*/
    SwTwips nXPos = pFS->nXPos;
    sal_Int16 eHRel = pFS->eHRel;
    rReader.MiserableRTLGraphicsHack(nXPos, pFS->nWidth, pFS->eHAlign, eHRel);
/*Above can all go when we have from left in rtl mode*/
    Put( SwFmtHoriOrient(nXPos, pFS->eHAlign, pFS->eHRel, pFS->bToggelPos ));
    Put( SwFmtVertOrient( pFS->nYPos, pFS->eVAlign, pFS->eVRel ) );

    if (pFS->nLeMgn || pFS->nRiMgn)     
        Put(SvxLRSpaceItem(pFS->nLeMgn, pFS->nRiMgn, 0, 0, RES_LR_SPACE));

    if (pFS->nUpMgn || pFS->nLoMgn)
        Put(SvxULSpaceItem(pFS->nUpMgn, pFS->nLoMgn, RES_UL_SPACE));

    
    SwFmtSurround aSurround(pFS->eSurround);
    if ( pFS->eSurround == SURROUND_IDEAL )
        aSurround.SetAnchorOnly( sal_True );
    Put( aSurround );

    short aSizeArray[5]={0};
    rReader.SetFlyBordersShadow(*this,(const WW8_BRC*)pFW->brc,&aSizeArray[0]);

    

    
    
    Put( SwFmtWrapInfluenceOnObjPos(
                text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ) );

    if( !bGraf )
    {
        Put( SwFmtAnchor(pFS->eAnchor) );
        

        
        
        
        
        
        Put( SwFmtFrmSize( pFS->eHeightFix, pFS->nWidth +
            aSizeArray[WW8_LEFT] + aSizeArray[WW8_RIGHT],
            pFS->nHeight));
    }
}


WW8FlySet::WW8FlySet( SwWW8ImplReader& rReader, const SwPaM* pPaM,
    const WW8_PIC& rPic, long nWidth, long nHeight )
    : SfxItemSet(rReader.rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
{
    Init(rReader, pPaM);

    Put(SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));

    short aSizeArray[5]={0};
    /*
    If we have set borders then in word the graphic is displaced from the left
    and top the width of the borders of those sides, and then the shadow
    itself is drawn to the bottom and right of the displaced graphic.  In word
    the total size is that of the graphic plus the borders, plus the total
    shadow around all edges, for this translation the top and left shadow
    region is translated spacing around the graphic to those sides, and the
    bottom and right shadow size is added to the graphic size.
    */
    if (rReader.SetFlyBordersShadow( *this, rPic.rgbrc, &aSizeArray[0]))
    {
        Put(SvxLRSpaceItem( aSizeArray[WW8_LEFT], 0, 0, 0, RES_LR_SPACE ) );
        Put(SvxULSpaceItem( aSizeArray[WW8_TOP], 0, RES_UL_SPACE ));
        aSizeArray[WW8_RIGHT]*=2;
        aSizeArray[WW8_BOT]*=2;
    }

    Put( SwFmtFrmSize( ATT_FIX_SIZE, nWidth+aSizeArray[WW8_LEFT]+
        aSizeArray[WW8_RIGHT], nHeight+aSizeArray[WW8_TOP]
        + aSizeArray[WW8_BOT]) );
}

void WW8FlySet::Init(const SwWW8ImplReader& rReader, const SwPaM* pPaM)
{
    if (!rReader.mbNewDoc)
        Reader::ResetFrmFmtAttrs(*this);  

    Put(SvxLRSpaceItem(RES_LR_SPACE)); 
    SwFmtAnchor aAnchor(FLY_AS_CHAR);

    aAnchor.SetAnchor(pPaM->GetPoint());
    Put(aAnchor);

    
    
    if (rReader.maSectionManager.CurrentSectionIsVertical())
        Put(SwFmtVertOrient(0, text::VertOrientation::CHAR_CENTER,text::RelOrientation::CHAR));
    else
        Put(SwFmtVertOrient(0, text::VertOrientation::TOP, text::RelOrientation::FRAME));
}

WW8DupProperties::WW8DupProperties(SwDoc &rDoc, SwWW8FltControlStack *pStk)
    : pCtrlStck(pStk),
    aChrSet(rDoc.GetAttrPool(), RES_CHRATR_BEGIN, RES_CHRATR_END - 1 ),
    aParSet(rDoc.GetAttrPool(), RES_PARATR_BEGIN, RES_PARATR_END - 1 )
{
    
    
    size_t nCnt = pCtrlStck->size();
    for (size_t i=0; i < nCnt; ++i)
    {
        const SwFltStackEntry& rEntry = (*pCtrlStck)[ i ];
        if (rEntry.bOpen)
        {
            if (isCHRATR(rEntry.pAttr->Which()))
            {
                aChrSet.Put( *rEntry.pAttr );

            }
            else if (isPARATR(rEntry.pAttr->Which()))
            {
                aParSet.Put( *rEntry.pAttr );
            }
        }
    }
}

void WW8DupProperties::Insert(const SwPosition &rPos)
{
    const SfxItemSet *pSet=&aChrSet;
    for(int i=0;i<2;i++)
    {
        if (i==1)
            pSet = &aParSet;

        if( pSet->Count() )
        {
            SfxItemIter aIter( *pSet );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            do
            {
                pCtrlStck->NewAttr(rPos, *pItem);
            }while( !aIter.IsAtEnd() && 0 != ( pItem = aIter.NextItem() ) );
        }
    }
}

void SwWW8ImplReader::MoveInsideFly(const SwFrmFmt *pFlyFmt)
{
    WW8DupProperties aDup(rDoc,pCtrlStck);

    pCtrlStck->SetAttr(*pPaM->GetPoint(), 0, false);

    
    const SwFmtCntnt& rCntnt = pFlyFmt->GetCntnt();
    OSL_ENSURE( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
    pPaM->GetPoint()->nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );

    aDup.Insert(*pPaM->GetPoint());
}

SwTwips SwWW8ImplReader::MoveOutsideFly(SwFrmFmt *pFlyFmt,
    const SwPosition &rPos, bool bTableJoin)
{
    SwTwips nRetWidth = 0;
    if (!pFlyFmt)
        return nRetWidth;
    
    
    WW8DupProperties aDup(rDoc,pCtrlStck);
    pCtrlStck->SetAttr(*pPaM->GetPoint(), 0, false);

    /*
    #i1291
    If this fly frame consists entirely of one table inside a frame
    followed by an empty paragraph then we want to delete the empty
    paragraph so as to get the frame to autoshrink to the size of the
    table to emulate words behaviour closer.
    */
    if (bTableJoin)
    {
        const SwNodeIndex* pNodeIndex = pFlyFmt->GetCntnt().
            GetCntntIdx();
        if (pNodeIndex)
        {
            SwNodeIndex aIdx( *pNodeIndex, 1 ),
            aEnd( *pNodeIndex->GetNode().EndOfSectionNode() );

            if (aIdx < aEnd)
            {
                if(aIdx.GetNode().IsTableNode())
                {
                    SwTableNode *pTable = aIdx.GetNode().GetTableNode();
                    aIdx = *aIdx.GetNode().EndOfSectionNode();
                    ++aIdx;
                    if ( (aIdx < aEnd) && aIdx.GetNode().IsTxtNode() )
                    {
                        SwTxtNode *pNd = aIdx.GetNode().GetTxtNode();
                        ++aIdx;
                        if (aIdx == aEnd && pNd && pNd->GetTxt().isEmpty())
                        {
                            
                            //
                            
                            
                            
                            m_aExtraneousParas.push_back(pNd);

                            SwTable& rTable = pTable->GetTable();
                            SwFrmFmt* pTblFmt = rTable.GetFrmFmt();

                            if (pTblFmt)
                            {
                                SwFmtFrmSize aSize = pTblFmt->GetFrmSize();
                                aSize.SetHeightSizeType(ATT_MIN_SIZE);
                                aSize.SetHeight(MINLAY);
                                pFlyFmt->SetFmtAttr(aSize);
                                SwFmtHoriOrient aHori = pTblFmt->GetHoriOrient();
                                
                                
                                
                                
                                
                                
                                pTblFmt->SetFmtAttr( SwFmtHoriOrient(0, ( aHori.GetHoriOrient() == text::HoriOrientation::LEFT_AND_WIDTH ) ? ::text::HoriOrientation::LEFT_AND_WIDTH : text::HoriOrientation::FULL ) );
                                nRetWidth = aSize.GetWidth();
                            }
                        }
                    }
                }
            }
        }
    }

    *pPaM->GetPoint() = rPos;
    aDup.Insert(*pPaM->GetPoint());
    return nRetWidth;
}

WW8FlyPara *SwWW8ImplReader::ConstructApo(const ApoTestResults &rApo,
    const WW8_TablePos *pTabPos)
{
    WW8FlyPara *pRet = 0;
    OSL_ENSURE(rApo.HasFrame() || pTabPos,
        "If no frame found, *MUST* be in a table");

    pRet = new WW8FlyPara(bVer67, rApo.mpStyleApo);

    
    if (rApo.HasFrame())
        pRet->ReadFull(rApo.m_nSprm29, this);

    pRet->ApplyTabPos(pTabPos);

    if (pRet->IsEmpty())
        delete pRet, pRet = 0;
    return pRet;
}

bool SwWW8ImplReader::IsDropCap()
{
    
    
    
    WW8PLCFx_Cp_FKP *pPap = pPlcxMan ? pPlcxMan->GetPapPLCF() : 0;
    if (pPap)
    {
        const sal_uInt8 *pDCS;
        if (bVer67)
            pDCS = pPap->HasSprm(46);
        else
            pDCS = pPlcxMan->GetPapPLCF()->HasSprm(0x442C);
        if(pDCS)
        {
            /*
              fdct   short :3   0007     drop cap type
                                         0 no drop cap
                                         1 normal drop cap
                                         2 drop cap in margin
            */
            short nDCS = SVBT16ToShort( pDCS );
            if (nDCS & 7)
                return true;
        }
    }
    return false;
}

bool SwWW8ImplReader::StartApo(const ApoTestResults &rApo,
    const WW8_TablePos *pTabPos)
{
    if (0 == (pWFlyPara = ConstructApo(rApo, pTabPos)))
        return false;

    
    
    pSFlyPara = new WW8SwFlyPara( *pPaM, *this, *pWFlyPara,
                                  maSectionManager.GetWWPageTopMargin(),
                                  maSectionManager.GetPageLeft(),
                                  maSectionManager.GetTextAreaWidth(),
                                  nIniFlyDx, nIniFlyDy);

    
    if (IsDropCap())
    {
        bDropCap = true;
        pAktItemSet = new SfxItemSet( rDoc.GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_END - 1 );
        return false;
    }

    if( !pWFlyPara->bGrafApo )
    {

        
        
        
        
        
        
        

        WW8FlySet aFlySet(*this, pWFlyPara, pSFlyPara, false);

        if (pTabPos && pTabPos->bNoFly)
            pSFlyPara->pFlyFmt = 0;
        else
        {
            pSFlyPara->pFlyFmt = rDoc.MakeFlySection( pSFlyPara->eAnchor,
                    pPaM->GetPoint(), &aFlySet );
            OSL_ENSURE(pSFlyPara->pFlyFmt->GetAnchor().GetAnchorId() ==
                    pSFlyPara->eAnchor, "Not the anchor type requested!");
        }

        if (pSFlyPara->pFlyFmt)
        {
            if (!pDrawModel)
                GrafikCtor();

            SdrObject* pOurNewObject = CreateContactObject(pSFlyPara->pFlyFmt);
            pWWZOrder->InsertTextLayerObject(pOurNewObject);
        }

        if (FLY_AS_CHAR != pSFlyPara->eAnchor && pSFlyPara->pFlyFmt)
        {
            pAnchorStck->AddAnchor(*pPaM->GetPoint(),pSFlyPara->pFlyFmt);
        }

        
        pSFlyPara->pMainTextPos = new SwPosition( *pPaM->GetPoint() );

        
        
        
        pSFlyPara->pOldAnchorStck = pAnchorStck;
        pAnchorStck = new SwWW8FltAnchorStack(&rDoc, nFieldFlags);

        if (pSFlyPara->pFlyFmt)
            MoveInsideFly(pSFlyPara->pFlyFmt);

        
        
        
        
        
        
        
        
    }
    return true;
}

void wwSectionManager::JoinNode(const SwPosition &rPos, const SwNode &rNode)
{
    if ((!maSegments.empty()) && (maSegments.back().maStart == rPos.nNode))
        maSegments.back().maStart = SwNodeIndex(rNode);
}

bool SwWW8ImplReader::JoinNode(SwPaM &rPam, bool bStealAttr)
{
    bool bRet = false;
    rPam.GetPoint()->nContent = 0;          

    SwNodeIndex aPref(rPam.GetPoint()->nNode, -1);

    if (SwTxtNode* pNode = aPref.GetNode().GetTxtNode())
    {
        maSectionManager.JoinNode(*rPam.GetPoint(), aPref.GetNode());
        rPam.GetPoint()->nNode = aPref;
        rPam.GetPoint()->nContent.Assign(pNode, pNode->GetTxt().getLength());
        if (bStealAttr)
            pCtrlStck->StealAttr(rPam.GetPoint()->nNode);

        pNode->JoinNext();

        bRet = true;
    }
    return bRet;
}

void SwWW8ImplReader::StopApo()
{
    OSL_ENSURE(pWFlyPara, "no pWFlyPara to close");
    if (!pWFlyPara)
        return;
    if (pWFlyPara->bGrafApo)
    {
        
        
        JoinNode(*pPaM, true);

    }
    else
    {
        if (!pSFlyPara->pMainTextPos || !pWFlyPara)
        {
            OSL_ENSURE( pSFlyPara->pMainTextPos, "StopApo: pMainTextPos ist 0" );
            OSL_ENSURE( pWFlyPara, "StopApo: pWFlyPara ist 0" );
            return;
        }

        /*
        What we are doing with this temporary nodeindex is as follows: The
        stack of attributes normally only places them into the document when
        the current insertion point has passed them by. Otherwise the end
        point of the attribute gets pushed along with the insertion point. The
        insertion point is moved and the properties commited during
        MoveOutsideFly. We also may want to remove the final paragraph in the
        frame, but we need to wait until the properties for that frame text
        have been commited otherwise they will be lost. So we first get a
        handle to the last the filter inserted. After the attributes are
        commited, if that paragraph exists we join it with the para after it
        that comes with the frame by default so that as normal we don't end up
        with one more paragraph than we wanted.
        */
        SwNodeIndex aPref(pPaM->GetPoint()->nNode, -1);

        SwTwips nNewWidth =
            MoveOutsideFly(pSFlyPara->pFlyFmt, *pSFlyPara->pMainTextPos);
        if (nNewWidth)
            pSFlyPara->BoxUpWidth(nNewWidth);

        Color aBg(0xFE, 0xFF, 0xFF, 0xFF);  

        if (SwTxtNode* pNd = aPref.GetNode().GetTxtNode())
        {
            /*
            #i582#
            Take the last paragraph background colour and fill the frame with
            it.  Otherwise, make it transparent, this appears to be how MSWord
            works
            */
            const SfxPoolItem &rItm = pNd->SwCntntNode::GetAttr(RES_BACKGROUND);
            const SvxBrushItem &rBrush = (const SvxBrushItem&)(rItm);
            if (rBrush.GetColor().GetColor() != COL_AUTO)
                aBg = rBrush.GetColor();

            
            pNd->JoinNext();
        }

        if (pSFlyPara->pFlyFmt)
            pSFlyPara->pFlyFmt->SetFmtAttr(SvxBrushItem(aBg, RES_BACKGROUND));

        DeleteAnchorStk();
        pAnchorStck = pSFlyPara->pOldAnchorStck;

        
        
        
        
        if( pSFlyPara->nNewNettoWidth > MINFLY )    
        {
            long nW = pSFlyPara->nNewNettoWidth;
            nW += pSFlyPara->nWidth - pSFlyPara->nNettoWidth;   
            pSFlyPara->pFlyFmt->SetFmtAttr(
                SwFmtFrmSize( pSFlyPara->eHeightFix, nW, pSFlyPara->nHeight ) );
        }
        /*
        Word set *no* width meaning its an automatic width. The
        SwFlyPara reader will have already set a fallback width of the
        printable regions width, so we should reuse it. Despite the related
        problems with layout addressed with a hack in WW8FlyPara's constructor
        #i27204# Added AutoWidth setting. Left the old CalculateFlySize in place
        so that if the user unselects autowidth, the width doesn't max out
        */
        else if( !pWFlyPara->nSp28 && pSFlyPara->pFlyFmt)
        {
            using namespace sw::util;
            SfxItemSet aFlySet( pSFlyPara->pFlyFmt->GetAttrSet() );

            SwFmtFrmSize aSize(ItemGet<SwFmtFrmSize>(aFlySet, RES_FRM_SIZE));

            aFlySet.ClearItem(RES_FRM_SIZE);

            CalculateFlySize(aFlySet, pSFlyPara->pMainTextPos->nNode,
                pSFlyPara->nWidth);

            nNewWidth = ItemGet<SwFmtFrmSize>(aFlySet, RES_FRM_SIZE).GetWidth();

            aSize.SetWidth(nNewWidth);
            aSize.SetWidthSizeType(ATT_VAR_SIZE);

            pSFlyPara->pFlyFmt->SetFmtAttr(aSize);
        }

        delete pSFlyPara->pMainTextPos, pSFlyPara->pMainTextPos = 0;





    }

    
    if (pSFlyPara && pSFlyPara->pFlyFmt)
        pFmtOfJustInsertedApo = pSFlyPara->pFlyFmt;

    DELETEZ( pSFlyPara );
    DELETEZ( pWFlyPara );
}


bool SwWW8ImplReader::TestSameApo(const ApoTestResults &rApo,
    const WW8_TablePos *pTabPos)
{
    if( !pWFlyPara )
    {
        OSL_ENSURE( pWFlyPara, " Wo ist mein pWFlyPara ? " );
        return true;
    }

    
    
    
    

    
    WW8FlyPara aF(bVer67, rApo.mpStyleApo);
    
    if (rApo.HasFrame())
        aF.Read(rApo.m_nSprm29, pPlcxMan->GetPapPLCF());
    aF.ApplyTabPos(pTabPos);

    return aF == *pWFlyPara;
}

/***************************************************************************
#       Attribut - Verwaltung
#**************************************************************************/

void SwWW8ImplReader::NewAttr( const SfxPoolItem& rAttr,
                               const bool bFirstLineOfStSet,
                               const bool bLeftIndentSet )
{
    if( !bNoAttrImport ) 
    {
        if (pAktColl)
        {
            OSL_ENSURE(rAttr.Which() != RES_FLTR_REDLINE, "redline in style!");
            pAktColl->SetFmtAttr(rAttr);
        }
        else if (pAktItemSet)
        {
            pAktItemSet->Put(rAttr);
        }
        else if (rAttr.Which() == RES_FLTR_REDLINE)
        {
            mpRedlineStack->open(*pPaM->GetPoint(), rAttr);
        }
        else
        {
            pCtrlStck->NewAttr(*pPaM->GetPoint(), rAttr);
            
            if ( bFirstLineOfStSet )
            {
                const SwNode* pNd = &(pPaM->GetPoint()->nNode.GetNode());
                maTxtNodesHavingFirstLineOfstSet.insert( pNd );
            }
            
            if ( bLeftIndentSet )
            {
                const SwNode* pNd = &(pPaM->GetPoint()->nNode.GetNode());
                maTxtNodesHavingLeftIndentSet.insert( pNd );
            }
        }

        if (mpPostProcessAttrsInfo && mpPostProcessAttrsInfo->mbCopy)
            mpPostProcessAttrsInfo->mItemSet.Put(rAttr);
    }
}


const SfxPoolItem* SwWW8ImplReader::GetFmtAttr( sal_uInt16 nWhich )
{
    const SfxPoolItem* pRet = 0;
    if (pAktColl)
        pRet = &(pAktColl->GetFmtAttr(nWhich));
    else if (pAktItemSet)
    {
        pRet = pAktItemSet->GetItem(nWhich);
        if (!pRet)
            pRet = pStandardFmtColl ? &(pStandardFmtColl->GetFmtAttr(nWhich)) : 0;
        if (!pRet)
            pRet = &rDoc.GetAttrPool().GetDefaultItem(nWhich);
    }
    else if (pPlcxMan && pPlcxMan->GetDoingDrawTextBox())
    {
        pRet = pCtrlStck->GetStackAttr(*pPaM->GetPoint(), nWhich);
        if (!pRet)
        {
            if (nAktColl < vColl.size() && vColl[nAktColl].pFmt &&
                vColl[nAktColl].bColl)
            {
                pRet = &(vColl[nAktColl].pFmt->GetFmtAttr(nWhich));
            }
        }
        if (!pRet)
            pRet = pStandardFmtColl ? &(pStandardFmtColl->GetFmtAttr(nWhich)) : 0;
        if (!pRet)
            pRet = &rDoc.GetAttrPool().GetDefaultItem(nWhich);
    }
    else
        pRet = pCtrlStck->GetFmtAttr(*pPaM->GetPoint(), nWhich);
    return pRet;
}

/***************************************************************************
#       eigentliche Attribute
#
# Die Methoden erhalten die Token-Id und die Laenge der noch folgenden
# Parameter gemaess Tabelle in WWScan.cxx als Parameter
#**************************************************************************/

/***************************************************************************
#  Spezial WW - Attribute
#**************************************************************************/

void SwWW8ImplReader::Read_Special(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
    {
        bSpec = false;
        return;
    }
    bSpec = ( *pData != 0 );
}


void SwWW8ImplReader::Read_Obj(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
        bObj = false;
    else
    {
        bObj = 0 != *pData;

        if( bObj && nPicLocFc && bEmbeddObj )
        {
            if (!maFieldStack.empty() && maFieldStack.back().mnFieldId == 56)
            {
                
                maFieldStack.back().mnObjLocFc = nPicLocFc;
            }
            else
            {
                nObjLocFc = nPicLocFc;
            }
        }
    }
}

void SwWW8ImplReader::Read_PicLoc(sal_uInt16 , const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        nPicLocFc = 0;
        bSpec = false;  
    }
    else
    {
        nPicLocFc = SVBT32ToUInt32( pData );
        bSpec = true;

        if( bObj && nPicLocFc && bEmbeddObj )
            nObjLocFc = nPicLocFc;
    }
}

void SwWW8ImplReader::Read_POutLvl(sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (pAktColl && (0 < nLen))
    {
        if (SwWW8StyInf* pSI = GetStyle(nAktColl))
        {
            pSI->nOutlineLevel = static_cast< sal_uInt8 >(
                ( (1 <= pSI->GetWWStyleId()) && (9 >= pSI->GetWWStyleId()) )
            ? pSI->GetWWStyleId()-1
            : (pData ? *pData : 0) );
        }
    }
}

void SwWW8ImplReader::Read_Symbol(sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( !bIgnoreText )
    {
        if( nLen < 0 )
        {
            
            if (pPlcxMan && pPlcxMan->GetDoingDrawTextBox())
                pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
            bSymbol = false;
        }
        else
        {
            
            

            
            
            
            
            if (SetNewFontAttr(SVBT16ToShort( pData ), false, RES_CHRATR_FONT))
            {
                if( bVer67 )
                {
                    
                    cSymbol = OUString(
                        reinterpret_cast<const sal_Char*>(pData+2), 1,
                        RTL_TEXTENCODING_MS_1252).toChar();
                }
                else
                {
                    
                    cSymbol = SVBT16ToShort( pData+2 );
                }
                bSymbol = true;
            }
        }
    }
}

SwWW8StyInf *SwWW8ImplReader::GetStyle(sal_uInt16 nColl) const
{
    return const_cast<SwWW8StyInf *>(nColl < vColl.size() ? &vColl[nColl] : 0);
}

/***************************************************************************
#  Zeichen - Attribute
#**************************************************************************/



void SwWW8ImplReader::Read_BoldUsw( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    const int nContigiousWestern = 8;
    const int nWestern = nContigiousWestern + 1;
    const int nEastern = 2;
    const int nCTL = 2;
    const int nIds = nWestern + nEastern + nCTL;
    static const sal_uInt16 nEndIds[ nIds ] =
    {
        RES_CHRATR_WEIGHT,          RES_CHRATR_POSTURE,
        RES_CHRATR_CROSSEDOUT,      RES_CHRATR_CONTOUR,
        RES_CHRATR_SHADOWED,        RES_CHRATR_CASEMAP,
        RES_CHRATR_CASEMAP,         RES_CHRATR_HIDDEN,

        RES_CHRATR_CROSSEDOUT,

        RES_CHRATR_CJK_WEIGHT,      RES_CHRATR_CJK_POSTURE,

        RES_CHRATR_CTL_WEIGHT,      RES_CHRATR_CTL_POSTURE
    };

    ww::WordVersion eVersion = pWwFib->GetFIBVersion();

    sal_uInt8 nI;
    
    if (0x2A53 == nId)
        nI = nContigiousWestern;               
    else
    {
        
        if (eVersion <= ww::eWW2)
            nI = static_cast< sal_uInt8 >(nId - 60);
        else if (eVersion < ww::eWW8)
            nI = static_cast< sal_uInt8 >(nId - 85);
        else
            nI = static_cast< sal_uInt8 >(nId - 0x0835);
    }

    sal_uInt16 nMask = 1 << nI;

    if (nLen < 0)
    {
        if (nI < 2)
        {
            if (eVersion <= ww::eWW6)
            {
                
                
                pCtrlStck->SetAttr( *pPaM->GetPoint(), nEndIds[ nWestern + nEastern + nI ] );
            }
            
            
            pCtrlStck->SetAttr( *pPaM->GetPoint(), nEndIds[ nWestern + nI ] );
        }
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nEndIds[ nI ] );
        pCtrlStck->SetToggleAttr(nI, false);
        return;
    }
    
    bool bOn = *pData & 1;
    SwWW8StyInf* pSI = GetStyle(nAktColl);
    if (pPlcxMan && eVersion > ww::eWW2)
    {
        const sal_uInt8 *pCharIstd =
            pPlcxMan->GetChpPLCF()->HasSprm(bVer67 ? 80 : 0x4A30);
        if (pCharIstd)
            pSI = GetStyle(SVBT16ToShort(pCharIstd));
    }

    if( pAktColl )                          
    {
        if (pSI)
        {
            
            if (
                pSI->nBase < vColl.size() && (*pData & 0x80) &&
                (vColl[pSI->nBase].n81Flags & nMask)
               )
            {
                bOn = !bOn;                     
            }

            if (bOn)
                pSI->n81Flags |= nMask;         
            else
                pSI->n81Flags &= ~nMask;        
       }
    }
    else
    {

        
        if( *pData & 0x80 )                 
        {
            if (pSI && pSI->n81Flags & nMask)       
                bOn = !bOn;                 
            
            pCtrlStck->SetToggleAttr(nI, true);
        }
    }

    SetToggleAttr( nI, bOn );
}

void SwWW8ImplReader::Read_Bidi(sal_uInt16, const sal_uInt8*, short nLen)
{
    if (nLen > 0)
        bBidi = true;
    else
        bBidi = false;
}


void SwWW8ImplReader::Read_BoldBiDiUsw(sal_uInt16 nId, const sal_uInt8* pData,
    short nLen)
{
    static const sal_uInt16 nEndIds[2] =
    {
        RES_CHRATR_CTL_WEIGHT, RES_CHRATR_CTL_POSTURE,
    };

    sal_uInt8 nI;
    ww::WordVersion eVersion = pWwFib->GetFIBVersion();
    if (eVersion <= ww::eWW2)
        nI = static_cast< sal_uInt8 >(nId - 80);
    else if (eVersion < ww::eWW8)
        nI = static_cast< sal_uInt8 >(nId - 111);
    else
        nI = static_cast< sal_uInt8 >(nId - 0x085C);

    OSL_ENSURE(nI <= 1, "not happening");
    if (nI > 1)
        return;

    sal_uInt16 nMask = 1 << nI;

    if( nLen < 0 )
    {
        pCtrlStck->SetAttr(*pPaM->GetPoint(),nEndIds[nI]);
        pCtrlStck->SetToggleBiDiAttr(nI, false);
        return;
    }
    bool bOn = *pData & 1;
    SwWW8StyInf* pSI = GetStyle(nAktColl);
    if (pPlcxMan)
    {
        const sal_uInt8 *pCharIstd =
            pPlcxMan->GetChpPLCF()->HasSprm(bVer67 ? 80 : 0x4A30);
        if (pCharIstd)
            pSI = GetStyle(SVBT16ToShort(pCharIstd));
    }

    if (pAktColl && eVersion > ww::eWW2)        
    {
        if (pSI)
        {
            if( pSI->nBase < vColl.size()             
                && ( *pData & 0x80 )            
                && ( vColl[pSI->nBase].n81BiDiFlags & nMask ) ) 
                    bOn = !bOn;                     

            if( bOn )
                pSI->n81BiDiFlags |= nMask;         
            else
                pSI->n81BiDiFlags &= ~nMask;        
        }
    }
    else
    {

        
        if (*pData & 0x80)                  
        {
            if (pSI && pSI->n81BiDiFlags & nMask) 
                bOn = !bOn;                     
            
            pCtrlStck->SetToggleBiDiAttr(nI, true);
        }
    }

    SetToggleBiDiAttr(nI, bOn);
}

void SwWW8ImplReader::SetToggleBiDiAttr(sal_uInt8 nAttrId, bool bOn)
{
    switch (nAttrId)
    {
        case 0:
            {
                SvxWeightItem aAttr( bOn ? WEIGHT_BOLD : WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
                aAttr.SetWhich( RES_CHRATR_CTL_WEIGHT );
                NewAttr( aAttr );
            }
            break;
        case 1:
            {
                SvxPostureItem aAttr( bOn ? ITALIC_NORMAL : ITALIC_NONE, RES_CHRATR_POSTURE );
                aAttr.SetWhich( RES_CHRATR_CTL_POSTURE );
                NewAttr( aAttr );
            }
            break;
        default:
            OSL_ENSURE(!this, "Unhandled unknown bidi toggle attribute");
            break;

    }
}

void SwWW8ImplReader::SetToggleAttr(sal_uInt8 nAttrId, bool bOn)
{
    ww::WordVersion eVersion = pWwFib->GetFIBVersion();

    switch (nAttrId)
    {
        case 0:
            {
                SvxWeightItem aAttr( bOn ? WEIGHT_BOLD : WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
                NewAttr( aAttr );
                aAttr.SetWhich( RES_CHRATR_CJK_WEIGHT );
                NewAttr( aAttr );
                if (eVersion <= ww::eWW6)
                {
                    aAttr.SetWhich( RES_CHRATR_CTL_WEIGHT );
                    NewAttr( aAttr );
                }
            }
            break;
        case 1:
            {
                SvxPostureItem aAttr( bOn ? ITALIC_NORMAL : ITALIC_NONE, RES_CHRATR_POSTURE );
                NewAttr( aAttr );
                aAttr.SetWhich( RES_CHRATR_CJK_POSTURE );
                NewAttr( aAttr );
                if (eVersion <= ww::eWW6)
                {
                    aAttr.SetWhich( RES_CHRATR_CTL_POSTURE );
                    NewAttr( aAttr );
                }
            }
            break;
        case 2:
            NewAttr(SvxCrossedOutItem(bOn ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT));
            break;
        case 3:
            NewAttr( SvxContourItem( bOn, RES_CHRATR_CONTOUR ) );
            break;
        case 4:
            NewAttr( SvxShadowedItem( bOn, RES_CHRATR_SHADOWED ) );
            break;
        case 5:
            NewAttr( SvxCaseMapItem( bOn ? SVX_CASEMAP_KAPITAELCHEN
                                              : SVX_CASEMAP_NOT_MAPPED, RES_CHRATR_CASEMAP ) );
            break;
        case 6:
            NewAttr( SvxCaseMapItem( bOn ? SVX_CASEMAP_VERSALIEN
                                             : SVX_CASEMAP_NOT_MAPPED, RES_CHRATR_CASEMAP ) );
            break;
        case 7:
            NewAttr(SvxCharHiddenItem(bOn, RES_CHRATR_HIDDEN));
            break;
        case 8:
            NewAttr( SvxCrossedOutItem( bOn ? STRIKEOUT_DOUBLE
                                                : STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ) );
            break;
        default:
            OSL_ENSURE(!this, "Unhandled unknown toggle attribute");
            break;
    }
}

void SwWW8ImplReader::_ChkToggleAttr( sal_uInt16 nOldStyle81Mask,
                                        sal_uInt16 nNewStyle81Mask )
{
    sal_uInt16 i = 1, nToggleAttrFlags = pCtrlStck->GetToggleAttrFlags();
    for (sal_uInt8 n = 0; n < 7; ++n, i <<= 1)
    {
        if (
            (i & nToggleAttrFlags) &&
            ((i & nOldStyle81Mask) != (i & nNewStyle81Mask))
           )
        {
            SetToggleAttr(n, (i & nOldStyle81Mask));
        }
    }
}

void SwWW8ImplReader::_ChkToggleBiDiAttr( sal_uInt16 nOldStyle81Mask,
                                        sal_uInt16 nNewStyle81Mask )
{
    sal_uInt16 i = 1, nToggleAttrFlags = pCtrlStck->GetToggleBiDiAttrFlags();
    for (sal_uInt8 n = 0; n < 7; ++n, i <<= 1)
    {
        if (
            (i & nToggleAttrFlags) &&
            ((i & nOldStyle81Mask) != (i & nNewStyle81Mask))
           )
        {
            SetToggleBiDiAttr(n, (i & nOldStyle81Mask));
        }
    }
}

void SwWW8ImplReader::Read_SubSuper( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
        return;
    }

    short nEs;
    sal_uInt8 nProp;
    switch( *pData )
    {
        case 1:
            nEs = DFLT_ESC_AUTO_SUPER;
            nProp = DFLT_ESC_PROP;
            break;
        case 2:
            nEs = DFLT_ESC_AUTO_SUB;
            nProp = DFLT_ESC_PROP;
            break;
        default:
            nEs = 0;
            nProp = 100;
            break;
    }
    NewAttr( SvxEscapementItem( nEs, nProp, RES_CHRATR_ESCAPEMENT ) );
}

SwFrmFmt *SwWW8ImplReader::ContainsSingleInlineGraphic(const SwPaM &rRegion)
{
    /*
    For inline graphics and objects word has a hacked in feature to use
    subscripting to force the graphic into a centered position on the line, so
    we must check when applying sub/super to see if it the subscript range
    contains only a single graphic, and if that graphic is anchored as
    FLY_AS_CHAR and then we can change its anchoring to centered in the line.
    */
    SwFrmFmt *pRet=0;
    SwNodeIndex aBegin(rRegion.Start()->nNode);
    const sal_Int32 nBegin(rRegion.Start()->nContent.GetIndex());
    SwNodeIndex aEnd(rRegion.End()->nNode);
    const sal_Int32 nEnd(rRegion.End()->nContent.GetIndex());
    const SwTxtNode* pTNd;
    const SwTxtAttr* pTFlyAttr;
    if (
         aBegin == aEnd && nBegin == nEnd - 1 &&
         0 != (pTNd = aBegin.GetNode().GetTxtNode()) &&
         0 != (pTFlyAttr = pTNd->GetTxtAttrForCharAt(nBegin, RES_TXTATR_FLYCNT))
       )
    {
        const SwFmtFlyCnt& rFly = pTFlyAttr->GetFlyCnt();
        SwFrmFmt *pFlyFmt = rFly.GetFrmFmt();
        if (pFlyFmt &&
            (FLY_AS_CHAR == pFlyFmt->GetAnchor().GetAnchorId()))
        {
            pRet = pFlyFmt;
        }
    }
    return pRet;
}

bool SwWW8ImplReader::ConvertSubToGraphicPlacement()
{
    /*
    For inline graphics and objects word has a hacked in feature to use
    subscripting to force the graphic into a centered position on the line, so
    we must check when applying sub/super to see if it the subscript range
    contains only a single graphic, and if that graphic is anchored as
    FLY_AS_CHAR and then we can change its anchoring to centered in the line.
    */
    bool bIsGraphicPlacementHack = false;
    sal_uInt16 nPos;
    if (pCtrlStck->GetFmtStackAttr(RES_CHRATR_ESCAPEMENT, &nPos))
    {
        SwPaM aRegion(*pPaM->GetPoint());

        SwFltPosition aMkPos((*pCtrlStck)[nPos].m_aMkPos);
        SwFltPosition aPtPos(*pPaM->GetPoint());

        SwFrmFmt *pFlyFmt = 0;
        if (
             SwFltStackEntry::MakeRegion(&rDoc,aRegion,false,aMkPos,aPtPos) &&
             0 != (pFlyFmt = ContainsSingleInlineGraphic(aRegion))
           )
        {
            pCtrlStck->DeleteAndDestroy(nPos);
            pFlyFmt->SetFmtAttr(SwFmtVertOrient(0, text::VertOrientation::CHAR_CENTER, text::RelOrientation::CHAR));
            bIsGraphicPlacementHack = true;
        }
    }
    return bIsGraphicPlacementHack;
}

void SwWW8ImplReader::Read_SubSuperProp( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        if (!ConvertSubToGraphicPlacement())
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
        return;
    }

    ww::WordVersion eVersion = pWwFib->GetFIBVersion();

    
    short nPos = eVersion <= ww::eWW2 ? static_cast< sal_Int8 >( *pData ) : SVBT16ToShort( pData );
    sal_Int32 nPos2 = nPos * ( 10 * 100 );      
    const SvxFontHeightItem* pF
        = (const SvxFontHeightItem*)GetFmtAttr(RES_CHRATR_FONTSIZE);
    OSL_ENSURE(pF, "Expected to have the fontheight available here");

    
    sal_Int32 nHeight = 240;
    if (pF != NULL && pF->GetHeight() != 0)
        nHeight = pF->GetHeight();
    nPos2 /= nHeight;                       
    if( nPos2 > 100 )                       
        nPos2 = 100;
    if( nPos2 < -100 )
        nPos2 = -100;
    SvxEscapementItem aEs( (short)nPos2, 100, RES_CHRATR_ESCAPEMENT );
    NewAttr( aEs );
}

void SwWW8ImplReader::Read_Underline( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    FontUnderline eUnderline = UNDERLINE_NONE;
    bool bWordLine = false;
    if( pData )
    {
        
                    
                    
                    


        
        bool bAlsoBold = /*( 6 == b )*/ false;
        
        if( bAlsoBold )
        {
            sal_uInt8 nOn = 1;
            Read_BoldUsw( 0x0835, &nOn, nLen );
            eUnderline = UNDERLINE_SINGLE;
        }
        else
        {
            switch( *pData )
            {
            case 2: bWordLine = true;       
            case 1: eUnderline = (FontUnderline)UNDERLINE_SINGLE;       break;
            case 3: eUnderline = (FontUnderline)UNDERLINE_DOUBLE;       break;
            case 4: eUnderline = (FontUnderline)UNDERLINE_DOTTED;       break;
            case 7: eUnderline = (FontUnderline)UNDERLINE_DASH;         break;
            case 9: eUnderline = (FontUnderline)UNDERLINE_DASHDOT;      break;
            case 10:eUnderline = (FontUnderline)UNDERLINE_DASHDOTDOT;   break;
            case 6: eUnderline = (FontUnderline)UNDERLINE_BOLD;         break;
            case 11:eUnderline = (FontUnderline)UNDERLINE_WAVE;         break;
            case 20:eUnderline = (FontUnderline)UNDERLINE_BOLDDOTTED;   break;
            case 23:eUnderline = (FontUnderline)UNDERLINE_BOLDDASH;     break;
            case 39:eUnderline = (FontUnderline)UNDERLINE_LONGDASH;     break;
            case 55:eUnderline = (FontUnderline)UNDERLINE_BOLDLONGDASH; break;
            case 25:eUnderline = (FontUnderline)UNDERLINE_BOLDDASHDOT;  break;
            case 26:eUnderline = (FontUnderline)UNDERLINE_BOLDDASHDOTDOT;break;
            case 27:eUnderline = (FontUnderline)UNDERLINE_BOLDWAVE;     break;
            case 43:eUnderline = (FontUnderline)UNDERLINE_DOUBLEWAVE;   break;
            }
        }
    }

    
    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_UNDERLINE );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_WORDLINEMODE );
    }
    else
    {
        NewAttr( SvxUnderlineItem( eUnderline, RES_CHRATR_UNDERLINE ));
        if( bWordLine )
            NewAttr(SvxWordLineModeItem(true, RES_CHRATR_WORDLINEMODE));
    }
}

/*

NoBracket   78 CA 06 -  02 00 00 02 34 52
()          78 CA 06 -  02 01 00 02 34 52
[]          78 CA 06 -  02 02 00 02 34 52
<>          78 CA 06 -  02 03 00 02 34 52
{}          78 CA 06 -  02 04 00 02 34 52
*/
void SwWW8ImplReader::Read_DoubleLine_Rotate( sal_uInt16, const sal_uInt8* pData,
    short nLen )
{
    if( nLen < 0 ) 
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_TWO_LINES );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ROTATE );
    }
    else if( pData && 6 == nLen )
    {
        switch( *pData )
        {
        case 2:                     
            {
                sal_Unicode cStt = 0, cEnd = 0;
                switch( SVBT16ToShort( pData+1 ) )
                {
                case 1: cStt = '(', cEnd = ')'; break;
                case 2: cStt = '[', cEnd = ']'; break;
                case 3: cStt = '<', cEnd = '>'; break;
                case 4: cStt = '{', cEnd = '}'; break;
                }
                NewAttr( SvxTwoLinesItem( sal_True, cStt, cEnd, RES_CHRATR_TWO_LINES ));
            }
            break;

        case 1:                         
            {
                bool bFitToLine = 0 != *(pData+1);
                NewAttr( SvxCharRotateItem( 900, bFitToLine, RES_CHRATR_ROTATE ));
            }
            break;
        }
    }
}

void SwWW8ImplReader::Read_TxtColor( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    
    if (!bVer67 && pPlcxMan && pPlcxMan->GetChpPLCF()->HasSprm(NS_sprm::LN_CCv))
        return;

    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
    else
    {
        sal_uInt8 b = *pData;            

        if( b > 16 )                
            b = 0;

        NewAttr( SvxColorItem(Color(GetCol(b)), RES_CHRATR_COLOR));
        if (pAktColl && pStyles)
            pStyles->bTxtColChanged = true;
    }
}

void SwWW8ImplReader::Read_TxtForeColor(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
    else
    {
        Color aColor(msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)));
        NewAttr(SvxColorItem(aColor, RES_CHRATR_COLOR));
        if (pAktColl && pStyles)
            pStyles->bTxtColChanged = true;
    }
}

void SwWW8ImplReader::Read_UnderlineColor(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
    {
        
        
        
        
        return;
    }
    else
    {
        if ( pAktColl ) 
        {
            if( SFX_ITEM_SET == pAktColl->GetItemState( RES_CHRATR_UNDERLINE, sal_False ) )
            {
                const SwAttrSet& aSet = pAktColl->GetAttrSet();
                SvxUnderlineItem *pUnderline
                    = (SvxUnderlineItem *)(aSet.Get( RES_CHRATR_UNDERLINE, sal_False ).Clone());
                if(pUnderline){
                    pUnderline->SetColor( Color( msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)) ) );
                    pAktColl->SetFmtAttr( *pUnderline );
                    delete pUnderline;
                }
            }
        }
        else if ( pAktItemSet )
        {
            if ( SFX_ITEM_SET == pAktItemSet->GetItemState( RES_CHRATR_UNDERLINE, false ) )
            {
                SvxUnderlineItem *pUnderline
                    = (SvxUnderlineItem *)(pAktItemSet->Get( RES_CHRATR_UNDERLINE, sal_False ) .Clone());
                if(pUnderline){
                    pUnderline->SetColor( Color( msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)) ) );
                    pAktItemSet->Put( *pUnderline );
                    delete pUnderline;
                }
            }
        }
        else
        {
            SvxUnderlineItem* pUnderlineAttr = (SvxUnderlineItem*)pCtrlStck->GetOpenStackAttr( *pPaM->GetPoint(), RES_CHRATR_UNDERLINE );
            if( pUnderlineAttr != NULL )
                pUnderlineAttr->SetColor( Color( msfilter::util::BGRToRGB(SVBT32ToUInt32( pData ))));
        }
    }
}
bool SwWW8ImplReader::GetFontParams( sal_uInt16 nFCode, FontFamily& reFamily,
    OUString& rName, FontPitch& rePitch, rtl_TextEncoding& reCharSet )
{
    
    static const FontPitch ePitchA[] =
    {
        PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, PITCH_DONTKNOW
    };

    static const FontFamily eFamilyA[] =
    {
        FAMILY_DONTKNOW, FAMILY_ROMAN, FAMILY_SWISS, FAMILY_MODERN,
        FAMILY_SCRIPT, FAMILY_DECORATIVE, FAMILY_DONTKNOW, FAMILY_DONTKNOW
    };

    const WW8_FFN* pF = pFonts->GetFont( nFCode );  
    if( !pF )                                   
        return false;                           

    rName = pF->sFontname;

    
    rePitch = ePitchA[pF->prg];

    
    if( 77 == pF->chs )             
        reCharSet = eTextCharSet;   
    else
    { 
        
        
        
        if (bVer67 && pF->chs == 0)
            reCharSet = RTL_TEXTENCODING_DONTKNOW;
        else
            reCharSet = rtl_getTextEncodingFromWindowsCharset( pF->chs );
    }

    
    
    
    
    if (rName.startsWithIgnoreAsciiCase("Tms Rmn") ||
        rName.startsWithIgnoreAsciiCase("Timmons") ||
        rName.startsWithIgnoreAsciiCase("CG Times") ||
        rName.startsWithIgnoreAsciiCase("MS Serif") ||
        rName.startsWithIgnoreAsciiCase("Garamond") ||
        rName.startsWithIgnoreAsciiCase("Times Roman") ||
        rName.startsWithIgnoreAsciiCase("Times New Roman"))
    {
        reFamily = FAMILY_ROMAN;
    }
    else if (rName.startsWithIgnoreAsciiCase("Helv") ||
             rName.startsWithIgnoreAsciiCase("Arial") ||
             rName.startsWithIgnoreAsciiCase("Univers") ||
             rName.startsWithIgnoreAsciiCase("LinePrinter") ||
             rName.startsWithIgnoreAsciiCase("Lucida Sans") ||
             rName.startsWithIgnoreAsciiCase("Small Fonts") ||
             rName.startsWithIgnoreAsciiCase("MS Sans Serif"))
    {
        reFamily = FAMILY_SWISS;
    }
    else
    {
        reFamily = eFamilyA[pF->ff];
    }

    return true;
}

bool SwWW8ImplReader::SetNewFontAttr(sal_uInt16 nFCode, bool bSetEnums,
    sal_uInt16 nWhich)
{
    FontFamily eFamily;
    OUString aName;
    FontPitch ePitch;
    rtl_TextEncoding eSrcCharSet;

    if( !GetFontParams( nFCode, eFamily, aName, ePitch, eSrcCharSet ) )
    {
        
        
        
        if (!pAktColl && IsListOrDropcap())
        {
            if (nWhich == RES_CHRATR_CJK_FONT)
            {
                if (!maFontSrcCJKCharSets.empty())
                {
                    eSrcCharSet = maFontSrcCJKCharSets.top();
                }
                else
                {
                    eSrcCharSet = RTL_TEXTENCODING_DONTKNOW;
                }

                maFontSrcCJKCharSets.push(eSrcCharSet);
            }
            else
            {
                if (!maFontSrcCharSets.empty())
                {
                    eSrcCharSet = maFontSrcCharSets.top();
                }
                else
                {
                    eSrcCharSet = RTL_TEXTENCODING_DONTKNOW;
                }

                maFontSrcCharSets.push(eSrcCharSet);
            }
        }
        return false;
    }

    rtl_TextEncoding eDstCharSet = eSrcCharSet;

    SvxFontItem aFont( eFamily, aName, OUString(), ePitch, eDstCharSet, nWhich);

    if( bSetEnums )
    {
        if( pAktColl && nAktColl < vColl.size() ) 
        {
            switch(nWhich)
            {
                default:
                case RES_CHRATR_FONT:
                    vColl[nAktColl].eLTRFontSrcCharSet = eSrcCharSet;
                    break;
                case RES_CHRATR_CTL_FONT:
                    vColl[nAktColl].eRTLFontSrcCharSet = eSrcCharSet;
                    break;
                case RES_CHRATR_CJK_FONT:
                    vColl[nAktColl].eCJKFontSrcCharSet = eSrcCharSet;
                    break;
            }
        }
        else if (IsListOrDropcap())
        {
            
            if (nWhich  == RES_CHRATR_CJK_FONT)
                maFontSrcCJKCharSets.push(eSrcCharSet);
            else
                maFontSrcCharSets.push(eSrcCharSet);
        }
    }

    NewAttr( aFont );                       

    return true;
}

void SwWW8ImplReader::ResetCharSetVars()
{
    OSL_ENSURE(!maFontSrcCharSets.empty(),"no charset to remove");
    if (!maFontSrcCharSets.empty())
        maFontSrcCharSets.pop();
}

void SwWW8ImplReader::ResetCJKCharSetVars()
{
    OSL_ENSURE(!maFontSrcCJKCharSets.empty(),"no charset to remove");
    if (!maFontSrcCJKCharSets.empty())
        maFontSrcCJKCharSets.pop();
}

void SwWW8ImplReader::openFont(sal_uInt16 nFCode, sal_uInt16 nId)
{
    if (SetNewFontAttr(nFCode, true, nId) && pAktColl && pStyles)
    {
        
        if (RES_CHRATR_CJK_FONT == nId)
            pStyles->bCJKFontChanged = true;
        else if (RES_CHRATR_CTL_FONT == nId)
            pStyles->bCTLFontChanged = true;
        else
            pStyles->bFontChanged = true;
    }
}

void SwWW8ImplReader::closeFont(sal_uInt16 nId)
{
    pCtrlStck->SetAttr( *pPaM->GetPoint(), nId );
    if (nId == RES_CHRATR_CJK_FONT)
        ResetCJKCharSetVars();
    else
        ResetCharSetVars();
}

/*
    Font ein oder ausschalten:
*/
void SwWW8ImplReader::Read_FontCode( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    if (!bSymbol)           
    {                       
        switch( nId )
        {
            case 113:       
            case 0x4A51:    
            case 0x4A5E:    
                nId = RES_CHRATR_CTL_FONT;
                break;
            case 93:        
            case 111:       
            case 0x4A4f:
                nId = RES_CHRATR_FONT;
                break;
            case 112:       
            case 0x4A50:
                nId = RES_CHRATR_CJK_FONT;
                break;
            default:
                return ;
        }

        ww::WordVersion eVersion = pWwFib->GetFIBVersion();

        if( nLen < 0 ) 
        {
            if (eVersion <= ww::eWW6)
            {
                closeFont(RES_CHRATR_CTL_FONT);
                closeFont(RES_CHRATR_CJK_FONT);
            }
            closeFont(nId);
        }
        else
        {
            sal_uInt16 nFCode = SVBT16ToShort( pData );     
            openFont(nFCode, nId);
            if (eVersion <= ww::eWW6)
            {
                openFont(nFCode, RES_CHRATR_CJK_FONT);
                openFont(nFCode, RES_CHRATR_CTL_FONT);
            }
        }
    }
}

void SwWW8ImplReader::Read_FontSize( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    switch( nId )
    {
        case 74:
        case 99:
        case 0x4a43:
            nId = RES_CHRATR_FONTSIZE;
            break;
        case 85:  
        case 116: 
        case 0x4a61:
            nId = RES_CHRATR_CTL_FONTSIZE;
            break;
        default:
            return ;
    }

    ww::WordVersion eVersion = pWwFib->GetFIBVersion();

    if( nLen < 0 )          
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nId  );
        if (eVersion <= ww::eWW6) 
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_CTL_FONTSIZE );
        if (RES_CHRATR_FONTSIZE == nId)  
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_CJK_FONTSIZE );
    }
    else
    {
        
        sal_uInt16 nFSize = eVersion <= ww::eWW2 ? *pData : SVBT16ToShort(pData);
        nFSize*= 10;

        SvxFontHeightItem aSz( nFSize, 100, nId );
        NewAttr( aSz );
        if (RES_CHRATR_FONTSIZE == nId)  
        {
            aSz.SetWhich( RES_CHRATR_CJK_FONTSIZE );
            NewAttr( aSz );
        }
        if (eVersion <= ww::eWW6) 
        {
            aSz.SetWhich( RES_CHRATR_CTL_FONTSIZE );
            NewAttr( aSz );
        }
        if (pAktColl && pStyles)            
        {
            
            if (nId == RES_CHRATR_CTL_FONTSIZE)
                pStyles->bFCTLSizeChanged = true;
            else
            {
                pStyles->bFSizeChanged = true;
                if (eVersion <= ww::eWW6)
                    pStyles->bFCTLSizeChanged= true;
            }
        }
    }
}



void SwWW8ImplReader::Read_CharSet(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
    {                   
        eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
        return;
    }
    sal_uInt8 nfChsDiff = *pData;

    if( nfChsDiff )
        eHardCharSet = rtl_getTextEncodingFromWindowsCharset( *(pData + 1) );
    else
        eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
}

void SwWW8ImplReader::Read_Language( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    switch( nId )
    {
        case 97:
        case 0x486D: 
        case 0x4873: 
            nId = RES_CHRATR_LANGUAGE;
            break;
        case 0x486E: 
        case 0x4874: 
            nId = RES_CHRATR_CJK_LANGUAGE;
            break;
        case 83:
        case 114:
        case 0x485F:
            nId = RES_CHRATR_CTL_LANGUAGE;
            break;
        default:
            return;
    }

    if( nLen < 0 )                  
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nId );
    else
    {
        sal_uInt16 nLang = SVBT16ToShort( pData );  
        NewAttr(SvxLanguageItem((const LanguageType)nLang, nId));
    }
}

/*
    Einschalten des Zeichen-Styles:
*/
void SwWW8ImplReader::Read_CColl( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 ){                 
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_TXTATR_CHARFMT );
        nCharFmt = -1;
        return;
    }
    sal_uInt16 nId = SVBT16ToShort( pData );    

    if( nId >= vColl.size() || !vColl[nId].pFmt  
        || vColl[nId].bColl )              
        return;                             

    
    
    
    
    if (mbLoadingTOCCache && vColl[nId].GetWWStyleId() == ww::stiHyperlink)
    {
        return;
    }

    NewAttr( SwFmtCharFmt( (SwCharFmt*)vColl[nId].pFmt ) );
    nCharFmt = (short) nId;
}


/*
    enger oder weiter als normal:
*/
void SwWW8ImplReader::Read_Kern( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 ){                 
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_KERNING );
        return;
    }
    sal_Int16 nKern = SVBT16ToShort( pData );    
    NewAttr( SvxKerningItem( nKern, RES_CHRATR_KERNING ) );
}

void SwWW8ImplReader::Read_FontKern( sal_uInt16, const sal_uInt8* , short nLen )
{
    if( nLen < 0 ) 
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_AUTOKERN );
    else
        NewAttr(SvxAutoKernItem(true, RES_CHRATR_AUTOKERN));
}

void SwWW8ImplReader::Read_CharShadow(  sal_uInt16, const sal_uInt8* pData, short nLen )
{
    
    if (!bVer67 && pPlcxMan && pPlcxMan->GetChpPLCF()->HasSprm(0xCA71))
        return;

    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
        if( bCharShdTxtCol )
        {
            
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
            bCharShdTxtCol = false;
        }
    }
    else
    {
        WW8_SHD aSHD;
        aSHD.SetWWValue( *(SVBT16*)pData );
        SwWW8Shade aSh( bVer67, aSHD );

        NewAttr( SvxBrushItem( aSh.aColor, RES_CHRATR_BACKGROUND ));
    }
}

void SwWW8ImplReader::Read_TxtBackColor(sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
        if( bCharShdTxtCol )
        {
            
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
            bCharShdTxtCol = false;
        }
    }
    else
    {
        OSL_ENSURE(nLen == 10, "Len of para back colour not 10!");
        if (nLen != 10)
            return;
        Color aColour(ExtractColour(pData, bVer67));
        NewAttr(SvxBrushItem(aColour, RES_CHRATR_BACKGROUND));
    }
}

void SwWW8ImplReader::Read_CharHighlight(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
        if( bCharShdTxtCol )
        {
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );  
            bCharShdTxtCol = false;
        }
    }
    else
    {
        sal_uInt8 b = *pData;            

        if( b > 16 )                
            b = 0;                  

        Color aCol(GetCol(b));
        NewAttr( SvxBrushItem( aCol , RES_CHRATR_BACKGROUND ));
    }
}


/***************************************************************************
#  Absatz - Attribute
#**************************************************************************/

void SwWW8ImplReader::Read_NoLineNumb(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )  
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_LINENUMBER );
        return;
    }
    SwFmtLineNumber aLN;
    if (const SwFmtLineNumber* pLN
        = (const SwFmtLineNumber*)GetFmtAttr(RES_LINENUMBER))
    {
        aLN.SetStartValue( pLN->GetStartValue() );
    }

    aLN.SetCountLines( pData && (0 == *pData) );
    NewAttr( aLN );
}

bool lcl_HasExplicitLeft(const WW8PLCFMan *pPlcxMan, bool bVer67)
{
    WW8PLCFx_Cp_FKP *pPap = pPlcxMan ? pPlcxMan->GetPapPLCF() : 0;
    if (pPap)
    {
        if (bVer67)
            return pPap->HasSprm(17);
        else
            return (pPap->HasSprm(0x840F) || pPap->HasSprm(0x845E));
    }
    return false;
}

void SwWW8ImplReader::Read_LR( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    if (nLen < 0)  
    {
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_LR_SPACE);
        return;
    }

    short nPara = SVBT16ToShort( pData );

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    const SfxPoolItem* pLR = GetFmtAttr(RES_LR_SPACE);
    if( pLR )
        aLR = *static_cast<const SvxLRSpaceItem*>(pLR);

    
    
    
    
    
    SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode();
    if( pTxtNode && pTxtNode->AreListLevelIndentsApplicable() )
    {
        SwNumRule * pNumRule = pTxtNode->GetNumRule();
        if( pNumRule )
        {
            sal_uInt8 nLvl = static_cast< sal_uInt8 >(pTxtNode->GetActualListLevel());
            const SwNumFmt* pFmt = pNumRule->GetNumFmt( nLvl );
            if ( pFmt && pFmt->GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aLR.SetTxtLeft( pFmt->GetIndentAt() );
                aLR.SetTxtFirstLineOfst( static_cast<short>(pFmt->GetFirstLineIndent()) );
                
                pTxtNode->SetAttr( aLR );
            }
        }
    }

    /*
    The older word sprms mean left/right, while the new ones mean before/after.
    Writer now also works with before after, so when we see old left/right and
    we're RTL. We swap them
    */
    if (IsRightToLeft())
    {
        switch (nId)
        {
            
            case 17:
                nId = 16;
                break;
            case 0x840F:
                nId = 0x840E;
                break;
            
            case 16:
                nId = 17;
                break;
            case 0x840E:
                nId = 0x840F;
                break;
        }
    }

    bool bFirstLinOfstSet( false ); 
    bool bLeftIndentSet( false ); 

    switch (nId)
    {
        
        case     17:
        case 0x840F:
        case 0x845E:
            aLR.SetTxtLeft( nPara );
            if (pAktColl && nAktColl < vColl.size())
            {
                vColl[nAktColl].bListReleventIndentSet = true;
            }
            bLeftIndentSet = true;  
            break;
        
        case     19:
        case 0x8411:
        case 0x8460:
            /*
            As part of an attempt to break my spirit ww 8+ formats can contain
            ww 7- lists. If they do and the list is part of the style, then
            when removing the list from a paragraph of that style there
            appears to be a bug where the hanging indent value which the list
            set is still factored into the left indent of the paragraph. Its
            not listed in the winword dialogs, but it is clearly there. So if
            our style has a broken ww 7- list and we know that the list has
            been removed then we will factor the original list applied hanging
            into our calculation.
            */
            if (pPlcxMan && nAktColl < vColl.size() && vColl[nAktColl].bHasBrokenWW6List)
            {
                const sal_uInt8 *pIsZeroed = pPlcxMan->GetPapPLCF()->HasSprm(0x460B);
                if (pIsZeroed && *pIsZeroed == 0)
                {
                    const SvxLRSpaceItem &rLR =
                        ItemGet<SvxLRSpaceItem>(*(vColl[nAktColl].pFmt),
                        RES_LR_SPACE);
                    nPara = nPara - rLR.GetTxtFirstLineOfst();
                }
            }

            aLR.SetTxtFirstLineOfst(nPara);

            if (!pAktColl)
            {
                if (const SwTxtNode* pNode = pPaM->GetNode()->GetTxtNode())
                {
                    if ( const SwNumFmt *pNumFmt = GetNumFmtFromTxtNode(*pNode) )
                    {
                        if (!lcl_HasExplicitLeft(pPlcxMan, bVer67))
                        {
                            aLR.SetTxtLeft(pNumFmt->GetIndentAt());

                            
                            const SvxTabStopItem *pDefaultStopItem = (const SvxTabStopItem *)rDoc.GetAttrPool().GetPoolDefaultItem(RES_PARATR_TABSTOP);
                            if ( pDefaultStopItem &&  pDefaultStopItem->Count() > 0 )
                                ((SwNumFmt*)(pNumFmt))->SetListtabPos( ((SvxTabStop&)(*pDefaultStopItem)[0]).GetTabPos() );
                        }
                    }
                }
            }
            if (pAktColl && nAktColl < vColl.size())
            {
                vColl[nAktColl].bListReleventIndentSet = true;
            }
            bFirstLinOfstSet = true; 
            break;
        
        case     16:
        case 0x840E:
        case 0x845D:
            aLR.SetRight( nPara );
            break;
        default:
            return;
    }

    NewAttr( aLR, bFirstLinOfstSet, bLeftIndentSet ); 
}


void SwWW8ImplReader::Read_LineSpace( sal_uInt16, const sal_uInt8* pData, short nLen )
{

    if (bStyNormal && bWWBugNormal)
        return;

    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_LINESPACING );
        if( !( nIniFlags & WW8FL_NO_IMPLPASP ) )
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }

    short nSpace = SVBT16ToShort( pData );
    ww::WordVersion eVersion = pWwFib->GetFIBVersion();
    short nMulti = (eVersion <= ww::eWW2) ? 1 : SVBT16ToShort( pData + 2 );

    SvxLineSpace eLnSpc;
    if( 0 > nSpace )
    {
        nSpace = -nSpace;
        eLnSpc = SVX_LINE_SPACE_FIX;
    }
    else
        eLnSpc = SVX_LINE_SPACE_MIN;






//





    sal_uInt16 nSpaceTw = 0;

    SvxLineSpacingItem aLSpc( LINE_SPACE_DEFAULT_HEIGHT, RES_PARATR_LINESPACING );

    if( 1 == nMulti )               
    {
        long n = nSpace * 10 / 24;  


        if( n>200 ) n = 200;        
        aLSpc.SetPropLineSpace( (const sal_uInt8)n );
        const SvxFontHeightItem* pH = (const SvxFontHeightItem*)
            GetFmtAttr( RES_CHRATR_FONTSIZE );
        nSpaceTw = (sal_uInt16)( n * pH->GetHeight() / 100 );
    }
    else                            
    {
        
        nSpaceTw = (sal_uInt16)nSpace;
        aLSpc.SetLineHeight( nSpaceTw );
        aLSpc.GetLineSpaceRule() = eLnSpc;
    }
    NewAttr( aLSpc );
    if( pSFlyPara )
        pSFlyPara->nLineSpace = nSpaceTw;   
}


sal_uInt16 SwWW8ImplReader::GetParagraphAutoSpace(bool fDontUseHTMLAutoSpacing)
{
    if (fDontUseHTMLAutoSpacing)
        return 100;  
    else
        return 280;  
}

void SwWW8ImplReader::Read_ParaAutoBefore(sal_uInt16, const sal_uInt8 *pData, short nLen)
{
    if (nLen < 0)
    {
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_UL_SPACE);
        return;
    }

    if (*pData)
    {
        SvxULSpaceItem aUL(*(const SvxULSpaceItem*)GetFmtAttr(RES_UL_SPACE));
        aUL.SetUpper(GetParagraphAutoSpace(pWDop->fDontUseHTMLAutoSpacing));
        NewAttr(aUL);
        if (pAktColl && nAktColl < vColl.size())
            vColl[nAktColl].bParaAutoBefore = true;
        else
            bParaAutoBefore = true;
    }
    else
    {
        if (pAktColl && nAktColl < vColl.size())
            vColl[nAktColl].bParaAutoBefore = false;
        else
            bParaAutoBefore = false;
    }
}

void SwWW8ImplReader::Read_ParaAutoAfter(sal_uInt16, const sal_uInt8 *pData, short nLen)
{
    if (nLen < 0)
    {
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_UL_SPACE);
        return;
    }

    if (*pData)
    {
        SvxULSpaceItem aUL(*(const SvxULSpaceItem*)GetFmtAttr(RES_UL_SPACE));
        aUL.SetLower(GetParagraphAutoSpace(pWDop->fDontUseHTMLAutoSpacing));
        NewAttr(aUL);
        if (pAktColl && nAktColl < vColl.size())
            vColl[nAktColl].bParaAutoAfter = true;
        else
            bParaAutoAfter = true;
    }
    else
    {
        if (pAktColl && nAktColl < vColl.size())
            vColl[nAktColl].bParaAutoAfter = false;
        else
            bParaAutoAfter = false;
    }
}


void SwWW8ImplReader::Read_UL( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{












    if( nLen < 0 )
    {
        
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    short nPara = SVBT16ToShort( pData );
    if( nPara < 0 )
        nPara = -nPara;

    SvxULSpaceItem aUL( *(const SvxULSpaceItem*)GetFmtAttr( RES_UL_SPACE ));

    switch( nId )
    {
        
        case     21:
        case 0xA413:
            aUL.SetUpper( nPara );
            break;
        
        case     22:
        case 0xA414:
            aUL.SetLower( nPara );
            break;
        default:
            return;
    };

    NewAttr( aUL );
}

void SwWW8ImplReader::Read_ParaContextualSpacing( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    SvxULSpaceItem aUL( *(const SvxULSpaceItem*)GetFmtAttr( RES_UL_SPACE ));
    aUL.SetContextValue(*pData);
    NewAttr( aUL );
}

void SwWW8ImplReader::Read_IdctHint( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 0)
        nIdctHint = 0;
    else
        nIdctHint = *pData;
}

void SwWW8ImplReader::Read_Justify( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_ADJUST );
        return;
    }

    SvxAdjust eAdjust(SVX_ADJUST_LEFT);
    bool bDistributed = false;
    switch (*pData)
    {
        default:
        case 0:
            break;
        case 1:
            eAdjust = SVX_ADJUST_CENTER;
            break;
        case 2:
            eAdjust = SVX_ADJUST_RIGHT;
            break;
        case 3:
            eAdjust = SVX_ADJUST_BLOCK;
            break;
        case 4:
            eAdjust = SVX_ADJUST_BLOCK;
            bDistributed = true;
            break;
    }
    SvxAdjustItem aAdjust(eAdjust, RES_PARATR_ADJUST);
    if (bDistributed)
        aAdjust.SetLastBlock(SVX_ADJUST_BLOCK);

    NewAttr(aAdjust);
}

bool SwWW8ImplReader::IsRightToLeft()
{
    bool bRTL = false;
    const sal_uInt8 *pDir =
        pPlcxMan ? pPlcxMan->GetPapPLCF()->HasSprm(0x2441) : 0;
    if (pDir)
        bRTL = *pDir ? true : false;
    else
    {
        const SvxFrameDirectionItem* pItem=
            (const SvxFrameDirectionItem*)GetFmtAttr(RES_FRAMEDIR);
        if (pItem && (pItem->GetValue() == FRMDIR_HORI_RIGHT_TOP))
            bRTL = true;
    }
    return bRTL;
}

void SwWW8ImplReader::Read_RTLJustify( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_ADJUST );
        return;
    }

    
    
    if (!IsRightToLeft())
        Read_Justify(0x2403 /*dummy*/, pData, nLen);
    else
    {
        SvxAdjust eAdjust(SVX_ADJUST_RIGHT);
        bool bDistributed = false;
        switch (*pData)
        {
            default:
            case 0:
                break;
            case 1:
                eAdjust = SVX_ADJUST_CENTER;
                break;
            case 2:
                eAdjust = SVX_ADJUST_LEFT;
                break;
            case 3:
                eAdjust = SVX_ADJUST_BLOCK;
                break;
            case 4:
                eAdjust = SVX_ADJUST_BLOCK;
                bDistributed = true;
                break;
        }
        SvxAdjustItem aAdjust(eAdjust, RES_PARATR_ADJUST);
        if (bDistributed)
            aAdjust.SetLastBlock(SVX_ADJUST_BLOCK);

        NewAttr(aAdjust);
    }
}

void SwWW8ImplReader::Read_BoolItem( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    switch( nId )
    {
        case 0x2433:
            nId = RES_PARATR_FORBIDDEN_RULES;
            break;
        case 0x2435:
            nId = RES_PARATR_HANGINGPUNCTUATION;
            break;
        case 0x2437:
            nId = RES_PARATR_SCRIPTSPACE;
            break;
        default:
            OSL_ENSURE( !this, "wrong Id" );
            return ;
    }

    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nId );
    else
    {
        SfxBoolItem* pI = (SfxBoolItem*)GetDfltAttr( nId )->Clone();
        pI->SetValue( 0 != *pData );
        NewAttr( *pI );
        delete pI;
    }
}

void SwWW8ImplReader::Read_Emphasis( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_EMPHASIS_MARK );
    else
    {
        LanguageType nLang;
        
        
        
        
        const sal_uInt8 *pLang =
            pPlcxMan ? pPlcxMan->GetChpPLCF()->HasSprm(0x486E) : 0;

        if (pLang)
            nLang = SVBT16ToShort( pLang );
        else
        {
            nLang = ((const SvxLanguageItem *)
                GetFmtAttr(RES_CHRATR_CJK_LANGUAGE))->GetLanguage();
        }

        sal_uInt16 nVal;
        switch( *pData )
        {
        case 0:
            nVal = EMPHASISMARK_NONE;
            break;
        case 2:
            if (MsLangId::isKorean(nLang) || MsLangId::isTraditionalChinese(nLang))
                nVal = EMPHASISMARK_CIRCLE_ABOVE;
            else if (nLang == LANGUAGE_JAPANESE)
                nVal = EMPHASISMARK_SIDE_DOTS;
            else
                nVal = EMPHASISMARK_DOTS_BELOW;
            break;
        case 3:
            nVal = EMPHASISMARK_CIRCLE_ABOVE;
            break;
        case 4:
            nVal = EMPHASISMARK_DOTS_BELOW;
            break;
        case 1:
            if (MsLangId::isSimplifiedChinese(nLang))
                nVal = EMPHASISMARK_DOTS_BELOW;
            else
                nVal = EMPHASISMARK_DOTS_ABOVE;
            break;
        default:
            nVal = EMPHASISMARK_DOTS_ABOVE;
            break;
        }

        NewAttr( SvxEmphasisMarkItem( nVal, RES_CHRATR_EMPHASIS_MARK ) );
    }
}

void SwWW8ImplReader::Read_ScaleWidth( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_SCALEW );
    else
    {
        sal_uInt16 nVal = SVBT16ToShort( pData );
        
        if (nVal < 1 || nVal > 600)
            nVal = 100;
        NewAttr( SvxCharScaleWidthItem( nVal, RES_CHRATR_SCALEW ) );
    }
}

void SwWW8ImplReader::Read_Relief( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_RELIEF );
    else
    {
        if( *pData )
        {




            const SvxCharReliefItem* pOld = (const SvxCharReliefItem*)
                                            GetFmtAttr( RES_CHRATR_RELIEF );
            FontRelief nNewValue = 0x854 == nId ? RELIEF_ENGRAVED
                                        : ( 0x858 == nId ? RELIEF_EMBOSSED
                                                         : RELIEF_NONE );
            if( pOld->GetValue() == nNewValue )
            {
                if( RELIEF_NONE != nNewValue )
                    nNewValue = RELIEF_NONE;
            }
            NewAttr( SvxCharReliefItem( nNewValue, RES_CHRATR_RELIEF ));
        }
    }
}

void SwWW8ImplReader::Read_TxtAnim(sal_uInt16 /*nId*/, const sal_uInt8* pData, short nLen)
{
    if (nLen < 0)
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_CHRATR_BLINK);
    else
    {
        if (*pData)
        {
            bool bBlink;

            
            
            
            
            
            
            if (*pData > 0 && *pData < 7 )
                bBlink = true;
            else
                bBlink = false;

            NewAttr(SvxBlinkItem(bBlink, RES_CHRATR_BLINK));
        }
    }
}

SwWW8Shade::SwWW8Shade(bool bVer67, const WW8_SHD& rSHD)
{
    sal_uInt8 b = rSHD.GetFore();
    OSL_ENSURE(b < 17, "ww8: colour out of range");
    if (b >= 17)
        b = 0;

    ColorData nFore(SwWW8ImplReader::GetCol(b));

    b = rSHD.GetBack();
    OSL_ENSURE(b < 17, "ww8: colour out of range");
    if( b >=  17 )
        b = 0;

    ColorData nBack(SwWW8ImplReader::GetCol(b));

    b = rSHD.GetStyle(bVer67);

    SetShade(nFore, nBack, b);
}

void SwWW8Shade::SetShade(ColorData nFore, ColorData nBack, sal_uInt16 nIndex)
{
    static const sal_uLong eMSGrayScale[] =
    {
        
           0,   
        
        1000,   
        
          50,   
         100,   
         200,   
         250,   
         300,   
         400,   
         500,   
         600,   
         700,   
         750,   
         800,   
         900,   
        
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
         333,   
        
         500,   
         500,   
         500,   
         500,   
         500,   
         500,   
         500,   
         500,   
         500,   
        
          25,   
          75,   
         125,   
         150,   
         175,   
         225,   
         275,   
         325,   
         350,   
         375,   
         425,   
         450,   
         475,   
         525,   
         550,   
         575,   
         625,   
         650,   
         675,   
         725,   
         775,   
         825,   
         850,   
         875,   
         925,   
         950,   
         975    
    };


    
    if (nFore == COL_AUTO)
        nFore = COL_BLACK;

    
    ColorData nUseBack = nBack;
    if (nUseBack == COL_AUTO)
        nUseBack = COL_WHITE;


    if( nIndex >= sizeof( eMSGrayScale ) / sizeof ( eMSGrayScale[ 0 ] ) )
        nIndex = 0;

    sal_uLong nWW8BrushStyle = eMSGrayScale[nIndex];

    switch (nWW8BrushStyle)
    {
        case 0: 
            aColor.SetColor( nBack );
            break;
        default:
            {
                Color aForeColor(nFore);
                Color aBackColor(nUseBack);

                sal_uInt32 nRed = aForeColor.GetRed() * nWW8BrushStyle;
                sal_uInt32 nGreen = aForeColor.GetGreen() * nWW8BrushStyle;
                sal_uInt32 nBlue = aForeColor.GetBlue() * nWW8BrushStyle;
                nRed += aBackColor.GetRed()  * (1000L - nWW8BrushStyle);
                nGreen += aBackColor.GetGreen()* (1000L - nWW8BrushStyle);
                nBlue += aBackColor.GetBlue() * (1000L - nWW8BrushStyle);

                aColor.SetColor( RGB_COLORDATA( nRed/1000, nGreen/1000,
                    nBlue/1000 ) );
            }
            break;
    }
}

void SwWW8ImplReader::Read_Shade( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (!bVer67 && pPlcxMan && pPlcxMan->GetPapPLCF()->HasSprm(0xC64D))
        return;

    if (nLen <= 0)
    {
        
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BACKGROUND );
        if (bShdTxtCol)
        {
            
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
            bShdTxtCol = false;
        }
    }
    else
    {
        WW8_SHD aSHD;
        aSHD.SetWWValue( *(SVBT16*)pData );
        SwWW8Shade aSh( bVer67, aSHD );

        NewAttr(SvxBrushItem(aSh.aColor, RES_BACKGROUND));
    }
}

void SwWW8ImplReader::Read_ParaBackColor(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if (nLen <= 0)
    {
        
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BACKGROUND );
        if (bShdTxtCol)
        {
            
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
            bShdTxtCol = false;
        }
    }
    else
    {
        OSL_ENSURE(nLen == 10, "Len of para back colour not 10!");
        if (nLen != 10)
            return;
        NewAttr(SvxBrushItem(Color(ExtractColour(pData, bVer67)), RES_BACKGROUND));
    }
}

sal_uInt32 SwWW8ImplReader::ExtractColour(const sal_uInt8* &rpData, bool bVer67)
{
    (void) bVer67; 
    OSL_ENSURE(bVer67 == false, "Impossible");
    sal_uInt32 nFore = msfilter::util::BGRToRGB(SVBT32ToUInt32(rpData));
    rpData+=4;
    sal_uInt32 nBack = msfilter::util::BGRToRGB(SVBT32ToUInt32(rpData));
    rpData+=4;
    sal_uInt16 nIndex = SVBT16ToShort(rpData);
    rpData+=2;
    
    
    if (nBack == 0xFF000000)
        nBack = COL_AUTO;
    OSL_ENSURE(nBack == COL_AUTO || !(nBack & 0xFF000000),
        "ww8: don't know what to do with such a transparent bg colour, report");
    SwWW8Shade aShade(nFore, nBack, nIndex);
    return aShade.aColor.GetColor();
}

void SwWW8ImplReader::Read_Border(sal_uInt16 , const sal_uInt8*, short nLen)
{
    if( nLen < 0 )
    {
        if( bHasBorder )
        {
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BOX );
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_SHADOW );
            bHasBorder = false;
        }
    }
    else if( !bHasBorder )
    {
        
        
        
        bHasBorder = true;

        WW8_BRC5 aBrcs;   
        sal_uInt8 nBorder;

        if( pAktColl )
            nBorder = ::lcl_ReadBorders(bVer67, aBrcs, 0, pStyles);
        else
            nBorder = ::lcl_ReadBorders(bVer67, aBrcs, pPlcxMan ? pPlcxMan->GetPapPLCF() : NULL);

        if( nBorder )                                   
        {
            bool bIsB = IsBorder(aBrcs, true);
            if (!InLocalApo() || !bIsB ||
                (pWFlyPara && !pWFlyPara->bBorderLines ))
            {
                
                
                
                

                
                
                
                const SvxBoxItem* pBox
                    = (const SvxBoxItem*)GetFmtAttr( RES_BOX );
                SvxBoxItem aBox(RES_BOX);
                if (pBox)
                    aBox = *pBox;
                short aSizeArray[5]={0};

                SetBorder(aBox, aBrcs, &aSizeArray[0], nBorder);

                Rectangle aInnerDist;
                GetBorderDistance( aBrcs, aInnerDist );

                if ((nBorder & WW8_LEFT)==WW8_LEFT)
                    aBox.SetDistance( (sal_uInt16)aInnerDist.Left(), BOX_LINE_LEFT );

                if ((nBorder & WW8_TOP)==WW8_TOP)
                    aBox.SetDistance( (sal_uInt16)aInnerDist.Top(), BOX_LINE_TOP );

                if ((nBorder & WW8_RIGHT)==WW8_RIGHT)
                    aBox.SetDistance( (sal_uInt16)aInnerDist.Right(), BOX_LINE_RIGHT );

                if ((nBorder & WW8_BOT)==WW8_BOT)
                    aBox.SetDistance( (sal_uInt16)aInnerDist.Bottom(), BOX_LINE_BOTTOM );

                NewAttr( aBox );

                SvxShadowItem aS(RES_SHADOW);
                if( SetShadow( aS, &aSizeArray[0], aBrcs[WW8_RIGHT] ) )
                    NewAttr( aS );
            }
        }
    }
}

void SwWW8ImplReader::Read_CharBorder(sal_uInt16 /*nId*/, const sal_uInt8* pData, short nLen )
{
    
    if (!bVer67 && pPlcxMan && pPlcxMan->GetChpPLCF()->HasSprm(0xCA72))
        return;

    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BOX );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_SHADOW );
    }
    else
    {
        const SvxBoxItem* pBox
            = (const SvxBoxItem*)GetFmtAttr( RES_CHRATR_BOX );
        if( pBox )
        {
            SvxBoxItem aBoxItem(RES_CHRATR_BOX);
            aBoxItem = *pBox;
            WW8_BRC aBrc;
            _SetWW8_BRC(bVer67, aBrc, pData);

            Set1Border(bVer67, aBoxItem, aBrc, BOX_LINE_TOP, 0, 0, true);
            Set1Border(bVer67, aBoxItem, aBrc, BOX_LINE_BOTTOM, 0, 0, true);
            Set1Border(bVer67, aBoxItem, aBrc, BOX_LINE_LEFT, 0, 0, true);
            Set1Border(bVer67, aBoxItem, aBrc, BOX_LINE_RIGHT, 0, 0, true);
            NewAttr( aBoxItem );

            short aSizeArray[WW8_RIGHT+1]={0}; aSizeArray[WW8_RIGHT] = 1;
            SvxShadowItem aShadowItem(RES_CHRATR_SHADOW);
            if( SetShadow( aShadowItem, &aSizeArray[0], aBrc ) )
                NewAttr( aShadowItem );
        }
    }


}

void SwWW8ImplReader::Read_Hyphenation( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_HYPHENZONE );
    else
    {
        SvxHyphenZoneItem aAttr(
            *(const SvxHyphenZoneItem*)GetFmtAttr( RES_PARATR_HYPHENZONE ) );

        aAttr.SetHyphen( 0 == *pData ); 

        if( !*pData )
        {
            aAttr.GetMinLead()    = 2;
            aAttr.GetMinTrail()   = 2;
            aAttr.GetMaxHyphens() = 0;
        }

        NewAttr( aAttr );
    }
}

void SwWW8ImplReader::Read_WidowControl( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_WIDOWS );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_ORPHANS );
    }
    else
    {
        sal_uInt8 nL = ( *pData & 1 ) ? 2 : 0;

        NewAttr( SvxWidowsItem( nL, RES_PARATR_WIDOWS ) );     
        NewAttr( SvxOrphansItem( nL, RES_PARATR_ORPHANS ) );

        if( pAktColl && pStyles )           
            pStyles->bWidowsChanged = true; 
                                            
    }
}

void SwWW8ImplReader::Read_UsePgsuSettings(sal_uInt16,const sal_uInt8* pData,short nLen)
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_SNAPTOGRID);
    else
    {
        if(nInTable)
            NewAttr( SvxParaGridItem(false, RES_PARATR_SNAPTOGRID) );
        else
            NewAttr( SvxParaGridItem(*pData, RES_PARATR_SNAPTOGRID) );
    }
}

void SwWW8ImplReader::Read_AlignFont( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_VERTALIGN);
    else
    {
        sal_uInt16 nVal = SVBT16ToShort( pData );
        switch (nVal)
        {
            case 0:
                nVal = SvxParaVertAlignItem::TOP;
                break;
            case 1:
                nVal = SvxParaVertAlignItem::CENTER;
                break;
            case 2:
                nVal = SvxParaVertAlignItem::BASELINE;
                break;
            case 3:
                nVal = SvxParaVertAlignItem::BOTTOM;
                break;
            case 4:
                nVal = SvxParaVertAlignItem::AUTOMATIC;
                break;
            default:
                nVal = SvxParaVertAlignItem::AUTOMATIC;
                OSL_ENSURE(!this,"Unknown paragraph vertical align");
                break;
        }
        NewAttr( SvxParaVertAlignItem( nVal, RES_PARATR_VERTALIGN ) );
    }
}

void SwWW8ImplReader::Read_KeepLines( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_SPLIT );
    else
        NewAttr( SvxFmtSplitItem( ( *pData & 1 ) == 0, RES_PARATR_SPLIT ) );
}

void SwWW8ImplReader::Read_KeepParas( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_KEEP );
    else
        NewAttr( SvxFmtKeepItem( ( *pData & 1 ) != 0 , RES_KEEP) );
}

void SwWW8ImplReader::Read_BreakBefore( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BREAK );
    else
        NewAttr( SvxFmtBreakItem(
                ( *pData & 1 ) ? SVX_BREAK_PAGE_BEFORE : SVX_BREAK_NONE, RES_BREAK ) );
}

void SwWW8ImplReader::Read_ApoPPC( sal_uInt16, const sal_uInt8* pData, short )
{
    if (pAktColl && nAktColl < vColl.size()) 
    {
        SwWW8StyInf& rSI = vColl[nAktColl];
        WW8FlyPara* pFly = rSI.pWWFly ? rSI.pWWFly : new WW8FlyPara(bVer67);
        vColl[nAktColl].pWWFly = pFly;
        pFly->Read(*pData, pStyles);
        if (pFly->IsEmpty())
            delete vColl[nAktColl].pWWFly, vColl[nAktColl].pWWFly = 0;
    }
}

bool SwWW8ImplReader::ParseTabPos(WW8_TablePos *pTabPos, WW8PLCFx_Cp_FKP* pPap)
{
    bool bRet = false;
    const sal_uInt8 *pRes=0;
    memset(pTabPos, 0, sizeof(WW8_TablePos));
    if (0 != (pRes = pPap->HasSprm(0x360D)))
    {
        pTabPos->nSp29 = *pRes;
        pTabPos->nSp37 = 2;     
        if (0 != (pRes = pPap->HasSprm(0x940E)))
            pTabPos->nSp26 = SVBT16ToShort(pRes);
        if (0 != (pRes = pPap->HasSprm(0x940F)))
            pTabPos->nSp27 = SVBT16ToShort(pRes);
        if (0 != (pRes = pPap->HasSprm(0x9410)))
            pTabPos->nLeMgn = SVBT16ToShort(pRes);
        if (0 != (pRes = pPap->HasSprm(0x941E)))
            pTabPos->nRiMgn = SVBT16ToShort(pRes);
        if (0 != (pRes = pPap->HasSprm(0x9411)))
            pTabPos->nUpMgn = SVBT16ToShort(pRes);
        if (0 != (pRes = pPap->HasSprm(0x941F)))
            pTabPos->nLoMgn = SVBT16ToShort(pRes);
        bRet = true;
    }
    if (0 != (pRes = pPap->HasSprm(NS_sprm::LN_TDefTable)))
    {
        WW8TabBandDesc aDesc;
        aDesc.ReadDef(false, pRes);
        int nTableWidth = aDesc.nCenter[aDesc.nWwCols] - aDesc.nCenter[0];
        int nTextAreaWidth = maSectionManager.GetTextAreaWidth();
        
        
        
        pTabPos->bNoFly = nTableWidth >= nTextAreaWidth;
    }
    return bRet;
}

/***************************************************************************
#  Seiten - Attribute werden nicht mehr als Attribute gehandhabt
#   ( ausser OLST )
#**************************************************************************/


long SwWW8ImplReader::ImportExtSprm(WW8PLCFManResult* pRes)
{
    /*************************************************************************
    #       Arrays zum Lesen der erweiterten ( selbstdefinierten ) SPRMs
    #*************************************************************************/
    typedef long (SwWW8ImplReader:: *FNReadRecordExt)(WW8PLCFManResult*);

    static const FNReadRecordExt aWwSprmTab[] =
    {
        /* 0 (256) */   &SwWW8ImplReader::Read_Ftn,     
        /* 1 (257) */   &SwWW8ImplReader::Read_Ftn,     
        /* 2 (258) */   &SwWW8ImplReader::Read_Field,  
        /* 3 (259) */   &SwWW8ImplReader::Read_Book,   
        /* 4 (260) */   &SwWW8ImplReader::Read_And     
    };

    if( pRes->nSprmId < 280 )
    {
        sal_uInt8 nIdx = static_cast< sal_uInt8 >(pRes->nSprmId - eFTN);
        if( nIdx < SAL_N_ELEMENTS(aWwSprmTab)
            && aWwSprmTab[nIdx] )
            return (this->*aWwSprmTab[nIdx])(pRes);
        else
            return 0;
    }
    else
        return 0;
}

void SwWW8ImplReader::EndExtSprm(sal_uInt16 nSprmId)
{
    typedef sal_uInt16 (SwWW8ImplReader:: *FNReadRecordExt)();

    static const FNReadRecordExt aWwSprmTab[] =
    {
        /* 0 (256) */   &SwWW8ImplReader::End_Ftn,      
        /* 1 (257) */   &SwWW8ImplReader::End_Ftn,      
        /* 2 (258) */   &SwWW8ImplReader::End_Field,  
        /* 3 (259) */   0,   
        /* 4 (260) */   0     
    };

    sal_uInt8 nIdx = static_cast< sal_uInt8 >(nSprmId - eFTN);
    if( nIdx < SAL_N_ELEMENTS(aWwSprmTab)
        && aWwSprmTab[nIdx] )
        (this->*aWwSprmTab[nIdx])();
}

/***************************************************************************
#       Arrays zum Lesen der SPRMs
#**************************************************************************/


typedef void (SwWW8ImplReader:: *FNReadRecord)( sal_uInt16, const sal_uInt8*, short );

struct SprmReadInfo
{
    sal_uInt16       nId;
    FNReadRecord pReadFnc;
};

bool operator==(const SprmReadInfo &rFirst, const SprmReadInfo &rSecond)
{
    return (rFirst.nId == rSecond.nId);
}

bool operator<(const SprmReadInfo &rFirst, const SprmReadInfo &rSecond)
{
    return (rFirst.nId < rSecond.nId);
}

typedef ww::SortedArray<SprmReadInfo> wwSprmDispatcher;

const wwSprmDispatcher *GetWW2SprmDispatcher()
{
    static SprmReadInfo aSprms[] =
    {
          {0, 0},                                    
                                                     
          {2, &SwWW8ImplReader::Read_StyleCode},     
                                                     
          {3, 0},                                    
                                                     
          {4, 0},                                    
                                                     
          {5, &SwWW8ImplReader::Read_Justify},       
                                                     
          {6, 0},                                    
                                                     
          {7, &SwWW8ImplReader::Read_KeepLines},     
          {8, &SwWW8ImplReader::Read_KeepParas},     
                                                     
          {9, &SwWW8ImplReader::Read_BreakBefore},   
                                                     
         {10, 0},                                    
         {11, 0},                                    
         {12, &SwWW8ImplReader::Read_ANLevelDesc},   
                                                     
         {13, &SwWW8ImplReader::Read_ANLevelNo},     
                                                     
         {14, &SwWW8ImplReader::Read_NoLineNumb},    
         {15, &SwWW8ImplReader::Read_Tab},           
                                                     
         {16, &SwWW8ImplReader::Read_LR},            
         {17, &SwWW8ImplReader::Read_LR},            
         {18, 0},                                    
         {19, &SwWW8ImplReader::Read_LR},            
         {20, &SwWW8ImplReader::Read_LineSpace},     
                                                     
         {21, &SwWW8ImplReader::Read_UL},            
                                                     
         {22, &SwWW8ImplReader::Read_UL},            
         {23, 0},                                    
                                                     
         {24, 0},                                    
         {25, &SwWW8ImplReader::Read_TabRowEnd},     
         {26, 0},                                    
         {27, 0},                                    
         {28, 0},                                    
         {29, &SwWW8ImplReader::Read_ApoPPC},        
                                                     
         {30, 0},                                    
                                                     
         {31, 0},                                    
                                                     
         {32, 0},                                    
                                                     
         {33, 0},                                    
                                                     
         {34, 0},                                    
                                                     
         {35, 0},                                    
                                                     
         {36, 0},                                    
                                                     
         {37, 0},                                    
         {38, &SwWW8ImplReader::Read_Border},        
         {39, &SwWW8ImplReader::Read_Border},        
                                                     
         {40, &SwWW8ImplReader::Read_Border},        
                                                     
         {41, &SwWW8ImplReader::Read_Border},        
                                                     
         {42, &SwWW8ImplReader::Read_Border},        
                                                     
         {43, 0},                                    
                                                     
         {44, &SwWW8ImplReader::Read_Hyphenation},   
                                                     
         {45, 0},                                    
                                                     
         {46, 0},                                    
         {47, &SwWW8ImplReader::Read_Shade},         
         {48, 0},                                    
                                                     
         {49, 0},                                    
                                                     
         {50, 0},                                    
                                                     
         {51, &SwWW8ImplReader::Read_WidowControl},  
                                                     
         {52, 0},                                    
         {53, 0},                                    
         {54, 0},                                    
         {55, 0},                                    
         {56, 0},                                    
         {57, 0},                                    
         {58, 0},                                    
         {59, 0},                                    

         {60, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {61, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {62, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {63, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {64, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {65, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
                                                     
         {66, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {67, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {68, &SwWW8ImplReader::Read_FontCode},      
         {69, &SwWW8ImplReader::Read_Underline},     
         {70, 0},                                    
                                                     
         {71, &SwWW8ImplReader::Read_Kern},          
                                                     
         {72, &SwWW8ImplReader::Read_Language},      
         {73, &SwWW8ImplReader::Read_TxtColor},      
         {74, &SwWW8ImplReader::Read_FontSize},      
         {75, 0},                                    
         {76, &SwWW8ImplReader::Read_SubSuperProp},  
                                                     
         {77, 0},                                    
                                                     
         {78, &SwWW8ImplReader::Read_Majority},      
                                                     
         {80, &SwWW8ImplReader::Read_BoldBiDiUsw},   
         {81, &SwWW8ImplReader::Read_BoldBiDiUsw},   
         {82, &SwWW8ImplReader::Read_FontCode},      
         {83, &SwWW8ImplReader::Read_Language},      
         {84, &SwWW8ImplReader::Read_TxtColor},      
         {85, &SwWW8ImplReader::Read_FontSize},      
         {86, 0},                                    
         {87, 0},                                    
         {94, 0},                                    
                                                     
                                                     
         {95, 0},                                    
                                                     
         {96, 0},                                    
                                                     
         {97, 0},                                    
                                                     
         {98, 0},                                    
                                                     
         {99, 0}                                     
    };

    static wwSprmDispatcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
}

const wwSprmDispatcher *GetWW6SprmDispatcher()
{
    static SprmReadInfo aSprms[] =
    {
          {0, 0},                                    
                                                     
          {2, &SwWW8ImplReader::Read_StyleCode},     
                                                     
          {3, 0},                                    
                                                     
          {4, 0},                                    
                                                     
          {5, &SwWW8ImplReader::Read_Justify},       
                                                     
          {6, 0},                                    
                                                     
          {7, &SwWW8ImplReader::Read_KeepLines},     
          {8, &SwWW8ImplReader::Read_KeepParas},     
                                                     
          {9, &SwWW8ImplReader::Read_BreakBefore},   
                                                     
         {10, 0},                                    
         {11, 0},                                    
         {12, &SwWW8ImplReader::Read_ANLevelDesc},   
                                                     
         {13, &SwWW8ImplReader::Read_ANLevelNo},     
                                                     
         {14, &SwWW8ImplReader::Read_NoLineNumb},    
         {15, &SwWW8ImplReader::Read_Tab},           
                                                     
         {16, &SwWW8ImplReader::Read_LR},            
         {17, &SwWW8ImplReader::Read_LR},            
         {18, 0},                                    
         {19, &SwWW8ImplReader::Read_LR},            
         {20, &SwWW8ImplReader::Read_LineSpace},     
                                                     
         {21, &SwWW8ImplReader::Read_UL},            
                                                     
         {22, &SwWW8ImplReader::Read_UL},            
         {23, 0},                                    
                                                     
         {24, 0},                                    
         {25, &SwWW8ImplReader::Read_TabRowEnd},     
         {26, 0},                                    
         {27, 0},                                    
         {28, 0},                                    
         {29, &SwWW8ImplReader::Read_ApoPPC},        
                                                     
         {30, 0},                                    
                                                     
         {31, 0},                                    
                                                     
         {32, 0},                                    
                                                     
         {33, 0},                                    
                                                     
         {34, 0},                                    
                                                     
         {35, 0},                                    
                                                     
         {36, 0},                                    
                                                     
         {37, 0},                                    
         {38, &SwWW8ImplReader::Read_Border},        
         {39, &SwWW8ImplReader::Read_Border},        
                                                     
         {40, &SwWW8ImplReader::Read_Border},        
                                                     
         {41, &SwWW8ImplReader::Read_Border},        
                                                     
         {42, &SwWW8ImplReader::Read_Border},        
                                                     
         {43, 0},                                    
                                                     
         {44, &SwWW8ImplReader::Read_Hyphenation},   
                                                     
         {45, 0},                                    
                                                     
         {46, 0},                                    
         {47, &SwWW8ImplReader::Read_Shade},         
         {48, 0},                                    
                                                     
         {49, 0},                                    
                                                     
         {50, 0},                                    
                                                     
         {51, &SwWW8ImplReader::Read_WidowControl},  
                                                     
         {52, 0},                                    
         {53, 0},                                    
         {54, 0},                                    
         {55, 0},                                    
         {56, 0},                                    
         {57, 0},                                    
         {58, 0},                                    
         {59, 0},                                    
         {60, 0},                                    
         {61, 0},                                    
         {62, 0},                                    
         {63, 0},                                    
         {64, &SwWW8ImplReader::Read_ParaBiDi},      
         {65, &SwWW8ImplReader::Read_CFRMarkDel},    
                                                     
         {66, &SwWW8ImplReader::Read_CFRMark},       
                                                     
         {67, &SwWW8ImplReader::Read_FldVanish},     
                                                     
         {68, &SwWW8ImplReader::Read_PicLoc},        
                                                     
         {69, 0},                                    
                                                     
                                                     
         {70, 0},                                    
                                                     
         {71, 0},                                    
                                                     
         {72, 0},                                    
                                                     
                                                     
         {73, &SwWW8ImplReader::Read_CharSet},       
                                                     
         {74, &SwWW8ImplReader::Read_Symbol},        
                                                     
         {75, &SwWW8ImplReader::Read_Obj},           
                                                     
         {76, 0},                                    
         {77, 0},                                    
         {78, 0},                                    
         {79, 0},                                    
         {80, &SwWW8ImplReader::Read_CColl},         
                                                     
                                                     
         {81, 0},                                    
                                                     
         {82, 0},                                    
                                                     
         {83, 0},                                    
                                                     
         {84, 0},                                    
         {85, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {86, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {87, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {88, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {89, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {90, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
                                                     
         {91, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {92, &SwWW8ImplReader::Read_BoldUsw},       
                                                     
         {93, &SwWW8ImplReader::Read_FontCode},      
         {94, &SwWW8ImplReader::Read_Underline},     
         {95, 0},                                    
                                                     
         {96, &SwWW8ImplReader::Read_Kern},          
                                                     
         {97, &SwWW8ImplReader::Read_Language},      
         {98, &SwWW8ImplReader::Read_TxtColor},      
         {99, &SwWW8ImplReader::Read_FontSize},      
        {100, 0},                                    
        {101, &SwWW8ImplReader::Read_SubSuperProp},  
                                                     
        {102, 0},                                    
                                                     
        {103, &SwWW8ImplReader::Read_Majority},      
                                                     
        {104, &SwWW8ImplReader::Read_SubSuper},      
        {105, 0},                                    
                                                     
                                                     
        {106, 0},                                    
                                                     
                                                     
        {107, &SwWW8ImplReader::Read_FontKern},      
                                                     
        {108, &SwWW8ImplReader::Read_Majority},      
                                                     
                                                     
        {109, 0},                                    
                                                     
        {110, 0},                                    
                                                     
        {111, &SwWW8ImplReader::Read_FontCode},      
        {112, &SwWW8ImplReader::Read_FontCode},      
        {113, &SwWW8ImplReader::Read_FontCode},      
        {114, &SwWW8ImplReader::Read_Language},      
        {115, &SwWW8ImplReader::Read_TxtColor},      
        {116, &SwWW8ImplReader::Read_FontSize},
        {117, &SwWW8ImplReader::Read_Special},       
                                                     
        {118, &SwWW8ImplReader::Read_Obj},           
                                                     
        {119, 0},                                    
                                                     
                                                     
        {120, 0},                                    
                                                     
        {121, 0},                                    
                                                     
        {122, 0},                                    
                                                     
        {123, 0},                                    
                                                     
        {124, 0},                                    
                                                     
        {125, 0},                                    
        {126, 0},                                    
        {127, 0},                                    
        {128, 0},                                    
        {129, 0},                                    
        {130, 0},                                    
        {131, 0},                                    
                                                     
        {132, 0},                                    
                                                     
                                                     
        {133, &SwWW8ImplReader::Read_OLST},          
                                                     
        {134, 0},                                    
        {135, 0},                                    
        {136, 0},                                    
                                                     
                                                     
        {137, 0},                                    
                                                     
                                                     
        {138, 0},                                    
                                                     
        {139, 0},                                    
                                                     
        {140, 0},                                    
                                                     
        {141, 0},                                    
                                                     
        {142, 0},                                    
                                                     
        {143, 0},                                    
                                                     
        {144, 0},                                    
                                                     
        {145, 0},                                    
                                                     
        {146, 0},                                    
                                                     
        {147, 0},                                    
                                                     
        {148, 0},                                    
                                                     
        {149, 0},                                    
                                                     
        {150, 0},                                    
                                                     
        {151, 0},                                    
                                                     
        {152, 0},                                    
        {153, 0},                                    
                                                     
        {154, 0},                                    
                                                     
        {155, 0},                                    
                                                     
        {156, 0},                                    
                                                     
        {157, 0},                                    
                                                     
        {158, 0},                                    
                                                     
        {159, 0},                                    
        {160, 0},                                    
                                                     
        {161, 0},                                    
                                                     
        {162, 0},                                    
                                                     
        {163, 0},                                    
        {164, 0},                                    
                                                     
        {165, 0},                                    
                                                     
        {166, 0},                                    
                                                     
        {167, 0},                                    
                                                     
        {168, 0},                                    
        {169, 0},                                    
                                                     
        {170, 0},                                    
                                                     
        {171, 0},                                    
                                                     
        {172, 0},                                    
        {173, 0},                                    
        {174, 0},                                    
        {175, 0},                                    
        {176, 0},                                    
        {177, 0},                                    
        {178, 0},                                    
        {179, 0},                                    
        {180, 0},                                    
        {181, 0},                                    
        {182, 0},                                    
                                                     
                                                     
        {183, 0},                                    
                                                     
        {184, 0},                                    
                                                     
                                                     
        {185, 0},                                    
                                                     
        {186, 0},                                    
                                                     
        {187, 0},                                    
                                                     
                                                     
        {188, 0},                                    
                                                     
                                                     
        {189, 0},                                    
                                                     
        {190, 0},                                    
                                                     
        {191, 0},                                    
                                                     
        {192, 0},                                    
                                                     
        {193, 0},                                    
                                                     
                                                     
        {194, 0},                                    
                                                     
                                                     
        {195, 0},                                    
                                                     
                                                     
        {196, 0},                                    
                                                     
                                                     
        {197, 0},                                    
                                                     
                                                     
        {198, 0},                                    
                                                     
                                                     
        {199, 0},                                    
                                                     
                                                     
        {200, 0},                                    
                                                     
        {207, 0},                                    
    };

    static wwSprmDispatcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
}

const wwSprmDispatcher *GetWW8SprmDispatcher()
{
    static SprmReadInfo aSprms[] =
    {
        {0,      0},                                 

        {0x4600, &SwWW8ImplReader::Read_StyleCode},  
                                                     
        {0xC601, 0},                                 
                                                     
                                                     
        {0x2602, 0},                                 
                                                     
                                                     
                                                     
        {0x2403, &SwWW8ImplReader::Read_Justify},    
                                                     
        {0x2404, 0},                                 
                                                     
        {0x2405, &SwWW8ImplReader::Read_KeepLines},  
                                                     
        {0x2406, &SwWW8ImplReader::Read_KeepParas},  
                                                     
        {0x2407, &SwWW8ImplReader::Read_BreakBefore},
                                                     
                                                     
        {0x2408, 0},                                 
                                                     
        {0x2409, 0},                                 
                                                     
        {0x260A, &SwWW8ImplReader::Read_ListLevel},  
                                                     
        {0x460B, &SwWW8ImplReader::Read_LFOPosition},
                                                     
        {0x240C, &SwWW8ImplReader::Read_NoLineNumb}, 
                                                     
        {0xC60D, &SwWW8ImplReader::Read_Tab},        
                                                     
                                                     
                                                     
        {0x840E, &SwWW8ImplReader::Read_LR},         
                                                     
        {0x840F, &SwWW8ImplReader::Read_LR},         
                                                     
        {0x4610, 0},                                 
                                                     
        {0x8411, &SwWW8ImplReader::Read_LR},         
                                                     
        {0x6412, &SwWW8ImplReader::Read_LineSpace},  
                                                     
                                                     
                                                     
                                                     
                                                     
        {0xA413, &SwWW8ImplReader::Read_UL},         
                                                     
        {0xA414, &SwWW8ImplReader::Read_UL},         
                                                     
        {0xC615, 0},                                 
                                                     
                                                     
        {0x2416, 0},                                 
                                                     
        {0x2417, &SwWW8ImplReader::Read_TabRowEnd},  
                                                     
        {0x8418, 0},                                 
                                                     
        {0x8419, 0},                                 
                                                     
        {0x841A, 0},                                 
                                                     
        {0x261B, &SwWW8ImplReader::Read_ApoPPC},     
                                                     
        {0x461C, 0},                                 
                                                     
        {0x461D, 0},                                 
                                                     
        {0x461E, 0},                                 
                                                     
        {0x461F, 0},                                 
                                                     
        {0x4620, 0},                                 
                                                     
        {0x4621, 0},                                 
                                                     
        {0x4622, 0},                                 
                                                     
        {0x2423, 0},                                 
        {0x6424, &SwWW8ImplReader::Read_Border},     
                                                     
        {0x6425, &SwWW8ImplReader::Read_Border},     
                                                     
        {0x6426, &SwWW8ImplReader::Read_Border},     
                                                     
        {0x6427, &SwWW8ImplReader::Read_Border},     
                                                     
        {0x6428, &SwWW8ImplReader::Read_Border},     
                                                     
        {0x6629, 0},                                 
                                                     
        {0x242A, &SwWW8ImplReader::Read_Hyphenation},
                                                     
        {0x442B, 0},                                 
                                                     
        {0x442C, 0},                                 
        {0x442D, &SwWW8ImplReader::Read_Shade},      
        {0x842E, 0},                                 
                                                     
        {0x842F, 0},                                 
                                                     
        {0x2430, 0},                                 
                                                     
        {0x2431, &SwWW8ImplReader::Read_WidowControl},
                                                     
        {0xC632, 0},                                 
        {0x2433, &SwWW8ImplReader::Read_BoolItem},   
                                                     
        {0x2434, 0},                                 
                                                     
        {0x2435, &SwWW8ImplReader::Read_BoolItem},   
                                                     
                                                     
        {0x2436, 0},                                 
                                                     
        {0x2437, &SwWW8ImplReader::Read_BoolItem},   
                                                     
        {0x2438, 0},                                 
                                                     
        {NS_sprm::LN_PWAlignFont, &SwWW8ImplReader::Read_AlignFont},  
                                                     
        {0x443A, 0},                                 
                                                     
                                                     
        {0x243B, 0},                                 
                                                     
                                                     
        {0xC63E, &SwWW8ImplReader::Read_ANLevelDesc},
                                                     
        {0xC63F, 0},                                 
                                                     
                                                     
        {0x2640,  &SwWW8ImplReader::Read_POutLvl},   
                                                     
                                                     
        {0x2441, &SwWW8ImplReader::Read_ParaBiDi},   
        {0x2443, 0},                                 
                                                     
        {0x2444, 0},                                 
        {0xC645, 0},                                 
                                                     
        {0x6645, 0},                                 
                                                     
                                                     
        {0x6646, 0},                                 
                                                     
                                                     
        {0x2447, &SwWW8ImplReader::Read_UsePgsuSettings},
                                                     
                                                     
        {0x2448, 0},                                 
                                                     
        {0x0800, &SwWW8ImplReader::Read_CFRMarkDel}, 
                                                     
        {0x0801, &SwWW8ImplReader::Read_CFRMark},    
                                                     
        {0x0802, &SwWW8ImplReader::Read_FldVanish},  
                                                     
        {0x6A03, &SwWW8ImplReader::Read_PicLoc},     
                                                     
                                                     
                                                     
        {0x4804, 0},                                 
                                                     
                                                     
        {0x6805, 0},                                 
                                                     
        {0x0806, 0},                                 
                                                     
        {0x4807, 0},                                 
                                                     
                                                     
                                                     
        {0xEA08, &SwWW8ImplReader::Read_CharSet},    
                                                     
        {0x6A09, &SwWW8ImplReader::Read_Symbol},     
                                                     
                                                     
                                                     
        {0x080A, &SwWW8ImplReader::Read_Obj},        
                                                     
      
                                                     
                                                     
        {0x2A0C, &SwWW8ImplReader::Read_CharHighlight},
                                                     
                                                     
                                                     
                                                     
        {0x680E, &SwWW8ImplReader::Read_PicLoc},     
                                                     
      
        {0x4A30, &SwWW8ImplReader::Read_CColl},      
                                                     
        {0xCA31, 0},                                 
                                                     
                                                     
        {0x2A32, 0},                                 
                                                     
        {0x2A33, 0},                                 
                                                     
        {0x2A34, &SwWW8ImplReader::Read_Emphasis},   
        {0x0835, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
        {0x0836, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
        {0x0837, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
        {0x0838, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
        {0x0839, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
        {0x083A, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
                                                     
        {0x083B, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
        {0x083C, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
      
                                                     
                                                     
        {0x2A3E, &SwWW8ImplReader::Read_Underline},  
        {0xEA3F, 0},                                 
                                                     
        {0x8840, &SwWW8ImplReader::Read_Kern},       
                                                     
        {0x4A41, &SwWW8ImplReader::Read_Language},   
                                                     
        {0x2A42, &SwWW8ImplReader::Read_TxtColor},   
        {0x4A43, &SwWW8ImplReader::Read_FontSize},   
        {0x2A44, 0},                                 
        {0x4845, &SwWW8ImplReader::Read_SubSuperProp},
                                                     
        {0x2A46, 0},                                 
                                                     
        {0xCA47, &SwWW8ImplReader::Read_Majority},   
                                                     
                                                     
                                                     
                                                     
                                                     
                                                     
                                                     
                                                     
        {0x2A48, &SwWW8ImplReader::Read_SubSuper},   
        {0xCA49, 0},                                 
                                                     
                                                     
        {0xCA4A, 0},                                 
                                                     
                                                     
        {0x484B, &SwWW8ImplReader::Read_FontKern},   
                                                     
        {0xCA4C, &SwWW8ImplReader::Read_Majority},   
                                                     
                                                     
                                                     
                                                     
                                                     
                                                     
        {0x4A4D, 0},                                 
                                                     
        {0x484E, 0},                                 
                                                     
        {0x4A4F, &SwWW8ImplReader::Read_FontCode},   
                                                     
        {0x4A50, &SwWW8ImplReader::Read_FontCode},   
                                                     
        {0x4A51, &SwWW8ImplReader::Read_FontCode},   
                                                     
                                                     
        {0x4852, &SwWW8ImplReader::Read_ScaleWidth}, 
        {0x2A53, &SwWW8ImplReader::Read_BoldUsw},    
                                                     
        {0x0854, &SwWW8ImplReader::Read_Relief},     
                                                     
        {0x0855, &SwWW8ImplReader::Read_Special},    
                                                     
        {0x0856, &SwWW8ImplReader::Read_Obj},        
                                                     
        {0xCA57, &SwWW8ImplReader::Read_CPropRMark}, 
                                                     
                                                     
                                                     
                                                     
                                                     
        {0x0858, &SwWW8ImplReader::Read_Relief},     
                                                     
        {0x2859, &SwWW8ImplReader::Read_TxtAnim},    
                                                     
        {0x085A, &SwWW8ImplReader::Read_Bidi},                                 
        {0x085B, 0},                                 
        {0x085C, &SwWW8ImplReader::Read_BoldBiDiUsw},
        {0x085D, &SwWW8ImplReader::Read_BoldBiDiUsw},
        {0x4A5E, &SwWW8ImplReader::Read_FontCode},   
        {0x485F, &SwWW8ImplReader::Read_Language},   
      
        {0x4A61, &SwWW8ImplReader::Read_FontSize},   
        {0xCA62, 0},                                 
                                                     
                                                     
                                                     
                                                     
                                                     
        {0x4863, 0},                                 
                                                     
                                                     
        {NS_sprm::LN_CDttmRMarkDel, 0},
                                                     
        {0x6865, &SwWW8ImplReader::Read_CharBorder}, 
        {0x4866, &SwWW8ImplReader::Read_CharShadow}, 
        {0x4867, 0},                                 
                                                     
                                                     
                                                     
                                                     
        {0x0868, 0},                                 
                                                     
                                                     
        {0x486B, 0},                                 
        {0x486D, &SwWW8ImplReader::Read_Language},   
                                                     
                                                     
        {0x486E, &SwWW8ImplReader::Read_Language},   
                                                     
        {0x286F, &SwWW8ImplReader::Read_IdctHint},   
                                                     
        {0x2E00, 0},                                 
                                                     
                                                     
        {0xCE01, 0},                                 
                                                     
                                                     
                                                     
                                                     
                                                     
        {0x6C02, 0},                                 
                                                     
        {0x6C03, 0},                                 
                                                     
        {0x6C04, 0},                                 
                                                     
        {0x6C05, 0},                                 
                                                     
        {0x3000, 0},                                 
                                                     
        {0x3001, 0},                                 
                                                     
                                                     
        {0xD202, &SwWW8ImplReader::Read_OLST},       
                                                     
        {0xF203, 0},                                 
                                                     
                                                     
        {0xF204, 0},                                 
                                                     
                                                     
        {0x3005, 0},                                 
                                                     
                                                     
        {0x3006, 0},                                 
                                                     
        {0x5007, 0},                                 
                                                     
        {0x5008, 0},                                 
                                                     
        {0x3009, 0},                                 
        {0x300A, 0},                                 
                                                     
        {0x500B, 0},                                 
                                                     
        {0x900C, 0},                                 
                                                     
        {0x300D, 0},                                 
                                                     
        {0x300E, 0},                                 
                                                     
        {0xB00F, 0},                                 
                                                     
        {0xB010, 0},                                 
                                                     
        {0x3011, 0},                                 
                                                     
        {0x3012, 0},                                 
                                                     
        {0x3013, 0},                                 
        {0x3014, 0},                                 
                                                     
        {0x5015, 0},                                 
                                                     
        {0x9016, 0},                                 
                                                     
        {0xB017, 0},                                 
                                                     
        {0xB018, 0},                                 
                                                     
        {0x3019, 0},                                 
                                                     
        {0x301A, 0},                                 
        {0x501B, 0},                                 
                                                     
        {0x501C, 0},                                 
                                                     
        {0x301D, 0},                                 
                                                     
      
        {0xB01F, 0},                                 
                                                     
        {0xB020, 0},                                 
                                                     
        {0x2205, 0},                                 
                                                     
        {0xB022, 0},                                 
                                                     
        {0x9023, 0},                                 
                                                     
        {0x9024, 0},                                 
                                                     
        {0xB025, 0},                                 
                                                     
        {0x5026, 0},                                 
                                                     
        {0xD227, 0},                                 
                                                     
                                                     
                                                     
                                                     
                                                     
      
      
        {0x322A, 0},                                 
                                                     
        {0x702B, 0},                                 
                                                     
        {0x702C, 0},                                 
                                                     
        {0x702D, 0},                                 
                                                     
        {0x702E, 0},                                 
                                                     
        {0x522F, 0},                                 
                                                     
        {0x7030, 0},                                 
                                                     
        {0x9031, 0},                                 
                                                     
                                                     
      
        {0x5033, 0},                                 
                                                     
        {0x5400, 0},                                 
                                                     
        {0x9601, 0},                                 
                                                     
        {0x9602, 0},                                 
                                                     
                                                     
        {0x3403, 0},                                 
                                                     
        {0x3404, 0},                                 
                                                     
        {0x3466, 0},                                 
                                                     
        {0xD605, 0},                                 
                                                     
                                                     
        {0xD606, 0},                                 
                                                     
                                                     
                                                     
        {0x9407, 0},                                 
                                                     
        {0xD608, 0},                                 
                                                     
        {0xD609, 0},                                 
                                                     
        {0x740A, 0},                                 
                                                     
      
      
        {0xD620, 0},                                 
                                                     
                                                     
        {0x7621, 0},                                 
                                                     
                                                     
        {0x5622, 0},                                 
                                                     
                                                     
        {0x7623, 0},                                 
                                                     
                                                     
        {0x5624, 0},                                 
                                                     
                                                     
        {0x5625, 0},                                 
                                                     
                                                     
        {0xD626, 0},                                 
                                                     
                                                     
        {0x7627, 0},                                 
                                                     
        {0x7628, 0},                                 
                                                     
        {0x7629, 0},                                 
                                                     
                                                     
                                                     
                                                     
      
        {0xD62B, 0},                                 
                                                     
                                                     
                                                     
        {0xD62C, 0},                                 
                                                     
                                                     
                                                     
        {0xCA78, &SwWW8ImplReader::Read_DoubleLine_Rotate},
        {0x6649, 0},                                 
        {0xF614, 0},                                 
                                                     
        {0xD612, 0},                                 
        {0xD613, 0},                                 
        {0xD61A, 0},                                 
        {0xD61B, 0},                                 
        {0xD61C, 0},                                 
        {0xD61D, 0},                                 
        {0xD634, 0},                                 
        {0xD632, 0},                                 
        {0xD238, 0},                                 
        {0xC64E, 0},                                 
        {0xC64F, 0},                                 
        {0xC650, 0},                                 
        {0xC651, 0},                                 
        {0xF661, 0},                                 
        {0x4873, &SwWW8ImplReader::Read_Language},   
                                                     
                                                     
        {0x4874, 0},                                 
                                                     
                                                     
        {0x6463, 0},                                 
        {0x2461, &SwWW8ImplReader::Read_RTLJustify}, 
                                                     
        {0x845E, &SwWW8ImplReader::Read_LR},         
        {0x8460, &SwWW8ImplReader::Read_LR},         
        {0x845D, &SwWW8ImplReader::Read_LR},         
        {0x3615, 0},                                 
        {0x360D, 0},                                 
        {0x940E, 0},                                 
        {0x940F, 0},                                 
        {0x9410, 0},                                 
        {0x703A, 0},                                 
        {0x303B, 0},                                 
        {0x244B, &SwWW8ImplReader::Read_TabCellEnd}, 
                                                     
        {0x244C, &SwWW8ImplReader::Read_TabRowEnd},  
                                                     
        {0x6815, 0},                                 
        {0x6816, 0},                                 
        {NS_sprm::LN_CCv, &SwWW8ImplReader::Read_TxtForeColor},
        {0x6877, &SwWW8ImplReader::Read_UnderlineColor},
        {0xC64D, &SwWW8ImplReader::Read_ParaBackColor},
        {0x6467, 0},                                 
        {0xF617, 0},                                 
        {0xD660, 0},                                 
        {0xD670, 0},                                 
        {0xCA71, &SwWW8ImplReader::Read_TxtBackColor},
        {0x303C, 0},                                 
        {0x245B, &SwWW8ImplReader::Read_ParaAutoBefore},
        {0x245C, &SwWW8ImplReader::Read_ParaAutoAfter},
        {0x246D, &SwWW8ImplReader::Read_ParaContextualSpacing} 
    };

    static wwSprmDispatcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
}





const SprmReadInfo& SwWW8ImplReader::GetSprmReadInfo(sal_uInt16 nId) const
{
    ww::WordVersion eVersion = pWwFib->GetFIBVersion();
    const wwSprmDispatcher *pDispatcher;
    if (eVersion <= ww::eWW2)
        pDispatcher = GetWW2SprmDispatcher();
    else if (eVersion < ww::eWW8)
        pDispatcher = GetWW6SprmDispatcher();
    else
        pDispatcher = GetWW8SprmDispatcher();

    SprmReadInfo aSrch = {0, 0};
    aSrch.nId = nId;
    const SprmReadInfo* pFound = pDispatcher->search(aSrch);

    if (!pFound)
    {
        aSrch.nId = 0;
        pFound = pDispatcher->search(aSrch);
    }

    return *pFound;
}




void SwWW8ImplReader::EndSprm( sal_uInt16 nId )
{
    if( ( nId > 255 ) && ( nId < 0x0800 ) ) return;

    const SprmReadInfo& rSprm = GetSprmReadInfo( nId );

    if (rSprm.pReadFnc)
        (this->*rSprm.pReadFnc)( nId, 0, -1 );
}

short SwWW8ImplReader::ImportSprm(const sal_uInt8* pPos,sal_uInt16 nId)
{
    if (!nId)
        nId = mpSprmParser->GetSprmId(pPos);

    OSL_ENSURE( nId != 0xff, "Sprm FF !!!!" );

    const SprmReadInfo& rSprm = GetSprmReadInfo(nId);

    sal_uInt16 nFixedLen = mpSprmParser->DistanceToData(nId);
    sal_uInt16 nL = mpSprmParser->GetSprmSize(nId, pPos);

    if (rSprm.pReadFnc)
        (this->*rSprm.pReadFnc)(nId, pPos + nFixedLen, nL - nFixedLen);

    return nL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
