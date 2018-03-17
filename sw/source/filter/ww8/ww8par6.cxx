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
#include "sprmids.hxx"
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
    SprmResult aRes = pSep->HasSprm(nId);          // sprm here?
    const sal_uInt8* pS = aRes.pSprm;
    short nVal = (pS && aRes.nRemainingData >= 2) ? SVBT16ToShort(pS) : nDefaultVal;
    return nVal;
}

static sal_uInt16 ReadUSprm( const WW8PLCFx_SEPX* pSep, sal_uInt16 nId, short nDefaultVal )
{
    SprmResult aRes = pSep->HasSprm(nId);          // sprm here?
    const sal_uInt8* pS = aRes.pSprm;
    sal_uInt16 nVal = (pS && aRes.nRemainingData >= 2) ? SVBT16ToShort(pS) : nDefaultVal;
    return nVal;
}

static sal_uInt8 ReadBSprm( const WW8PLCFx_SEPX* pSep, sal_uInt16 nId, sal_uInt8 nDefaultVal )
{
    SprmResult aRes = pSep->HasSprm(nId);          // sprm here?
    const sal_uInt8* pS = aRes.pSprm;
    sal_uInt8 nVal = (pS && aRes.nRemainingData >= 1) ? *pS : nDefaultVal;
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
            meDir=SvxFrameDirection::Horizontal_LR_TB;
            break;
        case 1:
            meDir=SvxFrameDirection::Vertical_RL_TB;
            break;
        case 2:
            //asian letters are not rotated, western are. We can't import
            //bottom to top going left to right, we can't do this in
            //pages, (in drawboxes we could partly hack it with a rotated
            //drawing box, though not frame)
            meDir=SvxFrameDirection::Vertical_RL_TB;
            break;
        case 3:
            //asian letters are not rotated, western are. We can't import
            meDir=SvxFrameDirection::Vertical_RL_TB;
            break;
        case 4:
            //asian letters are rotated, western not. We can't import
            meDir=SvxFrameDirection::Horizontal_LR_TB;
            break;
    }

    sal_uInt8 bRTLPgn = maSep.fBiDi;
    if ((meDir == SvxFrameDirection::Horizontal_LR_TB) && bRTLPgn)
        meDir = SvxFrameDirection::Horizontal_RL_TB;
}

bool wwSection::IsVertical() const
{
    return meDir == SvxFrameDirection::Vertical_RL_TB || meDir == SvxFrameDirection::Vertical_LR_TB;
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
    for (sal_uInt16 nI = 0; nI < m_xStyles->GetCount(); ++nI)
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

    aGrid.SetRubyHeight(0);

    rFormat.SetFormatAttr(aGrid);
}

void SwWW8ImplReader::Read_ParaBiDi(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if (nLen < 1)
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_FRAMEDIR);
    else
    {
        SvxFrameDirection eDir =
            *pData ? SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB;
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
        aCol.SetOrtho_(false);
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
    // 3. LR-Margin
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
    else if (!mrReader.m_xWDop->iGutterPos)
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

namespace {
// Returns corrected (ODF) margin size
long SetBorderDistance(bool bFromEdge, SvxBoxItem& aBox, SvxBoxItemLine eLine, long nMSMargin)
{
    const editeng::SvxBorderLine* pLine = aBox.GetLine(eLine);
    if (!pLine)
        return nMSMargin;
    sal_Int32 nNewMargin = nMSMargin;
    sal_Int32 nNewDist = aBox.GetDistance(eLine);
    sal_Int32 nLineWidth = pLine->GetScaledWidth();

    editeng::BorderDistanceFromWord(bFromEdge, nNewMargin, nNewDist, nLineWidth);
    aBox.SetDistance(nNewDist, eLine);

    return nNewMargin;
}
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
    bool bFromEdge = rSection.maSep.pgbOffsetFrom == 1;

    aLR.SetLeft(SetBorderDistance(bFromEdge, aBox, SvxBoxItemLine::LEFT, aLR.GetLeft()));
    aLR.SetRight(SetBorderDistance(bFromEdge, aBox, SvxBoxItemLine::RIGHT, aLR.GetRight()));
    aUL.SetUpper(SetBorderDistance(bFromEdge, aBox, SvxBoxItemLine::TOP, aUL.GetUpper()));
    aUL.SetLower(SetBorderDistance(bFromEdge, aBox, SvxBoxItemLine::BOTTOM, aUL.GetLower()));

    aSet.Put(aBox);
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
    if (!mrReader.m_bVer67 && mrReader.m_xWDop->iGutterPos &&
         rSection.maSep.fRTLGutter)
    {
        nWWUp += rSection.maSep.dzaGutter;
    }

    /* Check whether this section has headers / footers */
    sal_uInt16 nHeaderMask = WW8_HEADER_EVEN | WW8_HEADER_ODD;
    sal_uInt16 nFooterMask = WW8_FOOTER_EVEN | WW8_FOOTER_ODD;
    /* Ignore the presence of a first-page header/footer unless it is enabled */
    if( rSection.HasTitlePage() )
    {
        nHeaderMask |= WW8_HEADER_FIRST;
        nFooterMask |= WW8_FOOTER_FIRST;
    }
    rData.bHasHeader = (rSection.maSep.grpfIhdt & nHeaderMask) != 0;
    rData.bHasFooter = (rSection.maSep.grpfIhdt & nFooterMask) != 0;

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
    wwSectionManager::wwULSpaceData const & rData, const wwSection &rSection)
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
                const sal_Int32 nHdLowerSpace( std::abs(rSection.maSep.dyaTop) - rData.nSwUp - rData.nSwHLo );
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
    SwPaM const & rMyPaM, wwSection &rSection)
{
    SwSectionData aSection( CONTENT_SECTION,
            mrReader.m_rDoc.GetUniqueSectionName() );

    SfxItemSet aSet( mrReader.m_rDoc.GetAttrPool(), aFrameFormatSetRange );

    bool bRTLPgn = !maSegments.empty() && maSegments.back().IsBiDi();
    aSet.Put(SvxFrameDirectionItem(
        bRTLPgn ? SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR));

    if (2 == mrReader.m_xWDop->fpc)
        aSet.Put( SwFormatFootnoteAtTextEnd(FTNEND_ATTXTEND));
    if (0 == mrReader.m_xWDop->epc)
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
            m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_LINENUMBER);
        }
        m_bNoLnNumYet = false;
    }
}

wwSection::wwSection(const SwPosition &rPos) : maStart(rPos.nNode)
    , mpSection(nullptr)
    , mpPage(nullptr)
    , meDir(SvxFrameDirection::Horizontal_LR_TB)
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
    static const SvxNumType aNumTyp[5] =
    {
        SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N
    };

    SvxNumberType aType;
    aType.SetNumberingType( aNumTyp[rNewSection.maSep.nfcPgn] );
    rPageDesc.SetNumType(aType);
}

// CreateSep is called for every section change (even at the start of
// the document. CreateSep also creates the pagedesc(s) and
// fills it/them with attributes and KF texts.
// This has become necessary because the translation of the various
// page attributes is interconnected too much.
void wwSectionManager::CreateSep(const long nTextPos)
{
    /*
    #i1909# section/page breaks should not occur in tables or subpage
    elements like frames. Word itself ignores them in this case. The bug is
    more likely that this filter created such documents in the past!
    */
    if (mrReader.m_nInTable || mrReader.m_bTxbxFlySection || mrReader.InLocalApo())
        return;

    WW8PLCFx_SEPX* pSep = mrReader.m_xPlcxMan->GetSepPLCF();
    OSL_ENSURE(pSep, "impossible!");
    if (!pSep)
        return;

    if (!maSegments.empty() && mrReader.m_pLastAnchorPos.get() && *mrReader.m_pLastAnchorPos == *mrReader.m_pPaM->GetPoint())
    {
        bool insert = true;
        SwPaM pam( *mrReader.m_pLastAnchorPos );
        if( pam.Move(fnMoveBackward, GoInNode))
            if( SwTextNode* txtNode = pam.GetPoint()->nNode.GetNode().GetTextNode())
                if( txtNode->Len() == 0 )
                    insert = false;
        if( insert )
            mrReader.AppendTextNode(*mrReader.m_pPaM->GetPoint());
    }

    ww::WordVersion eVer = mrReader.GetFib().GetFIBVersion();

    // M.M. Create a linked section if the WkbPLCF
    // has an entry for one at this cp
    WW8PLCFspecial* pWkb = mrReader.m_xPlcxMan->GetWkbPLCF();
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
    sal_uInt16 nLIdx = ( ( (sal_uInt16)mrReader.m_xWwFib->m_lid & 0xff ) == 0x9 ) ? 1 : 0;

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
        NS_sprm::v6::sprmSBkc,
        NS_sprm::v6::sprmSFTitlePage,
        NS_sprm::v6::sprmSNfcPgn,
        NS_sprm::v6::sprmSCcolumns,
        NS_sprm::v6::sprmSDxaColumns,
        NS_sprm::v6::sprmSLBetween
    };

    static const sal_uInt16 aVer8Ids0[] =
    {
        NS_sprm::sprmSBkc,
        NS_sprm::sprmSFTitlePage,
        NS_sprm::sprmSNfcPgn,
        NS_sprm::sprmSCcolumns,
        NS_sprm::sprmSDxaColumns,
        NS_sprm::sprmSLBetween
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
        SprmResult aRes = pSep->HasSprm(pIds[0]);
        const sal_uInt8* pSprmBkc = aRes.pSprm;
        if (pSprmBkc && aRes.nRemainingData >= 1)
            aNewSection.maSep.bkc = *pSprmBkc;
    }

    // Has a table page
    aNewSection.maSep.fTitlePage =
        sal_uInt8(0 != ReadBSprm( pSep, pIds[1], 0 ));

    // sprmSNfcPgn
    aNewSection.maSep.nfcPgn = ReadBSprm( pSep, pIds[2], 0 );
    if (aNewSection.maSep.nfcPgn > 4)
        aNewSection.maSep.nfcPgn = 0;

    aNewSection.maSep.fUnlocked = eVer > ww::eWW2 ? ReadBSprm(pSep, (eVer <= ww::eWW7 ? NS_sprm::v6::sprmSFProtected : NS_sprm::sprmSFProtected), 0 ) : 0;

    // sprmSFBiDi
    aNewSection.maSep.fBiDi = eVer >= ww::eWW8 ? ReadBSprm(pSep, NS_sprm::sprmSFBiDi, 0) : 0;

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
            sal_uInt8(ReadBSprm(pSep, (eVer <= ww::eWW7 ? NS_sprm::v6::sprmSFEvenlySpaced : NS_sprm::sprmSFEvenlySpaced), 1) != 0);

        if (aNewSection.maSep.ccolM1 > 0 && !aNewSection.maSep.fEvenlySpaced)
        {
            int nColumnDataIdx = 0;
            aNewSection.maSep.rgdxaColumnWidthSpacing[nColumnDataIdx] = 0;

            const sal_uInt16 nColumnWidthSprmId = ( eVer <= ww::eWW7 ? NS_sprm::v6::sprmSDxaColWidth : NS_sprm::sprmSDxaColWidth);
            const sal_uInt16 nColumnSpacingSprmId = ( eVer <= ww::eWW7 ? NS_sprm::v6::sprmSDxaColSpacing : NS_sprm::sprmSDxaColSpacing);
            const sal_uInt8 nColumnCount = static_cast< sal_uInt8 >(aNewSection.maSep.ccolM1 + 1);
            for ( sal_uInt8 nColumn = 0; nColumn < nColumnCount; ++nColumn )
            {
                //sprmSDxaColWidth
                SprmResult aSWRes = pSep->HasSprm(nColumnWidthSprmId, nColumn);
                const sal_uInt8* pSW = aSWRes.pSprm;

                OSL_ENSURE( pSW, "+Sprm 136 (resp. 0xF203) (ColWidth) missing" );
                sal_uInt16 nWidth = (pSW && aSWRes.nRemainingData >= 3) ? SVBT16ToShort(pSW + 1) : 1440;

                aNewSection.maSep.rgdxaColumnWidthSpacing[++nColumnDataIdx] = nWidth;

                if ( nColumn < nColumnCount - 1 )
                {
                    //sprmSDxaColSpacing
                    SprmResult aSDRes = pSep->HasSprm(nColumnSpacingSprmId, nColumn);
                    const sal_uInt8* pSD = aSDRes.pSprm;

                    OSL_ENSURE( pSD, "+Sprm 137 (resp. 0xF204) (Colspacing) missing" );
                    if (pSD && aSDRes.nRemainingData >= 3)
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
        NS_sprm::v6::sprmSBOrientation,
        NS_sprm::v6::sprmSXaPage,
        NS_sprm::v6::sprmSYaPage,
        NS_sprm::v6::sprmSDxaLeft,
        NS_sprm::v6::sprmSDxaRight,
        NS_sprm::v6::sprmSDzaGutter,
        NS_sprm::v6::sprmSFPgnRestart,
        NS_sprm::v6::sprmSPgnStart,
        NS_sprm::v6::sprmSDmBinFirst,
        NS_sprm::v6::sprmSDmBinOther
    };

    static const sal_uInt16 aVer8Ids1[] =
    {
        NS_sprm::sprmSBOrientation,
        NS_sprm::sprmSXaPage,
        NS_sprm::sprmSYaPage,
        NS_sprm::sprmSDxaLeft,
        NS_sprm::sprmSDxaRight,
        NS_sprm::sprmSDzaGutter,
        NS_sprm::sprmSFPgnRestart,
        NS_sprm::sprmSPgnStart97,
        NS_sprm::sprmSDmBinFirst,
        NS_sprm::sprmSDmBinOther
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
    // it's not clear if dmOrientPage is the correct member to
    // decide on this.
    if(mrReader.m_xWDop->doptypography.f2on1 &&
            aNewSection.maSep.dmOrientPage == 2)
        std::swap(aNewSection.maSep.dxaLeft, aNewSection.maSep.dxaRight);

    aNewSection.maSep.dzaGutter = ReadUSprm( pSep, pIds[5], 0);

    aNewSection.maSep.fRTLGutter = static_cast< sal_uInt8 >(eVer >= ww::eWW8 ? ReadUSprm( pSep, NS_sprm::sprmSFRTLGutter, 0 ) : 0);

    // Page Number Restarts - sprmSFPgnRestart
    aNewSection.maSep.fPgnRestart = ReadBSprm(pSep, pIds[6], 0);

    aNewSection.maSep.pgnStart = ReadUSprm( pSep, pIds[7], 0 );

    SprmResult aRes;

    if (eVer >= ww::eWW6)
    {
        aRes = pSep->HasSprm(eVer <= ww::eWW7 ? NS_sprm::v6::sprmSiHeadingPgn : NS_sprm::sprmSiHeadingPgn);
        if (aRes.pSprm && aRes.nRemainingData >= 1)
            aNewSection.maSep.iHeadingPgn = *aRes.pSprm;

        aRes = pSep->HasSprm(eVer <= ww::eWW7 ? NS_sprm::v6::sprmSScnsPgn : NS_sprm::sprmScnsPgn);
        if (aRes.pSprm && aRes.nRemainingData >= 1)
            aNewSection.maSep.cnsPgn = *aRes.pSprm;
    }

    aRes = pSep->HasSprm(pIds[8]);
    const sal_uInt8* pSprmSDmBinFirst = aRes.pSprm;
    if (pSprmSDmBinFirst && aRes.nRemainingData >= 1)
        aNewSection.maSep.dmBinFirst = *pSprmSDmBinFirst;

    aRes = pSep->HasSprm(pIds[9]);
    const sal_uInt8* pSprmSDmBinOther = aRes.pSprm;
    if (pSprmSDmBinOther && aRes.nRemainingData >= 1)
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
        NS_sprm::v6::sprmSDyaTop,
        NS_sprm::v6::sprmSDyaBottom,
        NS_sprm::v6::sprmSDyaHdrTop,
        NS_sprm::v6::sprmSDyaHdrBottom,
        NS_sprm::v6::sprmSNLnnMod,
        NS_sprm::v6::sprmSLnc,
        NS_sprm::v6::sprmSDxaLnn,
        NS_sprm::v6::sprmSLnnMin
    };
    static const sal_uInt16 aVer8Ids2[] =
    {
        NS_sprm::sprmSDyaTop,
        NS_sprm::sprmSDyaBottom,
        NS_sprm::sprmSDyaHdrTop,
        NS_sprm::sprmSDyaHdrBottom,
        NS_sprm::sprmSNLnnMod,
        NS_sprm::sprmSLnc,
        NS_sprm::sprmSDxaLnn,
        NS_sprm::sprmSLnnMin
    };

    pIds = eVer <= ww::eWW2 ? aVer2Ids2 : eVer <= ww::eWW7 ? aVer67Ids2 : aVer8Ids2;

    aNewSection.maSep.dyaTop = ReadSprm( pSep, pIds[0], nTop[nLIdx] );
    aNewSection.maSep.dyaBottom = ReadSprm( pSep, pIds[1], nBot[nLIdx] );
    aNewSection.maSep.dyaHdrTop = ReadUSprm( pSep, pIds[2], 720 );
    aNewSection.maSep.dyaHdrBottom = ReadUSprm( pSep, pIds[3], 720 );

    if (eVer >= ww::eWW8)
    {
        aNewSection.maSep.wTextFlow = ReadUSprm(pSep, NS_sprm::sprmSTextFlow, 0);
        aNewSection.maSep.clm = ReadUSprm( pSep, NS_sprm::sprmSClm, 0 );
        aNewSection.maSep.dyaLinePitch = ReadUSprm(pSep, NS_sprm::sprmSDyaLinePitch, 360);
        aRes = pSep->HasSprm(NS_sprm::sprmSDxtCharSpace);
        if (aRes.pSprm && aRes.nRemainingData >= 4)
            aNewSection.maSep.dxtCharSpace = SVBT32ToUInt32(aRes.pSprm);

        //sprmSPgbProp
        sal_uInt16 pgbProp = ReadSprm( pSep, NS_sprm::sprmSPgbProp, 0 );
        aNewSection.maSep.pgbApplyTo = pgbProp & 0x0007;
        aNewSection.maSep.pgbPageDepth = (pgbProp & 0x0018) >> 3;
        aNewSection.maSep.pgbOffsetFrom = (pgbProp & 0x00E0) >> 5;

        aNewSection.mnBorders =
            ::lcl_ReadBorders(eVer <= ww::eWW7, aNewSection.brc, nullptr, nullptr, pSep);
    }

    // check if Line Numbering must be activated or reset
    SprmResult aSprmSNLnnMod = pSep->HasSprm(pIds[4]);
    if (aSprmSNLnnMod.pSprm && aSprmSNLnnMod.nRemainingData >= 1)
        aNewSection.maSep.nLnnMod = *aSprmSNLnnMod.pSprm;

    SprmResult aSprmSLnc = pSep->HasSprm(pIds[5]);
    if (aSprmSLnc.pSprm && aSprmSLnc.nRemainingData >= 1)
        aNewSection.maSep.lnc = *aSprmSLnc.pSprm;

    SprmResult aSprmSDxaLnn = pSep->HasSprm(pIds[6]);
    if (aSprmSDxaLnn.pSprm && aSprmSDxaLnn.nRemainingData >= 2)
        aNewSection.maSep.dxaLnn = SVBT16ToShort(aSprmSDxaLnn.pSprm);

    SprmResult aSprmSLnnMin = pSep->HasSprm(pIds[7]);
    if (aSprmSLnnMin.pSprm && aSprmSLnnMin.nRemainingData >= 1)
        aNewSection.maSep.lnnMin = *aSprmSLnnMin.pSprm;

    if (eVer <= ww::eWW7)
        aNewSection.maSep.grpfIhdt = ReadBSprm(pSep, eVer <= ww::eWW2 ? 128 : 153, 0);
    else if (mrReader.m_xHdFt)
    {
        aNewSection.maSep.grpfIhdt = WW8_HEADER_ODD | WW8_FOOTER_ODD
            | WW8_HEADER_FIRST | WW8_FOOTER_FIRST;

        // It is possible for a first page header to be provided
        // for this section, but not actually shown in this section.  In this
        // case (aNewSection.maSep.grpfIhdt & WW8_HEADER_FIRST) will be nonzero
        // but aNewSection.HasTitlePage() will be false.
        // Likewise for first page footer.

        if (mrReader.m_xWDop->fFacingPages)
            aNewSection.maSep.grpfIhdt |= WW8_HEADER_EVEN | WW8_FOOTER_EVEN;

        //See if we have a header or footer for each enabled possibility
        //if we do not then we inherit the previous sections header/footer,
        for (int nI = 0, nMask = 1; nI < 6; ++nI, nMask <<= 1)
        {
            if (aNewSection.maSep.grpfIhdt & nMask)
            {
                WW8_CP nStart, nLen;
                mrReader.m_xHdFt->GetTextPosExact( static_cast< short >(nI + ( maSegments.size() + 1) * 6), nStart, nLen);
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
static bool SetWW8_BRC(int nBrcVer, WW8_BRCVer9& rVar, const sal_uInt8* pS, size_t nLen)
{

    if( pS )
    {
        if (nBrcVer == 9 && nLen >= sizeof(WW8_BRCVer9))
            rVar = *reinterpret_cast<const WW8_BRCVer9*>(pS);
        else if (nBrcVer == 8 && nLen >= sizeof(WW8_BRC))
            rVar = WW8_BRCVer9(*reinterpret_cast<const WW8_BRC*>(pS));
        else if (nLen >= sizeof(WW8_BRCVer6)) // nBrcVer == 6
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
            SprmResult a8Sprm[4];
            if (pSep->Find4Sprms(
                    NS_sprm::sprmSBrcTop80, NS_sprm::sprmSBrcLeft80,
                    NS_sprm::sprmSBrcBottom80, NS_sprm::sprmSBrcRight80,
                    a8Sprm[0], a8Sprm[1], a8Sprm[2], a8Sprm[3]))
            {
                for( int i = 0; i < 4; ++i )
                    nBorder |= int(SetWW8_BRC(8, brc[i], a8Sprm[i].pSprm, a8Sprm[i].nRemainingData))<<i;
            }

            // Version 9 BRCs if present will override version 8
            SprmResult a9Sprm[4];
            if (pSep->Find4Sprms(
                    NS_sprm::sprmSBrcTop, NS_sprm::sprmSBrcLeft,
                    NS_sprm::sprmSBrcBottom, NS_sprm::sprmSBrcRight,
                    a9Sprm[0], a9Sprm[1], a9Sprm[2], a9Sprm[3]))
            {
                for( int i = 0; i < 4; ++i )
                    nBorder |= int(SetWW8_BRC(9, brc[i], a9Sprm[i].pSprm, a9Sprm[i].nRemainingData))<<i;
            }
        }
    }
    else
    {

        static const sal_uInt16 aVer67Ids[5] = {
            NS_sprm::v6::sprmPBrcTop,
            NS_sprm::v6::sprmPBrcLeft,
            NS_sprm::v6::sprmPBrcBottom,
            NS_sprm::v6::sprmPBrcRight,
            NS_sprm::v6::sprmPBrcBetween
        };

        static const sal_uInt16 aVer8Ids[5] = {
            NS_sprm::sprmPBrcTop80,
            NS_sprm::sprmPBrcLeft80,
            NS_sprm::sprmPBrcBottom80,
            NS_sprm::sprmPBrcRight80,
            NS_sprm::sprmPBrcBetween80
        };

        static const sal_uInt16 aVer9Ids[5] = {
            NS_sprm::sprmPBrcTop,
            NS_sprm::sprmPBrcLeft,
            NS_sprm::sprmPBrcBottom,
            NS_sprm::sprmPBrcRight,
            NS_sprm::sprmPBrcBetween
        };

        if( pPap )
        {
            if (bVer67)
            {
                for( int i = 0; i < 5; ++i )
                {
                    SprmResult aRes(pPap->HasSprm(aVer67Ids[i]));
                    nBorder |= int(SetWW8_BRC(6 , brc[i], aRes.pSprm, aRes.nRemainingData))<<i;
                }
            }
            else
            {
                for( int i = 0; i < 5; ++i )
                {
                    SprmResult aRes(pPap->HasSprm(aVer8Ids[i]));
                    nBorder |= int(SetWW8_BRC(8 , brc[i], aRes.pSprm, aRes.nRemainingData))<<i;
                }
                // Version 9 BRCs if present will override version 8
                for( int i = 0; i < 5; ++i )
                {
                    SprmResult aRes(pPap->HasSprm(aVer9Ids[i]));
                    nBorder |= int(SetWW8_BRC(9 , brc[i], aRes.pSprm, aRes.nRemainingData))<<i;
                }
            }
        }
        else if( pSty )
        {
            if (bVer67)
            {
                for( int i = 0; i < 5; ++i )
                {
                    SprmResult aRes(pSty->HasParaSprm(aVer67Ids[i]));
                    nBorder |= int(SetWW8_BRC(6 , brc[i], aRes.pSprm, aRes.nRemainingData))<<i;
                }
            }
            else
            {
                for( int i = 0; i < 5; ++i )
                {
                    SprmResult aRes(pSty->HasParaSprm(aVer8Ids[i]));
                    nBorder |= int(SetWW8_BRC(8 , brc[i], aRes.pSprm, aRes.nRemainingData))<<i;
                }
                // Version 9 BRCs if present will override version 8
                for( int i = 0; i < 5; ++i )
                {
                    SprmResult aRes(pSty->HasParaSprm(aVer9Ids[i]));
                    nBorder |= int(SetWW8_BRC(9 , brc[i], aRes.pSprm, aRes.nRemainingData))<<i;
                }
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
    short *pSize)
{
    // LO cannot handle outset/inset (new in WW9 BRC) so fall back same as WW8
    if ( nIdx == 0x1A || nIdx == 0x1B )
    {
        nIdx = (nIdx == 0x1A) ? 0x12 : 0x11;
        cv = 0xc0c0c0;
    }

    SvxBorderLineStyle const eStyle(
            ::editeng::ConvertBorderStyleFromWord(nIdx));

    ::editeng::SvxBorderLine aLine;
    aLine.SetBorderLineStyle( eStyle );
    double const fConverted( (SvxBorderLineStyle::NONE == eStyle) ? 0.0 :
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

    for( int i = 0; i < 4; ++i )
    {
        // filter out the invalid borders
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
        rShadow.SetLocation(SvxShadowLocation::BottomRight);
        bRet = true;
    }
    return bRet;
}

void SwWW8ImplReader::GetBorderDistance(const WW8_BRCVer9* pbrc,
    tools::Rectangle& rInnerDist)
{
    rInnerDist = tools::Rectangle( pbrc[ 1 ].dptSpace() * 20,
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
    SprmResult aS = pPap->HasSprm(nId);
    if (aS.pSprm && aS.nRemainingData >= 2)
        *pVar = (sal_Int16)SVBT16ToShort(aS.pSprm);
    return aS.pSprm != nullptr;
}

inline bool SetValSprm( sal_Int16* pVar, const WW8RStyle* pStyle, sal_uInt16 nId )
{
    SprmResult aS = pStyle->HasParaSprm(nId);
    if (aS.pSprm && aS.nRemainingData >= 2)
        *pVar = (sal_Int16)SVBT16ToShort(aS.pSprm);
    return aS.pSprm != nullptr;
}

/*
#i1930 revealed that sprm 0x360D (sprmTPc) as used in tables can affect the frame
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

        SprmResult aS = pPap->HasSprm(NS_sprm::v6::sprmPWr);
        if (aS.pSprm && aS.nRemainingData >= 1)
            nSp37 = *aS.pSprm;
    }
    else
    {
        SetValSprm( &nSp26, pPap, NS_sprm::sprmPDxaAbs ); // X-position
        //set in me or in parent style
        mbVertSet |= SetValSprm( &nSp27, pPap, NS_sprm::sprmPDyaAbs );    // Y-position
        SetValSprm( &nSp45, pPap, NS_sprm::sprmPWHeightAbs ); // height
        SetValSprm( &nSp28, pPap, NS_sprm::sprmPDxaWidth ); // width
        SetValSprm( &nLeMgn, pPap, NS_sprm::sprmPDxaFromText );    // L-border
        SetValSprm( &nRiMgn, pPap, NS_sprm::sprmPDxaFromText );    // R-border
        SetValSprm( &nUpMgn, pPap, NS_sprm::sprmPDyaFromText );    // U-border
        SetValSprm( &nLoMgn, pPap, NS_sprm::sprmPDyaFromText );    // D-border

        SprmResult aS = pPap->HasSprm(NS_sprm::sprmPWr);                               // wrapping
        if (aS.pSprm && aS.nRemainingData >= 1)
            nSp37 = *aS.pSprm;
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
    std::shared_ptr<WW8PLCFMan> xPlcxMan = pIo->m_xPlcxMan;
    WW8PLCFx_Cp_FKP* pPap = xPlcxMan->GetPapPLCF();

    Read(nOrigSp29, pPap);    // read Apo parameter

    do{             // block for quick exit
        if( nSp45 != 0 /* || nSp28 != 0 */ )
            break;                      // bGrafApo only automatic for height
        if( pIo->m_xWwFib->m_fComplex )
            break;                      // (*pPap)++ does not work for FastSave
                                        // -> for FastSave, no test for graphics APO
        SvStream* pIoStrm = pIo->m_pStrm;
        sal_uLong nPos = pIoStrm->Tell();
        WW8PLCFxSave1 aSave;
        xPlcxMan->GetPap()->Save( aSave );
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
            SprmResult aS = pPap->HasSprm( bVer67 ? NS_sprm::v6::sprmPPc : NS_sprm::sprmPPc);

            // no -> graphics Apo
            if (!aS.pSprm || aS.nRemainingData < 1)
            {
                bGrafApo = true;
                break;                              // end of APO
            }

            ww::WordVersion eVer = pIo->GetFib().GetFIBVersion();
            WW8FlyPara *pNowStyleApo=nullptr;
            sal_uInt16 nColl = pPap->GetIstd();
            ww::sti eSti = eVer < ww::eWW6 ? ww::GetCanonicalStiFromStc( static_cast< sal_uInt8 >(nColl) ) : static_cast<ww::sti>(nColl);
            while (eSti != ww::stiNil && sal::static_int_cast<size_t>(nColl) < pIo->m_vColl.size() && nullptr == (pNowStyleApo = pIo->m_vColl[nColl].m_xWWFly.get()))
            {
                nColl = pIo->m_vColl[nColl].m_nBase;
                eSti = eVer < ww::eWW6 ? ww::GetCanonicalStiFromStc( static_cast< sal_uInt8 >(nColl) ) : static_cast<ww::sti>(nColl);
            }

            WW8FlyPara aF(bVer67, pNowStyleApo);
                                                // new FlaPara for comparison
            aF.Read(*aS.pSprm, pPap);               // WWPara for new Para
            if( !( aF == *this ) )              // same APO? (or a new one?)
                bGrafApo = true;                // no -> 1-line APO
                                                //    -> graphics APO
        }
        while( false );                             // block for quick exit

        xPlcxMan->GetPap()->Restore( aSave );
        pIoStrm->Seek( nPos );
    }while( false );                                    // block for quick exit
}

// read for Apo definitions in Styledefs
void WW8FlyPara::Read(sal_uInt8 nOrigSp29, WW8RStyle const * pStyle)
{
    if (bVer67)
    {
        SetValSprm( &nSp26, pStyle, NS_sprm::v6::sprmPDxaAbs );            // X-position
        //set in me or in parent style
        mbVertSet |= SetValSprm(&nSp27, pStyle, NS_sprm::v6::sprmPDyaAbs); // Y-position
        SetValSprm( &nSp45, pStyle, NS_sprm::v6::sprmPWHeightAbs );        // height
        SetValSprm( &nSp28, pStyle, NS_sprm::v6::sprmPDxaWidth );          // width
        SetValSprm( &nLeMgn, pStyle, NS_sprm::v6::sprmPDxaFromText );      // L-border
        SetValSprm( &nRiMgn, pStyle, NS_sprm::v6::sprmPDxaFromText );      // R-border
        SetValSprm( &nUpMgn, pStyle, NS_sprm::v6::sprmPDyaFromText );      // U-border
        SetValSprm( &nLoMgn, pStyle, NS_sprm::v6::sprmPDyaFromText );      // D-border

        SprmResult aS = pStyle->HasParaSprm( NS_sprm::v6::sprmPWr );       // wrapping
        if (aS.pSprm && aS.nRemainingData >= 1)
            nSp37 = *aS.pSprm;
    }
    else
    {
        SetValSprm( &nSp26, pStyle, NS_sprm::sprmPDxaAbs );            // X-position
        //set in me or in parent style
        mbVertSet |= SetValSprm(&nSp27, pStyle, NS_sprm::sprmPDyaAbs); // Y-position
        SetValSprm( &nSp45, pStyle, NS_sprm::sprmPWHeightAbs );        // height
        SetValSprm( &nSp28, pStyle, NS_sprm::sprmPDxaWidth );          // width
        SetValSprm( &nLeMgn, pStyle, NS_sprm::sprmPDxaFromText );      // L-border
        SetValSprm( &nRiMgn, pStyle, NS_sprm::sprmPDxaFromText );      // R-border
        SetValSprm( &nUpMgn, pStyle, NS_sprm::sprmPDyaFromText );      // U-border
        SetValSprm( &nLoMgn, pStyle, NS_sprm::sprmPDyaFromText );      // D-border

        SprmResult aS = pStyle->HasParaSprm( NS_sprm::sprmPWr );       // wrapping
        if (aS.pSprm && aS.nRemainingData >= 1)
            nSp37 = *aS.pSprm;
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
    return aEmpty == *this;
}

// #i18732# - changes made on behalf of CMC
WW8SwFlyPara::WW8SwFlyPara( SwPaM& rPaM,
                            SwWW8ImplReader& rIo,
                            WW8FlyPara& rWW,
                            const sal_uInt32 nWWPgTop,
                            const sal_uInt32 nPgWidth,
                            const sal_Int32 nIniFlyDx,
                            const sal_Int32 nIniFlyDy ):
pFlyFormat(nullptr),
nXPos(0),
nYPos(0),
nLeMgn(rWW.nLeMgn),
nRiMgn(rWW.nRiMgn),
nUpMgn(rWW.nUpMgn),
nLoMgn(rWW.nLoMgn),
nWidth(rWW.nSp28),
nHeight(rWW.nSp45),
nNetWidth(rWW.nSp28),
eHeightFix(ATT_FIX_SIZE),
eAnchor(RndStdIds::FLY_AT_PARA),
eHRel(text::RelOrientation::PAGE_FRAME),
eVRel(text::RelOrientation::FRAME),
eVAlign(text::VertOrientation::NONE),
eHAlign(text::HoriOrientation::NONE),
eSurround(( rWW.nSp37 > 1 ) ? css::text::WrapTextMode_DYNAMIC : css::text::WrapTextMode_NONE),
nXBind(( rWW.nSp29 & 0xc0 ) >> 6),
nYBind(( rWW.nSp29 & 0x30 ) >> 4),
nNewNetWidth(MINFLY),
xMainTextPos(nullptr),
nLineSpace(0),
bAutoWidth(false),
bToggelPos(false)
{
    //#i119466 mapping "Around" wrap setting to "Parallel" for table
    const bool bIsTable = rIo.m_xPlcxMan->HasParaSprm(NS_sprm::sprmPFInTable).pSprm;
    if (bIsTable && rWW.nSp37 == 2)
        eSurround = css::text::WrapTextMode_PARALLEL;

    /*
     #95905#, #83307# seems to have gone away now, so reenable parallel
     wrapping support for frames in headers/footers. I don't know if we truly
     have an explicitly specified behaviour for these circumstances.
    */

    if( nHeight & 0x8000 )
    {
        nHeight &= 0x7fff;
        eHeightFix = ATT_MIN_SIZE;
    }

    if( nHeight <= MINFLY )
    {                           // no data, or bad data
        eHeightFix = ATT_MIN_SIZE;
        nHeight = MINFLY;
    }

    if( nWidth <= 10 )                              // auto width
    {
        bAutoWidth = true;
        nWidth = nNetWidth =
            msword_cast<sal_Int16>(nPgWidth ? nPgWidth : 2268); // 4 cm
    }
    if( nWidth <= MINFLY )
        nWidth = nNetWidth = MINFLY;              // minimum width

    /*
    See issue #i9178# for the 9 anchoring options, and make sure they stay
    working if you modify the anchoring logic here.
    */

    // If the Fly is aligned left, right, up, or down,
    // the outer text distance will be ignored, because otherwise
    // the Fly will end up in the wrong position.
    // The only problem is with inside/outside.

    //#i53725# - absolute positioned objects have to be
    // anchored at-paragraph to assure its correct anchor position.
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
            // put in initialization part eVRel = text::RelOrientation::FRAME;
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
            // put in initialization part eHRel= text::RelOrientation::PAGE_FRAME;
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
    sal_Int16 nLeBorderMgn( 0 );
    if ( !bAutoWidth )
    {
        WW8_BRCVer9 &rBrc = rWW.brc[WW8_LEFT];
        sal_Int16 nTemp = rBrc.DetermineBorderProperties(&nLeBorderMgn);
        nLeBorderMgn = nLeBorderMgn + nTemp;
    }
    // determine right border distance
    sal_Int16 nRiBorderMgn( 0 );
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

    FlySecur1( nWidth, rWW.bBorderLines );          // Do the borders match ?
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
    : SfxItemSet(rReader.m_rDoc.GetAttrPool(),svl::Items<RES_FRMATR_BEGIN,RES_FRMATR_END-1>{})
{
    if (!rReader.m_bNewDoc)
        Reader::ResetFrameFormatAttrs(*this);    // remove distance/border
                                            // position
    Put(SvxFrameDirectionItem(SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR));

/*Below can all go when we have from left in rtl mode*/
    SwTwips nXPos = pFS->nXPos;
    sal_Int16 eHRel = pFS->eHRel;
    rReader.MiserableRTLGraphicsHack(nXPos, pFS->nWidth, pFS->eHAlign, eHRel);
/*Above can all go when we have from left in rtl mode*/
    Put( SwFormatHoriOrient(nXPos, pFS->eHAlign, pFS->eHRel, pFS->bToggelPos ));
    Put( SwFormatVertOrient( pFS->nYPos, pFS->eVAlign, pFS->eVRel ) );

    if (pFS->nLeMgn || pFS->nRiMgn)     // set borders
        Put(SvxLRSpaceItem(pFS->nLeMgn, pFS->nRiMgn, 0, 0, RES_LR_SPACE));

    if (pFS->nUpMgn || pFS->nLoMgn)
        Put(SvxULSpaceItem(pFS->nUpMgn, pFS->nLoMgn, RES_UL_SPACE));

    //we no longer need to hack around the header/footer problems
    SwFormatSurround aSurround(pFS->eSurround);
    if ( pFS->eSurround == css::text::WrapTextMode_DYNAMIC )
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
    : SfxItemSet(rReader.m_rDoc.GetAttrPool(),svl::Items<RES_FRMATR_BEGIN,RES_FRMATR_END-1>{})
{
    Init(rReader, pPaM);

    Put(SvxFrameDirectionItem(SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR));

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
    SwFormatAnchor aAnchor(RndStdIds::FLY_AS_CHAR);

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
    aChrSet(rDoc.GetAttrPool(), svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END - 1>{} ),
    aParSet(rDoc.GetAttrPool(), svl::Items<RES_PARATR_BEGIN, RES_PARATR_END - 1>{} )
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
    for (const SfxItemSet* pSet : {&aChrSet, &aParSet})
    {
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
    WW8DupProperties aDup(m_rDoc, m_xCtrlStck.get());

    m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), 0, false);

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
    WW8DupProperties aDup(m_rDoc, m_xCtrlStck.get());
    m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), 0, false);

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
    WW8PLCFx_Cp_FKP *pPap = m_xPlcxMan ? m_xPlcxMan->GetPapPLCF() : nullptr;
    if (pPap)
    {
        SprmResult aDCS;
        if (m_bVer67)
            aDCS = pPap->HasSprm(NS_sprm::v6::sprmPDcs);
        else
            aDCS = m_xPlcxMan->GetPapPLCF()->HasSprm(NS_sprm::sprmPDcs);
        if (aDCS.pSprm && aDCS.nRemainingData >= 2)
        {
            /*
              fdct   short :3   0007     drop cap type
                                         0 no drop cap
                                         1 normal drop cap
                                         2 drop cap in margin
            */
            short nDCS = SVBT16ToShort(aDCS.pSprm);
            if (nDCS & 7)
                return true;
        }
    }
    return false;
}

bool SwWW8ImplReader::StartApo(const ApoTestResults &rApo, const WW8_TablePos *pTabPos)
{
    m_xWFlyPara.reset(ConstructApo(rApo, pTabPos));
    if (!m_xWFlyPara)
        return false;

    // <WW8SwFlyPara> constructor has changed - new 4th parameter
    // containing WW8 page top margin.
    m_xSFlyPara.reset(new WW8SwFlyPara( *m_pPaM, *this, *m_xWFlyPara,
                                  m_aSectionManager.GetWWPageTopMargin(),
                                  m_aSectionManager.GetTextAreaWidth(),
                                  m_nIniFlyDx, m_nIniFlyDy));

    // If this paragraph is a Dropcap set the flag and we will deal with it later
    if (IsDropCap())
    {
        m_bDropCap = true;
        m_xAktItemSet.reset(new SfxItemSet(m_rDoc.GetAttrPool(), svl::Items<RES_CHRATR_BEGIN, RES_PARATR_END - 1>{}));
        return false;
    }

    if (!m_xWFlyPara->bGrafApo)
    {

        // Within the GrafApo text attributes have to be ignored, because
        // they would apply to the following lines.  The frame is only inserted
        // if it is not merely positioning a single image.  If it is an image
        // frame, pWFlyPara and pSFlyPara are retained and the resulting
        // attributes applied to the image when inserting the image.

        WW8FlySet aFlySet(*this, m_xWFlyPara.get(), m_xSFlyPara.get(), false);

        if (pTabPos && pTabPos->bNoFly)
        {
            m_xSFlyPara->pFlyFormat = nullptr;
        }
        else
        {
            m_xSFlyPara->pFlyFormat = m_rDoc.MakeFlySection(m_xSFlyPara->eAnchor,
                    m_pPaM->GetPoint(), &aFlySet);
            OSL_ENSURE(m_xSFlyPara->pFlyFormat->GetAnchor().GetAnchorId() ==
                    m_xSFlyPara->eAnchor, "Not the anchor type requested!");
        }

        if (m_xSFlyPara->pFlyFormat)
        {
            if (!m_pDrawModel)
                GrafikCtor();

            SdrObject* pOurNewObject = CreateContactObject(m_xSFlyPara->pFlyFormat);
            m_xWWZOrder->InsertTextLayerObject(pOurNewObject);
        }

        if (RndStdIds::FLY_AS_CHAR != m_xSFlyPara->eAnchor && m_xSFlyPara->pFlyFormat)
        {
            m_xAnchorStck->AddAnchor(*m_pPaM->GetPoint(), m_xSFlyPara->pFlyFormat);
        }

        // remember Pos in body text
        m_xSFlyPara->xMainTextPos.reset(new SwPosition(*m_pPaM->GetPoint()));

        //remove fltanchors, otherwise they will be closed inside the
        //frame, which makes no sense, restore them after the frame is
        //closed
        m_xSFlyPara->xOldAnchorStck = std::move(m_xAnchorStck);
        m_xAnchorStck.reset(new SwWW8FltAnchorStack(&m_rDoc, m_nFieldFlags));

        if (m_xSFlyPara->pFlyFormat)
            MoveInsideFly(m_xSFlyPara->pFlyFormat);

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
        maSegments.back().maStart.Assign(rNode);
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
            m_xCtrlStck->StealAttr(rPam.GetPoint()->nNode);

        if (m_pLastAnchorPos || m_pPreviousNode)
        {
            SwNodeIndex aToBeJoined(aPref, 1);

            if (m_pLastAnchorPos)
            {
                //If the last anchor pos is here, then clear the anchor pos.
                //This "last anchor pos" is only used for fixing up the
                //positions of things anchored to page breaks and here
                //we are removing the last paragraph of a frame, so there
                //cannot be a page break at this point so we can
                //safely reset m_pLastAnchorPos to avoid any dangling
                //SwIndex's pointing into the deleted paragraph
                SwNodeIndex aLastAnchorPos(m_pLastAnchorPos->nNode);
                if (aLastAnchorPos == aToBeJoined)
                    m_pLastAnchorPos.reset();
            }

            if (m_pPreviousNode)
            {
                //If the drop character start pos is here, then clear it.
                SwNodeIndex aDropCharPos(*m_pPreviousNode);
                if (aDropCharPos == aToBeJoined)
                    m_pPreviousNode = nullptr;
            }
        }

        pNode->JoinNext();

        bRet = true;
    }
    return bRet;
}

//In auto-width word frames negative after-indent values are ignored
void SwWW8ImplReader::StripNegativeAfterIndent(SwFrameFormat const *pFlyFormat)
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
    OSL_ENSURE(m_xWFlyPara, "no pWFlyPara to close");
    if (!m_xWFlyPara)
        return;
    if (m_xWFlyPara->bGrafApo)
    {
        // image frame that has not been inserted: delete empty paragraph + attr
        JoinNode(*m_pPaM, true);

    }
    else
    {
        if (!m_xSFlyPara->xMainTextPos)
        {
            OSL_ENSURE(m_xSFlyPara->xMainTextPos.get(), "StopApo: xMainTextPos is nullptr");
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
            MoveOutsideFly(m_xSFlyPara->pFlyFormat, *m_xSFlyPara->xMainTextPos);
        if (nNewWidth)
            m_xSFlyPara->BoxUpWidth(nNewWidth);

        Color aBg(0xFE, 0xFF, 0xFF, 0xFF);  //Transparent by default

        SwTextNode* pNd = aPref.GetNode().GetTextNode();
        if (pNd && m_xSFlyPara->pFlyFormat)
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

        if (m_xSFlyPara->pFlyFormat)
            m_xSFlyPara->pFlyFormat->SetFormatAttr(SvxBrushItem(aBg, RES_BACKGROUND));

        DeleteAnchorStack();
        m_xAnchorStck = std::move(m_xSFlyPara->xOldAnchorStck);

        // When inserting a graphic into the fly frame using the auto
        // function, the extension of the SW-fly has to be set
        // manually as the SW fly has no auto function to adjust the
        // frame´s size.
        if (m_xSFlyPara->nNewNetWidth > MINFLY && m_xSFlyPara->pFlyFormat)    // BoxUpWidth ?
        {
            long nW = m_xSFlyPara->nNewNetWidth;
            nW += m_xSFlyPara->nWidth - m_xSFlyPara->nNetWidth;   // border for it
            m_xSFlyPara->pFlyFormat->SetFormatAttr(
                SwFormatFrameSize(m_xSFlyPara->eHeightFix, nW, m_xSFlyPara->nHeight));
        }
        /*
        Word set *no* width meaning it's an automatic width. The
        SwFlyPara reader will have already set a fallback width of the
        printable regions width, so we should reuse it. Despite the related
        problems with layout addressed with a hack in WW8FlyPara's constructor
        #i27204# Added AutoWidth setting. Left the old CalculateFlySize in place
        so that if the user unselects autowidth, the width doesn't max out
        */
        else if (!m_xWFlyPara->nSp28 && m_xSFlyPara->pFlyFormat)
        {
            using namespace sw::util;
            SfxItemSet aFlySet( m_xSFlyPara->pFlyFormat->GetAttrSet() );

            SwFormatFrameSize aSize(ItemGet<SwFormatFrameSize>(aFlySet, RES_FRM_SIZE));

            aFlySet.ClearItem(RES_FRM_SIZE);

            CalculateFlySize(aFlySet, m_xSFlyPara->xMainTextPos->nNode,
                m_xSFlyPara->nWidth);

            nNewWidth = ItemGet<SwFormatFrameSize>(aFlySet, RES_FRM_SIZE).GetWidth();

            aSize.SetWidth(nNewWidth);
            aSize.SetWidthSizeType(ATT_VAR_SIZE);

            m_xSFlyPara->pFlyFormat->SetFormatAttr(aSize);
        }

        m_xSFlyPara->xMainTextPos.reset();
// To create the SwFrames when inserting into an existing document, fltshell.cxx
// will call pFlyFrame->MakeFrames() when setting the FltAnchor attribute

    }

    //#i8062#
    if (m_xSFlyPara && m_xSFlyPara->pFlyFormat)
        m_pFormatOfJustInsertedApo = m_xSFlyPara->pFlyFormat;

    m_xSFlyPara.reset();
    m_xWFlyPara.reset();
}

// TestSameApo() returns if it's the same Apo or a different one
bool SwWW8ImplReader::TestSameApo(const ApoTestResults &rApo,
    const WW8_TablePos *pTabPos)
{
    if (!m_xWFlyPara)
    {
        OSL_ENSURE(m_xWFlyPara, " Where is my pWFlyPara ? ");
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
        aF.Read(rApo.m_nSprm29, m_xPlcxMan->GetPapPLCF());
    aF.ApplyTabPos(pTabPos);

    return aF == *m_xWFlyPara;
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
        else if (m_xAktItemSet)
        {
            m_xAktItemSet->Put(rAttr);
        }
        else if (rAttr.Which() == RES_FLTR_REDLINE)
        {
            m_xRedlineStack->open(*m_pPaM->GetPoint(), rAttr);
        }
        else
        {
            m_xCtrlStck->NewAttr(*m_pPaM->GetPoint(), rAttr);
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
    else if (m_xAktItemSet)
    {
        pRet = m_xAktItemSet->GetItem(nWhich);
        if (!pRet)
            pRet = m_pStandardFormatColl ? &(m_pStandardFormatColl->GetFormatAttr(nWhich)) : nullptr;
        if (!pRet)
            pRet = &m_rDoc.GetAttrPool().GetDefaultItem(nWhich);
    }
    else if (m_xPlcxMan && m_xPlcxMan->GetDoingDrawTextBox())
    {
        pRet = m_xCtrlStck->GetStackAttr(*m_pPaM->GetPoint(), nWhich);
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
        pRet = m_xCtrlStck->GetFormatAttr(*m_pPaM->GetPoint(), nWhich);
    return pRet;
}

// The methods get as parameters the token id and the length of the following
// parameters according to the table in WWScan.cxx.
void SwWW8ImplReader::Read_Special(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if (nLen < 1)
    {
        m_bSpec = false;
        return;
    }
    m_bSpec = ( *pData != 0 );
}

// Read_Obj is used for fObj and for fOle2 !
void SwWW8ImplReader::Read_Obj(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if (nLen < 1)
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
    if (nLen < 4)
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
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_PARATR_OUTLINELEVEL);
        return;
    }

    if (m_pAktColl != nullptr)
    {
        SwWW8StyInf* pSI = GetStyle(m_nAktColl);
        if (pSI != nullptr)
        {
            pSI->mnWW8OutlineLevel =
                    static_cast< sal_uInt8 >( ( (pData && nLen >= 1) ? *pData : 0 ) );
            NewAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL, SwWW8StyInf::WW8OutlineLevelToOutlinelevel( pSI->mnWW8OutlineLevel ) ) );
        }
    }
    else if (m_pPaM != nullptr)
    {
        const sal_uInt8 nOutlineLevel
            = SwWW8StyInf::WW8OutlineLevelToOutlinelevel(
                static_cast<sal_uInt8>(((pData && nLen >= 1) ? *pData : 0)));
        NewAttr(SfxUInt16Item(RES_PARATR_OUTLINELEVEL, nOutlineLevel));
    }
}

void SwWW8ImplReader::Read_Symbol(sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if( !m_bIgnoreText )
    {
        if (nLen < (m_bVer67 ? 3 : 4))
        {
            //otherwise disable after we print the char
            if (m_xPlcxMan && m_xPlcxMan->GetDoingDrawTextBox())
                m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_FONT );
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

    ww::WordVersion eVersion = m_xWwFib->GetFIBVersion();

    sal_uInt8 nI;
    // the attribute number for "double strike-through" breaks rank
    if (NS_sprm::sprmCFDStrike == nId)
        nI = nContigiousWestern;               // The out of sequence western id
    else
    {
        // The contiguous western ids
        if (eVersion <= ww::eWW2)
            nI = static_cast< sal_uInt8 >(nId - 60);
        else if (eVersion < ww::eWW8)
            nI = static_cast< sal_uInt8 >(nId - NS_sprm::v6::sprmCFBold);
        else
            nI = static_cast< sal_uInt8 >(nId - NS_sprm::sprmCFBold);
    }

    sal_uInt16 nMask = 1 << nI;

    if (nLen < 1)
    {
        if (nI < 2)
        {
            if (eVersion <= ww::eWW6)
            {
                // reset the CTL Weight and Posture, because they are the same as their
                // western equivalents in ww6
                m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), nEndIds[ nWestern + nEastern + nI ] );
            }
            // reset the CJK Weight and Posture, because they are the same as their
            // western equivalents in word
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), nEndIds[ nWestern + nI ] );
        }
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), nEndIds[ nI ] );
        m_xCtrlStck->SetToggleAttr(nI, false);
        return;
    }
    // value: 0 = off, 1 = on, 128 = like style, 129 contrary to style
    bool bOn = *pData & 1;
    SwWW8StyInf* pSI = GetStyle(m_nAktColl);
    if (m_xPlcxMan && eVersion > ww::eWW2)
    {
        SprmResult aCharIstd =
            m_xPlcxMan->GetChpPLCF()->HasSprm(m_bVer67 ? NS_sprm::v6::sprmCIstd : NS_sprm::sprmCIstd);
        if (aCharIstd.pSprm && aCharIstd.nRemainingData >= 2)
            pSI = GetStyle(SVBT16ToShort(aCharIstd.pSprm));
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
            m_xCtrlStck->SetToggleAttr(nI, true);
        }
    }

    SetToggleAttr( nI, bOn );
}

void SwWW8ImplReader::Read_Bidi(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if (nLen < 1)  //Property end
    {
        m_bBidi = false;
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(),RES_CHRATR_BIDIRTL);
    }
    else    //Property start
    {
        m_bBidi = true;
        sal_uInt8 nBidi = *pData;
        NewAttr( SfxInt16Item( RES_CHRATR_BIDIRTL, (nBidi!=0)? 1 : 0 ) );
    }
}

/*
 tdf#91916, #i8726, #i42685# there is an ambiguity
 around certain properties as to what they mean,
 which appears to be a problem with different versions
 of the file format where properties conflict, i.e.

ooo40606-2.doc, magic is a699
    : 0x6f 0x4 0x0 0x71 0x4 0x0
ooo40635-1.doc, magic is a699
    : 0x6f 0x4 0x0 0x71 0x4 0x0
ooo31093/SIMPCHIN.doc, magic is a699
    : 0x6f 0x2 0x0 0x70 0x0 0x0 0x71 0x2 0x0
    : 0x6f 0x5 0x0 0x70 0x5 0x0
ooo31093/TRADCHIN.doc, magic is a699
    : 0x6f 0x1 0x0 0x70 0x0 0x0 0x71 0x1 0x0
ooo31093/JAPANESE.doc, magic is a697
    : 0x6f 0x2 0x0 0x70 0x0 0x0 0x71 0x2 0x0
ooo31093/KOREAN.doc, magic is a698
    : 0x6f 0x2 0x0 0x70 0x0 0x0 0x71 0x2 0x0
ooo31093-1.doc, magic is a698
    : 0x6f 0x5 0x0 0x70 0x5 0x0
ooo31093-1.doc, magic is a698
    : 0x6f 0x5 0x0 0x70 0x5 0x0

meanwhile...

ooo27954-1.doc, magic is a5dc
    : 0x6f 0x1 0x81 0x71 0x2 0x4 0x0 0x74 0x2 0x20 0x0

ooo33251-1.doc, magic is a5dc
    : 0x6f 0x1 0x81 0x71 0x2 0x3 0x0 0x74 0x2 0x1c 0x0

---

So we have the same sprm values, but different payloads, where
the a5dc versions appear to use a len argument, followed by len
bytes, while the a698<->a699 versions use a 2byte argument

commit c2213db9ed70c1fd546482d22e36e4029c10aa45

    INTEGRATION: CWS tl28 (1.169.24); FILE MERGED
    2006/10/25 13:40:41 tl 1.169.24.2: RESYNC: (1.169-1.170); FILE MERGED
    2006/09/20 11:55:50 hbrinkm 1.169.24.1: #i42685# applied patch

changed 0x6f and 0x70 from Read_BoldBiDiUsw to Read_FontCode for all versions.

In the Word for Window 2 spec we have...
 78   //sprmCMajority
 80   //sprmCFBoldBi
 81   //sprmCFItalicBi
 82   //sprmCFtcBi
 83   //sprmClidBi
 84   //sprmCIcoBi
 85   //sprmCHpsBi
as see in GetWW2SprmDispatcher, different numbers, but the sequence starts with
the same sprmCMajority as appears before 0x6f in word 6/95

I think the easiest explanation is that the CJK Word for Window 95, or whatever
the product was went rogue, and did their own things with at least first three
slots after sprmCMajority to do a different thing. I have no reason to think Tono
was wrong with what they do in the a698<->a699 versions, but with magic
a5dc they probably did mean sprmCFBoldBi, sprmCFItalicBi cause they have that 0x81
pattern which has significance for those types of properties.
*/
void SwWW8ImplReader::Read_AmbiguousSPRM(sal_uInt16 nId, const sal_uInt8* pData,
    short nLen)
{
    if (m_xWwFib->m_wIdent >= 0xa697 && m_xWwFib->m_wIdent <= 0xa699)
    {
        Read_FontCode(nId, pData, nLen);
    }
    else
    {
        Read_BoldBiDiUsw(nId, pData, nLen);
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
    ww::WordVersion eVersion = m_xWwFib->GetFIBVersion();
    if (eVersion <= ww::eWW2)
        nI = static_cast< sal_uInt8 >(nId - 80);
    else if (eVersion < ww::eWW8)
        nI = static_cast< sal_uInt8 >(nId - 111);
    else
        nI = static_cast< sal_uInt8 >(nId - NS_sprm::sprmCFBoldBi);

    OSL_ENSURE(nI <= 1, "not happening");
    if (nI > 1)
        return;

    sal_uInt16 nMask = 1 << nI;

    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(),nEndIds[nI]);
        m_xCtrlStck->SetToggleBiDiAttr(nI, false);
        return;
    }
    bool bOn = *pData & 1;
    SwWW8StyInf* pSI = GetStyle(m_nAktColl);
    if (m_xPlcxMan)
    {
        SprmResult aCharIstd =
            m_xPlcxMan->GetChpPLCF()->HasSprm(m_bVer67 ? NS_sprm::v6::sprmCIstd : NS_sprm::sprmCIstd);
        if (aCharIstd.pSprm && aCharIstd.nRemainingData >= 2)
            pSI = GetStyle(SVBT16ToShort(aCharIstd.pSprm));
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
            m_xCtrlStck->SetToggleBiDiAttr(nI, true);
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
    ww::WordVersion eVersion = m_xWwFib->GetFIBVersion();

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
            NewAttr( SvxCaseMapItem( bOn ? SvxCaseMap::SmallCaps
                                              : SvxCaseMap::NotMapped, RES_CHRATR_CASEMAP ) );
            break;
        case 6:
            NewAttr( SvxCaseMapItem( bOn ? SvxCaseMap::Uppercase
                                             : SvxCaseMap::NotMapped, RES_CHRATR_CASEMAP ) );
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

void SwWW8ImplReader::ChkToggleAttr_( sal_uInt16 nOldStyle81Mask,
                                        sal_uInt16 nNewStyle81Mask )
{
    sal_uInt16 i = 1, nToggleAttrFlags = m_xCtrlStck->GetToggleAttrFlags();
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

void SwWW8ImplReader::ChkToggleBiDiAttr_( sal_uInt16 nOldStyle81Mask,
                                        sal_uInt16 nNewStyle81Mask )
{
    sal_uInt16 i = 1, nToggleAttrFlags = m_xCtrlStck->GetToggleBiDiAttrFlags();
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
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
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
    RndStdIds::FLY_AS_CHAR and then we can change its anchoring to centered in the line.
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
            (RndStdIds::FLY_AS_CHAR == pFlyFormat->GetAnchor().GetAnchorId()))
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
    RndStdIds::FLY_AS_CHAR and then we can change its anchoring to centered in the line.
    */
    bool bIsGraphicPlacementHack = false;
    sal_uInt16 nPos;
    if (m_xCtrlStck->GetFormatStackAttr(RES_CHRATR_ESCAPEMENT, &nPos))
    {
        SwPaM aRegion(*m_pPaM->GetPoint());

        SwFltPosition aMkPos((*m_xCtrlStck)[nPos].m_aMkPos);
        SwFltPosition aPtPos(*m_pPaM->GetPoint());

        SwFrameFormat *pFlyFormat = nullptr;
        if (
             SwFltStackEntry::MakeRegion(&m_rDoc,aRegion,false,aMkPos,aPtPos) &&
             nullptr != (pFlyFormat = ContainsSingleInlineGraphic(aRegion))
           )
        {
            m_xCtrlStck->DeleteAndDestroy(nPos);
            pFlyFormat->SetFormatAttr(SwFormatVertOrient(0, text::VertOrientation::CHAR_CENTER, text::RelOrientation::CHAR));
            bIsGraphicPlacementHack = true;
        }
    }
    return bIsGraphicPlacementHack;
}

void SwWW8ImplReader::Read_SubSuperProp( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    ww::WordVersion eVersion = m_xWwFib->GetFIBVersion();

    if (nLen < (eVersion <= ww::eWW2 ? 1 : 2))
    {
        if (!ConvertSubToGraphicPlacement())
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
        return;
    }

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
    FontLineStyle eUnderline = LINESTYLE_NONE;
    bool bWordLine = false;
    if (pData && nLen)
    {
        // Parameter:  0 = none,    1 = single,  2 = by Word,
                    // 3 = double,  4 = dotted,  5 = hidden
                    // 6 = thick,   7 = dash,    8 = dot(not used)
                    // 9 = dotdash 10 = dotdotdash 11 = wave
        switch( *pData )
        {
            case 2: bWordLine = true;
                SAL_FALLTHROUGH;
            case 1: eUnderline = LINESTYLE_SINGLE;       break;
            case 3: eUnderline = LINESTYLE_DOUBLE;       break;
            case 4: eUnderline = LINESTYLE_DOTTED;       break;
            case 7: eUnderline = LINESTYLE_DASH;         break;
            case 9: eUnderline = LINESTYLE_DASHDOT;      break;
            case 10:eUnderline = LINESTYLE_DASHDOTDOT;   break;
            case 6: eUnderline = LINESTYLE_BOLD;         break;
            case 11:eUnderline = LINESTYLE_WAVE;         break;
            case 20:eUnderline = LINESTYLE_BOLDDOTTED;   break;
            case 23:eUnderline = LINESTYLE_BOLDDASH;     break;
            case 39:eUnderline = LINESTYLE_LONGDASH;     break;
            case 55:eUnderline = LINESTYLE_BOLDLONGDASH; break;
            case 25:eUnderline = LINESTYLE_BOLDDASHDOT;  break;
            case 26:eUnderline = LINESTYLE_BOLDDASHDOTDOT;break;
            case 27:eUnderline = LINESTYLE_BOLDWAVE;     break;
            case 43:eUnderline = LINESTYLE_DOUBLEWAVE;   break;
        }
    }

    // if necessary, mix up stack and exit!
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_UNDERLINE );
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_WORDLINEMODE );
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
    if (nLen < 0) // close the tag
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_TWO_LINES );
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_ROTATE );
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
    if (!m_bVer67 && m_xPlcxMan && m_xPlcxMan->GetChpPLCF()->HasSprm(NS_sprm::sprmCCv).pSprm)
        return;

    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_COLOR );
    else
    {
        sal_uInt8 b = *pData;            // parameter: 0 = Auto, 1..16 colors

        if( b > 16 )                // unknown -> Black
            b = 0;

        NewAttr( SvxColorItem(Color(GetCol(b)), RES_CHRATR_COLOR));
        if (m_pAktColl && m_xStyles)
            m_xStyles->bTextColChanged = true;
    }
}

void SwWW8ImplReader::Read_TextForeColor(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if (nLen < 4)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_COLOR );
    else
    {
        Color aColor(msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)));
        NewAttr(SvxColorItem(aColor, RES_CHRATR_COLOR));
        if (m_pAktColl && m_xStyles)
            m_xStyles->bTextColChanged = true;
    }
}

void SwWW8ImplReader::Read_UnderlineColor(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if (nLen < 0)
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
                if (pUnderline && nLen >= 4)
                {
                    pUnderline->SetColor( Color( msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)) ) );
                    m_pAktColl->SetFormatAttr( *pUnderline );
                    delete pUnderline;
                }
            }
        }
        else if (m_xAktItemSet)
        {
            if ( SfxItemState::SET == m_xAktItemSet->GetItemState( RES_CHRATR_UNDERLINE, false ) )
            {
                SvxUnderlineItem *pUnderline
                    = static_cast<SvxUnderlineItem*>(m_xAktItemSet->Get(RES_CHRATR_UNDERLINE, false).Clone());
                if (pUnderline && nLen >= 4)
                {
                    pUnderline->SetColor( Color( msfilter::util::BGRToRGB(SVBT32ToUInt32(pData)) ) );
                    m_xAktItemSet->Put( *pUnderline );
                    delete pUnderline;
                }
            }
        }
        else
        {
            SvxUnderlineItem* pUnderlineAttr = const_cast<SvxUnderlineItem*>(static_cast<const SvxUnderlineItem*>(m_xCtrlStck->GetOpenStackAttr( *m_pPaM->GetPoint(), RES_CHRATR_UNDERLINE )));
            if (pUnderlineAttr && nLen >= 4)
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

    const WW8_FFN* pF = m_xFonts->GetFont( nFCode );  // Info for it
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
    if (SetNewFontAttr(nFCode, true, nId) && m_pAktColl && m_xStyles)
    {
        // remember for simulating default font
        if (RES_CHRATR_CJK_FONT == nId)
            m_xStyles->bCJKFontChanged = true;
        else if (RES_CHRATR_CTL_FONT == nId)
            m_xStyles->bCTLFontChanged = true;
        else
            m_xStyles->bFontChanged = true;
    }
}

void SwWW8ImplReader::closeFont(sal_uInt16 nId)
{
    m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), nId );
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
            case 113:                   //WW7
            case NS_sprm::sprmCRgFtc2:  //"Other" font, override with BiDi if it exists
            case NS_sprm::sprmCFtcBi:   //BiDi Font
                nId = RES_CHRATR_CTL_FONT;
                break;
            case NS_sprm::v6::sprmCFtc: //WW6
            case 111:                   //WW7
            case NS_sprm::sprmCRgFtc0:
                nId = RES_CHRATR_FONT;
                break;
            case 112:                   //WW7
            case NS_sprm::sprmCRgFtc1:
                nId = RES_CHRATR_CJK_FONT;
                break;
            default:
                return ;
        }

        ww::WordVersion eVersion = m_xWwFib->GetFIBVersion();

        if (nLen < 2) // end of attribute
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
        case 74: // WW2
        case NS_sprm::v6::sprmCHps:
        case NS_sprm::sprmCHps:
            nId = RES_CHRATR_FONTSIZE;
            break;
        case 85:  //WW2
        case 116: //WW7
        case NS_sprm::sprmCHpsBi:
            nId = RES_CHRATR_CTL_FONTSIZE;
            break;
        default:
            return ;
    }

    ww::WordVersion eVersion = m_xWwFib->GetFIBVersion();

    if (nLen < (eVersion <= ww::eWW2 ? 1 : 2))          // end of attribute
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), nId  );
        if (eVersion <= ww::eWW6) // reset additionally the CTL size
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_CTL_FONTSIZE );
        if (RES_CHRATR_FONTSIZE == nId)  // reset additionally the CJK size
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_CJK_FONTSIZE );
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
        if (m_pAktColl && m_xStyles)            // Style-Def ?
        {
            // remember for simulating default font size
            if (nId == RES_CHRATR_CTL_FONTSIZE)
                m_xStyles->bFCTLSizeChanged = true;
            else
            {
                m_xStyles->bFSizeChanged = true;
                if (eVersion <= ww::eWW6)
                    m_xStyles->bFCTLSizeChanged= true;
            }
        }
    }
}

void SwWW8ImplReader::Read_CharSet(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    if (nLen < 1)
    {                   // end of attribute
        m_eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
        return;
    }
    sal_uInt8 nfChsDiff = *pData;

    if (nfChsDiff && nLen >= 2)
        m_eHardCharSet = rtl_getTextEncodingFromWindowsCharset( *(pData + 1) );
    else
        m_eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
}

void SwWW8ImplReader::Read_Language( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    switch( nId )
    {
        case NS_sprm::v6::sprmCLid:
        case NS_sprm::sprmCRgLid0_80:
        case NS_sprm::sprmCRgLid0:
            nId = RES_CHRATR_LANGUAGE;
            break;
        case NS_sprm::sprmCRgLid1_80:
        case NS_sprm::sprmCRgLid1:
            nId = RES_CHRATR_CJK_LANGUAGE;
            break;
        case 83:  // WW2
        case 114: // WW7
        case NS_sprm::sprmCLidBi:
            nId = RES_CHRATR_CTL_LANGUAGE;
            break;
        default:
            return;
    }

    if (nLen < 2)                  // end of attribute
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), nId );
    else
    {
        sal_uInt16 nLang = SVBT16ToShort( pData );  // Language-Id
        NewAttr(SvxLanguageItem(LanguageType(nLang), nId));
    }
}

/*
    Turn on character style:
*/
void SwWW8ImplReader::Read_CColl( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 2)    // end of attribute
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_TXTATR_CHARFMT );
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
    if (nLen < 2)  // end of attribute
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_KERNING );
        return;
    }
    sal_Int16 nKern = SVBT16ToShort( pData );    // Kerning in Twips
    NewAttr( SvxKerningItem( nKern, RES_CHRATR_KERNING ) );
}

void SwWW8ImplReader::Read_FontKern( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 2) // end of attribute
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_AUTOKERN );
        return;
    }
    sal_Int16 nAutoKern = SVBT16ToShort( pData );    // Kerning in Twips
    NewAttr(SvxAutoKernItem((bool)nAutoKern, RES_CHRATR_AUTOKERN));
}

void SwWW8ImplReader::Read_CharShadow(  sal_uInt16, const sal_uInt8* pData, short nLen )
{
    //Has newer colour variant, ignore this old variant
    if (!m_bVer67 && m_xPlcxMan && m_xPlcxMan->GetChpPLCF()->HasSprm(NS_sprm::sprmCShd).pSprm)
        return;

    if (nLen < 2)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
    }
    else
    {
        WW8_SHD aSHD;
        aSHD.SetWWValue( *reinterpret_cast<SVBT16 const *>(pData) );
        SwWW8Shade aSh( m_bVer67, aSHD );

        NewAttr( SvxBrushItem( aSh.aColor, RES_CHRATR_BACKGROUND ));

        // Add a marker to the grabbag indicating that character background was imported from MSO shading
        SfxGrabBagItem aGrabBag = *static_cast<const SfxGrabBagItem*>(GetFormatAttr(RES_CHRATR_GRABBAG));
        std::map<OUString, css::uno::Any>& rMap = aGrabBag.GetGrabBag();
        rMap.insert(std::pair<OUString, css::uno::Any>("CharShadingMarker",uno::makeAny(true)));
        NewAttr(aGrabBag);
    }
}

void SwWW8ImplReader::Read_TextBackColor(sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen <= 0)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
    }
    else
    {
        OSL_ENSURE(nLen == 10, "Len of para back colour not 10!");
        if (nLen != 10)
            return;
        Color aColour(ExtractColour(pData, m_bVer67));
        NewAttr(SvxBrushItem(aColour, RES_CHRATR_BACKGROUND));

        // Add a marker to the grabbag indicating that character background was imported from MSO shading
        SfxGrabBagItem aGrabBag = *static_cast<const SfxGrabBagItem*>(GetFormatAttr(RES_CHRATR_GRABBAG));
        std::map<OUString, css::uno::Any>& rMap = aGrabBag.GetGrabBag();
        rMap.insert(std::pair<OUString, css::uno::Any>("CharShadingMarker",uno::makeAny(true)));
        NewAttr(aGrabBag);
    }
}

void SwWW8ImplReader::Read_CharHighlight(sal_uInt16, const sal_uInt8* pData, short nLen)
{
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_HIGHLIGHT );
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
    if (nLen < 0)  // end of attribute
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_LINENUMBER );
        return;
    }
    SwFormatLineNumber aLN;
    if (const SwFormatLineNumber* pLN
        = static_cast<const SwFormatLineNumber*>(GetFormatAttr(RES_LINENUMBER)))
    {
        aLN.SetStartValue( pLN->GetStartValue() );
    }

    aLN.SetCountLines(pData && nLen >= 1 && (0 == *pData));
    NewAttr( aLN );
}

bool lcl_HasExplicitLeft(const WW8PLCFMan *pPlcxMan, bool bVer67)
{
    WW8PLCFx_Cp_FKP *pPap = pPlcxMan ? pPlcxMan->GetPapPLCF() : nullptr;
    if (pPap)
    {
        if (bVer67)
            return pPap->HasSprm(NS_sprm::v6::sprmPDxaLeft).pSprm;
        else
            return (pPap->HasSprm(NS_sprm::sprmPDxaLeft80).pSprm || pPap->HasSprm(NS_sprm::sprmPDxaLeft).pSprm);
    }
    return false;
}

// Sprm 16, 17
void SwWW8ImplReader::Read_LR( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    if (nLen < 2)  // end of attribute
    {
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_LR_SPACE);
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
            case NS_sprm::v6::sprmPDxaLeft:
                nId = NS_sprm::v6::sprmPDxaRight;
                break;
            case NS_sprm::sprmPDxaLeft80:
                nId = NS_sprm::sprmPDxaRight80;
                break;
            //Right becomes before;
            case NS_sprm::v6::sprmPDxaRight:
                nId = NS_sprm::v6::sprmPDxaLeft;
                break;
            case NS_sprm::sprmPDxaRight80:
                nId = NS_sprm::sprmPDxaLeft80;
                break;
        }
    }

    bool bFirstLinOfstSet( false ); // #i103711#
    bool bLeftIndentSet( false ); // #i105414#

    switch (nId)
    {
        //sprmPDxaLeft
        case NS_sprm::v6::sprmPDxaLeft:
        case NS_sprm::sprmPDxaLeft80:
        case NS_sprm::sprmPDxaLeft:
            aLR.SetTextLeft( nPara );
            if (m_pAktColl && m_nAktColl < m_vColl.size())
            {
                m_vColl[m_nAktColl].m_bListReleventIndentSet = true;
            }
            bLeftIndentSet = true;  // #i105414#
            break;
        //sprmPDxaLeft1
        case NS_sprm::v6::sprmPDxaLeft1:
        case NS_sprm::sprmPDxaLeft180:
        case NS_sprm::sprmPDxaLeft1:
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
            if (m_xPlcxMan && m_nAktColl < m_vColl.size() && m_vColl[m_nAktColl].m_bHasBrokenWW6List)
            {
                SprmResult aIsZeroed = m_xPlcxMan->GetPapPLCF()->HasSprm(NS_sprm::sprmPIlfo);
                if (aIsZeroed.pSprm && aIsZeroed.nRemainingData >= 1 && *aIsZeroed.pSprm == 0)
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
                        if (!lcl_HasExplicitLeft(m_xPlcxMan.get(), m_bVer67))
                        {
                            aLR.SetTextLeft(pNumFormat->GetIndentAt());

                            // If have not explicit left, set number format list tab position is doc default tab
                            const SvxTabStopItem *pDefaultStopItem = static_cast<const SvxTabStopItem *>(m_rDoc.GetAttrPool().GetPoolDefaultItem(RES_PARATR_TABSTOP));
                            if ( pDefaultStopItem &&  pDefaultStopItem->Count() > 0 )
                                const_cast<SwNumFormat*>(pNumFormat)->SetListtabPos( const_cast<SvxTabStop&>((*pDefaultStopItem)[0]).GetTabPos() );
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
        case NS_sprm::v6::sprmPDxaRight:
        case NS_sprm::sprmPDxaRight80:
        case NS_sprm::sprmPDxaRight:
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
// comment see Read_UL()
    if (m_bStyNormal && m_bWWBugNormal)
        return;

    ww::WordVersion eVersion = m_xWwFib->GetFIBVersion();

    if (nLen < (eVersion <= ww::eWW2 ? 3 : 4))
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_LINESPACING );
        if( !( m_nIniFlags & WW8FL_NO_IMPLPASP ) )
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }

    short nSpace = SVBT16ToShort( pData );
    short nMulti = (eVersion <= ww::eWW2) ? 1 : SVBT16ToShort( pData + 2 );

    SvxLineSpaceRule eLnSpc;
    if( 0 > nSpace )
    {
        nSpace = -nSpace;
        eLnSpc = SvxLineSpaceRule::Fix;
    }
    else
        eLnSpc = SvxLineSpaceRule::Min;

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
        aLSpc.SetPropLineSpace( (sal_uInt8)n );
        const SvxFontHeightItem* pH = static_cast<const SvxFontHeightItem*>(
            GetFormatAttr( RES_CHRATR_FONTSIZE ));
        nSpaceTw = (sal_uInt16)( n * pH->GetHeight() / 100 );
    }
    else                            // Fixed / Minimum
    {
        // for negative space, the distance is "exact", otherwise "at least"
        nSpaceTw = (sal_uInt16)nSpace;
        aLSpc.SetLineHeight( nSpaceTw );
        aLSpc.SetLineSpaceRule( eLnSpc);
    }
    NewAttr( aLSpc );
    if (m_xSFlyPara)
        m_xSFlyPara->nLineSpace = nSpaceTw;   // linespace for graphics APOs
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
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_UL_SPACE);
        return;
    }

    if (*pData)
    {
        SvxULSpaceItem aUL(*static_cast<const SvxULSpaceItem*>(GetFormatAttr(RES_UL_SPACE)));
        aUL.SetUpper(GetParagraphAutoSpace(m_xWDop->fDontUseHTMLAutoSpacing));
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
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_UL_SPACE);
        return;
    }

    if (*pData)
    {
        SvxULSpaceItem aUL(*static_cast<const SvxULSpaceItem*>(GetFormatAttr(RES_UL_SPACE)));
        aUL.SetLower(GetParagraphAutoSpace(m_xWDop->fDontUseHTMLAutoSpacing));
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

    if (nLen < 2)
    {
        // end of attribute
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    short nPara = SVBT16ToShort( pData );
    if( nPara < 0 )
        nPara = -nPara;

    SvxULSpaceItem aUL( *static_cast<const SvxULSpaceItem*>(GetFormatAttr( RES_UL_SPACE )));

    switch( nId )
    {
        //sprmPDyaBefore
        case NS_sprm::v6::sprmPDyaBefore:
        case NS_sprm::sprmPDyaBefore:
            aUL.SetUpper( nPara );
            break;
        //sprmPDyaAfter
        case NS_sprm::v6::sprmPDyaAfter:
        case NS_sprm::sprmPDyaAfter:
            aUL.SetLower( nPara );
            break;
        default:
            return;
    }

    NewAttr( aUL );
}

void SwWW8ImplReader::Read_ParaContextualSpacing( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_UL_SPACE );
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
    // when this value is 2, text properties bias towards complex properties.
    if (nLen < 1)  //Property end
    {
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(),RES_CHRATR_IDCTHINT);
    }
    else    //Property start
    {
        NewAttr(SfxInt16Item(RES_CHRATR_IDCTHINT, *pData));
    }
}

void SwWW8ImplReader::Read_Justify( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_ADJUST );
        return;
    }

    SvxAdjust eAdjust(SvxAdjust::Left);
    bool bDistributed = false;
    switch (*pData)
    {
        default:
        case 0:
            break;
        case 1:
            eAdjust = SvxAdjust::Center;
            break;
        case 2:
            eAdjust = SvxAdjust::Right;
            break;
        case 3:
            eAdjust = SvxAdjust::Block;
            break;
        case 4:
            eAdjust = SvxAdjust::Block;
            bDistributed = true;
            break;
    }
    SvxAdjustItem aAdjust(eAdjust, RES_PARATR_ADJUST);
    if (bDistributed)
        aAdjust.SetLastBlock(SvxAdjust::Block);

    NewAttr(aAdjust);
}

bool SwWW8ImplReader::IsRightToLeft()
{
    bool bRTL = false;
    SprmResult aDir;
    if (m_xPlcxMan)
        aDir = m_xPlcxMan->GetPapPLCF()->HasSprm(NS_sprm::sprmPFBiDi);
    if (aDir.pSprm && aDir.nRemainingData >= 1)
        bRTL = *aDir.pSprm != 0;
    else
    {
        const SvxFrameDirectionItem* pItem=
            static_cast<const SvxFrameDirectionItem*>(GetFormatAttr(RES_FRAMEDIR));
        if (pItem && (pItem->GetValue() == SvxFrameDirection::Horizontal_RL_TB))
            bRTL = true;
    }
    return bRTL;
}

void SwWW8ImplReader::Read_RTLJustify( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_ADJUST );
        return;
    }

    //If we are in a ltr paragraph this is the same as normal Justify,
    //If we are in a rtl paragraph the meaning is reversed.
    if (!IsRightToLeft())
        Read_Justify(NS_sprm::sprmPJc80 /*dummy*/, pData, nLen);
    else
    {
        SvxAdjust eAdjust(SvxAdjust::Right);
        bool bDistributed = false;
        switch (*pData)
        {
            default:
            case 0:
                break;
            case 1:
                eAdjust = SvxAdjust::Center;
                break;
            case 2:
                eAdjust = SvxAdjust::Left;
                break;
            case 3:
                eAdjust = SvxAdjust::Block;
                break;
            case 4:
                eAdjust = SvxAdjust::Block;
                bDistributed = true;
                break;
        }
        SvxAdjustItem aAdjust(eAdjust, RES_PARATR_ADJUST);
        if (bDistributed)
            aAdjust.SetLastBlock(SvxAdjust::Block);

        NewAttr(aAdjust);
    }
}

void SwWW8ImplReader::Read_BoolItem( sal_uInt16 nId, const sal_uInt8* pData, short nLen )
{
    switch( nId )
    {
        case NS_sprm::sprmPFKinsoku:
            nId = RES_PARATR_FORBIDDEN_RULES;
            break;
        case NS_sprm::sprmPFOverflowPunct:
            nId = RES_PARATR_HANGINGPUNCTUATION;
            break;
        case NS_sprm::sprmPFAutoSpaceDE:
            nId = RES_PARATR_SCRIPTSPACE;
            break;
        default:
            OSL_ENSURE( false, "wrong Id" );
            return ;
    }

    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), nId );
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
    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_EMPHASIS_MARK );
    else
    {
        LanguageType nLang;
        //Check to see if there is an up and coming cjk language property. If
        //there is use it, if there is not fall back to the currently set one.
        //Only the cjk language setting seems to matter to word, the western
        //one is ignored
        SprmResult aLang;
        if (m_xPlcxMan)
            aLang = m_xPlcxMan->GetChpPLCF()->HasSprm(NS_sprm::sprmCRgLid1_80);

        if (aLang.pSprm && aLang.nRemainingData >= 2)
            nLang = LanguageType(SVBT16ToShort(aLang.pSprm));
        else
        {
            nLang = static_cast<const SvxLanguageItem *>(
                GetFormatAttr(RES_CHRATR_CJK_LANGUAGE))->GetLanguage();
        }

        FontEmphasisMark nVal;
        switch( *pData )
        {
        case 0:
            nVal = FontEmphasisMark::NONE;
            break;
        case 2:
            if (MsLangId::isKorean(nLang) || MsLangId::isTraditionalChinese(nLang))
                nVal = (FontEmphasisMark::Circle | FontEmphasisMark::PosAbove);
            else if (nLang == LANGUAGE_JAPANESE)
                nVal = (FontEmphasisMark::Accent | FontEmphasisMark::PosAbove);
            else
                nVal = (FontEmphasisMark::Dot | FontEmphasisMark::PosBelow);
            break;
        case 3:
            nVal = (FontEmphasisMark::Circle | FontEmphasisMark::PosAbove);
            break;
        case 4:
            nVal = (FontEmphasisMark::Dot | FontEmphasisMark::PosBelow);
            break;
        case 1:
            if (MsLangId::isSimplifiedChinese(nLang))
                nVal = (FontEmphasisMark::Dot | FontEmphasisMark::PosBelow);
            else
                nVal = (FontEmphasisMark::Dot | FontEmphasisMark::PosAbove);
            break;
        default:
            nVal = (FontEmphasisMark::Dot | FontEmphasisMark::PosAbove);
            break;
        }

        NewAttr( SvxEmphasisMarkItem( nVal, RES_CHRATR_EMPHASIS_MARK ) );
    }
}

void SwWW8ImplReader::Read_ScaleWidth( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 2)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_SCALEW );
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
    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_RELIEF );
    else
    {
        if( *pData )
        {
// not so easy because this is also a toggle attribute!
//  2 x emboss on -> no emboss !!!
// the actual value must be searched over the stack / template

            const SvxCharReliefItem* pOld = static_cast<const SvxCharReliefItem*>(
                                            GetFormatAttr( RES_CHRATR_RELIEF ));
            FontRelief nNewValue = NS_sprm::sprmCFImprint == nId ? FontRelief::Engraved
                                        : ( NS_sprm::sprmCFEmboss == nId ? FontRelief::Embossed
                                                         : FontRelief::NONE );
            if( pOld->GetValue() == nNewValue )
            {
                if( FontRelief::NONE != nNewValue )
                    nNewValue = FontRelief::NONE;
            }
            NewAttr( SvxCharReliefItem( nNewValue, RES_CHRATR_RELIEF ));
        }
    }
}

void SwWW8ImplReader::Read_TextAnim(sal_uInt16 /*nId*/, const sal_uInt8* pData, short nLen)
{
    if (nLen < 1)
        m_xCtrlStck->SetAttr(*m_pPaM->GetPoint(), RES_CHRATR_BLINK);
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
            bBlink = *pData > 0 && *pData < 7;

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

    if( nIndex >= SAL_N_ELEMENTS( eMSGrayScale ) )
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
    if (!m_bVer67 && m_xPlcxMan && m_xPlcxMan->GetPapPLCF()->HasSprm(NS_sprm::sprmPShd).pSprm)
        return;

    if (nLen < 2)
    {
        // end of attribute
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_BACKGROUND );
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
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_BACKGROUND );
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
    if (nLen < 0)
    {
        if( m_bHasBorder )
        {
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_BOX );
            m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_SHADOW );
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
            nBorder = ::lcl_ReadBorders(m_bVer67, aBrcs, nullptr, m_xStyles.get());
        else
            nBorder = ::lcl_ReadBorders(m_bVer67, aBrcs, m_xPlcxMan ? m_xPlcxMan->GetPapPLCF() : nullptr);

        if( nBorder )                                   // Border
        {
            bool bIsB = IsBorder(aBrcs, true);
            if (!InLocalApo() || !bIsB || (m_xWFlyPara && !m_xWFlyPara->bBorderLines))
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

                tools::Rectangle aInnerDist;
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
    //if (!bVer67 && pPlcxMan && pPlcxMan->GetChpPLCF()->HasSprm(NS_sprm::sprmCBrc))
    //    return;

    if (nLen < 0)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_BOX );
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_CHRATR_SHADOW );
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
            int nBrcVer = (nId == NS_sprm::sprmCBrc) ? 9 : (m_bVer67 ? 6 : 8);

            SetWW8_BRC(nBrcVer, aBrc, pData, nLen);

            // Border style is none -> no border, no shadow
            if( editeng::ConvertBorderStyleFromWord(aBrc.brcType()) != SvxBorderLineStyle::NONE )
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
    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_HYPHENZONE );
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
    if (nLen < 1)
    {
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_WIDOWS );
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_ORPHANS );
    }
    else
    {
        sal_uInt8 nL = ( *pData & 1 ) ? 2 : 0;

        NewAttr( SvxWidowsItem( nL, RES_PARATR_WIDOWS ) );     // Off -> nLines = 0
        NewAttr( SvxOrphansItem( nL, RES_PARATR_ORPHANS ) );

        if( m_pAktColl && m_xStyles )           // Style-Def ?
            m_xStyles->bWidowsChanged = true; // save for simulation
                                            // Default-Widows
    }
}

void SwWW8ImplReader::Read_UsePgsuSettings(sal_uInt16,const sal_uInt8* pData,short nLen)
{
    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_SNAPTOGRID);
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
    if (nLen < 2)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_VERTALIGN);
    else
    {
        sal_uInt16 nVal = SVBT16ToShort( pData );
        SvxParaVertAlignItem::Align nAlign;
        switch (nVal)
        {
            case 0:
                nAlign = SvxParaVertAlignItem::Align::Top;
                break;
            case 1:
                nAlign = SvxParaVertAlignItem::Align::Center;
                break;
            case 2:
                nAlign = SvxParaVertAlignItem::Align::Baseline;
                break;
            case 3:
                nAlign = SvxParaVertAlignItem::Align::Bottom;
                break;
            case 4:
                nAlign = SvxParaVertAlignItem::Align::Automatic;
                break;
            default:
                nAlign = SvxParaVertAlignItem::Align::Automatic;
                OSL_ENSURE(false,"Unknown paragraph vertical align");
                break;
        }
        NewAttr( SvxParaVertAlignItem( nAlign, RES_PARATR_VERTALIGN ) );
    }
}

void SwWW8ImplReader::Read_KeepLines( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_PARATR_SPLIT );
    else
        NewAttr( SvxFormatSplitItem( ( *pData & 1 ) == 0, RES_PARATR_SPLIT ) );
}

void SwWW8ImplReader::Read_KeepParas( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_KEEP );
    else
        NewAttr( SvxFormatKeepItem( ( *pData & 1 ) != 0 , RES_KEEP) );
}

void SwWW8ImplReader::Read_BreakBefore( sal_uInt16, const sal_uInt8* pData, short nLen )
{
    if (nLen < 1)
        m_xCtrlStck->SetAttr( *m_pPaM->GetPoint(), RES_BREAK );
    else
        NewAttr( SvxFormatBreakItem(
                ( *pData & 1 ) ? SvxBreak::PageBefore : SvxBreak::NONE, RES_BREAK ) );
}

void SwWW8ImplReader::Read_ApoPPC( sal_uInt16, const sal_uInt8* pData, short )
{
    if (m_pAktColl && m_nAktColl < m_vColl.size()) // only for Styledef, otherwise solved differently
    {
        SwWW8StyInf& rSI = m_vColl[m_nAktColl];
        if (!rSI.m_xWWFly)
            rSI.m_xWWFly.reset(new WW8FlyPara(m_bVer67));
        rSI.m_xWWFly->Read(*pData, m_xStyles.get());
        if (rSI.m_xWWFly->IsEmpty())
        {
            m_vColl[m_nAktColl].m_xWWFly.reset();
        }
    }
}

bool SwWW8ImplReader::ParseTabPos(WW8_TablePos *pTabPos, WW8PLCFx_Cp_FKP* pPap)
{
    bool bRet = false;
    memset(pTabPos, 0, sizeof(WW8_TablePos));
    // sprmTPc contains a PositionCodeOperand structure that specifies the origin
    // that is used to calculate the table position when it is absolutely positioned
    SprmResult aRes = pPap->HasSprm(NS_sprm::sprmTPc);
    if (aRes.pSprm && aRes.nRemainingData >= 1)
    {
        pTabPos->nSp29 = *aRes.pSprm;
        pTabPos->nSp37 = 2;     //Possible fail area, always parallel wrap
        aRes = pPap->HasSprm(NS_sprm::sprmTDxaAbs);
        if (aRes.pSprm && aRes.nRemainingData >= 2)
            pTabPos->nSp26 = SVBT16ToShort(aRes.pSprm);
        aRes = pPap->HasSprm(NS_sprm::sprmTDyaAbs);
        if (aRes.pSprm && aRes.nRemainingData >= 2)
            pTabPos->nSp27 = SVBT16ToShort(aRes.pSprm);
        aRes = pPap->HasSprm(NS_sprm::sprmTDxaFromText);
        if (aRes.pSprm && aRes.nRemainingData >= 2)
            pTabPos->nLeMgn = SVBT16ToShort(aRes.pSprm);
        aRes = pPap->HasSprm(NS_sprm::sprmTDxaFromTextRight);
        if (aRes.pSprm && aRes.nRemainingData >= 2)
            pTabPos->nRiMgn = SVBT16ToShort(aRes.pSprm);
        aRes = pPap->HasSprm(NS_sprm::sprmTDyaFromText);
        if (aRes.pSprm && aRes.nRemainingData >= 2)
            pTabPos->nUpMgn = SVBT16ToShort(aRes.pSprm);
        aRes = pPap->HasSprm(NS_sprm::sprmTDyaFromTextBottom);
        if (aRes.pSprm && aRes.nRemainingData >= 2)
            pTabPos->nLoMgn = SVBT16ToShort(aRes.pSprm);
        pTabPos->bNoFly = !FloatingTableConversion(pPap);
        bRet = true;
    }
    return bRet;
}

// page attribute won't be used as attribute anymore
// ( except OLST )
long SwWW8ImplReader::ImportExtSprm(WW8PLCFManResult* pRes)
{
    // array for reading of the extended ( self-defined ) SPRMs
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

// arrays for reading the SPRMs

// function for reading of SPRMs. Par1: SprmId
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
          {0, nullptr},                              // "0" default resp. error
                                                     // will be skipped! ,
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

    static wwSprmDispatcher aSprmSrch(aSprms, SAL_N_ELEMENTS(aSprms));
    return &aSprmSrch;
}

const wwSprmDispatcher *GetWW6SprmDispatcher()
{
    static SprmReadInfo aSprms[] =
    {
        {0,                                 nullptr},                             // "0" default resp. error
                                                                                  // will be skipped!
        {NS_sprm::v6::sprmPIstd,            &SwWW8ImplReader::Read_StyleCode},    // pap.istd (style code)
        {NS_sprm::v6::sprmPIstdPermute,     nullptr},                             // pap.istd permutation
        {NS_sprm::v6::sprmPIncLv1,          nullptr},                             // pap.istddifference
        {NS_sprm::v6::sprmPJc,              &SwWW8ImplReader::Read_Justify},      // pap.jc (justification)
        {NS_sprm::v6::sprmPFSideBySide,     nullptr},                             // pap.fSideBySide
        {NS_sprm::v6::sprmPFKeep,           &SwWW8ImplReader::Read_KeepLines},    // pap.fKeep
        {NS_sprm::v6::sprmPFKeepFollow,     &SwWW8ImplReader::Read_KeepParas},    // pap.fKeepFollow
        {NS_sprm::v6::sprmPPageBreakBefore, &SwWW8ImplReader::Read_BreakBefore},  // pap.fPageBreakBefore
        {NS_sprm::v6::sprmPBrcl,            nullptr},                             // pap.brcl
        {NS_sprm::v6::sprmPBrcp,            nullptr},                             // pap.brcp
        {NS_sprm::v6::sprmPAnld,            &SwWW8ImplReader::Read_ANLevelDesc},  // pap.anld (ANLD structure)
        {NS_sprm::v6::sprmPNLvlAnm,         &SwWW8ImplReader::Read_ANLevelNo},    // pap.nLvlAnm nn
        {NS_sprm::v6::sprmPFNoLineNumb,     &SwWW8ImplReader::Read_NoLineNumb},   // ap.fNoLnn
        {NS_sprm::v6::sprmPChgTabsPapx,     &SwWW8ImplReader::Read_Tab},          // pap.itbdMac, ...
        {NS_sprm::v6::sprmPDxaRight,        &SwWW8ImplReader::Read_LR},           // pap.dxaRight
        {NS_sprm::v6::sprmPDxaLeft,         &SwWW8ImplReader::Read_LR},           // pap.dxaLeft
        {NS_sprm::v6::sprmPNest,            nullptr},                             // pap.dxaLeft
        {NS_sprm::v6::sprmPDxaLeft1,        &SwWW8ImplReader::Read_LR},           // pap.dxaLeft1
        {NS_sprm::v6::sprmPDyaLine,         &SwWW8ImplReader::Read_LineSpace},    // pap.lspd an LSPD
        {NS_sprm::v6::sprmPDyaBefore,       &SwWW8ImplReader::Read_UL},           // pap.dyaBefore
        {NS_sprm::v6::sprmPDyaAfter,        &SwWW8ImplReader::Read_UL},           // pap.dyaAfter
        {NS_sprm::v6::sprmPChgTabs,         nullptr},                             // pap.itbdMac, pap.rgdxaTab, ...
        {NS_sprm::v6::sprmPFInTable,        nullptr},                             // pap.fInTable
        {NS_sprm::v6::sprmPTtp,             &SwWW8ImplReader::Read_TabRowEnd},    // pap.fTtp
        {NS_sprm::v6::sprmPDxaAbs,          nullptr},                             // pap.dxaAbs
        {NS_sprm::v6::sprmPDyaAbs,          nullptr},                             // pap.dyaAbs
        {NS_sprm::v6::sprmPDxaWidth,        nullptr},                             // pap.dxaWidth
        {NS_sprm::v6::sprmPPc,              &SwWW8ImplReader::Read_ApoPPC},       // pap.pcHorz, pap.pcVert
        {NS_sprm::v6::sprmPBrcTop10,        nullptr},                             // pap.brcTop BRC10
        {NS_sprm::v6::sprmPBrcLeft10,       nullptr},                             // pap.brcLeft BRC10
        {NS_sprm::v6::sprmPBrcBottom10,     nullptr},                             // pap.brcBottom BRC10
        {NS_sprm::v6::sprmPBrcRight10,      nullptr},                             // pap.brcRight BRC10
        {NS_sprm::v6::sprmPBrcBetween10,    nullptr},                             // pap.brcBetween BRC10
        {NS_sprm::v6::sprmPBrcBar10,        nullptr},                             // pap.brcBar BRC10
        {NS_sprm::v6::sprmPFromText10,      nullptr},                             // pap.dxaFromText dxa
        {NS_sprm::v6::sprmPWr,              nullptr},                             // pap.wr wr
        {NS_sprm::v6::sprmPBrcTop,          &SwWW8ImplReader::Read_Border},       // pap.brcTop BRC
        {NS_sprm::v6::sprmPBrcLeft,         &SwWW8ImplReader::Read_Border},       // pap.brcLeft BRC
        {NS_sprm::v6::sprmPBrcBottom,       &SwWW8ImplReader::Read_Border},       // pap.brcBottom BRC
        {NS_sprm::v6::sprmPBrcRight,        &SwWW8ImplReader::Read_Border},       // pap.brcRight BRC
        {NS_sprm::v6::sprmPBrcBetween,      &SwWW8ImplReader::Read_Border},       // pap.brcBetween BRC
        {NS_sprm::v6::sprmPBrcBar,          nullptr},                             // pap.brcBar BRC word
        {NS_sprm::v6::sprmPFNoAutoHyph,     &SwWW8ImplReader::Read_Hyphenation},  // pap.fNoAutoHyph
        {NS_sprm::v6::sprmPWHeightAbs,      nullptr},                             // pap.wHeightAbs w
        {NS_sprm::v6::sprmPDcs,             nullptr},                             // pap.dcs DCS
        {NS_sprm::v6::sprmPShd,             &SwWW8ImplReader::Read_Shade},        // pap.shd SHD
        {NS_sprm::v6::sprmPDyaFromText,     nullptr},                             // pap.dyaFromText dya
        {NS_sprm::v6::sprmPDxaFromText,     nullptr},                             // pap.dxaFromText dxa
        {NS_sprm::v6::sprmPFLocked,         nullptr},                             // pap.fLocked 0 or 1 byte
        {NS_sprm::v6::sprmPFWidowControl,   &SwWW8ImplReader::Read_WidowControl}, // pap.fWidowControl 0 or 1 byte
        {NS_sprm::v6::sprmPRuler,           nullptr},
        {53,                                nullptr},                             //"??53",
        {54,                                nullptr},                             //"??54",
        {55,                                nullptr},                             //"??55",
        {56,                                nullptr},                             //"??56",
        {57,                                nullptr},                             //"??57",
        {58,                                nullptr},                             //"??58",
        {59,                                nullptr},                             //"??59",
        {60,                                nullptr},                             //"??60",
        {61,                                nullptr},                             //"??61",
        {62,                                nullptr},                             //"??62",
        {63,                                nullptr},                             //"??63",
        {64,                                &SwWW8ImplReader::Read_ParaBiDi},     //"rtl bidi ?
        {NS_sprm::v6::sprmCFStrikeRM,       &SwWW8ImplReader::Read_CFRMarkDel},   // chp.fRMarkDel 1 or 0 bit
        {NS_sprm::v6::sprmCFRMark,          &SwWW8ImplReader::Read_CFRMark},      // chp.fRMark 1 or 0 bit
        {NS_sprm::v6::sprmCFFldVanish,      &SwWW8ImplReader::Read_FieldVanish},  // chp.fFieldVanish 1 or 0 bit
        {NS_sprm::v6::sprmCPicLocation,     &SwWW8ImplReader::Read_PicLoc},       // chp.fcPic and chp.fSpec
        {NS_sprm::v6::sprmCIbstRMark,       nullptr},                             // chp.ibstRMark index into sttbRMark
        {NS_sprm::v6::sprmCDttmRMark,       nullptr},                             // chp.dttm DTTM long
        {NS_sprm::v6::sprmCFData,           nullptr},                             // chp.fData 1 or 0 bit
        {NS_sprm::v6::sprmCRMReason,        nullptr},                             // chp.idslRMReason an index to a table
        {NS_sprm::v6::sprmCChse,            &SwWW8ImplReader::Read_CharSet},      // chp.fChsDiff and chp.chse 3 bytes
        {NS_sprm::v6::sprmCSymbol,          &SwWW8ImplReader::Read_Symbol},       // chp.fSpec, chp.chSym and chp.ftcSym
        {NS_sprm::v6::sprmCFOle2,           &SwWW8ImplReader::Read_Obj},          // chp.fOle2 1 or 0 bit
        {76,                                nullptr},                             //"??76",
        {77,                                nullptr},                             //"??77",
        {78,                                nullptr},                             //"??78",
        {79,                                nullptr},                             //"??79",
        {NS_sprm::v6::sprmCIstd,            &SwWW8ImplReader::Read_CColl},        // chp.istd istd, see stylesheet definition; short
        {NS_sprm::v6::sprmCIstdPermute,     nullptr},                             // chp.istd permutation vector
        {NS_sprm::v6::sprmCDefault,         nullptr},                             // whole CHP none variable length
        {NS_sprm::v6::sprmCPlain,           nullptr},                             // whole CHP none 0
        {84,                                nullptr},                             //"??84",
        {NS_sprm::v6::sprmCFBold,           &SwWW8ImplReader::Read_BoldUsw},      // chp.fBold 0,1, 128, or 129 byte
        {NS_sprm::v6::sprmCFItalic,         &SwWW8ImplReader::Read_BoldUsw},      // chp.fItalic 0,1, 128, or 129 byte
        {NS_sprm::v6::sprmCFStrike,         &SwWW8ImplReader::Read_BoldUsw},      // chp.fStrike 0,1, 128, or 129 byte
        {NS_sprm::v6::sprmCFOutline,        &SwWW8ImplReader::Read_BoldUsw},      // chp.fOutline 0,1, 128, or 129 byte
        {NS_sprm::v6::sprmCFShadow,         &SwWW8ImplReader::Read_BoldUsw},      // chp.fShadow 0,1, 128, or 129 byte
        {NS_sprm::v6::sprmCFSmallCaps,      &SwWW8ImplReader::Read_BoldUsw},      // chp.fSmallCaps 0,1, 128, or 129 byte
        {NS_sprm::v6::sprmCFCaps,           &SwWW8ImplReader::Read_BoldUsw},      // chp.fCaps 0,1, 128, or 129 byte
        {NS_sprm::v6::sprmCFVanish,         &SwWW8ImplReader::Read_BoldUsw},      // chp.fVanish 0,1, 128, or 129 byte
        {NS_sprm::v6::sprmCFtc,             &SwWW8ImplReader::Read_FontCode},     // chp.ftc ftc word
        {NS_sprm::v6::sprmCKul,             &SwWW8ImplReader::Read_Underline},    // chp.kul kul byte
        {NS_sprm::v6::sprmCSizePos,         nullptr},                             // chp.hps, chp.hpsPos 3 bytes
        {NS_sprm::v6::sprmCDxaSpace,        &SwWW8ImplReader::Read_Kern},         // chp.dxaSpace dxa word
        {NS_sprm::v6::sprmCLid,             &SwWW8ImplReader::Read_Language},     // chp.lid LID word
        {NS_sprm::v6::sprmCIco,             &SwWW8ImplReader::Read_TextColor},    // chp.ico ico byte
        {NS_sprm::v6::sprmCHps,             &SwWW8ImplReader::Read_FontSize},     // chp.hps hps word!
        {NS_sprm::v6::sprmCHpsInc,          nullptr},                             // chp.hps byte
        {NS_sprm::v6::sprmCHpsPos,          &SwWW8ImplReader::Read_SubSuperProp}, // chp.hpsPos hps byte
        {NS_sprm::v6::sprmCHpsPosAdj,       nullptr},                             // chp.hpsPos hps byte
        {NS_sprm::v6::sprmCMajority,        &SwWW8ImplReader::Read_Majority},     // chp.fBold, chp.fItalic, chp.fSmallCaps
        {NS_sprm::v6::sprmCIss,             &SwWW8ImplReader::Read_SubSuper},     // chp.iss iss byte
        {NS_sprm::v6::sprmCHpsNew50,        nullptr},                             // chp.hps hps variable width, length always recorded as 2
        {NS_sprm::v6::sprmCHpsInc1,         nullptr},                             // chp.hps complex variable width, length always recorded as 2
        {NS_sprm::v6::sprmCHpsKern,         &SwWW8ImplReader::Read_FontKern},     // chp.hpsKern hps short
        {NS_sprm::v6::sprmCMajority50,      &SwWW8ImplReader::Read_Majority},     // chp.fBold, chp.fItalic, chp.fSmallCaps, chp.fVanish, ...
        {NS_sprm::v6::sprmCHpsMul,          nullptr},                             // chp.hps percentage to grow hps short
        {NS_sprm::v6::sprmCCondHyhen,       nullptr},                             // chp.ysri ysri short
        {111,                               &SwWW8ImplReader::Read_AmbiguousSPRM},//sprmCFBoldBi or font code
        {112,                               &SwWW8ImplReader::Read_AmbiguousSPRM},//sprmCFItalicBi or font code
        {113,                               &SwWW8ImplReader::Read_FontCode},     //sprmCFtcBi
        {114,                               &SwWW8ImplReader::Read_Language},     //sprmClidBi
        {115,                               &SwWW8ImplReader::Read_TextColor},    //sprmCIcoBi
        {116,                               &SwWW8ImplReader::Read_FontSize},     //sprmCHpsBi
        {NS_sprm::v6::sprmCFSpec,           &SwWW8ImplReader::Read_Special},      // chp.fSpec 1 or 0 bit
        {NS_sprm::v6::sprmCFObj,            &SwWW8ImplReader::Read_Obj},          // chp.fObj 1 or 0 bit
        {NS_sprm::v6::sprmPicBrcl,          nullptr},                             // pic.brcl brcl (see PIC structure definition) byte
        {NS_sprm::v6::sprmPicScale,         nullptr},                             // pic.mx, pic.my, pic.dxaCropleft,
        {NS_sprm::v6::sprmPicBrcTop,        nullptr},                             // pic.brcTop BRC word
        {NS_sprm::v6::sprmPicBrcLeft,       nullptr},                             // pic.brcLeft BRC word
        {NS_sprm::v6::sprmPicBrcBottom,     nullptr},                             // pic.brcBottom BRC word
        {NS_sprm::v6::sprmPicBrcRight,      nullptr},                             // pic.brcRight BRC word
        {125,                               nullptr},                             //"??125",
        {126,                               nullptr},                             //"??126",
        {127,                               nullptr},                             //"??127",
        {128,                               nullptr},                             //"??128",
        {129,                               nullptr},                             //"??129",
        {130,                               nullptr},                             //"??130",
        {NS_sprm::v6::sprmSScnsPgn,         nullptr},                             // sep.cnsPgn cns byte
        {NS_sprm::v6::sprmSiHeadingPgn,     nullptr},                             // sep.iHeadingPgn heading number level byte
        {NS_sprm::v6::sprmSOlstAnm,         &SwWW8ImplReader::Read_OLST},         // sep.olstAnm OLST variable length
        {134,                               nullptr},                             //"??135",
        {135,                               nullptr},                             //"??135",
        {NS_sprm::v6::sprmSDxaColWidth,     nullptr},                             // sep.rgdxaColWidthSpacing complex 3 bytes
        {NS_sprm::v6::sprmSDxaColSpacing,   nullptr},                             // sep.rgdxaColWidthSpacing complex 3 bytes
        {NS_sprm::v6::sprmSFEvenlySpaced,   nullptr},                             // sep.fEvenlySpaced 1 or 0 byte
        {NS_sprm::v6::sprmSFProtected,      nullptr},                             // sep.fUnlocked 1 or 0 byte
        {NS_sprm::v6::sprmSDmBinFirst,      nullptr},                             // sep.dmBinFirst  word
        {NS_sprm::v6::sprmSDmBinOther,      nullptr},                             // sep.dmBinOther  word
        {NS_sprm::v6::sprmSBkc,             nullptr},                             // sep.bkc bkc byte BreakCode
        {NS_sprm::v6::sprmSFTitlePage,      nullptr},                             // sep.fTitlePage 0 or 1 byte
        {NS_sprm::v6::sprmSCcolumns,        nullptr},                             // sep.ccolM1 # of cols - 1 word
        {NS_sprm::v6::sprmSDxaColumns,      nullptr},                             // sep.dxaColumns dxa word
        {NS_sprm::v6::sprmSFAutoPgn,        nullptr},                             // sep.fAutoPgn obsolete byte
        {NS_sprm::v6::sprmSNfcPgn,          nullptr},                             // sep.nfcPgn nfc byte
        {NS_sprm::v6::sprmSDyaPgn,          nullptr},                             // sep.dyaPgn dya short
        {NS_sprm::v6::sprmSDxaPgn,          nullptr},                             // sep.dxaPgn dya short
        {NS_sprm::v6::sprmSFPgnRestart,     nullptr},                             // sep.fPgnRestart 0 or 1 byte
        {NS_sprm::v6::sprmSFEndnote,        nullptr},                             // sep.fEndnote 0 or 1 byte
        {NS_sprm::v6::sprmSLnc,             nullptr},                             // sep.lnc lnc byte
        {NS_sprm::v6::sprmSGprfIhdt,        nullptr},                             // sep.grpfIhdt grpfihdt byte
        {NS_sprm::v6::sprmSNLnnMod,         nullptr},                             // sep.nLnnMod non-neg int. word
        {NS_sprm::v6::sprmSDxaLnn,          nullptr},                             // sep.dxaLnn dxa word
        {NS_sprm::v6::sprmSDyaHdrTop,       nullptr},                             // sep.dyaHdrTop dya word
        {NS_sprm::v6::sprmSDyaHdrBottom,    nullptr},                             // sep.dyaHdrBottom dya word
        {NS_sprm::v6::sprmSLBetween,        nullptr},                             // sep.fLBetween 0 or 1 byte
        {NS_sprm::v6::sprmSVjc,             nullptr},                             // sep.vjc vjc byte
        {NS_sprm::v6::sprmSLnnMin,          nullptr},                             // sep.lnnMin lnn word
        {NS_sprm::v6::sprmSPgnStart,        nullptr},                             // sep.pgnStart pgn word
        {NS_sprm::v6::sprmSBOrientation,    nullptr},                             // sep.dmOrientPage dm byte
        {NS_sprm::v6::sprmSBCustomize,      nullptr},                             // ?
        {NS_sprm::v6::sprmSXaPage,          nullptr},                             // sep.xaPage xa word
        {NS_sprm::v6::sprmSYaPage,          nullptr},                             // sep.yaPage ya word
        {NS_sprm::v6::sprmSDxaLeft,         nullptr},                             // sep.dxaLeft dxa word
        {NS_sprm::v6::sprmSDxaRight,        nullptr},                             // sep.dxaRight dxa word
        {NS_sprm::v6::sprmSDyaTop,          nullptr},                             // sep.dyaTop dya word
        {NS_sprm::v6::sprmSDyaBottom,       nullptr},                             // sep.dyaBottom dya word
        {NS_sprm::v6::sprmSDzaGutter,       nullptr},                             // sep.dzaGutter dza word
        {NS_sprm::v6::sprmSDMPaperReq,      nullptr},                             // sep.dmPaperReq dm word
        {172,                               nullptr},                             //"??172",
        {173,                               nullptr},                             //"??173",
        {174,                               nullptr},                             //"??174",
        {175,                               nullptr},                             //"??175",
        {176,                               nullptr},                             //"??176",
        {177,                               nullptr},                             //"??177",
        {178,                               nullptr},                             //"??178",
        {179,                               nullptr},                             //"??179",
        {180,                               nullptr},                             //"??180",
        {181,                               nullptr},                             //"??181",
        {NS_sprm::v6::sprmTJc,              nullptr},                             // tap.jc jc word (low order byte is significant)
        {NS_sprm::v6::sprmTDxaLeft,         nullptr},                             // tap.rgdxaCenter dxa word
        {NS_sprm::v6::sprmTDxaGapHalf,      nullptr},                             // tap.dxaGapHalf, tap.rgdxaCenter dxa word
        {NS_sprm::v6::sprmTFCantSplit,      nullptr},                             // tap.fCantSplit 1 or 0 byte
        {NS_sprm::v6::sprmTTableHeader,     nullptr},                             // tap.fTableHeader 1 or 0 byte
        {NS_sprm::v6::sprmTTableBorders,    nullptr},                             // tap.rgbrcTable complex 12 bytes
        {NS_sprm::v6::sprmTDefTable10,      nullptr},                             // tap.rgdxaCenter, tap.rgtc complex variable length
        {NS_sprm::v6::sprmTDyaRowHeight,    nullptr},                             // tap.dyaRowHeight dya word
        {NS_sprm::v6::sprmTDefTable,        nullptr},                             // tap.rgtc complex
        {NS_sprm::v6::sprmTDefTableShd,     nullptr},                             // tap.rgshd complex
        {NS_sprm::v6::sprmTTlp,             nullptr},                             // tap.tlp TLP 4 bytes
        {NS_sprm::v6::sprmTSetBrc,          nullptr},                             // tap.rgtc[].rgbrc complex 5 bytes
        {NS_sprm::v6::sprmTInsert,          nullptr},                             // tap.rgdxaCenter, tap.rgtc complex 4 bytes
        {NS_sprm::v6::sprmTDelete,          nullptr},                             // tap.rgdxaCenter, tap.rgtc complex word
        {NS_sprm::v6::sprmTDxaCol,          nullptr},                             // tap.rgdxaCenter complex 4 bytes
        {NS_sprm::v6::sprmTMerge,           nullptr},                             // tap.fFirstMerged, tap.fMerged complex word
        {NS_sprm::v6::sprmTSplit,           nullptr},                             // tap.fFirstMerged, tap.fMerged complex word
        {NS_sprm::v6::sprmTSetBrc10,        nullptr},                             // tap.rgtc[].rgbrc complex 5 bytes
        {NS_sprm::v6::sprmTSetShd,          nullptr},                             // tap.rgshd complex 4 bytes
        {207,                               nullptr},                             //dunno
    };

    static wwSprmDispatcher aSprmSrch(aSprms, SAL_N_ELEMENTS(aSprms));
    return &aSprmSrch;
}

const wwSprmDispatcher *GetWW8SprmDispatcher()
{
    static SprmReadInfo aSprms[] =
    {
        {0,                              nullptr},                                       // "0" default resp. error

        {NS_sprm::sprmPIstd,             &SwWW8ImplReader::Read_StyleCode},              // pap.istd;istd (style code);short;
        {NS_sprm::sprmPIstdPermute,      nullptr},                                       // pap.istd;permutation vector;
                                                                                         // variable length;
        {NS_sprm::sprmPIncLvl,           nullptr},                                       // pap.istd, pap.lvl;difference between
                                                                                         // istd of base PAP and istd of
                                                                                         // PAP to be produced;byte;
        {NS_sprm::sprmPJc80,             &SwWW8ImplReader::Read_Justify},                // pap.jc;jc (justification);byte;
        {NS_sprm::LN_PFSideBySide,       nullptr},                                       // pap.fSideBySide;0 or 1;byte;
        {NS_sprm::sprmPFKeep,            &SwWW8ImplReader::Read_KeepLines},              // pap.fKeep;0 or 1;byte;
        {NS_sprm::sprmPFKeepFollow,      &SwWW8ImplReader::Read_KeepParas},              // pap.fKeepFollow;0 or 1;byte;
        {NS_sprm::sprmPFPageBreakBefore, &SwWW8ImplReader::Read_BreakBefore},            // pap.fPageBreakBefore;0 or 1;byte;
        {NS_sprm::LN_PBrcl,              nullptr},                                       // pap.brcl;brcl;byte;
        {NS_sprm::LN_PBrcp,              nullptr},                                       // pap.brcp;brcp;byte;
        {NS_sprm::sprmPIlvl,             &SwWW8ImplReader::Read_ListLevel},              // pap.ilvl;ilvl;byte;
        {NS_sprm::sprmPIlfo,             &SwWW8ImplReader::Read_LFOPosition},            // pap.ilfo;ilfo (list index);short;
        {NS_sprm::sprmPFNoLineNumb,      &SwWW8ImplReader::Read_NoLineNumb},             // pap.fNoLnn;0 or 1;byte;
        {NS_sprm::sprmPChgTabsPapx,      &SwWW8ImplReader::Read_Tab},                    // pap.itbdMac, pap.rgdxaTab, pap.rgtbd;
                                                                                         // complex;variable length
        {NS_sprm::sprmPDxaRight80,       &SwWW8ImplReader::Read_LR},                     // pap.dxaRight;dxa;word;
        {NS_sprm::sprmPDxaLeft80,        &SwWW8ImplReader::Read_LR},                     // pap.dxaLeft;dxa;word;
        {NS_sprm::sprmPNest80,           nullptr},                                       // pap.dxaLeft;dxa;word;
        {NS_sprm::sprmPDxaLeft180,       &SwWW8ImplReader::Read_LR},                     // pap.dxaLeft1;dxa;word;
        {NS_sprm::sprmPDyaLine,          &SwWW8ImplReader::Read_LineSpace},              // pap.lspd;an LSPD, a long word
                                                                                         // structure consisting of a short
                                                                                         // of dyaLine followed by a short
                                                                                         // of fMultLinespace;long;
        {NS_sprm::sprmPDyaBefore,        &SwWW8ImplReader::Read_UL},                     // pap.dyaBefore;dya;word;
        {NS_sprm::sprmPDyaAfter,         &SwWW8ImplReader::Read_UL},                     // pap.dyaAfter;dya;word;
        {NS_sprm::sprmPChgTabs,          nullptr},                                       // pap.itbdMac, pap.rgdxaTab, pap.rgtbd;
                                                                                         // complex;variable length;
        {NS_sprm::sprmPFInTable,         nullptr},                                       // pap.fInTable;0 or 1;byte;
        {NS_sprm::sprmPFTtp,             &SwWW8ImplReader::Read_TabRowEnd},              // pap.fTtp;0 or 1;byte;
        {NS_sprm::sprmPDxaAbs,           nullptr},                                       // pap.dxaAbs;dxa;word;
        {NS_sprm::sprmPDyaAbs,           nullptr},                                       // pap.dyaAbs;dya;word;
        {NS_sprm::sprmPDxaWidth,         nullptr},                                       // pap.dxaWidth;dxa;word;
        {NS_sprm::sprmPPc,               &SwWW8ImplReader::Read_ApoPPC},                 // pap.pcHorz, pap.pcVert;complex;byte;
        {NS_sprm::LN_PBrcTop10,          nullptr},                                       // pap.brcTop;BRC10;word;
        {NS_sprm::LN_PBrcLeft10,         nullptr},                                       // pap.brcLeft;BRC10;word;
        {NS_sprm::LN_PBrcBottom10,       nullptr},                                       // pap.brcBottom;BRC10;word;
        {NS_sprm::LN_PBrcRight10,        nullptr},                                       // pap.brcRight;BRC10;word;
        {NS_sprm::LN_PBrcBetween10,      nullptr},                                       // pap.brcBetween;BRC10;word;
        {NS_sprm::LN_PBrcBar10,          nullptr},                                       // pap.brcBar;BRC10;word;
        {NS_sprm::LN_PDxaFromText10,     nullptr},                                       // pap.dxaFromText;dxa;word;
        {NS_sprm::sprmPWr,               nullptr},                                       // pap.wr;wr;byte;
        {NS_sprm::sprmPBrcTop80,         &SwWW8ImplReader::Read_Border},                 // pap.brcTop;BRC;long;
        {NS_sprm::sprmPBrcLeft80,        &SwWW8ImplReader::Read_Border},                 // pap.brcLeft;BRC;long;
        {NS_sprm::sprmPBrcBottom80,      &SwWW8ImplReader::Read_Border},                 // pap.brcBottom;BRC;long;
        {NS_sprm::sprmPBrcRight80,       &SwWW8ImplReader::Read_Border},                 // pap.brcRight;BRC;long;
        {NS_sprm::sprmPBrcBetween80,     &SwWW8ImplReader::Read_Border},                 // pap.brcBetween;BRC;long;
        {NS_sprm::sprmPBrcBar80,         nullptr},                                       // pap.brcBar;BRC;long;
        {NS_sprm::sprmPFNoAutoHyph,      &SwWW8ImplReader::Read_Hyphenation},            // pap.fNoAutoHyph;0 or 1;byte;
        {NS_sprm::sprmPWHeightAbs,       nullptr},                                       // pap.wHeightAbs;w;word;
        {NS_sprm::sprmPDcs,              nullptr},                                       // pap.dcs;DCS;short;
        {NS_sprm::sprmPShd80,            &SwWW8ImplReader::Read_Shade},                  // pap.shd;SHD;word;
        {NS_sprm::sprmPDyaFromText,      nullptr},                                       // pap.dyaFromText;dya;word;
        {NS_sprm::sprmPDxaFromText,      nullptr},                                       // pap.dxaFromText;dxa;word;
        {NS_sprm::sprmPFLocked,          nullptr},                                       // pap.fLocked;0 or 1;byte;
        {NS_sprm::sprmPFWidowControl,    &SwWW8ImplReader::Read_WidowControl},           // pap.fWidowControl;0 or 1;byte
        {NS_sprm::LN_PRuler,             nullptr},                                       // variable length;
        {NS_sprm::sprmPFKinsoku,         &SwWW8ImplReader::Read_BoolItem},               // pap.fKinsoku;0 or 1;byte;
        {NS_sprm::sprmPFWordWrap,        nullptr},                                       // pap.fWordWrap;0 or 1;byte;
        {NS_sprm::sprmPFOverflowPunct,   &SwWW8ImplReader::Read_BoolItem},               // pap.fOverflowPunct; 0 or 1;byte;
        {NS_sprm::sprmPFTopLinePunct,    nullptr},                                       // pap.fTopLinePunct;0 or 1;byte
        {NS_sprm::sprmPFAutoSpaceDE,     &SwWW8ImplReader::Read_BoolItem},               // pap.fAutoSpaceDE;0 or 1;byte;
        {NS_sprm::sprmPFAutoSpaceDN,     nullptr},                                       // pap.fAutoSpaceDN;0 or 1;byte;
        {NS_sprm::sprmPWAlignFont,       &SwWW8ImplReader::Read_AlignFont},              // pap.wAlignFont;iFa;word;
        {NS_sprm::sprmPFrameTextFlow,    nullptr},                                       // pap.fVertical pap.fBackward
                                                                                         // pap.fRotateFont;complex; word
        {NS_sprm::LN_PISnapBaseLine,     nullptr},                                       // obsolete, not applicable in
                                                                                         // Word97 and later versions;;byte;
        {NS_sprm::LN_PAnld,              &SwWW8ImplReader::Read_ANLevelDesc},            // pap.anld;;variable length;
        {NS_sprm::LN_PPropRMark,         nullptr},                                       // pap.fPropRMark;complex;
                                                                                         // variable length;
        {NS_sprm::sprmPOutLvl,           &SwWW8ImplReader::Read_POutLvl},                // pap.lvl;has no effect if pap.istd
                                                                                         // is < 1 or is > 9;byte;
        {NS_sprm::sprmPFBiDi,            &SwWW8ImplReader::Read_ParaBiDi},               // ;;byte;
        {NS_sprm::sprmPFNumRMIns,        nullptr},                                       // pap.fNumRMIns;1 or 0;bit;
        {NS_sprm::LN_PCrLf,              nullptr},                                       // ;;byte;
        {NS_sprm::sprmPNumRM,            nullptr},                                       // pap.numrm;;variable length;
        {NS_sprm::LN_PHugePapx,          nullptr},                                       // ;fc in the data stream to locate
                                                                                         // the huge grpprl;long;
        {NS_sprm::sprmPHugePapx,         nullptr},                                       // ;fc in the data stream to locate
                                                                                         // the huge grpprl;long;
        {NS_sprm::sprmPFUsePgsuSettings, &SwWW8ImplReader::Read_UsePgsuSettings},        // pap.fUsePgsuSettings;1 or 0;byte;
        {NS_sprm::sprmPFAdjustRight,     nullptr},                                       // pap.fAdjustRight;1 or 0;byte;
        {NS_sprm::sprmCFRMarkDel,        &SwWW8ImplReader::Read_CFRMarkDel},             // chp.fRMarkDel;1 or 0;bit;
        {NS_sprm::sprmCFRMarkIns,        &SwWW8ImplReader::Read_CFRMark},                // chp.fRMark;1 or 0;bit;
        {NS_sprm::sprmCFFldVanish,       &SwWW8ImplReader::Read_FieldVanish},            // chp.fFieldVanish;1 or 0;bit;
        {NS_sprm::sprmCPicLocation,      &SwWW8ImplReader::Read_PicLoc},                 // chp.fcPic and chp.fSpec;variable
                                                                                         // length, length recorded is always 4;
        {NS_sprm::sprmCIbstRMark,        nullptr},                                       // chp.ibstRMark;index into
                                                                                         // sttbRMark;short;
        {NS_sprm::sprmCDttmRMark,        nullptr},                                       // chp.dttmRMark;DTTM;long;
        {NS_sprm::sprmCFData,            nullptr},                                       // chp.fData;1 or 0;bit;
        {NS_sprm::sprmCIdslRMark,        nullptr},                                       // chp.idslRMReason;an index to
                                                                                         // a table of strings defined in
                                                                                         // Word 6.0 executables;short;
        {NS_sprm::LN_CChs,               &SwWW8ImplReader::Read_CharSet},                // chp.fChsDiff and chp.chse;3 bytes;
        {NS_sprm::sprmCSymbol,           &SwWW8ImplReader::Read_Symbol},                 // chp.fSpec, chp.xchSym and chp.ftcSym;
                                                                                         // variable length, length
                                                                                         // recorded is always 4;
        {NS_sprm::sprmCFOle2,            &SwWW8ImplReader::Read_Obj},                    // chp.fOle2;1 or 0;bit;
      //NS_sprm::LN_CIdCharType,                                                         // obsolete: not applicable in Word97
                                                                                         // and later versions
        {NS_sprm::sprmCHighlight,        &SwWW8ImplReader::Read_CharHighlight},          // chp.fHighlight, chp.icoHighlight;ico
                                                                                         // (fHighlight is set to 1 iff
                                                                                         // ico is not 0);byte;
        {NS_sprm::LN_CObjLocation,       &SwWW8ImplReader::Read_PicLoc},                 // chp.fcObj;FC;long;
      //NS_sprm::LN_CFFtcAsciSymb, ? ? ?,
        {NS_sprm::sprmCIstd,             &SwWW8ImplReader::Read_CColl},                  // chp.istd;istd,short;
        {NS_sprm::sprmCIstdPermute,      nullptr},                                       // chp.istd;permutation vector;
                                                                                         // variable length;
        {NS_sprm::LN_CDefault,           nullptr},                                       // whole CHP;none;variable length;
        {NS_sprm::sprmCPlain,            nullptr},                                       // whole CHP;none;length: 0;
        {NS_sprm::sprmCKcd,              &SwWW8ImplReader::Read_Emphasis},
        {NS_sprm::sprmCFBold,            &SwWW8ImplReader::Read_BoldUsw},                // chp.fBold;0,1, 128, or 129;byte;
        {NS_sprm::sprmCFItalic,          &SwWW8ImplReader::Read_BoldUsw},                // chp.fItalic;0,1, 128, or 129; byte;
        {NS_sprm::sprmCFStrike,          &SwWW8ImplReader::Read_BoldUsw},                // chp.fStrike;0,1, 128, or 129; byte;
        {NS_sprm::sprmCFOutline,         &SwWW8ImplReader::Read_BoldUsw},                // chp.fOutline;0,1, 128, or 129; byte;
        {NS_sprm::sprmCFShadow,          &SwWW8ImplReader::Read_BoldUsw},                // chp.fShadow;0,1, 128, or 129; byte;
        {NS_sprm::sprmCFSmallCaps,       &SwWW8ImplReader::Read_BoldUsw},                // chp.fSmallCaps;0,1, 128, or 129;byte;
        {NS_sprm::sprmCFCaps,            &SwWW8ImplReader::Read_BoldUsw},                // chp.fCaps;0,1, 128, or 129; byte;
        {NS_sprm::sprmCFVanish,          &SwWW8ImplReader::Read_BoldUsw},                // chp.fVanish;0,1, 128, or 129; byte;
      //NS_sprm::LN_CFtcDefault, 0,                                                      // ftc, only used internally, never
                                                                                         // stored in file;word;
        {NS_sprm::sprmCKul,              &SwWW8ImplReader::Read_Underline},              // chp.kul;kul;byte;
        {NS_sprm::LN_CSizePos,           nullptr},                                       // chp.hps, chp.hpsPos;3 bytes;
        {NS_sprm::sprmCDxaSpace,         &SwWW8ImplReader::Read_Kern},                   // chp.dxaSpace;dxa;word;
        {NS_sprm::LN_CLid,               &SwWW8ImplReader::Read_Language},               // ;only used internally, never stored;
                                                                                         // word;
        {NS_sprm::sprmCIco,              &SwWW8ImplReader::Read_TextColor},              // chp.ico;ico;byte;
        {NS_sprm::sprmCHps,              &SwWW8ImplReader::Read_FontSize},               // chp.hps;hps;byte;
        {NS_sprm::LN_CHpsInc,            nullptr},                                       // chp.hps;byte;
        {NS_sprm::sprmCHpsPos,           &SwWW8ImplReader::Read_SubSuperProp},           // chp.hpsPos;hps;byte;
        {NS_sprm::LN_CHpsPosAdj,         nullptr},                                       // chp.hpsPos;hps;byte;
        {NS_sprm::sprmCMajority,         &SwWW8ImplReader::Read_Majority},               // chp.fBold, chp.fItalic, chp.fStrike,
                                                                                         // chp.fSmallCaps, chp.fVanish, chp.fCaps,
                                                                                         // chp.hps, chp.hpsPos, chp.dxaSpace,
                                                                                         // chp.kul, chp.ico, chp.rgftc, chp.rglid;
                                                                                         // complex;variable length, length byte
                                                                                         // plus size of following grpprl;
        {NS_sprm::sprmCIss,              &SwWW8ImplReader::Read_SubSuper},               // chp.iss;iss;byte;
        {NS_sprm::LN_CHpsNew50,          nullptr},                                       // chp.hps;hps;variable width, length
                                                                                         // always recorded as 2;
        {NS_sprm::LN_CHpsInc1,           nullptr},                                       // chp.hps;complex; variable width,
                                                                                         // length always recorded as 2;
        {NS_sprm::sprmCHpsKern,          &SwWW8ImplReader::Read_FontKern},               // chp.hpsKern;hps;short;
        {NS_sprm::LN_CMajority50,        &SwWW8ImplReader::Read_Majority},               // chp.fBold, chp.fItalic, chp.fStrike,
                                                                                         // chp.fSmallCaps, chp.fVanish, chp.fCaps,
                                                                                         // chp.ftc, chp.hps, chp.hpsPos, chp.kul,
                                                                                         // chp.dxaSpace, chp.ico;complex;
                                                                                         // variable length;
        {NS_sprm::LN_CHpsMul,            nullptr},                                       // chp.hps;percentage to grow hps;short;
        {NS_sprm::sprmCHresi,            nullptr},                                       // ???? "sprmCYsri" chp.ysri;ysri;short;
        {NS_sprm::sprmCRgFtc0,           &SwWW8ImplReader::Read_FontCode},               // chp.rgftc[0];ftc for ASCII text;short;
        {NS_sprm::sprmCRgFtc1,           &SwWW8ImplReader::Read_FontCode},               // chp.rgftc[1];ftc for Far East text;
                                                                                         // short;
        {NS_sprm::sprmCRgFtc2,           &SwWW8ImplReader::Read_FontCode},               // chp.rgftc[2];ftc for non-Far East text;
                                                                                         // short;
        {NS_sprm::sprmCCharScale,        &SwWW8ImplReader::Read_ScaleWidth},
        {NS_sprm::sprmCFDStrike,         &SwWW8ImplReader::Read_BoldUsw},                // chp.fDStrike;;byte;
        {NS_sprm::sprmCFImprint,         &SwWW8ImplReader::Read_Relief},                 // chp.fImprint;1 or 0;bit;
        {NS_sprm::sprmCFSpec,            &SwWW8ImplReader::Read_Special},                // chp.fSpec;1 or 0;bit;
        {NS_sprm::sprmCFObj,             &SwWW8ImplReader::Read_Obj},                    // chp.fObj;1 or 0;bit;
        {NS_sprm::sprmCPropRMark90,      &SwWW8ImplReader::Read_CPropRMark},             // chp.fPropRMark, chp.ibstPropRMark,
                                                                                         // chp.dttmPropRMark;Complex;variable
                                                                                         // length always recorded as 7 bytes;
        {NS_sprm::sprmCFEmboss,          &SwWW8ImplReader::Read_Relief},                 // chp.fEmboss;1 or 0;bit;
        {NS_sprm::sprmCSfxText,          &SwWW8ImplReader::Read_TextAnim},               // chp.sfxtText;text animation;byte;
        {NS_sprm::sprmCFBiDi,            &SwWW8ImplReader::Read_Bidi},
        {NS_sprm::LN_CFDiacColor,        nullptr},
        {NS_sprm::sprmCFBoldBi,          &SwWW8ImplReader::Read_BoldBiDiUsw},
        {NS_sprm::sprmCFItalicBi,        &SwWW8ImplReader::Read_BoldBiDiUsw},
        {NS_sprm::sprmCFtcBi,            &SwWW8ImplReader::Read_FontCode},
        {NS_sprm::sprmCLidBi,            &SwWW8ImplReader::Read_Language},
      //NS_sprm::sprmCIcoBi, ? ? ?,
        {NS_sprm::sprmCHpsBi,            &SwWW8ImplReader::Read_FontSize},
        {NS_sprm::sprmCDispFldRMark,     nullptr},                                       // chp.fDispFieldRMark,
                                                                                         // chp.ibstDispFieldRMark,
                                                                                         // chp.dttmDispFieldRMark;
                                                                                         // Complex;variable length
                                                                                         // always recorded as 39 bytes;
        {NS_sprm::sprmCIbstRMarkDel,     nullptr},                                       // chp.ibstRMarkDel;index into
                                                                                         // sttbRMark;short;
        {NS_sprm::sprmCDttmRMarkDel,     nullptr},                                       // chp.dttmRMarkDel;DTTM;long;
        {NS_sprm::sprmCBrc80,            &SwWW8ImplReader::Read_CharBorder},             // chp.brc;BRC;long;
        {NS_sprm::sprmCBrc,              &SwWW8ImplReader::Read_CharBorder},             // chp.brc;BRC;long;
        {NS_sprm::sprmCShd80,            &SwWW8ImplReader::Read_CharShadow},             // chp.shd;SHD;short;
        {NS_sprm::sprmCIdslRMarkDel,     nullptr},                                       // chp.idslRMReasonDel;an index to
                                                                                         // a table of strings defined in
                                                                                         // Word 6.0 executables;short;
        {NS_sprm::sprmCFUsePgsuSettings, nullptr},                                       // chp.fUsePgsuSettings; 1 or 0;bit;
        {NS_sprm::LN_CCpg,               nullptr},                                       // ;;word;
        {NS_sprm::sprmCRgLid0_80,        &SwWW8ImplReader::Read_Language},               // chp.rglid[0];
                                                                                         // LID: for non-Far East text;word;
        {NS_sprm::sprmCRgLid1_80,        &SwWW8ImplReader::Read_Language},               // chp.rglid[1];
                                                                                         // LID: for Far East text;word;
        {NS_sprm::sprmCIdctHint,         &SwWW8ImplReader::Read_IdctHint},               // chp.idctHint;IDCT: byte;
        {NS_sprm::LN_PicBrcl,            nullptr},                                       // pic.brcl;brcl (see PIC structure
                                                                                         // definition);byte;
        {NS_sprm::LN_PicScale,           nullptr},                                       // pic.mx, pic.my, pic.dxaCropleft,
                                                                                         // pic.dyaCropTop pic.dxaCropRight,
                                                                                         // pic.dyaCropBottom;Complex;
                                                                                         // length byte plus 12 bytes;
        {NS_sprm::sprmPicBrcTop80,       nullptr},                                       // pic.brcTop;BRC;long;
        {NS_sprm::sprmPicBrcLeft80,      nullptr},                                       // pic.brcLeft;BRC;long;
        {NS_sprm::sprmPicBrcBottom80,    nullptr},                                       // pic.brcBottom;BRC;long;
        {NS_sprm::sprmPicBrcRight80,     nullptr},                                       // pic.brcRight;BRC;long;
        {NS_sprm::sprmScnsPgn,           nullptr},                                       // sep.cnsPgn;cns;byte;
        {NS_sprm::sprmSiHeadingPgn,      nullptr},                                       // sep.iHeadingPgn;heading number level;
                                                                                         // byte;
        {NS_sprm::LN_SOlstAnm,           &SwWW8ImplReader::Read_OLST},                   // sep.olstAnm;OLST;variable length;
        {NS_sprm::sprmSDxaColWidth,      nullptr},                                       // sep.rgdxaColWidthSpacing;complex;
                                                                                         // 3 bytes;
        {NS_sprm::sprmSDxaColSpacing,    nullptr},                                       // sep.rgdxaColWidthSpacing;complex;
                                                                                         // 3 bytes;
        {NS_sprm::sprmSFEvenlySpaced,    nullptr},                                       // sep.fEvenlySpaced; 1 or 0;byte;
        {NS_sprm::sprmSFProtected,       nullptr},                                       // sep.fUnlocked;1 or 0;byte;
        {NS_sprm::sprmSDmBinFirst,       nullptr},                                       // sep.dmBinFirst;;word;
        {NS_sprm::sprmSDmBinOther,       nullptr},                                       // sep.dmBinOther;;word;
        {NS_sprm::sprmSBkc,              nullptr},                                       // sep.bkc;bkc;byte;
        {NS_sprm::sprmSFTitlePage,       nullptr},                                       // sep.fTitlePage;0 or 1;byte;
        {NS_sprm::sprmSCcolumns,         nullptr},                                       // sep.ccolM1;# of cols - 1;word;
        {NS_sprm::sprmSDxaColumns,       nullptr},                                       // sep.dxaColumns;dxa;word;
        {NS_sprm::LN_SFAutoPgn,          nullptr},                                       // sep.fAutoPgn;obsolete;byte;
        {NS_sprm::sprmSNfcPgn,           nullptr},                                       // sep.nfcPgn;nfc;byte;
        {NS_sprm::LN_SDyaPgn,            nullptr},                                       // sep.dyaPgn;dya;short;
        {NS_sprm::LN_SDxaPgn,            nullptr},                                       // sep.dxaPgn;dya;short;
        {NS_sprm::sprmSFPgnRestart,      nullptr},                                       // sep.fPgnRestart;0 or 1;byte;
        {NS_sprm::sprmSFEndnote,         nullptr},                                       // sep.fEndnote;0 or 1;byte;
        {NS_sprm::sprmSLnc,              nullptr},                                       // sep.lnc;lnc;byte;
        {NS_sprm::LN_SGprfIhdt,          nullptr},                                       // sep.grpfIhdt;grpfihdt;byte;
        {NS_sprm::sprmSNLnnMod,          nullptr},                                       // sep.nLnnMod;non-neg int.;word;
        {NS_sprm::sprmSDxaLnn,           nullptr},                                       // sep.dxaLnn;dxa;word;
        {NS_sprm::sprmSDyaHdrTop,        nullptr},                                       // sep.dyaHdrTop;dya;word;
        {NS_sprm::sprmSDyaHdrBottom,     nullptr},                                       // sep.dyaHdrBottom;dya;word;
        {NS_sprm::sprmSLBetween,         nullptr},                                       // sep.fLBetween;0 or 1;byte;
        {NS_sprm::sprmSVjc,              &SwWW8ImplReader::Read_TextVerticalAdjustment}, // sep.vjc;vjc;byte;
        {NS_sprm::sprmSLnnMin,           nullptr},                                       // sep.lnnMin;lnn;word;
        {NS_sprm::sprmSPgnStart97,       nullptr},                                       // sep.pgnStart;pgn;word;
        {NS_sprm::sprmSBOrientation,     nullptr},                                       // sep.dmOrientPage;dm;byte;
      //NS_sprm::LN_SBCustomize, ? ? ?,
        {NS_sprm::sprmSXaPage,           nullptr},                                       // sep.xaPage;xa;word;
        {NS_sprm::sprmSYaPage,           nullptr},                                       // sep.yaPage;ya;word;
        {0x2205,                         nullptr},                                       // ???? "sprmSDxaLeft" sep.dxaLeft;
                                                                                         // dxa;word;
        {NS_sprm::sprmSDxaLeft,          nullptr},                                       // sep.dxaLeft;dxa;word;
        {NS_sprm::sprmSDxaRight,         nullptr},                                       // sep.dxaRight;dxa;word;
        {NS_sprm::sprmSDyaTop,           nullptr},                                       // sep.dyaTop;dya;word;
        {NS_sprm::sprmSDyaBottom,        nullptr},                                       // sep.dyaBottom;dya;word;
        {NS_sprm::sprmSDzaGutter,        nullptr},                                       // sep.dzaGutter;dza;word;
        {NS_sprm::sprmSDmPaperReq,       nullptr},                                       // sep.dmPaperReq;dm;word;
        {NS_sprm::LN_SPropRMark,         nullptr},                                       // sep.fPropRMark, sep.ibstPropRMark,
                                                                                         // sep.dttmPropRMark;complex; variable
                                                                                         // length always recorded as 7 bytes;
      //NS_sprm::sprmSFBiDi, ? ? ?,
      //NS_sprm::LN_SFFacingCol, ? ? ?,
        {NS_sprm::sprmSFRTLGutter,       nullptr},                                       // set to 1 if gutter is on the right.
        {NS_sprm::sprmSBrcTop80,         nullptr},                                       // sep.brcTop;BRC;long;
        {NS_sprm::sprmSBrcLeft80,        nullptr},                                       // sep.brcLeft;BRC;long;
        {NS_sprm::sprmSBrcBottom80,      nullptr},                                       // sep.brcBottom;BRC;long;
        {NS_sprm::sprmSBrcRight80,       nullptr},                                       // sep.brcRight;BRC;long;
        {NS_sprm::sprmSPgbProp,          nullptr},                                       // sep.pgbProp;word;
        {NS_sprm::sprmSDxtCharSpace,     nullptr},                                       // sep.dxtCharSpace;dxt;long;
        {NS_sprm::sprmSDyaLinePitch,     nullptr},                                       // sep.dyaLinePitch;dya;
                                                                                         // WRONG:long; RIGHT:short; !
      //NS_sprm::sprmSClm, ? ? ?,
        {NS_sprm::sprmSTextFlow,         nullptr},                                       // sep.wTextFlow;complex;short
        {NS_sprm::sprmTJc90,             nullptr},                                       // tap.jc;jc;word
                                                                                         // (low order byte is significant);
        {NS_sprm::sprmTDxaLeft,          nullptr},                                       // tap.rgdxaCenter;dxa;word;
        {NS_sprm::sprmTDxaGapHalf,       nullptr},                                       // tap.dxaGapHalf,
                                                                                         // tap.rgdxaCenter;dxa;word;
        {NS_sprm::sprmTFCantSplit90,     nullptr},                                       // tap.fCantSplit90;1 or 0;byte;
        {NS_sprm::sprmTTableHeader,      nullptr},                                       // tap.fTableHeader;1 or 0;byte;
        {NS_sprm::sprmTFCantSplit,       nullptr},                                       // tap.fCantSplit;1 or 0;byte;
        {NS_sprm::sprmTTableBorders80,   nullptr},                                       // tap.rgbrcTable;complex;24 bytes;
        {NS_sprm::LN_TDefTable10,        nullptr},                                       // tap.rgdxaCenter, tap.rgtc;complex;
                                                                                         // variable length;
        {NS_sprm::sprmTDyaRowHeight,     nullptr},                                       // tap.dyaRowHeight;dya;word;
        {NS_sprm::sprmTDefTable,         nullptr},                                       // tap.rgtc;complex
        {NS_sprm::sprmTDefTableShd80,    nullptr},                                       // tap.rgshd;complex
        {NS_sprm::sprmTTlp,              nullptr},                                       // tap.tlp;TLP;4 bytes;
      //NS_sprm::sprmTFBiDi, ? ? ?,
      //NS_sprm::LN_THTMLProps, ? ? ?,
        {NS_sprm::sprmTSetBrc80,         nullptr},                                       // tap.rgtc[].rgbrc;complex;5 bytes;
        {NS_sprm::sprmTInsert,           nullptr},                                       // tap.rgdxaCenter, tap.rgtc;complex;
                                                                                         // 4 bytes;
        {NS_sprm::sprmTDelete,           nullptr},                                       // tap.rgdxaCenter, tap.rgtc;complex;
                                                                                         // word;
        {NS_sprm::sprmTDxaCol,           nullptr},                                       // tap.rgdxaCenter;complex;4 bytes;
        {NS_sprm::sprmTMerge,            nullptr},                                       // tap.fFirstMerged, tap.fMerged;
                                                                                         // complex; word;
        {NS_sprm::sprmTSplit,            nullptr},                                       // tap.fFirstMerged, tap.fMerged;
                                                                                         // complex;word;
        {NS_sprm::LN_TSetBrc10,          nullptr},                                       // tap.rgtc[].rgbrc;complex;5 bytes;
        {NS_sprm::LN_TSetShd80,          nullptr},                                       // tap.rgshd;complex;4 bytes;
        {NS_sprm::LN_TSetShdOdd80,       nullptr},                                       // tap.rgshd;complex;4 bytes;
        {NS_sprm::sprmTTextFlow,         nullptr},                                       // tap.rgtc[].fVertical
                                                                                         // tap.rgtc[].fBackward
                                                                                         // tap.rgtc[].fRotateFont
                                                                                         // 0 or 10 or 10 or 1;word;
      //NS_sprm::LN_TDiagLine, ? ? ?  ,
        {NS_sprm::sprmTVertMerge,        nullptr},                                       // tap.rgtc[].vertMerge;complex;variable
                                                                                         // length always recorded as 2 bytes;
        {NS_sprm::sprmTVertAlign,        nullptr},                                       // tap.rgtc[].vertAlign;complex;variable
                                                                                         // length always recorded as 3 bytes;
        {NS_sprm::sprmCFELayout,         &SwWW8ImplReader::Read_DoubleLine_Rotate},
        {NS_sprm::sprmPItap,             nullptr},
        {NS_sprm::sprmTTableWidth,       nullptr},                                       // recorded as 3 bytes;
        {NS_sprm::sprmTDefTableShd,      nullptr},
        {NS_sprm::sprmTTableBorders,     nullptr},
        {NS_sprm::sprmTBrcTopCv,         nullptr},
        {NS_sprm::sprmTBrcLeftCv,        nullptr},
        {NS_sprm::sprmTBrcBottomCv,      nullptr},
        {NS_sprm::sprmTBrcRightCv,       nullptr},
        {NS_sprm::sprmTCellPaddingDefault, nullptr},
        {NS_sprm::sprmTCellPadding,      nullptr},
        {0xD238,                         nullptr},                                       // undocumented sep
        {NS_sprm::sprmPBrcTop,           &SwWW8ImplReader::Read_Border},
        {NS_sprm::sprmPBrcLeft,          &SwWW8ImplReader::Read_Border},
        {NS_sprm::sprmPBrcBottom,        &SwWW8ImplReader::Read_Border},
        {NS_sprm::sprmPBrcRight,         &SwWW8ImplReader::Read_Border},
        {NS_sprm::sprmPBrcBetween,       &SwWW8ImplReader::Read_Border},
        {NS_sprm::sprmTWidthIndent,      nullptr},
        {NS_sprm::sprmCRgLid0,           &SwWW8ImplReader::Read_Language},               // chp.rglid[0];
                                                                                         // LID: for non-Far East text;
        {NS_sprm::sprmCRgLid1,           nullptr},                                       // chp.rglid[1];
                                                                                         // LID: for Far East text
        {0x6463,                         nullptr},                                       // undocumented
        {NS_sprm::sprmPJc,               &SwWW8ImplReader::Read_RTLJustify},
        {NS_sprm::sprmPDxaLeft,          &SwWW8ImplReader::Read_LR},
        {NS_sprm::sprmPDxaLeft1,         &SwWW8ImplReader::Read_LR},
        {NS_sprm::sprmPDxaRight,         &SwWW8ImplReader::Read_LR},
        {NS_sprm::sprmTFAutofit,         nullptr},
        {NS_sprm::sprmTPc,               nullptr},
        {NS_sprm::sprmTDxaAbs,           nullptr},
        {NS_sprm::sprmTDyaAbs,           nullptr},
        {NS_sprm::sprmTDxaFromText,      nullptr},
        {NS_sprm::sprmSRsid,             nullptr},
        {NS_sprm::sprmSFpc,              nullptr},
        {NS_sprm::sprmPFInnerTableCell,  &SwWW8ImplReader::Read_TabCellEnd},
        {NS_sprm::sprmPFInnerTtp,        &SwWW8ImplReader::Read_TabRowEnd},
        {NS_sprm::sprmCRsidProp,         nullptr},
        {NS_sprm::sprmCRsidText,         nullptr},
        {NS_sprm::sprmCCv,               &SwWW8ImplReader::Read_TextForeColor},
        {NS_sprm::sprmCCvUl,             &SwWW8ImplReader::Read_UnderlineColor},
        {NS_sprm::sprmPShd,              &SwWW8ImplReader::Read_ParaBackColor},
        {NS_sprm::sprmPRsid,             nullptr},
        {NS_sprm::sprmTWidthBefore,      nullptr},
        {NS_sprm::sprmTSetShdTable,      nullptr},
        {NS_sprm::sprmTDefTableShdRaw,   nullptr},
        {NS_sprm::sprmCShd,              &SwWW8ImplReader::Read_TextBackColor},
        {NS_sprm::sprmSRncFtn,           nullptr},
        {NS_sprm::sprmPFDyaBeforeAuto,   &SwWW8ImplReader::Read_ParaAutoBefore},
        {NS_sprm::sprmPFDyaAfterAuto,    &SwWW8ImplReader::Read_ParaAutoAfter},
        {NS_sprm::sprmPFContextualSpacing, &SwWW8ImplReader::Read_ParaContextualSpacing},
    };

    static wwSprmDispatcher aSprmSrch(aSprms, SAL_N_ELEMENTS(aSprms));
    return &aSprmSrch;
}

//      helper routines : find SPRM

const SprmReadInfo& SwWW8ImplReader::GetSprmReadInfo(sal_uInt16 nId) const
{
    ww::WordVersion eVersion = m_xWwFib->GetFIBVersion();
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

//      helper routines : SPRMs

void SwWW8ImplReader::EndSprm( sal_uInt16 nId )
{
    if( ( nId > 255 ) && ( nId < 0x0800 ) ) return;

    const SprmReadInfo& rSprm = GetSprmReadInfo( nId );

    if (rSprm.pReadFnc)
        (this->*rSprm.pReadFnc)( nId, nullptr, -1 );
}

short SwWW8ImplReader::ImportSprm(const sal_uInt8* pPos, sal_Int32 nMemLen, sal_uInt16 nId)
{
    if (!nId)
        nId = m_xSprmParser->GetSprmId(pPos);

    OSL_ENSURE( nId != 0xff, "Sprm FF !!!!" );

    const SprmReadInfo& rSprm = GetSprmReadInfo(nId);

    sal_uInt16 nFixedLen = m_xSprmParser->DistanceToData(nId);
    sal_uInt16 nL = m_xSprmParser->GetSprmSize(nId, pPos, nMemLen);

    if (rSprm.pReadFnc)
        (this->*rSprm.pReadFnc)(nId, pPos + nFixedLen, nL - nFixedLen);

    return nL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
