/*************************************************************************
 *
 *  $RCSfile: ww8par.cxx,v $
 *
 *  $Revision: 1.117 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:43:04 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): cmc@openoffice.org, tono@openoffice.org
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif

#ifndef _OFA_FLTRCFG_HXX
#include <offmgr/fltrcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_CSCOITEM_HXX //autogen
#include <svx/cscoitem.hxx>
#endif
#ifndef _SVX_SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx> // SvxLRSpaceItem
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVXMSBAS_HXX
#include <svx/svxmsbas.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif

#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _FMTURL_HXX
#include <fmturl.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPam
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>            // class SwTxtNode
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>         // class SwPageDesc
#endif
#ifndef _NUMRULE_HXX //autogen
#include <numrule.hxx>
#endif
#ifndef _PARATR_HXX //autogen
#include <paratr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>           // ReadFilterFlags
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _DOCUFLD_HXX //autogen
#include <docufld.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif
#ifndef _VIEWSH_HXX     // for the pagedescname from the ShellRes
#include <viewsh.hxx>
#endif
#ifndef _SHELLRES_HXX   // for the pagedescname from the ShellRes
#include <shellres.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>            // ERR_WW8_...
#endif
#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>          // class SwTableLines, ...
#endif

#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif

#include <algorithm>
#include <functional>

#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif
#ifndef SW_WRITERWORDGLUE
#include "writerwordglue.hxx"
#endif


#ifndef _WW8PAR2_HXX
#include "ww8par2.hxx"          // class WW8RStyle, class WW8AnchorPara
#endif
#ifndef _WW8PAR_HXX
#include "ww8par.hxx"
#endif

#define MM_250 1417             // WW-Default fuer Hor. Seitenraender: 2.5 cm
#define MM_200 1134             // WW-Default fuer u.Seitenrand: 2.0 cm


SwMSDffManager::SwMSDffManager( SwWW8ImplReader& rRdr )
    : SvxMSDffManager(*rRdr.pTableStream, rRdr.pWwFib->fcDggInfo,
        rRdr.pDataStream, 0, 0, COL_WHITE, 12, rRdr.pStrm,
        rRdr.maTracer.GetTrace()),
    rReader(rRdr), pFallbackStream(0), pOldEscherBlipCache(0)
{
    nSvxMSDffOLEConvFlags = SwMSDffManager::GetFilterFlags();
}

UINT32 SwMSDffManager::GetFilterFlags()
{
    UINT32 nFlags(0);
    if (const OfaFilterOptions* pOpt = OFF_APP()->GetFilterOptions())
    {
        if (pOpt->IsMathType2Math())
            nFlags |= OLE_MATHTYPE_2_STARMATH;
        if (pOpt->IsExcel2Calc())
            nFlags |= OLE_EXCEL_2_STARCALC;
        if (pOpt->IsPowerPoint2Impress())
            nFlags |= OLE_POWERPOINT_2_STARIMPRESS;
    }
    return nFlags;
}

/*
 * I would like to override the default OLE importing to add a test
 * and conversion of OCX controls from their native OLE type into our
 * native nonOLE Form Control Objects.
 *
 * cmc
 */
SdrObject* SwMSDffManager::ImportOLE( long nOLEId, const Graphic& rGrf,
                                        const Rectangle& rBoundRect ) const
{
    SdrObject* pRet = 0;
    String sStorageName;
    SvStorageRef xSrcStg, xDstStg;
    if( GetOLEStorageName( nOLEId, sStorageName, xSrcStg, xDstStg ))
    {
        SvStorageRef xSrc = xSrcStg->OpenStorage( sStorageName,
            STREAM_READWRITE| STREAM_SHARE_DENYALL );
        ASSERT(rReader.pFormImpl, "No Form Implementation!");
        STAR_REFERENCE( drawing::XShape ) xShape;
        if ( (!(rReader.bIsHeader || rReader.bIsFooter)) &&
            rReader.pFormImpl->ReadOCXStream(xSrc,&xShape,true))
        {
            pRet = GetSdrObjectFromXShape(xShape);
        }
        else
        {
            pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                rGrf, rBoundRect, pStData, nSvxMSDffOLEConvFlags );
        }
    }
    return pRet;
}

void SwMSDffManager::DisableFallbackStream()
{
    ASSERT(!pFallbackStream || !pOldEscherBlipCache,
        "if you're recursive, you're broken");
    pFallbackStream = pStData2;
    pOldEscherBlipCache = pEscherBlipCache;
    pEscherBlipCache = 0;
    pStData2 = 0;
}

void SwMSDffManager::EnableFallbackStream()
{
    pStData2 = pFallbackStream;
    pEscherBlipCache = pOldEscherBlipCache;
    pOldEscherBlipCache = 0;
    pFallbackStream = 0;
}

/***************************************************************************
#  Spezial FastSave - Attribute
#**************************************************************************/

void SwWW8ImplReader::Read_StyleCode( USHORT, const BYTE* pData, short nLen )
{
    if (nLen < 0)
    {
        bCpxStyle = false;
        return;
    }
    USHORT nColl = SVBT16ToShort(pData);
    if (nColl < nColls)
    {
        SetTxtFmtCollAndListLevel( *pPaM, pCollA[nColl] );
        bCpxStyle = true;
    }
}

// Read_Majority ist fuer Majority ( 103 ) und Majority50 ( 108 )
void SwWW8ImplReader::Read_Majority( USHORT, const BYTE* , short )
{
}

//-----------------------------------------
//            Stack
//-----------------------------------------
void SwWW8FltControlStack::NewAttr(const SwPosition& rPos,
    const SfxPoolItem& rAttr)
{
    ASSERT(RES_TXTATR_FIELD != rAttr.Which(), "probably don't want to put"
        "fields into the control stack");
    ASSERT(RES_FLTR_REDLINE != rAttr.Which(), "probably don't want to put"
        "redlines into the control stack");
    SwFltControlStack::NewAttr(rPos, rAttr);
}

void SwWW8FltControlStack::SetAttr(const SwPosition& rPos, USHORT nAttrId,
    bool bTstEnde, long nHand)
{
    //Doing a textbox, and using the control stack only as a temporary
    //collection point for properties which will are not to be set into
    //the real document
    if (rReader.pPlcxMan && rReader.pPlcxMan->GetDoingDrawTextBox())
    {
        USHORT nCnt = Count();
        for (USHORT i=0; i < nCnt; ++i)
        {
            SwFltStackEntry* pEntry = (*this)[i];
            if (nAttrId == pEntry->pAttr->Which())
            {
                DeleteAndDestroy(i--);
                --nCnt;
            }
        }
    }
    else //Normal case, set the attribute into the document
        SwFltControlStack::SetAttr(rPos, nAttrId, bTstEnde, nHand);
}

long GetListFirstLineIndent(const SwNumFmt &rFmt)
{
    SvxAdjust eAdj = rFmt.GetNumAdjust();
    long nReverseListIndented;
    if (eAdj == SVX_ADJUST_RIGHT)
        nReverseListIndented = -rFmt.GetCharTextDistance();
    else if (eAdj == SVX_ADJUST_CENTER)
        nReverseListIndented = rFmt.GetFirstLineOffset()/2;
    else
        nReverseListIndented = rFmt.GetFirstLineOffset();
    return nReverseListIndented;
}

long lcl_GetTrueMargin(const SvxLRSpaceItem &rLR, const SwNumFmt &rFmt,
    long &rFirstLinePos, long &rPseudoListFirstLine)
{
    long nBodyIndent = rLR.GetTxtLeft();
    long nFirstLineDiff = rLR.GetTxtFirstLineOfst();
    rFirstLinePos = nBodyIndent + nFirstLineDiff;
    long nPseudoListBodyIndent = rFmt.GetAbsLSpace();
    long nReverseListIndented = GetListFirstLineIndent(rFmt);
    rPseudoListFirstLine = nPseudoListBodyIndent + nReverseListIndented;
    return nReverseListIndented;
}

void SyncStyleIndentWithList(SvxLRSpaceItem &rLR, const SwNumFmt &rFmt)
{
    long nFirstLinePos, nPseudoListFirstLine;
    lcl_GetTrueMargin(rLR, rFmt, nFirstLinePos, nPseudoListFirstLine);
    nFirstLinePos -= nPseudoListFirstLine;
    rLR.SetTxtLeft(nFirstLinePos);
    rLR.SetTxtFirstLineOfst(0);
}

const SwNumFmt* GetNumFmtFromTxtNode(const SwTxtNode &rTxtNode,
    const SwDoc &rDoc)
{
    const SwNumFmt *pRet = 0;
    const SwNumRule *pRule = 0;
    const SwNodeNum* pNum;
    if (
        (pNum = rTxtNode.GetNum()) &&
        (MAXLEVEL > pNum->GetLevel()) &&
        (pRule = rTxtNode.GetNumRule())
       )
    {
        pRet = &(pRule->Get(pNum->GetLevel()));
    }
    else if (
              (pNum = rTxtNode.GetOutlineNum()) &&
              (MAXLEVEL > pNum->GetLevel()) &&
              (pRule = rDoc.GetOutlineNumRule())
            )
    {
        pRet = &(pRule->Get(pNum->GetLevel()));
    }
    return pRet;
}

const SwNumFmt* SwWW8FltControlStack::GetNumFmtFromStack(const SwPosition &rPos,
    const SwTxtNode &rTxtNode)
{
    const SwNumFmt *pRet = 0;
    const SfxPoolItem *pItem = GetStackAttr(rPos, RES_FLTR_NUMRULE);
    if (pItem && rTxtNode.GetNum())
    {
        String sName(((SfxStringItem*)pItem)->GetValue());
        const SwNodeNum *pNum = rTxtNode.GetNum();
        if (pNum && NO_NUM > pNum->GetLevel())
        {
            const SwNumRule *pRule = pDoc->FindNumRulePtr(sName);
            BYTE nLvl = GetRealLevel(pNum->GetLevel());
            pRet = &(pRule->Get(nLvl));
        }
    }
    return pRet;
}

void SyncParagraphIndentWithList(SvxLRSpaceItem &rLR, const SwNumFmt &rFmt)
{
    long nFirstLinePos, nPseudoListFirstLine;
    long nReverseListIndented = lcl_GetTrueMargin(rLR, rFmt, nFirstLinePos,
        nPseudoListFirstLine);
    if (nFirstLinePos < nPseudoListFirstLine)
        nFirstLinePos -= nPseudoListFirstLine;
    rLR.SetTxtLeft(nFirstLinePos-nReverseListIndented);
    rLR.SetTxtFirstLineOfst(nReverseListIndented);
}

void SwWW8FltControlStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry* pEntry)
{
    switch( pEntry->pAttr->Which() )
    {
        case RES_LR_SPACE:
            {
                SwPaM aRegion( rTmpPos );
                if (pEntry->MakeRegion(pDoc, aRegion, false))
                {
                    SvxLRSpaceItem aLR( *(SvxLRSpaceItem*)pEntry->pAttr );
                    ULONG nStart = aRegion.Start()->nNode.GetIndex();
                    ULONG nEnd   = aRegion.End()->nNode.GetIndex();
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = pDoc->GetNodes()[ nStart ];
                        if (pNode->IsTxtNode())
                        {
                            pNode->SetNumLSpace(false);
                            SwTxtNode *pTxtNode = (SwTxtNode*)pNode;

                            const SwNumFmt *pNum = 0;
                            pNum = GetNumFmtFromStack(*aRegion.GetPoint(),
                                *pTxtNode);

                            if (!pNum)
                                pNum = GetNumFmtFromTxtNode(*pTxtNode, *pDoc);

                            if (pNum)
                                SyncParagraphIndentWithList(aLR, *pNum);
                            ((SwCntntNode*)pNode)->SetAttr( aLR );
                            // wenn wir dies nicht tun, ueberschreibt die
                            // NumRule uns alle harten L-Randeinstellungen
                            pNode->SetNumLSpace(false);
                        }
                    }
                }
            }
            break;
        case RES_TXTATR_FIELD:
            ASSERT(!this, "What is a field doing in the control stack,"
                "probably should have been in the endstack");
            break;
        case RES_TXTATR_INETFMT:
            {
                SwPaM aRegion(rTmpPos);
                if (pEntry->MakeRegion(pDoc, aRegion, false))
                {
                    SwFrmFmt *pFrm;
                    //If we have just one single inline graphic then
                    //don't insert a field for the single frame, set
                    //the frames hyperlink field attribute directly.
                    if ((pFrm = rReader.ContainsSingleInlineGraphic(aRegion)))
                    {
                        const SwFmtINetFmt *pAttr = (const SwFmtINetFmt *)
                            pEntry->pAttr;
                        SwFmtURL aURL;
                        aURL.SetURL(pAttr->GetValue(), false);
                        aURL.SetTargetFrameName(pAttr->GetTargetFrame());
                        pFrm->SetAttr(aURL);
                    }
                    else
                        pDoc->Insert(aRegion, *pEntry->pAttr);
                }
            }
            break;
        default:
            SwFltControlStack::SetAttrInDoc(rTmpPos, pEntry);
            break;
    }
}

const SfxPoolItem* SwWW8FltControlStack::GetFmtAttr(const SwPosition& rPos,
    USHORT nWhich)
{
    const SfxPoolItem *pItem = GetStackAttr(rPos, nWhich);
    if (!pItem)
    {
        const SwCntntNode *pNd = pDoc->GetNodes()[ rPos.nNode ]->GetCntntNode();
        if (!pNd)
            pItem = &pDoc->GetAttrPool().GetDefaultItem(nWhich);
        else
            pItem = &pNd->GetAttr(nWhich);
    }
    return pItem;
}

const SfxPoolItem* SwWW8FltControlStack::GetStackAttr(const SwPosition& rPos,
    USHORT nWhich)
{
    SwNodeIndex aNode( rPos.nNode, -1 );
    USHORT nIdx = rPos.nContent.GetIndex();

    USHORT nSize = Count();
    while (nSize)
    {
        const SwFltStackEntry* pEntry = (*this)[ --nSize ];
        if (pEntry->pAttr->Which() == nWhich)
        {
            if ( (pEntry->bLocked) || (
                (pEntry->nMkNode <= aNode) && (pEntry->nPtNode >= aNode) &&
                (pEntry->nMkCntnt <= nIdx) && (pEntry->nPtCntnt >= nIdx) ) )
            {
                return (const SfxPoolItem*)pEntry->pAttr;
            }
        }
    }
    return 0;
}

void SwWW8FltControlStack::Delete(const SwPaM &rPam)
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    SwNodeIndex aStartNode( pStt->nNode, -1 );
    USHORT nStartIdx = pStt->nContent.GetIndex();
    SwNodeIndex aEndNode( pEnd->nNode, -1 );
    USHORT nEndIdx = pEnd->nContent.GetIndex();


    for (USHORT nSize = Count(); nSize > 0;)
    {
        SwFltStackEntry* pEntry = (*this)[--nSize];

        //to be adjusted
        if (pEntry->nMkNode > aStartNode ||
            (pEntry->nMkNode == aStartNode && pEntry->nMkCntnt >= nStartIdx) )
        {
            //after start, before end, delete
            if (!pEntry->bLocked && (
                    pEntry->nPtNode < aEndNode ||
                    (pEntry->nPtNode == aEndNode && pEntry->nPtCntnt < nEndIdx))
                    )
            {
                DeleteAndDestroy(nSize);
            }
            else
            {
                //move everything backwards
                pEntry->nMkNode = aStartNode;
                pEntry->nMkCntnt = nStartIdx;
                pEntry->nPtNode -= (aEndNode.GetIndex()-aStartNode.GetIndex());
                if (!pEntry->bLocked)
                    pEntry->nPtCntnt -= nEndIdx - nStartIdx;
            }
        }
        else if ((!pEntry->bLocked) && ((pEntry->nPtNode > aStartNode) || (
            pEntry->nPtNode == aStartNode && pEntry->nPtCntnt > nStartIdx)))
        {
            pEntry->nPtNode -= (aEndNode.GetIndex()-aStartNode.GetIndex());
            pEntry->nPtCntnt -= nEndIdx - nStartIdx;
        }
    }
}

bool SwWW8FltRefStack::IsFtnEdnBkmField(const SwFmtFld& rFmtFld, USHORT& rBkmNo)
{
    const SwField* pFld = rFmtFld.GetFld();
    USHORT nSubType;
    return (pFld && (RES_GETREFFLD == pFld->Which())
            && ((REF_FOOTNOTE == (nSubType = pFld->GetSubType())) ||
                (REF_ENDNOTE  == nSubType))
            && ((SwGetRefField*)pFld)->GetSetRefName().Len()
                // find Sequence No of corresponding Foot-/Endnote
            && (USHRT_MAX != (rBkmNo = pDoc->FindBookmark(
                ((SwGetRefField*)pFld)->GetSetRefName() ))));
}

#ifdef DEBUG
String lcl_GetSelTxt(const SwPaM &rPaM);
#endif

Position::Position(const SwPaM &rPaM)
    : maMkNode(rPaM.GetMark()->nNode),
    maPtNode(rPaM.GetPoint()->nNode),
    mnMkCntnt(rPaM.GetMark()->nContent.GetIndex()),
    mnPtCntnt(rPaM.GetPoint()->nContent.GetIndex())
{
}

Position::Position(const Position &rEntry)
    : maMkNode(rEntry.maMkNode), maPtNode(rEntry.maPtNode),
    mnMkCntnt(rEntry.mnMkCntnt), mnPtCntnt(rEntry.mnPtCntnt)
{
}

class DeletePaM
{
private:
    SwDoc &mrDoc;
public:
    explicit DeletePaM(SwDoc &rDoc) : mrDoc(rDoc) {}
    void operator()(const Position &rPaM);
private:
    //No assignment
    DeletePaM& operator=(const DeletePaM&);
};

SwWW8FltRefStack::~SwWW8FltRefStack()
{
    std::for_each(maScheduledForDelete.begin(), maScheduledForDelete.end(),
        DeletePaM(*pDoc));
}

void DeletePaM::operator()(const Position &rPaM)
{
    SwPaM aPaM(rPaM.maMkNode, rPaM.mnMkCntnt, rPaM.maPtNode, rPaM.mnPtCntnt);
#ifdef DEBUG
    String aStr(lcl_GetSelTxt(aPaM));
    aStr = aStr;
#endif
    mrDoc.Delete(aPaM);
}

bool lcl_ShouldMakeHidden(const SwFltStackEntry* pEntry, const SwPaM &rPaM)
{
    bool bEmpty = true;
    if (pEntry->nMkNode.GetIndex() == pEntry->nPtNode.GetIndex())
    {
        bEmpty = false;
        /*
        An empty paragraph that is hidden is acceptable, otherwise it only
        makes sense to make it hidden if there is something to hide.  and if
        there is some "hidden" content already, e.g. #110465# we will retain it
        this way.
        */
        if (pEntry->nMkCntnt == pEntry->nPtCntnt && !pEntry->nPtCntnt)
            bEmpty = false;
        else if (const SwTxtNode *pDest = rPaM.GetNode()->GetTxtNode())
        {
            String sString = pDest->GetExpandTxt(pEntry->nMkCntnt,
                pEntry->nPtCntnt-pEntry->nMkCntnt);
            if (!sString.Len())
                bEmpty = true;
        }
    }
    return bEmpty;
}

void SwWW8FltRefStack::SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry* pEntry)
{
    switch( pEntry->pAttr->Which() )
    {
        /*
        Look up these in our lists of bookmarks that were changed to
        variables, and replace the ref field with a var field, otherwise
        do normal (?) strange stuff
        */
        case RES_TXTATR_FIELD:
        {
            SwNodeIndex aIdx( pEntry->nMkNode, +1 );
            SwPaM aPaM( aIdx, pEntry->nMkCntnt );

            SwFmtFld& rFmtFld   = *(SwFmtFld*)pEntry->pAttr;
            SwField* pFld = rFmtFld.GetFld();

            bool bIsHidden = RangeToHidden(pFld, pEntry, aPaM);
            if ((!bIsHidden) && (!RefToVar(pFld,pEntry)))
            {
                USHORT nBkmNo;
                if( IsFtnEdnBkmField(rFmtFld, nBkmNo) )
                {
                    SwBookmark& rBkMrk = pDoc->GetBookmark( nBkmNo );

                    const SwPosition& rBkMrkPos = rBkMrk.GetPos();

                    SwTxtNode* pTxt = rBkMrkPos.nNode.GetNode().GetTxtNode();
                    if( pTxt && rBkMrkPos.nContent.GetIndex() )
                    {
                        SwTxtAttr* pFtn = pTxt->GetTxtAttr(
                            rBkMrkPos.nContent.GetIndex()-1, RES_TXTATR_FTN );
                        if( pFtn )
                        {
                            USHORT nRefNo = ((SwTxtFtn*)pFtn)->GetSeqRefNo();

                            ((SwGetRefField*)pFld)->SetSeqNo( nRefNo );

                            if( pFtn->GetFtn().IsEndNote() )
                                ((SwGetRefField*)pFld)->SetSubType(REF_ENDNOTE);
                        }
                    }
                }
            }

            bool bEmpty = false;
            if (bIsHidden)
                bEmpty = lcl_ShouldMakeHidden(pEntry, aPaM);

            if (!bEmpty)
            {
                if (bIsHidden)
                {
                    maScheduledForDelete.push_front(Position(aPaM));
                    aPaM.DeleteMark();
                }

                pDoc->Insert(aPaM, *pEntry->pAttr);
                MoveAttrs(*aPaM.GetPoint());
            }
        }
        break;
        case RES_FLTR_TOX:
        case RES_FLTR_BOOKMARK:
            SwFltEndStack::SetAttrInDoc(rTmpPos, pEntry);
            break;
        default:
            ASSERT(!this, "EndStck used with non field, not what we want");
            SwFltEndStack::SetAttrInDoc(rTmpPos, pEntry);
            break;
    }
}

/*
 For styles we will do our tabstop arithmetic in word style and adjust them to
 writer style after all the styles have been finished and the just settles as
 to what affects what.  For explicit attributes we use the style underlying
 margins and if there was an explicit margin change we handle the
 adjustment on the close of the margina attribute, which gaurantees the
 collection of all margin and tab begins.
*/
void SwWW8ImplReader::Read_Tab(USHORT , const BYTE* pData, short nLen)
{
    if( nLen < 0 )
    {
        if (!pTabNode) //Leave unclosed if the margins have not been finialized
            pCtrlStck->SetAttr(*pPaM->GetPoint(), RES_PARATR_TABSTOP);
        return;
    }

    // Para L-Space
    short nLeftPMgn = pAktColl ? 0 : pCollA[nAktColl].nLeftParaMgn;

    // Para L-Space OR Left Margin of Para's first line respectively
    short nLeftMostPos = pAktColl ? 0 : pCollA[nAktColl].nTxtFirstLineOfst;
    if( 0 < nLeftMostPos )
        nLeftMostPos = 0;

    short i;
    const BYTE* pDel = pData + 1;                       // Del - Array
    BYTE nDel = pData[0];
    const BYTE* pIns = pData + 2*nDel + 2;          // Ins - Array
    BYTE nIns = pData[nDel*2+1];
    WW8_TBD* pTyp = (WW8_TBD*)(pData + 2*nDel + 2*nIns + 2);// Typ - Array

    SvxTabStopItem aAttr( 0, 0, SVX_TAB_ADJUST_DEFAULT );

    const SwTxtFmtColl* pSty = 0;
    USHORT nTabBase;
    if (pAktColl)               // StyleDef
    {
        nTabBase = pCollA[nAktColl].nBase;
        if (nTabBase < nColls)              // Based On
            pSty = (const SwTxtFmtColl*)pCollA[nTabBase].pFmt;
    }
    else
    {                                       // Text
        nTabBase = nAktColl;
        pSty = (const SwTxtFmtColl*)pCollA[nAktColl].pFmt;
    }

    bool bFound = false;
    while( pSty && !bFound )
    {
        const SfxPoolItem* pTabs;
        bFound = pSty->GetAttrSet().GetItemState(RES_PARATR_TABSTOP, false,
            &pTabs) == SFX_ITEM_SET;
        if( bFound )
            aAttr = *((const SvxTabStopItem*)pTabs);
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

    SvxTabStop aTabStop;

    for( i=0; i<nDel; i++ )
    {
        USHORT nPos = aAttr.GetPos( SVBT16ToShort( pDel + i*2 ) - nLeftPMgn );
        if( nPos != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos, 1 );
    }

    for( i=0; i<nIns; i++ )
    {
        short nPos = SVBT16ToShort( pIns + i*2 ) - nLeftPMgn;
        if( nPos < nLeftMostPos )
            continue;
        aTabStop.GetTabPos() = nPos;
        switch( SVBT8ToByte( pTyp[i].aBits1 ) & 0x7 )       // pTyp[i].jc
        {
            case 0:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT;
                break;
            case 1:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER;
                break;
            case 2:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT;
                break;
            case 3:
                aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DECIMAL;
                break;
            case 4:
                continue;                   // ignoriere Bar
        }

        switch( SVBT8ToByte( pTyp[i].aBits1 ) >> 3 & 0x7 )
        {
            case 0:
                aTabStop.GetFill() = ' ';
                break;
            case 1:
                aTabStop.GetFill() = '.';
                break;
            case 2:
                aTabStop.GetFill() = '-';
                break;
            case 3:
            case 4:
                aTabStop.GetFill() = '_';
                break;
        }

        USHORT nPos2 = aAttr.GetPos( nPos );
        if( nPos2 != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos2, 1 );       // sonst weigert sich das Insert()

        aAttr.Insert( aTabStop );
    }
    NewAttr( aAttr );
}

//-----------------------------------------
//              DOP
//-----------------------------------------

void SwWW8ImplReader::ImportDop()
{
    sw::log::Context aContext(maTracer, sw::log::eDocumentProperties);
    // correct the LastPrinted date in DocumentInfo
    if (rDoc.GetpInfo())
    {
        DateTime aLastPrinted(
            WW8ScannerBase::WW8DTTM2DateTime(pWDop->dttmLastPrint));
        SfxDocumentInfo aNeuDocInf(*rDoc.GetpInfo());
        SfxStamp aPrinted(aNeuDocInf.GetPrinted());
        if (aPrinted.GetTime() != aLastPrinted)
        {
            // check if WW8 date was set
            if (aLastPrinted == DateTime(Date(0), Time(0)))
            {
                // create "invalid" value for SfxStamp
                // (as seen in sfx2/DOSINF.HXX)
                aPrinted.SetTime(DateTime(Date( 1, 1, 1601 ), Time( 0, 0, 0 )));
            }
            else
                aPrinted.SetTime( aLastPrinted );

            aNeuDocInf.SetPrinted( aPrinted );
            rDoc.SetInfo(aNeuDocInf);
        }
    }


    // Abstand zwischen zwei Absaetzen ist die SUMME von unterem
    // Abst. des ersten und oberem Abst. des zweiten
    rDoc.SetParaSpaceMax(!pWDop->fDontUseHTMLAutoSpacing, true);
    maTracer.Log(sw::log::eDontUseHTMLAutoSpacing);
    // move tabs on alignment
    rDoc.SetTabCompat(true);
    maTracer.Log(sw::log::eTabStopDistance);

    // Import Default-Tabs
    long nDefTabSiz = pWDop->dxaTab;
    if( nDefTabSiz < 56 )
        nDefTabSiz = 709;

    // wir wollen genau einen DefaultTab
    SvxTabStopItem aNewTab( 1, USHORT(nDefTabSiz), SVX_TAB_ADJUST_DEFAULT );
    ((SvxTabStop&)aNewTab[0]).GetAdjustment() = SVX_TAB_ADJUST_DEFAULT;

    rDoc.GetAttrPool().SetPoolDefaultItem( aNewTab );

    // set default language (from FIB)
    rDoc.GetAttrPool().SetPoolDefaultItem(
        SvxLanguageItem( (const LanguageType)pWwFib->lid )  );

    rDoc._SetUseVirtualDevice(!pWDop->fUsePrinterMetrics);
    if (!pWDop->fUsePrinterMetrics)
        maTracer.Log(sw::log::ePrinterMetrics);

    if (!pWDop->fNoLeading)
        maTracer.Log(sw::log::eExtraLeading);

    rDoc.SetAddFlyOffsets( true );

    //import magic doptypography information, if its there
    if (pWwFib->nFib > 105)
        ImportDopTypography(pWDop->doptypography);


    if (pWDop->fProtEnabled)
    {
        using namespace com::sun::star;

        uno::Reference<lang::XComponent> xModelComp(
            rDoc.GetDocShell()->GetModel(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDocProps(
            xModelComp, uno::UNO_QUERY);
        if (xDocProps.is())
        {
            uno::Reference<beans::XPropertySetInfo> xInfo =
                xDocProps->getPropertySetInfo();
            sal_Bool bValue = false;
            if (xInfo.is() &&
                xInfo->hasPropertyByName(C2U("ApplyFormDesignMode")))
            {
                xDocProps->setPropertyValue(C2U("ApplyFormDesignMode"),
                    cppu::bool2any(bValue));
            }
        }
    }

}

void SwWW8ImplReader::ImportDopTypography(const WW8DopTypography &rTypo)
{
    using namespace ::com::sun::star;
    switch (rTypo.iLevelOfKinsoku)
    {
#if 0
        /*
        Do the defaults differ between Microsoft versions ?, do we do
        something about it if so ?
        */
        case 0:
            if (pWwFib->nFib < 2000)
                //use old defaults ??
            else
                //use new defaults ??
            break;
        case 1:
            if (pWwFib->nFib < 2000)
                //use old defaults ??
            else
                //use new defaults ??
            break;
#endif
        case 2: //custom
            {
                i18n::ForbiddenCharacters aForbidden(rTypo.rgxchFPunct,
                    rTypo.rgxchLPunct);
                rDoc.SetForbiddenCharacters(rTypo.GetConvertedLang(),
                        aForbidden);
                //Obviously cannot set the standard level 1 for japanese, so
                //bail out now while we can.
                if (rTypo.GetConvertedLang() == LANGUAGE_JAPANESE)
                    return;
            }
            break;
        default:
            break;
    }

    /*
    This MS hack means that level 2 of japanese is not in operation, so we put
    in what we know are the MS defaults, there is a complementary reverse
    hack in the writer. Its our default as well, but we can set it anyway
    as a flag for later.
    */
    if (!rTypo.reserved2)
    {
        i18n::ForbiddenCharacters aForbidden(rTypo.GetJapanNotBeginLevel1(),
            rTypo.GetJapanNotEndLevel1());
        rDoc.SetForbiddenCharacters(LANGUAGE_JAPANESE,aForbidden);
    }

    rDoc.SetKernAsianPunctuation(rTypo.fKerningPunct);
    rDoc.SetCharCompressType(
        static_cast<SwCharCompressType>(rTypo.iJustification));
}

//-----------------------------------------
//      Fuss- und Endnoten
//-----------------------------------------

WW8ReaderSave::WW8ReaderSave(SwWW8ImplReader* pRdr ,WW8_CP nStartCp)
    : maTmpPos(*pRdr->pPaM->GetPoint()), mpOldStck(pRdr->pCtrlStck),
    mpOldAnchorStck(pRdr->pAnchorStck), mpOldRedlines(pRdr->mpRedlineStack),
    mpOldPlcxMan(pRdr->pPlcxMan), mpWFlyPara(pRdr->pWFlyPara),
    mpSFlyPara(pRdr->pSFlyPara), mpTableDesc(pRdr->pTableDesc),
    mnInTable(pRdr->nInTable), mnAktColl(pRdr->nAktColl),
    mcSymbol(pRdr->cSymbol), mbIgnoreText(pRdr->bIgnoreText),
    mbSymbol(pRdr->bSymbol), mbHdFtFtnEdn(pRdr->bHdFtFtnEdn),
    mbTxbxFlySection(pRdr->bTxbxFlySection), mbAnl(pRdr->bAnl),
    mbInHyperlink(pRdr->bInHyperlink), mbPgSecBreak(pRdr->bPgSecBreak),
    mbWasParaEnd(pRdr->bWasParaEnd), mbHasBorder(pRdr->bHasBorder)
{
    pRdr->bSymbol = false;
    pRdr->bHdFtFtnEdn = true;
    pRdr->bTxbxFlySection = pRdr->bAnl = pRdr->bPgSecBreak = pRdr->bWasParaEnd
        = pRdr->bHasBorder = false;
    pRdr->nInTable = 0;
    pRdr->pWFlyPara = 0;
    pRdr->pSFlyPara = 0;
    pRdr->pTableDesc = 0;
    pRdr->nAktColl = 0;

    pRdr->pCtrlStck = new SwWW8FltControlStack(&pRdr->rDoc, pRdr->nFieldFlags,
        *pRdr);

    pRdr->mpRedlineStack = new wwRedlineStack(pRdr->rDoc);

    pRdr->pAnchorStck = new SwWW8FltAnchorStack(&pRdr->rDoc, pRdr->nFieldFlags);

    // rette die Attributverwaltung: dies ist noetig, da der neu anzulegende
    // PLCFx Manager natuerlich auf die gleichen FKPs zugreift, wie der alte
    // und deren Start-End-Positionen veraendert...
    if (pRdr->pPlcxMan)
        pRdr->pPlcxMan->SaveAllPLCFx(maPLCFxSave);

    if (nStartCp != -1)
    {
        pRdr->pPlcxMan = new WW8PLCFMan(pRdr->pSBase,
            mpOldPlcxMan->GetManType(), nStartCp);
    }

    maOldApos.push_back(false);
    maOldApos.swap(pRdr->maApos);
    maOldFieldStack.swap(pRdr->maFieldStack);
}

void WW8ReaderSave::Restore( SwWW8ImplReader* pRdr )
{
    pRdr->pWFlyPara = mpWFlyPara;
    pRdr->pSFlyPara = mpSFlyPara;
    pRdr->pTableDesc = mpTableDesc;
    pRdr->cSymbol = mcSymbol;
    pRdr->bSymbol = mbSymbol;
    pRdr->bIgnoreText = mbIgnoreText;
    pRdr->bHdFtFtnEdn = mbHdFtFtnEdn;
    pRdr->bTxbxFlySection = mbTxbxFlySection;
    pRdr->nInTable = mnInTable;
    pRdr->bAnl = mbAnl;
    pRdr->bInHyperlink = mbInHyperlink;
    pRdr->bWasParaEnd = mbWasParaEnd;
    pRdr->bPgSecBreak = mbPgSecBreak;
    pRdr->nAktColl = mnAktColl;
    pRdr->bHasBorder = mbHasBorder;

    // schliesse alle Attribute, da sonst Attribute
    // entstehen koennen, die aus dem Fly rausragen
    pRdr->DeleteCtrlStk();
    pRdr->pCtrlStck = mpOldStck;

    pRdr->mpRedlineStack->closeall(*pRdr->pPaM->GetPoint());
    delete pRdr->mpRedlineStack;
    pRdr->mpRedlineStack = mpOldRedlines;

    pRdr->DeleteAnchorStk();
    pRdr->pAnchorStck = mpOldAnchorStck;

    *pRdr->pPaM->GetPoint() = maTmpPos;

    if (mpOldPlcxMan != pRdr->pPlcxMan)
    {
        delete pRdr->pPlcxMan;
        pRdr->pPlcxMan = mpOldPlcxMan;
    }
    if (pRdr->pPlcxMan)
        pRdr->pPlcxMan->RestoreAllPLCFx(maPLCFxSave);
    pRdr->maApos.swap(maOldApos);
    pRdr->maFieldStack.swap(maOldFieldStack);
}

void SwWW8ImplReader::Read_HdFtFtnText( const SwNodeIndex* pSttIdx,
    long nStartCp, long nLen, short nType )
{
    // rettet Flags u.ae. u. setzt sie zurueck
    WW8ReaderSave aSave( this );

    pPaM->GetPoint()->nNode = pSttIdx->GetIndex() + 1;      //
    pPaM->GetPoint()->nContent.Assign( pPaM->GetCntntNode(), 0 );

    // dann Text fuer Header, Footer o. Footnote einlesen

    ReadText( nStartCp, nLen, nType );              // Sepx dabei ignorieren
    aSave.Restore( this );
}

//Use authornames, if not available fall back to initials.
long SwWW8ImplReader::Read_And(WW8PLCFManResult* pRes)
{
    WW8PLCFx_SubDoc* pSD = pPlcxMan->GetAtn();
    if( !pSD )
        return 0;

    String sAuthor;
    if( bVer67 )
    {
        const WW67_ATRD* pDescri = (const WW67_ATRD*)pSD->GetData();
        const String* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst));
        if (pA)
            sAuthor = *pA;
        else
            sAuthor = String(pDescri->xstUsrInitl + 1, pDescri->xstUsrInitl[0],
                RTL_TEXTENCODING_MS_1252);
    }
    else
    {
        const WW8_ATRD* pDescri = (const WW8_ATRD*)pSD->GetData();

        const String* pA = GetAnnotationAuthor(SVBT16ToShort(pDescri->ibst));
        if( pA )
            sAuthor = *pA;
        else
        {
            sal_uInt16 nLen = SVBT16ToShort(pDescri->xstUsrInitl[0]);
            for(sal_uInt16 nIdx = 1; nIdx <= nLen; ++nIdx)
                sAuthor += SVBT16ToShort(pDescri->xstUsrInitl[nIdx]);
        }
    }

    SwNodeIndex aNdIdx( rDoc.GetNodes().GetEndOfExtras() );
    aNdIdx = *rDoc.GetNodes().MakeTextSection(aNdIdx, SwNormalStartNode,
        rDoc.GetTxtCollFromPoolSimple(RES_POOLCOLL_STANDARD, false));

    {
        SwPaM *pTempPaM = pPaM;
        SwPaM aPaM(aNdIdx);
        pPaM = &aPaM;
        Read_HdFtFtnText( &aNdIdx, pRes->nCp2OrIdx, pRes->nMemLen, MAN_AND );
        pPaM = pTempPaM;
    }

    // erzeuge das PostIt
    Date aDate;
    String sTxt;

    {   // Text aus den Nodes in den String uebertragen
        SwNodeIndex aIdx( aNdIdx, 1 ),
                    aEnd( *aNdIdx.GetNode().EndOfSectionNode() );
        SwTxtNode* pTxtNd;
        while( aIdx != aEnd )
        {
            if( 0 != ( pTxtNd = aIdx.GetNode().GetTxtNode() ))
            {
                if( sTxt.Len() )
                    sTxt += '\x0a';     // Zeilenumbruch
                sTxt += pTxtNd->GetExpandTxt();
            }
            aIdx++;
        }
        rDoc.DeleteSection( &aNdIdx.GetNode() );
    }

    rDoc.Insert( *pPaM, SwFmtFld( SwPostItField(
        (SwPostItFieldType*)rDoc.GetSysFldType( RES_POSTITFLD ), sAuthor, sTxt,
        aDate )));
    return 0;
}

void SwWW8ImplReader::Read_HdFtText(long nStart, long nLen, SwFrmFmt* pHdFtFmt)
{
    const SwNodeIndex* pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pSttIdx)
        return;

    SwPosition aTmpPos( *pPaM->GetPoint() );    // merke alte Cursorposition

    Read_HdFtFtnText(pSttIdx, nStart, nLen - 1, MAN_HDFT);

    *pPaM->GetPoint() = aTmpPos;
}

void SwWW8ImplReader::Read_HdFt(BYTE nWhichItems, BYTE grpfIhdt, int nSect, SwPageDesc* pPD,
    const SwPageDesc *pPrev)
{
    if( pHdFt )
    {
        WW8_CP start;
        long nLen;
        BYTE nNumber = 5;

        for( BYTE nI = 0x20; nI; nI >>= 1, nNumber-- )
        {
            if (nI & nWhichItems)
            {
                bool bOk = true;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos(grpfIhdt, nI, start, nLen ) && nLen >= 2 );
                else
                {
                    pHdFt->GetTextPosExact(nNumber + (nSect+1)*6, start, nLen);
                    bOk = ( 2 <= nLen );
                }

                   bool bUseLeft
                    = (nI & ( WW8_HEADER_EVEN | WW8_FOOTER_EVEN )) ? true: false;
                bool bFooter
                    = (nI & ( WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST )) ? true: false;

                SwFrmFmt* pFmt = bUseLeft ? &pPD->GetLeft() : &pPD->GetMaster();

                SwFrmFmt* pHdFtFmt;
                if (bFooter)
                {
                    bIsFooter = true;
                    //#i17196# Cannot have left without right
                    if (!pPD->GetMaster().GetFooter().GetFooterFmt())
                        pPD->GetMaster().SetAttr(SwFmtFooter(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetAttr(SwFmtFooter(true));
                    pHdFtFmt = (SwFrmFmt*)pFmt->GetFooter().GetFooterFmt();
                }
                else
                {
                    bIsHeader = true;
                    //#i17196# Cannot have left without right
                    if (!pPD->GetMaster().GetHeader().GetHeaderFmt())
                        pPD->GetMaster().SetAttr(SwFmtHeader(true));
                    if (bUseLeft)
                        pPD->GetLeft().SetAttr(SwFmtHeader(true));
                    pHdFtFmt = (SwFrmFmt*)pFmt->GetHeader().GetHeaderFmt();
                }

                if (bOk)
                    Read_HdFtText(start, nLen, pHdFtFmt);
                else if (!bOk && pPrev)
                    CopyPageDescHdFt(pPrev, pPD, nI);

                bIsHeader = bIsFooter = false;
            }
        }
    }
}

void wwSectionManager::SetHdFt(wwSection &rSection, int nSect,
    const wwSection *pPrevious)
{
    // Header / Footer nicht da
    if (!rSection.maSep.grpfIhdt)
        return;

    ASSERT(rSection.mpPage, "makes no sense to call with a main page");
    if (rSection.mpPage)
    {
        // 1 Pagedesc reicht
        mrReader.Read_HdFt(
            rSection.maSep.grpfIhdt & ~(WW8_HEADER_FIRST | WW8_FOOTER_FIRST),
            rSection.maSep.grpfIhdt,
            nSect, rSection.mpPage, pPrevious ? pPrevious->mpPage : 0);

    }

    if (rSection.mpTitlePage)
    {
        // 2 Pagedescs noetig: 1.Seite und folgende
        // 1. Seite einlesen
        mrReader.Read_HdFt(
            rSection.maSep.grpfIhdt & (WW8_HEADER_FIRST | WW8_FOOTER_FIRST),
            rSection.maSep.grpfIhdt,
            nSect, rSection.mpTitlePage, pPrevious ? pPrevious->mpTitlePage : 0);
    }

    // Kopf / Fuss - Index Updaten
    // Damit der Index auch spaeter noch stimmt
    if (mrReader.pHdFt)
        mrReader.pHdFt->UpdateIndex(rSection.maSep.grpfIhdt);

}

class AttribHere : public std::unary_function<const xub_StrLen*, bool>
{
private:
    xub_StrLen nPosition;
public:
    AttribHere(xub_StrLen nPos) : nPosition(nPos) {}
    bool operator()(const xub_StrLen *pPosition) const
    {
        return (*pPosition >= nPosition);
    }
};

void SwWW8ImplReader::AppendTxtNode(SwPosition& rPos)
{
    //If a paragraph ends in tabs and that paragraph has a tab at or beyond
    //the right of the page then word has a bug where further use of tab will
    //not wrap the paragraph around to the next page. We will, so we will
    //remove these tabs ourselves.

    //To be fair this is only a partial solution for the most common case of
    //ending in redundant tabs. It is possible to have text after these flawed
    //tabs which in word will not line wrap to the next page, and this
    //solution will not fix that.
    SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    const String &rStr = pTxt->GetTxt();
    xub_StrLen nLen=rStr.Len();
    //If we happen to end on a tab, something a well formed sentence shouldn't
    //do :-), then go into strip mode.
    if (nLen && rStr.GetChar(nLen-1) == '\x9')
    {
        std::vector<const xub_StrLen*> aLocs;
        GetNoninlineNodeAttribs(pTxt,aLocs);
        while (nLen)
        {
            if (rStr.GetChar(nLen-1) != '\x9')
                break;
            else
            {
                std::vector<const xub_StrLen*>::iterator aIter =
                    std::find_if(aLocs.begin(),aLocs.end(),AttribHere(nLen-1));
                if (aIter != aLocs.end())
                    break;
                else
                    --nLen;
            }
        }
        //We have some candidate tabs whose removal would not affect the
        //document logically. Let us see if this is one of the buggy
        //paragraphs that word would place all in one line, if so remove them
        //to match word's layout.
        if (xub_StrLen nDiff = rStr.Len() - nLen)
        {
            const SvxTabStopItem* pTabStop =
                (const SvxTabStopItem*)GetFmtAttr(RES_PARATR_TABSTOP);

            const SvxLRSpaceItem* pLR =
                (const SvxLRSpaceItem*)GetFmtAttr(RES_LR_SPACE);

            for (USHORT nI = pTabStop ? pTabStop->Count() : 0; nI--;)
            {
                //Give ourselves a leeway of 1 twip.
                if ((*pTabStop)[nI].GetTabPos() + pLR->GetTxtLeft() + 1
                    >= maSectionManager.GetPageWidth()
                    - maSectionManager.GetPageRight()
                    - maSectionManager.GetPageLeft())
                {
                    pPaM->SetMark();
                    pPaM->GetMark()->nContent-=nDiff;
                    pCtrlStck->Delete(*pPaM);
                    rDoc.Delete(*pPaM);     //this frobs the mark
                    pPaM->DeleteMark();
                    break;
                }
            }
        }
    }

    rDoc.AppendTxtNode(rPos);
    //We can flush all anchored graphics at the end of a paragraph.
    pAnchorStck->Flush();
}

USHORT SwWW8ImplReader::TabRowSprm(int nLevel) const
{
    if (bVer67)
        return 25;
    return nLevel ? 0x244C : 0x2417;
}

void SwWW8ImplReader::EndSpecial()
{
    // Frame / Table / Anl
    if (bAnl)
        StopAnl();                  // -> bAnl = false

    while(maApos.size() > 1)
    {
        StopTable();
        maApos.pop_back();
        --nInTable;
        if (maApos[nInTable] == true)
            StopApo();
    }

    if (maApos[0] == true)
        StopApo();

    ASSERT(!nInTable, "unclosed table!");
}

bool SwWW8ImplReader::ProcessSpecial(bool &rbReSync, WW8_CP nStartCp)
{
    // Frame / Table / Anl
    if (bInHyperlink)
        return false;

    rbReSync = false;

    ASSERT(nInTable >= 0,"nInTable < 0!");

    // TabRowEnd
    bool bTableRowEnd = (pPlcxMan->HasParaSprm(bVer67 ? 25 : 0x2417) != 0 );

// es muss leider fuer jeden Absatz zuerst nachgesehen werden,
// ob sich unter den sprms
// das sprm 29 (bzw. 0x261B) befindet, das ein APO einleitet.
// Alle weiteren sprms  beziehen sich dann naemlich auf das APO und nicht
// auf den normalen Text drumrum.
// Dasselbe gilt fuer eine Tabelle ( sprm 24 (bzw. 0x2416) )
// und Anls ( sprm 13 ).
// WW: Tabelle in APO geht ( Beide Anfaende treten gleichzeitig auf )
// WW: APO in Tabelle geht nicht
// d.h. Wenn eine Tabelle Inhalt eines Apo ist, dann muss der
// Apo-Anfang zuerst bearbeitet werden, damit die Tabelle im Apo steht
// und nicht umgekehrt. Am Ende muss dagegen zuerst das Tabellenende
// bearbeitet werden, da die Apo erst nach der Tabelle abgeschlossen
// werden darf ( sonst wird das Apo-Ende nie gefunden ).
// Dasselbe gilt fuer Fly / Anl, Tab / Anl, Fly / Tab / Anl.
//
// Wenn die Tabelle in einem Apo steht, fehlen im TabRowEnd-Bereich
// die Apo-Angaben. Damit hier die Apo nicht beendet wird, wird
// ProcessApo dann nicht aufgerufen.

// KHZ: When there is a table inside the Apo the Apo-flags are also
//      missing for the 2nd, 3rd... paragraphs of each cell.


//  1st look for in-table flag, for 2000+ there is a subtable flag to
//  be considered, the sprm 6649 gives the level of the table
    BYTE nCellLevel = 0;

    if (bVer67)
        nCellLevel = 0 != pPlcxMan->HasParaSprm(24);
    else
    {
        nCellLevel = 0 != pPlcxMan->HasParaSprm(0x2416);
        if (!nCellLevel)
            nCellLevel = 0 != pPlcxMan->HasParaSprm(0x244B);
    }

    WW8_TablePos *pTabPos=0;
    WW8_TablePos aTabPos;
    if (nCellLevel && !bVer67)
    {
        WW8PLCFxSave1 aSave;
        pPlcxMan->GetPap()->Save( aSave );
        rbReSync = true;
        WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();
        WW8_CP nMyStartCp=nStartCp;

        if (const BYTE *pLevel = pPlcxMan->HasParaSprm(0x6649))
            nCellLevel = *pLevel;

        if (
             SearchRowEnd(pPap, nMyStartCp, nCellLevel-1) &&
             ParseTabPos(&aTabPos,pPap)
           )
        {
            pTabPos = &aTabPos;
        }

        pPlcxMan->GetPap()->Restore( aSave );
    }

//  then look if we are in an Apo

    ApoTestResults aApo = TestApo(nCellLevel, bTableRowEnd , pTabPos);

    //look to see if we are in a Table, but Table in foot/end note not allowed
    bool bStartTab = (nInTable < nCellLevel) && !bFtnEdn;

    bool bStopTab = bWasTabRowEnd && (nInTable > nCellLevel) && !bFtnEdn;

    bWasTabRowEnd = false;  // must be deactivated right here to prevent next
                            // WW8TabDesc::TableCellEnd() from making nonsense

    if (nInTable && !bTableRowEnd && !bStopTab && (nInTable == nCellLevel && aApo.HasStartStop()))
        bStopTab = bStartTab = true;    // Required to stop and start table

//  Dann auf Anl (Nummerierung) testen
//  und dann alle Ereignisse in der richtigen Reihenfolge bearbeiten

    if( bAnl && !bTableRowEnd )
    {
        const BYTE* pSprm13 = pPlcxMan->HasParaSprm( 13 );
        if( pSprm13 )
        {                                   // Noch Anl ?
            BYTE nT = GetNumType( *pSprm13 );
            if( ( nT != WW8_Pause && nT != nWwNumType ) // Anl-Wechsel
                || aApo.HasStartStop()                  // erzwungenes Anl-Ende
                || bStopTab || bStartTab )
            {
                StopAnl();          // Anl-Restart ( = Wechsel ) ueber sprms
            }
            else
            {
                NextAnlLine( pSprm13 );                 // naechste Anl-Zeile
            }
        }
        else
        {                                           // Anl normal zuende
            StopAnl();                                  // Wirkliches Ende
        }
    }
    if (bStopTab)
    {
        StopTable();
        maApos.pop_back();
        --nInTable;
    }
    if (aApo.mbStopApo)
    {
        StopApo();
        maApos[nInTable] = false;
    }

    if (aApo.mbStartApo)
    {
        maApos[nInTable] = StartApo(aApo, pTabPos);
        // nach StartApo ist ein ReSync noetig ( eigentlich nur, falls die Apo
        // ueber eine FKP-Grenze geht
        rbReSync = true;
    }
    if (bStartTab)
    {
        if (bAnl)                           // Nummerierung ueber Zellengrenzen
            StopAnl();                      // fuehrt zu Absturz -> keine Anls
                                            // in Tabellen
        while (nInTable < nCellLevel)
        {
            nInTable += StartTable(nStartCp);
            maApos.push_back(false);
        }
        // nach StartTable ist ein ReSync noetig ( eigentlich nur, falls die
        // Tabelle ueber eine FKP-Grenze geht
        rbReSync = true;
    }
    return bTableRowEnd;
}

CharSet SwWW8ImplReader::GetCurrentCharSet()
{
    /*
    #i2015
    If the hard charset is set use it, if not see if there is an open
    character run that has set the charset, if not then fallback to the
    current underlying paragraph style.
    */
    CharSet eSrcCharSet = eHardCharSet;
    if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
    {
        if (!maFontSrcCharSets.empty())
            eSrcCharSet = maFontSrcCharSets.top();
        if ((eSrcCharSet == RTL_TEXTENCODING_DONTKNOW) && (nCharFmt != -1))
            eSrcCharSet = pCollA[nCharFmt].GetCharSet();
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
            eSrcCharSet = pCollA[nAktColl].GetCharSet();
    }
    return eSrcCharSet;
}

/*
 #i9241#
 It appears that some documents that are in a baltic 8 bit encoding which has
 some undefined characters can have use made of those characters, in which
 case they default to CP1252. If not then its perhaps that the font encoding
 is only in use for 6/7 and for 8+ if we are in 8bit mode then the encoding
 is always 1252.
*/
sal_Unicode Custom8BitToUnicode(rtl_TextToUnicodeConverter hConverter,
    sal_Char cChar)
{
    const sal_uInt32 nFlags =
        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE |
        RTL_TEXTTOUNICODE_FLAGS_FLUSH;

    sal_Unicode nConvChar;
    sal_uInt32 nInfo=0;
    sal_Size nSrcBytes=0;
    sal_Size nDestChars = rtl_convertTextToUnicode(hConverter, 0,
        &cChar, 1, &nConvChar, 1, nFlags, &nInfo, &nSrcBytes );

    if (nInfo & RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE)
    {
        rtl_TextToUnicodeConverter hCP1252Converter =
            rtl_createTextToUnicodeConverter(RTL_TEXTENCODING_MS_1252);
        nDestChars = rtl_convertTextToUnicode(hCP1252Converter, 0,
            &cChar, 1, &nConvChar, 1, nFlags, &nInfo, &nSrcBytes );
        rtl_destroyTextToUnicodeConverter(hCP1252Converter);
    }

    ASSERT(nDestChars == 1, "impossible to get more than 1 char");

    ASSERT(nInfo == 0, "A character conversion failed, gulp!");

    if (nDestChars == 1)
        return nConvChar;
    else
        return cChar;
}

// Returnwert: true for no Sonderzeichen
bool SwWW8ImplReader::ReadPlainChars(long& rPos, long nEnd, long nCpOfs)
{
    // Unicode-Flag neu setzen und notfalls File-Pos korrigieren
    // merke: Seek kostet nicht viel, da inline geprueft wird,
    //        ob die korrekte FilePos nicht schon erreicht ist.
    WW8_FC nStreamPos = pSBase->WW8Cp2Fc(nCpOfs+rPos, &bIsUnicode);
    pStrm->Seek( nStreamPos );
#if 0
    // amount of characters to read == length to next attribute
    DBG_ASSERT(nEnd - rPos <= (STRING_MAXLEN-1),
        "String too long for stringclass!");
#endif
    xub_StrLen nLen;
    if (nEnd - rPos <= (STRING_MAXLEN-1))
        nLen = sw::types::writer_cast<xub_StrLen>(nEnd - rPos);
    else
        nLen = STRING_MAXLEN-1;
    ASSERT(nLen, "String is 0");
    if (!nLen)
        return true;

    const CharSet eSrcCharSet = bVer67 ? GetCurrentCharSet() :
        RTL_TEXTENCODING_MS_1252;

    // (re)alloc UniString data
    String sPlainCharsBuf;

    sal_Unicode* pWork = sPlainCharsBuf.AllocBuffer( nLen );

    rtl_TextToUnicodeConverter hConverter = 0;
    if (!bIsUnicode || bVer67)
        hConverter = rtl_createTextToUnicodeConverter(eSrcCharSet);

    // read the stream data
    BYTE   nBCode;
    UINT16 nUCode;
    for( xub_StrLen nL2 = 0; nL2 < nLen; ++nL2, ++pWork )
    {
        if (bIsUnicode)
            *pStrm >> nUCode;   // unicode  --> read 2 bytes
        else
        {
            *pStrm >> nBCode;   // old code --> read 1 byte
            nUCode = nBCode;
        }

        if (pStrm->GetError())
        {
            rPos = LONG_MAX-10;     // -> eof or other error
            sPlainCharsBuf.ReleaseBufferAccess( 0 );
            return true;
        }

        if ((32 > nUCode) || (0xa0 == nUCode))
        {
            pStrm->SeekRel( bIsUnicode ? -2 : -1 );
            sPlainCharsBuf.ReleaseBufferAccess( nL2 );
            break;              // Sonderzeichen < 32, == 0xa0 gefunden
        }

        if (bIsUnicode)
        {
            if (!bVer67)
                *pWork = nUCode;
            else
            {
                if (nUCode >= 0x3000)       //0x8000 ?
                {
                    sal_Char aTest[2];
                    aTest[0] = (nUCode & 0xFF00) >> 8;
                    aTest[1] = (nUCode & 0x00FF);
                    String aTemp(aTest, 2, eSrcCharSet);
                    ASSERT(aTemp.Len() == 1, "so much for that theory");
                    *pWork = aTemp.GetChar(0);
                }
                else
                {
                    *pWork = Custom8BitToUnicode(hConverter, nUCode & 0x00FF);
                }
            }
        }
        else
            *pWork = Custom8BitToUnicode(hConverter, nBCode);
    }

    if (nL2)
    {
        AddTextToParagraph(sPlainCharsBuf);
        rPos += nL2;
        if (!maApos.back()) //a para end in apo doesn't count
            bWasParaEnd = false;            //kein CR
    }

    if (hConverter)
        rtl_destroyTextToUnicodeConverter(hConverter);
    return nL2 >= nLen;
}

bool SwWW8ImplReader::AddTextToParagraph(const String& rAddString)
{
    const SwTxtNode* pNd = pPaM->GetCntntNode()->GetTxtNode();
    if (rAddString.Len())
    {
        if ((pNd->GetTxt().Len() + rAddString.Len()) < STRING_MAXLEN -1)
        {
            rDoc.Insert (*pPaM, rAddString);
        }
        else
        {

            if (pNd->GetTxt().Len()< STRING_MAXLEN -1)
            {
                String sTempStr (rAddString,0,
                    STRING_MAXLEN - pNd->GetTxt().Len() -1);
                rDoc.Insert (*pPaM, sTempStr);
                sTempStr = rAddString.Copy(sTempStr.Len(),
                    rAddString.Len() - sTempStr.Len());
                AppendTxtNode(*pPaM->GetPoint());
                rDoc.Insert (*pPaM,sTempStr );
            }
            else
            {
                AppendTxtNode(*pPaM->GetPoint());
                rDoc.Insert (*pPaM, rAddString);
            }
        }
    }
    return true;
}

// Returnwert: true for para end
bool SwWW8ImplReader::ReadChars(long& rPos, long nNextAttr, long nTextEnd,
    long nCpOfs)
{
    long nEnd = ( nNextAttr < nTextEnd ) ? nNextAttr : nTextEnd;

    if (bSymbol || bIgnoreText)
    {
        if( bSymbol )   // Spezialzeichen einfuegen
        {
            for(USHORT nCh = 0; nCh < nEnd - rPos; ++nCh)
                rDoc.Insert( *pPaM, cSymbol );
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
        }
        pStrm->SeekRel( nEnd- rPos );
        rPos = nEnd;    // ignoriere bis Attributende
        return false;
    }

    while (true)
    {
        if (ReadPlainChars(rPos, nEnd, nCpOfs))
             return false;                  // Fertig

        bool bStartLine = ReadChar(rPos, nCpOfs);
        rPos++;
        if (bPgSecBreak || bStartLine || rPos == nEnd)  // CR oder Fertig
            return bStartLine;
    }
}

bool SwWW8ImplReader::HandlePageBreakChar()
{
    bool bParaEndAdded = false;
    //#i1909# section/page breaks should not occur in tables, word
    //itself ignores them in this case.
    if (!nInTable)
    {
        bPgSecBreak = true;
        pCtrlStck->KillUnlockedAttrs(*pPaM->GetPoint());
        /*
        #74468#
        If its a 0x0c without a paragraph end before it, act like a
        paragraph end, but nevertheless, numbering (and perhaps other
        similiar constructs) do not exist on the para.
        */
        if (!bWasParaEnd)
        {
            bParaEndAdded = true;
            if (0 >= pPaM->GetPoint()->nContent.GetIndex())
            {
                if (SwTxtNode* pTxtNode = pPaM->GetNode()->GetTxtNode())
                {
                    pTxtNode->SwCntntNode::SetAttr(
                        *GetDfltAttr(RES_PARATR_NUMRULE));
                    pTxtNode->UpdateNum(SwNodeNum(NO_NUMBERING));
                }
            }
        }
    }
    return bParaEndAdded;
}

bool SwWW8ImplReader::ReadChar(long nPosCp, long nCpOfs)
{
    bool bNewParaEnd = false;
    // Unicode-Flag neu setzen und notfalls File-Pos korrigieren
    // merke: Seek kostet nicht viel, da inline geprueft wird,
    //        ob die korrekte FilePos nicht schon erreicht ist.
    pStrm->Seek( pSBase->WW8Cp2Fc(nCpOfs+nPosCp, &bIsUnicode) );

    BYTE   nBCode;
    UINT16 nWCharVal;
    if( bIsUnicode )
        *pStrm >> nWCharVal;    // unicode  --> read 2 bytes
    else
    {
        *pStrm  >>  nBCode;     // old code --> read 1 byte
        nWCharVal = nBCode;
    }

    sal_Char cInsert = '\x0';
    bool bRet = false;
    switch (nWCharVal)
    {
        case 0:
            {
                // Seitennummer
                SwPageNumberField aFld(
                    (SwPageNumberFieldType*)rDoc.GetSysFldType(
                    RES_PAGENUMBERFLD ), PG_RANDOM, SVX_NUM_ARABIC);
                rDoc.Insert(*pPaM, SwFmtFld(aFld));
            }
            break;
        case 0xe:
            //#108817# if there is only one column word treats a column
            //break like a pagebreak.
            if (maSectionManager.CurrentSectionColCount() < 2)
                bRet = HandlePageBreakChar();
            else if (!nInTable)
            {
                /*
                #i2451#, similarly to i1909 column breaks appear to be ignored
                if they are inside tables. Appears impossible to create one
                from scratch with winword.
                */
                SwTxtNode* pNd = pPaM->GetCntntNode()->GetTxtNode();
                if ( pNd )
                {
                    const SfxItemSet* pSet = pNd->GetpSwAttrSet();
                    if (pSet &&
                        (SFX_ITEM_ON == pSet->GetItemState(RES_PAGEDESC,false)))
                    {
                        AppendTxtNode( *pPaM->GetPoint() );
                    }
                    else
                    {
                        const SwPosition* pPos  = pPaM->GetPoint();
                        const SwTxtNode* pSttNd =
                            rDoc.GetNodes()[ pPos->nNode ]->GetTxtNode();
                        USHORT nCntPos = pPos->nContent.GetIndex();
                        if( nCntPos && pSttNd->GetTxt().Len() )
                            rDoc.SplitNode( *pPos );
                    }
                }
                // column break
                rDoc.Insert( *pPaM, SvxFmtBreakItem(SVX_BREAK_COLUMN_BEFORE) );
            }
            break;
        case 0x7:
            bNewParaEnd = true;
            TabCellEnd();       // table cell end (Flags abfragen!)
            break;
        case 0xf:
            if( !bSpec )        // "Satellit"
                cInsert = '\xa4';
            break;
        case 0x14:
            if( !bSpec )        // "Para-Ende"-Zeichen
                cInsert = '\xb5';
            break;
        case 0x15:
            if( !bSpec )        // Juristenparagraph
                cInsert = '\xa7';
            break;
        case 0x9:
            cInsert = '\x9';    // Tab
            break;
        case 0xb:
            cInsert = '\xa';    // Hard NewLine
            break;
        case 0xc:
            bRet = HandlePageBreakChar();
            break;
        case 0x1e:
            rDoc.Insert( *pPaM, CHAR_HARDHYPHEN);   // Non-breaking hyphen
            break;
        case 0x1f:
            rDoc.Insert( *pPaM, CHAR_SOFTHYPHEN);   // Non-required hyphens
            break;
        case 0xa0:
            rDoc.Insert( *pPaM, CHAR_HARDBLANK);    // Non-breaking spaces
            break;
        case 0x1:
            /*
            Current thinking is that if bObj is set then we have a
            straightforward "traditional" ole object, otherwise we have a
            graphic preview of an associated ole2 object (or a simple
            graphic of course)
            */
            if (!IsInlineEscherHack())
            {
                SwFrmFmt *pResult = 0;
                if (bObj)
                    pResult = ImportOle();
                else if (bSpec)
                    pResult = ImportGraf();

                //#102160# If we have a bad 0x1 insert a space instead.
                if (!pResult)
                {
                    cInsert = ' ';
                    ASSERT(!bObj && !bEmbeddObj && !nObjLocFc,
                        "WW8: Please report this document, it may have a "
                        "missing graphic");
                }
                else
                {
                    // reset the flags.
                    bObj = bEmbeddObj = false;
                    nObjLocFc = 0;
                }
            }
            break;
        case 0x8:
            if( !bObj )
                Read_GrafLayer( nPosCp );
            break;
        case 0xd:
            bNewParaEnd = bRet = true;
            if (nInTable > 1)
            {
                /*
                i9666#
                Yes complex, if there is an entry in the undocumented PLCF
                which I believe to be a record of cell and row marks then seek
                the properties to that location and see if the next cell is
                below or equal to this one, if so end the cell otherwise keep
                it open
                */
                WW8PLCFspecial* pTest = pPlcxMan->GetMagicTables();
                if (pTest && pTest->SeekPosExact(nPosCp+1+nCpOfs) &&
                    pTest->Where() == nPosCp+1+nCpOfs)
                {
                    bool bCellend(false);
                    WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();
                    long nOrigPos = nPosCp;
                    pPap->SeekPos(nPosCp+1);
                    if (const BYTE *pLevel = pPap->HasSprm(0x6649))
                    {
                        if (*pLevel <= nInTable)
                            bCellend = true;
                    }
                    pPap->SeekPos(nOrigPos);
                    if (bCellend)
                    {
                        TabCellEnd();
                        bRet = false;
                    }
                }
            }
            break;              // line end
        case 0x5:               // Annotation reference
        case 0x13:
            break;
        case 0x2:
            if (!maFtnStack.empty())
                cInsert = 0x2;
            break;                  // Auto-Fussnoten-Nummer
#if OSL_DEBUG_LEVEL > 1
        default:
            {
                String sUnknown( '<' );
                sUnknown += String::CreateFromInt32( nWCharVal );
                sUnknown += '>';
                rDoc.Insert( *pPaM, sUnknown );
            }
#endif
            break;
    }

    if( '\x0' != cInsert )
    {
        String sInsert = ByteString::ConvertToUnicode(cInsert,
            RTL_TEXTENCODING_MS_1252 );
        AddTextToParagraph(sInsert);
    }
    if (!maApos.back()) //a para end in apo doesn't count
        bWasParaEnd = bNewParaEnd;
    return bRet;
}


void SwWW8ImplReader::ProcessAktCollChange(WW8PLCFManResult& rRes,
    bool* pStartAttr, bool bCallProcessSpecial)
{
    USHORT nOldColl = nAktColl;
    nAktColl = pPlcxMan->GetColl();

    if( nAktColl >= nColls || !pCollA[nAktColl].pFmt
        || !pCollA[nAktColl].bColl )
    {
        nAktColl = 0;           // Unguelige Style-Id
        nLeftParaMgn      = 0;
        nTxtFirstLineOfst = 0;
    }
    else
    {
        nLeftParaMgn      = pCollA[nAktColl].nLeftParaMgn;
        nTxtFirstLineOfst = pCollA[nAktColl].nTxtFirstLineOfst;
    }
    bool bTabRowEnd = false;
    if( pStartAttr && bCallProcessSpecial && !bInHyperlink )
    {
        bool bReSync;
        // Frame / Table / Autonumbering List Level
        bTabRowEnd = ProcessSpecial(bReSync, rRes.nAktCp+pPlcxMan->GetCpOfs());
        if( bReSync )
            *pStartAttr = pPlcxMan->Get( &rRes ); // hole Attribut-Pos neu
    }

    if (!bTabRowEnd && StyleExists(nAktColl))
    {
        SetTxtFmtCollAndListLevel( *pPaM, pCollA[ nAktColl ]);
        ChkToggleAttr(pCollA[ nOldColl ].n81Flags, pCollA[ nAktColl ].n81Flags);
        ChkToggleBiDiAttr(pCollA[nOldColl].n81BiDiFlags,
            pCollA[nAktColl].n81BiDiFlags);
    }
}

long SwWW8ImplReader::ReadTextAttr(long& rTxtPos, bool& rbStartLine)
{
    long nSkipChars = 0;
    WW8PLCFManResult aRes;

    ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    bool bStartAttr = pPlcxMan->Get(&aRes); // hole Attribut-Pos
    aRes.nAktCp = rTxtPos;              // Akt. Cp-Pos

    if ((aRes.nFlags & MAN_MASK_NEW_SEP) && !bIgnoreText)   // neue Section
    {
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
        // PageDesc erzeugen und fuellen
        maSectionManager.CreateSep(rTxtPos, bPgSecBreak);
                                            // -> 0xc war ein Sectionbreak, aber
                                            // kein Pagebreak;
        bPgSecBreak = false;                // PageDesc erzeugen und fuellen
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    }

    // neuer Absatz ueber Plcx.Fkp.papx
    if ( (aRes.nFlags & MAN_MASK_NEW_PAP)|| rbStartLine )
    {
        ProcessAktCollChange( aRes, &bStartAttr,
            MAN_MASK_NEW_PAP == (aRes.nFlags & MAN_MASK_NEW_PAP) &&
            !bIgnoreText );
        rbStartLine = false;
    }

    // position of last CP that's to be ignored
    long nSkipPos = -1;

    if( 0 < aRes.nSprmId )                      // leere Attrs ignorieren
    {
        if( ( eFTN > aRes.nSprmId ) || ( 0x0800 <= aRes.nSprmId ) )
        {
            if( bStartAttr )                            // WW-Attribute
            {
                if( aRes.nMemLen >= 0 )
                    ImportSprm(aRes.pMemPos, aRes.nSprmId);
            }
            else
                EndSprm( aRes.nSprmId );        // Attr ausschalten
        }
        else if( aRes.nSprmId < 0x800 ) // eigene Hilfs-Attribute
        {
            if( bStartAttr )
            {
                nSkipChars = ImportExtSprm(&aRes);
                if (
                    (aRes.nSprmId == eFTN) || (aRes.nSprmId == eEDN) ||
                    (aRes.nSprmId == eFLD)
                   )
                {
                    // Felder/Ftn-/End-Note hier ueberlesen
                    rTxtPos += nSkipChars;
                    nSkipPos = rTxtPos-1;
                }
            }
            else
                EndExtSprm( aRes.nSprmId );
        }
    }

    pStrm->Seek(pSBase->WW8Cp2Fc( pPlcxMan->GetCpOfs() + rTxtPos, &bIsUnicode));

    // Find next Attr position (and Skip attributes of field contents if needed)
    if (nSkipChars && !bIgnoreText)
        pCtrlStck->MarkAllAttrsOld();
    bool bOldIgnoreText = bIgnoreText;
    bIgnoreText = true;
    USHORT nOldColl = nAktColl;
    bool bDoPlcxManPlusPLus = true;
    long nNext;
    do
    {
        if( bDoPlcxManPlusPLus )
            (*pPlcxMan)++;
        nNext = pPlcxMan->Where();
        if( (0 <= nNext) && (nSkipPos >= nNext) )
        {
            nNext = ReadTextAttr( rTxtPos, rbStartLine );
            bDoPlcxManPlusPLus = false;
            bIgnoreText = true;
        }
    }
    while( nSkipPos >= nNext );
    bIgnoreText    = bOldIgnoreText;
    if( nSkipChars )
    {
        pCtrlStck->KillUnlockedAttrs( *pPaM->GetPoint() );
        if( nOldColl != pPlcxMan->GetColl() )
            ProcessAktCollChange(aRes, 0, false);
    }

    return nNext;
}

void SwWW8ImplReader::ReadAttrs(long& rNext, long& rTxtPos, bool& rbStartLine)
{
    if( rTxtPos >= rNext )
    {           // Stehen Attribute an ?

        do
        {
            rNext = ReadTextAttr( rTxtPos, rbStartLine );
        }while( rTxtPos >= rNext );

    }
    else if ( rbStartLine )
    {
// keine Attribute, aber trotzdem neue Zeile
// wenn eine Zeile mit einem Seitenumbruch aufhoert und sich keine
// Absatzattribute / Absatzvorlagen aendern, ist das Zeilenende
// nicht im Plcx.Fkp.papx eingetragen, d.h. ( nFlags & MAN_MASK_NEW_PAP )
// ist false. Deshalb muss als Sonderbehandlung hier die Vorlage gesetzt
// werden.
        if (!bCpxStyle && nAktColl < nColls)
            SetTxtFmtCollAndListLevel(*pPaM, pCollA[nAktColl]);
        rbStartLine = false;
    }
}

// CloseAttrEnds zum Lesen nur der Attributenden am Ende eines Textes oder
// Textbereiches ( Kopfzeile, Fussnote, ...). Attributanfaenge, Felder
// werden ignoriert.
void SwWW8ImplReader::CloseAttrEnds()
{
    //If there are any unclosed sprms then copy them to
    //another stack and close the ones that must be closed
    std::stack<USHORT> aStack;
    pPlcxMan->TransferOpenSprms(aStack);

    while (!aStack.empty())
    {
        USHORT nSprmId = aStack.top();
        if ((0 < nSprmId) && (( eFTN > nSprmId) || (0x0800 <= nSprmId)))
            EndSprm(nSprmId);
        aStack.pop();
    }

    EndSpecial();
}

bool SwWW8ImplReader::ReadText(long nStartCp, long nTextLen, short nType)
{
    sw::log::Environment eContext = sw::log::eMainText;
    if (nType == MAN_MAINTEXT)
        eContext = sw::log::eMainText;
    else
        eContext = sw::log::eSubDoc;
    sw::log::Context aContext(maTracer, eContext);

    bool bJoined=false;

    bool bStartLine = true;
    short nCrCount = 0;

    bWasParaEnd = false;
    nAktColl    =  0;
    pAktItemSet =  0;
    nCharFmt    = -1;
    bSpec = false;
    bPgSecBreak = false;

    pPlcxMan = new WW8PLCFMan( pSBase, nType, nStartCp );
    long nCpOfs = pPlcxMan->GetCpOfs(); // Offset fuer Header/Footer, Footnote

    WW8_CP nNext = pPlcxMan->Where();

    pStrm->Seek( pSBase->WW8Cp2Fc( nStartCp + nCpOfs, &bIsUnicode ) );

    WW8_CP l = nStartCp;
    while ( l<nStartCp+nTextLen )
    {

        ReadAttrs( nNext, l, bStartLine );// behandelt auch Section-Breaks
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");

        if( l>= nStartCp + nTextLen )
            break;

        bStartLine = ReadChars(l, nNext, nStartCp+nTextLen, nCpOfs);

        if (bStartLine) // Zeilenende
            AppendTxtNode(*pPaM->GetPoint());

        if (bStartLine || bWasTabRowEnd)
        {
            // alle 64 CRs aufrufen not for Header u. ae.
            if ((nCrCount++ & 0x40) == 0 && nType == MAN_MAINTEXT)
            {
                nProgress = (USHORT)( l * 100 / nTextLen );
                ::SetProgressState( nProgress, rDoc.GetDocShell() ); // Update
            }
        }

        // If we have encountered a 0x0c which indicates either section of
        // pagebreak then look it up to see if it is a section break, and
        // if it is not then insert a page break. If it is a section break
        // it will be handled as such in the ReadAttrs of the next loop
        if (bPgSecBreak)
        {
            // We need only to see if a section is ending at this cp,
            // the plcf will already be sitting on the correct location
            // if it is there.
            WW8PLCFxDesc aTemp;
            aTemp.nStartPos = aTemp.nEndPos = LONG_MAX;
            if (pPlcxMan->GetSepPLCF())
                pPlcxMan->GetSepPLCF()->GetSprms(&aTemp);
            if ((aTemp.nStartPos != l) && (aTemp.nEndPos != l))
            {
//              AppendTxtNode(*pPaM->GetPoint());
                rDoc.Insert(*pPaM, SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE));
                bPgSecBreak = false;
            }
        }
    }

    if (pPaM->GetPoint()->nContent.GetIndex())
        AppendTxtNode(*pPaM->GetPoint());

    CloseAttrEnds();
    if (!bInHyperlink)
        bJoined = JoinNode(*pPaM);
                                            // PlcxMan noch gibt
    delete pPlcxMan, pPlcxMan = 0;
    return bJoined;
}

/***************************************************************************
#           class SwWW8ImplReader
#**************************************************************************/

SwWW8ImplReader::SwWW8ImplReader(BYTE nVersionPara, SvStorage* pStorage,
    SvStream* pSt, SwDoc& rD, bool bNewDoc)
    : maTracer(*(rD.GetDocShell()->GetMedium())), pStg(pStorage), pStrm(pSt),
    pTableStream(0), pDataStream(0), rDoc(rD), maSectionManager(*this),
    maSectionNameGenerator(rD,CREATE_CONST_ASC("WW")),
    maGrfNameGenerator(bNewDoc,String('G')), pMSDffManager(0), mpAtnNames(0),
    pAuthorInfos(0), pTabNode(0), mbNewDoc(bNewDoc)
{
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    nWantedVersion = nVersionPara;
    pCtrlStck   = 0;
    mpRedlineStack = 0;
    pRefStck = 0;
    pAnchorStck = 0;
    pFonts = 0;
    pSBase = 0;
    pPlcxMan = 0;
    pStyles = 0;
    pAktColl = 0;
    pLstManager = 0;
    pAktItemSet = 0;
    pCollA = 0;
    pHdFt = 0;
    pWFlyPara = 0;
    pSFlyPara = 0;
    pFlyFmtOfJustInsertedGraphic   = 0;
    pFmtOfJustInsertedApo = 0;
    nColls = nAktColl = 0;
    nObjLocFc = nPicLocFc = 0;
    nInTable=0;
    bReadNoTbl = bPgSecBreak = bSpec = bObj = bTxbxFlySection
               = bHasBorder = bSymbol = bIgnoreText
               = bWasTabRowEnd = false;
    bShdTxtCol = bCharShdTxtCol = bAnl = bHdFtFtnEdn = bFtnEdn
               = bIsHeader = bIsFooter = bIsUnicode = bCpxStyle = bStyNormal =
                 bWWBugNormal  = false;

    bNoAttrImport = bPgChpLevel = bEmbeddObj = false;
    bAktAND_fNumberAcross = false;
    bNoLnNumYet = true;
    bInHyperlink = false;
    bWasParaEnd = false;
    nProgress = 0;
    nSwNumLevel = nWwNumType = 0xff;
    pTableDesc = 0;
    mpNumRule = 0;
    pNumOlst = 0;
    pNode_FLY_AT_CNTNT = 0;
    pDrawModel = 0;
    pDrawPg = 0;
    nDrawTxbx = 0;
    pDrawEditEngine = 0;
    pWWZOrder = 0;
    pFormImpl = 0;
    nLeftParaMgn = 0;
    nTxtFirstLineOfst = 0;
    pNumFldType = 0;
    nFldNum = 0;

    nLFOPosition = USHRT_MAX;
    nListLevel = WW8ListManager::nMaxLevel;
    eHardCharSet = RTL_TEXTENCODING_DONTKNOW;

    nPgChpDelim = nPgChpLevel = 0;

    maApos.push_back(false);
}

void SwWW8ImplReader::DeleteStk(SwFltControlStack* pStck)
{
    if( pStck )
    {
        pStck->SetAttr( *pPaM->GetPoint(), 0, false);
        pStck->SetAttr( *pPaM->GetPoint(), 0, false);
        delete pStck;
    }
    else
    {
        ASSERT( !this, "WW-Stack bereits geloescht" );
    }
}

void wwSectionManager::SetSegmentToPageDesc(const wwSection &rSection,
    bool bTitlePage, bool bIgnoreCols)
{
    SwPageDesc &rPage = bTitlePage ? *rSection.mpTitlePage : *rSection.mpPage;

    SetNumberingType(rSection, rPage);

    SwFrmFmt &rFmt = rPage.GetMaster();

    wwULSpaceData aULData;
    GetPageULData(rSection, bTitlePage, aULData);
    SetPageULSpaceItems(rFmt, aULData);

    SetPage(rPage, rFmt, rSection, bIgnoreCols);

    mrReader.SetDocumentGrid(rFmt, rSection);
}

void wwSectionManager::SetUseOn(wwSection &rSection)
{
    bool bEven = (rSection.maSep.grpfIhdt & (WW8_HEADER_EVEN|WW8_FOOTER_EVEN)) ?
        true : false;

    bool bMirror = mrReader.pWDop->fMirrorMargins ||
        mrReader.pWDop->doptypography.f2on1;

    UseOnPage eUseBase = bMirror ? PD_MIRROR : PD_ALL;
    UseOnPage eUse = eUseBase;
    if (!bEven)
        eUse = (UseOnPage)(eUse | PD_HEADERSHARE | PD_FOOTERSHARE);

    ASSERT(rSection.mpPage, "Makes no sense to call me with no pages to set");
    if (rSection.mpPage)
        rSection.mpPage->WriteUseOn(eUse);
    if (rSection.mpTitlePage)
    {
        rSection.mpTitlePage->WriteUseOn(
            (UseOnPage) (eUseBase | PD_HEADERSHARE | PD_FOOTERSHARE));
    }
}

void wwSectionManager::InsertSegments(bool bIsNewDoc)
{
    sal_uInt16 nDesc(0);
    mySegIter aEnd = maSegments.end();
    mySegIter aStart = maSegments.begin();
    for (mySegIter aIter = aStart; aIter != aEnd; ++aIter)
    {
        mySegIter aNext = aIter+1;

        bool bInsertSection = aIter != aStart ? aIter->IsContinous() : false;

        if (!bInsertSection)
        {
            /*
             If a cont section follow this section then we won't be
             creating a page desc with 2+ cols as we cannot host a one
             col section in a 2+ col pagedesc and make it look like
             word. But if the current section actually has columns then
             we are forced to insert a section here as well as a page
             descriptor.
            */

            /*
             Note for the future:
             If we want to import "protected sections" the here is
             where we would also test for that and force a section
             insertion if that was true.
            */
            bool bIgnoreCols = false;
            if (aNext != aEnd && aNext->IsContinous())
            {
                bIgnoreCols = true;
                if (aIter->NoCols() > 1)
                    bInsertSection = true;
            }

            if (aIter->HasTitlePage())
            {
                if (bIsNewDoc && aIter == aStart)
                {
                    aIter->mpTitlePage =
                        mrReader.rDoc.GetPageDescFromPool(RES_POOLPAGE_FIRST);
                }
                else
                {
                    USHORT nPos = mrReader.rDoc.MakePageDesc(
                        ViewShell::GetShellRes()->GetPageDescName(nDesc)
                        , 0, false);
                    aIter->mpTitlePage = &mrReader.rDoc._GetPageDesc(nPos);
                }
                ASSERT(aIter->mpTitlePage, "no page!");
                if (!aIter->mpTitlePage)
                    continue;

                SetSegmentToPageDesc(*aIter, true, bIgnoreCols);
            }

            if (bIsNewDoc && aIter == aStart)
            {
                aIter->mpPage =
                    mrReader.rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD);
            }
            else
            {
                USHORT nPos = mrReader.rDoc.MakePageDesc(
                    ViewShell::GetShellRes()->GetPageDescName(nDesc,
                        false, aIter->HasTitlePage()),
                        aIter->mpTitlePage, false);
                aIter->mpPage = &mrReader.rDoc._GetPageDesc(nPos);
            }
            ASSERT(aIter->mpPage, "no page!");
            if (!aIter->mpPage)
                continue;

            const wwSection *pPrevious = 0;
            if (aIter != aStart)
                pPrevious = &(*(aIter-1));
            SetHdFt(*aIter, std::distance(aStart, aIter), pPrevious);
            SetUseOn(*aIter);

            if (aIter->mpTitlePage)
                SetSegmentToPageDesc(*aIter, true, bIgnoreCols);
            SetSegmentToPageDesc(*aIter, false, bIgnoreCols);

            SwFmtPageDesc aPgDesc(aIter->HasTitlePage() ?
                    aIter->mpTitlePage : aIter->mpPage);

            if (aIter->mpTitlePage)
                aIter->mpTitlePage->SetFollow(aIter->mpPage);

            if (aIter->PageRestartNo())
                aPgDesc.SetNumOffset(aIter->PageStartAt());

            /*
            If its a table here, apply the pagebreak to the table
            properties, otherwise we add it to the para at this
            position
            */
            if (aIter->maStart.GetNode().IsTableNode())
            {
                SwTable& rTable =
                    aIter->maStart.GetNode().GetTableNode()->GetTable();
                SwFrmFmt* pApply = rTable.GetFrmFmt();
                ASSERT(pApply, "impossible");
                if (pApply)
                    pApply->SetAttr(aPgDesc);
            }
            else
            {
                SwPosition aPamStart(aIter->maStart);
                aPamStart.nContent.Assign(
                    aIter->maStart.GetNode().GetCntntNode(), 0);
                SwPaM aPage(aPamStart);

                mrReader.rDoc.Insert(aPage, aPgDesc);
            }
            ++nDesc;
        }

        SwTxtNode* pTxtNd = 0;
        if (bInsertSection)
        {
            SwPaM aSectPaM(*mrReader.pPaM);
            SwNodeIndex aAnchor(aSectPaM.GetPoint()->nNode);
            if (aNext != aEnd)
            {
                aAnchor = aNext->maStart;
                aSectPaM.GetPoint()->nNode = aAnchor;
                aSectPaM.GetPoint()->nContent.Assign(
                    aNext->maStart.GetNode().GetCntntNode(), 0);
                aSectPaM.Move(fnMoveBackward);
            }

            const SwPosition* pPos  = aSectPaM.GetPoint();
            const SwTxtNode* pSttNd =
                mrReader.rDoc.GetNodes()[ pPos->nNode ]->GetTxtNode();
            const SwTableNode* pTableNd = pSttNd ? pSttNd->FindTableNode() : 0;
            if (pTableNd)
            {
                pTxtNd =
                    mrReader.rDoc.GetNodes().MakeTxtNode(aAnchor,
                    mrReader.rDoc.GetTxtCollFromPool( RES_POOLCOLL_TEXT ));

                aSectPaM.GetPoint()->nNode = SwNodeIndex(*pTxtNd);
                aSectPaM.GetPoint()->nContent.Assign(
                    aSectPaM.GetCntntNode(), 0);
            }

            aSectPaM.SetMark();

            aSectPaM.GetPoint()->nNode = aIter->maStart;
            aSectPaM.GetPoint()->nContent.Assign(
                aSectPaM.GetCntntNode(), 0);

            SwSectionFmt *pRet = InsertSection(aSectPaM, *aIter);
            //The last section if continous is always unbalanced
            if (aNext == aEnd && pRet)
                pRet->SetAttr(SwFmtNoBalancedColumns(true));
        }

        if (pTxtNd)
        {
            SwNodeIndex aIdx(*pTxtNd);
            SwPosition aPos(aIdx);
            SwPaM aTest(aPos);
            mrReader.rDoc.DelFullPara(aTest);
            pTxtNd = 0;
        }
    }
}

void SwWW8ImplReader::StoreMacroCmds()
{
    if (pWwFib->lcbCmds)
    {
        pTableStream->Seek(pWwFib->fcCmds);

        SvStorageRef xDestRoot(rDoc.GetDocShell()->GetStorage());
        SvStorageStreamRef xDestStream =
            xDestRoot->OpenStream(CREATE_CONST_ASC(SL::aMSMacroCmds));

        sal_uInt8 *pBuffer = new sal_uInt8[pWwFib->lcbCmds];
        pTableStream->Read(pBuffer, pWwFib->lcbCmds);
        xDestStream->Write(pBuffer, pWwFib->lcbCmds);
        delete[] pBuffer;
    }
}

ULONG SwWW8ImplReader::LoadDoc1( SwPaM& rPaM ,WW8Glossary *pGloss)
{
    ULONG nErrRet = 0;

    if (mbNewDoc && pStg && !pGloss)
        ReadDocInfo();

    pPaM = new SwPaM( *rPaM.GetPoint() );

    pCtrlStck = new SwWW8FltControlStack( &rDoc, nFieldFlags, *this );

    mpRedlineStack = new wwRedlineStack(rDoc);

    /*
        RefFldStck: Keeps track of bookmarks which may be inserted as
        variables intstead.
    */
    pRefStck = new SwWW8FltRefStack(&rDoc, nFieldFlags);

    pAnchorStck = new SwWW8FltAnchorStack(&rDoc, nFieldFlags);

    sal_uInt16 nPageDescOffset = rDoc.GetPageDescCnt();

    SwNodeIndex aSttNdIdx( rDoc.GetNodes() );
    SwRelNumRuleSpaces aRelNumRule(rDoc, mbNewDoc);

    USHORT eMode = REDLINE_SHOW_INSERT;

    // enum-para mitgeben, welche Versions-Nummern erlaubt sind: ww6, ww8, ...

    if (pGloss)
        pWwFib = pGloss->GetFib();
    else
        pWwFib = new WW8Fib( *pStrm, nWantedVersion );  // Fib einlesen

    if ( pWwFib->nFibError )
    {           // ERR_NO_WW8_FILE
        nErrRet = ERR_SWG_READ_ERROR;               // oder ERR_SW6_READ_ERROR
    }
    else if( pWwFib->fEncrypted )
    {
        nErrRet = ERR_SW6_PASSWD;                   // gecryptet geht nicht



        // Annotation:  Crypted files  C O U L D  be imported if we used the methods
        //              provided by Caolan Mc Nammara.
        //              Written permission (my e-mail) was given by him
        //              but no special document has been signed...
        //              27. Sept. 1999, K.-H. Zimmer


    }
    else
    {
        mpSprmParser = new wwSprmParser(pWwFib->nVersion);

        // praktische Hilfsvariablen besetzen:
        bVer67 = (     (6 == pWwFib->nVersion)
                                || (7 == pWwFib->nVersion) );   // z.B.: altes Sprm-Id-Format!

        bVer6  =  (6 == pWwFib->nVersion);
        bVer7  =  (7 == pWwFib->nVersion);
        bVer8  =  (8 == pWwFib->nVersion);

        SvStorageStreamRef xTableStream, xDataStream;

        // Nachdem wir nun den FIB eingelesen haben, wissen wir ja,
        // welcher Table-Stream gueltig ist.
        // Diesen oeffnen wir nun.

        // 6 stands for "6 OR 7",  7 stand for "ONLY 7"
        switch( pWwFib->nVersion )
        {
        case 6:
        case 7: pTableStream = pStrm;
                pDataStream = pStrm;
                break;

        case 8:
                {
            if( !pStg )
            {
                ASSERT( pStg, "Version 8 muss immer einen Storage haben!" );
                nErrRet = ERR_SWG_READ_ERROR;
                break;
            }

            xTableStream = pStg->OpenStream( String::CreateFromAscii(
                pWwFib->fWhichTblStm ? SL::a1Table : SL::a0Table),
                STREAM_STD_READ );

            pTableStream = &xTableStream;
            pTableStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

            xDataStream = pStg->OpenStream(CREATE_CONST_ASC(SL::aData),
                STREAM_STD_READ | STREAM_NOCREATE );

            if( xDataStream.Is() && SVSTREAM_OK == xDataStream->GetError() )
            {
                pDataStream = &xDataStream;
                pDataStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            }
            else
                pDataStream = pStrm;

            if( pWwFib->lcbPlcfspaHdr || pWwFib->lcbPlcfspaMom )
            {
                pMSDffManager = new SwMSDffManager( *this );
                //#79055# Now the dff manager always needs a controls
                //converter as well, but a control converter may still exist
                //without a dffmanager. cmc
                pFormImpl = new SwMSConvertControls(rDoc.GetDocShell(), pPaM);
            }
                }
            break;
        default:
            // Programm-Fehler!
            ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            nErrRet = ERR_SWG_READ_ERROR;
            break;
        }

        if( ERR_SWG_READ_ERROR != nErrRet )
        {
            eTextCharSet = WW8Fib::GetFIBCharset(pWwFib->chse);
            eStructCharSet = WW8Fib::GetFIBCharset(pWwFib->chseTables);

            bWWBugNormal = pWwFib->nProduct == 0xc03d;

            if (!mbNewDoc)
                aSttNdIdx = pPaM->GetPoint()->nNode;

            ::StartProgress( STR_STATSTR_W4WREAD, 0, 100, rDoc.GetDocShell() );

            // read Font Table
            pFonts = new WW8Fonts( *pTableStream, *pWwFib );

            // Document Properties
            pWDop = new WW8Dop( *pTableStream, pWwFib->nFib, pWwFib->fcDop,
                pWwFib->lcbDop );

            if (mbNewDoc)
                ImportDop();

            /*
                Import revisioning data: author names
            */
            if( pWwFib->lcbSttbfRMark )
            {
                ReadRevMarkAuthorStrTabl( *pTableStream,
                                            pWwFib->fcSttbfRMark,
                                            pWwFib->lcbSttbfRMark, rDoc );
            }

            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update
            pLstManager = new WW8ListManager( *pTableStream, *this );

            /*
                zuerst(!) alle Styles importieren   (siehe WW8PAR2.CXX)
                    VOR dem Import der Listen !!
            */
            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update
            pStyles = new WW8RStyle( *pWwFib, this );   // Styles
            pStyles->Import();

            /*
                zu guter Letzt: (siehe ebenfalls WW8PAR3.CXX)
                ===============
                alle Styles durchgehen und ggfs. zugehoeriges Listen-Format
                anhaengen NACH dem Import der Styles und NACH dem Import der
                Listen !!
            */
            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update
            pStyles->PostProcessStyles();

            if (pCollA)
                SetOutLineStyles();

            pSBase = new WW8ScannerBase(pStrm,pTableStream,pDataStream,pWwFib);

            static const SvxExtNumType eNumTA[16] =
            {
                SVX_NUM_ARABIC, SVX_NUM_ROMAN_UPPER, SVX_NUM_ROMAN_LOWER,
                SVX_NUM_CHARS_UPPER_LETTER_N, SVX_NUM_CHARS_LOWER_LETTER_N,
                SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
                SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
                SVX_NUM_ARABIC, SVX_NUM_ARABIC, SVX_NUM_ARABIC,
                SVX_NUM_ARABIC, SVX_NUM_ARABIC
            };

            if (pSBase->AreThereFootnotes())
            {
                static const SwFtnNum eNumA[4] =
                {
                    FTNNUM_DOC, FTNNUM_CHAPTER, FTNNUM_PAGE, FTNNUM_DOC
                };

                SwFtnInfo aInfo;
                aInfo = rDoc.GetFtnInfo();      // Copy-Ctor privat

                aInfo.ePos = FTNPOS_PAGE;
                aInfo.eNum = eNumA[pWDop->rncFtn];
                aInfo.aFmt.SetNumberingType(eNumTA[pWDop->nfcFtnRef]);
                if( pWDop->nFtn )
                    aInfo.nFtnOffset = pWDop->nFtn - 1;
                rDoc.SetFtnInfo( aInfo );
            }
            if( pSBase->AreThereEndnotes() )
            {
                SwEndNoteInfo aInfo;
                aInfo = rDoc.GetEndNoteInfo();  // parallel zu Ftn

                // Ich kann nicht setzen, wann neu nummerieren...
                //  aInfo.eNum = eNumA[pWDop->pDop->rncEdn];
                aInfo.aFmt.SetNumberingType(eNumTA[pWDop->nfcEdnRef]);
                if( pWDop->nEdn )
                    aInfo.nFtnOffset = pWDop->nEdn - 1;
                rDoc.SetEndNoteInfo( aInfo );
            }

            if( pWwFib->lcbPlcfhdd )
                pHdFt = new WW8PLCF_HdFt( pTableStream, *pWwFib, *pWDop );

            if (!mbNewDoc)
            {
                // in ein Dokument einfuegen ?
                // Da immer ganze Zeile eingelesen werden, muessen
                // evtl. Zeilen eingefuegt / aufgebrochen werden
                //
                const SwPosition* pPos = pPaM->GetPoint();
                const SwTxtNode* pSttNd =
                    rDoc.GetNodes()[ pPos->nNode ]->GetTxtNode();

                USHORT nCntPos = pPos->nContent.GetIndex();

                // EinfuegePos nicht in leerer Zeile
                if( nCntPos && pSttNd->GetTxt().Len() )
                    rDoc.SplitNode( *pPos );            // neue Zeile erzeugen

                if( pSttNd->GetTxt().Len() )
                {   // EinfuegePos nicht am Ende der Zeile
                    rDoc.SplitNode( *pPos );    // neue Zeile
                    pPaM->Move( fnMoveBackward );   // gehe in leere Zeile
                }

                // verhinder das Einlesen von Tabellen in Fussnoten / Tabellen
                ULONG nNd = pPos->nNode.GetIndex();
                bReadNoTbl = 0 != pSttNd->FindTableNode() ||
                    ( nNd < rDoc.GetNodes().GetEndOfInserts().GetIndex() &&
                    rDoc.GetNodes().GetEndOfInserts().StartOfSectionIndex()
                    < nNd );

            }

            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update

            // loop for each glossary entry and add dummy section node
            if (pGloss)
            {
                WW8PLCF aPlc(&xTableStream,pWwFib->fcPlcfglsy,
                                pWwFib->lcbPlcfglsy,0 );

                WW8_CP nStart, nEnd;
                void* pDummy;

                for (int i=0;i<pGloss->GetNoStrings();i++,aPlc++)
                {
                    SwNodeIndex aIdx( rDoc.GetNodes().GetEndOfContent());
                    SwTxtFmtColl* pColl =
                        rDoc.GetTxtCollFromPoolSimple(RES_POOLCOLL_STANDARD,
                        false);
                    SwStartNode *pNode =
                        rDoc.GetNodes().MakeTextSection(aIdx,
                        SwNormalStartNode,pColl);
                    pPaM->GetPoint()->nNode = pNode->GetIndex()+1;
                    pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(),0);
                    aPlc.Get( nStart, nEnd, pDummy );
                    ReadText(nStart,nEnd-nStart-1,MAN_MAINTEXT);
                }
            }
            else //ordinary case
            {
                ReadText(0, pWwFib->ccpText, MAN_MAINTEXT);
            }

            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update

            if (pDrawPg && pMSDffManager && pMSDffManager->GetShapeOrders())
            {
                // Hilfsarray zum Verketten der (statt SdrTxtObj) eingefuegten
                // Rahmen
                SvxMSDffShapeTxBxSort aTxBxSort;

                // korrekte Z-Order der eingelesen Escher-Objekte sicherstellen
                USHORT nShapeCount = pMSDffManager->GetShapeOrders()->Count();

                for (USHORT nShapeNum=0; nShapeNum < nShapeCount; nShapeNum++)
                {
                    SvxMSDffShapeOrder *pOrder =
                        pMSDffManager->GetShapeOrders()->GetObject(nShapeNum);
                    // Pointer in neues Sort-Array einfuegen
                    if (pOrder->nTxBxComp && pOrder->pFly)
                        aTxBxSort.Insert(pOrder);
                }
                // zu verkettende Rahmen jetzt verketten
                USHORT nTxBxCount = aTxBxSort.Count();
                if( nTxBxCount )
                {
                    SwFmtChain aChain;
                    for (USHORT nTxBxNum=0; nTxBxNum < nTxBxCount; nTxBxNum++)
                    {
                        SvxMSDffShapeOrder *pOrder =
                            aTxBxSort.GetObject(nTxBxNum);

                        // Fly-Frame-Formate initialisieren
                        SwFlyFrmFmt* pFlyFmt     = pOrder->pFly;
                        SwFlyFrmFmt* pNextFlyFmt = 0;
                        SwFlyFrmFmt* pPrevFlyFmt = 0;
                        // ggfs. Nachfolger ermitteln
                        if( 1+nTxBxNum < nTxBxCount )
                        {
                            SvxMSDffShapeOrder *pNextOrder =
                                aTxBxSort.GetObject(nTxBxNum+1);
                            if ((0xFFFF0000 & pOrder->nTxBxComp)
                                   == (0xFFFF0000 & pNextOrder->nTxBxComp))
                                pNextFlyFmt = pNextOrder->pFly;
                        }
                        // ggfs. Vorgaenger ermitteln
                        if( nTxBxNum )
                        {
                            SvxMSDffShapeOrder *pPrevOrder =
                                aTxBxSort.GetObject(nTxBxNum-1);
                            if ((0xFFFF0000 & pOrder->nTxBxComp)
                                   == (0xFFFF0000 & pPrevOrder->nTxBxComp))
                                pPrevFlyFmt = pPrevOrder->pFly;
                        }
                        // Falls Nachfolger oder Vorgaenger vorhanden,
                        // die Verkettung am Fly-Frame-Format eintragen
                        if (pNextFlyFmt || pPrevFlyFmt)
                        {
                            aChain.SetNext( pNextFlyFmt );
                            aChain.SetPrev( pPrevFlyFmt );
                            pFlyFmt->SetAttr( aChain );
                        }
                    }

                }

            }

            if (mbNewDoc)
            {
                if( pWDop->fRevMarking )
                    eMode |= REDLINE_ON;
                if( pWDop->fRMView )
                    eMode |= REDLINE_SHOW_DELETE;
                if (pStg && !pGloss) /*meaningless for a glossary, cmc*/
                {
                    const OfaFilterOptions* pVBAFlags =
                        OFF_APP()->GetFilterOptions();
                    SvxImportMSVBasic aVBasic(*rDoc.GetDocShell(),*pStg,
                                    pVBAFlags->IsLoadWordBasicCode(),
                                    pVBAFlags->IsLoadWordBasicStorage() );
                    String s1(CREATE_CONST_ASC("Macros"));
                    String s2(CREATE_CONST_ASC("VBA"));
                    int nRet = aVBasic.Import( s1, s2 );
                    if( 2 & nRet )
                        rDoc.SetContainsMSVBasic(true);

                    StoreMacroCmds();
                  }
            }

            maSectionManager.InsertSegments(mbNewDoc);

            if (pCollA)
                delete[] pCollA;

            DELETEZ( pStyles );

            if( pFormImpl )
                DeleteFormImpl();
            GrafikDtor();
            DELETEZ( pMSDffManager );
            DELETEZ( pHdFt );
            DELETEZ( pLstManager );
            DELETEZ( pSBase );
            delete pWDop;
            DELETEZ( pFonts );
            delete mpAtnNames;
            DELETEZ( pAuthorInfos );
            DELETEZ( pTabNode );
            delete mpSprmParser;
            ::EndProgress( rDoc.GetDocShell() );
        }
        pDataStream = 0;
        pTableStream = 0;
    }

    if (!pGloss)
        DELETEZ(pWwFib);
    DeleteCtrlStk();
    mpRedlineStack->closeall(*pPaM->GetPoint());
    delete mpRedlineStack;
    DeleteAnchorStk();
    DeleteRefStk();

    aRelNumRule.SetNumRelSpaces(rDoc);
    if (!mbNewDoc && !nErrRet && aSttNdIdx.GetIndex())
    {
        aSttNdIdx++;
        aRelNumRule.SetOultineRelSpaces( aSttNdIdx,
                                    pPaM->GetPoint()->nNode );
    }

    UpdateFields();

    // delete the pam before the call for hide all redlines (Bug 73683)
    if (mbNewDoc)
        rDoc.SetRedlineMode( eMode );

    UpdatePageDescs(rDoc, nPageDescOffset);

    delete pPaM, pPaM = 0;
    return nErrRet;
}

class outlineeq : public std::unary_function<const SwTxtFmtColl*, bool>
{
private:
    BYTE mnNum;
public:
    outlineeq(BYTE nNum) : mnNum(nNum) {}
    bool operator()(const SwTxtFmtColl *pTest) const
    {
        return pTest->GetOutlineLevel() == mnNum;
    }
};

void SwWW8ImplReader::SetOutLineStyles()
{
    /*
    #i3674# & #101291# Load new document and insert document cases.
    */
    SwNumRule aOutlineRule(*rDoc.GetOutlineNumRule());

    sw::ParaStyles aOutLined(sw::util::GetParaStyles(rDoc));
    sw::util::SortByOutline(aOutLined);

    typedef sw::ParaStyleIter myiter;
    /*
    If we are inserted into a document then don't clobber existing existing
    levels.
    */
    USHORT nFlagsStyleOutlLevel = 0;
    if (!mbNewDoc)
    {
        myiter aEnd = aOutLined.end();
        for (myiter aIter = aOutLined.begin(); aIter < aEnd; ++aIter )
        {
            if ((*aIter)->GetOutlineLevel() < MAXLEVEL)
                nFlagsStyleOutlLevel |= 1 << (*aIter)->GetOutlineLevel();
            else
                break;
        }
    }

    USHORT nOldFlags = nFlagsStyleOutlLevel;

    for (USHORT nI = 0; nI < nColls; ++nI)
    {
        SwWW8StyInf& rSI = pCollA[nI];

        if (rSI.IsOutline())
        {
            USHORT nAktFlags = 1 << rSI.nOutlineLevel;
            if (nAktFlags & nFlagsStyleOutlLevel)
            {
                /*
                If our spot is already taken by something we can't replace
                then don't insert and remove our outline level.
                */
                rSI.pFmt->SetAttr(
                        SwNumRuleItem( rSI.pOutlineNumrule->GetName() ) );
                ((SwTxtFmtColl*)rSI.pFmt)->SetOutlineLevel(NO_NUMBERING);
            }
            else
            {
                /*
                If there is a style already set for this outline
                numbering level and its not a style set by us already
                then we can remove it outline numbering.
                (its one of the default headings in a new document
                so we can clobber it)
                Of course if we are being inserted into a document that
                already has some set we can't do this, thats covered by
                the list of level in nFlagsStyleOutlLevel to ignore.
                */
                outlineeq aCmp(rSI.nOutlineLevel);
                myiter aResult = std::find_if(aOutLined.begin(),
                    aOutLined.end(), aCmp);

                myiter aEnd = aOutLined.end();
                while (aResult != aEnd  && aCmp(*aResult))
                {
                    (*aResult)->SetOutlineLevel(NO_NUMBERING);
                    ++aResult;
                }

                /*
                #i1886#
                I believe that when a list is registered onto a winword
                style which is an outline numbering style (i.e.
                nOutlineLevel is set) that the style of numbering is for
                the level is indexed by the *list* level that was
                registered on that style, and not the outlinenumbering
                level, which is probably a logical sequencing, and not a
                physical mapping into the list style reged on that outline
                style.
                */
                BYTE nFromLevel = rSI.nListLevel;
                BYTE nToLevel = rSI.nOutlineLevel;
                const SwNumFmt& rRule=rSI.pOutlineNumrule->Get(nFromLevel);
                aOutlineRule.Set(nToLevel, rRule);
                // Set my outline level
                ((SwTxtFmtColl*)rSI.pFmt)->SetOutlineLevel( nToLevel );
                // If there are more styles on this level ignore them
                nFlagsStyleOutlLevel |= nAktFlags;
            }
        }
    }
    if (nOldFlags != nFlagsStyleOutlLevel)
        rDoc.SetOutlineNumRule(aOutlineRule);
}

const String* SwWW8ImplReader::GetAnnotationAuthor(sal_uInt16 nIdx)
{
    if (!mpAtnNames && pWwFib->lcbGrpStAtnOwners)
    {
        // Authoren bestimmen: steht im TableStream
        mpAtnNames = new ::std::vector<String>;
        SvStream& rStrm = *pTableStream;

        long nOldPos = rStrm.Tell();
        rStrm.Seek( pWwFib->fcGrpStAtnOwners );

        long nRead = 0, nCount = pWwFib->lcbGrpStAtnOwners;
        while (nRead < nCount)
        {
            if( bVer67 )
            {
                mpAtnNames->push_back(WW8ReadPString(rStrm, false));
                nRead += mpAtnNames->rbegin()->Len() + 1;   // Laenge + BYTE Count
            }
            else
            {
                mpAtnNames->push_back(WW8Read_xstz(rStrm, 0, false));
                // UNICode: doppelte Laenge + USHORT Count
                nRead += mpAtnNames->rbegin()->Len() * 2 + 2;
            }
        }
        rStrm.Seek( nOldPos );
    }

    const String *pRet = 0;
    if (mpAtnNames && nIdx < mpAtnNames->size())
        pRet = &((*mpAtnNames)[nIdx]);
    return pRet;
}

ULONG SwWW8ImplReader::LoadDoc( SwPaM& rPaM,WW8Glossary *pGloss)
{
    ULONG nErrRet = 0;

    pAktColl    = 0;
    pAktItemSet = 0;

    {
        static const sal_Char* aNames[ 12 ] = {
            "WinWord/WW", "WinWord/WW8", "WinWord/WWFT",
            "WinWord/WWFLX", "WinWord/WWFLY",
            "WinWord/WWF",
            "WinWord/WWFA0", "WinWord/WWFA1", "WinWord/WWFA2",
            "WinWord/WWFB0", "WinWord/WWFB1", "WinWord/WWFB2"
        };
        sal_uInt32 aVal[ 12 ];
        SwFilterOptions aOpt( 12, aNames, aVal );

        nIniFlags = aVal[ 0 ];
        nIniFlags1= aVal[ 1 ];
        // schiebt Flys um x twips nach rechts o. links
        nIniFlyDx = aVal[ 3 ];
        nIniFlyDy = aVal[ 4 ];

        nFieldFlags = aVal[ 5 ];
        if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO ) )
        {
            SwFltSetFlag( nFieldFlags, SwFltControlStack::BOOK_TO_VAR_REF );
            SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_DO_ID );
            SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_IN_TEXT );
            SwFltSetFlag( nFieldFlags, SwFltControlStack::ALLOW_FLD_CR );
            for( USHORT i = 0; i < 3; i++ )
                nFieldTagAlways[i] = 0;         // Hypo-Default: bekannte Felder
                                                // nicht taggen
            nFieldTagBad[0] = 0xffffffff;       // unbekannte Felder taggen
            nFieldTagBad[1] = 0xffffffff;       //
            nFieldTagBad[2] = 0xffffffef;       // "EinfuegenText" nicht taggen
        }
        else
        {
            nFieldTagAlways[0] = aVal[ 6 ];
            nFieldTagAlways[1] = aVal[ 7 ];
            nFieldTagAlways[2] = aVal[ 8 ];
            nFieldTagBad[0] = aVal[ 9 ];
            nFieldTagBad[1] = aVal[ 10 ];
            nFieldTagBad[2] = aVal[ 11 ];
        }
    }

    UINT16 nMagic;
    *pStrm >> nMagic;

    // beachte: 6 steht fuer "6 ODER 7",  7 steht fuer "NUR 7"
    switch (nWantedVersion)
    {
        case 6:
        case 7:
            if ((0xa5dc != nMagic) && ((nMagic < 0xa697 ) || (nMagic > 0xa699)))
            {
                //JP 06.05.99: teste auf eigenen 97-Fake!
                if (pStg && 0xa5ec == nMagic)
                {
                    ULONG nCurPos = pStrm->Tell();
                    if (pStrm->Seek(nCurPos + 22))
                    {
                        UINT32 nfcMin;
                        *pStrm >> nfcMin;
                        if (0x300 != nfcMin)
                            nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
                    }
                    pStrm->Seek( nCurPos );
                }
                else
                    nErrRet = ERR_WW6_NO_WW6_FILE_ERR;
            }
            break;
        case 8:
            if (0xa5ec != nMagic)
                nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            break;
        default:
            nErrRet = ERR_WW8_NO_WW8_FILE_ERR;
            ASSERT( !this, "Es wurde vergessen, nVersion zu kodieren!" );
            break;
    }

    if (!nErrRet)
        nErrRet = LoadDoc1(rPaM ,pGloss);

    return nErrRet;
}


ULONG WW8Reader::Read(SwDoc &rDoc, SwPaM &rPam, const String & /* FileName */)
{
    USHORT nOldBuffSize = 32768;
    bool bNew = !bInsertMode;               // Neues Doc ( kein Einfuegen )


    SvStorageStreamRef refStrm;         // damit uns keiner den Stream klaut
    SvStream* pIn = pStrm;

    ULONG nRet = 0;
    BYTE nVersion = 8;

    String sFltName = GetFltName();
    if( sFltName.EqualsAscii( "WW6" ) )
    {
        if (pStrm)
            nVersion = 6;
        else
        {
            ASSERT(!this, "WinWord 95 Reader-Read ohne Stream");
            nRet = ERR_SWG_READ_ERROR;
        }
    }
    else
    {
        if( sFltName.EqualsAscii( "CWW6" ) )
            nVersion = 6;
        else if( sFltName.EqualsAscii( "CWW7" ) )
            nVersion = 7;

        if( pStg )
        {
            nRet = OpenMainStream( refStrm, nOldBuffSize );
            pIn = &refStrm;
        }
        else
        {
            ASSERT(!this, "WinWord 95/97 Reader-Read ohne Storage");
            nRet = ERR_SWG_READ_ERROR;
        }
    }

    if( !nRet )
    {
        //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
        //              Kapitelnummer. Darum hier explizit abschalten
        //              weil das Default jetzt wieder auf AN ist.
        if (bNew)
        {
            Reader::SetNoOutlineNum( rDoc );
            // MIB 27.09.96: Umrandung uns Abstaende aus Frm-Vorlagen entf.
            Reader::ResetFrmFmts( rDoc );
        }
        SwWW8ImplReader* pRdr = new SwWW8ImplReader(nVersion, pStg, pIn, rDoc,
            bNew);
        nRet = pRdr->LoadDoc( rPam );
        delete pRdr;

        if( refStrm.Is() )
        {
            refStrm->SetBufferSize( nOldBuffSize );
            refStrm.Clear();
        }
    }
    return nRet;
}

int WW8Reader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}

BOOL WW8Reader::HasGlossaries() const
{
    return true;
}

BOOL WW8Reader::ReadGlossaries(SwTextBlocks& rBlocks, BOOL bSaveRelFiles) const
{
    bool bRet=false;

    WW8Reader *pThis = const_cast<WW8Reader *>(this);

    USHORT nOldBuffSize = 32768;
    SvStorageStreamRef refStrm;
    if (!pThis->OpenMainStream(refStrm, nOldBuffSize))
    {
        WW8Glossary aGloss( refStrm, 8, pStg );
        bRet = aGloss.Load( rBlocks, bSaveRelFiles ? true : false);
    }
    return bRet ? true : false;
}

BOOL SwMSDffManager::GetOLEStorageName(long nOLEId, String& rStorageName,
    SvStorageRef& rSrcStorage, SvStorageRef& rDestStorage) const
{
    bool bRet = false;

    long nPictureId = 0;
    if( !( rReader.nIniFlags & WW8FL_NO_OLE ) && rReader.pStg )
    {
        // dann holen wir uns mal ueber den TextBox-PLCF die richtigen
        // Char Start-/End-Positionen. In dem Bereich sollte dann
        // das EinbettenFeld und die entsprechenden Sprms zu finden
        // sein. Wir brauchen hier aber nur das Sprm fuer die Picture Id
        long nOldPos = rReader.pStrm->Tell();
        {
            long nStartCp, nEndCp;
            rReader.GetTxbxTextSttEndCp( nStartCp, nEndCp,
                                        ( nOLEId >> 16 ) & 0xFFFF,
                                        nOLEId & 0xFFFF );

            WW8PLCFxSaveAll aSave;
            memset( &aSave, 0, sizeof( aSave ) );
            rReader.pPlcxMan->SaveAllPLCFx( aSave );

            nStartCp += rReader.nDrawCpO;
            nEndCp   += rReader.nDrawCpO;
            WW8PLCFx_Cp_FKP* pChp = rReader.pPlcxMan->GetChpPLCF();
            wwSprmParser aSprmParser(rReader.pWwFib->nVersion);
            while (nStartCp <= nEndCp && !nPictureId)
            {
                WW8PLCFxDesc aDesc;
                pChp->SeekPos( nStartCp );
                pChp->GetSprms( &aDesc );

                if (aDesc.nSprmsLen && aDesc.pMemPos)   // Attribut(e) vorhanden
                {
                    long nLen = aDesc.nSprmsLen;
                    const BYTE* pSprm = aDesc.pMemPos;

                    while (nLen >= 2 && !nPictureId)
                    {
                        USHORT nId = aSprmParser.GetSprmId(pSprm);
                        USHORT nSL = aSprmParser.GetSprmSize(nId, pSprm);

                        if( nLen < nSL )
                            break;              // nicht mehr genug Bytes uebrig

                        if( 0x6A03 == nId && 0 < nLen )
                        {
                            nPictureId = SVBT32ToLong(pSprm +
                                aSprmParser.DistanceToData(nId));
                            bRet = true;
                        }
                        pSprm += nSL;
                        nLen -= nSL;
                    }
                }
                nStartCp = aDesc.nEndPos;
            }

            rReader.pPlcxMan->RestoreAllPLCFx( aSave );
        }
        rReader.pStrm->Seek( nOldPos );
    }

    if( bRet )
    {
        rStorageName = '_';
        rStorageName += String::CreateFromInt32( nPictureId );
        rSrcStorage = rReader.pStg->OpenStorage(CREATE_CONST_ASC(
            SL::aObjectPool));
        SwDocShell *pDocShell = rReader.rDoc.GetDocShell();
        if (pDocShell == 0)
            bRet=false;
        else
            rDestStorage = pDocShell->GetStorage();
    }
    return bRet;
}

BOOL SwMSDffManager::ShapeHasText(ULONG, ULONG) const
{
    // Zur Zeit des Einlesens einer einzelnen Box, die womoeglich Teil einer
    // Gruppe ist, liegen noch nicht genuegend Informationen vor, um
    // entscheiden zu koennen, ob wir sie nicht doch als Textfeld benoetigen.
    // Also vorsichtshalber mal alle umwandeln:
    return true;
}

bool SwWW8ImplReader::InEqualOrHigherApo(int nLvl) const
{
    if (nLvl)
        --nLvl;
    mycApoIter aIter = std::find(maApos.begin() + nLvl, maApos.end(), true);
    if (aIter != maApos.end())
        return true;
    else
        return false;
}

bool SwWW8ImplReader::InEqualApo(int nLvl) const
{
    //If we are in a table, see if an apo was inserted at the level below
    //the table.
    if (nLvl)
        --nLvl;
    return maApos[nLvl];
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
