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
#include <algorithm>
#include <memory>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <svx/unoshape.hxx>

#include <cmdid.h>
#include <unotextrange.hxx>
#include <unodraw.hxx>
#include <unofootnote.hxx>
#include <unobookmark.hxx>
#include <unomap.hxx>
#include <unorefmark.hxx>
#include <unoidx.hxx>
#include <unofield.hxx>
#include <unotbl.hxx>
#include <unosett.hxx>
#include <unoframe.hxx>
#include <unocrsr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <fmtftn.hxx>
#include <charfmt.hxx>
#include <pagedesc.hxx>
#include <docstyle.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <section.hxx>
#include <shellio.hxx>
#include <edimp.hxx>
#include <swundo.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <svl/eitem.hxx>
#include <svl/lngmisc.hxx>
#include <docary.hxx>
#include <swtable.hxx>
#include <tox.hxx>
#include <doctxm.hxx>
#include <fchrfmt.hxx>
#include <editeng/editids.hrc>
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
#include <unotools/mediadescriptor.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <SwNodeNum.hxx>
#include <fmtmeta.hxx>
#include <txtfld.hxx>
#include <unoparagraph.hxx>
#include <poolfmt.hxx>
#include <paratr.hxx>
#include <sal/log.hxx>

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
        OUString & o_rTableName, SwUnoTableCursor const*& o_rpTableCursor,
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

    SwXTextRanges* const pRanges(
        ::sw::UnoTunnelGetImplementation<SwXTextRanges>(xTunnel));
    if (pRanges)
    {
        SwUnoCursor const* pUnoCursor = pRanges->GetCursor();
        if (pUnoCursor && &pUnoCursor->GetDoc() == &rTargetDoc)
        {
            o_rpPaM = lcl_createPamCopy(*pUnoCursor);
        }
        return;
    }

    // check these before Range to prevent misinterpretation of text frames
    // and cells also implement XTextRange
    SwXFrame *const pFrame(
        ::sw::UnoTunnelGetImplementation<SwXFrame>(xTunnel));
    if (pFrame)
    {
        const SwFrameFormat *const pFrameFormat(pFrame->GetFrameFormat());
        if (pFrameFormat && pFrameFormat->GetDoc() == &rTargetDoc)
        {
            o_rFrame = std::make_pair(pFrameFormat->GetName(), pFrame->GetFlyCntType());
        }
        return;
    }

    SwXTextTable *const pTextTable(
        ::sw::UnoTunnelGetImplementation<SwXTextTable>(xTunnel));
    if (pTextTable)
    {
        SwFrameFormat *const pFrameFormat(pTextTable->GetFrameFormat());
        if (pFrameFormat && pFrameFormat->GetDoc() == &rTargetDoc)
        {
            o_rTableName = pFrameFormat->GetName();
        }
        return;
    }

    SwXCell *const pCell(
        ::sw::UnoTunnelGetImplementation<SwXCell>(xTunnel));
    if (pCell)
    {
        SwFrameFormat *const pFrameFormat(pCell->GetFrameFormat());
        if (pFrameFormat && pFrameFormat->GetDoc() == &rTargetDoc)
        {
            SwTableBox * pBox = pCell->GetTableBox();
            SwTable *const pTable = SwTable::FindTable(pFrameFormat);
            // ??? what's the benefit of setting pBox in this convoluted way?
            pBox = pCell->FindBox(pTable, pBox);
            if (pBox)
            {
                SwPosition const aPos(*pBox->GetSttNd());
                SwPaM aPam(aPos);
                aPam.Move(fnMoveForward, GoInNode);
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
        SwUnoCursor const*const pUnoCursor(pCellRange->GetTableCursor());
        if (pUnoCursor && &pUnoCursor->GetDoc() == &rTargetDoc)
        {
            // probably can't copy it to o_rpPaM for this since it's
            // a SwTableCursor
            o_rpTableCursor = dynamic_cast<SwUnoTableCursor const*>(pUnoCursor);
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
GetNestedTextContent(SwTextNode const & rTextNode, sal_Int32 const nIndex,
        bool const bParent)
{
    // these should be unambiguous because of the dummy character
    SwTextNode::GetTextAttrMode const eMode( bParent
        ? SwTextNode::PARENT : SwTextNode::EXPAND );
    SwTextAttr *const pMetaTextAttr =
        rTextNode.GetTextAttrAt(nIndex, RES_TXTATR_META, eMode);
    SwTextAttr *const pMetaFieldTextAttr =
        rTextNode.GetTextAttrAt(nIndex, RES_TXTATR_METAFIELD, eMode);
    // which is innermost?
    SwTextAttr *const pTextAttr = pMetaTextAttr
        ? (pMetaFieldTextAttr
            ? ((pMetaFieldTextAttr->GetStart() >
                    pMetaTextAttr->GetStart())
                ? pMetaFieldTextAttr : pMetaTextAttr)
            : pMetaTextAttr)
        : pMetaFieldTextAttr;
    uno::Reference<XTextContent> xRet;
    if (pTextAttr)
    {
        ::sw::Meta *const pMeta(
            static_cast<SwFormatMeta &>(pTextAttr->GetAttr()).GetMeta());
        assert(pMeta);
        xRet.set(pMeta->MakeUnoObject(), uno::UNO_QUERY);
    }
    return xRet;
}

static uno::Any GetParaListAutoFormat(SwTextNode const& rNode)
{
    SwFormatAutoFormat const*const pFormat(
        rNode.GetSwAttrSet().GetItem<SwFormatAutoFormat>(RES_PARATR_LIST_AUTOFMT, false));
    if (!pFormat)
    {
        return uno::Any();
    }
    const auto pSet(pFormat->GetStyleHandle());
    if (!pSet)
        return {};
    SfxItemPropertySet const& rPropSet(*aSwMapProvider.GetPropertySet(PROPERTY_MAP_CHAR_AUTO_STYLE));
    SfxItemPropertyMap const& rMap(rPropSet.getPropertyMap());
    std::vector<beans::NamedValue> props;
    // have to iterate the map, not the item set?
    for (auto const& rEntry : rMap.getPropertyEntries())
    {
        if (rPropSet.getPropertyState(rEntry, *pSet) == PropertyState_DIRECT_VALUE)
        {
            Any value;
            rPropSet.getPropertyValue(rEntry, *pSet, value);
            props.emplace_back(rEntry.sName, value);
        }
    }
    return uno::makeAny(comphelper::containerToSequence(props));
}

// Read the special properties of the cursor
bool getCursorPropertyValue(const SfxItemPropertySimpleEntry& rEntry
                                        , SwPaM& rPam
                                        , Any *pAny
                                        , PropertyState& eState
                                        , const SwTextNode* pNode  )
{
    PropertyState eNewState = PropertyState_DIRECT_VALUE;
    bool bDone = true;
    switch(rEntry.nWID)
    {
        case FN_UNO_PARA_CONT_PREV_SUBTREE:
            if (pAny)
            {
                const SwTextNode * pTmpNode = pNode;

                if (!pTmpNode)
                    pTmpNode = rPam.GetNode().GetTextNode();

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
                const SwTextNode * pTmpNode = pNode;

                if (!pTmpNode)
                    pTmpNode = rPam.GetNode().GetTextNode();

                OUString sRet;
                if ( pTmpNode && pTmpNode->GetNum() )
                {
                    sRet = pTmpNode->GetNumString();
                }

                *pAny <<= sRet;
            }
        break;
        case RES_PARATR_OUTLINELEVEL:
            if (pAny)
            {
                const SwTextNode * pTmpNode = pNode;

                if (!pTmpNode)
                    pTmpNode = rPam.GetNode().GetTextNode();

                sal_Int16 nRet = -1;
                if ( pTmpNode )
                    nRet = sal::static_int_cast< sal_Int16 >( pTmpNode->GetAttrOutlineLevel() );

                *pAny <<= nRet;
            }
        break;
        case FN_UNO_PARA_CONDITIONAL_STYLE_NAME:
        case FN_UNO_PARA_STYLE :
        {
            SwFormatColl* pFormat = nullptr;
            if(pNode)
                pFormat = FN_UNO_PARA_CONDITIONAL_STYLE_NAME == rEntry.nWID
                            ? pNode->GetFormatColl() : &pNode->GetAnyFormatColl();
            else
            {
                pFormat = SwUnoCursorHelper::GetCurTextFormatColl(rPam,
                        FN_UNO_PARA_CONDITIONAL_STYLE_NAME == rEntry.nWID);
            }
            if(pFormat)
            {
                if( pAny )
                {
                    OUString sVal;
                    SwStyleNameMapper::FillProgName(pFormat->GetName(), sVal, SwGetPoolIdFromName::TxtColl );
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
        case FN_UNO_PARA_NUM_AUTO_FORMAT:
        {
            if (!pAny)
            {
                break;
            }
            // a multi selection is not considered
            const SwTextNode* pTextNd = rPam.GetNode().GetTextNode();
            if ( pTextNd && pTextNd->IsInList() )
            {
                switch (rEntry.nWID)
                {
                    case FN_UNO_NUM_LEVEL:
                    {
                        *pAny <<= static_cast<sal_Int16>(pTextNd->GetActualListLevel());
                        break;
                    }
                    case FN_UNO_IS_NUMBER:
                    {
                        *pAny <<= pTextNd->IsCountedInList();
                        break;
                    }
                    // #i91601#
                    case FN_UNO_LIST_ID:
                    {
                        *pAny <<= pTextNd->GetListId();
                        break;
                    }
                    case FN_NUMBER_NEWSTART:
                    {
                        *pAny <<= pTextNd->IsListRestart();
                        break;
                    }
                    case FN_UNO_PARA_NUM_AUTO_FORMAT:
                    {
                        *pAny = GetParaListAutoFormat(*pTextNd);
                        break;
                    }
                    default:
                        assert(false);
                }
            }
            else
            {
                eNewState = PropertyState_DEFAULT_VALUE;

                    // #i30838# set default values for default properties
                switch (rEntry.nWID)
                {
                    case FN_UNO_NUM_LEVEL:
                    {
                        *pAny <<= static_cast<sal_Int16>( 0 );
                        break;
                    }
                    case FN_UNO_IS_NUMBER:
                    {
                        *pAny <<= false;
                        break;
                    }
                    // #i91601#
                    case FN_UNO_LIST_ID:
                    {
                        *pAny <<= OUString();
                        break;
                    }
                    case FN_NUMBER_NEWSTART:
                    {
                        *pAny <<= false;
                        break;
                    }
                    case FN_UNO_PARA_NUM_AUTO_FORMAT:
                    {
                        break; // void
                    }
                    default:
                        assert(false);
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
                if( !SwDoc::GetNumRuleAtPos( *rPam.GetPoint() ) )
                    eNewState = PropertyState_DEFAULT_VALUE;
            }
            break;
        case FN_UNO_DOCUMENT_INDEX_MARK:
        {
            std::vector<SwTextAttr *> marks;
            if (rPam.GetNode().IsTextNode())
            {
                marks = rPam.GetNode().GetTextNode()->GetTextAttrsAt(
                    rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_TOXMARK);
            }
            if (!marks.empty())
            {
                if( pAny )
                {   // hmm... can only return 1 here
                    SwTOXMark & rMark =
                        static_cast<SwTOXMark &>((*marks.begin())->GetAttr());
                    const uno::Reference< text::XDocumentIndexMark > xRef =
                        SwXDocumentIndexMark::CreateXDocumentIndexMark(
                            rPam.GetDoc(), &rMark);
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
            SwTOXBase* pBase = SwDoc::GetCurTOX(
                                                    *rPam.Start() );
            if( pBase )
            {
                if( pAny )
                {
                    const uno::Reference< text::XDocumentIndex > xRef =
                        SwXDocumentIndex::CreateXDocumentIndex(rPam.GetDoc(),
                            static_cast<SwTOXBaseSection *>(pBase));
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
            const SwTextNode *pTextNd =
                rPam.GetDoc().GetNodes()[pPos->nNode.GetIndex()]->GetTextNode();
            const SwTextAttr* pTextAttr = pTextNd
                ? pTextNd->GetFieldTextAttrAt( pPos->nContent.GetIndex(), true )
                : nullptr;
            if ( pTextAttr != nullptr )
            {
                if( pAny )
                {
                    uno::Reference<text::XTextField> const xField(
                        SwXTextField::CreateXTextField(&rPam.GetDoc(),
                           &pTextAttr->GetFormatField()));
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
            SwStartNode* pSttNode = rPam.GetNode().StartOfSectionNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();
            if(SwTableBoxStartNode == eType)
            {
                if( pAny )
                {
                    const SwTableNode* pTableNode = pSttNode->FindTableNode();
                    SwFrameFormat* pTableFormat = pTableNode->GetTable().GetFrameFormat();
                    //SwTable& rTable = static_cast<SwTableNode*>(pSttNode)->GetTable();
                    if(FN_UNO_TEXT_TABLE == rEntry.nWID)
                    {
                        uno::Reference< XTextTable >  xTable = SwXTextTables::GetObject(*pTableFormat);
                        *pAny <<= xTable;
                    }
                    else
                    {
                        SwTableBox* pBox = pSttNode->GetTableBox();
                        uno::Reference< XCell >  xCell = SwXCell::CreateXCell(pTableFormat, pBox);
                        *pAny <<= xCell;
                    }
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_FRAME:
        {
            SwStartNode* pSttNode = rPam.GetNode().StartOfSectionNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();

            SwFrameFormat* pFormat;
            if(eType == SwFlyStartNode && nullptr != (pFormat = pSttNode->GetFlyFormat()))
            {
                if( pAny )
                {
                    uno::Reference<XTextFrame> const xFrame(
                        SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat));
                    (*pAny) <<= xFrame;
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_SECTION:
        {
            SwSection* pSect = SwDoc::GetCurrSection(*rPam.GetPoint());
            if(pSect)
            {
                if( pAny )
                {
                    uno::Reference< XTextSection >  xSect = SwXTextSections::GetObject( *pSect->GetFormat() );
                    *pAny <<= xSect;
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_PARAGRAPH:
        {
            SwTextNode* pTextNode = rPam.GetPoint()->nNode.GetNode().GetTextNode();
            if (pTextNode)
            {
                if (pAny)
                {
                    uno::Reference<text::XTextContent> xParagraph = SwXParagraph::CreateXParagraph(pTextNode->GetDoc(), pTextNode);
                    *pAny <<= xParagraph;
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_ENDNOTE:
        case FN_UNO_FOOTNOTE:
        {
            SwTextAttr *const pTextAttr = rPam.GetNode().IsTextNode() ?
                rPam.GetNode().GetTextNode()->GetTextAttrForCharAt(
                    rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN) : nullptr;
            if(pTextAttr)
            {
                const SwFormatFootnote& rFootnote = pTextAttr->GetFootnote();
                if(rFootnote.IsEndNote() == (FN_UNO_ENDNOTE == rEntry.nWID))
                {
                    if( pAny )
                    {
                        const uno::Reference< text::XFootnote > xFootnote =
                            SwXFootnote::CreateXFootnote(rPam.GetDoc(),
                                    &const_cast<SwFormatFootnote&>(rFootnote));
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
            std::vector<SwTextAttr *> marks;
            if (rPam.GetNode().IsTextNode())
            {
                marks = rPam.GetNode().GetTextNode()->GetTextAttrsAt(
                            rPam.GetPoint()->nContent.GetIndex(), RES_TXTATR_REFMARK);
            }
            if (!marks.empty())
            {
                if( pAny )
                {   // hmm... can only return 1 here
                    const SwFormatRefMark& rRef = (*marks.begin())->GetRefMark();
                    uno::Reference<XTextContent> const xRef =
                        SwXReferenceMark::CreateXReferenceMark(rPam.GetDoc(),
                                const_cast<SwFormatRefMark*>(&rRef));
                    *pAny <<= xRef;
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_NESTED_TEXT_CONTENT:
        {
            uno::Reference<XTextContent> const xRet(rPam.GetNode().IsTextNode()
                ? GetNestedTextContent(*rPam.GetNode().GetTextNode(),
                    rPam.GetPoint()->nContent.GetIndex(), false)
                : nullptr);
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

            SwTextNode *const pTextNode = rPam.GetNode().GetTextNode();
            if (&rPam.GetNode() == &rPam.GetNode(false)
                && pTextNode && pTextNode->GetpSwpHints())
            {
                sal_Int32 nPaMStart = rPam.GetPoint()->nContent.GetIndex();
                sal_Int32 nPaMEnd = rPam.GetMark() ? rPam.GetMark()->nContent.GetIndex() : nPaMStart;
                if(nPaMStart > nPaMEnd)
                {
                    std::swap(nPaMStart, nPaMEnd);
                }
                Sequence< OUString> aCharStyles;
                SwpHints* pHints = pTextNode->GetpSwpHints();
                for( size_t nAttr = 0; nAttr < pHints->Count(); ++nAttr )
                {
                    SwTextAttr* pAttr = pHints->Get( nAttr );
                    if(pAttr->Which() != RES_TXTATR_CHARFMT)
                        continue;
                    const sal_Int32 nAttrStart = pAttr->GetStart();
                    const sal_Int32 nAttrEnd = *pAttr->GetEnd();
                    //check if the attribute touches the selection
                    if( ( nAttrEnd > nPaMStart && nAttrStart < nPaMEnd ) ||
                        ( !nAttrStart && !nAttrEnd && !nPaMStart && !nPaMEnd ) )
                    {
                        //check for overlapping
                        if(nAttrStart > nPaMStart ||
                                    nAttrEnd < nPaMEnd)
                        {
                            aCharStyles.realloc(0);
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
                            OSL_ENSURE(pAttr->GetCharFormat().GetCharFormat(), "no character format set");
                            aCharStyles.getArray()[aCharStyles.getLength() - 1] =
                                        SwStyleNameMapper::GetProgName(
                                            pAttr->GetCharFormat().GetCharFormat()->GetName(), SwGetPoolIdFromName::ChrFmt);
                        }
                    }

                }
                eNewState =
                    aCharStyles.hasElements() ?
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

sal_Int16 IsNodeNumStart(SwPaM const & rPam, PropertyState& eState)
{
    const SwTextNode* pTextNd = rPam.GetNode().GetTextNode();
    // correction: check, if restart value is set at the text node and use
    // new method <SwTextNode::GetAttrListRestartValue()> to retrieve the value
    if ( pTextNd && pTextNd->GetNumRule() && pTextNd->IsListRestart() &&
         pTextNd->HasAttrListRestartValue() )
    {
        eState = PropertyState_DIRECT_VALUE;
        sal_Int16 nTmp = sal::static_int_cast< sal_Int16 >(pTextNd->GetAttrListRestartValue());
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
        auto pSwNum = comphelper::getUnoTunnelImplementation<SwXNumberingRules>(xIndexReplace);
        if(pSwNum)
        {
            SwDoc& rDoc = rPam.GetDoc();
            if(pSwNum->GetNumRule())
            {
                SwNumRule aRule(*pSwNum->GetNumRule());
                const OUString* pNewCharStyles =  pSwNum->GetNewCharStyleNames();
                const OUString* pBulletFontNames = pSwNum->GetBulletFontNames();
                for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                {
                    SwNumFormat aFormat(aRule.Get( i ));
                    if (!pNewCharStyles[i].isEmpty() &&
                        !SwXNumberingRules::isInvalidStyle(pNewCharStyles[i]) &&
                        (!aFormat.GetCharFormat() || pNewCharStyles[i] != aFormat.GetCharFormat()->GetName()))
                    {
                        if (pNewCharStyles[i].isEmpty())
                        {
                            // FIXME
                            // Is something missing/wrong here?
                            // if condition is always false due to outer check!
                            aFormat.SetCharFormat(nullptr);
                        }
                        else
                        {

                            // get CharStyle and set the rule
                            const size_t nChCount = rDoc.GetCharFormats()->size();
                            SwCharFormat* pCharFormat = nullptr;
                            for(size_t nCharFormat = 0; nCharFormat < nChCount; ++nCharFormat)
                            {
                                SwCharFormat& rChFormat = *((*(rDoc.GetCharFormats()))[nCharFormat]);
                                if(rChFormat.GetName() == pNewCharStyles[i])
                                {
                                    pCharFormat = &rChFormat;
                                    break;
                                }
                            }

                            if(!pCharFormat)
                            {
                                SfxStyleSheetBasePool* pPool = rDoc.GetDocShell()->GetStyleSheetPool();
                                SfxStyleSheetBase* pBase;
                                pBase = pPool->Find(pNewCharStyles[i], SfxStyleFamily::Char);
                            // shall it really be created?
                                if(!pBase)
                                    pBase = &pPool->Make(pNewCharStyles[i], SfxStyleFamily::Page);
                                pCharFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();
                            }
                            if(pCharFormat)
                                aFormat.SetCharFormat(pCharFormat);
                        }
                    }
                    //Now again for fonts
                    if(
                       !pBulletFontNames[i].isEmpty() &&
                       !SwXNumberingRules::isInvalidStyle(pBulletFontNames[i]) &&
                       (!aFormat.GetBulletFont() || aFormat.GetBulletFont()->GetFamilyName() != pBulletFontNames[i])
                      )
                    {
                        const SvxFontListItem* pFontListItem =
                                static_cast<const SvxFontListItem* >(rDoc.GetDocShell()
                                                    ->GetItem( SID_ATTR_CHAR_FONTLIST ));
                        const FontList*  pList = pFontListItem->GetFontList();

                        FontMetric aFontMetric = pList->Get(
                            pBulletFontNames[i],WEIGHT_NORMAL, ITALIC_NONE);
                        vcl::Font aFont(aFontMetric);
                        aFormat.SetBulletFont(&aFont);
                    }
                    aRule.Set( i, aFormat );
                }
                UnoActionContext aAction(&rDoc);

                if( rPam.GetNext() != &rPam )           // Multiple selection?
                {
                    rDoc.GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
                    SwPamRanges aRangeArr( rPam );
                    SwPaM aPam( *rPam.GetPoint() );
                    for ( size_t n = 0; n < aRangeArr.Count(); ++n )
                    {
                        // no start of a new list
                        rDoc.SetNumRule( aRangeArr.SetPam( n, aPam ), aRule, false );
                    }
                    rDoc.GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
                }
                else
                {
                    // no start of a new list
                    rDoc.SetNumRule( rPam, aRule, false );
                }

            }
            else if(!pSwNum->GetCreatedNumRuleName().isEmpty())
            {
                UnoActionContext aAction( &rDoc );
                SwNumRule* pRule = rDoc.FindNumRulePtr( pSwNum->GetCreatedNumRuleName() );
                if ( !pRule )
                    throw RuntimeException();
                // no start of a new list
                rDoc.SetNumRule( rPam, *pRule, false );
            }
            else
            {
                // #i103817#
                // outline numbering
                UnoActionContext aAction(&rDoc);
                SwNumRule* pRule = rDoc.GetOutlineNumRule();
                if(!pRule)
                    throw RuntimeException();
                rDoc.SetNumRule( rPam, *pRule, false );
            }
        }
    }
    else if ( rValue.getValueType() == cppu::UnoType<void>::get() )
    {
        rPam.GetDoc().DelNumRules(rPam);
    }
}

void  getNumberingProperty(SwPaM& rPam, PropertyState& eState, Any * pAny )
{
    const SwNumRule* pNumRule = SwDoc::GetNumRuleAtPos( *rPam.GetPoint() );
    if(pNumRule)
    {
        uno::Reference< XIndexReplace >  xNum = new SwXNumberingRules(*pNumRule);
        if ( pAny )
            *pAny <<= xNum;
        eState = PropertyState_DIRECT_VALUE;
    }
    else
        eState = PropertyState_DEFAULT_VALUE;
}

void GetCurPageStyle(SwPaM const & rPaM, OUString &rString)
{
    if (!rPaM.GetContentNode())
        return; // TODO: is there an easy way to get it for tables/sections?
    SwRootFrame* pLayout = rPaM.GetDoc().getIDocumentLayoutAccess().GetCurrentLayout();
    // Consider the position inside the content node, since the node may span over multiple pages
    // with different page styles.
    SwContentFrame* pFrame = rPaM.GetContentNode()->getLayoutFrame(pLayout, rPaM.GetPoint());
    if(pFrame)
    {
        const SwPageFrame* pPage = pFrame->FindPageFrame();
        if(pPage)
        {
            SwStyleNameMapper::FillProgName(pPage->GetPageDesc()->GetName(),
                rString, SwGetPoolIdFromName::PageDesc);
        }
    }
}

// reset special properties of the cursor
void resetCursorPropertyValue(const SfxItemPropertySimpleEntry& rEntry, SwPaM& rPam)
{
    SwDoc& rDoc = rPam.GetDoc();
    switch(rEntry.nWID)
    {
        case FN_UNO_PARA_STYLE :
        break;
        case FN_UNO_PAGE_STYLE :
        break;
        case FN_UNO_NUM_START_VALUE  :
        {
            UnoActionContext aAction(&rDoc);

            if( rPam.GetNext() != &rPam )           // Multiple selection?
            {
                rDoc.GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
                SwPamRanges aRangeArr( rPam );
                SwPaM aPam( *rPam.GetPoint() );
                for( size_t n = 0; n < aRangeArr.Count(); ++n )
                    rDoc.SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), 1 );
                rDoc.GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
            }
            else
                rDoc.SetNodeNumStart( *rPam.GetPoint(), 0 );
        }

        break;
        case FN_UNO_NUM_LEVEL  :
        break;
        case FN_UNO_NUM_RULES:
        break;
        case FN_UNO_CHARFMT_SEQUENCE:
        {
            o3tl::sorted_vector<sal_uInt16> aWhichIds;
            aWhichIds.insert( RES_TXTATR_CHARFMT);
            rDoc.ResetAttrs(rPam, true, aWhichIds);
        }
        break;
    }
}

void InsertFile(SwUnoCursor* pUnoCursor, const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rOptions)
{
    std::unique_ptr<SfxMedium> pMed;
    SwDoc& rDoc = pUnoCursor->GetDoc();
    SwDocShell* pDocSh = rDoc.GetDocShell();
    utl::MediaDescriptor aMediaDescriptor( rOptions );
    OUString sFileName = rURL;
    OUString sFilterName, sFilterOptions, sPassword, sBaseURL;
    uno::Reference < io::XStream > xStream;
    uno::Reference < io::XInputStream > xInputStream;

    if( sFileName.isEmpty() )
        aMediaDescriptor[utl::MediaDescriptor::PROP_URL()] >>= sFileName;
    if( sFileName.isEmpty() )
        aMediaDescriptor[utl::MediaDescriptor::PROP_FILENAME()] >>= sFileName;
    aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
    aMediaDescriptor[utl::MediaDescriptor::PROP_STREAM()] >>= xStream;
    aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
    aMediaDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] >>= sFilterName;
    aMediaDescriptor[utl::MediaDescriptor::PROP_FILTEROPTIONS()] >>= sFilterOptions;
    aMediaDescriptor[utl::MediaDescriptor::PROP_PASSWORD()] >>= sPassword;
    aMediaDescriptor[utl::MediaDescriptor::PROP_DOCUMENTBASEURL() ] >>= sBaseURL;
    if ( !xInputStream.is() && xStream.is() )
        xInputStream = xStream->getInputStream();

    if(!pDocSh || (sFileName.isEmpty() && !xInputStream.is()))
        return;

    SfxObjectFactory& rFact = pDocSh->GetFactory();
    std::shared_ptr<const SfxFilter> pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( sFilterName );
    uno::Reference < embed::XStorage > xReadStorage;
    if( xInputStream.is() )
    {
        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= xInputStream;
        aArgs[1] <<= embed::ElementModes::READ;
        try
        {
            xReadStorage.set( ::comphelper::OStorageHelper::GetStorageFactory()->createInstanceWithArguments( aArgs ),
                              uno::UNO_QUERY );
        }
        catch( const io::IOException&) {}
    }
    if ( !pFilter )
    {
        if( xInputStream.is() && !xReadStorage.is())
        {
            pMed.reset(new SfxMedium);
            pMed->setStreamToLoadFrom(xInputStream, true );
        }
        else
            pMed.reset(xReadStorage.is() ?
                new SfxMedium(xReadStorage, sBaseURL ) :
                new SfxMedium(sFileName, StreamMode::READ ));
        if( !sBaseURL.isEmpty() )
            pMed->GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, sBaseURL ) );

        SfxFilterMatcher aMatcher( rFact.GetFilterContainer()->GetName() );
        ErrCode nErr = aMatcher.GuessFilter(*pMed, pFilter, SfxFilterFlags::NONE);
        if ( nErr || !pFilter)
            return;
        pMed->SetFilter( pFilter );
    }
    else
    {
        if( xInputStream.is() && !xReadStorage.is())
        {
            pMed.reset(new SfxMedium);
            pMed->setStreamToLoadFrom(xInputStream, true );
            pMed->SetFilter( pFilter );
        }
        else
        {
            if( xReadStorage.is() )
            {
                pMed.reset(new SfxMedium(xReadStorage, sBaseURL ));
                pMed->SetFilter( pFilter );
            }
            else
                pMed.reset(new SfxMedium(sFileName, StreamMode::READ, pFilter, nullptr));
        }
        if(!sFilterOptions.isEmpty())
            pMed->GetItemSet()->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, sFilterOptions ) );
        if(!sBaseURL.isEmpty())
            pMed->GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, sBaseURL ) );
    }

    // this sourcecode is not responsible for the lifetime of the shell, SfxObjectShellLock should not be used
    SfxObjectShellRef aRef( pDocSh );

    pMed->Download();   // if necessary: start the download
    if( !(aRef.is() && 1 < aRef->GetRefCount()) )  // Ref still valid?
        return;

    SwReaderPtr pRdr;
    SfxItemSet* pSet =  pMed->GetItemSet();
    pSet->Put(SfxBoolItem(FN_API_CALL, true));
    if(!sPassword.isEmpty())
        pSet->Put(SfxStringItem(SID_PASSWORD, sPassword));
    Reader *pRead = pDocSh->StartConvertFrom( *pMed, pRdr, nullptr, pUnoCursor);
    if( !pRead )
        return;

    UnoActionContext aContext(&rDoc);

    if(pUnoCursor->HasMark())
        rDoc.getIDocumentContentOperations().DeleteAndJoin(*pUnoCursor);

    SwNodeIndex aSave(  pUnoCursor->GetPoint()->nNode, -1 );
    sal_Int32 nContent = pUnoCursor->GetPoint()->nContent.GetIndex();

    ErrCode nErrno = pRdr->Read( *pRead );   // and paste the document

    if(!nErrno)
    {
        ++aSave;
        pUnoCursor->SetMark();
        pUnoCursor->GetMark()->nNode = aSave;

        SwContentNode* pCntNode = aSave.GetNode().GetContentNode();
        if( !pCntNode )
            nContent = 0;
        pUnoCursor->GetMark()->nContent.Assign( pCntNode, nContent );
    }
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

    for (sal_Int32 i = 0; i < rText.getLength(); ++i)
    {
        sal_Unicode const ch(rText[i]);
        if (linguistic::IsControlChar(ch)
            && ch != '\r' && ch != '\n' && ch != '\t')
        {
            SAL_WARN("sw.uno", "DocInsertStringSplitCR: refusing to insert control character " << int(ch));
            return false;
        }
    }

    const SwInsertFlags nInsertFlags =
            bForceExpandHints
            ? ( SwInsertFlags::FORCEHINTEXPAND | SwInsertFlags::EMPTYEXPAND)
            : SwInsertFlags::EMPTYEXPAND;

    // grouping done in InsertString is intended for typing, not API calls
    ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    SwTextNode* const pTextNd =
        rNewCursor.GetPoint()->nNode.GetNode().GetTextNode();
    if (!pTextNd)
    {
        SAL_INFO("sw.uno", "DocInsertStringSplitCR: need a text node");
        return false;
    }
    OUString aText;
    sal_Int32 nStartIdx = 0;
    const sal_Int32 nMaxLength = COMPLETE_STRING - pTextNd->GetText().getLength();

    sal_Int32 nIdx = rText.indexOf( '\r', nStartIdx );
    if( ( nIdx == -1 && nMaxLength < rText.getLength() ) ||
        ( nIdx != -1 && nMaxLength < nIdx ) )
    {
        nIdx = nMaxLength;
    }
    while (nIdx != -1 )
    {
        OSL_ENSURE( nIdx - nStartIdx >= 0, "index negative!" );
        aText = rText.copy( nStartIdx, nIdx - nStartIdx );
        if (!aText.isEmpty() &&
            !rDoc.getIDocumentContentOperations().InsertString( rNewCursor, aText, nInsertFlags ))
        {
            OSL_FAIL( "Doc->Insert(Str) failed." );
            bOK = false;
        }
        if (!rDoc.getIDocumentContentOperations().SplitNode( *rNewCursor.GetPoint(), false ) )
        {
            OSL_FAIL( "SplitNode failed" );
            bOK = false;
        }
        nStartIdx = nIdx + 1;
        nIdx = rText.indexOf( '\r', nStartIdx );
    }
    aText = rText.copy( nStartIdx );
    if (!aText.isEmpty() &&
        !rDoc.getIDocumentContentOperations().InsertString( rNewCursor, aText, nInsertFlags ))
    {
        OSL_FAIL( "Doc->Insert(Str) failed." );
        bOK = false;
    }

    return bOK;
}

void makeRedline( SwPaM const & rPaM,
    const OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
{
    IDocumentRedlineAccess& rRedlineAccess = rPaM.GetDoc().getIDocumentRedlineAccess();

    RedlineType eType;
    if      ( rRedlineType == "Insert" )
        eType = RedlineType::Insert;
    else if ( rRedlineType == "Delete" )
        eType = RedlineType::Delete;
    else if ( rRedlineType == "Format" )
        eType = RedlineType::Format;
    else if ( rRedlineType == "TextTable" )
        eType = RedlineType::Table;
    else if ( rRedlineType == "ParagraphFormat" )
        eType = RedlineType::ParagraphFormat;
    else
        throw lang::IllegalArgumentException();

    //todo: what about REDLINE_FMTCOLL?
    comphelper::SequenceAsHashMap aPropMap( rRedlineProperties );
    std::size_t nAuthor = 0;
    OUString sAuthor;
    if( aPropMap.getValue("RedlineAuthor") >>= sAuthor )
        nAuthor = rRedlineAccess.InsertRedlineAuthor(sAuthor);

    OUString sComment;
    SwRedlineData aRedlineData( eType, nAuthor );
    if( aPropMap.getValue("RedlineComment") >>= sComment )
        aRedlineData.SetComment( sComment );

    ::util::DateTime aStamp;
    if( aPropMap.getValue("RedlineDateTime") >>= aStamp )
    {
        aRedlineData.SetTimeStamp( DateTime( aStamp));
    }

    std::unique_ptr<SwRedlineExtraData_FormatColl> xRedlineExtraData;

    // Read the 'Redline Revert Properties' from the parameters
    uno::Sequence< beans::PropertyValue > aRevertProperties;
    // Check if the value exists
    if ( aPropMap.getValue("RedlineRevertProperties") >>= aRevertProperties )
    {
        int nMap = 0;
        // Make sure that paragraph format gets its own map, otherwise e.g. fill attributes are not preserved.
        if (eType == RedlineType::ParagraphFormat)
        {
            nMap = PROPERTY_MAP_PARAGRAPH;
            if (!aRevertProperties.hasElements())
            {
                // to reject the paragraph style change, use standard style
                xRedlineExtraData.reset(new SwRedlineExtraData_FormatColl( "",  RES_POOLCOLL_STANDARD, nullptr ));
            }
        }
        else
            nMap = PROPERTY_MAP_TEXTPORTION_EXTENSIONS;
        SfxItemPropertySet const& rPropSet = *aSwMapProvider.GetPropertySet(nMap);

        // Check if there are any properties
        if (aRevertProperties.hasElements())
        {
            SwDoc& rDoc = rPaM.GetDoc();

            // Build set of attributes we want to fetch
            std::vector<sal_uInt16> aWhichPairs;
            std::vector<SfxItemPropertySimpleEntry const*> aEntries;
            std::vector<uno::Any> aValues;
            aEntries.reserve(aRevertProperties.getLength());
            sal_uInt16 nStyleId = USHRT_MAX;
            sal_uInt16 nNumId = USHRT_MAX;
            for (const auto& rRevertProperty : std::as_const(aRevertProperties))
            {
                const OUString &rPropertyName = rRevertProperty.Name;
                SfxItemPropertySimpleEntry const* pEntry = rPropSet.getPropertyMap().getByName(rPropertyName);

                if (!pEntry)
                {
                    // unknown property
                    break;
                }
                else if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
                {
                    break;
                }
                else if (rPropertyName == "NumberingRules")
                {
                    aWhichPairs.push_back(RES_PARATR_NUMRULE);
                    aWhichPairs.push_back(RES_PARATR_NUMRULE);
                    nNumId = aEntries.size();
                }
                else
                {
                    // FIXME: we should have some nice way of merging ranges surely ?
                    aWhichPairs.push_back(pEntry->nWID);
                    aWhichPairs.push_back(pEntry->nWID);
                    if (rPropertyName == "ParaStyleName")
                        nStyleId = aEntries.size();
                }
                aEntries.push_back(pEntry);
                aValues.push_back(rRevertProperty.Value);
            }

            if (!aWhichPairs.empty())
            {
                sal_uInt16 nStylePoolId = USHRT_MAX;
                OUString sParaStyleName;
                aWhichPairs.push_back(0); // terminate
                SfxItemSet aItemSet(rDoc.GetAttrPool(), aWhichPairs.data());

                for (size_t i = 0; i < aEntries.size(); ++i)
                {
                    SfxItemPropertySimpleEntry const*const pEntry = aEntries[i];
                    const uno::Any &rValue = aValues[i];
                    if (i == nNumId)
                    {
                        uno::Reference<container::XNamed> xNumberingRules;
                        rValue >>= xNumberingRules;
                        if (xNumberingRules.is())
                        {
                            aItemSet.Put( SwNumRuleItem( xNumberingRules->getName() ));
                            // keep it during export
                            SwNumRule* pRule = rDoc.FindNumRulePtr(
                                        xNumberingRules->getName());
                            if (pRule)
                                pRule->SetUsedByRedline(true);
                        }
                    }
                    else
                    {
                        rPropSet.setPropertyValue(*pEntry, rValue, aItemSet);
                        if (i == nStyleId)
                            rValue >>= sParaStyleName;
                    }
                }

                if (eType == RedlineType::ParagraphFormat && sParaStyleName.isEmpty())
                    nStylePoolId = RES_POOLCOLL_STANDARD;

                xRedlineExtraData.reset(new SwRedlineExtraData_FormatColl( sParaStyleName, nStylePoolId, &aItemSet ));
            }
            else if (eType == RedlineType::ParagraphFormat)
                xRedlineExtraData.reset(new SwRedlineExtraData_FormatColl( "", RES_POOLCOLL_STANDARD, nullptr ));
        }
    }

    SwRangeRedline* pRedline = new SwRangeRedline( aRedlineData, rPaM );
    RedlineFlags nPrevMode = rRedlineAccess.GetRedlineFlags( );
    // xRedlineExtraData is copied here
    pRedline->SetExtraData( xRedlineExtraData.get() );

    rRedlineAccess.SetRedlineFlags_intern(RedlineFlags::On);
    auto const result(rRedlineAccess.AppendRedline(pRedline, false));
    rRedlineAccess.SetRedlineFlags_intern( nPrevMode );
    if (IDocumentRedlineAccess::AppendResult::IGNORED == result)
        throw lang::IllegalArgumentException();
}

void makeTableRowRedline( SwTableLine& rTableLine,
    const OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
{
    IDocumentRedlineAccess* pRedlineAccess = &rTableLine.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess();

    RedlineType eType;
    if ( rRedlineType == "TableRowInsert" )
    {
        eType = RedlineType::TableRowInsert;
    }
    else if ( rRedlineType == "TableRowDelete" )
    {
        eType = RedlineType::TableRowDelete;
    }
    else
    {
        throw lang::IllegalArgumentException();
    }

    comphelper::SequenceAsHashMap aPropMap( rRedlineProperties );
    std::size_t nAuthor = 0;
    OUString sAuthor;
    if( aPropMap.getValue("RedlineAuthor") >>= sAuthor )
        nAuthor = pRedlineAccess->InsertRedlineAuthor(sAuthor);

    OUString sComment;
    SwRedlineData aRedlineData( eType, nAuthor );
    if( aPropMap.getValue("RedlineComment") >>= sComment )
        aRedlineData.SetComment( sComment );

    ::util::DateTime aStamp;
    if( aPropMap.getValue("RedlineDateTime") >>= aStamp )
    {
       aRedlineData.SetTimeStamp(
        DateTime( Date( aStamp.Day, aStamp.Month, aStamp.Year ), tools::Time( aStamp.Hours, aStamp.Minutes, aStamp.Seconds ) ) );
    }

    SwTableRowRedline* pRedline = new SwTableRowRedline( aRedlineData, rTableLine );
    RedlineFlags nPrevMode = pRedlineAccess->GetRedlineFlags( );
    pRedline->SetExtraData( nullptr );

    pRedlineAccess->SetRedlineFlags_intern(RedlineFlags::On);
    bool bRet = pRedlineAccess->AppendTableRowRedline( pRedline );
    pRedlineAccess->SetRedlineFlags_intern( nPrevMode );
    if( !bRet )
        throw lang::IllegalArgumentException();
}

void makeTableCellRedline( SwTableBox& rTableBox,
    const OUString& rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
{
    IDocumentRedlineAccess* pRedlineAccess = &rTableBox.GetFrameFormat()->GetDoc()->getIDocumentRedlineAccess();

    RedlineType eType;
    if ( rRedlineType == "TableCellInsert" )
    {
        eType = RedlineType::TableCellInsert;
    }
    else if ( rRedlineType == "TableCellDelete" )
    {
        eType = RedlineType::TableCellDelete;
    }
    else
    {
        throw lang::IllegalArgumentException();
    }

    comphelper::SequenceAsHashMap aPropMap( rRedlineProperties );
    std::size_t nAuthor = 0;
    OUString sAuthor;
    if( aPropMap.getValue("RedlineAuthor") >>= sAuthor )
        nAuthor = pRedlineAccess->InsertRedlineAuthor(sAuthor);

    OUString sComment;
    SwRedlineData aRedlineData( eType, nAuthor );
    if( aPropMap.getValue("RedlineComment") >>= sComment )
        aRedlineData.SetComment( sComment );

    ::util::DateTime aStamp;
    if( aPropMap.getValue("RedlineDateTime") >>= aStamp )
    {
       aRedlineData.SetTimeStamp(
        DateTime( Date( aStamp.Day, aStamp.Month, aStamp.Year ), tools::Time( aStamp.Hours, aStamp.Minutes, aStamp.Seconds ) ) );
    }

    SwTableCellRedline* pRedline = new SwTableCellRedline( aRedlineData, rTableBox );
    RedlineFlags nPrevMode = pRedlineAccess->GetRedlineFlags( );
    pRedline->SetExtraData( nullptr );

    pRedlineAccess->SetRedlineFlags_intern(RedlineFlags::On);
    bool bRet = pRedlineAccess->AppendTableCellRedline( pRedline );
    pRedlineAccess->SetRedlineFlags_intern( nPrevMode );
    if( !bRet )
        throw lang::IllegalArgumentException();
}

void SwAnyMapHelper::SetValue( sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any& rAny )
{
    sal_uInt32 nKey = (nWhichId << 16) + nMemberId;
    m_Map[nKey] = rAny;
}

bool    SwAnyMapHelper::FillValue( sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any*& pAny )
{
    bool bRet = false;
    sal_uInt32 nKey = (nWhichId << 16) + nMemberId;
    auto aIt = m_Map.find( nKey );
    if (aIt != m_Map.end())
    {
        pAny = & aIt->second;
        bRet = true;
    }
    return bRet;
}

}//namespace SwUnoCursorHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
