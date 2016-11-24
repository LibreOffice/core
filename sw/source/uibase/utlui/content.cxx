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
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <o3tl/enumrange.hxx>
#include <o3tl/make_unique.hxx>
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
#include <crsskip.hxx>
#include <cmdid.h>
#include <helpid.h>
#include <navipi.hrc>
#include <utlui.hrc>
#include <misc.hrc>
#include <comcore.hrc>
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
#include <svtools/treelistentry.hxx>

#include "swabstdlg.hxx"
#include "globals.hrc"
#include <unomid.h>

#include "navmgr.hxx"
#include "SidebarWin.hxx"
#include <memory>

#define CTYPE_CNT   0
#define CTYPE_CTT   1

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

#define NAVI_BOOKMARK_DELIM     (sal_Unicode)1

class SwContentArr
    : public o3tl::sorted_vector<SwContent*, o3tl::less_ptr_to<SwContent>,
                o3tl::find_partialorder_ptrequals>
{
public:
    ~SwContentArr() { DeleteAndDestroyAll(); }
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

    bool lcl_FindShell(SwWrtShell* pShell)
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

    bool lcl_IsUiVisibleBookmark(const IDocumentMarkAccess::pMark_t& rpMark)
    {
        return IDocumentMarkAccess::GetType(*rpMark) == IDocumentMarkAccess::MarkType::BOOKMARK;
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
            SwURLFieldContent* pCnt = new SwURLFieldContent(
                                pCntType,
                                p->sText,
                                INetURLObject::decode(
                                    p->rINetAttr.GetINetFormat().GetValue(),
                                    INetURLObject::DECODE_UNAMBIGUOUS ),
                                &p->rINetAttr,
                                n );
            pMember->insert( pCnt );
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
    if (mbPostIt)
        return pField->IsProtect();
    else
        return false;
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

SwContentType::SwContentType(SwWrtShell* pShell, ContentTypeId nType, sal_uInt8 nLevel) :
    SwTypeNumber(CTYPE_CTT),
    pWrtShell(pShell),
    pMember(nullptr),
    sContentTypeName(SW_RES(STR_CONTENT_TYPE_FIRST + (int)nType)),
    sSingleContentTypeName(SW_RES(STR_CONTENT_TYPE_SINGLE_FIRST + (int)nType)),
    nMemberCount(0),
    nContentType(nType),
    nOutlineLevel(nLevel),
    bDataValid(false),
    bEdit(false),
    bDelete(true)
{
    Init();
}

void SwContentType::Init(bool* pbInvalidateWindow)
{
    // if the MemberCount is changing ...
    size_t nOldMemberCount = nMemberCount;
    nMemberCount = 0;
    switch(nContentType)
    {
        case ContentTypeId::OUTLINE   :
        {
            sTypeToken = "outline";
            nMemberCount = pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
            if (nMemberCount < MAXLEVEL)
            {
                const size_t nOutlineCount = nMemberCount;
                for(size_t j = 0; j < nOutlineCount; ++j)
                {
                    if(pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(j) > nOutlineLevel )
                        nMemberCount --;
                }
            }
            bDelete = false;
        }
        break;

        case ContentTypeId::TABLE     :
            sTypeToken = "table";
            nMemberCount = pWrtShell->GetTableFrameFormatCount(true);
            bEdit = true;
        break;

        case ContentTypeId::FRAME     :
        case ContentTypeId::GRAPHIC   :
        case ContentTypeId::OLE       :
        {
            FlyCntType eType = FLYCNTTYPE_FRM;
            sTypeToken = "frame";
            if(nContentType == ContentTypeId::OLE)
            {
                eType = FLYCNTTYPE_OLE;
                sTypeToken = "ole";
            }
            else if(nContentType == ContentTypeId::GRAPHIC)
            {
                eType = FLYCNTTYPE_GRF;
                sTypeToken = "graphic";
            }
            nMemberCount = pWrtShell->GetFlyCount(eType, /*bIgnoreTextBoxes=*/true);
            bEdit = true;
        }
        break;
        case ContentTypeId::BOOKMARK:
        {
            IDocumentMarkAccess* const pMarkAccess = pWrtShell->getIDocumentMarkAccess();
            nMemberCount = count_if(
                pMarkAccess->getBookmarksBegin(),
                pMarkAccess->getBookmarksEnd(),
                &lcl_IsUiVisibleBookmark);
            sTypeToken.clear();
            bEdit = true;
        }
        break;
        case ContentTypeId::REGION :
        {
            SwContentArr*   pOldMember = nullptr;
            size_t nOldRegionCount = 0;
            bool bInvalidate = false;
            if(!pMember)
                pMember = new SwContentArr;
            else if(!pMember->empty())
            {
                pOldMember = pMember;
                nOldRegionCount = pOldMember->size();
                pMember = new SwContentArr;
            }
            const Point aNullPt;
            nMemberCount = pWrtShell->GetSectionFormatCount();
            for(size_t i = 0; i < nMemberCount; ++i)
            {
                const SwSectionFormat* pFormat;
                SectionType eTmpType;
                if( (pFormat = &pWrtShell->GetSectionFormat(i))->IsInNodesArr() &&
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

                    SwContent* pCnt = new SwRegionContent(this, rSectionName,
                            nLevel,
                            pFormat->FindLayoutRect( false, &aNullPt ).Top());

                    SwPtrMsgPoolItem aAskItem( RES_CONTENT_VISIBLE, nullptr );
                    if( !pFormat->GetInfo( aAskItem ) &&
                        !aAskItem.pObject )     // not visible
                        pCnt->SetInvisible();
                    pMember->insert(pCnt);

                    const size_t nPos = pMember->size() - 1;
                    if(nOldRegionCount > nPos &&
                        ((*pOldMember)[nPos])->IsInvisible()
                                != pCnt->IsInvisible())
                            bInvalidate = true;
                }
            }
            nMemberCount = pMember->size();
            sTypeToken = "region";
            bEdit = true;
            bDelete = false;
            if(pOldMember)
            {
                pOldMember->DeleteAndDestroyAll();
                delete pOldMember;
                if(pbInvalidateWindow && bInvalidate)
                    *pbInvalidateWindow = true;
            }
        }
        break;
        case ContentTypeId::INDEX:
        {
            nMemberCount = pWrtShell->GetTOXCount();
            bEdit = true;
            bDelete = false;
        }
        break;
        case ContentTypeId::REFERENCE:
        {
            nMemberCount = pWrtShell->GetRefMarks();
            bDelete = false;
        }
        break;
        case ContentTypeId::URLFIELD:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(!pMember->empty())
                pMember->DeleteAndDestroyAll();

            nMemberCount = lcl_InsertURLFieldContent(pMember, pWrtShell, this);

            bEdit = true;
            nOldMemberCount = nMemberCount;
            bDelete = false;
        }
        break;
        case ContentTypeId::POSTIT:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(!pMember->empty())
                pMember->DeleteAndDestroyAll();

            SwPostItMgr* aMgr = pWrtShell->GetView().GetPostItMgr();
            if (aMgr)
            {
                for(SwPostItMgr::const_iterator i = aMgr->begin(); i != aMgr->end(); ++i)
                {
                    if ( dynamic_cast< const SwFormatField *>( (*i)->GetBroadCaster() ) != nullptr ) // SwPostit
                    {
                        const SwFormatField* aFormatField = static_cast<const SwFormatField*>((*i)->GetBroadCaster());
                        if (aFormatField->GetTextField() && aFormatField->IsFieldInDoc() &&
                            (*i)->mLayoutStatus!=SwPostItHelper::INVISIBLE )
                        {
                            OUString sEntry = aFormatField->GetField()->GetPar2();
                            sEntry = RemoveNewline(sEntry);
                            SwPostItContent* pCnt = new SwPostItContent(
                                                this,
                                                sEntry,
                                                aFormatField,
                                                nMemberCount);
                            pMember->insert(pCnt);
                            nMemberCount++;
                        }
                    }
                }
            }
            sTypeToken.clear();
            bEdit = true;
            nOldMemberCount = nMemberCount;
        }
        break;
        case ContentTypeId::DRAWOBJECT:
        {
            sTypeToken.clear();
            nMemberCount = 0;
            SwDrawModel* pModel = pWrtShell->getIDocumentDrawModelAccess().GetDrawModel();
            if(pModel)
            {
                SdrPage* pPage = pModel->GetPage(0);
                const size_t nCount = pPage->GetObjCount();
                for( size_t i=0; i<nCount; ++i )
                {
                    SdrObject* pTemp = pPage->GetObj(i);
                    // #i51726# - all drawing objects can be named now
                    if (!pTemp->GetName().isEmpty())
                        nMemberCount++;
                }
            }
        }
        break;
        default: break;
    }
    // ... then, the data can also no longer be valid,
    // apart from those which have already been corrected,
    // then nOldMemberCount is nevertheless not so old.
    if( nOldMemberCount != nMemberCount )
        bDataValid = false;
}

SwContentType::~SwContentType()
{
    delete pMember;
}

const SwContent* SwContentType::GetMember(size_t nIndex)
{
    if(!bDataValid || !pMember)
    {
        FillMemberList();
    }
    if(nIndex < pMember->size())
        return (*pMember)[nIndex];

    return nullptr;
}

void SwContentType::Invalidate()
{
    bDataValid = false;
}

void SwContentType::FillMemberList(bool* pbLevelOrVisibilityChanged)
{
    SwContentArr*   pOldMember = nullptr;
    size_t nOldMemberCount = 0;
    SwPtrMsgPoolItem aAskItem( RES_CONTENT_VISIBLE, nullptr );
    if(pMember && pbLevelOrVisibilityChanged)
    {
        pOldMember = pMember;
        nOldMemberCount = pOldMember->size();
        pMember = new SwContentArr;
        *pbLevelOrVisibilityChanged = false;
    }
    else if(!pMember)
        pMember = new SwContentArr;
    else if(!pMember->empty())
        pMember->DeleteAndDestroyAll();
    switch(nContentType)
    {
        case ContentTypeId::OUTLINE   :
        {
            const size_t nOutlineCount = nMemberCount =
                pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount();

            size_t nPos = 0;
            for (size_t i = 0; i < nOutlineCount; ++i)
            {
                const sal_Int8 nLevel = (sal_Int8)pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(i);
                if(nLevel >= nOutlineLevel )
                    nMemberCount--;
                else
                {
                    OUString aEntry(comphelper::string::stripStart(
                        pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(i), ' '));
                    aEntry = SwNavigationPI::CleanEntry(aEntry);
                    SwOutlineContent* pCnt = new SwOutlineContent(this, aEntry, i, nLevel,
                                                        pWrtShell->IsOutlineMovable( i ), nPos );
                    pMember->insert(pCnt);//, nPos);
                    // with the same number and existing "pOldMember" the
                    // old one is compared with the new OutlinePos.
                    // cast for Win16
                    if (nOldMemberCount > nPos &&
                        static_cast<SwOutlineContent*>((*pOldMember)[nPos])->GetOutlineLevel() != nLevel)
                        *pbLevelOrVisibilityChanged = true;

                    nPos++;
                }
            }

        }
        break;

        case ContentTypeId::TABLE     :
        {
            const size_t nCount = pWrtShell->GetTableFrameFormatCount(true);
            OSL_ENSURE(nMemberCount == nCount, "MemberCount differs");
            Point aNullPt;
            nMemberCount = nCount;
            for(size_t i = 0; i < nMemberCount; ++i)
            {
                const SwFrameFormat& rTableFormat = pWrtShell->GetTableFrameFormat(i, true);
                const OUString& sTableName( rTableFormat.GetName() );

                SwContent* pCnt = new SwContent(this, sTableName,
                        rTableFormat.FindLayoutRect(false, &aNullPt).Top() );
                if( !rTableFormat.GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();

                pMember->insert(pCnt);

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
            if(nContentType == ContentTypeId::OLE)
                eType = FLYCNTTYPE_OLE;
            else if(nContentType == ContentTypeId::GRAPHIC)
                eType = FLYCNTTYPE_GRF;
            OSL_ENSURE(nMemberCount ==  pWrtShell->GetFlyCount(eType, /*bIgnoreTextBoxes=*/true),
                    "MemberCount differs");
            Point aNullPt;
            nMemberCount = pWrtShell->GetFlyCount(eType, /*bIgnoreTextBoxes=*/true);
            for(size_t i = 0; i < nMemberCount; ++i)
            {
                const SwFrameFormat* pFrameFormat = pWrtShell->GetFlyNum(i,eType,/*bIgnoreTextBoxes=*/true);
                const OUString sFrameName = pFrameFormat->GetName();

                SwContent* pCnt;
                if(ContentTypeId::GRAPHIC == nContentType)
                {
                    OUString sLink;
                    pWrtShell->GetGrfNms( &sLink, nullptr, static_cast<const SwFlyFrameFormat*>( pFrameFormat));
                    pCnt = new SwGraphicContent(this, sFrameName,
                                INetURLObject::decode( sLink,
                                           INetURLObject::DECODE_UNAMBIGUOUS ),
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
                pMember->insert(pCnt);
                if (nOldMemberCount > i &&
                    (*pOldMember)[i]->IsInvisible() != pCnt->IsInvisible())
                        *pbLevelOrVisibilityChanged = true;
            }
        }
        break;
        case ContentTypeId::BOOKMARK:
        {
            IDocumentMarkAccess* const pMarkAccess = pWrtShell->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
                ppBookmark != pMarkAccess->getBookmarksEnd();
                ++ppBookmark)
            {
                if(lcl_IsUiVisibleBookmark(*ppBookmark))
                {
                    const OUString& rBkmName = ppBookmark->get()->GetName();
                    //nYPos from 0 -> text::Bookmarks will be sorted alphabetically
                    SwContent* pCnt = new SwContent(this, rBkmName, 0);
                    pMember->insert(pCnt);
                }
            }
        }
        break;
        case ContentTypeId::REGION    :
        {
            const Point aNullPt;
            nMemberCount = pWrtShell->GetSectionFormatCount();
            for(size_t i = 0; i < nMemberCount; ++i)
            {
                const SwSectionFormat* pFormat;
                SectionType eTmpType;
                if( (pFormat = &pWrtShell->GetSectionFormat(i))->IsInNodesArr() &&
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

                    SwContent* pCnt = new SwRegionContent(this, sSectionName,
                            nLevel,
                            pFormat->FindLayoutRect( false, &aNullPt ).Top());
                    if( !pFormat->GetInfo( aAskItem ) &&
                        !aAskItem.pObject )     // not visible
                        pCnt->SetInvisible();
                    pMember->insert(pCnt);

                    const size_t nPos = pMember->size() - 1;
                    if(nOldMemberCount > nPos &&
                        (*pOldMember)[nPos]->IsInvisible()
                                != pCnt->IsInvisible())
                            *pbLevelOrVisibilityChanged = true;
                }
            }
            nMemberCount = pMember->size();
        }
        break;
        case ContentTypeId::REFERENCE:
        {
            std::vector<OUString> aRefMarks;
            nMemberCount = pWrtShell->GetRefMarks( &aRefMarks );

            for(std::vector<OUString>::const_iterator i = aRefMarks.begin(); i != aRefMarks.end(); ++i)
            {
                // References sorted alphabetically
                SwContent* pCnt = new SwContent(this, *i, 0);
                pMember->insert(pCnt);
            }
        }
        break;
        case ContentTypeId::URLFIELD:
            nMemberCount = lcl_InsertURLFieldContent(pMember, pWrtShell, this);
        break;
        case ContentTypeId::INDEX:
        {

            const sal_uInt16 nCount = pWrtShell->GetTOXCount();
            nMemberCount = nCount;
            for ( sal_uInt16 nTox = 0; nTox < nCount; nTox++ )
            {
                const SwTOXBase* pBase = pWrtShell->GetTOX( nTox );
                OUString sTOXNm( pBase->GetTOXName() );

                SwContent* pCnt = new SwTOXBaseContent(
                        this, sTOXNm, nTox, *pBase);

                if( !pBase->GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();

                pMember->insert( pCnt );
                const size_t nPos = pMember->size() - 1;
                if(nOldMemberCount > nPos &&
                    (*pOldMember)[nPos]->IsInvisible()
                            != pCnt->IsInvisible())
                        *pbLevelOrVisibilityChanged = true;
            }
        }
        break;
        case ContentTypeId::POSTIT:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(!pMember->empty())
                pMember->DeleteAndDestroyAll();
            SwPostItMgr* aMgr = pWrtShell->GetView().GetPostItMgr();
            if (aMgr)
            {
                for(SwPostItMgr::const_iterator i = aMgr->begin(); i != aMgr->end(); ++i)
                {
                    if ( dynamic_cast< const SwFormatField *>( (*i)->GetBroadCaster() ) != nullptr ) // SwPostit
                    {
                        const SwFormatField* aFormatField = static_cast<const SwFormatField*>((*i)->GetBroadCaster());
                        if (aFormatField->GetTextField() && aFormatField->IsFieldInDoc() &&
                            (*i)->mLayoutStatus!=SwPostItHelper::INVISIBLE )
                        {
                            OUString sEntry = aFormatField->GetField()->GetPar2();
                            sEntry = RemoveNewline(sEntry);
                            SwPostItContent* pCnt = new SwPostItContent(
                                                this,
                                                sEntry,
                                                aFormatField,
                                                nMemberCount);
                            pMember->insert(pCnt);
                            nMemberCount++;
                        }
                    }
                }
            }
        }
        break;
        case ContentTypeId::DRAWOBJECT:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(!pMember->empty())
                pMember->DeleteAndDestroyAll();

            IDocumentDrawModelAccess& rIDDMA = pWrtShell->getIDocumentDrawModelAccess();
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
                        pMember->insert(pCnt);
                        nMemberCount++;
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
    bDataValid = true;
    if(pOldMember)
        pOldMember->DeleteAndDestroyAll();

}

SwContentTree::SwContentTree(vcl::Window* pParent, const ResId& rResId)
    : SvTreeListBox(pParent, rResId)
    , m_sSpace(OUString("                    "))
    , m_sRemoveIdx(SW_RES(STR_REMOVE_INDEX))
    , m_sUpdateIdx(SW_RES(STR_UPDATE))
    , m_sUnprotTable(SW_RES(STR_REMOVE_TBL_PROTECTION))
    , m_sRename(SW_RES(STR_RENAME))
    , m_sReadonlyIdx(SW_RES(STR_READONLY_IDX))
    , m_sInvisible(SW_RES(STR_INVISIBLE))
    , m_sPostItShow(SW_RES(STR_POSTIT_SHOW))
    , m_sPostItHide(SW_RES(STR_POSTIT_HIDE))
    , m_sPostItDelete(SW_RES(STR_POSTIT_DELETE))
    , m_pHiddenShell(nullptr)
    , m_pActiveShell(nullptr)
    , m_pConfig(SW_MOD()->GetNavigationConfig())
    , m_nActiveBlock(0)
    , m_nHiddenBlock(0)
    , m_nRootType(ContentTypeId::UNKNOWN)
    , m_nLastSelType(ContentTypeId::UNKNOWN)
    , m_nOutlineLevel(MAXLEVEL)
    , m_bIsActive(true)
    , m_bIsConstant(false)
    , m_bIsHidden(false)
    , m_bDocChgdInDragging(false)
    , m_bIsInternalDrag(false)
    , m_bIsRoot(false)
    , m_bIsIdleClear(false)
    , m_bIsLastReadOnly(false)
    , m_bIsOutlineMoveable(true)
    , m_bViewHasChanged(false)
    , m_bIsImageListInitialized(false)
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
        m_aContextStrings[i] = SW_RESSTR(i+STR_CONTEXT_FIRST);
    }
    m_nActiveBlock = m_pConfig->GetActiveBlock();
    m_aUpdTimer.SetTimeoutHdl(LINK(this, SwContentTree, TimerUpdate));
    m_aUpdTimer.SetTimeout(1000);
    Clear();
    EnableContextMenuHandling();
    SetStyle( GetStyle() | WB_QUICK_SEARCH );
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
    SvTreeListBox::dispose();
}

OUString SwContentTree::GetEntryAltText( SvTreeListEntry* pEntry ) const
{
    if( pEntry == nullptr)
        return OUString();

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
                        case OBJ_TEXTEXT:
                        case OBJ_wegFITTEXT:
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
                        case OBJ_TEXTEXT:
                        case OBJ_wegFITTEXT:
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

        TransferDataContainer* pContainer = new TransferDataContainer;
        uno::Reference<
            datatransfer::XTransferable > xRef( pContainer );

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
        nRet = GetParentWindow()->AcceptDrop( rEvt );
    return nRet;
}

// Drop will be executed in the navigator

static void* lcl_GetOutlineKey( SwContentTree* pTree, SwOutlineContent* pContent)
{
    void* key = nullptr;
    if( pTree && pContent )
    {
        SwWrtShell* pShell = pTree->GetWrtShell();
        sal_Int32 nPos = pContent->GetYPos();

        key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));

    }
    return key;
}

sal_Int8 SwContentTree::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    SvTreeListEntry* pEntry = pTargetEntry;
    if( pEntry && ( m_nRootType == ContentTypeId::OUTLINE ) && lcl_IsContent( pEntry ) )
    {
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
                        pChildEntry = NextSibling( pChildEntry );
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

std::unique_ptr<PopupMenu> SwContentTree::CreateContextMenu()
{
    std::unique_ptr<PopupMenu> pPop(new PopupMenu);
    PopupMenu* pSubPop1 = new PopupMenu;
    PopupMenu* pSubPop2 = new PopupMenu;
    PopupMenu* pSubPop3 = new PopupMenu;
    PopupMenu* pSubPop4 = new PopupMenu; // Edit

    for(int i = 1; i <= MAXLEVEL; ++i)
    {
        pSubPop1->InsertItem( i + 100, OUString::number(i));
    }
    pSubPop1->CheckItem(100 + m_nOutlineLevel);
    for(int i=0; i < 3; ++i)
    {
        pSubPop2->InsertItem( i + 201, m_aContextStrings[
                STR_HYPERLINK - STR_CONTEXT_FIRST + i]);
    }
    pSubPop2->CheckItem( 201 +
                    static_cast<int>(GetParentWindow()->GetRegionDropMode()));
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
            sInsert += m_aContextStrings[ STR_ACTIVE - STR_CONTEXT_FIRST];
            sInsert += ")";
        }
        pSubPop3->InsertItem(nId, sInsert);
        if(m_bIsConstant && m_pActiveShell == &pView->GetWrtShell())
            pSubPop3->CheckItem(nId);
        pView = SwModule::GetNextView(pView);
        nId++;
    }
    pSubPop3->InsertItem(nId++, m_aContextStrings[STR_ACTIVE_VIEW - STR_CONTEXT_FIRST]);
    if(m_pHiddenShell)
    {
        OUString sHiddenEntry = m_pHiddenShell->GetView().GetDocShell()->GetTitle();
        sHiddenEntry += " ( ";
        sHiddenEntry += m_aContextStrings[ STR_HIDDEN - STR_CONTEXT_FIRST];
        sHiddenEntry += " )";
        pSubPop3->InsertItem(nId, sHiddenEntry);
    }

    if(m_bIsActive)
        pSubPop3->CheckItem( --nId );
    else if(m_bIsHidden)
        pSubPop3->CheckItem( nId );

    pPop->InsertItem( 1, m_aContextStrings[STR_OUTLINE_LEVEL - STR_CONTEXT_FIRST]);
    pPop->InsertItem(2, m_aContextStrings[STR_DRAGMODE - STR_CONTEXT_FIRST]);
    pPop->InsertItem(3, m_aContextStrings[STR_DISPLAY - STR_CONTEXT_FIRST]);
    // Now edit
    SvTreeListEntry* pEntry = nullptr;
    // Edit only if the shown content is coming from the current view.
    if((m_bIsActive || m_pActiveShell == pActiveView->GetWrtShellPtr())
            && nullptr != (pEntry = FirstSelected()) && lcl_IsContent(pEntry))
    {
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
            bool bSubPop4 = false;
            if(ContentTypeId::INDEX == nContentType)
            {
                bSubPop4 = true;
                pSubPop4->InsertItem(401, m_sRemoveIdx);
                pSubPop4->InsertItem(402, m_sUpdateIdx);

                const SwTOXBase* pBase = static_cast<SwTOXBaseContent*>(pEntry->GetUserData())->GetTOXBase();
                if(!pBase->IsTOXBaseInReadonly())
                    pSubPop4->InsertItem(403, m_aContextStrings[STR_EDIT_ENTRY - STR_CONTEXT_FIRST]);
                pSubPop4->InsertItem(405, m_sReadonlyIdx);

                pSubPop4->CheckItem( 405, SwEditShell::IsTOXBaseReadonly(*pBase));
                pSubPop4->InsertItem(501, m_aContextStrings[STR_DELETE_ENTRY - STR_CONTEXT_FIRST]);
            }
            else if(ContentTypeId::TABLE == nContentType && !bReadonly)
            {
                bSubPop4 = true;
                pSubPop4->InsertItem(403, m_aContextStrings[STR_EDIT_ENTRY - STR_CONTEXT_FIRST]);
                pSubPop4->InsertItem(404, m_sUnprotTable);
                bool bFull = false;
                OUString sTableName = static_cast<SwContent*>(pEntry->GetUserData())->GetName();
                bool bProt = m_pActiveShell->HasTableAnyProtection( &sTableName, &bFull );
                pSubPop4->EnableItem(403, !bFull );
                pSubPop4->EnableItem(404, bProt );
                pSubPop4->InsertItem(501, m_aContextStrings[STR_DELETE_ENTRY - STR_CONTEXT_FIRST]);
            }
            else if(bEditable || bDeletable)
            {

                if(bEditable && bDeletable)
                {
                    pSubPop4->InsertItem(403, m_aContextStrings[STR_EDIT_ENTRY - STR_CONTEXT_FIRST]);
                    pSubPop4->InsertItem(501, m_aContextStrings[STR_DELETE_ENTRY - STR_CONTEXT_FIRST]);
                    bSubPop4 = true;
                }
                else if(bEditable)
                    pPop->InsertItem(403, m_aContextStrings[STR_EDIT_ENTRY - STR_CONTEXT_FIRST]);
                else if(bDeletable)
                {
                    pSubPop4->InsertItem(501, m_aContextStrings[STR_DELETE_ENTRY - STR_CONTEXT_FIRST]);
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
        SwContentType* pType = static_cast<SwContentType*>(pEntry->GetUserData());
        if ( (pType->GetType() == ContentTypeId::POSTIT) &&  (!m_pActiveShell->GetView().GetDocShell()->IsReadOnly()) && ( pType->GetMemberCount() > 0) )
        {
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
    return pPop;

}

// Indentation for outlines (and sections)

sal_IntPtr SwContentTree::GetTabPos( SvTreeListEntry* pEntry, SvLBoxTab* pTab)
{
    sal_IntPtr nLevel = 0;
    if(lcl_IsContent(pEntry))
    {
        nLevel++;
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
            OSL_ENSURE(pParent->GetUserData(), "no UserData?");
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
                                while(pChild &&
                                        lcl_IsContent(pChild) &&
                                        !(static_cast<SwOutlineContent*>(pChild->GetUserData())->GetOutlineLevel() < nLevel)
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
                    SvTreeListEntry* pChild = nullptr;
                    const SwContent* pCnt = pCntType->GetMember(i);
                    if (pCnt)
                    {
                        OUString sEntry = pCnt->GetName();
                        if (sEntry.isEmpty())
                            sEntry = m_sSpace;
                        pChild = InsertEntry(sEntry, pParent, false, TREELIST_APPEND, const_cast<SwContent *>(pCnt));
                    }
                    if (pChild)
                    {
                        //If object is marked , the corresponding entry is set true,
                        //else the corresponding entry is set false .
                        pChild->SetMarked(false);
                        SdrObject * pObj = GetDrawingObjectsByContent(pCnt);
                        if(pObj)
                        {
                            SdrView* pDrawView = m_pActiveShell->GetDrawView();
                            SdrPageView* pPV = pDrawView->/*GetPageViewPvNum*/GetSdrPageView(/*0*/);
                            if( pPV )
                            {
                                bool Marked = pDrawView->IsObjMarked(pObj);
                                if(Marked)
                                {
                                    pChild->SetMarked(true);
                                }
                            }
                        }
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
    if(!m_bIsRoot || (static_cast<SwContentType*>(pParent->GetUserData())->GetType() == ContentTypeId::OUTLINE) ||
            (m_nRootType == ContentTypeId::OUTLINE))
    {
        if(lcl_IsContentType(pParent))
        {
            SwContentType* pCntType = static_cast<SwContentType*>(pParent->GetUserData());
            const sal_Int32 nOr = 1 << (int)pCntType->GetType(); //linear -> Bitposition
            if(m_bIsActive || m_bIsConstant)
            {
                m_nActiveBlock |= nOr;
                m_pConfig->SetActiveBlock(m_nActiveBlock);
            }
            else
                m_nHiddenBlock |= nOr;
            if((pCntType->GetType() == ContentTypeId::OUTLINE))
            {
                std::map< void*, bool > aCurrOutLineNodeMap;

                SwWrtShell* pShell = GetWrtShell();
                bool bBool = SvTreeListBox::Expand(pParent);
                SvTreeListEntry* pChild = Next(pParent);
                while(pChild && lcl_IsContent(pChild) && pParent->HasChildren())
                {
                    if(pChild->HasChildren())
                    {
                        sal_Int32 nPos = static_cast<SwContent*>(pChild->GetUserData())->GetYPos();
                        void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
                        aCurrOutLineNodeMap.insert(std::map<void*, bool>::value_type( key, false ) );
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
            sal_Int32 nPos = static_cast<SwContent*>(pParent->GetUserData())->GetYPos();
            void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
            mOutLineNodeMap[key] = true;
        }
    }
    return SvTreeListBox::Expand(pParent);
}

bool  SwContentTree::Collapse( SvTreeListEntry* pParent )
{
    if(!m_bIsRoot || (static_cast<SwContentType*>(pParent->GetUserData())->GetType() == ContentTypeId::OUTLINE) ||
            (m_nRootType == ContentTypeId::OUTLINE))
    {
        if(lcl_IsContentType(pParent))
        {
            if(m_bIsRoot)
                return false;
            SwContentType* pCntType = static_cast<SwContentType*>(pParent->GetUserData());
            const sal_Int32 nAnd = ~(1 << (int)pCntType->GetType());
            if(m_bIsActive || m_bIsConstant)
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
            sal_Int32 nPos = static_cast<SwContent*>(pParent->GetUserData())->GetYPos();
            void* key = static_cast<void*>(pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos ));
            mOutLineNodeMap[key] = false;
        }
    }

    return SvTreeListBox::Collapse(pParent);
}

// Also on double click will be initially opened only.

IMPL_LINK_NOARG_TYPED(SwContentTree, ContentDoubleClickHdl, SvTreeListBox*, bool)
{
    SvTreeListEntry* pEntry = GetCurEntry();
    // Is it a content type?
    OSL_ENSURE(pEntry, "no current entry!");
    if(pEntry)
    {
        if(lcl_IsContentType(pEntry) && !pEntry->HasChildren())
            RequestingChildren(pEntry);
        else if(!lcl_IsContentType(pEntry) && (m_bIsActive || m_bIsConstant))
        {
            if(m_bIsConstant)
            {
                m_pActiveShell->GetView().GetViewFrame()->GetWindow().ToTop();
            }
            //Jump to content type:
            SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());
            OSL_ENSURE( pCnt, "no UserData");
            GotoContent(pCnt);
            if(pCnt->GetParent()->GetType() == ContentTypeId::FRAME)
                m_pActiveShell->EnterStdMode();
        }
    }
    return false;
}

void SwContentTree::Display( bool bActive )
{
    if(!m_bIsImageListInitialized)
    {
        m_aEntryImages = ImageList(SW_RES(IMG_NAVI_ENTRYBMP));

        if ( GetDPIScaleFactor() > 1 )
        {
            for (short i = 0; i < m_aEntryImages.GetImageCount(); i++)
            {
                OUString rImageName = m_aEntryImages.GetImageName(i);
                BitmapEx b = m_aEntryImages.GetImage(rImageName).GetBitmapEx();
                //Use Lanczos because it looks better with circles / diagonals
                b.Scale(GetDPIScaleFactor(), GetDPIScaleFactor(), BmpScaleFlag::Lanczos);
                m_aEntryImages.ReplaceImage(rImageName, Image(b));
            }
        }
        m_bIsImageListInitialized = true;
    }
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
    if(bActive && !m_bIsConstant && !m_bIsActive)
        m_bIsActive = bActive;
    m_bIsHidden = !bActive;
    SwWrtShell* pShell = GetWrtShell();
    const bool bReadOnly = !pShell || pShell->GetView().GetDocShell()->IsReadOnly();
    if(bReadOnly != m_bIsLastReadOnly)
    {
        m_bIsLastReadOnly = bReadOnly;
        bool bDisable =  pShell == nullptr || bReadOnly;
        SwNavigationPI* pNavi = GetParentWindow();
        pNavi->m_aContentToolBox->EnableItem(FN_ITEM_UP , !bDisable);
        pNavi->m_aContentToolBox->EnableItem(FN_ITEM_DOWN, !bDisable);
        pNavi->m_aContentToolBox->EnableItem(FN_ITEM_LEFT, !bDisable);
        pNavi->m_aContentToolBox->EnableItem(FN_ITEM_RIGHT, !bDisable);
        pNavi->m_aContentToolBox->EnableItem(FN_SELECT_SET_AUTO_BOOKMARK, !bDisable);
    }
    if(pShell)
    {
        SvTreeListEntry* pSelEntry = nullptr;
        if(m_nRootType == ContentTypeId::UNKNOWN)
        {
            for( ContentTypeId nCntType : o3tl::enumrange<ContentTypeId>() )
            {
                SwContentType** ppContentT = bActive ?
                                &m_aActiveContentArr[nCntType] :
                                    &m_aHiddenContentArr[nCntType];
                if(!*ppContentT)
                    (*ppContentT) = new SwContentType(pShell, nCntType, m_nOutlineLevel );

                OUString sEntry = (*ppContentT)->GetName();
                SvTreeListEntry* pEntry;
                const Image& rImage = m_aEntryImages.GetImage(SID_SW_START + (int)nCntType);
                bool bChOnDemand = 0 != (*ppContentT)->GetMemberCount();
                pEntry = InsertEntry(sEntry, rImage, rImage,
                                nullptr, bChOnDemand, TREELIST_APPEND, (*ppContentT));
                if(nCntType == m_nLastSelType)
                    pSelEntry = pEntry;
                sal_Int32 nExpandOptions = m_bIsActive || m_bIsConstant ?
                                            m_nActiveBlock :
                                                m_nHiddenBlock;
                if(nExpandOptions & (1 << (int)nCntType))
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
            SwContentType** ppRootContentT = bActive ?
                                &m_aActiveContentArr[m_nRootType] :
                                    &m_aHiddenContentArr[m_nRootType];
            if(!(*ppRootContentT))
                (*ppRootContentT) = new SwContentType(pShell, m_nRootType, m_nOutlineLevel );
            const Image& rImage = m_aEntryImages.GetImage(20000 + (int)m_nRootType);
            SvTreeListEntry* pParent = InsertEntry(
                    (*ppRootContentT)->GetName(), rImage, rImage,
                        nullptr, false, TREELIST_APPEND, *ppRootContentT);

            if(m_nRootType != ContentTypeId::OUTLINE)
            {
                for(size_t i = 0; i < (*ppRootContentT)->GetMemberCount(); ++i)
                {
                    const SwContent* pCnt = (*ppRootContentT)->GetMember(i);
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
            if( m_nRootType == ContentTypeId::OUTLINE && m_bIsActive )
            {
                // find out where the cursor is
                const sal_uInt16 nActPos = pShell->GetOutlinePos(MAXLEVEL);
                SvTreeListEntry* pEntry = First();

                while( nullptr != (pEntry = Next(pEntry)) )
                {
                    if(static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetPos() == nActPos)
                    {
                        MakeVisible(pEntry);
                        Select(pEntry);
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
        ScrollOutputArea( (short)nDelta );
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
    SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());

    const ContentTypeId nActType = pCnt->GetParent()->GetType();
    OUString sUrl;
    bool bOutline = false;
    OUString sOutlineText;
    switch( nActType )
    {
        case ContentTypeId::OUTLINE:
        {
            const sal_uInt16 nPos = static_cast<SwOutlineContent*>(pCnt)->GetPos();
            OSL_ENSURE(nPos < pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount(),
                       "outlinecnt changed");

            // make sure outline may actually be copied
            if( pWrtShell->IsOutlineCopyable( nPos ) )
            {
                const SwNumRule* pOutlRule = pWrtShell->GetOutlineNumRule();
                const SwTextNode* pTextNd =
                        pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNode(nPos);
                if( pTextNd && pOutlRule && pTextNd->IsNumbered())
                {
                    SwNumberTree::tNumberVector aNumVector =
                        pTextNd->GetNumberVector();
                    for( int nLevel = 0;
                         nLevel <= pTextNd->GetActualListLevel();
                         nLevel++ )
                    {
                        const SwNumberTree::tSwNumTreeNumber nVal = aNumVector[nLevel] + 1;
                        sEntry += OUString::number( nVal - pOutlRule->Get(nLevel).GetStart() );
                        sEntry += ".";
                    }
                }
                sEntry += pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(nPos, false);
                sOutlineText = pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(nPos);
                m_bIsOutlineMoveable = static_cast<SwOutlineContent*>(pCnt)->IsMoveable();
                bOutline = true;
            }
        }
        break;
        case ContentTypeId::POSTIT:
        case ContentTypeId::INDEX:
        case ContentTypeId::REFERENCE :
            // cannot be inserted, neither as URL nor as region
        break;
        case ContentTypeId::URLFIELD:
            sUrl = static_cast<SwURLFieldContent*>(pCnt)->GetURL();
            SAL_FALLTHROUGH;
        case ContentTypeId::OLE:
        case ContentTypeId::GRAPHIC:
            if(GetParentWindow()->GetRegionDropMode() != RegionMode::NONE)
                break;
            else
                rDragMode &= ~( DND_ACTION_MOVE | DND_ACTION_LINK );
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
            else if(m_bIsConstant &&
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
                sUrl += OUStringLiteral1<cMarkSeparator>() + rToken;
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

// Switch the display to Root

bool SwContentTree::ToggleToRoot()
{
    if(!m_bIsRoot)
    {
        SvTreeListEntry* pEntry = GetCurEntry();
        const SwContentType* pCntType;
        if(pEntry)
        {
            if(lcl_IsContentType(pEntry))
                pCntType = static_cast<SwContentType*>(pEntry->GetUserData());
            else
                pCntType = static_cast<SwContent*>(pEntry->GetUserData())->GetParent();
            m_nRootType = pCntType->GetType();
            m_bIsRoot = true;
            Display(m_bIsActive || m_bIsConstant);
        }
    }
    else
    {
        m_nRootType = ContentTypeId::UNKNOWN;
        m_bIsRoot = false;
        FindActiveTypeAndRemoveUserData();
        Display(m_bIsActive || m_bIsConstant);
        if( m_bIsKeySpace )
        {
            HideFocus();
            ShowFocus( m_aOldRectangle);
            m_bIsKeySpace = false;
        }
    }
    m_pConfig->SetRootType( m_nRootType );
    GetParentWindow()->m_aContentToolBox->CheckItem(FN_SHOW_ROOT, m_bIsRoot);
    return m_bIsRoot;
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

    if(!m_bIsActive && ! m_bIsConstant)
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
            const ContentTypeId nType = static_cast<SwContentType*>(pEntry->GetUserData())->GetType();
            bOutline = m_nRootType == ContentTypeId::OUTLINE;
            SwContentType* pArrType = m_aActiveContentArr[nType];
            if(!pArrType)
                bRepaint = true;
            else
            {
                SvTreeListEntry* pFirstSel;
                if(bOutline &&
                        nullptr != ( pFirstSel = FirstSelected()) &&
                            lcl_IsContent(pFirstSel))
                {
                    const auto nSelLevel =
                        static_cast<SwOutlineContent*>(pFirstSel->GetUserData())->GetOutlineLevel();
                    SwWrtShell* pSh = GetWrtShell();
                    const sal_uInt16 nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
                    if (nOutlinePos != USHRT_MAX &&
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
        if( !bRepaint && bOutline )
        {
            // find out where the cursor is
            const sal_uInt16 nActPos = GetWrtShell()->GetOutlinePos(MAXLEVEL);
            SvTreeListEntry* pFirstEntry = First();

            while( nullptr != (pFirstEntry = Next(pFirstEntry)) )
            {
                if(static_cast<SwOutlineContent*>(pFirstEntry->GetUserData())->GetPos() == nActPos)
                {
                    if(FirstSelected() != pFirstEntry)
                    {
                        Select(pFirstEntry);
                        MakeVisible(pFirstEntry);
                    }
                }
            }

        }

    }
    else
    {
        SvTreeListEntry* pEntry = First();
        while ( pEntry )
        {
            bool bNext = true; // at least a next must be
            SwContentType* pTreeType = static_cast<SwContentType*>(pEntry->GetUserData());
            const size_t nTreeCount = pTreeType->GetMemberCount();
            const ContentTypeId nType = pTreeType->GetType();
            SwContentType* pArrType = m_aActiveContentArr[nType];
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
                        SvTreeListEntry* pChild = FirstChild(pEntry);
                        SvTreeListEntry* pRemove = pChild;
                        for(size_t j = 0; j < nChildCount; ++j)
                        {
                            pChild = Next(pRemove);
                            GetModel()->Remove(pRemove);
                            pRemove = pChild;
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
            m_nLastSelType = static_cast<SwContentType*>(pEntry->GetUserData())->GetType();
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
    m_bIsHidden = true;
    m_bIsActive = m_bIsConstant = false;
    FindActiveTypeAndRemoveUserData();
    for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
    {
        DELETEZ(m_aHiddenContentArr[i]);
    }
    Display(m_bIsActive);

    GetParentWindow()->UpdateListBox();
}

void SwContentTree::SetActiveShell(SwWrtShell* pSh)
{
    if(m_bIsInternalDrag)
        m_bDocChgdInDragging = true;
    bool bClear = m_pActiveShell != pSh;
    if(m_bIsActive && bClear)
    {
        if (m_pActiveShell)
            EndListening(*m_pActiveShell->GetView().GetDocShell());
        m_pActiveShell = pSh;
        FindActiveTypeAndRemoveUserData();
        Clear();
    }
    else if(m_bIsConstant)
    {
        if (m_pActiveShell)
            EndListening(*m_pActiveShell->GetView().GetDocShell());
        m_pActiveShell = pSh;
        m_bIsActive = true;
        m_bIsConstant = false;
        bClear = true;
    }
    // Only if it is the active view, the array will be deleted and
    // the screen filled new.
    if(m_bIsActive && bClear)
    {
        if (m_pActiveShell)
            StartListening(*m_pActiveShell->GetView().GetDocShell());
        FindActiveTypeAndRemoveUserData();
        for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
        {
            DELETEZ(m_aActiveContentArr[i]);
        }
        Display(true);
    }
}

void SwContentTree::SetConstantShell(SwWrtShell* pSh)
{
    if (m_pActiveShell)
        EndListening(*m_pActiveShell->GetView().GetDocShell());
    m_pActiveShell = pSh;
    m_bIsActive       = false;
    m_bIsConstant     = true;
    StartListening(*m_pActiveShell->GetView().GetDocShell());
    FindActiveTypeAndRemoveUserData();
    for(ContentTypeId i : o3tl::enumrange<ContentTypeId>())
    {
        DELETEZ(m_aActiveContentArr[i]);
    }
    Display(true);
}


void SwContentTree::Notify(SfxBroadcaster & rBC, SfxHint const& rHint)
{
    SfxViewEventHint const*const pVEHint(
            dynamic_cast<SfxViewEventHint const*>(&rHint));
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
}

void SwContentTree::ExecCommand(sal_uInt16 nCmd, bool bModifier)
{
    bool bMove = false;
    switch( nCmd )
    {
        case FN_ITEM_DOWN:
        case FN_ITEM_UP:
            bMove = true;
            SAL_FALLTHROUGH;
        case FN_ITEM_LEFT:
        case FN_ITEM_RIGHT:
        if( !GetWrtShell()->GetView().GetDocShell()->IsReadOnly() &&
                (m_bIsActive ||
                    (m_bIsConstant && m_pActiveShell == GetParentWindow()->GetCreateView()->GetWrtShellPtr())))
        {
            SwWrtShell* pShell = GetWrtShell();
            sal_Int8 nActOutlineLevel = m_nOutlineLevel;
            sal_uInt16 nActPos = pShell->GetOutlinePos(nActOutlineLevel);
            SvTreeListEntry* pFirstEntry = FirstSelected();
            if (pFirstEntry && lcl_IsContent(pFirstEntry))
            {
                if ( (m_bIsRoot && m_nRootType == ContentTypeId::OUTLINE) ||
                    static_cast<SwContent*>(pFirstEntry->GetUserData())->GetParent()->GetType()
                                                ==  ContentTypeId::OUTLINE)
                {
                    nActPos = static_cast<SwOutlineContent*>(pFirstEntry->GetUserData())->GetPos();
                }
            }
            if ( nActPos < USHRT_MAX &&
                    ( !bMove || pShell->IsOutlineMovable( nActPos )) )
            {
                pShell->StartAllAction();
                pShell->GotoOutline( nActPos); // If text selection != box selection
                pShell->Push();
                pShell->MakeOutlineSel( nActPos, nActPos,
                                    bModifier);
                if( bMove )
                {
                    short nDir = nCmd == FN_ITEM_UP ? -1 : 1;
                    if( !bModifier && ( (nDir == -1 && nActPos > 0) ||
                        (nDir == 1 && nActPos < GetEntryCount() - 2) ) )
                    {
                        pShell->MoveOutlinePara( nDir );
                        // Set cursor back to the current position
                        pShell->GotoOutline( nActPos + nDir);
                    }
                    else if(bModifier && pFirstEntry)
                    {
                        sal_uInt16 nActEndPos = nActPos;
                        SvTreeListEntry* pEntry = pFirstEntry;
                        const auto nActLevel = static_cast<SwOutlineContent*>(
                                pFirstEntry->GetUserData())->GetOutlineLevel();
                        pEntry = Next(pEntry);
                        while( pEntry && CTYPE_CNT ==
                            static_cast<SwTypeNumber*>(pEntry->GetUserData())->GetTypeId() )
                        {
                            if(nActLevel >= static_cast<SwOutlineContent*>(
                                pEntry->GetUserData())->GetOutlineLevel())
                                break;
                            pEntry = Next(pEntry);
                            nActEndPos++;
                        }
                        if(nDir == 1)
                        {
                            // If the last entry is to be moved it is over!
                            if(pEntry && CTYPE_CNT ==
                                static_cast<SwTypeNumber*>(pEntry->GetUserData())->GetTypeId())
                            {
                                // pEntry now points to the following entry of the last
                                // selected entry.
                                sal_uInt16 nDest = nActEndPos + 1;
                                // here needs to found the next record after next.
                                // The selection must be inserted in front of.
                                while(pEntry )
                                {
                                    pEntry = Next(pEntry);
                                    // nDest++ may only executed if pEntry != 0
                                    if(pEntry && nDest++ &&
                                       ( nActLevel >= static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlineLevel()||
                                         CTYPE_CNT != static_cast<SwTypeNumber*>(pEntry->GetUserData())->GetTypeId()))
                                    {
                                        nDest--;
                                        break;
                                    }
                                }
                                nDir = nDest - nActEndPos;
                                // If no entry was found which corresponds the condition
                                // of the previously paste, it needs to be pushed slightly less.
                            }
                            else
                                nDir = 0;
                        }
                        else
                        {
                            sal_uInt16 nDest = nActPos;
                            pEntry = pFirstEntry;
                            while(pEntry && nDest )
                            {
                                nDest--;
                                pEntry = Prev(pEntry);
                                if(pEntry &&
                                    (nActLevel >= static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetOutlineLevel()||
                                     CTYPE_CNT != static_cast<SwTypeNumber*>(pEntry->GetUserData())->GetTypeId()))
                                {
                                    break;
                                }
                            }
                            nDir = nDest - nActPos;
                        }
                        if(nDir)
                        {
                            pShell->MoveOutlinePara( nDir );
                            //Set cursor back to the current position
                            pShell->GotoOutline( nActPos + nDir);
                        }
                    }
                }
                else
                {
                    if( !pShell->IsProtectedOutlinePara() )
                        pShell->OutlineUpDown( nCmd == FN_ITEM_LEFT ? -1 : 1 );
                }

                pShell->ClearMark();
                pShell->Pop(false); // Cursor is now back at the current superscription.
                pShell->EndAllAction();
                if(m_aActiveContentArr[ContentTypeId::OUTLINE])
                    m_aActiveContentArr[ContentTypeId::OUTLINE]->Invalidate();
                Display(true);
                if(!m_bIsRoot)
                {
                    const sal_uInt16 nCurrPos = pShell->GetOutlinePos(MAXLEVEL);
                    SvTreeListEntry* pFirst = First();

                    while( nullptr != (pFirst = Next(pFirst)) && lcl_IsContent(pFirst))
                    {
                        if(static_cast<SwOutlineContent*>(pFirst->GetUserData())->GetPos() == nCurrPos)
                        {
                            Select(pFirst);
                            MakeVisible(pFirst);
                        }
                    }
                }
            }
        }
    }
}

void SwContentTree::ShowTree()
{
    SvTreeListBox::Show();
}

void SwContentTree::Paint( vcl::RenderContext& rRenderContext,
                           const Rectangle& rRect )
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
IMPL_LINK_NOARG_TYPED(SwContentTree, TimerUpdate, Timer *, void)
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
        if( m_bIsConstant && !lcl_FindShell( m_pActiveShell ) )
        {
            SetActiveShell(pActShell);
            GetParentWindow()->UpdateListBox();
        }

        if(m_bIsActive && pActShell != GetWrtShell())
        {
            SetActiveShell(pActShell);
        }
        else if( (m_bIsActive || (m_bIsConstant && pActShell == GetWrtShell())) &&
                    HasContentChanged())
        {
            FindActiveTypeAndRemoveUserData();
            Display(true);
        }
    }
    else if(!pView && m_bIsActive && !m_bIsIdleClear)
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
    DragDropMode eMode = (DragDropMode)0;
    if( m_bIsActive && m_nRootType == ContentTypeId::OUTLINE &&
            GetModel()->GetAbsPos( pEntry ) > 0
            && !GetWrtShell()->GetView().GetDocShell()->IsReadOnly())
        eMode = GetDragDropMode();
    else if(!m_bIsActive && GetWrtShell()->GetView().GetDocShell()->HasName())
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
        sal_uInt16 nTargetPos = 0;
        sal_uInt16 nSourcePos = static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetPos();
        if(!lcl_IsContent(pTarget))
            nTargetPos = USHRT_MAX;
        else
            nTargetPos = static_cast<SwOutlineContent*>(pTarget->GetUserData())->GetPos();
        if( MAXLEVEL > m_nOutlineLevel && // Not all layers are displayed.
                        nTargetPos != USHRT_MAX)
        {
            SvTreeListEntry* pNext = Next(pTarget);
            if(pNext)
                nTargetPos = static_cast<SwOutlineContent*>(pNext->GetUserData())->GetPos() -1;
            else
                nTargetPos = static_cast<sal_uInt16>(GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount())- 1;

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
        sal_uInt16 nTargetPos = 0;
        sal_uInt16 nSourcePos = static_cast<SwOutlineContent*>(pEntry->GetUserData())->GetPos();
        if(!lcl_IsContent(pTarget))
            nTargetPos = USHRT_MAX;
        else
            nTargetPos = static_cast<SwOutlineContent*>(pTarget->GetUserData())->GetPos();

        if( MAXLEVEL > m_nOutlineLevel && // Not all layers are displayed.
                        nTargetPos != USHRT_MAX)
        {
            SvTreeListEntry* pNext = Next(pTarget);
            if(pNext)
                nTargetPos = static_cast<SwOutlineContent*>(pNext->GetUserData())->GetPos() - 1;
            else
                nTargetPos = static_cast<sal_uInt16>(GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount()) - 1;

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
        SvTreeListBox::MouseButtonDown( rMEvt );
}

// Update immediately

void SwContentTree::GetFocus()
{
    SwView* pActView = GetParentWindow()->GetCreateView();
    if(pActView)
    {
        SwWrtShell* pActShell = pActView->GetWrtShellPtr();
        if(m_bIsConstant && !lcl_FindShell(m_pActiveShell))
        {
            SetActiveShell(pActShell);
        }

        if(m_bIsActive && pActShell != GetWrtShell())
            SetActiveShell(pActShell);
        else if( (m_bIsActive || (m_bIsConstant && pActShell == GetWrtShell())) &&
                    HasContentChanged())
        {
            Display(true);
        }
    }
    else if(m_bIsActive)
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
        if( GetChildCount( pEntry ) == 0 )
            m_bIsKeySpace = true;
        Point tempPoint = GetEntryPosition( pEntry );//Change from "GetEntryPos" to "GetEntryPosition" for acc migration
        m_aOldRectangle = GetFocusRect( pEntry,tempPoint.Y() );

        if(pEntry)
        {
            if(m_bIsActive || m_bIsConstant)
            {
                if(m_bIsConstant)
                {
                    m_pActiveShell->GetView().GetViewFrame()->GetWindow().ToTop();
                }

                SwContent* pCnt = static_cast<SwContent*>(pEntry->GetUserData());

                switch(pCnt->GetParent()->GetType())
                {
                    case ContentTypeId::DRAWOBJECT:
                    {
                        SdrView* pDrawView = m_pActiveShell->GetDrawView();
                        if (pDrawView)
                        {
                            pDrawView->SdrEndTextEdit();//Change from "EndTextEdit" to "SdrEndTextEdit" for acc migration

                            SwDrawModel* pDrawModel = m_pActiveShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
                            SdrPage* pPage = pDrawModel->GetPage(0);
                            const size_t nCount = pPage->GetObjCount();
                            bool hasObjectMarked = false;

                            SdrObject* pObject = nullptr;
                            pObject = GetDrawingObjectsByContent( pCnt );
                            if( pObject )
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
                                    case OBJ_TEXTEXT:
                                    case OBJ_wegFITTEXT:
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
                    break;
                    default: break;
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
                nType = static_cast<SwContentType*>(pUserData)->GetType();
            else
            {
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
                        sEntry = static_cast<SwURLFieldContent*>(pUserData)->GetURL();
                        bRet = true;
                    break;

                    case ContentTypeId::POSTIT:
                        sEntry = static_cast<SwPostItContent*>(pUserData)->GetName();
                        bRet = true;
                        if(Help::IsBalloonHelpEnabled())
                            bBalloon = true;
                    break;
                    case ContentTypeId::OUTLINE:
                        sEntry = static_cast<SwOutlineContent*>(pUserData)->GetName();
                        bRet = true;
                    break;
                    case ContentTypeId::GRAPHIC:
                        sEntry = static_cast<SwGraphicContent*>(pUserData)->GetLink();
#if OSL_DEBUG_LEVEL > 1
                        sEntry += " ";
                        sEntry += OUString::number(
                                    ((SwGraphicContent*)pUserData)->GetYPos());
#endif
                        bRet = true;
                    break;
#if OSL_DEBUG_LEVEL > 1
                    case ContentTypeId::TABLE:
                    case ContentTypeId::FRAME:
                        sEntry = OUString::number(
                                        ((SwContent*)pUserData)->GetYPos() );
                        bRet = true;
                    break;
#endif
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
                if (pItem && SV_ITEM_ID_LBOXSTRING == pItem->GetType())
                {
                    aPos = GetEntryPosition( pEntry );

                    aPos.X() = GetTabPos( pEntry, pTab );
                    Size aSize( pItem->GetSize( this, pEntry ) );

                    if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                        aSize.Width() = GetSizePixel().Width() - aPos.X();

                    aPos = OutputToScreenPixel(aPos);
                    Rectangle aItemRect( aPos, aSize );
                    if(bBalloon)
                    {
                        aPos.X() += aSize.Width();
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
                Help::ShowQuickHelp( this, Rectangle(), OUString() );
                bCallBase = false;
            }
        }
    }
    if( bCallBase )
        Window::RequestHelp( rHEvt );
}

void SwContentTree::ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
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
                SetOutlineLevel((sal_Int8)nSelectedPopupEntry);
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
                m_bViewHasChanged = m_bIsActive = nSelectedPopupEntry == 1;
                m_bIsConstant = false;
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
    SwContentType** ppContentT = m_bIsActive ?
                    &m_aActiveContentArr[ContentTypeId::OUTLINE] :
                        &m_aHiddenContentArr[ContentTypeId::OUTLINE];
    if(*ppContentT)
    {
        (*ppContentT)->SetOutlineLevel(m_nOutlineLevel);
        (*ppContentT)->Init();
    }
    Display(m_bIsActive);
}

// Mode Change: Show dropped Doc

void SwContentTree::ShowHiddenShell()
{
    if(m_pHiddenShell)
    {
        m_bIsConstant = false;
        m_bIsActive = false;
        Display(false);
    }
}

// Mode Change: Show active view

void SwContentTree::ShowActualView()
{
    m_bIsActive = true;
    m_bIsConstant = false;
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
    pNavi->m_aContentToolBox->EnableItem(FN_ITEM_UP ,  bEnable);
    pNavi->m_aContentToolBox->EnableItem(FN_ITEM_DOWN, bEnable);
    pNavi->m_aContentToolBox->EnableItem(FN_ITEM_LEFT, bEnable);
    pNavi->m_aContentToolBox->EnableItem(FN_ITEM_RIGHT,bEnable);

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

void SwContentTree::EditEntry(SvTreeListEntry* pEntry, EditEntryMode nMode)
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
                OUString sTable = SW_RES(STR_TABLE_NAME);
                SwRewriter aRewriterTableName;
                aRewriterTableName.AddRule(UndoArg1, SW_RES(STR_START_QUOTE));
                aRewriterTableName.AddRule(UndoArg2, pCnt->GetName());
                aRewriterTableName.AddRule(UndoArg3, SW_RES(STR_END_QUOTE));
                sTable = aRewriterTableName.Apply(sTable);

                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, sTable);
                m_pActiveShell->StartUndo(UNDO_DELETE, &aRewriter);
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
                if (static_cast<SwPostItContent*>(pCnt)->IsPostIt())
                {
                    m_pActiveShell->GetView().GetPostItMgr()->SetActiveSidebarWin(nullptr);
                    m_pActiveShell->DelRight();
                }
            }
            else
            {
                if (static_cast<SwPostItContent*>(pCnt)->IsPostIt())
                    nSlot = FN_POSTIT;
                else
                    nSlot = FN_REDLINE_COMMENT;
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
                            GetDispatcher()->Execute(FN_INSERT_MULTI_TOX,
                                            SfxCallMode::ASYNCHRON, &aPtrItem, 0L);

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
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        std::unique_ptr<AbstractSwRenameXNamedDlg> pDlg(pFact->CreateSwRenameXNamedDlg(this, xNamed, xNameAccess));
        OSL_ENSURE(pDlg, "Dialog creation failed!");
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
            m_pActiveShell->GotoOutline(static_cast<SwOutlineContent*>(pCnt)->GetPos());
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
            const OUString sName(pCnt->GetName());
            if (!m_pActiveShell->GotoNextTOXBase(&sName))
                m_pActiveShell->GotoPrevTOXBase(&sName);
        }
        break;
        case ContentTypeId::POSTIT:
            m_pActiveShell->GetView().GetPostItMgr()->AssureStdModeAtShell();
            if (static_cast<SwPostItContent*>(pCnt)->IsPostIt())
                m_pActiveShell->GotoFormatField(*static_cast<SwPostItContent*>(pCnt)->GetPostIt());
            else
                m_pActiveShell->GetView().GetDocShell()->GetWrtShell()->GotoRedline(
                        m_pActiveShell->GetView().GetDocShell()->GetWrtShell()->FindRedlineOfData(static_cast<SwPostItContent*>(pCnt)->GetRedline()->GetRedlineData()));

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
                    if (pTemp->GetName().equals(pCnt->GetName()))
                    {
                        SdrPageView* pPV = pDrawView->GetSdrPageView();
                        if( pPV )
                        {
                            pDrawView->MarkObj( pTemp, pPV );
                        }
                    }
                }
                m_pActiveShell->GetNavigationMgr().addEntry(aPos);
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
                    OString::number((int)nDefDrag) + OString(NAVI_BOOKMARK_DELIM) +
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
    SwContentLBoxString( SvTreeListEntry* pEntry, sal_uInt16 nFlags,
        const OUString& rStr ) : SvLBoxString(pEntry,nFlags,rStr) {}

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
    pEntry->ReplaceItem(o3tl::make_unique<SwContentLBoxString>(pEntry, 0, rCol.GetText()), nColToHilite);
}

void SwContentLBoxString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                                const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{
    if (lcl_IsContent(&rEntry) && static_cast<SwContent *>(rEntry.GetUserData())->IsInvisible())
    {
        vcl::Font aOldFont(rRenderContext.GetFont());
        vcl::Font aFont(aOldFont);
        Color aCol(COL_LIGHTGRAY);
        aFont.SetColor(aCol);
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

        m_bIsImageListInitialized = false;
        Display(true);
    }

    SvTreeListBox::DataChanged( rDCEvt );
}

SwNavigationPI* SwContentTree::GetParentWindow()
{
    return static_cast<SwNavigationPI*>(Window::GetParent());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
