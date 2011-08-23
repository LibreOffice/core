/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <cmdid.h>

#include <horiornt.hxx>
#include <errhdl.hxx>

#include <unocrsrhelper.hxx>
#include <viscrs.hxx>
#include <unoobj.hxx>
#include <unostyle.hxx>
#include <unomap.hxx>
#include <unomid.h>
#include <unoidx.hxx>
#include <unofield.hxx>
#include <unotbl.hxx>
#include <unosett.hxx>
#include <unocoll.hxx>
#include <unoframe.hxx>
#include <fmtftn.hxx>
#include <fmtpdsc.hxx>
#include <charfmt.hxx>
#include <pagedesc.hxx>
#include <docstyle.hxx>
#include <ndtxt.hxx>
#include <txtrfmrk.hxx>
#include <fmtfld.hxx>
#include <docsh.hxx>
#include <section.hxx>
#include <shellio.hxx>
#include <edimp.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/eitem.hxx>
#include <tools/urlobj.hxx>
#include <docary.hxx>
#include <swtable.hxx>
#include <tox.hxx>
#include <doc.hxx>
#include <fchrfmt.hxx>
#include <bf_svx/flstitem.hxx>
#include <vcl/metric.hxx>
#include <bf_svtools/ctrltool.hxx>
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <bf_svtools/svstdarr.hxx>
#include <bf_sfx2/docfilt.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/fcontnr.hxx>

#include <bf_svtools/stritem.hxx>

#include <com/sun/star/beans/PropertyState.hpp>
#include <SwStyleNameMapper.hxx>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

using rtl::OUString;


namespace SwUnoCursorHelper
{
/* -----------------16.09.98 12:27-------------------
 * 	Lesen spezieller Properties am Cursor
 * --------------------------------------------------*/
sal_Bool getCrsrPropertyValue(const SfxItemPropertyMap* pMap
                                        , SwPaM& rPam
                                        , Any *pAny
                                        , PropertyState& eState
                                        , const SwTxtNode* pNode  )
{
    PropertyState eNewState = PropertyState_DIRECT_VALUE;
//    PropertyState_DEFAULT_VALUE
//    PropertyState_AMBIGUOUS_VALUE
    sal_Bool bDone = sal_True;
    switch(pMap->nWID)
    {
        case FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL:
            if( pAny )
            {
                SwFmtColl* pFmt = 0;
                if(pNode)
                    pFmt = pNode->GetFmtColl();
                else
                    pFmt = SwXTextCursor::GetCurTxtFmtColl(rPam, FALSE);
                sal_Int8 nRet = -1;
                if(pFmt && ((SwTxtFmtColl*)pFmt)->GetOutlineLevel() != NO_NUMBERING)
                    nRet = ((SwTxtFmtColl*)pFmt)->GetOutlineLevel();
                *pAny <<= nRet;
            }
        break;
        case FN_UNO_PARA_CONDITIONAL_STYLE_NAME:
        case FN_UNO_PARA_STYLE :
        {
            SwFmtColl* pFmt = 0;
            if(pNode)
                pFmt = FN_UNO_PARA_CONDITIONAL_STYLE_NAME == pMap->nWID
                            ? pNode->GetFmtColl() : &pNode->GetAnyFmtColl();
            else
                pFmt = SwXTextCursor::GetCurTxtFmtColl(rPam, FN_UNO_PARA_CONDITIONAL_STYLE_NAME == pMap->nWID);
            if(pFmt)
            {
                if( pAny )
                {
                    String sVal;
                    SwStyleNameMapper::FillProgName(pFmt->GetName(), sVal, GET_POOLID_TXTCOLL, sal_True );
                    *pAny <<= OUString(sVal);
                }
            }
            else
                eNewState = PropertyState_AMBIGUOUS_VALUE;
        }
        break;
        case FN_UNO_PAGE_STYLE :
        {
            String sVal;
            GetCurPageStyle(rPam, sVal);
            if( pAny )
                *pAny <<= OUString(sVal);
            if(!sVal.Len())
                eNewState = PropertyState_AMBIGUOUS_VALUE;
        }
        break;
        case FN_UNO_NUM_START_VALUE  :
            if( pAny )
            {
                sal_Int16 nValue = IsNodeNumStart(rPam, eNewState);
                *pAny <<= nValue;
            }
        break;
        case FN_UNO_NUM_LEVEL  :
        case FN_UNO_IS_NUMBER  :
        case FN_NUMBER_NEWSTART:
        {
            const SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
            const SwNumRule* pRule = pTxtNd->GetNumRule();
            // hier wird Multiselektion nicht beruecksichtigt
            if( pRule && pTxtNd->GetNum() )
            {
                if( pAny )
                {
                    if(pMap->nWID == FN_UNO_NUM_LEVEL)
                        *pAny <<= (sal_Int16)(pTxtNd->GetNum()->GetLevel()&~NO_NUMLEVEL);
                    else if(pMap->nWID == FN_UNO_IS_NUMBER)
                    {
                        BOOL bIsNumber = 0 == (pTxtNd->GetNum()->GetLevel() & NO_NUMLEVEL);
                        pAny->setValue(&bIsNumber, ::getBooleanCppuType());
                    }
                    else /*if(pMap->nWID == UNO_NAME_PARA_IS_NUMBERING_RESTART)*/
                    {
                        BOOL bIsRestart = pTxtNd->GetNum()->IsStart();
                        pAny->setValue(&bIsRestart, ::getBooleanCppuType());
                    }
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
            //PROPERTY_MAYBEVOID!
        }
        break;
        case FN_UNO_NUM_RULES  :
            if( pAny )
                getNumberingProperty(rPam, eNewState, pAny);
            else
            {
                if( !rPam.GetDoc()->GetCurrNumRule( *rPam.GetPoint() ) )
                    eNewState = PropertyState_DEFAULT_VALUE;
            }
            break;
        case FN_UNO_DOCUMENT_INDEX_MARK:
        {
            SwTxtAttr* pTxtAttr = rPam.GetNode()->GetTxtNode()->GetTxtAttr(
                                rPam.GetPoint()->nContent, RES_TXTATR_TOXMARK);
            if(pTxtAttr)
            {
                if( pAny )
                {
                    const SwTOXMark& rMark = pTxtAttr->GetTOXMark();
                    Reference< XDocumentIndexMark >  xRef = SwXDocumentIndexMark::GetObject(
                            (SwTOXType*)rMark.GetTOXType(), &rMark, rPam.GetDoc());
                    pAny->setValue(&xRef, ::getCppuType((Reference<XDocumentIndex>*)0));
                }
            }
            else
                //auch hier - nicht zu unterscheiden
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_DOCUMENT_INDEX:
        {
            const SwTOXBase* pBase = rPam.GetDoc()->GetCurTOX(
                                                    *rPam.Start() );
            if( pBase )
            {
                if( pAny )
                {
                    Reference< XDocumentIndex > aRef =
                        SwXDocumentIndexes::GetObject((SwTOXBaseSection*)pBase);
                    pAny->setValue(&aRef, ::getCppuType((Reference<XDocumentIndex>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_FIELD:
        {
            const SwPosition *pPos = rPam.Start();
            const SwTxtNode *pTxtNd =
                rPam.GetDoc()->GetNodes()[pPos->nNode.GetIndex()]->GetTxtNode();
            SwTxtAttr* pTxtAttr =
                pTxtNd ? pTxtNd->GetTxtAttr(pPos->nContent, RES_TXTATR_FIELD)
                       : 0;
            if(pTxtAttr)
            {
                if( pAny )
                {
                    const SwFmtFld& rFld = pTxtAttr->GetFld();
                    SwClientIter aIter(*rFld.GetFld()->GetTyp());
                    SwXTextField* pFld = 0;
                    SwXTextField* pTemp = (SwXTextField*)aIter.First(TYPE(SwXTextField));
                    while(pTemp && !pFld)
                    {
                        if(pTemp->GetFldFmt() == &rFld)
                            pFld = pTemp;
                        pTemp = (SwXTextField*)aIter.Next();
                    }
                    if(!pFld)
                        pFld = new SwXTextField( rFld, rPam.GetDoc());
                    Reference< XTextField >  xRet = pFld;
                    pAny->setValue(&xRet, ::getCppuType((Reference<XTextField>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
/*				laesst sich nicht feststellen
* 				case FN_UNO_BOOKMARK:
        {
            if()
            {
                Reference< XBookmark >  xBkm = SwXBookmarks::GetObject(rBkm);
                rAny.set(&xBkm, ::getCppuType((const XBookmark*)0)());
            }
        }
        break;*/
        case FN_UNO_TEXT_TABLE:
        case FN_UNO_CELL:
        {
            SwStartNode* pSttNode = rPam.GetNode()->FindStartNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();
            if(SwTableBoxStartNode == eType)
            {
                if( pAny )
                {
                    const SwTableNode* pTblNode = pSttNode->FindTableNode();
                    SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
                    SwTable& rTable = ((SwTableNode*)pSttNode)->GetTable();
                    if(FN_UNO_TEXT_TABLE == pMap->nWID)
                    {
                        Reference< XTextTable >  xTable = SwXTextTables::GetObject(*pTableFmt);
                        pAny->setValue(&xTable, ::getCppuType((Reference<XTextTable>*)0));
                    }
                    else
                    {
                        SwTableBox* pBox = pSttNode->GetTblBox();
                        Reference< XCell >  xCell = SwXCell::CreateXCell(pTableFmt, pBox);
                        pAny->setValue(&xCell, ::getCppuType((Reference<XCell>*)0));
                    }
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_FRAME:
        {
            SwStartNode* pSttNode = rPam.GetNode()->FindStartNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();

            SwFrmFmt* pFmt;
            if(eType == SwFlyStartNode && 0 != (pFmt = pSttNode->GetFlyFmt()))
            {
                if( pAny )
                {
                    Reference< XTextFrame >  xFrm = (SwXTextFrame*) SwXFrames::GetObject(*pFmt, FLYCNTTYPE_FRM);
                    pAny->setValue(&xFrm, ::getCppuType((Reference<XTextFrame>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_SECTION:
        {
            SwSection* pSect = rPam.GetDoc()->GetCurrSection(*rPam.GetPoint());
            if(pSect)
            {
                if( pAny )
                {
                    Reference< XTextSection >  xSect = SwXTextSections::GetObject( *pSect->GetFmt() );
                    pAny->setValue(&xSect, ::getCppuType((Reference<XTextSection>*)0) );
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_ENDNOTE:
        case FN_UNO_FOOTNOTE:
        {
            SwTxtAttr* pTxtAttr = rPam.GetNode()->GetTxtNode()->
                                        GetTxtAttr(rPam.GetPoint()->nContent, RES_TXTATR_FTN);
            if(pTxtAttr)
            {
                const SwFmtFtn& rFtn = pTxtAttr->GetFtn();
                if(rFtn.IsEndNote() == (FN_UNO_ENDNOTE == pMap->nWID))
                {
                    if( pAny )
                    {
                        Reference< XFootnote >  xFoot = new SwXFootnote(rPam.GetDoc(), rFtn);
                        pAny->setValue(&xFoot, ::getCppuType((Reference<XFootnote>*)0));
                    }
                }
                else
                    eNewState = PropertyState_DEFAULT_VALUE;
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_REFERENCE_MARK:
        {
            SwTxtAttr* pTxtAttr = rPam.GetNode()->GetTxtNode()->
                                        GetTxtAttr(rPam.GetPoint()->nContent, RES_TXTATR_REFMARK);
            if(pTxtAttr)
            {
                if( pAny )
                {
                    const SwFmtRefMark& rRef = pTxtAttr->GetRefMark();
                    Reference< XTextContent >  xRef = SwXReferenceMarks::GetObject( rPam.GetDoc(), &rRef );
                    pAny->setValue(&xRef, ::getCppuType((Reference<XTextContent>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_CHARFMT_SEQUENCE:
        {

            SwTxtNode* pTxtNode;
            if((pTxtNode = (SwTxtNode*)rPam.GetNode( TRUE )) == rPam.GetNode(FALSE) &&
                    pTxtNode->GetpSwpHints())
            {
                USHORT nPaMStart = rPam.GetPoint()->nContent.GetIndex();
                USHORT nPaMEnd = rPam.GetMark() ? rPam.GetMark()->nContent.GetIndex() : nPaMStart;
                if(nPaMStart > nPaMEnd)
                {
                    USHORT nTmp = nPaMStart;
                    nPaMStart = nPaMEnd;
                    nPaMEnd = nTmp;
                }
                Sequence< ::rtl::OUString> aCharStyles;
                USHORT nCharStylesFound = 0;
                SwpHints* pHints = pTxtNode->GetpSwpHints();
                for(USHORT nAttr = 0; nAttr < pHints->GetStartCount(); nAttr++ )
                {
                    SwTxtAttr* pAttr = pHints->GetStart( nAttr );
                    if(pAttr->Which() != RES_TXTATR_CHARFMT)
                        continue;
                    USHORT nAttrStart = *pAttr->GetStart();
                    USHORT nAttrEnd = *pAttr->GetEnd();
                    //check if the attribute touches the selection
                    if(nAttrEnd > nPaMStart && nAttrStart < nPaMEnd && nAttrEnd > nPaMStart)
                    {
                        //check for overlapping
                        if(nAttrStart > nPaMStart ||
                                    nAttrEnd < nPaMEnd)
                        {
                            aCharStyles.realloc(0);
                            eNewState = PropertyState_AMBIGUOUS_VALUE;
                            break;
                        }
                        else
                        {
                            //now the attribute should start before or at the selection
                            //and it should end at the end of the selection or behind
                            DBG_ASSERT(nAttrStart <= nPaMStart && nAttrEnd >=nPaMEnd,
                                    "attribute overlaps or is outside");
                            //now the name of the style has to be added to the sequence
                            aCharStyles.realloc(aCharStyles.getLength() + 1);
                            DBG_ASSERT(pAttr->GetCharFmt().GetCharFmt(), "no character format set");
                            aCharStyles.getArray()[aCharStyles.getLength() - 1] =
                                        SwStyleNameMapper::GetProgName(
                                            pAttr->GetCharFmt().GetCharFmt()->GetName(), GET_POOLID_CHRFMT);
                        }
                    }

                }
                if(aCharStyles.getLength())
                    eNewState = PropertyState_DIRECT_VALUE;
                if(pAny)
                    (*pAny) <<= aCharStyles;
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case RES_TXTATR_CHARFMT:
        // kein break hier!
        default: bDone = sal_False;
    }
    if( bDone )
        eState = eNewState;
    return bDone;
};
/* -----------------30.06.98 10:30-------------------
 *
 * --------------------------------------------------*/
sal_Int16 IsNodeNumStart(SwPaM& rPam, PropertyState& eState)
{
    const SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
    if( pTxtNd && pTxtNd->GetNum() && pTxtNd->GetNumRule() )
    {
        eState = PropertyState_DIRECT_VALUE;
        sal_uInt16 nTmp = pTxtNd->GetNum()->GetSetValue();
        return USHRT_MAX == nTmp ? -1 : (sal_Int16)nTmp;
    }
    eState = PropertyState_DEFAULT_VALUE;
    return -1;
}

/* -----------------25.05.98 11:41-------------------
 *
 * --------------------------------------------------*/
void setNumberingProperty(const Any& rValue, SwPaM& rPam)
{
    Reference<XIndexReplace> xIndexReplace;
    if(rValue >>= xIndexReplace)
    {
        SwXNumberingRules* pSwNum = 0;

        Reference<XUnoTunnel> xNumTunnel(xIndexReplace, UNO_QUERY);
        if(xNumTunnel.is())
        {
            pSwNum = (SwXNumberingRules*)
                xNumTunnel->getSomething( SwXNumberingRules::getUnoTunnelId() );
        }

        if(pSwNum)
        {
            if(pSwNum->GetNumRule())
            {
                SwDoc* pDoc = rPam.GetDoc();
                SwNumRule aRule(*pSwNum->GetNumRule());
                const String* pNewCharStyles =  pSwNum->GetNewCharStyleNames();
                const String* pBulletFontNames = pSwNum->GetBulletFontNames();
                for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                {
                    SwNumFmt aFmt(aRule.Get( i ));
                    if( pNewCharStyles[i].Len() &&
                        pNewCharStyles[i] != SwXNumberingRules::GetInvalidStyle() &&
                        (!aFmt.GetCharFmt() || pNewCharStyles[i] != aFmt.GetCharFmt()->GetName()))
                    {
                        if(!pNewCharStyles[i].Len())
                            aFmt.SetCharFmt(0);
                        else
                        {

                            // CharStyle besorgen und an der chaos::Rule setzen
                            sal_uInt16 nChCount = pDoc->GetCharFmts()->Count();
                            SwCharFmt* pCharFmt = 0;
                            for(sal_uInt16 i = 0; i< nChCount; i++)
                            {
                                SwCharFmt& rChFmt = *((*(pDoc->GetCharFmts()))[i]);;
                                if(rChFmt.GetName() == pNewCharStyles[i])
                                {
                                    pCharFmt = &rChFmt;
                                    break;
                                }
                            }

                            if(!pCharFmt)
                            {
                                SfxStyleSheetBasePool* pPool = pDoc->GetDocShell()->GetStyleSheetPool();
                                SfxStyleSheetBase* pBase;
                                pBase = pPool->Find(pNewCharStyles[i], SFX_STYLE_FAMILY_CHAR);
                            // soll das wirklich erzeugt werden?
                                if(!pBase)
                                    pBase = &pPool->Make(pNewCharStyles[i], SFX_STYLE_FAMILY_PAGE);
                                pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();
                            }
                            if(pCharFmt)
                                aFmt.SetCharFmt(pCharFmt);
                        }
                    }
                    //jetzt nochmal fuer Fonts
                    if(pBulletFontNames[i] != SwXNumberingRules::GetInvalidStyle() &&
                        ((pBulletFontNames[i].Len() && !aFmt.GetBulletFont()) ||
                        pBulletFontNames[i].Len() &&
                                aFmt.GetBulletFont()->GetName() != pBulletFontNames[i] ))
                    {
                        const SvxFontListItem* pFontListItem =
                                (const SvxFontListItem* )pDoc->GetDocShell()
                                                    ->GetItem( SID_ATTR_CHAR_FONTLIST );
                        const FontList*	 pList = pFontListItem->GetFontList();

                        FontInfo aInfo = pList->Get(
                            pBulletFontNames[i],WEIGHT_NORMAL, ITALIC_NONE);
                        Font aFont(aInfo);
                        aFmt.SetBulletFont(&aFont);
                    }
                    aRule.Set( i, aFmt );
                }
                UnoActionContext aAction(pDoc);

                if( rPam.GetNext() != &rPam )           // Mehrfachselektion ?
                {
                    SwPamRanges aRangeArr( rPam );
                    SwPaM aPam( *rPam.GetPoint() );
                    for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
                        pDoc->SetNumRule( aRangeArr.SetPam( n, aPam ), aRule );
                }
                else
                    pDoc->SetNumRule( rPam, aRule );


            }
            else if(pSwNum->GetCreatedNumRuleName().Len())
            {
                SwDoc* pDoc = rPam.GetDoc();
                UnoActionContext aAction(pDoc);
                SwNumRule* pRule = pDoc->FindNumRulePtr( pSwNum->GetCreatedNumRuleName() );
                if(!pRule)
                    throw RuntimeException();
                pDoc->SetNumRule( rPam, *pRule );
            }
        }
    }
    else if(rValue.getValueType() == ::getVoidCppuType())
    {
        rPam.GetDoc()->DelNumRules(rPam);
    }
}


/* -----------------25.05.98 11:40-------------------
 *
 * --------------------------------------------------*/
void  getNumberingProperty(SwPaM& rPam, PropertyState& eState, Any * pAny )
{
    const SwNumRule* pNumRule = rPam.GetDoc()->GetCurrNumRule( *rPam.GetPoint() );
    if(pNumRule)
    {
        Reference< XIndexReplace >  xNum = new SwXNumberingRules(*pNumRule);
        if ( pAny )
            pAny->setValue(&xNum, ::getCppuType((const Reference<XIndexReplace>*)0));
        eState = PropertyState_DIRECT_VALUE;
    }
    else
        eState = PropertyState_DEFAULT_VALUE;
}
/* -----------------04.07.98 15:15-------------------
 *
 * --------------------------------------------------*/
void GetCurPageStyle(SwPaM& rPaM, String &rString)
{
    const SwPageFrm* pPage = rPaM.GetCntntNode()->GetFrm()->FindPageFrm();
    if(pPage)
        SwStyleNameMapper::FillProgName( pPage->GetPageDesc()->GetName(), rString, GET_POOLID_PAGEDESC, sal_True );
}
/* -----------------30.03.99 10:52-------------------
 * spezielle Properties am Cursor zuruecksetzen
 * --------------------------------------------------*/
void resetCrsrPropertyValue(const SfxItemPropertyMap* pMap, SwPaM& rPam)
{
    SwDoc* pDoc = rPam.GetDoc();
    switch(pMap->nWID)
    {
        case FN_UNO_PARA_STYLE :
//			lcl_SetTxtFmtColl(aValue, pUnoCrsr);
        break;
        case FN_UNO_PAGE_STYLE :
        break;
        case FN_UNO_NUM_START_VALUE  :
        {
            UnoActionContext aAction(pDoc);

            if( rPam.GetNext() != &rPam )			// Mehrfachselektion ?
            {
                SwPamRanges aRangeArr( rPam );
                SwPaM aPam( *rPam.GetPoint() );
                for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
                    pDoc->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), 1 );
            }
            else
                pDoc->SetNodeNumStart( *rPam.GetPoint(), 0 );
        }

        break;
        case FN_UNO_NUM_LEVEL  :
        break;
        case FN_UNO_NUM_RULES:
//    		lcl_setNumberingProperty(aValue, pUnoCrsr);
        break;
        case FN_UNO_CHARFMT_SEQUENCE:
        {
            SvUShortsSort aWhichIds;
            aWhichIds.Insert(RES_TXTATR_CHARFMT);
            pDoc->ResetAttr(rPam, sal_True, &aWhichIds);
        }
        break;
    }
}
/* -----------------21.07.98 11:36-------------------
 *
 * --------------------------------------------------*/
void InsertFile(SwUnoCrsr* pUnoCrsr,
    const String& rFileName,
    const String& rFilterName,
    const String& rFilterOption,
    const String& rPassword)
{
    SfxMedium* pMed = 0;
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if(!pDocSh || !rFileName.Len())
        return;

    SfxObjectFactory& rFact = pDocSh->GetFactory();
    const SfxFilter* pFilter = rFact.GetFilterContainer()->GetFilter( rFilterName );
    if ( !pFilter )
    {
        pMed = new SfxMedium(rFileName, STREAM_READ, sal_True, 0, 0 );
        SfxFilterMatcher aMatcher( rFact.GetFilterContainer() );
        ErrCode nErr = aMatcher.GuessFilter( *pMed, &pFilter, sal_False );
        if ( nErr || !pFilter)
            DELETEZ(pMed);
        else
            pMed->SetFilter( pFilter );
    }
    else
    {
        pMed = new SfxMedium(rFileName, STREAM_READ, sal_True,
            pFilter, 0);
        if(rFilterOption.Len())
        {
            SfxItemSet* pSet = 	pMed->GetItemSet();
            SfxStringItem aOptionItem(SID_FILE_FILTEROPTIONS, rFilterOption);
            pSet->Put(aOptionItem);
        }
    }

    if( !pMed )
        return;

    SfxObjectShellRef aRef( pDocSh );

    pDocSh->RegisterTransfer( *pMed );
    pMed->DownLoad();	// ggfs. den DownLoad anstossen
    if( aRef.Is() && 1 < aRef->GetRefCount() )	// noch gueltige Ref?
    {
        SwReader* pRdr;
        SfxItemSet* pSet = 	pMed->GetItemSet();
        pSet->Put(SfxBoolItem(FN_API_CALL, sal_True));
        if(rPassword.Len())
            pSet->Put(SfxStringItem(SID_PASSWORD, rPassword));
        Reader *pRead = pDocSh->StartConvertFrom( *pMed, &pRdr, 0, pUnoCrsr);
        if( pRead )
        {
            String sTmpBaseURL( ::binfilter::StaticBaseUrl::GetBaseURL() );
            ::binfilter::StaticBaseUrl::SetBaseURL( pMed->GetName() );

            UnoActionContext aContext(pDoc);

            if(pUnoCrsr->HasMark())
                pDoc->DeleteAndJoin(*pUnoCrsr);

            SwNodeIndex aSave(  pUnoCrsr->GetPoint()->nNode, -1 );
            xub_StrLen nCntnt = pUnoCrsr->GetPoint()->nContent.GetIndex();

            sal_uInt32 nErrno = pRdr->Read( *pRead );	// und Dokument einfuegen

            if(!nErrno)
            {
                aSave++;
                pUnoCrsr->SetMark();
                pUnoCrsr->GetMark()->nNode = aSave;

                SwCntntNode* pCntNode = aSave.GetNode().GetCntntNode();
                if( !pCntNode )
                    nCntnt = 0;
                pUnoCrsr->GetMark()->nContent.Assign( pCntNode, nCntnt );
            }

            delete pRdr;

            ::binfilter::StaticBaseUrl::SetBaseURL( sTmpBaseURL );

            // ggfs. alle Verzeichnisse updaten:
/*			if( pWrtShell->IsUpdateTOX() )
            {
                SfxRequest aReq( *this, FN_UPDATE_TOX );
                Execute( aReq );
                pWrtShell->SetUpdateTOX( sal_False );		// wieder zurueck setzen
            }*/

        }
    }
    delete pMed;
}

}//namespace SwUnoCursorHelper

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
