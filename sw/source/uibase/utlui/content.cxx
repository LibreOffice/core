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

#include <comphelper/string.hxx>
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
#include <vcl/commandevent.hxx>
#include <vcl/weldutils.hxx>
#include <sot/formats.hxx>
#include <uiitems.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <fmtfld.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <content.hxx>
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
#include <dcontact.hxx>
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

#include <navmgr.hxx>
#include <AnnotationWin.hxx>
#include <memory>

#include <fmtcntnt.hxx>
#include <docstat.hxx>

#include <viewopt.hxx>

#define CTYPE_CNT   0
#define CTYPE_CTT   1

using namespace ::std;
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
    bool lcl_IsContent(const weld::TreeIter& rEntry, const weld::TreeView& rTreeView)
    {
        return reinterpret_cast<const SwTypeNumber*>(rTreeView.get_id(rEntry).toInt64())->GetTypeId() == CTYPE_CNT;
    }

    bool lcl_IsContentType(const weld::TreeIter& rEntry, const weld::TreeView& rTreeView)
    {
        return reinterpret_cast<const SwTypeNumber*>(rTreeView.get_id(rEntry).toInt64())->GetTypeId() == CTYPE_CTT;
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

    size_t lcl_InsertURLFieldContent(
        SwContentArr *pMember,
        SwWrtShell* pWrtShell,
        const SwContentType *pCntType)
    {
        SwGetINetAttrs aArr;
        pWrtShell->GetINetAttrs( aArr );
        const SwGetINetAttrs::size_type nCount {aArr.size()};
        for( SwGetINetAttrs::size_type n = 0; n < nCount; ++n )
        {
            SwGetINetAttr* p = &aArr[ n ];
            std::unique_ptr<SwURLFieldContent> pCnt(new SwURLFieldContent(
                                pCntType,
                                p->sText,
                                INetURLObject::decode(
                                    p->rINetAttr.GetINetFormat().GetValue(),
                                    INetURLObject::DecodeMechanism::Unambiguous ),
                                &p->rINetAttr,
                                n ));
            pMember->insert( std::move(pCnt) );
        }
        return nCount;
    }
}

// Content, contains names and reference at the content type.

SwContent::SwContent(const SwContentType* pCnt, const OUString& rName, tools::Long nYPos) :
    SwTypeNumber(CTYPE_CNT),
    pParent(pCnt),
    sContentName(rName),
    nYPosition(nYPos),
    bInvisible(false)
{
}


SwTypeNumber::~SwTypeNumber()
{
}

bool SwContent::IsProtect() const
{
    return false;
}

bool SwPostItContent::IsProtect() const
{
    return pField->IsProtect();
}

bool SwURLFieldContent::IsProtect() const
{
    return pINetAttr->IsProtect();
}

SwGraphicContent::~SwGraphicContent()
{
}

SwTOXBaseContent::~SwTOXBaseContent()
{
}

static const char* STR_CONTENT_TYPE_ARY[] =
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
    STR_CONTENT_TYPE_DRAWOBJECT
};

static const char* STR_CONTENT_TYPE_SINGLE_ARY[] =
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
    STR_CONTENT_TYPE_SINGLE_DRAWOBJECT
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
    Init();
}

void SwContentType::Init(bool* pbInvalidateWindow)
{
    // if the MemberCount is changing ...
    size_t nOldMemberCount = m_nMemberCount;
    m_nMemberCount = 0;
    switch(m_nContentType)
    {
        case ContentTypeId::OUTLINE   :
        {
            m_sTypeToken = "outline";
            m_nMemberCount = m_pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
            if (m_nMemberCount < MAXLEVEL)
            {
                const size_t nOutlineCount = m_nMemberCount;
                for(size_t j = 0; j < nOutlineCount; ++j)
                {
                    if (m_pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(j) > m_nOutlineLevel
                        || !m_pWrtShell->getIDocumentOutlineNodesAccess()->isOutlineInLayout(j, *m_pWrtShell->GetLayout()))
                    {
                        m_nMemberCount --;
                    }
                }
            }
        }
        break;

        case ContentTypeId::TABLE     :
            m_sTypeToken = "table";
            m_nMemberCount = m_pWrtShell->GetTableFrameFormatCount(true);
            m_bEdit = true;
        break;

        case ContentTypeId::FRAME     :
        case ContentTypeId::GRAPHIC   :
        case ContentTypeId::OLE       :
        {
            FlyCntType eType = FLYCNTTYPE_FRM;
            m_sTypeToken = "frame";
            if(m_nContentType == ContentTypeId::OLE)
            {
                eType = FLYCNTTYPE_OLE;
                m_sTypeToken = "ole";
            }
            else if(m_nContentType == ContentTypeId::GRAPHIC)
            {
                eType = FLYCNTTYPE_GRF;
                m_sTypeToken = "graphic";
            }
            m_nMemberCount = m_pWrtShell->GetFlyCount(eType, /*bIgnoreTextBoxes=*/true);
            m_bEdit = true;
        }
        break;
        case ContentTypeId::BOOKMARK:
        {
            IDocumentMarkAccess* const pMarkAccess = m_pWrtShell->getIDocumentMarkAccess();
            m_nMemberCount = count_if(
                pMarkAccess->getBookmarksBegin(),
                pMarkAccess->getBookmarksEnd(),
                &lcl_IsUiVisibleBookmark);
            m_sTypeToken.clear();
            const bool bProtectedBM = m_pWrtShell->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_BOOKMARKS);
            m_bEdit = !bProtectedBM;
            m_bDelete = !bProtectedBM;
        }
        break;
        case ContentTypeId::REGION :
        {
            std::unique_ptr<SwContentArr> pOldMember;
            if(!m_pMember)
                m_pMember.reset( new SwContentArr );
            else if(!m_pMember->empty())
            {
                pOldMember = std::move(m_pMember);
                m_pMember.reset( new SwContentArr );
            }
            const Point aNullPt;
            m_nMemberCount = m_pWrtShell->GetSectionFormatCount();
            for(size_t i = 0; i < m_nMemberCount; ++i)
            {
                const SwSectionFormat* pFormat;
                SectionType eTmpType;
                if( (pFormat = &m_pWrtShell->GetSectionFormat(i))->IsInNodesArr() &&
                (eTmpType = pFormat->GetSection()->GetType()) != SectionType::ToxContent
                && SectionType::ToxHeader != eTmpType )
                {
                    const OUString& rSectionName =
                        pFormat->GetSection()->GetSectionName();
                    sal_uInt8 nLevel = 0;
                    SwSectionFormat* pParentFormat = pFormat->GetParent();
                    while(pParentFormat)
                    {
                        nLevel++;
                        pParentFormat = pParentFormat->GetParent();
                    }

                    std::unique_ptr<SwContent> pCnt(new SwRegionContent(this, rSectionName,
                            nLevel,
                            pFormat->FindLayoutRect( false, &aNullPt ).Top()));

                    SwPtrMsgPoolItem aAskItem( RES_CONTENT_VISIBLE, nullptr );
                    if( !pFormat->GetInfo( aAskItem ) &&
                        !aAskItem.pObject )     // not visible
                        pCnt->SetInvisible();
                    m_pMember->insert(std::move(pCnt));
                }
            }
            m_nMemberCount = m_pMember->size();
            m_sTypeToken = "region";
            m_bEdit = true;
            m_bDelete = false;
            if(pOldMember)
            {
                if(nullptr != pbInvalidateWindow)
                {
                    // need to check visibility (and equal entry number) after
                    // creation due to a sorted list being used here (before,
                    // entries with same index were compared already at creation
                    // time what worked before a sorted list was used)
                    *pbInvalidateWindow = checkVisibilityChanged(
                        *pOldMember,
                        *m_pMember);
                }
            }
        }
        break;
        case ContentTypeId::INDEX:
        {
            m_nMemberCount = m_pWrtShell->GetTOXCount();
            m_bEdit = true;
            m_bDelete = false;
        }
        break;
        case ContentTypeId::REFERENCE:
        {
            m_nMemberCount = m_pWrtShell->GetRefMarks();
            m_bDelete = false;
        }
        break;
        case ContentTypeId::URLFIELD:
        {
            m_nMemberCount = 0;
            if(!m_pMember)
                m_pMember.reset( new SwContentArr );
            else
                m_pMember->clear();

            m_nMemberCount = lcl_InsertURLFieldContent(m_pMember.get(), m_pWrtShell, this);

            m_bEdit = true;
            nOldMemberCount = m_nMemberCount;
            m_bDelete = true;
        }
        break;
        case ContentTypeId::POSTIT:
        {
            m_nMemberCount = 0;
            if(!m_pMember)
                m_pMember.reset( new SwContentArr );
            else
                m_pMember->clear();

            SwPostItMgr* aMgr = m_pWrtShell->GetView().GetPostItMgr();
            if (aMgr)
            {
                for(SwPostItMgr::const_iterator i = aMgr->begin(); i != aMgr->end(); ++i)
                {
                    if (const SwFormatField* pFormatField = dynamic_cast<const SwFormatField *>((*i)->GetBroadcaster())) // SwPostit
                    {
                        if (pFormatField->GetTextField() && pFormatField->IsFieldInDoc() &&
                            (*i)->mLayoutStatus!=SwPostItHelper::INVISIBLE )
                        {
                            OUString sEntry = pFormatField->GetField()->GetPar2();
                            sEntry = RemoveNewline(sEntry);
                            std::unique_ptr<SwPostItContent> pCnt(new SwPostItContent(
                                                this,
                                                sEntry,
                                                pFormatField,
                                                m_nMemberCount));
                            m_pMember->insert(std::move(pCnt));
                            m_nMemberCount++;
                        }
                    }
                }
            }
            m_sTypeToken.clear();
            m_bEdit = true;
            nOldMemberCount = m_nMemberCount;
        }
        break;
        case ContentTypeId::DRAWOBJECT:
        {
            m_sTypeToken.clear();
            m_bEdit = true;
            m_nMemberCount = 0;
            SwDrawModel* pModel = m_pWrtShell->getIDocumentDrawModelAccess().GetDrawModel();
            if(pModel)
            {
                SdrPage* pPage = pModel->GetPage(0);
                const size_t nCount = pPage->GetObjCount();
                for( size_t i=0; i<nCount; ++i )
                {
                    SdrObject* pTemp = pPage->GetObj(i);
                    // #i51726# - all drawing objects can be named now
                    if (!pTemp->GetName().isEmpty())
                        m_nMemberCount++;
                }
            }
        }
        break;
        default: break;
    }
    // ... then, the data can also no longer be valid,
    // apart from those which have already been corrected,
    // then nOldMemberCount is nevertheless not so old.
    if( nOldMemberCount != m_nMemberCount )
        m_bDataValid = false;
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

void SwContentType::FillMemberList(bool* pbLevelOrVisibilityChanged)
{
    std::unique_ptr<SwContentArr> pOldMember;
    size_t nOldMemberCount = 0;
    SwPtrMsgPoolItem aAskItem( RES_CONTENT_VISIBLE, nullptr );
    if(m_pMember && pbLevelOrVisibilityChanged)
    {
        pOldMember = std::move(m_pMember);
        nOldMemberCount = pOldMember->size();
        m_pMember.reset( new SwContentArr );
        *pbLevelOrVisibilityChanged = false;
    }
    else if(!m_pMember)
        m_pMember.reset( new SwContentArr );
    else
        m_pMember->clear();
    switch(m_nContentType)
    {
        case ContentTypeId::OUTLINE   :
        {
            const size_t nOutlineCount = m_nMemberCount =
                m_pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount();

            size_t nPos = 0;
            for (size_t i = 0; i < nOutlineCount; ++i)
            {
                const sal_uInt8 nLevel = m_pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(i);
                if(nLevel >= m_nOutlineLevel )
                    m_nMemberCount--;
                else
                {
                    if (!m_pWrtShell->getIDocumentOutlineNodesAccess()->isOutlineInLayout(i, *m_pWrtShell->GetLayout()))
                    {
                        --m_nMemberCount;
                        continue; // don't hide it, just skip it
                    }
                    OUString aEntry(comphelper::string::stripStart(
                        m_pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(i, m_pWrtShell->GetLayout()), ' '));
                    aEntry = SwNavigationPI::CleanEntry(aEntry);
                    std::unique_ptr<SwOutlineContent> pCnt(new SwOutlineContent(this, aEntry, i, nLevel,
                                                        m_pWrtShell->IsOutlineMovable( i ), nPos ));
                    m_pMember->insert(std::move(pCnt));
                    // with the same number and existing "pOldMember" the
                    // old one is compared with the new OutlinePos.
                    if (nOldMemberCount > nPos && static_cast<SwOutlineContent*>((*pOldMember)[nPos].get())->GetOutlineLevel() != nLevel)
                        *pbLevelOrVisibilityChanged = true;

                    nPos++;
                }
            }

        }
        break;
        case ContentTypeId::TABLE     :
        {
            const size_t nCount = m_pWrtShell->GetTableFrameFormatCount(true);
            OSL_ENSURE(m_nMemberCount == nCount, "MemberCount differs");
            Point aNullPt;
            m_nMemberCount = nCount;
            for(size_t i = 0; i < m_nMemberCount; ++i)
            {
                const SwFrameFormat& rTableFormat = m_pWrtShell->GetTableFrameFormat(i, true);
                const OUString& sTableName( rTableFormat.GetName() );

                SwContent* pCnt = new SwContent(this, sTableName,
                        rTableFormat.FindLayoutRect(false, &aNullPt).Top() );
                if( !rTableFormat.GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();

                m_pMember->insert(std::unique_ptr<SwContent>(pCnt));
            }

            if (nullptr != pbLevelOrVisibilityChanged)
            {
                assert(pOldMember);
                // need to check visibility (and equal entry number) after
                // creation due to a sorted list being used here (before,
                // entries with same index were compared already at creation
                // time what worked before a sorted list was used)
                *pbLevelOrVisibilityChanged = checkVisibilityChanged(
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
            m_nMemberCount = m_pWrtShell->GetFlyCount(eType, /*bIgnoreTextBoxes=*/true);
            std::vector<SwFrameFormat const*> formats(m_pWrtShell->GetFlyFrameFormats(eType, /*bIgnoreTextBoxes=*/true));
            SAL_WARN_IF(m_nMemberCount != formats.size(), "sw.ui", "MemberCount differs");
            m_nMemberCount = formats.size();
            for (size_t i = 0; i < m_nMemberCount; ++i)
            {
                SwFrameFormat const*const pFrameFormat = formats[i];
                const OUString sFrameName = pFrameFormat->GetName();

                SwContent* pCnt;
                if(ContentTypeId::GRAPHIC == m_nContentType)
                {
                    OUString sLink;
                    m_pWrtShell->GetGrfNms( &sLink, nullptr, static_cast<const SwFlyFrameFormat*>( pFrameFormat));
                    pCnt = new SwGraphicContent(this, sFrameName,
                                INetURLObject::decode( sLink,
                                           INetURLObject::DecodeMechanism::Unambiguous ),
                                pFrameFormat->FindLayoutRect(false, &aNullPt).Top());
                }
                else
                {
                    pCnt = new SwContent(this, sFrameName,
                            pFrameFormat->FindLayoutRect(false, &aNullPt).Top() );
                }
                if( !pFrameFormat->GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();
                m_pMember->insert(std::unique_ptr<SwContent>(pCnt));
            }

            if(nullptr != pbLevelOrVisibilityChanged)
            {
                assert(pOldMember);
                // need to check visibility (and equal entry number) after
                // creation due to a sorted list being used here (before,
                // entries with same index were compared already at creation
                // time what worked before a sorted list was used)
                *pbLevelOrVisibilityChanged = checkVisibilityChanged(
                    *pOldMember,
                    *m_pMember);
            }
        }
        break;
        case ContentTypeId::BOOKMARK:
        {
            IDocumentMarkAccess* const pMarkAccess = m_pWrtShell->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
                ppBookmark != pMarkAccess->getBookmarksEnd();
                ++ppBookmark)
            {
                if(lcl_IsUiVisibleBookmark(*ppBookmark))
                {
                    const OUString& rBkmName = (*ppBookmark)->GetName();
                    //nYPos from 0 -> text::Bookmarks will be sorted alphabetically
                    std::unique_ptr<SwContent> pCnt(new SwContent(this, rBkmName, 0));
                    m_pMember->insert(std::move(pCnt));
                }
            }
        }
        break;
        case ContentTypeId::REGION    :
        {
            const Point aNullPt;
            m_nMemberCount = m_pWrtShell->GetSectionFormatCount();
            for(size_t i = 0; i < m_nMemberCount; ++i)
            {
                const SwSectionFormat* pFormat;
                SectionType eTmpType;
                if( (pFormat = &m_pWrtShell->GetSectionFormat(i))->IsInNodesArr() &&
                (eTmpType = pFormat->GetSection()->GetType()) != SectionType::ToxContent
                && SectionType::ToxHeader != eTmpType )
                {
                    OUString sSectionName = pFormat->GetSection()->GetSectionName();

                    sal_uInt8 nLevel = 0;
                    SwSectionFormat* pParentFormat = pFormat->GetParent();
                    while(pParentFormat)
                    {
                        nLevel++;
                        pParentFormat = pParentFormat->GetParent();
                    }

                    std::unique_ptr<SwContent> pCnt(new SwRegionContent(this, sSectionName,
                            nLevel,
                            pFormat->FindLayoutRect( false, &aNullPt ).Top()));
                    if( !pFormat->GetInfo( aAskItem ) &&
                        !aAskItem.pObject )     // not visible
                        pCnt->SetInvisible();
                    m_pMember->insert(std::move(pCnt));
                }

                if(nullptr != pbLevelOrVisibilityChanged)
                {
                    assert(pOldMember);
                    // need to check visibility (and equal entry number) after
                    // creation due to a sorted list being used here (before,
                    // entries with same index were compared already at creation
                    // time what worked before a sorted list was used)
                    *pbLevelOrVisibilityChanged = checkVisibilityChanged(
                        *pOldMember,
                        *m_pMember);
                }
            }
            m_nMemberCount = m_pMember->size();
        }
        break;
        case ContentTypeId::REFERENCE:
        {
            std::vector<OUString> aRefMarks;
            m_nMemberCount = m_pWrtShell->GetRefMarks( &aRefMarks );

            for (const auto& rRefMark : aRefMarks)
            {
                // References sorted alphabetically
                m_pMember->insert(std::make_unique<SwContent>(this, rRefMark, 0));
            }
        }
        break;
        case ContentTypeId::URLFIELD:
            m_nMemberCount = lcl_InsertURLFieldContent(m_pMember.get(), m_pWrtShell, this);
        break;
        case ContentTypeId::INDEX:
        {

            const sal_uInt16 nCount = m_pWrtShell->GetTOXCount();
            m_nMemberCount = nCount;
            for ( sal_uInt16 nTox = 0; nTox < nCount; nTox++ )
            {
                const SwTOXBase* pBase = m_pWrtShell->GetTOX( nTox );
                OUString sTOXNm( pBase->GetTOXName() );

                SwContent* pCnt = new SwTOXBaseContent(
                        this, sTOXNm, nTox, *pBase);

                if(pBase && !pBase->IsVisible())
                    pCnt->SetInvisible();

                m_pMember->insert( std::unique_ptr<SwContent>(pCnt) );
                const size_t nPos = m_pMember->size() - 1;
                if(nOldMemberCount > nPos &&
                    (*pOldMember)[nPos]->IsInvisible()
                            != pCnt->IsInvisible())
                        *pbLevelOrVisibilityChanged = true;
            }
        }
        break;
        case ContentTypeId::POSTIT:
        {
            m_nMemberCount = 0;
            m_pMember->clear();
            SwPostItMgr* aMgr = m_pWrtShell->GetView().GetPostItMgr();
            if (aMgr)
            {
                for(SwPostItMgr::const_iterator i = aMgr->begin(); i != aMgr->end(); ++i)
                {
                    if (const SwFormatField* pFormatField = dynamic_cast<const SwFormatField *>((*i)->GetBroadcaster())) // SwPostit
                    {
                        if (pFormatField->GetTextField() && pFormatField->IsFieldInDoc() &&
                            (*i)->mLayoutStatus!=SwPostItHelper::INVISIBLE )
                        {
                            OUString sEntry = pFormatField->GetField()->GetPar2();
                            sEntry = RemoveNewline(sEntry);
                            std::unique_ptr<SwPostItContent> pCnt(new SwPostItContent(
                                                this,
                                                sEntry,
                                                pFormatField,
                                                m_nMemberCount));
                            m_pMember->insert(std::move(pCnt));
                            m_nMemberCount++;
                        }
                    }
                }
            }
        }
        break;
        case ContentTypeId::DRAWOBJECT:
        {
            m_nMemberCount = 0;
            m_pMember->clear();

            IDocumentDrawModelAccess& rIDDMA = m_pWrtShell->getIDocumentDrawModelAccess();
            SwDrawModel* pModel = rIDDMA.GetDrawModel();
            if(pModel)
            {
                SdrPage* pPage = pModel->GetPage(0);
                const size_t nCount = pPage->GetObjCount();
                for( size_t i=0; i<nCount; ++i )
                {
                    SdrObject* pTemp = pPage->GetObj(i);
                    // #i51726# - all drawing objects can be named now
                    if (!pTemp->GetName().isEmpty())
                    {
                        SwContact* pContact = static_cast<SwContact*>(pTemp->GetUserCall());
                        tools::Long nYPos = 0;
                        const Point aNullPt;
                        if(pContact && pContact->GetFormat())
                            nYPos = pContact->GetFormat()->FindLayoutRect(false, &aNullPt).Top();
                        SwContent* pCnt = new SwContent(
                                            this,
                                            pTemp->GetName(),
                                            nYPos);
                        if(!rIDDMA.IsVisibleLayerId(pTemp->GetLayer()))
                            pCnt->SetInvisible();
                        m_pMember->insert(std::unique_ptr<SwContent>(pCnt));
                        m_nMemberCount++;
                    }
                }

                if (nullptr != pbLevelOrVisibilityChanged)
                {
                    assert(pOldMember);
                    // need to check visibility (and equal entry number) after
                    // creation due to a sorted list being used here (before,
                    // entries with same index were compared already at creation
                    // time what worked before a sorted list was used)
                    *pbLevelOrVisibilityChanged = checkVisibilityChanged(
                        *pOldMember,
                        *m_pMember);
                }
            }
        }
        break;
        default: break;
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

static const char* STR_CONTEXT_ARY[] =
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
    , m_xScratchIter(m_xTreeView->make_iterator())
    , m_aDropTargetHelper(*this)
    , m_xDialog(pDialog)
    , m_sSpace(OUString("                    "))
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

    for (ContentTypeId i : o3tl::enumrange<ContentTypeId>())
    {
        m_aActiveContentArr[i] = nullptr;
        m_aHiddenContentArr[i] = nullptr;
    }
    for (int i = 0; i < CONTEXT_COUNT; ++i)
    {
        m_aContextStrings[i] = SwResId(STR_CONTEXT_ARY[i]);
    }
    m_nActiveBlock = m_pConfig->GetActiveBlock();
    m_aUpdTimer.SetInvokeHandler(LINK(this, SwContentTree, TimerUpdate));
    m_aUpdTimer.SetTimeout(1000);
}

SwContentTree::~SwContentTree()
{
    clear(); // If applicable erase content types previously.
    m_aUpdTimer.Stop();
    SetActiveShell(nullptr);
    m_xDialog.clear();
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
            assert(dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xDropEntry).toInt64())));
            SwOutlineContent* pOutlineContent = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xDropEntry).toInt64());
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
            assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xDropEntry).toInt64())));
            nTargetPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xDropEntry).toInt64())->GetOutlinePos();
        }

        if( MAXLEVEL > m_nOutlineLevel && // Not all layers are displayed.
                        nTargetPos != SwOutlineNodes::npos)
        {
            std::unique_ptr<weld::TreeIter> xNext(m_xTreeView->make_iterator(xDropEntry.get()));
            bool bNext = m_xTreeView->iter_next(*xNext);
            if (bNext)
            {
                assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xNext).toInt64())));
                nTargetPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xNext).toInt64())->GetOutlinePos() - 1;
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
        rPop.set_label(OString::number(800), IsAllExpanded(rContentTree, rEntry) ? SwResId(STR_COLLAPSEALL) : SwResId(STR_EXPANDALL));
        return false;
    }
    return true;
}

static void lcl_SetOutlineContentEntriesSensitivities(SwContentTree* pThis, const weld::TreeView& rContentTree, const weld::TreeIter& rEntry, weld::Menu& rPop)
{
    rPop.set_sensitive(OString::number(TOGGLE_OUTLINE_CONTENT_VISIBILITY), false);
    rPop.set_sensitive(OString::number(HIDE_OUTLINE_CONTENT_VISIBILITY), false);
    rPop.set_sensitive(OString::number(SHOW_OUTLINE_CONTENT_VISIBILITY), false);

    if (!pThis->GetActiveWrtShell()->GetViewOptions()->IsShowOutlineContentVisibilityButton())
        return;

    // todo: multi selection
    if (rContentTree.count_selected_rows() > 1)
        return;

    const SwNodes& rNodes = pThis->GetWrtShell()->GetNodes();
    const SwOutlineNodes& rOutlineNodes = rNodes.GetOutLineNds();
    size_t nOutlinePos = weld::GetAbsPos(rContentTree, rEntry);

    bool bIsRoot = lcl_IsContentType(rEntry, rContentTree);

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

        rPop.set_sensitive(OString::number(HIDE_OUTLINE_CONTENT_VISIBILITY), bHasUnfolded);
        rPop.set_sensitive(OString::number(SHOW_OUTLINE_CONTENT_VISIBILITY), bHasFolded);
    }

    bIsRoot ? rPop.remove(OString::number(TOGGLE_OUTLINE_CONTENT_VISIBILITY))
            : rPop.set_sensitive(OString::number(TOGGLE_OUTLINE_CONTENT_VISIBILITY), true);
}

IMPL_LINK(SwContentTree, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

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

    for(int i = 1; i <= 3; ++i)
        xSubPopOutlineTracking->append_radio(OUString::number(i + 10), m_aContextStrings[IDX_STR_OUTLINE_TRACKING + i]);
    xSubPopOutlineTracking->set_active(OString::number(10 + m_nOutlineTracking), true);

    for (int i = 1; i <= MAXLEVEL; ++i)
        xSubPop1->append_radio(OUString::number(i + 100), OUString::number(i));
    xSubPop1->set_active(OString::number(100 + m_nOutlineLevel), true);

    for (int i=0; i < 3; ++i)
        xSubPop2->append_radio(OUString::number(i + 201), m_aContextStrings[IDX_STR_HYPERLINK + i]);
    xSubPop2->set_active(OString::number(201 + static_cast<int>(GetParentWindow()->GetRegionDropMode())), true);

    // Insert the list of the open files
    sal_uInt16 nId = 301;
    const SwView* pActiveView = ::GetActiveView();
    SwView *pView = SwModule::GetFirstView();
    while (pView)
    {
        OUString sInsert = pView->GetDocShell()->GetTitle();
        if (pView == pActiveView)
        {
            sInsert += "(" +
                m_aContextStrings[IDX_STR_ACTIVE] +
                ")";
        }
        xSubPop3->append_radio(OUString::number(nId), sInsert);
        if (State::CONSTANT == m_eState && m_pActiveShell == &pView->GetWrtShell())
            xSubPop3->set_active(OString::number(nId), true);
        pView = SwModule::GetNextView(pView);
        nId++;
    }
    xSubPop3->append_radio(OUString::number(nId++), m_aContextStrings[IDX_STR_ACTIVE_VIEW]);
    if (m_pHiddenShell)
    {
        OUString sHiddenEntry = m_pHiddenShell->GetView().GetDocShell()->GetTitle() +
            " ( " +
            m_aContextStrings[IDX_STR_HIDDEN] +
            " )";
        xSubPop3->append_radio(OUString::number(nId), sHiddenEntry);
    }

    if (State::ACTIVE == m_eState)
        xSubPop3->set_active(OString::number(--nId), true);
    else if (State::HIDDEN == m_eState)
        xSubPop3->set_active(OString::number(nId), true);

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xEntry.get()))
        xEntry.reset();

    if (State::HIDDEN == m_eState || !xEntry || !lcl_IsContent(*xEntry, *m_xTreeView))
        xPop->remove(OString::number(900)); // go to

    bool bRemovePostItEntries = true;
    bool bRemoveIndexEntries = true;
    bool bRemoveEditEntry = true;
    bool bRemoveUnprotectEntry = true;
    bool bRemoveDeleteEntry = true;
    bool bRemoveRenameEntry = true;
    bool bRemoveSelectEntry = true;
    bool bRemoveToggleExpandEntry = true;
    bool bRemoveChapterEntries = true;
    bool bRemoveSendOutlineEntry = true;

    // Edit only if the shown content is coming from the current view.
    if (State::HIDDEN != m_eState &&
            (State::ACTIVE == m_eState || m_pActiveShell == pActiveView->GetWrtShellPtr())
            && xEntry && lcl_IsContent(*xEntry, *m_xTreeView))
    {
        assert(dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
        const SwContentType* pContType = reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetParent();
        const ContentTypeId nContentType = pContType->GetType();
        const bool bReadonly = m_pActiveShell->GetView().GetDocShell()->IsReadOnly();
        const bool bVisible = !reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xEntry).toInt64())->IsInvisible();
        const bool bProtected = reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xEntry).toInt64())->IsProtect();
        const bool bProtectBM = (ContentTypeId::BOOKMARK == nContentType)
            && m_pActiveShell->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_BOOKMARKS);
        const bool bEditable = pContType->IsEditable() &&
            ((bVisible && !bProtected && !bProtectBM) || ContentTypeId::REGION == nContentType);
        const bool bDeletable = pContType->IsDeletable() &&
            ((bVisible && !bProtected && !bProtectBM) || ContentTypeId::REGION == nContentType);
        const bool bRenamable = bEditable && !bReadonly &&
            (ContentTypeId::TABLE == nContentType ||
                ContentTypeId::FRAME == nContentType ||
                ContentTypeId::GRAPHIC == nContentType ||
                ContentTypeId::OLE == nContentType ||
                (ContentTypeId::BOOKMARK == nContentType && !bProtectBM) ||
                ContentTypeId::REGION == nContentType ||
                ContentTypeId::INDEX == nContentType ||
                ContentTypeId::DRAWOBJECT == nContentType);

        if(ContentTypeId::OUTLINE == nContentType)
        {
            bOutline = true;
            lcl_SetOutlineContentEntriesSensitivities(this, *m_xTreeView, *xEntry, *xSubPopOutlineContent);
            bRemoveToggleExpandEntry = lcl_InsertExpandCollapseAllItem(*m_xTreeView, *xEntry, *xPop);
            if (!bReadonly)
            {
                bRemoveSelectEntry = false;
                bRemoveChapterEntries = false;
            }
        }
        else if (!bReadonly && (bEditable || bDeletable))
        {
            if(ContentTypeId::INDEX == nContentType)
            {
                bRemoveIndexEntries = false;

                const SwTOXBase* pBase = reinterpret_cast<SwTOXBaseContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetTOXBase();
                if (!pBase->IsTOXBaseInReadonly())
                    bRemoveEditEntry = false;

                xPop->set_active(OString::number(405), SwEditShell::IsTOXBaseReadonly(*pBase));
                bRemoveDeleteEntry = false;
            }
            else if(ContentTypeId::TABLE == nContentType)
            {
                bRemoveSelectEntry = false;
                bRemoveEditEntry = false;
                bRemoveUnprotectEntry = false;
                bool bFull = false;
                OUString sTableName = reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetName();
                bool bProt = m_pActiveShell->HasTableAnyProtection( &sTableName, &bFull );
                xPop->set_sensitive(OString::number(403), !bFull);
                xPop->set_sensitive(OString::number(404), bProt);
                bRemoveDeleteEntry = false;
            }
            else if(ContentTypeId::DRAWOBJECT == nContentType)
            {
                bRemoveDeleteEntry = false;
            }
            else if(ContentTypeId::REGION == nContentType)
            {
                bRemoveSelectEntry = false;
                bRemoveEditEntry = false;
            }
            else
            {
                if (bEditable && bDeletable)
                {
                    bRemoveEditEntry = false;
                    bRemoveDeleteEntry = false;
                }
                else if (bEditable)
                    bRemoveEditEntry = false;
                else if (bDeletable)
                {
                    bRemoveDeleteEntry = false;
                }
            }
            //Rename object
            if (bRenamable)
                bRemoveRenameEntry = false;
        }
    }
    else if (xEntry)
    {
        const SwContentType* pType;
        if (lcl_IsContentType(*xEntry, *m_xTreeView))
            pType = reinterpret_cast<SwContentType*>(m_xTreeView->get_id(*xEntry).toInt64());
        else
            pType = reinterpret_cast<SwContent*>(
                        m_xTreeView->get_id(*xEntry).toInt64())->GetParent();
        if (pType)
        {
            if (ContentTypeId::OUTLINE == pType->GetType())
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
            if (State::HIDDEN != m_eState &&
                    pType->GetType() == ContentTypeId::POSTIT &&
                    !m_pActiveShell->GetView().GetDocShell()->IsReadOnly() &&
                    pType->GetMemberCount() > 0)
                bRemovePostItEntries = false;
        }
    }

    if (bRemoveToggleExpandEntry)
    {
        xPop->remove("separator3");
        xPop->remove(OString::number(800));
    }

    if (bRemoveSelectEntry)
        xPop->remove(OString::number(805));

    if (bRemoveChapterEntries)
    {
        xPop->remove("separator2");
        xPop->remove(OString::number(806));
        xPop->remove(OString::number(801));
        xPop->remove(OString::number(802));
        xPop->remove(OString::number(803));
        xPop->remove(OString::number(804));
    }

    if (bRemoveSendOutlineEntry)
        xPop->remove(OString::number(700));

    if (bRemovePostItEntries)
    {
        xPop->remove(OString::number(600));
        xPop->remove(OString::number(601));
        xPop->remove(OString::number(602));
    }

    if (bRemoveDeleteEntry)
        xPop->remove(OString::number(501));

    if (bRemoveRenameEntry)
        xPop->remove(OString::number(502));

    if (bRemoveIndexEntries)
    {
        xPop->remove(OString::number(401));
        xPop->remove(OString::number(402));
        xPop->remove(OString::number(405));
    }

    if (bRemoveUnprotectEntry)
        xPop->remove(OString::number(404));

    if (bRemoveEditEntry)
        xPop->remove(OString::number(403));

    if (bRemoveToggleExpandEntry &&
        bRemoveSelectEntry &&
        bRemoveChapterEntries &&
        bRemoveSendOutlineEntry &&
        bRemovePostItEntries &&
        bRemoveDeleteEntry &&
        bRemoveRenameEntry &&
        bRemoveIndexEntries &&
        bRemoveUnprotectEntry &&
        bRemoveEditEntry)
    {
        xPop->remove("separator1");
    }

    if (!bOutline)
    {
        xSubPop1.reset();
        xPop->remove(OString::number(1)); // outline level menu
    }
    if (!bOutline || State::HIDDEN == m_eState)
    {
        xSubPopOutlineTracking.reset();
        xPop->remove(OString::number(4)); // outline tracking menu
    }
    if (!bOutline || State::HIDDEN == m_eState ||
            !m_pActiveShell->GetViewOptions()->IsShowOutlineContentVisibilityButton() ||
            m_pActiveShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount() == 0)
    {
        xSubPopOutlineContent.reset();
        xPop->remove(OString::number(5)); // outline content menu
        xPop->remove("separator1511");
    }

    OString sCommand = xPop->popup_at_rect(m_xTreeView.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));
    if (!sCommand.isEmpty())
        ExecuteContextMenuAction(sCommand);

    return true;
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
    bool bChild = m_xTreeView->iter_has_child(rParent);
    if (bChild || !m_xTreeView->get_children_on_demand(rParent))
        return bChild;

    // Is this a content type?
    if (lcl_IsContentType(rParent, *m_xTreeView))
    {
        std::unique_ptr<weld::TreeIter> xChild = m_xTreeView->make_iterator();

        assert(dynamic_cast<SwContentType*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(rParent).toInt64())));
        SwContentType* pCntType = reinterpret_cast<SwContentType*>(m_xTreeView->get_id(rParent).toInt64());

        const size_t nCount = pCntType->GetMemberCount();
        // Add for outline plus/minus
        if (pCntType->GetType() == ContentTypeId::OUTLINE)
        {
            for(size_t i = 0; i < nCount; ++i)
            {
                const SwContent* pCnt = pCntType->GetMember(i);
                if(pCnt)
                {
                    const auto nLevel = static_cast<const SwOutlineContent*>(pCnt)->GetOutlineLevel();
                    OUString sEntry = pCnt->GetName();
                    if(sEntry.isEmpty())
                        sEntry = m_sSpace;
                    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pCnt)));
                    if (!bChild || (nLevel == 0))
                    {
                        insert(&rParent, sEntry, sId, false, xChild.get());
                        m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());
                        m_xTreeView->set_extra_row_indent(*xChild, nLevel + 1 - m_xTreeView->get_iter_depth(*xChild));
                        bChild = true;
                    }
                    else
                    {
                        //back search parent.
                        if(static_cast<const SwOutlineContent*>(pCntType->GetMember(i-1))->GetOutlineLevel() < nLevel)
                        {
                            insert(xChild.get(), sEntry, sId, false, xChild.get());
                            m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());
                            m_xTreeView->set_extra_row_indent(*xChild, nLevel + 1 - m_xTreeView->get_iter_depth(*xChild));
                            bChild = true;
                        }
                        else
                        {
                            bChild = m_xTreeView->iter_previous(*xChild);
                            assert(!bChild || lcl_IsContentType(*xChild, *m_xTreeView) || dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xChild).toInt64())));
                            while (bChild &&
                                    lcl_IsContent(*xChild, *m_xTreeView) &&
                                    (reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xChild).toInt64())->GetOutlineLevel() >= nLevel)
                                )
                            {
                                bChild = m_xTreeView->iter_previous(*xChild);
                            }
                            if (bChild)
                            {
                                insert(xChild.get(), sEntry, sId, false, xChild.get());
                                m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());
                                m_xTreeView->set_extra_row_indent(*xChild, nLevel + 1 - m_xTreeView->get_iter_depth(*xChild));
                            }
                        }
                    }
                }
            }
        }
        else
        {
            bool bRegion = pCntType->GetType() == ContentTypeId::REGION;
            for(size_t i = 0; i < nCount; ++i)
            {
                const SwContent* pCnt = pCntType->GetMember(i);
                if (pCnt)
                {
                    OUString sEntry = pCnt->GetName();
                    if (sEntry.isEmpty())
                        sEntry = m_sSpace;
                    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pCnt)));
                    insert(&rParent, sEntry, sId, false, xChild.get());
                    m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());
                    if (bRegion)
                        m_xTreeView->set_extra_row_indent(*xChild, static_cast<const SwRegionContent*>(pCnt)->GetRegionLevel());
                    bChild = true;
                }
            }
        }
    }

    return bChild;
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
                const size_t nCount = pPage->GetObjCount();

                for( size_t i=0; i<nCount; ++i )
                {
                    SdrObject* pTemp = pPage->GetObj(i);
                    if( pTemp->GetName() == pCnt->GetName())
                    {
                        pRetObj = pTemp;
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

void SwContentTree::Expand(const weld::TreeIter& rParent, std::vector<std::unique_ptr<weld::TreeIter>>* pNodesToExpand)
{
    if (!(m_xTreeView->iter_has_child(rParent) || m_xTreeView->get_children_on_demand(rParent)))
        return;

    if (!m_bIsRoot
        || (lcl_IsContentType(rParent, *m_xTreeView) &&
            reinterpret_cast<SwContentType*>(m_xTreeView->get_id(rParent).toInt64())->GetType() == ContentTypeId::OUTLINE)
        || (m_nRootType == ContentTypeId::OUTLINE))
    {
        if (lcl_IsContentType(rParent, *m_xTreeView))
        {
            SwContentType* pCntType = reinterpret_cast<SwContentType*>(m_xTreeView->get_id(rParent).toInt64());
            const sal_Int32 nOr = 1 << static_cast<int>(pCntType->GetType()); //linear -> Bitposition
            if (State::HIDDEN != m_eState)
            {
                m_nActiveBlock |= nOr;
                m_pConfig->SetActiveBlock(m_nActiveBlock);
            }
            else
                m_nHiddenBlock |= nOr;
            if (pCntType->GetType() == ContentTypeId::OUTLINE)
            {
                std::map< void*, bool > aCurrOutLineNodeMap;

                SwWrtShell* pShell = GetWrtShell();
                bool bParentHasChild = RequestingChildren(rParent);
                if (pNodesToExpand)
                    pNodesToExpand->emplace_back(m_xTreeView->make_iterator(&rParent));
                if (bParentHasChild)
                {
                    std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(&rParent));
                    bool bChild = m_xTreeView->iter_next(*xChild);
                    while (bChild && lcl_IsContent(*xChild, *m_xTreeView))
                    {
                        if (m_xTreeView->iter_has_child(*xChild))
                        {
                            assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xChild).toInt64())));
                            auto const nPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xChild).toInt64())->GetOutlinePos();
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
        }
        else
        {
            if (lcl_IsContent(rParent, *m_xTreeView))
            {
                SwWrtShell* pShell = GetWrtShell();
                // paranoid assert now that outline type is checked
                assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(rParent).toInt64())));
                auto const nPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(rParent).toInt64())->GetOutlinePos();
                void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
                mOutLineNodeMap[key] = true;
            }
        }
    }

    RequestingChildren(rParent);
    if (pNodesToExpand)
        pNodesToExpand->emplace_back(m_xTreeView->make_iterator(&rParent));
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
        SwContentType* pCntType = reinterpret_cast<SwContentType*>(m_xTreeView->get_id(rParent).toInt64());
        const sal_Int32 nAnd = ~(1 << static_cast<int>(pCntType->GetType()));
        if (State::HIDDEN != m_eState)
        {
            m_nActiveBlock &= nAnd;
            m_pConfig->SetActiveBlock(m_nActiveBlock);
        }
        else
            m_nHiddenBlock &= nAnd;
    }
    else if (lcl_IsContent(rParent, *m_xTreeView))
    {
        SwWrtShell* pShell = GetWrtShell();
        assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(rParent).toInt64())));
        auto const nPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(rParent).toInt64())->GetOutlinePos();
        void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
        mOutLineNodeMap[key] = false;
    }

    return true;
}

// Also on double click will be initially opened only.
IMPL_LINK_NOARG(SwContentTree, ContentDoubleClickHdl, weld::TreeView&, bool)
{
    bool bConsumed = false;

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
            if (State::CONSTANT == m_eState)
            {
                m_pActiveShell->GetView().GetViewFrame()->GetWindow().ToTop();
            }
            //Jump to content type:
            assert(dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
            SwContent* pCnt = reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xEntry).toInt64());
            assert(pCnt && "no UserData");
            GotoContent(pCnt);
            // fdo#36308 don't expand outlines on double-click
            bConsumed = pCnt->GetParent()->GetType() == ContentTypeId::OUTLINE;
        }
    }

    return bConsumed; // false/true == allow/disallow more to be done, i.e. expand/collapse children
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
    OUString sEntryName;  // Name of the entry
    size_t nEntryRelPos = 0; // relative position to their parent
    size_t nOldEntryCount = GetEntryCount();
    sal_Int32 nOldScrollPos = 0;
    if (xOldSelEntry)
    {
        UpdateLastSelType();

        nOldScrollPos = m_xTreeView->vadjustment_get_value();
        sEntryName = m_xTreeView->get_text(*xOldSelEntry);
        std::unique_ptr<weld::TreeIter> xParentEntry = m_xTreeView->make_iterator(xOldSelEntry.get());
        while (m_xTreeView->get_iter_depth(*xParentEntry))
            m_xTreeView->iter_parent(*xParentEntry);
        if (m_xTreeView->get_iter_depth(*xOldSelEntry))
            nEntryRelPos = GetAbsPos(*xOldSelEntry) - GetAbsPos(*xParentEntry);
    }

    clear();

    if (!bActive)
        m_eState = State::HIDDEN;
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
        std::unique_ptr<weld::TreeIter> xSelEntry;
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
                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(rpContentT.get())));
                insert(nullptr, rpContentT->GetName(), sId, bChOnDemand, xEntry.get());
                m_xTreeView->set_image(*xEntry, aImage);

                m_xTreeView->set_sensitive(*xEntry, bChOnDemand);

                if (nCntType == m_nLastSelType)
                    xSelEntry = m_xTreeView->make_iterator(xEntry.get());
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

            (void)m_xTreeView->get_iter_first(*xEntry);
            for (ContentTypeId nCntType : o3tl::enumrange<ContentTypeId>())
            {
                sal_Int32 nExpandOptions = (State::HIDDEN == m_eState)
                                            ? m_nHiddenBlock
                                            : m_nActiveBlock;
                if (nExpandOptions & (1 << static_cast<int>(nCntType)))
                {
                    if (nEntryRelPos && nCntType == m_nLastSelType)
                    {
                        // reselect the entry
                        std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(xEntry.get()));
                        std::unique_ptr<weld::TreeIter> xTemp;
                        sal_uLong nPos = 1;
                        while (m_xTreeView->iter_next(*xChild))
                        {
                            // The old text will be slightly favored
                            if (sEntryName == m_xTreeView->get_text(*xChild) ||
                                nPos == nEntryRelPos)
                            {
                                m_xTreeView->copy_iterator(*xChild, *xSelEntry);
                                break;
                            }
                            xTemp = m_xTreeView->make_iterator(xChild.get());
                            nPos++;
                        }
                        if (!xSelEntry || lcl_IsContentType(*xSelEntry, *m_xTreeView))
                            xSelEntry = std::move(xTemp);
                    }
                }

                (void)m_xTreeView->iter_next_sibling(*xEntry);
            }

            if (!xSelEntry)
            {
                nOldScrollPos = 0;
                xSelEntry = m_xTreeView->make_iterator();
                if (!m_xTreeView->get_iter_first(*xSelEntry))
                    xSelEntry.reset();
            }

            if (xSelEntry)
            {
                m_xTreeView->set_cursor(*xSelEntry);
                Select();
            }
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
            bool bChOnDemand = m_nRootType == ContentTypeId::OUTLINE;
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(rpRootContentT.get())));
            insert(nullptr, rpRootContentT->GetName(), sId, bChOnDemand, xEntry.get());
            m_xTreeView->set_image(*xEntry, aImage);

            if (!bChOnDemand)
            {
                bool bRegion = rpRootContentT->GetType() == ContentTypeId::REGION;

                std::unique_ptr<weld::TreeIter> xChild = m_xTreeView->make_iterator();
                for (size_t i = 0; i < rpRootContentT->GetMemberCount(); ++i)
                {
                    const SwContent* pCnt = rpRootContentT->GetMember(i);
                    if (pCnt)
                    {
                        OUString sEntry = pCnt->GetName();
                        if(sEntry.isEmpty())
                            sEntry = m_sSpace;
                        OUString sSubId(OUString::number(reinterpret_cast<sal_Int64>(pCnt)));
                        insert(xEntry.get(), sEntry, sSubId, false, xChild.get());
                        m_xTreeView->set_sensitive(*xChild, !pCnt->IsInvisible());
                        if (bRegion)
                            m_xTreeView->set_extra_row_indent(*xChild, static_cast<const SwRegionContent*>(pCnt)->GetRegionLevel());
                    }
                }
            }
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

            // reselect the entry
            if (nEntryRelPos)
            {
                std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(xEntry.get()));
                sal_uLong nPos = 1;
                while (m_xTreeView->iter_next(*xChild))
                {
                    // The old text will be slightly favored
                    if (sEntryName == m_xTreeView->get_text(*xChild) || nPos == nEntryRelPos)
                    {
                        xSelEntry = std::move(xChild);
                        break;
                    }
                    nPos++;
                }
                if (xSelEntry)
                {
                    m_xTreeView->set_cursor(*xSelEntry); // unselect all entries, make pSelEntry visible, and select
                    Select();
                }
            }
            else
            {
                m_xTreeView->set_cursor(*xEntry);
                Select();
            }
        }
    }

    if (!m_bIgnoreViewChange && GetEntryCount() == nOldEntryCount)
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
    assert(dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
    SwContent* pCnt = reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xEntry).toInt64());

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
                assert(dynamic_cast<SwContentType*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
                pCntType = reinterpret_cast<SwContentType*>(m_xTreeView->get_id(*xEntry).toInt64());
            }
            else
            {
                assert(dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
                pCntType = reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetParent();
            }
            m_nRootType = pCntType->GetType();
            m_bIsRoot = true;
            Display(State::HIDDEN != m_eState);
            if (m_nRootType == ContentTypeId::OUTLINE)
            {
                m_xTreeView->set_selection_mode(SelectionMode::Multiple);
            }
        }
    }
    else
    {
        m_xTreeView->set_selection_mode(SelectionMode::Single);
        m_nRootType = ContentTypeId::UNKNOWN;
        m_bIsRoot = false;
        FindActiveTypeAndRemoveUserData();
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

    // bVisibilityChanged gets set to true if some element, like a section,
    // changed visibility and should have its name rerendered with a new
    // grayed-out state
    bool bVisibilityChanged = false;

    if (State::HIDDEN == m_eState)
    {
        for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
        {
            if(m_aActiveContentArr[i])
                m_aActiveContentArr[i]->Invalidate();
        }
    }
    // root content navigation view
    else if(m_bIsRoot)
    {
        std::unique_ptr<weld::TreeIter> xRootEntry(m_xTreeView->make_iterator());
        if (!m_xTreeView->get_iter_first(*xRootEntry))
            bContentChanged = true;
        else
        {
            assert(dynamic_cast<SwContentType*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xRootEntry).toInt64())));
            const ContentTypeId nType = reinterpret_cast<SwContentType*>(m_xTreeView->get_id(*xRootEntry).toInt64())->GetType();
            SwContentType* pArrType = m_aActiveContentArr[nType].get();
            if (!pArrType)
                bContentChanged = true;
            else
            {
                // start check if first selected outline level has changed
                bool bCheckChanged = m_nRootType == ContentTypeId::OUTLINE && !m_xTreeView->has_focus();
                if (bCheckChanged)
                {
                    std::unique_ptr<weld::TreeIter> xFirstSel(m_xTreeView->make_iterator());
                    bool bFirstSel = m_xTreeView->get_selected(xFirstSel.get());
                    if (bFirstSel && lcl_IsContent(*xFirstSel, *m_xTreeView))
                    {
                        assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xFirstSel).toInt64())));
                        const auto nSelLevel = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xFirstSel).toInt64())->GetOutlineLevel();
                        SwWrtShell* pSh = GetWrtShell();
                        const SwOutlineNodes::size_type nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
                        if (nOutlinePos != SwOutlineNodes::npos && pSh->getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos) != nSelLevel)
                            bContentChanged = true;
                    }
                }
                // end check if first selected outline level has changed

                pArrType->Init(&bVisibilityChanged);
                pArrType->FillMemberList();
                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pArrType)));
                m_xTreeView->set_id(*xRootEntry, sId);
                if (!bContentChanged)
                {
                    const size_t nChildCount = GetChildCount(*xRootEntry);
                    if (nChildCount != pArrType->GetMemberCount())
                        bContentChanged = true;
                    else
                    {
                        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(xRootEntry.get()));
                        for (size_t j = 0; j < nChildCount; ++j)
                        {
                            m_xTreeView->iter_next(*xEntry);
                            const SwContent* pCnt = pArrType->GetMember(j);
                            OUString sSubId(OUString::number(reinterpret_cast<sal_Int64>(pCnt)));
                            m_xTreeView->set_id(*xEntry, sSubId);
                            OUString sEntryText = m_xTreeView->get_text(*xEntry);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
                                bContentChanged = true;
                        }
                    }
                }
            }
        }
    }
    // all content navigation view
    else
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        bool bEntry = m_xTreeView->get_iter_first(*xEntry);
        while (bEntry)
        {
            bool bNext = true; // at least a next must be
            assert(dynamic_cast<SwContentType*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
            SwContentType* pCntType = reinterpret_cast<SwContentType*>(m_xTreeView->get_id(*xEntry).toInt64());
            const size_t nCntCount = pCntType->GetMemberCount();
            const ContentTypeId nType = pCntType->GetType();
            SwContentType* pArrType = m_aActiveContentArr[nType].get();
            if (!pArrType)
                bContentChanged = true;
            else
            {
                pArrType->Init(&bVisibilityChanged);
                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pArrType)));
                m_xTreeView->set_id(*xEntry, sId);
                if (m_xTreeView->get_row_expanded(*xEntry))
                {
                    bool bLevelOrVisibilityChanged = false;
                    // bLevelOrVisibilityChanged is set if outlines have changed their level
                    // or if the visibility of objects (frames, sections, tables) has changed
                    // i.e. in header/footer
                    pArrType->FillMemberList(&bLevelOrVisibilityChanged);
                    const size_t nChildCount = GetChildCount(*xEntry);
                    if (bLevelOrVisibilityChanged)
                    {
                        if (nType == ContentTypeId::OUTLINE)
                            bContentChanged = true;
                        else
                            bVisibilityChanged = true;
                    }

                    if(nChildCount != pArrType->GetMemberCount())
                        bContentChanged = true;
                    else
                    {
                        for(size_t j = 0; j < nChildCount; ++j)
                        {
                            bEntry = m_xTreeView->iter_next(*xEntry);
                            bNext = false;
                            const SwContent* pCnt = pArrType->GetMember(j);
                            OUString sSubId(OUString::number(reinterpret_cast<sal_Int64>(pCnt)));
                            m_xTreeView->set_id(*xEntry, sSubId);
                            OUString sEntryText = m_xTreeView->get_text(*xEntry);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
                                bContentChanged = true;
                        }
                    }
                }
                // not expanded and has children
                else if (m_xTreeView->iter_has_child(*xEntry))
                {
                    // was the entry once opened, then must also the
                    // invisible records be examined.
                    // At least the user data must be updated.
                    bool bLevelOrVisibilityChanged = false;
                    // bLevelOrVisibilityChanged is set if outlines have changed their level
                    // or if the visibility of objects (frames, sections, tables) has changed
                    // i.e. in header/footer
                    pArrType->FillMemberList(&bLevelOrVisibilityChanged);
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
                            OUString sSubId(OUString::number(reinterpret_cast<sal_Int64>(pCnt)));
                            m_xTreeView->set_id(*xChild, sSubId);
                            OUString sEntryText = m_xTreeView->get_text(*xChild);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
                                bRemoveChildren = true;
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
                }
            }
            // The Root-Entry has to be found now
            while (bEntry && (bNext || m_xTreeView->get_iter_depth(*xEntry)))
            {
                bEntry = m_xTreeView->iter_next(*xEntry);
                bNext = false;
            }
        }
    }

    if (!bContentChanged && bVisibilityChanged)
        m_aUpdTimer.Start();

    return bContentChanged || bVisibilityChanged;
}

void SwContentTree::UpdateLastSelType()
{
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (m_xTreeView->get_selected(xEntry.get()))
    {
        while (m_xTreeView->get_iter_depth(*xEntry))
            m_xTreeView->iter_parent(*xEntry);
        sal_Int64 nId = m_xTreeView->get_id(*xEntry).toInt64();
        if (nId && lcl_IsContentType(*xEntry, *m_xTreeView))
        {
            assert(dynamic_cast<SwContentType*>(reinterpret_cast<SwTypeNumber*>(nId)));
            m_nLastSelType = reinterpret_cast<SwContentType*>(nId)->GetType();
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
        if (m_pActiveShell)
            EndListening(*m_pActiveShell->GetView().GetDocShell());
        m_pActiveShell = pSh;
        FindActiveTypeAndRemoveUserData();
        clear();
    }
    else if (State::CONSTANT == m_eState)
    {
        if (m_pActiveShell)
            EndListening(*m_pActiveShell->GetView().GetDocShell());
        m_pActiveShell = pSh;
        m_eState = State::ACTIVE;
        bClear = true;
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
    if (m_pActiveShell)
        EndListening(*m_pActiveShell->GetView().GetDocShell());
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
            if (!m_bIgnoreViewChange)
            {
                m_bViewHasChanged = true;
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

void SwContentTree::ExecCommand(std::string_view rCmd, bool bOutlineWithChildren)
{
    const bool bUp = rCmd == "chapterup";
    const bool bUpDown = bUp || rCmd == "chapterdown";
    const bool bLeft = rCmd == "promote";
    const bool bLeftRight = bLeft || rCmd == "demote";
    if (!bUpDown && !bLeftRight)
        return;
    if (GetWrtShell()->GetView().GetDocShell()->IsReadOnly() ||
        (State::ACTIVE != m_eState &&
         (State::CONSTANT != m_eState || m_pActiveShell != GetParentWindow()->GetCreateView()->GetWrtShellPtr())))
    {
        return;
    }

    m_bIgnoreViewChange = true;

    SwWrtShell *const pShell = GetWrtShell();
    sal_Int8 nActOutlineLevel = m_nOutlineLevel;
    SwOutlineNodes::size_type nActPos = pShell->GetOutlinePos(nActOutlineLevel);

    std::vector<SwTextNode*> selectedOutlineNodes;
    std::vector<std::unique_ptr<weld::TreeIter>> selected;

    m_xTreeView->selected_foreach([this, pShell, &bLeftRight, &bOutlineWithChildren, &selected, &selectedOutlineNodes](weld::TreeIter& rEntry){
        // it's possible to select the root node too which is a really bad idea
        bool bSkip = lcl_IsContentType(rEntry, *m_xTreeView);
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
                    bSkip = true;
                    break;
                }
            }
        }
        if (!bSkip)
        {
            selected.emplace_back(m_xTreeView->make_iterator(&rEntry));
            const SwNodes& rNodes = pShell->GetNodes();
            const size_t nPos = GetAbsPos(rEntry) - 1;
            if (nPos < rNodes.GetOutLineNds().size())
            {
                SwNode* pNode = rNodes.GetOutLineNds()[ nPos ];
                if (pNode)
                {
                    selectedOutlineNodes.push_back(pNode->GetTextNode());
                }
            }
        }
        return false;
    });

    if (bUpDown && !bUp)
    {   // to move down, start at the end!
        std::reverse(selected.begin(), selected.end());
    }

    SwOutlineNodes::difference_type nDirLast = bUp ? -1 : 1;
    bool bStartedAction = false;
    std::vector<SwNode*> aOutlineNdsArray;
    for (auto const& pCurrentEntry : selected)
    {
        assert(pCurrentEntry && lcl_IsContent(*pCurrentEntry, *m_xTreeView));
        if (lcl_IsContent(*pCurrentEntry, *m_xTreeView))
        {
            assert(dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*pCurrentEntry).toInt64())));
            if ((m_bIsRoot && m_nRootType == ContentTypeId::OUTLINE) ||
                reinterpret_cast<SwContent*>(m_xTreeView->get_id(*pCurrentEntry).toInt64())->GetParent()->GetType()
                                            ==  ContentTypeId::OUTLINE)
            {
                nActPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*pCurrentEntry).toInt64())->GetOutlinePos();
            }
        }
        if (nActPos == SwOutlineNodes::npos || (bUpDown && !pShell->IsOutlineMovable(nActPos)))
        {
            continue;
        }

        if (!bStartedAction)
        {
            pShell->StartAllAction();
            if (bUpDown)
            {
                if (pShell->GetViewOptions()->IsShowOutlineContentVisibilityButton())
                {
                    // make all outline nodes content visible before move
                    // restore outline nodes content visible state after move
                    SwOutlineNodes rOutlineNds = pShell->GetDoc()->GetNodes().GetOutLineNds();
                    for (SwOutlineNodes::size_type nPos = 0; nPos < rOutlineNds.size(); ++nPos)
                    {
                        SwNode* pNd = rOutlineNds[nPos];
                        if (pNd->IsTextNode()) // should always be true
                        {
                            bool bOutlineContentVisibleAttr = true;
                            pNd->GetTextNode()->GetAttrOutlineContentVisible(bOutlineContentVisibleAttr);
                            if (!bOutlineContentVisibleAttr)
                            {
                                aOutlineNdsArray.push_back(pNd);
                                pShell->ToggleOutlineContentVisibility(nPos);
                            }
                        }
                    }
                }
            }
            pShell->StartUndo(bLeftRight ? SwUndoId::OUTLINE_LR : SwUndoId::OUTLINE_UD);
            bStartedAction = true;
        }
        pShell->GotoOutline( nActPos); // If text selection != box selection
        pShell->Push();
        pShell->MakeOutlineSel(nActPos, nActPos, bOutlineWithChildren);
        if (bUpDown)
        {
            const size_t nEntryAbsPos(GetAbsPos(*pCurrentEntry));
            SwOutlineNodes::difference_type nDir = bUp ? -1 : 1;
            if (!bOutlineWithChildren && ((nDir == -1 && nActPos > 0) ||
                       (nDir == 1 && nEntryAbsPos < GetEntryCount() - 2)))
            {
                pShell->MoveOutlinePara( nDir );
                // Set cursor back to the current position
                pShell->GotoOutline( nActPos + nDir);
            }
            else if (bOutlineWithChildren)
            {
                SwOutlineNodes::size_type nActEndPos = nActPos;
                std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(pCurrentEntry.get()));
                assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*pCurrentEntry).toInt64())));
                const auto nActLevel = reinterpret_cast<SwOutlineContent*>(
                        m_xTreeView->get_id(*pCurrentEntry).toInt64())->GetOutlineLevel();
                bool bEntry = m_xTreeView->iter_next(*xEntry);
                while (bEntry && lcl_IsContent(*xEntry, *m_xTreeView))
                {
                    assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
                    if (nActLevel >= reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetOutlineLevel())
                        break;
                    nActEndPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetOutlinePos();
                    bEntry = m_xTreeView->iter_next(*xEntry);
                }
                if (nDir == 1) // move down
                {
                    std::unique_ptr<weld::TreeIter> xNextSibling(m_xTreeView->make_iterator(pCurrentEntry.get()));
                    if (m_xTreeView->iter_next_sibling(*xNextSibling) && m_xTreeView->is_selected(*xNextSibling))
                        nDir = nDirLast;
                    else
                    {
                    // If the last entry is to be moved we're done
                    if (bEntry && lcl_IsContent(*xEntry, *m_xTreeView))
                    {
                        // xEntry now points to the entry following the last
                        // selected entry.
                        SwOutlineNodes::size_type nDest = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetOutlinePos();
                        // here needs to found the next entry after next.
                        // The selection must be inserted in front of that.
                        while (bEntry)
                        {
                            bEntry = m_xTreeView->iter_next(*xEntry);
                            assert(!bEntry || !lcl_IsContent(*xEntry, *m_xTreeView)||
                                   dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
                            // nDest++ may only executed if bEntry
                            if (bEntry)
                            {
                                if (!lcl_IsContent(*xEntry, *m_xTreeView))
                                    break;
                                else if (nActLevel >= reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetOutlineLevel())
                                {
                                    // nDest needs adjusted if there are selected entries (including ancestral lineage)
                                    // immediately before the current moved entry.
                                    std::unique_ptr<weld::TreeIter> xTmp(m_xTreeView->make_iterator(xEntry.get()));
                                    bool bTmp = m_xTreeView->iter_previous(*xTmp);
                                    while (bTmp && lcl_IsContent(*xTmp, *m_xTreeView) &&
                                           nActLevel < reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xTmp).toInt64())->GetOutlineLevel())
                                    {
                                        while (bTmp && lcl_IsContent(*xTmp, *m_xTreeView) && !m_xTreeView->is_selected(*xTmp) &&
                                               nActLevel < reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xTmp).toInt64())->GetOutlineLevel())
                                        {
                                            bTmp = m_xTreeView->iter_parent(*xTmp);
                                        }
                                        if (!bTmp || !m_xTreeView->is_selected(*xTmp))
                                            break;
                                        bTmp = m_xTreeView->iter_previous(*xTmp);
                                        nDest = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xTmp).toInt64())->GetOutlinePos();
                                    }
                                    std::unique_ptr<weld::TreeIter> xPrevSibling(m_xTreeView->make_iterator(xEntry.get()));
                                    if (!m_xTreeView->iter_previous_sibling(*xPrevSibling) || !m_xTreeView->is_selected(*xPrevSibling))
                                        break;
                                }
                                else
                                {
                                    nDest = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetOutlinePos();
                                }
                            }
                        }
                        nDirLast = nDir = nDest - nActEndPos;
                        // If no entry was found that allows insertion before
                        // it, we just move it to the end.
                    }
                    else
                        nDirLast = nDir = 0;
                    }
                }
                else // move up
                {
                    std::unique_ptr<weld::TreeIter> xPrevSibling(m_xTreeView->make_iterator(pCurrentEntry.get()));
                    if (m_xTreeView->iter_previous_sibling(*xPrevSibling) && m_xTreeView->is_selected(*xPrevSibling))
                        nDir = nDirLast;
                    else
                    {
                        SwOutlineNodes::size_type nDest = nActPos;
                        bEntry = true;
                        m_xTreeView->copy_iterator(*pCurrentEntry, *xEntry);
                        while (bEntry && nDest)
                        {
                            bEntry = m_xTreeView->iter_previous(*xEntry);
                            assert(!bEntry || !lcl_IsContent(*xEntry, *m_xTreeView) ||
                                   dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
                            if (bEntry && lcl_IsContent(*xEntry, *m_xTreeView))
                            {
                                nDest = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetOutlinePos();
                            }
                            else
                            {
                                nDest = 0; // presumably?
                            }
                            if (bEntry)
                            {
                                if (!lcl_IsContent(*xEntry, *m_xTreeView))
                                    break;
                                else if (nActLevel >= reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetOutlineLevel())
                                {
                                    // nDest needs adjusted if there are selected entries immediately
                                    // after the level change.
                                    std::unique_ptr<weld::TreeIter> xTmp(m_xTreeView->make_iterator(xEntry.get()));
                                    bool bTmp = m_xTreeView->iter_next(*xTmp);
                                    while (bTmp && lcl_IsContent(*xTmp, *m_xTreeView) &&
                                           nActLevel < reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xTmp).toInt64())->GetOutlineLevel() &&
                                           m_xTreeView->is_selected(*xTmp))
                                    {
                                        nDest = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xTmp).toInt64())->GetOutlinePos();
                                        const auto nLevel = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xTmp).toInt64())->GetOutlineLevel();
                                        // account for selected entries' descendent lineage
                                        bTmp = m_xTreeView->iter_next(*xTmp);
                                        while (bTmp && lcl_IsContent(*xTmp, *m_xTreeView) &&
                                               nLevel < reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xTmp).toInt64())->GetOutlineLevel())
                                        {
                                            nDest = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xTmp).toInt64())->GetOutlinePos();
                                            bTmp = m_xTreeView->iter_next(*xTmp);
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        nDirLast = nDir = nDest - nActPos;
                    }
                }
                if (nDir)
                {
                    pShell->MoveOutlinePara( nDir );
                    // Set cursor back to the current position
                    pShell->GotoOutline(nActPos + nDir);
                }
            }
        }
        else
        {
            if (!pShell->IsProtectedOutlinePara())
                pShell->OutlineUpDown(bLeft ? -1 : 1);
        }

        pShell->ClearMark();
        pShell->Pop(SwCursorShell::PopMode::DeleteCurrent); // Cursor is now back at the current heading.
    }

    if (bStartedAction)
    {
        pShell->EndUndo();
        if (bUpDown)
        {
            if (pShell->GetViewOptions()->IsShowOutlineContentVisibilityButton())
            {
                // restore state of outline content visibility to before move
                for (SwNode* pNd : aOutlineNdsArray)
                    pShell->ToggleOutlineContentVisibility(pNd, true);
            }
        }
        pShell->EndAllAction();
        if (m_aActiveContentArr[ContentTypeId::OUTLINE])
            m_aActiveContentArr[ContentTypeId::OUTLINE]->Invalidate();

        // clear all selections to prevent the Display function from trying to reselect selected entries
        m_xTreeView->unselect_all();
        Display(true);

        // reselect entries
        const SwOutlineNodes::size_type nCurrPos = pShell->GetOutlinePos(MAXLEVEL);
        std::unique_ptr<weld::TreeIter> xListEntry(m_xTreeView->make_iterator());
        bool bListEntry = m_xTreeView->get_iter_first(*xListEntry);
        while ((bListEntry = m_xTreeView->iter_next(*xListEntry)) && lcl_IsContent(*xListEntry, *m_xTreeView))
        {
            assert(dynamic_cast<SwOutlineContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xListEntry).toInt64())));
            if (reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xListEntry).toInt64())->GetOutlinePos() == nCurrPos)
            {
                std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(xListEntry.get()));
                if (m_xTreeView->iter_parent(*xParent) && !m_xTreeView->get_row_expanded(*xParent))
                    m_xTreeView->expand_row(*xParent);
                m_xTreeView->set_cursor(*xListEntry); // unselect all entries, make entry visible, set focus, and select
                Select();
                break;
            }
        }

        if (m_bIsRoot)
        {
            const SwOutlineNodes& rOutLineNds = pShell->GetNodes().GetOutLineNds();
            for (SwTextNode* pNode : selectedOutlineNodes)
            {
                SwOutlineNodes::const_iterator aFndIt = rOutLineNds.find(pNode);
                if(aFndIt == rOutLineNds.end())
                    continue;
                const size_t nFndPos = aFndIt - rOutLineNds.begin();
                std::unique_ptr<weld::TreeIter> xEntry = GetEntryAtAbsPos(nFndPos + 1);
                if (xEntry)
                {
                    m_xTreeView->select(*xEntry);
                    std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(xEntry.get()));
                    if (m_xTreeView->iter_parent(*xParent) && !m_xTreeView->get_row_expanded(*xParent))
                        m_xTreeView->expand_row(*xParent);
                }
            }
        }
    }
    m_bIgnoreViewChange = false;
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

static void lcl_SelectByContentTypeAndName(SwContentTree* pThis, weld::TreeView& rContentTree,
                                           std::u16string_view rContentTypeName, std::u16string_view rName)
{
    if (!rName.empty())
    {
        // find content type entry
        std::unique_ptr<weld::TreeIter> xIter(rContentTree.make_iterator());
        bool bFoundEntry = rContentTree.get_iter_first(*xIter);
        while (bFoundEntry && rContentTypeName != rContentTree.get_text(*xIter))
            bFoundEntry = rContentTree.iter_next_sibling(*xIter);
        // find content type content entry and select it
        if (bFoundEntry)
        {
            rContentTree.expand_row(*xIter); // assure content type entry is expanded
            while (rContentTree.iter_next(*xIter) && lcl_IsContent(*xIter, rContentTree))
            {
                if (rName == rContentTree.get_text(*xIter))
                {
                    // get first selected for comparison
                    std::unique_ptr<weld::TreeIter> xFirstSelected(rContentTree.make_iterator());
                    if (!rContentTree.get_selected(xFirstSelected.get()))
                        xFirstSelected.reset();
                    if (rContentTree.count_selected_rows() != 1 ||
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
    }
}

/** No idle with focus or while dragging */
IMPL_LINK_NOARG(SwContentTree, TimerUpdate, Timer *, void)
{
    // No update while focus is not in document.
    // No update while drag and drop.
    // Query view because the Navigator is cleared too late.
    SwView* pView = GetParentWindow()->GetCreateView();
    if(pView && pView->GetWrtShellPtr() && pView->GetWrtShellPtr()->GetWin() &&
        (pView->GetWrtShellPtr()->GetWin()->HasFocus() || m_bViewHasChanged) &&
        !IsInDrag() && !pView->GetWrtShellPtr()->ActionPend())
    {
        m_bViewHasChanged = false;
        m_bIsIdleClear = false;
        SwWrtShell* pActShell = pView->GetWrtShellPtr();
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

        UpdateTracking();
    }
    else if (!pView && State::ACTIVE == m_eState && !m_bIsIdleClear)
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

    // m_bIgnoreViewChange is set on delete
    if (m_bIgnoreViewChange)
    {
        m_bIgnoreViewChange = false;
        return;
    }

    // drawing
    if ((m_pActiveShell->GetSelectionType() & (SelectionType::DrawObject |
                                               SelectionType::DrawObjectEditMode |
                                               SelectionType::DbForm)) &&
            !(m_bIsRoot && m_nRootType != ContentTypeId::DRAWOBJECT))
    {
        SdrView* pSdrView = m_pActiveShell->GetDrawView();
        if(pSdrView && 1 == pSdrView->GetMarkedObjectCount())
        {
            SdrObject* pSelected = pSdrView->GetMarkedObjectByIndex(0);
            OUString aName(pSelected->GetName());
            if (!aName.isEmpty())
                lcl_SelectByContentTypeAndName(this, *m_xTreeView,
                                               SwResId(STR_CONTENT_TYPE_DRAWOBJECT), aName);
            else
            {
                // clear treeview selections
                m_xTreeView->unselect_all();
                Select();
            }
        }
        else
        {
            // clear treeview selections
            m_xTreeView->unselect_all();
            Select();
        }
        return;
    }
    // graphic, frame, and ole
    OUString aContentTypeName;
    if (m_pActiveShell->GetSelectionType() == SelectionType::Graphic &&
            !(m_bIsRoot && m_nRootType != ContentTypeId::GRAPHIC))
        aContentTypeName = SwResId(STR_CONTENT_TYPE_GRAPHIC);
    else if (m_pActiveShell->GetSelectionType() == SelectionType::Frame &&
             !(m_bIsRoot && m_nRootType != ContentTypeId::FRAME))
        aContentTypeName = SwResId(STR_CONTENT_TYPE_FRAME);
    else if (m_pActiveShell->GetSelectionType() == SelectionType::Ole &&
             !(m_bIsRoot && m_nRootType != ContentTypeId::OLE))
        aContentTypeName = SwResId(STR_CONTENT_TYPE_OLE);
    if (!aContentTypeName.isEmpty())
    {
        OUString aName(m_pActiveShell->GetFlyName());
        lcl_SelectByContentTypeAndName(this, *m_xTreeView, aContentTypeName, aName);
        return;
    }
    // table
    if (m_pActiveShell->IsCursorInTable()  &&
            !(m_bIsRoot && m_nRootType != ContentTypeId::TABLE))
    {
        if(m_pActiveShell->GetTableFormat())
        {
            OUString aName = m_pActiveShell->GetTableFormat()->GetName();
            lcl_SelectByContentTypeAndName(this, *m_xTreeView, SwResId(STR_CONTENT_TYPE_TABLE),
                                           aName);
        }
        return;
    }
    // outline
    // find out where the cursor is
    const SwOutlineNodes::size_type nActPos = GetWrtShell()->GetOutlinePos(MAXLEVEL);
    if (!((m_bIsRoot && m_nRootType != ContentTypeId::OUTLINE) ||
          m_nOutlineTracking == 3 || nActPos == SwOutlineNodes::npos))
    {
        // assure outline content type is expanded
        // this assumes outline content type is first in treeview
        std::unique_ptr<weld::TreeIter> xFirstEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_iter_first(*xFirstEntry))
            m_xTreeView->expand_row(*xFirstEntry);

        m_xTreeView->all_foreach([this, nActPos](weld::TreeIter& rEntry){
            bool bRet = false;
            if (lcl_IsContent(rEntry, *m_xTreeView) && reinterpret_cast<SwContent*>(
                        m_xTreeView->get_id(rEntry).toInt64())->GetParent()->GetType() ==
                        ContentTypeId::OUTLINE)
            {
                if (reinterpret_cast<SwOutlineContent*>(
                            m_xTreeView->get_id(rEntry).toInt64())->GetOutlinePos() == nActPos)
                {
                    std::unique_ptr<weld::TreeIter> xFirstSelected(
                                m_xTreeView->make_iterator());
                    if (!m_xTreeView->get_selected(xFirstSelected.get()))
                        xFirstSelected.reset();
                    // only select if not already selected or tree has multiple entries selected
                    if (m_xTreeView->count_selected_rows() != 1 ||
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
                                    if (reinterpret_cast<SwContent*>(
                                                m_xTreeView->get_id(*xChildEntry).toInt64())->
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
                    }
                    bRet = true;
                }
            }
            else
            {
                // use of this break assumes outline content type is first in tree
                if (lcl_IsContentType(rEntry, *m_xTreeView) &&
                        reinterpret_cast<SwContentType*>(
                            m_xTreeView->get_id(rEntry).toInt64())->GetType() !=
                        ContentTypeId::OUTLINE)
                    bRet = true;
            }
            return bRet;
        });
    }
    else
    {
        // clear treeview selections
        m_xTreeView->unselect_all();
        Select();
    }
}

void SwContentTree::SelectOutlinesWithSelection()
{
    SwCursor* pFirstCursor = m_pActiveShell->GetSwCursor();
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

    if (!aOutlinePositions.empty())
    {
        // remove duplicates before selecting
        aOutlinePositions.erase(std::unique(aOutlinePositions.begin(), aOutlinePositions.end()),
                                aOutlinePositions.end());

        m_xTreeView->unselect_all();

        for (auto nOutlinePosition : aOutlinePositions)
        {
            m_xTreeView->all_foreach([this, nOutlinePosition](const weld::TreeIter& rEntry){
                if (lcl_IsContent(rEntry, *m_xTreeView) &&
                        reinterpret_cast<SwContent*>(
                        m_xTreeView->get_id(rEntry).toInt64())->GetParent()->GetType() ==
                        ContentTypeId::OUTLINE)
                {
                    if (reinterpret_cast<SwOutlineContent*>(
                            m_xTreeView->get_id(rEntry).toInt64())->GetOutlinePos() ==
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
}

void SwContentTree::MoveOutline(SwOutlineNodes::size_type nTargetPos)
{
    SwWrtShell *const pShell = GetWrtShell();
    pShell->StartAllAction();
    std::vector<SwNode*> aOutlineNdsArray;

    if (pShell->GetViewOptions()->IsShowOutlineContentVisibilityButton())
    {
        // make all outline nodes content visible before move
        // restore outline nodes content visible state after move
        SwOutlineNodes rOutlineNds = pShell->GetDoc()->GetNodes().GetOutLineNds();
        for (SwOutlineNodes::size_type nPos = 0; nPos < rOutlineNds.size(); ++nPos)
        {
            SwNode* pNd = rOutlineNds[nPos];
            if (pNd->IsTextNode()) // should always be true
            {
                bool bOutlineContentVisibleAttr = true;
                pNd->GetTextNode()->GetAttrOutlineContentVisible(bOutlineContentVisibleAttr);
                if (!bOutlineContentVisibleAttr)
                {
                    aOutlineNdsArray.push_back(pNd);
                    pShell->ToggleOutlineContentVisibility(nPos);
                }
            }
        }
    }
    pShell->StartUndo(SwUndoId::OUTLINE_UD);

    SwOutlineNodes::size_type nPrevSourcePos = SwOutlineNodes::npos;
    SwOutlineNodes::size_type nPrevTargetPosOrOffset = SwOutlineNodes::npos;

    bool bFirstMove = true;

    for (const auto& source : m_aDndOutlinesSelected)
    {
        SwOutlineNodes::size_type nSourcePos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*source).toInt64())->GetOutlinePos();

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
    if (pShell->GetViewOptions()->IsShowOutlineContentVisibilityButton())
    {
        // restore state of outline content visibility to before move
        for (SwNode* pNd : aOutlineNdsArray)
            pShell->ToggleOutlineContentVisibility(pNd, true);
    }
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
        else if ((State::ACTIVE == m_eState || (State::CONSTANT == m_eState && pActShell == GetWrtShell())) &&
                    HasContentChanged())
        {
            Display(true);
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
            }
        }
    }
    else if(aCode.GetCode() == KEY_DELETE && 0 == aCode.GetModifier())
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_selected(xEntry.get()) && lcl_IsContent(*xEntry, *m_xTreeView))
        {
            assert(dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64())));
            if (reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xEntry).toInt64())->GetParent()->IsDeletable() &&
                    !m_pActiveShell->GetView().GetDocShell()->IsReadOnly())
            {
                EditEntry(*xEntry, EditEntryMode::DELETE);
            }
        }
    }
    //Make KEY_SPACE has same function as DoubleClick ,
    //and realize multi-selection .
    else if (aCode.GetCode() == KEY_SPACE && 0 == aCode.GetModifier())
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        if (m_xTreeView->get_cursor(xEntry.get()))
        {
            if (State::HIDDEN != m_eState)
            {
                if (State::CONSTANT == m_eState)
                {
                    m_pActiveShell->GetView().GetViewFrame()->GetWindow().ToTop();
                }

                SwContent* pCnt = dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64()));

                if (pCnt && pCnt->GetParent()->GetType() == ContentTypeId::DRAWOBJECT)
                {
                    SdrView* pDrawView = m_pActiveShell->GetDrawView();
                    if (pDrawView)
                    {
                        pDrawView->SdrEndTextEdit();

                        SwDrawModel* pDrawModel = m_pActiveShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
                        SdrPage* pPage = pDrawModel->GetPage(0);
                        const size_t nCount = pPage->GetObjCount();
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
                        for( size_t i=0; i<nCount; ++i )
                        {
                            SdrObject* pTemp = pPage->GetObj(i);
                            bool bMark = pDrawView->IsObjMarked(pTemp);
                            switch( pTemp->GetObjIdentifier() )
                            {
                                case OBJ_GRUP:
                                case OBJ_TEXT:
                                case OBJ_LINE:
                                case OBJ_RECT:
                                case OBJ_CIRC:
                                case OBJ_SECT:
                                case OBJ_CARC:
                                case OBJ_CCUT:
                                case OBJ_POLY:
                                case OBJ_PLIN:
                                case OBJ_PATHLINE:
                                case OBJ_PATHFILL:
                                case OBJ_FREELINE:
                                case OBJ_FREEFILL:
                                case OBJ_PATHPOLY:
                                case OBJ_PATHPLIN:
                                case OBJ_CAPTION:
                                case OBJ_CUSTOMSHAPE:
                                    if( bMark )
                                        hasObjectMarked = true;
                                    break;
                                default:
                                    if ( bMark )
                                    {
                                        SdrPageView* pPV = pDrawView->GetSdrPageView/*GetPageViewPvNum*/(/*0*/);
                                        if (pPV)
                                        {
                                            pDrawView->MarkObj(pTemp, pPV, true);
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
            SwContent* pCnt = dynamic_cast<SwContent*>(reinterpret_cast<SwTypeNumber*>(m_xTreeView->get_id(*xEntry).toInt64()));
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
                        ExecCommand("promote", !aCode.IsShift());
                    else if (aCode.GetCode() == KEY_RIGHT)
                        ExecCommand("demote", !aCode.IsShift());
                    else if (aCode.GetCode() == KEY_UP)
                        ExecCommand("chapterup", !aCode.IsShift());
                    else if (aCode.GetCode() == KEY_DOWN)
                        ExecCommand("chapterdown", !aCode.IsShift());
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
    void* pUserData = reinterpret_cast<void*>(m_xTreeView->get_id(rEntry).toInt64());
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
                        SwPaM aPaM(*pIdx, *pIdx->GetNode().EndOfSectionNode());
                        SwDoc::CountWords(aPaM, aDocStat);
                        sEntry = SwResId(STR_REGION_DEFNAME) + ": " + sEntry + "\n" +
                                 SwResId(FLD_STAT_WORD) + ": " + OUString::number(aDocStat.nWord) + "\n" +
                                 SwResId(FLD_STAT_CHAR) + ": " + OUString::number(aDocStat.nChar);
                        break;
                    }
                }
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
        const size_t nMemberCount = static_cast<SwContentType*>(pUserData)->GetMemberCount();
        sEntry = OUString::number(nMemberCount) + " " +
            (nMemberCount == 1
                    ? static_cast<SwContentType*>(pUserData)->GetSingleName()
                    : static_cast<SwContentType*>(pUserData)->GetName());
    }

    return sEntry;
}

void SwContentTree::ExecuteContextMenuAction(const OString& rSelectedPopupEntry)
{
    std::unique_ptr<weld::TreeIter> xFirst(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xFirst.get()))
        xFirst.reset();

    auto nSelectedPopupEntry = rSelectedPopupEntry.toUInt32();
    switch (nSelectedPopupEntry)
    {
        case TOGGLE_OUTLINE_CONTENT_VISIBILITY:
        case HIDE_OUTLINE_CONTENT_VISIBILITY:
        case SHOW_OUTLINE_CONTENT_VISIBILITY:
        {
            m_pActiveShell->EnterStdMode();
            m_bIgnoreViewChange = true;
            SwOutlineContent* pCntFirst = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(*xFirst).toInt64());
            if (nSelectedPopupEntry == TOGGLE_OUTLINE_CONTENT_VISIBILITY)
            {
                m_pActiveShell->ToggleOutlineContentVisibility(pCntFirst->GetOutlinePos());
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
                        m_pActiveShell->ToggleOutlineContentVisibility(nPos);
                } while (++nPos < nOutlineNodesCount
                         && (nLevel == -1 || m_pActiveShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos) > nLevel));
            }
            // show in the document what was toggled
            if (lcl_IsContentType(*xFirst, *m_xTreeView)) // Headings root entry
                m_pActiveShell->GotoPage(1, true);
            else
                GotoContent(pCntFirst);
            grab_focus();
            m_bIgnoreViewChange = false;
        }
        break;
        case 11:
        case 12:
        case 13:
            nSelectedPopupEntry -= 10;
            if(m_nOutlineTracking != nSelectedPopupEntry)
                m_nOutlineTracking = nSelectedPopupEntry;
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
            const SwTOXBase* pBase = reinterpret_cast<SwTOXBaseContent*>(m_xTreeView->get_id(*xFirst).toInt64())
                                                                ->GetTOXBase();
            m_pActiveShell->SetTOXBaseReadonly(*pBase, !SwEditShell::IsTOXBaseReadonly(*pBase));
        }
        break;
        case 4:
        break;
        case 501:
            EditEntry(*xFirst, EditEntryMode::DELETE);
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
                m_pActiveShell->GetView().GetViewFrame()->GetDispatcher()->Execute(FN_OUTLINE_TO_CLIPBOARD);
                break;
            }
        case 800:
            ExpandOrCollapseAll(*m_xTreeView, *xFirst);
            break;
        case 801:
            ExecCommand("chapterup", true);
            break;
        case 802:
            ExecCommand("chapterdown", true);
            break;
        case 803:
            ExecCommand("promote", true);
            break;
        case 804:
            ExecCommand("demote", true);
            break;
        case 805:
        {
            m_pActiveShell->KillPams();
            m_pActiveShell->ClearMark();
            m_pActiveShell->EnterAddMode();
            SwContent* pCnt = reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xFirst).toInt64());
            const ContentTypeId eTypeId = pCnt->GetParent()->GetType();
            if (eTypeId == ContentTypeId::OUTLINE)
            {
                m_xTreeView->selected_foreach([this](weld::TreeIter& rEntry){
                    m_pActiveShell->SttSelect();
                    SwOutlineNodes::size_type nActPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(rEntry).toInt64())->GetOutlinePos();
                    m_pActiveShell->MakeOutlineSel(nActPos, nActPos, !m_xTreeView->get_row_expanded(rEntry), false); // select children if not expanded
                    m_pActiveShell->EndSelect();
                    return false;
                });
            }
            else if (eTypeId == ContentTypeId::TABLE)
            {
                m_pActiveShell->GotoTable(pCnt->GetName());
                m_pActiveShell->SelAll();
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
        case 806:
            // Delete outline selections
            EditEntry(*xFirst, EditEntryMode::DELETE);
            break;
        case 900:
        {
            SwContent* pCnt = reinterpret_cast<SwContent*>(m_xTreeView->get_id(*xFirst).toInt64());
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
        }
    }
    GetParentWindow()->UpdateListBox();
}

void SwContentTree::DeleteOutlineSelections()
{
    m_pActiveShell->StartAction();
    m_pActiveShell->EnterAddMode();
    auto nChapters(0);

    m_xTreeView->selected_foreach([this, &nChapters](weld::TreeIter& rEntry){
        ++nChapters;
        if (m_xTreeView->iter_has_child(rEntry) &&
            !m_xTreeView->get_row_expanded(rEntry)) // only count children if not expanded
        {
            nChapters += m_xTreeView->iter_n_children(rEntry);
        }
        m_pActiveShell->SttSelect();
        SwOutlineNodes::size_type nActPos = reinterpret_cast<SwOutlineContent*>(m_xTreeView->get_id(rEntry).toInt64())->GetOutlinePos();
        m_pActiveShell->MakeOutlineSel(nActPos, nActPos, !m_xTreeView->get_row_expanded(rEntry), false); // select children if not expanded
        m_pActiveShell->EndSelect();
        return false;
    });
    m_pActiveShell->LeaveAddMode();
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_CHAPTERS, nChapters));
    m_pActiveShell->StartUndo(SwUndoId::DELETE, &aRewriter);
    m_pActiveShell->SetTextFormatColl(nullptr);
    m_pActiveShell->Delete();
    m_pActiveShell->ClearMark();
    m_pActiveShell->EndUndo();
    m_pActiveShell->EndAction();
}

void SwContentTree::SetOutlineLevel(sal_uInt8 nSet)
{
    m_nOutlineLevel = nSet;
    m_pConfig->SetOutlineLevel( m_nOutlineLevel );
    std::unique_ptr<SwContentType>& rpContentT = (State::ACTIVE == m_eState)
            ? m_aActiveContentArr[ContentTypeId::OUTLINE]
            : m_aHiddenContentArr[ContentTypeId::OUTLINE];
    if(rpContentT)
    {
        rpContentT->SetOutlineLevel(m_nOutlineLevel);
        rpContentT->Init();
    }
    Display(State::ACTIVE == m_eState);
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
    Select();
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
                    reinterpret_cast<SwContentType*>(m_xTreeView->get_id(*xParentEntry).toInt64())->GetType() == ContentTypeId::OUTLINE))
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
    SwContent* pCnt = reinterpret_cast<SwContent*>(m_xTreeView->get_id(rEntry).toInt64());
    GotoContent(pCnt);
    const ContentTypeId nType = pCnt->GetParent()->GetType();
    sal_uInt16 nSlot = 0;

    if(EditEntryMode::DELETE == nMode)
        m_bIgnoreViewChange = true;

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
                m_pActiveShell->GetView().GetViewFrame()->GetDispatcher()->Execute(FN_TABLE_SELECT_ALL);
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
            assert(!m_pActiveShell->getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_BOOKMARKS));
            if(nMode == EditEntryMode::DELETE)
            {
                IDocumentMarkAccess* const pMarkAccess = m_pActiveShell->getIDocumentMarkAccess();
                pMarkAccess->deleteMark( pMarkAccess->findMark(pCnt->GetName()) );
            }
            else if(nMode == EditEntryMode::RENAME)
            {
                uno::Reference< frame::XModel >  xModel = m_pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XBookmarksSupplier >  xBkms(xModel, uno::UNO_QUERY);
                xNameAccess = xBkms->getBookmarks();
            }
            else
                nSlot = FN_INSERT_BOOKMARK;
        break;

        case ContentTypeId::REGION    :
            if(nMode == EditEntryMode::RENAME)
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
            nSlot = FN_EDIT_FIELD;
        break;

        case ContentTypeId::POSTIT:
            m_pActiveShell->GetView().GetPostItMgr()->AssureStdModeAtShell();
            if(nMode == EditEntryMode::DELETE)
            {
                m_pActiveShell->GetView().GetPostItMgr()->SetActiveSidebarWin(nullptr);
                m_pActiveShell->DelRight();
            }
            else
            {
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
                        m_pActiveShell->GetView().GetViewFrame()->
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
        break;
        default: break;
    }
    if(nSlot)
        m_pActiveShell->GetView().GetViewFrame()->
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
        m_bViewHasChanged = true;
        GetParentWindow()->UpdateListBox();
        TimerUpdate(&m_aUpdTimer);
        grab_focus();
    }
}

static void lcl_AssureStdModeAtShell(SwWrtShell* pWrtShell)
{
    // deselect any drawing or frame and leave editing mode
    SdrView* pSdrView = pWrtShell->GetDrawView();
    if (pSdrView && pSdrView->IsTextEdit() )
    {
        bool bLockView = pWrtShell->IsViewLocked();
        pWrtShell->LockView(true);
        pWrtShell->EndTextEdit();
        pWrtShell->LockView(bLockView);
    }

    if (pWrtShell->IsSelFrameMode() || pWrtShell->IsObjSelected())
    {
        pWrtShell->UnSelectFrame();
        pWrtShell->LeaveSelFrameMode();
        pWrtShell->GetView().LeaveDrawCreate();
        pWrtShell->EnterStdMode();
        pWrtShell->DrawSelChanged();
        pWrtShell->GetView().StopShellTimer();
    }
    else
        pWrtShell->EnterStdMode();
}

void SwContentTree::GotoContent(const SwContent* pCnt)
{
    lcl_AssureStdModeAtShell(m_pActiveShell);
    switch(pCnt->GetParent()->GetType())
    {
        case ContentTypeId::OUTLINE   :
        {
            m_pActiveShell->GotoOutline(static_cast<const SwOutlineContent*>(pCnt)->GetOutlinePos());
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
            m_pActiveShell->GotoMark(pCnt->GetName());
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
                m_pActiveShell->Right( CRSR_SKIP_CHARS, true, 1, false);
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
        default: break;
    }

    if (m_pActiveShell->IsFrameSelected() || m_pActiveShell->IsObjSelected())
    {
        m_pActiveShell->HideCursor();
        m_pActiveShell->EnterSelFrameMode();
    }

    SwView& rView = m_pActiveShell->GetView();
    rView.StopShellTimer();
    rView.GetPostItMgr()->SetActiveSidebarWin(nullptr);
    rView.GetEditWin().GrabFocus();

    // assure visible view area is at cursor position
    if (!m_pActiveShell->IsCursorVisible() && !m_pActiveShell->IsFrameSelected() &&
            !m_pActiveShell->IsObjSelected())
    {
        Point rPoint = m_pActiveShell->GetCursorDocPos();
        rPoint.setX(0);
        rView.SetVisArea(rPoint);
    }
}

// Now even the matching text::Bookmark
NaviContentBookmark::NaviContentBookmark()
    :
    nDocSh(0),
    nDefDrag( RegionMode::NONE )
{
}

NaviContentBookmark::NaviContentBookmark( const OUString &rUrl,
                    const OUString& rDesc,
                    RegionMode nDragType,
                    const SwDocShell* pDocSh ) :
    aUrl( rUrl ),
    aDescr(rDesc),
    nDocSh(reinterpret_cast<sal_IntPtr>(pDocSh)),
    nDefDrag( nDragType )
{
}

void NaviContentBookmark::Copy( TransferDataContainer& rData ) const
{
    rtl_TextEncoding eSysCSet = osl_getThreadTextEncoding();

    OString sStrBuf(OUStringToOString(aUrl, eSysCSet) + OStringChar(NAVI_BOOKMARK_DELIM) +
                    OUStringToOString(aDescr, eSysCSet) + OStringChar(NAVI_BOOKMARK_DELIM) +
                    OString::number(static_cast<int>(nDefDrag)) + OStringChar(NAVI_BOOKMARK_DELIM) +
                    OString::number(nDocSh));
    rData.CopyByteString(SotClipboardFormatId::SONLK, sStrBuf);
}

bool NaviContentBookmark::Paste( TransferableDataHelper& rData )
{
    OUString sStr;
    bool bRet = rData.GetString( SotClipboardFormatId::SONLK, sStr );
    if( bRet )
    {
        sal_Int32 nPos = 0;
        aUrl    = sStr.getToken(0, NAVI_BOOKMARK_DELIM, nPos );
        aDescr  = sStr.getToken(0, NAVI_BOOKMARK_DELIM, nPos );
        nDefDrag= static_cast<RegionMode>( sStr.getToken(0, NAVI_BOOKMARK_DELIM, nPos ).toInt32() );
        nDocSh  = sStr.getToken(0, NAVI_BOOKMARK_DELIM, nPos ).toInt32();
    }
    return bRet;
}

SwNavigationPI* SwContentTree::GetParentWindow()
{
    return m_xDialog;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
