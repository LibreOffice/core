/*************************************************************************
 *
 *  $RCSfile: ww8par6.cxx,v $
 *
 *  $Revision: 1.143 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-25 07:46:00 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#include <stdlib.h>


#pragma hdrstop

#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX //autogen
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_AKRNITEM_HXX //autogen
#include <svx/akrnitem.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX //autogen
#include <svx/emphitem.hxx>
#endif
#ifndef _SVX_FORBIDDENRULEITEM_HXX
#include <svx/forbiddenruleitem.hxx>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
#endif
#ifndef _SVX_SCRIPSPACEITEM_HXX
#include <svx/scriptspaceitem.hxx>
#endif
#ifndef _SVX_HNGPNCTITEM_HXX
#include <svx/hngpnctitem.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <svx/charscaleitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <svx/charrotateitem.hxx>
#endif
#ifndef _SVX_CHARRELIEFITEM_HXX
#include <svx/charreliefitem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_PARAVERTALIGNITEM_HXX
#include <svx/paravertalignitem.hxx>
#endif
#ifndef _SVX_PGRDITEM_HXX
#include <svx/pgrditem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx> // SwTxtNode, siehe unten: JoinNode()
#endif
#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>         // class SwPageDesc
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef SW_LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _FMTFLCNT_HXX
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif
#ifndef _SW_HF_EAT_SPACINGITEM_HXX
#include <hfspacingitem.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif

#ifndef _FLTINI_HXX
#include <fltini.hxx>   //For CalculateFlySize
#endif

#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif
#ifndef SW_WRITERWORDGLUE
#include "writerwordglue.hxx"
#endif

#ifndef _WW8SCAN_HXX
#include "ww8scan.hxx"
#endif
#ifndef _WW8PAR2_HXX
#include "ww8par2.hxx"          // class WW8RStyle, class WwAnchorPara
#endif
#ifndef _WW8GRAF_HXX
#include "ww8graf.hxx"
#endif

using namespace sw::util;
using namespace sw::types;
//-----------------------------------------
//              diverses
//-----------------------------------------

#define MM_250 1417             // WW-Default fuer Hor. Seitenraender: 2.5 cm
#define MM_200 1134             // WW-Default fuer u.Seitenrand: 2.0 cm

BYTE lcl_ReadBorders(bool bVer67, WW8_BRC* brc, WW8PLCFx_Cp_FKP* pPap,
    const WW8RStyle* pSty = 0, const WW8PLCFx_SEPX* pSep = 0);


ColorData SwWW8ImplReader::GetCol(BYTE nIco)
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

inline short MSRoundTweak(short x)
{
#if 0
    //keep this around in case it turns out to be true. But I don't think so.
    //I think its a tab compatability issue
    if (x < 0)
        ++x;
    else if (x > 0)
        --x;
#endif
    return x;
}

/***************************************************************************
#  Seiten - Attribute, die nicht ueber die Attribut-Verwaltung, sondern
#   ueber ...->HasSprm abgearbeitet werden
#   ( ausser OLST, dass weiterhin ein normales Attribut ist )
#**************************************************************************/

static short ReadSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, short nDefaultVal )
{
    const BYTE* pS = pSep->HasSprm( nId );          // sprm da ?
    short nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    return nVal;
}

static USHORT ReadUSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, short nDefaultVal )
{
    const BYTE* pS = pSep->HasSprm( nId );          // sprm da ?
    USHORT nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    return nVal;
}

static BYTE ReadBSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, BYTE nDefaultVal )
{
    const BYTE* pS = pSep->HasSprm( nId );          // sprm da ?
    BYTE nVal = ( pS ) ? SVBT8ToByte( pS ) : nDefaultVal;
    return nVal;
}

static short ReadULSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, short nDefaultVal )
{
    const BYTE* pS = pSep->HasSprm( nId );          // sprm da ?
    short nVal = ( pS ) ? SVBT16ToShort( pS ) : nDefaultVal;
    if( nVal < 0 )      // < 0 bedeutet: nicht verschieben, wenns nicht passt
        nVal = -nVal;
    return nVal;
}

void wwSection::SetDirection()
{
    //sprmSTextFlow
    switch (maSep.wTextFlow)
    {
        default:
            ASSERT(!this, "Unknown layout type");
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

void SwWW8ImplReader::SetDocumentGrid(SwFrmFmt &rFmt, const wwSection &rSection)
{
    if (bVer67)
        return;

    rFmt.SetAttr(SvxFrameDirectionItem(rSection.meDir));

    SwTwips nTextareaHeight = rFmt.GetFrmSize().GetHeight();
    const SvxULSpaceItem &rUL = ItemGet<SvxULSpaceItem>(rFmt, RES_UL_SPACE);
    nTextareaHeight -= rUL.GetUpper();
    nTextareaHeight -= rUL.GetLower();

    SwTwips nTextareaWidth = rFmt.GetFrmSize().GetWidth();
    const SvxLRSpaceItem &rLR = ItemGet<SvxLRSpaceItem>(rFmt, RES_LR_SPACE);
    nTextareaWidth -= rLR.GetLeft();
    nTextareaWidth -= rLR.GetRight();

    if (rSection.IsVertical())
    {
        SwTwips nSwap = nTextareaHeight;
        nTextareaHeight = nTextareaWidth;
        nTextareaWidth = nSwap;
    }

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
            ASSERT(!this, "Unknown grid type");
        case 3:
            //Text snaps to char grid, this doesn't make a lot of sense to
            //me. This is closer than LINES_CHARS
            eType = GRID_LINES_ONLY;
            break;
        case 1:
            eType = GRID_LINES_CHARS;
            break;
        case 2:
            eType = GRID_LINES_ONLY;
            break;
    }

    aGrid.SetGridType(eType);

    //sep.dyaLinePitch
    short nLinePitch = rSection.maSep.dyaLinePitch;

    aGrid.SetLines(nTextareaHeight/nLinePitch);

    //It remains to be seen if a base height of 14points and a ruby height of
    //4 points should allow 12point text with ruby of 4pt to fit in single
    //line. I reckon it should (why not), but it doesn't at present :-(
    //So right now this doesn't always work as I think it should

    //This seems emperically correct, but might require some future tweaking.
    short nRubyHeight = nLinePitch*2/9;
    short nBaseHeight = nLinePitch-nRubyHeight;
    aGrid.SetBaseHeight(nBaseHeight);
    aGrid.SetRubyHeight(nRubyHeight);

    //Get the size of word's default styles font
    UINT32 nRubyWidth=240;
    for (USHORT nI = 0; nI < pStyles->GetCount(); nI++ )
    {
        if (pCollA[nI].bValid && pCollA[nI].pFmt &&
            pCollA[nI].GetWWStyleId() == 0)
        {
            nRubyWidth = ItemGet<SvxFontHeightItem>(*(pCollA[nI].pFmt),
                RES_CHRATR_CJK_FONTSIZE).GetHeight();
            break;
        }
    }

    //dxtCharSpace
    if (rSection.maSep.dxtCharSpace)
    {
        UINT32 nCharSpace = rSection.maSep.dxtCharSpace;
        //main lives in top 20 bits, and is signed.
        INT32 nMain = (nCharSpace & 0xFFFFF000);
        nMain/=0x1000;
        nRubyWidth += nMain*20;

        int nFraction = (nCharSpace & 0x00000FFF);
        nFraction = (nFraction*20)/0xFFF;
        nRubyWidth += nFraction;
    }

//    nothing I can do with this value at the moment I think
//    aGrid.SetNoChars(nTextareaWidth/nRubyWidth);

    rFmt.SetAttr(aGrid);
}

void SwWW8ImplReader::Read_ParaBiDi(USHORT, const BYTE* pData, short nLen)
{
    if( nLen < 0 )
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_FRAMEDIR);
    else
    {
        SvxFrameDirection eDir =
            *pData ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP;
        NewAttr(SvxFrameDirectionItem(eDir));
    }
}

bool wwSectionManager::SetCols(SwFrmFmt &rFmt, const wwSection &rSection,
    USHORT nNettoWidth)
{
    //sprmSCcolumns - Anzahl der Spalten - 1
    USHORT nCols = rSection.NoCols();

    if (nCols < 2)
        return false;                   // keine oder bloedsinnige Spalten

    SwFmtCol aCol;                      // Erzeuge SwFmtCol

    //sprmSDxaColumns   - Default-Abstand 1.25 cm
    USHORT nColSpace = rSection.StandardColSeperation();

    // sprmSLBetween
    if (rSection.maSep.fLBetween)
    {
        aCol.SetLineAdj( COLADJ_TOP );      // Line
        aCol.SetLineHeight( 100 );
        aCol.SetLineColor( Color( COL_BLACK ));
        aCol.SetLineWidth( 1 );
    }

    aCol.Init( nCols, nColSpace, nNettoWidth );
    // sprmSFEvenlySpaced
    if (!rSection.maSep.fEvenlySpaced)
    {
        aCol._SetOrtho(false);
        int nIdx = 1;
        for (USHORT i = 0; i < nCols; i++ )
        {
            SwColumn* pCol = aCol.GetColumns()[ i ];
            USHORT nLeft = rSection.maSep.rgdxaColumnWidthSpacing[nIdx - 1]/2;
            USHORT nRight = rSection.maSep.rgdxaColumnWidthSpacing[nIdx + 1]/2;
            USHORT nWishWidth = rSection.maSep.rgdxaColumnWidthSpacing[nIdx]
                + nLeft + nRight;
            pCol->SetWishWidth(nWishWidth);
            pCol->SetLeft(nLeft);
            pCol->SetRight(nRight);
            nIdx += 2;
        }
        aCol.SetWishWidth(nNettoWidth);
    }
    rFmt.SetAttr(aCol);
    return true;
}

void wwSectionManager::SetLeftRight(wwSection &rSection)
{
    // 3. LR-Raender
    short nWWLe = MSRoundTweak(rSection.maSep.dxaLeft);
    short nWWRi = MSRoundTweak(rSection.maSep.dxaRight);
    short nWWGu = rSection.maSep.dzaGutter;

    /*
    fRTLGutter is set if the gutter is on the right, the gutter is otherwise
    placed on the left unless the global dop options are to put it on top, that
    case is handled in GetPageULData.
    */
    if (rSection.maSep.fRTLGutter)
        nWWRi += nWWGu;
    else if (!mrReader.pWDop->iGutterPos)
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

void wwSectionManager::SetPage(SwPageDesc &rInPageDesc, SwFrmFmt &rFmt,
    const wwSection &rSection, bool bIgnoreCols)
{
    // 1. Orientierung
    rInPageDesc.SetLandscape(rSection.IsLandScape());

    // 2. Papiergroesse
    SwFmtFrmSize aSz( rFmt.GetFrmSize() );
    aSz.SetWidth(rSection.GetPageWidth());
    aSz.SetHeight(SnapPageDimension(rSection.GetPageHeight()));
    rFmt.SetAttr(aSz);

    rFmt.SetAttr(
        SvxLRSpaceItem(rSection.GetPageLeft(), rSection.GetPageRight()));

    if (!bIgnoreCols)
    {
        SetCols(rFmt, rSection, rSection.GetPageWidth() -
            rSection.GetPageLeft() - rSection.GetPageRight());
    }
}

void wwSectionManager::GetPageULData(const wwSection &rSection, bool bFirst,
    wwSectionManager::wwULSpaceData& rData)
{
    short nWWUp = rSection.maSep.dyaTop;
    short nWWLo = rSection.maSep.dyaBottom;
    short nWWHTop = rSection.maSep.dyaHdrTop;
    short nWWFBot = rSection.maSep.dyaHdrBottom;

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

    if( bFirst )
        rData.bHasHeader = (rSection.maSep.grpfIhdt & WW8_HEADER_FIRST) !=0;
    else
    {
        rData.bHasHeader = (rSection.maSep.grpfIhdt &
            (WW8_HEADER_EVEN | WW8_HEADER_ODD)) != 0;
    }

    if( rData.bHasHeader )
    {
        rData.nSwUp  = nWWHTop;             // Header -> umrechnen
        rData.nSwHLo = nWWUp - nWWHTop;

        if (rData.nSwHLo < MM50)
            rData.nSwHLo = MM50;
    }
    else // kein Header -> Up einfach uebernehmen
        rData.nSwUp = nWWUp;

    if( bFirst )
        rData.bHasFooter = (rSection.maSep.grpfIhdt &  WW8_FOOTER_FIRST) !=0;
    else
    {
        rData.bHasFooter = (rSection.maSep.grpfIhdt &
            (WW8_FOOTER_EVEN | WW8_FOOTER_ODD)) != 0;
    }

    if( rData.bHasFooter )
    {
        rData.nSwLo = nWWFBot;              // Footer -> Umrechnen
        rData.nSwFUp = nWWLo - nWWFBot;

        if (rData.nSwFUp < MM50)
            rData.nSwFUp = MM50;
    }
    else // kein Footer -> Lo einfach uebernehmen
        rData.nSwLo = nWWLo;
}

void wwSectionManager::SetPageULSpaceItems(SwFrmFmt &rFmt,
    wwSectionManager::wwULSpaceData& rData)
{
    if (rData.bHasHeader)               // ... und Header-Lower setzen
    {
        //Kopfzeilenhoehe minimal sezten
        if (SwFrmFmt* pHdFmt = (SwFrmFmt*)rFmt.GetHeader().GetHeaderFmt())
        {
            pHdFmt->SetAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwHLo));
            SvxULSpaceItem aHdUL(pHdFmt->GetULSpace());
            aHdUL.SetLower(rData.nSwHLo - MM50);
            pHdFmt->SetAttr(aHdUL);
            pHdFmt->SetAttr(SwHeaderAndFooterEatSpacingItem(
                RES_HEADER_FOOTER_EAT_SPACING, true));
        }
    }

    if (rData.bHasFooter)               // ... und Footer-Upper setzen
    {
        if (SwFrmFmt* pFtFmt = (SwFrmFmt*)rFmt.GetFooter().GetFooterFmt())
        {
            pFtFmt->SetAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwFUp));
            SvxULSpaceItem aFtUL(pFtFmt->GetULSpace());
            aFtUL.SetUpper(rData.nSwFUp - MM50);
            pFtFmt->SetAttr(aFtUL);
            pFtFmt->SetAttr(SwHeaderAndFooterEatSpacingItem(
                RES_HEADER_FOOTER_EAT_SPACING, true));
        }
    }

    SvxULSpaceItem aUL(rData.nSwUp, rData.nSwLo); // Page-UL setzen
    rFmt.SetAttr(aUL);
}

SwSectionFmt *wwSectionManager::InsertSection(
    SwPaM& rMyPaM, wwSection &rSection)
{
    SwSection aSection( CONTENT_SECTION, mrReader.rDoc.GetUniqueSectionName() );

    SfxItemSet aSet( mrReader.rDoc.GetAttrPool(), aFrmFmtSetRange );

    sal_uInt8 nRTLPgn = maSegments.empty() ? 0 : maSegments.back().IsBiDi();
    aSet.Put(SvxFrameDirectionItem(
        nRTLPgn ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP));

    if (2 == mrReader.pWDop->fpc)
        aSet.Put( SwFmtFtnAtTxtEnd(FTNEND_ATTXTEND));
    if (0 == mrReader.pWDop->epc)
        aSet.Put( SwFmtEndAtTxtEnd(FTNEND_ATTXTEND));

    rSection.mpSection = mrReader.rDoc.Insert( rMyPaM, aSection, &aSet );
    ASSERT(rSection.mpSection, "section not inserted!");
    if (!rSection.mpSection)
        return 0;

    SwPageDesc *pPage = 0;
    mySegrIter aEnd = maSegments.rend();
    for (mySegrIter aIter = maSegments.rbegin(); aIter != aEnd; ++aIter)
    {
        if (pPage = aIter->mpPage)
            break;
    }

    ASSERT(pPage, "no page outside this section!");

    if (!pPage)
        pPage = &mrReader.rDoc._GetPageDesc(0);

    if (!pPage)
        return 0;

    SwSectionFmt *pFmt = rSection.mpSection->GetFmt();
    ASSERT(pFmt, "impossible");
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
        SvxLRSpaceItem aLR(nSectionLeft, nSectionRight);
        pFmt->SetAttr(aLR);
    }

    SetCols(*pFmt, rSection, rSection.GetPageWidth() -
        rSection.GetPageLeft() - rSection.GetPageRight());

    //Set the columns to be UnBalanced if compatability option is set
    if (mrReader.pWDop->fNoColumnBalance  )
    {
        SwSectionFmt *pFmt = rSection.mpSection->GetFmt();
        pFmt->SetAttr(SwFmtNoBalancedColumns(true));
    }

    return pFmt;
}

void SwWW8ImplReader::HandleLineNumbering(const wwSection &rSection)
{
    // check if Line Numbering must be activated or resetted
    if (mbNewDoc && rSection.maSep.nLnnMod)
    {
        // restart-numbering-mode: 0 per page, 1 per section, 2 never restart
        bool bRestartLnNumPerSection = (1 == rSection.maSep.lnc);

        if (bNoLnNumYet)
        {
            SwLineNumberInfo aInfo( rDoc.GetLineNumberInfo() );

            aInfo.SetPaintLineNumbers(true);

            aInfo.SetRestartEachPage(!bRestartLnNumPerSection);

            aInfo.SetPosFromLeft( rSection.maSep.dxaLnn);

            //Paint only for every n line
            aInfo.SetCountBy(rSection.maSep.nLnnMod);

            // to be defaulted features ( HARDCODED in MS Word 6,7,8,9 )
            aInfo.SetCountBlankLines(true);
            aInfo.SetCountInFlys(false);
            aInfo.SetPos( LINENUMBER_POS_LEFT );
            SvxNumberType aNumType; // this sets SVX_NUM_ARABIC per default
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
            aLN.SetStartValue(1 + rSection.maSep.lnnMin);
            NewAttr(aLN);
            pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_LINENUMBER);
        }
        bNoLnNumYet = false;
    }
}

wwSection::wwSection(const SwPosition &rPos) : maStart(rPos.nNode),
    mpSection(0), mpTitlePage(0), mpPage(0), meDir(FRMDIR_HORI_LEFT_TOP),
    nPgWidth(lA4Width), nPgLeft(MM_250), nPgRight(MM_250), mnBorders(0),
    mbHasFootnote(false)
{
}

void wwSectionManager::SetNumberingType(const wwSection &rNewSection,
    SwPageDesc &rPageDesc)
{
    // Seitennummernformat speichern
    static const SvxExtNumType aNumTyp[5] =
    {
        SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
        SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N
    };

    SvxNumberType aType;
    aType.SetNumberingType(aNumTyp[rNewSection.maSep.nfcPgn]);
    rPageDesc.SetNumType(aType);
}

// Bei jedem Abschnittswechsel ( auch am Anfang eines Dokuments ) wird
// CreateSep gerufen, dass dann den / die Pagedesc(s) erzeugt und
// mit Attributen un KF-Texten fuellt.
// Dieses Vorgehen ist noetig geworden, da die UEbersetzung der verschiedenen
// Seiten-Attribute zu stark verflochten ist.
void wwSectionManager::CreateSep(const long nTxtPos, bool bMustHaveBreak)
{
    /*
    #i1909# #100688# section/page breaks should not occur in tables or subpage
    elements like frames. Word itself ignores them in this case. The bug is
    more likely that this filter created such documents in the past!
    */
    if (mrReader.nInTable || mrReader.bTxbxFlySection || mrReader.InLocalApo())
        return;

    WW8PLCFx_SEPX* pSep = mrReader.pPlcxMan->GetSepPLCF();
    ASSERT(pSep, "impossible!");
    if (!pSep)
        return;

    bool bVer67(mrReader.bVer67);

    wwSection aLastSection(*mrReader.pPaM->GetPoint());
    if (!maSegments.empty())
        aLastSection = maSegments.back();

    //Here
    USHORT nLIdx = ( ( mrReader.pWwFib->lid & 0xff ) == 0x9 ) ? 1 : 0;

    //BEGIN read section values
    wwSection aNewSection(*mrReader.pPaM->GetPoint());

    if (!maSegments.empty())
    {
        // Type of break: break codes are:
        // 0 No break
        // 1 New column
        // 2 New page
        // 3 Even page
        // 4 Odd page
        if (const BYTE* pSprmBkc = pSep->HasSprm(bVer67 ? 142 : 0x3009))
            aNewSection.maSep.bkc = *pSprmBkc;
    }

    // Has a table page
    aNewSection.maSep.fTitlePage =
        (0 != ReadBSprm( pSep, bVer67 ? 143 : 0x300A, 0 ));

    // sprmSNfcPgn
    aNewSection.maSep.nfcPgn = ReadBSprm( pSep, (bVer67 ? 147 : 0x300E), 0 );
    if (aNewSection.maSep.nfcPgn > 4)
        aNewSection.maSep.nfcPgn = 0;

    // sprmSFBiDi
    if (!bVer67)
        aNewSection.maSep.fBiDi = ReadBSprm(pSep, 0x3228, 0);

    aNewSection.maSep.ccolM1 = ReadSprm(pSep, (bVer67 ? 144 : 0x500B), 0 );

    //sprmSDxaColumns   - Default-Abstand 1.25 cm
    aNewSection.maSep.dxaColumns =
        ReadUSprm( pSep, (bVer67 ? 145 : 0x900C), 708 );

    // sprmSLBetween
    aNewSection.maSep.fLBetween = ReadBSprm(pSep, (bVer67 ? 158 : 0x3019), 0 );

    // sprmSFEvenlySpaced
    aNewSection.maSep.fEvenlySpaced =
        ReadBSprm(pSep, (bVer67 ? 138 : 0x3005), 1) ? true : false;

    if (aNewSection.maSep.ccolM1 > 0 && !aNewSection.maSep.fEvenlySpaced)
    {
        aNewSection.maSep.rgdxaColumnWidthSpacing[0] = 0;
        USHORT nWidth;
        int nCols = aNewSection.maSep.ccolM1 + 1;
        int nIdx = 0;
        for (int i = 0; i < nCols; ++i)
        {
            //sprmSDxaColWidth
            const BYTE* pSW =
                pSep->HasSprm( (bVer67 ? 136 : 0xF203), BYTE( i ) );

            ASSERT( pSW, "+Sprm 136 (bzw. 0xF203) (ColWidth) fehlt" );
            if (pSW)
                nWidth = SVBT16ToShort(pSW + 1);

            aNewSection.maSep.rgdxaColumnWidthSpacing[++nIdx] = nWidth;

            if (i < nCols-1)
            {
                //sprmSDxaColSpacing
                const BYTE* pSD =
                    pSep->HasSprm( (bVer67 ? 137 : 0xF204), BYTE( i ) );

                ASSERT( pSD, "+Sprm 137 (bzw. 0xF204) (Colspacing) fehlt" );
                if( pSD )
                {
                    nWidth = SVBT16ToShort(pSD + 1);
                    aNewSection.maSep.rgdxaColumnWidthSpacing[++nIdx] = nWidth;
                }
            }
        }
    }

    static const USHORT aVer67Ids[] =
    {
        /*sprmSBOrientation*/   162,
        /*sprmSXaPage*/         164,
        /*sprmSYaPage*/         165,
        /*sprmSDxaLeft*/        166,
        /*sprmSDxaRight*/       167,
        /*sprmSDzaGutter*/      170
    };

    static const USHORT aVer8Ids[] =
    {
        /*sprmSBOrientation*/   0x301d,
        /*sprmSXaPage*/         0xB01F,
        /*sprmSYaPage*/         0xB020,
        /*sprmSDxaLeft*/        0xB021,
        /*sprmSDxaRight*/       0xB022,
        /*sprmSDzaGutter*/      0xB025
    };

    const USHORT* pIds = bVer67 ? aVer67Ids : aVer8Ids;

                                            // 1. Orientierung
    aNewSection.maSep.dmOrientPage = ReadBSprm(pSep, pIds[0], 0);

    // 2. Papiergroesse
    aNewSection.maSep.xaPage = ReadUSprm(pSep, pIds[1], (USHORT)lLetterWidth);

    aNewSection.nPgWidth = SnapPageDimension(aNewSection.maSep.xaPage);

    aNewSection.maSep.yaPage = ReadUSprm(pSep, pIds[2], (USHORT)lLetterHeight);

    // 3. LR-Raender
    static const USHORT nLef[] = { MM_250, 1800 };
    static const USHORT nRig[] = { MM_250, 1800 };

    aNewSection.maSep.dxaLeft = ReadULSprm( pSep, pIds[3], nLef[nLIdx]);
    aNewSection.maSep.dxaRight = ReadULSprm( pSep, pIds[4], nRig[nLIdx]);

    //#110175# 2pages in 1sheet hackery ?
    if (aNewSection.maSep.dmOrientPage == 2)
        std::swap(aNewSection.maSep.dxaLeft, aNewSection.maSep.dxaRight);

    aNewSection.maSep.dzaGutter = ReadULSprm( pSep, pIds[5], 0);

    aNewSection.maSep.fRTLGutter = !bVer67 ? ReadULSprm( pSep, 0x322A, 0 ) : 0;

    // Page Number Restarts - sprmSFPgnRestart
    aNewSection.maSep.fPgnRestart = ReadBSprm(pSep, (bVer67 ? 150 : 0x3011), 0);

    aNewSection.maSep.pgnStart = ReadBSprm( pSep, (bVer67 ? 161 : 0x501C), 0 );

    if (const BYTE* p = pSep->HasSprm( bVer67 ? 132 : 0x3001 ))
        aNewSection.maSep.iHeadingPgn = *p;

    if (const BYTE* p = pSep->HasSprm( bVer67 ? 131 : 0x3000 ))
        aNewSection.maSep.cnsPgn = *p;

    static const BYTE aPaperBinIds[17] = {5,2,4,0,3,0,0,0,0,0,0,0,0,0,0,0,1};
        // WW                          SW
        // ------------------------------
        //  0 == default                5
        //  1 == Upper paper tray       2
        //  2 == Lower paper tray       4
        //  4 == Manual paper feed      3
        // 15 == Automatically select   0
        // 16 == First tray available   1

    if(const BYTE* pSprmSDmBinFirst = pSep->HasSprm( bVer67 ? 140 : 0x5007 ))
        aNewSection.maSep.dmBinFirst = *pSprmSDmBinFirst;

    if (const BYTE* pSprmSDmBinOther = pSep->HasSprm( bVer67 ? 141 : 0x5008))
        aNewSection.maSep.dmBinOther = *pSprmSDmBinOther;

    static const USHORT nTop[] = { MM_250, 1440 };
    static const USHORT nBot[] = { MM_200, 1440 };

    static const USHORT aVer67Ids2[] =
    {
        /*sprmSDyaTop*/         168,
        /*sprmSDyaBottom*/      169,
        /*sprmSDyaHdrTop*/      156,
        /*sprmSDyaHdrBottom*/   157
    };
    static const USHORT aVer8Ids2[] =
    {
        /*sprmSDyaTop*/         0x9023,
        /*sprmSDyaBottom*/      0x9024,
        /*sprmSDyaHdrTop*/      0xB017,
        /*sprmSDyaHdrBottom*/   0xB018
    };

    pIds = bVer67 ? aVer67Ids2 : aVer8Ids2;

    aNewSection.maSep.dyaTop = ReadULSprm( pSep, pIds[0], nTop[nLIdx] );
    aNewSection.maSep.dyaBottom = ReadULSprm( pSep, pIds[1], nBot[nLIdx] );
    aNewSection.maSep.dyaHdrTop = ReadULSprm( pSep, pIds[2], 720 );
    aNewSection.maSep.dyaHdrBottom = ReadULSprm( pSep, pIds[3], 720 );

    if (!bVer67)
    {
        aNewSection.maSep.wTextFlow = ReadULSprm(pSep, 0x5033, 0);
        aNewSection.maSep.clm = ReadULSprm( pSep, 0x5032, 0 );
        aNewSection.maSep.dyaLinePitch = ReadULSprm(pSep, 0x9031, 360);
        if (const BYTE* pS = pSep->HasSprm(0x7030))
            aNewSection.maSep.dxtCharSpace = SVBT32ToLong(pS);

        //sprmSPgbProp
        sal_uInt16 pgbProp = ReadSprm( pSep, 0x522F, 0 );
        aNewSection.maSep.pgbApplyTo = pgbProp & 0x0007;
        aNewSection.maSep.pgbPageDepth = (pgbProp & 0x0018) >> 3;
        aNewSection.maSep.pgbOffsetFrom = (pgbProp & 0x00E0) >> 5;

        aNewSection.mnBorders =
            ::lcl_ReadBorders(bVer67, aNewSection.brc, 0, 0, pSep);
    }

    // check if Line Numbering must be activated or resetted
    if (const BYTE* pSprmSNLnnMod = pSep->HasSprm( bVer67 ? 154 : 0x5015 ))
        aNewSection.maSep.nLnnMod = *pSprmSNLnnMod;

    if (const BYTE* pSprmSLnc = pSep->HasSprm( bVer67 ? 152 : 0x3013 ))
        aNewSection.maSep.lnc = *pSprmSLnc;

    if (const BYTE* pSprmSDxaLnn = pSep->HasSprm( bVer67 ? 155:0x9016 ))
        aNewSection.maSep.dxaLnn = SVBT16ToShort( pSprmSDxaLnn );

    if (const BYTE* pSprmSLnnMin = pSep->HasSprm( bVer67 ? 160:0x501B ))
        aNewSection.maSep.lnnMin = *pSprmSLnnMin;

    if (bVer67)
        aNewSection.maSep.grpfIhdt = ReadBSprm(pSep, 153, 0);
    else if (mrReader.pHdFt)
    {
        aNewSection.maSep.grpfIhdt = WW8_HEADER_ODD | WW8_FOOTER_ODD;

        if (aNewSection.HasTitlePage())
            aNewSection.maSep.grpfIhdt |= WW8_HEADER_FIRST | WW8_FOOTER_FIRST;

        if (mrReader.pWDop->fFacingPages)
            aNewSection.maSep.grpfIhdt |= WW8_HEADER_EVEN | WW8_FOOTER_EVEN;

        //See if we have a header or footer for each enabled possibility
        //if we do not then we inherit the previous sections header/footer,
        for (int nI = 0, nMask = 1; nI < 6; ++nI, nMask <<= 1)
        {
            if (aNewSection.maSep.grpfIhdt & nMask)
            {
                WW8_CP nStart;
                long nLen;
                mrReader.pHdFt->GetTextPosExact(nI + ( maSegments.size() + 1) * 6, nStart, nLen);
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

    if (!bVer67)
        aNewSection.SetDirection();

    mrReader.HandleLineNumbering(aNewSection);
    maSegments.push_back(aNewSection);
}

void SwWW8ImplReader::CopyPageDescHdFt(const SwPageDesc* pOrgPageDesc,
    SwPageDesc* pNewPageDesc, BYTE nCode )
{
    // copy first header content section
    if (nCode & WW8_HEADER_FIRST)
        rDoc.CopyHeader(pOrgPageDesc->GetMaster(), pNewPageDesc->GetMaster());

    // copy first footer content section
    if( nCode & WW8_FOOTER_FIRST )
        rDoc.CopyFooter(pOrgPageDesc->GetMaster(), pNewPageDesc->GetMaster());

    if( nCode & (   WW8_HEADER_ODD  | WW8_FOOTER_ODD
                  | WW8_HEADER_EVEN | WW8_FOOTER_EVEN ) )
    {
        // copy odd header content section
        if( nCode & WW8_HEADER_ODD )
        {
            rDoc.CopyHeader(pOrgPageDesc->GetMaster(),
                            pNewPageDesc->GetMaster() );
        }
        // copy odd footer content section
        if( nCode & WW8_FOOTER_ODD )
        {
            rDoc.CopyFooter(pOrgPageDesc->GetMaster(),
                            pNewPageDesc->GetMaster());
        }
        // copy even header content section
        if( nCode & WW8_HEADER_EVEN )
        {
            rDoc.CopyHeader(pOrgPageDesc->GetLeft(),
                            pNewPageDesc->GetLeft());
        }
        // copy even footer content section
        if( nCode & WW8_FOOTER_EVEN )
        {
            rDoc.CopyFooter(pOrgPageDesc->GetLeft(),
                            pNewPageDesc->GetLeft());
        }
    }
}

//------------------------------------------------------
//   Hilfsroutinen fuer Grafiken und Apos und Tabellen
//------------------------------------------------------

static bool _SetWW8_BRC(bool bVer67, WW8_BRC& rVar, const BYTE* pS)
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

BYTE lcl_ReadBorders(bool bVer67, WW8_BRC* brc, WW8PLCFx_Cp_FKP* pPap,
    const WW8RStyle* pSty, const WW8PLCFx_SEPX* pSep)
{
// Ausgegend von diesen defines:
//      #define WW8_TOP 0
//      #define WW8_LEFT 1
//      #define WW8_BOT 2
//      #define WW8_RIGHT 3
//      #define WW8_BETW 4

//returns a BYTE filled with a bit for each position that had a sprm
//setting that border

    BYTE nBorder = false;
    if( pSep )
    {
        if( !bVer67 )
        {
             BYTE* pSprm[4];

            //  sprmSBrcTop, sprmSBrcLeft, sprmSBrcBottom, sprmSBrcRight
             if( pSep->Find4Sprms(  0x702B,   0x702C,   0x702D,   0x702E,
                                    pSprm[0], pSprm[1], pSprm[2], pSprm[3] ) )
             {
                for( int i = 0; i < 4; ++i )
                    nBorder |= (_SetWW8_BRC( bVer67, brc[ i ], pSprm[ i ] ))<<i;
             }
        }
    }
    else
    {

        static const USHORT aVer67Ids[5] = { 38, 39, 40, 41, 42 };

        static const USHORT aVer8Ids[5] =
                { 0x6424, 0x6425, 0x6426, 0x6427, 0x6428 };

        const USHORT* pIds = bVer67 ? aVer67Ids : aVer8Ids;

        if( pPap )
        {
            for( int i = 0; i < 5; ++i, ++pIds )
                nBorder |= (_SetWW8_BRC( bVer67, brc[ i ], pPap->HasSprm( *pIds )))<<i;
        }
        else if( pSty )
        {
            for( int i = 0; i < 5; ++i, ++pIds )
                nBorder |= (_SetWW8_BRC( bVer67, brc[ i ], pSty->HasParaSprm( *pIds )))<<i;
        }
        else
            ASSERT( pSty || pPap, "WW8PLCFx_Cp_FKP and WW8RStyle "
                               "and WW8PLCFx_SEPX is 0" );
    }

    return nBorder;
}

void GetLineIndex(SvxBoxItem &rBox, short nLineThickness, short nSpace, BYTE nCol, short nIdx,
    USHORT nOOIndex, USHORT nWWIndex, short *pSize=0)
{
    WW8_BordersSO::eBorderCode eCodeIdx;

    //Word mirrors some indexes inside outside depending on its position, we
    //don't do that, so flip them here
    if (nWWIndex == WW8_TOP || nWWIndex == WW8_LEFT)
    {
        switch (nIdx)
        {
            case 11:
            case 12:
                nIdx = (nIdx == 11) ? 12 : 11;
                break;
            case 14:
            case 15:
                nIdx = (nIdx == 14) ? 15 : 14;
                break;
            case 17:
            case 18:
                nIdx = (nIdx == 17) ? 18 : 17;
                break;
            case 24:
            case 25:
                nIdx = (nIdx == 24) ? 25 : 24;
                break;
        }
    }

    // Map to our border types, we should use of one equal line
    // thickness, or one of smaller thickness. If too small we
    // can make the defecit up in additional white space or
    // object size
    switch (nIdx)
    {
        // First the single lines
        case  1:
        case  2:
        case  5:
        // and the unsupported special cases which we map to a single line
        case  6:
        case  7:
        case  8:
        case  9:
        case 22:
        // or if in necessary by a double line
        case 24:
        case 25:
            if( nLineThickness < 20)
                eCodeIdx = WW8_BordersSO::single0;//   1 Twip for us
            else if (nLineThickness < 50)
                eCodeIdx = WW8_BordersSO::single1;//  20 Twips
            else if (nLineThickness < 80)
                eCodeIdx = WW8_BordersSO::single2;//  50
            else if (nLineThickness < 100)
                eCodeIdx = WW8_BordersSO::single3;//  80
            else if (nLineThickness < 150)
                eCodeIdx = WW8_BordersSO::single4;// 100
            // Hack: for the quite thick lines we must paint double lines,
            // because our singles lines don't come thicker than 5 points.
            else if (nLineThickness < 180)
                eCodeIdx = WW8_BordersSO::double2;// 150
            else
                eCodeIdx = WW8_BordersSO::double5;// 180
        break;
        // then the shading beams which we represent by a double line
        case 23:
            eCodeIdx = WW8_BordersSO::double1;
        break;
        // then the double lines, for which we have good matches
        case  3:
        case 10: //Don't have tripple so use double
            if (nLineThickness < 60)
                eCodeIdx = WW8_BordersSO::double0;// 22 Twips for us
            else if (nLineThickness < 135)
                eCodeIdx = WW8_BordersSO::double7;// some more space
            else if (nLineThickness < 180)
                eCodeIdx = WW8_BordersSO::double1;// 60
            else
                eCodeIdx = WW8_BordersSO::double2;// 150
            break;
        case 11:
            eCodeIdx = WW8_BordersSO::double4;//  90 Twips for us
            break;
        case 12:
        case 13: //Don't have thin thick thin, so use thick thin
            if (nLineThickness < 87)
                eCodeIdx = WW8_BordersSO::double8;//  71 Twips for us
            else if (nLineThickness < 117)
                eCodeIdx = WW8_BordersSO::double9;// 101
            else if (nLineThickness < 166)
                eCodeIdx = WW8_BordersSO::double10;// 131
            else
                eCodeIdx = WW8_BordersSO::double5;// 180
            break;
        case 14:
            if (nLineThickness < 46)
                eCodeIdx = WW8_BordersSO::double0;//  22 Twips for us
            else if (nLineThickness < 76)
                eCodeIdx = WW8_BordersSO::double1;//  60
            else if (nLineThickness < 121)
                eCodeIdx = WW8_BordersSO::double4;//  90
            else if (nLineThickness < 166)
                eCodeIdx = WW8_BordersSO::double2;// 150
            else
                eCodeIdx = WW8_BordersSO::double6;// 180
            break;
        case 15:
        case 16: //Don't have thin thick thin, so use thick thin
            if (nLineThickness < 46)
                eCodeIdx = WW8_BordersSO::double0;//  22 Twips for us
            else if (nLineThickness < 76)
                eCodeIdx = WW8_BordersSO::double1;//  60
            else if (nLineThickness < 121)
                eCodeIdx = WW8_BordersSO::double3;//  90
            else if (nLineThickness < 166)
                eCodeIdx = WW8_BordersSO::double2;// 150
            else
                eCodeIdx = WW8_BordersSO::double5;// 180
            break;
        case 17:
            if (nLineThickness < 46)
                eCodeIdx = WW8_BordersSO::double0;//  22 Twips for us
            else if (nLineThickness < 72)
                eCodeIdx = WW8_BordersSO::double7;//  52
            else if (nLineThickness < 137)
                eCodeIdx = WW8_BordersSO::double4;//  90
            else
                eCodeIdx = WW8_BordersSO::double6;// 180
        break;
        case 18:
        case 19: //Don't have thin thick thin, so use thick thin
            if (nLineThickness < 46)
                eCodeIdx = WW8_BordersSO::double0;//  22 Twips for us
            else if (nLineThickness < 62)
                eCodeIdx = WW8_BordersSO::double7;//  52
            else if (nLineThickness < 87)
                eCodeIdx = WW8_BordersSO::double8;//  71
            else if (nLineThickness < 117)
                eCodeIdx = WW8_BordersSO::double9;// 101
            else if (nLineThickness < 156)
                eCodeIdx = WW8_BordersSO::double10;// 131
            else
                eCodeIdx = WW8_BordersSO::double5;// 180
            break;
        case 20:
            if (nLineThickness < 46)
                eCodeIdx = WW8_BordersSO::single1; //  20 Twips for us
            else
                eCodeIdx = WW8_BordersSO::double1;//  60
            break;
        case 21:
            eCodeIdx = WW8_BordersSO::double1;//  60 Twips for us
            break;
        default:
            eCodeIdx = WW8_BordersSO::single0;
            break;
    }

    const WW8_BordersSO& rBorders = WW8_BordersSO::Get0x01LineMatch(eCodeIdx);
    SvxBorderLine aLine;
    aLine.SetOutWidth(rBorders.mnOut);
    aLine.SetInWidth(rBorders.mnIn);
    aLine.SetDistance(rBorders.mnDist);

    //No AUTO for borders as yet, so if AUTO, use BLACK
    if (nCol == 0)
        nCol = 1;

    aLine.SetColor(SwWW8ImplReader::GetCol(nCol));

    if (pSize)
        pSize[nWWIndex] = nLineThickness+nSpace;

    rBox.SetLine(&aLine, nOOIndex);
    rBox.SetDistance(nSpace, nOOIndex);

}

void Set1Border(bool bVer67, SvxBoxItem &rBox, const WW8_BRC& rBor,
    USHORT nOOIndex, USHORT nWWIndex, short *pSize=0)
{
    BYTE nCol;
    short nSpace, nIdx;
    short nLineThickness = rBor.DetermineBorderProperties(bVer67,&nSpace,&nCol,
        &nIdx);

    GetLineIndex(rBox, nLineThickness, nSpace, nCol, nIdx, nOOIndex, nWWIndex, pSize );

}

bool lcl_IsBorder(bool bVer67, const WW8_BRC* pbrc, bool bChkBtwn = false)
{
    if( bVer67  )
        return ( pbrc[WW8_TOP  ].aBits1[0] & 0x18 ) ||  // brcType  != 0
               ( pbrc[WW8_LEFT ].aBits1[0] & 0x18 ) ||
               ( pbrc[WW8_BOT  ].aBits1[0] & 0x18 ) ||
               ( pbrc[WW8_RIGHT].aBits1[0] & 0x18 ) ||
               ( bChkBtwn && ( pbrc[WW8_BETW ].aBits1[0] )) ||
               //can have dotted and dashed with a brcType of 0
               ( (pbrc[WW8_TOP  ].aBits1[0] & 0x07)+1 > 6) ||
               ( (pbrc[WW8_LEFT ].aBits1[0] & 0x07)+1 > 6) ||
               ( (pbrc[WW8_BOT  ].aBits1[0] & 0x07)+1 > 6) ||
               ( (pbrc[WW8_RIGHT].aBits1[0] & 0x07)+1 > 6) ||
               ( bChkBtwn && ( (pbrc[WW8_BETW ].aBits1[0] & 0x07)+1 > 6))
               ;
                // Abfrage auf 0x1f statt 0x18 ist noetig, da zumindest einige
                // WW-Versionen ( 6.0 US ) bei dotted brcType auf 0 setzen
    else
        return pbrc[WW8_TOP  ].aBits1[1] ||         // brcType  != 0
               pbrc[WW8_LEFT ].aBits1[1] ||
               pbrc[WW8_BOT  ].aBits1[1] ||
               pbrc[WW8_RIGHT].aBits1[1] ||
               (bChkBtwn && pbrc[WW8_BETW ].aBits1[1]);
}

bool SwWW8ImplReader::IsBorder(const WW8_BRC* pbrc, bool bChkBtwn)
{
    return lcl_IsBorder( bVer67, pbrc, bChkBtwn );
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
    short *pSizeArray, BYTE nSetBorders, bool bChkBtwn)
{
    bool bChange = false;
    static const USHORT aIdArr[ 10 ] =
    {
        WW8_TOP,    BOX_LINE_TOP,
        WW8_LEFT,   BOX_LINE_LEFT,
        WW8_RIGHT,  BOX_LINE_RIGHT,
        WW8_BOT,    BOX_LINE_BOTTOM,
        WW8_BETW,   BOX_LINE_BOTTOM
    };

    for( int i = 0, nEnd = 8; i < nEnd; i += 2 )
    {
        // ungueltige Borders ausfiltern
        const WW8_BRC& rB = pbrc[ aIdArr[ i ] ];
        if( !rB.IsEmpty(bVer67))
        {
            Set1Border(bVer67, rBox, rB, aIdArr[i+1], aIdArr[i], pSizeArray);
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
            appropiate border
            */
            rBox.SetLine( 0, aIdArr[ i+1 ] );
        }
        else if( 6 == i && bChkBtwn )   // wenn Botton nichts war,
            nEnd += 2;                  // dann ggfs. auch Between befragen
    }
    return bChange;
}


bool SwWW8ImplReader::SetShadow(SvxShadowItem& rShadow, const SvxBoxItem& rBox,
    const WW8_BRC pbrc[4] )
{
    bool bRet = ( bVer67 ? (pbrc[WW8_RIGHT].aBits1[ 1 ] & 0x20 )
                         : (pbrc[WW8_RIGHT].aBits2[ 1 ] & 0x20 ) ) &&
                rBox.GetRight();
    if( bRet )
    {
        rShadow.SetColor( Color( COL_BLACK ));

//          aS.SetWidth( 28 );
// JP 19.11.98: abhaengig von der Breite der rechten Kante der Box
        const SvxBorderLine& rLine = *rBox.GetRight();
        rShadow.SetWidth( ( rLine.GetOutWidth() + rLine.GetInWidth() +
                        rLine.GetDistance() ) );

        rShadow.SetLocation( SVX_SHADOW_BOTTOMRIGHT );
        bRet = true;
    }
    return bRet;
}

void SwWW8ImplReader::GetBorderDistance(const WW8_BRC* pbrc,
    Rectangle& rInnerDist)
{
    // 'dptSpace' is stored in 3 bits of 'Border Code (BRC)'
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
    const WW8_BRC pbrc[4], short *pSizeArray )
{
    bool bShadowed = false;
    if( IsBorder( pbrc ) )
    {
        SvxBoxItem aBox;
        SetBorder( aBox, pbrc, pSizeArray );

        rFlySet.Put( aBox );

        // fShadow
        SvxShadowItem aShadow;
        if( SetShadow( aShadow, aBox, pbrc ))
        {
            bShadowed = true;
            rFlySet.Put( aShadow );
        }
    }
    return bShadowed;
}

//-----------------------------------------
//              APOs
//-----------------------------------------
                            // fuer Berechnung der minimalen FrameSize
#define MAX_BORDER_SIZE 210         // so breit ist max. der Border
#define MAX_EMPTY_BORDER 10         // fuer +-1-Fehler, mindestens 1

static void FlySecur1(short& rSize, const bool bBorder)
{
    register short nMin = MINFLY +
        bBorder ? MAX_BORDER_SIZE : MAX_EMPTY_BORDER;

    if ( rSize < nMin )
        rSize = nMin;
}

inline bool SetValSprm( INT16* pVar, WW8PLCFx_Cp_FKP* pPap, USHORT nId )
{
    register const BYTE* pS = pPap->HasSprm( nId );
    if( pS )
        *pVar = (INT16)SVBT16ToShort( pS );
    return ( pS != 0 );
}

inline bool SetValSprm( INT16* pVar, const WW8RStyle* pStyle, USHORT nId )
{
    register const BYTE* pS = pStyle->HasParaSprm( nId );
    if( pS )
        *pVar = (INT16)SVBT16ToShort( pS );
    return ( pS != 0 );
}

/*
#i1930 revealed that sprm 0x360D as used in tables can affect the frame
around the table. Its full structure is not fully understood as yet.
*/
void WW8FlyPara::ApplyTabPos(WW8_TablePos *pTabPos)
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
        nSp37 = 2;                                  // Default: Umfluss
    }
    bVer67 = bIsVer67;
}

bool WW8FlyPara::operator==(const WW8FlyPara& rSrc) const
{
    /*
     Compare the parts that word seems to compare for equivalence.
     Interestingly being autoheight or absolute height (the & 0x7fff) doesn't
     matter to word e.g. #110507#
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

// Read fuer normalen Text
void WW8FlyPara::Read(const BYTE* pSprm29, WW8PLCFx_Cp_FKP* pPap)
{
    sal_uInt8 nOrigSp29 = nSp29;
    if (pSprm29)
        nOrigSp29 = *pSprm29;                           // PPC ( Bindung )

    const BYTE* pS = 0;
    if( bVer67 )
    {
        SetValSprm( &nSp26, pPap, 26 ); // X-Position   //sprmPDxaAbs
        //set in me or in parent style
        mbVertSet |= SetValSprm( &nSp27, pPap, 27 );    // Y-Position   //sprmPDyaAbs
        SetValSprm( &nSp45, pPap, 45 ); // Hoehe        //sprmPWHeightAbs
        SetValSprm( &nSp28, pPap, 28 ); // Breite       //sprmPDxaWidth
        SetValSprm( &nLeMgn, pPap, 49 ); // L-Raender   //sprmPDxaFromText
        SetValSprm( &nRiMgn, pPap, 49 ); // R-Raender   //sprmPDxaFromText
        SetValSprm( &nUpMgn, pPap, 48 ); // U-Raender   //sprmPDyaFromText
        SetValSprm( &nLoMgn, pPap, 48 ); // D-Raender   //sprmPDyaFromText

        pS = pPap->HasSprm( 37 );                       //sprmPWr
        if( pS )
            nSp37 = *pS;
    }
    else
    {
        SetValSprm( &nSp26, pPap, 0x8418 ); // X-Position
        //set in me or in parent style
        mbVertSet |= SetValSprm( &nSp27, pPap, 0x8419 );    // Y-Position
        SetValSprm( &nSp45, pPap, 0x442B ); // Hoehe
        SetValSprm( &nSp28, pPap, 0x841A ); // Breite
        SetValSprm( &nLeMgn, pPap, 0x842F );    // L-Raender
        SetValSprm( &nRiMgn, pPap, 0x842F );    // R-Raender
        SetValSprm( &nUpMgn, pPap, 0x842E );    // U-Raender
        SetValSprm( &nLoMgn, pPap, 0x842E );    // D-Raender

        pS = pPap->HasSprm( 0x2423 );                               // Umfluss
        if( pS )
            nSp37 = *pS;
    }

    if( ::lcl_ReadBorders( bVer67, brc, pPap ))     // Umrandung
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

void WW8FlyPara::ReadFull(const BYTE* pSprm29, SwWW8ImplReader* pIo)
{
    WW8PLCFMan* pPlcxMan = pIo->pPlcxMan;
    WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();

    Read(pSprm29, pPap);    // Lies Apo-Parameter

    do{             // Block zum rausspringen
        if( nSp45 != 0 /* || nSp28 != 0 */ )
            break;                      // bGrafApo nur bei Hoehe automatisch
        if( pIo->pWwFib->fComplex )
            break;                      // (*pPap)++ geht bei FastSave schief
                                        // -> bei FastSave kein Test auf Grafik-APO
        SvStream* pIoStrm = pIo->pStrm;
        ULONG nPos = pIoStrm->Tell();
        WW8PLCFxSave1 aSave;
        pPlcxMan->GetPap()->Save( aSave );
        bGrafApo = false;

        do{             // Block zum rausspringen

            BYTE nTxt[2];

            pIoStrm->Read( nTxt, 2 );                   // lies Text
            if( nTxt[0] != 0x01 || nTxt[1] != 0x0d )// nur Grafik + CR ?
                break;                              // Nein

            (*pPap)++;                              // Naechste Zeile

            // In APO ?
            //sprmPPc
            const BYTE* pS = pPap->HasSprm( bVer67 ? 29 : 0x261B );

            // Nein -> Grafik-Apo
            if( !pS ){
                bGrafApo = true;
                break;                              // Ende des APO
            }
            USHORT nColl = pPap->GetIstd();
            WW8FlyPara *pNowStyleApo=0;
            while (nColl < pIo->nColls &&
                !(pNowStyleApo = pIo->pCollA[nColl].pWWFly))
            {
                nColl = pIo->pCollA[nColl].nBase;
            }

            WW8FlyPara aF(bVer67, pNowStyleApo);
                                                // Neuer FlaPara zum Vergleich
            aF.Read( pS, pPap );                // WWPara fuer neuen Para
            if( !( aF == *this ) )              // selber APO ? ( oder neuer ? )
                bGrafApo = true;                // nein -> 1-zeiliger APO
                                                //      -> Grafik-APO
        }
        while( 0 );                             // Block zum rausspringen

        pPlcxMan->GetPap()->Restore( aSave );
        pIoStrm->Seek( nPos );
    }while( 0 );                                    // Block zum rausspringen
}


// Read fuer Apo-Defs in Styledefs
void WW8FlyPara::Read(const BYTE* pSprm29, WW8RStyle* pStyle)
{
    sal_uInt8 nOrigSp29 = 0;
    if (pSprm29)
        nOrigSp29 = *pSprm29;                           // PPC ( Bindung )

    const BYTE* pS = 0;
    if (bVer67)
    {
        SetValSprm( &nSp26, pStyle, 26 );   // X-Position
        //set in me or in parent style
        mbVertSet |= SetValSprm(&nSp27, pStyle, 27);    // Y-Position
        SetValSprm( &nSp45, pStyle, 45 );   // Hoehe
        SetValSprm( &nSp28, pStyle, 28 );   // Breite
        SetValSprm( &nLeMgn,    pStyle, 49 );   // L-Raender
        SetValSprm( &nRiMgn,    pStyle, 49 );   // R-Raender
        SetValSprm( &nUpMgn,    pStyle, 48 );   // U-Raender
        SetValSprm( &nLoMgn,    pStyle, 48 );   // D-Raender

        pS = pStyle->HasParaSprm( 37 );             // Umfluss
        if( pS )
            nSp37 = *pS;
    }
    else
    {
        SetValSprm( &nSp26, pStyle, 0x8418 );   // X-Position
        //set in me or in parent style
        mbVertSet |= SetValSprm(&nSp27, pStyle, 0x8419);    // Y-Position
        SetValSprm( &nSp45, pStyle, 0x442B );   // Hoehe
        SetValSprm( &nSp28, pStyle, 0x841A );   // Breite
        SetValSprm( &nLeMgn, pStyle, 0x842F );  // L-Raender
        SetValSprm( &nRiMgn, pStyle, 0x842F );  // R-Raender
        SetValSprm( &nUpMgn, pStyle, 0x842E );  // U-Raender
        SetValSprm( &nLoMgn, pStyle, 0x842E );  // D-Raender

        pS = pStyle->HasParaSprm( 0x2423 );             // Umfluss
        if( pS )
            nSp37 = *pS;
    }

    if (::lcl_ReadBorders(bVer67, brc, 0, pStyle))      // Umrandung
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
    ASSERT(aEmpty.nSp37 == 2, "this is not what we expect for nSp37");
    if (this->nSp37 == 0)
        aEmpty.nSp37 = 0;
    if (aEmpty == *this)
        return true;
    return false;
}

WW8SwFlyPara::WW8SwFlyPara( SwPaM& rPaM, SwWW8ImplReader& rIo, WW8FlyPara& rWW,
    short nPgLeft, short nPgWidth, INT32 nIniFlyDx, INT32 nIniFlyDy )
{
    memset( this, 0, sizeof( WW8SwFlyPara ) );  // Initialisieren
    nNewNettoWidth = MINFLY;                    // Minimum

    eSurround = ( rWW.nSp37 > 1 ) ? SURROUND_IDEAL : SURROUND_NONE;

    /*
     #95905#, #83307# seems to have gone away now, so reenable parallel
     wrapping support for frames in headers/footers. I don't know if we truly
     have an explictly specified behaviour for these circumstances.
    */
#if 0
    /*
     #83307# These old style WinWord textboxes are a terrible problem for
     headers and footers. They can be anchored in a footer and moved to just
     about anyplace. Also they have flexible heights and can be higher than
     the actual header/footer area, if we do not set wrap to NONE then boxes
     narrower than a header/footer but taller will be forced into the
     header/footer area and the look of the word original will be lost
    */
    if (rIo.bIsHeader || rIo.bIsFooter)
        eSurround = SURROUND_NONE;
#endif

    nHeight = rWW.nSp45;
    if( nHeight & 0x8000 )
    {
        nHeight &= 0x7fff;
        eHeightFix = ATT_MIN_SIZE;
    }
    else
        eHeightFix = ATT_FIX_SIZE;

    if( nHeight <= MINFLY )
    {                           // keine Angabe oder Stuss
        eHeightFix = ATT_MIN_SIZE;
        nHeight = MINFLY;
    }

    nWidth = nNettoWidth = rWW.nSp28;
    if( nWidth <= 10 )                              // Auto-Breite
    {
        bAutoWidth = true;
        rIo.maTracer.Log(sw::log::eAutoWidthFrame);
        nWidth = nNettoWidth = (nPgWidth ? nPgWidth : 2268); // 4 cm
    }
    if( nWidth <= MINFLY )
        nWidth = nNettoWidth = MINFLY;              // Minimale Breite

    eVAlign = VERT_NONE;                            // Defaults
    eHAlign = HORI_NONE;
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

    // Wenn der Fly links, rechts, oben oder unten aligned ist,
    // wird der aeussere Textabstand ignoriert, da sonst
    // der Fly an falscher Position landen wuerde
    // JP 18.11.98: Problematisch wird es nur bei Innen/Aussen

    // Bindung
    nYBind = (( rWW.nSp29 & 0x30 ) >> 4);
    switch ( nYBind )
    {                                       // Y - Bindung bestimmt Sw-Bindung
        case 0:
            eAnchor = FLY_PAGE;             // Vert Margin
            eVRel = REL_PG_PRTAREA;
            break;
        case 1:
            eAnchor = FLY_PAGE;             // Vert Page
            eVRel = REL_PG_FRAME;
            break;                          // 2=Vert. Paragraph, 3=Use Default
        default:
            eAnchor = FLY_AT_CNTNT;
            eVRel = PRTAREA;
            if( nYPos < 0 )
                nYPos = 0;                  // koennen wir nicht
            break;
    }

    switch( rWW.nSp27 )             // besondere Y-Positionen ?
    {
        case -4:
            eVAlign = VERT_TOP;
            if (eAnchor == FLY_PAGE)
                nUpMgn = 0;
            break;  // oben
        case -8:
            eVAlign = VERT_CENTER;
            break;  // zentriert
        case -12:
            eVAlign = VERT_BOTTOM;
            if (eAnchor == FLY_PAGE)
                nLoMgn = 0;
            break;  // unten
        default:
            nYPos = rWW.nSp27 + (short)nIniFlyDy;
            break;  // Korrekturen per Ini-Datei
    }

    switch( rWW.nSp26 )                 // besondere X-Positionen ?
    {
        case 0:
            eHAlign = HORI_LEFT;
            nLeMgn = 0;
            break;  // links
        case -4:
            eHAlign = HORI_CENTER;
            break;  // zentriert
        case -8:
            eHAlign = HORI_RIGHT;
            nRiMgn = 0;
            break;  // rechts
        case -12:
            eHAlign = HORI_LEFT;
            bToggelPos = true;
            break;  // innen
        case -16:
            eHAlign = HORI_RIGHT;
            bToggelPos = true;
            break;  // aussen
        default:
            nXPos = rWW.nSp26 + (short)nIniFlyDx;
            break;  // Korrekturen per Ini-Datei
    }

    nXBind = ( rWW.nSp29 & 0xc0 ) >> 6;
    switch ( nXBind )           // X - Bindung -> Koordinatentransformation
    {
        case 0:
            eHRel = (FLY_PAGE == eAnchor) ? REL_PG_PRTAREA : PRTAREA;
            break;
        case 1:                                 // Hor. Absatz
            eHRel = REL_PG_PRTAREA;
            break;
    /*  case 2:*/                               // Hor. Seite
    /*  case 3:*/                               // Use Default
        default:
            eHRel = (FLY_AT_CNTNT== eAnchor) ? REL_PG_FRAME : FRAME;

            // important: allways set REL_PG_FRAME in sections with columns
            if (eHRel != REL_PG_FRAME)
            {
                const SwSectionNode* pSectNd
                    = rPaM.GetPoint()->nNode.GetNode().FindSectionNode();
                if (pSectNd)
                {
                    const SwSectionFmt* pFmt = pSectNd->GetSection().GetFmt();
                    if (pFmt)
                    {
                        if (1 < pFmt->GetCol().GetNumCols())
                            eHRel = REL_PG_FRAME;
                    }
                }
            }
            break;
    }

    if (rWW.bBorderLines)
    {
        /*
        #i582#
        Word has a curious bug where the offset stored do not take into
        account the internal distance from the corner both
        */
        INT16 nLeLMgn = 0;
        INT16 nTemp = rWW.brc[WW8_LEFT].DetermineBorderProperties(rWW.bVer67,
            &nLeLMgn);
        nLeLMgn += nTemp;

        if (nLeLMgn)
        {
            if (eHAlign == HORI_LEFT)
                eHAlign = HORI_NONE;
            nXPos -= nLeLMgn;
        }
    }

    FlySecur1( nWidth, rWW.bBorderLines );          // passen Raender ?
    FlySecur1( nHeight, rWW.bBorderLines );

    /*
        // eine Writer-Kuriositaet: auch wenn Abstaende vom Seitenrand
        // gezaehlt werden sollen, muessen die Positionen als Abstaende vom
        // Papierrand angegeben werden
        // bei Absatzgebundenen Frames geht die Zaehlung immer von
        // der Printarea aus
    */
    if( (FRAME == eHRel) && (FLY_AT_CNTNT == eAnchor) )
    {
        // hier duerfen neg. Werte bis minimal -nPgLeft entstehen
        nXPos -= nPgLeft;
        if( rIo.nInTable )
            nXPos -= rIo.GetTableLeft();
    }
}

// hat ein Fly in WW eine automatische Breite, dann muss das durch
// nachtraegliches Anpassen der ( im SW festen ) Fly-Breite simuliert werden.
// Dabei kann die Fly-Breite groesser oder kleiner werden, da der Default-Wert
// ohne Wissen ueber den Inhalt eingesetzt wird.
void WW8SwFlyPara::BoxUpWidth( long nInWidth )
{
    if( bAutoWidth && nInWidth > nNewNettoWidth )
        nNewNettoWidth = nInWidth;
};

// Die Klasse WW8FlySet ist von SfxItemSet abgeleitet und stellt auch
// im Prizip nicht mehr zur Verfuegung, ist aber fuer mich besser
// zu handeln
// WW8FlySet-ctor fuer Apos und Graf-Apos
WW8FlySet::WW8FlySet(SwWW8ImplReader& rReader, const WW8FlyPara* pFW,
    const WW8SwFlyPara* pFS, bool bGraf)
    : SfxItemSet(rReader.rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
{
    if (!rReader.mbNewDoc)
        Reader::ResetFrmFmtAttrs(*this);    // Abstand/Umrandung raus
                                            // Position
/*Below can all go when we have from left in rtl mode*/
    long nXPos = pFS->nXPos;
    SwRelationOrient eHRel = pFS->eHRel;
    if ((pFS->eAnchor == FLY_PAGE) && (eHRel == FRAME))
        eHRel = REL_PG_FRAME;
    rReader.MiserableRTLGraphicsHack(nXPos, pFS->nWidth, pFS->eHAlign, eHRel);
/*Above can all go when we have from left in rtl mode*/
    Put( SwFmtHoriOrient(nXPos, pFS->eHAlign, pFS->eHRel, pFS->bToggelPos ));
    Put( SwFmtVertOrient( pFS->nYPos, pFS->eVAlign, pFS->eVRel ) );

    if (pFS->nLeMgn || pFS->nRiMgn)     // Raender setzen
        Put(SvxLRSpaceItem(pFS->nLeMgn, pFS->nRiMgn));

    if (pFS->nUpMgn || pFS->nLoMgn)
        Put(SvxULSpaceItem(pFS->nUpMgn, pFS->nLoMgn));

    //This is only a hack: #110876#
    if ((rReader.bIsHeader || rReader.bIsFooter))
        Put(SwFmtSurround(SURROUND_THROUGHT));
    else
        Put(SwFmtSurround(pFS->eSurround));

    short aSizeArray[5]={0};
    rReader.SetFlyBordersShadow(*this,(const WW8_BRC*)pFW->brc,&aSizeArray[0]);

    // der 5. Parameter ist immer 0, daher geht beim Cast nix verloren

    if( !bGraf )
    {
        Put( SwFmtAnchor(pFS->eAnchor) );
        // Groesse einstellen

        //Ordinarily with frames, the border width and spacing is
        //placed outside the frame, making it larger. With these
        //types of frames, the left right thickness and space makes
        //it wider, but the top bottom spacing and border thickness
        //is placed inside.
        Put( SwFmtFrmSize( pFS->eHeightFix, pFS->nWidth +
            aSizeArray[WW8_LEFT] + aSizeArray[WW8_RIGHT],
            pFS->nHeight));
    }
}

// WW8FlySet-ctor fuer zeichengebundene Grafiken
WW8FlySet::WW8FlySet( SwWW8ImplReader& rReader, const SwPaM* pPaM,
    const WW8_PIC& rPic, long nWidth, long nHeight )
    : SfxItemSet(rReader.rDoc.GetAttrPool(),RES_FRMATR_BEGIN,RES_FRMATR_END-1)
{
    if (!rReader.mbNewDoc)
        Reader::ResetFrmFmtAttrs( *this );  // Abstand/Umrandung raus

    Put( SvxLRSpaceItem() ); //inline writer ole2 objects start with 0.2cm l/r
    SwFmtAnchor aAnchor( FLY_IN_CNTNT );

    aAnchor.SetAnchor( pPaM->GetPoint() );
    Put( aAnchor );

    //The horizontal default is on the baseline, the vertical is centered
    //around the character center it appears
    if (rReader.maSectionManager.CurrentSectionIsVertical())
        Put(SwFmtVertOrient(0, VERT_CHAR_CENTER,REL_CHAR));
    else
        Put(SwFmtVertOrient(0, VERT_TOP, FRAME));

    Put(SvxFrameDirectionItem(FRMDIR_HORI_LEFT_TOP));

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
        Put(SvxLRSpaceItem( aSizeArray[WW8_LEFT], 0 ) );
        Put(SvxULSpaceItem( aSizeArray[WW8_TOP], 0 ));
        aSizeArray[WW8_RIGHT]*=2;
        aSizeArray[WW8_BOT]*=2;
    }

    Put( SwFmtFrmSize( ATT_FIX_SIZE, nWidth+aSizeArray[WW8_LEFT]+
        aSizeArray[WW8_RIGHT], nHeight+aSizeArray[WW8_TOP]
        + aSizeArray[WW8_BOT]) );
}

WW8DupProperties::WW8DupProperties(SwDoc &rDoc,
    SwWW8FltControlStack *pStk) :
    pCtrlStck(pStk),
    aChrSet(rDoc.GetAttrPool(), RES_CHRATR_BEGIN, RES_CHRATR_END - 1 ),
    aParSet(rDoc.GetAttrPool(), RES_PARATR_BEGIN, RES_PARATR_END - 1 )
{
    //Close any open character properties and duplicate them inside the
    //first table cell
    USHORT nCnt = pCtrlStck->Count();
    for (USHORT i=0; i < nCnt; i++)
    {
        const SwFltStackEntry* pEntry = (*pCtrlStck)[ i ];
        if(pEntry->bLocked)
        {
            if (pEntry->pAttr->Which() > RES_CHRATR_BEGIN &&
                pEntry->pAttr->Which() < RES_CHRATR_END)
            {
                aChrSet.Put( *pEntry->pAttr );

            }
            else if (pEntry->pAttr->Which() > RES_PARATR_BEGIN &&
                pEntry->pAttr->Which() < RES_PARATR_END)
            {
                aParSet.Put( *pEntry->pAttr );
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

    // Setze Pam in den FlyFrame
    const SwFmtCntnt& rCntnt = pFlyFmt->GetCntnt();
    ASSERT( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
    pPaM->GetPoint()->nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
    pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );

    aDup.Insert(*pPaM->GetPoint());
}

sal_uInt16 SwWW8ImplReader::MoveOutsideFly(SwFrmFmt *pFlyFmt,
    const SwPosition &rPos, bool bTableJoin)
{
    sal_uInt16 nRetWidth = 0;
    // Alle Attribute schliessen, da sonst Attribute entstehen koennen,
    // die aus Flys rausragen
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
                    aIdx++;
                    if ( (aIdx < aEnd) && aIdx.GetNode().IsTxtNode() )
                    {
                        SwTxtNode *pNd = aIdx.GetNode().GetTxtNode();
                        aIdx++;
                        if (aIdx == aEnd && pNd && !pNd->GetTxt().Len())
                        {
                            rDoc.DelFullPara( *pPaM );

                            SwTable& rTable = pTable->GetTable();
                            SwFrmFmt* pTblFmt = rTable.GetFrmFmt();

                            if (pTblFmt)
                            {
                                SwFmtFrmSize aSize = pTblFmt->GetFrmSize();
                                aSize.SetSizeType(ATT_MIN_SIZE);
                                aSize.SetHeight(MINLAY);
                                pFlyFmt->SetAttr(aSize);
                                pTblFmt->SetAttr(SwFmtHoriOrient(0,HORI_FULL));
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
    WW8_TablePos *pTabPos)
{
    WW8FlyPara *pRet = 0;
    ASSERT(rApo.HasFrame() || pTabPos,
        "If no frame found, *MUST* be in a table");

    pRet = new WW8FlyPara(bVer67, rApo.mpStyleApo);

    // APO-Parameter ermitteln und Test auf bGrafApo
    if (rApo.HasFrame())
        pRet->ReadFull(rApo.mpSprm29, this);

    pRet->ApplyTabPos(pTabPos);

    if (pRet->IsEmpty())
        delete pRet, pRet = 0;
    return pRet;
}

bool SwWW8ImplReader::IsDropCap()
{
    // Find the DCS (Drop Cap Specifier) for the paragraph
    // if does not exist or if the first three bits are 0
    // then there is no dropcap on the paragraph
    WW8PLCFx_Cp_FKP *pPap = pPlcxMan ? pPlcxMan->GetPapPLCF() : 0;
    if (pPap)
    {
        const BYTE *pDCS = pPlcxMan->GetPapPLCF()->HasSprm(0x442C);
        if(pDCS)
        {
            short nDCS = SVBT16ToShort( pDCS );
            if((nDCS | 7) != 0)
                return true;
        }
    }
    return false;
}

bool SwWW8ImplReader::StartApo(const ApoTestResults &rApo,
    WW8_TablePos *pTabPos)
{
    if (!(pWFlyPara = ConstructApo(rApo, pTabPos)))
        return false;

    pSFlyPara = new WW8SwFlyPara( *pPaM, *this, *pWFlyPara,
        maSectionManager.GetPageLeft(),
        (maSectionManager.GetPageWidth() - maSectionManager.GetPageRight() -
         maSectionManager.GetPageLeft()), nIniFlyDx, nIniFlyDy );

    // If this paragraph is a Dropcap set the flag and we will deal with it later
    if (IsDropCap())
    {
        bDropCap = true;
        return false;
    }

    if( !pWFlyPara->bGrafApo )
    {

        // Innerhalb des GrafApo muessen Textattribute ignoriert werden, da
        // sie sonst auf den folgenden Zeilen landen.  Der Rahmen wird nur
        // eingefuegt, wenn er *nicht* nur zum Positionieren einer einzelnen
        // Grafik dient.  Ist es ein Grafik-Rahmen, dann werden pWFlyPara und
        // pSFlyPara behalten und die
        // daraus resultierenden Attribute beim Einfuegen der Grafik auf die
        // Grafik angewendet.

        WW8FlySet aFlySet(*this, pWFlyPara, pSFlyPara, false);

        pSFlyPara->pFlyFmt = rDoc.MakeFlySection( pSFlyPara->eAnchor,
            pPaM->GetPoint(), &aFlySet );
        ASSERT(pSFlyPara->pFlyFmt->GetAnchor().GetAnchorId() ==
            pSFlyPara->eAnchor, "Not the anchor type requested!");

        if (pSFlyPara->pFlyFmt)
        {
            if (!pDrawModel)
                GrafikCtor();

            SdrObject* pOurNewObject = CreateContactObject(pSFlyPara->pFlyFmt);
            pWWZOrder->InsertTextLayerObject(pOurNewObject);
        }

        if (FLY_IN_CNTNT != pSFlyPara->eAnchor)
            pAnchorStck->AddAnchor(*pPaM->GetPoint(),pSFlyPara->pFlyFmt);

        // merke Pos im Haupttext
        pSFlyPara->pMainTextPos = new SwPosition( *pPaM->GetPoint() );

        //remove fltanchors, otherwise they will be closed inside the
        //frame, which makes no sense, restore them after the frame is
        //closed
        pSFlyPara->pOldAnchorStck = pAnchorStck;
        pAnchorStck = new SwWW8FltAnchorStack(&rDoc, nFieldFlags);

        MoveInsideFly(pSFlyPara->pFlyFmt);

        // 1) ReadText() wird nicht wie beim W4W-Reader rekursiv aufgerufen,
        //    da die Laenge des Apo zu diesen Zeitpunkt noch nicht feststeht,
        //    ReadText() diese Angabe aber braucht.
        // 2) Der CtrlStck wird nicht neu erzeugt.
        //    die Char-Attribute laufen weiter ( AErger mit SW-Attributen )
        //    Paraattribute muessten am Ende jeden Absatzes zurueckgesetzt
        //    sein, d.h. es duerften am Absatzende keine Paraattribute
        //    auf dem Stack liegen
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
    rPam.GetPoint()->nContent = 0;          // an den Anfang der Zeile gehen

    SwNodeIndex aPref(rPam.GetPoint()->nNode, -1);

    if (SwTxtNode* pNode = aPref.GetNode().GetTxtNode())
    {
        maSectionManager.JoinNode(*rPam.GetPoint(), aPref.GetNode());
        rPam.GetPoint()->nNode = aPref;
        rPam.GetPoint()->nContent.Assign(pNode, pNode->GetTxt().Len());
        if (bStealAttr)
            pCtrlStck->StealAttr(rPam.GetPoint());

        pNode->JoinNext();

        bRet = true;
    }
    return bRet;
}

void SwWW8ImplReader::StopApo()
{
    ASSERT(pWFlyPara, "no pWFlyPara to close");
    if (!pWFlyPara)
        return;
    if (pWFlyPara->bGrafApo)
    {
        // Grafik-Rahmen, der *nicht* eingefuegt wurde leeren Absatz incl.
        // Attributen entfernen
        JoinNode(*pPaM, true);

    }
    else
    {
        if (!pSFlyPara->pMainTextPos || !pWFlyPara)
        {
            ASSERT( pSFlyPara->pMainTextPos, "StopApo: pMainTextPos ist 0" );
            ASSERT( pWFlyPara, "StopApo: pWFlyPara ist 0" );
            return;
        }

        /*
        ##582##
        Take the last paragraph background colour and fill the frame with it.
        This is how MSWord works
        */
        const SfxPoolItem *pItem = GetFmtAttr(RES_BACKGROUND);
        if (pItem)
            pSFlyPara->pFlyFmt->SetAttr(*pItem);

        /*
        #104920#
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

        sal_uInt16 nNewWidth =
            MoveOutsideFly(pSFlyPara->pFlyFmt, *pSFlyPara->pMainTextPos);
        if (nNewWidth)
            pSFlyPara->BoxUpWidth(nNewWidth);

        if (SwTxtNode* pNode = aPref.GetNode().GetTxtNode())
            pNode->JoinNext();

        DeleteAnchorStk();
        pAnchorStck = pSFlyPara->pOldAnchorStck;

        // Ist die Fly-Breite durch eine innenliegende Grafik vergroessert
        // worden ( bei automatischer Breite des Flys ), dann muss die Breite
        // des SW-Flys entsprechend umgesetzt werden, da der SW keine
        // automatische Breite kennt.
        if( pSFlyPara->nNewNettoWidth > MINFLY )    // BoxUpWidth ?
        {
            long nW = pSFlyPara->nNewNettoWidth;
            nW += pSFlyPara->nWidth - pSFlyPara->nNettoWidth;   // Rand dazu
            pSFlyPara->pFlyFmt->SetAttr(
                SwFmtFrmSize( pSFlyPara->eHeightFix, nW, pSFlyPara->nHeight ) );
        }
        /*
        #83307# Word set *no* width meaning its an automatic width. The
        SwFlyPara reader will have already set a fallback width of the
        printable regions width, so we should reuse it. Despite the related
        problems with layout addressed with a hack in WW8FlyPara's constructor
        */
        else if( !pWFlyPara->nSp28 )
        {
            SfxItemSet aFlySet( pSFlyPara->pFlyFmt->GetAttrSet() );
            aFlySet.ClearItem( RES_FRM_SIZE );
            CalculateFlySize( aFlySet, pSFlyPara->pMainTextPos->nNode,
                pSFlyPara->nWidth );
            pSFlyPara->pFlyFmt->SetAttr( aFlySet.Get( RES_FRM_SIZE ) );
        }

        DELETEZ( pSFlyPara->pMainTextPos );

// Damit die Frames bei Einfuegen in existierendes Doc erzeugt werden,
// wird in fltshell.cxx beim Setzen des FltAnchor-Attributes
// pFlyFrm->MakeFrms() gerufen

    }

    //#i8062#
    if (pSFlyPara && pSFlyPara->pFlyFmt)
        pFmtOfJustInsertedApo = pSFlyPara->pFlyFmt;

    DELETEZ( pSFlyPara );
    DELETEZ( pWFlyPara );
}

// TestSameApo() beantwortet die Frage, ob es dasselbe APO oder ein neues ist
bool SwWW8ImplReader::TestSameApo(const ApoTestResults &rApo,
    WW8_TablePos *pTabPos)
{
    if( !pWFlyPara )
    {
        ASSERT( pWFlyPara, " Wo ist mein pWFlyPara ? " );
        return true;
    }

    // Es muss ein kompletter Vergleich ( ausser Borders ) stattfinden, um
    // alle Kombinationen Style / Hart richtig einzuordnen. Deshalb wird ein
    // temporaerer WW8FlyPara angelegt ( abh. ob Style oder nicht ), darauf
    // die harten Attrs angewendet, und dann verglichen

    // Zum Vergleich
    WW8FlyPara aF(bVer67, rApo.mpStyleApo);
    // WWPara fuer akt. Para
    if (rApo.HasFrame())
        aF.Read(rApo.mpSprm29, pPlcxMan->GetPapPLCF());
    aF.ApplyTabPos(pTabPos);

    return aF == *pWFlyPara;
}

/***************************************************************************
#       Attribut - Verwaltung
#**************************************************************************/

void SwWW8ImplReader::NewAttr( const SfxPoolItem& rAttr )
{
    if( !bNoAttrImport ) // zum Ignorieren von Styles beim Doc-Einfuegen
    {
        if (pAktColl)
        {
            ASSERT(rAttr.Which() != RES_FLTR_REDLINE, "redline in style!");
            pAktColl->SetAttr(rAttr);
        }
        else if (pAktItemSet)
            pAktItemSet->Put(rAttr);
        else if (rAttr.Which() == RES_FLTR_REDLINE)
            mpRedlineStack->open(*pPaM->GetPoint(), rAttr);
        else
            pCtrlStck->NewAttr(*pPaM->GetPoint(), rAttr);
    }
}

// holt Attribut aus der FmtColl / Stack / Doc
const SfxPoolItem* SwWW8ImplReader::GetFmtAttr( USHORT nWhich )
{
    const SfxPoolItem* pRet = 0;
    if (pAktColl)
        pRet = &(pAktColl->GetAttr(nWhich));
    else if (pAktItemSet)
    {
        pRet = pAktItemSet->GetItem(nWhich);
        if (!pRet)
            pRet = pStandardFmtColl ? &(pStandardFmtColl->GetAttr(nWhich)) : 0;
        if (!pRet)
            pRet = &rDoc.GetAttrPool().GetDefaultItem(nWhich);
    }
    else if (pPlcxMan && pPlcxMan->GetDoingDrawTextBox())
    {
        pRet = pCtrlStck->GetStackAttr(*pPaM->GetPoint(), nWhich);
        if (!pRet)
        {
            if (nAktColl < nColls && pCollA[nAktColl].pFmt &&
                pCollA[nAktColl].bColl)
            {
                pRet = &(pCollA[nAktColl].pFmt->GetAttr(nWhich));
            }
        }
        if (!pRet)
            pRet = pStandardFmtColl ? &(pStandardFmtColl->GetAttr(nWhich)) : 0;
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

void SwWW8ImplReader::Read_Special(USHORT, const BYTE* pData, short nLen)
{
    if( nLen < 0 )
    {
        bSpec = false;
        return;
    }
    bSpec = ( *pData != 0 );
}

// Read_Obj wird fuer fObj und fuer fOle2 benutzt !
void SwWW8ImplReader::Read_Obj(USHORT , const BYTE* pData, short nLen)
{
    if( nLen < 0 )
        bObj = false;
    else
    {
        bObj = 0 != *pData;

        if( bObj && nPicLocFc && bEmbeddObj )
            nObjLocFc = nPicLocFc;
    }
}

void SwWW8ImplReader::Read_PicLoc(USHORT , const BYTE* pData, short nLen )
{
    if( nLen < 0 )
    {
        nPicLocFc = 0;
        bSpec = false;  // Stimmt das immer ?
    }
    else
    {
        nPicLocFc = SVBT32ToLong( pData );
        bSpec = true;

        if( bObj && nPicLocFc && bEmbeddObj )
            nObjLocFc = nPicLocFc;
    }
}

void SwWW8ImplReader::Read_POutLvl(USHORT, const BYTE* pData, short nLen )
{
    if (pAktColl && (0 < nLen))
    {
        if (SwWW8StyInf* pSI = GetStyle(nAktColl))
        {
            pSI->nOutlineLevel =
                ( (1 <= pSI->GetWWStyleId()) && (9 >= pSI->GetWWStyleId()) )
            ? pSI->GetWWStyleId()-1
            : (pData ? *pData : 0);
        }
    }
}

void SwWW8ImplReader::Read_Symbol(USHORT, const BYTE* pData, short nLen )
{
    if( !bIgnoreText )
    {
        if( nLen < 0 )
        {
            //otherwise disable after we print the char
            if (pPlcxMan && pPlcxMan->GetDoingDrawTextBox())
                pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
            bSymbol = false;
        }
        else
        {
            // Make new Font-Atribut
            // (will be closed in SwWW8ImplReader::ReadChars() )

            //Will not be added to the charencoding stack, for styles the real
            //font setting will be put in as the styles charset, and for plain
            //text encoding for symbols is moot. Drawing boxes will check bSymbol
            //themselves so they don't need to add it to the stack either.
            if (SetNewFontAttr(SVBT16ToShort( pData ), false, RES_CHRATR_FONT))
            {
                if( bVer67 )
                {
                    cSymbol = ByteString::ConvertToUnicode(
                        *(sal_Char*)(pData+2), RTL_TEXTENCODING_MS_1252 );
                }
                else
                    cSymbol = SVBT16ToShort( pData+2 );
                bSymbol = true;
            }
        }
    }
}

SwWW8StyInf *SwWW8ImplReader::GetStyle(USHORT nColl) const
{
    return nColl < nColls ? &pCollA[nColl] : 0;
}

/***************************************************************************
#  Zeichen - Attribute
#**************************************************************************/

// Read_BoldUsw fuer Italic, Bold, Kapitaelchen, Versalien, durchgestrichen,
// Contour und Shadow
void SwWW8ImplReader::Read_BoldUsw( USHORT nId, const BYTE* pData, short nLen )
{
    static const USHORT nEndIds[ 8 + 2 ] =
    {
        RES_CHRATR_WEIGHT,          RES_CHRATR_POSTURE,
        RES_CHRATR_CROSSEDOUT,      RES_CHRATR_CONTOUR,
        RES_CHRATR_SHADOWED,        RES_CHRATR_CASEMAP,
        RES_CHRATR_CASEMAP,         RES_CHRATR_CROSSEDOUT,

        RES_CHRATR_CJK_WEIGHT,      RES_CHRATR_CJK_POSTURE
    };

    BYTE nI;
    // die Attribut-Nr fuer "doppelt durchgestrichen" tanzt aus der Reihe
    if( 0x2A53 != nId )
        nI = bVer67 ? nId - 85 : nId - 0x0835;  // Index 0..6
    else
        nI = 7;                         // Index 7 (Doppelt durchgestrichen)

    BYTE nMask = 1 << nI;

    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nEndIds[ nI ] );
        // reset the CJK Weight and Posture
        if( nI < 2 )
            pCtrlStck->SetAttr( *pPaM->GetPoint(), nEndIds[ 8 + nI ] );
        pCtrlStck->SetToggleAttr(nI, false);
        return;
    }
    // Wert: 0 = Aus, 1 = An, 128 = Wie Style, 129 entgegen Style
    bool bOn = *pData & 1;
    SwWW8StyInf* pSI = GetStyle(nAktColl);
    if (pPlcxMan)
    {
        const BYTE *pCharIstd =
            pPlcxMan->GetChpPLCF()->HasSprm(bVer67 ? 80 : 0x4A30);
        if (pCharIstd)
            pSI = GetStyle(SVBT16ToShort(pCharIstd));
    }

    if( pAktColl )                          // StyleDef -> Flags merken
    {
        if (pSI)
        {
            // The style based on has Bit 7 set ?
            if (
                pSI->nBase < nColls && (*pData & 0x80) &&
                (pCollA[pSI->nBase].n81Flags & nMask)
               )
            {
                bOn = !bOn;                     // umdrehen
            }

            if (bOn)
                pSI->n81Flags |= nMask;         // Flag setzen
            else
                pSI->n81Flags &= ~nMask;        // Flag loeschen
       }
    }
    else
    {

        // im Text -> Flags abfragen
        if( *pData & 0x80 )                 // Bit 7 gesetzt ?
        {
            if (pSI && pSI->n81Flags & nMask)       // und in StyleDef an ?
                bOn = !bOn;                 // dann invertieren
            // am Stack vermerken, das dieses ein Toggle-Attribut ist
            pCtrlStck->SetToggleAttr(nI, true);
        }
    }

    SetToggleAttr( nI, bOn );
}

// Read_BoldUsw for BiDi Italic, Bold
void SwWW8ImplReader::Read_BoldBiDiUsw(USHORT nId, const BYTE* pData,
    short nLen)
{
    static const USHORT nEndIds[2] =
    {
        RES_CHRATR_CTL_WEIGHT, RES_CHRATR_CTL_POSTURE,
    };

    BYTE nI = bVer67 ? nId - 111 : nId - 0x085C;     // Index 0..2 (for now)

    ASSERT(nI <= 1, "not happening");
    if (nI > 1)
        return;

    BYTE nMask = 1 << nI;

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
        const BYTE *pCharIstd =
            pPlcxMan->GetChpPLCF()->HasSprm(bVer67 ? 80 : 0x4A30);
        if (pCharIstd)
            pSI = GetStyle(SVBT16ToShort(pCharIstd));
    }

    if (pAktColl)                           // StyleDef -> Flags merken
    {
        if (pSI)
        {
            if( pSI->nBase < nColls             // Style Based on
                && ( *pData & 0x80 )            // Bit 7 gesetzt ?
                && ( pCollA[pSI->nBase].n81BiDiFlags & nMask ) ) // BasisMaske ?
                    bOn = !bOn;                     // umdrehen

            if( bOn )
                pSI->n81BiDiFlags |= nMask;         // Flag setzen
            else
                pSI->n81BiDiFlags &= ~nMask;        // Flag loeschen
        }
    }
    else
    {

        // im Text -> Flags abfragen
        if (*pData & 0x80)                  // Bit 7 gesetzt ?
        {
            if (pSI && pSI->n81BiDiFlags & nMask) // und in StyleDef an ?
                bOn = !bOn;                     // dann invertieren
            // am Stack vermerken, das dieses ein Toggle-Attribut ist
            pCtrlStck->SetToggleBiDiAttr(nI, true);
        }
    }

    SetToggleBiDiAttr(nI, bOn);
}

void SwWW8ImplReader::SetToggleBiDiAttr(BYTE nAttrId, bool bOn)
{
    switch(nAttrId)
    {
        case 0:
            {
                SvxWeightItem aAttr( bOn ? WEIGHT_BOLD : WEIGHT_NORMAL );
                aAttr.SetWhich( RES_CHRATR_CTL_WEIGHT );
                NewAttr( aAttr );
            }
            break;
        case 1:
            {
                SvxPostureItem aAttr( bOn ? ITALIC_NORMAL : ITALIC_NONE );
                aAttr.SetWhich( RES_CHRATR_CTL_POSTURE );
                NewAttr( aAttr );
            }
            break;
    }
}

void SwWW8ImplReader::SetToggleAttr(BYTE nAttrId, bool bOn)
{
    switch( nAttrId )
    {
        case 0:
            {
                SvxWeightItem aAttr( bOn ? WEIGHT_BOLD : WEIGHT_NORMAL );
                NewAttr( aAttr );
                aAttr.SetWhich( RES_CHRATR_CJK_WEIGHT );
                NewAttr( aAttr );
            }
            break;
        case 1:
            {
                SvxPostureItem aAttr( bOn ? ITALIC_NORMAL : ITALIC_NONE );
                NewAttr( aAttr );
                aAttr.SetWhich( RES_CHRATR_CJK_POSTURE );
                NewAttr( aAttr );
            }
            break;
        case 2:
            NewAttr(SvxCrossedOutItem(bOn ? STRIKEOUT_SINGLE : STRIKEOUT_NONE));
            break;
        case 3:
            NewAttr( SvxContourItem( bOn ) );
            break;
        case 4:
            NewAttr( SvxShadowedItem( bOn ) );
            break;
        case 5:
            NewAttr( SvxCaseMapItem( bOn ? SVX_CASEMAP_KAPITAELCHEN
                                              : SVX_CASEMAP_NOT_MAPPED ) );
            break;
        case 6:
            NewAttr( SvxCaseMapItem( bOn ? SVX_CASEMAP_VERSALIEN
                                             : SVX_CASEMAP_NOT_MAPPED ) );
            break;
        case 7:
            NewAttr( SvxCrossedOutItem( bOn ? STRIKEOUT_DOUBLE
                                                : STRIKEOUT_NONE ) );
            break;
    }
}

void SwWW8ImplReader::_ChkToggleAttr( USHORT nOldStyle81Mask,
                                        USHORT nNewStyle81Mask )
{
    USHORT i = 1, nToggleAttrFlags = pCtrlStck->GetToggleAttrFlags();
    BYTE n = 0;
    for( ; n < 7; ++n, i <<= 1 )
        if( (i & nToggleAttrFlags) &&
            ( (i & nOldStyle81Mask) != (i & nNewStyle81Mask)))
        {
            SetToggleAttr( n, 0 != (i & nOldStyle81Mask ) );
        }
}

void SwWW8ImplReader::_ChkToggleBiDiAttr( USHORT nOldStyle81Mask,
                                        USHORT nNewStyle81Mask )
{
    USHORT i = 1, nToggleAttrFlags = pCtrlStck->GetToggleBiDiAttrFlags();
    BYTE n = 0;
    for( ; n < 7; ++n, i <<= 1 )
        if( (i & nToggleAttrFlags) &&
            ( (i & nOldStyle81Mask) != (i & nNewStyle81Mask)))
        {
            SetToggleBiDiAttr( n, 0 != (i & nOldStyle81Mask ) );
        }
}


void SwWW8ImplReader::Read_SubSuper( USHORT, const BYTE* pData, short nLen )
{
    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
        return;
    }

    short nEs;
    BYTE nProp;
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
    NewAttr( SvxEscapementItem( nEs, nProp ) );
}

SwFrmFmt *SwWW8ImplReader::ContainsSingleInlineGraphic(const SwPaM &rRegion)
{
    /*
    #92489# & #92946#
    For inline graphics and objects word has a hacked in feature to use
    subscripting to force the graphic into a centered position on the line, so
    we must check when applying sub/super to see if it the subscript range
    contains only a single graphic, and if that graphic is anchored as
    FLY_IN_CNTNT and then we can change its anchoring to centered in the line.
    */
    SwFrmFmt *pRet=0;
    SwNodeIndex aBegin(rRegion.Start()->nNode);
    xub_StrLen nBegin(rRegion.Start()->nContent.GetIndex());
    SwNodeIndex aEnd(rRegion.End()->nNode);
    xub_StrLen nEnd(rRegion.End()->nContent.GetIndex());
    const SwTxtNode* pTNd;
    const SwTxtAttr* pTFlyAttr;
    if (
         aBegin == aEnd && nBegin == nEnd - 1 &&
         (pTNd = aBegin.GetNode().GetTxtNode()) &&
         (pTFlyAttr = pTNd->GetTxtAttr(nBegin, RES_TXTATR_FLYCNT))
       )
    {
        const SwFmtFlyCnt& rFly = pTFlyAttr->GetFlyCnt();
        SwFrmFmt *pFlyFmt = rFly.GetFrmFmt();
        if( pFlyFmt &&
            FLY_IN_CNTNT == pFlyFmt->GetAnchor().GetAnchorId() )
        {
            pRet = pFlyFmt;
        }
    }
    return pRet;
}

bool SwWW8ImplReader::ConvertSubToGraphicPlacement()
{
    /*
    #92489# & #92946#
    For inline graphics and objects word has a hacked in feature to use
    subscripting to force the graphic into a centered position on the line, so
    we must check when applying sub/super to see if it the subscript range
    contains only a single graphic, and if that graphic is anchored as
    FLY_IN_CNTNT and then we can change its anchoring to centered in the line.
    */
    bool bIsGraphicPlacementHack = false;
    USHORT nPos;
    if (pCtrlStck->GetFmtStackAttr(RES_CHRATR_ESCAPEMENT, &nPos))
    {
        SwPaM aRegion(*pPaM->GetPoint());

        SwFltStackEntry aEntry = *((*pCtrlStck)[nPos]);
        aEntry.SetEndPos(*pPaM->GetPoint());

        SwFrmFmt *pFlyFmt = 0;
        if (
             aEntry.MakeRegion(&rDoc,aRegion,false) &&
             (pFlyFmt = ContainsSingleInlineGraphic(aRegion))
           )
        {
            pCtrlStck->DeleteAndDestroy(nPos);
            pFlyFmt->SetAttr(SwFmtVertOrient(0, VERT_CHAR_CENTER, REL_CHAR));
            bIsGraphicPlacementHack = true;
        }
    }
    return bIsGraphicPlacementHack;
}

void SwWW8ImplReader::Read_SubSuperProp( USHORT, const BYTE* pData, short nLen )
{
    if( nLen < 0 )
    {
        if (!ConvertSubToGraphicPlacement())
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
        return;
    }

    short nPos = SVBT16ToShort( pData );    // Font-Position in HalfPoints
    INT32 nPos2 = nPos * ( 10 * 100 );      // HalfPoints in 100 * tw
    const SvxFontHeightItem* pF
        = (const SvxFontHeightItem*)GetFmtAttr( RES_CHRATR_FONTSIZE );
    nPos2 /= (INT32)pF->GetHeight();        // ... nun in % ( gerundet )
    if( nPos2 > 100 )                       // zur Sicherheit
        nPos2 = 100;
    if( nPos2 < -100 )
        nPos2 = -100;
    SvxEscapementItem aEs( (short)nPos2, 100 );
    NewAttr( aEs );
}

void SwWW8ImplReader::Read_Underline( USHORT, const BYTE* pData, short nLen )
{
    FontUnderline eUnderline = UNDERLINE_NONE;
    bool bWordLine = false;
    if( pData )
    {
        // Parameter:  0 = none,    1 = single,  2 = by Word,
                    // 3 = double,  4 = dotted,  5 = hidden
                    // 6 = thick,   7 = dash,    8 = dot(not used)
                    // 9 = dotdash 10 = dotdotdash 11 = wave


        // pruefe auf Sonderfall "fett+unterstrichen"
        bool bAlsoBold = /*( 6 == b )*/ false;
        // erst mal ggfs. *bold* einschalten!
        if( bAlsoBold )
        {
            BYTE nOn = 1;
            Read_BoldUsw( 0x0835, &nOn, nLen );
            eUnderline = UNDERLINE_SINGLE;
        }
        else
        {
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
    }

    // dann Stack ggfs. verwursteln und exit!
    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_UNDERLINE );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_WORDLINEMODE );
    }
    else
    {
        NewAttr( SvxUnderlineItem( eUnderline ));
        if( bWordLine )
            NewAttr(SvxWordLineModeItem(true));
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
void SwWW8ImplReader::Read_DoubleLine_Rotate( USHORT, const BYTE* pData,
    short nLen )
{
    if( nLen < 0 ) // close the tag
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_TWO_LINES );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_ROTATE );
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
                case 1: cStt = '(', cEnd = ')'; break;
                case 2: cStt = '[', cEnd = ']'; break;
                case 3: cStt = '<', cEnd = '>'; break;
                case 4: cStt = '{', cEnd = '}'; break;
                }
                NewAttr( SvxTwoLinesItem( sal_True, cStt, cEnd ));
            }
            break;

        case 1:                         // rotated characters
            {
                bool bFitToLine = 0 != *(pData+1);
                NewAttr( SvxCharRotateItem( 900, bFitToLine ));
            }
            break;
        }
    }
}

void SwWW8ImplReader::Read_TxtColor( USHORT, const BYTE* pData, short nLen )
{
    //Has newer colour varient, ignore this old varient
    if (!bVer67 && pPlcxMan && pPlcxMan->GetChpPLCF()->HasSprm(0x6870))
        return;

    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
    else
    {
        BYTE b = *pData;            // Parameter: 0 = Auto, 1..16 Farben

        if( b > 16 )                // unbekannt -> Black
            b = 0;

        NewAttr( SvxColorItem(Color(GetCol(b))));
        if (pAktColl && pStyles)
            pStyles->bTxtColChanged = true;
    }
}

sal_uInt32 wwUtility::BGRToRGB(sal_uInt32 nColor)
{
    sal_uInt8
        r(static_cast<sal_uInt8>(nColor&0xFF)),
        g(static_cast<sal_uInt8>(((nColor)>>8)&0xFF)),
        b(static_cast<sal_uInt8>((nColor>>16)&0xFF)),
        t(static_cast<sal_uInt8>((nColor>>24)&0xFF));
    nColor = (t<<24) + (r<<16) + (g<<8) + b;
    return nColor;
}

void SwWW8ImplReader::Read_TxtForeColor(USHORT, const BYTE* pData, short nLen)
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
    else
    {
        Color aColor(wwUtility::BGRToRGB(SVBT32ToLong(pData)));
        NewAttr(SvxColorItem(aColor));
        if (pAktColl && pStyles)
            pStyles->bTxtColChanged = true;
    }
}

bool SwWW8ImplReader::GetFontParams( USHORT nFCode, FontFamily& reFamily,
    String& rName, FontPitch& rePitch, CharSet& reCharSet )
{
    // Die Defines, aus denen diese Tabellen erzeugt werden, stehen in windows.h
    static const FontPitch ePitchA[] =
    {
        PITCH_DONTKNOW, PITCH_FIXED, PITCH_VARIABLE, PITCH_DONTKNOW
    };

    static const FontFamily eFamilyA[] =
    {
        FAMILY_DONTKNOW, FAMILY_ROMAN, FAMILY_SWISS, FAMILY_MODERN,
        FAMILY_SCRIPT, FAMILY_DECORATIVE
    };

    const WW8_FFN* pF = pFonts->GetFont( nFCode );  // Info dazu
    if( !pF )                                   // FontNummer unbekannt ?
        return false;                           // dann ignorieren

    rName = String( pF->sFontname );

    // pF->prg : Pitch
    rePitch = ePitchA[pF->prg];

    // pF->chs: Charset
    if( 77 == pF->chs )             // Mac-Font im Mac-Charset oder
        reCharSet = eTextCharSet;   // auf ANSI-Charset uebersetzt
    else
        reCharSet = rtl_getTextEncodingFromWindowsCharset( pF->chs );

    // pF->ff : Family
    BYTE b = pF->ff;

    // make sure Font Family Code is set correctly
    // at least for the most important fonts
    // ( might be set wrong when Doc was not created by
    //   Winword but by third party program like Applixware... )
        /*
        0: FAMILY_DONTKNOW
        1: FAMILY_ROMAN
        2: FAMILY_SWISS
        3: FAMILY_MODERN
        4: FAMILY_SCRIPT
        5: FAMILY_DECORATIVE
    */
#define FONTNAMETAB_SZ    14
#define MAX_FONTNAME_ROMAN 6
    static const sal_Char
        // first comes ROMAN
        sFontName0[] = "\x07""Tms Rmn",
        sFontName1[] = "\x07""Timmons",
        sFontName2[] = "\x08""CG Times",
        sFontName3[] = "\x08""MS Serif",
        sFontName4[] = "\x08""Garamond",
        sFontName5[] = "\x11""Times Roman",
        sFontName6[] = "\x15""Times New Roman",
        // from here SWISS --> see above: #define MAX_FONTNAME_ROMAN 6
        sFontName7[] = "\x04""Helv",
        sFontName8[] = "\x05""Arial",
        sFontName9[] = "\x07""Univers",
        sFontName10[]= "\x11""LinePrinter",
        sFontName11[]= "\x11""Lucida Sans",
        sFontName12[]= "\x11""Small Fonts",
        sFontName13[]= "\x13""MS Sans Serif";
    static const sal_Char* const aFontNameTab[ FONTNAMETAB_SZ ] =
    {
        sFontName0,  sFontName1,  sFontName2,  sFontName3,
        sFontName4,  sFontName5,  sFontName6,  sFontName7,
        sFontName8,  sFontName9,  sFontName10, sFontName11,
        sFontName12, sFontName13
    };

    for( USHORT n = 0;  n < FONTNAMETAB_SZ; n++ )
    {
        const sal_Char* pCmp = aFontNameTab[ n ];
        int nLen = *pCmp++;
        if( rName.EqualsIgnoreCaseAscii(pCmp, 0, nLen) )
        {
            b = n <= MAX_FONTNAME_ROMAN ? 1 : 2;
            break;
        }
    }
    if( b < sizeof( eFamilyA ) )
        reFamily = eFamilyA[b];
    else
        reFamily = FAMILY_DONTKNOW;

    return true;
}

bool SwWW8ImplReader::SetNewFontAttr(USHORT nFCode, bool bSetEnums,
    USHORT nWhich)
{
    FontFamily eFamily;
    String aName;
    FontPitch ePitch;
    CharSet eSrcCharSet;

    if( !GetFontParams( nFCode, eFamily, aName, ePitch, eSrcCharSet ) )
    {
        //If we fail (and are not doing a style) then put something into the
        //character encodings stack anyway so that the property end that pops
        //off the stack will keep in sync
        if (!pAktColl && !pAktItemSet)
        {
            if (!maFontSrcCharSets.empty())
                eSrcCharSet = maFontSrcCharSets.top();
            else
                eSrcCharSet = RTL_TEXTENCODING_DONTKNOW;
            maFontSrcCharSets.push(eSrcCharSet);
        }
        return false;
    }

    CharSet eDstCharSet = eSrcCharSet;

    SvxFontItem aFont( eFamily, aName, aEmptyStr, ePitch, eDstCharSet, nWhich);

    if( bSetEnums )
    {
        if( pAktColl ) // StyleDef
        {
            switch(nWhich)
            {
                default:
                case RES_CHRATR_FONT:
                    pCollA[nAktColl].eLTRFontSrcCharSet = eSrcCharSet;
                    break;
                case RES_CHRATR_CTL_FONT:
                    pCollA[nAktColl].eRTLFontSrcCharSet = eSrcCharSet;
                    break;
                case RES_CHRATR_CJK_FONT:
                    pCollA[nAktColl].eCJKFontSrcCharSet = eSrcCharSet;
                    break;
            }
        }
        else if (!pAktItemSet)
        {
            //Add character text encoding to stack
            maFontSrcCharSets.push(eSrcCharSet);
        }
    }

    NewAttr( aFont );                       // ...und 'reinsetzen

    return true;
}

void SwWW8ImplReader::ResetCharSetVars()
{
    ASSERT(!maFontSrcCharSets.empty(),"no charset to remove");
    if (!maFontSrcCharSets.empty())
        maFontSrcCharSets.pop();
}

/*
    Font ein oder ausschalten:
*/
void SwWW8ImplReader::Read_FontCode( USHORT nId, const BYTE* pData, short nLen )
{
    if (!bSymbol)           // falls bSymbol, gilt der am Symbol
    {                       // (siehe sprmCSymbol) gesetzte Font !
        switch( nId )
        {
    //      case 0x4a51:    //font to bias towards all else being equal ?
            case 113:
            case 0x4a5E:
                nId = RES_CHRATR_CTL_FONT;
                break;
            case 93:
            case 0x4a4f:
                nId = RES_CHRATR_FONT;
                break;
            case 0x4a50:
                nId = RES_CHRATR_CJK_FONT;
                break;
            default:
                return ;
        }

        if( nLen < 0 ) // Ende des Attributes
        {
            pCtrlStck->SetAttr( *pPaM->GetPoint(), nId );
            ResetCharSetVars();
        }
        else
        {
            USHORT nFCode = SVBT16ToShort( pData );     // Font-Nummer
            if (SetNewFontAttr(nFCode, true, nId)   // Lies Inhalt
                && pAktColl && pStyles )                // Style-Def ?
            {
                // merken zur Simulation Default-Font
                if (RES_CHRATR_CJK_FONT == nId)
                    pStyles->bCJKFontChanged = true;
                else if (RES_CHRATR_CTL_FONT == nId)
                    pStyles->bCTLFontChanged = true;
                else
                    pStyles->bFontChanged = true;
            }
        }
    }
}

void SwWW8ImplReader::Read_FontSize( USHORT nId, const BYTE* pData, short nLen )
{
    switch( nId )
    {
        case 99:
        case 0x4a43:
            nId = RES_CHRATR_FONTSIZE;
            break;
        case 116:
        case 0x4a61:
            nId = RES_CHRATR_CTL_FONTSIZE;
            break;
        default:
            return ;
    }

    if( nLen < 0 )          // Ende des Attributes
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nId  );
        if( RES_CHRATR_FONTSIZE == nId )  // reset additional the CJK size
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_CJK_FONTSIZE );
    }
    else
    {
        USHORT nFSize = SVBT16ToShort( pData ) * 10;         // Font-Groesse in halben Point
                                                //  10 = 1440 / ( 72 * 2 )

        SvxFontHeightItem aSz( nFSize, 100, nId );
        NewAttr( aSz );
        if( RES_CHRATR_FONTSIZE == nId )  // set additional the CJK size
        {
            aSz.SetWhich( RES_CHRATR_CJK_FONTSIZE );
            NewAttr( aSz );
        }
        if (pAktColl && pStyles)            // Style-Def ?
        {
            // merken zur Simulation Default-FontSize
            if (nId == RES_CHRATR_CTL_FONTSIZE)
                pStyles->bFCTLSizeChanged = true;
            else
                pStyles->bFSizeChanged = true;
        }
    }
}



void SwWW8ImplReader::Read_CharSet(USHORT , const BYTE* pData, short nLen)
{
    if( nLen < 0 )
    {                   // Ende des Attributes
        eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
        return;
    }
    BYTE nfChsDiff = SVBT8ToByte( pData );

    if( nfChsDiff )
        eHardCharSet = rtl_getTextEncodingFromWindowsCharset( *(pData + 1) );
    else
        eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
}

void SwWW8ImplReader::Read_Language( USHORT nId, const BYTE* pData, short nLen )
{
    switch( nId )
    {
        case 97:
        case 0x486D:
        case 0x4873: //Methinks, uncertain
            nId = RES_CHRATR_LANGUAGE;
            break;
        case 0x486E:
            nId = RES_CHRATR_CJK_LANGUAGE;
            break;
        case 0x485F:
            nId = RES_CHRATR_CTL_LANGUAGE;
            break;
        default:
            return ;
    }

    if( nLen < 0 )                  // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), nId );
    else
    {
        USHORT nLang = SVBT16ToShort( pData );  // Language-Id
        NewAttr(SvxLanguageItem((const LanguageType)nLang, nId));
    }
}

/*
    Einschalten des Zeichen-Styles:
*/
void SwWW8ImplReader::Read_CColl( USHORT, const BYTE* pData, short nLen )
{
    if( nLen < 0 ){                 // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_TXTATR_CHARFMT );
        nCharFmt = -1;
        return;
    }
    USHORT nId = SVBT16ToShort( pData );    // Style-Id (NICHT Sprm-Id!)

    if( nId >= nColls || !pCollA[nId].pFmt  // ungueltige Id ?
        || pCollA[nId].bColl )              // oder Para-Style ?
        return;                             // dann ignorieren

    NewAttr( SwFmtCharFmt( (SwCharFmt*)pCollA[nId].pFmt ) );
    nCharFmt = (short) nId;
}


/*
    enger oder weiter als normal:
*/
void SwWW8ImplReader::Read_Kern( USHORT, const BYTE* pData, short nLen )
{
    if( nLen < 0 ){                 // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_KERNING );
        return;
    }
    INT16 nKern = SVBT16ToShort( pData );    // Kerning in Twips
    NewAttr( SvxKerningItem( nKern ) );
}

void SwWW8ImplReader::Read_FontKern( USHORT, const BYTE* , short nLen )
{
    if( nLen < 0 ) // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_AUTOKERN );
    else
        NewAttr(SvxAutoKernItem(true));
}

void SwWW8ImplReader::Read_CharShadow(  USHORT, const BYTE* pData, short nLen )
{
    //Has newer colour varient, ignore this old varient
    if (!bVer67 && pPlcxMan && pPlcxMan->GetChpPLCF()->HasSprm(0xCA71))
        return;

    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
        if( bCharShdTxtCol )
        {
            // Zeichenfarbe auch
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

void SwWW8ImplReader::Read_TxtBackColor(USHORT, const BYTE* pData, short nLen )
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
        if( bCharShdTxtCol )
        {
            // Zeichenfarbe auch
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
            bCharShdTxtCol = false;
        }
    }
    else
    {
        ASSERT(nLen == 10, "Len of para back colour not 10!");
        if (nLen != 10)
            return;
        Color aColour(ExtractColour(pData, bVer67));
        NewAttr(SvxBrushItem(aColour, RES_CHRATR_BACKGROUND));
    }
}

void SwWW8ImplReader::Read_CharHighlight(USHORT, const BYTE* pData, short nLen)
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_BACKGROUND );
        if( bCharShdTxtCol )
        {
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );  // Zeichenfarbe auch
            bCharShdTxtCol = false;
        }
    }
    else
    {
        BYTE b = *pData;            // Parameter: 0 = Auto, 1..16 Farben

        if( b > 16 )                // unbekannt -> Black
            b = 0;                  // Auto -> Black

        Color aCol(GetCol(b));
        NewAttr( SvxBrushItem( aCol , RES_CHRATR_BACKGROUND ));
    }
}


/***************************************************************************
#  Absatz - Attribute
#**************************************************************************/

void SwWW8ImplReader::Read_NoLineNumb(USHORT , const BYTE* pData, short nLen)
{
    if( nLen < 0 )  // Ende des Attributes
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_LINENUMBER );
        return;
    }
    SwFmtLineNumber aLN;
    aLN.SetCountLines( pData && (0 == *pData) );
    NewAttr( aLN );
}

bool sw::util::AdjustTabs(long nDestLeft, long nSrcLeft, SvxTabStopItem &rTStop)
{
    bool bChanged = false;
    if (nDestLeft != nSrcLeft)
    {
        for (USHORT nCnt = 0; nCnt < rTStop.Count(); ++nCnt)
        {
            SvxTabStop& rTab = const_cast<SvxTabStop&>(rTStop[nCnt]);
            if (SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment())
            {
                rTab.GetTabPos() += nSrcLeft;
                rTab.GetTabPos() -= nDestLeft;
                bChanged = true;
            }
        }
    }
    return bChanged;
}

void SwWW8ImplReader::AdjustStyleTabStops(short nLeft, SwWW8StyInf *pWWSty)
{
    const SfxPoolItem* pTabs=0;
    bool bOnMarginStyle(false);
    if (pWWSty->pFmt)
    {
        bOnMarginStyle = pWWSty->pFmt->GetAttrSet().GetItemState(
                RES_PARATR_TABSTOP, false, &pTabs ) == SFX_ITEM_SET;
    }

    if (pTabs)
    {
        SvxTabStopItem aTStop(*(const SvxTabStopItem*)pTabs);
        long nOldLeft = 0;

        const SwTxtFmtColl* pSty = 0;
        USHORT nTabBase = pWWSty->nBase;
        if (nTabBase < nColls)              // Based On
            pSty = (const SwTxtFmtColl*)pCollA[nTabBase].pFmt;

        while (pSty && !bOnMarginStyle)
        {
            bOnMarginStyle = pSty->GetAttrSet().GetItemState(RES_PARATR_TABSTOP,
                false, &pTabs ) == SFX_ITEM_SET;
            if (bOnMarginStyle)
            {
                const SvxLRSpaceItem &rLR =
                    ItemGet<SvxLRSpaceItem>(*pSty, RES_LR_SPACE);
                nOldLeft = rLR.GetTxtLeft();
            }
            else
            {
                // If based on another
                if( nTabBase < nColls &&
                    (nTabBase = pCollA[nTabBase].nBase) < nColls )
                    pSty = (const SwTxtFmtColl*)pCollA[nTabBase].pFmt;
                else
                    pSty = 0;                           // gib die Suche auf
            }
        }

        if (sw::util::AdjustTabs(nLeft, nOldLeft, aTStop))
            pWWSty->pFmt->SetAttr(aTStop);
    }
}

bool lcl_HasExplicitLeft(const WW8RStyle *pStyles, bool bVer67)
{
    if (pStyles)
    {
        if (bVer67)
            return pStyles->HasParaSprm(17);
        else
        {
            return (
                    pStyles->HasParaSprm(0x840F) ||
                    pStyles->HasParaSprm(0x845E)
                   );
        }
    }
    return false;
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

// Sprm 16, 17
void SwWW8ImplReader::Read_LR( USHORT nId, const BYTE* pData, short nLen )
{
    if (nLen < 0)  // End of the Attributes
    {
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_LR_SPACE);
        return;
    }

    short nPara = SVBT16ToShort( pData );

    SvxLRSpaceItem aLR;
    const SfxPoolItem* pLR = GetFmtAttr(RES_LR_SPACE);
    if( pLR )
        aLR = *(const SvxLRSpaceItem*)pLR;

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

    switch (nId)
    {
        //sprmPDxaLeft
        case     17:
        case 0x840F:
        case 0x845E:
            aLR.SetTxtLeft( nPara );
            if (pAktColl)
                pCollA[nAktColl].bListReleventIndentSet = true;
            break;
        //sprmPDxaLeft1
        case     19:
        case 0x8411:
        case 0x8460:
            /*
            #94672# #99584#
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
            if (pCollA[nAktColl].bHasBrokenWW6List && pPlcxMan)
            {
                const BYTE *pIsZeroed = pPlcxMan->GetPapPLCF()->HasSprm(0x460B);
                if (pIsZeroed && *pIsZeroed == 0)
                {
                    const SvxLRSpaceItem &rLR =
                        ItemGet<SvxLRSpaceItem>(*(pCollA[nAktColl].pFmt),
                        RES_LR_SPACE);
                    nPara -= rLR.GetTxtFirstLineOfst();
                }
            }

            aLR.SetTxtFirstLineOfst(nPara);
            if (pAktColl)
                pCollA[nAktColl].bListReleventIndentSet = true;
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

    NewAttr(aLR);
}

// Sprm 20
void SwWW8ImplReader::Read_LineSpace( USHORT, const BYTE* pData, short nLen )
{
// Kommentear siehe Read_UL()
    if (bStyNormal && bWWBugNormal)
        return;

    if( nLen < 0 ){
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_LINESPACING );
        if( !( nIniFlags & WW8FL_NO_IMPLPASP ) )
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    short nSpace = SVBT16ToShort( pData );
    short nMulti = SVBT16ToShort( pData + 2 );

    SvxLineSpace eLnSpc;
    if( 0 > nSpace )
    {
        nSpace = -nSpace;
        eLnSpc = SVX_LINE_SPACE_FIX;
    }
    else
        eLnSpc = SVX_LINE_SPACE_MIN;

// WW hat einen impliziten zusaetzlichen Absatzabstand abhaengig vom
// Zeilenabstand. Er betraegt bei "genau", 0.8*Zeilenabstand "vor" und
// 0.2*Zeilenabstand "nach".
// Bei "Mindestens" sind es 1*Zeilenabstand "vor" und 0*Zeilenabstand "nach".
// Bei Mehrfach sind es 0 "vor" und min( 0cm, FontSize*(nFach-1) ) "nach".
//
// SW hat auch einen impliziten Zeilenabstand. er betraegt bei "mindestens"
// 1*Zeilenabstand "vor" und 0 "nach"
// bei proportional betraegt er min( 0cm, FontSize*(nFach-1) ) sowohl "vor"
// wie auch "nach"

    USHORT nWwPre = 0;
    USHORT nWwPost = 0;
    USHORT nSwPre = 0;
    USHORT nSwPost = 0;
    USHORT nSpaceTw = 0;

    SvxLineSpacingItem aLSpc;

    if( 1 == nMulti )               // MultilineSpace ( proportional )
    {
        long n = nSpace * 10 / 24;  // WW: 240 = 100%, SW: 100 = 100%

//JP 03.12.98: nach Absprache mit AMA ist die Begrenzung unsinnig
        if( n>200 ) n = 200;        // SW_UI-Maximum
        aLSpc.SetPropLineSpace( (const BYTE)n );
        const SvxFontHeightItem* pH = (const SvxFontHeightItem*)
            GetFmtAttr( RES_CHRATR_FONTSIZE );
        nSpaceTw = (USHORT)( n * pH->GetHeight() / 100 );

        if( n > 100 )
            nWwPost = nSwPre = nSwPost = (USHORT)( ( n - 100 )
                                                    * pH->GetHeight() / 100 );
    }
    else                            // Fixed / Minimum
    {
        // bei negativen Space ist der Abstand exakt, sonst minimum
        nSpaceTw = (USHORT)nSpace;
        aLSpc.SetLineHeight( nSpaceTw );
        aLSpc.GetLineSpaceRule() = eLnSpc;
        nSwPre = nSpace;

        if( SVX_LINE_SPACE_FIX == eLnSpc )                  // Genau
        {
            nWwPre = (USHORT)( 8L * nSpace / 10 );
            nWwPost = (USHORT)( 2L * nSpace / 10 );
            nSwPre = nSpace;
        }
        else                                                // Minimum
        {
            nWwPre = (USHORT)( 129L * nSpace / 100 - 95 );// erst bei groesseren
                                                          // Zeilenabstaenden
        }
    }
    NewAttr( aLSpc );
    if( pSFlyPara )
        pSFlyPara->nLineSpace = nSpaceTw;   // LineSpace fuer Graf-Apos
#if 0
    if( ( nWwPre > nSwPre || nWwPost > nSwPost )
            && !( nIniFlags & WW8FL_NO_IMPLPASP ) )
    {
        SvxULSpaceItem aUL( *(const SvxULSpaceItem*)GetFmtAttr( RES_UL_SPACE ));
        short nDU = aUL.GetUpper() + nWwPre - nSwPre;
        short nDL = aUL.GetLower() + nWwPost - nSwPost;
        if( nDU > 0 )
            aUL.SetUpper( nDU );
        else
            nDL += nDU;

        if( nDL > 0 )
            aUL.SetLower( nDL );
        NewAttr( aUL );
    }
#endif
}

//#i18519# AutoSpace value depends on Dop fDontUseHTMLAutoSpacing setting
sal_uInt16 SwWW8ImplReader::GetParagraphAutoSpace(bool fDontUseHTMLAutoSpacing)
{
    if (fDontUseHTMLAutoSpacing)
        return 100;  //Seems to be always 5points in this case
    else
        return 280;  //Seems to be always 14points in this case
}

void SwWW8ImplReader::Read_ParaAutoBefore(USHORT, const BYTE *pData, short nLen)
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
        if (pAktColl)
            pCollA[nAktColl].bParaAutoBefore = true;
        else
            bParaAutoBefore = true;
    }
    else
    {
        if (pAktColl)
            pCollA[nAktColl].bParaAutoBefore = false;
        else
            bParaAutoBefore = false;
    }
}

void SwWW8ImplReader::Read_ParaAutoAfter(USHORT, const BYTE *pData, short nLen)
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
        if (pAktColl)
            pCollA[nAktColl].bParaAutoAfter = true;
        else
            bParaAutoAfter = true;
    }
    else
    {
        if (pAktColl)
            pCollA[nAktColl].bParaAutoAfter = false;
        else
            bParaAutoAfter = false;
    }
}

// Sprm 21, 22
void SwWW8ImplReader::Read_UL( USHORT nId, const BYTE* pData, short nLen )
{
// Nun eine Umpopelung eines WW-Fehlers: Bei nProduct == 0c03d wird
// faelschlicherweise ein DyaAfter 240 ( delta y abstand after, amn.d.Åb.)
// im Style "Normal" eingefuegt, der
// gar nicht da ist. Ueber das IniFlag WW8FL_NO_STY_DYA laesst sich dieses
// Verhalten auch fuer andere WW-Versionen erzwingen
//  ASSERT( !bStyNormal || bWWBugNormal, "+Dieses Doc deutet evtl. auf einen
// Fehler in der benutzten WW-Version hin. Wenn sich die Styles <Standard> bzw.
// <Normal> zwischen WW und SW im Absatz- oder Zeilenabstand unterscheiden,
// dann bitte dieses Doc SH zukommen lassen." );
// bWWBugNormal ist kein hinreichendes Kriterium dafuer, dass der
// angegebene Abstand falsch ist

    if( nLen < 0 )
    {
        // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_UL_SPACE );
        return;
    }
    short nPara = SVBT16ToShort( pData );
    if( nPara < 0 )
        nPara = -nPara;

    SvxULSpaceItem aUL( *(const SvxULSpaceItem*)GetFmtAttr( RES_UL_SPACE ));

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
    };

    NewAttr( aUL );
}

void SwWW8ImplReader::Read_Justify( USHORT, const BYTE* pData, short nLen )
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
    SvxAdjustItem aAdjust(eAdjust);
    if (bDistributed)
        aAdjust.SetLastBlock(SVX_ADJUST_BLOCK);

    NewAttr(aAdjust);
}

bool SwWW8ImplReader::IsRightToLeft()
{
    bool bRTL = false;
    const BYTE *pDir =
        pPlcxMan ? pPlcxMan->GetPapPLCF()->HasSprm(0x2441) : 0;
    if (pDir)
        bRTL = *pDir ? true : false;
    else
    {
        const SvxFrameDirectionItem* pDir=
            (const SvxFrameDirectionItem*)GetFmtAttr(RES_FRAMEDIR);
        if (pDir && (pDir->GetValue() == FRMDIR_HORI_RIGHT_TOP))
            bRTL = true;
    }
    return bRTL;
}

void SwWW8ImplReader::Read_RTLJustify( USHORT, const BYTE* pData, short nLen )
{
    if( nLen < 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_ADJUST );
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
        SvxAdjustItem aAdjust(eAdjust);
        if (bDistributed)
            aAdjust.SetLastBlock(SVX_ADJUST_BLOCK);

        NewAttr(aAdjust);
    }
}

void SwWW8ImplReader::Read_BoolItem( USHORT nId, const BYTE* pData, short nLen )
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
            ASSERT( !this, "wrong Id" );
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

void SwWW8ImplReader::Read_Emphasis( USHORT, const BYTE* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_EMPHASIS_MARK );
    else
    {
        LanguageType nLang;
        //Check to see if there is an up and coming cjk language property. If
        //there is use it, if there is not fall back to the currently set one.
        //Only the cjk language setting seems to matter to word, the western
        //one is ignored
        const BYTE *pLang =
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
            if ((nLang == LANGUAGE_CHINESE_HONGKONG) ||
                (nLang == LANGUAGE_CHINESE_MACAU) ||
                (nLang == LANGUAGE_CHINESE_TRADITIONAL) ||
                (nLang == LANGUAGE_KOREAN))
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
            if ((nLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
                (nLang == LANGUAGE_CHINESE_SINGAPORE))
                nVal = EMPHASISMARK_DOTS_BELOW;
            else
                nVal = EMPHASISMARK_DOTS_ABOVE;
            break;
        default:
            nVal = EMPHASISMARK_DOTS_ABOVE;
            break;
        }

        NewAttr( SvxEmphasisMarkItem( nVal ) );
    }
}

void SwWW8ImplReader::Read_ScaleWidth( USHORT, const BYTE* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_SCALEW );
    else
    {
        sal_uInt16 nVal = SVBT16ToShort( pData );
        NewAttr( SvxCharScaleWidthItem( nVal ) );
    }
}

void SwWW8ImplReader::Read_Relief( USHORT nId, const BYTE* pData, short nLen )
{
    if( nLen < 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_RELIEF );
    else
    {
        if( *pData )
        {
// JP 16.03.2001 - not so eays because this is also a toggle attribute!
//  2 x emboss on -> no emboss !!!
// the actual value must be searched over the stack / template

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
            NewAttr( SvxCharReliefItem( nNewValue ));
        }
    }
}

void SwWW8ImplReader::Read_TxtAnim(USHORT nId, const BYTE* pData, short nLen)
{
    if (nLen < 0)
        pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_CHRATR_BLINK);
    else
    {
        if (*pData)
        {
            bool bBlink;

            // #110851# The 7 animated text effects available in word all get
            // mapped to a blinking text effect in StarOffice
            // 0 no animation       1 Las Vegas lights
            // 2 background blink   3 sparkle text
            // 4 marching ants      5 marchine red ants
            // 6 shimmer
            if (*pData > 0 && *pData < 7 )
                bBlink = true;
            else
                bBlink = false;

            NewAttr(SvxBlinkItem(bBlink));
        }
    }
}

SwWW8Shade::SwWW8Shade(bool bVer67, const WW8_SHD& rSHD)
{
    BYTE b = rSHD.GetFore();
    ASSERT(b < 17, "ww8: colour out of range");
    if (b >= 17)
        b = 0;

    ColorData nFore(SwWW8ImplReader::GetCol(b));

    b = rSHD.GetBack();
    ASSERT(b < 17, "ww8: colour out of range");
    if( b >=  17 )
        b = 0;

    ColorData nBack(SwWW8ImplReader::GetCol(b));

    b = rSHD.GetStyle(bVer67);

    SetShade(nFore, nBack, b);
}

void SwWW8Shade::SetShade(ColorData nFore, ColorData nBack, sal_uInt16 nIndex)
{
    static const ULONG eMSGrayScale[] =
    {
        // Nul-Brush
           0,   // 0
        // Solid-Brush
        1000,   // 1
        // promillemaessig abgestufte Schattierungen
          50,   // 2
         100,   // 3
         200,   // 4
         250,   // 5
         300,   // 6
         400,   // 7
         500,   // 8
         600,   // 9
         700,   // 10
         750,   // 11
         800,   // 12
         900,   // 13
         333, // 14 Dark Horizontal
         333, // 15 Dark Vertical
         333, // 16 Dark Forward Diagonal
         333, // 17 Dark Backward Diagonal
         333, // 18 Dark Cross
         333, // 19 Dark Diagonal Cross
         333, // 20 Horizontal
         333, // 21 Vertical
         333, // 22 Forward Diagonal
         333, // 23 Backward Diagonal
         333, // 24 Cross
         333, // 25 Diagonal Cross
         // neun Nummern ohne Bedeutung in Ver8
         500, // 26
         500, // 27
         500, // 28
         500, // 29
         500, // 30
         500, // 31
         500, // 32
         500, // 33
         500, // 34
         // und weiter gehts mit tollen Schattierungen ;-)
          25,   // 35
          75,   // 36
         125,   // 37
         150,   // 38
         175,   // 39
         225,   // 40
         275,   // 41
         325,   // 42
         350,   // 43
         375,   // 44
         425,   // 45
         450,   // 46
         475,   // 47
         525,   // 48
         550,   // 49
         575,   // 50
         625,   // 51
         650,   // 52
         675,   // 53
         725,   // 54
         775,   // 55
         825,   // 56
         850,   // 57
         875,   // 58
         925,   // 59
         950,   // 60
         975,   // 61
         // und zu guter Letzt:
         970
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

    ULONG nWW8BrushStyle = eMSGrayScale[nIndex];

    switch (nWW8BrushStyle)
    {
        case 0: // Null-Brush
            aColor.SetColor( nBack );
            break;
        default:
            {
                Color aForeColor(nFore);
                Color aBackColor(nUseBack);
#if 0
                //Transparancy (if thats what it is) doesn't seem to matter
                //in word
                nWW8BrushStyle -=
                    nWW8BrushStyle * aForeColor.GetTransparency() / 0xFF;
#endif

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

void SwWW8ImplReader::Read_Shade( USHORT, const BYTE* pData, short nLen )
{
    if (!bVer67 && pPlcxMan && pPlcxMan->GetPapPLCF()->HasSprm(0xC64D))
        return;

    if (nLen <= 0)
    {
        // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BACKGROUND );
        if (bShdTxtCol)
        {
            // Zeichenfarbe auch
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
            bShdTxtCol = false;
        }
    }
    else
    {
        WW8_SHD aSHD;
        aSHD.SetWWValue( *(SVBT16*)pData );
        SwWW8Shade aSh( bVer67, aSHD );

        NewAttr(SvxBrushItem(aSh.aColor));
    }
}

void SwWW8ImplReader::Read_ParaBackColor(USHORT, const BYTE* pData, short nLen)
{
    if (nLen <= 0)
    {
        // Ende des Attributes
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BACKGROUND );
        if (bShdTxtCol)
        {
            // Zeichenfarbe auch
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_COLOR );
            bShdTxtCol = false;
        }
    }
    else
    {
        ASSERT(nLen == 10, "Len of para back colour not 10!");
        if (nLen != 10)
            return;
        NewAttr(SvxBrushItem(Color(ExtractColour(pData, bVer67))));
    }
}

sal_uInt32 SwWW8ImplReader::ExtractColour(const BYTE* &rpData, bool bVer67)
{
    ASSERT(bVer67 == false, "Impossible");
    //ASSERT(SVBT32ToLong(rpData) == 0xFF000000, "Unknown 1 not 0xff000000");
    sal_uInt32 nFore = wwUtility::BGRToRGB(SVBT32ToLong(rpData));
    rpData+=4;
    sal_uInt32 nBack = wwUtility::BGRToRGB(SVBT32ToLong(rpData));
    rpData+=4;
    sal_uInt16 nIndex = SVBT16ToShort(rpData);
    rpData+=2;
    //Being a transparent background colour doesn't actually show the page
    //background through, it merely acts like white
    if (nBack == 0xFF000000)
        nBack = COL_AUTO;
    ASSERT(nBack == COL_AUTO || !(nBack & 0xFF000000),
        "ww8: don't know what to do with such a transparent bg colour, report");
    SwWW8Shade aShade(nFore, nBack, nIndex);
    return aShade.aColor.GetColor();
}

void SwWW8ImplReader::Read_Border(USHORT , const BYTE* , short nLen)
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
        // die Borders auf allen 4 Seiten werden gebuendelt.  dieses
        // vereinfacht die Verwaltung, d.h. die Box muss nicht 4 mal auf den
        // CtrlStack und wieder runter
        bHasBorder = true;

        WW8_BRC5 aBrcs;   // Top, Left, Bottom, Right, Between
        BYTE nBorder;

        if( pAktColl )
            nBorder = ::lcl_ReadBorders(bVer67, aBrcs, 0, pStyles);
        else
            nBorder = ::lcl_ReadBorders(bVer67, aBrcs, pPlcxMan->GetPapPLCF());

        if( nBorder )                                   // Border
        {
            bool bIsB = IsBorder(aBrcs, true);
            if (!InLocalApo() || !bIsB ||
                (pWFlyPara && !pWFlyPara->bBorderLines ))
            {
                // in Apo keine Umrandungen *ein*-schalten, da ich
                // sonst die Flyumrandungen doppelt bekomme
                // JP 04.12.98: aber nur wenn am Fly ein gesetzt ist, keine
                //              uebernehmen. Sonst wird gar keine gesetzt!
                //              Bug #59619#

                // auch wenn kein Rand gesetzt ist, muss das Attribut gesetzt
                // werden, sonst ist kein hartes Ausschalten von Style-Attrs
                // moeglich
                const SvxBoxItem* pBox
                    = (const SvxBoxItem*)GetFmtAttr( RES_BOX );
                SvxBoxItem aBox;
                if (pBox)
                    aBox = *pBox;
                SetBorder(aBox, aBrcs, 0, nBorder, true);

                Rectangle aInnerDist;
                GetBorderDistance( aBrcs, aInnerDist );

                aBox.SetDistance( (USHORT)aInnerDist.Left(), BOX_LINE_LEFT );
                aBox.SetDistance( (USHORT)aInnerDist.Top(), BOX_LINE_TOP );
                aBox.SetDistance( (USHORT)aInnerDist.Right(), BOX_LINE_RIGHT );
                aBox.SetDistance( (USHORT)aInnerDist.Bottom(), BOX_LINE_BOTTOM );

                NewAttr( aBox );

                SvxShadowItem aS;
                if( SetShadow( aS, aBox, aBrcs ) )
                    NewAttr( aS );
            }
        }
    }
}

void SwWW8ImplReader::Read_Hyphenation( USHORT, const BYTE* pData, short nLen )
{
    // set Hyphenation flag
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_HYPHENZONE );
    else
    {
        SvxHyphenZoneItem aAttr(
            *(const SvxHyphenZoneItem*)GetFmtAttr( RES_PARATR_HYPHENZONE ) );

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

void SwWW8ImplReader::Read_WidowControl( USHORT, const BYTE* pData, short nLen )
{
    if( nLen <= 0 )
    {
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_WIDOWS );
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_ORPHANS );
    }
    else
    {
        BYTE nL = ( *pData & 1 ) ? 2 : 0;

        NewAttr( SvxWidowsItem( nL ) );     // Aus -> nLines = 0
        NewAttr( SvxOrphansItem( nL ) );

        if( pAktColl && pStyles )           // Style-Def ?
            pStyles->bWidowsChanged = true; // merken zur Simulation
                                            // Default-Widows
    }
}

void SwWW8ImplReader::Read_UsePgsuSettings(USHORT,const BYTE* pData,short nLen)
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_SNAPTOGRID);
    else
        NewAttr( SvxParaGridItem(*pData) );
}

void SwWW8ImplReader::Read_AlignFont( USHORT, const BYTE* pData, short nLen )
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
                ASSERT(!this,"Unknown paragraph vertical align");
                break;
        }
        NewAttr( SvxParaVertAlignItem( nVal ) );
    }
}

void SwWW8ImplReader::Read_KeepLines( USHORT, const BYTE* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_PARATR_SPLIT );
    else
        NewAttr( SvxFmtSplitItem( ( *pData & 1 ) == 0 ) );
}

void SwWW8ImplReader::Read_KeepParas( USHORT, const BYTE* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_KEEP );
    else
        NewAttr( SvxFmtKeepItem( ( *pData & 1 ) != 0 ) );
}

void SwWW8ImplReader::Read_BreakBefore( USHORT, const BYTE* pData, short nLen )
{
    if( nLen <= 0 )
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_BREAK );
    else
        NewAttr( SvxFmtBreakItem(
                ( *pData & 1 ) ? SVX_BREAK_PAGE_BEFORE : SVX_BREAK_NONE ) );
}

void SwWW8ImplReader::Read_ApoPPC( USHORT, const BYTE* pData, short )
{
    if (pAktColl) // only for Styledef, sonst anders geloest
    {
        SwWW8StyInf& rSI = pCollA[nAktColl];
        WW8FlyPara* pFly = rSI.pWWFly ? rSI.pWWFly : new WW8FlyPara(bVer67);
        pCollA[nAktColl].pWWFly = pFly;
        pFly->Read(pData, pStyles);
        if (pFly->IsEmpty())
            delete pCollA[nAktColl].pWWFly, pCollA[nAktColl].pWWFly = 0;
    }
}

bool SwWW8ImplReader::ParseTabPos(WW8_TablePos *pTabPos, WW8PLCFx_Cp_FKP* pPap)
{
    bool bRet = false;
    const BYTE *pRes=0;
    memset(pTabPos, 0, sizeof(WW8_TablePos));
    if ((pRes = pPap->HasSprm(0x360D)))
    {
        pTabPos->nSp29 = *pRes;
        pTabPos->nSp37 = 2;     //Possible fail area, always parallel wrap
        if ((pRes = pPap->HasSprm(0x940E)))
            pTabPos->nSp26 = SVBT16ToShort(pRes);
        if ((pRes = pPap->HasSprm(0x940F)))
            pTabPos->nSp27 = SVBT16ToShort(pRes);
        if ((pRes = pPap->HasSprm(0x9410)))
            pTabPos->nLeMgn = SVBT16ToShort(pRes);
        if ((pRes = pPap->HasSprm(0x941E)))
            pTabPos->nRiMgn = SVBT16ToShort(pRes);
        if ((pRes = pPap->HasSprm(0x9411)))
            pTabPos->nUpMgn = SVBT16ToShort(pRes);
        if ((pRes = pPap->HasSprm(0x941F)))
            pTabPos->nLoMgn = SVBT16ToShort(pRes);
        bRet = true;
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
        /* 0 (256) */   &SwWW8ImplReader::Read_Ftn,     // FootNote
        /* 1 (257) */   &SwWW8ImplReader::Read_Ftn,     // EndNote
        /* 2 (258) */   &SwWW8ImplReader::Read_Field,  // Feld
        /* 3 (259) */   &SwWW8ImplReader::Read_Book,   // Bookmark
        /* 4 (260) */   &SwWW8ImplReader::Read_And     // Annotation
    };

    if( pRes->nSprmId < 280 )
    {
        BYTE nIdx = pRes->nSprmId - eFTN;
        if( nIdx < sizeof( aWwSprmTab ) / sizeof( *aWwSprmTab )
            && aWwSprmTab[nIdx] )
            return (this->*aWwSprmTab[nIdx])(pRes);
        else
            return 0;
    }
    else
        return 0;
}

void SwWW8ImplReader::EndExtSprm(USHORT nSprmId)
{
    typedef sal_uInt16 (SwWW8ImplReader:: *FNReadRecordExt)();

    static const FNReadRecordExt aWwSprmTab[] =
    {
        /* 0 (256) */   &SwWW8ImplReader::End_Ftn,      // FootNote
        /* 1 (257) */   &SwWW8ImplReader::End_Ftn,      // EndNote
        /* 2 (258) */   &SwWW8ImplReader::End_Field,  // Feld
        /* 3 (259) */   0,   // Bookmark
        /* 4 (260) */   0     // Annotation
    };

    BYTE nIdx = nSprmId - eFTN;
    if( nIdx < sizeof( aWwSprmTab ) / sizeof( *aWwSprmTab )
        && aWwSprmTab[nIdx] )
        (this->*aWwSprmTab[nIdx])();
}

/***************************************************************************
#       Arrays zum Lesen der SPRMs
#**************************************************************************/

// Funktion zum Einlesen von Sprms. Par1: SprmId
typedef void (SwWW8ImplReader:: *FNReadRecord)( USHORT, const BYTE*, short );

struct SprmReadInfo
{
    USHORT       nId;
    FNReadRecord pReadFnc;
};

SprmReadInfo& WW8GetSprmReadInfo( USHORT nId );


SprmReadInfo aSprmReadTab[] = {
      {0, (FNReadRecord)0},                      // "0" Default bzw. Error
                                                 //wird uebersprungen! ,
      {2, &SwWW8ImplReader::Read_StyleCode},     //"sprmPIstd",  pap.istd
                                                 //(style code)
      {3, (FNReadRecord)0},                      //"sprmPIstdPermute", pap.istd
                                                 //permutation
      {4, (FNReadRecord)0},                      //"sprmPIncLv1",
                                                 //pap.istddifference
      {5, &SwWW8ImplReader::Read_Justify},       //"sprmPJc", pap.jc
                                                 //(justification)
      {6, (FNReadRecord)0},                      //"sprmPFSideBySide",
                                                 //pap.fSideBySide
      {7, &SwWW8ImplReader::Read_KeepLines},     //"sprmPFKeep", pap.fKeep
      {8, &SwWW8ImplReader::Read_KeepParas},     //"sprmPFKeepFollow ",
                                                 //pap.fKeepFollow
      {9, &SwWW8ImplReader::Read_BreakBefore},   //"sprmPPageBreakBefore",
                                                 //pap.fPageBreakBefore
     {10, (FNReadRecord)0},                      //"sprmPBrcl", pap.brcl
     {11, (FNReadRecord)0},                      //"sprmPBrcp ", pap.brcp
     {12, &SwWW8ImplReader::Read_ANLevelDesc},   //"sprmPAnld", pap.anld (ANLD
                                                 //structure)
     {13, &SwWW8ImplReader::Read_ANLevelNo},     //"sprmPNLvlAnm", pap.nLvlAnm
                                                 //nn
     {14, &SwWW8ImplReader::Read_NoLineNumb},    //"sprmPFNoLineNumb", ap.fNoLnn
     {15, &SwWW8ImplReader::Read_Tab},           //"?sprmPChgTabsPapx",
                                                 //pap.itbdMac, ...
     {16, &SwWW8ImplReader::Read_LR},            //"sprmPDxaRight", pap.dxaRight
     {17, &SwWW8ImplReader::Read_LR},            //"sprmPDxaLeft", pap.dxaLeft
     {18, (FNReadRecord)0},                      //"sprmPNest", pap.dxaLeft
     {19, &SwWW8ImplReader::Read_LR},            //"sprmPDxaLeft1", pap.dxaLeft1
     {20, &SwWW8ImplReader::Read_LineSpace},     //"sprmPDyaLine", pap.lspd
                                                 //an LSPD
     {21, &SwWW8ImplReader::Read_UL},            //"sprmPDyaBefore",
                                                 //pap.dyaBefore
     {22, &SwWW8ImplReader::Read_UL},            //"sprmPDyaAfter", pap.dyaAfter
     {23, (FNReadRecord)0},                      //"?sprmPChgTabs", pap.itbdMac,
                                                 //pap.rgdxaTab, ...
     {24, (FNReadRecord)0},                      //"sprmPFInTable", pap.fInTable
     {25, &SwWW8ImplReader::Read_TabRowEnd},     //"sprmPTtp", pap.fTtp
     {26, (FNReadRecord)0},                      //"sprmPDxaAbs", pap.dxaAbs
     {27, (FNReadRecord)0},                      //"sprmPDyaAbs", pap.dyaAbs
     {28, (FNReadRecord)0},                      //"sprmPDxaWidth", pap.dxaWidth
     {29, &SwWW8ImplReader::Read_ApoPPC},        //"sprmPPc", pap.pcHorz,
                                                 //pap.pcVert
     {30, (FNReadRecord)0},                      //"sprmPBrcTop10", pap.brcTop
                                                 //BRC10
     {31, (FNReadRecord)0},                      //"sprmPBrcLeft10",
                                                 //pap.brcLeft BRC10
     {32, (FNReadRecord)0},                      //"sprmPBrcBottom10",
                                                 //pap.brcBottom BRC10
     {33, (FNReadRecord)0},                      //"sprmPBrcRight10",
                                                 //pap.brcRight BRC10
     {34, (FNReadRecord)0},                      //"sprmPBrcBetween10",
                                                 //pap.brcBetween BRC10
     {35, (FNReadRecord)0},                      //"sprmPBrcBar10", pap.brcBar
                                                 //BRC10
     {36, (FNReadRecord)0},                      //"sprmPFromText10",
                                                 //pap.dxaFromText dxa
     {37, (FNReadRecord)0},                      //"sprmPWr", pap.wr wr
     {38, &SwWW8ImplReader::Read_Border},        //"sprmPBrcTop", pap.brcTop BRC
     {39, &SwWW8ImplReader::Read_Border},        //"sprmPBrcLeft",
                                                 //pap.brcLeft BRC
     {40, &SwWW8ImplReader::Read_Border},        //"sprmPBrcBottom",
                                                 //pap.brcBottom BRC
     {41, &SwWW8ImplReader::Read_Border},        //"sprmPBrcRight",
                                                 //pap.brcRight BRC
     {42, &SwWW8ImplReader::Read_Border},        //"sprmPBrcBetween",
                                                 //pap.brcBetween BRC
     {43, (FNReadRecord)0},                      //"sprmPBrcBar", pap.brcBar
                                                 //BRC word
     {44, &SwWW8ImplReader::Read_Hyphenation},   //"sprmPFNoAutoHyph",
                                                 //pap.fNoAutoHyph
     {45, (FNReadRecord)0},                      //"sprmPWHeightAbs",
                                                 //pap.wHeightAbs w
     {46, (FNReadRecord)0},                      //"sprmPDcs", pap.dcs DCS
     {47, &SwWW8ImplReader::Read_Shade},         //"sprmPShd", pap.shd SHD
     {48, (FNReadRecord)0},                      //"sprmPDyaFromText",
                                                 //pap.dyaFromText dya
     {49, (FNReadRecord)0},                      //"sprmPDxaFromText",
                                                 //pap.dxaFromText dxa
     {50, (FNReadRecord)0},                      //"sprmPFLocked", pap.fLocked
                                                 //0 or 1 byte
     {51, &SwWW8ImplReader::Read_WidowControl},  //"sprmPFWidowControl",
                                                 //pap.fWidowControl 0 or 1 byte
     {52, (FNReadRecord)0},                      //"?sprmPRuler 52",
     {53, (FNReadRecord)0},                      //"??53",
     {54, (FNReadRecord)0},                      //"??54",
     {55, (FNReadRecord)0},                      //"??55",
     {56, (FNReadRecord)0},                      //"??56",
     {57, (FNReadRecord)0},                      //"??57",
     {58, (FNReadRecord)0},                      //"??58",
     {59, (FNReadRecord)0},                      //"??59",
     {60, (FNReadRecord)0},                      //"??60",
     {61, (FNReadRecord)0},                      //"??61",
     {62, (FNReadRecord)0},                      //"??62",
     {63, (FNReadRecord)0},                      //"??63",
     {64, &SwWW8ImplReader::Read_ParaBiDi},      //"rtl bidi ?
     {65, &SwWW8ImplReader::Read_CFRMarkDel},    //"sprmCFStrikeRM",
                                                 //chp.fRMarkDel 1 or 0 bit
     {66, &SwWW8ImplReader::Read_CFRMark},       //"sprmCFRMark", chp.fRMark
                                                 //1 or 0 bit
     {67, &SwWW8ImplReader::Read_FldVanish},     //"sprmCFFldVanish",
                                                 //chp.fFldVanish 1 or 0 bit
     {68, &SwWW8ImplReader::Read_PicLoc},        //"sprmCPicLocation",
                                                 //chp.fcPic and chp.fSpec
     {69, (FNReadRecord)0},                      //"sprmCIbstRMark",
                                                 //chp.ibstRMark index into
                                                 //sttbRMark
     {70, (FNReadRecord)0},                      //"sprmCDttmRMark", chp.dttm
                                                 //DTTM long
     {71, (FNReadRecord)0},                      //"sprmCFData", chp.fData 1 or
                                                 //0 bit
     {72, (FNReadRecord)0},                      //"sprmCRMReason",
                                                 //chp.idslRMReason an index to
                                                 //a table
     {73, &SwWW8ImplReader::Read_CharSet},       //"sprmCChse", chp.fChsDiff
                                                 //and chp.chse 3 bytes
     {74, &SwWW8ImplReader::Read_Symbol},        //"sprmCSymbol", chp.fSpec,
                                                 //chp.chSym and chp.ftcSym
     {75, &SwWW8ImplReader::Read_Obj},           //"sprmCFOle2", chp.fOle2 1
                                                 //or 0 bit
     {76, (FNReadRecord)0},                      //"??76",
     {77, (FNReadRecord)0},                      //"??77",
     {78, (FNReadRecord)0},                      //"??78",
     {79, (FNReadRecord)0},                      //"??79",
     {80, &SwWW8ImplReader::Read_CColl},         //"sprmCIstd", chp.istd istd,
                                                 //see stylesheet definition
                                                 //short
     {81, (FNReadRecord)0},                      //"sprmCIstdPermute", chp.istd
                                                 //permutation vector
     {82, (FNReadRecord)0},                      //"sprmCDefault", whole CHP
                                                 //none variable length
     {83, (FNReadRecord)0},                      //"sprmCPlain", whole CHP
                                                 //none 0
     {84, (FNReadRecord)0},                      //"??84",
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
     {92, &SwWW8ImplReader::Read_Invisible},     //"sprmCFVanish", chp.fVanish
                                                 //0,1, 128, or 129 byte
     {93, &SwWW8ImplReader::Read_FontCode},      //"sprmCFtc", chp.ftc ftc word
     {94, &SwWW8ImplReader::Read_Underline},     // "sprmCKul", chp.kul kul byte
     {95, (FNReadRecord)0},                      //"sprmCSizePos", chp.hps,
                                                 //chp.hpsPos 3 bytes
     {96, &SwWW8ImplReader::Read_Kern},        //"sprmCDxaSpace",
                                                 //chp.dxaSpace dxa word
     {97, &SwWW8ImplReader::Read_Language},      //"sprmCLid", chp.lid LID word
     {98, &SwWW8ImplReader::Read_TxtColor},      //"sprmCIco", chp.ico ico byte
     {99, &SwWW8ImplReader::Read_FontSize},      //"sprmCHps", chp.hps hps word!
    {100, (FNReadRecord)0},                      //"sprmCHpsInc", chp.hps byte
    {101, &SwWW8ImplReader::Read_SubSuperProp},  //"sprmCHpsPos", chp.hpsPos
                                                 //hps byte
    {102, (FNReadRecord)0},                      //"sprmCHpsPosAdj", chp.hpsPos
                                                 //hps byte
    {103, &SwWW8ImplReader::Read_Majority},      //"?sprmCMajority", chp.fBold,
                                                 //chp.fItalic, chp.fSmallCaps
    {104, &SwWW8ImplReader::Read_SubSuper},      //"sprmCIss", chp.iss iss byte
    {105, (FNReadRecord)0},                      //"sprmCHpsNew50", chp.hps hps
                                                 //variable width, length
                                                 //always recorded as 2
    {106, (FNReadRecord)0},                      //"sprmCHpsInc1", chp.hps
                                                 //complex variable width,
                                                 //length always recorded as 2
    {107, &SwWW8ImplReader::Read_FontKern},      //"sprmCHpsKern", chp.hpsKern
                                                 //hps short
    {108, &SwWW8ImplReader::Read_Majority},      //"sprmCMajority50", chp.fBold,
                                                 //chp.fItalic, chp.fSmallCaps,
                                                 // chp.fVanish, ...
    {109, (FNReadRecord)0},                      //"sprmCHpsMul", chp.hps
                                                 //percentage to grow hps short
    {110, (FNReadRecord)0},                      //"sprmCCondHyhen", chp.ysri
                                                 //ysri short
    {111, &SwWW8ImplReader::Read_BoldBiDiUsw},   //"111 bidi bold ?",
    {112, &SwWW8ImplReader::Read_BoldBiDiUsw},   //"112 bidi italic ?",
    {113, &SwWW8ImplReader::Read_FontCode},      //ww7 rtl font
    {114, (FNReadRecord)0},                      //"??114",
    {115, &SwWW8ImplReader::Read_TxtColor},      //ww7 rtl colour ?
    {116, &SwWW8ImplReader::Read_FontSize},
    {117, &SwWW8ImplReader::Read_Special},       //"sprmCFSpec", chp.fSpec 1
                                                 //or 0 bit
    {118, &SwWW8ImplReader::Read_Obj},           //"sprmCFObj", chp.fObj 1 or 0
                                                 //bit
    {119, (FNReadRecord)0},                      //"sprmPicBrcl", pic.brcl brcl
                                                 //(see PIC structure
                                                 //definition) byte
    {120, (FNReadRecord)0},                      //"sprmPicScale", pic.mx,
                                                 //pic.my, pic.dxaCropleft,
    {121, (FNReadRecord)0},                      //"sprmPicBrcTop", pic.brcTop
                                                 //BRC word
    {122, (FNReadRecord)0},                      //"sprmPicBrcLeft",
                                                 //pic.brcLeft BRC word
    {123, (FNReadRecord)0},                      //"sprmPicBrcBottom",
                                                 //pic.brcBottom BRC word
    {124, (FNReadRecord)0},                      //"sprmPicBrcRight",
                                                 //pic.brcRight BRC word
    {125, (FNReadRecord)0},                      //"??125",
    {126, (FNReadRecord)0},                      //"??126",
    {127, (FNReadRecord)0},                      //"??127",
    {128, (FNReadRecord)0},                      //"??128",
    {129, (FNReadRecord)0},                      //"??129",
    {130, (FNReadRecord)0},                      //"??130",
    {131, (FNReadRecord)0},                      //"sprmSScnsPgn", sep.cnsPgn
                                                 //cns byte
    {132, (FNReadRecord)0},                      //"sprmSiHeadingPgn",
                                                 //sep.iHeadingPgn heading
                                                 //number level byte
    {133, &SwWW8ImplReader::Read_OLST},          //"sprmSOlstAnm", sep.olstAnm
                                                 //OLST variable length
    {134, (FNReadRecord)0},                      //"??135",
    {135, (FNReadRecord)0},                      //"??135",
    {136, (FNReadRecord)0},                      //"sprmSDxaColWidth",
                                                 //sep.rgdxaColWidthSpacing
                                                 //complex 3 bytes
    {137, (FNReadRecord)0},                      //"sprmSDxaColSpacing",
                                                 //sep.rgdxaColWidthSpacing
                                                 //complex 3 bytes
    {138, (FNReadRecord)0},                      //"sprmSFEvenlySpaced",
                                                 //sep.fEvenlySpaced 1 or 0 byte
    {139, (FNReadRecord)0},                      //"sprmSFProtected",
                                                 //sep.fUnlocked 1 or 0 byte
    {140, (FNReadRecord)0},                      //"sprmSDmBinFirst",
                                                 //sep.dmBinFirst  word
    {141, (FNReadRecord)0},                      //"sprmSDmBinOther",
                                                 //sep.dmBinOther  word
    {142, (FNReadRecord)0},                      //"sprmSBkc", sep.bkc bkc
                                                 //byte BreakCode
    {143, (FNReadRecord)0},                      //"sprmSFTitlePage",
                                                 //sep.fTitlePage 0 or 1 byte
    {144, (FNReadRecord)0},                      //"sprmSCcolumns", sep.ccolM1
                                                 //# of cols - 1 word
    {145, (FNReadRecord)0},                      //"sprmSDxaColumns",
                                                 //sep.dxaColumns dxa word
    {146, (FNReadRecord)0},                      //"sprmSFAutoPgn",
                                                 //sep.fAutoPgn obsolete byte
    {147, (FNReadRecord)0},                      //"sprmSNfcPgn", sep.nfcPgn
                                                 //nfc byte
    {148, (FNReadRecord)0},                      //"sprmSDyaPgn", sep.dyaPgn
                                                 //dya short
    {149, (FNReadRecord)0},                      //"sprmSDxaPgn", sep.dxaPgn
                                                 //dya short
    {150, (FNReadRecord)0},                      //"sprmSFPgnRestart",
                                                 //sep.fPgnRestart 0 or 1 byte
    {151, (FNReadRecord)0},                      //"sprmSFEndnote", sep.fEndnote
                                                 //0 or 1 byte
    {152, (FNReadRecord)0},                      //"sprmSLnc", sep.lnc lnc byte
    {153, (FNReadRecord)0},                      //"sprmSGprfIhdt", sep.grpfIhdt
                                                 //grpfihdt byte
    {154, (FNReadRecord)0},                      //"sprmSNLnnMod", sep.nLnnMod
                                                 //non-neg int. word
    {155, (FNReadRecord)0},                      //"sprmSDxaLnn", sep.dxaLnn
                                                 //dxa word
    {156, (FNReadRecord)0},                      //"sprmSDyaHdrTop",
                                                 //sep.dyaHdrTop dya word
    {157, (FNReadRecord)0},                      //"sprmSDyaHdrBottom",
                                                 //sep.dyaHdrBottom dya word
    {158, (FNReadRecord)0},                      //"sprmSLBetween",
                                                 //sep.fLBetween 0 or 1 byte
    {159, (FNReadRecord)0},                      //"sprmSVjc", sep.vjc vjc byte
    {160, (FNReadRecord)0},                      //"sprmSLnnMin", sep.lnnMin
                                                 //lnn word
    {161, (FNReadRecord)0},                      //"sprmSPgnStart", sep.pgnStart
                                                 //pgn word
    {162, (FNReadRecord)0},                      //"sprmSBOrientation",
                                                 //sep.dmOrientPage dm byte
    {163, (FNReadRecord)0},                      //"?SprmSBCustomize 163", ?
    {164, (FNReadRecord)0},                      //"sprmSXaPage", sep.xaPage xa
                                                 //word
    {165, (FNReadRecord)0},                      //"sprmSYaPage", sep.yaPage ya
                                                 //word
    {166, (FNReadRecord)0},                      //"sprmSDxaLeft", sep.dxaLeft
                                                 //dxa word
    {167, (FNReadRecord)0},                      //"sprmSDxaRight", sep.dxaRight
                                                 //dxa word
    {168, (FNReadRecord)0},                      //"sprmSDyaTop", sep.dyaTop                                                     //dya word
    {169, (FNReadRecord)0},                      //"sprmSDyaBottom",
                                                 //sep.dyaBottom dya word
    {170, (FNReadRecord)0},                      //"sprmSDzaGutter",
                                                 //sep.dzaGutter dza word
    {171, (FNReadRecord)0},                      //"sprmSDMPaperReq",
                                                 //sep.dmPaperReq dm word
    {172, (FNReadRecord)0},                      //"??172",
    {173, (FNReadRecord)0},                      //"??173",
    {174, (FNReadRecord)0},                      //"??174",
    {175, (FNReadRecord)0},                      //"??175",
    {176, (FNReadRecord)0},                      //"??176",
    {177, (FNReadRecord)0},                      //"??177",
    {178, (FNReadRecord)0},                      //"??178",
    {179, (FNReadRecord)0},                      //"??179",
    {180, (FNReadRecord)0},                      //"??180",
    {181, (FNReadRecord)0},                      //"??181",
    {182, (FNReadRecord)0},                      //"sprmTJc", tap.jc jc word
                                                 //(low order byte is
                                                 //significant)
    {183, (FNReadRecord)0},                      //"sprmTDxaLeft",
                                                 //tap.rgdxaCenter dxa word
    {184, (FNReadRecord)0},                      //"sprmTDxaGapHalf",
                                                 //tap.dxaGapHalf,
                                                 //tap.rgdxaCenter dxa word
    {185, (FNReadRecord)0},                      //"sprmTFCantSplit"
                                                 //tap.fCantSplit 1 or 0 byte
    {186, (FNReadRecord)0},                      //"sprmTTableHeader",
                                                 //tap.fTableHeader 1 or 0 byte
    {187, (FNReadRecord)0},                      //"sprmTTableBorders",
                                                 //tap.rgbrcTable complex
                                                 //12 bytes
    {188, (FNReadRecord)0},                      //"sprmTDefTable10",
                                                 //tap.rgdxaCenter, tap.rgtc
                                                 //complex variable length
    {189, (FNReadRecord)0},                      //"sprmTDyaRowHeight",
                                                 //tap.dyaRowHeight dya word
    {190, (FNReadRecord)0},                      //"?sprmTDefTable", tap.rgtc
                                                 //complex
    {191, (FNReadRecord)0},                      //"?sprmTDefTableShd",
                                                 //tap.rgshd complex
    {192, (FNReadRecord)0},                      //"sprmTTlp", tap.tlp TLP
                                                 //4 bytes
    {193, (FNReadRecord)0},                      //"sprmTSetBrc",
                                                 //tap.rgtc[].rgbrc complex
                                                 //5 bytes
    {194, (FNReadRecord)0},                      //"sprmTInsert",
                                                 //tap.rgdxaCenter,
                                                 //tap.rgtc complex 4 bytes
    {195, (FNReadRecord)0},                      //"sprmTDelete",
                                                 //tap.rgdxaCenter,
                                                 //tap.rgtc complex word
    {196, (FNReadRecord)0},                      //"sprmTDxaCol",
                                                 //tap.rgdxaCenter complex
                                                 //4 bytes
    {197, (FNReadRecord)0},                      //"sprmTMerge",
                                                 //tap.fFirstMerged,
                                                 //tap.fMerged complex word
    {198, (FNReadRecord)0},                      //"sprmTSplit",
                                                 //tap.fFirstMerged,
                                                 //tap.fMerged complex word
    {199, (FNReadRecord)0},                      //"sprmTSetBrc10",
                                                 //tap.rgtc[].rgbrc complex
                                                 //5 bytes
    {200, (FNReadRecord)0},                      //"sprmTSetShd", tap.rgshd
                                                 //complex 4 bytes
    {207, (FNReadRecord)0},                      //dunno

//- new with Ver8 ------------------------------------------------------------

    {0x4600, &SwWW8ImplReader::Read_StyleCode},  //"sprmPIstd" pap.istd;istd
                                                 //(style code);short;
    {0xC601, (FNReadRecord)0},                   //"sprmPIstdPermute" pap.istd;
                                                 //permutation vector;
                                                 //variable length;
    {0x2602, (FNReadRecord)0},                   //"sprmPIncLvl" pap.istd,
                                                 //pap.lvl;difference between
                                                 //istd of base PAP and istd of
                                                 //PAP to be produced; byte;
    {0x2403, &SwWW8ImplReader::Read_Justify},    //"sprmPJc" pap.jc;jc
                                                 //(justification);byte;
    {0x2404, (FNReadRecord)0},                   //"sprmPFSideBySide"
                                                 //pap.fSideBySide;0 or 1;byte;
    {0x2405, &SwWW8ImplReader::Read_KeepLines},  //"sprmPFKeep" pap.fKeep;0 or
                                                 //1;byte;
    {0x2406, &SwWW8ImplReader::Read_KeepParas},  //"sprmPFKeepFollow"
                                                 //pap.fKeepFollow;0 or 1;byte;
    {0x2407, &SwWW8ImplReader::Read_BreakBefore},//"sprmPFPageBreakBefore"
                                                 //pap.fPageBreakBefore;0 or 1;
                                                 //byte;
    {0x2408, (FNReadRecord)0},                   //"sprmPBrcl" pap.brcl;brcl;
                                                 //byte;
    {0x2409, (FNReadRecord)0},                   //"sprmPBrcp" pap.brcp;brcp;
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
    {0x840E, &SwWW8ImplReader::Read_LR},         //"sprmPDxaRight" pap.dxaRight;
                                                 //dxa;word;
    {0x840F, &SwWW8ImplReader::Read_LR},         //"sprmPDxaLeft" pap.dxaLeft;
                                                 //dxa;word;
    {0x4610, (FNReadRecord)0},                   //"sprmPNest" pap.dxaLeft;
                                                 //dxa;word;
    {0x8411, &SwWW8ImplReader::Read_LR},         //"sprmPDxaLeft1" pap.dxaLeft1;
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
    {0xC615, (FNReadRecord)0},                   //"sprmPChgTabs" pap.itbdMac,
                                                 //pap.rgdxaTab, pap.rgtbd;
                                                 //complex;variable length;
    {0x2416, (FNReadRecord)0},                   //"sprmPFInTable" pap.fInTable;
                                                 //0 or 1;byte;
    {0x2417, &SwWW8ImplReader::Read_TabRowEnd},  //"sprmPFTtp" pap.fTtp;0 or 1;
                                                 //byte;
    {0x8418, (FNReadRecord)0},                   //"sprmPDxaAbs" pap.dxaAbs;dxa;
                                                 //word;
    {0x8419, (FNReadRecord)0},                   //"sprmPDyaAbs" pap.dyaAbs;dya;
                                                 //word;
    {0x841A, (FNReadRecord)0},                   //"sprmPDxaWidth" pap.dxaWidth;
                                                 //dxa;word;
    {0x261B, &SwWW8ImplReader::Read_ApoPPC},     //"sprmPPc" pap.pcHorz,
                                                 //pap.pcVert;complex;byte;
    {0x461C, (FNReadRecord)0},                   //"sprmPBrcTop10" pap.brcTop;
                                                 //BRC10;word;
    {0x461D, (FNReadRecord)0},                   //"sprmPBrcLeft10" pap.brcLeft;
                                                 //BRC10;word;
    {0x461E, (FNReadRecord)0},                   //"sprmPBrcBottom10"
                                                 //pap.brcBottom;BRC10;word;
    {0x461F, (FNReadRecord)0},                   //"sprmPBrcRight10"
                                                 //pap.brcRight;BRC10;word;
    {0x4620, (FNReadRecord)0},                   //"sprmPBrcBetween10"
                                                 //pap.brcBetween;BRC10;word;
    {0x4621, (FNReadRecord)0},                   //"sprmPBrcBar10" pap.brcBar;
                                                 //BRC10;word;
    {0x4622, (FNReadRecord)0},                   //"sprmPDxaFromText10"
                                                 //pap.dxaFromText;dxa;word;
    {0x2423, (FNReadRecord)0},                   //"sprmPWr" pap.wr;wr;byte;
    {0x6424, &SwWW8ImplReader::Read_Border},     //"sprmPBrcTop" pap.brcTop;BRC;
                                                 //long;
    {0x6425, &SwWW8ImplReader::Read_Border},     //"sprmPBrcLeft" pap.brcLeft;
                                                 //BRC;long;
    {0x6426, &SwWW8ImplReader::Read_Border},     //"sprmPBrcBottom"
                                                 //pap.brcBottom;BRC;long;
    {0x6427, &SwWW8ImplReader::Read_Border},     //"sprmPBrcRight" pap.brcRight;
                                                 //BRC;long;
    {0x6428, &SwWW8ImplReader::Read_Border},     //"sprmPBrcBetween"
                                                 //pap.brcBetween;BRC;long;
    {0x6629, (FNReadRecord)0},                   //"sprmPBrcBar" pap.brcBar;BRC;
                                                 //long;
    {0x242A, &SwWW8ImplReader::Read_Hyphenation},//"sprmPFNoAutoHyph"
                                                 //pap.fNoAutoHyph;0 or 1;byte;
    {0x442B, (FNReadRecord)0},                   //"sprmPWHeightAbs"
                                                 //pap.wHeightAbs;w;word;
    {0x442C, (FNReadRecord)0},                   //"sprmPDcs" pap.dcs;DCS;short;
    {0x442D, &SwWW8ImplReader::Read_Shade},      //"sprmPShd" pap.shd;SHD;word;
    {0x842E, (FNReadRecord)0},                   //"sprmPDyaFromText"
                                                 //pap.dyaFromText;dya;word;
    {0x842F, (FNReadRecord)0},                   //"sprmPDxaFromText"
                                                 //pap.dxaFromText;dxa;word;
    {0x2430, (FNReadRecord)0},                   //"sprmPFLocked" pap.fLocked;
                                                 //0 or 1;byte;
    {0x2431, &SwWW8ImplReader::Read_WidowControl},//"sprmPFWidowControl"
                                                 //pap.fWidowControl;0 or 1;byte
    {0xC632, (FNReadRecord)0},                   //"sprmPRuler" variable length;
    {0x2433, &SwWW8ImplReader::Read_BoolItem},   //"sprmPFKinsoku" pap.fKinsoku;
                                                 //0 or 1;byte;
    {0x2434, (FNReadRecord)0},                   //"sprmPFWordWrap"
                                                 //pap.fWordWrap;0 or 1;byte;
    {0x2435, &SwWW8ImplReader::Read_BoolItem},   //"sprmPFOverflowPunct"
                                                 //pap.fOverflowPunct; 0 or 1;
                                                 //byte;
    {0x2436, (FNReadRecord)0},                   //"sprmPFTopLinePunct"
                                                 //pap.fTopLinePunct;0 or 1;byte
    {0x2437, &SwWW8ImplReader::Read_BoolItem},   //"sprmPFAutoSpaceDE"
                                                 //pap.fAutoSpaceDE;0 or 1;byte;
    {0x2438, (FNReadRecord)0},                   //"sprmPFAutoSpaceDN"
                                                 //pap.fAutoSpaceDN;0 or 1;byte;
    {0x4439, &SwWW8ImplReader::Read_AlignFont},  //"sprmPWAlignFont"
                                                 //pap.wAlignFont;iFa; word;
    {0x443A, (FNReadRecord)0},                   //"sprmPFrameTextFlow"
                                                 //pap.fVertical pap.fBackward
                                                 //pap.fRotateFont;complex; word
    {0x243B, (FNReadRecord)0},                   //"sprmPISnapBaseLine" obsolete
                                                 //not applicable in Word97
                                                 //and later versions;;byte;
    {0xC63E, &SwWW8ImplReader::Read_ANLevelDesc},//"sprmPAnld" pap.anld;;
                                                 //variable length;
    {0xC63F, (FNReadRecord)0},                   //"sprmPPropRMark"
                                                 //pap.fPropRMark;complex;
                                                 //variable length;
    {0x2640,  &SwWW8ImplReader::Read_POutLvl},   //"sprmPOutLvl" pap.lvl;has no
                                                 //effect if pap.istd is < 1 or
                                                 //is > 9;byte;
    {0x2441, &SwWW8ImplReader::Read_ParaBiDi},   //"sprmPFBiDi" ;;byte;
    {0x2443, (FNReadRecord)0},                   //"sprmPFNumRMIns"
                                                 //pap.fNumRMIns;1 or 0;bit;
    {0x2444, (FNReadRecord)0},                   //"sprmPCrLf" ;;byte;
    {0xC645, (FNReadRecord)0},                   //"sprmPNumRM" pap.numrm;;
                                                 //variable length;
    {0x6645, (FNReadRecord)0},                   //"sprmPHugePapx" ;fc in the
                                                 //data stream to locate the
                                                 //huge grpprl; long;
    {0x6646, (FNReadRecord)0},                   //"sprmPHugePapx" ;fc in the
                                                 //data stream to locate the
                                                 //huge grpprl; long;
    {0x2447, &SwWW8ImplReader::Read_UsePgsuSettings},//"sprmPFUsePgsuSettings"
                                                 //pap.fUsePgsuSettings;1 or 0;
                                                 //byte;
    {0x2448, (FNReadRecord)0},                   //"sprmPFAdjustRight"
                                                 //pap.fAdjustRight;1 or 0;byte;
    {0x0800, &SwWW8ImplReader::Read_CFRMarkDel}, //"sprmCFRMarkDel"
                                                 //chp.fRMarkDel;1 or 0;bit;
    {0x0801, &SwWW8ImplReader::Read_CFRMark},    //"sprmCFRMark" chp.fRMark;1
                                                 //or 0;bit;
    {0x0802, &SwWW8ImplReader::Read_FldVanish},  //"sprmCFFldVanish"
                                                 //chp.fFldVanish;1 or 0;bit;
    {0x6A03, &SwWW8ImplReader::Read_PicLoc},     //"sprmCPicLocation" chp.fcPic
                                                 //and chp.fSpec;variable
                                                 //length, length recorded is
                                                 //always 4;
    {0x4804, (FNReadRecord)0},                   //"sprmCIbstRMark"
                                                 //chp.ibstRMark;index into
                                                 //sttbRMark;short;
    {0x6805, (FNReadRecord)0},                   //"sprmCDttmRMark"
                                                 //chp.dttmRMark;DTTM;long;
    {0x0806, (FNReadRecord)0},                   //"sprmCFData" chp.fData;1 or
                                                 //0;bit;
    {0x4807, (FNReadRecord)0},                   //"sprmCIdslRMark"
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
    {0xCA31, (FNReadRecord)0},                   //"sprmCIstdPermute" chp.istd;
                                                 //permutation vector; variable
                                                 //length;
    {0x2A32, (FNReadRecord)0},                   //"sprmCDefault" whole CHP;none
                                                 //;variable length;
    {0x2A33, (FNReadRecord)0},                   //"sprmCPlain" whole CHP;none;
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
    {0x083C, &SwWW8ImplReader::Read_Invisible},  //"sprmCFVanish" chp.fVanish;0,
                                                 //1, 128, or 129; byte;
  //0x4A3D, (FNReadRecord)0,                     //"sprmCFtcDefault" ftc, only
                                                 //used internally, never
                                                 //stored in file;word;
    {0x2A3E, &SwWW8ImplReader::Read_Underline},  //"sprmCKul" chp.kul;kul;byte;
    {0xEA3F, (FNReadRecord)0},                   //"sprmCSizePos" chp.hps,
                                                 //chp.hpsPos;3 bytes;
    {0x8840, &SwWW8ImplReader::Read_Kern},       //"sprmCDxaSpace" chp.dxaSpace;
                                                 //dxa;word;
    {0x4A41, &SwWW8ImplReader::Read_Language},   //"sprmCLid" ;only used
                                                 //internally never stored;word;
    {0x2A42, &SwWW8ImplReader::Read_TxtColor},   //"sprmCIco" chp.ico;ico;byte;
    {0x4A43, &SwWW8ImplReader::Read_FontSize},   //"sprmCHps" chp.hps;hps;byte;
    {0x2A44, (FNReadRecord)0},                   //"sprmCHpsInc" chp.hps;byte;
    {0x4845, &SwWW8ImplReader::Read_SubSuperProp},//"sprmCHpsPos" chp.hpsPos;
                                                 //hps; byte;
    {0x2A46, (FNReadRecord)0},                   //"sprmCHpsPosAdj" chp.hpsPos;
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
    {0xCA49, (FNReadRecord)0},                   //"sprmCHpsNew50" chp.hps;hps;
                                                 //variable width, length
                                                 //always recorded as 2;
    {0xCA4A, (FNReadRecord)0},                   //"sprmCHpsInc1" chp.hps;
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
    {0x4A4D, (FNReadRecord)0},                   //"sprmCHpsMul" chp.hps;
                                                 //percentage to grow hps;short;
    {0x484E, (FNReadRecord)0},                   //"sprmCYsri" chp.ysri;ysri;
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
    {0x2859, &SwWW8ImplReader::Read_TxtAnim},    //"sprmCSfxText" chp.sfxtText;
                                                 //text animation;byte;
    {0x085A, (FNReadRecord)0},                   //"sprmCFBiDi"
    {0x085B, (FNReadRecord)0},                   //"sprmCFDiacColor"
    {0x085C, &SwWW8ImplReader::Read_BoldBiDiUsw},//"sprmCFBoldBi"
    {0x085D, &SwWW8ImplReader::Read_BoldBiDiUsw},//"sprmCFItalicBi"
    {0x4A5E, &SwWW8ImplReader::Read_FontCode},
    {0x485F, &SwWW8ImplReader::Read_Language},   // "sprmCLidBi"
  //0x4A60, ? ? ?,                               //"sprmCIcoBi",
    {0x4A61, &SwWW8ImplReader::Read_FontSize},   //"sprmCHpsBi"
    {0xCA62, (FNReadRecord)0},                   //"sprmCDispFldRMark"
                                                 //chp.fDispFldRMark,
                                                 //chp.ibstDispFldRMark,
                                                 //chp.dttmDispFldRMark ;
                                                 //Complex;variable length
                                                 //always recorded as 39 bytes;
    {0x4863, (FNReadRecord)0},                   //"sprmCIbstRMarkDel"
                                                 //chp.ibstRMarkDel;index into
                                                 //sttbRMark;short;
    {0x6864, (FNReadRecord)0},                   //"sprmCDttmRMarkDel"
                                                 //chp.dttmRMarkDel;DTTM;long;
    {0x6865, (FNReadRecord)0},                   //"sprmCBrc" chp.brc;BRC;long;
    {0x4866, &SwWW8ImplReader::Read_CharShadow}, //"sprmCShd" chp.shd;SHD;short;
    {0x4867, (FNReadRecord)0},                   //"sprmCIdslRMarkDel"
                                                 //chp.idslRMReasonDel;an index
                                                 //to a table of strings
                                                 //defined in Word 6.0
                                                 //executables;short;
    {0x0868, (FNReadRecord)0},                   //"sprmCFUsePgsuSettings"
                                                 //chp.fUsePgsuSettings; 1 or 0;
                                                 //bit;
    {0x486B, (FNReadRecord)0},                   //"sprmCCpg" ;;word;
    {0x486D, &SwWW8ImplReader::Read_Language},   //"sprmCRgLid0" chp.rglid[0];
                                                 //LID: for non-Far East text;
                                                 //word;
    {0x486E, &SwWW8ImplReader::Read_Language},   //"sprmCRgLid1" chp.rglid[1];
                                                 //LID: for Far East text;word;
    {0x286F, (FNReadRecord)0},                   //"sprmCIdctHint" chp.idctHint;
                                                 //IDCT: byte;
    {0x2E00, (FNReadRecord)0},                   //"sprmPicBrcl" pic.brcl;brcl
                                                 //(see PIC structure
                                                 //definition);byte;
    {0xCE01, (FNReadRecord)0},                   //"sprmPicScale" pic.mx,
                                                 //pic.my, pic.dxaCropleft,
                                                 //pic.dyaCropTop
                                                 //pic.dxaCropRight,
                                                 //pic.dyaCropBottom;Complex;
                                                 //length byte plus 12 bytes;
    {0x6C02, (FNReadRecord)0},                   //"sprmPicBrcTop" pic.brcTop;
                                                 //BRC;long;
    {0x6C03, (FNReadRecord)0},                   //"sprmPicBrcLeft" pic.brcLeft;
                                                 //BRC;long;
    {0x6C04, (FNReadRecord)0},                   //"sprmPicBrcBottom"
                                                 //pic.brcBottom;BRC;long;
    {0x6C05, (FNReadRecord)0},                   //"sprmPicBrcRight"
                                                 //pic.brcRight;BRC;long;
    {0x3000, (FNReadRecord)0},                   //"sprmScnsPgn" sep.cnsPgn;cns;
                                                 //byte;
    {0x3001, (FNReadRecord)0},                   //"sprmSiHeadingPgn"
                                                 //sep.iHeadingPgn;heading
                                                 //number level;byte;
    {0xD202, &SwWW8ImplReader::Read_OLST},       //"sprmSOlstAnm" sep.olstAnm;
                                                 //OLST;variable length;
    {0xF203, (FNReadRecord)0},                   //"sprmSDxaColWidth"
                                                 //sep.rgdxaColWidthSpacing;
                                                 //complex; 3 bytes;
    {0xF204, (FNReadRecord)0},                   //"sprmSDxaColSpacing"
                                                 //sep.rgdxaColWidthSpacing;
                                                 //complex; 3 bytes;
    {0x3005, (FNReadRecord)0},                   //"sprmSFEvenlySpaced"
                                                 //sep.fEvenlySpaced; 1 or 0;
                                                 //byte;
    {0x3006, (FNReadRecord)0},                   //"sprmSFProtected"
                                                 //sep.fUnlocked;1 or 0;byte;
    {0x5007, (FNReadRecord)0},                   //"sprmSDmBinFirst"
                                                 //sep.dmBinFirst;;word;
    {0x5008, (FNReadRecord)0},                   //"sprmSDmBinOther"
                                                 //sep.dmBinOther;;word;
    {0x3009, (FNReadRecord)0},                   //"sprmSBkc" sep.bkc;bkc;byte;
    {0x300A, (FNReadRecord)0},                   //"sprmSFTitlePage"
                                                 //sep.fTitlePage;0 or 1;byte;
    {0x500B, (FNReadRecord)0},                   //"sprmSCcolumns" sep.ccolM1;
                                                 //# of cols - 1;word;
    {0x900C, (FNReadRecord)0},                   //"sprmSDxaColumns"
                                                 //sep.dxaColumns;dxa;word;
    {0x300D, (FNReadRecord)0},                   //"sprmSFAutoPgn" sep.fAutoPgn;
                                                 //obsolete;byte;
    {0x300E, (FNReadRecord)0},                   //"sprmSNfcPgn" sep.nfcPgn;nfc;
                                                 //byte;
    {0xB00F, (FNReadRecord)0},                   //"sprmSDyaPgn" sep.dyaPgn;dya;
                                                 //short;
    {0xB010, (FNReadRecord)0},                   //"sprmSDxaPgn" sep.dxaPgn;dya;
                                                 //short;
    {0x3011, (FNReadRecord)0},                   //"sprmSFPgnRestart"
                                                 //sep.fPgnRestart;0 or 1;byte;
    {0x3012, (FNReadRecord)0},                   //"sprmSFEndnote" sep.fEndnote;
                                                 //0 or 1;byte;
    {0x3013, (FNReadRecord)0},                   //"sprmSLnc" sep.lnc;lnc;byte;
    {0x3014, (FNReadRecord)0},                   //"sprmSGprfIhdt" sep.grpfIhdt;
                                                 //grpfihdt; byte;
    {0x5015, (FNReadRecord)0},                   //"sprmSNLnnMod" sep.nLnnMod;
                                                 //non-neg int.;word;
    {0x9016, (FNReadRecord)0},                   //"sprmSDxaLnn" sep.dxaLnn;dxa;
                                                 //word;
    {0xB017, (FNReadRecord)0},                   //"sprmSDyaHdrTop"
                                                 //sep.dyaHdrTop;dya;word;
    {0xB018, (FNReadRecord)0},                   //"sprmSDyaHdrBottom"
                                                 //sep.dyaHdrBottom;dya;word;
    {0x3019, (FNReadRecord)0},                   //"sprmSLBetween"
                                                 //sep.fLBetween;0 or 1;byte;
    {0x301A, (FNReadRecord)0},                   //"sprmSVjc" sep.vjc;vjc;byte;
    {0x501B, (FNReadRecord)0},                   //"sprmSLnnMin" sep.lnnMin;lnn;
                                                 //word;
    {0x501C, (FNReadRecord)0},                   //"sprmSPgnStart" sep.pgnStart;
                                                 //pgn;word;
    {0x301D, (FNReadRecord)0},                   //"sprmSBOrientation"
                                                 //sep.dmOrientPage;dm;byte;
  //0x301E, ? ? ?,                               //"sprmSBCustomize"
    {0xB01F, (FNReadRecord)0},                   //"sprmSXaPage" sep.xaPage;xa;
                                                 //word;
    {0xB020, (FNReadRecord)0},                   //"sprmSYaPage" sep.yaPage;ya;
                                                 //word;
    {0x2205, (FNReadRecord)0},                   //"sprmSDxaLeft" sep.dxaLeft;
                                                 //dxa;word;
    {0xB022, (FNReadRecord)0},                   //"sprmSDxaRight" sep.dxaRight;
                                                 //dxa;word;
    {0x9023, (FNReadRecord)0},                   //"sprmSDyaTop" sep.dyaTop;dya;
                                                 //word;
    {0x9024, (FNReadRecord)0},                   //"sprmSDyaBottom"
                                                 //sep.dyaBottom;dya;word;
    {0xB025, (FNReadRecord)0},                   //"sprmSDzaGutter"
                                                 //sep.dzaGutter;dza;word;
    {0x5026, (FNReadRecord)0},                   //"sprmSDmPaperReq"
                                                 //sep.dmPaperReq;dm;word;
    {0xD227, (FNReadRecord)0},                   //"sprmSPropRMark"
                                                 //sep.fPropRMark,
                                                 //sep.ibstPropRMark,
                                                 //sep.dttmPropRMark ;complex;
                                                 //variable length always
                                                 //recorded as 7 bytes;
  //0x3228, ? ? ?,                               //"sprmSFBiDi",
  //0x3229, ? ? ?,                               //"sprmSFFacingCol"
    {0x322A, (FNReadRecord)0},                   //"sprmSFRTLGutter", set to 1
                                                 //if gutter is on the right.
    {0x702B, (FNReadRecord)0},                   //"sprmSBrcTop" sep.brcTop;BRC;
                                                 //long;
    {0x702C, (FNReadRecord)0},                   //"sprmSBrcLeft" sep.brcLeft;
                                                 //BRC;long;
    {0x702D, (FNReadRecord)0},                   //"sprmSBrcBottom"
                                                 //sep.brcBottom;BRC;long;
    {0x702E, (FNReadRecord)0},                   //"sprmSBrcRight" sep.brcRight;
                                                 //BRC;long;
    {0x522F, (FNReadRecord)0},                   //"sprmSPgbProp" sep.pgbProp;
                                                 //word;
    {0x7030, (FNReadRecord)0},                   //"sprmSDxtCharSpace"
                                                 //sep.dxtCharSpace;dxt;long;
    {0x9031, (FNReadRecord)0},                   //"sprmSDyaLinePitch"
                                                 //sep.dyaLinePitch;dya;
                                                 //WRONG:long; RIGHT:short; !
  //0x5032, ? ? ?,                               //"sprmSClm"
    {0x5033, (FNReadRecord)0},                   //"sprmSTextFlow"
                                                 //sep.wTextFlow;complex ;short
    {0x5400, (FNReadRecord)0},                   //"sprmTJc" tap.jc;jc;word (low
                                                 //order byte is significant);
    {0x9601, (FNReadRecord)0},                   //"sprmTDxaLeft"
                                                 //tap.rgdxaCenter; dxa; word;
    {0x9602, (FNReadRecord)0},                   //"sprmTDxaGapHalf"
                                                 //tap.dxaGapHalf,
                                                 //tap.rgdxaCenter; dxa; word;
    {0x3403, (FNReadRecord)0},                   //"sprmTFCantSplit"
                                                 //tap.fCantSplit;1 or 0;byte;
    {0x3404, (FNReadRecord)0},                   //"sprmTTableHeader"
                                                 //tap.fTableHeader;1 or 0;byte;
    {0xD605, (FNReadRecord)0},                   //"sprmTTableBorders"
                                                 //tap.rgbrcTable;complex;
                                                 //24 bytes;
    {0xD606, (FNReadRecord)0},                   //"sprmTDefTable10"
                                                 //tap.rgdxaCenter,
                                                 //tap.rgtc;complex; variable
                                                 //length;
    {0x9407, (FNReadRecord)0},                   //"sprmTDyaRowHeight"
                                                 //tap.dyaRowHeight;dya;word;
    {0xD608, (FNReadRecord)0},                   //"sprmTDefTable"
                                                 //tap.rgtc;complex
    {0xD609, (FNReadRecord)0},                   //"sprmTDefTableShd"
                                                 //tap.rgshd;complex
    {0x740A, (FNReadRecord)0},                   //"sprmTTlp" tap.tlp;TLP;
                                                 //4 bytes;
  //0x560B, ? ? ?,                               //"sprmTFBiDi"
  //0x740C, ? ? ?,                               //"sprmTHTMLProps"
    {0xD620, (FNReadRecord)0},                   //"sprmTSetBrc"
                                                 //tap.rgtc[].rgbrc;complex;
                                                 //5 bytes;
    {0x7621, (FNReadRecord)0},                   //"sprmTInsert"
                                                 //tap.rgdxaCenter,
                                                 //tap.rgtc;complex ;4 bytes;
    {0x5622, (FNReadRecord)0},                   //"sprmTDelete"
                                                 //tap.rgdxaCenter,
                                                 //tap.rgtc;complex ;word;
    {0x7623, (FNReadRecord)0},                   //"sprmTDxaCol"
                                                 //tap.rgdxaCenter;complex;
                                                 //4 bytes;
    {0x5624, (FNReadRecord)0},                   //"sprmTMerge"
                                                 //tap.fFirstMerged,
                                                 //tap.fMerged;complex; word;
    {0x5625, (FNReadRecord)0},                   //"sprmTSplit"
                                                 //tap.fFirstMerged,
                                                 //tap.fMerged;complex ;word;
    {0xD626, (FNReadRecord)0},                   //"sprmTSetBrc10"
                                                 //tap.rgtc[].rgbrc;complex;
                                                 //5 bytes;
    {0x7627, (FNReadRecord)0},                   //"sprmTSetShd" tap.rgshd;
                                                 //complex; 4 bytes;
    {0x7628, (FNReadRecord)0},                   //"sprmTSetShdOdd"
                                                 //tap.rgshd;complex;4 bytes;
    {0x7629, (FNReadRecord)0},                   //"sprmTTextFlow"
                                                 //tap.rgtc[].fVertical
                                                 //tap.rgtc[].fBackward
                                                 //tap.rgtc[].fRotateFont
                                                 //0 or 10 or 10 or 1;word;
  //0xD62A, ? ? ?  ,                             //"sprmTDiagLine"
    {0xD62B, (FNReadRecord)0},                   //"sprmTVertMerge"
                                                 //tap.rgtc[].vertMerge;complex;
                                                 //variable length always
                                                 //recorded as 2 bytes;
    {0xD62C, (FNReadRecord)0},                   //"sprmTVertAlign"
                                                 //tap.rgtc[].vertAlign;complex
                                                 //variable length always
                                                 //recorded as 3 byte;
    {0xCA78, &SwWW8ImplReader::Read_DoubleLine_Rotate},
    {0x6649, (FNReadRecord)0},                   //undocumented
    {0x6649, (FNReadRecord)0},                   //undocumented
    {0xF614, (FNReadRecord)0},                   //undocumented
    {0xD612, (FNReadRecord)0},                   //undocumented
    {0xD613, (FNReadRecord)0},                   //undocumented
    {0xD61A, (FNReadRecord)0},                   //undocumented
    {0xD61B, (FNReadRecord)0},                   //undocumented
    {0xD61C, (FNReadRecord)0},                   //undocumented
    {0xD61D, (FNReadRecord)0},                   //undocumented
    {0xD634, (FNReadRecord)0},                   //undocumented
    {0xD632, (FNReadRecord)0},                   //undocumented
    {0xD238, (FNReadRecord)0},                   //undocumented sep
    {0xC64E, (FNReadRecord)0},                   //undocumented
    {0xC64F, (FNReadRecord)0},                   //undocumented
    {0xC650, (FNReadRecord)0},                   //undocumented
    {0xC651, (FNReadRecord)0},                   //undocumented
    {0xF661, (FNReadRecord)0},                   //undocumented
    {0x4873, &SwWW8ImplReader::Read_Language},   //"sprmCRgLid3?" chp.rglid[0];
                                                 //LID: for non-Far East text
                                                 //(like a duplicate of 486D)
    {0x4874, (FNReadRecord)0},                   //undocumented
    {0x6463, (FNReadRecord)0},                   //undocumented
    {0x2461, &SwWW8ImplReader::Read_RTLJustify}, //undoc, must be asian version
                                                 //of "sprmPJc"
    {0x845E, &SwWW8ImplReader::Read_LR},         //undoc, must be asian version
                                                 //of "sprmPDxaLeft"
    {0x8460, &SwWW8ImplReader::Read_LR},         //undoc, must be asian version
                                                 //of "sprmPDxaLeft1"
    {0x845D, &SwWW8ImplReader::Read_LR},         //undoc, must be asian version
                                                 //of "sprmPDxaRight"
    {0x3615, (FNReadRecord)0},                   //undocumented
    {0x360D, (FNReadRecord)0},                   //undocumented
    {0x940E, (FNReadRecord)0},                   //undocumented
    {0x940F, (FNReadRecord)0},                   //undocumented
    {0x9410, (FNReadRecord)0},                   //undocumented
    {0x703A, (FNReadRecord)0},                   //undocumented
    {0x303B, (FNReadRecord)0},                   //undocumented
    {0x244B, (FNReadRecord)0},                   //undocumented, must be
                                                 //subtable "sprmPFInTable"
    {0x244C, &SwWW8ImplReader::Read_TabRowEnd},  //undocumented, must be
                                                 // subtable "sprmPFTtp"
    {0x6815, (FNReadRecord)0},                   //undocumented
    {0x6816, (FNReadRecord)0},                   //undocumented
    {0x6870, &SwWW8ImplReader::Read_TxtForeColor},
    {0xC64D, &SwWW8ImplReader::Read_ParaBackColor},
    {0x6467, (FNReadRecord)0},                   //undocumented
    {0xF617, (FNReadRecord)0},                   //undocumented
    {0xD660, (FNReadRecord)0},                   //undocumented
    {0xD670, (FNReadRecord)0},                   //undocumented
    {0xCA71, &SwWW8ImplReader::Read_TxtBackColor},//undocumented
    {0x303C, (FNReadRecord)0},                   //undocumented
    {0x245B, &SwWW8ImplReader::Read_ParaAutoBefore},//undocumented, para
    {0x245C, &SwWW8ImplReader::Read_ParaAutoAfter}//undocumented, para
                                                 //autobefore ?
};

//-----------------------------------------
//      Hilfsroutinen : SPRM finden
//-----------------------------------------

extern "C"
{
    static int CompSprmReadId( const void *pFirst, const void *pSecond)
    {
        return( ((SprmReadInfo*)pFirst )->nId - ((SprmReadInfo*)pSecond)->nId );
    }
}

SprmReadInfo& WW8GetSprmReadInfo( USHORT nId )
{
    // ggfs. Tab sortieren
    static bool bInit = false;
    if (!bInit)
    {
        qsort( (void*)aSprmReadTab, sizeof(aSprmReadTab) /
            sizeof(aSprmReadTab[0]), sizeof(aSprmReadTab[0]), CompSprmReadId );
        bInit = true;
    }
    // Sprm heraussuchen
    void* pFound;
    SprmReadInfo aSrch;
    aSrch.nId = nId;
    if ( 0 == (pFound = bsearch( (const void*)&aSrch, (const void*)aSprmReadTab,
        sizeof(aSprmReadTab) / sizeof (aSprmReadTab[0]),
        sizeof(aSprmReadTab[0] ), CompSprmReadId )))
    {
        // im Fehlerfall auf Nulltes Element verweisen
        pFound = (void*)aSprmReadTab;
    }
    return *(SprmReadInfo*) pFound;
}

//-----------------------------------------
//      Hilfsroutinen : SPRMs
//-----------------------------------------
void SwWW8ImplReader::EndSprm( USHORT nId )
{
    if( ( nId > 255 ) && ( nId < 0x0800 ) ) return;

    SprmReadInfo& rSprm = WW8GetSprmReadInfo( nId );
    /*
    if ( aWwSprmTab[nId] == 0 )
        return;

    (this->*SprmReadInfo)( nId, 0, -1 );
    */
    if( rSprm.pReadFnc )
        (this->*rSprm.pReadFnc)( nId, 0, -1 );
}

short SwWW8ImplReader::ImportSprm(const BYTE* pPos,USHORT nId)
{
    if (!nId)
        nId = mpSprmParser->GetSprmId(pPos);

#if OSL_DEBUG_LEVEL > 1
    ASSERT( nId != 0xff, "Sprm FF !!!!" );
#endif

    SprmReadInfo& rSprm = WW8GetSprmReadInfo(nId);

    USHORT nFixedLen = mpSprmParser->DistanceToData(nId);
    USHORT nL = mpSprmParser->GetSprmSize(nId, pPos);

    if (rSprm.pReadFnc)
        (this->*rSprm.pReadFnc)(nId, pPos + nFixedLen, nL - nFixedLen);

    return nL;
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
