/*************************************************************************
 *
 *  $RCSfile: ww8par.cxx,v $
 *
 *  $Revision: 1.73 $
 *
 *  last change: $Author: cmc $ $Date: 2002-07-23 12:39:46 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#ifndef _COM_SUN_STAR_I18N_FORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#endif

#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif

#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif
#ifndef __SGI_STL_FUNCTIONAL
#include <functional>
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
    : SvxMSDffManager( *rRdr.pTableStream, rRdr.pWwFib->fcDggInfo,
    rRdr.pDataStream, 0, 0, COL_WHITE, 12 , rRdr.pStrm ), rReader( rRdr ),
    pFallbackStream(0), pOldEscherBlipCache(0)
{
    nSvxMSDffOLEConvFlags = SwMSDffManager::GetFilterFlags();
}

UINT32 SwMSDffManager::GetFilterFlags()
{
    UINT32 nFlags = 0;
    const OfaFilterOptions* pOpt = OFF_APP()->GetFilterOptions();
    if( pOpt->IsMathType2Math() )
        nFlags |= OLE_MATHTYPE_2_STARMATH;
/*
    // !! don't convert the OLE-Object into the own format
    if( pOpt->IsWinWord2Writer() )
        nFlags |= OLE_WINWORD_2_STARWRITER;
*/
    if( pOpt->IsExcel2Calc() )
        nFlags |= OLE_EXCEL_2_STARCALC;
    if( pOpt->IsPowerPoint2Impress() )
        nFlags |= OLE_POWERPOINT_2_STARIMPRESS;
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
            rReader.pFormImpl->ReadOCXStream(xSrc,&xShape,TRUE))
        {
            pRet = GetSdrObjectFromXShape(xShape);
        }
        else
            pRet = CreateSdrOLEFromStorage( sStorageName, xSrcStg, xDstStg,
                rGrf, rBoundRect, pStData, nSvxMSDffOLEConvFlags );
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
        bCpxStyle = FALSE;
        return;
    }
    USHORT nColl = SVBT16ToShort(pData);
    if (nColl < nColls)
    {
        SetTxtFmtCollAndListLevel( *pPaM, pCollA[nColl] );
        bCpxStyle = TRUE;
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
    SwFltControlStack::NewAttr(rPos, rAttr);
}

void SwWW8FltControlStack::SetAttr(const SwPosition& rPos, USHORT nAttrId, BOOL bTstEnde, long nHand)
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

void SwWW8FltControlStack::SetAttrInDoc(const SwPosition& rTmpPos,
    SwFltStackEntry* pEntry)
{
    switch( pEntry->pAttr->Which() )
    {
        case RES_LR_SPACE:
            {
                SwPaM aRegion( rTmpPos );
                if (pEntry->MakeRegion(pDoc, aRegion, FALSE))
                {
                    SvxLRSpaceItem aLR( *(SvxLRSpaceItem*)pEntry->pAttr );
                    BOOL bChange1stLine = 1 == aLR.GetTxtFirstLineOfst();
                    ULONG nStart = aRegion.Start()->nNode.GetIndex();
                    ULONG nEnd   = aRegion.End()->nNode.GetIndex();
                    const SwNumRule* pRule;
                    const SwNodeNum* pNum;
                    for(; nStart <= nEnd; ++nStart)
                    {
                        SwNode* pNode = pDoc->GetNodes()[ nStart ];
                        if (pNode->IsTxtNode())
                        {
                            if( bChange1stLine )
                            {
                                if ( (pNum = ((SwTxtNode*)pNode)->GetNum()) &&
                                    (MAXLEVEL > pNum->GetLevel()) &&
                                    (pRule = ((SwTxtNode*)pNode)->GetNumRule())
                                   )
                                {
                                    const SwNumFmt rNumFmt =
                                        pRule->Get(pNum->GetLevel());
                                    aLR.SetTxtFirstLineOfst(
                                        rNumFmt.GetFirstLineOffset());
                                }
                                else
                                    aLR.SetTxtFirstLineOfst( 0 );
                            }
                            ((SwCntntNode*)pNode)->SetAttr( aLR );

                            // wenn wir dies nicht tun, ueberschreibt die
                            // NumRule uns alle harten L-Randeinstellungen
                            pNode->SetNumLSpace( FALSE );
                        }
                    }
                }
            }
            break;
        case RES_TXTATR_FIELD:
            ASSERT(0,"What is a field doing in the control stack,"
                "probably should have been in the endstack");
            break;
        case RES_TXTATR_INETFMT:
            {
                SwPaM aRegion(rTmpPos);
                if (pEntry->MakeRegion(pDoc, aRegion, FALSE))
                {
                    SwFrmFmt *pFrm;
                    //If we have just one single inline graphic then
                    //don't insert a field for the single frame, set
                    //the frames hyperlink field attribute directly.
                    if (pFrm = rReader.ContainsSingleInlineGraphic(aRegion))
                    {
                        const SwFmtINetFmt *pAttr = (const SwFmtINetFmt *)
                            pEntry->pAttr;
                        SwFmtURL aURL;
                        aURL.SetURL(pAttr->GetValue(), FALSE);
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

BOOL SwWW8FltRefStack::IsFtnEdnBkmField(const SwFmtFld& rFmtFld, USHORT& rBkmNo)
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
            const SwFmtFld& rFmtFld   = *(const SwFmtFld*)pEntry->pAttr;
            const SwField* pFld = rFmtFld.GetFld();

            if (!RefToVar(pFld,pEntry))
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

            SwNodeIndex aIdx( pEntry->nMkNode, +1 );
            SwPaM aPaM( aIdx, pEntry->nMkCntnt );
            pDoc->Insert(aPaM, *pEntry->pAttr);
            MoveAttrs(*aPaM.GetPoint());
        }
        break;
        case RES_FLTR_TOX:
        case RES_FLTR_BOOKMARK:
            SwFltEndStack::SetAttrInDoc(rTmpPos, pEntry);
            break;
        default:
            ASSERT(0,"EndStck used with non field, not what we want");
            SwFltEndStack::SetAttrInDoc(rTmpPos, pEntry);
            break;
    }
}

//-----------------------------------------
//            Tabs
//-----------------------------------------
#define DEF_TAB_ANZ 13          // So viele Default-Tabs


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

    BOOL bFound = FALSE;
    while( pSty && !bFound )
    {
        const SfxPoolItem* pTabs;
        bFound = pSty->GetAttrSet().GetItemState( RES_PARATR_TABSTOP, FALSE,
            &pTabs ) == SFX_ITEM_SET;
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
    // correct the LastPrinted date in DocumentInfo
    if (rDoc.GetpInfo())
    {
        DateTime aLastPrinted(
            WW8ScannerBase::WW8DTTM2DateTime(pWDop->dttmLastPrint));
        SfxDocumentInfo* pNeuDocInf = new SfxDocumentInfo(*rDoc.GetpInfo());
        SfxStamp aPrinted(pNeuDocInf->GetPrinted());
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

            pNeuDocInf->SetPrinted( aPrinted );
            rDoc.SetInfo( *pNeuDocInf );
            delete pNeuDocInf;
        }
    }

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

    //import magic doptypography information, if its there
    if (pWwFib->nFib > 105)
        ImportDopTypography(pWDop->doptypography);
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
        i18n::ForbiddenCharacters aForbidden(
            WW8DopTypography::aJapanNotBeginLevel1,
            WW8DopTypography::aJapanNotEndLevel1);
        rDoc.SetForbiddenCharacters(LANGUAGE_JAPANESE,aForbidden);
    }

    rDoc.SetKernAsianPunctuation(rTypo.fKerningPunct);
    rDoc.SetCharCompressType(
        static_cast<SwCharCompressType>(rTypo.iJustification));
}


//-----------------------------------------
//      Fuss- und Endnoten
//-----------------------------------------

WW8ReaderSave::WW8ReaderSave( SwWW8ImplReader* pRdr ,WW8_CP nStartCp)
    : aTmpPos(*pRdr->pPaM->GetPoint())
{
    pWFlyPara       = pRdr->pWFlyPara;
    pSFlyPara       = pRdr->pSFlyPara;
    pTableDesc      = pRdr->pTableDesc;
    cSymbol         = pRdr->cSymbol;
    bSymbol         = pRdr->bSymbol;
    bIgnoreText     = pRdr->bIgnoreText;
    bDontCreateSep  = pRdr->bDontCreateSep;
    bHdFtFtnEdn     = pRdr->bHdFtFtnEdn;
    bApo            = pRdr->bApo;
    bTxbxFlySection = pRdr->bTxbxFlySection;
    nTable          = pRdr->nTable ;
    bTableInApo     = pRdr->bTableInApo;
    bAnl            = pRdr->bAnl;
    bInHyperlink    = pRdr->bInHyperlink;
    bPgSecBreak     = pRdr->bPgSecBreak;
    bWasParaEnd = pRdr->bWasParaEnd;
    bHasBorder = pRdr->bHasBorder;
    nAktColl        = pRdr->nAktColl;

    //Honestly should inherit this from parent environment so don't reset this
    bVerticalEnviron = pRdr->bVerticalEnviron;

    pRdr->bHdFtFtnEdn = TRUE;
    pRdr->bApo = pRdr->bTxbxFlySection = pRdr->bTableInApo = pRdr->bAnl =
        pRdr->bPgSecBreak = pRdr->bWasParaEnd = pRdr->bHasBorder = FALSE;
    pRdr->nTable = 0;
    pRdr->pWFlyPara = 0;
    pRdr->pSFlyPara = 0;
    pRdr->pTableDesc = 0;
    pRdr->nAktColl = 0;

    pOldStck = pRdr->pCtrlStck;
    pRdr->pCtrlStck = new SwWW8FltControlStack(&pRdr->rDoc, pRdr->nFieldFlags,
        *pRdr);

    pOldAnchorStck = pRdr->pAnchorStck;
    pRdr->pAnchorStck = new SwWW8FltAnchorStack(&pRdr->rDoc, pRdr->nFieldFlags);

    // rette die Attributverwaltung: dies ist noetig, da der neu anzulegende
    // PLCFx Manager natuerlich auf die gleichen FKPs zugreift, wie der alte
    // und deren Start-End-Positionen veraendert...
    pRdr->pPlcxMan->SaveAllPLCFx( aPLCFxSave );

    pOldPlcxMan = pRdr->pPlcxMan;

    if (nStartCp != -1)
    {
        pRdr->pPlcxMan = new WW8PLCFMan(pRdr->pSBase, pOldPlcxMan->GetManType(),
            nStartCp );
    }
}

void WW8ReaderSave::Restore( SwWW8ImplReader* pRdr )
{
    pRdr->pWFlyPara     = pWFlyPara;
    pRdr->pSFlyPara     = pSFlyPara;
    pRdr->pTableDesc    = pTableDesc;
    pRdr->cSymbol       = cSymbol;
    pRdr->bSymbol       = bSymbol;
    pRdr->bIgnoreText   = bIgnoreText;
    pRdr->bDontCreateSep= bDontCreateSep;
    pRdr->bHdFtFtnEdn   = bHdFtFtnEdn;
    pRdr->bApo          = bApo;
    pRdr->bTxbxFlySection=bTxbxFlySection;
    pRdr->nTable        = nTable;
    pRdr->bTableInApo   = bTableInApo;
    pRdr->bAnl          = bAnl;
    pRdr->bInHyperlink  = bInHyperlink;
    pRdr->bVerticalEnviron = bVerticalEnviron;
    pRdr->bWasParaEnd = bWasParaEnd;
    pRdr->bPgSecBreak   = bPgSecBreak;
    pRdr->nAktColl      = nAktColl;
    pRdr->bHasBorder = bHasBorder;

    // schliesse alle Attribute, da sonst Attribute
    // entstehen koennen, die aus dem Fly rausragen
    pRdr->DeleteCtrlStk();
    pRdr->pCtrlStck = pOldStck;

    pRdr->DeleteAnchorStk();
    pRdr->pAnchorStck = pOldAnchorStck;

    *pRdr->pPaM->GetPoint() = aTmpPos;

    if (pOldPlcxMan != pRdr->pPlcxMan)
    {
        delete pRdr->pPlcxMan;
        pRdr->pPlcxMan = pOldPlcxMan;
    }
    pRdr->pPlcxMan->RestoreAllPLCFx( aPLCFxSave );
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
    aNdIdx = *rDoc.GetNodes().MakeTextSection( aNdIdx, SwNormalStartNode,
        rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

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

//-----------------------------------------
//      Header und Footer
//-----------------------------------------

void SwWW8ImplReader::Read_HdFtText( long nStartCp, long nLen, SwPageDesc* pPD,
                             BOOL bUseLeft, BOOL bFooter )
{
    SwFrmFmt* pFmt = ( bUseLeft ) ? &pPD->GetLeft() : &pPD->GetMaster();
    SwFrmFmt* pHdFtFmt;

    if( bFooter )
    {
        bIsFooter = TRUE;
        pFmt->SetAttr( SwFmtFooter( TRUE ) );
        pHdFtFmt = (SwFrmFmt*)pFmt->GetFooter().GetFooterFmt();
    }else
    {
        bIsHeader = TRUE;
        pFmt->SetAttr( SwFmtHeader( TRUE ) );
        pHdFtFmt = (SwFrmFmt*)pFmt->GetHeader().GetHeaderFmt();
    }

    const SwNodeIndex* pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
    if (!pSttIdx)
        return;

    SwPosition aTmpPos( *pPaM->GetPoint() );    // merke alte Cursorposition

    Read_HdFtFtnText( pSttIdx, nStartCp, nLen - 1, MAN_HDFT );

    *pPaM->GetPoint() = aTmpPos;
    bIsHeader = bIsFooter = FALSE;
}

BYTE SwWW8ImplReader::HdFtCorrectPara( BYTE nPara )
{
    WW8_CP start;
    long nLen;
    BYTE nNewPara = nPara;

    for( BYTE nI = 0x20; nI; nI >>= 1 )
        if(     ( nI & nPara )
            &&  pHdFt
            &&  ( !pHdFt->GetTextPos( nPara, nI, start, nLen ) || nLen <= 2 ) )

        {
            nNewPara &= ~nI;        // leere KF-Texte nicht anlegen
        }

// Wenn im Doc keine Facing Pages vorhanden sind, sollen
// die geraden Header/Footer ignoriert werden.
// Die Facing Pages lassen sich nur Doc-weit,
// nicht aber abschnittsweise umschalten !
    if( pWDop->fFacingPages )
    {       // Wenn pDoc->FacingPages
//      if( nNewPara & WW8_HEADER_ODD )     // und ungerader Header vorhanden
//          nNewPara |= WW8_HEADER_EVEN;        // ... dann muss auch ein gerader
                                            // Header erzeugt werden
//      if( nNewPara & WW8_FOOTER_ODD )     // Footer ebenso.
//          nNewPara |= WW8_FOOTER_EVEN;
    }
    else
    {
        nNewPara &= ~( WW8_HEADER_EVEN |  WW8_FOOTER_EVEN ); // ergibt bEven = 0
    }
    return nNewPara;
}

void SwWW8ImplReader::Read_HdFt1( BYTE nPara, BYTE nWhichItems, SwPageDesc* pPD )
{
    if( pHdFt )
    {
        WW8_CP start;
        long nLen;
        BYTE nNumber = 5;

        for( BYTE nI = 0x20; nI; nI >>= 1, nNumber-- )
            if( nI & nWhichItems )
            {
                BOOL bOk = TRUE;
                if( bVer67 )
                    bOk = ( pHdFt->GetTextPos( nPara, nI, start, nLen ) && nLen > 2 );
                else
                {
                    pHdFt->GetTextPosExact(nNumber+ (nActSectionNo+1)*6, start, nLen);
                    bOk = ( 2 < nLen );
                }
                if( bOk )
                {
                    BOOL bUseLeft
                        = (nI & ( WW8_HEADER_EVEN | WW8_FOOTER_EVEN )) ? TRUE : FALSE;
                    BOOL bFooter
                        = (nI & ( WW8_FOOTER_EVEN | WW8_FOOTER_ODD | WW8_FOOTER_FIRST )) ? TRUE : FALSE;
                    Read_HdFtText( start, nLen, pPD, bUseLeft, bFooter );
                }
            }
    }
}

static BYTE ReadBSprm( const WW8PLCFx_SEPX* pSep, USHORT nId, BYTE nDefaultVal )
{
    const BYTE* pS = pSep->HasSprm( nId );          // sprm da ?
    BYTE nVal = ( pS ) ? SVBT8ToByte( pS ) : nDefaultVal;
    return nVal;
}

void SwWW8ImplReader::SetHdFt( SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1,
    BYTE nIPara )
{

    // Header / Footer nicht da oder StyleDef
    if( !nCorrIhdt || ( pAktColl != 0 ) )
        return;

    long nOldDoc = pStrm->Tell(     );      // WW-Scanner: alles retten
    long nOldTbl = pTableStream->Tell();    // WW-Scanner: alles retten
    WW8PLCFxSaveAll aSave;
    pPlcxMan->SaveAllPLCFx( aSave );        // Attributverwaltung sichern
    WW8PLCFMan* pOldPlcxMan = pPlcxMan;

    if( !pPageDesc1 ){          // 1 Pagedesc reicht

        Read_HdFt1( nIPara,
                    nCorrIhdt & ~( WW8_HEADER_FIRST | WW8_FOOTER_FIRST ),
                    pPageDesc0 );

    }else{                      // 2 Pagedescs noetig: 1.Seite und folgende


                                // 1. Seite einlesen
        Read_HdFt1( nIPara,
                    nCorrIhdt & ( WW8_HEADER_FIRST | WW8_FOOTER_FIRST ),
                    pPageDesc0 );

                                // Folgeseiten einlesen
        Read_HdFt1( nIPara,
                    nCorrIhdt & ~( WW8_HEADER_FIRST | WW8_FOOTER_FIRST ),
                    pPageDesc1 );
    }
                                // dann WW-Scanner-Kram restoren
    pPlcxMan = pOldPlcxMan;             // Attributverwaltung restoren
    pPlcxMan->RestoreAllPLCFx( aSave );
    pStrm->Seek(      nOldDoc );                // FilePos dito (Sprms)
    pTableStream->Seek( nOldTbl );              // FilePos dito (Sprms)
}

//-----------------------------------------
//          PageDescs
//-----------------------------------------


void SwWW8ImplReader::SetLastPgDeskIdx()
{
    if(    0 == pWDop->epc      // store NodeIndex for adding section when FT-/End-Note
        || 2 == pWDop->fpc )    // found that shall be on end of section.
    {
        if(  pLastPgDeskIdx )
            *pLastPgDeskIdx = pPaM->GetPoint()->nNode;
        else
            pLastPgDeskIdx = new SwNodeIndex( pPaM->GetPoint()->nNode );
        (*pLastPgDeskIdx)--;
    }
}

class AttribHere : public std::unary_function<const xub_StrLen*, bool>
{
private:
    xub_StrLen nPosition;
public:
    AttribHere(xub_StrLen nPos) : nPosition(nPos) {}
    bool operator()(const xub_StrLen *pPosition) const
    {
        return (0 != (*pPosition == nPosition));
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
                    >= nPgWidth-nPgRight-nPgLeft)
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

SwPageDesc* SwWW8ImplReader::CreatePageDesc(SwPageDesc* pFirstPageDesc,
    SwPaM** ppPaMWanted)
{
    ASSERT(pFirstPageDesc || ppPaMWanted, "!pFirstPageDesc but NO ppPaMWanted");

    BOOL bFollow = ( pFirstPageDesc != 0 );
    SwPageDesc* pNewPD;
    USHORT nPos;

    if(    bFollow
        && pFirstPageDesc->GetFollow() != pFirstPageDesc )
        return pFirstPageDesc;      // Fehler: hat schon Follow

    // compose name of PageDescriptor
    USHORT nPageDescCount = rDoc.GetPageDescCnt();
    nPos = rDoc.MakePageDesc(
        ViewShell::GetShellRes()->GetPageDescName(nPageDescCount,FALSE,bFollow),
        bFollow ? pFirstPageDesc : 0 );

    pNewPD = &rDoc._GetPageDesc( nPos );

    if (bFollow)
    {
        // Dieser ist der folgende von pPageDesc
        pFirstPageDesc->SetFollow( pNewPD );
        pNewPD->SetFollow( pNewPD );
    }
    else
    {
        // setze PgDesc-Attr ins Doc
        if( bApo || bTxbxFlySection )
        {
            // PageDesc *muss* ausserhalb des Apo stehen
            if( pSFlyPara && pSFlyPara->pMainTextPos )
            {
                SwPaM aMyPaM( *pSFlyPara->GetMainTextPos() );
                if( 1 < nPageDescCount )
                    AppendTxtNode( *aMyPaM.GetPoint() );
                SwFmtCol* pCol;
                RemoveCols(  *pNewPD, pCol );
                rDoc.Insert( aMyPaM, SwFmtPageDesc( pNewPD ) );
                if( ppPaMWanted )
                    *ppPaMWanted = new SwPaM( aMyPaM );
                if( pCol )
                {
                    InsertSectionWithWithoutCols( aMyPaM, pCol );
                    delete pCol;
                }
                else
                    SetLastPgDeskIdx();
            }
        }
        else
        {
            if( 0 < pPaM->GetPoint()->nContent.GetIndex() )
                AppendTxtNode( *pPaM->GetPoint() );
            SwFmtCol* pCol;
            RemoveCols(  *pNewPD, pCol );
            if( ppPaMWanted )
                *ppPaMWanted = new SwPaM( *pPaM );

            rDoc.Insert( *pPaM, SwFmtPageDesc( pNewPD ) );
            if( pCol )
            {
                InsertSectionWithWithoutCols( *pPaM, pCol );
                delete pCol;
            }
            else
                SetLastPgDeskIdx();
        }
    }
    return pNewPD;
}

// UpdatePageDescs muss am Ende des Einlesevorganges aufgerufen werden, damit
// der Writer den Inhalt der Pagedescs wirklich akzeptiert
void SwWW8ImplReader::UpdatePageDescs(USHORT nInPageDescOffset)
{
    USHORT i;

    // Pagedescriptoren am Dokument updaten (nur so werden auch die
    // linken Seiten usw. eingestellt).

    // PageDesc "Standard"
    rDoc.ChgPageDesc( 0, rDoc.GetPageDesc( 0 ));

    // PageDescs "Konvert..."
    for ( i=nInPageDescOffset; i < rDoc.GetPageDescCnt(); i++ )
    {
        const SwPageDesc* pPD = &rDoc.GetPageDesc( i );
        rDoc.ChgPageDesc( i, *pPD );
    }
}

USHORT SwWW8ImplReader::TabCellSprm(int nLevel) const
{
    if (bVer67)
        return 24;
    return nLevel ? 0x244B : 0x2416;
}

USHORT SwWW8ImplReader::TabRowSprm(int nLevel) const
{
    if (bVer67)
        return 25;
    return nLevel ? 0x244C : 0x2417;
}

BOOL SwWW8ImplReader::ProcessSpecial( BOOL bAllEnd, BOOL* pbReSync,
    WW8_CP nStartCp )   // Frame / Table / Anl
{
    if( bInHyperlink )
        return FALSE;

    *pbReSync = FALSE;
    if( bAllEnd )
    {
        if( bAnl )
            StopAnl();                  // -> bAnl = FALSE
        if( nTable && !bFtnEdn )        // Tabelle in FtnEdn nicht erlaubt
            StopTable();
        if( bApo )
            StopApo();
        --nTable;
        bApo = FALSE;
        return FALSE;
    }

    ASSERT(nTable >= 0,"nTable < 0!");

    // TabRowEnd
    BOOL bTableRowEnd = (pPlcxMan->HasParaSprm(bVer67 ? 25 : 0x2417) != 0 );

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
        *pbReSync = TRUE;
        WW8PLCFx_Cp_FKP* pPap = pPlcxMan->GetPapPLCF();
        WW8_CP nMyStartCp=nStartCp;

        if (const BYTE *pLevel = pPlcxMan->HasParaSprm(0x6649))
            nCellLevel = *pLevel;

        if (SearchRowEnd(pPap, nMyStartCp, nCellLevel-1) && (ParseTabPos(&aTabPos,pPap)))
            pTabPos = &aTabPos;

        pPlcxMan->GetPap()->Restore( aSave );
    }

//  then look if we are in an Apo

    BOOL bStartApo, bStopApo;
    WW8FlyPara *pNowStyleApo=0;
    const BYTE* pSprm29 = TestApo( bStartApo, bStopApo, pNowStyleApo, nTable,
        bTableRowEnd && bTableInApo, pTabPos);

    //look to see if we are in a Table, but Table in foot/end note not allowed
    BOOL bStartTab = (nTable < nCellLevel) && !bFtnEdn;

    BOOL bStopTab = bWasTabRowEnd && (nTable > nCellLevel) && !bFtnEdn;

    bWasTabRowEnd = FALSE;  // must be deactivated right here to prevent next
                            // WW8TabDesc::TableCellEnd() from making nonsense
#if 0
    //we shouldn't need this anymore with table in table support.
    if( nTable && !bStopTab && ( bStartApo || bStopApo ) )
    {                                   // Wenn Apowechsel in Tabelle
        bStopTab = bStartTab = TRUE;    // ... dann auch neue Tabelle
    }
#endif
//  Dann auf Anl (Nummerierung) testen
//  und dann alle Ereignisse in der richtigen Reihenfolge bearbeiten

    if( bAnl && !bTableRowEnd )
    {
        const BYTE* pSprm13 = pPlcxMan->HasParaSprm( 13 );
        if( pSprm13 )
        {                                   // Noch Anl ?
            BYTE nT = GetNumType( *pSprm13 );
            if( ( nT != WW8_Pause && nT != nWwNumType ) // Anl-Wechsel
                || bStopApo  || bStopTab                // erzwungenes Anl-Ende
                || bStartApo || bStartTab )
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
    if( bStopTab )
    {
        StopTable();
        --nTable;
    }
    if( bStopApo )
    {
        StopApo();
        bApo = FALSE;
    }

    if( bStartApo && !( nIniFlags & WW8FL_NO_APO ) )
    {
        bApo = StartApo(pSprm29, pNowStyleApo, pTabPos);
        *pbReSync = TRUE;                   // nach StartApo ist ein ReSync
                                            // noetig ( eigentlich nur, falls
                                            // die Apo ueber eine FKP-Grenze
                                            // geht
    }
    if( bStartTab && !( nIniFlags & WW8FL_NO_TABLE ) )
    {
        if( bAnl )                          // Nummerierung ueber Zellengrenzen
            StopAnl();                      // fuehrt zu Absturz -> keine Anls
                                            // in Tabellen
        nTable += StartTable(nStartCp);
        *pbReSync = TRUE;                   // nach StartTable ist ein ReSync
                                            // noetig ( eigentlich nur, falls
                                            // die Tabelle ueber eine
                                            // FKP-Grenze geht
        bTableInApo = nTable && bApo;
    }
    return bTableRowEnd;
}

#if defined OS2
// eigentlich besser inline, aber das kann der BLC nicht
static UCHAR ConvOs2( UCHAR ch, CharSet eDst )
{
    switch( ch )
    {
        case 132:
        case 148:
            return ( eDst == CHARSET_IBMPC_865 ) ? '"' : 175;
                                // typographische "(links) gegen aehnliche
                                // im OS/2-Charset
        case 147:
            return ( eDst == CHARSET_IBMPC_865 ) ? '"' : 174;
        case 173:
            // kurze, mittellange und lange Striche gegen Minus
        case 150:
        case 151:
            return  '-';
        case 130:
            return ',';
        case 145:
        case 146:
            return '\'';    // typographische ' gegen normale
        case 139:
            return '<';
        case 155:
            return '>';
        case 152:
            return '~';
    }                       // ansonsten macht noch TM AErger.

    return 0;               // kenn ick nich
}
#endif

// Returnwert: TRUE fuer keine Sonderzeichen
BOOL SwWW8ImplReader::ReadPlainChars( long& rPos, long nEnd, long nCpOfs )
{
    // Unicode-Flag neu setzen und notfalls File-Pos korrigieren
    // merke: Seek kostet nicht viel, da inline geprueft wird,
    //        ob die korrekte FilePos nicht schon erreicht ist.
    WW8_FC nStreamPos = pSBase->WW8Cp2Fc(nCpOfs+rPos, &bIsUnicode);
    pStrm->Seek( nStreamPos );

    // amount of characters to read == length to next attribute
    ASSERT(nEnd - rPos <= STRING_MAXLEN, "String too long for stringclass!");
    xub_StrLen nLen = static_cast<xub_StrLen>(nEnd - rPos);
    ASSERT(nLen, "String is 0");
    if (!nLen)
        return TRUE;

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
        if (eSrcCharSet == RTL_TEXTENCODING_DONTKNOW)
            eSrcCharSet = pCollA[nAktColl].eFontSrcCharSet;
    }

    // (re)alloc UniString data
    String sPlainCharsBuf;

    sal_Unicode* pWork = sPlainCharsBuf.AllocBuffer( nLen );

    // read the stream data
    BYTE   nBCode;
    UINT16 nUCode;
    for( xub_StrLen nL2 = 0; nL2 < nLen; ++nL2, ++pWork )
    {
        if( bIsUnicode )
            *pStrm >> nUCode;   // unicode  --> read 2 bytes
        else
        {
            *pStrm >> nBCode;   // old code --> read 1 byte
            nUCode = nBCode;
        }

        if( 0 != pStrm->GetError() )
        {
            rPos = LONG_MAX-10;     // -> eof or other error
            sPlainCharsBuf.ReleaseBufferAccess( 0 );
            return TRUE;
        }

        if( (32 > nUCode) || (0xa0 == nUCode) )
        {
            pStrm->SeekRel( bIsUnicode ? -2 : -1 );
            sPlainCharsBuf.ReleaseBufferAccess( nL2 );
            break;              // Sonderzeichen < 32, == 0xa0 gefunden
        }

        if( bIsUnicode )
            *pWork = nUCode;
        else
            *pWork = ByteString::ConvertToUnicode( nBCode, eSrcCharSet );
    }
    if( sPlainCharsBuf.Len() )
        rDoc.Insert( *pPaM, sPlainCharsBuf );

    rPos += nL2;
    return nL2 >= nLen;
}


// Returnwert: TRUE fuer Zeilenende
BOOL SwWW8ImplReader::ReadChars( long& rPos, long nNextAttr, long nTextEnd,
    long nCpOfs )
{
    long nEnd = ( nNextAttr < nTextEnd ) ? nNextAttr : nTextEnd;

    if( bSymbol || bIgnoreText )
    {
        if( bSymbol )   // Spezialzeichen einfuegen
        {
            for(USHORT nCh = 0; nCh < nEnd - rPos; ++nCh)
                rDoc.Insert( *pPaM, cSymbol );
            pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_CHRATR_FONT );
        }
        pStrm->SeekRel( nEnd- rPos );
        rPos = nEnd;    // ignoriere bis Attributende
        return FALSE;
    }
    while( TRUE )
    {
        if( ReadPlainChars( rPos, nEnd, nCpOfs ) )
            return FALSE;                   // Fertig, kein CR

        BOOL bStartLine = ReadChar( rPos, nCpOfs );
        rPos++;
        if( bPgSecBreak || bStartLine || rPos == nEnd ) // CR oder Fertig
            return bStartLine;
    }
}

BOOL SwWW8ImplReader::ReadChar( long nPosCp, long nCpOfs )
{
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
    BOOL bRet = FALSE;
    switch( nWCharVal )
    {
        case 0xe:
            /*
            #i2451#, similarly to i1909 column breaks appear to be ignored if
            they are inside tables. Appears impossible to create one from
            scratch with winword.
            */
            if (!nTable)
            {
                SwTxtNode* pNd = pPaM->GetCntntNode()->GetTxtNode();
                if ( pNd )
                {
                    const SfxItemSet* pSet = pNd->GetpSwAttrSet();
                    if (pSet &&
                        (SFX_ITEM_ON == pSet->GetItemState(RES_PAGEDESC,FALSE)))
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
            //#i1909# section/page breaks should not occur in tables, word
            //itself ignores them in this case.
            if (!nTable)
            {
                bPgSecBreak = TRUE;
                pCtrlStck->KillUnlockedAttrs(*pPaM->GetPoint());
                /*
                #74468#
                If its a 0x0c without a paragraph end before it, act like a
                paragraph end, but nevertheless, numbering (and perhaps other
                similiar constructs) do not exist on the para.
                */
                if (!bWasParaEnd)
                {
                    bRet = TRUE;
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
            if( bObj )
                pFmtOfJustInsertedGraphicOrOLE = ImportOle();
            else
                pFmtOfJustInsertedGraphicOrOLE = ImportGraf();
            // reset the flags.
            bObj = bEmbeddObj = FALSE;
            nObjLocFc = 0;
            //##515## set nLastFlyNode so we can determine if a section
            //has ended with this paragraph unclosed
            nLastFlyNode = (*pPaM->GetPoint()).nNode.GetIndex();
            break;
        case 0x8:
            if( !bObj )
            {
                Read_GrafLayer( nPosCp );
                //##515##. Set nLastFlyNode so we can determine if a
                //section has ended with this paragraph unclosed
                nLastFlyNode = (*pPaM->GetPoint()).nNode.GetIndex();
            }
            break;
        case 0xd:
            bRet = TRUE;
            if (nTable > 1)
            {
                WW8PLCFspecial* pTest = pPlcxMan->GetMagicTables();
                if (pTest && pTest->SeekPosExact(nPosCp+1+nCpOfs) &&
                    pTest->Where() == nPosCp+1+nCpOfs)
                {
                    TabCellEnd();
                    ASSERT(!(bWasTabRowEnd && (nTable > 1)),
                        "Complicated subtable mishap");
                    bRet = FALSE;
                }
            }
            break;              // line end
        case 0x5:                           // Annotation reference
        case 0x13:
        case 0x2:
            break;                  // Auto-Fussnoten-Nummer
#ifdef DEBUG
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
        rDoc.Insert( *pPaM, ByteString::ConvertToUnicode(cInsert,
            RTL_TEXTENCODING_MS_1252 ) );
    }
    return bRet;
}


void SwWW8ImplReader::ProcessAktCollChange( WW8PLCFManResult& rRes,
                                            BOOL* pStartAttr,
                                            BOOL bCallProcessSpecial )
{
    USHORT nOldColl = nAktColl;
    nAktColl = pPlcxMan->GetColl();

    if( bIsHeader )             // Fuer Kopfzeilenabstaende
        nHdTextHeight += pPlcxMan->GetPapPLCF()->GetParaHeight();
    else
    if( bIsFooter )             // Fuer Kopfzeilenabstaende
        nFtTextHeight += pPlcxMan->GetPapPLCF()->GetParaHeight();

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
    BOOL bTabRowEnd = FALSE;
    if( pStartAttr && bCallProcessSpecial && !bInHyperlink )
    {
        BOOL bReSync;
        // Frame / Table / Autonumbering List Level
        bTabRowEnd = ProcessSpecial( FALSE, &bReSync,
            rRes.nAktCp + pPlcxMan->GetCpOfs() );
        if( bReSync )
            *pStartAttr = pPlcxMan->Get( &rRes ); // hole Attribut-Pos neu
    }

    if( !bTabRowEnd )
    {
        SetTxtFmtCollAndListLevel( *pPaM, pCollA[ nAktColl ]);
        ChkToggleAttr(pCollA[ nOldColl ].n81Flags, pCollA[ nAktColl ].n81Flags);
        ChkToggleBiDiAttr(pCollA[nOldColl].n81BiDiFlags,
            pCollA[nAktColl].n81BiDiFlags);
    }
}

long SwWW8ImplReader::ReadTextAttr( long& rTxtPos, BOOL& rbStartLine )
{
    long nSkipChars = 0;
    WW8PLCFManResult aRes;

    ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    BOOL bStartAttr = pPlcxMan->Get( &aRes ); // hole Attribut-Pos
    aRes.nAktCp = rTxtPos;              // Akt. Cp-Pos

    if (aRes.nFlags & MAN_MASK_NEW_SEP) // neue Section
    {
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
        CreateSep( rTxtPos, bPgSecBreak );  // PageDesc erzeugen und fuellen
                                            // -> 0xc war ein Sectionbreak, aber
                                            // kein Pagebreak;
        bPgSecBreak = FALSE;                // PageDesc erzeugen und fuellen
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");
    }

    // neuer Absatz ueber Plcx.Fkp.papx
    if ( (aRes.nFlags & MAN_MASK_NEW_PAP)|| rbStartLine )
    {
        ProcessAktCollChange( aRes, &bStartAttr,
            MAN_MASK_NEW_PAP == (aRes.nFlags & MAN_MASK_NEW_PAP) &&
            !bIgnoreText );
        rbStartLine = FALSE;
    }

    // position of last CP that's to be ignored
    long nSkipPos = -1;
    BOOL bOldDontCreateSep = bDontCreateSep;

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
    if( nSkipChars && !bIgnoreText )
        pCtrlStck->MarkAllAttrsOld();
    BOOL  bOldIgnoreText = bIgnoreText;
    bIgnoreText          = TRUE;
    USHORT nOldColl = nAktColl;
    BOOL bDoPlcxManPlusPLus = TRUE;
    long nNext;
    do
    {
        if( bDoPlcxManPlusPLus )
            (*pPlcxMan)++;
        nNext = pPlcxMan->Where();
        if( (0 <= nNext) && (nSkipPos >= nNext) )
        {
            nNext = ReadTextAttr( rTxtPos, rbStartLine );
            bDoPlcxManPlusPLus = FALSE;
            bIgnoreText = TRUE;
        }
    }
    while( nSkipPos >= nNext );
    bIgnoreText    = bOldIgnoreText;
    bDontCreateSep = bOldDontCreateSep;
    if( nSkipChars )
    {
        pCtrlStck->KillUnlockedAttrs( *pPaM->GetPoint() );
        if( nOldColl != pPlcxMan->GetColl() )
            ProcessAktCollChange(aRes, 0, FALSE);
    }

    return nNext;
}

void SwWW8ImplReader::ReadAttrs( long& rNext, long& rTxtPos, BOOL& rbStartLine )
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
// ist FALSE. Deshalb muss als Sonderbehandlung hier die Vorlage gesetzt
// werden.
        if( !bCpxStyle )
        {
            SetTxtFmtCollAndListLevel( *pPaM, pCollA[ nAktColl ] );
        }
        rbStartLine = FALSE;
    }
}

// ReadAttrEnds zum Lesen nur der Attributenden am Ende eines Textes oder
// Textbereiches ( Kopfzeile, Fussnote, ...). Attributanfaenge, Felder
// werden ignoriert.
void SwWW8ImplReader::ReadAttrEnds( long& rNext, long& rTxtPos )
{

    while( rTxtPos >= rNext )
    {
        WW8PLCFManResult aRes;

        BOOL b = pPlcxMan->Get( &aRes );     // hole Attribut-Pos

        if(    !b
            && (aRes.nSprmId >=  0)     // nur Attributenden noch bearbeiten,
            && (    (aRes.nSprmId <     eFTN)
                 || (aRes.nSprmId >= 0x0800) )
            )
        {                                                           // Anfaenge gehoeren zum naechsten Spezialtext
            EndSprm( aRes.nSprmId );                                // Fussnoten und Felder ignorieren
        }
        (*pPlcxMan)++;
        rNext = pPlcxMan->Where();
    }
    BOOL bDummyReSync;
    ProcessSpecial( TRUE, &bDummyReSync, -1 );
}

BOOL SwWW8ImplReader::ReadText( long nStartCp, long nTextLen, short nType )
{
    BOOL bJoined=FALSE;
    if( nIniFlags & WW8FL_NO_TEXT )
        return bJoined;

    BOOL bStartLine = TRUE;
    short nCrCount = 0;

    bWasParaEnd = FALSE;
    nAktColl    =  0;
    pAktItemSet =  0;
    nCharFmt    = -1;
    bSpec = FALSE;
    bPgSecBreak = FALSE;
    nHdTextHeight = nFtTextHeight = 0;

    pPlcxMan = new WW8PLCFMan( pSBase, nType, nStartCp );
    long nCpOfs = pPlcxMan->GetCpOfs(); // Offset fuer Header/Footer, Footnote

    WW8_CP nNext = pPlcxMan->Where();

    pStrm->Seek( pSBase->WW8Cp2Fc( nStartCp + nCpOfs, &bIsUnicode ) );

    if ( (0 == nStartCp+nCpOfs) && SetCols(0, pPlcxMan->GetSepPLCF(), 0, TRUE) )
    {
        // Start of text:
        //
        // look for cols and insert a section at the very beginning...
        if( !pPageDesc )
            pPageDesc = &rDoc._GetPageDesc( 0 );
        SwFrmFmt &rFmt = pPageDesc->GetMaster();
        USHORT nLIdx = ( ( pWwFib->lid & 0xff ) == 0x9 ) ? 1 : 0;
        SetPage1( pPageDesc, rFmt, pPlcxMan->GetSepPLCF(), nLIdx, FALSE );
        const SwFmtCol& rCol = rFmt.GetCol();
        // if PageDesc has been inserted and has cols
        // insert a *section* with cols instead
        if( rCol.GetNumCols() )
        {
            InsertSectionWithWithoutCols( *pPaM, &rCol );
            // remove columns from PageDesc
            SwFmtCol aCol;
            rFmt.SetAttr( aCol );
        }
    }

    WW8_CP l = nStartCp;
    while ( l<nStartCp+nTextLen )
    {

        ReadAttrs( nNext, l, bStartLine );// behandelt auch Section-Breaks
        ASSERT(pPaM->GetNode()->GetTxtNode(), "Missing txtnode");

        if( l>= nStartCp + nTextLen )
            break;

        bStartLine = bWasParaEnd = ReadChars(l, nNext, nStartCp+nTextLen, nCpOfs);

        if( bStartLine ) // Zeilenende
        {
            AppendTxtNode( *pPaM->GetPoint() );

            if( ( nCrCount++ & 0x40 ) == 0          // alle 64 CRs aufrufen
                && nType == MAN_MAINTEXT ){         // nicht fuer Header u. ae.
                nProgress = (USHORT)( l * 100 / nTextLen );
                ::SetProgressState( nProgress, rDoc.GetDocShell() ); // Update
            }
        }

        // If we have encountered a 0x0c which indicates either section of
        // pagebreak then look it up to see if it is a section break, and
        // if it is not then insert a page break. If it is a section break
        // it will be handled as such in the ReadAttrs of the next loop
        if( bPgSecBreak)
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
                /*
                #74468#, ##515##
                Insert additional node only WHEN the Pagebreak is contained in
                a NODE that is NOT EMPTY. Word can have empty paragraphs with
                numbering information before a section break that are not
                closed before the section break. In this case they are ignored
                and not numbered, this is 74468's problem. But word can have
                open paragraphs that only contain a node that in word can be
                anchored, i.e. a single character indicating that a graphic or
                an ole node or a text box is anchored here. In this case then
                we should close the paragraph to ensure that it is is anchored
                to the current page, and not pushed to the next page, this is
                515's problem. nLastFlyNode is set on insertion of 0x01 and
                0x08 graphics as well as on insertion of old ww6 textboxes.
                */
                SwPosition&  rPt = *pPaM->GetPoint();
                if ( (nLastFlyNode == rPt.nNode.GetIndex())
                    || (0 < rPt.nContent.GetIndex()) )
                {
                    AppendTxtNode( rPt );
                }

                rDoc.Insert( *pPaM, SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE ) );
                bPgSecBreak = FALSE;
            }
        }
    }
    ReadAttrEnds( nNext, l );
    if (!bInHyperlink)
        bJoined = JoinNode( pPaM );
    if( nType == MAN_MAINTEXT )
        UpdatePageDescs( nPageDescOffset ); // muss passieren, solange es den
                                            // PlcxMan noch gibt
    DELETEZ( pPlcxMan );
    return bJoined;
}

/***************************************************************************
#           class SwWW8ImplReader
#**************************************************************************/

SwWW8ImplReader::SwWW8ImplReader( BYTE nVersionPara, SvStorage* pStorage,
    SvStream* pSt, SwDoc& rD, bool bNewDoc )
    : pStg( pStorage ), rDoc( rD ), pStrm( pSt ), mbNewDoc(bNewDoc),
    pMSDffManager( 0 ), mpAtnNames( 0 ), pAuthorInfos( 0 ), pOleMap(0),
    pTabNode(0), pLastPgDeskIdx( 0 ), pDataStream( 0 ), pTableStream( 0 ),
    aGrfNameGenerator(bNewDoc,String('G'))
{
    pStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    nWantedVersion = nVersionPara;
    pCtrlStck   = 0;
    pRefStck = 0;
    pAnchorStck = 0;
    pFonts          = 0;
    pSBase          = 0;
    pPlcxMan        = 0;
    pStyles         = 0;
    pAktColl        = 0;
    pLstManager = 0;
    pAktItemSet = 0;
    pCollA          = 0;
    pHdFt           = 0;
    pWFlyPara       = 0;
    pSFlyPara       = 0;
    pFlyFmtOfJustInsertedGraphic   = 0;
    pFmtOfJustInsertedGraphicOrOLE = 0;
    nColls = nAktColl = 0;
    nObjLocFc = nPicLocFc = 0;
    nTable=0;
    bReadNoTbl = bPgSecBreak = bSpec = bObj = bApo = bTxbxFlySection
               = bHasBorder = bSymbol = bIgnoreText = bDontCreateSep
               = bTableInApo = bWasTabRowEnd = FALSE;
    bShdTxtCol = bCharShdTxtCol = bAnl = bHdFtFtnEdn = bFtnEdn
               = bIsHeader = bIsFooter = bSectionHasATitlePage
               = bIsUnicode = bCpxStyle = bStyNormal = bWWBugNormal  = FALSE;
    bNoAttrImport = bPgChpLevel = bEmbeddObj = FALSE;
    bAktAND_fNumberAcross = FALSE;
    bNoLnNumYet = TRUE;
    bRestartLnNumPerSection = FALSE;
    bInHyperlink = FALSE;
    bVerticalEnviron = FALSE;
    bWasParaEnd = FALSE;
    nProgress = 0;
    nHdTextHeight = nFtTextHeight = 0;
    nPgWidth = lA4Width;
    nPgLeft = nPgRight = nPgTop = MM_250;
    nCorrIhdt = 0;
    nSwNumLevel = nWwNumType = 0xff;
    pTableDesc = 0;
    pNumRule = 0;
    pNumOlst = 0;
    pAfterSection = 0;
    pNewSection    = 0;
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

    nLastFlyNode = ULONG_MAX;

    nLFOPosition = USHRT_MAX;
    nListLevel   = nWW8MaxListLevel;
    eHardCharSet = RTL_TEXTENCODING_DONTKNOW;
    pPageDesc = 0;

    nNfcPgn = nPgChpDelim = nPgChpLevel = 0;
}

void SwWW8ImplReader::DeleteStk(SwFltControlStack* pStck)
{
    if( pStck )
    {
        pStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );
        pStck->SetAttr( *pPaM->GetPoint(), 0, FALSE );
        delete pStck;
    }
    else
    {
        ASSERT( !this, "WW-Stack bereits geloescht" );
    }
}

ULONG SwWW8ImplReader::LoadDoc1( SwPaM& rPaM ,WW8Glossary *pGloss)
{
    ULONG nErrRet = 0;

    if (mbNewDoc && pStg && !pGloss)
        ReadDocInfo();

    pPaM = new SwPaM( *rPaM.GetPoint() );

    pCtrlStck = new SwWW8FltControlStack( &rDoc, nFieldFlags, *this );

    /*
        RefFldStck: Keeps track of bookmarks which may be inserted as
        variables intstead.
    */
    pRefStck = new SwWW8FltRefStack(&rDoc, nFieldFlags);

    pAnchorStck = new SwWW8FltAnchorStack(&rDoc, nFieldFlags);

    nPageDescOffset = rDoc.GetPageDescCnt();

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
    else if( ( nIniFlags & WW8FL_NO_COMPLEX ) && pWwFib->fComplex )
    {
        nErrRet = WARN_WW6_FASTSAVE_ERR;            // Warning melden
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

            if (mbNewDoc)
            {
                // Abstand zwischen zwei Absaetzen ist die SUMME von unterem
                // Abst. des ersten und oberem Abst. des zweiten
                rDoc.SetParaSpaceMax( TRUE, TRUE );
                // move tabs on alignment
                rDoc.SetTabCompat( TRUE );
            }

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

            /*
                zuerst(!) alle Styles importieren   (siehe WW8PAR2.CXX)
                    VOR dem Import der Listen !!
            */
            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update
            pStyles = new WW8RStyle( *pWwFib, this );   // Styles
            pStyles->Import();
            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update



            /*
                jetzt erst alle Listen importieren  (siehe WW8PAR3.CXX)
                    NACH dem Import der Styles !!
            */
            pLstManager = new WW8ListManager( *pTableStream, *this );
            ::SetProgressState( nProgress, rDoc.GetDocShell() );    // Update


            /*
                zu guter Letzt: (siehe ebenfalls WW8PAR3.CXX)
                ===============
                alle Styles durchgehen und ggfs. zugehoeriges Listen-Format
                anhaengen NACH dem Import der Styles und NACH dem Import der
                Listen !!
            */
            pStyles->PostProcessStyles();

            pSBase = new WW8ScannerBase(pStrm,pTableStream,pDataStream,pWwFib);

            static SvxExtNumType __READONLY_DATA eNumTA[16] =
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
                static SwFtnNum __READONLY_DATA eNumA[4] =
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
                        rDoc.GetTxtCollFromPool( RES_POOLCOLL_STANDARD);
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

            if (pCollA)
            {
                SetOutLineStyles();
                delete[] pCollA;
            }

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
                            if(       (0xFFFF0000 &     pOrder->nTxBxComp)
                                   == (0xFFFF0000 & pNextOrder->nTxBxComp)
                                &&    pOrder->nHdFtSection
                                   == pNextOrder->nHdFtSection  )
                                pNextFlyFmt = pNextOrder->pFly;
                        }
                        // ggfs. Vorgaenger ermitteln
                        if( nTxBxNum )
                        {
                            SvxMSDffShapeOrder *pPrevOrder =
                                aTxBxSort.GetObject(nTxBxNum-1);
                            if(       (0xFFFF0000 &     pOrder->nTxBxComp)
                                   == (0xFFFF0000 & pPrevOrder->nTxBxComp)
                                &&    pOrder->nHdFtSection
                                   == pPrevOrder->nHdFtSection  )
                                pPrevFlyFmt = pPrevOrder->pFly;
                        }
                        // Falls Nachfolger oder Vorgaenger vorhanden,
                        // die Verkettung am Fly-Frame-Format eintragen
                        if( pNextFlyFmt || pPrevFlyFmt )
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
                if(pStg && !pGloss) /*meaningless for a glossary, cmc*/
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
                        rDoc.SetContainsMSVBasic( TRUE );
                  }
            }

            DELETEZ( pStyles );

            if( pFormImpl )
                DeleteFormImpl();
            GrafikDtor();
            DELETEZ( pMSDffManager );
            DELETEZ( pHdFt );
            DELETEZ( pLstManager );
            DELETEZ( pSBase );
            DELETEZ( pWDop );
            DELETEZ( pFonts );
            delete mpAtnNames;
            DELETEZ( pAuthorInfos );
            DELETEZ( pOleMap );
            DELETEZ( pTabNode );
            DELETEZ( pLastPgDeskIdx );
            delete mpSprmParser;
            ::EndProgress( rDoc.GetDocShell() );
        }
        pDataStream = 0;
        pTableStream = 0;
    }

    if (!pGloss)
        DELETEZ( pWwFib );
    DeleteCtrlStk();
    DeleteAnchorStk();
    DeleteRefStk();

    aRelNumRule.SetNumRelSpaces( rDoc );
    if( !mbNewDoc && !nErrRet && aSttNdIdx.GetIndex() )
    {
        aSttNdIdx++;
        aRelNumRule.SetOultineRelSpaces( aSttNdIdx,
                                    pPaM->GetPoint()->nNode );
    }

    UpdateFields();

    // delete the pam before the call for hide all redlines (Bug 73683)
    if (mbNewDoc)
        rDoc.SetRedlineMode( eMode );

    // set NoBallanced flag on last inserted section and remove the trailing
    // para that inserting the section pushed after the PaM
    if (pNewSection)
    {
        SwSectionFmt *pFmt = pNewSection->GetFmt();
        pFmt->SetAttr(SwFmtNoBalancedColumns(TRUE));
    }
    if (pAfterSection)
    {
        if (mbNewDoc)
        {
            //Needed to unlock last node so that we can delete it without
            //giving writer a fit. Necessary for deleting the para after a
            //section.
            rPaM.GetBound( TRUE ).nContent.Assign( 0, 0 );
            rPaM.GetBound( FALSE ).nContent.Assign( 0, 0 );

            pPaM->SetMark();
            pPaM->GetPoint()->nNode = *pAfterSection;
            pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);

            rDoc.DeleteAndJoin(*pPaM);
        }
        delete pAfterSection;
    }
    DELETEZ(pPaM);
    return nErrRet;
}

class outlinecmp
{
public:
    bool operator()(const SwTxtFmtColl *pOne, const SwTxtFmtColl *pTwo) const
    {
        return pOne->GetOutlineLevel() < pTwo->GetOutlineLevel();
    }
};

class outlineeq
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
    const SwTxtFmtColls& rColls = *rDoc.GetTxtFmtColls();

    //Get a list of txt styles sorted by outline number
    ::std::vector<SwTxtFmtColl *> aOutLined;
    aOutLined.reserve(rColls.Count());
    for(USHORT nA = 0; nA < rColls.Count(); ++nA )
        aOutLined.push_back(rColls[nA]);
    ::std::sort(aOutLined.begin(), aOutLined.end(), outlinecmp());

    typedef ::std::vector<SwTxtFmtColl *>::iterator myiter;

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

    /*
    Leave the first 10 styles until after the rest of them are handled to give
    priority to the non standard styles in the case of a collision between
    styles that want into the outlinenumbering list. Its an inherited hack
    which I'm dubious about.
    */
    USHORT nIa=10;
    USHORT nIz=nColls;
    for (USHORT nJ = 0; nJ < 2; ++nJ)
    {
        for (USHORT nI = nIa; nI < nIz; ++nI)
        {
            SwWW8StyInf& rSI = pCollA[nI];

            if ((MAXLEVEL > rSI.nOutlineLevel) && rSI.pOutlineNumrule
                    && rSI.pFmt)
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
                else if (nJ || rSI.pFmt->GetDepends())
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
                    outlineeq aCmp(rSI.nListLevel);
                    myiter aResult = ::std::find_if(aOutLined.begin(),
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
                    BYTE nFromLevel=rSI.nListLevel;
                    BYTE nToLevel=rSI.nOutlineLevel;
                    const SwNumFmt& rRule=rSI.pOutlineNumrule->Get(nFromLevel);
                    aOutlineRule.Set(nToLevel, rRule);
                    // Set my outline level
                    ((SwTxtFmtColl*)rSI.pFmt)->SetOutlineLevel( nToLevel );
                    // If there are more styles on this level ignore them
                    nFlagsStyleOutlLevel |= nAktFlags;
                }
            }
        }
        nIa = 1;
        nIz = 10;
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
                mpAtnNames->push_back(WW8ReadPString(rStrm, FALSE));
                nRead += mpAtnNames->rend()->Len() + 1; // Laenge + BYTE Count
            }
            else
            {
                mpAtnNames->push_back(WW8Read_xstz(rStrm, 0, FALSE));
                // UNICode: doppelte Laenge + USHORT Count
                nRead += mpAtnNames->rend()->Len() * 2 + 2;
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
        nIniFtSiz = aVal[ 2 ];
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
            if ( (0xa5dc != nMagic) && (0xa699 != nMagic) )
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


ULONG WW8Reader::Read( SwDoc &rDoc, SwPaM &rPam,
                    const String & /* FileName, falls benoetigt wird */ )
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
        if( pStrm )
            nVersion = 6;
        else
        {
            ASSERT( FALSE, "WinWord 95 Reader-Read ohne Stream" );
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
            ASSERT( FALSE, "WinWord 95/97 Reader-Read ohne Storage" );
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
    return TRUE;
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
    return bRet ? TRUE : FALSE;
}

BOOL SwMSDffManager::GetOLEStorageName( long nOLEId, String& rStorageName,
                                        SvStorageRef& rSrcStorage,
                                        SvStorageRef& rDestStorage ) const
{
    BOOL bRet = FALSE;

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
            pChp->SeekPos( nStartCp );

            WW8_CP nStart = pChp->Where();
            wwSprmParser aSprmParser(rReader.pWwFib->nVersion);
            while( nStart <= nEndCp && !nPictureId )
            {
                WW8PLCFxDesc aDesc;
                pChp->GetSprms( &aDesc );
                (*pChp)++;
                WW8_CP nNextEnd = pChp->Where();

                if( aDesc.nSprmsLen && aDesc.pMemPos )  // Attribut(e) vorhanden
                {
                    long nLen = aDesc.nSprmsLen;
                    const BYTE* pSprm = aDesc.pMemPos;

                    while( nLen >= 2 && !nPictureId )
                    {
                        USHORT nId = aSprmParser.GetSprmId(pSprm);
                        USHORT nSL = aSprmParser.GetSprmSize(nId, pSprm);

                        if( nLen < nSL )
                            break;              // nicht mehr genug Bytes uebrig

                        if( 0x6A03 == nId && 0 < nLen )
                        {
                            nPictureId = SVBT32ToLong(pSprm +
                                aSprmParser.DistanceToData(nId));
                            bRet = TRUE;
                        }
                        pSprm += nSL;
                        nLen -= nSL;
                    }
                }
                nStart = nNextEnd;
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
            bRet=FALSE;
        else
            rDestStorage = pDocShell->GetStorage();
    }
    return bRet;
}

BOOL SwMSDffManager::ShapeHasText(ULONG, ULONG ) const
{
    // Zur Zeit des Einlesens einer einzelnen Box, die womoeglich Teil einer
    // Gruppe ist, liegen noch nicht genuegend Informationen vor, um
    // entscheiden zu koennen, ob wir sie nicht doch als Textfeld benoetigen.
    // Also vorsichtshalber mal alle umwandeln:
    return TRUE;
}
