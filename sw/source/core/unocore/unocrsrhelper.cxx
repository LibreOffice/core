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
#include <hintids.hxx>
#include <unotextrange.hxx>
#include <unodraw.hxx>
#include <unofootnote.hxx>
#include <unobookmark.hxx>
#include <unomap.hxx>
#include <unorefmark.hxx>
#include <unoidx.hxx>
#include <unofield.hxx>
#include <unotbl.hxx>
#include <unosection.hxx>
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
#include <swtable.hxx>
#include <tox.hxx>
#include <doctxm.hxx>
#include <fchrfmt.hxx>
#include <editeng/editids.hrc>
#include <editeng/flstitem.hxx>
#include <editeng/prntitem.hxx>
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
                        comphelper::getFromUnoTunnel<SvxShape>(xShape));
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

    SwXShape *const pShape(comphelper::getFromUnoTunnel<SwXShape>(xTunnel));
    if (pShape)
    {
        if (SvxShape * pSvxShape = pShape->GetSvxShape())
        {
            SdrObject *const pSdrObject = pSvxShape->GetSdrObject();
            if (pSdrObject)
            {   // hmm... needs view to verify it's in right doc...
                o_rSdrObjects.push_back(pSdrObject);
            }
        }
        return;
    }

    OTextCursorHelper *const pCursor(
        dynamic_cast<OTextCursorHelper*>(xIfc.get()));
    if (pCursor)
    {
        if (pCursor->GetDoc() == &rTargetDoc)
        {
            o_rpPaM = lcl_createPamCopy(*pCursor->GetPaM());
        }
        return;
    }

    SwXTextRanges* const pRanges = dynamic_cast<SwXTextRanges*>(xIfc.get());
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
    SwXFrame *const pFrame = dynamic_cast<SwXFrame*>(xIfc.get());
    if (pFrame)
    {
        const SwFrameFormat *const pFrameFormat(pFrame->GetFrameFormat());
        if (pFrameFormat && pFrameFormat->GetDoc() == &rTargetDoc)
        {
            o_rFrame = std::make_pair(pFrameFormat->GetName(), pFrame->GetFlyCntType());
        }
        return;
    }

    SwXTextTable *const pTextTable = dynamic_cast<SwXTextTable*>(xIfc.get());
    if (pTextTable)
    {
        SwFrameFormat *const pFrameFormat(pTextTable->GetFrameFormat());
        if (pFrameFormat && pFrameFormat->GetDoc() == &rTargetDoc)
        {
            o_rTableName = pFrameFormat->GetName();
        }
        return;
    }

    SwXCell *const pCell = dynamic_cast<SwXCell*>(xIfc.get());
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
                SwPaM aPam(*pBox->GetSttNd());
                aPam.Move(fnMoveForward, GoInNode);
                o_rpPaM = lcl_createPamCopy(aPam);
            }
        }
        return;
    }

    uno::Reference<text::XTextRange> const xTextRange(xIfc, UNO_QUERY);
    if (xTextRange.is())
    {
        SwUnoInternalPaM aPam(rTargetDoc);
        if (::sw::XTextRangeToSwPaM(aPam, xTextRange))
        {
            o_rpPaM = lcl_createPamCopy(aPam);
        }
        return;
    }

    SwXCellRange *const pCellRange = dynamic_cast<SwXCellRange*>(xIfc.get());
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

    ::sw::mark::IMark const*const pMark =
            SwXBookmark::GetBookmarkInDoc(& rTargetDoc, xIfc);
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
    auto const eMode( bParent
        ? ::sw::GetTextAttrMode::Parent : ::sw::GetTextAttrMode::Expand );
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
    for (auto const pEntry : rMap.getPropertyEntries())
    {
        if (SfxItemPropertySet::getPropertyState(*pEntry, *pSet) == PropertyState_DIRECT_VALUE)
        {
            Any value;
            SfxItemPropertySet::getPropertyValue(*pEntry, *pSet, value);
            props.emplace_back(pEntry->aName, value);
        }
    }
    return uno::Any(comphelper::containerToSequence(props));
}

// Read the special properties of the cursor
bool getCursorPropertyValue(const SfxItemPropertyMapEntry& rEntry
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
                    pTmpNode = rPam.GetPointNode().GetTextNode();

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
                    pTmpNode = rPam.GetPointNode().GetTextNode();

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
                    pTmpNode = rPam.GetPointNode().GetTextNode();

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
            const SwTextNode* pTextNd = rPam.GetPointNode().GetTextNode();
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
            if (rPam.GetPointNode().IsTextNode())
            {
                marks = rPam.GetPointNode().GetTextNode()->GetTextAttrsAt(
                    rPam.GetPoint()->GetContentIndex(), RES_TXTATR_TOXMARK);
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
                rPam.GetDoc().GetNodes()[pPos->GetNodeIndex()]->GetTextNode();
            const SwTextAttr* pTextAttr = pTextNd
                ? pTextNd->GetFieldTextAttrAt(pPos->GetContentIndex(), ::sw::GetTextAttrMode::Default)
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
            SwStartNode* pSttNode = rPam.GetPointNode().StartOfSectionNode();
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
            SwStartNode* pSttNode = rPam.GetPointNode().StartOfSectionNode();
            SwStartNodeType eType = pSttNode->GetStartNodeType();

            SwFrameFormat* pFormat;
            if(eType == SwFlyStartNode && nullptr != (pFormat = pSttNode->GetFlyFormat()))
            {
                // Create a wrapper only for text frames, not for graphic or OLE nodes.
                if (pAny && !rPam.GetPointNode().IsNoTextNode())
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
                    rtl::Reference< SwXTextSection > xSect = SwXTextSections::GetObject( *pSect->GetFormat() );
                    *pAny <<= uno::Reference< XTextSection >(xSect);
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_TEXT_PARAGRAPH:
        {
            SwTextNode* pTextNode = rPam.GetPoint()->GetNode().GetTextNode();
            if (pTextNode)
            {
                if (pAny)
                {
                    uno::Reference<text::XTextContent> xParagraph = SwXParagraph::CreateXParagraph(pTextNode->GetDoc(), pTextNode, nullptr);
                    *pAny <<= xParagraph;
                }
            }
            else
                eNewState = PropertyState_DEFAULT_VALUE;
        }
        break;
        case FN_UNO_SORTED_TEXT_ID:
        {
            if( pAny )
            {
                sal_Int32 nIndex = -1;
                SwTextNode* pTextNode = rPam.GetPoint()->GetNode().GetTextNode();
                if ( pTextNode )
                    nIndex = pTextNode->GetIndex().get();
                *pAny <<= nIndex;
            }
        }
        break;
        case FN_UNO_ENDNOTE:
        case FN_UNO_FOOTNOTE:
        {
            SwTextAttr *const pTextAttr = rPam.GetPointNode().IsTextNode() ?
                rPam.GetPointNode().GetTextNode()->GetTextAttrForCharAt(
                    rPam.GetPoint()->GetContentIndex(), RES_TXTATR_FTN) : nullptr;
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
            if (rPam.GetPointNode().IsTextNode())
            {
                marks = rPam.GetPointNode().GetTextNode()->GetTextAttrsAt(
                            rPam.GetPoint()->GetContentIndex(), RES_TXTATR_REFMARK);
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
            uno::Reference<XTextContent> const xRet(rPam.GetPointNode().IsTextNode()
                ? GetNestedTextContent(*rPam.GetPointNode().GetTextNode(),
                    rPam.GetPoint()->GetContentIndex(), false)
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

            SwTextNode *const pTextNode = rPam.GetPointNode().GetTextNode();
            if (&rPam.GetPointNode() == &rPam.GetMarkNode()
                && pTextNode && pTextNode->GetpSwpHints())
            {
                sal_Int32 nPaMStart = rPam.Start()->GetContentIndex();
                sal_Int32 nPaMEnd = rPam.End()->GetContentIndex();
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
    const SwTextNode* pTextNd = rPam.GetPointNode().GetTextNode();
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
        auto pSwNum = dynamic_cast<SwXNumberingRules*>(xIndexReplace.get());
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
                        const FontList* pList = pFontListItem->GetFontList();

                        vcl::Font aFont(pList->Get(
                            pBulletFontNames[i],WEIGHT_NORMAL, ITALIC_NONE));
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
    if (!rPaM.GetPointContentNode())
        return; // TODO: is there an easy way to get it for tables/sections?
    SwRootFrame* pLayout = rPaM.GetDoc().getIDocumentLayoutAccess().GetCurrentLayout();
    // Consider the position inside the content node, since the node may span over multiple pages
    // with different page styles.
    SwContentFrame* pFrame = rPaM.GetPointContentNode()->getLayoutFrame(pLayout, rPaM.GetPoint());
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
void resetCursorPropertyValue(const SfxItemPropertyMapEntry& rEntry, SwPaM& rPam)
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
            rDoc.ResetAttrs(rPam, true, { RES_TXTATR_CHARFMT });
        }
        break;
    }
}

void InsertFile(SwUnoCursor* pUnoCursor, const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rOptions)
{
    if (SwTextNode const*const pTextNode = pUnoCursor->GetPoint()->GetNode().GetTextNode())
    {
        // TODO: check meta field here too in case it ever grows a 2nd char
        if (pTextNode->GetTextAttrAt(pUnoCursor->GetPoint()->GetContentIndex(),
                RES_TXTATR_INPUTFIELD, ::sw::GetTextAttrMode::Parent))
        {
            throw uno::RuntimeException("cannot insert file inside input field");
        }

        if (pTextNode->GetTextAttrAt(pUnoCursor->GetPoint()->GetContentIndex(),
                RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent))
        {
            throw uno::RuntimeException("cannot insert file inside content controls");
        }
    }

    std::unique_ptr<SfxMedium> pMed;
    SwDoc& rDoc = pUnoCursor->GetDoc();
    SwDocShell* pDocSh = rDoc.GetDocShell();
    utl::MediaDescriptor aMediaDescriptor( rOptions );
    OUString sFileName = rURL;
    OUString sFilterName, sFilterOptions, sPassword, sBaseURL;
    uno::Reference < io::XStream > xStream;
    uno::Reference < io::XInputStream > xInputStream;

    if( sFileName.isEmpty() )
        aMediaDescriptor[utl::MediaDescriptor::PROP_URL] >>= sFileName;
    if( sFileName.isEmpty() )
        aMediaDescriptor[utl::MediaDescriptor::PROP_FILENAME] >>= sFileName;
    aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM] >>= xInputStream;
    aMediaDescriptor[utl::MediaDescriptor::PROP_STREAM] >>= xStream;
    aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM] >>= xInputStream;
    aMediaDescriptor[utl::MediaDescriptor::PROP_FILTERNAME] >>= sFilterName;
    aMediaDescriptor[utl::MediaDescriptor::PROP_FILTEROPTIONS] >>= sFilterOptions;
    aMediaDescriptor[utl::MediaDescriptor::PROP_PASSWORD] >>= sPassword;
    aMediaDescriptor[utl::MediaDescriptor::PROP_DOCUMENTBASEURL ] >>= sBaseURL;
    if ( !xInputStream.is() && xStream.is() )
        xInputStream = xStream->getInputStream();

    if(!pDocSh || (sFileName.isEmpty() && !xInputStream.is()))
        return;

    SfxObjectFactory& rFact = pDocSh->GetFactory();
    std::shared_ptr<const SfxFilter> pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( sFilterName );
    uno::Reference < embed::XStorage > xReadStorage;
    if( xInputStream.is() )
    {
        uno::Sequence< uno::Any > aArgs{ uno::Any(xInputStream),
                                         uno::Any(embed::ElementModes::READ) };
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
            pMed->GetItemSet().Put( SfxStringItem( SID_DOC_BASEURL, sBaseURL ) );

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
            pMed->GetItemSet().Put( SfxStringItem( SID_FILE_FILTEROPTIONS, sFilterOptions ) );
        if(!sBaseURL.isEmpty())
            pMed->GetItemSet().Put( SfxStringItem( SID_DOC_BASEURL, sBaseURL ) );
    }

    // this sourcecode is not responsible for the lifetime of the shell, SfxObjectShellLock should not be used
    SfxObjectShellRef aRef( pDocSh );

    pMed->Download();   // if necessary: start the download
    if( !(aRef.is() && 1 < aRef->GetRefCount()) )  // Ref still valid?
        return;

    SwReaderPtr pRdr;
    SfxItemSet& rSet =  pMed->GetItemSet();
    rSet.Put(SfxBoolItem(FN_API_CALL, true));
    if(!sPassword.isEmpty())
        rSet.Put(SfxStringItem(SID_PASSWORD, sPassword));
    Reader *pRead = pDocSh->StartConvertFrom( *pMed, pRdr, nullptr, pUnoCursor);
    if( !pRead )
        return;

    UnoActionContext aContext(&rDoc);

    if(pUnoCursor->HasMark())
        rDoc.getIDocumentContentOperations().DeleteAndJoin(*pUnoCursor);

    SwNodeIndex aSave(  pUnoCursor->GetPoint()->GetNode(), -1 );
    sal_Int32 nContent = pUnoCursor->GetPoint()->GetContentIndex();

    ErrCodeMsg nErrno = pRdr->Read( *pRead );   // and paste the document

    if(!nErrno)
    {
        ++aSave;
        pUnoCursor->SetMark();
        pUnoCursor->GetMark()->Assign( aSave );

        SwContentNode* pCntNode = aSave.GetNode().GetContentNode();
        if( !pCntNode )
            nContent = 0;
        pUnoCursor->GetMark()->SetContent( nContent );
    }
}

// insert text and scan for CR characters in order to insert
// paragraph breaks at those positions by calling SplitNode
bool DocInsertStringSplitCR(
        SwDoc &rDoc,
        const SwPaM &rNewCursor,
        std::u16string_view rText,
        const bool bForceExpandHints )
{
    bool bOK = true;

    for (size_t i = 0; i < rText.size(); ++i)
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
        rNewCursor.GetPoint()->GetNode().GetTextNode();
    if (!pTextNd)
    {
        SAL_INFO("sw.uno", "DocInsertStringSplitCR: need a text node");
        return false;
    }
    OUString aText;
    sal_Int32 nStartIdx = 0;
    const sal_Int32 nMaxLength = COMPLETE_STRING - pTextNd->GetText().getLength();

    size_t nIdx = rText.find( '\r', nStartIdx );
    if( ( nIdx == std::u16string_view::npos && nMaxLength < sal_Int32(rText.size()) ) ||
        ( nIdx != std::u16string_view::npos && nMaxLength < sal_Int32(nIdx) ) )
    {
        nIdx = nMaxLength;
    }
    while (nIdx != std::u16string_view::npos )
    {
        OSL_ENSURE( sal_Int32(nIdx) - nStartIdx >= 0, "index negative!" );
        aText = rText.substr( nStartIdx, nIdx - nStartIdx );
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
        nIdx = rText.find( '\r', nStartIdx );
    }
    aText = rText.substr( nStartIdx );
    if (!aText.isEmpty() &&
        !rDoc.getIDocumentContentOperations().InsertString( rNewCursor, aText, nInsertFlags ))
    {
        OSL_FAIL( "Doc->Insert(Str) failed." );
        bOK = false;
    }

    return bOK;
}

void makeRedline( SwPaM const & rPaM,
    std::u16string_view rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
{
    IDocumentRedlineAccess& rRedlineAccess = rPaM.GetDoc().getIDocumentRedlineAccess();

    RedlineType eType;
    if      ( rRedlineType == u"Insert" )
        eType = RedlineType::Insert;
    else if ( rRedlineType == u"Delete" )
        eType = RedlineType::Delete;
    else if ( rRedlineType == u"Format" )
        eType = RedlineType::Format;
    else if ( rRedlineType == u"TextTable" )
        eType = RedlineType::Table;
    else if ( rRedlineType == u"ParagraphFormat" )
        eType = RedlineType::ParagraphFormat;
    else
        throw lang::IllegalArgumentException();

    //todo: what about REDLINE_FMTCOLL?
    std::size_t nAuthor = 0;
    OUString sAuthor;
    OUString sComment;
    ::util::DateTime aStamp;
    uno::Sequence< beans::PropertyValue > aRevertProperties;
    sal_uInt32 nMovedID = 0;
    bool bFoundComment = false;
    bool bFoundStamp = false;
    bool bFoundRevertProperties = false;
    for (const css::beans::PropertyValue & rProp : rRedlineProperties )
    {
        if (rProp.Name == "RedlineAuthor")
        {
            if( rProp.Value >>= sAuthor )
                nAuthor = rRedlineAccess.InsertRedlineAuthor(sAuthor);
        }
        else if (rProp.Name == "RedlineComment")
            bFoundComment = rProp.Value >>= sComment;
        else if (rProp.Name == "RedlineDateTime")
            bFoundStamp = rProp.Value >>= aStamp;
        else if (rProp.Name == "RedlineRevertProperties")
            bFoundRevertProperties = rProp.Value >>= aRevertProperties;
        else if (rProp.Name == "RedlineMoved")
            rProp.Value >>= nMovedID;
    }

    SwRedlineData aRedlineData( eType, nAuthor );
    if( bFoundComment )
        aRedlineData.SetComment( sComment );
    if( bFoundStamp )
        aRedlineData.SetTimeStamp( DateTime( aStamp));

    std::unique_ptr<SwRedlineExtraData_FormatColl> xRedlineExtraData;

    // Read the 'Redline Revert Properties' from the parameters
    // Check if the value exists
    if ( bFoundRevertProperties )
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
            WhichRangesContainer aWhichPairs;
            std::vector<SfxItemPropertyMapEntry const*> aEntries;
            std::vector<uno::Any> aValues;
            aEntries.reserve(aRevertProperties.getLength());
            sal_uInt16 nStyleId = USHRT_MAX;
            sal_uInt16 nNumId = USHRT_MAX;
            for (const auto& rRevertProperty : aRevertProperties)
            {
                const OUString &rPropertyName = rRevertProperty.Name;
                SfxItemPropertyMapEntry const* pEntry = rPropSet.getPropertyMap().getByName(rPropertyName);

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
                    aWhichPairs = aWhichPairs.MergeRange(RES_PARATR_NUMRULE, RES_PARATR_NUMRULE);
                    nNumId = aEntries.size();
                }
                else
                {
                    aWhichPairs = aWhichPairs.MergeRange(pEntry->nWID, pEntry->nWID);
                    if (rPropertyName == "ParaStyleName")
                        nStyleId = aEntries.size();
                }
                aEntries.push_back(pEntry);
                aValues.push_back(rRevertProperty.Value);
            }

            if (!aWhichPairs.empty())
            {
                sal_uInt16 nStylePoolId = USHRT_MAX;
                OUString sParaStyleName, sUIStyle;
                SfxItemSet aItemSet(rDoc.GetAttrPool(), std::move(aWhichPairs));

                for (size_t i = 0; i < aEntries.size(); ++i)
                {
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
                        SfxItemPropertyMapEntry const*const pEntry = aEntries[i];
                        SfxItemPropertySet::setPropertyValue(*pEntry, rValue, aItemSet);
                        if (i == nStyleId)
                            rValue >>= sParaStyleName;
                    }
                }

                if (eType == RedlineType::ParagraphFormat && sParaStyleName.isEmpty())
                    nStylePoolId = RES_POOLCOLL_STANDARD;

                // tdf#149747 Get UI style name from programmatic style name
                SwStyleNameMapper::FillUIName(sParaStyleName, sUIStyle,
                                              SwGetPoolIdFromName::TxtColl);
                xRedlineExtraData.reset(new SwRedlineExtraData_FormatColl(
                    sUIStyle.isEmpty() ? sParaStyleName : sUIStyle, nStylePoolId, &aItemSet));
            }
            else if (eType == RedlineType::ParagraphFormat)
                xRedlineExtraData.reset(new SwRedlineExtraData_FormatColl( "", RES_POOLCOLL_STANDARD, nullptr ));
        }
    }

    SwRangeRedline* pRedline = new SwRangeRedline( aRedlineData, rPaM );

    // set IsMoved bit of the redline to show and handle moved text
    if ( nMovedID > 0 )
    {
        pRedline->SetMoved( nMovedID );
        rRedlineAccess.GetRedlineTable().setMovedIDIfNeeded(nMovedID);
    }

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
    std::u16string_view rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
{
    SwDoc* pDoc = rTableLine.GetFrameFormat()->GetDoc();
    IDocumentRedlineAccess* pRedlineAccess = &pDoc->getIDocumentRedlineAccess();

    RedlineType eType;
    if ( rRedlineType == u"TableRowInsert" )
    {
        eType = RedlineType::TableRowInsert;
    }
    else if ( rRedlineType == u"TableRowDelete" )
    {
        eType = RedlineType::TableRowDelete;
    }
    else
    {
        throw lang::IllegalArgumentException();
    }

    // set table row property "HasTextChangesOnly" to false
    // to handle tracked deletion or insertion of the table row on the UI
    const SvxPrintItem *pHasTextChangesOnlyProp =
         rTableLine.GetFrameFormat()->GetAttrSet().GetItem<SvxPrintItem>(RES_PRINT);
    if ( !pHasTextChangesOnlyProp || pHasTextChangesOnlyProp->GetValue() )
    {
        SvxPrintItem aSetTracking(RES_PRINT, false);
        SwNodeIndex aInsPos( *(rTableLine.GetTabBoxes()[0]->GetSttNd()), 1 );
        // as a workaround for the rows without text content,
        // add a redline with invisible text CH_TXT_TRACKED_DUMMY_CHAR
        if ( rTableLine.IsEmpty() )
        {
            SwPaM aPaM(aInsPos);
            pDoc->getIDocumentContentOperations().InsertString( aPaM,
                    OUStringChar(CH_TXT_TRACKED_DUMMY_CHAR) );
            aPaM.SetMark();
            aPaM.GetMark()->SetContent(0);
            makeRedline(aPaM, RedlineType::TableRowInsert == eType
                    ? u"Insert"
                    : u"Delete", rRedlineProperties);
        }
        SwCursor aCursor( SwPosition(aInsPos), nullptr );
        pDoc->SetRowNotTracked( aCursor, aSetTracking );
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
    std::u16string_view rRedlineType,
    const uno::Sequence< beans::PropertyValue >& rRedlineProperties )
{
    SwDoc* pDoc = rTableBox.GetFrameFormat()->GetDoc();
    IDocumentRedlineAccess* pRedlineAccess = &pDoc->getIDocumentRedlineAccess();

    RedlineType eType;
    if ( rRedlineType == u"TableCellInsert" )
    {
        eType = RedlineType::TableCellInsert;
    }
    else if ( rRedlineType == u"TableCellDelete" )
    {
        eType = RedlineType::TableCellDelete;
    }
    else
    {
        throw lang::IllegalArgumentException();
    }

    // set table row property "HasTextChangesOnly" to false
    // to handle tracked deletion or insertion of the table row on the UI
    const SvxPrintItem *pHasTextChangesOnlyProp =
         rTableBox.GetFrameFormat()->GetAttrSet().GetItem<SvxPrintItem>(RES_PRINT);
    if ( !pHasTextChangesOnlyProp || pHasTextChangesOnlyProp->GetValue() )
    {
        SvxPrintItem aSetTracking(RES_PRINT, false);
        SwNodeIndex aInsPos( *rTableBox.GetSttNd(), 1 );
        // as a workaround for the cells without text content,
        // add a redline with invisible text CH_TXT_TRACKED_DUMMY_CHAR
        if ( rTableBox.IsEmpty() )
        {
            SwPaM aPaM(aInsPos);
            pDoc->getIDocumentContentOperations().InsertString( aPaM,
                    OUStringChar(CH_TXT_TRACKED_DUMMY_CHAR) );
            aPaM.SetMark();
            aPaM.GetMark()->SetContent(0);
            makeRedline(aPaM, RedlineType::TableCellInsert == eType
                    ? u"Insert"
                    : u"Delete", rRedlineProperties);
        }
        SwCursor aCursor( SwPosition(aInsPos), nullptr );
        pDoc->SetBoxAttr( aCursor, aSetTracking );
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
