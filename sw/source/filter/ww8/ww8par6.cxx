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

#include <stdlib.h>
#include <svl/itemiter.hxx>
#include <svl/grabbagitem.hxx>
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
#include <sprmids.hxx>
#include <fmtpdsc.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
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

//              various

#define MM_250 1417             // WW default for horizontal borders: 2.5 cm
#define MM_200 1134             // WW default for lower border: 2.0 cm


static sal_uInt8 lcl_ReadBorders(bool bVer67, WW8_BRCVer9* brc, WW8PLCFx_Cp_FKP* pPap,
    const WW8RStyle* pSty = nullptr, const WW8PLCFx_SEPX* pSep = nullptr);

ColorData SwWW8ImplReader::GetCol(sal_uInt8 nIco)
{
    static const ColorData eSwWW8ColA[] =
    {
        COL_AUTO, COL_BLACK, COL_LIGHTBLUE, COL_LIGHTCYAN, COL_LIGHTGREEN,
        COL_LIGHTMAGENTA, COL_LIGHTRED, COL_YELLOW, COL_WHITE, COL_BLUE,
        COL_CYAN, COL_GREEN, COL_MAGENTA, COL_RED, COL_BROWN, COL_GRAY,
        COL_LIGHTGRAY
    };
    SAL_WARN_IF(
        nIco >= SAL_N_ELEMENTS(eSwWW8ColA), "sw.ww8",
        "ico " << sal_uInt32(nIco) << " >= " << SAL_N_ELEMENTS(eSwWW8ColA));
    return nIco < SAL_N_ELEMENTS(eSwWW8ColA) ? eSwWW8ColA[nIco] : COL_AUTO;
}

inline sal_uInt32 MSRoundTweak(sal_uInt32 x)
{
    return x;
}

// page attribute which are not handled via the attribute management but
// using ...->HasSprm
// (except OLST which stays a normal attribute)
static short ReadSprm( const WW8PLCFx_SEPX* pSep, sal_uInt16 nId, short nDefaultVal )
{
    const sal_uInt8* pS = pSep->HasSprm( nId );          // sprm here?
    short nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    return nVal;
}

static sal_uInt16 ReadUSprm( const WW8PLCFx_SEPX* pSep, sal_uInt16 nId, short nDefaultVal )
{
    const sal_uInt8* pS = pSep->HasSprm( nId );          // sprm here?
    sal_uInt16 nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    return nVal;
}

static sal_uInt8 ReadBSprm( const WW8PLCFx_SEPX* pSep, sal_uInt16 nId, sal_uInt8 nDefaultVal )
{
    const sal_uInt8* pS = pSep->HasSprm( nId );          // sprm here?
    sal_uInt8 nVal = pS ? *pS : nDefaultVal;
    return nVal;
}

void wwSection::SetDirection()
{
    //sprmSTextFlow
    switch (maSep.wTextFlow)
    {
        default:
            OSL_ENSURE(false, "Unknown layout type");
            SAL_FALLTHROUGH;
        case 0:
            meDir=FRMDIR_HORI_LEFT_TOP;
            break;
        case 1:
            meDir=FRMDIR_VERT_TOP_RIGHT;
            break;
        case 2:
            //asian letters are not rotated, western are. We can't import
            //bottom to top going left to right, we can't do this in
            //pages, (in drawboxes we could partly hack it with a rotated
            //drawing box, though not frame)
            meDir=FRMDIR_VERT_TOP_RIGHT;
            break;
        case 3:
            //asian letters are not rotated, western are. We can't import
            meDir=FRMDIR_VERT_TOP_RIGHT;
            break;
        case 4:
            //asian letters are rotated, western not. We can't import
            meDir=FRMDIR_HORI_LEFT_TOP;
            break;
    }

    sal_uInt8 bRTLPgn = maSep.fBiDi;
    if ((meDir == FRMDIR_HORI_LEFT_TOP) && bRTLPgn)
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
void SwWW8ImplReader::SetDocumentGrid(SwFrameFormat &rFormat, const wwSection &rSection)
{
    if (m_bVer67)
        return;

    rFormat.SetFormatAttr(SvxFrameDirectionItem(rSection.meDir, RES_FRAMEDIR));

    SwTwips nTextareaHeight = rFormat.GetFrameSize().GetHeight();
    const SvxULSpaceItem &rUL = ItemGet<SvxULSpaceItem>(rFormat, RES_UL_SPACE);
    nTextareaHeight -= rUL.GetUpper();
    nTextareaHeight -= rUL.GetLower();

    SwTwips nTextareaWidth = rFormat.GetFrameSize().GetWidth();
    const SvxLRSpaceItem &rLR = ItemGet<SvxLRSpaceItem>(rFormat, RES_LR_SPACE);
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
            OSL_ENSURE(false, "Unknown grid type");
            SAL_FALLTHROUGH;
        case 3:
            eType = GRID_LINES_CHARS;
            aGrid.SetSnapToChars(true);
            break;
        case 1:
            eType = GRID_LINES_CHARS;
            aGrid.SetSnapToChars(false);
            break;
        case 2:
            eType = GRID_LINES_ONLY;
            break;
    }

    aGrid.SetGridType(eType);

    // seem to not add external leading in word, or the character would run across
    // two line in some cases.
    if (eType != GRID_NONE)
        m_rDoc.getIDocumentSettingAccess().set(DocumentSettingId::ADD_EXT_LEADING, false);

    //force to set document as standard page mode
    bool bSquaredMode = false;
    m_rDoc.SetDefaultPageMode( bSquaredMode );
    aGrid.SetSquaredMode( bSquaredMode );

    //Get the size of word's default styles font
    sal_uInt32 nCharWidth=240;
    for (sal_uInt16 nI = 0; nI < m_pStyles->GetCount(); ++nI)
    {
        if (m_vColl[nI].m_bValid && m_vColl[nI].m_pFormat &&
            m_vColl[nI].IsWW8BuiltInDefaultStyle())
        {
            nCharWidth = ItemGet<SvxFontHeightItem>(*(m_vColl[nI].m_pFormat),
                RES_CHRATR_CJK_FONTSIZE).GetHeight();
            break;
        }
    }

    //dxtCharSpace
    if (rSection.maSep.dxtCharSpace)
    {
        sal_uInt32 nCharSpace = rSection.maSep.dxtCharSpace;
        //main lives in top 20 bits, and is signed.
        sal_Int32 nMain = (nCharSpace & 0xFFFFF000);
        nMain/=0x1000;
        nCharWidth += nMain*20;

        int nFraction = (nCharSpace & 0x00000FFF);
        nFraction = (nFraction*20)/0xFFF;
        nCharWidth += nFraction;
    }

    aGrid.SetBaseWidth( writer_cast<sal_uInt16>(nCharWidth));

    //sep.dyaLinePitch
    sal_Int32 nLinePitch = rSection.maSep.dyaLinePitch;
    if (nLinePitch >= 1 && nLinePitch <= 31680)
    {
        aGrid.SetLines(writer_cast<sal_uInt16>(nTextareaHeight/nLinePitch));
        aGrid.SetBaseHeight(writer_cast<sal_uInt16>(nLinePitch));
    }

    sal_Int32 nRubyHeight = 0;
    aGrid.SetRubyHeight(writer_cast<sal_uInt16>(nRubyHeight));

    rFormat.SetFormatAttr(aGrid);
}

void SwWW8ImplReader::Read_ParaBiDi(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_FRAMEDIR);
    else
    {
        SvxFrameDirection eDir =
            *pData ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP;
        NewAttr(SvxFrameDirectionItem(eDir, RES_FRAMEDIR));
    }
}

bool wwSectionManager::SetCols(SwFrameFormat &rFormat, const wwSection &rSection,
    sal_uInt32 nNetWidth)
{
    //sprmSCcolumns - number of columns - 1
    const sal_Int16 nCols = rSection.NoCols();

    if (nCols < 2)          //check for no columns or other weird state
        return false;

    SwFormatCol aCol;                      // Create SwFormatCol

    //sprmSDxaColumns   - Default distance is 1.25 cm
    sal_Int32 nColSpace = rSection.StandardColSeparation();

    const SEPr& rSep = rSection.maSep;

    // sprmSLBetween
    if (rSep.fLBetween)
    {
        aCol.SetLineAdj(COLADJ_TOP);      // Line
        aCol.SetLineHeight(100);
        aCol.SetLineColor(Color(COL_BLACK));
        aCol.SetLineWidth(1);
    }

    aCol.Init(nCols, writer_cast<sal_uInt16>(nColSpace),
        writer_cast<sal_uInt16>(nNetWidth));

    // sprmSFEvenlySpaced
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
        aCol.SetWishWidth(writer_cast<sal_uInt16>(nNetWidth));
    }
    rFormat.SetFormatAttr(aCol);
    return true;
}

void wwSectionManager::SetLeftRight(wwSection &rSection)
{
    // 3. LR-Raender
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
    else if (!mrReader.m_pWDop->iGutterPos)
        nWWLe += nWWGu;

    // Left / Right
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

void wwSectionManager::SetPage(SwPageDesc &rInPageDesc, SwFrameFormat &rFormat,
    const wwSection &rSection, bool bIgnoreCols)
{
    // 1. orientation
    rInPageDesc.SetLandscape(rSection.IsLandScape());

    // 2. paper size
    SwFormatFrameSize aSz( rFormat.GetFrameSize() );
    aSz.SetWidth(rSection.GetPageWidth());
    aSz.SetHeight(SvxPaperInfo::GetSloppyPaperDimension(rSection.GetPageHeight()));
    rFormat.SetFormatAttr(aSz);

    rFormat.SetFormatAttr(
        SvxLRSpaceItem(rSection.GetPageLeft(), rSection.GetPageRight(), 0, 0, RES_LR_SPACE));

    if (!bIgnoreCols)
        SetCols(rFormat, rSection, rSection.GetTextAreaWidth());
}

static sal_uInt16 lcl_MakeSafeNegativeSpacing(sal_uInt16 nIn)
{
    if (nIn > SHRT_MAX)
        nIn = 0;
    return nIn;
}

void SwWW8ImplReader::SetPageBorder(SwFrameFormat &rFormat, const wwSection &rSection)
{
    if (!IsBorder(rSection.brc))
        return;

    SfxItemSet aSet(rFormat.GetAttrSet());
    short aSizeArray[5]={0};
    SetFlyBordersShadow(aSet, rSection.brc, &aSizeArray[0]);
    SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(aSet, RES_LR_SPACE));
    SvxULSpaceItem aUL(ItemGet<SvxULSpaceItem>(aSet, RES_UL_SPACE));

    SvxBoxItem aBox(ItemGet<SvxBoxItem>(aSet, RES_BOX));
    short aOriginalBottomMargin = aBox.GetDistance(SvxBoxItemLine::BOTTOM);

    if (rSection.maSep.pgbOffsetFrom == 1)
    {
        sal_uInt16 nDist;
        if (aBox.GetLeft())
        {
            nDist = aBox.GetDistance(SvxBoxItemLine::LEFT);
            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aLR.GetLeft() - nDist)), SvxBoxItemLine::LEFT);
            aSizeArray[WW8_LEFT] =
                aSizeArray[WW8_LEFT] - nDist + aBox.GetDistance(SvxBoxItemLine::LEFT);
        }

        if (aBox.GetRight())
        {
            nDist = aBox.GetDistance(SvxBoxItemLine::RIGHT);
            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aLR.GetRight() - nDist)), SvxBoxItemLine::RIGHT);
            aSizeArray[WW8_RIGHT] =
                aSizeArray[WW8_RIGHT] - nDist + aBox.GetDistance(SvxBoxItemLine::RIGHT);
        }

        if (aBox.GetTop())
        {
            nDist = aBox.GetDistance(SvxBoxItemLine::TOP);
            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aUL.GetUpper() - nDist)), SvxBoxItemLine::TOP);
            aSizeArray[WW8_TOP] =
                aSizeArray[WW8_TOP] - nDist + aBox.GetDistance(SvxBoxItemLine::TOP);
        }

        if (aBox.GetBottom())
        {
            nDist = aBox.GetDistance(SvxBoxItemLine::BOTTOM);
            aBox.SetDistance(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aUL.GetLower() - nDist)), SvxBoxItemLine::BOTTOM);
            aSizeArray[WW8_BOT] =
                aSizeArray[WW8_BOT] - nDist + aBox.GetDistance(SvxBoxItemLine::BOTTOM);
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
        //#i30088# and #i30074# - do a final sanity check on
        //bottom value. Do not allow a resulting zero if bottom
        //Border margin value was not originally zero.
        if(aUL.GetLower() != 0)
            aUL.SetLower(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aUL.GetLower() - aSizeArray[WW8_BOT])));
        else
            aUL.SetLower(lcl_MakeSafeNegativeSpacing(static_cast<sal_uInt16>(aOriginalBottomMargin - aSizeArray[WW8_BOT])));
    }

    aSet.Put(aLR);
    aSet.Put(aUL);
    rFormat.SetFormatAttr(aSet);
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
         (!mrReader.m_bVer67 && mrReader.m_pWDop->iGutterPos &&
         rSection.maSep.fRTLGutter)
       )
    {
        nWWUp += rSection.maSep.dzaGutter;
    }

    rData.bHasHeader = (rSection.maSep.grpfIhdt &
        (WW8_HEADER_EVEN | WW8_HEADER_ODD | WW8_HEADER_FIRST)) != 0;

    if( rData.bHasHeader )
    {
        rData.nSwUp  = nWWHTop;             // Header -> convert
        // #i19922# - correction:
        // consider that <nWWUp> can be negative, compare only if it's positive
        if ( nWWUp > 0 &&
             static_cast<sal_uInt32>(abs(nWWUp)) >= nWWHTop )
            rData.nSwHLo = nWWUp - nWWHTop;
        else
            rData.nSwHLo = 0;

        // #i19922# - minimum page header height is now 1mm
        // use new constant <cMinHdFtHeight>
        if (rData.nSwHLo < sal::static_int_cast< sal_uInt32 >(cMinHdFtHeight))
            rData.nSwHLo = sal::static_int_cast< sal_uInt32 >(cMinHdFtHeight);
    }
    else // no header -> just use Up as-is
        rData.nSwUp = std::abs(nWWUp);

    rData.bHasFooter = (rSection.maSep.grpfIhdt &
        (WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST)) != 0;

    if( rData.bHasFooter )
    {
        rData.nSwLo = nWWFBot;              // footer -> convert
        // #i19922# - correction: consider that <nWWLo> can be negative, compare only if it's positive
        if ( nWWLo > 0 &&
             static_cast<sal_uInt32>(abs(nWWLo)) >= nWWFBot )
            rData.nSwFUp = nWWLo - nWWFBot;
        else
            rData.nSwFUp = 0;

        // #i19922# - minimum page header height is now 1mm
        // use new constant <cMinHdFtHeight>
        if (rData.nSwFUp < sal::static_int_cast< sal_uInt32 >(cMinHdFtHeight))
            rData.nSwFUp = sal::static_int_cast< sal_uInt32 >(cMinHdFtHeight);
    }
    else // no footer -> just use Lo as-is
        rData.nSwLo = std::abs(nWWLo);
}

void wwSectionManager::SetPageULSpaceItems(SwFrameFormat &rFormat,
    wwSectionManager::wwULSpaceData& rData, const wwSection &rSection)
{
    if (rData.bHasHeader)               // ... and set Header-Lower
    {
        // set header height to minimum
        if (SwFrameFormat* pHdFormat = const_cast<SwFrameFormat*>(rFormat.GetHeader().GetHeaderFormat()))
        {
            SvxULSpaceItem aHdUL(pHdFormat->GetULSpace());
            if (!rSection.IsFixedHeightHeader())    //normal
            {
                pHdFormat->SetFormatAttr(SwFormatFrameSize(ATT_MIN_SIZE, 0, rData.nSwHLo));
                // #i19922# - minimum page header height is now 1mm
                // use new constant <cMinHdFtHeight>
                aHdUL.SetLower( writer_cast<sal_uInt16>(rData.nSwHLo - cMinHdFtHeight) );
                pHdFormat->SetFormatAttr(SwHeaderAndFooterEatSpacingItem(
                    RES_HEADER_FOOTER_EAT_SPACING, true));
            }
            else
            {
                // #i48832# - set correct spacing between header and body.
                const SwTwips nHdLowerSpace( std::abs(rSection.maSep.dyaTop) - rData.nSwUp - rData.nSwHLo );
                pHdFormat->SetFormatAttr(SwFormatFrameSize(ATT_FIX_SIZE, 0, rData.nSwHLo + nHdLowerSpace));
                aHdUL.SetLower( static_cast< sal_uInt16 >(nHdLowerSpace) );
                pHdFormat->SetFormatAttr(SwHeaderAndFooterEatSpacingItem(
                    RES_HEADER_FOOTER_EAT_SPACING, false));
            }
            pHdFormat->SetFormatAttr(aHdUL);
        }
    }

    if (rData.bHasFooter)               // ... and set footer-upper
    {
        if (SwFrameFormat* pFtFormat = const_cast<SwFrameFormat*>(rFormat.GetFooter().GetFooterFormat()))
        {
            SvxULSpaceItem aFtUL(pFtFormat->GetULSpace());
            if (!rSection.IsFixedHeightFooter())    //normal
            {
                pFtFormat->SetFormatAttr(SwFormatFrameSize(ATT_MIN_SIZE, 0, rData.nSwFUp));
                // #i19922# - minimum page header height is now 1mm
                // use new constant <cMinHdFtHeight>
                aFtUL.SetUpper( writer_cast<sal_uInt16>(rData.nSwFUp - cMinHdFtHeight) );
                pFtFormat->SetFormatAttr(SwHeaderAndFooterEatSpacingItem(
                    RES_HEADER_FOOTER_EAT_SPACING, true));
            }
            else
            {
                // #i48832# - set correct spacing between footer and body.
                const SwTwips nFtUpperSpace( std::abs(rSection.maSep.dyaBottom) - rData.nSwLo - rData.nSwFUp );
                pFtFormat->SetFormatAttr(SwFormatFrameSize(ATT_FIX_SIZE, 0, rData.nSwFUp + nFtUpperSpace));
                aFtUL.SetUpper( static_cast< sal_uInt16 >(nFtUpperSpace) );
                pFtFormat->SetFormatAttr(SwHeaderAndFooterEatSpacingItem(
                    RES_HEADER_FOOTER_EAT_SPACING, false));
            }
            pFtFormat->SetFormatAttr(aFtUL);
        }
    }

    SvxULSpaceItem aUL(writer_cast<sal_uInt16>(rData.nSwUp),
        writer_cast<sal_uInt16>(rData.nSwLo), RES_UL_SPACE);
    rFormat.SetFormatAttr(aUL);
}

SwSectionFormat *wwSectionManager::InsertSection(
    SwPaM& rMyPaM, wwSection &rSection)
{
    SwSectionData aSection( CONTENT_SECTION,
            mrReader.m_rDoc.GetUniqueSectionName() );

    SfxItemSet aSet( mrReader.m_rDoc.GetAttrPool(), aFrameFormatSetRange );

    bool bRTLPgn = !maSegments.empty() && maSegments.back().IsBiDi();
    aSet.Put(SvxFrameDirectionItem(
        bRTLPgn ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));

    if (2 == mrReader.m_pWDop->fpc)
        aSet.Put( SwFormatFootnoteAtTextEnd(FTNEND_ATTXTEND));
    if (0 == mrReader.m_pWDop->epc)
        aSet.Put( SwFormatEndAtTextEnd(FTNEND_ATTXTEND));

    aSection.SetProtectFlag(SectionIsProtected(rSection));

    rSection.mpSection =
        mrReader.m_rDoc.InsertSwSection( rMyPaM, aSection, nullptr, & aSet );
    OSL_ENSURE(rSection.mpSection, "section not inserted!");
    if (!rSection.mpSection)
        return nullptr;

    SwPageDesc *pPage = nullptr;
    mySegrIter aEnd = maSegments.rend();
    for (mySegrIter aIter = maSegments.rbegin(); aIter != aEnd; ++aIter)
    {
        if (nullptr != (pPage = aIter->mpPage))
            break;
    }

    OSL_ENSURE(pPage, "no page outside this section!");

    if (!pPage)
        pPage = &mrReader.m_rDoc.GetPageDesc(0);

    if (!pPage)
        return nullptr;

    SwSectionFormat *pFormat = rSection.mpSection->GetFormat();
    OSL_ENSURE(pFormat, "impossible");
    if (!pFormat)
        return nullptr;

    SwFrameFormat& rFormat = pPage->GetMaster();
    const SvxLRSpaceItem& rLR = rFormat.GetLRSpace();
    long nPageLeft  = rLR.GetLeft();
    long nPageRight = rLR.GetRight();
    long nSectionLeft = rSection.GetPageLeft() - nPageLeft;
    long nSectionRight = rSection.GetPageRight() - nPageRight;
    if ((nSectionLeft != 0) || (nSectionRight != 0))
    {
        SvxLRSpaceItem aLR(nSectionLeft, nSectionRight, 0, 0, RES_LR_SPACE);
        pFormat->SetFormatAttr(aLR);
    }

    SetCols(*pFormat, rSection, rSection.GetTextAreaWidth());
    return pFormat;
}

void SwWW8ImplReader::HandleLineNumbering(const wwSection &rSection)
{
    // check if Line Numbering must be activated or reset
    if (m_bNewDoc && rSection.maSep.nLnnMod)
    {
        // restart-numbering-mode: 0 per page, 1 per section, 2 never restart
        bool bRestartLnNumPerSection = (1 == rSection.maSep.lnc);

        if (m_bNoLnNumYet)
        {
            SwLineNumberInfo aInfo( m_rDoc.GetLineNumberInfo() );

            aInfo.SetPaintLineNumbers(true);

            aInfo.SetRestartEachPage(rSection.maSep.lnc == 0);

            aInfo.SetPosFromLeft(writer_cast<sal_uInt16>(rSection.maSep.dxaLnn));

            //Paint only for every n line
            aInfo.SetCountBy(rSection.maSep.nLnnMod);

            // to be defaulted features ( HARDCODED in MS Word 6,7,8,9 )
            aInfo.SetCountBlankLines(true);
            aInfo.SetCountInFlys(false);
            aInfo.SetPos( LINENUMBER_POS_LEFT );
            SvxNumberType aNumType; // this sets SVX_NUM_ARABIC per default
            aInfo.SetNumType( aNumType );

            m_rDoc.SetLineNumberInfo( aInfo );
            m_bNoLnNumYet = false;
        }

        if (
            (0 < rSection.maSep.lnnMin) ||
            (bRestartLnNumPerSection && !m_bNoLnNumYet)
           )
        {
            SwFormatLineNumber aLN;
            if (const SwFormatLineNumber* pLN
                = static_cast<const SwFormatLineNumber*>(GetFormatAttr(RES_LINENUMBER)))
            {
                aLN.SetCountLines( pLN->IsCount() );
            }
            aLN.SetStartValue(1 + rSection.maSep.lnnMin);
            NewAttr(aLN);
            m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_LINENUMBER);
        }
        m_bNoLnNumYet = false;
    }
}

wwSection::wwSection(const SwPosition &rPos) : maStart(rPos.nNode)
    , mpSection(nullptr)
    , mpPage(nullptr)
    , meDir(FRMDIR_HORI_LEFT_TOP)
    , mLinkId(0)
    , nPgWidth(SvxPaperInfo::GetPaperSize(PAPER_A4).Width())
    , nPgLeft(MM_250)
    , nPgRight(MM_250)
    , mnVerticalAdjustment(drawing::TextVerticalAdjust_TOP)
    , mnBorders(0)
    , mbHasFootnote(false)
{
}

void wwSectionManager::SetNumberingType(const wwSection &rNewSection,
    SwPageDesc &rPageDesc)
{
    // save page number format
    static const SvxExtNumType aNumTyp[5] =
    {
        SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N
    };

    SvxNumberType aType;
    aType.SetNumberingType( static_cast< sal_Int16 >(aNumTyp[rNewSection.maSep.nfcPgn]) );
    rPageDesc.SetNumType(aType);
}

// CreateSep is called for every section change (even at the start of
// the document. CreateSep also creates the pagedesc(s) and
// fills it/them with attributes and KF texts.
// This has become necessary because the translation of the various
// page attributes is interconnected too much.
void wwSectionManager::CreateSep(const long nTextPos, bool /*bMustHaveBreak*/)
{
    /*
    #i1909# section/page breaks should not occur in tables or subpage
    elements like frames. Word itself ignores them in this case. The bug is
    more likely that this filter created such documents in the past!
    */
    if (mrReader.m_nInTable || mrReader.m_bTxbxFlySection || mrReader.InLocalApo())
        return;

    WW8PLCFx_SEPX* pSep = mrReader.m_pPlcxMan->GetSepPLCF();
    OSL_ENSURE(pSep, "impossible!");
    if (!pSep)
        return;

    if (!maSegments.empty() && mrReader.m_pLastAnchorPos.get() && *mrReader.m_pLastAnchorPos == *mrReader.m_pPaM->GetPoint())
    {
        bool insert = true;
        SwPaM pam( *mrReader.m_pLastAnchorPos );
        if( pam.Move(fnMoveBackward, fnGoNode))
            if( SwTextNode* txtNode = pam.GetPoint()->nNode.GetNode().GetTextNode())
                if( txtNode->Len() == 0 )
                    insert = false;
        if( insert )
            mrReader.AppendTextNode(*mrReader.m_pPaM->GetPoint());
    }

    ww::WordVersion eVer = mrReader.GetFib().GetFIBVersion();

    // M.M. Create a linked section if the WkbPLCF
    // has an entry for one at this cp
    WW8PLCFspecial* pWkb = mrReader.m_pPlcxMan->GetWkbPLCF();
    if (pWkb && pWkb->SeekPosExact(nTextPos) &&
            pWkb->Where() == nTextPos)
    {
        void* pData;
        WW8_CP nTest;
        bool bSuccess = pWkb->Get(nTest, pData);
        if (!bSuccess)
            return;
        OUString sSectionName = mrReader.m_aLinkStringMap[SVBT16ToShort( static_cast<WW8_WKB*>(pData)->nLinkId) ];
        sSectionName = mrReader.ConvertFFileName(sSectionName);
        SwSectionData aSection(FILE_LINK_SECTION, sSectionName);
        aSection.SetLinkFileName( sSectionName );
        aSection.SetProtectFlag(true);
        // #i19922# - improvement: return value of method <Insert> not used.
        mrReader.m_rDoc.InsertSwSection(*mrReader.m_pPaM, aSection, nullptr, nullptr, false);
    }

    wwSection aLastSection(*mrReader.m_pPaM->GetPoint());
    if (!maSegments.empty())
        aLastSection = maSegments.back();

    //Here
    sal_uInt16 nLIdx = ( ( mrReader.m_pWwFib->lid & 0xff ) == 0x9 ) ? 1 : 0;

    //BEGIN read section values
    wwSection aNewSection(*mrReader.m_pPaM->GetPoint());

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
        // Type of break: break codes are:
        // 0 No break
        // 1 New column
        // 2 New page
        // 3 Even page
        // 4 Odd page
        if (const sal_uInt8* pSprmBkc = pSep->HasSprm(pIds[0]))
            aNewSection.maSep.bkc = *pSprmBkc;
    }

    // Has a table page
    aNewSection.maSep.fTitlePage =
        sal_uInt8(0 != ReadBSprm( pSep, pIds[1], 0 ));

    // sprmSNfcPgn
    aNewSection.maSep.nfcPgn = ReadBSprm( pSep, pIds[2], 0 );
    if (aNewSection.maSep.nfcPgn > 4)
        aNewSection.maSep.nfcPgn = 0;

    aNewSection.maSep.fUnlocked = eVer > ww::eWW2 ? ReadBSprm(pSep, (eVer <= ww::eWW7 ? 139 : 0x3006), 0 ) : 0;

    // sprmSFBiDi
    aNewSection.maSep.fBiDi = eVer >= ww::eWW8 ? ReadBSprm(pSep, 0x3228, 0) : 0;

    // Reading section property sprmSCcolumns - one less than the number of columns in the section.
    // It must be less than MAX_NO_OF_SEP_COLUMNS according the WW8 specification.
    aNewSection.maSep.ccolM1 = ReadSprm(pSep, pIds[3], 0 );
    if ( aNewSection.maSep.ccolM1 >= MAX_NO_OF_SEP_COLUMNS )
    {
        // clip to max
        aNewSection.maSep.ccolM1 = MAX_NO_OF_SEP_COLUMNS-1;
    }

    //sprmSDxaColumns   - default distance 1.25 cm
    aNewSection.maSep.dxaColumns = ReadUSprm( pSep, pIds[4], 708 );

    // sprmSLBetween
    aNewSection.maSep.fLBetween = ReadBSprm(pSep, pIds[5], 0 );

    if (eVer >= ww::eWW6)
    {
        // sprmSFEvenlySpaced
        aNewSection.maSep.fEvenlySpaced =
            sal_uInt8(ReadBSprm(pSep, (eVer <= ww::eWW7 ? 138 : 0x3005), 1) != 0);

        if (aNewSection.maSep.ccolM1 > 0 && !aNewSection.maSep.fEvenlySpaced)
        {
            int nColumnDataIdx = 0;
            aNewSection.maSep.rgdxaColumnWidthSpacing[nColumnDataIdx] = 0;

            const sal_uInt16 nColumnWidthSprmId = ( eVer <= ww::eWW7 ? 136 : 0xF203 );
            const sal_uInt16 nColumnSpacingSprmId = ( eVer <= ww::eWW7 ? 137 : 0xF204 );
            const sal_uInt8 nColumnCount = static_cast< sal_uInt8 >(aNewSection.maSep.ccolM1 + 1);
            for ( sal_uInt8 nColumn = 0; nColumn < nColumnCount; ++nColumn )
            {
                //sprmSDxaColWidth
                const sal_uInt8* pSW = pSep->HasSprm( nColumnWidthSprmId, nColumn );

                OSL_ENSURE( pSW, "+Sprm 136 (resp. 0xF203) (ColWidth) missing" );
                sal_uInt16 nWidth = pSW ? SVBT16ToShort(pSW + 1) : 1440;

                aNewSection.maSep.rgdxaColumnWidthSpacing[++nColumnDataIdx] = nWidth;

                if ( nColumn < nColumnCount - 1 )
                {
                    //sprmSDxaColSpacing
                    const sal_uInt8* pSD = pSep->HasSprm( nColumnSpacingSprmId, nColumn );

                    OSL_ENSURE( pSD, "+Sprm 137 (resp. 0xF204) (Colspacing) missing" );
                    if( pSD )
                    {
                        nWidth = SVBT16ToShort(pSD + 1);
                        aNewSection.maSep.rgdxaColumnWidthSpacing[++nColumnDataIdx] = nWidth;
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

    // 1. orientation
    aNewSection.maSep.dmOrientPage = ReadBSprm(pSep, pIds[0], 0);

    // 2. paper size
    aNewSection.maSep.xaPage = ReadUSprm(pSep, pIds[1], lLetterWidth);
    aNewSection.nPgWidth = SvxPaperInfo::GetSloppyPaperDimension(aNewSection.maSep.xaPage);

    aNewSection.maSep.yaPage = ReadUSprm(pSep, pIds[2], lLetterHeight);

    // 3. LR borders
    static const sal_uInt16 nLef[] = { MM_250, 1800 };
    static const sal_uInt16 nRig[] = { MM_250, 1800 };

    aNewSection.maSep.dxaLeft = ReadUSprm( pSep, pIds[3], nLef[nLIdx]);
    aNewSection.maSep.dxaRight = ReadUSprm( pSep, pIds[4], nRig[nLIdx]);

    // 2pages in 1sheet hackery ?
    // #i31806# but only swap if 2page in 1sheet is enabled.
    // its not clear if dmOrientPage is the correct member to
    // decide on this.
    if(mrReader.m_pWDop->doptypography.f2on1 &&
            aNewSection.maSep.dmOrientPage == 2)
        std::swap(aNewSection.maSep.dxaLeft, aNewSection.maSep.dxaRight);

    aNewSection.maSep.dzaGutter = ReadUSprm( pSep, pIds[5], 0);

    aNewSection.maSep.fRTLGutter = static_cast< sal_uInt8 >(eVer >= ww::eWW8 ? ReadUSprm( pSep, 0x322A, 0 ) : 0);

    // Page Number Restarts - sprmSFPgnRestart
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

        //sprmSPgbProp
        sal_uInt16 pgbProp = ReadSprm( pSep, 0x522F, 0 );
        aNewSection.maSep.pgbApplyTo = pgbProp & 0x0007;
        aNewSection.maSep.pgbPageDepth = (pgbProp & 0x0018) >> 3;
        aNewSection.maSep.pgbOffsetFrom = (pgbProp & 0x00E0) >> 5;

        aNewSection.mnBorders =
            ::lcl_ReadBorders(eVer <= ww::eWW7, aNewSection.brc, nullptr, nullptr, pSep);
    }

    // check if Line Numbering must be activated or reset
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
    else if (mrReader.m_pHdFt)
    {
        aNewSection.maSep.grpfIhdt = WW8_HEADER_ODD | WW8_FOOTER_ODD
            | WW8_HEADER_FIRST | WW8_FOOTER_FIRST;

        // It is possible for a first page header to be provided
        // for this section, but not actually shown in this section.  In this
        // case (aNewSection.maSep.grpfIhdt & WW8_HEADER_FIRST) will be nonzero
        // but aNewSection.HasTitlePage() will be false.
        // Likewise for first page footer.

        if (mrReader.m_pWDop->fFacingPages)
            aNewSection.maSep.grpfIhdt |= WW8_HEADER_EVEN | WW8_FOOTER_EVEN;

        //See if we have a header or footer for each enabled possibility
        //if we do not then we inherit the previous sections header/footer,
        for (int nI = 0, nMask = 1; nI < 6; ++nI, nMask <<= 1)
        {
            if (aNewSection.maSep.grpfIhdt & nMask)
            {
                WW8_CP nStart, nLen;
                mrReader.m_pHdFt->GetTextPosExact( static_cast< short >(nI + ( maSegments.size() + 1) * 6), nStart, nLen);
                //No header or footer, inherit pervious one, or set to zero
                //if no previous one
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
    //END read section values

    if (eVer >= ww::eWW8)
        aNewSection.SetDirection();

    mrReader.HandleLineNumbering(aNewSection);
    maSegments.push_back(aNewSection);
}

void SwWW8ImplReader::CopyPageDescHdFt(const SwPageDesc* pOrgPageDesc,
    SwPageDesc* pNewPageDesc, sal_uInt8 nCode )
{
    // copy odd header content section
    if( nCode & WW8_HEADER_ODD )
    {
        m_rDoc.CopyHeader(pOrgPageDesc->GetMaster(),
                        pNewPageDesc->GetMaster() );
    }
    // copy odd footer content section
    if( nCode & WW8_FOOTER_ODD )
    {
        m_rDoc.CopyFooter(pOrgPageDesc->GetMaster(),
                        pNewPageDesc->GetMaster());
    }
    // copy even header content section
    if( nCode & WW8_HEADER_EVEN )
    {
        m_rDoc.CopyHeader(pOrgPageDesc->GetLeft(),
                        pNewPageDesc->GetLeft());
    }
    // copy even footer content section
    if( nCode & WW8_FOOTER_EVEN )
    {
        m_rDoc.CopyFooter(pOrgPageDesc->GetLeft(),
                        pNewPageDesc->GetLeft());
    }
    // copy first page header content section
    if( nCode & WW8_HEADER_FIRST )
    {
        m_rDoc.CopyHeader(pOrgPageDesc->GetFirstMaster(),
                        pNewPageDesc->GetFirstMaster());
    }
    // copy first page footer content section
    if( nCode & WW8_FOOTER_FIRST )
    {
        m_rDoc.CopyFooter(pOrgPageDesc->GetFirstMaster(),
                        pNewPageDesc->GetFirstMaster());
    }
}

//   helper functions for graphics, Apos and tables

// Read BoRder Control structure
// nBrcVer should be set to the version of the BRC record being read (6, 8 or 9)
// This will be converted to the latest format (9).
static bool _SetWW8_BRC(int nBrcVer, WW8_BRCVer9& rVar, const sal_uInt8* pS)
{

    if( pS )
    {
        if ( nBrcVer == 9 )
            rVar = *reinterpret_cast<const WW8_BRCVer9*>(pS);
        else if( nBrcVer == 8 )
            rVar = WW8_BRCVer9(*reinterpret_cast<const WW8_BRC*>(pS));
        else // nBrcVer == 6
            rVar = WW8_BRCVer9(WW8_BRC(*reinterpret_cast<const WW8_BRCVer6*>(pS)));
    }

    return nullptr != pS;
}

static sal_uInt8 lcl_ReadBorders(bool bVer67, WW8_BRCVer9* brc, WW8PLCFx_Cp_FKP* pPap,
    const WW8RStyle* pSty, const WW8PLCFx_SEPX* pSep)
{

//returns a sal_uInt8 filled with a bit for each position that had a sprm
//setting that border

    sal_uInt8 nBorder = 0;
    if( pSep )
    {
        if( !bVer67 )
        {
            sal_uInt8* pSprm[4];

            if( pSep->Find4Sprms(
                    NS_sprm::LN_SBrcTop80, NS_sprm::LN_SBrcLeft80,
                    NS_sprm::LN_SBrcBottom80, NS_sprm::LN_SBrcRight80,
                    pSprm[0], pSprm[1], pSprm[2], pSprm[3] ) )
            {
                for( int i = 0; i < 4; ++i )
                    nBorder |= int(_SetWW8_BRC( 8, brc[ i ], pSprm[ i ] ))<<i;
            }
            // Version 9 BRCs if present will override version 8
            if( pSep->Find4Sprms(
                    NS_sprm::LN_SBrcTop, NS_sprm::LN_SBrcLeft,
                    NS_sprm::LN_SBrcBottom, NS_sprm::LN_SBrcRight,
                    pSprm[0], pSprm[1], pSprm[2], pSprm[3] ) )
            {
                for( int i = 0; i < 4; ++i )
                    nBorder |= int(_SetWW8_BRC( 9, brc[ i ], pSprm[ i ] ))<<i;
            }
        }
    }
    else
    {

        static const sal_uInt16 aVer67Ids[5] = { 38, 39, 40, 41, 42 };

        static const sal_uInt16 aVer8Ids[5] =
                { 0x6424, 0x6425, 0x6426, 0x6427, 0x6428 };

        static const sal_uInt16 aVer9Ids[5] =
                { 0xC64E, 0xC64F, 0xC650, 0xC651, 0xC652 };

        if( pPap )
        {
            if (bVer67)
            {
                for( int i = 0; i < 5; ++i )
                    nBorder |= int(_SetWW8_BRC( 6 , brc[ i ], pPap->HasSprm( aVer67Ids[ i ] )))<<i;
            }
            else
            {
                for( int i = 0; i < 5; ++i )
                    nBorder |= int(_SetWW8_BRC( 8 , brc[ i ], pPap->HasSprm( aVer8Ids[ i ] )))<<i;
                // Version 9 BRCs if present will override version 8
                for( int i = 0; i < 5; ++i )
                    nBorder |= int(_SetWW8_BRC( 9 , brc[ i ], pPap->HasSprm( aVer9Ids[ i ] )))<<i;
            }
        }
        else if( pSty )
        {
            if (bVer67)
            {
                for( int i = 0; i < 5; ++i )
                    nBorder |= int(_SetWW8_BRC( 6 , brc[ i ], pSty->HasParaSprm( aVer67Ids[ i ] )))<<i;
            }
            else
            {
                for( int i = 0; i < 5; ++i )
                    nBorder |= int(_SetWW8_BRC( 8 , brc[ i ], pSty->HasParaSprm( aVer8Ids[ i ] )))<<i;
                // Version 9 BRCs if present will override version 8
                for( int i = 0; i < 5; ++i )
                    nBorder |= int(_SetWW8_BRC( 9 , brc[ i ], pSty->HasParaSprm( aVer9Ids[ i ] )))<<i;
            }
        }
        else {
            OSL_ENSURE( pSty || pPap, "WW8PLCFx_Cp_FKP and WW8RStyle "
                               "and WW8PLCFx_SEPX is 0" );
        }
    }

    return nBorder;
}

void GetLineIndex(SvxBoxItem &rBox, short nLineThickness, short nSpace,
    sal_uInt32 cv, short nIdx, SvxBoxItemLine nOOIndex, sal_uInt16 nWWIndex,
    short *pSize=nullptr)
{
    // LO cannot handle outset/inset (new in WW9 BRC) so fall back same as WW8
    if ( nIdx == 0x1A || nIdx == 0x1B )
    {
        nIdx = (nIdx == 0x1A) ? 0x12 : 0x11;
        cv = 0xc0c0c0;
    }

    ::editeng::SvxBorderStyle const eStyle(
            ::editeng::ConvertBorderStyleFromWord(nIdx));

    ::editeng::SvxBorderLine aLine;
    aLine.SetBorderLineStyle( eStyle );
    double const fConverted( (table::BorderLineStyle::NONE == eStyle) ? 0.0 :
        ::editeng::ConvertBorderWidthFromWord(eStyle, nLineThickness, nIdx));
    aLine.SetWidth(fConverted);

    //No AUTO for borders as yet, so if AUTO, use BLACK
    ColorData col = (cv==0xff000000) ? COL_BLACK : msfilter::util::BGRToRGB(cv);

    aLine.SetColor(col);

    if (pSize)
        pSize[nWWIndex] = fConverted + nSpace;

    rBox.SetLine(&aLine, nOOIndex);
    rBox.SetDistance(nSpace, nOOIndex);

}

void Set1Border(SvxBoxItem &rBox, const WW8_BRCVer9& rBor, SvxBoxItemLine nOOIndex,
    sal_uInt16 nWWIndex, short *pSize, const bool bIgnoreSpace)
{
    short nSpace;
    short nLineThickness = rBor.DetermineBorderProperties(&nSpace);

    GetLineIndex(rBox, nLineThickness, bIgnoreSpace ? 0 : nSpace,
        rBor.cv(), rBor.brcType(), nOOIndex, nWWIndex, pSize );

}

static bool lcl_IsBorder(const WW8_BRCVer9* pbrc, bool bChkBtwn = false)
{
    return pbrc[WW8_TOP  ].brcType() ||         // brcType  != 0
           pbrc[WW8_LEFT ].brcType() ||
           pbrc[WW8_BOT  ].brcType() ||
           pbrc[WW8_RIGHT].brcType() ||
           (bChkBtwn && pbrc[WW8_BETW ].brcType());
}

bool SwWW8ImplReader::IsBorder(const WW8_BRCVer9* pbrc, bool bChkBtwn)
{
    return lcl_IsBorder(pbrc, bChkBtwn);
}

bool SwWW8ImplReader::SetBorder(SvxBoxItem& rBox, const WW8_BRCVer9* pbrc,
    short *pSizeArray, sal_uInt8 nSetBorders)
{
    bool bChange = false;
    static const std::pair<sal_uInt16, SvxBoxItemLine> aIdArr[] =
    {
        { WW8_TOP,    SvxBoxItemLine::TOP },
        { WW8_LEFT,   SvxBoxItemLine::LEFT },
        { WW8_RIGHT,  SvxBoxItemLine::RIGHT },
        { WW8_BOT,    SvxBoxItemLine::BOTTOM },
        { WW8_BETW,   SvxBoxItemLine::BOTTOM }
    };

    for( int i = 0, nEnd = 4; i < nEnd; ++i )
    {
        // ungueltige Borders ausfiltern
        const WW8_BRCVer9& rB = pbrc[ aIdArr[ i ].first ];
        if( !rB.isNil() && rB.brcType() )
        {
            Set1Border(rBox, rB, aIdArr[i].second, aIdArr[i].first, pSizeArray, false);
            bChange = true;
        }
        else if ( nSetBorders & (1 << aIdArr[i].first) )
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
            rBox.SetLine( nullptr, aIdArr[ i ].second );
        }
    }
    return bChange;
}

bool SwWW8ImplReader::SetShadow(SvxShadowItem& rShadow, const short *pSizeArray,
    const WW8_BRCVer9& aRightBrc)
{
    bool bRet = aRightBrc.fShadow() && pSizeArray && pSizeArray[WW8_RIGHT];
    if (bRet)
    {
        rShadow.SetColor(Color(COL_BLACK));
    //i120718
        short nVal = aRightBrc.DetermineBorderProperties();
    //End
        if (nVal < 0x10)
            nVal = 0x10;
        rShadow.SetWidth(nVal);
        rShadow.SetLocation(SVX_SHADOW_BOTTOMRIGHT);
        bRet = true;
    }
    return bRet;
}

void SwWW8ImplReader::GetBorderDistance(const WW8_BRCVer9* pbrc,
    Rectangle& rInnerDist)
{
    rInnerDist = Rectangle( pbrc[ 1 ].dptSpace() * 20,
                            pbrc[ 0 ].dptSpace() * 20,
                            pbrc[ 3 ].dptSpace() * 20,
                            pbrc[ 2 ].dptSpace() * 20 );
}

bool SwWW8ImplReader::SetFlyBordersShadow(SfxItemSet& rFlySet,
    const WW8_BRCVer9 *pbrc, short *pSizeArray)
{
    bool bShadowed = false;
    if (IsBorder(pbrc))
    {
        SvxBoxItem aBox( RES_BOX );
        SetBorder(aBox, pbrc, pSizeArray);

        rFlySet.Put( aBox );

        // fShadow
        SvxShadowItem aShadow( RES_SHADOW );
        if( SetShadow( aShadow, pSizeArray, pbrc[WW8_RIGHT] ))
        {
            bShadowed = true;
            rFlySet.Put( aShadow );
        }
    }
    return bShadowed;
}

//              APOs

                            // for computing the minimal FrameSize
#define MAX_BORDER_SIZE 210         // max. size of border
#define MAX_EMPTY_BORDER 10         // for off-by-one errors, at least 1

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
    return ( pS != nullptr );
}

inline bool SetValSprm( sal_Int16* pVar, const WW8RStyle* pStyle, sal_uInt16 nId )
{
    const sal_uInt8* pS = pStyle->HasParaSprm( nId );
    if( pS )
        *pVar = (sal_Int16)SVBT16ToShort( pS );
    return ( pS != nullptr );
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
        memcpy( this, pSrc, sizeof( WW8FlyPara ) ); // Copy-Ctor
    else
    {
        memset( this, 0, sizeof( WW8FlyPara ) );    // Default-Ctor
        nSp37 = 2;                                  // Default: wrapping
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

// Read for normal text
void WW8FlyPara::Read(sal_uInt8 nOrigSp29, WW8PLCFx_Cp_FKP* pPap)
{
    const sal_uInt8* pS = nullptr;
    if( bVer67 )
    {
        SetValSprm( &nSp26, pPap, 26 ); // X-position   //sprmPDxaAbs
        //set in me or in parent style
        mbVertSet |= SetValSprm( &nSp27, pPap, 27 );    // Y-position   //sprmPDyaAbs
        SetValSprm( &nSp45, pPap, 45 ); // height       //sprmPWHeightAbs
        SetValSprm( &nSp28, pPap, 28 ); // width        //sprmPDxaWidth
        SetValSprm( &nLeMgn, pPap, 49 ); // L-border    //sprmPDxaFromText
        SetValSprm( &nRiMgn, pPap, 49 ); // R-border    //sprmPDxaFromText
        SetValSprm( &nUpMgn, pPap, 48 ); // U-border    //sprmPDyaFromText
        SetValSprm( &nLoMgn, pPap, 48 ); // D-border    //sprmPDyaFromText

        pS = pPap->HasSprm( 37 );                       //sprmPWr
        if( pS )
            nSp37 = *pS;
    }
    else
    {
        SetValSprm( &nSp26, pPap, NS_sprm::LN_PDxaAbs ); // X-position
        //set in me or in parent style
        mbVertSet |= SetValSprm( &nSp27, pPap, NS_sprm::LN_PDyaAbs );    // Y-position
        SetValSprm( &nSp45, pPap, NS_sprm::LN_PWHeightAbs ); // height
        SetValSprm( &nSp28, pPap, NS_sprm::LN_PDxaWidth ); // width
        SetValSprm( &nLeMgn, pPap, NS_sprm::LN_PDxaFromText );    // L-border
        SetValSprm( &nRiMgn, pPap, NS_sprm::LN_PDxaFromText );    // R-border
        SetValSprm( &nUpMgn, pPap, NS_sprm::LN_PDyaFromText );    // U-border
        SetValSprm( &nLoMgn, pPap, NS_sprm::LN_PDyaFromText );    // D-border

        pS = pPap->HasSprm( NS_sprm::LN_PWr );                               // wrapping
        if( pS )
            nSp37 = *pS;
    }

    if( ::lcl_ReadBorders( bVer67, brc, pPap ))     // borders
        bBorderLines = ::lcl_IsBorder( brc );

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
    WW8PLCFMan* pPlcxMan = pIo->m_pPlcxMan;
    WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();

    Read(nOrigSp29, pPap);    // read Apo parameter

    do{             // block for quick exit
        if( nSp45 != 0 /* || nSp28 != 0 */ )
            break;                      // bGrafApo only automatic for height
        if( pIo->m_pWwFib->fComplex )
            break;                      // (*pPap)++ does not work for FastSave
                                        // -> for FastSave, no test for graphics APO
        SvStream* pIoStrm = pIo->m_pStrm;
        sal_uLong nPos = pIoStrm->Tell();
        WW8PLCFxSave1 aSave;
        pPlcxMan->GetPap()->Save( aSave );
        bGrafApo = false;

        do{             // block for quick exit
            sal_uInt8 nText[2];

            if (!checkRead(*pIoStrm, nText, 2)) // read text
                break;

            if( nText[0] != 0x01 || nText[1] != 0x0d )// only graphics + CR?
                break;                              // no

            pPap->advance();                        // next line

            // in APO ?
            //sprmPPc
            const sal_uInt8* pS = pPap->HasSprm( bVer67 ? 29 : 0x261B );

            // no -> graphics Apo
            if (!pS)
            {
                bGrafApo = true;
                break;                              // end of APO
            }

            ww::WordVersion eVer = pIo->GetFib().GetFIBVersion();
            WW8FlyPara *pNowStyleApo=nullptr;
            sal_uInt16 nColl = pPap->GetIstd();
            ww::sti eSti = eVer < ww::eWW6 ? ww::GetCanonicalStiFromStc( static_cast< sal_uInt8 >(nColl) ) : static_cast<ww::sti>(nColl);
            while (eSti != ww::stiNil && nColl < pIo->m_vColl.size() && nullptr == (pNowStyleApo = pIo->m_vColl[nColl].m_pWWFly))
            {
                nColl = pIo->m_vColl[nColl].m_nBase;
                eSti = eVer < ww::eWW6 ? ww::GetCanonicalStiFromStc( static_cast< sal_uInt8 >(nColl) ) : static_cast<ww::sti>(nColl);
            }

            WW8FlyPara aF(bVer67, pNowStyleApo);
                                                // new FlaPara for comparison
            aF.Read( *pS, pPap );               // WWPara for new Para
            if( !( aF == *this ) )              // same APO? (or a new one?)
                bGrafApo = true;                // no -> 1-line APO
                                                //    -> graphics APO
        }
        while( false );                             // block for quick exit

        pPlcxMan->GetPap()->Restore( aSave );
        pIoStrm->Seek( nPos );
    }while( false );                                    // block for quick exit
}

// read for Apo definitions in Styledefs
void WW8FlyPara::Read(sal_uInt8 nOrigSp29, WW8RStyle* pStyle)
{
    const sal_uInt8* pS = nullptr;
    if (bVer67)
    {
        SetValSprm( &nSp26, pStyle, 26 );   // X-position
        //set in me or in parent style
        mbVertSet |= SetValSprm(&nSp27, pStyle, 27);    // Y-position
        SetValSprm( &nSp45, pStyle, 45 );   // height
        SetValSprm( &nSp28, pStyle, 28 );   // width
        SetValSprm( &nLeMgn,    pStyle, 49 );   // L-border
        SetValSprm( &nRiMgn,    pStyle, 49 );   // R-border
        SetValSprm( &nUpMgn,    pStyle, 48 );   // U-border
        SetValSprm( &nLoMgn,    pStyle, 48 );   // D-border

        pS = pStyle->HasParaSprm( 37 );             // wrapping
        if( pS )
            nSp37 = *pS;
    }
    else
    {
        SetValSprm( &nSp26, pStyle, 0x8418 );   // X-position
        //set in me or in parent style
        mbVertSet |= SetValSprm(&nSp27, pStyle, 0x8419);    // Y-position
        SetValSprm( &nSp45, pStyle, 0x442B );   // height
        SetValSprm( &nSp28, pStyle, 0x841A );   // width
        SetValSprm( &nLeMgn, pStyle, 0x842F );  // L-border
        SetValSprm( &nRiMgn, pStyle, 0x842F );  // R-border
        SetValSprm( &nUpMgn, pStyle, 0x842E );  // U-border
        SetValSprm( &nLoMgn, pStyle, 0x842E );  // D-border

        pS = pStyle->HasParaSprm( 0x2423 );             // wrapping
        if( pS )
            nSp37 = *pS;
    }

    if (::lcl_ReadBorders(bVer67, brc, nullptr, pStyle))      // border
        bBorderLines = ::lcl_IsBorder(brc);

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

// #i18732# - changes made on behalf of CMC
WW8SwFlyPara::WW8SwFlyPara( SwPaM& rPaM,
                            SwWW8ImplReader& rIo,
                            WW8FlyPara& rWW,
                            const sal_uInt32 nWWPgTop,
                            const sal_uInt32 nPgLeft,
                            const sal_uInt32 nPgWidth,
                            const sal_Int32 nIniFlyDx,
                            const sal_Int32 nIniFlyDy )
{
    (void) nPgLeft;

    memset( this, 0, sizeof( WW8SwFlyPara ) );  // initialize
    nNewNetWidth = MINFLY;                    // minimum

    eSurround = ( rWW.nSp37 > 1 ) ? SURROUND_IDEAL : SURROUND_NONE;
    //#i119466 mapping "Around" wrap setting to "Parallel" for table
    const bool bIsTable = rIo.m_pPlcxMan->HasParaSprm(0x2416);
    if (  bIsTable && rWW.nSp37 == 2 )
        eSurround = SURROUND_PARALLEL;

    /*
     #95905#, #83307# seems to have gone away now, so reenable parallel
     wrapping support for frames in headers/footers. I don't know if we truly
     have an explicitly specified behaviour for these circumstances.
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
    {                           // no data, or bad data
        eHeightFix = ATT_MIN_SIZE;
        nHeight = MINFLY;
    }

    nWidth = nNetWidth = rWW.nSp28;
    if( nWidth <= 10 )                              // auto width
    {
        bAutoWidth = true;
        nWidth = nNetWidth =
            msword_cast<sal_Int16>((nPgWidth ? nPgWidth : 2268)); // 4 cm
    }
    if( nWidth <= MINFLY )
        nWidth = nNetWidth = MINFLY;              // minimum width

    eVAlign = text::VertOrientation::NONE;                            // defaults
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

    // If the Fly is aligned left, right, up, or down,
    // the outer text distance will be ignored, because otherwise
    // the Fly will end up in the wrong position.
    // The only problem is with inside/outside.

    // Bindung
    nYBind = (( rWW.nSp29 & 0x30 ) >> 4);
    //#i53725# - absolute positioned objects have to be
    // anchored at-paragraph to assure its correct anchor position.
    eAnchor = FLY_AT_PARA;
    rIo.m_pLastAnchorPos.reset( new SwPosition(*rPaM.GetPoint()));

    switch (nYBind)
    {
        case 0:     //relative to margin
            eVRel = text::RelOrientation::PAGE_PRINT_AREA;
            break;
        case 1:     //relative to page
            eVRel = text::RelOrientation::PAGE_FRAME;
            break;
        default:    //relative to text
            eVRel = text::RelOrientation::FRAME;
            break;
    }

// #i18732#
    switch( rWW.nSp27 )             // particular Y-positions ?
    {
        case -4:
            eVAlign = text::VertOrientation::TOP;
            if (nYBind < 2)
                nUpMgn = 0;
            break;  // up
        case -8:
            eVAlign = text::VertOrientation::CENTER;
            break;  // centered
        case -12:
            eVAlign = text::VertOrientation::BOTTOM;
            if (nYBind < 2)
                nLoMgn = 0;
            break;  // down
        default:
            nYPos = rWW.nSp27 + (short)nIniFlyDy;
            break;  // corrections from ini file
    }

    switch( rWW.nSp26 )                 // particular X-positions ?
    {
        case 0:
            eHAlign = text::HoriOrientation::LEFT;
            nLeMgn = 0;
            break;  // left
        case -4:
            eHAlign = text::HoriOrientation::CENTER;
            break;  // centered
        case -8:
            eHAlign = text::HoriOrientation::RIGHT;
            nRiMgn = 0;
            break;  // right
        case -12:
            eHAlign = text::HoriOrientation::LEFT;
            bToggelPos = true;
            break;  // inside
        case -16:
            eHAlign = text::HoriOrientation::RIGHT;
            bToggelPos = true;
            break;  // outside
        default:
            nXPos = rWW.nSp26 + (short)nIniFlyDx;
            break;  // corrections from ini file
    }

    nXBind = ( rWW.nSp29 & 0xc0 ) >> 6;
// #i18732#
    switch (nXBind)           // X - binding -> transform coordinates
    {
        case 0:     //relative to column
            eHRel = text::RelOrientation::FRAME;
            break;
        case 1:     //relative to margin
            eHRel = text::RelOrientation::PAGE_PRINT_AREA;
            break;
        default:    //relative to page
            eHRel = text::RelOrientation::PAGE_FRAME;
            break;
    }

    // #i36649# - adjustments for certain horizontal alignments
    // Note: These special adjustments found by an investigation of documents
    //       containing frames with different left/right border distances and
    //       distances to text. The outcome is some how strange.
    // Note: These adjustments causes wrong horizontal positions for frames,
    //       which are aligned inside|outside to page|margin on even pages,
    //       the left and right border distances are different.
    // no adjustments possible, if frame has automatic width.
    // determine left border distance
    sal_Int16 nLeBorderMgn( 0L );
    if ( !bAutoWidth )
    {
        WW8_BRCVer9 &rBrc = rWW.brc[WW8_LEFT];
        sal_Int16 nTemp = rBrc.DetermineBorderProperties(&nLeBorderMgn);
        nLeBorderMgn = nLeBorderMgn + nTemp;
    }
    // determine right border distance
    sal_Int16 nRiBorderMgn( 0L );
    if ( !bAutoWidth )
    {
        WW8_BRCVer9 &rBrc = rWW.brc[WW8_RIGHT];
        sal_Int16 nTemp = rBrc.DetermineBorderProperties(&nRiBorderMgn);
        nRiBorderMgn = nRiBorderMgn + nTemp;
    }
    if ( !bAutoWidth && eHAlign == text::HoriOrientation::LEFT && eHRel == text::RelOrientation::PAGE_FRAME )
    {
        // convert 'left to page' to
        // 'from left -<width>-<2*left border distance>-<right wrap distance>
        // to page text area'
        eHAlign = text::HoriOrientation::NONE;
        eHRel = text::RelOrientation::PAGE_PRINT_AREA;
        nXPos = -nWidth - (2*nLeBorderMgn) - rWW.nRiMgn;
        // re-set left wrap distance
        nLeMgn = rWW.nLeMgn;
    }
    else if ( !bAutoWidth && eHAlign == text::HoriOrientation::RIGHT && eHRel == text::RelOrientation::PAGE_FRAME )
    {
        // convert 'right to page' to
        // 'from left <right border distance-left border distance>+<left wrap distance>
        // to right page border'
        eHAlign = text::HoriOrientation::NONE;
        eHRel = text::RelOrientation::PAGE_RIGHT;
        nXPos = ( nRiBorderMgn - nLeBorderMgn ) + rWW.nLeMgn;
        // re-set right wrap distance
        nRiMgn = rWW.nRiMgn;
    }
    else if ( !bAutoWidth && eHAlign == text::HoriOrientation::LEFT && eHRel == text::RelOrientation::PAGE_PRINT_AREA )
    {
        // convert 'left to margin' to
        // 'from left -<left border distance> to page text area'
        eHAlign = text::HoriOrientation::NONE;
        eHRel = text::RelOrientation::PAGE_PRINT_AREA;
        nXPos = -nLeBorderMgn;
        // re-set left wrap distance
        nLeMgn = rWW.nLeMgn;
    }
    else if ( !bAutoWidth && eHAlign == text::HoriOrientation::RIGHT && eHRel == text::RelOrientation::PAGE_PRINT_AREA )
    {
        // convert 'right to margin' to
        // 'from left -<width>-<left border distance> to right page border'
        eHAlign = text::HoriOrientation::NONE;
        eHRel = text::RelOrientation::PAGE_RIGHT;
        nXPos = -nWidth - nLeBorderMgn;
        // re-set right wrap distance
        nRiMgn = rWW.nRiMgn;
    }
    else if (rWW.bBorderLines)
    {
        /*
        #i582#
        Word has a curious bug where the offset stored do not take into
        account the internal distance from the corner both
        */
        WW8_BRCVer9 &rBrc = rWW.brc[WW8_LEFT];
        sal_Int16 nLeLMgn = 0;
        sal_Int16 nTemp = rBrc.DetermineBorderProperties(&nLeLMgn);
        nLeLMgn = nLeLMgn + nTemp;

        if (nLeLMgn)
        {
            if (eHAlign == text::HoriOrientation::LEFT)
                eHAlign = text::HoriOrientation::NONE;
            nXPos = nXPos - nLeLMgn;
        }
    }

    // adjustments for certain vertical alignments
    if ( eVAlign == text::VertOrientation::NONE && eVRel == text::RelOrientation::PAGE_PRINT_AREA )
    {
        // convert "<X> from top page text area" to
        // "<X + page top margin> from page"
        eVRel = text::RelOrientation::PAGE_FRAME;
        nYPos = static_cast< sal_Int16 >( nYPos + nWWPgTop );
    }

    FlySecur1( nWidth, rWW.bBorderLines );          // passen Raender ?
    FlySecur1( nHeight, rWW.bBorderLines );

}

// If a Fly in WW has automatic width, this has to be simulated
// by modifying the Fly width (fixed in SW) afterwards.
// This can increase or decrease the Fly width, because the default value
// is set without knowledge of the contents.
void WW8SwFlyPara::BoxUpWidth( long nInWidth )
{
    if( bAutoWidth && nInWidth > nNewNetWidth )
        nNewNetWidth = nInWidth;
};

// The class WW8FlySet is derived from SfxItemSet and does not
// provide more, but is easier to handle for me.
// WW8FlySet-ctor for Apos and graphics Apos
WW8FlySet::WW8FlySet(SwWW8ImplReader& rReader, const WW8FlyPara* pFW,
    const WW8SwFlyPara* pFS, bool bGraf)
    : SfxItemSet(rReader.m_rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
{
    if (!rReader.m_bNewDoc)
        Reader::ResetFrameFormatAttrs(*this);    // remove distance/border
                                            // position
    Put(SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR));

/*Below can all go when we have from left in rtl mode*/
    SwTwips nXPos = pFS->nXPos;
    sal_Int16 eHRel = pFS->eHRel;
    rReader.MiserableRTLGraphicsHack(nXPos, pFS->nWidth, pFS->eHAlign, eHRel);
/*Above can all go when we have from left in rtl mode*/
    Put( SwFormatHoriOrient(nXPos, pFS->eHAlign, pFS->eHRel, pFS->bToggelPos ));
    Put( SwFormatVertOrient( pFS->nYPos, pFS->eVAlign, pFS->eVRel ) );

    if (pFS->nLeMgn || pFS->nRiMgn)     // Raender setzen
        Put(SvxLRSpaceItem(pFS->nLeMgn, pFS->nRiMgn, 0, 0, RES_LR_SPACE));

    if (pFS->nUpMgn || pFS->nLoMgn)
        Put(SvxULSpaceItem(pFS->nUpMgn, pFS->nLoMgn, RES_UL_SPACE));

    //we no longer need to hack around the header/footer problems
    SwFormatSurround aSurround(pFS->eSurround);
    if ( pFS->eSurround == SURROUND_IDEAL )
        aSurround.SetAnchorOnly( true );
    Put( aSurround );

    short aSizeArray[5]={0};
    SwWW8ImplReader::SetFlyBordersShadow(*this,pFW->brc,&aSizeArray[0]);

    // the 5th parameter is always 0, thus we lose nothing due to the cast

    // #i27767#
    // #i35017# - constant name has changed
    Put( SwFormatWrapInfluenceOnObjPos(
                text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ) );

    if( !bGraf )
    {
        Put( SwFormatAnchor(pFS->eAnchor) );
        // adjust size

        //Ordinarily with frames, the border width and spacing is
        //placed outside the frame, making it larger. With these
        //types of frames, the left right thickness and space makes
        //it wider, but the top bottom spacing and border thickness
        //is placed inside.
        Put( SwFormatFrameSize( pFS->eHeightFix, pFS->nWidth +
            aSizeArray[WW8_LEFT] + aSizeArray[WW8_RIGHT],
            pFS->nHeight));
    }
}

// WW8FlySet-ctor for character bound graphics
WW8FlySet::WW8FlySet( SwWW8ImplReader& rReader, const SwPaM* pPaM,
    const WW8_PIC& rPic, long nWidth, long nHeight )
    : SfxItemSet(rReader.m_rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
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
    WW8_BRCVer9 brcVer9[4];
    for (int i = 0; i < 4; i++)
        brcVer9[i] = WW8_BRCVer9(rPic.rgbrc[i]);
    if (SwWW8ImplReader::SetFlyBordersShadow( *this, brcVer9, &aSizeArray[0]))
    {
        Put(SvxLRSpaceItem( aSizeArray[WW8_LEFT], 0, 0, 0, RES_LR_SPACE ) );
        Put(SvxULSpaceItem( aSizeArray[WW8_TOP], 0, RES_UL_SPACE ));
        aSizeArray[WW8_RIGHT]*=2;
        aSizeArray[WW8_BOT]*=2;
    }

    Put( SwFormatFrameSize( ATT_FIX_SIZE, nWidth+aSizeArray[WW8_LEFT]+
        aSizeArray[WW8_RIGHT], nHeight+aSizeArray[WW8_TOP]
        + aSizeArray[WW8_BOT]) );
}

void WW8FlySet::Init(const SwWW8ImplReader& rReader, const SwPaM* pPaM)
{
    if (!rReader.m_bNewDoc)
        Reader::ResetFrameFormatAttrs(*this);  // remove distance/borders

    Put(SvxLRSpaceItem(RES_LR_SPACE)); //inline writer ole2 objects start with 0.2cm l/r
    SwFormatAnchor aAnchor(FLY_AS_CHAR);

    aAnchor.SetAnchor(pPaM->GetPoint());
    Put(aAnchor);

    //The horizontal default is on the baseline, the vertical is centered
    //around the character center it appears
    if (rReader.m_aSectionManager.CurrentSectionIsVertical())
        Put(SwFormatVertOrient(0, text::VertOrientation::CHAR_CENTER,text::RelOrientation::CHAR));
    else
        Put(SwFormatVertOrient(0, text::VertOrientation::TOP, text::RelOrientation::FRAME));
}

WW8DupProperties::WW8DupProperties(SwDoc &rDoc, SwWW8FltControlStack *pStack)
    : pCtrlStck(pStack),
    aChrSet(rDoc.GetAttrPool(), RES_CHRATR_BEGIN, RES_CHRATR_END - 1 ),
    aParSet(rDoc.GetAttrPool(), RES_PARATR_BEGIN, RES_PARATR_END - 1 )
{
    //Close any open character properties and duplicate them inside the
    //first table cell
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
            }while( !aIter.IsAtEnd() && nullptr != ( pItem = aIter.NextItem() ) );
        }
    }
}

void SwWW8ImplReader::MoveInsideFly(const SwFrameFormat *pFlyFormat)
{
    WW8DupProperties aDup(m_rDoc,m_pCtrlStck);

    m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), 0, false);

    // set Pam in FlyFrame
    const SwFormatContent& rContent = pFlyFormat->GetContent();
    OSL_ENSURE( rContent.GetContentIdx(), "No content prepared." );
    m_pPaM->GetPoint()->nNode = rContent.GetContentIdx()->GetIndex() + 1;
    m_pPaM->GetPoint()->nContent.Assign( m_pPaM->GetContentNode(), 0 );

    aDup.Insert(*m_pPaM->GetPoint());
}

SwTwips SwWW8ImplReader::MoveOutsideFly(SwFrameFormat *pFlyFormat,
    const SwPosition &rPos, bool bTableJoin)
{
    SwTwips nRetWidth = 0;
    if (!pFlyFormat)
        return nRetWidth;
    // Close all attributes, because otherwise attributes can appear
    // that extend out of Flys
    WW8DupProperties aDup(m_rDoc,m_pCtrlStck);
    m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), 0, false);

    /*
    #i1291
    If this fly frame consists entirely of one table inside a frame
    followed by an empty paragraph then we want to delete the empty
    paragraph so as to get the frame to autoshrink to the size of the
    table to emulate words behaviour closer.
    */
    if (bTableJoin)
    {
        const SwNodeIndex* pNodeIndex = pFlyFormat->GetContent().
            GetContentIdx();
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
                    if ( (aIdx < aEnd) && aIdx.GetNode().IsTextNode() )
                    {
                        SwTextNode *pNd = aIdx.GetNode().GetTextNode();
                        ++aIdx;
                        if (aIdx == aEnd && pNd && pNd->GetText().isEmpty())
                        {
                            //An extra pre-created by writer unused paragraph

                            //delete after import is complete rather than now
                            //to avoid the complication of managing uncommitted
                            //ctrlstack properties that refer to it.
                            m_aExtraneousParas.insert(pNd);

                            SwTable& rTable = pTable->GetTable();
                            SwFrameFormat* pTableFormat = rTable.GetFrameFormat();

                            if (pTableFormat)
                            {
                                SwFormatFrameSize aSize = pTableFormat->GetFrameSize();
                                aSize.SetHeightSizeType(ATT_MIN_SIZE);
                                aSize.SetHeight(MINLAY);
                                pFlyFormat->SetFormatAttr(aSize);
                                SwFormatHoriOrient aHori = pTableFormat->GetHoriOrient();
                                // passing the table orientaion of
                                // LEFT_AND_WIDTH to the frame seems to
                                // work better than FULL, especially if the
                                // table width exceeds the page width, however
                                // I am not brave enough to set it in all
                                // instances
                                pTableFormat->SetFormatAttr( SwFormatHoriOrient(0, ( aHori.GetHoriOrient() == text::HoriOrientation::LEFT_AND_WIDTH ) ? ::text::HoriOrientation::LEFT_AND_WIDTH : text::HoriOrientation::FULL ) );
                                nRetWidth = aSize.GetWidth();
                            }
                        }
                    }
                }
            }
        }
    }

    *m_pPaM->GetPoint() = rPos;
    aDup.Insert(*m_pPaM->GetPoint());
    return nRetWidth;
}

WW8FlyPara *SwWW8ImplReader::ConstructApo(const ApoTestResults &rApo,
    const WW8_TablePos *pTabPos)
{
    WW8FlyPara *pRet = nullptr;
    OSL_ENSURE(rApo.HasFrame() || pTabPos,
        "If no frame found, *MUST* be in a table");

    pRet = new WW8FlyPara(m_bVer67, rApo.mpStyleApo);

    // find APO parameter and test for bGrafApo
    if (rApo.HasFrame())
        pRet->ReadFull(rApo.m_nSprm29, this);

    pRet->ApplyTabPos(pTabPos);

    if (pRet->IsEmpty())
    {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool SwWW8ImplReader::IsDropCap()
{
    // Find the DCS (Drop Cap Specifier) for the paragraph
    // if does not exist or if the first three bits are 0
    // then there is no dropcap on the paragraph
    WW8PLCFx_Cp_FKP *pPap = m_pPlcxMan ? m_pPlcxMan->GetPapPLCF() : nullptr;
    if (pPap)
    {
        const sal_uInt8 *pDCS;
        if (m_bVer67)
            pDCS = pPap->HasSprm(46);
        else
            pDCS = m_pPlcxMan->GetPapPLCF()->HasSprm(0x442C);
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
    const WW8_TablePos *pTabPos,
    SvxULSpaceItem* pULSpaceItem)
{
    if (nullptr == (m_pWFlyPara = ConstructApo(rApo, pTabPos)))
        return false;

    // <WW8SwFlyPara> constructor has changed - new 4th parameter
    // containing WW8 page top margin.
    m_pSFlyPara = new WW8SwFlyPara( *m_pPaM, *this, *m_pWFlyPara,
                                  m_aSectionManager.GetWWPageTopMargin(),
                                  m_aSectionManager.GetPageLeft(),
                                  m_aSectionManager.GetTextAreaWidth(),
                                  m_nIniFlyDx, m_nIniFlyDy);

    // If this paragraph is a Dropcap set the flag and we will deal with it later
    if (IsDropCap())
    {
        m_bDropCap = true;
        m_pAktItemSet = new SfxItemSet( m_rDoc.GetAttrPool(), RES_CHRATR_BEGIN, RES_PARATR_END - 1 );
        return false;
    }

    if( !m_pWFlyPara->bGrafApo )
    {

        // Within the GrafApo text attributes have to be ignored, because
        // they would apply to the following lines.  The frame is only inserted
        // if it is not merely positioning a single image.  If it is an image
        // frame, pWFlyPara and pSFlyPara are retained and the resulting
        // attributes applied to the image when inserting the image.

        WW8FlySet aFlySet(*this, m_pWFlyPara, m_pSFlyPara, false);

        if (pTabPos && pTabPos->bNoFly)
        {
            m_pSFlyPara->pFlyFormat = nullptr;
            if (pULSpaceItem)
            {
                // Word positioned tables can have a position (like a
                // fly-frame), but they also support flowing across multiple
                // pages. If we decide to import this as a normal table (so it
                // can flow across multiple pages), then turn the vertical
                // orientation position of the fly into a table upper margin.
                const SfxPoolItem* pItem = nullptr;
                if (aFlySet.HasItem(RES_VERT_ORIENT, &pItem))
                {
                    const SwFormatVertOrient* pOrient = static_cast<const SwFormatVertOrient*>(pItem);
                    if (pOrient->GetPos() != 0)
                        pULSpaceItem->SetUpper(pOrient->GetPos());
                }
            }
        }
        else
        {
            m_pSFlyPara->pFlyFormat = m_rDoc.MakeFlySection( m_pSFlyPara->eAnchor,
                    m_pPaM->GetPoint(), &aFlySet );
            OSL_ENSURE(m_pSFlyPara->pFlyFormat->GetAnchor().GetAnchorId() ==
                    m_pSFlyPara->eAnchor, "Not the anchor type requested!");
        }

        if (m_pSFlyPara->pFlyFormat)
        {
            if (!m_pDrawModel)
                GrafikCtor();

            SdrObject* pOurNewObject = CreateContactObject(m_pSFlyPara->pFlyFormat);
            m_pWWZOrder->InsertTextLayerObject(pOurNewObject);
        }

        if (FLY_AS_CHAR != m_pSFlyPara->eAnchor && m_pSFlyPara->pFlyFormat)
        {
            m_pAnchorStck->AddAnchor(*m_pPaM->GetPoint(),m_pSFlyPara->pFlyFormat);
        }

        // remember Pos in body text
        m_pSFlyPara->pMainTextPos = new SwPosition( *m_pPaM->GetPoint() );

        //remove fltanchors, otherwise they will be closed inside the
        //frame, which makes no sense, restore them after the frame is
        //closed
        m_pSFlyPara->pOldAnchorStck = m_pAnchorStck;
        m_pAnchorStck = new SwWW8FltAnchorStack(&m_rDoc, m_nFieldFlags);

        if (m_pSFlyPara->pFlyFormat)
            MoveInsideFly(m_pSFlyPara->pFlyFormat);

        // 1) ReadText() is not called recursively because the length of
        //    the Apo is unknown at that  time, and ReadText() needs it.
        // 2) the CtrlStck is not re-created.
        //    the Char attributes continue (trouble with Sw-attributes)
        //    Para attributes must be reset at the end of every paragraph,
        //    i.e. at the end of a paragraph there must not be para attributes
        //    on the stack
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
    rPam.GetPoint()->nContent = 0; // go to start of paragraph

    SwNodeIndex aPref(rPam.GetPoint()->nNode, -1);

    if (SwTextNode* pNode = aPref.GetNode().GetTextNode())
    {
        m_aSectionManager.JoinNode(*rPam.GetPoint(), aPref.GetNode());
        rPam.GetPoint()->nNode = aPref;
        rPam.GetPoint()->nContent.Assign(pNode, pNode->GetText().getLength());
        if (bStealAttr)
            m_pCtrlStck->StealAttr(rPam.GetPoint()->nNode);

        pNode->JoinNext();

        bRet = true;
    }
    return bRet;
}

//In auto-width word frames negative after-indent values are ignored
void SwWW8ImplReader::StripNegativeAfterIndent(SwFrameFormat *pFlyFormat)
{
    const SwNodeIndex* pSttNd = pFlyFormat->GetContent().GetContentIdx();
    if (!pSttNd)
        return;

    SwNodeIndex aIdx(*pSttNd, 1);
    SwNodeIndex aEnd(*pSttNd->GetNode().EndOfSectionNode());
    while (aIdx < aEnd)
    {
        SwTextNode *pNd = aIdx.GetNode().GetTextNode();
        if (pNd)
        {
            const SvxLRSpaceItem& rLR = ItemGet<SvxLRSpaceItem>(*pNd, RES_LR_SPACE);
            if (rLR.GetRight() < 0)
            {
                SvxLRSpaceItem aLR(rLR);
                aLR.SetRight(0);
                pNd->SetAttr(aLR);
            }
        }
        ++aIdx;
    }
}

void SwWW8ImplReader::StopApo()
{
    OSL_ENSURE(m_pWFlyPara, "no pWFlyPara to close");
    if (!m_pWFlyPara)
        return;
    if (m_pWFlyPara->bGrafApo)
    {
        // image frame that has not been inserted: delete empty paragraph + attr
        JoinNode(*m_pPaM, true);

    }
    else
    {
        if (!m_pSFlyPara->pMainTextPos || !m_pWFlyPara)
        {
            OSL_ENSURE( m_pSFlyPara->pMainTextPos, "StopApo: pMainTextPos ist 0" );
            OSL_ENSURE( m_pWFlyPara, "StopApo: pWFlyPara ist 0" );
            return;
        }

        /*
        What we are doing with this temporary nodeindex is as follows: The
        stack of attributes normally only places them into the document when
        the current insertion point has passed them by. Otherwise the end
        point of the attribute gets pushed along with the insertion point. The
        insertion point is moved and the properties committed during
        MoveOutsideFly. We also may want to remove the final paragraph in the
        frame, but we need to wait until the properties for that frame text
        have been committed otherwise they will be lost. So we first get a
        handle to the last the filter inserted. After the attributes are
        committed, if that paragraph exists we join it with the para after it
        that comes with the frame by default so that as normal we don't end up
        with one more paragraph than we wanted.
        */
        SwNodeIndex aPref(m_pPaM->GetPoint()->nNode, -1);

        SwTwips nNewWidth =
            MoveOutsideFly(m_pSFlyPara->pFlyFormat, *m_pSFlyPara->pMainTextPos);
        if (nNewWidth)
            m_pSFlyPara->BoxUpWidth(nNewWidth);

        Color aBg(0xFE, 0xFF, 0xFF, 0xFF);  //Transparent by default

        SwTextNode* pNd = aPref.GetNode().GetTextNode();
        if (pNd && m_pSFlyPara->pFlyFormat)
        {
            /*
            #i582#
            Take the last paragraph background colour and fill the frame with
            it.  Otherwise, make it transparent, this appears to be how MSWord
            works
            */
            const SfxPoolItem &rItm = pNd->SwContentNode::GetAttr(RES_BACKGROUND);
            const SvxBrushItem &rBrush = static_cast<const SvxBrushItem&>(rItm);
            if (rBrush.GetColor().GetColor() != COL_AUTO)
                aBg = rBrush.GetColor();

            if (m_pLastAnchorPos.get())
            {
                //If the last anchor pos is here, then clear the anchor pos.
                //This "last anchor pos" is only used for fixing up the
                //positions of things anchored to page breaks and here
                //we are removing the last paragraph of a frame, so there
                //cannot be a page break at this point so we can
                //safely reset m_pLastAnchorPos to avoid any dangling
                //SwIndex's pointing into the deleted paragraph
                SwNodeIndex aLastAnchorPos(m_pLastAnchorPos->nNode);
                SwNodeIndex aToBeJoined(aPref, 1);
                if (aLastAnchorPos == aToBeJoined)
                    m_pLastAnchorPos.reset();
            }

            //Get rid of extra empty paragraph
            pNd->JoinNext();
        }

        if (m_pSFlyPara->pFlyFormat)
            m_pSFlyPara->pFlyFormat->SetFormatAttr(SvxBrushItem(aBg, RES_BACKGROUND));

        DeleteAnchorStack();
        m_pAnchorStck = m_pSFlyPara->pOldAnchorStck;

        // When inserting a graphic into the fly frame using the auto
        // function, the extension of the SW-fly has to be set
        // manually as the SW fly has no auto function to adjust the
        // frame´s size.
        if(m_pSFlyPara->nNewNetWidth > MINFLY && m_pSFlyPara->pFlyFormat)    // BoxUpWidth ?
        {
            long nW = m_pSFlyPara->nNewNetWidth;
            nW += m_pSFlyPara->nWidth - m_pSFlyPara->nNetWidth;   // border for it
            m_pSFlyPara->pFlyFormat->SetFormatAttr(
                SwFormatFrameSize( m_pSFlyPara->eHeightFix, nW, m_pSFlyPara->nHeight ) );
        }
        /*
        Word set *no* width meaning its an automatic width. The
        SwFlyPara reader will have already set a fallback width of the
        printable regions width, so we should reuse it. Despite the related
        problems with layout addressed with a hack in WW8FlyPara's constructor
        #i27204# Added AutoWidth setting. Left the old CalculateFlySize in place
        so that if the user unselects autowidth, the width doesn't max out
        */
        else if( !m_pWFlyPara->nSp28 && m_pSFlyPara->pFlyFormat)
        {
            using namespace sw::util;
            SfxItemSet aFlySet( m_pSFlyPara->pFlyFormat->GetAttrSet() );

            SwFormatFrameSize aSize(ItemGet<SwFormatFrameSize>(aFlySet, RES_FRM_SIZE));

            aFlySet.ClearItem(RES_FRM_SIZE);

            CalculateFlySize(aFlySet, m_pSFlyPara->pMainTextPos->nNode,
                m_pSFlyPara->nWidth);

            nNewWidth = ItemGet<SwFormatFrameSize>(aFlySet, RES_FRM_SIZE).GetWidth();

            aSize.SetWidth(nNewWidth);
            aSize.SetWidthSizeType(ATT_VAR_SIZE);

            m_pSFlyPara->pFlyFormat->SetFormatAttr(aSize);
        }

        delete m_pSFlyPara->pMainTextPos;
        m_pSFlyPara->pMainTextPos = nullptr;
// To create the SwFrames when inserting into an existing document, fltshell.cxx
// will call pFlyFrame->MakeFrames() when setting the FltAnchor attribute

    }

    //#i8062#
    if (m_pSFlyPara && m_pSFlyPara->pFlyFormat)
        m_pFormatOfJustInsertedApo = m_pSFlyPara->pFlyFormat;

    DELETEZ( m_pSFlyPara );
    DELETEZ( m_pWFlyPara );
}

// TestSameApo() returns if it's the same Apo or a different one
bool SwWW8ImplReader::TestSameApo(const ApoTestResults &rApo,
    const WW8_TablePos *pTabPos)
{
    if( !m_pWFlyPara )
    {
        OSL_ENSURE( m_pWFlyPara, " Where is my pWFlyPara ? " );
        return true;
    }

    // We need to a full comparison (excepting borders) to identify all
    // combinations style/hard correctly. For this reason we create a
    // temporary WW8FlyPara (depending on if style or not), apply the
    // hard attributes and then compare.

    // For comparison
    WW8FlyPara aF(m_bVer67, rApo.mpStyleApo);
    // WWPara for current para
    if (rApo.HasFrame())
        aF.Read(rApo.m_nSprm29, m_pPlcxMan->GetPapPLCF());
    aF.ApplyTabPos(pTabPos);

    return aF == *m_pWFlyPara;
}

void SwWW8ImplReader::NewAttr( const SfxPoolItem& rAttr,
                               const bool bFirstLineOfStSet,
                               const bool bLeftIndentSet )
{
    if( !m_bNoAttrImport ) // for ignoring styles during doc inserts
    {
        if (m_pAktColl)
        {
            OSL_ENSURE(rAttr.Which() != RES_FLTR_REDLINE, "redline in style!");
            m_pAktColl->SetFormatAttr(rAttr);
        }
        else if (m_pAktItemSet)
        {
            m_pAktItemSet->Put(rAttr);
        }
        else if (rAttr.Which() == RES_FLTR_REDLINE)
        {
            m_pRedlineStack->open(*m_pPaM->GetPoint(), rAttr);
        }
        else
        {
            m_pCtrlStck->NewAttr(*m_pPaM->GetPoint(), rAttr);
            // #i103711#
            if ( bFirstLineOfStSet )
            {
                const SwNode* pNd = &(m_pPaM->GetPoint()->nNode.GetNode());
                m_aTextNodesHavingFirstLineOfstSet.insert( pNd );
            }
            // #i105414#
            if ( bLeftIndentSet )
            {
                const SwNode* pNd = &(m_pPaM->GetPoint()->nNode.GetNode());
                m_aTextNodesHavingLeftIndentSet.insert( pNd );
            }
        }

        if (m_pPostProcessAttrsInfo && m_pPostProcessAttrsInfo->mbCopy)
            m_pPostProcessAttrsInfo->mItemSet.Put(rAttr);
    }
}

// fetches attribute from FormatColl / Stack / Doc
const SfxPoolItem* SwWW8ImplReader::GetFormatAttr( sal_uInt16 nWhich )
{
    const SfxPoolItem* pRet = nullptr;
    if (m_pAktColl)
        pRet = &(m_pAktColl->GetFormatAttr(nWhich));
    else if (m_pAktItemSet)
    {
        pRet = m_pAktItemSet->GetItem(nWhich);
        if (!pRet)
            pRet = m_pStandardFormatColl ? &(m_pStandardFormatColl->GetFormatAttr(nWhich)) : nullptr;
        if (!pRet)
            pRet = &m_rDoc.GetAttrPool().GetDefaultItem(nWhich);
    }
    else if (m_pPlcxMan && m_pPlcxMan->GetDoingDrawTextBox())
    {
        pRet = m_pCtrlStck->GetStackAttr(*m_pPaM->GetPoint(), nWhich);
        if (!pRet)
        {
            if (m_nAktColl < m_vColl.size() && m_vColl[m_nAktColl].m_pFormat &&
                m_vColl[m_nAktColl].m_bColl)
            {
                pRet = &(m_vColl[m_nAktColl].m_pFormat->GetFormatAttr(nWhich));
            }
        }
        if (!pRet)
            pRet = m_pStandardFormatColl ? &(m_pStandardFormatColl->GetFormatAttr(nWhich)) : nullptr;
        if (!pRet)
            pRet = &m_rDoc.GetAttrPool().GetDefaultItem(nWhich);
    }
    else
        pRet = m_pCtrlStck->GetFormatAttr(*m_pPaM->GetPoint(), nWhich);
    return pRet;
}

// The methods get as parameters the token id and the length of the following
// parameters according to the table in WWScan.cxx.
void SwWW8ImplReader::Read_Special(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
    {
        m_bSpec = false;
        return;
    }
    m_bSpec = ( *pData != 0 );
}

// Read_Obj is used for fObj and for fOle2 !
void SwWW8ImplReader::Read_Obj(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
        m_bObj = false;
    else
    {
        m_bObj = 0 != *pData;

        if( m_bObj && m_nPicLocFc && m_bEmbeddObj )
        {
            if (!m_aFieldStack.empty() && m_aFieldStack.back().mnFieldId == 56)
            {
                // For LINK fields, store the nObjLocFc value in the field entry
                m_aFieldStack.back().mnObjLocFc = m_nPicLocFc;
            }
            else
            {
                m_nObjLocFc = m_nPicLocFc;
            }
        }
    }
}

void SwWW8ImplReader::Read_PicLoc(sal_uInt16 , const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        m_nPicLocFc = 0;
        m_bSpec = false;  // Is this always correct?
    }
    else
    {
        m_nPicLocFc = SVBT32ToUInt32( pData );
        m_bSpec = true;

        if( m_bObj && m_nPicLocFc && m_bEmbeddObj )
            m_nObjLocFc = m_nPicLocFc;
    }
}

void SwWW8ImplReader::Read_POutLvl(sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 0)
    {
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_PARATR_OUTLINELEVEL);
        return;
    }

    if (m_pAktColl != nullptr)
    {
        SwWW8StyInf* pSI = GetStyle(m_nAktColl);
        if (pSI != nullptr)
        {
            pSI->mnWW8OutlineLevel =
                    static_cast< sal_uInt8 >( ( pData ? *pData : 0 ) );
            NewAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL, SwWW8StyInf::WW8OutlineLevelToOutlinelevel( pSI->mnWW8OutlineLevel ) ) );
        }
    }
    else if (m_pPaM != nullptr)
    {
        const sal_uInt8 nOutlineLevel
            = SwWW8StyInf::WW8OutlineLevelToOutlinelevel(
                static_cast<sal_uInt8>((pData ? *pData : 0)));
        NewAttr(SfxUInt16Item(RES_PARATR_OUTLINELEVEL, nOutlineLevel));
    }
}

void SwWW8ImplReader::Read_Symbol(sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( !m_bIgnoreText )
    {
        if( nLen < 0 )
        {
            //otherwise disable after we print the char
            if (m_pPlcxMan && m_pPlcxMan->GetDoingDrawTextBox())
                m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_FONT );
            m_bSymbol = false;
        }
        else
        {
            // Make new Font-Attribut
            // (will be closed in SwWW8ImplReader::ReadChars() )

            //Will not be added to the charencoding stack, for styles the real
            //font setting will be put in as the styles charset, and for plain
            //text encoding for symbols is moot. Drawing boxes will check bSymbol
            //themselves so they don't need to add it to the stack either.
            if (SetNewFontAttr(SVBT16ToShort( pData ), false, RES_CHRATR_FONT))
            {
                SetNewFontAttr(SVBT16ToShort( pData ), false, RES_CHRATR_CJK_FONT);
                SetNewFontAttr(SVBT16ToShort( pData ), false, RES_CHRATR_CTL_FONT);
                if( m_bVer67 )
                {
                    //convert single byte from MS1252 to Unicode
                    m_cSymbol = OUString(
                        reinterpret_cast<const sal_Char*>(pData+2), 1,
                        RTL_TEXTENCODING_MS_1252).toChar();
                }
                else
                {
                    //already is Unicode
                    m_cSymbol = SVBT16ToShort( pData+2 );
                }
                m_bSymbol = true;
            }
        }
    }
}

SwWW8StyInf *SwWW8ImplReader::GetStyle(sal_uInt16 nColl) const
{
    return const_cast<SwWW8StyInf *>(nColl < m_vColl.size() ? &m_vColl[nColl] : nullptr);
}

// Read_BoldUsw for italic, bold, small caps, majuscule, struck out,
// contour and shadow
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

    ww::WordVersion eVersion = m_pWwFib->GetFIBVersion();

    sal_uInt8 nI;
    // the attribute number for "double strike-through" breaks rank
    if (0x2A53 == nId)
        nI = nContigiousWestern;               // The out of sequence western id
    else
    {
        // The contiguous western ids
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
                // reset the CTL Weight and Posture, because they are the same as their
                // western equivalents in ww6
                m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), nEndIds[ nWestern + nEastern + nI ] );
            }
            // reset the CJK Weight and Posture, because they are the same as their
            // western equivalents in word
            m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), nEndIds[ nWestern + nI ] );
        }
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), nEndIds[ nI ] );
        m_pCtrlStck->SetToggleAttr(nI, false);
        return;
    }
    // value: 0 = off, 1 = on, 128 = like style, 129 contrary to style
    bool bOn = *pData & 1;
    SwWW8StyInf* pSI = GetStyle(m_nAktColl);
    if (m_pPlcxMan && eVersion > ww::eWW2)
    {
        const sal_uInt8 *pCharIstd =
            m_pPlcxMan->GetChpPLCF()->HasSprm(m_bVer67 ? 80 : 0x4A30);
        if (pCharIstd)
            pSI = GetStyle(SVBT16ToShort(pCharIstd));
    }

    if( m_pAktColl )                          // StyleDef -> remember flags
    {
        if (pSI)
        {
            // The style based on has Bit 7 set ?
            if (
                pSI->m_nBase < m_vColl.size() && (*pData & 0x80) &&
                (m_vColl[pSI->m_nBase].m_n81Flags & nMask)
               )
            {
                bOn = !bOn;                     // invert
            }

            if (bOn)
                pSI->m_n81Flags |= nMask;         // set flag
            else
                pSI->m_n81Flags &= ~nMask;        // delete flag
       }
    }
    else
    {

        // in text -> look at flags
        if( *pData & 0x80 )                 // bit 7 set?
        {
            if (pSI && pSI->m_n81Flags & nMask)       // and in StyleDef at ?
                bOn = !bOn;                 // then invert
            // remember on stack that this is a toggle-attribute
            m_pCtrlStck->SetToggleAttr(nI, true);
        }
    }

    SetToggleAttr( nI, bOn );
}

void SwWW8ImplReader::Read_Bidi(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )  //Property end
    {
        m_bBidi = false;
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(),RES_CHRATR_BIDIRTL);
    }
    else    //Property start
    {
        m_bBidi = true;
        sal_uInt8 nBidi = *pData;
        NewAttr( SfxInt16Item( RES_CHRATR_BIDIRTL, (nBidi!=0)? 1 : 0 ) );
    }
}

// Read_BoldUsw for BiDi Italic, Bold
void SwWW8ImplReader::Read_BoldBiDiUsw(sal_uInt16 nId, const sal_uInt8* pData,
    short nLen)
{
    static const sal_uInt16 nEndIds[2] =
    {
        RES_CHRATR_CTL_WEIGHT, RES_CHRATR_CTL_POSTURE,
    };

    sal_uInt8 nI;
    ww::WordVersion eVersion = m_pWwFib->GetFIBVersion();
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
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(),nEndIds[nI]);
        m_pCtrlStck->SetToggleBiDiAttr(nI, false);
        return;
    }
    bool bOn = *pData & 1;
    SwWW8StyInf* pSI = GetStyle(m_nAktColl);
    if (m_pPlcxMan)
    {
        const sal_uInt8 *pCharIstd =
            m_pPlcxMan->GetChpPLCF()->HasSprm(m_bVer67 ? 80 : 0x4A30);
        if (pCharIstd)
            pSI = GetStyle(SVBT16ToShort(pCharIstd));
    }

    if (m_pAktColl && eVersion > ww::eWW2)        // StyleDef -> remember flags
    {
        if (pSI)
        {
            if( pSI->m_nBase < m_vColl.size()             // Style Based on
                && ( *pData & 0x80 )            // bit 7 set?
                && ( m_vColl[pSI->m_nBase].m_n81BiDiFlags & nMask ) ) // base mask?
                    bOn = !bOn;                     // invert

            if( bOn )
                pSI->m_n81BiDiFlags |= nMask;         // set flag
            else
                pSI->m_n81BiDiFlags &= ~nMask;        // delete flag
        }
    }
    else
    {

        // in text -> look at flags
        if (*pData & 0x80)                  // Bit 7 set?
        {
            if (pSI && pSI->m_n81BiDiFlags & nMask) // and in StyleDef at ?
                bOn = !bOn;                     // then invert
            // remember on stack that this is a toggle-attribute
            m_pCtrlStck->SetToggleBiDiAttr(nI, true);
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
            OSL_ENSURE(false, "Unhandled unknown bidi toggle attribute");
            break;

    }
}

void SwWW8ImplReader::SetToggleAttr(sal_uInt8 nAttrId, bool bOn)
{
    ww::WordVersion eVersion = m_pWwFib->GetFIBVersion();

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
            OSL_ENSURE(false, "Unhandled unknown toggle attribute");
            break;
    }
}

void SwWW8ImplReader::_ChkToggleAttr( sal_uInt16 nOldStyle81Mask,
                                        sal_uInt16 nNewStyle81Mask )
{
    sal_uInt16 i = 1, nToggleAttrFlags = m_pCtrlStck->GetToggleAttrFlags();
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
    sal_uInt16 i = 1, nToggleAttrFlags = m_pCtrlStck->GetToggleBiDiAttrFlags();
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
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
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

SwFrameFormat *SwWW8ImplReader::ContainsSingleInlineGraphic(const SwPaM &rRegion)
{
    /*
    For inline graphics and objects word has a hacked in feature to use
    subscripting to force the graphic into a centered position on the line, so
    we must check when applying sub/super to see if it the subscript range
    contains only a single graphic, and if that graphic is anchored as
    FLY_AS_CHAR and then we can change its anchoring to centered in the line.
    */
    SwFrameFormat *pRet=nullptr;
    SwNodeIndex aBegin(rRegion.Start()->nNode);
    const sal_Int32 nBegin(rRegion.Start()->nContent.GetIndex());
    SwNodeIndex aEnd(rRegion.End()->nNode);
    const sal_Int32 nEnd(rRegion.End()->nContent.GetIndex());
    const SwTextNode* pTNd;
    const SwTextAttr* pTFlyAttr;
    if (
         aBegin == aEnd && nBegin == nEnd - 1 &&
         nullptr != (pTNd = aBegin.GetNode().GetTextNode()) &&
         nullptr != (pTFlyAttr = pTNd->GetTextAttrForCharAt(nBegin, RES_TXTATR_FLYCNT))
       )
    {
        const SwFormatFlyCnt& rFly = pTFlyAttr->GetFlyCnt();
        SwFrameFormat *pFlyFormat = rFly.GetFrameFormat();
        if (pFlyFormat &&
            (FLY_AS_CHAR == pFlyFormat->GetAnchor().GetAnchorId()))
        {
            pRet = pFlyFormat;
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
    if (m_pCtrlStck->GetFormatStackAttr(RES_CHRATR_ESCAPEMENT, &nPos))
    {
        SwPaM aRegion(*m_pPaM->GetPoint());

        SwFltPosition aMkPos((*m_pCtrlStck)[nPos].m_aMkPos);
        SwFltPosition aPtPos(*m_pPaM->GetPoint());

        SwFrameFormat *pFlyFormat = nullptr;
        if (
             SwFltStackEntry::MakeRegion(&m_rDoc,aRegion,false,aMkPos,aPtPos) &&
             nullptr != (pFlyFormat = ContainsSingleInlineGraphic(aRegion))
           )
        {
            m_pCtrlStck->DeleteAndDestroy(nPos);
            pFlyFormat->SetFormatAttr(SwFormatVertOrient(0, text::VertOrientation::CHAR_CENTER, text::RelOrientation::CHAR));
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
            m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
        return;
    }

    ww::WordVersion eVersion = m_pWwFib->GetFIBVersion();

    // font position in HalfPoints
    short nPos = eVersion <= ww::eWW2 ? static_cast< sal_Int8 >( *pData ) : SVBT16ToShort( pData );
    sal_Int32 nPos2 = nPos * ( 10 * 100 );      // HalfPoints in 100 * tw
    const SvxFontHeightItem* pF
        = static_cast<const SvxFontHeightItem*>(GetFormatAttr(RES_CHRATR_FONTSIZE));
    OSL_ENSURE(pF, "Expected to have the fontheight available here");

    // #i59022: Check ensure nHeight != 0. Div by zero otherwise.
    sal_Int32 nHeight = 240;
    if (pF != nullptr && pF->GetHeight() != 0)
        nHeight = pF->GetHeight();
    nPos2 /= nHeight;                       // ... now in % (rounded)
    if( nPos2 > 100 )                       // for safety
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
        // Parameter:  0 = none,    1 = single,  2 = by Word,
                    // 3 = double,  4 = dotted,  5 = hidden
                    // 6 = thick,   7 = dash,    8 = dot(not used)
                    // 9 = dotdash 10 = dotdotdash 11 = wave
        switch( *pData )
        {
            case 2: bWordLine = true;       // no break;
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

    // if necessary, mix up stack and exit!
    if( nLen < 0 )
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_UNDERLINE );
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_WORDLINEMODE );
    }
    else
    {
        NewAttr( SvxUnderlineItem( eUnderline, RES_CHRATR_UNDERLINE ));
        if( bWordLine )
            NewAttr(SvxWordLineModeItem(true, RES_CHRATR_WORDLINEMODE));
    }
}

/*
//The last three vary, measurements, rotation ? ?
NoBracket   78 CA 06 -  02 00 00 02 34 52
()          78 CA 06 -  02 01 00 02 34 52
[]          78 CA 06 -  02 02 00 02 34 52
<>          78 CA 06 -  02 03 00 02 34 52
{}          78 CA 06 -  02 04 00 02 34 52
*/
void SwWW8ImplReader::Read_DoubleLine_Rotate( sal_uInt16, const sal_uInt8* pData,
    short nLen )
{
    if( nLen < 0 ) // close the tag
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_TWO_LINES );
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_ROTATE );
    }
    else if( pData && 6 == nLen )
    {
        switch( *pData )
        {
        case 2:                     // double line
            {
                sal_Unicode cStt = 0, cEnd = 0;
                switch( SVBT16ToShort( pData+1 ) )
                {
                case 1: cStt = '('; cEnd = ')'; break;
                case 2: cStt = '['; cEnd = ']'; break;
                case 3: cStt = '<'; cEnd = '>'; break;
                case 4: cStt = '{'; cEnd = '}'; break;
                }
                NewAttr( SvxTwoLinesItem( true, cStt, cEnd, RES_CHRATR_TWO_LINES ));
            }
            break;

        case 1:                         // rotated characters
            {
                bool bFitToLine = 0 != *(pData+1);
                NewAttr( SvxCharRotateItem( 900, bFitToLine, RES_CHRATR_ROTATE ));
            }
            break;
        }
    }
}

void SwWW8ImplReader::Read_TextColor( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    //Has newer colour variant, ignore this old variant
    if (!m_bVer67 && m_pPlcxMan && m_pPlcxMan->GetChpPLCF()->HasSprm(NS_sprm::LN_CCv))
        return;

    if( nLen < 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_COLOR );
    else
    {
        sal_uInt8 b = *pData;            // parameter: 0 = Auto, 1..16 colors

        if( b > 16 )                // unknown -> Black
            b = 0;

        NewAttr( SvxColorItem(Color(GetCol(b)), RES_CHRATR_COLOR));
        if (m_pAktColl && m_pStyles)
            m_pStyles->bTextColChanged = true;
    }
}

void SwWW8ImplReader::Read_TextForeColor(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_COLOR );
    else
    {
        Color aColor(msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)));
        NewAttr(SvxColorItem(aColor, RES_CHRATR_COLOR));
        if (m_pAktColl && m_pStyles)
            m_pStyles->bTextColChanged = true;
    }
}

void SwWW8ImplReader::Read_UnderlineColor(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
    {
        //because the UnderlineColor is not a standalone attribute in SW, it belongs to the underline attribute.
        //And, the .doc file stores attributes separately, this attribute ends here, the "underline"
        //attribute also terminates (if the character next owns underline, that will be a new underline attribute).
        //so nothing is left to be done here.
        return;
    }
    else
    {
        if ( m_pAktColl ) //importing style
        {
            if( SfxItemState::SET == m_pAktColl->GetItemState( RES_CHRATR_UNDERLINE, false ) )
            {
                const SwAttrSet& aSet = m_pAktColl->GetAttrSet();
                SvxUnderlineItem *pUnderline
                    = static_cast<SvxUnderlineItem *>(aSet.Get( RES_CHRATR_UNDERLINE, false ).Clone());
                if(pUnderline){
                    pUnderline->SetColor( Color( msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)) ) );
                    m_pAktColl->SetFormatAttr( *pUnderline );
                    delete pUnderline;
                }
            }
        }
        else if ( m_pAktItemSet )
        {
            if ( SfxItemState::SET == m_pAktItemSet->GetItemState( RES_CHRATR_UNDERLINE, false ) )
            {
                SvxUnderlineItem *pUnderline
                    = static_cast<SvxUnderlineItem *>(m_pAktItemSet->Get( RES_CHRATR_UNDERLINE, false ) .Clone());
                if(pUnderline){
                    pUnderline->SetColor( Color( msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)) ) );
                    m_pAktItemSet->Put( *pUnderline );
                    delete pUnderline;
                }
            }
        }
        else
        {
            SvxUnderlineItem* pUnderlineAttr = const_cast<SvxUnderlineItem*>(static_cast<const SvxUnderlineItem*>(m_pCtrlStck->GetOpenStackAttr( *m_pPaM->GetPoint(), RES_CHRATR_UNDERLINE )));
            if( pUnderlineAttr != nullptr )
                pUnderlineAttr->SetColor( Color( msfilter::util::BGRToRGB(SVBT32ToUInt32( pData ))));
        }
    }
}
bool SwWW8ImplReader::GetFontParams( sal_uInt16 nFCode, FontFamily& reFamily,
    OUString& rName, FontPitch& rePitch, rtl_TextEncoding& reCharSet )
{
    // the definitions that are the base for these tables are in windows.h
    static const FontPitch ePitchA[] =
    {
        PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, PITCH_DONTKNOW
    };

    static const FontFamily eFamilyA[] =
    {
        FAMILY_DONTKNOW, FAMILY_ROMAN, FAMILY_SWISS, FAMILY_MODERN,
        FAMILY_SCRIPT, FAMILY_DECORATIVE, FAMILY_DONTKNOW, FAMILY_DONTKNOW
    };

    const WW8_FFN* pF = m_pFonts->GetFont( nFCode );  // Info for it
    if( !pF )                                   // font number unknown ?
        return false;                           // then ignore

    rName = pF->sFontname;

    // pF->prg : Pitch
    rePitch = ePitchA[pF->prg];

    // pF->chs: Charset
    if( 77 == pF->chs )             // Mac font in Mac Charset or
        reCharSet = m_eTextCharSet;   // translated to ANSI charset
    else
    {
        // #i52786#, for word 67 we'll assume that ANSI is basically invalid,
        // might be true for (above) mac as well, but would need a mac example
        // that exercises this to be sure
        if (m_bVer67 && pF->chs == 0)
            reCharSet = RTL_TEXTENCODING_DONTKNOW;
        else
            reCharSet = rtl_getTextEncodingFromWindowsCharset( pF->chs );
    }

    // Make sure charset is not set in case of Symbol, otherwise in case a
    // character is missing in our OpenSymbol, the character won't be replaced.
    if (reCharSet == RTL_TEXTENCODING_SYMBOL && rName == "Symbol")
        reCharSet = RTL_TEXTENCODING_DONTKNOW;

    // make sure Font Family Code is set correctly
    // at least for the most important fonts
    // ( might be set wrong when Doc was not created by
    //   Winword but by third party program like Applixware... )
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
        //If we fail (and are not doing a style) then put something into the
        //character encodings stack anyway so that the property end that pops
        //off the stack will keep in sync
        if (!m_pAktColl && IsListOrDropcap())
        {
            if (nWhich == RES_CHRATR_CJK_FONT)
            {
                if (!m_aFontSrcCJKCharSets.empty())
                {
                    eSrcCharSet = m_aFontSrcCJKCharSets.top();
                }
                else
                {
                    eSrcCharSet = RTL_TEXTENCODING_DONTKNOW;
                }

                m_aFontSrcCJKCharSets.push(eSrcCharSet);
            }
            else
            {
                if (!m_aFontSrcCharSets.empty())
                {
                    eSrcCharSet = m_aFontSrcCharSets.top();
                }
                else
                {
                    eSrcCharSet = RTL_TEXTENCODING_DONTKNOW;
                }

                m_aFontSrcCharSets.push(eSrcCharSet);
            }
        }
        return false;
    }

    rtl_TextEncoding eDstCharSet = eSrcCharSet;

    SvxFontItem aFont( eFamily, aName, OUString(), ePitch, eDstCharSet, nWhich);

    if( bSetEnums )
    {
        if( m_pAktColl && m_nAktColl < m_vColl.size() ) // StyleDef
        {
            switch(nWhich)
            {
                default:
                case RES_CHRATR_FONT:
                    m_vColl[m_nAktColl].m_eLTRFontSrcCharSet = eSrcCharSet;
                    break;
                case RES_CHRATR_CTL_FONT:
                    m_vColl[m_nAktColl].m_eRTLFontSrcCharSet = eSrcCharSet;
                    break;
                case RES_CHRATR_CJK_FONT:
                    m_vColl[m_nAktColl].m_eCJKFontSrcCharSet = eSrcCharSet;
                    break;
            }
        }
        else if (IsListOrDropcap())
        {
            //Add character text encoding to stack
            if (nWhich  == RES_CHRATR_CJK_FONT)
                m_aFontSrcCJKCharSets.push(eSrcCharSet);
            else
                m_aFontSrcCharSets.push(eSrcCharSet);
        }
    }

    NewAttr( aFont );                       // ...and insert

    return true;
}

void SwWW8ImplReader::ResetCharSetVars()
{
    OSL_ENSURE(!m_aFontSrcCharSets.empty(),"no charset to remove");
    if (!m_aFontSrcCharSets.empty())
        m_aFontSrcCharSets.pop();
}

void SwWW8ImplReader::ResetCJKCharSetVars()
{
    OSL_ENSURE(!m_aFontSrcCJKCharSets.empty(),"no charset to remove");
    if (!m_aFontSrcCJKCharSets.empty())
        m_aFontSrcCJKCharSets.pop();
}

void SwWW8ImplReader::openFont(sal_uInt16 nFCode, sal_uInt16 nId)
{
    if (SetNewFontAttr(nFCode, true, nId) && m_pAktColl && m_pStyles)
    {
        // remember for simulating default font
        if (RES_CHRATR_CJK_FONT == nId)
            m_pStyles->bCJKFontChanged = true;
        else if (RES_CHRATR_CTL_FONT == nId)
            m_pStyles->bCTLFontChanged = true;
        else
            m_pStyles->bFontChanged = true;
    }
}

void SwWW8ImplReader::closeFont(sal_uInt16 nId)
{
    m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), nId );
    if (nId == RES_CHRATR_CJK_FONT)
        ResetCJKCharSetVars();
    else
        ResetCharSetVars();
}

/*
    Turn font on or off:
*/
void SwWW8ImplReader::Read_FontCode( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    if (!m_bSymbol)           // if bSymbol, the symbol's font
    {                       // (see sprmCSymbol) is valid!
        switch( nId )
        {
            case 113:       //WW7
            case 0x4A51:    //"Other" font, override with BiDi if it exists
            case 0x4A5E:    //BiDi Font
                nId = RES_CHRATR_CTL_FONT;
                break;
            case 93:        //WW6
            case 111:       //WW7
            case 0x4A4f:
                nId = RES_CHRATR_FONT;
                break;
            case 112:       //WW7
            case 0x4A50:
                nId = RES_CHRATR_CJK_FONT;
                break;
            default:
                return ;
        }

        ww::WordVersion eVersion = m_pWwFib->GetFIBVersion();

        if( nLen < 0 ) // end of attribute
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
            sal_uInt16 nFCode = SVBT16ToShort( pData );     // font number
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
        case 85:  //WW2
        case 116: //WW7
        case 0x4a61:
            nId = RES_CHRATR_CTL_FONTSIZE;
            break;
        default:
            return ;
    }

    ww::WordVersion eVersion = m_pWwFib->GetFIBVersion();

    if( nLen < 0 )          // end of attribute
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), nId  );
        if (eVersion <= ww::eWW6) // reset additionally the CTL size
            m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_CTL_FONTSIZE );
        if (RES_CHRATR_FONTSIZE == nId)  // reset additionally the CJK size
            m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_CJK_FONTSIZE );
    }
    else
    {
        // Font-Size in half points e.g. 10 = 1440 / ( 72 * 2 )
        sal_uInt16 nFSize = eVersion <= ww::eWW2 ? *pData : SVBT16ToShort(pData);
        nFSize*= 10;

        SvxFontHeightItem aSz( nFSize, 100, nId );
        NewAttr( aSz );
        if (RES_CHRATR_FONTSIZE == nId)  // set additionally the CJK size
        {
            aSz.SetWhich( RES_CHRATR_CJK_FONTSIZE );
            NewAttr( aSz );
        }
        if (eVersion <= ww::eWW6) // set additionally the CTL size
        {
            aSz.SetWhich( RES_CHRATR_CTL_FONTSIZE );
            NewAttr( aSz );
        }
        if (m_pAktColl && m_pStyles)            // Style-Def ?
        {
            // remember for simulating default font size
            if (nId == RES_CHRATR_CTL_FONTSIZE)
                m_pStyles->bFCTLSizeChanged = true;
            else
            {
                m_pStyles->bFSizeChanged = true;
                if (eVersion <= ww::eWW6)
                    m_pStyles->bFCTLSizeChanged= true;
            }
        }
    }
}

void SwWW8ImplReader::Read_CharSet(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )
    {                   // end of attribute
        m_eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
        return;
    }
    sal_uInt8 nfChsDiff = *pData;

    if( nfChsDiff )
        m_eHardCharSet = rtl_getTextEncodingFromWindowsCharset( *(pData + 1) );
    else
        m_eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
}

void SwWW8ImplReader::Read_Language( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    switch( nId )
    {
        case 97:
        case 0x486D: //sprmCRgLid0_80
        case 0x4873: //Methinks, uncertain
            nId = RES_CHRATR_LANGUAGE;
            break;
        case 0x486E: //sprmCRgLid1_80
        case 0x4874: //Methinks, uncertain
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

    if( nLen < 0 )                  // end of attribute
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), nId );
    else
    {
        sal_uInt16 nLang = SVBT16ToShort( pData );  // Language-Id
        NewAttr(SvxLanguageItem((const LanguageType)nLang, nId));
    }
}

/*
    Turn on character style:
*/
void SwWW8ImplReader::Read_CColl( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 ){                 // end of attribute
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_TXTATR_CHARFMT );
        m_nCharFormat = -1;
        return;
    }
    sal_uInt16 nId = SVBT16ToShort( pData );    // Style-Id (NOT Sprm-Id!)

    if( nId >= m_vColl.size() || !m_vColl[nId].m_pFormat  // invalid Id?
        || m_vColl[nId].m_bColl )              // or paragraph style?
        return;                             // then ignore

    // if current on loading a TOX field, and current trying to apply a hyperlink character style,
    // just ignore. For the hyperlinks inside TOX in MS Word is not same with a common hyperlink
    // Character styles: without underline and blue font color. And such type style will be applied in others
    // processes.
    if (m_bLoadingTOXCache && m_vColl[nId].GetWWStyleId() == ww::stiHyperlink)
    {
        return;
    }

    NewAttr( SwFormatCharFormat( static_cast<SwCharFormat*>(m_vColl[nId].m_pFormat) ) );
    m_nCharFormat = (short) nId;
}

/*
    Narrower or wider than normal:
*/
void SwWW8ImplReader::Read_Kern( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 ){                 // end of attribute
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_KERNING );
        return;
    }
    sal_Int16 nKern = SVBT16ToShort( pData );    // Kerning in Twips
    NewAttr( SvxKerningItem( nKern, RES_CHRATR_KERNING ) );
}

void SwWW8ImplReader::Read_FontKern( sal_uInt16, const sal_uInt8* , short nLen )
{
    if( nLen < 0 ) // end of attribute
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_AUTOKERN );
    else
        NewAttr(SvxAutoKernItem(true, RES_CHRATR_AUTOKERN));
}

void SwWW8ImplReader::Read_CharShadow(  sal_uInt16, const sal_uInt8* pData, short nLen )
{
    //Has newer colour variant, ignore this old variant
    if (!m_bVer67 && m_pPlcxMan && m_pPlcxMan->GetChpPLCF()->HasSprm(0xCA71))
        return;

    if( nLen <= 0 )
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
    }
    else
    {
        WW8_SHD aSHD;
        aSHD.SetWWValue( *reinterpret_cast<SVBT16 const *>(pData) );
        SwWW8Shade aSh( m_bVer67, aSHD );

        NewAttr( SvxBrushItem( aSh.aColor, RES_CHRATR_BACKGROUND ));

        // Add a marker to the grabbag indicating that character background was imported from MSO shading
        SfxGrabBagItem aGrabBag = *(static_cast<const SfxGrabBagItem*>(GetFormatAttr(RES_CHRATR_GRABBAG)));
        std::map<OUString, css::uno::Any>& rMap = aGrabBag.GetGrabBag();
        rMap.insert(std::pair<OUString, css::uno::Any>("CharShadingMarker",uno::makeAny(true)));
        NewAttr(aGrabBag);
    }
}

void SwWW8ImplReader::Read_TextBackColor(sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
    }
    else
    {
        OSL_ENSURE(nLen == 10, "Len of para back colour not 10!");
        if (nLen != 10)
            return;
        Color aColour(ExtractColour(pData, m_bVer67));
        NewAttr(SvxBrushItem(aColour, RES_CHRATR_BACKGROUND));

        // Add a marker to the grabbag indicating that character background was imported from MSO shading
        SfxGrabBagItem aGrabBag = *(static_cast<const SfxGrabBagItem*>(GetFormatAttr(RES_CHRATR_GRABBAG)));
        std::map<OUString, css::uno::Any>& rMap = aGrabBag.GetGrabBag();
        rMap.insert(std::pair<OUString, css::uno::Any>("CharShadingMarker",uno::makeAny(true)));
        NewAttr(aGrabBag);
    }
}

void SwWW8ImplReader::Read_CharHighlight(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if( nLen <= 0 )
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_HIGHLIGHT );
    }
    else
    {
        sal_uInt8 b = *pData;            // Parameter: 0 = Auto, 1..16 colors

        if( b > 16 )                // invalid -> Black
            b = 0;                  // Auto -> Black

        Color aCol(GetCol(b));
        NewAttr( SvxBrushItem( aCol , RES_CHRATR_HIGHLIGHT ));
    }
}

void SwWW8ImplReader::Read_NoLineNumb(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if( nLen < 0 )  // end of attribute
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_LINENUMBER );
        return;
    }
    SwFormatLineNumber aLN;
    if (const SwFormatLineNumber* pLN
        = static_cast<const SwFormatLineNumber*>(GetFormatAttr(RES_LINENUMBER)))
    {
        aLN.SetStartValue( pLN->GetStartValue() );
    }

    aLN.SetCountLines( pData && (0 == *pData) );
    NewAttr( aLN );
}

bool lcl_HasExplicitLeft(const WW8PLCFMan *pPlcxMan, bool bVer67)
{
    WW8PLCFx_Cp_FKP *pPap = pPlcxMan ? pPlcxMan->GetPapPLCF() : nullptr;
    if (pPap)
    {
        if (bVer67)
            return pPap->HasSprm(17);
        else
            return (pPap->HasSprm(0x840F) || pPap->HasSprm(0x845E));
    }
    return false;
}
// Sprm 16, 17
void SwWW8ImplReader::Read_LR( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    if (nLen < 0)  // end of attribute
    {
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_LR_SPACE);
        return;
    }

    short nPara = SVBT16ToShort( pData );

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    const SfxPoolItem* pLR = GetFormatAttr(RES_LR_SPACE);
    if( pLR )
        aLR = *static_cast<const SvxLRSpaceItem*>(pLR);

    // Fix the regression issue: #i99822#: Discussion?
    // Since the list level formatting doesn't apply into paragraph style
    // for list levels of mode LABEL_ALIGNMENT.(see ww8par3.cxx
    // W8ImplReader::RegisterNumFormatOnTextNode).
    // Need to apply the list format to the paragraph here.
    SwTextNode* pTextNode = m_pPaM->GetNode().GetTextNode();
    if( pTextNode && pTextNode->AreListLevelIndentsApplicable() )
    {
        SwNumRule * pNumRule = pTextNode->GetNumRule();
        if( pNumRule )
        {
            sal_uInt8 nLvl = static_cast< sal_uInt8 >(pTextNode->GetActualListLevel());
            const SwNumFormat* pFormat = pNumRule->GetNumFormat( nLvl );
            if ( pFormat && pFormat->GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aLR.SetTextLeft( pFormat->GetIndentAt() );
                aLR.SetTextFirstLineOfst( static_cast<short>(pFormat->GetFirstLineIndent()) );
                // make paragraph have hard-set indent attributes
                pTextNode->SetAttr( aLR );
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
            //Left becomes after;
            case 17:
                nId = 16;
                break;
            case 0x840F:
                nId = 0x840E;
                break;
            //Right becomes before;
            case 16:
                nId = 17;
                break;
            case 0x840E:
                nId = 0x840F;
                break;
        }
    }

    bool bFirstLinOfstSet( false ); // #i103711#
    bool bLeftIndentSet( false ); // #i105414#

    switch (nId)
    {
        //sprmPDxaLeft
        case     17:
        case 0x840F:
        case 0x845E:
            aLR.SetTextLeft( nPara );
            if (m_pAktColl && m_nAktColl < m_vColl.size())
            {
                m_vColl[m_nAktColl].m_bListReleventIndentSet = true;
            }
            bLeftIndentSet = true;  // #i105414#
            break;
        //sprmPDxaLeft1
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
            if (m_pPlcxMan && m_nAktColl < m_vColl.size() && m_vColl[m_nAktColl].m_bHasBrokenWW6List)
            {
                const sal_uInt8 *pIsZeroed = m_pPlcxMan->GetPapPLCF()->HasSprm(0x460B);
                if (pIsZeroed && *pIsZeroed == 0)
                {
                    const SvxLRSpaceItem &rLR =
                        ItemGet<SvxLRSpaceItem>(*(m_vColl[m_nAktColl].m_pFormat),
                        RES_LR_SPACE);
                    nPara = nPara - rLR.GetTextFirstLineOfst();
                }
            }

            aLR.SetTextFirstLineOfst(nPara);

            if (!m_pAktColl)
            {
                if (const SwTextNode* pNode = m_pPaM->GetNode().GetTextNode())
                {
                    if ( const SwNumFormat *pNumFormat = GetNumFormatFromTextNode(*pNode) )
                    {
                        if (!lcl_HasExplicitLeft(m_pPlcxMan, m_bVer67))
                        {
                            aLR.SetTextLeft(pNumFormat->GetIndentAt());

                            // If have not explicit left, set number format list tab position is doc default tab
                            const SvxTabStopItem *pDefaultStopItem = static_cast<const SvxTabStopItem *>(m_rDoc.GetAttrPool().GetPoolDefaultItem(RES_PARATR_TABSTOP));
                            if ( pDefaultStopItem &&  pDefaultStopItem->Count() > 0 )
                                const_cast<SwNumFormat*>(pNumFormat)->SetListtabPos( ((SvxTabStop&)(*pDefaultStopItem)[0]).GetTabPos() );
                        }
                    }
                }
            }
            if (m_pAktColl && m_nAktColl < m_vColl.size())
            {
                m_vColl[m_nAktColl].m_bListReleventIndentSet = true;
            }
            bFirstLinOfstSet = true; // #i103711#
            break;
        //sprmPDxaRight
        case     16:
        case 0x840E:
        case 0x845D:
            aLR.SetRight( nPara );
            break;
        default:
            return;
    }

    NewAttr( aLR, bFirstLinOfstSet, bLeftIndentSet ); // #i103711#, #i105414#
}

// Sprm 20
void SwWW8ImplReader::Read_LineSpace( sal_uInt16, const sal_uInt8* pData, short nLen )
{
// commment see Read_UL()
    if (m_bStyNormal && m_bWWBugNormal)
        return;

    if( nLen < 0 ){
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_LINESPACING );
        if( !( m_nIniFlags & WW8FL_NO_IMPLPASP ) )
            m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }

    short nSpace = SVBT16ToShort( pData );
    ww::WordVersion eVersion = m_pWwFib->GetFIBVersion();
    short nMulti = (eVersion <= ww::eWW2) ? 1 : SVBT16ToShort( pData + 2 );

    SvxLineSpace eLnSpc;
    if( 0 > nSpace )
    {
        nSpace = -nSpace;
        eLnSpc = SVX_LINE_SPACE_FIX;
    }
    else
        eLnSpc = SVX_LINE_SPACE_MIN;

    // WW has implicit additional paragraph spacing depending on
    // the line spacing. It is, for "exactly", 0.8 * line spacing "before"
    // and 0.2 * line spacing "after".
    // For "at least", it is 1 * line spacing "before" and 0 * line spacing "after".
    // For "multiple", it is 0 "before" and min(0cm, FontSize*(nFach-1)) "after".

    // SW also has implicit line spacing. It is, for "at least"
    // 1 * line spacing "before" and 0 "after".
    // For proportional, it is min(0cm, FontSize*(nFach-1)) both "before" and "after".

    sal_uInt16 nSpaceTw = 0;

    SvxLineSpacingItem aLSpc( LINE_SPACE_DEFAULT_HEIGHT, RES_PARATR_LINESPACING );

    if( 1 == nMulti )               // MultilineSpace ( proportional )
    {
        long n = nSpace * 10 / 24;  // WW: 240 = 100%, SW: 100 = 100%

        // as discussed with AMA, the limit is nonsensical
        if( n>200 ) n = 200;        // SW_UI maximum
        aLSpc.SetPropLineSpace( (const sal_uInt8)n );
        const SvxFontHeightItem* pH = static_cast<const SvxFontHeightItem*>(
            GetFormatAttr( RES_CHRATR_FONTSIZE ));
        nSpaceTw = (sal_uInt16)( n * pH->GetHeight() / 100 );
    }
    else                            // Fixed / Minimum
    {
        // for negative space, the distance is "exact", otherwise "at least"
        nSpaceTw = (sal_uInt16)nSpace;
        aLSpc.SetLineHeight( nSpaceTw );
        aLSpc.GetLineSpaceRule() = eLnSpc;
    }
    NewAttr( aLSpc );
    if( m_pSFlyPara )
        m_pSFlyPara->nLineSpace = nSpaceTw;   // linespace for graphics APOs
}

//#i18519# AutoSpace value depends on Dop fDontUseHTMLAutoSpacing setting
sal_uInt16 SwWW8ImplReader::GetParagraphAutoSpace(bool fDontUseHTMLAutoSpacing)
{
    if (fDontUseHTMLAutoSpacing)
        return 100;  //Seems to be always 5points in this case
    else
        return 280;  //Seems to be always 14points in this case
}

void SwWW8ImplReader::Read_ParaAutoBefore(sal_uInt16, const sal_uInt8 *pData, short nLen)
{
    if (nLen < 0)
    {
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_UL_SPACE);
        return;
    }

    if (*pData)
    {
        SvxULSpaceItem aUL(*static_cast<const SvxULSpaceItem*>(GetFormatAttr(RES_UL_SPACE)));
        aUL.SetUpper(GetParagraphAutoSpace(m_pWDop->fDontUseHTMLAutoSpacing));
        NewAttr(aUL);
        if (m_pAktColl && m_nAktColl < m_vColl.size())
            m_vColl[m_nAktColl].m_bParaAutoBefore = true;
        else
            m_bParaAutoBefore = true;
    }
    else
    {
        if (m_pAktColl && m_nAktColl < m_vColl.size())
            m_vColl[m_nAktColl].m_bParaAutoBefore = false;
        else
            m_bParaAutoBefore = false;
    }
}

void SwWW8ImplReader::Read_ParaAutoAfter(sal_uInt16, const sal_uInt8 *pData, short nLen)
{
    if (nLen < 0)
    {
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_UL_SPACE);
        return;
    }

    if (*pData)
    {
        SvxULSpaceItem aUL(*static_cast<const SvxULSpaceItem*>(GetFormatAttr(RES_UL_SPACE)));
        aUL.SetLower(GetParagraphAutoSpace(m_pWDop->fDontUseHTMLAutoSpacing));
        NewAttr(aUL);
        if (m_pAktColl && m_nAktColl < m_vColl.size())
            m_vColl[m_nAktColl].m_bParaAutoAfter = true;
        else
            m_bParaAutoAfter = true;
    }
    else
    {
        if (m_pAktColl && m_nAktColl < m_vColl.size())
            m_vColl[m_nAktColl].m_bParaAutoAfter = false;
        else
            m_bParaAutoAfter = false;
    }
}

// Sprm 21, 22
void SwWW8ImplReader::Read_UL( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    // A workaround for an error in WW: For nProduct == 0c03d, usually
    // DyaAfter 240 (delta y distance after, comment of the translator)
    // is incorrectly inserted into style "Normal", even though it isn't there.
    // Using the ini flag  WW8FL_NO_STY_DYA you can force this behavior for other
    // WW versions as well.
    // OSL_ENSURE( !bStyNormal || bWWBugNormal, "+This Document may point to a bug
    // in the WW version used for creating it. If the Styles <Standard> resp.
    // <Normal> differentiate between WW and SW in paragraph or line spacing,
    // then please send this Document to SH.");
    // bWWBugNormal is not a sufficient criterion for this distance being wrong.

    if( nLen < 0 )
    {
        // end of attribute
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    short nPara = SVBT16ToShort( pData );
    if( nPara < 0 )
        nPara = -nPara;

    SvxULSpaceItem aUL( *static_cast<const SvxULSpaceItem*>(GetFormatAttr( RES_UL_SPACE )));

    switch( nId )
    {
        //sprmPDyaBefore
        case     21:
        case 0xA413:
            aUL.SetUpper( nPara );
            break;
        //sprmPDyaAfter
        case     22:
        case 0xA414:
            aUL.SetLower( nPara );
            break;
        default:
            return;
    }

    NewAttr( aUL );
}

void SwWW8ImplReader::Read_ParaContextualSpacing( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    SvxULSpaceItem aUL( *static_cast<const SvxULSpaceItem*>(GetFormatAttr( RES_UL_SPACE )));
    aUL.SetContextValue(*pData);
    NewAttr( aUL );
}

void SwWW8ImplReader::Read_IdctHint( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    // sprmcidcthint (opcode 0x286f) specifies a script bias for the text in the run.
    // for unicode characters that are shared between far east and non-far east scripts,
    // this property determines what font and language the character will use.
    // when this value is 0, text properties bias towards non-far east properties.
    // when this value is 1, text properties bias towards far east properties.
    if( nLen < 0 )  //Property end
    {
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(),RES_CHRATR_IDCTHINT);
    }
    else    //Property start
    {
        sal_uInt8 nVal = *pData;
        NewAttr( SfxInt16Item( RES_CHRATR_IDCTHINT, (nVal!=0)? 1 : 0 ) );
    }
}

void SwWW8ImplReader::Read_Justify( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_ADJUST );
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
        m_pPlcxMan ? m_pPlcxMan->GetPapPLCF()->HasSprm(0x2441) : nullptr;
    if (pDir)
        bRTL = *pDir != 0;
    else
    {
        const SvxFrameDirectionItem* pItem=
            static_cast<const SvxFrameDirectionItem*>(GetFormatAttr(RES_FRAMEDIR));
        if (pItem && (pItem->GetValue() == FRMDIR_HORI_RIGHT_TOP))
            bRTL = true;
    }
    return bRTL;
}

void SwWW8ImplReader::Read_RTLJustify( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_ADJUST );
        return;
    }

    //If we are in a ltr paragraph this is the same as normal Justify,
    //If we are in a rtl paragraph the meaning is reversed.
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
            OSL_ENSURE( false, "wrong Id" );
            return ;
    }

    if( nLen < 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), nId );
    else
    {
        SfxBoolItem* pI = static_cast<SfxBoolItem*>(GetDfltAttr( nId )->Clone());
        pI->SetValue( 0 != *pData );
        NewAttr( *pI );
        delete pI;
    }
}

void SwWW8ImplReader::Read_Emphasis( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_EMPHASIS_MARK );
    else
    {
        LanguageType nLang;
        //Check to see if there is an up and coming cjk language property. If
        //there is use it, if there is not fall back to the currently set one.
        //Only the cjk language setting seems to matter to word, the western
        //one is ignored
        const sal_uInt8 *pLang =
            m_pPlcxMan ? m_pPlcxMan->GetChpPLCF()->HasSprm(0x486E) : nullptr;

        if (pLang)
            nLang = SVBT16ToShort( pLang );
        else
        {
            nLang = static_cast<const SvxLanguageItem *>(
                GetFormatAttr(RES_CHRATR_CJK_LANGUAGE))->GetLanguage();
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
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_SCALEW );
    else
    {
        sal_uInt16 nVal = SVBT16ToShort( pData );
        //The number must be between 1 and 600
        if (nVal < 1 || nVal > 600)
            nVal = 100;
        NewAttr( SvxCharScaleWidthItem( nVal, RES_CHRATR_SCALEW ) );
    }
}

void SwWW8ImplReader::Read_Relief( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    if( nLen < 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_RELIEF );
    else
    {
        if( *pData )
        {
// not so easy because this is also a toggle attribute!
//  2 x emboss on -> no emboss !!!
// the actual value must be searched over the stack / template

            const SvxCharReliefItem* pOld = static_cast<const SvxCharReliefItem*>(
                                            GetFormatAttr( RES_CHRATR_RELIEF ));
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

void SwWW8ImplReader::Read_TextAnim(sal_uInt16 /*nId*/, const sal_uInt8* pData, short nLen)
{
    if (nLen < 0)
        m_pCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_CHRATR_BLINK);
    else
    {
        if (*pData)
        {
            bool bBlink;

            // The 7 animated text effects available in word all get
            // mapped to a blinking text effect in LibreOffice
            // 0 no animation       1 Las Vegas lights
            // 2 background blink   3 sparkle text
            // 4 marching ants      5 marching red ants
            // 6 shimmer
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
        // Clear-Brush
           0,   // 0    clear
        // Solid-Brush
        1000,   // 1    solid
        // Percent values
          50,   // 2    pct5
         100,   // 3    pct10
         200,   // 4    pct20
         250,   // 5    pct25
         300,   // 6    pct30
         400,   // 7    pct40
         500,   // 8    pct50
         600,   // 9    pct60
         700,   // 10   pct70
         750,   // 11   pct75
         800,   // 12   pct80
         900,   // 13   pct90
        // Special cases
         333,   // 14   Dark Horizontal
         333,   // 15   Dark Vertical
         333,   // 16   Dark Forward Diagonal
         333,   // 17   Dark Backward Diagonal
         333,   // 18   Dark Cross
         333,   // 19   Dark Diagonal Cross
         333,   // 20   Horizontal
         333,   // 21   Vertical
         333,   // 22   Forward Diagonal
         333,   // 23   Backward Diagonal
         333,   // 24   Cross
         333,   // 25   Diagonal Cross
        // Undefined values in DOC spec-sheet
         500,   // 26
         500,   // 27
         500,   // 28
         500,   // 29
         500,   // 30
         500,   // 31
         500,   // 32
         500,   // 33
         500,   // 34
        // Different shading types
          25,   // 35   [available in DOC, not available in DOCX]
          75,   // 36   [available in DOC, not available in DOCX]
         125,   // 37   pct12
         150,   // 38   pct15
         175,   // 39   [available in DOC, not available in DOCX]
         225,   // 40   [available in DOC, not available in DOCX]
         275,   // 41   [available in DOC, not available in DOCX]
         325,   // 42   [available in DOC, not available in DOCX]
         350,   // 43   pct35
         375,   // 44   pct37
         425,   // 45   [available in DOC, not available in DOCX]
         450,   // 46   pct45
         475,   // 47   [available in DOC, not available in DOCX]
         525,   // 48   [available in DOC, not available in DOCX]
         550,   // 49   pct55
         575,   // 50   [available in DOC, not available in DOCX]
         625,   // 51   pct62
         650,   // 52   pct65
         675,   // 53   [available in DOC, not available in DOCX]
         725,   // 54   [available in DOC, not available in DOCX]
         775,   // 55   [available in DOC, not available in DOCX]
         825,   // 56   [available in DOC, not available in DOCX]
         850,   // 57   pct85
         875,   // 58   pct87
         925,   // 59   [available in DOC, not available in DOCX]
         950,   // 60   pct95
         975    // 61   [available in DOC, not available in DOCX]
    };// 62

    //NO auto for shading so Foreground: Auto = Black
    if (nFore == COL_AUTO)
        nFore = COL_BLACK;

    //NO auto for shading so background: Auto = Weiss
    ColorData nUseBack = nBack;
    if (nUseBack == COL_AUTO)
        nUseBack = COL_WHITE;

    if( nIndex >= sizeof( eMSGrayScale ) / sizeof ( eMSGrayScale[ 0 ] ) )
        nIndex = 0;

    sal_uLong nWW8BrushStyle = eMSGrayScale[nIndex];

    switch (nWW8BrushStyle)
    {
        case 0: // Null-Brush
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
    if (!m_bVer67 && m_pPlcxMan && m_pPlcxMan->GetPapPLCF()->HasSprm(0xC64D))
        return;

    if (nLen <= 0)
    {
        // end of attribute
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_BACKGROUND );
    }
    else
    {
        WW8_SHD aSHD;
        aSHD.SetWWValue( *reinterpret_cast<SVBT16 const *>(pData) );
        SwWW8Shade aSh( m_bVer67, aSHD );

        NewAttr(SvxBrushItem(aSh.aColor, RES_BACKGROUND));
    }
}

void SwWW8ImplReader::Read_ParaBackColor(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if (nLen <= 0)
    {
        // end of attribute
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_BACKGROUND );
    }
    else
    {
        OSL_ENSURE(nLen == 10, "Len of para back colour not 10!");
        if (nLen != 10)
            return;
        NewAttr(SvxBrushItem(Color(ExtractColour(pData, m_bVer67)), RES_BACKGROUND));
    }
}

sal_uInt32 SwWW8ImplReader::ExtractColour(const sal_uInt8* &rpData, bool bVer67)
{
    (void) bVer67; // unused in non-debug
    OSL_ENSURE(!bVer67, "Impossible");
    sal_uInt32 nFore = msfilter::util::BGRToRGB(SVBT32ToUInt32(rpData));
    rpData+=4;
    sal_uInt32 nBack = msfilter::util::BGRToRGB(SVBT32ToUInt32(rpData));
    rpData+=4;
    sal_uInt16 nIndex = SVBT16ToShort(rpData);
    rpData+=2;
    //Being a transparent background colour doesn't actually show the page
    //background through, it merely acts like white
    if (nBack == 0xFF000000)
        nBack = COL_AUTO;
    OSL_ENSURE(nBack == COL_AUTO || !(nBack & 0xFF000000),
        "ww8: don't know what to do with such a transparent bg colour, report");
    SwWW8Shade aShade(nFore, nBack, nIndex);
    return aShade.aColor.GetColor();
}

void SwWW8ImplReader::Read_TextVerticalAdjustment( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen > 0 )
    {
        drawing::TextVerticalAdjust nVA = drawing::TextVerticalAdjust_TOP;
        switch( *pData )
        {
            case 1:
                nVA = drawing::TextVerticalAdjust_CENTER;
                break;
            case 2: //justify
                nVA = drawing::TextVerticalAdjust_BLOCK;
                break;
            case 3:
                nVA = drawing::TextVerticalAdjust_BOTTOM;
                break;
            default:
                break;
        }
        m_aSectionManager.SetCurrentSectionVerticalAdjustment( nVA );
    }
}

void SwWW8ImplReader::Read_Border(sal_uInt16 , const sal_uInt8*, short nLen)
{
    if( nLen < 0 )
    {
        if( m_bHasBorder )
        {
            m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_BOX );
            m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_SHADOW );
            m_bHasBorder = false;
        }
    }
    else if( !m_bHasBorder )
    {
        // the borders on all four sides are bundled. That
        // simplifies the administration, i.e., the box does not have
        // to be put on and removed from CtrlStack 4 times.
        m_bHasBorder = true;

        WW8_BRCVer9_5 aBrcs;   // Top, Left, Bottom, Right, Between
        sal_uInt8 nBorder;

        if( m_pAktColl )
            nBorder = ::lcl_ReadBorders(m_bVer67, aBrcs, nullptr, m_pStyles);
        else
            nBorder = ::lcl_ReadBorders(m_bVer67, aBrcs, m_pPlcxMan ? m_pPlcxMan->GetPapPLCF() : nullptr);

        if( nBorder )                                   // Border
        {
            bool bIsB = IsBorder(aBrcs, true);
            if (!InLocalApo() || !bIsB ||
                (m_pWFlyPara && !m_pWFlyPara->bBorderLines ))
            {
                // Do not turn *on* borders in APO, since otherwise
                // I get the Fly border twice;
                // but only when it is set on in the Fly, skip it;
                // otherwise there is none at all!

                // even if no border is set, the attribute has to be set,
                // otherwise it's not possible to turn of the style attribute hard.
                const SvxBoxItem* pBox
                    = static_cast<const SvxBoxItem*>(GetFormatAttr( RES_BOX ));
                SvxBoxItem aBox(RES_BOX);
                if (pBox)
                    aBox = *pBox;
                short aSizeArray[5]={0};

                SetBorder(aBox, aBrcs, &aSizeArray[0], nBorder);

                Rectangle aInnerDist;
                GetBorderDistance( aBrcs, aInnerDist );

                if (nBorder & (1 << WW8_LEFT))
                    aBox.SetDistance( (sal_uInt16)aInnerDist.Left(), SvxBoxItemLine::LEFT );

                if (nBorder & (1 << WW8_TOP))
                    aBox.SetDistance( (sal_uInt16)aInnerDist.Top(), SvxBoxItemLine::TOP );

                if (nBorder & (1 << WW8_RIGHT))
                    aBox.SetDistance( (sal_uInt16)aInnerDist.Right(), SvxBoxItemLine::RIGHT );

                if (nBorder & (1 << WW8_BOT))
                    aBox.SetDistance( (sal_uInt16)aInnerDist.Bottom(), SvxBoxItemLine::BOTTOM );

                NewAttr( aBox );

                SvxShadowItem aS(RES_SHADOW);
                if( SetShadow( aS, &aSizeArray[0], aBrcs[WW8_RIGHT] ) )
                    NewAttr( aS );
            }
        }
    }
}

void SwWW8ImplReader::Read_CharBorder(sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    //Ignore this old border type
    //if (!bVer67 && pPlcxMan && pPlcxMan->GetChpPLCF()->HasSprm(0xCA72))
    //    return;

    if( nLen < 0 )
    {
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_BOX );
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_SHADOW );
    }
    else
    {
        const SvxBoxItem* pBox
            = static_cast<const SvxBoxItem*>(GetFormatAttr( RES_CHRATR_BOX ));
        if( pBox )
        {
            SvxBoxItem aBoxItem(RES_CHRATR_BOX);
            aBoxItem = *pBox;
            WW8_BRCVer9 aBrc;
            int nBrcVer = (nId == NS_sprm::LN_CBrc) ? 9 : (m_bVer67 ? 6 : 8);

            _SetWW8_BRC(nBrcVer, aBrc, pData);

            // Border style is none -> no border, no shadow
            if( editeng::ConvertBorderStyleFromWord(aBrc.brcType()) != table::BorderLineStyle::NONE )
            {
                Set1Border(aBoxItem, aBrc, SvxBoxItemLine::TOP, 0, nullptr, true);
                Set1Border(aBoxItem, aBrc, SvxBoxItemLine::BOTTOM, 0, nullptr, true);
                Set1Border(aBoxItem, aBrc, SvxBoxItemLine::LEFT, 0, nullptr, true);
                Set1Border(aBoxItem, aBrc, SvxBoxItemLine::RIGHT, 0, nullptr, true);
                NewAttr( aBoxItem );

                short aSizeArray[WW8_RIGHT+1]={0}; aSizeArray[WW8_RIGHT] = 1;
                SvxShadowItem aShadowItem(RES_CHRATR_SHADOW);
                if( SetShadow( aShadowItem, &aSizeArray[0], aBrc ) )
                    NewAttr( aShadowItem );
            }
        }
    }

}

void SwWW8ImplReader::Read_Hyphenation( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    // set Hyphenation flag
    if( nLen <= 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_HYPHENZONE );
    else
    {
        SvxHyphenZoneItem aAttr(
            *static_cast<const SvxHyphenZoneItem*>(GetFormatAttr( RES_PARATR_HYPHENZONE ) ));

        aAttr.SetHyphen( 0 == *pData ); // sic !

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
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_WIDOWS );
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_ORPHANS );
    }
    else
    {
        sal_uInt8 nL = ( *pData & 1 ) ? 2 : 0;

        NewAttr( SvxWidowsItem( nL, RES_PARATR_WIDOWS ) );     // Aus -> nLines = 0
        NewAttr( SvxOrphansItem( nL, RES_PARATR_ORPHANS ) );

        if( m_pAktColl && m_pStyles )           // Style-Def ?
            m_pStyles->bWidowsChanged = true; // merken zur Simulation
                                            // Default-Widows
    }
}

void SwWW8ImplReader::Read_UsePgsuSettings(sal_uInt16,const sal_uInt8* pData,short nLen)
{
    if( nLen <= 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_SNAPTOGRID);
    else
    {
        if(m_nInTable)
            NewAttr( SvxParaGridItem(false, RES_PARATR_SNAPTOGRID) );
        else
            NewAttr( SvxParaGridItem(*pData, RES_PARATR_SNAPTOGRID) );
    }
}

void SwWW8ImplReader::Read_AlignFont( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_VERTALIGN);
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
                OSL_ENSURE(false,"Unknown paragraph vertical align");
                break;
        }
        NewAttr( SvxParaVertAlignItem( nVal, RES_PARATR_VERTALIGN ) );
    }
}

void SwWW8ImplReader::Read_KeepLines( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_SPLIT );
    else
        NewAttr( SvxFormatSplitItem( ( *pData & 1 ) == 0, RES_PARATR_SPLIT ) );
}

void SwWW8ImplReader::Read_KeepParas( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_KEEP );
    else
        NewAttr( SvxFormatKeepItem( ( *pData & 1 ) != 0 , RES_KEEP) );
}

void SwWW8ImplReader::Read_BreakBefore( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( nLen <= 0 )
        m_pCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_BREAK );
    else
        NewAttr( SvxFormatBreakItem(
                ( *pData & 1 ) ? SVX_BREAK_PAGE_BEFORE : SVX_BREAK_NONE, RES_BREAK ) );
}

void SwWW8ImplReader::Read_ApoPPC( sal_uInt16, const sal_uInt8* pData, short )
{
    if (m_pAktColl && m_nAktColl < m_vColl.size()) // only for Styledef, sonst anders geloest
    {
        SwWW8StyInf& rSI = m_vColl[m_nAktColl];
        WW8FlyPara* pFly = rSI.m_pWWFly ? rSI.m_pWWFly : new WW8FlyPara(m_bVer67);
        m_vColl[m_nAktColl].m_pWWFly = pFly;
        pFly->Read(*pData, m_pStyles);
        if (pFly->IsEmpty())
        {
            delete m_vColl[m_nAktColl].m_pWWFly;
            m_vColl[m_nAktColl].m_pWWFly = nullptr;
        }
    }
}

bool SwWW8ImplReader::ParseTabPos(WW8_TablePos *pTabPos, WW8PLCFx_Cp_FKP* pPap)
{
    bool bRet = false;
    const sal_uInt8 *pRes=nullptr;
    memset(pTabPos, 0, sizeof(WW8_TablePos));
    if (nullptr != (pRes = pPap->HasSprm(0x360D)))
    {
        pTabPos->nSp29 = *pRes;
        pTabPos->nSp37 = 2;     //Possible fail area, always parallel wrap
        if (nullptr != (pRes = pPap->HasSprm(0x940E)))
            pTabPos->nSp26 = SVBT16ToShort(pRes);
        if (nullptr != (pRes = pPap->HasSprm(0x940F)))
            pTabPos->nSp27 = SVBT16ToShort(pRes);
        if (nullptr != (pRes = pPap->HasSprm(0x9410)))
            pTabPos->nLeMgn = SVBT16ToShort(pRes);
        if (nullptr != (pRes = pPap->HasSprm(0x941E)))
            pTabPos->nRiMgn = SVBT16ToShort(pRes);
        if (nullptr != (pRes = pPap->HasSprm(0x9411)))
            pTabPos->nUpMgn = SVBT16ToShort(pRes);
        if (nullptr != (pRes = pPap->HasSprm(0x941F)))
            pTabPos->nLoMgn = SVBT16ToShort(pRes);
        bRet = true;
    }
    if (nullptr != (pRes = pPap->HasSprm(NS_sprm::LN_TDefTable)))
    {
        WW8TabBandDesc aDesc;
        aDesc.ReadDef(false, pRes);
        int nTableWidth = aDesc.nCenter[aDesc.nWwCols] - aDesc.nCenter[0];
        int nTextAreaWidth = m_aSectionManager.GetTextAreaWidth();
        // If the table is wider than the text area, then don't create a fly
        // for the table: no wrapping will be performed anyway, but multi-page
        // tables will be broken.
        // If there are columns, do create a fly, as the flow of the columns
        // would otherwise restrict the table.
        pTabPos->bNoFly = nTableWidth >= nTextAreaWidth  && m_aSectionManager.CurrentSectionColCount() < 2;
    }
    return bRet;
}

// Seiten - Attribute werden nicht mehr als Attribute gehandhabt
//  ( ausser OLST )
long SwWW8ImplReader::ImportExtSprm(WW8PLCFManResult* pRes)
{
    // Arrays zum Lesen der erweiterten ( selbstdefinierten ) SPRMs
    typedef long (SwWW8ImplReader:: *FNReadRecordExt)(WW8PLCFManResult*);

    static const FNReadRecordExt aWwSprmTab[] =
    {
        /* 0 (256) */   &SwWW8ImplReader::Read_Footnote,     // FootNote
        /* 1 (257) */   &SwWW8ImplReader::Read_Footnote,     // EndNote
        /* 2 (258) */   &SwWW8ImplReader::Read_Field,  // Feld
        /* 3 (259) */   &SwWW8ImplReader::Read_Book,   // Bookmark
        /* 4 (260) */   &SwWW8ImplReader::Read_And,     // Annotation
        /* 5 (261) */   &SwWW8ImplReader::Read_AtnBook, // Annotationmark
        /* 6 (262) */   &SwWW8ImplReader::Read_FactoidBook // Smart tag bookmark
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
        /* 0 (256) */   &SwWW8ImplReader::End_Footnote,      // FootNote
        /* 1 (257) */   &SwWW8ImplReader::End_Footnote,      // EndNote
        /* 2 (258) */   &SwWW8ImplReader::End_Field,  // Feld
        /* 3 (259) */   nullptr,   // Bookmark
        /* 4 (260) */   nullptr     // Annotation
    };

    sal_uInt8 nIdx = static_cast< sal_uInt8 >(nSprmId - eFTN);
    if( nIdx < SAL_N_ELEMENTS(aWwSprmTab)
        && aWwSprmTab[nIdx] )
        (this->*aWwSprmTab[nIdx])();
}

// Arrays zum Lesen der SPRMs

// Funktion zum Einlesen von Sprms. Par1: SprmId
typedef void (SwWW8ImplReader:: *FNReadRecord)( sal_uInt16, const sal_uInt8*, short );

struct SprmReadInfo
{
    sal_uInt16       nId;
    FNReadRecord pReadFnc;
};

bool operator<(const SprmReadInfo &rFirst, const SprmReadInfo &rSecond)
{
    return (rFirst.nId < rSecond.nId);
}

typedef ww::SortedArray<SprmReadInfo> wwSprmDispatcher;

const wwSprmDispatcher *GetWW2SprmDispatcher()
{
    static SprmReadInfo aSprms[] =
    {
          {0, nullptr},                                    // "0" Default bzw. Error
                                                     //wird uebersprungen! ,
          {2, &SwWW8ImplReader::Read_StyleCode},     //"sprmPIstd",  pap.istd
                                                     //(style code)
          {3, nullptr},                                    //"sprmPIstdPermute", pap.istd
                                                     //permutation
          {4, nullptr},                                    //"sprmPIncLv1",
                                                     //pap.istddifference
          {5, &SwWW8ImplReader::Read_Justify},       //"sprmPJc", pap.jc
                                                     //(justification)
          {6, nullptr},                                    //"sprmPFSideBySide",
                                                     //pap.fSideBySide
          {7, &SwWW8ImplReader::Read_KeepLines},     //"sprmPFKeep", pap.fKeep
          {8, &SwWW8ImplReader::Read_KeepParas},     //"sprmPFKeepFollow ",
                                                     //pap.fKeepFollow
          {9, &SwWW8ImplReader::Read_BreakBefore},   //"sprmPPageBreakBefore",
                                                     //pap.fPageBreakBefore
         {10, nullptr},                                    //"sprmPBrcl", pap.brcl
         {11, nullptr},                                    //"sprmPBrcp ", pap.brcp
         {12, &SwWW8ImplReader::Read_ANLevelDesc},   //"sprmPAnld", pap.anld (ANLD
                                                     //structure)
         {13, &SwWW8ImplReader::Read_ANLevelNo},     //"sprmPNLvlAnm", pap.nLvlAnm
                                                     //nn
         {14, &SwWW8ImplReader::Read_NoLineNumb},    //"sprmPFNoLineNumb", ap.fNoLnn
         {15, &SwWW8ImplReader::Read_Tab},           //"?sprmPChgTabsPapx",
                                                     //pap.itbdMac, ...
         {16, &SwWW8ImplReader::Read_LR},            //"sprmPDxaRight", pap.dxaRight
         {17, &SwWW8ImplReader::Read_LR},            //"sprmPDxaLeft", pap.dxaLeft
         {18, nullptr},                                    //"sprmPNest", pap.dxaLeft
         {19, &SwWW8ImplReader::Read_LR},            //"sprmPDxaLeft1", pap.dxaLeft1
         {20, &SwWW8ImplReader::Read_LineSpace},     //"sprmPDyaLine", pap.lspd
                                                     //an LSPD
         {21, &SwWW8ImplReader::Read_UL},            //"sprmPDyaBefore",
                                                     //pap.dyaBefore
         {22, &SwWW8ImplReader::Read_UL},            //"sprmPDyaAfter", pap.dyaAfter
         {23, nullptr},                                    //"?sprmPChgTabs", pap.itbdMac,
                                                     //pap.rgdxaTab, ...
         {24, nullptr},                                    //"sprmPFInTable", pap.fInTable
         {25, &SwWW8ImplReader::Read_TabRowEnd},     //"sprmPTtp", pap.fTtp
         {26, nullptr},                                    //"sprmPDxaAbs", pap.dxaAbs
         {27, nullptr},                                    //"sprmPDyaAbs", pap.dyaAbs
         {28, nullptr},                                    //"sprmPDxaWidth", pap.dxaWidth
         {29, &SwWW8ImplReader::Read_ApoPPC},        //"sprmPPc", pap.pcHorz,
                                                     //pap.pcVert
         {30, nullptr},                                    //"sprmPBrcTop10", pap.brcTop
                                                     //BRC10
         {31, nullptr},                                    //"sprmPBrcLeft10",
                                                     //pap.brcLeft BRC10
         {32, nullptr},                                    //"sprmPBrcBottom10",
                                                     //pap.brcBottom BRC10
         {33, nullptr},                                    //"sprmPBrcRight10",
                                                     //pap.brcRight BRC10
         {34, nullptr},                                    //"sprmPBrcBetween10",
                                                     //pap.brcBetween BRC10
         {35, nullptr},                                    //"sprmPBrcBar10", pap.brcBar
                                                     //BRC10
         {36, nullptr},                                    //"sprmPFromText10",
                                                     //pap.dxaFromText dxa
         {37, nullptr},                                    //"sprmPWr", pap.wr wr
         {38, &SwWW8ImplReader::Read_Border},        //"sprmPBrcTop", pap.brcTop BRC
         {39, &SwWW8ImplReader::Read_Border},        //"sprmPBrcLeft",
                                                     //pap.brcLeft BRC
         {40, &SwWW8ImplReader::Read_Border},        //"sprmPBrcBottom",
                                                     //pap.brcBottom BRC
         {41, &SwWW8ImplReader::Read_Border},        //"sprmPBrcRight",
                                                     //pap.brcRight BRC
         {42, &SwWW8ImplReader::Read_Border},        //"sprmPBrcBetween",
                                                     //pap.brcBetween BRC
         {43, nullptr},                                    //"sprmPBrcBar", pap.brcBar
                                                     //BRC word
         {44, &SwWW8ImplReader::Read_Hyphenation},   //"sprmPFNoAutoHyph",
                                                     //pap.fNoAutoHyph
         {45, nullptr},                                    //"sprmPWHeightAbs",
                                                     //pap.wHeightAbs w
         {46, nullptr},                                    //"sprmPDcs", pap.dcs DCS
         {47, &SwWW8ImplReader::Read_Shade},         //"sprmPShd", pap.shd SHD
         {48, nullptr},                                    //"sprmPDyaFromText",
                                                     //pap.dyaFromText dya
         {49, nullptr},                                    //"sprmPDxaFromText",
                                                     //pap.dxaFromText dxa
         {50, nullptr},                                    //"sprmPFLocked", pap.fLocked
                                                     //0 or 1 byte
         {51, &SwWW8ImplReader::Read_WidowControl},  //"sprmPFWidowControl",
                                                     //pap.fWidowControl 0 or 1 byte
         {52, nullptr},                                    //"?sprmPRuler 52",
         {53, nullptr},                                    //"??53",
         {54, nullptr},                                    //"??54",
         {55, nullptr},                                    //"??55",
         {56, nullptr},                                    //"??56",
         {57, nullptr},                                    //"??57",
         {58, nullptr},                                    //"??58",
         {59, nullptr},                                    //"??59",

         {60, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFBold", chp.fBold 0,1,
                                                     //128, or 129 byte
         {61, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFItalic", chp.fItalic
                                                     //0,1, 128, or 129 byte
         {62, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFStrike", chp.fStrike
                                                     //0,1, 128, or 129 byte
         {63, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFOutline", chp.fOutline
                                                     //0,1, 128, or 129 byte
         {64, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFShadow", chp.fShadow
                                                     //0,1, 128, or 129 byte
         {65, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFSmallCaps",
                                                     //chp.fSmallCaps 0,1, 128, or
                                                     //129 byte
         {66, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFCaps", chp.fCaps 0,1,
                                                     //128, or 129 byte
         {67, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFVanish", chp.fVanish
                                                     //0,1, 128, or 129 byte
         {68, &SwWW8ImplReader::Read_FontCode},      //"sprmCFtc", chp.ftc ftc word
         {69, &SwWW8ImplReader::Read_Underline},     // "sprmCKul", chp.kul kul byte
         {70, nullptr},                                    //"sprmCSizePos", chp.hps,
                                                     //chp.hpsPos 3 bytes
         {71, &SwWW8ImplReader::Read_Kern},          //"sprmCDxaSpace",
                                                     //chp.dxaSpace dxa word
         {72, &SwWW8ImplReader::Read_Language},      //"sprmCLid", chp.lid LID word
         {73, &SwWW8ImplReader::Read_TextColor},      //"sprmCIco", chp.ico ico byte
         {74, &SwWW8ImplReader::Read_FontSize},      //"sprmCHps", chp.hps hps word!
         {75, nullptr},                                    //"sprmCHpsInc", chp.hps byte
         {76, &SwWW8ImplReader::Read_SubSuperProp},  //"sprmCHpsPos", chp.hpsPos
                                                     //hps byte
         {77, nullptr},                                    //"sprmCHpsPosAdj", chp.hpsPos
                                                     //hps byte
         {78, &SwWW8ImplReader::Read_Majority},      //"?sprmCMajority", chp.fBold,
                                                     //chp.fItalic, chp.fSmallCaps
         {80, &SwWW8ImplReader::Read_BoldBiDiUsw},   //sprmCFBoldBi
         {81, &SwWW8ImplReader::Read_BoldBiDiUsw},   //sprmCFItalicBi
         {82, &SwWW8ImplReader::Read_FontCode},      //sprmCFtcBi
         {83, &SwWW8ImplReader::Read_Language},      //sprmClidBi
         {84, &SwWW8ImplReader::Read_TextColor},      //sprmCIcoBi
         {85, &SwWW8ImplReader::Read_FontSize},      //sprmCHpsBi
         {86, nullptr},                                    //sprmCFBiDi
         {87, nullptr},                                    //sprmCFDiacColor
         {94, nullptr},                                    //"sprmPicBrcl", pic.brcl brcl
                                                     //(see PIC structure
                                                     //definition) byte
         {95, nullptr},                                    //"sprmPicScale", pic.mx,
                                                     //pic.my, pic.dxaCropleft,
         {96, nullptr},                                    //"sprmPicBrcTop", pic.brcTop
                                                     //BRC word
         {97, nullptr},                                    //"sprmPicBrcLeft",
                                                     //pic.brcLeft BRC word
         {98, nullptr},                                    //"sprmPicBrcBottom",
                                                     //pic.brcBottom BRC word
         {99, nullptr}                                     //"sprmPicBrcRight",
    };

    static wwSprmDispatcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
}

const wwSprmDispatcher *GetWW6SprmDispatcher()
{
    static SprmReadInfo aSprms[] =
    {
          {0, nullptr},                                    // "0" Default bzw. Error
                                                     //wird uebersprungen! ,
          {2, &SwWW8ImplReader::Read_StyleCode},     //"sprmPIstd",  pap.istd
                                                     //(style code)
          {3, nullptr},                                    //"sprmPIstdPermute", pap.istd
                                                     //permutation
          {4, nullptr},                                    //"sprmPIncLv1",
                                                     //pap.istddifference
          {5, &SwWW8ImplReader::Read_Justify},       //"sprmPJc", pap.jc
                                                     //(justification)
          {6, nullptr},                                    //"sprmPFSideBySide",
                                                     //pap.fSideBySide
          {7, &SwWW8ImplReader::Read_KeepLines},     //"sprmPFKeep", pap.fKeep
          {8, &SwWW8ImplReader::Read_KeepParas},     //"sprmPFKeepFollow ",
                                                     //pap.fKeepFollow
          {9, &SwWW8ImplReader::Read_BreakBefore},   //"sprmPPageBreakBefore",
                                                     //pap.fPageBreakBefore
         {10, nullptr},                                    //"sprmPBrcl", pap.brcl
         {11, nullptr},                                    //"sprmPBrcp ", pap.brcp
         {12, &SwWW8ImplReader::Read_ANLevelDesc},   //"sprmPAnld", pap.anld (ANLD
                                                     //structure)
         {13, &SwWW8ImplReader::Read_ANLevelNo},     //"sprmPNLvlAnm", pap.nLvlAnm
                                                     //nn
         {14, &SwWW8ImplReader::Read_NoLineNumb},    //"sprmPFNoLineNumb", ap.fNoLnn
         {15, &SwWW8ImplReader::Read_Tab},           //"?sprmPChgTabsPapx",
                                                     //pap.itbdMac, ...
         {16, &SwWW8ImplReader::Read_LR},            //"sprmPDxaRight", pap.dxaRight
         {17, &SwWW8ImplReader::Read_LR},            //"sprmPDxaLeft", pap.dxaLeft
         {18, nullptr},                                    //"sprmPNest", pap.dxaLeft
         {19, &SwWW8ImplReader::Read_LR},            //"sprmPDxaLeft1", pap.dxaLeft1
         {20, &SwWW8ImplReader::Read_LineSpace},     //"sprmPDyaLine", pap.lspd
                                                     //an LSPD
         {21, &SwWW8ImplReader::Read_UL},            //"sprmPDyaBefore",
                                                     //pap.dyaBefore
         {22, &SwWW8ImplReader::Read_UL},            //"sprmPDyaAfter", pap.dyaAfter
         {23, nullptr},                                    //"?sprmPChgTabs", pap.itbdMac,
                                                     //pap.rgdxaTab, ...
         {24, nullptr},                                    //"sprmPFInTable", pap.fInTable
         {25, &SwWW8ImplReader::Read_TabRowEnd},     //"sprmPTtp", pap.fTtp
         {26, nullptr},                                    //"sprmPDxaAbs", pap.dxaAbs
         {27, nullptr},                                    //"sprmPDyaAbs", pap.dyaAbs
         {28, nullptr},                                    //"sprmPDxaWidth", pap.dxaWidth
         {29, &SwWW8ImplReader::Read_ApoPPC},        //"sprmPPc", pap.pcHorz,
                                                     //pap.pcVert
         {30, nullptr},                                    //"sprmPBrcTop10", pap.brcTop
                                                     //BRC10
         {31, nullptr},                                    //"sprmPBrcLeft10",
                                                     //pap.brcLeft BRC10
         {32, nullptr},                                    //"sprmPBrcBottom10",
                                                     //pap.brcBottom BRC10
         {33, nullptr},                                    //"sprmPBrcRight10",
                                                     //pap.brcRight BRC10
         {34, nullptr},                                    //"sprmPBrcBetween10",
                                                     //pap.brcBetween BRC10
         {35, nullptr},                                    //"sprmPBrcBar10", pap.brcBar
                                                     //BRC10
         {36, nullptr},                                    //"sprmPFromText10",
                                                     //pap.dxaFromText dxa
         {37, nullptr},                                    //"sprmPWr", pap.wr wr
         {38, &SwWW8ImplReader::Read_Border},        //"sprmPBrcTop", pap.brcTop BRC
         {39, &SwWW8ImplReader::Read_Border},        //"sprmPBrcLeft",
                                                     //pap.brcLeft BRC
         {40, &SwWW8ImplReader::Read_Border},        //"sprmPBrcBottom",
                                                     //pap.brcBottom BRC
         {41, &SwWW8ImplReader::Read_Border},        //"sprmPBrcRight",
                                                     //pap.brcRight BRC
         {42, &SwWW8ImplReader::Read_Border},        //"sprmPBrcBetween",
                                                     //pap.brcBetween BRC
         {43, nullptr},                                    //"sprmPBrcBar", pap.brcBar
                                                     //BRC word
         {44, &SwWW8ImplReader::Read_Hyphenation},   //"sprmPFNoAutoHyph",
                                                     //pap.fNoAutoHyph
         {45, nullptr},                                    //"sprmPWHeightAbs",
                                                     //pap.wHeightAbs w
         {46, nullptr},                                    //"sprmPDcs", pap.dcs DCS
         {47, &SwWW8ImplReader::Read_Shade},         //"sprmPShd", pap.shd SHD
         {48, nullptr},                                    //"sprmPDyaFromText",
                                                     //pap.dyaFromText dya
         {49, nullptr},                                    //"sprmPDxaFromText",
                                                     //pap.dxaFromText dxa
         {50, nullptr},                                    //"sprmPFLocked", pap.fLocked
                                                     //0 or 1 byte
         {51, &SwWW8ImplReader::Read_WidowControl},  //"sprmPFWidowControl",
                                                     //pap.fWidowControl 0 or 1 byte
         {52, nullptr},                                    //"?sprmPRuler 52",
         {53, nullptr},                                    //"??53",
         {54, nullptr},                                    //"??54",
         {55, nullptr},                                    //"??55",
         {56, nullptr},                                    //"??56",
         {57, nullptr},                                    //"??57",
         {58, nullptr},                                    //"??58",
         {59, nullptr},                                    //"??59",
         {60, nullptr},                                    //"??60",
         {61, nullptr},                                    //"??61",
         {62, nullptr},                                    //"??62",
         {63, nullptr},                                    //"??63",
         {64, &SwWW8ImplReader::Read_ParaBiDi},      //"rtl bidi ?
         {65, &SwWW8ImplReader::Read_CFRMarkDel},    //"sprmCFStrikeRM",
                                                     //chp.fRMarkDel 1 or 0 bit
         {66, &SwWW8ImplReader::Read_CFRMark},       //"sprmCFRMark", chp.fRMark
                                                     //1 or 0 bit
         {67, &SwWW8ImplReader::Read_FieldVanish},     //"sprmCFFieldVanish",
                                                     //chp.fFieldVanish 1 or 0 bit
         {68, &SwWW8ImplReader::Read_PicLoc},        //"sprmCPicLocation",
                                                     //chp.fcPic and chp.fSpec
         {69, nullptr},                                    //"sprmCIbstRMark",
                                                     //chp.ibstRMark index into
                                                     //sttbRMark
         {70, nullptr},                                    //"sprmCDttmRMark", chp.dttm
                                                     //DTTM long
         {71, nullptr},                                    //"sprmCFData", chp.fData 1 or
                                                     //0 bit
         {72, nullptr},                                    //"sprmCRMReason",
                                                     //chp.idslRMReason an index to
                                                     //a table
         {73, &SwWW8ImplReader::Read_CharSet},       //"sprmCChse", chp.fChsDiff
                                                     //and chp.chse 3 bytes
         {74, &SwWW8ImplReader::Read_Symbol},        //"sprmCSymbol", chp.fSpec,
                                                     //chp.chSym and chp.ftcSym
         {75, &SwWW8ImplReader::Read_Obj},           //"sprmCFOle2", chp.fOle2 1
                                                     //or 0 bit
         {76, nullptr},                                    //"??76",
         {77, nullptr},                                    //"??77",
         {78, nullptr},                                    //"??78",
         {79, nullptr},                                    //"??79",
         {80, &SwWW8ImplReader::Read_CColl},         //"sprmCIstd", chp.istd istd,
                                                     //see stylesheet definition
                                                     //short
         {81, nullptr},                                    //"sprmCIstdPermute", chp.istd
                                                     //permutation vector
         {82, nullptr},                                    //"sprmCDefault", whole CHP
                                                     //none variable length
         {83, nullptr},                                    //"sprmCPlain", whole CHP
                                                     //none 0
         {84, nullptr},                                    //"??84",
         {85, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFBold", chp.fBold 0,1,
                                                     //128, or 129 byte
         {86, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFItalic", chp.fItalic
                                                     //0,1, 128, or 129 byte
         {87, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFStrike", chp.fStrike
                                                     //0,1, 128, or 129 byte
         {88, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFOutline", chp.fOutline
                                                     //0,1, 128, or 129 byte
         {89, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFShadow", chp.fShadow
                                                     //0,1, 128, or 129 byte
         {90, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFSmallCaps",
                                                     //chp.fSmallCaps 0,1, 128, or
                                                     //129 byte
         {91, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFCaps", chp.fCaps 0,1,
                                                     //128, or 129 byte
         {92, &SwWW8ImplReader::Read_BoldUsw},       //"sprmCFVanish", chp.fVanish
                                                     //0,1, 128, or 129 byte
         {93, &SwWW8ImplReader::Read_FontCode},      //"sprmCFtc", chp.ftc ftc word
         {94, &SwWW8ImplReader::Read_Underline},     // "sprmCKul", chp.kul kul byte
         {95, nullptr},                                    //"sprmCSizePos", chp.hps,
                                                     //chp.hpsPos 3 bytes
         {96, &SwWW8ImplReader::Read_Kern},          //"sprmCDxaSpace",
                                                     //chp.dxaSpace dxa word
         {97, &SwWW8ImplReader::Read_Language},      //"sprmCLid", chp.lid LID word
         {98, &SwWW8ImplReader::Read_TextColor},      //"sprmCIco", chp.ico ico byte
         {99, &SwWW8ImplReader::Read_FontSize},      //"sprmCHps", chp.hps hps word!
        {100, nullptr},                                    //"sprmCHpsInc", chp.hps byte
        {101, &SwWW8ImplReader::Read_SubSuperProp},  //"sprmCHpsPos", chp.hpsPos
                                                     //hps byte
        {102, nullptr},                                    //"sprmCHpsPosAdj", chp.hpsPos
                                                     //hps byte
        {103, &SwWW8ImplReader::Read_Majority},      //"?sprmCMajority", chp.fBold,
                                                     //chp.fItalic, chp.fSmallCaps
        {104, &SwWW8ImplReader::Read_SubSuper},      //"sprmCIss", chp.iss iss byte
        {105, nullptr},                                    //"sprmCHpsNew50", chp.hps hps
                                                     //variable width, length
                                                     //always recorded as 2
        {106, nullptr},                                    //"sprmCHpsInc1", chp.hps
                                                     //complex variable width,
                                                     //length always recorded as 2
        {107, &SwWW8ImplReader::Read_FontKern},      //"sprmCHpsKern", chp.hpsKern
                                                     //hps short
        {108, &SwWW8ImplReader::Read_Majority},      //"sprmCMajority50", chp.fBold,
                                                     //chp.fItalic, chp.fSmallCaps,
                                                     // chp.fVanish, ...
        {109, nullptr},                                    //"sprmCHpsMul", chp.hps
                                                     //percentage to grow hps short
        {110, nullptr},                                    //"sprmCCondHyhen", chp.ysri
                                                     //ysri short
        {111, &SwWW8ImplReader::Read_FontCode},      //ww7 font
        {112, &SwWW8ImplReader::Read_FontCode},      //ww7 CJK font
        {113, &SwWW8ImplReader::Read_FontCode},      //ww7 rtl font
        {114, &SwWW8ImplReader::Read_Language},      //ww7 lid
        {115, &SwWW8ImplReader::Read_TextColor},      //ww7 rtl colour ?
        {116, &SwWW8ImplReader::Read_FontSize},
        {117, &SwWW8ImplReader::Read_Special},       //"sprmCFSpec", chp.fSpec 1
                                                     //or 0 bit
        {118, &SwWW8ImplReader::Read_Obj},           //"sprmCFObj", chp.fObj 1 or 0
                                                     //bit
        {119, nullptr},                                    //"sprmPicBrcl", pic.brcl brcl
                                                     //(see PIC structure
                                                     //definition) byte
        {120, nullptr},                                    //"sprmPicScale", pic.mx,
                                                     //pic.my, pic.dxaCropleft,
        {121, nullptr},                                    //"sprmPicBrcTop", pic.brcTop
                                                     //BRC word
        {122, nullptr},                                    //"sprmPicBrcLeft",
                                                     //pic.brcLeft BRC word
        {123, nullptr},                                    //"sprmPicBrcBottom",
                                                     //pic.brcBottom BRC word
        {124, nullptr},                                    //"sprmPicBrcRight",
                                                     //pic.brcRight BRC word
        {125, nullptr},                                    //"??125",
        {126, nullptr},                                    //"??126",
        {127, nullptr},                                    //"??127",
        {128, nullptr},                                    //"??128",
        {129, nullptr},                                    //"??129",
        {130, nullptr},                                    //"??130",
        {131, nullptr},                                    //"sprmSScnsPgn", sep.cnsPgn
                                                     //cns byte
        {132, nullptr},                                    //"sprmSiHeadingPgn",
                                                     //sep.iHeadingPgn heading
                                                     //number level byte
        {133, &SwWW8ImplReader::Read_OLST},          //"sprmSOlstAnm", sep.olstAnm
                                                     //OLST variable length
        {134, nullptr},                                    //"??135",
        {135, nullptr},                                    //"??135",
        {136, nullptr},                                    //"sprmSDxaColWidth",
                                                     //sep.rgdxaColWidthSpacing
                                                     //complex 3 bytes
        {137, nullptr},                                    //"sprmSDxaColSpacing",
                                                     //sep.rgdxaColWidthSpacing
                                                     //complex 3 bytes
        {138, nullptr},                                    //"sprmSFEvenlySpaced",
                                                     //sep.fEvenlySpaced 1 or 0 byte
        {139, nullptr},                                    //"sprmSFProtected",
                                                     //sep.fUnlocked 1 or 0 byte
        {140, nullptr},                                    //"sprmSDmBinFirst",
                                                     //sep.dmBinFirst  word
        {141, nullptr},                                    //"sprmSDmBinOther",
                                                     //sep.dmBinOther  word
        {142, nullptr},                                    //"sprmSBkc", sep.bkc bkc
                                                     //byte BreakCode
        {143, nullptr},                                    //"sprmSFTitlePage",
                                                     //sep.fTitlePage 0 or 1 byte
        {144, nullptr},                                    //"sprmSCcolumns", sep.ccolM1
                                                     //# of cols - 1 word
        {145, nullptr},                                    //"sprmSDxaColumns",
                                                     //sep.dxaColumns dxa word
        {146, nullptr},                                    //"sprmSFAutoPgn",
                                                     //sep.fAutoPgn obsolete byte
        {147, nullptr},                                    //"sprmSNfcPgn", sep.nfcPgn
                                                     //nfc byte
        {148, nullptr},                                    //"sprmSDyaPgn", sep.dyaPgn
                                                     //dya short
        {149, nullptr},                                    //"sprmSDxaPgn", sep.dxaPgn
                                                     //dya short
        {150, nullptr},                                    //"sprmSFPgnRestart",
                                                     //sep.fPgnRestart 0 or 1 byte
        {151, nullptr},                                    //"sprmSFEndnote", sep.fEndnote
                                                     //0 or 1 byte
        {152, nullptr},                                    //"sprmSLnc", sep.lnc lnc byte
        {153, nullptr},                                    //"sprmSGprfIhdt", sep.grpfIhdt
                                                     //grpfihdt byte
        {154, nullptr},                                    //"sprmSNLnnMod", sep.nLnnMod
                                                     //non-neg int. word
        {155, nullptr},                                    //"sprmSDxaLnn", sep.dxaLnn
                                                     //dxa word
        {156, nullptr},                                    //"sprmSDyaHdrTop",
                                                     //sep.dyaHdrTop dya word
        {157, nullptr},                                    //"sprmSDyaHdrBottom",
                                                     //sep.dyaHdrBottom dya word
        {158, nullptr},                                    //"sprmSLBetween",
                                                     //sep.fLBetween 0 or 1 byte
        {159, nullptr},                                    //"sprmSVjc", sep.vjc vjc byte
        {160, nullptr},                                    //"sprmSLnnMin", sep.lnnMin
                                                     //lnn word
        {161, nullptr},                                    //"sprmSPgnStart", sep.pgnStart
                                                     //pgn word
        {162, nullptr},                                    //"sprmSBOrientation",
                                                     //sep.dmOrientPage dm byte
        {163, nullptr},                                    //"?SprmSBCustomize 163", ?
        {164, nullptr},                                    //"sprmSXaPage", sep.xaPage xa
                                                     //word
        {165, nullptr},                                    //"sprmSYaPage", sep.yaPage ya
                                                     //word
        {166, nullptr},                                    //"sprmSDxaLeft", sep.dxaLeft
                                                     //dxa word
        {167, nullptr},                                    //"sprmSDxaRight", sep.dxaRight
                                                     //dxa word
        {168, nullptr},                                    //"sprmSDyaTop", sep.dyaTop                                                     //dya word
        {169, nullptr},                                    //"sprmSDyaBottom",
                                                     //sep.dyaBottom dya word
        {170, nullptr},                                    //"sprmSDzaGutter",
                                                     //sep.dzaGutter dza word
        {171, nullptr},                                    //"sprmSDMPaperReq",
                                                     //sep.dmPaperReq dm word
        {172, nullptr},                                    //"??172",
        {173, nullptr},                                    //"??173",
        {174, nullptr},                                    //"??174",
        {175, nullptr},                                    //"??175",
        {176, nullptr},                                    //"??176",
        {177, nullptr},                                    //"??177",
        {178, nullptr},                                    //"??178",
        {179, nullptr},                                    //"??179",
        {180, nullptr},                                    //"??180",
        {181, nullptr},                                    //"??181",
        {182, nullptr},                                    //"sprmTJc", tap.jc jc word
                                                     //(low order byte is
                                                     //significant)
        {183, nullptr},                                    //"sprmTDxaLeft",
                                                     //tap.rgdxaCenter dxa word
        {184, nullptr},                                    //"sprmTDxaGapHalf",
                                                     //tap.dxaGapHalf,
                                                     //tap.rgdxaCenter dxa word
        {185, nullptr},                                    //"sprmTFCantSplit"
                                                     //tap.fCantSplit 1 or 0 byte
        {186, nullptr},                                    //"sprmTTableHeader",
                                                     //tap.fTableHeader 1 or 0 byte
        {187, nullptr},                                    //"sprmTTableBorders",
                                                     //tap.rgbrcTable complex
                                                     //12 bytes
        {188, nullptr},                                    //"sprmTDefTable10",
                                                     //tap.rgdxaCenter, tap.rgtc
                                                     //complex variable length
        {189, nullptr},                                    //"sprmTDyaRowHeight",
                                                     //tap.dyaRowHeight dya word
        {190, nullptr},                                    //"?sprmTDefTable", tap.rgtc
                                                     //complex
        {191, nullptr},                                    //"?sprmTDefTableShd",
                                                     //tap.rgshd complex
        {192, nullptr},                                    //"sprmTTlp", tap.tlp TLP
                                                     //4 bytes
        {193, nullptr},                                    //"sprmTSetBrc",
                                                     //tap.rgtc[].rgbrc complex
                                                     //5 bytes
        {194, nullptr},                                    //"sprmTInsert",
                                                     //tap.rgdxaCenter,
                                                     //tap.rgtc complex 4 bytes
        {195, nullptr},                                    //"sprmTDelete",
                                                     //tap.rgdxaCenter,
                                                     //tap.rgtc complex word
        {196, nullptr},                                    //"sprmTDxaCol",
                                                     //tap.rgdxaCenter complex
                                                     //4 bytes
        {197, nullptr},                                    //"sprmTMerge",
                                                     //tap.fFirstMerged,
                                                     //tap.fMerged complex word
        {198, nullptr},                                    //"sprmTSplit",
                                                     //tap.fFirstMerged,
                                                     //tap.fMerged complex word
        {199, nullptr},                                    //"sprmTSetBrc10",
                                                     //tap.rgtc[].rgbrc complex
                                                     //5 bytes
        {200, nullptr},                                    //"sprmTSetShd", tap.rgshd
                                                     //complex 4 bytes
        {207, nullptr},                                    //dunno
    };

    static wwSprmDispatcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
}

const wwSprmDispatcher *GetWW8SprmDispatcher()
{
    static SprmReadInfo aSprms[] =
    {
        {0,      nullptr},                                 // "0" Default bzw. Error

        {0x4600, &SwWW8ImplReader::Read_StyleCode},  //"sprmPIstd" pap.istd;istd
                                                     //(style code);short;
        {0xC601, nullptr},                                 //"sprmPIstdPermute" pap.istd;
                                                     //permutation vector;
                                                     //variable length;
        {0x2602, nullptr},                                 //"sprmPIncLvl" pap.istd,
                                                     //pap.lvl;difference between
                                                     //istd of base PAP and istd of
                                                     //PAP to be produced; byte;
        {0x2403, &SwWW8ImplReader::Read_Justify},    //"sprmPJc" pap.jc;jc
                                                     //(justification);byte;
        {0x2404, nullptr},                                 //"sprmPFSideBySide"
                                                     //pap.fSideBySide;0 or 1;byte;
        {0x2405, &SwWW8ImplReader::Read_KeepLines},  //"sprmPFKeep" pap.fKeep;0 or
                                                     //1;byte;
        {0x2406, &SwWW8ImplReader::Read_KeepParas},  //"sprmPFKeepFollow"
                                                     //pap.fKeepFollow;0 or 1;byte;
        {0x2407, &SwWW8ImplReader::Read_BreakBefore},//"sprmPFPageBreakBefore"
                                                     //pap.fPageBreakBefore;0 or 1;
                                                     //byte;
        {0x2408, nullptr},                                 //"sprmPBrcl" pap.brcl;brcl;
                                                     //byte;
        {0x2409, nullptr},                                 //"sprmPBrcp" pap.brcp;brcp;
                                                     //byte;
        {0x260A, &SwWW8ImplReader::Read_ListLevel},  //"sprmPIlvl" pap.ilvl;ilvl;
                                                     //byte;
        {0x460B, &SwWW8ImplReader::Read_LFOPosition},//"sprmPIlfo" pap.ilfo;ilfo
                                                     //(list index) ;short;
        {0x240C, &SwWW8ImplReader::Read_NoLineNumb}, //"sprmPFNoLineNumb"
                                                     //pap.fNoLnn;0 or 1;byte;
        {0xC60D, &SwWW8ImplReader::Read_Tab},        //"sprmPChgTabsPapx"
                                                     //pap.itbdMac, pap.rgdxaTab,
                                                     //pap.rgtbd;complex;variable
                                                     //length
        {0x840E, &SwWW8ImplReader::Read_LR},         //Word 97 version of "sprmPDxaRight" pap.dxaRight;
                                                     //dxa;word;
        {0x840F, &SwWW8ImplReader::Read_LR},         //Apparently Word 97 version of "sprmPDxaLeft" pap.dxaLeft;
                                                     //dxa;word;
        {0x4610, nullptr},                                 //"sprmPNest" pap.dxaLeft;
                                                     //dxa;word;
        {0x8411, &SwWW8ImplReader::Read_LR},         //Word 97 version of "sprmPDxaLeft1" pap.dxaLeft1;
                                                     //dxa;word;
        {0x6412, &SwWW8ImplReader::Read_LineSpace},  //"sprmPDyaLine" pap.lspd;
                                                     //an LSPD, a long word
                                                     //structure consisting of a
                                                     //short of dyaLine followed by
                                                     //a short of fMultLinespace;
                                                     //long;
        {0xA413, &SwWW8ImplReader::Read_UL},         //"sprmPDyaBefore"
                                                     //pap.dyaBefore;dya;word;
        {0xA414, &SwWW8ImplReader::Read_UL},         //"sprmPDyaAfter" pap.dyaAfter;
                                                     //dya;word;
        {0xC615, nullptr},                                 //"sprmPChgTabs" pap.itbdMac,
                                                     //pap.rgdxaTab, pap.rgtbd;
                                                     //complex;variable length;
        {0x2416, nullptr},                                 //"sprmPFInTable" pap.fInTable;
                                                     //0 or 1;byte;
        {0x2417, &SwWW8ImplReader::Read_TabRowEnd},  //"sprmPFTtp" pap.fTtp;0 or 1;
                                                     //byte;
        {0x8418, nullptr},                                 //"sprmPDxaAbs" pap.dxaAbs;dxa;
                                                     //word;
        {0x8419, nullptr},                                 //"sprmPDyaAbs" pap.dyaAbs;dya;
                                                     //word;
        {0x841A, nullptr},                                 //"sprmPDxaWidth" pap.dxaWidth;
                                                     //dxa;word;
        {0x261B, &SwWW8ImplReader::Read_ApoPPC},     //"sprmPPc" pap.pcHorz,
                                                     //pap.pcVert;complex;byte;
        {0x461C, nullptr},                                 //"sprmPBrcTop10" pap.brcTop;
                                                     //BRC10;word;
        {0x461D, nullptr},                                 //"sprmPBrcLeft10" pap.brcLeft;
                                                     //BRC10;word;
        {0x461E, nullptr},                                 //"sprmPBrcBottom10"
                                                     //pap.brcBottom;BRC10;word;
        {0x461F, nullptr},                                 //"sprmPBrcRight10"
                                                     //pap.brcRight;BRC10;word;
        {0x4620, nullptr},                                 //"sprmPBrcBetween10"
                                                     //pap.brcBetween;BRC10;word;
        {0x4621, nullptr},                                 //"sprmPBrcBar10" pap.brcBar;
                                                     //BRC10;word;
        {0x4622, nullptr},                                 //"sprmPDxaFromText10"
                                                     //pap.dxaFromText;dxa;word;
        {0x2423, nullptr},                                 //"sprmPWr" pap.wr;wr;byte;
        {0x6424, &SwWW8ImplReader::Read_Border},     //"sprmPBrcTop80" pap.brcTop;BRC;
                                                     //long;
        {0x6425, &SwWW8ImplReader::Read_Border},     //"sprmPBrcLeft80" pap.brcLeft;
                                                     //BRC;long;
        {0x6426, &SwWW8ImplReader::Read_Border},     //"sprmPBrcBottom80"
                                                     //pap.brcBottom;BRC;long;
        {0x6427, &SwWW8ImplReader::Read_Border},     //"sprmPBrcRight80" pap.brcRight;
                                                     //BRC;long;
        {0x6428, &SwWW8ImplReader::Read_Border},     //"sprmPBrcBetween80"
                                                     //pap.brcBetween;BRC;long;
        {0x6629, nullptr},                                 //"sprmPBrcBar" pap.brcBar;BRC;
                                                     //long;
        {0x242A, &SwWW8ImplReader::Read_Hyphenation},//"sprmPFNoAutoHyph"
                                                     //pap.fNoAutoHyph;0 or 1;byte;
        {0x442B, nullptr},                                 //"sprmPWHeightAbs"
                                                     //pap.wHeightAbs;w;word;
        {0x442C, nullptr},                                 //"sprmPDcs" pap.dcs;DCS;short;
        {0x442D, &SwWW8ImplReader::Read_Shade},      //"sprmPShd" pap.shd;SHD;word;
        {0x842E, nullptr},                                 //"sprmPDyaFromText"
                                                     //pap.dyaFromText;dya;word;
        {0x842F, nullptr},                                 //"sprmPDxaFromText"
                                                     //pap.dxaFromText;dxa;word;
        {0x2430, nullptr},                                 //"sprmPFLocked" pap.fLocked;
                                                     //0 or 1;byte;
        {0x2431, &SwWW8ImplReader::Read_WidowControl},//"sprmPFWidowControl"
                                                     //pap.fWidowControl;0 or 1;byte
        {0xC632, nullptr},                                 //"sprmPRuler" variable length;
        {0x2433, &SwWW8ImplReader::Read_BoolItem},   //"sprmPFKinsoku" pap.fKinsoku;
                                                     //0 or 1;byte;
        {0x2434, nullptr},                                 //"sprmPFWordWrap"
                                                     //pap.fWordWrap;0 or 1;byte;
        {0x2435, &SwWW8ImplReader::Read_BoolItem},   //"sprmPFOverflowPunct"
                                                     //pap.fOverflowPunct; 0 or 1;
                                                     //byte;
        {0x2436, nullptr},                                 //"sprmPFTopLinePunct"
                                                     //pap.fTopLinePunct;0 or 1;byte
        {0x2437, &SwWW8ImplReader::Read_BoolItem},   //"sprmPFAutoSpaceDE"
                                                     //pap.fAutoSpaceDE;0 or 1;byte;
        {0x2438, nullptr},                                 //"sprmPFAutoSpaceDN"
                                                     //pap.fAutoSpaceDN;0 or 1;byte;
        {NS_sprm::LN_PWAlignFont, &SwWW8ImplReader::Read_AlignFont},  //"sprmPWAlignFont"
                                                     //pap.wAlignFont;iFa; word;
        {0x443A, nullptr},                                 //"sprmPFrameTextFlow"
                                                     //pap.fVertical pap.fBackward
                                                     //pap.fRotateFont;complex; word
        {0x243B, nullptr},                                 //"sprmPISnapBaseLine" obsolete
                                                     //not applicable in Word97
                                                     //and later versions;;byte;
        {0xC63E, &SwWW8ImplReader::Read_ANLevelDesc},//"sprmPAnld" pap.anld;;
                                                     //variable length;
        {0xC63F, nullptr},                                 //"sprmPPropRMark"
                                                     //pap.fPropRMark;complex;
                                                     //variable length;
        {0x2640,  &SwWW8ImplReader::Read_POutLvl},   //"sprmPOutLvl" pap.lvl;has no
                                                     //effect if pap.istd is < 1 or
                                                     //is > 9;byte;
        {0x2441, &SwWW8ImplReader::Read_ParaBiDi},   //"sprmPFBiDi" ;;byte;
        {0x2443, nullptr},                                 //"sprmPFNumRMIns"
                                                     //pap.fNumRMIns;1 or 0;bit;
        {0x2444, nullptr},                                 //"sprmPCrLf" ;;byte;
        {0xC645, nullptr},                                 //"sprmPNumRM" pap.numrm;;
                                                     //variable length;
        {0x6645, nullptr},                                 //"sprmPHugePapx" ;fc in the
                                                     //data stream to locate the
                                                     //huge grpprl; long;
        {0x6646, nullptr},                                 //"sprmPHugePapx" ;fc in the
                                                     //data stream to locate the
                                                     //huge grpprl; long;
        {0x2447, &SwWW8ImplReader::Read_UsePgsuSettings},//"sprmPFUsePgsuSettings"
                                                     //pap.fUsePgsuSettings;1 or 0;
                                                     //byte;
        {0x2448, nullptr},                                 //"sprmPFAdjustRight"
                                                     //pap.fAdjustRight;1 or 0;byte;
        {0x0800, &SwWW8ImplReader::Read_CFRMarkDel}, //"sprmCFRMarkDel"
                                                     //chp.fRMarkDel;1 or 0;bit;
        {0x0801, &SwWW8ImplReader::Read_CFRMark},    //"sprmCFRMark" chp.fRMark;1
                                                     //or 0;bit;
        {0x0802, &SwWW8ImplReader::Read_FieldVanish},  //"sprmCFFieldVanish"
                                                     //chp.fFieldVanish;1 or 0;bit;
        {0x6A03, &SwWW8ImplReader::Read_PicLoc},     //"sprmCPicLocation" chp.fcPic
                                                     //and chp.fSpec;variable
                                                     //length, length recorded is
                                                     //always 4;
        {0x4804, nullptr},                                 //"sprmCIbstRMark"
                                                     //chp.ibstRMark;index into
                                                     //sttbRMark;short;
        {0x6805, nullptr},                                 //"sprmCDttmRMark"
                                                     //chp.dttmRMark;DTTM;long;
        {0x0806, nullptr},                                 //"sprmCFData" chp.fData;1 or
                                                     //0;bit;
        {0x4807, nullptr},                                 //"sprmCIdslRMark"
                                                     //chp.idslRMReason;an index to
                                                     //a table of strings defined in
                                                     //Word 6.0 executables;short;
        {0xEA08, &SwWW8ImplReader::Read_CharSet},    //"sprmCChs" chp.fChsDiff and
                                                     //chp.chse;3 bytes;
        {0x6A09, &SwWW8ImplReader::Read_Symbol},     //"sprmCSymbol" chp.fSpec,
                                                     //chp.xchSym and chp.ftcSym;
                                                     //variable length, length
                                                     //recorded is always 4;
        {0x080A, &SwWW8ImplReader::Read_Obj},        //"sprmCFOle2" chp.fOle2;1 or
                                                     //0;bit;
      //0x480B,                                      //"sprmCIdCharType", obsolete:
                                                     //not applicable in Word97
                                                     //and later versions
        {0x2A0C, &SwWW8ImplReader::Read_CharHighlight},//"sprmCHighlight"
                                                     //chp.fHighlight,
                                                     //chp.icoHighlight;ico
                                                     //(fHighlight is set to 1 iff
                                                     //ico is not 0);byte;
        {0x680E, &SwWW8ImplReader::Read_PicLoc},     //"sprmCObjLocation" chp.fcObj;
                                                     //FC;long;
      //0x2A10, ? ? ?,                               //"sprmCFFtcAsciSymb"
        {0x4A30, &SwWW8ImplReader::Read_CColl},      //"sprmCIstd" chp.istd;istd,
                                                     //short;
        {0xCA31, nullptr},                                 //"sprmCIstdPermute" chp.istd;
                                                     //permutation vector; variable
                                                     //length;
        {0x2A32, nullptr},                                 //"sprmCDefault" whole CHP;none
                                                     //;variable length;
        {0x2A33, nullptr},                                 //"sprmCPlain" whole CHP;none;
                                                     //Laenge: 0;
        {0x2A34, &SwWW8ImplReader::Read_Emphasis},   //"sprmCKcd"
        {0x0835, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFBold" chp.fBold;0,1,
                                                     //128, or 129; byte;
        {0x0836, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFItalic" chp.fItalic;0,
                                                     //1, 128, or 129; byte;
        {0x0837, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFStrike" chp.fStrike;0,
                                                     //1, 128, or 129; byte;
        {0x0838, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFOutline" chp.fOutline;
                                                     //0,1, 128, or 129; byte;
        {0x0839, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFShadow" chp.fShadow;0,
                                                     //1, 128, or 129; byte;
        {0x083A, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFSmallCaps"
                                                     //chp.fSmallCaps;0,1, 128, or
                                                     //129;byte;
        {0x083B, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFCaps" chp.fCaps;0,1,
                                                     //128, or 129; byte;
        {0x083C, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFVanish" chp.fVanish;0,
                                                     //1, 128, or 129; byte;
      //0x4A3D, 0,                                   //"sprmCFtcDefault" ftc, only
                                                     //used internally, never
                                                     //stored in file;word;
        {0x2A3E, &SwWW8ImplReader::Read_Underline},  //"sprmCKul" chp.kul;kul;byte;
        {0xEA3F, nullptr},                                 //"sprmCSizePos" chp.hps,
                                                     //chp.hpsPos;3 bytes;
        {0x8840, &SwWW8ImplReader::Read_Kern},       //"sprmCDxaSpace" chp.dxaSpace;
                                                     //dxa;word;
        {0x4A41, &SwWW8ImplReader::Read_Language},   //"sprmCLid" ;only used
                                                     //internally never stored;word;
        {0x2A42, &SwWW8ImplReader::Read_TextColor},   //"sprmCIco" chp.ico;ico;byte;
        {0x4A43, &SwWW8ImplReader::Read_FontSize},   //"sprmCHps" chp.hps;hps;byte;
        {0x2A44, nullptr},                                 //"sprmCHpsInc" chp.hps;byte;
        {0x4845, &SwWW8ImplReader::Read_SubSuperProp},//"sprmCHpsPos" chp.hpsPos;
                                                     //hps; byte;
        {0x2A46, nullptr},                                 //"sprmCHpsPosAdj" chp.hpsPos;
                                                     //hps; byte;
        {0xCA47, &SwWW8ImplReader::Read_Majority},   //"sprmCMajority" chp.fBold,
                                                     //chp.fItalic, chp.fSmallCaps,
                                                     //chp.fVanish, chp.fStrike,
                                                     //chp.fCaps, chp.rgftc,
                                                     //chp.hps, chp.hpsPos, chp.kul,
                                                     //chp.dxaSpace, chp.ico,
                                                     //chp.rglid;complex;variable
                                                     //length, length byte plus
                                                     //size of following grpprl;
        {0x2A48, &SwWW8ImplReader::Read_SubSuper},   //"sprmCIss" chp.iss;iss;byte;
        {0xCA49, nullptr},                                 //"sprmCHpsNew50" chp.hps;hps;
                                                     //variable width, length
                                                     //always recorded as 2;
        {0xCA4A, nullptr},                                 //"sprmCHpsInc1" chp.hps;
                                                     //complex; variable width,
                                                     //length always recorded as 2;
        {0x484B, &SwWW8ImplReader::Read_FontKern},   //"sprmCHpsKern" chp.hpsKern;
                                                     //hps;short;
        {0xCA4C, &SwWW8ImplReader::Read_Majority},   //"sprmCMajority50" chp.fBold,
                                                     //chp.fItalic, chp.fSmallCaps,
                                                     //chp.fVanish, chp.fStrike,
                                                     //chp.fCaps, chp.ftc, chp.hps,
                                                     //chp.hpsPos, chp.kul,
                                                     //chp.dxaSpace, chp.ico;
                                                     //complex; variable length;
        {0x4A4D, nullptr},                                 //"sprmCHpsMul" chp.hps;
                                                     //percentage to grow hps;short;
        {0x484E, nullptr},                                 //"sprmCYsri" chp.ysri;ysri;
                                                     //short;
        {0x4A4F, &SwWW8ImplReader::Read_FontCode},   //"sprmCRgFtc0" chp.rgftc[0];
                                                     //ftc for ASCII text; short;
        {0x4A50, &SwWW8ImplReader::Read_FontCode},   //"sprmCRgFtc1" chp.rgftc[1];
                                                     //ftc for Far East text;short;
        {0x4A51, &SwWW8ImplReader::Read_FontCode},   //"sprmCRgFtc2" chp.rgftc[2];
                                                     //ftc for non-Far East text;
                                                     //short;
        {0x4852, &SwWW8ImplReader::Read_ScaleWidth}, //"sprmCCharScale"
        {0x2A53, &SwWW8ImplReader::Read_BoldUsw},    //"sprmCFDStrike" chp.fDStrike;
                                                     //;byte;
        {0x0854, &SwWW8ImplReader::Read_Relief},     //"sprmCFImprint" chp.fImprint;
                                                     //1 or 0;bit;
        {0x0855, &SwWW8ImplReader::Read_Special},    //"sprmCFSpec" chp.fSpec;
                                                     //1 or 0;bit;
        {0x0856, &SwWW8ImplReader::Read_Obj},        //"sprmCFObj" chp.fObj;1 or 0;
                                                     //bit;
        {0xCA57, &SwWW8ImplReader::Read_CPropRMark}, //"sprmCPropRMark"
                                                     //chp.fPropRMark,
                                                     //chp.ibstPropRMark,
                                                     //chp.dttmPropRMark;Complex;
                                                     //variable length always
                                                     //recorded as 7 bytes;
        {0x0858, &SwWW8ImplReader::Read_Relief},     //"sprmCFEmboss" chp.fEmboss;
                                                     //1 or 0;bit;
        {0x2859, &SwWW8ImplReader::Read_TextAnim},    //"sprmCSfxText" chp.sfxtText;
                                                     //text animation;byte;
        {0x085A, &SwWW8ImplReader::Read_Bidi},                                 //"sprmCFBiDi"
        {0x085B, nullptr},                                 //"sprmCFDiacColor"
        {0x085C, &SwWW8ImplReader::Read_BoldBiDiUsw},//"sprmCFBoldBi"
        {0x085D, &SwWW8ImplReader::Read_BoldBiDiUsw},//"sprmCFItalicBi"
        {0x4A5E, &SwWW8ImplReader::Read_FontCode},   //"sprmCFtcBi"
        {0x485F, &SwWW8ImplReader::Read_Language},   //"sprmCLidBi"
      //0x4A60, ? ? ?,                               //"sprmCIcoBi",
        {0x4A61, &SwWW8ImplReader::Read_FontSize},   //"sprmCHpsBi"
        {0xCA62, nullptr},                                 //"sprmCDispFieldRMark"
                                                     //chp.fDispFieldRMark,
                                                     //chp.ibstDispFieldRMark,
                                                     //chp.dttmDispFieldRMark ;
                                                     //Complex;variable length
                                                     //always recorded as 39 bytes;
        {0x4863, nullptr},                                 //"sprmCIbstRMarkDel"
                                                     //chp.ibstRMarkDel;index into
                                                     //sttbRMark;short;
        {NS_sprm::LN_CDttmRMarkDel, nullptr},
                                                     //chp.dttmRMarkDel;DTTM;long;
        {0x6865, &SwWW8ImplReader::Read_CharBorder}, //"sprmCBrc80" chp.brc;BRC;long;
        {0xca72, &SwWW8ImplReader::Read_CharBorder}, //"sprmCBrc" chp.brc;BRC;long;
        {0x4866, &SwWW8ImplReader::Read_CharShadow}, //"sprmCShd80" chp.shd;SHD;short;
        {0x4867, nullptr},                                 //"sprmCIdslRMarkDel"
                                                     //chp.idslRMReasonDel;an index
                                                     //to a table of strings
                                                     //defined in Word 6.0
                                                     //executables;short;
        {0x0868, nullptr},                                 //"sprmCFUsePgsuSettings"
                                                     //chp.fUsePgsuSettings; 1 or 0;
                                                     //bit;
        {0x486B, nullptr},                                 //"sprmCCpg" ;;word;
        {0x486D, &SwWW8ImplReader::Read_Language},   //"sprmCRgLid0_80" chp.rglid[0];
                                                     //LID: for non-Far East text;
                                                     //word;
        {0x486E, &SwWW8ImplReader::Read_Language},   //"sprmCRgLid1_80" chp.rglid[1];
                                                     //LID: for Far East text;word;
        {0x286F, &SwWW8ImplReader::Read_IdctHint},   //"sprmCIdctHint" chp.idctHint;
                                                     //IDCT: byte;
        {0x2E00, nullptr},                                 //"sprmPicBrcl" pic.brcl;brcl
                                                     //(see PIC structure
                                                     //definition);byte;
        {0xCE01, nullptr},                                 //"sprmPicScale" pic.mx,
                                                     //pic.my, pic.dxaCropleft,
                                                     //pic.dyaCropTop
                                                     //pic.dxaCropRight,
                                                     //pic.dyaCropBottom;Complex;
                                                     //length byte plus 12 bytes;
        {0x6C02, nullptr},                                 //"sprmPicBrcTop80" pic.brcTop;
                                                     //BRC;long;
        {0x6C03, nullptr},                                 //"sprmPicBrcLeft80" pic.brcLeft;
                                                     //BRC;long;
        {0x6C04, nullptr},                                 //"sprmPicBrcBottom80"
                                                     //pic.brcBottom;BRC;long;
        {0x6C05, nullptr},                                 //"sprmPicBrcRight80"
                                                     //pic.brcRight;BRC;long;
        {0x3000, nullptr},                                 //"sprmScnsPgn" sep.cnsPgn;cns;
                                                     //byte;
        {0x3001, nullptr},                                 //"sprmSiHeadingPgn"
                                                     //sep.iHeadingPgn;heading
                                                     //number level;byte;
        {0xD202, &SwWW8ImplReader::Read_OLST},       //"sprmSOlstAnm" sep.olstAnm;
                                                     //OLST;variable length;
        {0xF203, nullptr},                                 //"sprmSDxaColWidth"
                                                     //sep.rgdxaColWidthSpacing;
                                                     //complex; 3 bytes;
        {0xF204, nullptr},                                 //"sprmSDxaColSpacing"
                                                     //sep.rgdxaColWidthSpacing;
                                                     //complex; 3 bytes;
        {0x3005, nullptr},                                 //"sprmSFEvenlySpaced"
                                                     //sep.fEvenlySpaced; 1 or 0;
                                                     //byte;
        {0x3006, nullptr},                                 //"sprmSFProtected"
                                                     //sep.fUnlocked;1 or 0;byte;
        {0x5007, nullptr},                                 //"sprmSDmBinFirst"
                                                     //sep.dmBinFirst;;word;
        {0x5008, nullptr},                                 //"sprmSDmBinOther"
                                                     //sep.dmBinOther;;word;
        {0x3009, nullptr},                                 //"sprmSBkc" sep.bkc;bkc;byte;
        {0x300A, nullptr},                                 //"sprmSFTitlePage"
                                                     //sep.fTitlePage;0 or 1;byte;
        {0x500B, nullptr},                                 //"sprmSCcolumns" sep.ccolM1;
                                                     //# of cols - 1;word;
        {0x900C, nullptr},                                 //"sprmSDxaColumns"
                                                     //sep.dxaColumns;dxa;word;
        {0x300D, nullptr},                                 //"sprmSFAutoPgn" sep.fAutoPgn;
                                                     //obsolete;byte;
        {0x300E, nullptr},                                 //"sprmSNfcPgn" sep.nfcPgn;nfc;
                                                     //byte;
        {0xB00F, nullptr},                                 //"sprmSDyaPgn" sep.dyaPgn;dya;
                                                     //short;
        {0xB010, nullptr},                                 //"sprmSDxaPgn" sep.dxaPgn;dya;
                                                     //short;
        {0x3011, nullptr},                                 //"sprmSFPgnRestart"
                                                     //sep.fPgnRestart;0 or 1;byte;
        {0x3012, nullptr},                                 //"sprmSFEndnote" sep.fEndnote;
                                                     //0 or 1;byte;
        {0x3013, nullptr},                                 //"sprmSLnc" sep.lnc;lnc;byte;
        {0x3014, nullptr},                                 //"sprmSGprfIhdt" sep.grpfIhdt;
                                                     //grpfihdt; byte;
        {0x5015, nullptr},                                 //"sprmSNLnnMod" sep.nLnnMod;
                                                     //non-neg int.;word;
        {0x9016, nullptr},                                 //"sprmSDxaLnn" sep.dxaLnn;dxa;
                                                     //word;
        {0xB017, nullptr},                                 //"sprmSDyaHdrTop"
                                                     //sep.dyaHdrTop;dya;word;
        {0xB018, nullptr},                                 //"sprmSDyaHdrBottom"
                                                     //sep.dyaHdrBottom;dya;word;
        {0x3019, nullptr},                                 //"sprmSLBetween"
                                                     //sep.fLBetween;0 or 1;byte;
        {0x301A, &SwWW8ImplReader::Read_TextVerticalAdjustment},  //"sprmSVjc" sep.vjc;vjc;byte;
        {0x501B, nullptr},                                 //"sprmSLnnMin" sep.lnnMin;lnn;
                                                     //word;
        {0x501C, nullptr},                                 //"sprmSPgnStart" sep.pgnStart;
                                                     //pgn;word;
        {0x301D, nullptr},                                 //"sprmSBOrientation"
                                                     //sep.dmOrientPage;dm;byte;
      //0x301E, ? ? ?,                               //"sprmSBCustomize"
        {0xB01F, nullptr},                                 //"sprmSXaPage" sep.xaPage;xa;
                                                     //word;
        {0xB020, nullptr},                                 //"sprmSYaPage" sep.yaPage;ya;
                                                     //word;
        {0x2205, nullptr},                                 //"sprmSDxaLeft" sep.dxaLeft;
                                                     //dxa;word;
        {0xB022, nullptr},                                 //"sprmSDxaRight" sep.dxaRight;
                                                     //dxa;word;
        {0x9023, nullptr},                                 //"sprmSDyaTop" sep.dyaTop;dya;
                                                     //word;
        {0x9024, nullptr},                                 //"sprmSDyaBottom"
                                                     //sep.dyaBottom;dya;word;
        {0xB025, nullptr},                                 //"sprmSDzaGutter"
                                                     //sep.dzaGutter;dza;word;
        {0x5026, nullptr},                                 //"sprmSDmPaperReq"
                                                     //sep.dmPaperReq;dm;word;
        {0xD227, nullptr},                                 //"sprmSPropRMark"
                                                     //sep.fPropRMark,
                                                     //sep.ibstPropRMark,
                                                     //sep.dttmPropRMark ;complex;
                                                     //variable length always
                                                     //recorded as 7 bytes;
      //0x3228, ? ? ?,                               //"sprmSFBiDi",
      //0x3229, ? ? ?,                               //"sprmSFFacingCol"
        {0x322A, nullptr},                                 //"sprmSFRTLGutter", set to 1
                                                     //if gutter is on the right.
        {0x702B, nullptr},                                 //"sprmSBrcTop80" sep.brcTop;BRC;
                                                     //long;
        {0x702C, nullptr},                                 //"sprmSBrcLeft80" sep.brcLeft;
                                                     //BRC;long;
        {0x702D, nullptr},                                 //"sprmSBrcBottom80"
                                                     //sep.brcBottom;BRC;long;
        {0x702E, nullptr},                                 //"sprmSBrcRight80" sep.brcRight;
                                                     //BRC;long;
        {0x522F, nullptr},                                 //"sprmSPgbProp" sep.pgbProp;
                                                     //word;
        {0x7030, nullptr},                                 //"sprmSDxtCharSpace"
                                                     //sep.dxtCharSpace;dxt;long;
        {0x9031, nullptr},                                 //"sprmSDyaLinePitch"
                                                     //sep.dyaLinePitch;dya;
                                                     //WRONG:long; RIGHT:short; !
      //0x5032, ? ? ?,                               //"sprmSClm"
        {0x5033, nullptr},                                 //"sprmSTextFlow"
                                                     //sep.wTextFlow;complex ;short
        {0x5400, nullptr},                                 //"sprmTJc90" tap.jc;jc;word (low
                                                     //order byte is significant);
        {0x9601, nullptr},                                 //"sprmTDxaLeft"
                                                     //tap.rgdxaCenter; dxa; word;
        {0x9602, nullptr},                                 //"sprmTDxaGapHalf"
                                                     //tap.dxaGapHalf,
                                                     //tap.rgdxaCenter; dxa; word;
        {0x3403, nullptr},                                 //"sprmTFCantSplit"
                                                     //tap.fCantSplit;1 or 0;byte;
        {0x3404, nullptr},                                 //"sprmTTableHeader"
                                                     //tap.fTableHeader;1 or 0;byte;
        {0x3466, nullptr},                                 //"sprmTFCantSplit90"
                                                     //tap.fCantSplit90;1 or 0;byte;
        {0xD605, nullptr},                                 //"sprmTTableBorders80"
                                                     //tap.rgbrcTable;complex;
                                                     //24 bytes;
        {0xD606, nullptr},                                 //"sprmTDefTable10"
                                                     //tap.rgdxaCenter,
                                                     //tap.rgtc;complex; variable
                                                     //length;
        {0x9407, nullptr},                                 //"sprmTDyaRowHeight"
                                                     //tap.dyaRowHeight;dya;word;
        {0xD608, nullptr},                                 //"sprmTDefTable"
                                                     //tap.rgtc;complex
        {0xD609, nullptr},                                 //"sprmTDefTableShd80"
                                                     //tap.rgshd;complex
        {0x740A, nullptr},                                 //"sprmTTlp" tap.tlp;TLP;
                                                     //4 bytes;
      //0x560B, ? ? ?,                               //"sprmTFBiDi"
      //0x740C, ? ? ?,                               //"sprmTHTMLProps"
        {0xD620, nullptr},                                 //"sprmTSetBrc80"
                                                     //tap.rgtc[].rgbrc;complex;
                                                     //5 bytes;
        {0x7621, nullptr},                                 //"sprmTInsert"
                                                     //tap.rgdxaCenter,
                                                     //tap.rgtc;complex ;4 bytes;
        {0x5622, nullptr},                                 //"sprmTDelete"
                                                     //tap.rgdxaCenter,
                                                     //tap.rgtc;complex ;word;
        {0x7623, nullptr},                                 //"sprmTDxaCol"
                                                     //tap.rgdxaCenter;complex;
                                                     //4 bytes;
        {0x5624, nullptr},                                 //"sprmTMerge"
                                                     //tap.fFirstMerged,
                                                     //tap.fMerged;complex; word;
        {0x5625, nullptr},                                 //"sprmTSplit"
                                                     //tap.fFirstMerged,
                                                     //tap.fMerged;complex ;word;
        {0xD626, nullptr},                                 //"sprmTSetBrc10"
                                                     //tap.rgtc[].rgbrc;complex;
                                                     //5 bytes;
        {0x7627, nullptr},                                 //"sprmTSetShd80" tap.rgshd;
                                                     //complex; 4 bytes;
        {0x7628, nullptr},                                 //"sprmTSetShdOdd80"
                                                     //tap.rgshd;complex;4 bytes;
        {0x7629, nullptr},                                 //"sprmTTextFlow"
                                                     //tap.rgtc[].fVertical
                                                     //tap.rgtc[].fBackward
                                                     //tap.rgtc[].fRotateFont
                                                     //0 or 10 or 10 or 1;word;
      //0xD62A, ? ? ?  ,                             //"sprmTDiagLine"
        {0xD62B, nullptr},                                 //"sprmTVertMerge"
                                                     //tap.rgtc[].vertMerge;complex;
                                                     //variable length always
                                                     //recorded as 2 bytes;
        {0xD62C, nullptr},                                 //"sprmTVertAlign"
                                                     //tap.rgtc[].vertAlign;complex
                                                     //variable length always
                                                     //recorded as 3 byte;
        {NS_sprm::LN_CFELayout, &SwWW8ImplReader::Read_DoubleLine_Rotate},
        {0x6649, nullptr},                                 //undocumented
        {0xF614, nullptr},                                 //"sprmTTableWidth"
                                                     //recorded as 3 bytes;
        {0xD612, nullptr},                                 //"sprmTCellShd"
        {0xD613, nullptr},                                 //"sprmTTableBorders"
        {0xD61A, nullptr},                                 //undocumented
        {0xD61B, nullptr},                                 //undocumented
        {0xD61C, nullptr},                                 //undocumented
        {0xD61D, nullptr},                                 //undocumented
        {0xD634, nullptr},                                 //undocumented
        {0xD632, nullptr},                                 //undocumented
        {0xD238, nullptr},                                 //undocumented sep
        {0xC64E, &SwWW8ImplReader::Read_Border},     //"sprmPBrcTop"
        {0xC64F, &SwWW8ImplReader::Read_Border},     //"sprmPBrcLeft"
        {0xC650, &SwWW8ImplReader::Read_Border},     //"sprmPBrcBottom"
        {0xC651, &SwWW8ImplReader::Read_Border},     //"sprmPBrcRight"
        {0xC652, &SwWW8ImplReader::Read_Border},     //"sprmPBorderBetween"
        {0xF661, nullptr},                                 //undocumented
        {0x4873, &SwWW8ImplReader::Read_Language},   //"sprmCRgLid0" chp.rglid[0];
                                                     //LID: for non-Far East text;
                                                     //(like a duplicate of 486D)
        {0x4874, nullptr},                                 //"sprmCRgLid1" chp.rglid[1];
                                                     //LID: for Far East text
                                                     //(like a duplicate of 486E)
        {0x6463, nullptr},                                 //undocumented
        {0x2461, &SwWW8ImplReader::Read_RTLJustify}, //undoc, must be asian version
                                                     //of "sprmPJc"
        {0x845E, &SwWW8ImplReader::Read_LR},         //Apparently post-Word 97 version of "sprmPDxaLeft"
        {0x8460, &SwWW8ImplReader::Read_LR},         //Post-Word 97 version of "sprmPDxaLeft1"
        {0x845D, &SwWW8ImplReader::Read_LR},         //Post-Word 97 version of "sprmPDxaRight"
        {0x3615, nullptr},                                 //undocumented
        {0x360D, nullptr},                                 //undocumented
        {0x940E, nullptr},                                 //undocumented
        {0x940F, nullptr},                                 //undocumented
        {0x9410, nullptr},                                 //undocumented
        {0x703A, nullptr},                                 //undocumented
        {0x303B, nullptr},                                 //undocumented
        {0x244B, &SwWW8ImplReader::Read_TabCellEnd}, //undocumented, must be
                                                     //subtable "sprmPFInTable"
        {0x244C, &SwWW8ImplReader::Read_TabRowEnd},  //undocumented, must be
                                                     // subtable "sprmPFTtp"
        {0x6815, nullptr},                                 //undocumented
        {0x6816, nullptr},                                 //undocumented
        {NS_sprm::LN_CCv, &SwWW8ImplReader::Read_TextForeColor},
        {NS_sprm::LN_CCvUl, &SwWW8ImplReader::Read_UnderlineColor},
        {0xC64D, &SwWW8ImplReader::Read_ParaBackColor},
        {0x6467, nullptr},                                 //undocumented
        {0xF617, nullptr},                                 //undocumented
        {0xD660, nullptr},                                 //undocumented
        {0xD670, nullptr},                                 //undocumented
        {0xCA71, &SwWW8ImplReader::Read_TextBackColor},//"sprmCShd"
        {0x303C, nullptr},                                 //undocumented
        {0x245B, &SwWW8ImplReader::Read_ParaAutoBefore},//undocumented, para
        {0x245C, &SwWW8ImplReader::Read_ParaAutoAfter},//undocumented, para
        {0x246D, &SwWW8ImplReader::Read_ParaContextualSpacing} //"sprmPFContextualSpacing"
    };

    static wwSprmDispatcher aSprmSrch(aSprms, sizeof(aSprms) / sizeof(aSprms[0]));
    return &aSprmSrch;
}

//      Hilfsroutinen : SPRM finden

const SprmReadInfo& SwWW8ImplReader::GetSprmReadInfo(sal_uInt16 nId) const
{
    ww::WordVersion eVersion = m_pWwFib->GetFIBVersion();
    const wwSprmDispatcher *pDispatcher;
    if (eVersion <= ww::eWW2)
        pDispatcher = GetWW2SprmDispatcher();
    else if (eVersion < ww::eWW8)
        pDispatcher = GetWW6SprmDispatcher();
    else
        pDispatcher = GetWW8SprmDispatcher();

    SprmReadInfo aSrch = {0, nullptr};
    aSrch.nId = nId;
    const SprmReadInfo* pFound = pDispatcher->search(aSrch);

    if (!pFound)
    {
        aSrch.nId = 0;
        pFound = pDispatcher->search(aSrch);
    }

    return *pFound;
}

//      Hilfsroutinen : SPRMs

void SwWW8ImplReader::EndSprm( sal_uInt16 nId )
{
    if( ( nId > 255 ) && ( nId < 0x0800 ) ) return;

    const SprmReadInfo& rSprm = GetSprmReadInfo( nId );

    if (rSprm.pReadFnc)
        (this->*rSprm.pReadFnc)( nId, nullptr, -1 );
}

short SwWW8ImplReader::ImportSprm(const sal_uInt8* pPos,sal_uInt16 nId)
{
    if (!nId)
        nId = m_pSprmParser->GetSprmId(pPos);

    OSL_ENSURE( nId != 0xff, "Sprm FF !!!!" );

    const SprmReadInfo& rSprm = GetSprmReadInfo(nId);

    sal_uInt16 nFixedLen = m_pSprmParser->DistanceToData(nId);
    sal_uInt16 nL = m_pSprmParser->GetSprmSize(nId, pPos);

    if (rSprm.pReadFnc)
        (this->*rSprm.pReadFnc)(nId, pPos + nFixedLen, nL - nFixedLen);

    return nL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
