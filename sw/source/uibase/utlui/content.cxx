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
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
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
#include <txtatr.hxx>
#include <IMark.hxx>
#include <section.hxx>
#include <tox.hxx>
#include <navipi.hxx>
#include <navicont.hxx>
#include <navicfg.hxx>
#include <edtwin.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentOutlineNodes.hxx>
#include <unotools.hxx>
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
#include <svx/svdogrp.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <vcl/scrbar.hxx>
#include <SwRewriter.hxx>
#include <hints.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <fmtcntnt.hxx>
#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <vcl/treelistentry.hxx>

#include <swabstdlg.hxx>
#include <globals.hrc>
#include <bitmaps.hlst>
#include <unomid.h>

#include <navmgr.hxx>
#include <AnnotationWin.hxx>
#include <memory>

#define CTYPE_CNT   0
#define CTYPE_CTT   1

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

#define NAVI_BOOKMARK_DELIM     u'\x0001'

class SwContentArr
    : public o3tl::sorted_vector<std::unique_ptr<SwContent>, o3tl::less_uniqueptr_to<SwContent>,
                o3tl::find_partialorder_ptrequals>
{
};

bool SwContentTree::bIsInDrag = false;

namespace
{
    bool lcl_IsContent(const SvTreeListEntry* pEntry)
    {
        return static_cast<const SwTypeNumber*>(pEntry->GetUserData())->GetTypeId() == CTYPE_CNT;
    }

    bool lcl_IsContentType(const SvTreeListEntry* pEntry)
    {
        return static_cast<const SwTypeNumber*>(pEntry->GetUserData())->GetTypeId() == CTYPE_CTT;
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

SwContent::SwContent(const SwContentType* pCnt, const OUString& rName, long nYPos) :
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
            m_bDelete = false;
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
            m_bEdit = true;
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
                (eTmpType = pFormat->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
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
            m_bDelete = false;
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
                const sal_Int8 nLevel = static_cast<sal_Int8>(m_pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(i));
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

                if(nOldMemberCount > i &&
                    (*pOldMember)[i]->IsInvisible() != pCnt->IsInvisible())
                        *pbLevelOrVisibilityChanged = true;
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
                if (nOldMemberCount > i &&
                    (*pOldMember)[i]->IsInvisible() != pCnt->IsInvisible())
                        *pbLevelOrVisibilityChanged = true;
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
                (eTmpType = pFormat->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
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

                if( !pBase->GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
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
                        long nYPos = 0;
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
                        if (nOldMemberCount > i &&
                            (*pOldMember)[i]->IsInvisible() != pCnt->IsInvisible() )
                                *pbLevelOrVisibilityChanged = true;
                    }
                }
            }
        }
        break;
        default: break;
    }
    m_bDataValid = true;
}

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
    IDX_STR_SEND_OUTLINE_TO_CLIPBOARD_ENTRY = 12
};

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
    STR_SEND_OUTLINE_TO_CLIPBOARD_ENTRY
};

SwContentTree::SwContentTree(vcl::Window* pParent, SwNavigationPI* pDialog)
    : SvTreeListBox(pParent)
    , m_xDialog(pDialog)
    , m_sSpace(OUString("                    "))
    , m_sRemoveIdx(SwResId(STR_REMOVE_INDEX))
    , m_sUpdateIdx(SwResId(STR_UPDATE))
    , m_sUnprotTable(SwResId(STR_REMOVE_TBL_PROTECTION))
    , m_sRename(SwResId(STR_RENAME))
    , m_sReadonlyIdx(SwResId(STR_READONLY_IDX))
    , m_sInvisible(SwResId(STR_INVISIBLE))
    , m_sPostItShow(SwResId(STR_POSTIT_SHOW))
    , m_sPostItHide(SwResId(STR_POSTIT_HIDE))
    , m_sPostItDelete(SwResId(STR_POSTIT_DELETE))
    , m_pHiddenShell(nullptr)
    , m_pActiveShell(nullptr)
    , m_pConfig(SW_MOD()->GetNavigationConfig())
    , m_nActiveBlock(0)
    , m_nHiddenBlock(0)
    , m_nRootType(ContentTypeId::UNKNOWN)
    , m_nLastSelType(ContentTypeId::UNKNOWN)
    , m_nOutlineLevel(MAXLEVEL)
    , m_eState(State::ACTIVE)
    , m_bDocChgdInDragging(false)
    , m_bIsInternalDrag(false)
    , m_bIsRoot(false)
    , m_bIsIdleClear(false)
    , m_bIsLastReadOnly(false)
    , m_bIsOutlineMoveable(true)
    , m_bViewHasChanged(false)
    , m_bIsKeySpace(false)
{
    SetHelpId(HID_NAVIGATOR_TREELIST);

    SetNodeDefaultImages();
    SetDoubleClickHdl(LINK(this, SwContentTree, ContentDoubleClickHdl));
    SetDragDropMode(DragDropMode::APP_COPY);
    for (ContentTypeId i : o3tl::enumrange<ContentTypeId>())
    {
        m_aActiveContentArr[i]    = nullptr;
        m_aHiddenContentArr[i]    = nullptr;
    }
    for (int i = 0; i < CONTEXT_COUNT; ++i)
    {
        m_aContextStrings[i] = SwResId(STR_CONTEXT_ARY[i]);
    }
    m_nActiveBlock = m_pConfig->GetActiveBlock();
    m_aUpdTimer.SetInvokeHandler(LINK(this, SwContentTree, TimerUpdate));
    m_aUpdTimer.SetTimeout(1000);
    Clear();
    EnableContextMenuHandling();
    SetQuickSearch(true);
}

SwContentTree::~SwContentTree()
{
    disposeOnce();
}

void SwContentTree::dispose()
{
    Clear(); // If applicable erase content types previously.
    bIsInDrag = false;
    m_aUpdTimer.Stop();
    SetActiveShell(nullptr);
    m_xDialog.clear();
    SvTreeListBox::dispose();
}

Size SwContentTree::GetOptimalSize() const
{
    return LogicToPixel(Size(110, 112), MapMode(MapUnit::MapAppFont));
}

OUString SwContentTree::GetEntryAltText( SvTreeListEntry* pEntry ) const
{
    if (pEntry == nullptr || !lcl_IsContent(pEntry))
        return OUString();

    assert(pEntry->GetUserData() == nullptr || dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
    SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());
    if( pCnt == nullptr || pCnt->GetParent() == nullptr)
        return OUString();

    ContentTypeId nJumpType = pCnt->GetParent()->GetType();
    SdrObject* pTemp;

    switch(nJumpType)
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
                        pTemp = pPage->GetObj(i);
                        sal_uInt16 nCmpId;
                        switch( pTemp->GetObjIdentifier() )
                        {
                        case OBJ_GRUP:
                        case OBJ_TEXT:
                        case OBJ_LINE:
                        case OBJ_RECT:
                        case OBJ_CUSTOMSHAPE:
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
                            nCmpId = OBJ_GRUP;
                            break;
                        default:
                            nCmpId = pTemp->GetObjIdentifier();
                        }
                        if(nCmpId == OBJ_GRUP && pTemp->GetName() == pCnt->GetName())
                        {
                            return pTemp->GetTitle();
                        }
                    }
                }
            }
            break;
        case ContentTypeId::GRAPHIC   :
            {
                if( m_pActiveShell && m_pActiveShell->GetDoc() )
                {
                    const SwFlyFrameFormat* pFrameFormat = m_pActiveShell->GetDoc()->FindFlyByName( pCnt->GetName());
                    if( pFrameFormat )
                        return pFrameFormat->GetObjTitle();
                }
            }
            break;
        case ContentTypeId::OLE       :
        case ContentTypeId::FRAME     :
            {
                //Can't find the GetAlternateText function. Need to verify again.
                const SwFlyFrameFormat* pFlyFormat = m_pActiveShell->GetDoc()->FindFlyByName( pCnt->GetName());
                if( pFlyFormat )
                    return pFlyFormat->/*GetAlternateText*/GetName();
            }
            break;
        default: break;
    }
    return OUString();
}

OUString SwContentTree::GetEntryLongDescription( SvTreeListEntry* pEntry ) const
{
    if( pEntry == nullptr)
        return OUString();

    assert(pEntry->GetUserData() == nullptr || dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
    SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());
    if( pCnt == nullptr || pCnt->GetParent() == nullptr)
        return OUString();

    SdrObject* pTemp;

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
                        pTemp = pPage->GetObj(i);
                        sal_uInt16 nCmpId;
                        switch( pTemp->GetObjIdentifier() )
                        {
                        case OBJ_GRUP:
                        case OBJ_TEXT:
                        case OBJ_LINE:
                        case OBJ_RECT:
                        case OBJ_CUSTOMSHAPE:
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
                            nCmpId = OBJ_GRUP;
                            break;
                        default:
                            nCmpId = pTemp->GetObjIdentifier();
                        }
                        if(nCmpId == OBJ_GRUP /*dynamic_cast< const SdrObjGroup *>( pTemp ) !=  nullptr*/ && pTemp->GetName() == pCnt->GetName())
                        {
                            return pTemp->GetDescription();
                        }
                    }
                }
            }
            break;
        case ContentTypeId::GRAPHIC   :
        case ContentTypeId::OLE       :
        case ContentTypeId::FRAME     :
            {
                //Can't find the function "GetLongDescription". Need to verify again.
                const SwFlyFrameFormat* pFlyFormat = m_pActiveShell->GetDoc()->FindFlyByName( pCnt->GetName());
                if( pFlyFormat )
                    return pFlyFormat->GetDescription();
            }
            break;
        default: break;
    }
    return OUString();
}

// Drag&Drop methods

void SwContentTree::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    if( !m_bIsRoot || m_nRootType != ContentTypeId::OUTLINE )
    {
        ReleaseMouse();

        rtl::Reference<TransferDataContainer> pContainer = new TransferDataContainer;

        sal_Int8 nDragMode = DND_ACTION_COPYMOVE | DND_ACTION_LINK;
        if( FillTransferData( *pContainer, nDragMode ))
        {
            SwContentTree::SetInDrag(true);
            pContainer->StartDrag( this, nDragMode, GetDragFinishedHdl() );
        }
    }
    else
        SvTreeListBox::StartDrag( nAction, rPosPixel );
}

void SwContentTree::DragFinished( sal_Int8 nAction )
{
    // To prevent the removing of the selected entry in external drag and drop
    // the drag action mustn't be MOVE.
    SvTreeListBox::DragFinished( m_bIsInternalDrag ? nAction : DND_ACTION_COPY );
    SwContentTree::SetInDrag(false);
    m_bIsInternalDrag = false;
}

// QueryDrop will be executed in the navigator

sal_Int8 SwContentTree::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    if( m_bIsRoot )
    {
        if( m_bIsOutlineMoveable )
            nRet = SvTreeListBox::AcceptDrop( rEvt );
    }
    else if( !bIsInDrag )
        nRet = GetParentWindow()->AcceptDrop();
    return nRet;
}

// Drop will be executed in the navigator

static void* lcl_GetOutlineKey( SwContentTree* pTree, SwOutlineContent const * pContent)
{
    void* key = nullptr;
    if( pTree && pContent )
    {
        SwWrtShell* pShell = pTree->GetWrtShell();
        auto const nPos = pContent->GetOutlinePos();

        key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
    }
    return key;
}

sal_Int8 SwContentTree::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    SvTreeListEntry* pEntry = pTargetEntry;
    if( pEntry && ( m_nRootType == ContentTypeId::OUTLINE ) && lcl_IsContent( pEntry ) )
    {
        assert(pEntry->GetUserData() == nullptr || dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
        SwOutlineContent* pOutlineContent = static_cast<SwOutlineContent*>(pEntry->GetUserData());
        if( pOutlineContent )
        {
            void* key = lcl_GetOutlineKey(this, pOutlineContent);
            if( !mOutLineNodeMap[key] )
            {
                while( pEntry->HasChildren() )
                {
                    SvTreeListEntry* pChildEntry = FirstChild( pEntry );
                    while( pChildEntry )
                    {
                        pEntry = pChildEntry;
                        pChildEntry = pChildEntry->NextSibling();
                    }
                }
                pTargetEntry = pEntry;
            }
        }
    }
    if( m_bIsRoot )
        return SvTreeListBox::ExecuteDrop( rEvt );
    return bIsInDrag ? DND_ACTION_NONE : GetParentWindow()->ExecuteDrop(rEvt);
}

// Handler for Dragging and ContextMenu

VclPtr<PopupMenu> SwContentTree::CreateContextMenu()
{
    auto pPop = VclPtr<PopupMenu>::Create();
    VclPtrInstance<PopupMenu> pSubPop1;
    VclPtrInstance<PopupMenu> pSubPop2;
    VclPtrInstance<PopupMenu> pSubPop3;
    VclPtrInstance<PopupMenu> pSubPop4; // Edit
    bool bSubPop4 = false;

    for(int i = 1; i <= MAXLEVEL; ++i)
    {
        pSubPop1->InsertItem(i + 100, OUString::number(i), MenuItemBits::AUTOCHECK | MenuItemBits::RADIOCHECK);
    }
    pSubPop1->CheckItem(100 + m_nOutlineLevel);
    for(int i=0; i < 3; ++i)
    {
        pSubPop2->InsertItem(i + 201, m_aContextStrings[
                IDX_STR_HYPERLINK + i], MenuItemBits::AUTOCHECK | MenuItemBits::RADIOCHECK);
    }
    pSubPop2->CheckItem(201 + static_cast<int>(GetParentWindow()->GetRegionDropMode()));
    // Insert the list of the open files
    sal_uInt16 nId = 301;
    const SwView* pActiveView = ::GetActiveView();
    SwView *pView = SwModule::GetFirstView();
    while (pView)
    {
        OUString sInsert = pView->GetDocShell()->GetTitle();
        if(pView == pActiveView)
        {
            sInsert += "(";
            sInsert += m_aContextStrings[IDX_STR_ACTIVE];
            sInsert += ")";
        }
        pSubPop3->InsertItem(nId, sInsert, MenuItemBits::AUTOCHECK | MenuItemBits::RADIOCHECK);
        if (State::CONSTANT == m_eState && m_pActiveShell == &pView->GetWrtShell())
            pSubPop3->CheckItem(nId);
        pView = SwModule::GetNextView(pView);
        nId++;
    }
    pSubPop3->InsertItem(nId++, m_aContextStrings[IDX_STR_ACTIVE_VIEW], MenuItemBits::AUTOCHECK | MenuItemBits::RADIOCHECK);
    if(m_pHiddenShell)
    {
        OUString sHiddenEntry = m_pHiddenShell->GetView().GetDocShell()->GetTitle();
        sHiddenEntry += " ( ";
        sHiddenEntry += m_aContextStrings[IDX_STR_HIDDEN];
        sHiddenEntry += " )";
        pSubPop3->InsertItem(nId, sHiddenEntry, MenuItemBits::AUTOCHECK | MenuItemBits::RADIOCHECK);
    }

    if (State::ACTIVE == m_eState)
        pSubPop3->CheckItem( --nId );
    else if (State::HIDDEN == m_eState)
        pSubPop3->CheckItem( nId );

    pPop->InsertItem( 1, m_aContextStrings[IDX_STR_OUTLINE_LEVEL]);
    pPop->InsertItem(2, m_aContextStrings[IDX_STR_DRAGMODE]);
    pPop->InsertItem(3, m_aContextStrings[IDX_STR_DISPLAY]);
    // Now edit
    SvTreeListEntry* pEntry = nullptr;
    // Edit only if the shown content is coming from the current view.
    if ((State::ACTIVE == m_eState || m_pActiveShell == pActiveView->GetWrtShellPtr())
            && nullptr != (pEntry = FirstSelected()) && lcl_IsContent(pEntry))
    {
        assert(dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
        const SwContentType* pContType = static_cast<SwContent*>(pEntry->GetUserData())->GetParent();
        const ContentTypeId nContentType = pContType->GetType();
        bool bReadonly = m_pActiveShell->GetView().GetDocShell()->IsReadOnly();
        bool bVisible = !static_cast<SwContent*>(pEntry->GetUserData())->IsInvisible();
        bool bProtected = static_cast<SwContent*>(pEntry->GetUserData())->IsProtect();
        bool bEditable = pContType->IsEditable() &&
            ((bVisible && !bProtected) ||ContentTypeId::REGION == nContentType);
        bool bDeletable = pContType->IsDeletable() &&
            ((bVisible && !bProtected) ||ContentTypeId::REGION == nContentType);
        bool bRenamable = bEditable && !bReadonly &&
            (ContentTypeId::TABLE == nContentType ||
                ContentTypeId::FRAME == nContentType ||
                ContentTypeId::GRAPHIC == nContentType ||
                ContentTypeId::OLE == nContentType ||
                ContentTypeId::BOOKMARK == nContentType ||
                ContentTypeId::REGION == nContentType||
                ContentTypeId::INDEX == nContentType);

        if(!bReadonly && (bEditable || bDeletable))
        {
            if(ContentTypeId::INDEX == nContentType)
            {
                bSubPop4 = true;
                pSubPop4->InsertItem(401, m_sRemoveIdx);
                pSubPop4->InsertItem(402, m_sUpdateIdx);

                const SwTOXBase* pBase = static_cast<SwTOXBaseContent*>(pEntry->GetUserData())->GetTOXBase();
                if(!pBase->IsTOXBaseInReadonly())
                    pSubPop4->InsertItem(403, m_aContextStrings[IDX_STR_EDIT_ENTRY]);
                pSubPop4->InsertItem(405, m_sReadonlyIdx);

                pSubPop4->CheckItem( 405, SwEditShell::IsTOXBaseReadonly(*pBase));
                pSubPop4->InsertItem(501, m_aContextStrings[IDX_STR_DELETE_ENTRY]);
            }
            else if(ContentTypeId::TABLE == nContentType)
            {
                bSubPop4 = true;
                pSubPop4->InsertItem(403, m_aContextStrings[IDX_STR_EDIT_ENTRY]);
                pSubPop4->InsertItem(404, m_sUnprotTable);
                bool bFull = false;
                OUString sTableName = static_cast<SwContent*>(pEntry->GetUserData())->GetName();
                bool bProt = m_pActiveShell->HasTableAnyProtection( &sTableName, &bFull );
                pSubPop4->EnableItem(403, !bFull );
                pSubPop4->EnableItem(404, bProt );
                pSubPop4->InsertItem(501, m_aContextStrings[IDX_STR_DELETE_ENTRY]);
            }
            else
            {

                if(bEditable && bDeletable)
                {
                    pSubPop4->InsertItem(403, m_aContextStrings[IDX_STR_EDIT_ENTRY]);
                    pSubPop4->InsertItem(501, m_aContextStrings[IDX_STR_DELETE_ENTRY]);
                    bSubPop4 = true;
                }
                else if(bEditable)
                    pPop->InsertItem(403, m_aContextStrings[IDX_STR_EDIT_ENTRY]);
                else if(bDeletable)
                {
                    pSubPop4->InsertItem(501, m_aContextStrings[IDX_STR_DELETE_ENTRY]);
                }
            }
            //Rename object
            if(bRenamable)
            {
                if(bSubPop4)
                    pSubPop4->InsertItem(502, m_sRename);
                else
                    pPop->InsertItem(502, m_sRename);
            }

            if(bSubPop4)
            {
                pPop->InsertItem(4, pContType->GetSingleName());
                pPop->SetPopupMenu(4, pSubPop4);
            }
        }
    }
    else if( pEntry )
    {
        assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
        SwContentType* pType = static_cast<SwContentType*>(pEntry->GetUserData());
        if(ContentTypeId::OUTLINE == pType->GetType())
        {
            pPop->InsertSeparator();
            pPop->InsertItem(700, m_aContextStrings[IDX_STR_SEND_OUTLINE_TO_CLIPBOARD_ENTRY]);
        }
        if ( (pType->GetType() == ContentTypeId::POSTIT) &&  (!m_pActiveShell->GetView().GetDocShell()->IsReadOnly()) && ( pType->GetMemberCount() > 0) )
        {
            bSubPop4 = true;
            pSubPop4->InsertItem(600, m_sPostItShow );
            pSubPop4->InsertItem(601, m_sPostItHide );
            pSubPop4->InsertItem(602, m_sPostItDelete );
            pPop->InsertItem(4, pType->GetSingleName());
            pPop->SetPopupMenu(4, pSubPop4);
        }
    }

    pPop->SetPopupMenu( 1, pSubPop1 );
    pPop->SetPopupMenu( 2, pSubPop2 );
    pPop->SetPopupMenu( 3, pSubPop3 );
    if (!bSubPop4)
        pSubPop4.disposeAndClear();
    return pPop;
}

// Indentation for outlines (and sections)

sal_IntPtr SwContentTree::GetTabPos( SvTreeListEntry* pEntry, SvLBoxTab* pTab)
{
    sal_IntPtr nLevel = 0;
    if(lcl_IsContent(pEntry))
    {
        nLevel++;
        assert(pEntry->GetUserData() == nullptr || dynamic_cast<SwContent *>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
        SwContent* pCnt = static_cast<SwContent *>(pEntry->GetUserData());
        const SwContentType*    pParent;
        if(pCnt &&  nullptr != (pParent = pCnt->GetParent()))
        {
            if(pParent->GetType() == ContentTypeId::OUTLINE)
                nLevel = nLevel + static_cast<SwOutlineContent*>(pCnt)->GetOutlineLevel();
            else if(pParent->GetType() == ContentTypeId::REGION)
                nLevel = nLevel + static_cast<SwRegionContent*>(pCnt)->GetRegionLevel();
        }
    }
    return nLevel * 10 + (m_bIsRoot ? 0 : 5) + pTab->GetPos();  //determined empirically
}

// Content will be integrated into the Box only on demand.

void SwContentTree::RequestingChildren( SvTreeListEntry* pParent )
{
    // Is this a content type?
    if(lcl_IsContentType(pParent))
    {
        if(!pParent->HasChildren())
        {
            assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pParent->GetUserData())));
            SwContentType* pCntType = static_cast<SwContentType*>(pParent->GetUserData());

            const size_t nCount = pCntType->GetMemberCount();
            // Add for outline plus/minus
            if(pCntType->GetType() == ContentTypeId::OUTLINE)
            {
                SvTreeListEntry* pChild = nullptr;
                for(size_t i = 0; i < nCount; ++i)
                {
                    const SwContent* pCnt = pCntType->GetMember(i);
                    if(pCnt)
                    {
                        const auto nLevel = static_cast<const SwOutlineContent*>(pCnt)->GetOutlineLevel();
                        OUString sEntry = pCnt->GetName();
                        if(sEntry.isEmpty())
                            sEntry = m_sSpace;
                        if(!pChild || (nLevel == 0))
                            pChild = InsertEntry(sEntry, pParent, false, TREELIST_APPEND,const_cast<SwContent *>(pCnt));
                        else
                        {
                            //back search parent.
                            if(static_cast<const SwOutlineContent*>(pCntType->GetMember(i-1))->GetOutlineLevel() < nLevel)
                                pChild = InsertEntry(sEntry, pChild, false, TREELIST_APPEND, const_cast<SwContent *>(pCnt));
                            else
                            {
                                pChild = Prev(pChild);
                                assert(!pChild || lcl_IsContentType(pChild) || dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pChild->GetUserData())));
                                while(pChild &&
                                        lcl_IsContent(pChild) &&
                                        (static_cast<SwOutlineContent*>(pChild->GetUserData())->GetOutlineLevel() >= nLevel)
                                    )
                                {
                                    pChild = Prev(pChild);
                                }
                                if(pChild)
                                    pChild = InsertEntry(sEntry, pChild,
                                                false, TREELIST_APPEND, const_cast<SwContent *>(pCnt));
                            }
                        }
                    }
                }
            }
            else
            {
                for(size_t i = 0; i < nCount; ++i)
                {
                    const SwContent* pCnt = pCntType->GetMember(i);
                    if (pCnt)
                    {
                        OUString sEntry = pCnt->GetName();
                        if (sEntry.isEmpty())
                            sEntry = m_sSpace;
                        InsertEntry(sEntry, pParent, false, TREELIST_APPEND, const_cast<SwContent *>(pCnt));
                    }
                }
            }
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

bool  SwContentTree::Expand( SvTreeListEntry* pParent )
{
    if (!m_bIsRoot
        || (lcl_IsContentType(pParent) && static_cast<SwContentType*>(pParent->GetUserData())->GetType() == ContentTypeId::OUTLINE)
        || (m_nRootType == ContentTypeId::OUTLINE))
    {
        if(lcl_IsContentType(pParent))
        {
            SwContentType* pCntType = static_cast<SwContentType*>(pParent->GetUserData());
            const sal_Int32 nOr = 1 << static_cast<int>(pCntType->GetType()); //linear -> Bitposition
            if (State::HIDDEN != m_eState)
            {
                m_nActiveBlock |= nOr;
                m_pConfig->SetActiveBlock(m_nActiveBlock);
            }
            else
                m_nHiddenBlock |= nOr;
            if(pCntType->GetType() == ContentTypeId::OUTLINE)
            {
                std::map< void*, bool > aCurrOutLineNodeMap;

                SwWrtShell* pShell = GetWrtShell();
                bool bBool = SvTreeListBox::Expand(pParent);
                SvTreeListEntry* pChild = Next(pParent);
                while(pChild && lcl_IsContent(pChild) && pParent->HasChildren())
                {
                    if(pChild->HasChildren())
                    {
                        assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pChild->GetUserData())));
                        auto const nPos = static_cast<SwOutlineContent*>(pChild->GetUserData())->GetOutlinePos();
                        void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
                        aCurrOutLineNodeMap.emplace( key, false );
                        std::map<void*, bool>::iterator iter = mOutLineNodeMap.find( key );
                        if( iter != mOutLineNodeMap.end() && mOutLineNodeMap[key])
                        {
                            aCurrOutLineNodeMap[key] = true;
                            SvTreeListBox::Expand(pChild);
                        }
                    }
                    pChild = Next(pChild);
                }
                mOutLineNodeMap = aCurrOutLineNodeMap;
                return bBool;
            }

        }
        else if( lcl_IsContent(pParent) )
        {
            SwWrtShell* pShell = GetWrtShell();
            assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pParent->GetUserData())));
            auto const nPos = static_cast<SwOutlineContent*>(pParent->GetUserData())->GetOutlinePos();
            void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
            mOutLineNodeMap[key] = true;
        }
    }
    return SvTreeListBox::Expand(pParent);
}

bool  SwContentTree::Collapse( SvTreeListEntry* pParent )
{
    if (!m_bIsRoot
        || (lcl_IsContentType(pParent) && static_cast<SwContentType*>(pParent->GetUserData())->GetType() == ContentTypeId::OUTLINE)
        || (m_nRootType == ContentTypeId::OUTLINE))
    {
        if(lcl_IsContentType(pParent))
        {
            if(m_bIsRoot)
                return false;
            SwContentType* pCntType = static_cast<SwContentType*>(pParent->GetUserData());
            const sal_Int32 nAnd = ~(1 << static_cast<int>(pCntType->GetType()));
            if (State::HIDDEN != m_eState)
            {
                m_nActiveBlock &= nAnd;
                m_pConfig->SetActiveBlock(m_nActiveBlock);
            }
            else
                m_nHiddenBlock &= nAnd;
        }
        else if( lcl_IsContent(pParent) )
        {
            SwWrtShell* pShell = GetWrtShell();
            assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pParent->GetUserData())));
            auto const nPos = static_cast<SwOutlineContent*>(pParent->GetUserData())->GetOutlinePos();
            void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
            mOutLineNodeMap[key] = false;
        }
    }

    return SvTreeListBox::Collapse(pParent);
}

// Also on double click will be initially opened only.

IMPL_LINK_NOARG(SwContentTree, ContentDoubleClickHdl, SvTreeListBox*, bool)
{
    SvTreeListEntry* pEntry = GetCurEntry();
    // Is it a content type?
    OSL_ENSURE(pEntry, "no current entry!");
    if(pEntry)
    {
        if(lcl_IsContentType(pEntry) && !pEntry->HasChildren())
        {
            RequestingChildren(pEntry);
        }
        else if (!lcl_IsContentType(pEntry) && (State::HIDDEN != m_eState))
        {
            if (State::CONSTANT == m_eState)
            {
                m_pActiveShell->GetView().GetViewFrame()->GetWindow().ToTop();
            }
            //Jump to content type:
            assert(dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
            SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());
            OSL_ENSURE( pCnt, "no UserData");
            GotoContent(pCnt);
            if(pCnt->GetParent()->GetType() == ContentTypeId::FRAME)
                m_pActiveShell->EnterStdMode();
            return false;   // treelist processing finished
        }
        return true;        // signal more to be done, i.e. expand/collapse children
    }
    return false;
}

namespace
{
    BitmapEx GetBitmapForContentTypeId(ContentTypeId eType)
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

        return BitmapEx(sResId);
    };
}

void SwContentTree::Display( bool bActive )
{
    // First read the selected entry to select it later again if necessary
    // -> the user data here are no longer valid!
    SvTreeListEntry* pOldSelEntry = FirstSelected();
    OUString sEntryName;  // Name of the entry
    sal_uLong nEntryRelPos = 0; // relative position to their parent
    sal_uInt32 nOldEntryCount = GetEntryCount();
    sal_Int32 nOldScrollPos = 0;
    if(pOldSelEntry)
    {
        ScrollBar* pVScroll = GetVScroll();
        if(pVScroll && pVScroll->IsVisible())
            nOldScrollPos = pVScroll->GetThumbPos();

        sEntryName = GetEntryText(pOldSelEntry);
        SvTreeListEntry* pParantEntry = pOldSelEntry;
        while( GetParent(pParantEntry))
        {
            pParantEntry = GetParent(pParantEntry);
        }
        if(GetParent(pOldSelEntry))
        {
            nEntryRelPos = GetModel()->GetAbsPos(pOldSelEntry) - GetModel()->GetAbsPos(pParantEntry);
        }
    }
    Clear();
    SetUpdateMode( false );
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
        pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("up"), !bDisable);
        pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("down"), !bDisable);
        pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("promote"), !bDisable);
        pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("demote"), !bDisable);
        pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("reminder"), !bDisable);
    }
    if(pShell)
    {
        SvTreeListEntry* pSelEntry = nullptr;
        if(m_nRootType == ContentTypeId::UNKNOWN)
        {
            for( ContentTypeId nCntType : o3tl::enumrange<ContentTypeId>() )
            {
                std::unique_ptr<SwContentType>& rpContentT = bActive ?
                                    m_aActiveContentArr[nCntType] :
                                    m_aHiddenContentArr[nCntType];
                if(!rpContentT)
                    rpContentT.reset(new SwContentType(pShell, nCntType, m_nOutlineLevel ));

                OUString sEntry = rpContentT->GetName();
                SvTreeListEntry* pEntry;
                Image aImage(GetBitmapForContentTypeId(nCntType));
                bool bChOnDemand = 0 != rpContentT->GetMemberCount();
                pEntry = InsertEntry(sEntry, aImage, aImage,
                                nullptr, bChOnDemand, TREELIST_APPEND, rpContentT.get());
                if(nCntType == m_nLastSelType)
                    pSelEntry = pEntry;
                sal_Int32 nExpandOptions = (State::HIDDEN == m_eState)
                                            ? m_nHiddenBlock
                                            : m_nActiveBlock;
                if(nExpandOptions & (1 << static_cast<int>(nCntType)))
                {
                    Expand(pEntry);
                    if(nEntryRelPos && nCntType == m_nLastSelType)
                    {
                        // Now maybe select a additional child
                        SvTreeListEntry* pChild = pEntry;
                        SvTreeListEntry* pTemp = nullptr;
                        sal_uLong nPos = 1;
                        while(nullptr != (pChild = Next(pChild)))
                        {
                            // The old text will be slightly favored
                            if(sEntryName == GetEntryText(pChild) ||
                                nPos == nEntryRelPos )
                            {
                                pSelEntry = pChild;
                                break;
                            }
                            pTemp = pChild;
                            nPos++;
                        }
                        if(!pSelEntry || lcl_IsContentType(pSelEntry))
                            pSelEntry = pTemp;
                    }

                }
            }
            if(pSelEntry)
            {
                MakeVisible(pSelEntry);
                Select(pSelEntry);
            }
            else
                nOldScrollPos = 0;
        }
        else
        {
            std::unique_ptr<SwContentType>& rpRootContentT = bActive ?
                                    m_aActiveContentArr[m_nRootType] :
                                    m_aHiddenContentArr[m_nRootType];
            if(!rpRootContentT)
                rpRootContentT.reset(new SwContentType(pShell, m_nRootType, m_nOutlineLevel ));
            Image aImage(GetBitmapForContentTypeId(m_nRootType));
            SvTreeListEntry* pParent = InsertEntry(
                    rpRootContentT->GetName(), aImage, aImage,
                        nullptr, false, TREELIST_APPEND, rpRootContentT.get());

            if(m_nRootType != ContentTypeId::OUTLINE)
            {
                for(size_t i = 0; i < rpRootContentT->GetMemberCount(); ++i)
                {
                    const SwContent* pCnt = rpRootContentT->GetMember(i);
                    if(pCnt)
                    {
                        OUString sEntry = pCnt->GetName();
                        if(sEntry.isEmpty())
                            sEntry = m_sSpace;
                        InsertEntry( sEntry, pParent,
                            false, TREELIST_APPEND, const_cast<SwContent *>(pCnt));
                    }
                }
            }
            else
                RequestingChildren(pParent);
            Expand(pParent);
            if (m_nRootType == ContentTypeId::OUTLINE && State::ACTIVE == m_eState)
            {
                // find out where the cursor is
                const SwOutlineNodes::size_type nActPos = pShell->GetOutlinePos(MAXLEVEL);
                SvTreeListEntry* pEntry = First();

                while( nullptr != (pEntry = Next(pEntry)) )
                {
                    assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
                    if (static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlinePos() == nActPos)
                    {
                        MakeVisible(pEntry);
                        Select(pEntry);
                        SetCurEntry(pEntry);
                    }
                }

            }
            else
            {
                // Now maybe select a additional child
                SvTreeListEntry* pChild = pParent;
                SvTreeListEntry* pTemp = nullptr;
                sal_uLong nPos = 1;
                while(nullptr != (pChild = Next(pChild)))
                {
                    // The old text will be slightly favored
                    if(sEntryName == GetEntryText(pChild) ||
                        nPos == nEntryRelPos )
                    {
                        pSelEntry = pChild;
                        break;
                    }
                    pTemp = pChild;
                    nPos++;
                }
                if(!pSelEntry)
                    pSelEntry = pTemp;
                if(pSelEntry)
                {
                    MakeVisible(pSelEntry);
                    Select(pSelEntry);
                }
            }
        }
    }
    SetUpdateMode( true );
    ScrollBar* pVScroll = GetVScroll();
    if(GetEntryCount() == nOldEntryCount &&
        nOldScrollPos && pVScroll && pVScroll->IsVisible()
        && pVScroll->GetThumbPos() != nOldScrollPos)
    {
        sal_Int32 nDelta = pVScroll->GetThumbPos() - nOldScrollPos;
        ScrollOutputArea( static_cast<short>(nDelta) );
    }
}

void SwContentTree::Clear()
{
    SetUpdateMode(false);
    SvTreeListBox::Clear();
    SetUpdateMode(true);
}

bool SwContentTree::FillTransferData( TransferDataContainer& rTransfer,
                                            sal_Int8& rDragMode )
{
    SwWrtShell* pWrtShell = GetWrtShell();
    OSL_ENSURE(pWrtShell, "no Shell!");
    SvTreeListEntry* pEntry = GetCurEntry();
    if(!pEntry || lcl_IsContentType(pEntry) || !pWrtShell)
        return false;
    OUString sEntry;
    assert(dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
    SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());

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
                        sEntry += OUString::number( nVal - pOutlRule->Get(nLevel).GetStart() );
                        sEntry += ".";
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
            sEntry = GetEntryText(pEntry);
    }

    bool bRet = false;
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
                sUrl += OUStringLiteral1(cMarkSeparator) + rToken;
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
        SvTreeListEntry* pEntry = GetCurEntry();
        const SwContentType* pCntType;
        if(pEntry)
        {
            if(lcl_IsContentType(pEntry))
            {
                assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
                pCntType = static_cast<SwContentType*>(pEntry->GetUserData());
            }
            else
            {
                assert(dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
                pCntType = static_cast<SwContent*>(pEntry->GetUserData())->GetParent();
            }
            m_nRootType = pCntType->GetType();
            m_bIsRoot = true;
            Display(State::HIDDEN != m_eState);
            if (m_nRootType == ContentTypeId::OUTLINE)
            {
                SetSelectionMode(SelectionMode::Multiple);
                SetDragDropMode(DragDropMode::CTRL_MOVE |
                    DragDropMode::CTRL_COPY |
                    DragDropMode::ENABLE_TOP);
            }
        }
    }
    else
    {
        SetSelectionMode(SelectionMode::Single);
        m_nRootType = ContentTypeId::UNKNOWN;
        m_bIsRoot = false;
        FindActiveTypeAndRemoveUserData();
        Display(State::HIDDEN != m_eState);
        if( m_bIsKeySpace )
        {
            HideFocus();
            ShowFocus( m_aOldRectangle);
            m_bIsKeySpace = false;
        }
    }
    m_pConfig->SetRootType( m_nRootType );
    VclPtr<SwNavHelpToolBox> xBox = GetParentWindow()->m_aContentToolBox;
    xBox->CheckItem(xBox->GetItemId("root"), m_bIsRoot);
}

bool SwContentTree::HasContentChanged()
{

//  - Run through the local array and the Treelistbox in parallel.
//  - Are the records not expanded, they are discarded only in the array
//    and the content type will be set as the new UserData.
//  - Is the root mode is active only this will be updated.

//  Valid for the displayed content types is:
//  the Memberlist will be erased and the membercount will be updated
//  If content will be checked, the memberlists will be replenished
//  at the same time. Once a difference occurs it will be only replenished
//  no longer checked. Finally, the box is filled again.

    bool bRepaint = false;
    bool bInvalidate = false;

    if (State::HIDDEN == m_eState)
    {
        for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
        {
            if(m_aActiveContentArr[i])
                m_aActiveContentArr[i]->Invalidate();
        }
    }
    else if(m_bIsRoot)
    {
        bool bOutline = false;
        SvTreeListEntry* pEntry = First();
        if(!pEntry)
            bRepaint = true;
        else
        {
            assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
            const ContentTypeId nType = static_cast<SwContentType*>(pEntry->GetUserData())->GetType();
            bOutline = m_nRootType == ContentTypeId::OUTLINE;
            SwContentType* pArrType = m_aActiveContentArr[nType].get();
            if(!pArrType)
                bRepaint = true;
            else
            {
                SvTreeListEntry* pFirstSel;
                if(bOutline &&
                        nullptr != ( pFirstSel = FirstSelected()) &&
                            lcl_IsContent(pFirstSel))
                {
                    assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pFirstSel->GetUserData())));
                    const auto nSelLevel =
                        static_cast<SwOutlineContent*>(pFirstSel->GetUserData())->GetOutlineLevel();
                    SwWrtShell* pSh = GetWrtShell();
                    const SwOutlineNodes::size_type nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
                    if (nOutlinePos != SwOutlineNodes::npos &&
                        pSh->getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos) != nSelLevel)
                        bRepaint = true;
                }

                pArrType->Init(&bInvalidate);
                pArrType->FillMemberList();
                pEntry->SetUserData(static_cast<void*>(pArrType));
                if(!bRepaint)
                {
                    if(GetChildCount(pEntry) != pArrType->GetMemberCount())
                            bRepaint = true;
                    else
                    {
                        const size_t nChildCount = GetChildCount(pEntry);
                        for(size_t j = 0; j < nChildCount; ++j)
                        {
                            pEntry = Next(pEntry);
                            assert(pEntry);
                            const SwContent* pCnt = pArrType->GetMember(j);
                            pEntry->SetUserData(const_cast<SwContent *>(pCnt));
                            OUString sEntryText = GetEntryText(pEntry);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
                                bRepaint = true;
                        }
                    }
                }
            }
        }
        if( !bRepaint && bOutline && !HasFocus() )
        {
            // find out where the cursor is
            const SwOutlineNodes::size_type nActPos = GetWrtShell()->GetOutlinePos(MAXLEVEL);
            SvTreeListEntry* pFirstEntry = First();

            while( nullptr != (pFirstEntry = Next(pFirstEntry)) )
            {
                assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pFirstEntry->GetUserData())));
                if (static_cast<SwOutlineContent*>(pFirstEntry->GetUserData())->GetOutlinePos() == nActPos)
                {
                    if(FirstSelected() != pFirstEntry)
                    {
                        Select(pFirstEntry);
                        MakeVisible(pFirstEntry);
                    }
                }
                else if (IsSelected(pFirstEntry))
                {
                    SvTreeListBox::SelectListEntry(pFirstEntry, false);
                }
            }
            bInvalidate = true;

        }

    }
    else
    {
        SvTreeListEntry* pEntry = First();
        while ( pEntry )
        {
            bool bNext = true; // at least a next must be
            assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
            SwContentType* pTreeType = static_cast<SwContentType*>(pEntry->GetUserData());
            const size_t nTreeCount = pTreeType->GetMemberCount();
            const ContentTypeId nType = pTreeType->GetType();
            SwContentType* pArrType = m_aActiveContentArr[nType].get();
            if(!pArrType)
                bRepaint = true;
            else
            {
                pArrType->Init(&bInvalidate);
                pEntry->SetUserData(static_cast<void*>(pArrType));
                if(IsExpanded(pEntry))
                {
                    bool bLevelOrVisibiblityChanged = false;
                    // bLevelOrVisibiblityChanged is set if outlines have changed their level
                    // or if the visibility of objects (frames, sections, tables) has changed
                    // i.e. in header/footer
                    pArrType->FillMemberList(&bLevelOrVisibiblityChanged);
                    const size_t nChildCount = GetChildCount(pEntry);
                    if((nType == ContentTypeId::OUTLINE) && bLevelOrVisibiblityChanged)
                        bRepaint = true;
                    if(bLevelOrVisibiblityChanged)
                        bInvalidate = true;

                    if(nChildCount != pArrType->GetMemberCount())
                        bRepaint = true;
                    else
                    {
                        for(size_t j = 0; j < nChildCount; ++j)
                        {
                            pEntry = Next(pEntry);
                            assert(pEntry);
                            bNext = false;
                            const SwContent* pCnt = pArrType->GetMember(j);
                            pEntry->SetUserData(const_cast<SwContent *>(pCnt));
                            OUString sEntryText = GetEntryText(pEntry);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
                                bRepaint = true;
                        }
                    }

                }
                else if(pEntry->HasChildren())
                {
                    // was the entry once opened, then must also the
                    // invisible records be examined.
                    // At least the user data must be updated.
                    bool bLevelOrVisibiblityChanged = false;
                    // bLevelOrVisibiblityChanged is set if outlines have changed their level
                    // or if the visibility of objects (frames, sections, tables) has changed
                    // i.e. in header/footer
                    pArrType->FillMemberList(&bLevelOrVisibiblityChanged);
                    bool bRemoveChildren = false;
                    const size_t nChildCount = GetChildCount(pEntry);
                    if( nChildCount != pArrType->GetMemberCount() )
                    {
                        bRemoveChildren = true;
                    }
                    else
                    {
                        SvTreeListEntry* pChild = FirstChild(pEntry);
                        for(size_t j = 0; j < nChildCount; ++j)
                        {
                            const SwContent* pCnt = pArrType->GetMember(j);
                            assert(pChild);
                            pChild->SetUserData(const_cast<SwContent *>(pCnt));
                            OUString sEntryText = GetEntryText(pChild);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == m_sSpace && pCnt->GetName().isEmpty()))
                                bRemoveChildren = true;
                            pChild = Next(pChild);
                        }
                    }
                    if(bRemoveChildren)
                    {
                        for(size_t j = 0; j < nChildCount; ++j)
                        {
                            SvTreeListEntry *const pRemove = FirstChild(pEntry);
                            assert(pRemove);
                            GetModel()->Remove(pRemove);
                        }
                    }
                    if(!nChildCount)
                    {
                        pEntry->EnableChildrenOnDemand(false);
                        InvalidateEntry(pEntry);
                    }

                }
                else if((nTreeCount != 0)
                            != (pArrType->GetMemberCount()!=0))
                {
                    bRepaint = true;
                }
            }
            // The Root-Entry has to be found now
            while( pEntry && (bNext || GetParent(pEntry ) ))
            {
                pEntry = Next(pEntry);
                bNext = false;
            }
        }
    }
    if(!bRepaint && bInvalidate)
        Invalidate();
    return bRepaint;
}

void SwContentTree::FindActiveTypeAndRemoveUserData()
{
    SvTreeListEntry* pEntry = FirstSelected();
    if(pEntry)
    {
        // If clear is called by TimerUpdate:
        // Only for root can the validity of the UserData be guaranteed.
        SvTreeListEntry* pParent;
        while(nullptr != (pParent = GetParent(pEntry)))
            pEntry = pParent;
        if(pEntry->GetUserData() && lcl_IsContentType(pEntry))
        {
            assert(dynamic_cast<SwContentType*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
            m_nLastSelType = static_cast<SwContentType*>(pEntry->GetUserData())->GetType();
        }
    }
    pEntry = First();
    while(pEntry)
    {
        pEntry->SetUserData(nullptr);
        pEntry = Next(pEntry);
    }
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
    if(m_bIsInternalDrag)
        m_bDocChgdInDragging = true;
    bool bClear = m_pActiveShell != pSh;
    if (State::ACTIVE == m_eState && bClear)
    {
        if (m_pActiveShell)
            EndListening(*m_pActiveShell->GetView().GetDocShell());
        m_pActiveShell = pSh;
        FindActiveTypeAndRemoveUserData();
        Clear();
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
        case SfxHintId::DocChanged:
            if (!(m_bIsRoot && m_nRootType == ContentTypeId::OUTLINE && HasFocus()))
            {
                m_bViewHasChanged = true;
            }
            break;
        case SfxHintId::ModeChanged:
            if (SwWrtShell* pShell = GetWrtShell())
            {
                const bool bReadOnly = pShell->GetView().GetDocShell()->IsReadOnly();
                if (bReadOnly != m_bIsLastReadOnly)
                {
                    m_bIsLastReadOnly = bReadOnly;
                    Select(GetCurEntry());
                }
            }
            break;
        default:
            break;
    }
}



void SwContentTree::ExecCommand(const OUString& rCmd, bool bOutlineWithChildren)
{
    const bool bUp = rCmd == "up";
    const bool bUpDown = bUp || rCmd == "down";
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

    SwWrtShell *const pShell = GetWrtShell();
    sal_Int8 nActOutlineLevel = m_nOutlineLevel;
    SwOutlineNodes::size_type nActPos = pShell->GetOutlinePos(nActOutlineLevel);

    std::vector<SwTextNode*> selectedOutlineNodes;
    std::vector<SvTreeListEntry*> selected;
    for (SvTreeListEntry * pEntry = FirstSelected(); pEntry; pEntry = NextSelected(pEntry))
    {
        // it's possible to select the root node too which is a really bad idea
        bool bSkip = lcl_IsContentType(pEntry);
        // filter out children of selected parents so they don't get promoted
        // or moved twice (except if there is Ctrl modifier, since in that
        // case children are re-parented)
        if ((bLeftRight || bOutlineWithChildren) && !selected.empty())
        {
            for (auto pParent = GetParent(pEntry); pParent; pParent = GetParent(pParent))
            {
                if (selected.back() == pParent)
                {
                    bSkip = true;
                    break;
                }
            }
        }
        if (!bSkip)
        {
            selected.push_back(pEntry);
            const SwNodes& rNodes = pShell->GetNodes();
            const sal_uLong nPos = GetAbsPos(pEntry) - 1;
            if (nPos < rNodes.GetOutLineNds().size())
            {
                SwNode* pNode = rNodes.GetOutLineNds()[ nPos ];
                if (pNode)
                {
                    selectedOutlineNodes.push_back(pNode->GetTextNode());
                }
            }
        }
    }
    if (bUpDown && !bUp)
    {   // to move down, start at the end!
        std::reverse(selected.begin(), selected.end());
    }

    SwOutlineNodes::difference_type nDirLast = bUp ? -1 : 1;
    bool bStartedAction = false;
    for (auto const pCurrentEntry : selected)
    {
        assert(pCurrentEntry && lcl_IsContent(pCurrentEntry));
        if (pCurrentEntry && lcl_IsContent(pCurrentEntry))
        {
            assert(dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pCurrentEntry->GetUserData())));
            if ((m_bIsRoot && m_nRootType == ContentTypeId::OUTLINE) ||
                static_cast<SwContent*>(pCurrentEntry->GetUserData())->GetParent()->GetType()
                                            ==  ContentTypeId::OUTLINE)
            {
                nActPos = static_cast<SwOutlineContent*>(pCurrentEntry->GetUserData())->GetOutlinePos();
            }
        }
        if (nActPos == SwOutlineNodes::npos || (bUpDown && !pShell->IsOutlineMovable(nActPos)))
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
        pShell->MakeOutlineSel(nActPos, nActPos, bOutlineWithChildren);
        if (bUpDown)
        {
            sal_uLong const nEntryAbsPos(GetModel()->GetAbsPos(pCurrentEntry));
            SwOutlineNodes::difference_type nDir = bUp ? -1 : 1;
            if (!bOutlineWithChildren && ((nDir == -1 && nActPos > 0) ||
                       (nDir == 1 && nEntryAbsPos < GetEntryCount() - 2)))
            {
                pShell->MoveOutlinePara( nDir );
                // Set cursor back to the current position
                pShell->GotoOutline( nActPos + nDir);
            }
            else if (bOutlineWithChildren && pCurrentEntry)
            {
                SwOutlineNodes::size_type nActEndPos = nActPos;
                SvTreeListEntry* pEntry = pCurrentEntry;
                assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pCurrentEntry->GetUserData())));
                const auto nActLevel = static_cast<SwOutlineContent*>(
                        pCurrentEntry->GetUserData())->GetOutlineLevel();
                pEntry = Next(pEntry);
                while (pEntry && lcl_IsContent(pEntry))
                {
                    assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
                    if (nActLevel >= static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlineLevel())
                        break;
                    nActEndPos = static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlinePos();
                    pEntry = Next(pEntry);
                }
                if (nDir == 1) // move down
                {
                    if (pCurrentEntry && IsSelected(pCurrentEntry->NextSibling()))
                        nDir = nDirLast;
                    else
                    {
                    // If the last entry is to be moved we're done
                    if (pEntry && lcl_IsContent(pEntry))
                    {
                        // pEntry now points to the entry following the last
                        // selected entry.
                        SwOutlineNodes::size_type nDest = static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlinePos();
                        // here needs to found the next entry after next.
                        // The selection must be inserted in front of that.
                        while (pEntry)
                        {
                            pEntry = Next(pEntry);
                            assert(pEntry == nullptr || !lcl_IsContent(pEntry) || dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
                            // nDest++ may only executed if pEntry != 0
                            if (pEntry)
                            {
                                if (!lcl_IsContent(pEntry))
                                    break;
                                else if (nActLevel >= static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlineLevel())
                                {
                                    // nDest needs adjusted if there are selected entries (including ancestral lineage)
                                    // immediately before the current moved entry.
                                    SvTreeListEntry* pTmp = Prev(pEntry);
                                    while (pTmp && lcl_IsContent(pTmp) &&
                                           nActLevel < static_cast<SwOutlineContent*>(pTmp->GetUserData())->GetOutlineLevel())
                                    {
                                        while (pTmp && lcl_IsContent(pTmp) && !IsSelected(pTmp) &&
                                               nActLevel < static_cast<SwOutlineContent*>(pTmp->GetUserData())->GetOutlineLevel())
                                        {
                                            pTmp = GetParent(pTmp);
                                        }
                                        if (!IsSelected(pTmp))
                                            break;
                                        pTmp = Prev(pTmp);
                                        nDest = static_cast<SwOutlineContent*>(pTmp->GetUserData())->GetOutlinePos();
                                    }
                                    if (!IsSelected(pEntry->PrevSibling()))
                                        break;
                                }
                                else
                                {
                                    nDest = static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlinePos();
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
                    if (pCurrentEntry && IsSelected(pCurrentEntry->PrevSibling()))
                        nDir = nDirLast;
                    else
                    {
                        SwOutlineNodes::size_type nDest = nActPos;
                        pEntry = pCurrentEntry;
                        while (pEntry && nDest)
                        {
                            pEntry = Prev(pEntry);
                            assert(pEntry == nullptr || !lcl_IsContent(pEntry) || dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
                            if (pEntry && lcl_IsContent(pEntry))
                            {
                                nDest = static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlinePos();
                            }
                            else
                            {
                                nDest = 0; // presumably?
                            }
                            if (pEntry)
                            {
                                if (!lcl_IsContent(pEntry))
                                    break;
                                else if (nActLevel >= static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlineLevel())
                                {
                                    // nDest needs adjusted if there are selected entries immediately
                                    // after the level change.
                                    SvTreeListEntry* pTmp = Next(pEntry);
                                    while (pTmp && lcl_IsContent(pTmp) &&
                                           nActLevel < static_cast<SwOutlineContent*>(pTmp->GetUserData())->GetOutlineLevel() &&
                                           IsSelected(pTmp))
                                    {
                                        nDest = static_cast<SwOutlineContent*>(pTmp->GetUserData())->GetOutlinePos();
                                        const auto nLevel = static_cast<SwOutlineContent*>(pTmp->GetUserData())->GetOutlineLevel();
                                        // account for selected entries' descendent lineage
                                        pTmp = Next(pTmp);
                                        while (pTmp && lcl_IsContent(pTmp) &&
                                               nLevel < static_cast<SwOutlineContent*>(pTmp->GetUserData())->GetOutlineLevel())
                                        {
                                            nDest = static_cast<SwOutlineContent*>(pTmp->GetUserData())->GetOutlinePos();
                                            pTmp = Next(pTmp);
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
        pShell->EndAllAction();
        if (m_aActiveContentArr[ContentTypeId::OUTLINE])
            m_aActiveContentArr[ContentTypeId::OUTLINE]->Invalidate();
        Display(true);
        if (!m_bIsRoot)
        {
            const SwOutlineNodes::size_type nCurrPos = pShell->GetOutlinePos(MAXLEVEL);
            SvTreeListEntry* pFirst = First();

            while (nullptr != (pFirst = Next(pFirst)) && lcl_IsContent(pFirst))
            {
                assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pFirst->GetUserData())));
                if (static_cast<SwOutlineContent*>(pFirst->GetUserData())->GetOutlinePos() == nCurrPos)
                {
                    Select(pFirst);
                    MakeVisible(pFirst);
                }
            }
        }
        else
        {
            // Reselect entries
            const SwOutlineNodes& rOutLineNds = pShell->GetNodes().GetOutLineNds();
            for (SwTextNode* pNode : selectedOutlineNodes)
            {
                SwOutlineNodes::const_iterator aFndIt = rOutLineNds.find(pNode);
                if(aFndIt == rOutLineNds.end())
                    continue;
                const size_t nFndPos = aFndIt - rOutLineNds.begin();
                SvTreeListEntry* pEntry = GetEntryAtAbsPos(nFndPos + 1);
                if (pEntry)
                {
                    SvTreeListBox::SelectListEntry(pEntry, true);
                    if (!IsExpanded(pEntry->GetParent()))
                        Expand(pEntry->GetParent());
                }
            }
            SvTreeListBox::Invalidate();
        }
    }
}

void SwContentTree::ShowTree()
{
    SvTreeListBox::Show();
}

void SwContentTree::Paint( vcl::RenderContext& rRenderContext,
                           const tools::Rectangle& rRect )
{
    // Start the update timer on the first paint; avoids
    // flicker on the first reveal.
    m_aUpdTimer.Start();
    SvTreeListBox::Paint( rRenderContext, rRect );
}

void SwContentTree::HideTree()
{
    m_aUpdTimer.Stop();
    SvTreeListBox::Hide();
}

/** No idle with focus or while dragging */
IMPL_LINK_NOARG(SwContentTree, TimerUpdate, Timer *, void)
{
    if (IsDisposed())
        return;

    // No update while drag and drop.
    // Query view because the Navigator is cleared too late.
    SwView* pView = GetParentWindow()->GetCreateView();
    if( (!HasFocus() || m_bViewHasChanged) &&
         !bIsInDrag && !m_bIsInternalDrag && pView &&
         pView->GetWrtShellPtr() && !pView->GetWrtShellPtr()->ActionPend() )
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
    }
    else if (!pView && State::ACTIVE == m_eState && !m_bIsIdleClear)
    {
        if(m_pActiveShell)
        {
            SetActiveShell(nullptr);
        }
        Clear();
        m_bIsIdleClear = true;
    }
}

DragDropMode SwContentTree::NotifyStartDrag(
                TransferDataContainer& rContainer,
                SvTreeListEntry* pEntry )
{
    DragDropMode eMode = DragDropMode(0);
    if (State::ACTIVE == m_eState && m_nRootType == ContentTypeId::OUTLINE &&
            GetModel()->GetAbsPos( pEntry ) > 0
            && !GetWrtShell()->GetView().GetDocShell()->IsReadOnly())
        eMode = GetDragDropMode();
    else if (State::ACTIVE != m_eState && GetWrtShell()->GetView().GetDocShell()->HasName())
        eMode = DragDropMode::APP_COPY;

    sal_Int8 nDragMode;
    FillTransferData( rContainer, nDragMode );
    m_bDocChgdInDragging = false;
    m_bIsInternalDrag = true;
    return eMode;
}
// After the drag the current paragraph will be moved  w i t h  the children.

TriState SwContentTree::NotifyMoving( SvTreeListEntry*  pTarget,
        SvTreeListEntry*  pEntry, SvTreeListEntry*& , sal_uLong& )
{
    if(!m_bDocChgdInDragging)
    {
        SwOutlineNodes::size_type nTargetPos = 0;
        assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
        SwOutlineNodes::size_type nSourcePos = static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlinePos();
        if(!lcl_IsContent(pTarget))
            nTargetPos = SwOutlineNodes::npos;
        else
        {
            assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pTarget->GetUserData())));
            nTargetPos = static_cast<SwOutlineContent*>(pTarget->GetUserData())->GetOutlinePos();
        }
        if( MAXLEVEL > m_nOutlineLevel && // Not all layers are displayed.
                        nTargetPos != SwOutlineNodes::npos)
        {
            SvTreeListEntry* pNext = Next(pTarget);
            if(pNext)
            {
                assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pNext->GetUserData())));
                nTargetPos = static_cast<SwOutlineContent*>(pNext->GetUserData())->GetOutlinePos() - 1;
            }
            else
                nTargetPos = GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount() - 1;
        }

        OSL_ENSURE( pEntry &&
            lcl_IsContent(pEntry),"Source == 0 or Source has no Content" );
        GetParentWindow()->MoveOutline( nSourcePos,
                                    nTargetPos,
                                    true);

        m_aActiveContentArr[ContentTypeId::OUTLINE]->Invalidate();
        Display(true);
    }
    //TreeListBox will be reloaded from the document
    return TRISTATE_FALSE;
}

// After the drag the current paragraph will be moved  w i t h o u t  the children.

TriState SwContentTree::NotifyCopying( SvTreeListEntry*  pTarget,
        SvTreeListEntry*  pEntry, SvTreeListEntry*& , sal_uLong& )
{
    if(!m_bDocChgdInDragging)
    {
        SwOutlineNodes::size_type nTargetPos = 0;
        assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
        SwOutlineNodes::size_type nSourcePos = static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlinePos();
        if(!lcl_IsContent(pTarget))
            nTargetPos = SwOutlineNodes::npos;
        else
        {
            assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pTarget->GetUserData())));
            nTargetPos = static_cast<SwOutlineContent*>(pTarget->GetUserData())->GetOutlinePos();
        }

        if( MAXLEVEL > m_nOutlineLevel && // Not all layers are displayed.
                        nTargetPos != SwOutlineNodes::npos)
        {
            SvTreeListEntry* pNext = Next(pTarget);
            if(pNext)
            {
                assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pNext->GetUserData())));
                nTargetPos = static_cast<SwOutlineContent*>(pNext->GetUserData())->GetOutlinePos() - 1;
            }
            else
                nTargetPos = GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount() - 1;
        }

        OSL_ENSURE( pEntry &&
            lcl_IsContent(pEntry),"Source == 0 or Source has no Content" );
        GetParentWindow()->MoveOutline( nSourcePos, nTargetPos, false);

        //TreeListBox will be reloaded from the document
        m_aActiveContentArr[ContentTypeId::OUTLINE]->Invalidate();
        Display(true);
    }
    return TRISTATE_FALSE;
}

// No drop before the first entry - it's a SwContentType

bool  SwContentTree::NotifyAcceptDrop( SvTreeListEntry* pEntry)
{
    return pEntry != nullptr;
}

// If a Ctrl + DoubleClick are executed in an open area,
// then the base function of the control is to be called.

void SwContentTree::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPos( rMEvt.GetPosPixel());
    SvTreeListEntry* pEntry = GetEntry( aPos, true );
    if( !pEntry && rMEvt.IsLeft() && rMEvt.IsMod1() && (rMEvt.GetClicks() % 2) == 0)
        Control::MouseButtonDown( rMEvt );
    else
    {
        if( pEntry && (rMEvt.GetClicks() % 2) == 0)
        {
            SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());
            const ContentTypeId nActType = pCnt->GetParent()->GetType();
            SetSublistDontOpenWithDoubleClick( nActType == ContentTypeId::OUTLINE );
        }
        SvTreeListBox::MouseButtonDown( rMEvt );
    }
}

// Update immediately

void SwContentTree::GetFocus()
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
        Clear();
    SvTreeListBox::GetFocus();
}

void SwContentTree::KeyInput(const KeyEvent& rEvent)
{
    const vcl::KeyCode aCode = rEvent.GetKeyCode();
    if(aCode.GetCode() == KEY_RETURN)
    {
        SvTreeListEntry* pEntry = FirstSelected();
        if ( pEntry )
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
                    if(lcl_IsContentType(pEntry))
                    {
                        IsExpanded(pEntry) ? Collapse(pEntry) : Expand(pEntry);
                    }
                    else
                        ContentDoubleClickHdl(nullptr);
                break;
            }
        }
    }
    else if(aCode.GetCode() == KEY_DELETE && 0 == aCode.GetModifier())
    {
        SvTreeListEntry* pEntry = FirstSelected();
        assert(!pEntry || dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData())));
        if(pEntry &&
            lcl_IsContent(pEntry) &&
                static_cast<SwContent*>(pEntry->GetUserData())->GetParent()->IsDeletable() &&
                    !m_pActiveShell->GetView().GetDocShell()->IsReadOnly())
        {
            EditEntry(pEntry, EditEntryMode::DELETE);
            m_bViewHasChanged = true;
            GetParentWindow()->UpdateListBox();
            TimerUpdate(&m_aUpdTimer);
            GrabFocus();
        }
    }
    //Make KEY_SPACE has same function as DoubleClick ,
    //and realize multi-selection .
    else if(aCode.GetCode() == KEY_SPACE && 0 == aCode.GetModifier())
    {
        SvTreeListEntry* pEntry = GetCurEntry();
        if(pEntry)
        {
            if( GetChildCount( pEntry ) == 0 )
                m_bIsKeySpace = true;
            Point tempPoint = GetEntryPosition( pEntry );//Change from "GetEntryPos" to "GetEntryPosition" for acc migration
            m_aOldRectangle = GetFocusRect(pEntry, tempPoint.Y());

            if (State::HIDDEN != m_eState)
            {
                if (State::CONSTANT == m_eState)
                {
                    m_pActiveShell->GetView().GetViewFrame()->GetWindow().ToTop();
                }

                SwContent* pCnt = dynamic_cast<SwContent*>(static_cast<SwTypeNumber*>(pEntry->GetUserData()));

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
        SvTreeListBox::KeyInput(rEvent);

}

void SwContentTree::RequestHelp( const HelpEvent& rHEvt )
{
    bool bCallBase = true;
    if( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            ContentTypeId nType;
            bool bBalloon = false;
            bool bContent = false;
            void* pUserData = pEntry->GetUserData();
            if(lcl_IsContentType(pEntry))
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
            bool bRet = false;
            if(bContent)
            {
                switch( nType )
                {
                    case ContentTypeId::URLFIELD:
                        assert(dynamic_cast<SwURLFieldContent*>(static_cast<SwTypeNumber*>(pUserData)));
                        sEntry = static_cast<SwURLFieldContent*>(pUserData)->GetURL();
                        bRet = true;
                    break;

                    case ContentTypeId::POSTIT:
                        assert(dynamic_cast<SwPostItContent*>(static_cast<SwTypeNumber*>(pUserData)));
                        sEntry = static_cast<SwPostItContent*>(pUserData)->GetName();
                        bRet = true;
                        if(Help::IsBalloonHelpEnabled())
                            bBalloon = true;
                    break;
                    case ContentTypeId::OUTLINE:
                        assert(dynamic_cast<SwOutlineContent*>(static_cast<SwTypeNumber*>(pUserData)));
                        sEntry = static_cast<SwOutlineContent*>(pUserData)->GetName();
                        bRet = true;
                    break;
                    case ContentTypeId::GRAPHIC:
                        assert(dynamic_cast<SwGraphicContent*>(static_cast<SwTypeNumber*>(pUserData)));
                        sEntry = static_cast<SwGraphicContent*>(pUserData)->GetLink();
                        bRet = true;
                    break;
                    default: break;
                }
                if(static_cast<SwContent*>(pUserData)->IsInvisible())
                {
                    if(!sEntry.isEmpty())
                        sEntry += ", ";
                    sEntry += m_sInvisible;
                    bRet = true;
                }
            }
            else
            {
                const size_t nMemberCount = static_cast<SwContentType*>(pUserData)->GetMemberCount();
                sEntry = OUString::number(nMemberCount);
                sEntry += " ";
                sEntry += nMemberCount == 1
                            ? static_cast<SwContentType*>(pUserData)->GetSingleName()
                            : static_cast<SwContentType*>(pUserData)->GetName();
                bRet = true;
            }
            if(bRet)
            {
                SvLBoxTab* pTab;
                SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
                if (pItem && SvLBoxItemType::String == pItem->GetType())
                {
                    aPos = GetEntryPosition( pEntry );

                    aPos.setX( GetTabPos( pEntry, pTab ) );
                    Size aSize(pItem->GetWidth(this, pEntry), pItem->GetHeight(this, pEntry));

                    if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                        aSize.setWidth( GetSizePixel().Width() - aPos.X() );

                    aPos = OutputToScreenPixel(aPos);
                    tools::Rectangle aItemRect( aPos, aSize );
                    if(bBalloon)
                    {
                        aPos.AdjustX(aSize.Width() );
                        Help::ShowBalloon( this, aPos, aItemRect, sEntry );
                    }
                    else
                        Help::ShowQuickHelp( this, aItemRect, sEntry,
                            QuickHelpFlags::Left|QuickHelpFlags::VCenter );
                    bCallBase = false;
                }
            }
            else
            {
                Help::ShowQuickHelp( this, tools::Rectangle(), OUString() );
                bCallBase = false;
            }
        }
    }
    if( bCallBase )
        Window::RequestHelp( rHEvt );
}

void SwContentTree::ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
{
    SvTreeListEntry* pFirst = FirstSelected();
    switch( nSelectedPopupEntry )
    {
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
            EditEntry(pFirst, nSelectedPopupEntry == 401 ? EditEntryMode::RMV_IDX : EditEntryMode::UPD_IDX);
        break;
        // Edit entry
        case 403:
            EditEntry(pFirst, EditEntryMode::EDIT);
        break;
        case 404:
            EditEntry(pFirst, EditEntryMode::UNPROTECT_TABLE);
        break;
        case 405 :
        {
            const SwTOXBase* pBase = static_cast<SwTOXBaseContent*>(pFirst->GetUserData())
                                                                ->GetTOXBase();
            m_pActiveShell->SetTOXBaseReadonly(*pBase, !SwEditShell::IsTOXBaseReadonly(*pBase));
        }
        break;
        case 4:
        break;
        case 501:
            EditEntry(pFirst, EditEntryMode::DELETE);
        break;
        case 502 :
            EditEntry(pFirst, EditEntryMode::RENAME);
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

// Here the buttons for moving outlines are en-/disabled.
bool SwContentTree::Select( SvTreeListEntry* pEntry, bool bSelect )
{
    if(!pEntry)
        return false;
    bool bEnable = false;
    SvTreeListEntry* pParentEntry = GetParent(pEntry);
    while(pParentEntry && (!lcl_IsContentType(pParentEntry)))
    {
        pParentEntry = GetParent(pParentEntry);
    }
    if (!m_bIsLastReadOnly)
    {
        if (!IsVisible())
            bEnable = true;
        else if (pParentEntry)
        {
            if ((m_bIsRoot && m_nRootType == ContentTypeId::OUTLINE) ||
                (lcl_IsContent(pEntry) &&
                    static_cast<SwContentType*>(pParentEntry->GetUserData())->GetType() == ContentTypeId::OUTLINE))
            {
                bEnable = true;
            }
        }
    }
    SwNavigationPI* pNavi = GetParentWindow();
    pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("up"),  bEnable);
    pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("down"), bEnable);
    pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("promote"), bEnable);
    pNavi->m_aContentToolBox->EnableItem(pNavi->m_aContentToolBox->GetItemId("demote"), bEnable);

    return SvTreeListBox::Select(pEntry, bSelect);
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

void SwContentTree::EditEntry(SvTreeListEntry const * pEntry, EditEntryMode nMode)
{
    SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());
    GotoContent(pCnt);
    const ContentTypeId nType = pCnt->GetParent()->GetType();
    sal_uInt16 nSlot = 0;

    uno::Reference< container::XNameAccess >  xNameAccess, xSecond, xThird;
    switch(nType)
    {
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
            nSlot = FN_EDIT_HYPERLINK;
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
        break;
        default: break;
    }
    if(nSlot)
        m_pActiveShell->GetView().GetViewFrame()->
                    GetDispatcher()->Execute(nSlot, SfxCallMode::ASYNCHRON);
    else if(xNameAccess.is())
    {
        uno::Any aObj = xNameAccess->getByName(pCnt->GetName());
        uno::Reference< uno::XInterface >  xTmp;
        aObj >>= xTmp;
        uno::Reference< container::XNamed >  xNamed(xTmp, uno::UNO_QUERY);
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSwRenameXNamedDlg> pDlg(pFact->CreateSwRenameXNamedDlg(GetFrameWeld(), xNamed, xNameAccess));
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
}

void SwContentTree::GotoContent(SwContent* pCnt)
{
    m_pActiveShell->EnterStdMode();

    bool bSel = false;
    switch(pCnt->GetParent()->GetType())
    {
        case ContentTypeId::OUTLINE   :
        {
            m_pActiveShell->GotoOutline(static_cast<SwOutlineContent*>(pCnt)->GetOutlinePos());
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
            if(m_pActiveShell->GotoFly(pCnt->GetName()))
                bSel = true;
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
                            *static_cast<SwURLFieldContent*>(pCnt)->GetINetAttr() ))
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
            m_pActiveShell->GetView().GetPostItMgr()->AssureStdModeAtShell();
            m_pActiveShell->GotoFormatField(*static_cast<SwPostItContent*>(pCnt)->GetPostIt());
        break;
        case ContentTypeId::DRAWOBJECT:
        {
            SwPosition aPos = *m_pActiveShell->GetCursor()->GetPoint();
            SdrView* pDrawView = m_pActiveShell->GetDrawView();
            if (pDrawView)
            {
                pDrawView->SdrEndTextEdit();
                pDrawView->UnmarkAll();
                SwDrawModel* _pModel = m_pActiveShell->getIDocumentDrawModelAccess().GetDrawModel();
                SdrPage* pPage = _pModel->GetPage(0);
                const size_t nCount = pPage->GetObjCount();
                for( size_t i=0; i<nCount; ++i )
                {
                    SdrObject* pTemp = pPage->GetObj(i);
                    if (pTemp->GetName() == pCnt->GetName())
                    {
                        SdrPageView* pPV = pDrawView->GetSdrPageView();
                        if( pPV )
                        {
                            pDrawView->MarkObj( pTemp, pPV );
                        }
                    }
                }
                m_pActiveShell->GetNavigationMgr().addEntry(aPos);
                m_pActiveShell->EnterStdMode();
                bSel = true;
            }
        }
        break;
        default: break;
    }
    if(bSel)
    {
        m_pActiveShell->HideCursor();
        m_pActiveShell->EnterSelFrameMode();
    }
    SwView& rView = m_pActiveShell->GetView();
    rView.StopShellTimer();
    rView.GetPostItMgr()->SetActiveSidebarWin(nullptr);
    rView.GetEditWin().GrabFocus();

    // force scroll to cursor position when navigating to inactive document
    if(!bSel)
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

    OString sStrBuf(OUStringToOString(aUrl, eSysCSet) + OString(NAVI_BOOKMARK_DELIM) +
                    OUStringToOString(aDescr, eSysCSet) + OString(NAVI_BOOKMARK_DELIM) +
                    OString::number(static_cast<int>(nDefDrag)) + OString(NAVI_BOOKMARK_DELIM) +
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

class SwContentLBoxString : public SvLBoxString
{
public:
    explicit SwContentLBoxString(const OUString& rStr) : SvLBoxString(rStr) {}

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};

void SwContentTree::InitEntry(SvTreeListEntry* pEntry,
        const OUString& rStr ,const Image& rImg1,const Image& rImg2,
        SvLBoxButtonKind eButtonKind)
{
    const size_t nColToHilite = 1; //0==Bitmap;1=="Column1";2=="Column2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString& rCol = static_cast<SvLBoxString&>(pEntry->GetItem( nColToHilite ));
    pEntry->ReplaceItem(std::make_unique<SwContentLBoxString>(rCol.GetText()), nColToHilite);
}

void SwContentLBoxString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                                const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{
    if (lcl_IsContent(&rEntry) && static_cast<SwContent *>(rEntry.GetUserData())->IsInvisible())
    {
        vcl::Font aOldFont(rRenderContext.GetFont());
        vcl::Font aFont(aOldFont);
        aFont.SetColor(COL_LIGHTGRAY);
        rRenderContext.SetFont(aFont );
        rRenderContext.DrawText(rPos, GetText());
        rRenderContext.SetFont(aOldFont);
    }
    else
    {
        SvLBoxString::Paint(rPos, rDev, rRenderContext, pView, rEntry);
    }
}

void SwContentTree::DataChanged(const DataChangedEvent& rDCEvt)
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        FindActiveTypeAndRemoveUserData();

        Display(true);
    }

    SvTreeListBox::DataChanged( rDCEvt );
}

SwNavigationPI* SwContentTree::GetParentWindow()
{
    return m_xDialog;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
