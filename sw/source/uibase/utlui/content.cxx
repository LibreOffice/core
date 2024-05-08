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

#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/urlbmk.hxx>
#include <osl/thread.h>
#include <sal/log.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/viewfrm.hxx>
#include <o3tl/enumrange.hxx>
#include <o3tl/sorted_vector.hxx>
#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/weldutils.hxx>
#include <sot/formats.hxx>
#include <o3tl/string_view.hxx>
#include <uiitems.hxx>
#include <fmtanchr.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <fmtfld.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <content.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <fldbas.hxx>
#include <IMark.hxx>
#include <section.hxx>
#include <tox.hxx>
#include <navipi.hxx>
#include <navicont.hxx>
#include <navicfg.hxx>
#include <edtwin.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentOutlineNodes.hxx>
#include <unotxvw.hxx>
#include <cmdid.h>
#include <helpids.h>
#include <strings.hrc>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/ui/XSidebarProvider.hpp>
#include <com/sun/star/ui/XDecks.hpp>
#include <com/sun/star/ui/XDeck.hpp>
#include <com/sun/star/ui/XPanels.hpp>
#include <com/sun/star/ui/XPanel.hpp>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <SwRewriter.hxx>
#include <hints.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <PostItMgr.hxx>
#include <postithelper.hxx>

#include <swabstdlg.hxx>
#include <bitmaps.hlst>

#include <AnnotationWin.hxx>
#include <memory>

#include <fmtcntnt.hxx>
#include <docstat.hxx>

#include <viewopt.hxx>

#include <IDocumentFieldsAccess.hxx>
#include <txtfld.hxx>
#include <fldmgr.hxx>

#include <frameformats.hxx>

#include <ftnidx.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>

#include <txtannotationfld.hxx>
#include <txtfrm.hxx>
#include <txtrfmrk.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include <svx/sdr/overlay/overlayobject.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <node2lay.hxx>

#include <sectfrm.hxx>

#include <docufld.hxx>

#define CTYPE_CNT   0
#define CTYPE_CTT   1

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

namespace {

/*
    Symbolic name representations of numeric values used for the Outline Content Visibility popup
    menu item ids. The numbers are chosen arbitrarily to not over overlap other menu item ids.
    see: SwContentTree::ExecuteContextMenuAction, navigatorcontextmenu.ui

    1512 toggle outline content visibility of the selected outline entry
    1513 make the outline content of the selected outline entry and children not visible
    1514 make the outline content of the selected entry and children visible
*/
const sal_uInt32 TOGGLE_OUTLINE_CONTENT_VISIBILITY = 1512;
const sal_uInt32 HIDE_OUTLINE_CONTENT_VISIBILITY = 1513;
const sal_uInt32 SHOW_OUTLINE_CONTENT_VISIBILITY = 1514;

constexpr char NAVI_BOOKMARK_DELIM = '\x01';

}

class SwContentArr
    : public o3tl::sorted_vector<std::unique_ptr<SwContent>, o3tl::less_uniqueptr_to<SwContent>,
                o3tl::find_partialorder_ptrequals>
{
};

namespace
{
    std::map<OUString, std::map<void*, bool>> lcl_DocOutLineExpandStateMap;

    bool lcl_IsContent(const weld::TreeIter& rEntry, const weld::TreeView& rTreeView)
    {
        return weld::fromId<const SwTypeNumber*>(rTreeView.get_id(rEntry))->GetTypeId() == CTYPE_CNT;
    }

    bool lcl_IsContentType(const weld::TreeIter& rEntry, const weld::TreeView& rTreeView)
    {
        return weld::fromId<const SwTypeNumber*>(rTreeView.get_id(rEntry))->GetTypeId() == CTYPE_CTT;
    }

    bool lcl_IsLowerOutlineContent(const weld::TreeIter& rEntry, const weld::TreeView& rTreeView, sal_uInt8 nLevel)
    {
        return weld::fromId<const SwOutlineContent*>(rTreeView.get_id(rEntry))->GetOutlineLevel() < nLevel;
    }

    bool lcl_FindShell(SwWrtShell const * pShell)
    {
        bool bFound = false;
        SwView *pView = SwModule::GetFirstView();
        while (pView)
        {
            if(pShell == &pView->GetWrtShell())
            {
                bFound = true;
                break;
            }
            pView = SwModule::GetNextView(pView);
        }
        return bFound;
    }

    bool lcl_IsUiVisibleBookmark(const ::sw::mark::IMark* pMark)
    {
        return IDocumentMarkAccess::GetType(*pMark) == IDocumentMarkAccess::MarkType::BOOKMARK;
    }

    OUString lcl_GetFootnoteText(const SwTextFootnote& rTextFootnote)
    {
        SwNodeIndex aIdx(*rTextFootnote.GetStartNode(), 1);
        SwContentNode* pCNd = aIdx.GetNode().GetTextNode();
        if(!pCNd)
            pCNd = aIdx.GetNodes().GoNext(&aIdx);
        return pCNd->IsTextNode() ? static_cast<SwTextNode*>(pCNd)->GetText() : OUString();
    }

    void getAnchorPos(SwPosition& rPos)
    {
        // get the top most anchor position of the position
        if (SwFrameFormat* pFlyFormat = rPos.GetNode().GetFlyFormat())
        {
            SwNode* pAnchorNode;
            SwFrameFormat* pTmp = pFlyFormat;
            while (pTmp && (pAnchorNode = pTmp->GetAnchor().GetAnchorNode()) &&
                   (pTmp = pAnchorNode->GetFlyFormat()))
            {
                pFlyFormat = pTmp;
            }
            if (const SwPosition* pPos = pFlyFormat->GetAnchor().GetContentAnchor())
                rPos = *pPos;
        }
    }

    bool lcl_IsLowerRegionContent(const weld::TreeIter& rEntry, const weld::TreeView& rTreeView, sal_uInt8 nLevel)
    {
        return weld::fromId<const SwRegionContent*>(rTreeView.get_id(rEntry))->GetRegionLevel() < nLevel;
    }
}

// Content, contains names and reference at the content type.

SwContent::SwContent(const SwContentType* pCnt, OUString aName, double nYPos) :
    SwTypeNumber(CTYPE_CNT),
    m_pParent(pCnt),
    m_sContentName(std::move(aName)),
    m_nYPosition(nYPos),
    m_bInvisible(false)
{
}


SwTypeNumber::~SwTypeNumber()
{
}

bool SwContent::IsProtect() const
{
    return false;
}

bool SwTextFieldContent::IsProtect() const
{
    return m_pFormatField->IsProtect();
}

bool SwPostItContent::IsProtect() const
{
    return m_pField->IsProtect();
}

bool SwURLFieldContent::IsProtect() const
{
    return m_pINetAttr->IsProtect();
}

bool SwRegionContent::IsProtect() const
{
    return m_pSectionFormat->GetSection()->IsProtect();
}

SwGraphicContent::~SwGraphicContent()
{
}

SwTOXBaseContent::~SwTOXBaseContent()
{
}

const TranslateId STR_CONTENT_TYPE_ARY[] =
{
    STR_CONTENT_TYPE_OUTLINE,
    STR_CONTENT_TYPE_TABLE,
    STR_CONTENT_TYPE_FRAME,
    STR_CONTENT_TYPE_GRAPHIC,
    STR_CONTENT_TYPE_OLE,
    STR_CONTENT_TYPE_BOOKMARK,
    STR_CONTENT_TYPE_REGION,
    STR_CONTENT_TYPE_URLFIELD,
    STR_CONTENT_TYPE_REFERENCE,
    STR_CONTENT_TYPE_INDEX,
    STR_CONTENT_TYPE_POSTIT,
    STR_CONTENT_TYPE_DRAWOBJECT,
    STR_CONTENT_TYPE_TEXTFIELD,
    STR_CONTENT_TYPE_FOOTNOTE,
    STR_CONTENT_TYPE_ENDNOTE
};

const TranslateId STR_CONTENT_TYPE_SINGLE_ARY[] =
{
    STR_CONTENT_TYPE_SINGLE_OUTLINE,
    STR_CONTENT_TYPE_SINGLE_TABLE,
    STR_CONTENT_TYPE_SINGLE_FRAME,
    STR_CONTENT_TYPE_SINGLE_GRAPHIC,
    STR_CONTENT_TYPE_SINGLE_OLE,
    STR_CONTENT_TYPE_SINGLE_BOOKMARK,
    STR_CONTENT_TYPE_SINGLE_REGION,
    STR_CONTENT_TYPE_SINGLE_URLFIELD,
    STR_CONTENT_TYPE_SINGLE_REFERENCE,
    STR_CONTENT_TYPE_SINGLE_INDEX,
    STR_CONTENT_TYPE_SINGLE_POSTIT,
    STR_CONTENT_TYPE_SINGLE_DRAWOBJECT,
    STR_CONTENT_TYPE_SINGLE_TEXTFIELD,
    STR_CONTENT_TYPE_SINGLE_FOOTNOTE,
    STR_CONTENT_TYPE_SINGLE_ENDNOTE
};

namespace
{
    bool checkVisibilityChanged(
        const SwContentArr& rSwContentArrA,
        const SwContentArr& rSwContentArrB)
    {
        if(rSwContentArrA.size() != rSwContentArrB.size())
        {
            return true;
        }

        for(size_t a(0); a < rSwContentArrA.size(); a++)
        {
            if(rSwContentArrA[a]->IsInvisible() != rSwContentArrB[a]->IsInvisible())
            {
                return true;
            }
        }

        return false;
    }
// Gets "YPos" for content, i.e. a number used to sort content members in Navigator's list
sal_Int32 getYPos(const SwNode& rNode)
{
    SwNodeOffset nIndex = rNode.GetIndex();
    if (rNode.GetNodes().GetEndOfExtras().GetIndex() >= nIndex)
    {
        // Not a node of BodyText
        // Are we in a fly?
        if (const auto pFlyFormat = rNode.GetFlyFormat())
        {
            // Get node index of anchor
            if (SwNode* pAnchorNode = pFlyFormat->GetAnchor().GetAnchorNode())
            {
                return getYPos(*pAnchorNode);
            }
        }
    }
    return sal_Int32(nIndex);
}
} // end of anonymous namespace

SwContentType::SwContentType(SwWrtShell* pShell, ContentTypeId nType, sal_uInt8 nLevel) :
    SwTypeNumber(CTYPE_CTT),
    m_pWrtShell(pShell),
    m_sContentTypeName(SwResId(STR_CONTENT_TYPE_ARY[static_cast<int>(nType)])),
    m_sSingleContentTypeName(SwResId(STR_CONTENT_TYPE_SINGLE_ARY[static_cast<int>(nType)])),
    m_nMemberCount(0),
    m_nContentType(nType),
    m_nOutlineLevel(nLevel),
    m_bDataValid(false),
    m_bEdit(false),
    m_bDelete(true)
{
    switch(m_nContentType)
    {
        case ContentTypeId::OUTLINE:
            m_sTypeToken = "outline";
        break;
        case ContentTypeId::TABLE:
            m_sTypeToken = "table";
            m_bEdit = true;
        break;
        case ContentTypeId::FRAME:
            m_sTypeToken = "frame";
            m_bEdit = true;
        break;
        case ContentTypeId::GRAPHIC:
            m_sTypeToken = "graphic";
            m_bEdit = true;
        break;
        case ContentTypeId::OLE:
            m_sTypeToken = "ole";
            m_bEdit = true;
        break;
        case ContentTypeId::TEXTFIELD:
            m_bEdit = true;
            m_bDelete = true;
        break;
        case ContentTypeId::FOOTNOTE:
        case ContentTypeId::ENDNOTE:
            m_bEdit = true;
            m_bDelete = false;
        break;
        case ContentTypeId::BOOKMARK:
        {
            const bool bProtectedBM = m_pWrtShell->getIDocumentSettingAccess().get(
                        DocumentSettingId::PROTECT_BOOKMARKS);
            m_bEdit = true;
            m_bDelete = !bProtectedBM;
        }
        break;
        case ContentTypeId::REGION:
            m_sTypeToken = "region";
            m_bEdit = true;
            m_bDelete = true;
        break;
        case ContentTypeId::INDEX:
            m_bEdit = true;
            m_bDelete = true;
        break;
        case ContentTypeId::REFERENCE:
            m_bEdit = false;
            m_bDelete = true;
        break;
        case ContentTypeId::URLFIELD:
            m_bEdit = true;
            m_bDelete = true;
        break;
        case ContentTypeId::POSTIT:
            m_bEdit = true;
        break;
        case ContentTypeId::DRAWOBJECT:
            m_sTypeToken = "drawingobject";
            m_bEdit = true;
        break;
        default: break;
    }

    const int nShift = static_cast<int>(m_nContentType);
    assert(nShift > -1);
    const sal_Int32 nMask = 1 << nShift;
    const sal_Int32 nBlock = SW_MOD()->GetNavigationConfig()->GetSortAlphabeticallyBlock();
    m_bAlphabeticSort = nBlock & nMask;

    FillMemberList();
}

SwContentType::~SwContentType()
{
}

const SwContent* SwContentType::GetMember(size_t nIndex)
{
    if(!m_bDataValid || !m_pMember)
    {
        FillMemberList();
    }
    if(nIndex < m_pMember->size())
        return (*m_pMember)[nIndex].get();

    return nullptr;
}

void SwContentType::Invalidate()
{
    m_bDataValid = false;
}

void SwContentType::FillMemberList(bool* pbContentChanged)
{
    std::unique_ptr<SwContentArr> pOldMember;
    size_t nOldMemberCount = 0;
    if(m_pMember && pbContentChanged)
    {
        pOldMember = std::move(m_pMember);
        nOldMemberCount = pOldMember->size();
        m_pMember.reset( new SwContentArr );
        *pbContentChanged = false;
    }
    else if(!m_pMember)
        m_pMember.reset( new SwContentArr );
    else
        m_pMember->clear();
    switch(m_nContentType)
    {
        case ContentTypeId::OUTLINE   :
        {
            const SwNodeOffset nEndOfExtrasIndex = m_pWrtShell->GetNodes().GetEndOfExtras().GetIndex();
            // provide for up to 99999 outline nodes in frames to be sorted in document layout order
            double nOutlinesInFramesIndexAdjustment = 0.00001;
            const SwOutlineNodes& rOutlineNodes(m_pWrtShell->GetNodes().GetOutLineNds());
            const size_t nOutlineCount = rOutlineNodes.size();

            for (size_t i = 0; i < nOutlineCount; ++i)
            {
                SwTextNode* pNode = rOutlineNodes[i]->GetTextNode();
                const sal_uInt8 nLevel = pNode->GetAttrOutlineLevel() - 1;
                if (nLevel >= m_nOutlineLevel)
                    continue;
                double nYPos = m_bAlphabeticSort ? 0 : static_cast<double>(getYPos(*pNode));
                if (nEndOfExtrasIndex >= pNode->GetIndex() && pNode->GetFlyFormat())
                {
                    nYPos += nOutlinesInFramesIndexAdjustment;
                    nOutlinesInFramesIndexAdjustment += 0.00001;
                }
                OUString aEntry(comphelper::string::stripStart(
                                m_pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(
                                i, m_pWrtShell->GetLayout(), true, false, false), ' '));
                aEntry = SwNavigationPI::CleanEntry(aEntry);
                auto pCnt(std::make_unique<SwOutlineContent>(this, aEntry, i, nLevel,
                                                        m_pWrtShell->IsOutlineMovable(i), nYPos));
                if (!pNode->getLayoutFrame(m_pWrtShell->GetLayout()))
                    pCnt->SetInvisible();
                m_pMember->insert(std::move(pCnt));
            }

            // need to check level and equal entry number after creation due to possible outline
            // nodes in frames, headers, footers
            if (pOldMember)
            {
                assert(pbContentChanged && "pbContentChanged is always set if pOldMember is");
                if (pOldMember->size() != m_pMember->size())
                {
                    *pbContentChanged = true;
                    break;
                }
                for (size_t i = 0; i < pOldMember->size(); i++)
                {
                    if (static_cast<SwOutlineContent*>((*pOldMember)[i].get())->GetOutlineLevel() !=
                            static_cast<SwOutlineContent*>((*m_pMember)[i].get())->GetOutlineLevel())
                    {
                        *pbContentChanged = true;
                        break;
                    }
                }
            }
        }
        break;
        case ContentTypeId::TABLE     :
        {
            const size_t nCount = m_pWrtShell->GetTableFrameFormatCount(true);
            const sw::TableFrameFormats* pFrameFormats = m_pWrtShell->GetDoc()->GetTableFrameFormats();
            for(size_t n = 0, i = 0; i < nCount + n; ++i)
            {
                const SwTableFormat& rTableFormat = *(*pFrameFormats)[i];
                if(!rTableFormat.IsUsed())  // skip deleted tables
                {
                    n++;
                    continue;
                }
                tools::Long nYPos = 0;
                if (!m_bAlphabeticSort)
                {
                    if (SwTable* pTable = SwTable::FindTable(&rTableFormat))
                        nYPos = getYPos(*pTable->GetTableNode());
                }
                auto pCnt = std::make_unique<SwContent>(this, rTableFormat.GetName(), nYPos);
                if(!rTableFormat.IsVisible())
                    pCnt->SetInvisible();
                m_pMember->insert(std::move(pCnt));
            }

            if (pOldMember)
            {
                // need to check visibility (and equal entry number) after
                // creation due to a sorted list being used here (before,
                // entries with same index were compared already at creation
                // time what worked before a sorted list was used)
                *pbContentChanged = checkVisibilityChanged(
                    *pOldMember,
                    *m_pMember);
            }
        }
        break;
        case ContentTypeId::OLE       :
        case ContentTypeId::FRAME     :
        case ContentTypeId::GRAPHIC   :
        {
            FlyCntType eType = FLYCNTTYPE_FRM;
            if(m_nContentType == ContentTypeId::OLE)
                eType = FLYCNTTYPE_OLE;
            else if(m_nContentType == ContentTypeId::GRAPHIC)
                eType = FLYCNTTYPE_GRF;
            Point aNullPt;
            size_t nCount = m_pWrtShell->GetFlyCount(eType, /*bIgnoreTextBoxes=*/true);
            std::vector<SwFrameFormat const*> formats(m_pWrtShell->GetFlyFrameFormats(eType, /*bIgnoreTextBoxes=*/true));
            SAL_WARN_IF(nCount != formats.size(), "sw.ui", "Count differs");
            nCount = formats.size();
            for (size_t i = 0; i < nCount; ++i)
            {
                SwFrameFormat const*const pFrameFormat = formats[i];
                const OUString sFrameName = pFrameFormat->GetName();

                SwContent* pCnt;
                tools::Long nYPos =
                        m_bAlphabeticSort ? 0 : pFrameFormat->FindLayoutRect(false, &aNullPt).Top();
                if(ContentTypeId::GRAPHIC == m_nContentType)
                {
                    OUString sLink;
                    m_pWrtShell->GetGrfNms( &sLink, nullptr, static_cast<const SwFlyFrameFormat*>( pFrameFormat));
                    pCnt = new SwGraphicContent(this, sFrameName, INetURLObject::decode(sLink,
                                           INetURLObject::DecodeMechanism::Unambiguous), nYPos);
                }
                else
                {
                    pCnt = new SwContent(this, sFrameName, nYPos);
                }
                if(!pFrameFormat->IsVisible())
                    pCnt->SetInvisible();
                m_pMember->insert(std::unique_ptr<SwContent>(pCnt));
            }

            if (pOldMember)
            {
                // need to check visibility (and equal entry number) after
                // creation due to a sorted list being used here (before,
                // entries with same index were compared already at creation
                // time what worked before a sorted list was used)
                assert(pbContentChanged && "pbContentChanged is always set if pOldMember is");
                *pbContentChanged = checkVisibilityChanged(
                    *pOldMember,
                    *m_pMember);
            }
        }
        break;
        case ContentTypeId::BOOKMARK:
        {
            tools::Long nYPos = 0;
            IDocumentMarkAccess* const pMarkAccess = m_pWrtShell->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
                ppBookmark != pMarkAccess->getBookmarksEnd();
                ++ppBookmark)
            {
                if(lcl_IsUiVisibleBookmark(*ppBookmark))
                {
                    const OUString& rBkmName = (*ppBookmark)->GetName();
                    //nYPos from 0 -> text::Bookmarks will be sorted alphabetically
                    auto pCnt(std::make_unique<SwContent>(this, rBkmName,
                                                          m_bAlphabeticSort ? 0 : nYPos++));
                    m_pMember->insert(std::move(pCnt));
                }
            }
        }
        break;
        case ContentTypeId::TEXTFIELD:
        {
            std::vector<SwTextField*> aArr;
            const SwFieldTypes& rFieldTypes =
                    *m_pWrtShell->GetDoc()->getIDocumentFieldsAccess().GetFieldTypes();
            const size_t nSize = rFieldTypes.size();
            for (size_t i = 0; i < nSize; ++i)
            {
                const SwFieldType* pFieldType = rFieldTypes[i].get();
                if (pFieldType->Which() == SwFieldIds::Postit)
                    continue;
                std::vector<SwFormatField*> vFields;
                pFieldType->GatherFields(vFields);
                for (SwFormatField* pFormatField: vFields)
                {
                    if (SwTextField* pTextField = pFormatField->GetTextField())
                    {
                        // fields in header footer don't behave well, skip them
                        if (m_pWrtShell->GetDoc()->IsInHeaderFooter(pTextField->GetTextNode()))
                            continue;
                        aArr.emplace_back(pTextField);
                    }
                }
            }
            if (!m_bAlphabeticSort)
            {
                const SwNodeOffset nEndOfExtrasIndex =
                        m_pWrtShell->GetNodes().GetEndOfExtras().GetIndex();
                // use stable sort array to list fields in document model order
                std::stable_sort(aArr.begin(), aArr.end(),
                                 [&nEndOfExtrasIndex, this](
                                 const SwTextField* a, const SwTextField* b){
                    SwPosition aPos(a->GetTextNode(), a->GetStart());
                    SwPosition bPos(b->GetTextNode(), b->GetStart());
                    // use anchor position for entries that are located in flys
                    if (nEndOfExtrasIndex >= aPos.GetNodeIndex())
                        getAnchorPos(aPos);
                    if (nEndOfExtrasIndex >= bPos.GetNodeIndex())
                        getAnchorPos(bPos);
                    if (aPos == bPos)
                    {
                        // probably in same or nested fly frame
                        // sort using layout position
                        SwRect aCharRect, bCharRect;
                        std::shared_ptr<SwPaM> pPamForTextField;
                        if (SwTextFrame* pFrame = static_cast<SwTextFrame*>(
                                    a->GetTextNode().getLayoutFrame(m_pWrtShell->GetLayout())))
                        {
                            SwTextField::GetPamForTextField(*a, pPamForTextField);
                            if (pPamForTextField)
                                pFrame->GetCharRect(aCharRect, *pPamForTextField->GetPoint());
                        }
                        if (SwTextFrame* pFrame = static_cast<SwTextFrame*>(
                                    b->GetTextNode().getLayoutFrame(m_pWrtShell->GetLayout())))
                        {
                            SwTextField::GetPamForTextField(*b, pPamForTextField);
                            if (pPamForTextField)
                                pFrame->GetCharRect(bCharRect, *pPamForTextField->GetPoint());
                        }
                        return aCharRect.Top() < bCharRect.Top();
                    }
                    return aPos < bPos;});
            }
            std::vector<OUString> aDocumentStatisticsSubTypesList;
            tools::Long nYPos = 0;
            for (SwTextField* pTextField : aArr)
            {
                const SwField* pField = pTextField->GetFormatField().GetField();
                OUString sExpandField = pField->ExpandField(true, m_pWrtShell->GetLayout());
                if (!sExpandField.isEmpty())
                    sExpandField = u" - " + sExpandField;
                OUString sText;
                if (pField->GetTypeId() == SwFieldTypesEnum::DocumentStatistics)
                {
                    if (aDocumentStatisticsSubTypesList.empty())
                        SwFieldMgr(m_pWrtShell).GetSubTypes(SwFieldTypesEnum::DocumentStatistics,
                                                            aDocumentStatisticsSubTypesList);
                    OUString sSubType;
                    if (pField->GetSubType() < aDocumentStatisticsSubTypesList.size())
                        sSubType = u" - " + aDocumentStatisticsSubTypesList[pField->GetSubType()];
                    sText = pField->GetDescription() + u" - " + pField->GetFieldName() + sSubType +
                            sExpandField;
                }
                else if (pField->GetTypeId() == SwFieldTypesEnum::GetRef)
                {
                    assert(dynamic_cast<const SwGetRefField*>(pField));
                    const SwGetRefField* pRefField(static_cast<const SwGetRefField*>(pField));
                    if (pRefField->IsRefToHeadingCrossRefBookmark() ||
                            pRefField->IsRefToNumItemCrossRefBookmark())
                    {
                        OUString sExpandedTextOfReferencedTextNode =
                                pRefField->GetExpandedTextOfReferencedTextNode(
                                    *m_pWrtShell->GetLayout(), nullptr, nullptr);
                        if (sExpandedTextOfReferencedTextNode.getLength() > 80)
                        {
                            sExpandedTextOfReferencedTextNode = OUString::Concat(
                                        sExpandedTextOfReferencedTextNode.subView(0, 80)) + u"...";
                        }
                        sText = pField->GetDescription() + u" - "
                                + sExpandedTextOfReferencedTextNode + sExpandField;
                    }
                    else
                    {
                        OUString sFieldSubTypeOrName;
                        auto nSubType = pField->GetSubType();
                        if (nSubType == REF_FOOTNOTE)
                            sFieldSubTypeOrName = SwResId(STR_FLDREF_FOOTNOTE);
                        else if (nSubType == REF_ENDNOTE)
                            sFieldSubTypeOrName = SwResId(STR_FLDREF_ENDNOTE);
                        else
                            sFieldSubTypeOrName = pField->GetFieldName();
                        sText = pField->GetDescription() + u" - " + sFieldSubTypeOrName
                                + sExpandField;
                    }
                }
                else
                    sText = pField->GetDescription() + u" - " + pField->GetFieldName()
                            + sExpandField;
                auto pCnt(std::make_unique<SwTextFieldContent>(this, sText,
                                                               &pTextField->GetFormatField(),
                                                               m_bAlphabeticSort ? 0 : nYPos++));
                if (!pTextField->GetTextNode().getLayoutFrame(m_pWrtShell->GetLayout()))
                    pCnt->SetInvisible();
                m_pMember->insert(std::move(pCnt));
            }
        }
        break;
        // We will separate footnotes and endnotes here.
        case ContentTypeId::FOOTNOTE:
        case ContentTypeId::ENDNOTE:
        {
            const SwFootnoteIdxs& rFootnoteIdxs = m_pWrtShell->GetDoc()->GetFootnoteIdxs();
            if (rFootnoteIdxs.size() == 0)
                break;
            // insert footnotes and endnotes
            tools::Long nPos = 0;
            for (const SwTextFootnote* pTextFootnote : rFootnoteIdxs)
            {
                if ((!pTextFootnote->GetFootnote().IsEndNote()
                     && m_nContentType == ContentTypeId::FOOTNOTE)
                    || (pTextFootnote->GetFootnote().IsEndNote()
                        && m_nContentType == ContentTypeId::ENDNOTE))
                {
                    const SwFormatFootnote& rFormatFootnote = pTextFootnote->GetFootnote();
                    const OUString& sText
                        = rFormatFootnote.GetViewNumStr(*m_pWrtShell->GetDoc(),
                                                        m_pWrtShell->GetLayout(), true)
                          + " " + lcl_GetFootnoteText(*pTextFootnote);
                    auto pCnt(std::make_unique<SwTextFootnoteContent>(
                        this, sText, pTextFootnote, ++nPos));
                    if (!pTextFootnote->GetTextNode().getLayoutFrame(m_pWrtShell->GetLayout()))
                        pCnt->SetInvisible();
                    m_pMember->insert(std::move(pCnt));
                }
            }
        }
        break;
        case ContentTypeId::REGION    :
        {
            size_t nCount = m_pWrtShell->GetSectionFormatCount();
            for (size_t i = 0; i < nCount; ++i)
            {
                const SwSectionFormat* pFormat = &m_pWrtShell->GetSectionFormat(i);
                if (!pFormat->IsInNodesArr())
                    continue;
                const SwSection* pSection = pFormat->GetSection();
                if (SectionType eTmpType = pSection->GetType();
                    eTmpType == SectionType::ToxContent || eTmpType == SectionType::ToxHeader)
                    continue;
                const SwNodeIndex* pNodeIndex = pFormat->GetContent().GetContentIdx();
                if (pNodeIndex)
                {
                    const OUString& sSectionName = pSection->GetSectionName();

                    sal_uInt8 nLevel = 0;
                    SwSectionFormat* pParentFormat = pFormat->GetParent();
                    while(pParentFormat)
                    {
                        nLevel++;
                        pParentFormat = pParentFormat->GetParent();
                    }

                    auto pCnt(std::make_unique<SwRegionContent>(this, sSectionName, nLevel,
                                            m_bAlphabeticSort ? 0 : getYPos(pNodeIndex->GetNode()),
                                                                pFormat));

                    if (!pFormat->IsVisible() || pSection->IsHidden())
                        pCnt->SetInvisible();
                    m_pMember->insert(std::move(pCnt));
                }

                if (pOldMember)
                {
                    // need to check visibility (and equal entry number) after
                    // creation due to a sorted list being used here (before,
                    // entries with same index were compared already at creation
                    // time what worked before a sorted list was used)
                    assert(pbContentChanged && "pbContentChanged is always set if pOldMember is");
                    *pbContentChanged = checkVisibilityChanged(
                        *pOldMember,
                        *m_pMember);
                }
            }
        }
        break;
        case ContentTypeId::REFERENCE:
        {
            std::vector<OUString> aRefMarks;
            m_pWrtShell->GetRefMarks( &aRefMarks );

            tools::Long nYPos = 0;
            for (const auto& rRefMark : aRefMarks)
            {
                m_pMember->insert(std::make_unique<SwContent>(this, rRefMark,
                                                              m_bAlphabeticSort ? 0 : nYPos++));
            }
        }
        break;
        case ContentTypeId::URLFIELD:
        {
            SwGetINetAttrs aArr;
            m_pWrtShell->GetINetAttrs(aArr, false);

            if (m_bAlphabeticSort)
            {
                for (auto& r : aArr)
                {
                    auto pCnt(std::make_unique<SwURLFieldContent>(this, r.sText, INetURLObject::decode(
                                                    r.rINetAttr.GetINetFormat().GetValue(),
                                                    INetURLObject::DecodeMechanism::Unambiguous),
                                                             &r.rINetAttr, 0));
                    m_pMember->insert(std::move(pCnt));
                }
                break;
            }

            // use stable sort array to list hyperlinks in document order
            const SwNodeOffset nEndOfExtrasIndex = m_pWrtShell->GetNodes().GetEndOfExtras().GetIndex();
            bool bHasEntryInFly = false;
            std::vector<SwGetINetAttr*> aStableSortINetAttrsArray;

            for (SwGetINetAttr& r : aArr)
            {
                aStableSortINetAttrsArray.emplace_back(&r);
                if (!bHasEntryInFly)
                {
                    if (nEndOfExtrasIndex >= r.rINetAttr.GetTextNode().GetIndex())
                    {
                        // Not a node of BodyText
                        // Are we in a fly?
                        if (r.rINetAttr.GetTextNode().GetFlyFormat())
                            bHasEntryInFly = true;
                    }
                }
            }

            std::stable_sort(aStableSortINetAttrsArray.begin(), aStableSortINetAttrsArray.end(),
                             [](const SwGetINetAttr* a, const SwGetINetAttr* b){
                SwPosition aSwPos(a->rINetAttr.GetTextNode(),
                                  a->rINetAttr.GetStart());
                SwPosition bSwPos(b->rINetAttr.GetTextNode(),
                                  b->rINetAttr.GetStart());
                return aSwPos < bSwPos;});

            // When there are hyperlinks in text frames do an additional sort using the text frame
            // anchor position to place entries in the order of document layout appearance.
            if (bHasEntryInFly)
            {
                std::stable_sort(aStableSortINetAttrsArray.begin(), aStableSortINetAttrsArray.end(),
                                 [nEndOfExtrasIndex](const SwGetINetAttr* a, const SwGetINetAttr* b){
                    const SwTextNode& aTextNode = a->rINetAttr.GetTextNode();
                    const SwTextNode& bTextNode = b->rINetAttr.GetTextNode();
                    SwPosition aPos(aTextNode, a->rINetAttr.GetStart());
                    SwPosition bPos(bTextNode, b->rINetAttr.GetStart());
                    // use anchor position for entries that are located in flys
                    if (nEndOfExtrasIndex >= aTextNode.GetIndex())
                        if (auto pFlyFormat = aTextNode.GetFlyFormat())
                            if (const SwPosition* pPos = pFlyFormat->GetAnchor().GetContentAnchor())
                                aPos = *pPos;
                    if (nEndOfExtrasIndex >= bTextNode.GetIndex())
                        if (auto pFlyFormat = bTextNode.GetFlyFormat())
                            if (const SwPosition* pPos = pFlyFormat->GetAnchor().GetContentAnchor())
                                bPos = *pPos;
                    return aPos < bPos;});
            }

            SwGetINetAttrs::size_type n = 0;
            for (auto p : aStableSortINetAttrsArray)
            {
                auto pCnt = std::make_unique<SwURLFieldContent>(this, p->sText,
                            INetURLObject::decode(p->rINetAttr.GetINetFormat().GetValue(),
                                                  INetURLObject::DecodeMechanism::Unambiguous),
                            &p->rINetAttr, ++n);
                m_pMember->insert(std::move(pCnt));
            }
        }
        break;
        case ContentTypeId::INDEX:
        {
            const sal_uInt16 nCount = m_pWrtShell->GetTOXCount();

            for ( sal_uInt16 nTox = 0; nTox < nCount; nTox++ )
            {
                const SwTOXBase* pBase = m_pWrtShell->GetTOX( nTox );
                OUString sTOXNm( pBase->GetTOXName() );

                SwContent* pCnt = new SwTOXBaseContent(
                        this, sTOXNm, m_bAlphabeticSort ? 0 : nTox, *pBase);

                if(pBase && !pBase->IsVisible())
                    pCnt->SetInvisible();

                m_pMember->insert( std::unique_ptr<SwContent>(pCnt) );
                const size_t nPos = m_pMember->size() - 1;
                if (pOldMember)
                {
                    assert(pbContentChanged && "pbContentChanged is always set if pOldMember is");
                    if (!*pbContentChanged && nOldMemberCount > nPos &&
                            (*pOldMember)[nPos]->IsInvisible() != pCnt->IsInvisible())
                        *pbContentChanged = true;
                }
            }
        }
        break;
        case ContentTypeId::POSTIT:
        {
            SwPostItMgr* aMgr = m_pWrtShell->GetView().GetPostItMgr();
            if (aMgr)
            {
                tools::Long nYPos = 0;
                for(SwPostItMgr::const_iterator i = aMgr->begin(); i != aMgr->end(); ++i)
                {
                    if (const SwFormatField* pFormatField = dynamic_cast<const SwFormatField *>((*i)->GetBroadcaster())) // SwPostit
                    {
                        if (pFormatField->GetTextField() && pFormatField->IsFieldInDoc())
                        {
                            OUString sEntry = pFormatField->GetField()->GetPar2();
                            sEntry = RemoveNewline(sEntry);
                            std::unique_ptr<SwPostItContent> pCnt(new SwPostItContent(
                                                this,
                                                sEntry,
                                                pFormatField,
                                                nYPos));
                            if (!pFormatField->GetTextField()->GetTextNode().getLayoutFrame(
                                        m_pWrtShell->GetLayout()))
                                pCnt->SetInvisible();
                            if (pOldMember)
                            {
                                assert(pbContentChanged && "pbContentChanged is always set if pOldMember is");
                                if (!*pbContentChanged &&
                                        nOldMemberCount > o3tl::make_unsigned(nYPos) &&
                                        (*pOldMember)[nYPos]->IsInvisible() != pCnt->IsInvisible())
                                    *pbContentChanged = true;
                            }
                            m_pMember->insert(std::move(pCnt));
                            nYPos++;
                        }
                    }
                }
            }
        }
        break;
        case ContentTypeId::DRAWOBJECT:
        {
            IDocumentDrawModelAccess& rIDDMA = m_pWrtShell->getIDocumentDrawModelAccess();
            SwDrawModel* pModel = rIDDMA.GetDrawModel();
            if(pModel)
            {
                SdrPage* pPage = pModel->GetPage(0);
                for (const rtl::Reference<SdrObject>& pTemp : *pPage)
                {
                    // #i51726# - all drawing objects can be named now
                    if (!pTemp->IsVirtualObj() && !pTemp->GetName().isEmpty())
                    {
                        tools::Long nYPos = LONG_MIN;
                        const bool bIsVisible = rIDDMA.IsVisibleLayerId(pTemp->GetLayer());
                        if (bIsVisible)
                            nYPos = m_bAlphabeticSort ? 0 : pTemp->GetLogicRect().Top();
                        auto pCnt(std::make_unique<SwContent>(this, pTemp->GetName(), nYPos));
                        if (!bIsVisible)
                            pCnt->SetInvisible();
                        m_pMember->insert(std::move(pCnt));
                    }
                }

                if (pOldMember)
                {
                    // need to check visibility (and equal entry number) after
                    // creation due to a sorted list being used here (before,
                    // entries with same index were compared already at creation
                    // time what worked before a sorted list was used)
                    assert(pbContentChanged && "pbContentChanged is always set if pOldMember is");
                    *pbContentChanged = checkVisibilityChanged(
                        *pOldMember,
                        *m_pMember);
                }
            }
        }
        break;
        default: break;
    }
    m_nMemberCount = m_pMember->size();
    if (pOldMember)
    {
        assert(pbContentChanged && "pbContentChanged is always set if pOldMember is");
        if (!*pbContentChanged && pOldMember->size() != m_nMemberCount)
            *pbContentChanged = true;
    }

    m_bDataValid = true;
}

namespace {

enum STR_CONTEXT_IDX
{
    IDX_STR_OUTLINE_LEVEL = 0,
    IDX_STR_DRAGMODE = 1,
    IDX_STR_HYPERLINK = 2,
    IDX_STR_LINK_REGION = 3,
    IDX_STR_COPY_REGION = 4,
    IDX_STR_DISPLAY = 5,
    IDX_STR_ACTIVE_VIEW = 6,
    IDX_STR_HIDDEN = 7,
    IDX_STR_ACTIVE = 8,
    IDX_STR_INACTIVE = 9,
    IDX_STR_EDIT_ENTRY = 10,
    IDX_STR_DELETE_ENTRY = 11,
    IDX_STR_SEND_OUTLINE_TO_CLIPBOARD_ENTRY = 12,
    IDX_STR_OUTLINE_TRACKING = 13,
    IDX_STR_OUTLINE_TRACKING_DEFAULT = 14,
    IDX_STR_OUTLINE_TRACKING_FOCUS = 15,
    IDX_STR_OUTLINE_TRACKING_OFF = 16
};

}

const TranslateId STR_CONTEXT_ARY[] =
{
    STR_OUTLINE_LEVEL,
    STR_DRAGMODE,
    STR_HYPERLINK,
    STR_LINK_REGION,
    STR_COPY_REGION,
    STR_DISPLAY,
    STR_ACTIVE_VIEW,
    STR_HIDDEN,
    STR_ACTIVE,
    STR_INACTIVE,
    STR_EDIT_ENTRY,
    STR_DELETE_ENTRY,
    STR_SEND_OUTLINE_TO_CLIPBOARD_ENTRY,
    STR_OUTLINE_TRACKING,
    STR_OUTLINE_TRACKING_DEFAULT,
    STR_OUTLINE_TRACKING_FOCUS,
    STR_OUTLINE_TRACKING_OFF
};

SwContentTree::SwContentTree(std::unique_ptr<weld::TreeView> xTreeView, SwNavigationPI* pDialog)
    : m_xTreeView(std::move(xTreeView))
    , m_aDropTargetHelper(*this)
    , m_pDialog(pDialog)
    , m_sSpace(OUString("                    "))
    , m_aUpdTimer("SwContentTree m_aUpdTimer")
    , m_aOverlayObjectDelayTimer("SwContentTree m_aOverlayObjectDelayTimer")
    , m_sInvisible(SwResId(STR_INVISIBLE))
    , m_pHiddenShell(nullptr)
    , m_pActiveShell(nullptr)
    , m_pConfig(SW_MOD()->GetNavigationConfig())
    , m_nActiveBlock(0)
    , m_nHiddenBlock(0)
    , m_nEntryCount(0)
    , m_nRootType(ContentTypeId::UNKNOWN)
    , m_nLastSelType(ContentTypeId::UNKNOWN)
    , m_nOutlineLevel(MAXLEVEL)
    , m_eState(State::ACTIVE)
    , m_bIsRoot(false)
    , m_bIsIdleClear(false)
    , m_bIsLastReadOnly(false)
    , m_bIsOutlineMoveable(true)
    , m_bViewHasChanged(false)
{
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 30,
                                  m_xTreeView->get_text_height() * 14);

    m_xTreeView->set_help_id(HID_NAVIGATOR_TREELIST);

    m_xTreeView->connect_expanding(LINK(this, SwContentTree, ExpandHdl));
    m_xTreeView->connect_collapsing(LINK(this, SwContentTree, CollapseHdl));
    m_xTreeView->connect_row_activated(LINK(this, SwContentTree, ContentDoubleClickHdl));
    m_xTreeView->connect_changed(LINK(this, SwContentTree, SelectHdl));
    m_xTreeView->connect_focus_in(LINK(this, SwContentTree, FocusInHdl));
    m_xTreeView->connect_key_press(LINK(this, SwContentTree, KeyInputHdl));
    m_xTreeView->connect_popup_menu(LINK(this, SwContentTree, CommandHdl));
    m_xTreeView->connect_query_tooltip(LINK(this, SwContentTree, QueryTooltipHdl));
    m_xTreeView->connect_drag_begin(LINK(this, SwContentTree, DragBeginHdl));
    m_xTreeView->connect_mouse_move(LINK(this, SwContentTree, MouseMoveHdl));
    m_xTreeView->connect_mouse_press(LINK(this, SwContentTree, MousePressHdl));

    for (ContentTypeId i : o3tl::enumrange<ContentTypeId>())
    {
        if (i != ContentTypeId::OUTLINE)
            mTrackContentType[i] = true;
        m_aActiveContentArr[i] = nullptr;
        m_aHiddenContentArr[i] = nullptr;
    }
    for (int i = 0; i < CONTEXT_COUNT; ++i)
    {
        m_aContextStrings[i] = SwResId(STR_CONTEXT_ARY[i]);
    }
    m_nActiveBlock = m_pConfig->GetActiveBlock();

    // Restore outline headings expand state (same session persistence only)
    if (SwView* pView = GetActiveView(); pView && pView->GetDocShell())
    {
        OUString sDocTitle = pView->GetDocShell()->GetTitle();
        auto it = lcl_DocOutLineExpandStateMap.find(sDocTitle);
        if (it != lcl_DocOutLineExpandStateMap.end())
            mOutLineNodeMap = it->second;
        if (comphelper::LibreOfficeKit::isActive()) {
            if (pView->m_nNaviExpandedStatus < 0)
                m_nActiveBlock = 1;
            else
                m_nActiveBlock = pView->m_nNaviExpandedStatus;
        }
    }

    m_aUpdTimer.SetInvokeHandler(LINK(this, SwContentTree, TimerUpdate));
    m_aUpdTimer.SetTimeout(1000);
    m_aOverlayObjectDelayTimer.SetInvokeHandler(LINK(this, SwContentTree, OverlayObjectDelayTimerHdl));
    m_aOverlayObjectDelayTimer.SetTimeout(500);
}

SwContentTree::~SwContentTree()
{
    if (SwView* pView = GetActiveView(); pView && pView->GetDocShell())
    {
        OUString sDocTitle = pView->GetDocShell()->GetTitle();
        lcl_DocOutLineExpandStateMap[sDocTitle] = mOutLineNodeMap;
        if (comphelper::LibreOfficeKit::isActive())
            pView->m_nNaviExpandedStatus = m_nActiveBlock;
    }
    clear(); // If applicable erase content types previously.
    m_aUpdTimer.Stop();
    SetActiveShell(nullptr);
}

IMPL_LINK(SwContentTree, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    m_bSelectTo = rMEvt.IsShift() && (m_pConfig->IsNavigateOnSelect() || rMEvt.GetClicks() == 2);
    return false;
}

IMPL_LINK(SwContentTree, MouseMoveHdl, const MouseEvent&, rMEvt, bool)
{
    if (m_eState == State::HIDDEN)
        return false;
    if (std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
            m_xTreeView->get_dest_row_at_pos(rMEvt.GetPosPixel(), xEntry.get(), false, false) &&
            !rMEvt.IsLeaveWindow())
    {
        if (!m_xOverlayCompareEntry)
            m_xOverlayCompareEntry.reset(m_xTreeView->make_iterator().release());
        else if (m_xTreeView->iter_compare(*xEntry, *m_xOverlayCompareEntry) == 0)
            return false; // The entry under the mouse has not changed.
        m_xTreeView->copy_iterator(*xEntry, *m_xOverlayCompareEntry);
        BringEntryToAttention(*xEntry);
    }
    else
    {
        if (m_xOverlayCompareEntry)
            m_xOverlayCompareEntry.reset();
        m_aOverlayObjectDelayTimer.Stop();
        if (m_xOverlayObject && m_xOverlayObject->getOverlayManager())
        {
            m_xOverlayObject->getOverlayManager()->remove(*m_xOverlayObject);
            m_xOverlayObject.reset();
        }
    }
    return false;
}

// Drag&Drop methods
IMPL_LINK(SwContentTree, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = true;

    bool bDisallow = true;

    // don't allow if tree root is selected
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    bool bEntry = m_xTreeView->get_selected(xEntry.get());
    if (!bEntry || lcl_IsContentType(*xEntry, *m_xTreeView))
    {
        return true; // disallow
    }

    rtl::Reference<TransferDataContainer> xContainer = new TransferDataContainer;
    sal_Int8 nDragMode = DND_ACTION_COPYMOVE | DND_ACTION_LINK;

    if (FillTransferData(*xContainer, nDragMode))
        bDisallow = false;

    if (m_bIsRoot && m_nRootType == ContentTypeId::OUTLINE)
    {
        // Only move drag entry and continuous selected siblings:
        m_aDndOutlinesSelected.clear();

        std::unique_ptr<weld::TreeIter> xScratch(m_xTreeView->make_iterator());

        // Find first selected of continuous siblings
        while (true)
        {
            m_xTreeView->copy_iterator(*xEntry, *xScratch);
            if (!m_xTreeView->iter_previous_sibling(*xScratch))
                break;
            if (!m_xTreeView->is_selected(*xScratch))
                break;
            m_xTreeView->copy_iterator(*xScratch, *xEntry);
        }
        // Record continuous selected siblings
        do
        {
            m_aDndOutlinesSelected.push_back(m_xTreeView->make_iterator(xEntry.get()));
        }
        while (m_xTreeView->iter_next_sibling(*xEntry) && m_xTreeView->is_selected(*xEntry));
        bDisallow = false;
    }

    if (!bDisallow)
        m_xTreeView->enable_drag_source(xContainer, nDragMode);
    return bDisallow;
}

SwContentTreeDropTarget::SwContentTreeDropTarget(SwContentTree& rTreeView)
    : DropTargetHelper(rTreeView.get_widget().get_drop_target())
    , m_rTreeView(rTreeView)
{
}

sal_Int8 SwContentTreeDropTarget::AcceptDrop(const AcceptDropEvent& rEvt)
{
    sal_Int8 nAccept = m_rTreeView.AcceptDrop(rEvt);

    if (nAccept != DND_ACTION_NONE)
    {
        // to enable the autoscroll when we're close to the edges
        weld::TreeView& rWidget = m_rTreeView.get_widget();
        rWidget.get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);
    }

    return nAccept;
}

bool SwContentTree::IsInDrag() const
{
    return m_xTreeView->get_drag_source() == m_xTreeView.get();
}

bool SwContentTree::HasHeadings() const
{
    const std::unique_ptr<SwContentType>& rpContentT = m_aActiveContentArr[ContentTypeId::OUTLINE];
    if (rpContentT && rpContentT->GetMemberCount() > 0)
        return true;
    return false;
}

// QueryDrop will be executed in the navigator
sal_Int8 SwContentTree::AcceptDrop(const AcceptDropEvent& rEvt)
{
    sal_Int8 nRet = DND_ACTION_NONE;
    if( m_bIsRoot )
    {
        if( m_bIsOutlineMoveable )
            nRet = rEvt.mnAction;
    }
    else if (!IsInDrag())
        nRet = GetParentWindow()->AcceptDrop();
    return nRet;
}

// Drop will be executed in the navigator
static void* lcl_GetOutlineKey(SwContentTree& rTree, SwOutlineContent const * pContent)
{
    void* key = nullptr;
    if (pContent)
    {
        SwWrtShell* pShell = rTree.GetWrtShell();
        auto const nPos = pContent->GetOutlinePos();

        key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
    }
    return key;
}

sal_Int8 SwContentTreeDropTarget::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    return m_rTreeView.ExecuteDrop(rEvt);
}

sal_Int8 SwContentTree::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    std::unique_ptr<weld::TreeIter> xDropEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_dest_row_at_pos(rEvt.maPosPixel, xDropEntry.get(), true))
        xDropEntry.reset();

    if (m_nRootType == ContentTypeId::OUTLINE)
    {
        if (xDropEntry && lcl_IsContent(*xDropEntry, *m_xTreeView))
        {
            assert(dynamic_cast<SwContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xDropEntry))));
            SwOutlineContent* pOutlineContent = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(*xDropEntry));
            assert(pOutlineContent);

            void* key = lcl_GetOutlineKey(*this, pOutlineContent);
            assert(key);
            if (!mOutLineNodeMap[key])
            {
                while (m_xTreeView->iter_has_child(*xDropEntry))
                {
                    std::unique_ptr<weld::TreeIter> xChildEntry(m_xTreeView->make_iterator(xDropEntry.get()));
                    bool bChildEntry = m_xTreeView->iter_children(*xChildEntry);
                    while (bChildEntry)
                    {
                        m_xTreeView->copy_iterator(*xChildEntry, *xDropEntry);
                        bChildEntry = m_xTreeView->iter_next_sibling(*xChildEntry);
                    }
                }
            }
        }

        SwOutlineNodes::size_type nTargetPos = 0;
        if (!xDropEntry)
        {
            // dropped in blank space -> move to bottom
            nTargetPos = GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount() - 1;
        }
        else if (!lcl_IsContent(*xDropEntry, *m_xTreeView))
        {
            // dropped on "heading" parent -> move to start
            nTargetPos = SwOutlineNodes::npos;
        }
        else
        {
            assert(dynamic_cast<SwOutlineContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xDropEntry))));
            nTargetPos = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(*xDropEntry))->GetOutlinePos();
        }

        if( MAXLEVEL > m_nOutlineLevel && // Not all layers are displayed.
                        nTargetPos != SwOutlineNodes::npos)
        {
            std::unique_ptr<weld::TreeIter> xNext(m_xTreeView->make_iterator(xDropEntry.get()));
            bool bNext = m_xTreeView->iter_next(*xNext);
            if (bNext)
            {
                assert(dynamic_cast<SwOutlineContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xNext))));
                nTargetPos = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(*xNext))->GetOutlinePos() - 1;
            }
            else
                nTargetPos = GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount() - 1;
        }

        // remove the drop highlight before we change the contents of the tree so we don't
        // try and dereference a removed entry in post-processing drop
        m_xTreeView->unset_drag_dest_row();
        MoveOutline(nTargetPos);

    }
    return IsInDrag() ? DND_ACTION_NONE : GetParentWindow()->ExecuteDrop(rEvt);
}

namespace
{
    bool IsAllExpanded(const weld::TreeView& rContentTree, const weld::TreeIter& rEntry)
    {
        if (!rContentTree.get_row_expanded(rEntry))
            return false;

        if (!rContentTree.iter_has_child(rEntry))
            return false;

        std::unique_ptr<weld::TreeIter> xChild(rContentTree.make_iterator(&rEntry));
        (void)rContentTree.iter_children(*xChild);

        do
        {
            if (rContentTree.iter_has_child(*xChild) || rContentTree.get_children_on_demand(*xChild))
            {
                if (!IsAllExpanded(rContentTree, *xChild))
                    return false;
            }
        }
        while (rContentTree.iter_next_sibling(*xChild));
        return true;
    }

    void ExpandOrCollapseAll(weld::TreeView& rContentTree, weld::TreeIter& rEntry)
    {
        bool bExpand = !IsAllExpanded(rContentTree, rEntry);
        bExpand ? rContentTree.expand_row(rEntry) : rContentTree.collapse_row(rEntry);
        int nRefDepth = rContentTree.get_iter_depth(rEntry);
        while (rContentTree.iter_next(rEntry) && rContentTree.get_iter_depth(rEntry) > nRefDepth)
        {
            if (rContentTree.iter_has_child(rEntry))
                bExpand ? rContentTree.expand_row(rEntry) : rContentTree.collapse_row(rEntry);
        }
    }
}

// Handler for Dragging and ContextMenu
static bool lcl_InsertExpandCollapseAllItem(const weld::TreeView& rContentTree, const weld::TreeIter& rEntry, weld::Menu& rPop)
{
    if (rContentTree.iter_has_child(rEntry) || rContentTree.get_children_on_demand(rEntry))
    {
        rPop.set_label(OUString::number(800), IsAllExpanded(rContentTree, rEntry) ? SwResId(STR_COLLAPSEALL) : SwResId(STR_EXPANDALL));
        return false;
    }
    return true;
}

static void lcl_SetOutlineContentEntriesSensitivities(SwContentTree* pThis, const weld::TreeView& rContentTree, const weld::TreeIter& rEntry, weld::Menu& rPop)
{
    rPop.set_sensitive(OUString::number(TOGGLE_OUTLINE_CONTENT_VISIBILITY), false);
    rPop.set_sensitive(OUString::number(HIDE_OUTLINE_CONTENT_VISIBILITY), false);
    rPop.set_sensitive(OUString::number(SHOW_OUTLINE_CONTENT_VISIBILITY), false);

    // todo: multi selection
    if (rContentTree.count_selected_rows() > 1)
        return;

    bool bIsRoot = lcl_IsContentType(rEntry, rContentTree);

    const SwNodes& rNodes = pThis->GetWrtShell()->GetNodes();
    const SwOutlineNodes& rOutlineNodes = rNodes.GetOutLineNds();
    size_t nOutlinePos = weld::GetAbsPos(rContentTree, rEntry);

    if (!bIsRoot)
        --nOutlinePos;

    if (nOutlinePos >= rOutlineNodes.size())
         return;

    int nFirstLevel = pThis->GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos);
    {
        // determine if any concerned outline node has content
        bool bHasContent(false);
        size_t nPos = nOutlinePos;
        SwNode* pSttNd = rOutlineNodes[nPos];
        SwNode* pEndNd = &rNodes.GetEndOfContent();
        if (rOutlineNodes.size() > nPos + 1)
            pEndNd = rOutlineNodes[nPos + 1];

        // selected
        SwNodeIndex aIdx(*pSttNd);
        if (rNodes.GoNext(&aIdx) != pEndNd)
            bHasContent = true;

        // descendants
        if (!bHasContent && (rContentTree.iter_has_child(rEntry) || rContentTree.get_children_on_demand(rEntry)))
        {
            while (++nPos < rOutlineNodes.size() &&
                  (bIsRoot || pThis->GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos) > nFirstLevel))
            {
                pSttNd = rOutlineNodes[nPos];
                pEndNd = &rNodes.GetEndOfContent();
                if (rOutlineNodes.size() > nPos + 1)
                    pEndNd = rOutlineNodes[nPos + 1];

                // test for content in outline node
                aIdx.Assign(*pSttNd);
                if (rNodes.GoNext(&aIdx) != pEndNd)
                {
                    bHasContent = true;
                    break;
                }
            }
        }

        if (!bHasContent)
            return; // no content in any of the concerned outline nodes
    }

    // determine for subs if all are folded or unfolded or if they are mixed
    if (rContentTree.iter_has_child(rEntry) || rContentTree.get_children_on_demand(rEntry))
    {
        // skip no content nodes
        // we know there is content from results above so this is presumably safe
        size_t nPos = nOutlinePos;
        while (true)
        {
            SwNode* pSttNd = rOutlineNodes[nPos];
            SwNode* pEndNd = rOutlineNodes.back();
            if (!bIsRoot && rOutlineNodes.size() > nPos + 1)
                pEndNd = rOutlineNodes[nPos + 1];

            SwNodeIndex aIdx(*pSttNd);
            if (rNodes.GoNext(&aIdx) != pEndNd)
                break;
            nPos++;
        }

        bool bHasFolded(!pThis->GetWrtShell()->IsOutlineContentVisible(nPos));
        bool bHasUnfolded(!bHasFolded);

        while ((++nPos < pThis->GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount()) &&
               (bIsRoot || pThis->GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos) > nFirstLevel))
        {

            SwNode* pSttNd = rOutlineNodes[nPos];
            SwNode* pEndNd = &rNodes.GetEndOfContent();
            if (rOutlineNodes.size() > nPos + 1)
                pEndNd = rOutlineNodes[nPos + 1];

            SwNodeIndex aIdx(*pSttNd);
            if (rNodes.GoNext(&aIdx) == pEndNd)
                continue; // skip if no content

            if (!pThis->GetWrtShell()->IsOutlineContentVisible(nPos))
                bHasFolded = true;
            else
                bHasUnfolded = true;

            if (bHasFolded && bHasUnfolded)
                break; // mixed so no need to continue
        }

        rPop.set_sensitive(OUString::number(HIDE_OUTLINE_CONTENT_VISIBILITY), bHasUnfolded);
        rPop.set_sensitive(OUString::number(SHOW_OUTLINE_CONTENT_VISIBILITY), bHasFolded);
    }

    rPop.set_sensitive(OUString::number(TOGGLE_OUTLINE_CONTENT_VISIBILITY), !bIsRoot);
}

IMPL_LINK(SwContentTree, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    grab_focus();

    // select clicked entry or limit selection to root entry if needed
    if (std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
            rCEvt.IsMouseEvent() &&  m_xTreeView->get_dest_row_at_pos(
                rCEvt.GetMousePosPixel(), xEntry.get(), false))
    {
        // if clicked entry is not currently selected then clear selections and select it
        if (!m_xTreeView->is_selected(*xEntry))
            m_xTreeView->set_cursor(*xEntry);
        // if root entry is selected then clear selections and select it
        else if (m_xTreeView->is_selected(0))
            m_xTreeView->set_cursor(0);
    }

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xTreeView.get(), "modules/swriter/ui/navigatorcontextmenu.ui"));
    std::unique_ptr<weld::Menu> xPop = xBuilder->weld_menu("navmenu");

    bool bOutline(false);
    std::unique_ptr<weld::Menu> xSubPop1 = xBuilder->weld_menu("outlinelevel");
    std::unique_ptr<weld::Menu> xSubPop2 = xBuilder->weld_menu("dragmodemenu");
    std::unique_ptr<weld::Menu> xSubPop3 = xBuilder->weld_menu("displaymenu");
    std::unique_ptr<weld::Menu> xSubPopOutlineTracking = xBuilder->weld_menu("outlinetracking");

    std::unique_ptr<weld::Menu> xSubPopOutlineContent = xBuilder->weld_menu("outlinecontent");

    xSubPopOutlineContent->append(OUString::number(TOGGLE_OUTLINE_CONTENT_VISIBILITY),
                                  SwResId(STR_OUTLINE_CONTENT_VISIBILITY_TOGGLE));
    xSubPopOutlineContent->append(OUString::number(HIDE_OUTLINE_CONTENT_VISIBILITY),
                                  SwResId(STR_OUTLINE_CONTENT_VISIBILITY_HIDE_ALL));
    xSubPopOutlineContent->append(OUString::number(SHOW_OUTLINE_CONTENT_VISIBILITY),
                                  SwResId(STR_OUTLINE_CONTENT_VISIBILITY_SHOW_ALL));

    xSubPopOutlineContent->set_item_help_id(OUString::number(TOGGLE_OUTLINE_CONTENT_VISIBILITY),
                                            HID_NAVIGATOR_TREELIST);
    xSubPopOutlineContent->set_item_help_id(OUString::number(HIDE_OUTLINE_CONTENT_VISIBILITY),
                                            HID_NAVIGATOR_TREELIST);
    xSubPopOutlineContent->set_item_help_id(OUString::number(SHOW_OUTLINE_CONTENT_VISIBILITY),
                                            HID_NAVIGATOR_TREELIST);

    // Add entries to the Outline Tracking submenu
    OUString sId;
    for(int i = 1; i <= 3; ++i)
    {
        sId = OUString::number(i + 10);
        xSubPopOutlineTracking->append_radio(sId, m_aContextStrings[IDX_STR_OUTLINE_TRACKING + i]);
        xSubPopOutlineTracking->set_item_help_id(sId, HID_NAV_OUTLINE_TRACKING);
    }
    xSubPopOutlineTracking->set_active(OUString::number(10 + m_nOutlineTracking), true);

    // Add entries to the Outline Level submenu
    for (int i = 1; i <= MAXLEVEL; ++i)
    {
        sId = OUString::number(i + 100);
        xSubPop1->append_radio(sId, OUString::number(i));
        xSubPop1->set_item_help_id(sId, HID_NAV_OUTLINE_LEVEL);
    }
    xSubPop1->set_active(OUString::number(100 + m_nOutlineLevel), true);

    // Add entries to the Drag Mode submenu
    for (int i=0; i < 3; ++i)
    {
        sId = OUString::number(i + 201);
        xSubPop2->append_radio(sId, m_aContextStrings[IDX_STR_HYPERLINK + i]);
        xSubPop2->set_item_help_id(sId, HID_NAV_DRAG_MODE);
    }
    xSubPop2->set_active(OUString::number(201 + static_cast<int>(GetParentWindow()->GetRegionDropMode())), true);

    // Insert the list of the open files in the Display submenu
    {
    sal_uInt16 nId = 301;
    SwView *pView = SwModule::GetFirstView();
    while (pView)
    {
        OUString sInsert = pView->GetDocShell()->GetTitle() + " (" +
                m_aContextStrings[pView == GetActiveView() ? IDX_STR_ACTIVE :
                                                             IDX_STR_INACTIVE] + ")";
        sId = OUString::number(nId);
        xSubPop3->append_radio(sId, sInsert);
        xSubPop3->set_item_help_id(sId, HID_NAV_DISPLAY);
        if (State::CONSTANT == m_eState && m_pActiveShell == &pView->GetWrtShell())
            xSubPop3->set_active(sId, true);
        pView = SwModule::GetNextView(pView);
        nId++;
    }
    // Active Window
    sId = OUString::number(nId++);
    xSubPop3->append_radio(sId, m_aContextStrings[IDX_STR_ACTIVE_VIEW]);
    xSubPop3->set_item_help_id(sId, HID_NAV_DISPLAY);
    // There can be only one hidden shell
    if (m_pHiddenShell)
    {
        OUString sHiddenEntry = m_pHiddenShell->GetView().GetDocShell()->GetTitle() +
            " (" +
            m_aContextStrings[IDX_STR_HIDDEN] +
            ")";
        sId = OUString::number(nId);
        xSubPop3->append_radio(sId, sHiddenEntry);
        xSubPop3->set_item_help_id(sId, HID_NAV_DISPLAY);
    }
    if (State::ACTIVE == m_eState)
        xSubPop3->set_active(OUString::number(--nId), true);
    else if (State::HIDDEN == m_eState)
        xSubPop3->set_active(OUString::number(nId), true);
    }

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xEntry.get()))
        xEntry.reset();

    bool bRemoveGotoEntry = false;
    if (State::HIDDEN == m_eState || !xEntry || !lcl_IsContent(*xEntry, *m_xTreeView) ||
            weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry))->IsInvisible())
        bRemoveGotoEntry = true;

    bool bRemovePostItEntries = true;
    bool bRemoveIndexEntries = true;
    bool bRemoveCopyEntry = true;
    bool bRemoveEditEntry = true;
    bool bRemoveUnprotectEntry = true;
    bool bRemoveDeleteChapterEntry = true,
         bRemoveDeleteTableEntry = true,
         bRemoveDeleteFrameEntry = true,
         bRemoveDeleteImageEntry = true,
         bRemoveDeleteOLEObjectEntry = true,
         bRemoveDeleteBookmarkEntry = true,
         bRemoveDeleteRegionEntry = true,
         bRemoveDeleteHyperlinkEntry = true,
         bRemoveDeleteReferenceEntry = true,
         bRemoveDeleteIndexEntry= true,
         bRemoveDeleteCommentEntry = true,
         bRemoveDeleteDrawingObjectEntry = true,
         bRemoveDeleteFieldEntry = true;
    bool bRemoveRenameEntry = true;
    bool bRemoveSelectEntry = true;
    bool bRemoveToggleExpandEntry = true;
    bool bRemoveChapterEntries = true;
    bool bRemoveSendOutlineEntry = true;

    bool bRemoveTableTracking = true;
    bool bRemoveSectionTracking = true;
    bool bRemoveFrameTracking = true;
    bool bRemoveImageTracking = true;
    bool bRemoveOLEobjectTracking = true;
    bool bRemoveBookmarkTracking = true;
    bool bRemoveHyperlinkTracking = true;
    bool bRemoveReferenceTracking = true;
    bool bRemoveIndexTracking = true;
    bool bRemoveCommentTracking = true;
    bool bRemoveDrawingObjectTracking = true;
    bool bRemoveFieldTracking = true;
    bool bRemoveFootnoteTracking = true;
    bool bRemoveEndnoteTracking = true;

    bool bRemoveSortEntry = true;

    bool bRemoveProtectSection = true;
    bool bRemoveHideSection = true;

    if (xEntry)
    {
        const SwContentType* pType;
        if (lcl_IsContentType(*xEntry, *m_xTreeView))
            pType = weld::fromId<SwContentType*>(m_xTreeView->get_id(*xEntry));
        else
            pType = weld::fromId<SwContent*>(
                        m_xTreeView->get_id(*xEntry))->GetParent();
        const ContentTypeId nContentType = pType->GetType();

        if (nContentType != ContentTypeId::FOOTNOTE && nContentType != ContentTypeId::ENDNOTE
            && nContentType != ContentTypeId::POSTIT && nContentType != ContentTypeId::UNKNOWN)
        {
            bRemoveSortEntry = false;
            const sal_Int32 nMask = 1 << static_cast<int>(nContentType);
            sal_uInt64 nSortAlphabeticallyBlock = m_pConfig->GetSortAlphabeticallyBlock();
            xPop->set_active("sort", nSortAlphabeticallyBlock & nMask);
        }

        OUString aIdent;
        switch (nContentType)
        {
            case ContentTypeId::TABLE:
                aIdent = "tabletracking";
                bRemoveTableTracking = false;
            break;
            case ContentTypeId::REGION:
                aIdent = "sectiontracking";
                bRemoveSectionTracking = false;
            break;
            case ContentTypeId::FRAME:
                aIdent = "frametracking";
                bRemoveFrameTracking = false;
            break;
            case ContentTypeId::GRAPHIC:
                aIdent = "imagetracking";
                bRemoveImageTracking = false;
            break;
            case ContentTypeId::OLE:
                aIdent = "oleobjecttracking";
                bRemoveOLEobjectTracking = false;
            break;
            case ContentTypeId::BOOKMARK:
                aIdent = "bookmarktracking";
                bRemoveBookmarkTracking = false;
            break;
            case ContentTypeId::URLFIELD:
                aIdent = "hyperlinktracking";
                bRemoveHyperlinkTracking = false;
            break;
            case ContentTypeId::REFERENCE:
                aIdent = "referencetracking";
                bRemoveReferenceTracking = false;
            break;
            case ContentTypeId::INDEX:
                aIdent = "indextracking";
                bRemoveIndexTracking = false;
            break;
            case ContentTypeId::POSTIT:
                aIdent = "commenttracking";
                bRemoveCommentTracking = false;
            break;
            case ContentTypeId::DRAWOBJECT:
                aIdent = "drawingobjecttracking";
                bRemoveDrawingObjectTracking = false;
            break;
            case ContentTypeId::TEXTFIELD:
                aIdent = "fieldtracking";
                bRemoveFieldTracking = false;
            break;
            case ContentTypeId::FOOTNOTE:
                aIdent = "footnotetracking";
                bRemoveFootnoteTracking = false;
            break;
            case ContentTypeId::ENDNOTE:
                aIdent = "endnotetracking";
                bRemoveEndnoteTracking = false;
            break;
            default: break;
        }
        if (!aIdent.isEmpty())
            xPop->set_active(aIdent, mTrackContentType[nContentType]);

        // Edit only if the shown content is coming from the current view.
        if (State::HIDDEN != m_eState &&
                (State::ACTIVE == m_eState || (GetActiveView() && m_pActiveShell == GetActiveView()->GetWrtShellPtr()))
                && lcl_IsContent(*xEntry, *m_xTreeView))
        {
            const bool bReadonly = m_pActiveShell->GetView().GetDocShell()->IsReadOnly();
            const bool bVisible = !weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry))->IsInvisible();
            const bool bProtected = weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry))->IsProtect();
            const bool bProtectBM = (ContentTypeId::BOOKMARK == nContentType)
                    && m_pActiveShell->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_BOOKMARKS);
            const bool bEditable = pType->IsEditable() &&
                    ((bVisible && !bProtected) || ContentTypeId::REGION == nContentType);
            const bool bDeletable = pType->IsDeletable()
                    && ((bVisible && !bProtected && !bProtectBM) || ContentTypeId::REGION == nContentType);
            const bool bRenamable = bEditable && !bReadonly &&
                    (ContentTypeId::TABLE == nContentType ||
                     ContentTypeId::FRAME == nContentType ||
                     ContentTypeId::GRAPHIC == nContentType ||
                     ContentTypeId::OLE == nContentType ||
                     (ContentTypeId::BOOKMARK == nContentType && !bProtectBM) ||
                     ContentTypeId::REGION == nContentType ||
                     ContentTypeId::INDEX == nContentType ||
                     ContentTypeId::DRAWOBJECT == nContentType);
            // Choose which Delete entry to show.
            if (bDeletable)
            {
                switch (nContentType)
                {
                    case ContentTypeId::OUTLINE:
                        bRemoveDeleteChapterEntry = false;
                    break;
                    case ContentTypeId::TABLE:
                        bRemoveDeleteTableEntry = false;
                    break;
                    case ContentTypeId::FRAME:
                        bRemoveDeleteFrameEntry = false;
                    break;
                    case ContentTypeId::GRAPHIC:
                        bRemoveDeleteImageEntry = false;
                    break;
                    case ContentTypeId::OLE:
                        bRemoveDeleteOLEObjectEntry = false;
                    break;
                    case ContentTypeId::BOOKMARK:
                        bRemoveDeleteBookmarkEntry = false;
                    break;
                    case ContentTypeId::REGION:
                        bRemoveDeleteRegionEntry = false;
                    break;
                    case ContentTypeId::URLFIELD:
                        bRemoveDeleteHyperlinkEntry = false;
                    break;
                    case ContentTypeId::REFERENCE:
                        bRemoveDeleteReferenceEntry = false;
                    break;
                    case ContentTypeId::INDEX:
                        bRemoveDeleteIndexEntry = false;
                    break;
                    case ContentTypeId::POSTIT:
                        bRemoveDeleteCommentEntry = false;
                    break;
                    case ContentTypeId::DRAWOBJECT:
                        bRemoveDeleteDrawingObjectEntry = false;
                    break;
                    case ContentTypeId::TEXTFIELD:
                        bRemoveDeleteFieldEntry = false;
                    break;
                    default: break;
                }
            }
            if (ContentTypeId::FOOTNOTE == nContentType || ContentTypeId::ENDNOTE == nContentType)
            {
                void* pUserData = weld::fromId<void*>(m_xTreeView->get_id(*xEntry));
                const SwTextFootnote* pFootnote =
                        static_cast<const SwTextFootnoteContent*>(pUserData)->GetTextFootnote();
                if (!pFootnote)
                    bRemoveGotoEntry = true;
            }
            else if(ContentTypeId::OUTLINE == nContentType)
            {
                bOutline = true;
                lcl_SetOutlineContentEntriesSensitivities(this, *m_xTreeView, *xEntry, *xSubPopOutlineContent);
                bRemoveToggleExpandEntry = lcl_InsertExpandCollapseAllItem(*m_xTreeView, *xEntry, *xPop);
                if (!bReadonly)
                {
                    bRemoveSelectEntry = false;
                    bRemoveChapterEntries = false;
                }
                bRemoveCopyEntry = false;
            }
            else if (!bReadonly && bEditable)
            {
                if(ContentTypeId::INDEX == nContentType)
                {
                    bRemoveIndexEntries = false;

                    const SwTOXBase* pBase = weld::fromId<SwTOXBaseContent*>(m_xTreeView->get_id(*xEntry))->GetTOXBase();
                    if (!pBase->IsTOXBaseInReadonly())
                        bRemoveEditEntry = false;

                    xPop->set_active(OUString::number(405), SwEditShell::IsTOXBaseReadonly(*pBase));
                }
                else if(ContentTypeId::TABLE == nContentType)
                {
                    bRemoveSelectEntry = false;
                    bRemoveEditEntry = false;
                    bRemoveUnprotectEntry = false;
                    bool bFull = false;
                    OUString sTableName = weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry))->GetName();
                    bool bProt = m_pActiveShell->HasTableAnyProtection( &sTableName, &bFull );
                    xPop->set_sensitive(OUString::number(403), !bFull);
                    xPop->set_sensitive(OUString::number(404), bProt);
                }
                else if(ContentTypeId::REGION == nContentType)
                {
                    bRemoveEditEntry = false;
                    bRemoveProtectSection = false;
                    bRemoveHideSection = false;
                    SwContent* pCnt = weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry));
                    assert(dynamic_cast<SwRegionContent*>(static_cast<SwTypeNumber*>(pCnt)));
                    const SwSectionFormat* pSectionFormat
                            = static_cast<SwRegionContent*>(pCnt)->GetSectionFormat();
                    bool bHidden = pSectionFormat->GetSection()->IsHidden();
                    bRemoveSelectEntry = bHidden || !bVisible;
                    xPop->set_active("protectsection", bProtected);
                    xPop->set_active("hidesection", bHidden);
                }
                else if (bEditable)
                    bRemoveEditEntry = false;
                //Rename object
                if (bRenamable)
                    bRemoveRenameEntry = false;
            }
        }
        else
        {
            if (lcl_IsContentType(*xEntry, *m_xTreeView))
                pType = weld::fromId<SwContentType*>(m_xTreeView->get_id(*xEntry));
            else
                pType = weld::fromId<SwContent*>(
                            m_xTreeView->get_id(*xEntry))->GetParent();
            if (pType)
            {
                if (ContentTypeId::OUTLINE == nContentType)
                {
                    bOutline = true;
                    if (State::HIDDEN != m_eState)
                    {
                        lcl_SetOutlineContentEntriesSensitivities(this, *m_xTreeView, *xEntry,
                                                                  *xSubPopOutlineContent);
                        bRemoveSendOutlineEntry = false;
                    }
                    bRemoveToggleExpandEntry = lcl_InsertExpandCollapseAllItem(*m_xTreeView, *xEntry,
                                                                               *xPop);
                }
                else if (State::HIDDEN != m_eState &&
                         nContentType == ContentTypeId::POSTIT &&
                         !m_pActiveShell->GetView().GetDocShell()->IsReadOnly() &&
                         pType->GetMemberCount() > 0)
                    bRemovePostItEntries = false;
            }
        }
    }

    if (bRemoveToggleExpandEntry)
        xPop->remove(OUString::number(800));

    if (bRemoveGotoEntry)
        xPop->remove(OUString::number(900));

    if (bRemoveSelectEntry)
        xPop->remove(OUString::number(805));

    if (bRemoveChapterEntries)
    {
        xPop->remove(OUString::number(801));
        xPop->remove(OUString::number(802));
        xPop->remove(OUString::number(803));
        xPop->remove(OUString::number(804));
    }

    if (bRemoveSendOutlineEntry)
        xPop->remove(OUString::number(700));

    if (bRemovePostItEntries)
    {
        xPop->remove(OUString::number(600));
        xPop->remove(OUString::number(601));
        xPop->remove(OUString::number(602));
    }

    if (bRemoveDeleteChapterEntry)
        xPop->remove("deletechapter");
    if (bRemoveDeleteTableEntry)
        xPop->remove("deletetable");
    if (bRemoveDeleteFrameEntry)
        xPop->remove("deleteframe");
    if (bRemoveDeleteImageEntry)
        xPop->remove("deleteimage");
    if (bRemoveDeleteOLEObjectEntry)
        xPop->remove("deleteoleobject");
    if (bRemoveDeleteBookmarkEntry)
        xPop->remove("deletebookmark");
    if (bRemoveDeleteRegionEntry)
        xPop->remove("deleteregion");
    if (bRemoveDeleteHyperlinkEntry)
        xPop->remove("deletehyperlink");
    if (bRemoveDeleteReferenceEntry)
        xPop->remove("deletereference");
    if (bRemoveDeleteIndexEntry)
        xPop->remove("deleteindex");
    if (bRemoveDeleteCommentEntry)
        xPop->remove("deletecomment");
    if (bRemoveDeleteDrawingObjectEntry)
        xPop->remove("deletedrawingobject");
    if (bRemoveDeleteFieldEntry)
        xPop->remove("deletefield");

    bool bRemoveDeleteEntry =
            bRemoveDeleteChapterEntry &&
            bRemoveDeleteTableEntry &&
            bRemoveDeleteFrameEntry &&
            bRemoveDeleteImageEntry &&
            bRemoveDeleteOLEObjectEntry &&
            bRemoveDeleteBookmarkEntry &&
            bRemoveDeleteRegionEntry &&
            bRemoveDeleteHyperlinkEntry &&
            bRemoveDeleteReferenceEntry &&
            bRemoveDeleteIndexEntry &&
            bRemoveDeleteCommentEntry &&
            bRemoveDeleteDrawingObjectEntry &&
            bRemoveDeleteFieldEntry;

    if (bRemoveRenameEntry)
        xPop->remove(OUString::number(502));

    if (bRemoveIndexEntries)
    {
        xPop->remove(OUString::number(401));
        xPop->remove(OUString::number(402));
        xPop->remove(OUString::number(405));
    }

    if (bRemoveUnprotectEntry)
        xPop->remove(OUString::number(404));

    if (bRemoveEditEntry)
        xPop->remove(OUString::number(403));

    if (bRemoveToggleExpandEntry &&
            bRemoveSendOutlineEntry)
        xPop->remove("separator1");

    if (bRemoveCopyEntry)
        xPop->remove("copy");

    if (bRemoveGotoEntry &&
            bRemoveCopyEntry &&
            bRemoveSelectEntry &&
            bRemoveDeleteEntry &&
            bRemoveChapterEntries &&
            bRemovePostItEntries &&
            bRemoveRenameEntry &&
            bRemoveIndexEntries &&
            bRemoveUnprotectEntry &&
            bRemoveEditEntry)
        xPop->remove("separator2");

    if (!bOutline)
    {
        xSubPop1.reset();
        xPop->remove(OUString::number(1)); // outline level menu
    }
    if (!bOutline || State::HIDDEN == m_eState)
    {
        xSubPopOutlineTracking.reset();
        xPop->remove(OUString::number(4)); // outline tracking menu
    }
    if (!bOutline || State::HIDDEN == m_eState ||
            !m_pActiveShell->GetViewOptions()->IsShowOutlineContentVisibilityButton() ||
            m_pActiveShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount() == 0)
    {
        xSubPopOutlineContent.reset();
        xPop->remove(OUString::number(5)); // outline folding menu
        xPop->remove("separator3");
    }

    if (bRemoveTableTracking)
        xPop->remove("tabletracking");
    if (bRemoveSectionTracking)
        xPop->remove("sectiontracking");
    if (bRemoveFrameTracking)
        xPop->remove("frametracking");
    if (bRemoveImageTracking)
        xPop->remove("imagetracking");
    if (bRemoveOLEobjectTracking)
        xPop->remove("oleobjecttracking");
    if (bRemoveBookmarkTracking)
        xPop->remove("bookmarktracking");
    if (bRemoveHyperlinkTracking)
        xPop->remove("hyperlinktracking");
    if (bRemoveReferenceTracking)
        xPop->remove("referencetracking");
    if (bRemoveIndexTracking)
        xPop->remove("indextracking");
    if (bRemoveCommentTracking)
        xPop->remove("commenttracking");
    if (bRemoveDrawingObjectTracking)
        xPop->remove("drawingobjecttracking");
    if (bRemoveFieldTracking)
        xPop->remove("fieldtracking");
    if (bRemoveFootnoteTracking)
        xPop->remove("footnotetracking");
    if (bRemoveEndnoteTracking)
        xPop->remove("endnotetracking");
    if (bRemoveSortEntry)
        xPop->remove("sort");
    if (bRemoveProtectSection)
        xPop->remove("protectsection");
    if (bRemoveHideSection)
        xPop->remove("hidesection");

    bool bSetSensitiveCollapseAllCategories = false;
    if (!m_bIsRoot && xEntry)
    {
        bool bEntry = m_xTreeView->get_iter_first(*xEntry);
        while (bEntry)
        {
            if (m_xTreeView->get_row_expanded(*xEntry))
            {
                bSetSensitiveCollapseAllCategories = true;
                break;
            }
            bEntry = m_xTreeView->iter_next_sibling(*xEntry);
        }
    }
    xPop->set_sensitive("collapseallcategories", bSetSensitiveCollapseAllCategories);

    OUString sCommand = xPop->popup_at_rect(m_xTreeView.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));
    if (!sCommand.isEmpty())
        ExecuteContextMenuAction(sCommand);

    return true;
}

void SwContentTree::InsertContent(const weld::TreeIter& rParent)
{
    assert(dynamic_cast<SwContentType*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(rParent))));
    SwContentType* pCntType = weld::fromId<SwContentType*>(m_xTreeView->get_id(rParent));
    bool bGraphic = pCntType->GetType() == ContentTypeId::GRAPHIC;
    std::unique_ptr<weld::TreeIter> xChild = m_xTreeView->make_iterator();
    const size_t nCount = pCntType->GetMemberCount();
    for(size_t i = 0; i < nCount; ++i)
    {
        const SwContent* pCnt = pCntType->GetMember(i);
        OUString sEntry = pCnt->GetName();
        if (sEntry.isEmpty())
            sEntry = m_sSpace;
        OUString sId(weld::toId(pCnt));
        insert(&rParent, sEntry, sId, false, xChild.get());
        m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());
        if (bGraphic && !static_cast<const SwGraphicContent*>(pCnt)->GetLink().isEmpty())
            m_xTreeView->set_image(*xChild, RID_BMP_NAVI_GRAPHIC_LINK);
    }
}

void SwContentTree::insert(const weld::TreeIter* pParent, const OUString& rStr, const OUString& rId,
                           bool bChildrenOnDemand, weld::TreeIter* pRet)
{
    m_xTreeView->insert(pParent, -1, &rStr, &rId, nullptr, nullptr, bChildrenOnDemand, pRet);
    ++m_nEntryCount;
}

void SwContentTree::remove(const weld::TreeIter& rIter)
{
    if (m_xTreeView->iter_has_child(rIter))
    {
        std::unique_ptr<weld::TreeIter> xChild = m_xTreeView->make_iterator(&rIter);
        (void)m_xTreeView->iter_children(*xChild);
        remove(*xChild);
    }
    m_xTreeView->remove(rIter);
    --m_nEntryCount;
}

// Content will be integrated into the Box only on demand.
bool SwContentTree::RequestingChildren(const weld::TreeIter& rParent)
{
    // Does the parent already have children or is it not a 'children on demand' node?
    if (m_xTreeView->iter_has_child(rParent) || !m_xTreeView->get_children_on_demand(rParent))
        return false;

    // Is this a content type?
    if (lcl_IsContentType(rParent, *m_xTreeView))
    {
        std::unique_ptr<weld::TreeIter> xChild = m_xTreeView->make_iterator();

        assert(dynamic_cast<SwContentType*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(rParent))));
        SwContentType* pCntType = weld::fromId<SwContentType*>(m_xTreeView->get_id(rParent));

        const size_t nCount = pCntType->GetMemberCount();
        // Add for outline plus/minus
        if (pCntType->GetType() == ContentTypeId::OUTLINE)
        {
            std::vector<std::unique_ptr<weld::TreeIter>> aParentCandidates;
            for(size_t i = 0; i < nCount; ++i)
            {
                const SwContent* pCnt = pCntType->GetMember(i);
                if(pCnt)
                {
                    const auto nLevel = static_cast<const SwOutlineContent*>(pCnt)->GetOutlineLevel();
                    OUString sEntry = pCnt->GetName();
                    if(sEntry.isEmpty())
                        sEntry = m_sSpace;
                    OUString sId(weld::toId(pCnt));

                    auto lambda = [nLevel, this](const std::unique_ptr<weld::TreeIter>& entry)
                    {
                        return lcl_IsLowerOutlineContent(*entry, *m_xTreeView, nLevel);
                    };

                    // if there is a preceding outline node candidate with a lower outline level use
                    // that as a parent, otherwise use the root node
                    auto aFind = std::find_if(aParentCandidates.rbegin(), aParentCandidates.rend(), lambda);
                    if (aFind != aParentCandidates.rend())
                        insert(aFind->get(), sEntry, sId, false, xChild.get());
                    else
                        insert(&rParent, sEntry, sId, false, xChild.get());
                    m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());
                    m_xTreeView->set_extra_row_indent(*xChild, nLevel + 1 - m_xTreeView->get_iter_depth(*xChild));

                    // remove any parent candidates equal to or higher than this node
                    std::erase_if(aParentCandidates, std::not_fn(lambda));

                    // add this node as a parent candidate for any following nodes at a higher outline level
                    aParentCandidates.emplace_back(m_xTreeView->make_iterator(xChild.get()));
                }
            }
        }
        else if (pCntType->GetType() == ContentTypeId::REGION)
        {
            if (pCntType->IsAlphabeticSort())
            {
                for(size_t i = 0; i < nCount; ++i)
                {
                    const SwRegionContent* pCnt =
                            static_cast<const SwRegionContent*>(pCntType->GetMember(i));

                    OUString sEntry = pCnt->GetName();
                    OUString sId(weld::toId(pCnt));

                    const auto nLevel = pCnt->GetRegionLevel();
                    insert(&rParent, sEntry, sId, false, xChild.get());

                    m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());
                    m_xTreeView->set_extra_row_indent(*xChild, nLevel);

                    bool bHidden = pCnt->GetSectionFormat()->GetSection()->IsHidden();
                    if (pCnt->IsProtect())
                        m_xTreeView->set_image(*xChild, bHidden ? RID_BMP_PROT_HIDE : RID_BMP_PROT_NO_HIDE);
                    else
                        m_xTreeView->set_image(*xChild, bHidden ? RID_BMP_HIDE : RID_BMP_NO_HIDE);
                }
            }
            else
            {
                std::vector<std::unique_ptr<weld::TreeIter>> aParentCandidates;
                for(size_t i = 0; i < nCount; ++i)
                {
                    const SwRegionContent* pCnt =
                            static_cast<const SwRegionContent*>(pCntType->GetMember(i));

                    OUString sEntry = pCnt->GetName();
                    OUString sId(weld::toId(pCnt));

                    const auto nLevel = pCnt->GetRegionLevel();
                    auto lambda = [nLevel, this](const std::unique_ptr<weld::TreeIter>& xEntry)
                    {
                        return lcl_IsLowerRegionContent(*xEntry, *m_xTreeView, nLevel);
                    };

                    // if there is a preceding region node candidate with a lower region level use
                    // that as a parent, otherwise use the root node
                    auto aFind = std::find_if(aParentCandidates.rbegin(), aParentCandidates.rend(), lambda);
                    if (aFind != aParentCandidates.rend())
                        insert(aFind->get(), sEntry, sId, false, xChild.get());
                    else
                        insert(&rParent, sEntry, sId, false, xChild.get());
                    m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());

                    bool bHidden = pCnt->GetSectionFormat()->GetSection()->IsHidden();
                    if (pCnt->IsProtect())
                        m_xTreeView->set_image(*xChild, bHidden ? RID_BMP_PROT_HIDE : RID_BMP_PROT_NO_HIDE);
                    else
                        m_xTreeView->set_image(*xChild, bHidden ? RID_BMP_HIDE : RID_BMP_NO_HIDE);

                    // remove any parent candidates equal to or higher than this node
                    std::erase_if(aParentCandidates, std::not_fn(lambda));

                    // add this node as a parent candidate for any following nodes at a higher region level
                    aParentCandidates.emplace_back(m_xTreeView->make_iterator(xChild.get()));
                }
            }
        }
        else if (pCntType->GetType() == ContentTypeId::POSTIT)
        {
            std::vector<std::unique_ptr<weld::TreeIter>> aParentCandidates;
            for(size_t i = 0; i < nCount; ++i)
            {
                const SwPostItContent* pCnt =
                        static_cast<const SwPostItContent*>(pCntType->GetMember(i));

                OUString sEntry = pCnt->GetName();
                OUString sId(weld::toId(pCnt));

                const SwPostItField* pPostItField =
                        static_cast<const SwPostItField*>(pCnt->GetPostIt()->GetField());
                auto lambda = [&pPostItField, this](const std::unique_ptr<weld::TreeIter>& xEntry)
                {
                    SwPostItContent* pParentCandidateCnt =
                            weld::fromId<SwPostItContent*>(m_xTreeView->get_id(*xEntry));
                    return pPostItField->GetParentPostItId() ==
                            static_cast<const SwPostItField*>(pParentCandidateCnt->GetPostIt()
                                                              ->GetField())->GetPostItId();
                };

                // if a parent candidate is not found use the passed root node
                auto aFind = std::find_if(aParentCandidates.rbegin(), aParentCandidates.rend(), lambda);
                if (aFind != aParentCandidates.rend())
                    insert(aFind->get(), sEntry, sId, false, xChild.get());
                else
                    insert(&rParent, sEntry, sId, false, xChild.get());

                m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());

                // clear parent candidates when encountering a postit that doesn't have a parent
                // following postits can't have a parent that is in these candidates
                if (pPostItField->GetParentPostItId() == 0)
                    aParentCandidates.clear();

                aParentCandidates.emplace_back(m_xTreeView->make_iterator(xChild.get()));
            }
        }
        else
            InsertContent(rParent);

        return nCount != 0;
    }

    return false;
}

void SwContentTree::ExpandAllHeadings()
{
    if (HasHeadings())
    {
        std::unique_ptr<weld::TreeIter> xEntry = GetEntryAtAbsPos(0);
        if (xEntry)
        {
            if (!IsAllExpanded(*m_xTreeView, *xEntry))
                ExpandOrCollapseAll(*m_xTreeView, *xEntry);
        }
    }
}

SdrObject* SwContentTree::GetDrawingObjectsByContent(const SwContent *pCnt)
{
    SdrObject *pRetObj = nullptr;
    switch(pCnt->GetParent()->GetType())
    {
        case ContentTypeId::DRAWOBJECT:
        {
            SdrView* pDrawView = m_pActiveShell->GetDrawView();
            if (pDrawView)
            {
                SwDrawModel* pDrawModel = m_pActiveShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
                SdrPage* pPage = pDrawModel->GetPage(0);

                for (const rtl::Reference<SdrObject>& pTemp : *pPage)
                {
                    if( pTemp->GetName() == pCnt->GetName())
                    {
                        pRetObj = pTemp.get();
                        break;
                    }
                }
            }
            break;
        }
        default:
            pRetObj = nullptr;
    }
    return pRetObj;
}

void SwContentTree::Expand(const weld::TreeIter& rParent,
                           std::vector<std::unique_ptr<weld::TreeIter>>* pNodesToExpand)
{
    if (!m_xTreeView->iter_has_child(rParent) && !m_xTreeView->get_children_on_demand(rParent))
        return;

    // pNodesToExpand is used by the Display function to restore the trees expand structure for
    // hierarchical content types, e.g., OUTLINE and REGION.
    if (pNodesToExpand)
        pNodesToExpand->emplace_back(m_xTreeView->make_iterator(&rParent));

    // rParentId is a string representation of a pointer to SwContentType or SwContent
    const OUString& rParentId = m_xTreeView->get_id(rParent);
    // bParentIsContentType tells if the passed rParent tree entry is a content type or content
    const bool bParentIsContentType = lcl_IsContentType(rParent, *m_xTreeView);
    // eParentContentTypeId is the content type of the passed rParent tree entry
    const ContentTypeId eParentContentTypeId =
            bParentIsContentType ? weld::fromId<SwContentType*>(rParentId)->GetType() :
                                   weld::fromId<SwContent*>(rParentId)->GetParent()->GetType();

    if (m_nRootType == ContentTypeId::UNKNOWN && bParentIsContentType)
    {
        // m_nActiveBlock and m_nHiddenBlock are used to persist the content type expand state for
        // the all content view mode
        const int nShift = static_cast<int>(eParentContentTypeId);
        SAL_WARN_IF(nShift < 0, "sw.ui", "ContentTypeId::UNKNOWN negative shift");
        if (nShift >= 0)
        {
            const sal_Int32 nOr = 1 << nShift; //linear -> Bitposition
            if (State::HIDDEN != m_eState)
            {
                m_nActiveBlock |= nOr;
                m_pConfig->SetActiveBlock(m_nActiveBlock);
            }
            else
                m_nHiddenBlock |= nOr;
        }
    }

    if (m_nRootType == ContentTypeId::OUTLINE || (m_nRootType == ContentTypeId::UNKNOWN &&
                                                  eParentContentTypeId == ContentTypeId::OUTLINE))
    {
        if (bParentIsContentType)
        {
            std::map< void*, bool > aCurrOutLineNodeMap;

            SwWrtShell* pShell = GetWrtShell();
            bool bParentHasChild = RequestingChildren(rParent);
            if (bParentHasChild)
            {
                std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rParent));
                bool bChild = m_xTreeView->iter_next(*xChild);
                while (bChild && lcl_IsContent(*xChild, *m_xTreeView))
                {
                    if (m_xTreeView->iter_has_child(*xChild))
                    {
                        assert(dynamic_cast<SwOutlineContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xChild))));
                        auto const nPos = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(*xChild))->GetOutlinePos();
                        void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
                        aCurrOutLineNodeMap.emplace( key, false );
                        std::map<void*, bool>::iterator iter = mOutLineNodeMap.find( key );
                        if( iter != mOutLineNodeMap.end() && mOutLineNodeMap[key])
                        {
                            aCurrOutLineNodeMap[key] = true;
                            RequestingChildren(*xChild);
                            if (pNodesToExpand)
                                pNodesToExpand->emplace_back(m_xTreeView->make_iterator(xChild.get()));
                            m_xTreeView->set_children_on_demand(*xChild, false);
                        }
                    }
                    bChild = m_xTreeView->iter_next(*xChild);
                }
            }
            mOutLineNodeMap = aCurrOutLineNodeMap;
            return;
        }
        else // content entry
        {
            SwWrtShell* pShell = GetWrtShell();
            assert(dynamic_cast<SwOutlineContent*>(weld::fromId<SwTypeNumber*>(rParentId)));
            auto const nPos = weld::fromId<SwOutlineContent*>(rParentId)->GetOutlinePos();
            void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
            mOutLineNodeMap[key] = true;
        }
    }
    else if (m_nRootType == ContentTypeId::REGION || (m_nRootType == ContentTypeId::UNKNOWN &&
                                                      eParentContentTypeId == ContentTypeId::REGION))
    {
        if (bParentIsContentType)
        {
            std::map<const void*, bool> aCurrentRegionNodeExpandMap;
            if (RequestingChildren(rParent))
            {
                std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rParent));
                while (m_xTreeView->iter_next(*xChild) && lcl_IsContent(*xChild, *m_xTreeView))
                {
                    if (m_xTreeView->iter_has_child(*xChild))
                    {
                        assert(dynamic_cast<SwRegionContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xChild))));
                        const void* key =
                                static_cast<const void*>(weld::fromId<SwRegionContent*>(
                                                m_xTreeView->get_id(*xChild))->GetSectionFormat());
                        bool bExpandNode =
                                m_aRegionNodeExpandMap.contains(key) && m_aRegionNodeExpandMap[key];
                        aCurrentRegionNodeExpandMap.emplace(key, bExpandNode);
                        if (bExpandNode)
                        {
                            if (pNodesToExpand)
                                pNodesToExpand->emplace_back(m_xTreeView->make_iterator(xChild.get()));
                            RequestingChildren(*xChild);
                            m_xTreeView->set_children_on_demand(*xChild, false);
                        }
                    }
                }
            }
            m_aRegionNodeExpandMap = aCurrentRegionNodeExpandMap;
            return;
        }
        else // content entry
        {
            assert(dynamic_cast<SwRegionContent*>(weld::fromId<SwTypeNumber*>(rParentId)));
            const void* key = static_cast<const void*>(
                        weld::fromId<SwRegionContent*>(rParentId)->GetSectionFormat());
            m_aRegionNodeExpandMap[key] = true;
        }
    }
    else if (m_nRootType == ContentTypeId::POSTIT || (m_nRootType == ContentTypeId::UNKNOWN &&
                                                      eParentContentTypeId == ContentTypeId::POSTIT))
    {
        if (bParentIsContentType)
        {
            std::map<const void*, bool> aCurrentPostItNodeExpandMap;
            if (RequestingChildren(rParent))
            {
                std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rParent));
                while (m_xTreeView->iter_next(*xChild) && lcl_IsContent(*xChild, *m_xTreeView))
                {
                    if (m_xTreeView->iter_has_child(*xChild))
                    {
                        assert(dynamic_cast<SwPostItContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xChild))));
                        const void* key =
                                static_cast<const void*>(weld::fromId<SwPostItContent*>(
                                                             m_xTreeView->get_id(*xChild))->GetPostIt());
                        bool bExpandNode =
                                m_aPostItNodeExpandMap.contains(key) && m_aPostItNodeExpandMap[key];
                        aCurrentPostItNodeExpandMap.emplace(key, bExpandNode);
                        if (bExpandNode)
                        {
                            if (pNodesToExpand)
                                pNodesToExpand->emplace_back(m_xTreeView->make_iterator(xChild.get()));
                            RequestingChildren(*xChild);
                            m_xTreeView->set_children_on_demand(*xChild, false);
                        }
                    }
                }
            }
            m_aPostItNodeExpandMap = aCurrentPostItNodeExpandMap;
            return;
        }
        else // content entry
        {
            assert(dynamic_cast<SwPostItContent*>(weld::fromId<SwTypeNumber*>(rParentId)));
            const void* key = static_cast<const void*>(
                        weld::fromId<SwPostItContent*>(rParentId)->GetPostIt());
            m_aPostItNodeExpandMap[key] = true;
        }
    }

    RequestingChildren(rParent);
}

IMPL_LINK(SwContentTree, ExpandHdl, const weld::TreeIter&, rParent, bool)
{
    Expand(rParent, nullptr);
    return true;
}

IMPL_LINK(SwContentTree, CollapseHdl, const weld::TreeIter&, rParent, bool)
{
    if (!m_xTreeView->iter_has_child(rParent) || m_xTreeView->get_children_on_demand(rParent))
        return true;

    if (lcl_IsContentType(rParent, *m_xTreeView))
    {
        if (m_bIsRoot)
        {
            // collapse to children of root node
            std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(&rParent));
            if (m_xTreeView->iter_children(*xEntry))
            {
                do
                {
                    m_xTreeView->collapse_row(*xEntry);
                }
                while (m_xTreeView->iter_next(*xEntry));
            }
            return false; // return false to notify caller not to do collapse
        }
        ContentTypeId eContentTypeId =
                weld::fromId<SwContentType*>(m_xTreeView->get_id(rParent))->GetType();
        const int nShift = static_cast<int>(eContentTypeId);
        SAL_WARN_IF(nShift < 0, "sw.ui", "ContentTypeId::UNKNOWN negative shift");
        if (nShift >= 0)
        {
            const sal_Int32 nAnd = ~(1 << nShift);
            if (State::HIDDEN != m_eState)
            {
                m_nActiveBlock &= nAnd;
                m_pConfig->SetActiveBlock(m_nActiveBlock);
            }
            else
                m_nHiddenBlock &= nAnd;
        }
    }
    else // content entry
    {
        SwWrtShell* pShell = GetWrtShell();
        ContentTypeId eContentTypeId =
                weld::fromId<SwContent*>(m_xTreeView->get_id(rParent))->GetParent()->GetType();
        if (eContentTypeId == ContentTypeId::OUTLINE)
        {
            assert(dynamic_cast<SwOutlineContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(rParent))));
            auto const nPos = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(rParent))->GetOutlinePos();
            void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
            mOutLineNodeMap[key] = false;
        }
        else if(eContentTypeId == ContentTypeId::REGION)
        {
            assert(dynamic_cast<SwRegionContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(rParent))));
            const void* key = static_cast<const void*>(weld::fromId<SwRegionContent*>(m_xTreeView->get_id(rParent))->GetSectionFormat());
            m_aRegionNodeExpandMap[key] = false;
        }
        else if(eContentTypeId == ContentTypeId::POSTIT)
        {
            assert(dynamic_cast<SwPostItContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(rParent))));
            const void* key = static_cast<const void*>(weld::fromId<SwPostItContent*>(m_xTreeView->get_id(rParent))->GetPostIt());
            m_aPostItNodeExpandMap[key] = false;
        }
    }

    return true;
}

// Also on double click will be initially opened only.
IMPL_LINK_NOARG(SwContentTree, ContentDoubleClickHdl, weld::TreeView&, bool)
{
    if (m_nRowActivateEventId)
        Application::RemoveUserEvent(m_nRowActivateEventId);
    // post the event to process row activate after mouse press event to be able to set key
    // modifier for selection feature (tdf#154211)
    m_nRowActivateEventId
            = Application::PostUserEvent(LINK(this, SwContentTree, AsyncContentDoubleClickHdl));

    bool bConsumed = false;

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (m_xTreeView->get_cursor(xEntry.get()) && lcl_IsContent(*xEntry, *m_xTreeView) &&
            (State::HIDDEN != m_eState))
    {
        SwContent* pCnt = weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry));
        assert(pCnt && "no UserData");
        if (pCnt && !pCnt->IsInvisible())
        {
            // fdo#36308 don't expand outlines on double-click
            bConsumed = pCnt->GetParent()->GetType() == ContentTypeId::OUTLINE;
        }
    }

    return bConsumed; // false/true == allow/disallow more to be done, i.e. expand/collapse children
}

IMPL_LINK_NOARG(SwContentTree, AsyncContentDoubleClickHdl, void*, void)
{
    m_nRowActivateEventId = nullptr;

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    bool bEntry = m_xTreeView->get_cursor(xEntry.get());
    // Is it a content type?
    OSL_ENSURE(bEntry, "no current entry!");
    if (bEntry)
    {
        if (lcl_IsContentType(*xEntry, *m_xTreeView) && !m_xTreeView->iter_has_child(*xEntry))
        {
            RequestingChildren(*xEntry);
            m_xTreeView->set_children_on_demand(*xEntry, false);
        }
        else if (!lcl_IsContentType(*xEntry, *m_xTreeView) && (State::HIDDEN != m_eState))
        {
            assert(dynamic_cast<SwContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xEntry))));
            SwContent* pCnt = weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry));
            assert(pCnt && "no UserData");
            if (pCnt && !pCnt->IsInvisible())
            {
                if (State::CONSTANT == m_eState)
                {
                    m_pActiveShell->GetView().GetViewFrame().GetWindow().ToTop();
                }
                //Jump to content type:
                GotoContent(pCnt);
            }
        }
    }
}

namespace
{
    OUString GetImageIdForContentTypeId(ContentTypeId eType)
    {
        OUString sResId;

        switch (eType)
        {
            case ContentTypeId::OUTLINE:
                sResId = RID_BMP_NAVI_OUTLINE;
                break;
            case ContentTypeId::TABLE:
                sResId = RID_BMP_NAVI_TABLE;
                break;
            case ContentTypeId::FRAME:
                sResId = RID_BMP_NAVI_FRAME;
                break;
            case ContentTypeId::GRAPHIC:
                sResId = RID_BMP_NAVI_GRAPHIC;
                break;
            case ContentTypeId::OLE:
                sResId = RID_BMP_NAVI_OLE;
                break;
            case ContentTypeId::BOOKMARK:
                sResId = RID_BMP_NAVI_BOOKMARK;
                break;
            case ContentTypeId::REGION:
                sResId = RID_BMP_NAVI_REGION;
                break;
            case ContentTypeId::URLFIELD:
                sResId = RID_BMP_NAVI_URLFIELD;
                break;
            case ContentTypeId::REFERENCE:
                sResId = RID_BMP_NAVI_REFERENCE;
                break;
            case ContentTypeId::INDEX:
                sResId = RID_BMP_NAVI_INDEX;
                break;
            case ContentTypeId::POSTIT:
                sResId = RID_BMP_NAVI_POSTIT;
                break;
            case ContentTypeId::DRAWOBJECT:
                sResId = RID_BMP_NAVI_DRAWOBJECT;
                break;
            case ContentTypeId::TEXTFIELD:
                sResId = RID_BMP_NAVI_TEXTFIELD;
                break;
            case ContentTypeId::FOOTNOTE:
                sResId = RID_BMP_NAVI_FOOTNOTE;
                break;
            case ContentTypeId::ENDNOTE:
                sResId = RID_BMP_NAVI_ENDNOTE;
                break;
            case ContentTypeId::UNKNOWN:
                SAL_WARN("sw.ui", "ContentTypeId::UNKNOWN has no bitmap preview");
                break;
        }

        return sResId;
    };
}

size_t SwContentTree::GetAbsPos(const weld::TreeIter& rIter)
{
    return weld::GetAbsPos(*m_xTreeView, rIter);
}

size_t SwContentTree::GetEntryCount() const
{
    return m_nEntryCount;
}

size_t SwContentTree::GetChildCount(const weld::TreeIter& rParent) const
{
    if (!m_xTreeView->iter_has_child(rParent))
        return 0;

    std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rParent));

    size_t nCount = 0;
    auto nRefDepth = m_xTreeView->get_iter_depth(*xParent);
    auto nActDepth = nRefDepth;
    do
    {
        if (!m_xTreeView->iter_next(*xParent))
            xParent.reset();
        else
            nActDepth = m_xTreeView->get_iter_depth(*xParent);
        nCount++;
    } while(xParent && nRefDepth < nActDepth);

    nCount--;
    return nCount;
}

std::unique_ptr<weld::TreeIter> SwContentTree::GetEntryAtAbsPos(size_t nAbsPos) const
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_iter_first(*xEntry))
        xEntry.reset();

    while (nAbsPos && xEntry)
    {
        if (!m_xTreeView->iter_next(*xEntry))
            xEntry.reset();
        nAbsPos--;
    }
    return xEntry;
}

void SwContentTree::Display( bool bActive )
{
    // First read the selected entry to select it later again if necessary
    // -> the user data here are no longer valid!
    std::unique_ptr<weld::TreeIter> xOldSelEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xOldSelEntry.get()))
        xOldSelEntry.reset();
    OUString sOldSelEntryId;
    size_t nEntryRelPos = 0; // relative position to their parent
    size_t nOldEntryCount = GetEntryCount();
    sal_Int32 nOldScrollPos = 0;
    if (xOldSelEntry)
    {
        UpdateLastSelType();
        sOldSelEntryId = m_xTreeView->get_id(*xOldSelEntry);
        nOldScrollPos = m_xTreeView->vadjustment_get_value();
        std::unique_ptr<weld::TreeIter> xParentEntry = m_xTreeView->make_iterator(xOldSelEntry.get());
        while (m_xTreeView->get_iter_depth(*xParentEntry))
            m_xTreeView->iter_parent(*xParentEntry);
        if (m_xTreeView->get_iter_depth(*xOldSelEntry))
            nEntryRelPos = GetAbsPos(*xOldSelEntry) - GetAbsPos(*xParentEntry);
    }

    clear();

    if (!bActive)
    {
        m_aOverlayObjectDelayTimer.Stop();
        if (m_xOverlayObject && m_xOverlayObject->getOverlayManager())
        {
            m_xOverlayObject->getOverlayManager()->remove(*m_xOverlayObject);
            m_xOverlayObject.reset();
        }
        m_eState = State::HIDDEN;
    }
    else if (State::HIDDEN == m_eState)
        m_eState = State::ACTIVE;
    SwWrtShell* pShell = GetWrtShell();
    const bool bReadOnly = !pShell || pShell->GetView().GetDocShell()->IsReadOnly();
    if(bReadOnly != m_bIsLastReadOnly)
    {
        m_bIsLastReadOnly = bReadOnly;
        bool bDisable =  pShell == nullptr || bReadOnly;
        SwNavigationPI* pNavi = GetParentWindow();
        pNavi->m_xContent6ToolBox->set_item_sensitive("chapterup", !bDisable);
        pNavi->m_xContent6ToolBox->set_item_sensitive("chapterdown", !bDisable);
        pNavi->m_xContent6ToolBox->set_item_sensitive("promote", !bDisable);
        pNavi->m_xContent6ToolBox->set_item_sensitive("demote", !bDisable);
        pNavi->m_xContent5ToolBox->set_item_sensitive("reminder", !bDisable);
    }

    if (pShell)
    {
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
        std::unique_ptr<weld::TreeIter> xCntTypeEntry;
        std::vector<std::unique_ptr<weld::TreeIter>> aNodesToExpand;
        // all content navigation view
        if(m_nRootType == ContentTypeId::UNKNOWN)
        {
            m_xTreeView->freeze();

            for( ContentTypeId nCntType : o3tl::enumrange<ContentTypeId>() )
            {
                std::unique_ptr<SwContentType>& rpContentT = bActive ?
                                    m_aActiveContentArr[nCntType] :
                                    m_aHiddenContentArr[nCntType];
                if(!rpContentT)
                    rpContentT.reset(new SwContentType(pShell, nCntType, m_nOutlineLevel ));

                OUString aImage(GetImageIdForContentTypeId(nCntType));
                bool bChOnDemand = 0 != rpContentT->GetMemberCount();

                // In case of LOK, empty content types must be hidden in the contenttree
                if (comphelper::LibreOfficeKit::isActive() && !bChOnDemand)
                {
                    continue;
                }

                OUString sId(weld::toId(rpContentT.get()));
                insert(nullptr, rpContentT->GetName(), sId, bChOnDemand, xEntry.get());
                m_xTreeView->set_image(*xEntry, aImage);

                m_xTreeView->set_sensitive(*xEntry, bChOnDemand);

                if (nCntType == m_nLastSelType)
                    xCntTypeEntry = m_xTreeView->make_iterator(xEntry.get());

                sal_Int32 nExpandOptions = (State::HIDDEN == m_eState)
                                            ? m_nHiddenBlock
                                            : m_nActiveBlock;
                if (nExpandOptions & (1 << static_cast<int>(nCntType)))
                {
                    // fill contents of to-be expanded entries while frozen
                    Expand(*xEntry, &aNodesToExpand);
                    m_xTreeView->set_children_on_demand(*xEntry, false);
                }
            }

            m_xTreeView->thaw();

            // restore visual expanded tree state
            for (const auto& rNode : aNodesToExpand)
                m_xTreeView->expand_row(*rNode);
        }
        // root content navigation view
        else
        {
            m_xTreeView->freeze();

            std::unique_ptr<SwContentType>& rpRootContentT = bActive ?
                                    m_aActiveContentArr[m_nRootType] :
                                    m_aHiddenContentArr[m_nRootType];
            if(!rpRootContentT)
                rpRootContentT.reset(new SwContentType(pShell, m_nRootType, m_nOutlineLevel ));
            OUString aImage(GetImageIdForContentTypeId(m_nRootType));
            bool bChOnDemand(m_nRootType == ContentTypeId::OUTLINE ||
                             m_nRootType == ContentTypeId::REGION ||
                             m_nRootType == ContentTypeId::POSTIT);
            OUString sId(weld::toId(rpRootContentT.get()));
            insert(nullptr, rpRootContentT->GetName(), sId, bChOnDemand, xEntry.get());
            m_xTreeView->set_image(*xEntry, aImage);

            xCntTypeEntry = m_xTreeView->make_iterator(xEntry.get());

            if (!bChOnDemand)
                InsertContent(*xEntry);
            else
            {
                // fill contents of to-be expanded entries while frozen
                Expand(*xEntry, &aNodesToExpand);
                m_xTreeView->set_children_on_demand(*xEntry, false);
            }

            m_xTreeView->set_sensitive(*xEntry, m_xTreeView->iter_has_child(*xEntry));

            m_xTreeView->thaw();

            if (bChOnDemand)
            {
                // restore visual expanded tree state
                for (const auto& rNode : aNodesToExpand)
                    m_xTreeView->expand_row(*rNode);
            }
            else
                m_xTreeView->expand_row(*xEntry);
        }

        // Reselect the old selected entry. If it is not available, select the entry at the old
        // selected entry position unless that entry position is now a content type or is past the
        // end of the member list then select the entry at the previous entry position.
        if (xOldSelEntry)
        {
            std::unique_ptr<weld::TreeIter> xSelEntry = m_xTreeView->make_iterator(xCntTypeEntry.get());
            if (nEntryRelPos)
            {
                std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator(xCntTypeEntry.get()));
                std::unique_ptr<weld::TreeIter> xTemp(m_xTreeView->make_iterator(xIter.get()));
                sal_uLong nPos = 1;
                bool bNext;
                while ((bNext = m_xTreeView->iter_next(*xIter) && lcl_IsContent(*xIter, *m_xTreeView)))
                {
                    if (m_xTreeView->get_id(*xIter) == sOldSelEntryId || nPos == nEntryRelPos)
                    {
                        m_xTreeView->copy_iterator(*xIter, *xSelEntry);
                        break;
                    }
                    m_xTreeView->copy_iterator(*xIter, *xTemp); // note previous entry
                    nPos++;
                }
                if (!bNext)
                    xSelEntry = std::move(xTemp);
            }
            // set_cursor unselects all entries, makes passed entry visible, and selects it
            m_xTreeView->set_cursor(*xSelEntry);
            Select();
        }
    }

    if (!m_bIgnoreDocChange && GetEntryCount() == nOldEntryCount)
    {
        m_xTreeView->vadjustment_set_value(nOldScrollPos);
    }
}

void SwContentTree::clear()
{
    m_xTreeView->freeze();
    m_xTreeView->clear();
    m_nEntryCount = 0;
    m_xTreeView->thaw();
}

bool SwContentTree::FillTransferData( TransferDataContainer& rTransfer,
                                            sal_Int8& rDragMode )
{
    bool bRet = false;
    SwWrtShell* pWrtShell = GetWrtShell();
    OSL_ENSURE(pWrtShell, "no Shell!");

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    bool bEntry = m_xTreeView->get_cursor(xEntry.get());
    if (!bEntry || lcl_IsContentType(*xEntry, *m_xTreeView) || !pWrtShell)
        return false;
    OUString sEntry;
    assert(dynamic_cast<SwContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xEntry))));
    SwContent* pCnt = weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry));

    const ContentTypeId nActType = pCnt->GetParent()->GetType();
    OUString sUrl;
    bool bOutline = false;
    OUString sOutlineText;
    switch( nActType )
    {
        case ContentTypeId::OUTLINE:
        {
            const SwOutlineNodes::size_type nPos = static_cast<SwOutlineContent*>(pCnt)->GetOutlinePos();
            OSL_ENSURE(nPos < pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount(),
                       "outlinecnt changed");

            // make sure outline may actually be copied
            if( pWrtShell->IsOutlineCopyable( nPos ) )
            {
                const SwNumRule* pOutlRule = pWrtShell->GetOutlineNumRule();
                const SwTextNode* pTextNd =
                        pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNode(nPos);
                if (pTextNd && pOutlRule && pTextNd->IsNumbered(pWrtShell->GetLayout()))
                {
                    SwNumberTree::tNumberVector aNumVector =
                        pTextNd->GetNumberVector(pWrtShell->GetLayout());
                    for( int nLevel = 0;
                         nLevel <= pTextNd->GetActualListLevel();
                         nLevel++ )
                    {
                        const SwNumberTree::tSwNumTreeNumber nVal = aNumVector[nLevel] + 1;
                        sEntry += OUString::number( nVal - pOutlRule->Get(nLevel).GetStart() ) + ".";
                    }
                }
                sEntry += pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(nPos, pWrtShell->GetLayout(), false);
                sOutlineText = pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(nPos, pWrtShell->GetLayout());
                m_bIsOutlineMoveable = static_cast<SwOutlineContent*>(pCnt)->IsMoveable();
                bOutline = true;
            }
        }
        break;
        case ContentTypeId::POSTIT:
        case ContentTypeId::INDEX:
        case ContentTypeId::REFERENCE :
        case ContentTypeId::TEXTFIELD:
        case ContentTypeId::FOOTNOTE:
        case ContentTypeId::ENDNOTE:
            // cannot be inserted, neither as URL nor as section
        break;
        case ContentTypeId::URLFIELD:
            sUrl = static_cast<SwURLFieldContent*>(pCnt)->GetURL();
            [[fallthrough]];
        case ContentTypeId::OLE:
        case ContentTypeId::GRAPHIC:
            if(GetParentWindow()->GetRegionDropMode() != RegionMode::NONE)
                break;
            else
                rDragMode &= ~( DND_ACTION_MOVE | DND_ACTION_LINK );
            [[fallthrough]];
        default:
            sEntry = m_xTreeView->get_text(*xEntry);
    }

    if(!sEntry.isEmpty())
    {
        const SwDocShell* pDocShell = pWrtShell->GetView().GetDocShell();
        if(sUrl.isEmpty())
        {
            if(pDocShell->HasName())
            {
                SfxMedium* pMedium = pDocShell->GetMedium();
                sUrl = pMedium->GetURLObject().GetURLNoMark();
                // only if a primarily link shall be integrated.
                bRet = true;
            }
            else if ( nActType == ContentTypeId::REGION || nActType == ContentTypeId::BOOKMARK )
            {
                // For field and bookmarks a link is also allowed
                // without a filename into its own document.
                bRet = true;
            }
            else if (State::CONSTANT == m_eState &&
                    ( !::GetActiveView() ||
                        m_pActiveShell != ::GetActiveView()->GetWrtShellPtr()))
            {
                // Urls of inactive views cannot dragged without
                // file names, also.
                bRet = false;
            }
            else
            {
                bRet = GetParentWindow()->GetRegionDropMode() == RegionMode::NONE;
                rDragMode = DND_ACTION_MOVE;
            }

            const OUString& rToken = pCnt->GetParent()->GetTypeToken();
            sUrl += "#" + sEntry;
            if(!rToken.isEmpty())
            {
                sUrl += OUStringChar(cMarkSeparator) + rToken;
            }
        }
        else
            bRet = true;

        if( bRet )
        {
            // In Outlines of heading text must match
            // the real number into the description.
            if(bOutline)
                sEntry = sOutlineText;

            {
                NaviContentBookmark aBmk( sUrl, sEntry,
                                    GetParentWindow()->GetRegionDropMode(),
                                    pDocShell);
                aBmk.Copy( rTransfer );
            }

            // An INetBookmark must a be delivered to foreign DocShells
            if( pDocShell->HasName() )
            {
                INetBookmark aBkmk( sUrl, sEntry );
                rTransfer.CopyINetBookmark( aBkmk );
            }
        }
    }
    return bRet;
}

void SwContentTree::ToggleToRoot()
{
    if(!m_bIsRoot)
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        bool bEntry = m_xTreeView->get_cursor(xEntry.get());
        if (bEntry)
        {
            const SwContentType* pCntType;
            if (lcl_IsContentType(*xEntry, *m_xTreeView))
            {
                assert(dynamic_cast<SwContentType*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xEntry))));
                pCntType = weld::fromId<SwContentType*>(m_xTreeView->get_id(*xEntry));
            }
            else
            {
                assert(dynamic_cast<SwContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xEntry))));
                pCntType = weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry))->GetParent();
            }
            m_nRootType = pCntType->GetType();
            m_bIsRoot = true;
            if (m_nRootType == ContentTypeId::OUTLINE || m_nRootType == ContentTypeId::DRAWOBJECT)
            {
                m_xTreeView->set_selection_mode(SelectionMode::Multiple);
            }
            Display(State::HIDDEN != m_eState);
        }
    }
    else
    {
        m_xTreeView->set_selection_mode(SelectionMode::Single);
        m_nLastSelType = m_nRootType;
        m_nRootType = ContentTypeId::UNKNOWN;
        m_bIsRoot = false;
        // Other content type member data could have changed while in root view. Fill the content
        // member lists excluding the toggled from root content which should already have the most
        // recent data.
        if (State::HIDDEN != m_eState)
        {
            for (ContentTypeId i : o3tl::enumrange<ContentTypeId>())
            {
                if (i != m_nLastSelType && m_aActiveContentArr[i])
                    m_aActiveContentArr[i]->FillMemberList();
            }
        }
        Display(State::HIDDEN != m_eState);
    }
    m_pConfig->SetRootType( m_nRootType );
    weld::Toolbar* pBox = GetParentWindow()->m_xContent5ToolBox.get();
    pBox->set_item_active("root", m_bIsRoot);
}

bool SwContentTree::HasContentChanged()
{
    bool bContentChanged = false;

//  - Run through the local array and the Treelistbox in parallel.
//  - Are the records not expanded, they are discarded only in the array
//    and the content type will be set as the new UserData.
//  - Is the root mode is active only this will be updated.

//  Valid for the displayed content types is:
//  the Memberlist will be erased and the membercount will be updated
//  If content will be checked, the memberlists will be replenished
//  at the same time. Once a difference occurs it will be only replenished
//  no longer checked. Finally, the box is filled again.

    if (State::HIDDEN == m_eState)
    {
        for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
        {
            if(m_aActiveContentArr[i])
                m_aActiveContentArr[i]->Invalidate();
        }
        return false;
    }

    // single content type navigation view
    if(m_bIsRoot)
    {
        std::unique_ptr<weld::TreeIter> xRootEntry(m_xTreeView->make_iterator());
        if (!m_xTreeView->get_iter_first(*xRootEntry))
            return true;

        assert(dynamic_cast<SwContentType*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xRootEntry))));
        const ContentTypeId nType = weld::fromId<SwContentType*>(m_xTreeView->get_id(*xRootEntry))->GetType();
        SwContentType* pArrType = m_aActiveContentArr[nType].get();
        assert(weld::toId(pArrType) == m_xTreeView->get_id(*xRootEntry));
        if (!pArrType)
            return true;

        pArrType->FillMemberList(&bContentChanged);
        if (bContentChanged)
            return true;

        // FillMemberList tests if member count in old member array equals member count in new
        // member array. Test here for member count difference between array and tree.
        const size_t nChildCount = GetChildCount(*xRootEntry);
        if (nChildCount != pArrType->GetMemberCount())
            return true;

        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(xRootEntry.get()));
        for (size_t j = 0; j < nChildCount; ++j)
        {
            if (!m_xTreeView->iter_next(*xEntry))
            {
                SAL_WARN("sw.ui", "unexpected missing entry");
                return true;
            }

            // FillMemberList clears the content type member list and refills with new data.
            // Treeview entry user data is set here to the string representation of the pointer to
            // the member data in the array. The Display function will clear and recreate the
            // treeview from the content type member arrays if content change is detected.
            const SwContent* pCnt = pArrType->GetMember(j);

            if (pCnt->IsInvisible() != m_xTreeView->get_sensitive(*xEntry, 0))
                return true;

            OUString sEntryText = m_xTreeView->get_text(*xEntry);
            if (sEntryText != pCnt->GetName() &&
                    !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
            {
                return true;
            }

            // Set_id needs to be done here because FillMemberList clears the content type member
            // list and refills with new data making the previously set id invalid. If there is no
            // content change detected the Display function will not be called and the tree entry
            // user data will not be set to the new content member pointer address.
            OUString sSubId(weld::toId(pCnt));
            m_xTreeView->set_id(*xEntry, sSubId);
        }
    }
    // all content types navigation view
    else
    {
        // Fill member list for each content type and check for content change. If content change
        // is detected only fill member lists for remaining content types. The Display function
        // will clear and recreate the treeview from the content type member arrays if content has
        // changed.

        if (comphelper::LibreOfficeKit::isActive())
        {
            // In case of LOK, empty contentTypes are hidden, even in all content view
            // so it is not enough to check only the m_xTreeView.
            bool bCountChanged = false;
            bool bHasContentChanged = false;
            for (ContentTypeId i : o3tl::enumrange<ContentTypeId>())
            {
                if (m_aActiveContentArr[i])
                {
                    auto nLastTMCount = m_aActiveContentArr[i]->GetMemberCount();
                    if (i == ContentTypeId::OUTLINE) // this is required for checking if header level is changed
                        m_aActiveContentArr[i]->FillMemberList(&bHasContentChanged);
                    else
                        m_aActiveContentArr[i]->FillMemberList();
                    // If the member count of a type is changed, then the content is surely changed
                    if (m_aActiveContentArr[i]->GetMemberCount() != nLastTMCount)
                        bCountChanged = true;
                    if (bHasContentChanged)
                        bContentChanged = true;
                }
            }
            if (bCountChanged || bContentChanged)
                return true;
        }

        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());

        // lambda function to find the next content type entry
        auto lcl_nextContentTypeEntry = [this, &xEntry](){
            while (m_xTreeView->get_iter_depth(*xEntry))
                m_xTreeView->iter_parent(*xEntry);
            return m_xTreeView->iter_next_sibling(*xEntry);
            };

        for (bool bEntry = m_xTreeView->get_iter_first(*xEntry); bEntry;
             bEntry = lcl_nextContentTypeEntry())
        {
            assert(dynamic_cast<SwContentType*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xEntry))));
            SwContentType* pCntType = weld::fromId<SwContentType*>(m_xTreeView->get_id(*xEntry));
            const size_t nCntCount = pCntType->GetMemberCount();
            const ContentTypeId nType = pCntType->GetType();
            SwContentType* pArrType = m_aActiveContentArr[nType].get();
            assert(weld::toId(pArrType) == m_xTreeView->get_id(*xEntry));

            if (!pArrType)
            {
                bContentChanged = true;
                continue;
            }

            // all content type member lists must be filled!
            if (bContentChanged)
            {
                // If content change has already been detected there is no need to detect
                // other content change so no argument is supplied here to FillMemberList.
                pArrType->FillMemberList();
                continue;
            }

            pArrType->FillMemberList(&bContentChanged);
            if (bContentChanged)
                continue;

            // does entry have children?
            if (m_xTreeView->get_row_expanded(*xEntry))
            {
                const size_t nChildCount = GetChildCount(*xEntry);
                if(nChildCount != pArrType->GetMemberCount())
                {
                    bContentChanged = true;
                    continue;
                }

                for(size_t j = 0; j < nChildCount; ++j)
                {
                    if (!m_xTreeView->iter_next(*xEntry))
                    {
                        SAL_WARN("sw.ui", "unexpected missing entry");
                        bContentChanged = true;
                        break;
                    }

                    const SwContent* pCnt = pArrType->GetMember(j);

                    if (pCnt->IsInvisible() != m_xTreeView->get_sensitive(*xEntry, 0))
                    {
                        bContentChanged = true;
                        break;
                    }

                    OUString sEntryText = m_xTreeView->get_text(*xEntry);
                    if( sEntryText != pCnt->GetName() &&
                            !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
                    {
                        bContentChanged = true;
                        break;
                    }

                    // See comment above in single content type navigation view block for why the
                    // following is done here.
                    OUString sSubId(weld::toId(pCnt));
                    m_xTreeView->set_id(*xEntry, sSubId);
                }
            }
            // not expanded and has children
            else if (m_xTreeView->iter_has_child(*xEntry))
            {
                bool bRemoveChildren = false;
                const size_t nOldChildCount = GetChildCount(*xEntry);
                const size_t nNewChildCount = pArrType->GetMemberCount();
                if (nOldChildCount != nNewChildCount)
                {
                    bRemoveChildren = true;
                }
                else
                {
                    std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(xEntry.get()));
                    (void)m_xTreeView->iter_children(*xChild);
                    for (size_t j = 0; j < nOldChildCount; ++j)
                    {
                        const SwContent* pCnt = pArrType->GetMember(j);
                        OUString sSubId(weld::toId(pCnt));
                        m_xTreeView->set_id(*xChild, sSubId);
                        OUString sEntryText = m_xTreeView->get_text(*xChild);
                        if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
                        {
                            bRemoveChildren = true;
                        }
                        (void)m_xTreeView->iter_next(*xChild);
                    }
                }
                if (bRemoveChildren)
                {
                    std::unique_ptr<weld::TreeIter> xRemove(m_xTreeView->make_iterator(xEntry.get()));
                    while (m_xTreeView->iter_children(*xRemove))
                    {
                        remove(*xRemove);
                        m_xTreeView->copy_iterator(*xEntry, *xRemove);
                    }
                    m_xTreeView->set_children_on_demand(*xEntry, nNewChildCount != 0);
                }
            }
            else if((nCntCount != 0)
                    != (pArrType->GetMemberCount()!=0))
            {
                bContentChanged = true;
                continue;
            }
        }
    }

    return bContentChanged;
}

void SwContentTree::UpdateLastSelType()
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (m_xTreeView->get_selected(xEntry.get()))
    {
        while (m_xTreeView->get_iter_depth(*xEntry))
            m_xTreeView->iter_parent(*xEntry);
        void* pId = weld::fromId<void*>(m_xTreeView->get_id(*xEntry));
        if (pId && lcl_IsContentType(*xEntry, *m_xTreeView))
        {
            assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pId)));
            m_nLastSelType = static_cast<SwContentType*>(pId)->GetType();
        }
    }
}

void SwContentTree::FindActiveTypeAndRemoveUserData()
{
    UpdateLastSelType();

    // If clear is called by TimerUpdate:
    // Only for root can the validity of the UserData be guaranteed.
    m_xTreeView->all_foreach([this](weld::TreeIter& rEntry){
        m_xTreeView->set_id(rEntry, "");
        return false;
    });
}

void SwContentTree::SetHiddenShell(SwWrtShell* pSh)
{
    m_pHiddenShell = pSh;
    m_eState = State::HIDDEN;
    FindActiveTypeAndRemoveUserData();
    for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
    {
        m_aHiddenContentArr[i].reset();
    }
    Display(false);

    GetParentWindow()->UpdateListBox();
}

void SwContentTree::SetActiveShell(SwWrtShell* pSh)
{
    bool bClear = m_pActiveShell != pSh;
    if (State::ACTIVE == m_eState && bClear)
    {
        EndListeningAll();
        m_pActiveShell = pSh;
        FindActiveTypeAndRemoveUserData();
        clear();
    }
    else if (State::CONSTANT == m_eState)
    {
        EndListeningAll();
        m_pActiveShell = pSh;
        m_eState = State::ACTIVE;
        bClear = true;
    }

    // tdf#148432 in LTR UI override the navigator treeview direction based on
    // the first page directionality
    if (m_pActiveShell && !AllSettings::GetLayoutRTL())
    {
        const SwPageDesc& rDesc = m_pActiveShell->GetPageDesc(0);
        const SvxFrameDirectionItem& rFrameDir = rDesc.GetMaster().GetFrameDir();
        m_xTreeView->set_direction(rFrameDir.GetValue() == SvxFrameDirection::Horizontal_RL_TB);
    }

    // Only if it is the active view, the array will be deleted and
    // the screen filled new.
    if (State::ACTIVE == m_eState && bClear)
    {
        if (m_pActiveShell)
            StartListening(*m_pActiveShell->GetView().GetDocShell());
        FindActiveTypeAndRemoveUserData();
        for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
        {
            m_aActiveContentArr[i].reset();
        }
        Display(true);
    }
}

void SwContentTree::SetConstantShell(SwWrtShell* pSh)
{
    EndListeningAll();
    m_pActiveShell = pSh;
    m_eState = State::CONSTANT;
    StartListening(*m_pActiveShell->GetView().GetDocShell());
    FindActiveTypeAndRemoveUserData();
    for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
    {
        m_aActiveContentArr[i].reset();
    }
    Display(true);
}

void SwContentTree::Notify(SfxBroadcaster & rBC, SfxHint const& rHint)
{
    SfxViewEventHint const*const pVEHint(dynamic_cast<SfxViewEventHint const*>(&rHint));
    SwXTextView* pDyingShell = nullptr;
    if (m_pActiveShell && pVEHint && pVEHint->GetEventName() == "OnViewClosed")
        pDyingShell = dynamic_cast<SwXTextView*>(pVEHint->GetController().get());
    if (pDyingShell && pDyingShell->GetView() == &m_pActiveShell->GetView())
    {
        SetActiveShell(nullptr); // our view is dying, clear our pointers to it
    }
    else
    {
        SfxListener::Notify(rBC, rHint);
    }
    switch (rHint.GetId())
    {
        case SfxHintId::SwNavigatorUpdateTracking:
            UpdateTracking();
            break;
        case SfxHintId::SwNavigatorSelectOutlinesWithSelections:
        {
            if (m_nRootType == ContentTypeId::OUTLINE)
            {
                SelectOutlinesWithSelection();
                // make first selected entry visible
                std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
                if (xEntry && m_xTreeView->get_selected(xEntry.get()))
                    m_xTreeView->scroll_to_row(*xEntry);
            }
            else if (m_nRootType == ContentTypeId::UNKNOWN)
                m_xTreeView->unselect_all();
            break;
        }
        case SfxHintId::DocChanged:
            OverlayObject();
            if (!m_bIgnoreDocChange)
            {
                m_bDocHasChanged = true;
                TimerUpdate(&m_aUpdTimer);
            }
            break;
        case SfxHintId::ModeChanged:
            if (SwWrtShell* pShell = GetWrtShell())
            {
                const bool bReadOnly = pShell->GetView().GetDocShell()->IsReadOnly();
                if (bReadOnly != m_bIsLastReadOnly)
                {
                    m_bIsLastReadOnly = bReadOnly;

                    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
                    if (m_xTreeView->get_cursor(xEntry.get()))
                    {
                        m_xTreeView->select(*xEntry);
                        Select();
                    }
                    else
                        m_xTreeView->unselect_all();
                }
            }
            break;
        default:
            break;
    }
}

// Handler for outline entry up/down left/right movement
void SwContentTree::ExecCommand(std::u16string_view rCmd, bool bOutlineWithChildren)
{
    if (m_xTreeView->count_selected_rows() == 0)
        return;

    const bool bUp = rCmd == u"chapterup";
    const bool bUpDown = bUp || rCmd == u"chapterdown";
    const bool bLeft = rCmd == u"promote";
    const bool bLeftRight = bLeft || rCmd == u"demote";
    if (!bUpDown && !bLeftRight)
        return;
    if (GetWrtShell()->GetView().GetDocShell()->IsReadOnly() ||
        (State::ACTIVE != m_eState &&
         (State::CONSTANT != m_eState || m_pActiveShell != GetParentWindow()->GetCreateView()->GetWrtShellPtr())))
    {
        return;
    }

    SwWrtShell *const pShell = GetWrtShell();

    const SwNodes& rNodes = pShell->GetNodes();
    const SwOutlineNodes& rOutlineNodes = rNodes.GetOutLineNds();
    const SwOutlineNodes::size_type nOutlineNdsSize = rOutlineNodes.size();

    std::vector<SwTextNode*> selectedOutlineNodes;
    std::vector<std::unique_ptr<weld::TreeIter>> selected;

    m_xTreeView->selected_foreach([&](weld::TreeIter& rEntry){
        // it's possible to select the root node too which is a really bad idea
        if (lcl_IsContentType(rEntry, *m_xTreeView))
            return false;
        // filter out children of selected parents so they don't get promoted
        // or moved twice (except if there is Ctrl modifier, since in that
        // case children are re-parented)
        if ((bLeftRight || bOutlineWithChildren) && !selected.empty())
        {
            std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rEntry));
            for (bool bParent = m_xTreeView->iter_parent(*xParent); bParent; bParent = m_xTreeView->iter_parent(*xParent))
            {
                if (m_xTreeView->iter_compare(*selected.back(), *xParent) == 0)
                {
                    return false;
                }
            }
        }
        selected.emplace_back(m_xTreeView->make_iterator(&rEntry));

        // Use the outline node position in the SwOutlineNodes array. Bad things
        // happen if the tree entry position is used and it doesn't match the node position
        // in SwOutlineNodes, which is usually the case for outline nodes in frames.
        const SwOutlineNodes::size_type nPos
                = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(rEntry))->GetOutlinePos();
        if (nPos < nOutlineNdsSize)
        {
            SwNode* pNode = rNodes.GetOutLineNds()[ nPos ];
            if (pNode)
            {
                selectedOutlineNodes.push_back(pNode->GetTextNode());
            }
        }
        return false;
    });

    if (!selected.size())
        return;

    if (bUpDown && !bUp)
    {   // to move down, start at the end!
        std::reverse(selected.begin(), selected.end());
    }

    m_bIgnoreDocChange = true;

    SwOutlineNodes::size_type nActPos;
    bool bStartedAction = false;

    MakeAllOutlineContentTemporarilyVisible a(GetWrtShell()->GetDoc());

    // get first regular document content node outline node position in outline nodes array
    SwOutlineNodes::size_type nFirstRegularDocContentOutlineNodePos = SwOutlineNodes::npos;
    SwNodeOffset nEndOfExtrasIndex = rNodes.GetEndOfExtras().GetIndex();
    for (SwOutlineNodes::size_type nPos = 0; nPos < nOutlineNdsSize; nPos++)
    {
        if (rOutlineNodes[nPos]->GetIndex() > nEndOfExtrasIndex)
        {
            nFirstRegularDocContentOutlineNodePos = nPos;
            break;
        }
    }

    for (auto const& pCurrentEntry : selected)
    {
        nActPos = weld::fromId<SwOutlineContent*>(
                    m_xTreeView->get_id(*pCurrentEntry))->GetOutlinePos();

        // outline nodes in frames and tables are not up/down moveable
        if (nActPos == SwOutlineNodes::npos ||
                (bUpDown && (!pShell->IsOutlineMovable(nActPos) ||
                 nFirstRegularDocContentOutlineNodePos == SwOutlineNodes::npos)))
        {
            continue;
        }

        if (!bStartedAction)
        {
            pShell->StartAllAction();
            pShell->StartUndo(bLeftRight ? SwUndoId::OUTLINE_LR : SwUndoId::OUTLINE_UD);
            bStartedAction = true;
        }

        pShell->GotoOutline( nActPos); // If text selection != box selection
        pShell->Push();

        if (bUpDown)
        {
            // move outline position up/down (outline position promote/demote)
            SwOutlineNodes::difference_type nDir = bUp ? -1 : 1;
            if ((nDir == -1 && nActPos > 0) || (nDir == 1 && nActPos < nOutlineNdsSize - 1))
            {
                // make outline selection for use by MoveOutlinePara
                pShell->MakeOutlineSel(nActPos, nActPos, bOutlineWithChildren);

                int nActPosOutlineLevel =
                        rOutlineNodes[nActPos]->GetTextNode()->GetAttrOutlineLevel();
                SwOutlineNodes::size_type nPos = nActPos;
                if (!bUp)
                {
                    // move down
                    int nPosOutlineLevel = -1;
                    while (++nPos < nOutlineNdsSize)
                    {
                        nPosOutlineLevel =
                                rOutlineNodes[nPos]->GetTextNode()->GetAttrOutlineLevel();
                        // discontinue if moving out of parent or equal level is found
                        if (nPosOutlineLevel <= nActPosOutlineLevel)
                            break;
                        // count the children of the node when they are not included in the move
                        if (!bOutlineWithChildren)
                            nDir++;
                    }
                    if (nPosOutlineLevel >= nActPosOutlineLevel)
                    {
                        // move past children
                        while (++nPos < nOutlineNdsSize)
                        {
                            nPosOutlineLevel =
                                    rOutlineNodes[nPos]->GetTextNode()->GetAttrOutlineLevel();
                            // discontinue if moving out of parent or equal level is found
                            if (nPosOutlineLevel <= nActPosOutlineLevel)
                                break;
                            nDir++;
                        }
                    }
                }
                else
                {
                    // move up
                    while (nPos && --nPos >= nFirstRegularDocContentOutlineNodePos)
                    {
                        int nPosOutlineLevel =
                                rOutlineNodes[nPos]->GetTextNode()->GetAttrOutlineLevel();
                        // discontinue if equal level is found
                        if (nPosOutlineLevel == nActPosOutlineLevel)
                            break;
                        // discontinue if moving out of parent
                        if (nPosOutlineLevel < nActPosOutlineLevel)
                        {
                            // Required for expected chapter placement when the chapter being moved
                            // up has an outline level less than the outline level of chapters it
                            // is being moved above and then encounters a chapter with an outline
                            // level that is greater before reaching a chapter with the same
                            // outline level as itself.
                            if (nDir < -1)
                                nDir++;
                            break;
                        }
                        nDir--;
                    }
                }
                pShell->MoveOutlinePara(nDir);
            }
            pShell->ClearMark();
        }
        else
        {
            // move outline left/right (outline level promote/demote)
            if (!pShell->IsProtectedOutlinePara())
            {
                bool bAllow = true;
                const SwOutlineNodes& rOutlNds = pShell->GetDoc()->GetNodes().GetOutLineNds();
                const int nActLevel = rOutlNds[nActPos]->GetTextNode()->GetAttrOutlineLevel();
                if (!bLeft)
                {
                    // disallow if any outline node to demote will exceed MAXLEVEL
                    SwOutlineNodes::size_type nPos = nActPos;
                    do
                    {
                        int nLevel = rOutlNds[nPos]->GetTextNode()->GetAttrOutlineLevel();
                        if (nLevel == MAXLEVEL)
                        {
                            bAllow = false;
                            break;
                        }
                    } while (bOutlineWithChildren && ++nPos < rOutlNds.size() &&
                             rOutlNds[nPos]->GetTextNode()->GetAttrOutlineLevel() > nActLevel);
                }
                else
                {
                    // disallow if trying to promote outline of level 1
                    if (nActLevel == 1)
                        bAllow = false;
                }
                if (bAllow)
                {
                    SwOutlineNodes::size_type nPos = nActPos;
                    do
                    {
                        pShell->SwCursorShell::GotoOutline(nPos);
                        pShell->OutlineUpDown(bLeft ? -1 : 1);
                    } while (bOutlineWithChildren && ++nPos < rOutlNds.size() &&
                             rOutlNds[nPos]->GetTextNode()->GetAttrOutlineLevel() > nActLevel);
                }
            }
        }

        pShell->Pop(SwCursorShell::PopMode::DeleteCurrent); // Cursor is now back at the current heading.
    }

    if (bStartedAction)
    {
        pShell->EndUndo();
        pShell->EndAllAction();
        if (m_aActiveContentArr[ContentTypeId::OUTLINE])
            m_aActiveContentArr[ContentTypeId::OUTLINE]->Invalidate();

        // tdf#143547 LO Writer: navigator should stand still on promoting and demoting
        // In addition to m_bIgnoreDocChange being true, selections are cleared before the Display
        // call. Either of these conditions disable restore of scroll position happening in the
        // Display function so it needs to be done here.
        auto nOldScrollPos = m_xTreeView->vadjustment_get_value();

        // clear all selections to prevent the Display function from trying to reselect selected entries
        m_xTreeView->unselect_all();
        Display(true);
        m_xTreeView->vadjustment_set_value(nOldScrollPos);

        if (m_bIsRoot)
        {
            // reselect entries, do this only when in outline content navigation mode
            const SwOutlineNodes& rOutlineNds = pShell->GetNodes().GetOutLineNds();
            for (SwTextNode* pNode : selectedOutlineNodes)
            {
                m_xTreeView->all_foreach([this, &rOutlineNds, pNode](weld::TreeIter& rEntry){
                    if (lcl_IsContentType(rEntry, *m_xTreeView))
                        return false;
                    SwOutlineNodes::size_type nPos = weld::fromId<SwOutlineContent*>(
                                m_xTreeView->get_id(rEntry))->GetOutlinePos();
                    if (pNode == rOutlineNds[nPos]->GetTextNode())
                    {
                        std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rEntry));
                        if (m_xTreeView->iter_parent(*xParent)
                                && !m_xTreeView->get_row_expanded(*xParent))
                        {
                            m_xTreeView->expand_row(*xParent);
                        }
                        m_xTreeView->select(rEntry);
                        return true;
                    }
                    return false;
                });
            }
        }
        else
        {
            m_pActiveShell->GetView().GetEditWin().GrabFocus();
            m_bIgnoreDocChange = false;
            UpdateTracking();
            grab_focus();
        }
    }
    m_bIgnoreDocChange = false;
}

void SwContentTree::ShowTree()
{
    m_xTreeView->show();
    m_aUpdTimer.Start();
}

void SwContentTree::HideTree()
{
    // folded together will not be idled
    m_aUpdTimer.Stop();
    m_xTreeView->hide();
}

static void lcl_SelectByContentTypeAndAddress(SwContentTree* pThis, weld::TreeView& rContentTree,
                                              ContentTypeId nType, const void* ptr)
{
    if (!ptr)
    {
        rContentTree.set_cursor(-1);
        pThis->Select();
        return;
    }

    // find content type entry
    std::unique_ptr<weld::TreeIter> xIter(rContentTree.make_iterator());

    bool bFoundEntry = rContentTree.get_iter_first(*xIter);
    while (bFoundEntry)
    {
        void* pUserData = weld::fromId<void*>(rContentTree.get_id(*xIter));
        assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pUserData)));
        if (nType == static_cast<SwContentType*>(pUserData)->GetType())
            break;
        bFoundEntry = rContentTree.iter_next_sibling(*xIter);
    }

    if (!bFoundEntry)
    {
        rContentTree.set_cursor(-1);
        pThis->Select();
        return;
    }

    // assure content type entry is expanded
    rContentTree.expand_row(*xIter);

    // find content type content entry and select it
    const void* p = nullptr;
    while (rContentTree.iter_next(*xIter) && lcl_IsContent(*xIter, rContentTree))
    {
        void* pUserData = weld::fromId<void*>(rContentTree.get_id(*xIter));
        switch( nType )
        {
            case ContentTypeId::FOOTNOTE:
            case ContentTypeId::ENDNOTE:
            {
                assert(dynamic_cast<SwTextFootnoteContent*>(static_cast<SwTypeNumber*>(pUserData)));
                SwTextFootnoteContent* pCnt = static_cast<SwTextFootnoteContent*>(pUserData);
                p = pCnt->GetTextFootnote();
                break;
            }
            case ContentTypeId::URLFIELD:
            {
                assert(dynamic_cast<SwURLFieldContent*>(static_cast<SwTypeNumber*>(pUserData)));
                SwURLFieldContent* pCnt = static_cast<SwURLFieldContent*>(pUserData);
                p = static_cast<const SwTextAttr*>(pCnt->GetINetAttr());
                break;
            }
            case ContentTypeId::TEXTFIELD:
            {
                assert(dynamic_cast<SwTextFieldContent*>(static_cast<SwTypeNumber*>(pUserData)));
                SwTextFieldContent* pCnt = static_cast<SwTextFieldContent*>(pUserData);
                p = pCnt->GetFormatField()->GetField();
                break;
            }
            case ContentTypeId::POSTIT:
            {
                assert(dynamic_cast<SwPostItContent*>(static_cast<SwTypeNumber*>(pUserData)));
                SwPostItContent* pCnt = static_cast<SwPostItContent*>(pUserData);
                p = pCnt->GetPostIt()->GetField();
                break;
            }
            default:
                break;
        }
        if (ptr == p)
        {
            // get first selected for comparison
            std::unique_ptr<weld::TreeIter> xFirstSelected(rContentTree.make_iterator());
            if (!rContentTree.get_selected(xFirstSelected.get()))
                xFirstSelected.reset();
            if (rContentTree.count_selected_rows() != 1 || !xFirstSelected ||
                    rContentTree.iter_compare(*xIter, *xFirstSelected) != 0)
            {
                // unselect all entries and make passed entry visible and selected
                rContentTree.set_cursor(*xIter);
                pThis->Select();
            }
            return;
        }
    }

    rContentTree.set_cursor(-1);
    pThis->Select();
    return;
}

static void lcl_SelectByContentTypeAndName(SwContentTree* pThis, weld::TreeView& rContentTree,
                                           std::u16string_view rContentTypeName, std::u16string_view rName)
{
    if (rName.empty())
        return;

    // find content type entry
    std::unique_ptr<weld::TreeIter> xIter(rContentTree.make_iterator());
    bool bFoundEntry = rContentTree.get_iter_first(*xIter);
    while (bFoundEntry && rContentTypeName != rContentTree.get_text(*xIter))
        bFoundEntry = rContentTree.iter_next_sibling(*xIter);
    // find content type content entry and select it
    if (!bFoundEntry)
        return;

    rContentTree.expand_row(*xIter); // assure content type entry is expanded
    while (rContentTree.iter_next(*xIter) && lcl_IsContent(*xIter, rContentTree))
    {
        if (rName == rContentTree.get_text(*xIter))
        {
            // get first selected for comparison
            std::unique_ptr<weld::TreeIter> xFirstSelected(rContentTree.make_iterator());
            if (!rContentTree.get_selected(xFirstSelected.get()))
                xFirstSelected.reset();
            if (rContentTree.count_selected_rows() != 1 || !xFirstSelected ||
                    rContentTree.iter_compare(*xIter, *xFirstSelected) != 0)
            {
                // unselect all entries and make passed entry visible and selected
                rContentTree.set_cursor(*xIter);
                pThis->Select();
            }
            break;
        }
    }
}

static void lcl_SelectDrawObjectByName(weld::TreeView& rContentTree, std::u16string_view rName)
{
    if (rName.empty())
        return;

    // find content type entry
    std::unique_ptr<weld::TreeIter> xIter(rContentTree.make_iterator());
    bool bFoundEntry = rContentTree.get_iter_first(*xIter);
    while (bFoundEntry && SwResId(STR_CONTENT_TYPE_DRAWOBJECT) != rContentTree.get_text(*xIter))
        bFoundEntry = rContentTree.iter_next_sibling(*xIter);
    // find content type content entry and select it
    if (bFoundEntry)
    {
        rContentTree.expand_row(*xIter); // assure content type entry is expanded
        while (rContentTree.iter_next(*xIter) && lcl_IsContent(*xIter, rContentTree))
        {
            if (rName == rContentTree.get_text(*xIter))
            {
                if (!rContentTree.is_selected(*xIter))
                {
                    rContentTree.select(*xIter);
                    rContentTree.scroll_to_row(*xIter);
                }
                break;
            }
        }
    }
}

/** No idle with focus or while dragging */
IMPL_LINK_NOARG(SwContentTree, TimerUpdate, Timer *, void)
{
    // No need to update if content tree is not visible
    if (!m_xTreeView->is_visible())
        return;

    // No update while focus is not in document.
    // No update while drag and drop.
    // Query view because the Navigator is cleared too late.
    SwView* pView = GetParentWindow()->GetCreateView();

    SwWrtShell* pActShell = pView ? pView->GetWrtShellPtr() : nullptr;
    if(pActShell && pActShell->GetWin() &&
        (pActShell->GetWin()->HasFocus() || m_bDocHasChanged || m_bViewHasChanged) &&
        !IsInDrag() && !pActShell->ActionPend())
    {
        if (m_bDocHasChanged || m_bViewHasChanged)
        {
            if (State::CONSTANT == m_eState && !lcl_FindShell(m_pActiveShell))
            {
                SetActiveShell(pActShell);
                GetParentWindow()->UpdateListBox();
            }
            if (State::ACTIVE == m_eState && pActShell != GetWrtShell())
            {
                SetActiveShell(pActShell);
            }
            else if ((State::ACTIVE == m_eState || (State::CONSTANT == m_eState && pActShell == GetWrtShell())) &&
                        HasContentChanged())
            {
                FindActiveTypeAndRemoveUserData();
                Display(true);
            }
        }
        UpdateTracking();
        m_bIsIdleClear = false;
        m_bDocHasChanged = false;
        m_bViewHasChanged = false;
    }
    else if (!pView && State::ACTIVE == m_eState && !m_bIsIdleClear) // this block seems never to be entered
    {
        if(m_pActiveShell)
        {
            SetActiveShell(nullptr);
        }
        clear();
        m_bIsIdleClear = true;
    }
}

void SwContentTree::UpdateTracking()
{
    if (State::HIDDEN == m_eState || !m_pActiveShell)
        return;

    // only when treeview or treeview context menu does not have focus
    if (m_xTreeView->has_focus() || m_xTreeView->has_child_focus())
        return;

    // m_bIgnoreDocChange is set on delete and outline visibility toggle
    if (m_bIgnoreDocChange)
    {
        m_bIgnoreDocChange = false;
        return;
    }

    // bTrack is used to disallow tracking after jumping to an outline until the outline position
    // that was jumped to is no longer the current outline position.
    bool bTrack = true;
    if (m_nLastGotoContentWasOutlinePos != SwOutlineNodes::npos)
    {
        if (m_pActiveShell->GetOutlinePos() == m_nLastGotoContentWasOutlinePos)
            bTrack = false;
        else
            m_nLastGotoContentWasOutlinePos = SwOutlineNodes::npos;
    }

    if (bTrack)
    {
        // graphic, frame, and ole
        if (m_pActiveShell->GetSelectionType() &
                (SelectionType::Graphic | SelectionType::Frame | SelectionType::Ole))
        {
            OUString aContentTypeName;
            if (m_pActiveShell->GetSelectionType() == SelectionType::Graphic &&
                    !(m_bIsRoot && m_nRootType != ContentTypeId::GRAPHIC))
            {
                if (!mTrackContentType[ContentTypeId::GRAPHIC]) return;
                aContentTypeName = SwResId(STR_CONTENT_TYPE_GRAPHIC);
            }
            else if (m_pActiveShell->GetSelectionType() == SelectionType::Frame &&
                     !(m_bIsRoot && m_nRootType != ContentTypeId::FRAME))
            {
                if (!mTrackContentType[ContentTypeId::FRAME]) return;
                aContentTypeName = SwResId(STR_CONTENT_TYPE_FRAME);
            }
            else if (m_pActiveShell->GetSelectionType() == SelectionType::Ole &&
                     !(m_bIsRoot && m_nRootType != ContentTypeId::OLE))
            {
                if (!mTrackContentType[ContentTypeId::OLE]) return;
                aContentTypeName = SwResId(STR_CONTENT_TYPE_OLE);
            }
            if (!aContentTypeName.isEmpty())
            {
                OUString aName(m_pActiveShell->GetFlyName());
                lcl_SelectByContentTypeAndName(this, *m_xTreeView, aContentTypeName, aName);
                return;
            }
        }
        // drawing
        if ((m_pActiveShell->GetSelectionType() & (SelectionType::DrawObject |
                                                   SelectionType::DrawObjectEditMode |
                                                   SelectionType::DbForm)) &&
                !(m_bIsRoot && m_nRootType != ContentTypeId::DRAWOBJECT))
        {
            if (mTrackContentType[ContentTypeId::DRAWOBJECT])
            {
                // Multiple selection is possible when in root content navigation view so unselect all
                // selected entries before reselecting. This causes a bit of an annoyance when the treeview
                // scroll bar is used and focus is in the document by causing the last selected entry to
                // scroll back into view.
                if (m_bIsRoot)
                    m_xTreeView->unselect_all();
                SdrView* pSdrView = m_pActiveShell->GetDrawView();
                if (pSdrView)
                {
                    for (size_t nIdx(0); nIdx < pSdrView->GetMarkedObjectCount(); nIdx++)
                    {
                        SdrObject* pSelected = pSdrView->GetMarkedObjectByIndex(nIdx);
                        OUString aName(pSelected->GetName());
                        if (!aName.isEmpty())
                            lcl_SelectDrawObjectByName(*m_xTreeView, aName);
                    }
                }
                else
                {
                    // clear treeview selections
                    m_xTreeView->unselect_all();
                }
                Select();
            }
            return;
        }
        // footnotes and endnotes
        if (SwContentAtPos aContentAtPos(IsAttrAtPos::Ftn);
                m_pActiveShell->GetContentAtPos(m_pActiveShell->GetCursorDocPos(), aContentAtPos)
                && aContentAtPos.pFndTextAttr &&
                !(m_bIsRoot && (m_nRootType != ContentTypeId::FOOTNOTE &&
                                m_nRootType != ContentTypeId::ENDNOTE)))
        {
            if (!aContentAtPos.pFndTextAttr->GetFootnote().IsEndNote())
            {
                if (mTrackContentType[ContentTypeId::FOOTNOTE])
                    lcl_SelectByContentTypeAndAddress(this, *m_xTreeView, ContentTypeId::FOOTNOTE,
                                                      aContentAtPos.pFndTextAttr);
            }
            else if (mTrackContentType[ContentTypeId::ENDNOTE])
                lcl_SelectByContentTypeAndAddress(this, *m_xTreeView, ContentTypeId::ENDNOTE,
                                                  aContentAtPos.pFndTextAttr);
            return;
        }
        // bookmarks - track first bookmark at cursor
        if (mTrackContentType[ContentTypeId::BOOKMARK] &&
                (m_pActiveShell->GetSelectionType() & SelectionType::Text))
        {
            SwPaM* pCursor = m_pActiveShell->GetCursor();
            IDocumentMarkAccess* const pMarkAccess = m_pActiveShell->getIDocumentMarkAccess();
            IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
            if (pCursor && ppBookmark != pMarkAccess->getBookmarksEnd() &&
                    !(m_bIsRoot && m_nRootType != ContentTypeId::BOOKMARK))
            {
                OUString sBookmarkName;
                SwPosition* pCursorPoint = pCursor->GetPoint();
                while (ppBookmark != pMarkAccess->getBookmarksEnd())
                {
                    if (lcl_IsUiVisibleBookmark(*ppBookmark) &&
                            *pCursorPoint >= (*ppBookmark)->GetMarkStart() &&
                            *pCursorPoint <= (*ppBookmark)->GetMarkEnd())
                    {
                        sBookmarkName = (*ppBookmark)->GetName();
                        // keep previously selected bookmark instead
                        // of selecting a different bookmark inside of it
                        if (sBookmarkName == m_sSelectedItem)
                            break;
                    }
                    else if (!sBookmarkName.isEmpty() &&
                        *pCursorPoint < (*ppBookmark)->GetMarkStart())
                    {
                        // don't search a different bookmark inside the
                        // previous one, if the starting position of the next bookmarks
                        // is after the cursor position (assuming that the
                        // bookmark iterator jumps inside the same text by positions)
                        break;
                    }
                    ++ppBookmark;
                }

                if (!sBookmarkName.isEmpty())
                {
                    // select the bookmark
                    lcl_SelectByContentTypeAndName(this, *m_xTreeView,
                                                       SwResId(STR_CONTENT_TYPE_BOOKMARK),
                                                       sBookmarkName);
                    return;
                }
            }
        }
        // references
        if (SwContentAtPos aContentAtPos(IsAttrAtPos::RefMark);
                m_pActiveShell->GetContentAtPos(m_pActiveShell->GetCursorDocPos(), aContentAtPos) &&
                aContentAtPos.pFndTextAttr &&
                !(m_bIsRoot && m_nRootType != ContentTypeId::REFERENCE))
        {
            if (mTrackContentType[ContentTypeId::REFERENCE])
            {
                const SwFormatRefMark& rRefMark = aContentAtPos.pFndTextAttr->GetRefMark();
                lcl_SelectByContentTypeAndName(this, *m_xTreeView, SwResId(STR_CONTENT_TYPE_REFERENCE),
                                               rRefMark.GetRefName());
            }
            return;
        }
        // hyperlinks
        // not in ToxContent tdf#148312
        if (const SwSection* pSection = m_pActiveShell->GetCurrSection(); !pSection
            || (pSection && pSection->GetType() != SectionType::ToxContent))
        {
            if (SwContentAtPos aContentAtPos(IsAttrAtPos::InetAttr);
                m_pActiveShell->GetContentAtPos(m_pActiveShell->GetCursorDocPos(), aContentAtPos)
                && (!m_bIsRoot || m_nRootType == ContentTypeId::URLFIELD))
            {
                // Because hyperlink item names do not need to be unique, finding the corresponding
                // item in the tree by name may result in incorrect selection. Find the item in the
                // tree by comparing the SwTextINetFormat pointer at the document cursor position to
                // that stored in the item SwURLFieldContent.
                if (mTrackContentType[ContentTypeId::URLFIELD])
                    lcl_SelectByContentTypeAndAddress(this, *m_xTreeView, ContentTypeId::URLFIELD,
                                                      aContentAtPos.pFndTextAttr);
                return;
            }
        }
        // fields, comments
        if (SwField* pField = m_pActiveShell->GetCurField(); pField &&
                !(m_bIsRoot &&
                  m_nRootType != ContentTypeId::TEXTFIELD &&
                  m_nRootType != ContentTypeId::POSTIT))
        {
            ContentTypeId eCntTypeId =
                    pField->GetTypeId() == SwFieldTypesEnum::Postit ? ContentTypeId::POSTIT :
                                                                      ContentTypeId::TEXTFIELD;
            if (mTrackContentType[eCntTypeId])
                lcl_SelectByContentTypeAndAddress(this, *m_xTreeView, eCntTypeId, pField);
            return;
        }
        // table
        if (m_pActiveShell->IsCursorInTable() &&
                !(m_bIsRoot && m_nRootType != ContentTypeId::TABLE))
        {
            if (mTrackContentType[ContentTypeId::TABLE] && m_pActiveShell->GetTableFormat())
            {
                OUString aName = m_pActiveShell->GetTableFormat()->GetName();
                lcl_SelectByContentTypeAndName(this, *m_xTreeView, SwResId(STR_CONTENT_TYPE_TABLE),
                                               aName);
                return;
            }
        }
        // indexes
        if (const SwTOXBase* pTOX = m_pActiveShell->GetCurTOX(); pTOX &&
                !(m_bIsRoot && m_nRootType != ContentTypeId::INDEX))
        {
            if (mTrackContentType[ContentTypeId::INDEX])
                lcl_SelectByContentTypeAndName(this, *m_xTreeView, SwResId(STR_CONTENT_TYPE_INDEX),
                                               pTOX->GetTOXName());
            return;
        }
        // section
        if (const SwSection* pSection = m_pActiveShell->GetCurrSection(); pSection &&
                !(m_bIsRoot && m_nRootType != ContentTypeId::REGION))
        {
            if (mTrackContentType[ContentTypeId::REGION])
            {
                lcl_SelectByContentTypeAndName(this, *m_xTreeView, SwResId(STR_CONTENT_TYPE_REGION),
                                               pSection->GetSectionName());
                return;
            }
            else
            {
                // prevent fall through to outline tracking when section tracking is off and the last
                // GotoContent is the current section
                if (m_nLastSelType == ContentTypeId::REGION &&
                        m_xTreeView->get_selected_text() == pSection->GetSectionName())
                    return;
            }
            // fall through to outline tracking when section tracking is off and the last GotoContent
            // is not the current section
        }
    }
    // outline
    if (m_nOutlineTracking == 3)
        return;
    // find out where the cursor is
    const SwOutlineNodes::size_type nActPos = GetWrtShell()->GetOutlinePos(MAXLEVEL);
    if (!((m_bIsRoot && m_nRootType != ContentTypeId::OUTLINE) || nActPos == SwOutlineNodes::npos))
    {
        // assure outline content type is expanded
        // this assumes outline content type is first in treeview
        std::unique_ptr<weld::TreeIter> xFirstEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_iter_first(*xFirstEntry))
            m_xTreeView->expand_row(*xFirstEntry);

        m_xTreeView->all_foreach([this, nActPos](weld::TreeIter& rEntry){
            bool bRet = false;
            if (lcl_IsContent(rEntry, *m_xTreeView) && weld::fromId<SwContent*>(
                        m_xTreeView->get_id(rEntry))->GetParent()->GetType() ==
                    ContentTypeId::OUTLINE)
            {
                if (weld::fromId<SwOutlineContent*>(
                            m_xTreeView->get_id(rEntry))->GetOutlinePos() == nActPos)
                {
                    std::unique_ptr<weld::TreeIter> xFirstSelected(
                                m_xTreeView->make_iterator());
                    if (!m_xTreeView->get_selected(xFirstSelected.get()))
                        xFirstSelected.reset();
                    // only select if not already selected or tree has multiple entries selected
                    if (m_xTreeView->count_selected_rows() != 1 || !xFirstSelected ||
                            m_xTreeView->iter_compare(rEntry, *xFirstSelected) != 0)
                    {
                        if (m_nOutlineTracking == 2) // focused outline tracking
                        {
                            // collapse to children of root node
                            std::unique_ptr<weld::TreeIter> xChildEntry(
                                        m_xTreeView->make_iterator());
                            if (m_xTreeView->get_iter_first(*xChildEntry) &&
                                    m_xTreeView->iter_children(*xChildEntry))
                            {
                                do
                                {
                                    if (weld::fromId<SwContent*>(
                                                m_xTreeView->get_id(*xChildEntry))->
                                            GetParent()->GetType() == ContentTypeId::OUTLINE)
                                        m_xTreeView->collapse_row(*xChildEntry);
                                    else
                                        break;
                                }
                                while (m_xTreeView->iter_next(*xChildEntry));
                            }
                        }
                        // unselect all entries, make pEntry visible, and select
                        m_xTreeView->set_cursor(rEntry);
                        Select();

                        // tdf#149279 show at least two outline entries before the set cursor entry
                        std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator(&rEntry));
                        for (int i = 0; i < 2; i++)
                        {
                            if (m_xTreeView->get_iter_depth(*xIter) == 0)
                                break;
                            if (!m_xTreeView->iter_previous(*xIter))
                                break;
                            while (!weld::IsEntryVisible(*m_xTreeView, *xIter))
                                m_xTreeView->iter_parent(*xIter);
                        }
                        // Assure the scroll to row is collapsed after scrolling if it was collapsed
                        // before. This is required here to make gtkinst scroll_to_row behave like
                        // salinst.
                        const bool bRowExpanded = m_xTreeView->get_row_expanded(*xIter);
                        m_xTreeView->scroll_to_row(*xIter);
                        if (!bRowExpanded)
                            m_xTreeView->collapse_row(*xIter);
                    }
                    bRet = true;
                }
            }
            else
            {
                // use of this break assumes outline content type is first in tree
                if (lcl_IsContentType(rEntry, *m_xTreeView) &&
                        weld::fromId<SwContentType*>(
                            m_xTreeView->get_id(rEntry))->GetType() !=
                        ContentTypeId::OUTLINE)
                    bRet = true;
            }
            return bRet;
        });
    }
    else
    {
        // clear treeview selections
        if (m_xTreeView->count_selected_rows() > 0)
        {
            m_xTreeView->unselect_all();
            m_xTreeView->set_cursor(-1);
            Select();
        }
    }
}

void SwContentTree::SelectOutlinesWithSelection()
{
    SwCursor* pFirstCursor = m_pActiveShell->GetCursor();
    SwCursor* pCursor = pFirstCursor;
    std::vector<SwOutlineNodes::size_type> aOutlinePositions;
    do
    {
        if (pCursor)
        {
            if (pCursor->HasMark())
            {
                aOutlinePositions.push_back(m_pActiveShell->GetOutlinePos(UCHAR_MAX, pCursor));
            }
            pCursor = pCursor->GetNext();
        }
    } while (pCursor && pCursor != pFirstCursor);

    if (aOutlinePositions.empty())
        return;

    // remove duplicates before selecting
    aOutlinePositions.erase(std::unique(aOutlinePositions.begin(), aOutlinePositions.end()),
                            aOutlinePositions.end());

    m_xTreeView->unselect_all();

    for (auto nOutlinePosition : aOutlinePositions)
    {
        m_xTreeView->all_foreach([this, nOutlinePosition](const weld::TreeIter& rEntry){
            if (lcl_IsContent(rEntry, *m_xTreeView) &&
                    weld::fromId<SwContent*>(
                    m_xTreeView->get_id(rEntry))->GetParent()->GetType() ==
                    ContentTypeId::OUTLINE)
            {
                if (weld::fromId<SwOutlineContent*>(
                        m_xTreeView->get_id(rEntry))->GetOutlinePos() ==
                        nOutlinePosition)
                {
                    std::unique_ptr<weld::TreeIter> xParent =
                            m_xTreeView->make_iterator(&rEntry);
                    if (m_xTreeView->iter_parent(*xParent) &&
                            !m_xTreeView->get_row_expanded(*xParent))
                        m_xTreeView->expand_row(*xParent);
                    m_xTreeView->select(rEntry);
                    return true;
                }
            }
            return false;
        });
    }

    Select();
}

void SwContentTree::MoveOutline(SwOutlineNodes::size_type nTargetPos)
{
    MakeAllOutlineContentTemporarilyVisible a(GetWrtShell()->GetDoc());

    SwWrtShell *const pShell = GetWrtShell();
    pShell->StartAllAction();
    pShell->StartUndo(SwUndoId::OUTLINE_UD);

    SwOutlineNodes::size_type nPrevSourcePos = SwOutlineNodes::npos;
    SwOutlineNodes::size_type nPrevTargetPosOrOffset = SwOutlineNodes::npos;

    bool bFirstMove = true;

    for (const auto& source : m_aDndOutlinesSelected)
    {
        SwOutlineNodes::size_type nSourcePos = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(*source))->GetOutlinePos();

        // Done on the first selection move
        if (bFirstMove) // only do once
        {
            if (nTargetPos == SwOutlineNodes::npos || nSourcePos > nTargetPos)
            {
                // Up moves
                // The first up move sets the up move amount for the remaining selected outlines to be moved
                if (nTargetPos != SwOutlineNodes::npos)
                    nPrevTargetPosOrOffset = nSourcePos - nTargetPos;
                else
                    nPrevTargetPosOrOffset = nSourcePos + 1;
            }
            else if (nSourcePos < nTargetPos)
            {
                // Down moves
                // The first down move sets the source and target positions for the remaining selected outlines to be moved
                nPrevSourcePos = nSourcePos;
                nPrevTargetPosOrOffset = nTargetPos;
            }
            bFirstMove = false;
        }
        else
        {
            if (nTargetPos == SwOutlineNodes::npos || nSourcePos > nTargetPos)
            {
                // Move up
                nTargetPos = nSourcePos - nPrevTargetPosOrOffset;
            }
            else if (nSourcePos < nTargetPos)
            {
                // Move down
                nSourcePos = nPrevSourcePos;
                nTargetPos = nPrevTargetPosOrOffset;
            }
        }
        GetParentWindow()->MoveOutline(nSourcePos, nTargetPos);
    }

    pShell->EndUndo();
    pShell->EndAllAction();
    m_aActiveContentArr[ContentTypeId::OUTLINE]->Invalidate();
    Display(true);
    m_aDndOutlinesSelected.clear();
}

// Update immediately
IMPL_LINK_NOARG(SwContentTree, FocusInHdl, weld::Widget&, void)
{
    SwView* pActView = GetParentWindow()->GetCreateView();
    if(pActView)
    {
        SwWrtShell* pActShell = pActView->GetWrtShellPtr();
        if (State::CONSTANT == m_eState && !lcl_FindShell(m_pActiveShell))
        {
            SetActiveShell(pActShell);
        }

        if (State::ACTIVE == m_eState && pActShell != GetWrtShell())
            SetActiveShell(pActShell);
        // Only call HasContentChanged() if the document has changed since last called
        else if ((State::ACTIVE == m_eState || (State::CONSTANT == m_eState && pActShell == GetWrtShell())) &&
                    m_bDocHasChanged)
        {
            if (HasContentChanged())
                Display(true);
            m_bDocHasChanged = false;
        }
    }
    else if (State::ACTIVE == m_eState)
        clear();
}

IMPL_LINK(SwContentTree, KeyInputHdl, const KeyEvent&, rEvent, bool)
{
    bool bConsumed = true;

    const vcl::KeyCode aCode = rEvent.GetKeyCode();
    if (aCode.GetCode() == KEY_MULTIPLY && aCode.IsMod1())
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_selected(xEntry.get()))
            ExpandOrCollapseAll(*m_xTreeView, *xEntry);
    }
    else if (aCode.GetCode() == KEY_RETURN)
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_selected(xEntry.get()))
        {
            switch(aCode.GetModifier())
            {
                case KEY_MOD2:
                    // Switch boxes
                    GetParentWindow()->ToggleTree();
                break;
                case KEY_MOD1:
                    // Switch RootMode
                    ToggleToRoot();
                break;
                case 0:
                    if (lcl_IsContentType(*xEntry, *m_xTreeView))
                    {
                        m_xTreeView->get_row_expanded(*xEntry) ? m_xTreeView->collapse_row(*xEntry)
                                                               : m_xTreeView->expand_row(*xEntry);
                    }
                    else
                        ContentDoubleClickHdl(*m_xTreeView);
                break;
                case KEY_SHIFT:
                    m_bSelectTo = true;
                    ContentDoubleClickHdl(*m_xTreeView);
                break;
            }
        }
    }
    else if(aCode.GetCode() == KEY_DELETE && 0 == aCode.GetModifier())
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_selected(xEntry.get()) && lcl_IsContent(*xEntry, *m_xTreeView))
        {
            assert(dynamic_cast<SwContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xEntry))));
            if (weld::fromId<SwContent*>(m_xTreeView->get_id(*xEntry))->GetParent()->IsDeletable() &&
                    !m_pActiveShell->GetView().GetDocShell()->IsReadOnly())
            {
                EditEntry(*xEntry, EditEntryMode::DELETE);
            }
        }
    }
    //Make KEY_SPACE has same function as DoubleClick, and realize
    //multi-selection.
    else if (aCode.GetCode() == KEY_SPACE && 0 == aCode.GetModifier())
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_cursor(xEntry.get()))
        {
            if (State::HIDDEN != m_eState)
            {
                if (State::CONSTANT == m_eState)
                {
                    m_pActiveShell->GetView().GetViewFrame().GetWindow().ToTop();
                }

                SwContent* pCnt = dynamic_cast<SwContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xEntry)));

                if (pCnt && pCnt->GetParent()->GetType() == ContentTypeId::DRAWOBJECT)
                {
                    SdrView* pDrawView = m_pActiveShell->GetDrawView();
                    if (pDrawView)
                    {
                        pDrawView->SdrEndTextEdit();

                        SwDrawModel* pDrawModel = m_pActiveShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
                        SdrPage* pPage = pDrawModel->GetPage(0);
                        bool hasObjectMarked = false;

                        if (SdrObject* pObject = GetDrawingObjectsByContent(pCnt))
                        {
                            SdrPageView* pPV = pDrawView->GetSdrPageView/*GetPageViewPvNum*/(/*0*/);
                            if( pPV )
                            {
                                bool bUnMark = pDrawView->IsObjMarked(pObject);
                                pDrawView->MarkObj( pObject, pPV, bUnMark);

                            }
                        }
                        for (const rtl::Reference<SdrObject>& pTemp : *pPage)
                        {
                            bool bMark = pDrawView->IsObjMarked(pTemp.get());
                            switch( pTemp->GetObjIdentifier() )
                            {
                                case SdrObjKind::Group:
                                case SdrObjKind::Text:
                                case SdrObjKind::Line:
                                case SdrObjKind::Rectangle:
                                case SdrObjKind::CircleOrEllipse:
                                case SdrObjKind::CircleSection:
                                case SdrObjKind::CircleArc:
                                case SdrObjKind::CircleCut:
                                case SdrObjKind::Polygon:
                                case SdrObjKind::PolyLine:
                                case SdrObjKind::PathLine:
                                case SdrObjKind::PathFill:
                                case SdrObjKind::FreehandLine:
                                case SdrObjKind::FreehandFill:
                                case SdrObjKind::PathPoly:
                                case SdrObjKind::PathPolyLine:
                                case SdrObjKind::Caption:
                                case SdrObjKind::CustomShape:
                                    if( bMark )
                                        hasObjectMarked = true;
                                    break;
                                default:
                                    if ( bMark )
                                    {
                                        SdrPageView* pPV = pDrawView->GetSdrPageView/*GetPageViewPvNum*/(/*0*/);
                                        if (pPV)
                                        {
                                            pDrawView->MarkObj(pTemp.get(), pPV, true);
                                        }
                                    }
                            }
                            //mod end
                        }
                        if ( !hasObjectMarked )
                        {
                            SwEditWin& rEditWindow = m_pActiveShell->GetView().GetEditWin();
                            vcl::KeyCode tempKeycode( KEY_ESCAPE );
                            KeyEvent rKEvt( 0 , tempKeycode );
                            static_cast<vcl::Window*>(&rEditWindow)->KeyInput( rKEvt );
                        }
                    }
                }

                m_bViewHasChanged = true;
            }
        }
    }
    else
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_cursor(xEntry.get()))
        {
            SwContent* pCnt = dynamic_cast<SwContent*>(weld::fromId<SwTypeNumber*>(m_xTreeView->get_id(*xEntry)));
            if (pCnt && pCnt->GetParent()->GetType() == ContentTypeId::OUTLINE)
            {
                if (m_bIsRoot && aCode.GetCode() == KEY_LEFT && aCode.GetModifier() == 0)
                {
                    m_xTreeView->unselect_all();
                    bConsumed = false;
                }
                else if (aCode.IsMod1())
                {
                    if (aCode.GetCode() == KEY_LEFT)
                        ExecCommand(u"promote", !aCode.IsShift());
                    else if (aCode.GetCode() == KEY_RIGHT)
                        ExecCommand(u"demote", !aCode.IsShift());
                    else if (aCode.GetCode() == KEY_UP)
                        ExecCommand(u"chapterup", !aCode.IsShift());
                    else if (aCode.GetCode() == KEY_DOWN)
                        ExecCommand(u"chapterdown", !aCode.IsShift());
                    else if (aCode.GetCode() == KEY_C)
                        CopyOutlineSelections();
                    else
                        bConsumed = false;
                }
                else
                    bConsumed = false;
            }
            else
                bConsumed = false;
        }
        else
            bConsumed = false;
    }
    return bConsumed;
}

IMPL_LINK(SwContentTree, QueryTooltipHdl, const weld::TreeIter&, rEntry, OUString)
{
    ContentTypeId nType;
    bool bContent = false;
    void* pUserData = weld::fromId<void*>(m_xTreeView->get_id(rEntry));
    if (lcl_IsContentType(rEntry, *m_xTreeView))
    {
        assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pUserData)));
        nType = static_cast<SwContentType*>(pUserData)->GetType();
    }
    else
    {
        assert(dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pUserData)));
        nType = static_cast<SwContent*>(pUserData)->GetParent()->GetType();
        bContent = true;
    }
    OUString sEntry;
    if(bContent)
    {
        switch( nType )
        {
            case ContentTypeId::URLFIELD:
                assert(dynamic_cast<SwURLFieldContent*>(static_cast<SwTypeNumber*>(pUserData)));
                sEntry = static_cast<SwURLFieldContent*>(pUserData)->GetURL();
            break;

            case ContentTypeId::POSTIT:
                assert(dynamic_cast<SwPostItContent*>(static_cast<SwTypeNumber*>(pUserData)));
                sEntry = static_cast<SwPostItContent*>(pUserData)->GetName();
            break;
            case ContentTypeId::OUTLINE:
                assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pUserData)));
                sEntry = static_cast<SwOutlineContent*>(pUserData)->GetName();
            break;
            case ContentTypeId::GRAPHIC:
                assert(dynamic_cast<SwGraphicContent*>(static_cast<SwTypeNumber*>(pUserData)));
                sEntry = static_cast<SwGraphicContent*>(pUserData)->GetLink();
            break;
            case ContentTypeId::REGION:
            {
                assert(dynamic_cast<SwRegionContent*>(static_cast<SwTypeNumber*>(pUserData)));
                sEntry = static_cast<SwRegionContent*>(pUserData)->GetName();
                const SwSectionFormats& rFormats = GetWrtShell()->GetDoc()->GetSections();
                for (SwSectionFormats::size_type n = rFormats.size(); n;)
                {
                    const SwNodeIndex* pIdx = nullptr;
                    const SwSectionFormat* pFormat = rFormats[--n];
                    const SwSection* pSect;
                    if (nullptr != (pSect = pFormat->GetSection()) &&
                        pSect->GetSectionName() == sEntry &&
                        nullptr != (pIdx = pFormat->GetContent().GetContentIdx()) &&
                        pIdx->GetNode().GetNodes().IsDocNodes())
                    {
                        SwDocStat aDocStat;
                        SwPaM aPaM(pIdx->GetNode(), *pIdx->GetNode().EndOfSectionNode());
                        SwDoc::CountWords(aPaM, aDocStat);
                        sEntry = SwResId(STR_REGION_DEFNAME) + ": " + sEntry + "\n" +
                                 SwResId(FLD_STAT_WORD) + ": " + OUString::number(aDocStat.nWord) + "\n" +
                                 SwResId(FLD_STAT_CHAR) + ": " + OUString::number(aDocStat.nChar);
                        break;
                    }
                }
            }
            break;
            case ContentTypeId::FOOTNOTE:
            case ContentTypeId::ENDNOTE:
            {
                assert(dynamic_cast<SwTextFootnoteContent*>(static_cast<SwTypeNumber*>(pUserData)));
                const SwTextFootnote* pFootnote =
                        static_cast<const SwTextFootnoteContent*>(pUserData)->GetTextFootnote();

                sEntry = pFootnote->GetFootnote().IsEndNote() ? SwResId(STR_CONTENT_ENDNOTE) :
                                                                SwResId(STR_CONTENT_FOOTNOTE);
            }
            break;
            default: break;
        }
        if(static_cast<SwContent*>(pUserData)->IsInvisible())
        {
            if(!sEntry.isEmpty())
                sEntry += ", ";
            sEntry += m_sInvisible;
        }
    }
    else
    {
        size_t nMemberCount = static_cast<SwContentType*>(pUserData)->GetMemberCount();
        sEntry = OUString::number(nMemberCount) + " " +
            (nMemberCount == 1
                    ? static_cast<SwContentType*>(pUserData)->GetSingleName()
                    : static_cast<SwContentType*>(pUserData)->GetName());
    }

    return sEntry;
}

void SwContentTree::ExecuteContextMenuAction(const OUString& rSelectedPopupEntry)
{
    if (rSelectedPopupEntry == "copy")
    {
        CopyOutlineSelections();
        return;
    }
    if (rSelectedPopupEntry == "collapseallcategories")
    {
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
        bool bEntry = m_xTreeView->get_iter_first(*xEntry);
        while (bEntry)
        {
            m_xTreeView->collapse_row(*xEntry);
            bEntry = m_xTreeView->iter_next_sibling(*xEntry);
        }
        return;
    }

    {
        std::map<OUString, ContentTypeId> mPopupEntryToContentTypeId
        {
            {"tabletracking", ContentTypeId::TABLE},
            {"frametracking", ContentTypeId::FRAME},
            {"imagetracking", ContentTypeId::GRAPHIC},
            {"oleobjecttracking", ContentTypeId::OLE},
            {"bookmarktracking", ContentTypeId::BOOKMARK},
            {"sectiontracking", ContentTypeId::REGION},
            {"hyperlinktracking", ContentTypeId::URLFIELD},
            {"referencetracking", ContentTypeId::REFERENCE},
            {"indextracking", ContentTypeId::INDEX},
            {"commenttracking", ContentTypeId::POSTIT},
            {"drawingobjecttracking", ContentTypeId::DRAWOBJECT},
            {"fieldtracking", ContentTypeId::TEXTFIELD},
            {"footnotetracking", ContentTypeId::FOOTNOTE},
            {"endnotetracking", ContentTypeId::ENDNOTE}
        };

        if (mPopupEntryToContentTypeId.count(rSelectedPopupEntry))
        {
            ContentTypeId eCntTypeId = mPopupEntryToContentTypeId[rSelectedPopupEntry];
            SetContentTypeTracking(eCntTypeId, !mTrackContentType[eCntTypeId]);
            return;
        }
    }

    std::unique_ptr<weld::TreeIter> xFirst(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xFirst.get()))
        return; // this shouldn't happen, but better to be safe than ...

    if (rSelectedPopupEntry == "protectsection" || rSelectedPopupEntry == "hidesection")
    {
        SwRegionContent* pCnt = weld::fromId<SwRegionContent*>(m_xTreeView->get_id(*xFirst));
        assert(dynamic_cast<SwRegionContent*>(static_cast<SwTypeNumber*>(pCnt)));
        const SwSectionFormat* pSectionFormat = pCnt->GetSectionFormat();
        SwSection* pSection = pSectionFormat->GetSection();
        SwSectionData aSectionData(*pSection);
        if (rSelectedPopupEntry == "protectsection")
            aSectionData.SetProtectFlag(!pSection->IsProtect());
        else
            aSectionData.SetHidden(!pSection->IsHidden());
        m_pActiveShell->UpdateSection(m_pActiveShell->GetSectionFormatPos(*pSectionFormat),
                                      aSectionData);
    }
    else if (rSelectedPopupEntry == "sort")
    {
        SwContentType* pCntType;
        const OUString& rId(m_xTreeView->get_id(*xFirst));
        if (lcl_IsContentType(*xFirst, *m_xTreeView))
            pCntType = weld::fromId<SwContentType*>(rId);
        else
            pCntType = const_cast<SwContentType*>(weld::fromId<SwContent*>(rId)->GetParent());

        // toggle and persist alphabetical sort setting
        const int nShift = static_cast<int>(pCntType->GetType());
        assert(nShift > -1);
        const sal_Int32 nMask = 1 << nShift;
        const sal_Int32 nBlock = m_pConfig->GetSortAlphabeticallyBlock();
        pCntType->SetAlphabeticSort(~nBlock & nMask);
        m_pConfig->SetSortAlphabeticallyBlock(nBlock ^ nMask);

        pCntType->FillMemberList();
        Display(true);
        return;
    }
    else if (rSelectedPopupEntry == "deletechapter" ||
             rSelectedPopupEntry == "deletetable" ||
             rSelectedPopupEntry == "deleteframe" ||
             rSelectedPopupEntry == "deleteimage" ||
             rSelectedPopupEntry == "deleteoleobject" ||
             rSelectedPopupEntry == "deletebookmark" ||
             rSelectedPopupEntry == "deleteregion" ||
             rSelectedPopupEntry == "deletehyperlink" ||
             rSelectedPopupEntry == "deletereference" ||
             rSelectedPopupEntry == "deleteindex" ||
             rSelectedPopupEntry == "deletecomment" ||
             rSelectedPopupEntry == "deletedrawingobject" ||
             rSelectedPopupEntry == "deletefield")
    {
        EditEntry(*xFirst, EditEntryMode::DELETE);
        return;
    }

    auto nSelectedPopupEntry = rSelectedPopupEntry.toUInt32();
    switch (nSelectedPopupEntry)
    {
        case TOGGLE_OUTLINE_CONTENT_VISIBILITY:
        case HIDE_OUTLINE_CONTENT_VISIBILITY:
        case SHOW_OUTLINE_CONTENT_VISIBILITY:
        {
            m_pActiveShell->EnterStdMode();
            m_bIgnoreDocChange = true;
            SwOutlineContent* pCntFirst = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(*xFirst));

            // toggle the outline node outline content visible attribute
            if (nSelectedPopupEntry == TOGGLE_OUTLINE_CONTENT_VISIBILITY)
            {
                SwNode* pNode = m_pActiveShell->GetDoc()->GetNodes().GetOutLineNds()[pCntFirst->GetOutlinePos()];
                pNode->GetTextNode()->SetAttrOutlineContentVisible(
                            !m_pActiveShell->GetAttrOutlineContentVisible(pCntFirst->GetOutlinePos()));
            }
            else
            {
                // with subs
                SwOutlineNodes::size_type nPos = pCntFirst->GetOutlinePos();
                if (lcl_IsContentType(*xFirst, *m_xTreeView)) // Headings root entry
                    nPos = SwOutlineNodes::npos;
                SwOutlineNodes::size_type nOutlineNodesCount = m_pActiveShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
                int nLevel = -1;
                if (nPos != SwOutlineNodes::npos) // not root
                    nLevel = m_pActiveShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos);
                else
                    nPos = 0;
                bool bShow(nSelectedPopupEntry == SHOW_OUTLINE_CONTENT_VISIBILITY);
                do
                {
                    if (m_pActiveShell->IsOutlineContentVisible(nPos) != bShow)
                        m_pActiveShell->GetDoc()->GetNodes().GetOutLineNds()[nPos]->GetTextNode()->SetAttrOutlineContentVisible(bShow);
                } while (++nPos < nOutlineNodesCount
                         && (nLevel == -1 || m_pActiveShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos) > nLevel));
            }
            m_pActiveShell->InvalidateOutlineContentVisibility();
            // show in the document what was toggled
            if (lcl_IsContentType(*xFirst, *m_xTreeView)) // Headings root entry
                m_pActiveShell->GotoPage(1, true);
            else
                m_pActiveShell->GotoOutline(pCntFirst->GetOutlinePos());
            grab_focus();
            m_bIgnoreDocChange = false;
            m_pActiveShell->SetModified();
            m_pActiveShell->GetDoc()->GetDocShell()->Broadcast(SfxHint(SfxHintId::DocChanged));
        }
        break;
        case 11:
        case 12:
        case 13:
            nSelectedPopupEntry -= 10;
            if(m_nOutlineTracking != nSelectedPopupEntry)
                SetOutlineTracking(static_cast<sal_uInt8>(nSelectedPopupEntry));
        break;
        //Outlinelevel
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:
            nSelectedPopupEntry -= 100;
            if(m_nOutlineLevel != nSelectedPopupEntry )
                SetOutlineLevel(static_cast<sal_Int8>(nSelectedPopupEntry));
        break;
        case 201:
        case 202:
        case 203:
            GetParentWindow()->SetRegionDropMode(static_cast<RegionMode>(nSelectedPopupEntry - 201));
        break;
        case 401:
        case 402:
            EditEntry(*xFirst, nSelectedPopupEntry == 401 ? EditEntryMode::RMV_IDX : EditEntryMode::UPD_IDX);
        break;
        // Edit entry
        case 403:
            EditEntry(*xFirst, EditEntryMode::EDIT);
        break;
        case 404:
            EditEntry(*xFirst, EditEntryMode::UNPROTECT_TABLE);
        break;
        case 405 :
        {
            const SwTOXBase* pBase = weld::fromId<SwTOXBaseContent*>(m_xTreeView->get_id(*xFirst))
                                                                ->GetTOXBase();
            m_pActiveShell->SetTOXBaseReadonly(*pBase, !SwEditShell::IsTOXBaseReadonly(*pBase));
        }
        break;
        case 502 :
            EditEntry(*xFirst, EditEntryMode::RENAME);
        break;
        case 600:
            m_pActiveShell->GetView().GetPostItMgr()->Show();
            break;
        case 601:
            m_pActiveShell->GetView().GetPostItMgr()->Hide();
            break;
        case 602:
            {
                m_pActiveShell->GetView().GetPostItMgr()->SetActiveSidebarWin(nullptr);
                m_pActiveShell->GetView().GetPostItMgr()->Delete();
                break;
            }
        case 700:
            {
                m_pActiveShell->GetView().GetViewFrame().GetDispatcher()->Execute(FN_OUTLINE_TO_CLIPBOARD);
                break;
            }
        case 800:
            ExpandOrCollapseAll(*m_xTreeView, *xFirst);
            break;
        case 801:
            ExecCommand(u"chapterup", true);
            break;
        case 802:
            ExecCommand(u"chapterdown", true);
            break;
        case 803:
            ExecCommand(u"promote", true);
            break;
        case 804:
            ExecCommand(u"demote", true);
            break;
        case 805: // select document content
        {
            m_pActiveShell->KillPams();
            m_pActiveShell->ClearMark();
            m_pActiveShell->EnterAddMode();
            SwContent* pCnt = weld::fromId<SwContent*>(m_xTreeView->get_id(*xFirst));
            const ContentTypeId eTypeId = pCnt->GetParent()->GetType();
            if (eTypeId == ContentTypeId::OUTLINE)
            {
                SwOutlineNodes::size_type nActPos = weld::fromId<SwOutlineContent*>(
                            m_xTreeView->get_id(*xFirst))->GetOutlinePos();
                m_pActiveShell->GotoOutline(nActPos);
                m_xTreeView->selected_foreach([this](weld::TreeIter& rEntry){
                    SwOutlineNodes::size_type nPos = weld::fromId<SwOutlineContent*>(
                                m_xTreeView->get_id(rEntry))->GetOutlinePos();
                    m_pActiveShell->SttSelect();
                    // select children if not expanded and don't kill PaMs
                    m_pActiveShell->MakeOutlineSel(nPos, nPos,
                                                   !m_xTreeView->get_row_expanded(rEntry), false);
                    m_pActiveShell->EndSelect();
                    return false;
                });
            }
            else if (eTypeId == ContentTypeId::TABLE)
            {
                m_pActiveShell->GotoTable(pCnt->GetName());
                m_pActiveShell->GetView().GetViewFrame().GetDispatcher()->Execute(FN_TABLE_SELECT_ALL);
            }
            else if (eTypeId == ContentTypeId::REGION)
            {
                m_pActiveShell->EnterStdMode();
                m_pActiveShell->GotoRegion(pCnt->GetName());
                GotoCurrRegionAndSkip(m_pActiveShell->GetCurrentShellCursor(), fnRegionEnd, m_pActiveShell->IsReadOnlyAvailable());
                m_pActiveShell->SttSelect();
                GotoCurrRegionAndSkip(m_pActiveShell->GetCurrentShellCursor(), fnRegionStart, m_pActiveShell->IsReadOnlyAvailable());
                m_pActiveShell->EndSelect();
                m_pActiveShell->UpdateCursor();
            }
            m_pActiveShell->LeaveAddMode();
        }
        break;
        case 900:
        {
            SwContent* pCnt = weld::fromId<SwContent*>(m_xTreeView->get_id(*xFirst));
            GotoContent(pCnt);
        }
        break;
        //Display
        default:
        if(nSelectedPopupEntry > 300 && nSelectedPopupEntry < 400)
        {
            nSelectedPopupEntry -= 300;
            SwView *pView = SwModule::GetFirstView();
            while (pView)
            {
                nSelectedPopupEntry --;
                if(nSelectedPopupEntry == 0)
                {
                    SetConstantShell(&pView->GetWrtShell());
                    break;
                }
                pView = SwModule::GetNextView(pView);
            }
            if(nSelectedPopupEntry)
            {
                m_bViewHasChanged = nSelectedPopupEntry == 1;
                m_eState = (nSelectedPopupEntry == 1) ? State::ACTIVE : State::HIDDEN;
                Display(nSelectedPopupEntry == 1);
            }
            GetParentWindow()->UpdateListBox();
        }
    }
}

void SwContentTree::DeleteOutlineSelections()
{
    const SwOutlineNodes& rOutlineNodes = m_pActiveShell->GetNodes().GetOutLineNds();
    auto nChapters(0);

    m_pActiveShell->StartAction();

    m_pActiveShell->EnterAddMode();
    m_xTreeView->selected_foreach([this, &rOutlineNodes, &nChapters](weld::TreeIter& rEntry){
        ++nChapters;
        if (m_xTreeView->iter_has_child(rEntry) &&
            !m_xTreeView->get_row_expanded(rEntry)) // only count children if not expanded
        {
            nChapters += m_xTreeView->iter_n_children(rEntry);
        }
        SwOutlineNodes::size_type nActPos = weld::fromId<SwOutlineContent*>(m_xTreeView->get_id(rEntry))->GetOutlinePos();
        if (m_pActiveShell->GetViewOptions()->IsShowOutlineContentVisibilityButton())
        {
            // make folded content visible so it can be selected
            if (!m_pActiveShell->IsOutlineContentVisible(nActPos))
                m_pActiveShell->MakeOutlineContentVisible(nActPos);
            if (!m_xTreeView->get_row_expanded(rEntry))
            {
                // include children
                SwNode* pNode = rOutlineNodes[nActPos];
                const int nLevel = pNode->GetTextNode()->GetAttrOutlineLevel() - 1;
                for (auto nPos = nActPos + 1; nPos < rOutlineNodes.size(); ++nPos)
                {
                    pNode = rOutlineNodes[nPos];
                    const int nNextLevel = pNode->GetTextNode()->GetAttrOutlineLevel() - 1;
                    if (nNextLevel <= nLevel)
                        break;
                    if (!m_pActiveShell->IsOutlineContentVisible(nNextLevel))
                        m_pActiveShell->MakeOutlineContentVisible(nNextLevel);
                }
            }
        }
        m_pActiveShell->SttSelect();
        m_pActiveShell->MakeOutlineSel(nActPos, nActPos, !m_xTreeView->get_row_expanded(rEntry), false); // select children if not expanded
        // The outline selection may already be to the start of the following outline paragraph
        // as happens when a table is the last content of the to be deleted outline. In this case
        // do not extend the to be deleted selection right or the first character of the following
        // outline paragraph will be removed. Also check if no selection was made which indicates
        // an empty paragraph and selection right is needed.
        if (!m_pActiveShell->IsSttPara() || !m_pActiveShell->HasSelection())
            m_pActiveShell->Right(SwCursorSkipMode::Chars, true, 1, false);
        m_pActiveShell->EndSelect();
        return false;
    });
    m_pActiveShell->LeaveAddMode();

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_CHAPTERS, nChapters));
    m_pActiveShell->StartUndo(SwUndoId::DELETE, &aRewriter);
    m_pActiveShell->Delete(false);
    m_pActiveShell->EndUndo();

    m_pActiveShell->EndAction();
}

void SwContentTree::SetOutlineLevel(sal_uInt8 nSet)
{
    if (nSet == m_nOutlineLevel)
        return;
    m_nOutlineLevel = nSet;
    m_pConfig->SetOutlineLevel( m_nOutlineLevel );
    std::unique_ptr<SwContentType>& rpContentT = (State::ACTIVE == m_eState)
            ? m_aActiveContentArr[ContentTypeId::OUTLINE]
            : m_aHiddenContentArr[ContentTypeId::OUTLINE];
    if(rpContentT)
    {
        rpContentT->SetOutlineLevel(m_nOutlineLevel);
        rpContentT->FillMemberList();
    }
    Display(State::ACTIVE == m_eState);
}

void SwContentTree::SetOutlineTracking(sal_uInt8 nSet)
{
    m_nOutlineTracking = nSet;
    m_pConfig->SetOutlineTracking(m_nOutlineTracking);
}

void SwContentTree::SetContentTypeTracking(ContentTypeId eCntTypeId, bool bSet)
{
    mTrackContentType[eCntTypeId] = bSet;
    m_pConfig->SetContentTypeTrack(eCntTypeId, bSet);
}

// Mode Change: Show dropped Doc
void SwContentTree::ShowHiddenShell()
{
    if(m_pHiddenShell)
    {
        m_eState = State::HIDDEN;
        Display(false);
    }
}

// Mode Change: Show active view
void SwContentTree::ShowActualView()
{
    m_eState = State::ACTIVE;
    Display(true);
    GetParentWindow()->UpdateListBox();
}

IMPL_LINK_NOARG(SwContentTree, SelectHdl, weld::TreeView&, void)
{
    if (m_pConfig->IsNavigateOnSelect())
    {
        ContentDoubleClickHdl(*m_xTreeView);
        grab_focus();
    }
    Select();
    if (m_bIsRoot)
        return;
    // Select the content type in the Navigate By control
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xEntry.get()))
        return;
    while (m_xTreeView->get_iter_depth(*xEntry))
        m_xTreeView->iter_parent(*xEntry);
    m_pDialog->SelectNavigateByContentType(m_xTreeView->get_text(*xEntry));
}

// Here the buttons for moving outlines are en-/disabled.
void SwContentTree::Select()
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xEntry.get()))
        return;

    bool bEnable = false;
    std::unique_ptr<weld::TreeIter> xParentEntry(m_xTreeView->make_iterator(xEntry.get()));
    bool bParentEntry = m_xTreeView->iter_parent(*xParentEntry);
    while (bParentEntry && (!lcl_IsContentType(*xParentEntry, *m_xTreeView)))
        bParentEntry = m_xTreeView->iter_parent(*xParentEntry);
    if (!m_bIsLastReadOnly)
    {
        if (!m_xTreeView->get_visible())
            bEnable = true;
        else if (bParentEntry)
        {
            if ((m_bIsRoot && m_nRootType == ContentTypeId::OUTLINE) ||
                (lcl_IsContent(*xEntry, *m_xTreeView) &&
                    weld::fromId<SwContentType*>(m_xTreeView->get_id(*xParentEntry))->GetType() == ContentTypeId::OUTLINE))
            {
                bEnable = true;
            }
        }
    }

    SwNavigationPI* pNavi = GetParentWindow();
    pNavi->m_xContent6ToolBox->set_item_sensitive("chapterup",  bEnable);
    pNavi->m_xContent6ToolBox->set_item_sensitive("chapterdown", bEnable);
    pNavi->m_xContent6ToolBox->set_item_sensitive("promote", bEnable);
    pNavi->m_xContent6ToolBox->set_item_sensitive("demote", bEnable);
}

void SwContentTree::SetRootType(ContentTypeId nType)
{
    m_nRootType = nType;
    m_bIsRoot = true;
    m_pConfig->SetRootType( m_nRootType );
}

OUString SwContentType::RemoveNewline(const OUString& rEntry)
{
    if (rEntry.isEmpty())
        return rEntry;

    OUStringBuffer aEntry(rEntry);
    for (sal_Int32 i = 0; i < rEntry.getLength(); ++i)
        if(aEntry[i] == 10 || aEntry[i] == 13)
            aEntry[i] = 0x20;

    return aEntry.makeStringAndClear();
}

void SwContentTree::EditEntry(const weld::TreeIter& rEntry, EditEntryMode nMode)
{
    SwContent* pCnt = weld::fromId<SwContent*>(m_xTreeView->get_id(rEntry));
    GotoContent(pCnt);
    const ContentTypeId nType = pCnt->GetParent()->GetType();
    sal_uInt16 nSlot = 0;

    if(EditEntryMode::DELETE == nMode)
        m_bIgnoreDocChange = true;

    uno::Reference< container::XNameAccess >  xNameAccess, xSecond, xThird;
    switch(nType)
    {
        case ContentTypeId::OUTLINE :
            if(nMode == EditEntryMode::DELETE)
            {
                DeleteOutlineSelections();
            }
        break;

        case ContentTypeId::TABLE     :
            if(nMode == EditEntryMode::UNPROTECT_TABLE)
            {
                m_pActiveShell->GetView().GetDocShell()->
                        GetDoc()->UnProtectCells( pCnt->GetName());
            }
            else if(nMode == EditEntryMode::DELETE)
            {
                m_pActiveShell->StartAction();
                OUString sTable = SwResId(STR_TABLE_NAME);
                SwRewriter aRewriterTableName;
                aRewriterTableName.AddRule(UndoArg1, SwResId(STR_START_QUOTE));
                aRewriterTableName.AddRule(UndoArg2, pCnt->GetName());
                aRewriterTableName.AddRule(UndoArg3, SwResId(STR_END_QUOTE));
                sTable = aRewriterTableName.Apply(sTable);

                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, sTable);
                m_pActiveShell->StartUndo(SwUndoId::DELETE, &aRewriter);
                m_pActiveShell->GetView().GetViewFrame().GetDispatcher()->Execute(FN_TABLE_SELECT_ALL);
                m_pActiveShell->DeleteRow();
                m_pActiveShell->EndUndo();
                m_pActiveShell->EndAction();
            }
            else if(nMode == EditEntryMode::RENAME)
            {
                uno::Reference< frame::XModel >  xModel = m_pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
                xNameAccess = xTables->getTextTables();
            }
            else
                nSlot = FN_FORMAT_TABLE_DLG;
        break;

        case ContentTypeId::GRAPHIC   :
            if(nMode == EditEntryMode::DELETE)
            {
                m_pActiveShell->DelRight();
            }
            else if(nMode == EditEntryMode::RENAME)
            {
                uno::Reference< frame::XModel >  xModel = m_pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
                xNameAccess = xGraphics->getGraphicObjects();
                uno::Reference< text::XTextFramesSupplier >  xFrames(xModel, uno::UNO_QUERY);
                xSecond = xFrames->getTextFrames();
                uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
                xThird = xObjs->getEmbeddedObjects();
            }
            else
                nSlot = FN_FORMAT_GRAFIC_DLG;
        break;

        case ContentTypeId::FRAME     :
        case ContentTypeId::OLE       :
            if(nMode == EditEntryMode::DELETE)
            {
                m_pActiveShell->DelRight();
            }
            else if(nMode == EditEntryMode::RENAME)
            {
                uno::Reference< frame::XModel >  xModel = m_pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextFramesSupplier >  xFrames(xModel, uno::UNO_QUERY);
                uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
                if(ContentTypeId::FRAME == nType)
                {
                    xNameAccess = xFrames->getTextFrames();
                    xSecond = xObjs->getEmbeddedObjects();
                }
                else
                {
                    xNameAccess = xObjs->getEmbeddedObjects();
                    xSecond = xFrames->getTextFrames();
                }
                uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
                xThird = xGraphics->getGraphicObjects();
            }
            else
                nSlot = FN_FORMAT_FRAME_DLG;
        break;
        case ContentTypeId::BOOKMARK  :
            if(nMode == EditEntryMode::DELETE)
            {
                assert(!m_pActiveShell->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_BOOKMARKS));
                IDocumentMarkAccess* const pMarkAccess = m_pActiveShell->getIDocumentMarkAccess();
                pMarkAccess->deleteMark(pMarkAccess->findMark(pCnt->GetName()), false);
            }
            else if(nMode == EditEntryMode::RENAME)
            {
                assert(!m_pActiveShell->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_BOOKMARKS));
                uno::Reference< frame::XModel >  xModel = m_pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XBookmarksSupplier >  xBkms(xModel, uno::UNO_QUERY);
                xNameAccess = xBkms->getBookmarks();
            }
            else
            {
                // allowed despite PROTECT_BOOKMARKS: the dialog itself enforces it
                SfxStringItem const name(FN_EDIT_BOOKMARK, pCnt->GetName());
                SfxPoolItem const* args[2] = { &name, nullptr };
                m_pActiveShell->GetView().GetViewFrame().
                    GetDispatcher()->Execute(FN_EDIT_BOOKMARK, SfxCallMode::SYNCHRON, args);
            }
        break;

        case ContentTypeId::REGION    :
            if (nMode == EditEntryMode::DELETE)
            {
                assert(dynamic_cast<SwRegionContent*>(static_cast<SwTypeNumber*>(pCnt)));
                const SwSectionFormat* pSectionFormat
                        = static_cast<SwRegionContent*>(pCnt)->GetSectionFormat();
                m_pActiveShell->GetDoc()->DelSectionFormat(
                            const_cast<SwSectionFormat*>(pSectionFormat), false);
            }
            else if (nMode == EditEntryMode::RENAME)
            {
                uno::Reference< frame::XModel >  xModel = m_pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextSectionsSupplier >  xSects(xModel, uno::UNO_QUERY);
                xNameAccess = xSects->getTextSections();
            }
            else
                nSlot = FN_EDIT_REGION;
        break;

        case ContentTypeId::URLFIELD:
            if (nMode == EditEntryMode::DELETE)
                nSlot = SID_REMOVE_HYPERLINK;
            else
                nSlot = SID_EDIT_HYPERLINK;
        break;
        case ContentTypeId::REFERENCE:
        {
            if(nMode == EditEntryMode::DELETE)
            {
                const OUString& rName = pCnt->GetName();
                for (const SfxPoolItem* pItem :
                     m_pActiveShell->GetDoc()->GetAttrPool().GetItemSurrogates(RES_TXTATR_REFMARK))
                {
                    assert(dynamic_cast<const SwFormatRefMark*>(pItem));
                    const auto pFormatRefMark = static_cast<const SwFormatRefMark*>(pItem);
                    if (!pFormatRefMark)
                        continue;
                    const SwTextRefMark* pTextRef = pFormatRefMark->GetTextRefMark();
                    if (pTextRef && &pTextRef->GetTextNode().GetNodes() ==
                            &m_pActiveShell->GetNodes() && rName == pFormatRefMark->GetRefName())
                    {
                        m_pActiveShell->GetDoc()->DeleteFormatRefMark(pFormatRefMark);
                        m_pActiveShell->SwViewShell::UpdateFields();
                        break;
                    }
                }
            }
        }
        break;
        case ContentTypeId::TEXTFIELD:
        {
            if (nMode == EditEntryMode::DELETE)
            {
                const SwTextFieldContent* pTextFieldCnt =
                        static_cast<const SwTextFieldContent*>(pCnt);
                const SwTextField* pTextField = pTextFieldCnt->GetFormatField()->GetTextField();
                SwTextField::DeleteTextField(*pTextField);
            }
            else
                nSlot = FN_EDIT_FIELD;
        }
        break;
        case ContentTypeId::POSTIT:
        {
            auto& rView = m_pActiveShell->GetView();
            auto pPostItMgr = rView.GetPostItMgr();
            pPostItMgr->AssureStdModeAtShell();
            pPostItMgr->SetActiveSidebarWin(nullptr);
            rView.GetEditWin().GrabFocus();
            if(nMode == EditEntryMode::DELETE)
                m_pActiveShell->DelRight();
            else
                nSlot = FN_POSTIT;
        }
        break;
        case ContentTypeId::INDEX:
        {
            const SwTOXBase* pBase = static_cast<SwTOXBaseContent*>(pCnt)->GetTOXBase();
            switch(nMode)
            {
                case EditEntryMode::EDIT:
                    if(pBase)
                    {
                        SwPtrItem aPtrItem( FN_INSERT_MULTI_TOX, const_cast<SwTOXBase *>(pBase));
                        m_pActiveShell->GetView().GetViewFrame().
                            GetDispatcher()->ExecuteList(FN_INSERT_MULTI_TOX,
                                SfxCallMode::ASYNCHRON, { &aPtrItem });

                    }
                break;
                case EditEntryMode::RMV_IDX:
                case EditEntryMode::DELETE:
                {
                    if( pBase )
                        m_pActiveShell->DeleteTOX(*pBase, EditEntryMode::DELETE == nMode);
                }
                break;
                case EditEntryMode::UPD_IDX:
                case EditEntryMode::RENAME:
                {
                    Reference< frame::XModel >  xModel = m_pActiveShell->GetView().GetDocShell()->GetBaseModel();
                    Reference< XDocumentIndexesSupplier >  xIndexes(xModel, UNO_QUERY);
                    Reference< XIndexAccess> xIdxAcc(xIndexes->getDocumentIndexes());
                    Reference< XNameAccess >xLocalNameAccess(xIdxAcc, UNO_QUERY);
                    if(EditEntryMode::RENAME == nMode)
                        xNameAccess = xLocalNameAccess;
                    else if(xLocalNameAccess.is() && xLocalNameAccess->hasByName(pBase->GetTOXName()))
                    {
                        Any aIdx = xLocalNameAccess->getByName(pBase->GetTOXName());
                        Reference< XDocumentIndex> xIdx;
                        if(aIdx >>= xIdx)
                            xIdx->update();
                    }
                }
                break;
                default: break;
            }
        }
        break;
        case ContentTypeId::DRAWOBJECT :
            if(EditEntryMode::DELETE == nMode)
                nSlot = SID_DELETE;
            else if(nMode == EditEntryMode::RENAME)
                nSlot = FN_NAME_SHAPE;
            else if (nMode == EditEntryMode::EDIT)
            {
                vcl::KeyCode aKeyCode(KEY_RETURN, false, false, false, false);
                KeyEvent aKeyEvent(0, aKeyCode);
                m_pActiveShell->GetWin()->KeyInput(aKeyEvent);
            }
        break;
        case ContentTypeId::FOOTNOTE:
        case ContentTypeId::ENDNOTE:
            if (EditEntryMode::EDIT == nMode)
                nSlot = FN_FORMAT_FOOTNOTE_DLG;
        break;
        default: break;
    }
    if(nSlot)
        m_pActiveShell->GetView().GetViewFrame().
                    GetDispatcher()->Execute(nSlot, SfxCallMode::SYNCHRON);
    else if(xNameAccess.is())
    {
        uno::Any aObj = xNameAccess->getByName(pCnt->GetName());
        uno::Reference< uno::XInterface >  xTmp;
        aObj >>= xTmp;
        uno::Reference< container::XNamed >  xNamed(xTmp, uno::UNO_QUERY);
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSwRenameXNamedDlg> pDlg(pFact->CreateSwRenameXNamedDlg(m_xTreeView.get(), xNamed, xNameAccess));
        if(xSecond.is())
            pDlg->SetAlternativeAccess( xSecond, xThird);

        OUString sForbiddenChars;
        if(ContentTypeId::BOOKMARK == nType)
        {
            sForbiddenChars = "/\\@:*?\";,.#";
        }
        else if(ContentTypeId::TABLE == nType)
        {
            sForbiddenChars = " .<>";
        }
        pDlg->SetForbiddenChars(sForbiddenChars);
        pDlg->Execute();
    }
    if(EditEntryMode::DELETE == nMode)
    {
        auto nPos = m_xTreeView->vadjustment_get_value();
        m_bViewHasChanged = true;
        TimerUpdate(&m_aUpdTimer);
        grab_focus();
        m_xTreeView->vadjustment_set_value(nPos);
    }
}

void SwContentTree::CopyOutlineSelections()
{
    m_pActiveShell->LockView(true);
    {
        MakeAllOutlineContentTemporarilyVisible a(m_pActiveShell->GetDoc());
        m_pActiveShell->AssureStdMode();
        m_pActiveShell->EnterAddMode();
        size_t nCount = m_xTreeView->get_selected_rows().size();
        m_xTreeView->selected_foreach([this, &nCount](weld::TreeIter& rEntry){
            SwOutlineNodes::size_type nOutlinePos = reinterpret_cast<SwOutlineContent*>(
                        m_xTreeView->get_id(rEntry).toInt64())->GetOutlinePos();
            m_pActiveShell->SttSelect();
            m_pActiveShell->MakeOutlineSel(nOutlinePos, nOutlinePos,
                                           !m_xTreeView->get_row_expanded(rEntry), false);
            // don't move if this is the last selected outline or the cursor is at start of para
            if (--nCount && !m_pActiveShell->IsSttPara())
                m_pActiveShell->Right(SwCursorSkipMode::Chars, true, 1, false);
            m_pActiveShell->EndSelect();
            return false;
        });
        m_pActiveShell->LeaveAddMode();
        m_pActiveShell->GetView().GetViewFrame().GetBindings().Execute(SID_COPY);
    }
    m_pActiveShell->LockView(false);
}

void SwContentTree::GotoContent(const SwContent* pCnt)
{
    if (pCnt->GetParent()->GetType() == ContentTypeId::OUTLINE)
    {
        // Maybe the outline node doesn't have a layout frame to go to.
        const SwOutlineNodes::size_type nPos =
                static_cast<const SwOutlineContent*>(pCnt)->GetOutlinePos();
        const SwNodes& rNds = m_pActiveShell->GetDoc()->GetNodes();
        SwTextNode* pTextNd = rNds.GetOutLineNds()[nPos]->GetTextNode();
        if (!pTextNd->getLayoutFrame(m_pActiveShell->GetLayout()))
            return;
    }

    if (m_bSelectTo)
    {
        if (m_pActiveShell->IsCursorInTable() ||
                (m_pActiveShell->GetCursor()->GetPoint()->nNode.GetIndex() <=
                 m_pActiveShell->GetDoc()->GetNodes().GetEndOfExtras().GetIndex()))
        {
            m_bSelectTo = false;
            m_pActiveShell->GetView().GetEditWin().GrabFocus();
            return;
        }
    }

    m_nLastGotoContentWasOutlinePos = SwOutlineNodes::npos;
    m_sSelectedItem = "";

    m_pActiveShell->AssureStdMode();

    std::optional<std::unique_ptr<SwPosition>> oPosition;
    if (m_bSelectTo)
        oPosition.emplace(new SwPosition(m_pActiveShell->GetCursor()->GetPoint()->nNode,
                                         m_pActiveShell->GetCursor()->GetPoint()->nContent));

    switch(m_nLastSelType = pCnt->GetParent()->GetType())
    {
        case ContentTypeId::TEXTFIELD:
        {
            m_pActiveShell->GotoFormatField(
                        *static_cast<const SwTextFieldContent*>(pCnt)->GetFormatField());
        }
        break;
        case ContentTypeId::OUTLINE   :
        {
            const SwOutlineNodes::size_type nPos =
                    static_cast<const SwOutlineContent*>(pCnt)->GetOutlinePos();
            m_pActiveShell->GotoOutline(nPos);
            m_nLastGotoContentWasOutlinePos = nPos;
        }
        break;
        case ContentTypeId::TABLE     :
        {
            m_pActiveShell->GotoTable(pCnt->GetName());
        }
        break;
        case ContentTypeId::FRAME     :
        case ContentTypeId::GRAPHIC   :
        case ContentTypeId::OLE       :
        {
            m_pActiveShell->GotoFly(pCnt->GetName());
        }
        break;
        case ContentTypeId::BOOKMARK:
        {
            m_pActiveShell->StartAction();
            m_pActiveShell->GotoMark(pCnt->GetName());
            m_pActiveShell->EndAction();
            m_sSelectedItem = pCnt->GetName();

            // If the hidden title of SwNavigatorPanel was emptied via UNO XPanel interface,
            // store the name of the selected bookmark there. This allows to query the
            // selected bookmark using UNO e.g. in add-ons, i.e. to disambiguate when
            // multiple bookmarks are there on the selected text range.
            // Note: this is a workaround because getDialog() of XPanel is not implemented
            // for SwNavigatorPanel.
            uno::Reference< frame::XModel > xModel = m_pActiveShell->GetView().GetDocShell()->GetBaseModel();

            Reference<frame::XController2> xController( xModel->getCurrentController(), uno::UNO_QUERY);
            if ( !xController.is() )
                break;

            Reference<ui::XSidebarProvider> xSidebarProvider = xController->getSidebar();
            if ( !xSidebarProvider.is() )
                break;

            Reference<ui::XDecks> xDecks = xSidebarProvider->getDecks();
            if ( !xDecks.is() )
                break;

            if (!xDecks->hasByName("NavigatorDeck"))
                break;

            Reference<ui::XDeck> xDeck ( xDecks->getByName("NavigatorDeck"), uno::UNO_QUERY);
            if ( !xDeck.is() )
                break;

            Reference<ui::XPanels> xPanels = xDeck->getPanels();
            if ( !xPanels.is() )
                break;

            if (xPanels->hasByName("SwNavigatorPanel"))
            {
                Reference<ui::XPanel> xPanel ( xPanels->getByName("SwNavigatorPanel"), uno::UNO_QUERY);
                if ( !xPanel.is() || !xPanel->getTitle().isEmpty() )
                    break;

                xPanel->setTitle( pCnt->GetName() );
            }
        }
        break;
        case ContentTypeId::REGION    :
        {
            m_pActiveShell->GotoRegion(pCnt->GetName());
        }
        break;
        case ContentTypeId::URLFIELD:
        {
            if(m_pActiveShell->GotoINetAttr(
                            *static_cast<const SwURLFieldContent*>(pCnt)->GetINetAttr() ))
            {
                m_pActiveShell->Right( SwCursorSkipMode::Chars, true, 1, false);
                m_pActiveShell->SwCursorShell::SelectTextAttr( RES_TXTATR_INETFMT, true );
            }
        }
        break;
        case ContentTypeId::REFERENCE:
        {
            m_pActiveShell->GotoRefMark(pCnt->GetName());
        }
        break;
        case ContentTypeId::INDEX:
        {
            const OUString& sName(pCnt->GetName());
            if (!m_pActiveShell->GotoNextTOXBase(&sName))
                m_pActiveShell->GotoPrevTOXBase(&sName);
        }
        break;
        case ContentTypeId::POSTIT:
            m_pActiveShell->GotoFormatField(*static_cast<const SwPostItContent*>(pCnt)->GetPostIt());
        break;
        case ContentTypeId::DRAWOBJECT:
        {
            m_pActiveShell->GotoDrawingObject(pCnt->GetName());
        }
        break;
        case ContentTypeId::FOOTNOTE:
        case ContentTypeId::ENDNOTE:
        {
            const SwTextFootnote* pFootnote =
                    static_cast<const SwTextFootnoteContent*>(pCnt)->GetTextFootnote();
            if (!pFootnote)
                return;
            m_pActiveShell->GotoFootnoteAnchor(*pFootnote);
        }
        break;
        default: break;
    }

    if (m_bSelectTo)
    {
        m_pActiveShell->SttCursorMove();
        while (m_pActiveShell->IsCursorInTable())
        {
            m_pActiveShell->MoveTable(GotoCurrTable, fnTableStart);
            if (!m_pActiveShell->Left(SwCursorSkipMode::Chars, false, 1, false))
                break; // Table is at the beginning of the document. It can't be selected this way.
        }
        m_pActiveShell->EndCursorMove();

        m_pActiveShell->AssureStdMode();

        m_pActiveShell->SetMark();
        m_pActiveShell->GetCursor()->GetMark()->nNode = oPosition.value()->nNode;
        m_pActiveShell->GetCursor()->GetMark()->nContent = oPosition.value()->nContent;
        m_pActiveShell->UpdateCursor();

        m_pActiveShell->GetView().GetEditWin().GrabFocus();

        m_bSelectTo = false;
    }
    else
    {
        if (m_pActiveShell->IsFrameSelected() || m_pActiveShell->IsObjSelected())
        {
            m_pActiveShell->HideCursor();
            m_pActiveShell->EnterSelFrameMode();
        }

        SwView& rView = m_pActiveShell->GetView();
        rView.StopShellTimer();
        rView.GetPostItMgr()->SetActiveSidebarWin(nullptr);
        rView.GetEditWin().GrabFocus();

        // Assure cursor is in visible view area.
        // (tdf#147041) Always show the navigated outline at the top of the visible view area.
        if (pCnt->GetParent()->GetType() == ContentTypeId::OUTLINE ||
                (!m_pActiveShell->IsCursorVisible() && !m_pActiveShell->IsFrameSelected() &&
                 !m_pActiveShell->IsObjSelected()))
        {
            Point aPoint(rView.GetVisArea().getX(), m_pActiveShell->GetCursorDocPos().getY());
            rView.SetVisArea(aPoint);
        }
    }
}

// Now even the matching text::Bookmark
NaviContentBookmark::NaviContentBookmark()
    :
    m_nDocSh(0),
    m_nDefaultDrag( RegionMode::NONE )
{
}

NaviContentBookmark::NaviContentBookmark( OUString aUrl,
                    OUString aDesc,
                    RegionMode nDragType,
                    const SwDocShell* pDocSh ) :
    m_aUrl(std::move( aUrl )),
    m_aDescription(std::move(aDesc)),
    m_nDocSh(reinterpret_cast<sal_IntPtr>(pDocSh)),
    m_nDefaultDrag( nDragType )
{
}

void NaviContentBookmark::Copy( TransferDataContainer& rData ) const
{
    rtl_TextEncoding eSysCSet = osl_getThreadTextEncoding();

    OString sStrBuf(OUStringToOString(m_aUrl, eSysCSet) + OStringChar(NAVI_BOOKMARK_DELIM) +
                    OUStringToOString(m_aDescription, eSysCSet) + OStringChar(NAVI_BOOKMARK_DELIM) +
                    OString::number(static_cast<int>(m_nDefaultDrag)) + OStringChar(NAVI_BOOKMARK_DELIM) +
                    OString::number(m_nDocSh));
    rData.CopyByteString(SotClipboardFormatId::SONLK, sStrBuf);
}

bool NaviContentBookmark::Paste( const TransferableDataHelper& rData, const OUString& rsDesc )
{
    OUString sStr;
    bool bRet = rData.GetString( SotClipboardFormatId::SONLK, sStr );
    if( bRet )
    {
        sal_Int32 nPos = 0;
        m_aUrl    = sStr.getToken(0, NAVI_BOOKMARK_DELIM, nPos );
        m_aDescription  = sStr.getToken(0, NAVI_BOOKMARK_DELIM, nPos );
        m_nDefaultDrag= static_cast<RegionMode>( o3tl::toInt32(o3tl::getToken(sStr, 0, NAVI_BOOKMARK_DELIM, nPos )) );
        m_nDocSh  = o3tl::toInt32(o3tl::getToken(sStr, 0, NAVI_BOOKMARK_DELIM, nPos ));
        if (!rsDesc.isEmpty())
            m_aDescription = rsDesc;
    }
    return bRet;
}

SwNavigationPI* SwContentTree::GetParentWindow()
{
    return m_pDialog;
}

void SwContentTree::SelectContentType(std::u16string_view rContentTypeName)
{
    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_iter_first(*xIter))
        return;
    do
    {
        if (m_xTreeView->get_text(*xIter) == rContentTypeName)
        {
            m_xTreeView->set_cursor(*xIter);
            Select();
            break;
        }
    } while (m_xTreeView->iter_next_sibling(*xIter));
}

IMPL_LINK_NOARG(SwContentTree, OverlayObjectDelayTimerHdl, Timer *, void)
{
    m_aOverlayObjectDelayTimer.Stop();
    if (m_xOverlayObject)
    {
        if (SdrView* pView = m_pActiveShell->GetDrawView())
        {
            if (SdrPaintWindow* pPaintWindow = pView->GetPaintWindow(0))
            {
                const rtl::Reference<sdr::overlay::OverlayManager>& xOverlayManager =
                        pPaintWindow->GetOverlayManager();
                xOverlayManager->add(*m_xOverlayObject);
            }
        }
    }
}

void SwContentTree::OverlayObject(std::vector<basegfx::B2DRange>&& aRanges)
{
    m_aOverlayObjectDelayTimer.Stop();
    if (m_xOverlayObject && m_xOverlayObject->getOverlayManager())
        m_xOverlayObject->getOverlayManager()->remove(*m_xOverlayObject);
    if (aRanges.empty())
        m_xOverlayObject.reset();
    else
    {
        m_xOverlayObject.reset(new sdr::overlay::OverlaySelection(
                                   sdr::overlay::OverlayType::Invert,
                                   Color(), std::move(aRanges), true/*unused for Invert type*/));
        m_aOverlayObjectDelayTimer.Start();
    }
}

void SwContentTree::BringEntryToAttention(const weld::TreeIter& rEntry)
{
    if (lcl_IsContent(rEntry, *m_xTreeView)) // content entry
    {
        SwContent* pCnt = weld::fromId<SwContent*>(m_xTreeView->get_id(rEntry));
        if (pCnt->IsInvisible())
            OverlayObject();
        else
        {
            const ContentTypeId nType = pCnt->GetParent()->GetType();
            if (nType == ContentTypeId::OUTLINE)
            {
                BringTypesWithFlowFramesToAttention({m_pActiveShell->GetNodes().
                        GetOutLineNds()[static_cast<SwOutlineContent*>(pCnt)->GetOutlinePos()]},
                                                    /*bIncludeTopMargin*/ false);
            }
            else if (nType == ContentTypeId::TABLE)
            {
                if (const sw::TableFrameFormats* pFrameFormats = m_pActiveShell->GetDoc()->GetTableFrameFormats())
                    if (const SwTableFormat* pFrameFormat = pFrameFormats->FindFrameFormatByName(pCnt->GetName()))
                    {
                        SwTable* pTable = SwTable::FindTable(pFrameFormat);
                        if (pTable)
                            BringTypesWithFlowFramesToAttention({pTable->GetTableNode()}, false);
                    }
            }
            else if (nType == ContentTypeId::FRAME || nType == ContentTypeId::GRAPHIC ||
                     nType == ContentTypeId::OLE)
            {
                SwNodeType eNodeType = SwNodeType::Text;
                if(nType == ContentTypeId::GRAPHIC)
                    eNodeType = SwNodeType::Grf;
                else if(nType == ContentTypeId::OLE)
                    eNodeType = SwNodeType::Ole;
                if (const SwFrameFormat* pFrameFormat =
                        m_pActiveShell->GetDoc()->FindFlyByName(pCnt->GetName(), eNodeType))
                    BringFramesToAttention(std::vector<const SwFrameFormat*> {pFrameFormat});
            }
            else if (nType == ContentTypeId::BOOKMARK)
            {
                BringBookmarksToAttention(std::vector<OUString> {pCnt->GetName()});
            }
            else if (nType == ContentTypeId::REGION || nType == ContentTypeId::INDEX)
            {
                size_t nSectionFormatCount = m_pActiveShell->GetSectionFormatCount();
                for (size_t i = 0; i < nSectionFormatCount; ++i)
                {
                    const SwSectionFormat& rSectionFormat = m_pActiveShell->GetSectionFormat(i);
                    if (!rSectionFormat.IsInNodesArr())
                        continue;
                    const SwSection* pSection = rSectionFormat.GetSection();
                    if (!pSection)
                        continue;
                    if (pCnt->GetName() == pSection->GetSectionName())
                    {
                        BringTypesWithFlowFramesToAttention({rSectionFormat.GetSectionNode()});
                        break;
                    }
                }
            }
            else if (nType == ContentTypeId::URLFIELD)
            {
                // tdf#159147 - Assure the SwURLFieldContent::SwTextINetFormat pointer is valid
                // before bringing to attention.
                const SwTextINetFormat* pTextINetFormat
                        = static_cast<SwURLFieldContent*>(pCnt)->GetINetAttr();
                const SwCharFormats* pFormats = m_pActiveShell->GetDoc()->GetCharFormats();
                for (auto n = pFormats->size(); 1 < n;)
                {
                    SwIterator<SwTextINetFormat, SwCharFormat> aIter(*(*pFormats)[--n]);
                    for (SwTextINetFormat* pFnd = aIter.First(); pFnd; pFnd = aIter.Next())
                    {
                        if (pTextINetFormat == pFnd)
                        {
                            BringURLFieldsToAttention(SwGetINetAttrs {SwGetINetAttr(pCnt->GetName(),
                                                                      *pTextINetFormat)});
                            return;
                        }
                    }
                }
            }
            else if (nType == ContentTypeId::REFERENCE)
            {
                if (const SwTextAttr* pTextAttr =
                        m_pActiveShell->GetDoc()->GetRefMark(pCnt->GetName())->GetTextRefMark())
                {
                    std::vector<const SwTextAttr*> aTextAttrArr {pTextAttr};
                    BringReferencesToAttention(aTextAttrArr);
                }
            }
            else if (nType == ContentTypeId::POSTIT)
            {
                if (const SwTextAttr* pTextAttr =
                        static_cast<SwPostItContent*>(pCnt)->GetPostIt()->GetTextField())
                {
                    std::vector<const SwTextAttr*> aTextAttrArr {pTextAttr};
                    BringPostItFieldsToAttention(aTextAttrArr);
                }
            }
            else if (nType == ContentTypeId::DRAWOBJECT)
            {
                std::vector<const SdrObject*> aSdrObjectArr {GetDrawingObjectsByContent(pCnt)};
                BringDrawingObjectsToAttention(aSdrObjectArr);
            }
            else if (nType == ContentTypeId::TEXTFIELD)
            {
                if (const SwTextAttr* pTextAttr =
                        static_cast<SwTextFieldContent*>(pCnt)->GetFormatField()->GetTextField())
                {
                    std::vector<const SwTextAttr*> aTextAttrArr {pTextAttr};
                    BringTextFieldsToAttention(aTextAttrArr);
                }
            }
            else if (nType == ContentTypeId::FOOTNOTE || nType == ContentTypeId::ENDNOTE)
            {
                if (const SwTextAttr* pTextAttr =
                        static_cast<SwTextFootnoteContent*> (pCnt)->GetTextFootnote())
                {
                    std::vector<const SwTextAttr*> aTextAttrArr {pTextAttr};
                    BringFootnotesToAttention(aTextAttrArr);
                }
            }
        }
    }
    else // content type entry
    {
        SwContentType* pCntType = weld::fromId<SwContentType*>(m_xTreeView->get_id(rEntry));
        if (pCntType->GetMemberCount() == 0)
            OverlayObject();
        else
        {
            const ContentTypeId nType = pCntType->GetType();
            if (nType == ContentTypeId::OUTLINE)
            {
                std::vector<const SwNode*> aNodesArr(
                            m_pActiveShell->GetNodes().GetOutLineNds().begin(),
                            m_pActiveShell->GetNodes().GetOutLineNds().end());
                BringTypesWithFlowFramesToAttention(aNodesArr, /*bIncludeTopMargin*/ false);
            }
            else if (nType == ContentTypeId::TABLE)
            {
                std::vector<const SwNode*> aNodesArr;
                const size_t nCount = m_pActiveShell->GetTableFrameFormatCount(false);
                const sw::TableFrameFormats& rTableFormats = *m_pActiveShell->GetDoc()->GetTableFrameFormats();
                for(size_t i = 0; i < nCount; ++i)
                {
                    if (const SwTableFormat* pTableFormat = rTableFormats[i])
                        if(pTableFormat->IsUsed())  // skip deleted tables
                        {
                            SwTable* pTable = SwTable::FindTable(pTableFormat);
                            if (pTable)
                                aNodesArr.push_back(pTable->GetTableNode());
                        }
                }
                BringTypesWithFlowFramesToAttention(aNodesArr, false);
            }
            else if (nType == ContentTypeId::FRAME || nType == ContentTypeId::GRAPHIC ||
                     nType == ContentTypeId::OLE)
            {
                FlyCntType eType = FLYCNTTYPE_FRM;
                if(nType == ContentTypeId::GRAPHIC)
                    eType = FLYCNTTYPE_GRF;
                else if(nType == ContentTypeId::OLE)
                    eType = FLYCNTTYPE_OLE;
                BringFramesToAttention(m_pActiveShell->GetFlyFrameFormats(eType, true));
            }
            else if (nType == ContentTypeId::BOOKMARK)
            {
                std::vector<OUString> aNames;
                const auto nCount = pCntType->GetMemberCount();
                for (size_t i = 0; i < nCount; i++)
                {
                    const SwContent* pMember = pCntType->GetMember(i);
                    if (pMember && !pMember->IsInvisible())
                        aNames.push_back(pMember->GetName());
                }
                BringBookmarksToAttention(aNames);
            }
            else if (nType == ContentTypeId::REGION || nType == ContentTypeId::INDEX)
            {
                std::vector<const SwNode*> aNodesArr;
                const SwSectionFormats& rFormats = m_pActiveShell->GetDoc()->GetSections();
                const size_t nSize = rFormats.size();
                for (SwSectionFormats::size_type n = nSize; n;)
                {
                    const SwSectionFormat* pSectionFormat = rFormats[--n];
                    if (pSectionFormat && pSectionFormat->IsInNodesArr())
                    {
                        const SwSection* pSection = pSectionFormat->GetSection();
                        if (pSection && !pSection->IsHiddenFlag())
                        {
                            const SectionType eSectionType = pSection->GetType();
                            if (nType == ContentTypeId::REGION &&
                                    (eSectionType == SectionType::ToxContent ||
                                     eSectionType == SectionType::ToxHeader))
                                continue;
                            if (nType == ContentTypeId::INDEX &&
                                    eSectionType != SectionType::ToxContent)
                                continue;
                            if (const SwNode* pNode = pSectionFormat->GetSectionNode())
                                aNodesArr.push_back(pNode);
                        }
                    }
                }
                BringTypesWithFlowFramesToAttention(aNodesArr);
            }
            else if (nType == ContentTypeId::URLFIELD)
            {
                SwGetINetAttrs aINetAttrsArr;
                m_pActiveShell->GetINetAttrs(aINetAttrsArr, false);
                BringURLFieldsToAttention(aINetAttrsArr);
            }
            else if (nType == ContentTypeId::REFERENCE)
            {
                std::vector<const SwTextAttr*> aTextAttrArr;
                for (const SfxPoolItem* pItem :
                     m_pActiveShell->GetAttrPool().GetItemSurrogates(RES_TXTATR_REFMARK))
                {
                    if (const auto pRefMark = dynamic_cast<const SwFormatRefMark*>(pItem))
                    {
                        const SwTextRefMark* pTextRef = pRefMark->GetTextRefMark();
                        if (pTextRef && &pTextRef->GetTextNode().GetNodes() ==
                                &m_pActiveShell->GetNodes())
                            aTextAttrArr.push_back(pTextRef);
                    }
                }
                BringReferencesToAttention(aTextAttrArr);
            }
            else if (nType == ContentTypeId::POSTIT)
            {
                std::vector<const SwTextAttr*> aTextAttrArr;
                const auto nCount = pCntType->GetMemberCount();
                for (size_t i = 0; i < nCount; i++)
                {
                    const SwPostItContent* pPostItContent = static_cast<const SwPostItContent*>(
                                pCntType->GetMember(i));
                    if (pPostItContent && !pPostItContent->IsInvisible())
                        if (const SwFormatField* pFormatField = pPostItContent->GetPostIt())
                            if (const SwTextAttr* pTextAttr = pFormatField->GetTextField())
                                aTextAttrArr.push_back(pTextAttr);
                }
                BringPostItFieldsToAttention(aTextAttrArr);
            }
            else if (nType == ContentTypeId::DRAWOBJECT)
            {
                IDocumentDrawModelAccess& rIDDMA = m_pActiveShell->getIDocumentDrawModelAccess();
                if (const SwDrawModel* pModel = rIDDMA.GetDrawModel())
                {
                    if (const SdrPage* pPage = pModel->GetPage(0))
                    {
                        if (pPage->GetObjCount())
                        {
                            std::vector<const SdrObject*> aSdrObjectArr;
                            for (const rtl::Reference<SdrObject>& pObject : *pPage)
                            {
                                if (pObject && !pObject->GetName().isEmpty() &&
                                        rIDDMA.IsVisibleLayerId(pObject->GetLayer()))
                                    aSdrObjectArr.push_back(pObject.get());
                            }
                            BringDrawingObjectsToAttention(aSdrObjectArr);
                        }
                    }
                }
            }
            else if (nType == ContentTypeId::TEXTFIELD)
            {
                std::vector<const SwTextAttr*> aTextAttrArr;
                const auto nCount = pCntType->GetMemberCount();
                for (size_t i = 0; i < nCount; i++)
                {
                    const SwTextFieldContent* pTextFieldCnt =
                            static_cast<const SwTextFieldContent*>(pCntType->GetMember(i));
                    if (pTextFieldCnt && !pTextFieldCnt->IsInvisible())
                        if (const SwFormatField* pFormatField = pTextFieldCnt->GetFormatField())
                            if (const SwTextAttr* pTextAttr = pFormatField->GetTextField())
                                aTextAttrArr.push_back(pTextAttr);
                }
                BringTextFieldsToAttention(aTextAttrArr);
            }
            else if (nType == ContentTypeId::FOOTNOTE || nType == ContentTypeId::ENDNOTE)
            {
                std::vector<const SwTextAttr*> aTextAttrArr;
                const auto nCount = pCntType->GetMemberCount();
                for (size_t i = 0; i < nCount; i++)
                {
                    const SwTextFootnoteContent* pTextFootnoteCnt =
                            static_cast<const SwTextFootnoteContent*>(pCntType->GetMember(i));
                    if (pTextFootnoteCnt && !pTextFootnoteCnt->IsInvisible())
                        if (const SwTextAttr* pTextAttr = pTextFootnoteCnt->GetTextFootnote())
                            aTextAttrArr.push_back(pTextAttr);
                }
                BringFootnotesToAttention(aTextAttrArr);
            }
        }
    }
}

static void lcl_CalcOverlayRanges(const SwTextFrame* pStartFrame, const SwTextFrame* pEndFrame,
                                  const SwPosition& aStartPos, const SwPosition& aEndPos,
                                  std::vector<basegfx::B2DRange>& aRanges)
{
    if (pStartFrame && pEndFrame)
    {
        SwRect aStartCharRect;
        pStartFrame->GetCharRect(aStartCharRect, aStartPos);
        SwRect aEndCharRect;
        pEndFrame->GetCharRect(aEndCharRect, aEndPos);
        if (aStartCharRect.Top() == aEndCharRect.Top())
        {
            // single line range
            aRanges.emplace_back(aStartCharRect.Left(), aStartCharRect.Top(),
                                 aEndCharRect.Right() + 1, aEndCharRect.Bottom() + 1);
        }
        else
        {
            // multi line range
            SwRect aFrameRect = pStartFrame->getFrameArea();
            aRanges.emplace_back(aStartCharRect.Left(), aStartCharRect.Top(),
                                 aFrameRect.Right(), aStartCharRect.Bottom() + 1);
            if (aStartCharRect.Bottom() + 1 != aEndCharRect.Top())
                aRanges.emplace_back(aFrameRect.Left(), aStartCharRect.Bottom() + 1,
                                     aFrameRect.Right(), aEndCharRect.Top() + 1);
            aRanges.emplace_back(aFrameRect.Left(), aEndCharRect.Top() + 1,
                                 aEndCharRect.Right() + 1, aEndCharRect.Bottom() + 1);
        }
    }
}

void SwContentTree::BringFramesToAttention(const std::vector<const SwFrameFormat*>& rFrameFormats)
{
    std::vector<basegfx::B2DRange> aRanges;
    for (const SwFrameFormat* pFrameFormat : rFrameFormats)
    {
        if (!pFrameFormat)
            continue;
        SwRect aFrameRect = pFrameFormat->FindLayoutRect();
        if (!aFrameRect.IsEmpty())
            aRanges.emplace_back(aFrameRect.Left(), aFrameRect.Top(), aFrameRect.Right(),
                                 aFrameRect.Bottom());
    }
    OverlayObject(std::move(aRanges));
}

void SwContentTree::BringBookmarksToAttention(const std::vector<OUString>& rNames)
{
    std::vector<basegfx::B2DRange> aRanges;
    IDocumentMarkAccess* const pMarkAccess = m_pActiveShell->getIDocumentMarkAccess();
    for (const auto& rName : rNames)
    {
        IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findBookmark(rName);
        if (ppBkmk == pMarkAccess->getBookmarksEnd())
            continue;
        SwPosition aMarkStart = (*ppBkmk)->GetMarkStart();
        const SwTextNode* pMarkStartTextNode = aMarkStart.GetNode().GetTextNode();
        if (!pMarkStartTextNode)
            continue;
        const SwTextFrame* pMarkStartFrame = static_cast<const SwTextFrame*>(
                    pMarkStartTextNode->getLayoutFrame(m_pActiveShell->GetLayout()));
        if (!pMarkStartFrame)
            continue;
        SwPosition aMarkEnd = (*ppBkmk)->GetMarkEnd();
        const SwTextNode* pMarkEndTextNode = aMarkEnd.GetNode().GetTextNode();
        if (!pMarkEndTextNode)
            continue;
        const SwTextFrame* pMarkEndFrame = static_cast<const SwTextFrame*>(
                    pMarkEndTextNode->getLayoutFrame(m_pActiveShell->GetLayout()));
        if (!pMarkEndFrame)
            continue;
        // adjust span when mark start equals mark end
        if (aMarkStart == aMarkEnd)
        {
            if (aMarkEnd.GetContentIndex() < pMarkEndTextNode->Len() - 1)
                aMarkEnd.AdjustContent(+1);
            else if (aMarkStart.GetContentIndex() > 0)
                aMarkStart.AdjustContent(-1);
        }
        lcl_CalcOverlayRanges(pMarkStartFrame, pMarkEndFrame, aMarkStart, aMarkEnd, aRanges);
    }
    OverlayObject(std::move(aRanges));
}

void SwContentTree::BringTypesWithFlowFramesToAttention(const std::vector<const SwNode*>& rNodes,
                                                        const bool bIncludeTopMargin)
{
    std::vector<basegfx::B2DRange> aRanges;
    for (const auto* pNode : rNodes)
    {
        if (!pNode)
            continue;
        const SwFrame* pFrame;
        if (pNode->IsContentNode() || pNode->IsTableNode())
        {
            if (pNode->IsContentNode())
                pFrame = pNode->GetContentNode()->getLayoutFrame(m_pActiveShell->GetLayout());
            else // table node
            {
                SwNode2Layout aTmp(*pNode, pNode->GetIndex() - 1);
                pFrame = aTmp.NextFrame();
            }
            while (pFrame)
            {
                const SwRect& rFrameRect = pFrame->getFrameArea();
                if (!rFrameRect.IsEmpty())
                    aRanges.emplace_back(rFrameRect.Left(), bIncludeTopMargin ? rFrameRect.Top() :
                                         rFrameRect.Top() + pFrame->GetTopMargin(),
                                         rFrameRect.Right(), rFrameRect.Bottom());
                if (!pFrame->IsFlowFrame())
                    break;
                const SwFlowFrame *pFollow = SwFlowFrame::CastFlowFrame(pFrame)->GetFollow();
                if (!pFollow)
                    break;
                pFrame = &pFollow->GetFrame();
            }
        }
        else if (pNode->IsSectionNode())
        {
            const SwNode* pEndOfSectionNode = pNode->EndOfSectionNode();
            SwNodeIndex aIdx(*pNode);
            while (&aIdx.GetNode() != pEndOfSectionNode)
            {
                if (aIdx.GetNode().IsContentNode())
                {
                    if ((pFrame = aIdx.GetNode().GetContentNode()->
                         getLayoutFrame(m_pActiveShell->GetLayout())))
                    {
                        if (pFrame->IsInSct())
                            pFrame = pFrame->FindSctFrame();
                        if (pFrame)
                        {
                            const SwRect& rFrameRect = pFrame->getFrameArea();
                            if (!rFrameRect.IsEmpty())
                                aRanges.emplace_back(rFrameRect.Left(), rFrameRect.Top(),
                                                     rFrameRect.Right(), rFrameRect.Bottom());
                        }
                    }
                    ++aIdx;
                    while (!aIdx.GetNode().IsEndNode() && !aIdx.GetNode().IsSectionNode())
                        ++aIdx;
                    continue;
                }
                if (!aIdx.GetNode().IsSectionNode())
                {
                    ++aIdx;
                    continue;
                }
                SwNode2Layout aTmp(aIdx.GetNode(), aIdx.GetNode().GetIndex() - 1);
                pFrame = aTmp.NextFrame();
                if (pFrame)
                {
                    if (!pFrame->getFrameArea().IsEmpty())
                    {
                        const SwRect& rFrameRect = pFrame->getFrameArea();
                        aRanges.emplace_back(rFrameRect.Left(), rFrameRect.Top(),
                                             rFrameRect.Right(), rFrameRect.Bottom());
                    }
                    if (pFrame->IsSctFrame())
                    {
                        const SwSectionFrame* pSectionFrame
                                = static_cast<const SwSectionFrame*>(pFrame);
                        if (pSectionFrame->HasFollow())
                        {
                            const SwFlowFrame *pFollow
                                    = SwFlowFrame::CastFlowFrame(pSectionFrame)->GetFollow();
                            while (pFollow)
                            {
                                pFrame = &pFollow->GetFrame();
                                if (!pFrame->getFrameArea().IsEmpty())
                                {
                                    const SwRect& rFrameRect = pFrame->getFrameArea();
                                    aRanges.emplace_back(rFrameRect.Left(), rFrameRect.Top(),
                                                         rFrameRect.Right(), rFrameRect.Bottom());
                                }
                                pFollow = SwFlowFrame::CastFlowFrame(pFrame)->GetFollow();
                            }
                        }
                    }
                }
                ++aIdx;
                while (!aIdx.GetNode().IsEndNode() && !aIdx.GetNode().IsSectionNode())
                    ++aIdx;
            }
            // Remove nested sections. This wouldn't be needed if the overlay wasn't invert type.
            auto end = aRanges.end();
            for (auto it = aRanges.begin(); it != end; ++it)
                end = std::remove_if(it + 1, end, [&it](auto itt){ return it->isInside(itt); });
            aRanges.erase(end, aRanges.end());
        }
    }
    OverlayObject(std::move(aRanges));
}

void SwContentTree::BringURLFieldsToAttention(const SwGetINetAttrs& rINetAttrsArr)
{
    std::vector<basegfx::B2DRange> aRanges;
    for (const auto& r : rINetAttrsArr)
    {
        const SwTextNode& rTextNode = r.rINetAttr.GetTextNode();
        if (SwTextFrame* pFrame = static_cast<SwTextFrame*>(
                    rTextNode.getLayoutFrame(m_pActiveShell->GetLayout())))
        {
            auto nStart = r.rINetAttr.GetStart();
            auto nEnd = r.rINetAttr.GetAnyEnd();
            SwPosition aStartPos(rTextNode, nStart), aEndPos(rTextNode, nEnd);
            lcl_CalcOverlayRanges(pFrame, pFrame, aStartPos, aEndPos, aRanges);
        }
    }
    OverlayObject(std::move(aRanges));
}

void SwContentTree::BringReferencesToAttention(std::vector<const SwTextAttr*>& rTextAttrsArr)
{
    std::vector<basegfx::B2DRange> aRanges;
    for (const SwTextAttr* p : rTextAttrsArr)
    {
        if (!p)
            continue;
        const SwTextRefMark* pTextRefMark = p->GetRefMark().GetTextRefMark();
        if (!pTextRefMark)
            continue;
        const SwTextNode& rTextNode = pTextRefMark->GetTextNode();
        if (SwTextFrame* pFrame = static_cast<SwTextFrame*>(
                    rTextNode.getLayoutFrame(m_pActiveShell->GetLayout())))
        {
            auto nStart = p->GetStart();
            auto nEnd = p->GetAnyEnd();
            SwPosition aStartPos(rTextNode, nStart), aEndPos(rTextNode, nEnd);
            lcl_CalcOverlayRanges(pFrame, pFrame, aStartPos, aEndPos, aRanges);
        }
    }
    OverlayObject(std::move(aRanges));
}

void SwContentTree::BringPostItFieldsToAttention(std::vector<const SwTextAttr*>& rTextAttrsArr)
{
    std::vector<basegfx::B2DRange> aRanges;
    for (const SwTextAttr* p : rTextAttrsArr)
    {
        if (!p)
            continue;
        const SwTextField* pTextField = p->GetFormatField().GetTextField();
        if (!pTextField)
            continue;
        // use as a fallback when there is no mark
        SwTextNode& rTextNode = pTextField->GetTextNode();
        if (!rTextNode.getLayoutFrame(m_pActiveShell->GetLayout()))
            continue;
        assert(dynamic_cast<const SwTextAnnotationField*>(pTextField));
        const SwTextAnnotationField* pTextAnnotationField =
                static_cast<const SwTextAnnotationField*>(pTextField);
        const ::sw::mark::IMark* pAnnotationMark = pTextAnnotationField->GetAnnotationMark();
        const SwPosition aMarkStart = pAnnotationMark ? pAnnotationMark->GetMarkStart()
                                                : SwPosition(rTextNode, p->GetStart());
        const SwPosition aMarkEnd = pAnnotationMark ? pAnnotationMark->GetMarkEnd()
                                              : SwPosition(rTextNode, p->GetAnyEnd());
        const SwTextFrame* pMarkStartFrame = static_cast<SwTextFrame*>(
                    aMarkStart.GetNode().GetTextNode()->getLayoutFrame(m_pActiveShell->GetLayout()));
        const SwTextFrame* pMarkEndFrame = static_cast<SwTextFrame*>(
                    aMarkEnd.GetNode().GetTextNode()->getLayoutFrame(m_pActiveShell->GetLayout()));
        if (!pMarkStartFrame || !pMarkEndFrame)
            continue;
        lcl_CalcOverlayRanges(pMarkStartFrame, pMarkEndFrame, aMarkStart,
                              aMarkEnd, aRanges);
    }
    OverlayObject(std::move(aRanges));
}

void SwContentTree::BringFootnotesToAttention(std::vector<const SwTextAttr*>& rTextAttrsArr)
{
    std::vector<basegfx::B2DRange> aRanges;
    for (const SwTextAttr* p : rTextAttrsArr)
    {
        if (!p)
            continue;
        const SwTextFootnote* pTextFootnote = p->GetFootnote().GetTextFootnote();
        if (!pTextFootnote)
            continue;
        const SwTextNode& rTextNode = pTextFootnote->GetTextNode();
        if (SwTextFrame* pFrame = static_cast<SwTextFrame*>(
                    rTextNode.getLayoutFrame(m_pActiveShell->GetLayout())))
        {
            auto nStart = p->GetStart();
            auto nEnd = nStart + 1;
            SwPosition aStartPos(rTextNode, nStart), aEndPos(rTextNode, nEnd);
            lcl_CalcOverlayRanges(pFrame, pFrame, aStartPos, aEndPos, aRanges);
        }
    }
    OverlayObject(std::move(aRanges));
}

void SwContentTree::BringDrawingObjectsToAttention(std::vector<const SdrObject*>& rDrawingObjectsArr)
{
    std::vector<basegfx::B2DRange> aRanges;
    for (const SdrObject* pObject : rDrawingObjectsArr)
    {
        if (pObject)
        {
            tools::Rectangle aRect(pObject->GetLogicRect());
            if (!aRect.IsEmpty())
                aRanges.emplace_back(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
        }
    }
    OverlayObject(std::move(aRanges));
}

void SwContentTree::BringTextFieldsToAttention(std::vector<const SwTextAttr*>& rTextAttrsArr)
{
    std::vector<basegfx::B2DRange> aRanges;
    std::shared_ptr<SwPaM> pPamForTextField;
    for (const SwTextAttr* p : rTextAttrsArr)
    {
        if (!p)
            continue;
        const SwTextField* pTextField = p->GetFormatField().GetTextField();
        if (!pTextField)
            continue;
        if (SwTextFrame* pFrame = static_cast<SwTextFrame*>(
                    pTextField->GetTextNode().getLayoutFrame(m_pActiveShell->GetLayout())))
        {
            SwTextField::GetPamForTextField(*pTextField, pPamForTextField);
            if (!pPamForTextField)
                continue;
            SwPosition aStartPos(*pPamForTextField->GetMark());
            SwPosition aEndPos(*pPamForTextField->GetPoint());
            lcl_CalcOverlayRanges(pFrame, pFrame, aStartPos, aEndPos, aRanges);
        }
    }
    OverlayObject(std::move(aRanges));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
