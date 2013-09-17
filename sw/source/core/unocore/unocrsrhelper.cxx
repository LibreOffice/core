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

#include <unocrsrhelper.hxx>

#include <map>

#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/text/XTextSection.hpp>

#include <svx/svxids.hrc>
#include <svx/unoshape.hxx>

#include <cmdid.h>
#include <unotextrange.hxx>
#include <unodraw.hxx>
#include <unofootnote.hxx>
#include <unobookmark.hxx>
#include <unorefmark.hxx>
#include <unostyle.hxx>
#include <unoidx.hxx>
#include <unofield.hxx>
#include <unotbl.hxx>
#include <unosett.hxx>
#include <unoframe.hxx>
#include <unocrsr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
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
#include <swundo.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <svl/eitem.hxx>
#include <docary.hxx>
#include <swtable.hxx>
#include <tox.hxx>
#include <doctxm.hxx>
#include <fchrfmt.hxx>
#include <editeng/flstitem.hxx>
#include <vcl/metric.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <svl/stritem.hxx>
#include <SwStyleNameMapper.hxx>
#include <redline.hxx>
#include <numrule.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <SwNodeNum.hxx>
#include <fmtmeta.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace SwUnoCursorHelper
{

static SwPaM* lcl_createPamCopy(const SwPaM& rPam)
{
    SwPaM *const pRet = new SwPaM(*rPam.GetPoint());
    ::sw::DeepCopyPaM(rPam, *pRet);
    return pRet;
}

void GetSelectableFromAny(uno::Reference<uno::XInterface> const& xIfc,
        SwDoc & rTargetDoc,
        SwPaM *& o_rpPaM, std::pair<OUString, FlyCntType> & o_rFrame,
        OUString & o_rTableName, SwUnoTableCrsr const*& o_rpTableCursor,
        ::sw::mark::IMark const*& o_rpMark,
        std::vector<SdrObject *> & o_rSdrObjects)
{
    uno::Reference<drawing::XShapes> const xShapes(xIfc, UNO_QUERY);
    if (xShapes.is())
    {
        sal_Int32 nShapes(xShapes->getCount());
        for (sal_Int32 i = 0; i < nShapes; ++i)
        {
            uno::Reference<lang::XUnoTunnel> xShape;
            xShapes->getByIndex(i) >>= xShape;
            if (xShape.is())
            {
                SvxShape *const pSvxShape(
                        ::sw::UnoTunnelGetImplementation<SvxShape>(xShape));
                if (pSvxShape)
                {
                    SdrObject *const pSdrObject = pSvxShape->GetSdrObject();
                    if (pSdrObject)
                    {   // hmm... needs view to verify it's in right doc...
                        o_rSdrObjects.push_back(pSdrObject);
                    }
                }
            }
        }
        return;
    }

    uno::Reference<lang::XUnoTunnel> const xTunnel(xIfc, UNO_QUERY);
    if (!xTunnel.is()) // everything below needs tunnel
    {
        return;
    }

    SwXShape *const pShape(::sw::UnoTunnelGetImplementation<SwXShape>(xTunnel));
    if (pShape)
    {
        uno::Reference<uno::XAggregation> const xAgg(
                pShape->GetAggregationInterface());
        if (xAgg.is())
        {
            SvxShape *const pSvxShape(
                    ::sw::UnoTunnelGetImplementation<SvxShape>(xTunnel));
            if (pSvxShape)
            {
                SdrObject *const pSdrObject = pSvxShape->GetSdrObject();
                if (pSdrObject)
                {   // hmm... needs view to verify it's in right doc...
                    o_rSdrObjects.push_back(pSdrObject);
                }
            }
        }
        return;
    }

    OTextCursorHelper *const pCursor(
        ::sw::UnoTunnelGetImplementation<OTextCursorHelper>(xTunnel));
    if (pCursor)
    {
        if (pCursor->GetDoc() == &rTargetDoc)
        {
            o_rpPaM = lcl_createPamCopy(*pCursor->GetPaM());
        }
        return;
    }

    SwXTextRanges *const pRanges(
        ::sw::UnoTunnelGetImplementation<SwXTextRanges>(xTunnel));
    if (pRanges)
    {
        SwUnoCrsr const* pUnoCrsr = pRanges->GetCursor();
        if (pUnoCrsr && pUnoCrsr->GetDoc() == &rTargetDoc)
        {
            o_rpPaM = lcl_createPamCopy(*pUnoCrsr);
        }
        return;
    }

    // check these before Range to prevent misinterpretation of text frames
    // and cells also implement XTextRange
    SwXFrame *const pFrame(
        ::sw::UnoTunnelGetImplementation<SwXFrame>(xTunnel));
    if (pFrame)
    {
        SwFrmFmt *const pFrmFmt(pFrame->GetFrmFmt());
        if (pFrmFmt && pFrmFmt->GetDoc() == &rTargetDoc)
        {
            o_rFrame = std::make_pair(pFrmFmt->GetName(), pFrame->GetFlyCntType());
        }
        return;
    }

    SwXTextTable *const pTextTable(
        ::sw::UnoTunnelGetImplementation<SwXTextTable>(xTunnel));
    if (pTextTable)
    {
        SwFrmFmt *const pFrmFmt(pTextTable->GetFrmFmt());
        if (pFrmFmt && pFrmFmt->GetDoc() == &rTargetDoc)
        {
            o_rTableName = pFrmFmt->GetName();
        }
        return;
    }

    SwXCell *const pCell(
        ::sw::UnoTunnelGetImplementation<SwXCell>(xTunnel));
    if (pCell)
    {
        SwFrmFmt *const pFrmFmt(pCell->GetFrmFmt());
        if (pFrmFmt && pFrmFmt->GetDoc() == &rTargetDoc)
        {
            SwTableBox * pBox = pCell->GetTblBox();
            SwTable *const pTable = SwTable::FindTable(pFrmFmt);
            // ??? what's the benefit of setting pBox in this convoluted way?
            pBox = pCell->FindBox(pTable, pBox);
            if (pBox)
            {
                SwPosition const aPos(*pBox->GetSttNd());
                SwPaM aPam(aPos);
                aPam.Move(fnMoveForward, fnGoNode);
                o_rpPaM = lcl_createPamCopy(aPam);
            }
        }
        return;
    }

    uno::Reference<text::XTextRange> const xTextRange(xTunnel, UNO_QUERY);
    if (xTextRange.is())
    {
        SwUnoInternalPaM aPam(rTargetDoc);
        if (::sw::XTextRangeToSwPaM(aPam, xTextRange))
        {
            o_rpPaM = lcl_createPamCopy(aPam);
        }
        return;
    }

    SwXCellRange *const pCellRange(
        ::sw::UnoTunnelGetImplementation<SwXCellRange>(xTunnel));
    if (pCellRange)
    {
        SwUnoCrsr const*const pUnoCrsr(pCellRange->GetTblCrsr());
        if (pUnoCrsr && pUnoCrsr->GetDoc() == &rTargetDoc)
        {
            // probably can't copy it to o_rpPaM for this since it's
            // a SwTableCursor
            o_rpTableCursor = dynamic_cast<SwUnoTableCrsr const*>(pUnoCrsr);
        }
        return;
    }

    ::sw::mark::IMark const*const pMark(
            SwXBookmark::GetBookmarkInDoc(& rTargetDoc, xTunnel));
    if (pMark)
    {
        o_rpMark = pMark;
        return;
    }
}

uno::Reference<text::XTextContent>
GetNestedTextContent(SwTxtNode & rTextNode, xub_StrLen const nIndex,
        bool const bParent)
{
    // these should be unambiguous because of the dummy character
    SwTxtNode::GetTxtAttrMode const eMode( (bParent)
        ? SwTxtNode::PARENT : SwTxtNode::EXPAND );
    SwTxtAttr *const pMetaTxtAttr =
        rTextNode.GetTxtAttrAt(nIndex, RES_TXTATR_META, eMode);
    SwTxtAttr *const pMetaFieldTxtAttr =
        rTextNode.GetTxtAttrAt(nIndex, RES_TXTATR_METAFIELD, eMode);
    // which is innermost?
    SwTxtAttr *const pTxtAttr = (pMetaTxtAttr)
        ? ((pMetaFieldTxtAttr)
            ? ((*pMetaFieldTxtAttr->GetStart() >
                    *pMetaTxtAttr->GetStart())
                ? pMetaFieldTxtAttr : pMetaTxtAttr)
            : pMetaTxtAttr)
        : pMetaFieldTxtAttr;
    uno::Reference<XTextContent> xRet;
    if (pTxtAttr)
    {
        ::sw::Meta *const pMeta(
            static_cast<SwFmtMeta &>(pTxtAttr->GetAttr()).GetMeta());
        OSL_ASSERT(pMeta);
        xRet.set(pMeta->MakeUnoObject(), uno::UNO_QUERY);
    }
    return xRet;
}

/* --------------------------------------------------
 *  Read the special properties of the cursor
 * --------------------------------------------------*/
bool getCrsrPropertyValue(const SfxItemPropertySimpleEntry& rEntry
                                        , SwPaM& rPam
                                        , Any *pAny
                                        , PropertyState& eState
                                        , const SwTxtNode* pNode  )
{
    PropertyState eNewState = PropertyState_DIRECT_VALUE;
//    PropertyState_DEFAULT_VALUE
//    PropertyState_AMBIGUOUS_VALUE
    bool bDone = true;
    switch(rEntry.nWID)
    {
        case FN_UNO_PARA_CONT_PREV_SUBTREE:
            if (pAny)
            {
                const SwTxtNode * pTmpNode = pNode;

                if (!pTmpNode)
                    pTmpNode = rPam.GetNode()->GetTxtNode();

                bool bRet = false;

                if ( pTmpNode &&
                     pTmpNode->GetNum() &&
                     pTmpNode->GetNum()->IsContinueingPreviousSubTree() )
                {
                    bRet = true;
                }

                *pAny <<= bRet;
            }
        break;
        case FN_UNO_PARA_NUM_STRING:
            if (pAny)
            {
                const SwTxtNode * pTmpNode = pNode;

                if (!pTmpNode)
                    pTmpNode = rPam.GetNode()->GetTxtNode();

                String sRet;
                if ( pTmpNode && pTmpNode->GetNum() )
                {
                    sRet = pTmpNode->GetNumString();
                }

                *pAny <<= OUString(sRet);
            }
        break;
        case RES_PARATR_OUTLINELEVEL:
            if (pAny)
            {
                const SwTxtNode * pTmpNode = pNode;

                if (!pTmpNode)
                    pTmpNode = rPam.GetNode()->GetTxtNode();

                sal_Int16 nRet = -1;
                if ( pTmpNode )
                    nRet = sal::static_int_cast< sal_Int16 >( pTmpNode->GetAttrOutlineLevel() );

                *pAny <<= nRet;
            }
        break;
        case FN_UNO_PARA_CONDITIONAL_STYLE_NAME:
        case FN_UNO_PARA_STYLE :
        {
            SwFmtColl* pFmt = 0;
            if(pNode)
                pFmt = FN_UNO_PARA_CONDITIONAL_STYLE_NAME == rEntry.nWID
                            ? pNode->GetFmtColl() : &pNode->GetAnyFmtColl();
            else
            {
                pFmt = SwUnoCursorHelper::GetCurTxtFmtColl(rPam,
                        FN_UNO_PARA_CONDITIONAL_STYLE_NAME == rEntry.nWID);
            }
            if(pFmt)
            {
                if( pAny )
                {
                    OUString sVal;
                    SwStyleNameMapper::FillProgName(pFmt->GetName(), sVal, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, true );
                    *pAny <<= sVal;
                }
            }
            else
                eNewState = PropertyState_AMBIGUOUS_VALUE;
        }
        break;
        case FN_UNO_PAGE_STYLE :
        {
            OUString sVal;
            GetCurPageStyle(rPam, sVal);
            if( pAny )
                *pAny <<= sVal;
            if(sVal.isEmpty())
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
        // #i91601#
        case FN_UNO_LIST_ID:
        case FN_NUMBER_NEWSTART:
        {
            // a multi selection is not considered
            const SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
            if ( pTxtNd && pTxtNd->IsInList() )
            {
                if( pAny )
                {
                    if(rEntry.nWID == FN_UNO_NUM_LEVEL)
                        *pAny <<= (sal_Int16)(pTxtNd->GetActualListLevel());
                    else if(rEntry.nWID == FN_UNO_IS_NUMBER)
                    {
                        sal_Bool bIsNumber = pTxtNd->IsCountedInList();
                        pAny->setValue(&bIsNumber, ::getBooleanCppuType());
                    }
                    // #i91601#
                    else if ( rEntry.nWID == FN_UNO_LIST_ID )
                    {
                        const String sListId = pTxtNd->GetListId();
                        *pAny <<= OUString(sListId);
                    }
                    else /*if(rEntry.nWID == UNO_NAME_PARA_IS_NUMBERING_RESTART)*/
                    {
                        sal_Bool bIsRestart = pTxtNd->IsListRestart();
                        pAny->setValue(&bIsRestart, ::getBooleanCppuType());
                    }
                }
            }
            else
            {
                eNewState = PropertyState_DEFAULT_VALUE;

                if( pAny )
                {
                    // #i30838# set default values for default properties
                    if(rEntry.nWID == FN_UNO_NUM_LEVEL)
                        *pAny <<= static_cast<sal_Int16>( 0 );
                    else if(rEntry.nWID == FN_UNO_IS_NUMBER)
                        *pAny <<= false;
                    // #i91601#
                    else if ( rEntry.nWID == FN_UNO_LIST_ID )
                    {
                        *pAny <<= OUString();
                    }
                    else /*if(rEntry.nWID == UNO_NAME_PARA_IS_NUMBERING_RESTART)*/
                        *pAny <<= false;
                }
            }
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
            ::std::vector<SwTxtAttr *> marks;
            if (rPam.GetNode()->IsTxtNode())
            {
                marks = rPam.GetNode()->GetTxtNode()->GetTxtAttrsAt(
                    rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_TOXMARK);
            }
            if (marks.size())
            {
                if( pAny )
                {   // hmm... can only return 1 here
                    SwTOXMark & rMark =
                        static_cast<SwTOXMark &>((*marks.begin())->GetAttr());
                    const uno::Reference< text::XDocumentIndexMark > xRef =
                        SwXDocumentIndexMark::CreateXDocumentIndexMark(
                            *rPam.GetDoc(),
                            *const_cast<SwTOXType*>(rMark.GetTOXType()), rMark);
                    (*pAny) <<= xRef;
                }
            }
            else
                //also here - indistinguishable
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
                    const uno::Reference< text::XDocumentIndex > xRef =
                        SwXDocumentIndex::CreateXDocumentIndex(*rPam.GetDoc(),
                            *static_cast<SwTOXBaseSection const*>(pBase));
                    (*pAny) <<= xRef;
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
            SwTxtAttr *const pTxtAttr = (pTxtNd)
                ? pTxtNd->GetTxtAttrForCharAt(
                        pPos->nContent.GetIndex(), RES_TXTATR_FIELD)
                : 0;
            if(pTxtAttr)
            {
                if( pAny )
                {
                    uno::Reference<text::XTextField> const xField(
                        SwXTextField::CreateXTextField(*rPam.GetDoc(),
                           pTxtAttr->GetFld()));
                    *pAny <<= xField;
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_TABLE:
        case FN_UNO_CELL:
        {
            SwStartNode* pSttNode = rPam.GetNode()->StartOfSectionNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();
            if(SwTableBoxStartNode == eType)
            {
                if( pAny )
                {
                    const SwTableNode* pTblNode = pSttNode->FindTableNode();
                    SwFrmFmt* pTableFmt = (SwFrmFmt*)pTblNode->GetTable().GetFrmFmt();
                    //SwTable& rTable = ((SwTableNode*)pSttNode)->GetTable();
                    if(FN_UNO_TEXT_TABLE == rEntry.nWID)
                    {
                        uno::Reference< XTextTable >  xTable = SwXTextTables::GetObject(*pTableFmt);
                        pAny->setValue(&xTable, ::getCppuType((uno::Reference<XTextTable>*)0));
                    }
                    else
                    {
                        SwTableBox* pBox = pSttNode->GetTblBox();
                        uno::Reference< XCell >  xCell = SwXCell::CreateXCell(pTableFmt, pBox);
                        pAny->setValue(&xCell, ::getCppuType((uno::Reference<XCell>*)0));
                    }
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_FRAME:
        {
            SwStartNode* pSttNode = rPam.GetNode()->StartOfSectionNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();

            SwFrmFmt* pFmt;
            if(eType == SwFlyStartNode && 0 != (pFmt = pSttNode->GetFlyFmt()))
            {
                if( pAny )
                {
                    uno::Reference< XTextFrame >  xFrm = (SwXTextFrame*) SwXFrames::GetObject(*pFmt, FLYCNTTYPE_FRM);
                    pAny->setValue(&xFrm, ::getCppuType((uno::Reference<XTextFrame>*)0));
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
                    uno::Reference< XTextSection >  xSect = SwXTextSections::GetObject( *pSect->GetFmt() );
                    pAny->setValue(&xSect, ::getCppuType((uno::Reference<XTextSection>*)0) );
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_ENDNOTE:
        case FN_UNO_FOOTNOTE:
        {
            SwTxtAttr *const pTxtAttr = rPam.GetNode()->IsTxtNode() ?
                rPam.GetNode()->GetTxtNode()->GetTxtAttrForCharAt(
                    rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN) : 0;
            if(pTxtAttr)
            {
                const SwFmtFtn& rFtn = pTxtAttr->GetFtn();
                if(rFtn.IsEndNote() == (FN_UNO_ENDNOTE == rEntry.nWID))
                {
                    if( pAny )
                    {
                        const uno::Reference< text::XFootnote > xFootnote =
                            SwXFootnote::CreateXFootnote(*rPam.GetDoc(), rFtn);
                        *pAny <<= xFootnote;
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
            ::std::vector<SwTxtAttr *> marks;
            if (rPam.GetNode()->IsTxtNode())
            {
                marks = (
                rPam.GetNode()->GetTxtNode()->GetTxtAttrsAt(
                    rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_REFMARK));
            }
            if (marks.size())
            {
                if( pAny )
                {   // hmm... can only return 1 here
                    const SwFmtRefMark& rRef = (*marks.begin())->GetRefMark();
                    uno::Reference< XTextContent >  xRef = SwXReferenceMarks::GetObject( rPam.GetDoc(), &rRef );
                    pAny->setValue(&xRef, ::getCppuType((uno::Reference<XTextContent>*)0));
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_NESTED_TEXT_CONTENT:
        {
            uno::Reference<XTextContent> const xRet(rPam.GetNode()->IsTxtNode()
                ? GetNestedTextContent(*rPam.GetNode()->GetTxtNode(),
                    rPam.GetPoint()->nContent.GetIndex(), false)
                : 0);
            if (xRet.is())
            {
                if (pAny)
                {
                    (*pAny) <<= xRet;
                }
            }
            else
            {
                eNewState = PropertyState_DEFAULT_VALUE;
            }
        }
        break;
        case FN_UNO_CHARFMT_SEQUENCE:
        {

            SwTxtNode *const pTxtNode = rPam.GetNode()->GetTxtNode();
            if (rPam.GetNode(sal_True) == rPam.GetNode(sal_False)
                && pTxtNode && pTxtNode->GetpSwpHints())
            {
                sal_uInt16 nPaMStart = rPam.GetPoint()->nContent.GetIndex();
                sal_uInt16 nPaMEnd = rPam.GetMark() ? rPam.GetMark()->nContent.GetIndex() : nPaMStart;
                if(nPaMStart > nPaMEnd)
                {
                    sal_uInt16 nTmp = nPaMStart;
                    nPaMStart = nPaMEnd;
                    nPaMEnd = nTmp;
                }
                Sequence< OUString> aCharStyles;
                SwpHints* pHints = pTxtNode->GetpSwpHints();
                for(sal_uInt16 nAttr = 0; nAttr < pHints->GetStartCount(); nAttr++ )
                {
                    SwTxtAttr* pAttr = pHints->GetStart( nAttr );
                    if(pAttr->Which() != RES_TXTATR_CHARFMT)
                        continue;
                    sal_uInt16 nAttrStart = *pAttr->GetStart();
                    sal_uInt16 nAttrEnd = *pAttr->GetEnd();
                    //check if the attribute touches the selection
                    if( ( nAttrEnd > nPaMStart && nAttrStart < nPaMEnd ) ||
                        ( !nAttrStart && !nAttrEnd && !nPaMStart && !nPaMEnd ) )
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
                            OSL_ENSURE(nAttrStart <= nPaMStart && nAttrEnd >=nPaMEnd,
                                    "attribute overlaps or is outside");
                            //now the name of the style has to be added to the sequence
                            aCharStyles.realloc(aCharStyles.getLength() + 1);
                            OSL_ENSURE(pAttr->GetCharFmt().GetCharFmt(), "no character format set");
                            aCharStyles.getArray()[aCharStyles.getLength() - 1] =
                                        SwStyleNameMapper::GetProgName(
                                            pAttr->GetCharFmt().GetCharFmt()->GetName(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
                        }
                    }

                }
                eNewState =
                    aCharStyles.getLength() ?
                        PropertyState_DIRECT_VALUE : PropertyState_DEFAULT_VALUE;
                if(pAny)
                    (*pAny) <<= aCharStyles;
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case RES_TXTATR_CHARFMT:
        // no break here!
        default: bDone = false;
    }
    if( bDone )
        eState = eNewState;
    return bDone;
};

sal_Int16 IsNodeNumStart(SwPaM& rPam, PropertyState& eState)
{
    const SwTxtNode* pTxtNd = rPam.GetNode()->GetTxtNode();
    // correction: check, if restart value is set at the text node and use
    // new method <SwTxtNode::GetAttrListRestartValue()> to retrieve the value
    if ( pTxtNd && pTxtNd->GetNumRule() && pTxtNd->IsListRestart() &&
         pTxtNd->HasAttrListRestartValue() )
    {
        eState = PropertyState_DIRECT_VALUE;
        sal_Int16 nTmp = sal::static_int_cast< sal_Int16 >(pTxtNd->GetAttrListRestartValue());
        return nTmp;
    }
    eState = PropertyState_DEFAULT_VALUE;
    return -1;
}

void setNumberingProperty(const Any& rValue, SwPaM& rPam)
{
    uno::Reference<XIndexReplace> xIndexReplace;
    if(rValue >>= xIndexReplace)
    {
        SwXNumberingRules* pSwNum = 0;

        uno::Reference<XUnoTunnel> xNumTunnel(xIndexReplace, UNO_QUERY);
        if(xNumTunnel.is())
        {
            pSwNum = reinterpret_cast< SwXNumberingRules * >(
                sal::static_int_cast< sal_IntPtr >( xNumTunnel->getSomething( SwXNumberingRules::getUnoTunnelId() )));
        }

        if(pSwNum)
        {
            SwDoc* pDoc = rPam.GetDoc();
            if(pSwNum->GetNumRule())
            {
                SwNumRule aRule(*pSwNum->GetNumRule());
                const OUString* pNewCharStyles =  pSwNum->GetNewCharStyleNames();
                const OUString* pBulletFontNames = pSwNum->GetBulletFontNames();
                for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                {
                    SwNumFmt aFmt(aRule.Get( i ));
                    if (!pNewCharStyles[i].isEmpty() &&
                        !SwXNumberingRules::isInvalidStyle(pNewCharStyles[i]) &&
                        (!aFmt.GetCharFmt() || pNewCharStyles[i] != aFmt.GetCharFmt()->GetName()))
                    {
                        if (pNewCharStyles[i].isEmpty())
                        {
                            // FIXME
                            // Is something missing/wrong here?
                            // if condition is always false due to outer check!
                            aFmt.SetCharFmt(0);
                        }
                        else
                        {

                            // get CharStyle and set the rule
                            sal_uInt16 nChCount = pDoc->GetCharFmts()->size();
                            SwCharFmt* pCharFmt = 0;
                            for(sal_uInt16 nCharFmt = 0; nCharFmt < nChCount; nCharFmt++)
                            {
                                SwCharFmt& rChFmt = *((*(pDoc->GetCharFmts()))[nCharFmt]);
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
                            // shall it really be created?
                                if(!pBase)
                                    pBase = &pPool->Make(pNewCharStyles[i], SFX_STYLE_FAMILY_PAGE);
                                pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();
                            }
                            if(pCharFmt)
                                aFmt.SetCharFmt(pCharFmt);
                        }
                    }
                    //Now again for fonts
                    if(
                       !pBulletFontNames[i].isEmpty() &&
                       !SwXNumberingRules::isInvalidStyle(pBulletFontNames[i]) &&
                       (!aFmt.GetBulletFont() || aFmt.GetBulletFont()->GetName() != pBulletFontNames[i])
                      )
                    {
                        const SvxFontListItem* pFontListItem =
                                (const SvxFontListItem* )pDoc->GetDocShell()
                                                    ->GetItem( SID_ATTR_CHAR_FONTLIST );
                        const FontList*  pList = pFontListItem->GetFontList();

                        FontInfo aInfo = pList->Get(
                            pBulletFontNames[i],WEIGHT_NORMAL, ITALIC_NONE);
                        Font aFont(aInfo);
                        aFmt.SetBulletFont(&aFont);
                    }
                    aRule.Set( i, aFmt );
                }
                UnoActionContext aAction(pDoc);

                if( rPam.GetNext() != &rPam )           // Multiple selection?
                {
                    pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
                    SwPamRanges aRangeArr( rPam );
                    SwPaM aPam( *rPam.GetPoint() );
                    for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
                    {
                        // no start of a new list
                        pDoc->SetNumRule( aRangeArr.SetPam( n, aPam ), aRule, false );
                    }
                    pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
                }
                else
                {
                    // no start of a new list
                    pDoc->SetNumRule( rPam, aRule, false );
                }


            }
            else if(!pSwNum->GetCreatedNumRuleName().isEmpty())
            {
                UnoActionContext aAction(pDoc);
                SwNumRule* pRule = pDoc->FindNumRulePtr( pSwNum->GetCreatedNumRuleName() );
                if(!pRule)
                    throw RuntimeException();
                // no start of a new list
                pDoc->SetNumRule( rPam, *pRule, false );
            }
            // #i103817#
            // outline numbering
            else
            {
                UnoActionContext aAction(pDoc);
                SwNumRule* pRule = pDoc->GetOutlineNumRule();
                if(!pRule)
                    throw RuntimeException();
                pDoc->SetNumRule( rPam, *pRule, false );
            }
        }
    }
    else if(rValue.getValueType() == ::getVoidCppuType())
    {
        rPam.GetDoc()->DelNumRules(rPam);
    }
}

void  getNumberingProperty(SwPaM& rPam, PropertyState& eState, Any * pAny )
{
    const SwNumRule* pNumRule = rPam.GetDoc()->GetCurrNumRule( *rPam.GetPoint() );
    if(pNumRule)
    {
        uno::Reference< XIndexReplace >  xNum = new SwXNumberingRules(*pNumRule);
        if ( pAny )
            pAny->setValue(&xNum, ::getCppuType((const uno::Reference<XIndexReplace>*)0));
        eState = PropertyState_DIRECT_VALUE;
    }
    else
        eState = PropertyState_DEFAULT_VALUE;
}

void GetCurPageStyle(SwPaM& rPaM, OUString &rString)
{
    if (!rPaM.GetCntntNode())
        return; // TODO: is there an easy way to get it for tables/sections?
    const SwPageFrm* pPage = rPaM.GetCntntNode()->getLayoutFrm(rPaM.GetDoc()->GetCurrentLayout())->FindPageFrm();
    if(pPage)
    {
        OUString tmp;
        SwStyleNameMapper::FillProgName(pPage->GetPageDesc()->GetName(),
            tmp, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true);
        rString = tmp;
    }
}

/* --------------------------------------------------
 * reset special properties of the cursor
 * --------------------------------------------------*/
void resetCrsrPropertyValue(const SfxItemPropertySimpleEntry& rEntry, SwPaM& rPam)
{
    SwDoc* pDoc = rPam.GetDoc();
    switch(rEntry.nWID)
    {
        case FN_UNO_PARA_STYLE :
//          lcl_SetTxtFmtColl(aValue, pUnoCrsr);
        break;
        case FN_UNO_PAGE_STYLE :
        break;
        case FN_UNO_NUM_START_VALUE  :
        {
            UnoActionContext aAction(pDoc);

            if( rPam.GetNext() != &rPam )           // Multiple selection?
            {
                pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
                SwPamRanges aRangeArr( rPam );
                SwPaM aPam( *rPam.GetPoint() );
                for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
                    pDoc->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), 1 );
                pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
            }
            else
                pDoc->SetNodeNumStart( *rPam.GetPoint(), 0 );
        }

        break;
        case FN_UNO_NUM_LEVEL  :
        break;
        case FN_UNO_NUM_RULES:
//          lcl_setNumberingProperty(aValue, pUnoCrsr);
        break;
        case FN_UNO_CHARFMT_SEQUENCE:
        {
            std::set<sal_uInt16> aWhichIds;
            aWhichIds.insert( RES_TXTATR_CHARFMT);
            pDoc->ResetAttrs(rPam, true, aWhichIds);
        }
        break;
    }
}

void InsertFile(SwUnoCrsr* pUnoCrsr,
    const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rOptions
    ) throw( lang::IllegalArgumentException, io::IOException, uno::RuntimeException )
{
    SfxMedium* pMed = 0;
    SwDoc* pDoc = pUnoCrsr->GetDoc();
    SwDocShell* pDocSh = pDoc->GetDocShell();
    comphelper::MediaDescriptor aMediaDescriptor( rOptions );
    OUString sFileName = rURL;
    OUString sFilterName, sFilterOptions, sPassword, sBaseURL;
    uno::Reference < io::XStream > xStream;
    uno::Reference < io::XInputStream > xInputStream;

    if( sFileName.isEmpty() )
        aMediaDescriptor[comphelper::MediaDescriptor::PROP_URL()] >>= sFileName;
    if( sFileName.isEmpty() )
        aMediaDescriptor[comphelper::MediaDescriptor::PROP_FILENAME()] >>= sFileName;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_STREAM()] >>= xStream;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_FILTERNAME()] >>= sFilterName;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_FILTEROPTIONS()] >>= sFilterOptions;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_PASSWORD()] >>= sPassword;
    aMediaDescriptor[comphelper::MediaDescriptor::PROP_DOCUMENTBASEURL() ] >>= sBaseURL;
    if ( !xInputStream.is() && xStream.is() )
        xInputStream = xStream->getInputStream();

    if(!pDocSh || (sFileName.isEmpty() && !xInputStream.is()))
        return;

    SfxObjectFactory& rFact = pDocSh->GetFactory();
    const SfxFilter* pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( sFilterName );
    uno::Reference < embed::XStorage > xReadStorage;
    if( xInputStream.is() )
    {
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= xInputStream;
        aArgs[1] <<= embed::ElementModes::READ;
        try
        {
            xReadStorage = uno::Reference< embed::XStorage >(
                            ::comphelper::OStorageHelper::GetStorageFactory()->createInstanceWithArguments( aArgs ),
                            uno::UNO_QUERY );
        }
        catch( const io::IOException& rEx)
        {
            (void)rEx;
        }
    }
    if ( !pFilter )
    {
        if( xInputStream.is() && !xReadStorage.is())
        {
            pMed = new SfxMedium;
            pMed->setStreamToLoadFrom(xInputStream, sal_True );
        }
        else
            pMed = xReadStorage.is() ?
                new SfxMedium(xReadStorage, sBaseURL, 0 ) :
                new SfxMedium(sFileName, STREAM_READ, 0, 0 );
        if( !sBaseURL.isEmpty() )
            pMed->GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, sBaseURL ) );

        SfxFilterMatcher aMatcher( rFact.GetFilterContainer()->GetName() );
        ErrCode nErr = aMatcher.GuessFilter( *pMed, &pFilter, sal_False );
        if ( nErr || !pFilter)
            DELETEZ(pMed);
        else
            pMed->SetFilter( pFilter );
    }
    else
    {
        if(!pMed)
        {
            if( xInputStream.is() && !xReadStorage.is())
            {
                pMed = new SfxMedium;
                pMed->setStreamToLoadFrom(xInputStream, sal_True );
                pMed->SetFilter( pFilter );
            }
            else
            {
                if( xReadStorage.is() )
                {
                    pMed = new SfxMedium(xReadStorage, sBaseURL, 0 );
                    pMed->SetFilter( pFilter );
                }
                else
                    pMed = new SfxMedium(sFileName, STREAM_READ, pFilter, 0);
            }
        }
        if(!sFilterOptions.isEmpty())
            pMed->GetItemSet()->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, sFilterOptions ) );
        if(!sBaseURL.isEmpty())
            pMed->GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, sBaseURL ) );
    }

    if( !pMed )
        return;

    // this sourcecode is not responsible for the lifetime of the shell, SfxObjectShellLock should not be used
    SfxObjectShellRef aRef( pDocSh );

    pMed->DownLoad();   // if necessary: start the download
    if( aRef.Is() && 1 < aRef->GetRefCount() )  // Ref still valid?
    {
        SwReader* pRdr;
        SfxItemSet* pSet =  pMed->GetItemSet();
        pSet->Put(SfxBoolItem(FN_API_CALL, sal_True));
        if(!sPassword.isEmpty())
            pSet->Put(SfxStringItem(SID_PASSWORD, sPassword));
        Reader *pRead = pDocSh->StartConvertFrom( *pMed, &pRdr, 0, pUnoCrsr);
        if( pRead )
        {

            UnoActionContext aContext(pDoc);

            if(pUnoCrsr->HasMark())
                pDoc->DeleteAndJoin(*pUnoCrsr);

            SwNodeIndex aSave(  pUnoCrsr->GetPoint()->nNode, -1 );
            xub_StrLen nCntnt = pUnoCrsr->GetPoint()->nContent.GetIndex();

            sal_uInt32 nErrno = pRdr->Read( *pRead );   // and paste the document

            if(!nErrno)
            {
                ++aSave;
                pUnoCrsr->SetMark();
                pUnoCrsr->GetMark()->nNode = aSave;

                SwCntntNode* pCntNode = aSave.GetNode().GetCntntNode();
                if( !pCntNode )
                    nCntnt = 0;
                pUnoCrsr->GetMark()->nContent.Assign( pCntNode, nCntnt );
            }

            delete pRdr;


        }
    }
    delete pMed;
}

// insert text and scan for CR characters in order to insert
// paragraph breaks at those positions by calling SplitNode
bool DocInsertStringSplitCR(
        SwDoc &rDoc,
        const SwPaM &rNewCursor,
        const OUString &rText,
        const bool bForceExpandHints )
{
    bool bOK = true;

        const enum IDocumentContentOperations::InsertFlags nInsertFlags =
            (bForceExpandHints)
            ? static_cast<IDocumentContentOperations::InsertFlags>(
                    IDocumentContentOperations::INS_FORCEHINTEXPAND |
                    IDocumentContentOperations::INS_EMPTYEXPAND)
            : IDocumentContentOperations::INS_EMPTYEXPAND;

    // grouping done in InsertString is intended for typing, not API calls
    ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    OUString aTxt;
    xub_StrLen nStartIdx = 0;
    SwTxtNode* const pTxtNd =
        rNewCursor.GetPoint()->nNode.GetNode().GetTxtNode();
    if (!pTxtNd)
    {
        SAL_INFO("sw.uno", "DocInsertStringSplitCR: need a text node");
        return false;
    }
    const xub_StrLen nMaxLength = ( pTxtNd )
        ? STRING_LEN - pTxtNd->GetTxt().getLength()
        : STRING_LEN;
    sal_Int32 nIdx = rText.indexOf( '\r', nStartIdx );
    if( ( nIdx == -1 && nMaxLength < rText.getLength() ) ||
        ( nIdx != -1 && nMaxLength < nIdx ) )
    {
        nIdx = nMaxLength;
    }
    while (nIdx != -1 )
    {
        OSL_ENSURE( nIdx - nStartIdx >= 0, "index negative!" );
        aTxt = rText.copy( nStartIdx, nIdx - nStartIdx );
        if (!aTxt.isEmpty() &&
            !rDoc.InsertString( rNewCursor, aTxt, nInsertFlags ))
        {
            OSL_FAIL( "Doc->Insert(Str) failed." );
            bOK = false;
        }
        if (!rDoc.SplitNode( *rNewCursor.GetPoint(), false ) )
        {
            OSL_FAIL( "SplitNode failed" );
            bOK = false;
        }
        nStartIdx = nIdx + 1;
        nIdx = rText.indexOf( '\r', nStartIdx );
    }
    aTxt = rText.copy( nStartIdx );
    if (!aTxt.isEmpty() &&
        !rDoc.InsertString( rNewCursor, aTxt, nInsertFlags ))
    {
        OSL_FAIL( "Doc->Insert(Str) failed." );
        bOK = false;
    }

    return bOK;
}

void makeRedline( SwPaM& rPaM,
    const OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    IDocumentRedlineAccess* pRedlineAccess = rPaM.GetDoc();

    RedlineType_t eType = nsRedlineType_t::REDLINE_INSERT;
    if ( rRedlineType == "Delete" )
        eType = nsRedlineType_t::REDLINE_DELETE;
    else if ( rRedlineType == "Format" )
        eType = nsRedlineType_t::REDLINE_FORMAT;
    else if ( rRedlineType == "TextTable" )
        eType = nsRedlineType_t::REDLINE_TABLE;
    else if( !rRedlineType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Insert" ) ))
        throw lang::IllegalArgumentException();

    //todo: what about REDLINE_FMTCOLL?
    comphelper::SequenceAsHashMap aPropMap( rRedlineProperties );
    uno::Any aAuthorValue;
    aAuthorValue = aPropMap.getUnpackedValueOrDefault("RedlineAuthor", aAuthorValue);
    sal_uInt16 nAuthor = 0;
    OUString sAuthor;
    if( aAuthorValue >>= sAuthor )
        nAuthor = pRedlineAccess->InsertRedlineAuthor(sAuthor);

    OUString sComment;
    uno::Any aCommentValue;
    aCommentValue = aPropMap.getUnpackedValueOrDefault("RedlineComment", aCommentValue);

    SwRedlineData aRedlineData( eType, nAuthor );
    if( aCommentValue >>= sComment )
        aRedlineData.SetComment( sComment );

    ::util::DateTime aStamp;
    uno::Any aDateTimeValue;
    aDateTimeValue = aPropMap.getUnpackedValueOrDefault("RedlineDateTime", aDateTimeValue);
    if( aDateTimeValue >>= aStamp )
    {
       aRedlineData.SetTimeStamp(
        DateTime( Date( aStamp.Day, aStamp.Month, aStamp.Year ), Time( aStamp.Hours, aStamp.Minutes, aStamp.Seconds ) ) );
    }

    SwRedline* pRedline = new SwRedline( aRedlineData, rPaM );
    RedlineMode_t nPrevMode = pRedlineAccess->GetRedlineMode( );

    pRedlineAccess->SetRedlineMode_intern(nsRedlineMode_t::REDLINE_ON);
    bool bRet = pRedlineAccess->AppendRedline( pRedline, false );
    pRedlineAccess->SetRedlineMode_intern( nPrevMode );
    if( !bRet )
        throw lang::IllegalArgumentException();
}

SwAnyMapHelper::~SwAnyMapHelper()
{
    AnyMapHelper_t::iterator aIt = begin();
    while( aIt != end() )
    {
        delete ( aIt->second );
        ++aIt;
    }
}

void SwAnyMapHelper::SetValue( sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any& rAny )
{
    sal_uInt32 nKey = (nWhichId << 16) + nMemberId;
    AnyMapHelper_t::iterator aIt = find( nKey );
    if( aIt != end() )
    {
        *(aIt->second) = rAny;
    }
    else
        insert( value_type(nKey, new uno::Any( rAny )) );
}

bool    SwAnyMapHelper::FillValue( sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any*& pAny )
{
    bool bRet = false;
    sal_uInt32 nKey = (nWhichId << 16) + nMemberId;
    AnyMapHelper_t::iterator aIt = find( nKey );
    if( aIt != end() )
    {
        pAny = aIt->second;
        bRet = true;
    }
    return bRet;
}

}//namespace SwUnoCursorHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
