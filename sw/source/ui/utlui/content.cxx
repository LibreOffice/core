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
#include <tools/urlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/help.hxx>
#include <sot/formats.hxx>
#include <uiitems.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <fmtfld.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
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
#include <unotools.hxx>
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
#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <o3tl/sorted_vector.hxx>
#include "svtools/treelistentry.hxx"

#include "swabstdlg.hxx"
#include "globals.hrc"
#include <unomid.h>

#include "navmgr.hxx"

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
    static bool lcl_IsContent(const SvTreeListEntry* pEntry)
    {
        return ((const SwTypeNumber*)pEntry->GetUserData())->GetTypeId() == CTYPE_CNT;
    }

    static bool lcl_IsContentType(const SvTreeListEntry* pEntry)
    {
        return ((const SwTypeNumber*)pEntry->GetUserData())->GetTypeId() == CTYPE_CTT;
    }

    static bool lcl_FindShell(SwWrtShell* pShell)
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

    static bool lcl_IsUiVisibleBookmark(const IDocumentMarkAccess::pMark_t& rpMark)
    {
        return IDocumentMarkAccess::GetType(*rpMark) == IDocumentMarkAccess::BOOKMARK;
    }
}

// Content, contains names and reference at the content type.

SwContent::SwContent(const SwContentType* pCnt, const String& rName, long nYPos) :
    SwTypeNumber(CTYPE_CNT),
    pParent(pCnt),
    sContentName(rName),
    nYPosition(nYPos),
    bInvisible(sal_False)
{
}

sal_uInt8 SwTypeNumber::GetTypeId() const
{
    return nTypeId;
}

SwTypeNumber::~SwTypeNumber()
{
}

sal_Bool SwContent::IsProtect() const
{
    return sal_False;
}

sal_Bool SwPostItContent::IsProtect() const
{
    if (mbPostIt)
        return pFld->IsProtect();
    else
        return false;
}

sal_Bool SwURLFieldContent::IsProtect() const
{
    return pINetAttr->IsProtect();
}

SwGraphicContent::~SwGraphicContent()
{
}

SwTOXBaseContent::~SwTOXBaseContent()
{
}

// Content type, knows it's contents and the WrtShell.

SwContentType::SwContentType(SwWrtShell* pShell, sal_uInt16 nType, sal_uInt8 nLevel) :
    SwTypeNumber(CTYPE_CTT),
    pWrtShell(pShell),
    pMember(0),
    sContentTypeName(SW_RES(STR_CONTENT_TYPE_FIRST + nType)),
    sSingleContentTypeName(SW_RES(STR_CONTENT_TYPE_SINGLE_FIRST + nType)),
    nMemberCount(0),
    nContentType(nType),
    nOutlineLevel(nLevel),
    bDataValid(false),
    bEdit(false),
    bDelete(true)
{
    Init();
}

// Init

void SwContentType::Init(sal_Bool* pbInvalidateWindow)
{
    // if the MemberCount is changing ...
    sal_uInt16 nOldMemberCount = nMemberCount;
    nMemberCount = 0;
    switch(nContentType)
    {
        case CONTENT_TYPE_OUTLINE   :
        {
            sTypeToken = OUString::createFromAscii(pMarkToOutline);
            sal_uInt16 nOutlineCount = nMemberCount =
                static_cast<sal_uInt16>(pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount());
            if(nOutlineLevel < MAXLEVEL)
            {
                for(sal_uInt16 j = 0; j < nOutlineCount; j++)
                {
                    if(pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(j) > nOutlineLevel )
                        nMemberCount --;
                }
            }
            bDelete = false;
        }
        break;

        case CONTENT_TYPE_TABLE     :
            sTypeToken = OUString::createFromAscii(pMarkToTable);
            nMemberCount = pWrtShell->GetTblFrmFmtCount(true);
            bEdit = true;
        break;

        case CONTENT_TYPE_FRAME     :
        case CONTENT_TYPE_GRAPHIC   :
        case CONTENT_TYPE_OLE       :
        {
            FlyCntType eType = FLYCNTTYPE_FRM;
            sTypeToken = OUString::createFromAscii(pMarkToFrame);
            if(nContentType == CONTENT_TYPE_OLE)
            {
                eType = FLYCNTTYPE_OLE;
                sTypeToken = OUString::createFromAscii(pMarkToOLE);
            }
            else if(nContentType == CONTENT_TYPE_GRAPHIC)
            {
                eType = FLYCNTTYPE_GRF;
                sTypeToken = OUString::createFromAscii(pMarkToGraphic);
            }
            nMemberCount = pWrtShell->GetFlyCount(eType);
            bEdit = true;
        }
        break;
        case CONTENT_TYPE_BOOKMARK:
        {
            IDocumentMarkAccess* const pMarkAccess = pWrtShell->getIDocumentMarkAccess();
            nMemberCount = static_cast<sal_uInt16>(count_if(
                pMarkAccess->getBookmarksBegin(),
                pMarkAccess->getBookmarksEnd(),
                &lcl_IsUiVisibleBookmark));
            sTypeToken = aEmptyStr;
            bEdit = true;
        }
        break;
        case CONTENT_TYPE_REGION :
        {
            SwContentArr*   pOldMember = 0;
            sal_uInt16 nOldRegionCount = 0;
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
            nMemberCount = pWrtShell->GetSectionFmtCount();
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                const SwSectionFmt* pFmt;
                SectionType eTmpType;
                if( (pFmt = &pWrtShell->GetSectionFmt(i))->IsInNodesArr() &&
                (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
                {
                    const String& rSectionName =
                        pFmt->GetSection()->GetSectionName();
                    sal_uInt8 nLevel = 0;
                    SwSectionFmt* pParentFmt = pFmt->GetParent();
                    while(pParentFmt)
                    {
                        nLevel++;
                        pParentFmt = pParentFmt->GetParent();
                    }

                    SwContent* pCnt = new SwRegionContent(this, rSectionName,
                            nLevel,
                            pFmt->FindLayoutRect( sal_False, &aNullPt ).Top());

                    SwPtrMsgPoolItem aAskItem( RES_CONTENT_VISIBLE, 0 );
                    if( !pFmt->GetInfo( aAskItem ) &&
                        !aAskItem.pObject )     // not visible
                        pCnt->SetInvisible();
                    pMember->insert(pCnt);

                    sal_uInt16 nPos = pMember->size() - 1;
                    if(nOldRegionCount > nPos &&
                        ((*pOldMember)[nPos])->IsInvisible()
                                != pCnt->IsInvisible())
                            bInvalidate = true;
                }
            }
            nMemberCount = pMember->size();
            sTypeToken = OUString::createFromAscii(pMarkToRegion);
            bEdit = true;
            bDelete = false;
            if(pOldMember)
            {
                pOldMember->DeleteAndDestroyAll();
                delete pOldMember;
                if(pbInvalidateWindow && bInvalidate)
                    *pbInvalidateWindow = sal_True;
            }
        }
        break;
        case CONTENT_TYPE_INDEX:
        {
            nMemberCount = pWrtShell->GetTOXCount();
            bEdit = true;
            bDelete = false;
        }
        break;
        case CONTENT_TYPE_REFERENCE:
        {
            nMemberCount = pWrtShell->GetRefMarks( 0 );
            bDelete = false;
        }
        break;
        case CONTENT_TYPE_URLFIELD:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(!pMember->empty())
                pMember->DeleteAndDestroyAll();

            SwGetINetAttrs aArr;
            nMemberCount = pWrtShell->GetINetAttrs( aArr );
            for( sal_uInt16 n = 0; n < nMemberCount; ++n )
            {
                SwGetINetAttr* p = &aArr[ n ];
                SwURLFieldContent* pCnt = new SwURLFieldContent(
                                    this,
                                    p->sText,
                                    INetURLObject::decode(
                                        p->rINetAttr.GetINetFmt().GetValue(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ),
                                    &p->rINetAttr,
                                    n );
                pMember->insert( pCnt );
            }
            bEdit = true;
            nOldMemberCount = nMemberCount;
            bDelete = false;
        }
        break;
        case CONTENT_TYPE_POSTIT:
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
                    if ( (*i)->GetBroadCaster()->ISA(SwFmtFld)) // SwPostit
                    {
                        SwFmtFld* aFmtFld = static_cast<SwFmtFld*>((*i)->GetBroadCaster());
                        if (aFmtFld->GetTxtFld() && aFmtFld->IsFldInDoc() &&
                            (*i)->mLayoutStatus!=SwPostItHelper::INVISIBLE )
                        {
                            String sEntry = aFmtFld->GetFld()->GetPar2();
                            RemoveNewline(sEntry);
                            SwPostItContent* pCnt = new SwPostItContent(
                                                this,
                                                sEntry,
                                                (const SwFmtFld*)aFmtFld,
                                                nMemberCount);
                            pMember->insert(pCnt);
                            nMemberCount++;
                        }
                    }
                }
            }
            sTypeToken = aEmptyStr;
            bEdit = true;
            nOldMemberCount = nMemberCount;
        }
        break;
        case CONTENT_TYPE_DRAWOBJECT:
        {
            sTypeToken = aEmptyStr;
            nMemberCount = 0;
            SdrModel* pModel = pWrtShell->getIDocumentDrawModelAccess()->GetDrawModel();
            if(pModel)
            {
                SdrPage* pPage = pModel->GetPage(0);
                sal_uInt32 nCount = pPage->GetObjCount();
                for( sal_uInt32 i=0; i< nCount; i++ )
                {
                    SdrObject* pTemp = pPage->GetObj(i);
                    // #i51726# - all drawing objects can be named now
                    if (!pTemp->GetName().isEmpty())
                        nMemberCount++;
                }
            }
        }
        break;
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

// Deliver content, for that if necessary fill the list

const SwContent* SwContentType::GetMember(sal_uInt16 nIndex)
{
    if(!bDataValid || !pMember)
    {
        FillMemberList();
    }
    if(nIndex < pMember->size())
        return (*pMember)[nIndex];
    else
        return 0;

}

void    SwContentType::Invalidate()
{
    bDataValid = false;
}

// Fill the List of contents

void    SwContentType::FillMemberList(sal_Bool* pbLevelOrVisibilityChanged)
{
    SwContentArr*   pOldMember = 0;
    int nOldMemberCount = -1;
    SwPtrMsgPoolItem aAskItem( RES_CONTENT_VISIBLE, 0 );
    if(pMember && pbLevelOrVisibilityChanged)
    {
        pOldMember = pMember;
        nOldMemberCount = pOldMember->size();
        pMember = new SwContentArr;
        *pbLevelOrVisibilityChanged = sal_False;
    }
    else if(!pMember)
        pMember = new SwContentArr;
    else if(!pMember->empty())
        pMember->DeleteAndDestroyAll();
    switch(nContentType)
    {
        case CONTENT_TYPE_OUTLINE   :
        {
            sal_uInt16 nOutlineCount = nMemberCount =
                static_cast<sal_uInt16>(pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount());

            sal_uInt16 nPos = 0;
            for (sal_uInt16 i = 0; i < nOutlineCount; ++i)
            {
                const sal_Int8 nLevel = (sal_Int8)pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineLevel(i);
                if(nLevel >= nOutlineLevel )
                    nMemberCount--;
                else
                {
                    String aEntry(comphelper::string::stripStart(
                        pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(i), ' '));
                    SwNavigationPI::CleanEntry( aEntry );
                    SwOutlineContent* pCnt = new SwOutlineContent(this, aEntry, i, nLevel,
                                                        pWrtShell->IsOutlineMovable( i ), nPos );
                    pMember->insert(pCnt);//, nPos);
                    // with the same number and existing "pOldMember" the
                    // old one is compared with the new OutlinePos.
                    // cast for Win16
                    if(nOldMemberCount > (int)nPos &&
                        ((SwOutlineContent*)(*pOldMember)[nPos])->GetOutlineLevel() != nLevel)
                        *pbLevelOrVisibilityChanged = sal_True;

                    nPos++;
                }
            }

        }
        break;

        case CONTENT_TYPE_TABLE     :
        {
            OSL_ENSURE(nMemberCount == pWrtShell->GetTblFrmFmtCount(true),
                       "MemberCount differs");
            Point aNullPt;
            nMemberCount =  pWrtShell->GetTblFrmFmtCount(true);
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                const SwFrmFmt& rTblFmt = pWrtShell->GetTblFrmFmt(i, true);
                const OUString sTblName( rTblFmt.GetName() );

                SwContent* pCnt = new SwContent(this, sTblName,
                        rTblFmt.FindLayoutRect(sal_False, &aNullPt).Top() );
                if( !rTblFmt.GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();

                pMember->insert(pCnt);

                if(nOldMemberCount > (int)i &&
                    (*pOldMember)[i]->IsInvisible() != pCnt->IsInvisible())
                        *pbLevelOrVisibilityChanged = sal_True;
            }
        }
        break;
        case CONTENT_TYPE_OLE       :
        case CONTENT_TYPE_FRAME     :
        case CONTENT_TYPE_GRAPHIC   :
        {
            FlyCntType eType = FLYCNTTYPE_FRM;
            if(nContentType == CONTENT_TYPE_OLE)
                eType = FLYCNTTYPE_OLE;
            else if(nContentType == CONTENT_TYPE_GRAPHIC)
                eType = FLYCNTTYPE_GRF;
            OSL_ENSURE(nMemberCount ==  pWrtShell->GetFlyCount(eType),
                    "MemberCount differs");
            Point aNullPt;
            nMemberCount = pWrtShell->GetFlyCount(eType);
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                const SwFrmFmt* pFrmFmt = pWrtShell->GetFlyNum(i,eType);
                const OUString sFrmName = pFrmFmt->GetName();

                SwContent* pCnt;
                if(CONTENT_TYPE_GRAPHIC == nContentType)
                {
                    String sLink;
                    pWrtShell->GetGrfNms( &sLink, 0, (SwFlyFrmFmt*) pFrmFmt);
                    pCnt = new SwGraphicContent(this, sFrmName,
                                INetURLObject::decode( sLink, INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ),
                                pFrmFmt->FindLayoutRect(sal_False, &aNullPt).Top());
                }
                else
                {
                    pCnt = new SwContent(this, sFrmName,
                            pFrmFmt->FindLayoutRect(sal_False, &aNullPt).Top() );
                }
                if( !pFrmFmt->GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();
                pMember->insert(pCnt);
                if(nOldMemberCount > (int)i &&
                    (*pOldMember)[i]->IsInvisible() != pCnt->IsInvisible())
                        *pbLevelOrVisibilityChanged = sal_True;
            }
        }
        break;
        case CONTENT_TYPE_BOOKMARK:
        {
            IDocumentMarkAccess* const pMarkAccess = pWrtShell->getIDocumentMarkAccess();
            for(IDocumentMarkAccess::const_iterator_t ppBookmark = pMarkAccess->getBookmarksBegin();
                ppBookmark != pMarkAccess->getBookmarksEnd();
                ++ppBookmark)
            {
                if(lcl_IsUiVisibleBookmark(*ppBookmark))
                {
                    const String& rBkmName = ppBookmark->get()->GetName();
                    //nYPos from 0 -> text::Bookmarks will be sorted alphabetically
                    SwContent* pCnt = new SwContent(this, rBkmName, 0);
                    pMember->insert(pCnt);
                }
            }
        }
        break;
        case CONTENT_TYPE_REGION    :
        {
            const Point aNullPt;
            nMemberCount = pWrtShell->GetSectionFmtCount();
            for(sal_uInt16 i = 0; i < nMemberCount; i++)
            {
                const SwSectionFmt* pFmt;
                SectionType eTmpType;
                if( (pFmt = &pWrtShell->GetSectionFmt(i))->IsInNodesArr() &&
                (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType )
                {
                    String sSectionName = pFmt->GetSection()->GetSectionName();

                    sal_uInt8 nLevel = 0;
                    SwSectionFmt* pParentFmt = pFmt->GetParent();
                    while(pParentFmt)
                    {
                        nLevel++;
                        pParentFmt = pParentFmt->GetParent();
                    }

                    SwContent* pCnt = new SwRegionContent(this, sSectionName,
                            nLevel,
                            pFmt->FindLayoutRect( sal_False, &aNullPt ).Top());
                    if( !pFmt->GetInfo( aAskItem ) &&
                        !aAskItem.pObject )     // not visible
                        pCnt->SetInvisible();
                    pMember->insert(pCnt);

                    sal_uInt16 nPos = pMember->size() - 1;
                    if(nOldMemberCount > nPos &&
                        (*pOldMember)[nPos]->IsInvisible()
                                != pCnt->IsInvisible())
                            *pbLevelOrVisibilityChanged = sal_True;
                }
            }
            nMemberCount = pMember->size();
        }
        break;
        case CONTENT_TYPE_REFERENCE:
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
        case CONTENT_TYPE_URLFIELD:
        {
            SwGetINetAttrs aArr;
            nMemberCount = pWrtShell->GetINetAttrs( aArr );
            for( sal_uInt16 n = 0; n < nMemberCount; ++n )
            {
                SwGetINetAttr* p = &aArr[ n ];
                SwURLFieldContent* pCnt = new SwURLFieldContent(
                                    this,
                                    p->sText,
                                    INetURLObject::decode(
                                        p->rINetAttr.GetINetFmt().GetValue(),
                                        INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_UNAMBIGUOUS,
                                        RTL_TEXTENCODING_UTF8 ),
                                    &p->rINetAttr,
                                    n );
                pMember->insert( pCnt );
            }
        }
        break;
        case CONTENT_TYPE_INDEX:
        {

            sal_uInt16 nCount = nMemberCount = pWrtShell->GetTOXCount();
            for ( sal_uInt16 nTox = 0; nTox < nCount; nTox++ )
            {
                const SwTOXBase* pBase = pWrtShell->GetTOX( nTox );
                String sTOXNm( pBase->GetTOXName() );

                SwContent* pCnt = new SwTOXBaseContent(
                        this, sTOXNm, nTox, *pBase);

                if( !pBase->GetInfo( aAskItem ) &&
                    !aAskItem.pObject )     // not visible
                    pCnt->SetInvisible();

                pMember->insert( pCnt );
                sal_uInt16 nPos = pMember->size() - 1;
                if(nOldMemberCount > nPos &&
                    (*pOldMember)[nPos]->IsInvisible()
                            != pCnt->IsInvisible())
                        *pbLevelOrVisibilityChanged = sal_True;
            }
        }
        break;
        case CONTENT_TYPE_POSTIT:
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
                    if ( (*i)->GetBroadCaster()->ISA(SwFmtFld)) // SwPostit
                    {
                        SwFmtFld* aFmtFld = static_cast<SwFmtFld*>((*i)->GetBroadCaster());
                        if (aFmtFld->GetTxtFld() && aFmtFld->IsFldInDoc() &&
                            (*i)->mLayoutStatus!=SwPostItHelper::INVISIBLE )
                        {
                            String sEntry = aFmtFld->GetFld()->GetPar2();
                            RemoveNewline(sEntry);
                            SwPostItContent* pCnt = new SwPostItContent(
                                                this,
                                                sEntry,
                                                (const SwFmtFld*)aFmtFld,
                                                nMemberCount);
                            pMember->insert(pCnt);
                            nMemberCount++;
                        }
                    }
                }
            }
        }
        break;
        case CONTENT_TYPE_DRAWOBJECT:
        {
            nMemberCount = 0;
            if(!pMember)
                pMember = new SwContentArr;
            else if(!pMember->empty())
                pMember->DeleteAndDestroyAll();

            IDocumentDrawModelAccess* pIDDMA = pWrtShell->getIDocumentDrawModelAccess();
            SdrModel* pModel = pIDDMA->GetDrawModel();
            if(pModel)
            {
                SdrPage* pPage = pModel->GetPage(0);
                sal_uInt32 nCount = pPage->GetObjCount();
                for( sal_uInt32 i=0; i< nCount; i++ )
                {
                    SdrObject* pTemp = pPage->GetObj(i);
                    // #i51726# - all drawing objects can be named now
                    if (!pTemp->GetName().isEmpty())
                    {
                        SwContact* pContact = (SwContact*)pTemp->GetUserCall();
                        long nYPos = 0;
                        const Point aNullPt;
                        if(pContact && pContact->GetFmt())
                            nYPos = pContact->GetFmt()->FindLayoutRect(sal_False, &aNullPt).Top();
                        SwContent* pCnt = new SwContent(
                                            this,
                                            pTemp->GetName(),
                                            nYPos);
                        if(!pIDDMA->IsVisibleLayerId(pTemp->GetLayer()))
                            pCnt->SetInvisible();
                        pMember->insert(pCnt);
                        nMemberCount++;
                        if(nOldMemberCount > (int)i &&
                            (*pOldMember)[i]->IsInvisible() != pCnt->IsInvisible() )
                                *pbLevelOrVisibilityChanged = sal_True;
                    }
                }
            }
        }
        break;
    }
    bDataValid = true;
    if(pOldMember)
        pOldMember->DeleteAndDestroyAll();

}

// TreeListBox for content indicator

SwContentTree::SwContentTree(Window* pParent, const ResId& rResId) :
        SvTreeListBox( pParent, rResId ),

        sSpace(OUString("                    ")),

        sRemoveIdx(SW_RES(ST_REMOVE_INDEX)),
        sUpdateIdx(SW_RES(ST_UPDATE)),
        sUnprotTbl(SW_RES(ST_REMOVE_TBL_PROTECTION)),
        sRename(SW_RES(ST_RENAME)),
        sReadonlyIdx(SW_RES(ST_READONLY_IDX)),
        sInvisible(SW_RES(ST_INVISIBLE)),

    sPostItShow(SW_RES(ST_POSTIT_SHOW)),
    sPostItHide(SW_RES(ST_POSTIT_HIDE)),
    sPostItDelete(SW_RES(ST_POSTIT_DELETE)),

        pHiddenShell(0),
    pActiveShell(0),
    pConfig(SW_MOD()->GetNavigationConfig()),

        nActiveBlock(0),
    nHiddenBlock(0),

        nRootType(USHRT_MAX),
        nLastSelType(USHRT_MAX),
        nOutlineLevel(MAXLEVEL),

        bIsActive(true),
        bIsConstant(false),
        bIsHidden(false),
        bDocChgdInDragging(false),
        bIsInternalDrag(false),
        bIsRoot(false),
        bIsIdleClear(false),
        bIsLastReadOnly(sal_False),
        bIsOutlineMoveable(true),
        bViewHasChanged(false),
        bIsImageListInitialized(false)
{
    sal_uInt16 i;

    SetHelpId(HID_NAVIGATOR_TREELIST);

    SetNodeDefaultImages();
    SetDoubleClickHdl(LINK(this, SwContentTree, ContentDoubleClickHdl));
    SetDragDropMode(SV_DRAGDROP_APP_COPY);
    for( i = 0; i < CONTENT_TYPE_MAX; i++)
    {
        aActiveContentArr[i]    = 0;
        aHiddenContentArr[i]    = 0;
    }
    for( i = 0; i < CONTEXT_COUNT; i++  )
    {
        aContextStrings[i] = SW_RESSTR(i+ST_CONTEXT_FIRST);
    }
    nActiveBlock = pConfig->GetActiveBlock();
    aUpdTimer.SetTimeoutHdl(LINK(this, SwContentTree, TimerUpdate));
    aUpdTimer.SetTimeout(1000);
    Clear();
    EnableContextMenuHandling();
    SetStyle( GetStyle() | WB_QUICK_SEARCH );
}

SwContentTree::~SwContentTree()
{
    Clear(); // If applicable erase content types previously.
    bIsInDrag = false;
}

// Drag&Drop methods

void SwContentTree::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    if( !bIsRoot || nRootType != CONTENT_TYPE_OUTLINE )
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
    SvTreeListBox::DragFinished( bIsInternalDrag ? nAction : DND_ACTION_COPY );
    SwContentTree::SetInDrag(false);
    bIsInternalDrag = false;
}

// QueryDrop will be executed in the navigator

sal_Int8 SwContentTree::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;
    if( bIsRoot )
    {
        if( bIsOutlineMoveable )
            nRet = SvTreeListBox::AcceptDrop( rEvt );
    }
    else if( !bIsInDrag )
        nRet = GetParentWindow()->AcceptDrop( rEvt );
    return nRet;
}

// Drop will be executed in the navigator

static void* lcl_GetOutlineKey( SwContentTree* pTree, SwOutlineContent* pContent)
{
    void* key = 0;
    if( pTree && pContent )
    {
        SwWrtShell* pShell = pTree->GetWrtShell();
        sal_Int32 nPos = pContent->GetYPos();
        if( nPos )
        {
            key = (void*)pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos );
        }
    }
    return key;
}

sal_Int8 SwContentTree::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    SvTreeListEntry* pEntry = pTargetEntry;
    if( pEntry && ( nRootType == CONTENT_TYPE_OUTLINE ) && lcl_IsContent( pEntry ) )
    {
        SwOutlineContent* pOutlineContent = ( SwOutlineContent* )( pEntry->GetUserData() );
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
    if( bIsRoot )
        return SvTreeListBox::ExecuteDrop( rEvt );
    return bIsInDrag ? DND_ACTION_NONE : GetParentWindow()->ExecuteDrop(rEvt);
}

// Handler for Dragging and ContextMenu

PopupMenu* SwContentTree::CreateContextMenu( void )
{
    PopupMenu* pPop = new PopupMenu;
    PopupMenu* pSubPop1 = new PopupMenu;
    PopupMenu* pSubPop2 = new PopupMenu;
    PopupMenu* pSubPop3 = new PopupMenu;
    PopupMenu* pSubPop4 = new PopupMenu; // Edit

    sal_uInt16 i;
    for(i = 1; i <= MAXLEVEL; i++ )
    {
        pSubPop1->InsertItem( i + 100, OUString::number(i));
    }
    pSubPop1->CheckItem(100 + nOutlineLevel);
    for(i=0; i < 3; i++ )
    {
        pSubPop2->InsertItem( i + 201, aContextStrings[
                ST_HYPERLINK - ST_CONTEXT_FIRST + i]);
    }
    pSubPop2->CheckItem( 201 +
                    GetParentWindow()->GetRegionDropMode());
    // Insert the list of the open files
    sal_uInt16 nId = 301;
    const SwView* pActiveView = ::GetActiveView();
    SwView *pView = SwModule::GetFirstView();
    while (pView)
    {
        String sInsert = pView->GetDocShell()->GetTitle();
        if(pView == pActiveView)
        {
            sInsert += '(';
            sInsert += aContextStrings[ ST_ACTIVE - ST_CONTEXT_FIRST];
            sInsert += ')';
        }
        pSubPop3->InsertItem(nId, sInsert);
        if(bIsConstant && pActiveShell == &pView->GetWrtShell())
            pSubPop3->CheckItem(nId);
        pView = SwModule::GetNextView(pView);
        nId++;
    }
    pSubPop3->InsertItem(nId++, aContextStrings[ST_ACTIVE_VIEW - ST_CONTEXT_FIRST]);
    if(pHiddenShell)
    {
        String sHiddenEntry = pHiddenShell->GetView().GetDocShell()->GetTitle();
        sHiddenEntry += OUString(" ( ");
        sHiddenEntry += aContextStrings[ ST_HIDDEN - ST_CONTEXT_FIRST];
        sHiddenEntry += OUString(" )");
        pSubPop3->InsertItem(nId, sHiddenEntry);
    }

    if(bIsActive)
        pSubPop3->CheckItem( --nId );
    else if(bIsHidden)
        pSubPop3->CheckItem( nId );

    pPop->InsertItem( 1, aContextStrings[ST_OUTLINE_LEVEL - ST_CONTEXT_FIRST]);
    pPop->InsertItem(2, aContextStrings[ST_DRAGMODE - ST_CONTEXT_FIRST]);
    pPop->InsertItem(3, aContextStrings[ST_DISPLAY - ST_CONTEXT_FIRST]);
    // Now edit
    SvTreeListEntry* pEntry = 0;
    // Edit only if the shown content is coming from the current view.
    if((bIsActive || pActiveShell == pActiveView->GetWrtShellPtr())
            && 0 != (pEntry = FirstSelected()) && lcl_IsContent(pEntry))
    {
        const SwContentType* pContType = ((SwContent*)pEntry->GetUserData())->GetParent();
        const sal_uInt16 nContentType = pContType->GetType();
        sal_Bool bReadonly = pActiveShell->GetView().GetDocShell()->IsReadOnly();
        bool bVisible = !((SwContent*)pEntry->GetUserData())->IsInvisible();
        sal_Bool bProtected = ((SwContent*)pEntry->GetUserData())->IsProtect();
        bool bEditable = pContType->IsEditable() &&
            ((bVisible && !bProtected) ||CONTENT_TYPE_REGION == nContentType);
        bool bDeletable = pContType->IsDeletable() &&
            ((bVisible && !bProtected) ||CONTENT_TYPE_REGION == nContentType);
        bool bRenamable = bEditable && !bReadonly &&
            (CONTENT_TYPE_TABLE == nContentType ||
                CONTENT_TYPE_FRAME == nContentType ||
                CONTENT_TYPE_GRAPHIC == nContentType ||
                CONTENT_TYPE_OLE == nContentType ||
                CONTENT_TYPE_BOOKMARK == nContentType ||
                CONTENT_TYPE_REGION == nContentType||
                CONTENT_TYPE_INDEX == nContentType);

        if(!bReadonly && (bEditable || bDeletable))
        {
            bool bSubPop4 = false;
            if(CONTENT_TYPE_INDEX == nContentType)
            {
                bSubPop4 = true;
                pSubPop4->InsertItem(401, sRemoveIdx);
                pSubPop4->InsertItem(402, sUpdateIdx);

                const SwTOXBase* pBase = ((SwTOXBaseContent*)pEntry->GetUserData())->GetTOXBase();
                if(!pBase->IsTOXBaseInReadonly())
                    pSubPop4->InsertItem(403, aContextStrings[ST_EDIT_ENTRY - ST_CONTEXT_FIRST]);
                pSubPop4->InsertItem(405, sReadonlyIdx);

                pSubPop4->CheckItem( 405, pActiveShell->IsTOXBaseReadonly(*pBase));
                pSubPop4->InsertItem(501, aContextStrings[ST_DELETE_ENTRY - ST_CONTEXT_FIRST]);
            }
            else if(CONTENT_TYPE_TABLE == nContentType && !bReadonly)
            {
                bSubPop4 = true;
                pSubPop4->InsertItem(403, aContextStrings[ST_EDIT_ENTRY - ST_CONTEXT_FIRST]);
                pSubPop4->InsertItem(404, sUnprotTbl);
                sal_Bool bFull = sal_False;
                String sTblName = ((SwContent*)pEntry->GetUserData())->GetName();
                sal_Bool bProt =pActiveShell->HasTblAnyProtection( &sTblName, &bFull );
                pSubPop4->EnableItem(403, !bFull );
                pSubPop4->EnableItem(404, bProt );
                pSubPop4->InsertItem(501, aContextStrings[ST_DELETE_ENTRY - ST_CONTEXT_FIRST]);
            }
            else if(bEditable || bDeletable)
            {

                if(bEditable && bDeletable)
                {
                    pSubPop4->InsertItem(403, aContextStrings[ST_EDIT_ENTRY - ST_CONTEXT_FIRST]);
                    pSubPop4->InsertItem(501, aContextStrings[ST_DELETE_ENTRY - ST_CONTEXT_FIRST]);
                    bSubPop4 = true;
                }
                else if(bEditable)
                    pPop->InsertItem(403, aContextStrings[ST_EDIT_ENTRY - ST_CONTEXT_FIRST]);
                else if(bDeletable)
                {
                    pSubPop4->InsertItem(501, aContextStrings[ST_DELETE_ENTRY - ST_CONTEXT_FIRST]);
                }
            }
            //Rename object
            if(bRenamable)
            {
                if(bSubPop4)
                    pSubPop4->InsertItem(502, sRename);
                else
                    pPop->InsertItem(502, sRename);
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
        SwContentType* pType = (SwContentType*)pEntry->GetUserData();
        if ( (pType->GetType() == CONTENT_TYPE_POSTIT) &&  (!pActiveShell->GetView().GetDocShell()->IsReadOnly()) && ( pType->GetMemberCount() > 0) )
        {
                pSubPop4->InsertItem(600, sPostItShow );
                pSubPop4->InsertItem(601, sPostItHide );
                pSubPop4->InsertItem(602, sPostItDelete );
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
    sal_uInt16 nLevel = 0;
    if(lcl_IsContent(pEntry))
    {
        nLevel++;
        SwContent* pCnt = (SwContent *) pEntry->GetUserData();
        const SwContentType*    pParent;
        if(pCnt &&  0 != (pParent = pCnt->GetParent()))
        {
            if(pParent->GetType() == CONTENT_TYPE_OUTLINE)
                nLevel = nLevel + ((SwOutlineContent*)pCnt)->GetOutlineLevel();
            else if(pParent->GetType() == CONTENT_TYPE_REGION)
                nLevel = nLevel + ((SwRegionContent*)pCnt)->GetRegionLevel();
        }
    }
    sal_uInt16 nBasis = bIsRoot ? 0 : 5;
    return nLevel * 10 + nBasis + pTab->GetPos();  //determined empirically
}

// Content will be integrated into the Box only on demand.

void  SwContentTree::RequestingChildren( SvTreeListEntry* pParent )
{
    // Is this a content type?
    if(lcl_IsContentType(pParent))
    {
        if(!pParent->HasChildren())
        {
            OSL_ENSURE(pParent->GetUserData(), "no UserData?");
            SwContentType* pCntType = (SwContentType*)pParent->GetUserData();

            sal_uInt16 nCount = pCntType->GetMemberCount();
             // Add for outline plus/minus
             if(pCntType->GetType() == CONTENT_TYPE_OUTLINE)
             {
                 SvTreeListEntry* pChild = 0;
                 for(sal_uInt16 i = 0; i < nCount; i++)
                 {
                     const SwContent* pCnt = pCntType->GetMember(i);
                     if(pCnt)
                     {
                         sal_uInt16 nLevel = ((SwOutlineContent*)pCnt)->GetOutlineLevel();
                         String sEntry = pCnt->GetName();
                         if(!sEntry.Len())
                             sEntry = sSpace;
                         if(!pChild || (nLevel == 0))
                             pChild = InsertEntry(sEntry, pParent,
                                         sal_False, LIST_APPEND,(void*)pCnt);
                         else
                         {
                             //back search parent.
                             if(((SwOutlineContent*)pCntType->GetMember(i-1))->GetOutlineLevel() < nLevel)
                                 pChild = InsertEntry(sEntry, pChild,
                                         sal_False, LIST_APPEND, (void*)pCnt);
                             else
                             {
                                 pChild = Prev(pChild);
                                 while(pChild &&
                                         lcl_IsContent(pChild) &&
                                         !(((SwOutlineContent*)pChild->GetUserData())->GetOutlineLevel() < nLevel)
                                      )
                                 {
                                     pChild = Prev(pChild);
                                 }
                                 if(pChild)
                                     pChild = InsertEntry(sEntry, pChild,
                                                 sal_False, LIST_APPEND, (void*)pCnt);
                             }
                        }
                     }
                 }
             }
             else
             {
                 for(sal_uInt16 i = 0; i < nCount; i++)
                 {
                     const SwContent* pCnt = pCntType->GetMember(i);
                     if(pCnt)
                     {
                         String sEntry = pCnt->GetName();
                         if(!sEntry.Len())
                             sEntry = sSpace;
                         InsertEntry(sEntry, pParent,
                             sal_False, LIST_APPEND, (void*)pCnt);
                     }
                 }
            }
        }
    }
}

// Expand - Remember the state for content types.

sal_Bool  SwContentTree::Expand( SvTreeListEntry* pParent )
{
    if(!bIsRoot || (((SwContentType*)pParent->GetUserData())->GetType() == CONTENT_TYPE_OUTLINE) ||
            (nRootType == CONTENT_TYPE_OUTLINE))
    {
        if(lcl_IsContentType(pParent))
        {
            SwContentType* pCntType = (SwContentType*)pParent->GetUserData();
            sal_uInt16 nOr = 1 << pCntType->GetType(); //linear -> Bitposition
            if(bIsActive || bIsConstant)
            {
                nActiveBlock |= nOr;
                pConfig->SetActiveBlock(nActiveBlock);
            }
            else
                nHiddenBlock |= nOr;
            if((pCntType->GetType() == CONTENT_TYPE_OUTLINE))
            {
                std::map< void*, bool > mCurrOutLineNodeMap;

                SwWrtShell* pShell = GetWrtShell();
                sal_Bool bBool = SvTreeListBox::Expand(pParent);
                SvTreeListEntry* pChild = Next(pParent);
                while(pChild && lcl_IsContent(pChild) && pParent->HasChildren())
                {
                    if(pChild->HasChildren())
                    {
                        sal_Int32 nPos = ((SwContent*)pChild->GetUserData())->GetYPos();
                        void* key = (void*)pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos );
                        mCurrOutLineNodeMap.insert(std::map<void*, bool>::value_type( key, false ) );
                        std::map<void*, bool>::iterator iter = mOutLineNodeMap.find( key );
                        if( iter != mOutLineNodeMap.end() && mOutLineNodeMap[key])
                        {
                            mCurrOutLineNodeMap[key] = true;
                            SvTreeListBox::Expand(pChild);
                        }
                    }
                    pChild = Next(pChild);
                }
                mOutLineNodeMap = mCurrOutLineNodeMap;
                return bBool;
            }

        }
        else if( lcl_IsContent(pParent) )
        {
            SwWrtShell* pShell = GetWrtShell();
            sal_Int32 nPos = ((SwContent*)pParent->GetUserData())->GetYPos();
            void* key = (void*)pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos );
            mOutLineNodeMap[key] = true;
        }
    }
    return SvTreeListBox::Expand(pParent);
}

// Collapse - Remember the state for content types.

sal_Bool  SwContentTree::Collapse( SvTreeListEntry* pParent )
{
    sal_Bool bRet;
    if(!bIsRoot || (((SwContentType*)pParent->GetUserData())->GetType() == CONTENT_TYPE_OUTLINE) ||
            (nRootType == CONTENT_TYPE_OUTLINE))
    {
        if(lcl_IsContentType(pParent))
        {
            if(bIsRoot)
                return bRet = sal_False;
            SwContentType* pCntType = (SwContentType*)pParent->GetUserData();
            sal_uInt16 nAnd = 1 << pCntType->GetType();
            nAnd = ~nAnd;
            if(bIsActive || bIsConstant)
            {
                nActiveBlock &= nAnd;
                pConfig->SetActiveBlock(nActiveBlock);
            }
            else
                nHiddenBlock &= nAnd;
        }
        else if( lcl_IsContent(pParent) )
        {
            SwWrtShell* pShell = GetWrtShell();
            sal_Int32 nPos = ((SwContent*)pParent->GetUserData())->GetYPos();
            void* key = (void*)pShell->getIDocumentOutlineNodesAccess()->getOutlineNode( nPos );
            mOutLineNodeMap[key] = false;
        }
            bRet = SvTreeListBox::Collapse(pParent);
    }
    else
        bRet = SvTreeListBox::Collapse(pParent);
    return bRet;
}

// Also on double click will be initially opened only.

IMPL_LINK_NOARG(SwContentTree, ContentDoubleClickHdl)
{
    SvTreeListEntry* pEntry = GetCurEntry();
    // Is it a content type?
    OSL_ENSURE(pEntry, "no current entry!");
    if(pEntry)
    {
        if(lcl_IsContentType(pEntry) && !pEntry->HasChildren())
            RequestingChildren(pEntry);
        else if(!lcl_IsContentType(pEntry) && (bIsActive || bIsConstant))
        {
            if(bIsConstant)
            {
                pActiveShell->GetView().GetViewFrame()->GetWindow().ToTop();
            }
            //Jump to content type:
            SwContent* pCnt = (SwContent*)pEntry->GetUserData();
            OSL_ENSURE( pCnt, "no UserData");
            GotoContent(pCnt);
            if(pCnt->GetParent()->GetType() == CONTENT_TYPE_FRAME)
                pActiveShell->EnterStdMode();
        }
    }
    return 0;
}

// Show the file

void SwContentTree::Display( bool bActive )
{
    if(!bIsImageListInitialized)
    {
        aEntryImages = ImageList(SW_RES(IMG_NAVI_ENTRYBMP));
        bIsImageListInitialized = true;
    }
    // First read the selected entry to select it later again if necessary
    // -> the user data here are no longer valid!
    SvTreeListEntry* pOldSelEntry = FirstSelected();
    String sEntryName;  // Name of the entry
    sal_uInt16 nEntryRelPos = 0; // relative position to their parent
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
            nEntryRelPos = (sal_uInt16)(GetModel()->GetAbsPos(pOldSelEntry) - GetModel()->GetAbsPos(pParantEntry));
        }
    }
    Clear();
    SetUpdateMode( sal_False );
    if(bActive && !bIsConstant && !bIsActive)
        bIsActive = bActive;
    bIsHidden = !bActive;
    SwWrtShell* pShell = GetWrtShell();
    sal_Bool bReadOnly = pShell ? pShell->GetView().GetDocShell()->IsReadOnly() : sal_True;
    if(bReadOnly != bIsLastReadOnly)
    {
        bIsLastReadOnly = bReadOnly;
        bool bDisable =  pShell == 0 || bReadOnly;
        SwNavigationPI* pNavi = GetParentWindow();
        pNavi->aContentToolBox.EnableItem(FN_ITEM_UP , !bDisable);
        pNavi->aContentToolBox.EnableItem(FN_ITEM_DOWN, !bDisable);
        pNavi->aContentToolBox.EnableItem(FN_ITEM_LEFT, !bDisable);
        pNavi->aContentToolBox.EnableItem(FN_ITEM_RIGHT, !bDisable);
        pNavi->aContentToolBox.EnableItem(FN_SELECT_SET_AUTO_BOOKMARK, !bDisable);
    }
    if(pShell)
    {
        SvTreeListEntry* pSelEntry = 0;
        if(nRootType == USHRT_MAX)
        {
            for(sal_uInt16 nCntType = CONTENT_TYPE_OUTLINE;
                        nCntType <= CONTENT_TYPE_DRAWOBJECT; nCntType++ )
            {
                SwContentType** ppContentT = bActive ?
                                &aActiveContentArr[nCntType] :
                                    &aHiddenContentArr[nCntType];
                if(!*ppContentT)
                    (*ppContentT) = new SwContentType(pShell, nCntType, nOutlineLevel );

                String sEntry = (*ppContentT)->GetName();
                SvTreeListEntry* pEntry;
                const Image& rImage = aEntryImages.GetImage(SID_SW_START + nCntType);
                sal_Bool bChOnDemand = 0 != (*ppContentT)->GetMemberCount();
                pEntry = InsertEntry(sEntry, rImage, rImage,
                                0, bChOnDemand, LIST_APPEND, (*ppContentT));
                if(nCntType == nLastSelType)
                    pSelEntry = pEntry;
                sal_Int32 nExpandOptions = bIsActive || bIsConstant ?
                                            nActiveBlock :
                                                nHiddenBlock;
                if(nExpandOptions & (1 << nCntType))
                {
                    Expand(pEntry);
                    if(nEntryRelPos && nCntType == nLastSelType)
                    {
                        // Now maybe select a additional child
                        SvTreeListEntry* pChild = pEntry;
                        SvTreeListEntry* pTemp = 0;
                        sal_uInt16 nPos = 1;
                        while(0 != (pChild = Next(pChild)))
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
                                &aActiveContentArr[nRootType] :
                                    &aHiddenContentArr[nRootType];
            if(!(*ppRootContentT))
                (*ppRootContentT) = new SwContentType(pShell, nRootType, nOutlineLevel );
            const Image& rImage = aEntryImages.GetImage(20000 + nRootType);
            SvTreeListEntry* pParent = InsertEntry(
                    (*ppRootContentT)->GetName(), rImage, rImage,
                        0, sal_False, LIST_APPEND, *ppRootContentT);

            if(nRootType != CONTENT_TYPE_OUTLINE)
            {
                for(sal_uInt16 i = 0; i < (*ppRootContentT)->GetMemberCount(); i++ )
                {
                    const SwContent* pCnt = (*ppRootContentT)->GetMember(i);
                    if(pCnt)
                    {
                        String sEntry = pCnt->GetName();
                        if(!sEntry.Len())
                            sEntry = sSpace;
                        InsertEntry( sEntry, pParent,
                            sal_False, LIST_APPEND, (void*)pCnt);
                    }
                }
             }
             else
                 RequestingChildren(pParent);
            Expand(pParent);
            if( nRootType == CONTENT_TYPE_OUTLINE && bIsActive )
            {
                // find out where the cursor is
                const sal_uInt16 nActPos = pShell->GetOutlinePos(MAXLEVEL);
                SvTreeListEntry* pEntry = First();

                while( 0 != (pEntry = Next(pEntry)) )
                {
                    if(((SwOutlineContent*)pEntry->GetUserData())->GetPos() == nActPos)
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
                SvTreeListEntry* pTemp = 0;
                sal_uInt16 nPos = 1;
                while(0 != (pChild = Next(pChild)))
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
    SetUpdateMode( sal_True );
    ScrollBar* pVScroll = GetVScroll();
    if(GetEntryCount() == nOldEntryCount &&
        nOldScrollPos && pVScroll && pVScroll->IsVisible()
        && pVScroll->GetThumbPos() != nOldScrollPos)
    {
        sal_Int32 nDelta = pVScroll->GetThumbPos() - nOldScrollPos;
        ScrollOutputArea( (short)nDelta );
    }

}

// In the Clear the content types have to be deleted, also.

void SwContentTree::Clear()
{
    SetUpdateMode(sal_False);
    SvTreeListBox::Clear();
    SetUpdateMode(sal_True);
}

bool SwContentTree::FillTransferData( TransferDataContainer& rTransfer,
                                            sal_Int8& rDragMode )
{
    SwWrtShell* pWrtShell = GetWrtShell();
    OSL_ENSURE(pWrtShell, "no Shell!");
    SvTreeListEntry* pEntry = GetCurEntry();
    if(!pEntry || lcl_IsContentType(pEntry) || !pWrtShell)
        return false;
    String sEntry;
    SwContent* pCnt = ((SwContent*)pEntry->GetUserData());

    sal_uInt16 nActType = pCnt->GetParent()->GetType();
    String sUrl;
    bool bOutline = false;
    String sOutlineText;
    switch( nActType )
    {
        case CONTENT_TYPE_OUTLINE:
        {
            sal_uInt16 nPos = ((SwOutlineContent*)pCnt)->GetPos();
            OSL_ENSURE(nPos < pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNodesCount(),
                       "outlinecnt changed");

            // make sure outline may actually be copied
            if( pWrtShell->IsOutlineCopyable( nPos ) )
            {
                const SwNumRule* pOutlRule = pWrtShell->GetOutlineNumRule();
                const SwTxtNode* pTxtNd =
                        pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineNode(nPos);
                if( pTxtNd && pOutlRule && pTxtNd->IsNumbered())
                {
                    SwNumberTree::tNumberVector aNumVector =
                        pTxtNd->GetNumberVector();
                    for( sal_Int8 nLevel = 0;
                         nLevel <= pTxtNd->GetActualListLevel();
                         nLevel++ )
                    {
                        sal_uInt16 nVal = (sal_uInt16)aNumVector[nLevel];
                        nVal ++;
                        nVal = nVal - pOutlRule->Get(nLevel).GetStart();
                        sEntry += OUString::number( nVal );
                        sEntry += '.';
                    }
                }
                sEntry += pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(nPos, false);
                sOutlineText = pWrtShell->getIDocumentOutlineNodesAccess()->getOutlineText(nPos, true);
                bIsOutlineMoveable = ((SwOutlineContent*)pCnt)->IsMoveable();
                bOutline = true;
            }
        }
        break;
        case CONTENT_TYPE_POSTIT:
        case CONTENT_TYPE_INDEX:
        case CONTENT_TYPE_REFERENCE :
            // cannot inserted as URL or as  koennen weder als URL noch als region
        break;
        case CONTENT_TYPE_URLFIELD:
            sUrl = ((SwURLFieldContent*)pCnt)->GetURL();
        // no break;
        case CONTENT_TYPE_OLE:
        case CONTENT_TYPE_GRAPHIC:
            if(GetParentWindow()->GetRegionDropMode() != REGION_MODE_NONE)
                break;
            else
                rDragMode &= ~( DND_ACTION_MOVE | DND_ACTION_LINK );
        default:
            sEntry = GetEntryText(pEntry);
    }

    bool bRet = false;
    if(sEntry.Len())
    {
        const SwDocShell* pDocShell = pWrtShell->GetView().GetDocShell();
        if(!sUrl.Len())
        {
            if(pDocShell->HasName())
            {
                SfxMedium* pMedium = pDocShell->GetMedium();
                sUrl = pMedium->GetURLObject().GetURLNoMark();
                // only if a primarily link shall be integrated.
                bRet = true;
            }
            else if(    nActType == CONTENT_TYPE_REGION ||
                        nActType == CONTENT_TYPE_BOOKMARK )
            {
                // For field and bookmarks a link is also allowed
                // without a filename into its own document.
                bRet = true;
            }
            else if(bIsConstant &&
                    ( !::GetActiveView() ||
                        pActiveShell != ::GetActiveView()->GetWrtShellPtr()))
            {
                // Urls of inactive views cannot dragged without
                // file names, also.
                bRet = false;
            }
            else
            {
                bRet = GetParentWindow()->GetRegionDropMode() == REGION_MODE_NONE;
                rDragMode = DND_ACTION_MOVE;
            }

            const String& rToken = pCnt->GetParent()->GetTypeToken();
            sUrl += '#';
            sUrl += sEntry;
            if(rToken.Len())
            {
                sUrl += cMarkSeparator;
                sUrl += rToken;
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
    if(!bIsRoot)
    {
        SvTreeListEntry* pEntry = GetCurEntry();
        const SwContentType* pCntType;
        if(pEntry)
        {
            if(lcl_IsContentType(pEntry))
                pCntType = (SwContentType*)pEntry->GetUserData();
            else
                pCntType = ((SwContent*)pEntry->GetUserData())->GetParent();
            nRootType = pCntType->GetType();
            bIsRoot = true;
            Display(bIsActive || bIsConstant);
        }
    }
    else
    {
        nRootType = USHRT_MAX;
        bIsRoot = false;
        FindActiveTypeAndRemoveUserData();
        Display(bIsActive || bIsConstant);
    }
    pConfig->SetRootType( nRootType );
    GetParentWindow()->aContentToolBox.CheckItem(FN_SHOW_ROOT, bIsRoot ? sal_True : sal_False);
    return bIsRoot;
}

// Check if the displayed content is valid.

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
    sal_Bool bInvalidate = sal_False;

    if(!bIsActive && ! bIsConstant)
    {
        for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
        {
            if(aActiveContentArr[i])
                aActiveContentArr[i]->Invalidate();
        }
    }
    else if(bIsRoot)
    {
        bool bOutline = false;
        SvTreeListEntry* pEntry = First();
        if(!pEntry)
            bRepaint = true;
        else
        {
            sal_uInt16 nType = ((SwContentType*)pEntry->GetUserData())->GetType();
            bOutline = nRootType == CONTENT_TYPE_OUTLINE;
            SwContentType* pArrType = aActiveContentArr[nType];
            if(!pArrType)
                bRepaint = true;
            else
            {
                sal_uInt16 nSelLevel = USHRT_MAX;

                SvTreeListEntry* pFirstSel;
                if(bOutline &&
                        0 != ( pFirstSel = FirstSelected()) &&
                            lcl_IsContent(pFirstSel))
                {
                    nSelLevel = ((SwOutlineContent*)pFirstSel->GetUserData())->GetOutlineLevel();
                    SwWrtShell* pSh = GetWrtShell();
                    sal_uInt16 nOutlinePos = pSh->GetOutlinePos(MAXLEVEL);
                    if (nOutlinePos != USHRT_MAX &&
                        pSh->getIDocumentOutlineNodesAccess()->getOutlineLevel(nOutlinePos) != nSelLevel)
                        bRepaint = true;
                }

                pArrType->Init(&bInvalidate);
                pArrType->FillMemberList();
                pEntry->SetUserData((void*)pArrType);
                if(!bRepaint)
                {
                    if(GetChildCount(pEntry) != pArrType->GetMemberCount())
                            bRepaint = true;
                    else
                    {
                        sal_uInt16 nChildCount = (sal_uInt16)GetChildCount(pEntry);
                        for(sal_uInt16 j = 0; j < nChildCount; j++)
                        {
                            pEntry = Next(pEntry);
                            const SwContent* pCnt = pArrType->GetMember(j);
                            pEntry->SetUserData((void*)pCnt);
                            String sEntryText = GetEntryText(pEntry);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == sSpace && !pCnt->GetName().Len()))
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

            while( 0 != (pFirstEntry = Next(pFirstEntry)) )
            {
                if(((SwOutlineContent*)pFirstEntry->GetUserData())->GetPos() == nActPos)
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
            SwContentType* pTreeType = (SwContentType*)pEntry->GetUserData();
            sal_uInt16 nType = pTreeType->GetType();
            sal_uInt16 nTreeCount = pTreeType->GetMemberCount();
            SwContentType* pArrType = aActiveContentArr[nType];
            if(!pArrType)
                bRepaint = true;
            else
            {
                pArrType->Init(&bInvalidate);
                pEntry->SetUserData((void*)pArrType);
                if(IsExpanded(pEntry))
                {
                    sal_Bool bLevelOrVisibiblityChanged = sal_False;
                    // bLevelOrVisibiblityChanged is set if outlines have changed their level
                    // or if the visibility of objects (frames, sections, tables) has changed
                    // i.e. in header/footer
                    pArrType->FillMemberList(&bLevelOrVisibiblityChanged);
                    sal_uInt16 nChildCount = (sal_uInt16)GetChildCount(pEntry);
                    if((nType == CONTENT_TYPE_OUTLINE) && bLevelOrVisibiblityChanged)
                        bRepaint = true;
                    if(bLevelOrVisibiblityChanged)
                        bInvalidate = sal_True;

                    if(nChildCount != pArrType->GetMemberCount())
                        bRepaint = true;
                    else
                    {
                        for(sal_uInt16 j = 0; j < nChildCount; j++)
                        {
                            pEntry = Next(pEntry);
                            bNext = false;
                            const SwContent* pCnt = pArrType->GetMember(j);
                            pEntry->SetUserData((void*)pCnt);
                            String sEntryText = GetEntryText(pEntry);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == sSpace && !pCnt->GetName().Len()))
                                bRepaint = true;
                        }
                    }

                }
                else if(pEntry->HasChildren())
                {
                    // was the entry once opened, then must also the
                    // invisible records be examined.
                    // At least the user data must be updated.
                    sal_Bool bLevelOrVisibiblityChanged = sal_False;
                    // bLevelOrVisibiblityChanged is set if outlines have changed their level
                    // or if the visibility of objects (frames, sections, tables) has changed
                    // i.e. in header/footer
                    pArrType->FillMemberList(&bLevelOrVisibiblityChanged);
                    sal_Bool bRemoveChildren = sal_False;
                    sal_uInt16 nChildCount = (sal_uInt16)GetChildCount(pEntry);
                    if( nChildCount != pArrType->GetMemberCount() )
                    {
                        bRemoveChildren = sal_True;
                    }
                    else
                    {
                        SvTreeListEntry* pChild = FirstChild(pEntry);
                        for(sal_uInt16 j = 0; j < nChildCount; j++)
                        {
                            const SwContent* pCnt = pArrType->GetMember(j);
                            pChild->SetUserData((void*)pCnt);
                            String sEntryText = GetEntryText(pChild);
                            if( sEntryText != pCnt->GetName() &&
                                !(sEntryText == sSpace && !pCnt->GetName().Len()))
                                bRemoveChildren = sal_True;
                            pChild = Next(pChild);
                        }
                    }
                    if(bRemoveChildren)
                    {
                        SvTreeListEntry* pChild = FirstChild(pEntry);
                        SvTreeListEntry* pRemove = pChild;
                        for(sal_uInt16 j = 0; j < nChildCount; j++)
                        {
                            pChild = Next(pRemove);
                            GetModel()->Remove(pRemove);
                            pRemove = pChild;
                        }
                    }
                    if(!nChildCount)
                    {
                        pEntry->EnableChildrenOnDemand(sal_False);
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

// Before any data will be deleted, the last active entry has to be found.
// After this the UserData will be deleted

void SwContentTree::FindActiveTypeAndRemoveUserData()
{
    SvTreeListEntry* pEntry = FirstSelected();
    if(pEntry)
    {
        // If clear is called by TimerUpdate:
        // Only for root can the validity of the UserData be guaranteed.
        SvTreeListEntry* pParent;
        while(0 != (pParent = GetParent(pEntry)))
            pEntry = pParent;
        if(pEntry->GetUserData() && lcl_IsContentType(pEntry))
            nLastSelType = ((SwContentType*)pEntry->GetUserData())->GetType();
    }
    pEntry = First();
    while(pEntry)
    {
        pEntry->SetUserData(0);
        pEntry = Next(pEntry);
    }
}

// After a file is dropped on the Navigator,
// the new shell will be set.

void SwContentTree::SetHiddenShell(SwWrtShell* pSh)
{
    pHiddenShell = pSh;
    bIsHidden = true;
    bIsActive = bIsConstant = false;
    FindActiveTypeAndRemoveUserData();
    for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
    {
        DELETEZ(aHiddenContentArr[i]);
    }
    Display(bIsActive);

    GetParentWindow()->UpdateListBox();
}

//  Document change - set new Shell

void SwContentTree::SetActiveShell(SwWrtShell* pSh)
{
    if(bIsInternalDrag)
        bDocChgdInDragging = true;
    bool bClear = pActiveShell != pSh;
    if(bIsActive && bClear)
    {
        pActiveShell = pSh;
        FindActiveTypeAndRemoveUserData();
        Clear();
    }
    else if(bIsConstant)
    {
        if(!lcl_FindShell(pActiveShell))
        {
            pActiveShell = pSh;
            bIsActive = true;
            bIsConstant = false;
            bClear = true;
        }
    }
    // Only if it is the active view, the array will be deleted and
    // the screen filled new.
    if(bIsActive && bClear)
    {
        FindActiveTypeAndRemoveUserData();
        for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
        {
            DELETEZ(aActiveContentArr[i]);
        }
        Display(true);
    }
}

// Set an open view as active.

void SwContentTree::SetConstantShell(SwWrtShell* pSh)
{
    pActiveShell = pSh;
    bIsActive       = false;
    bIsConstant     = true;
    FindActiveTypeAndRemoveUserData();
    for(sal_uInt16 i=0; i < CONTENT_TYPE_MAX; i++)
    {
        DELETEZ(aActiveContentArr[i]);
    }
    Display(true);
}

// Execute commands of the Navigator

void SwContentTree::ExecCommand(sal_uInt16 nCmd, sal_Bool bModifier)
{
    bool nMove = false;
    switch( nCmd )
    {
        case FN_ITEM_DOWN:
        case FN_ITEM_UP:   nMove = true;
        case FN_ITEM_LEFT:
        case FN_ITEM_RIGHT:
        if( !GetWrtShell()->GetView().GetDocShell()->IsReadOnly() &&
                (bIsActive ||
                    (bIsConstant && pActiveShell == GetParentWindow()->GetCreateView()->GetWrtShellPtr())))
        {
            SwWrtShell* pShell = GetWrtShell();
            sal_Int8 nActOutlineLevel = nOutlineLevel;
            sal_uInt16 nActPos = pShell->GetOutlinePos(nActOutlineLevel);
            SvTreeListEntry* pFirstEntry = FirstSelected();
            if (pFirstEntry && lcl_IsContent(pFirstEntry))
            {
                if ( (bIsRoot && nRootType == CONTENT_TYPE_OUTLINE) ||
                    ((SwContent*)pFirstEntry->GetUserData())->GetParent()->GetType()
                                                ==  CONTENT_TYPE_OUTLINE)
                {
                    nActPos = ((SwOutlineContent*)pFirstEntry->GetUserData())->GetPos();
                }
            }
            if ( nActPos < USHRT_MAX &&
                    ( !nMove || pShell->IsOutlineMovable( nActPos )) )
            {
                pShell->StartAllAction();
                pShell->GotoOutline( nActPos); // If text selection != box selection
                pShell->Push();
                pShell->MakeOutlineSel( nActPos, nActPos,
                                    bModifier);
                if( nMove )
                {
                    short nDir = nCmd == FN_ITEM_UP ? -1 : 1;
                    if( !bModifier && ( (nDir == -1 && nActPos > 0) ||
                        (nDir == 1 && nActPos < GetEntryCount() - 2) ) )
                    {
                        pShell->MoveOutlinePara( nDir );
                        // Set cursor back to the current position
                        pShell->GotoOutline( nActPos + nDir);
                    }
                    else if(bModifier)
                    {
                        sal_uInt16 nActEndPos = nActPos;
                        SvTreeListEntry* pEntry = pFirstEntry;
                        sal_uInt16 nActLevel = ((SwOutlineContent*)
                                pFirstEntry->GetUserData())->GetOutlineLevel();
                        pEntry = Next(pEntry);
                        while( pEntry && CONTENT_TYPE_OUTLINE ==
                            ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId() )
                        {
                            if(nActLevel >= ((SwOutlineContent*)
                                pEntry->GetUserData())->GetOutlineLevel())
                                break;
                            pEntry = Next(pEntry);
                            nActEndPos++;
                        }
                        sal_uInt16 nDest;
                        if(nDir == 1)
                        {
                            // If the last entry is to be moved it is over!
                            if(pEntry && CONTENT_TYPE_OUTLINE ==
                                ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId())
                            {
                                // pEntry now points to the following entry of the last
                                // selected entry.
                                nDest = nActEndPos;
                                nDest++;
                                // here needs to found the next record after next.
                                // The selection must be inserted in front of.
                                while(pEntry )
                                {
                                    pEntry = Next(pEntry);
                                    // nDest++ may only executed if pEntry != 0
                                    if(pEntry && nDest++ &&
                                    ( nActLevel >= ((SwOutlineContent*)pEntry->GetUserData())->GetOutlineLevel()||
                                     CONTENT_TYPE_OUTLINE != ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId()))
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
                            nDest = nActPos;
                            pEntry = pFirstEntry;
                            while(pEntry && nDest )
                            {
                                nDest--;
                                pEntry = Prev(pEntry);
                                if(pEntry &&
                                    (nActLevel >= ((SwOutlineContent*)pEntry->GetUserData())->GetOutlineLevel()||
                                    CONTENT_TYPE_OUTLINE !=
                                ((SwTypeNumber*)pEntry->GetUserData())->GetTypeId()))
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
                pShell->Pop(sal_False); // Cursor is now back at the current superscription.
                pShell->EndAllAction();
                if(aActiveContentArr[CONTENT_TYPE_OUTLINE])
                    aActiveContentArr[CONTENT_TYPE_OUTLINE]->Invalidate();
                Display(true);
                if(!bIsRoot)
                {
                    const sal_uInt16 nCurrPos = pShell->GetOutlinePos(MAXLEVEL);
                    SvTreeListEntry* pFirst = First();

                    while( 0 != (pFirst = Next(pFirst)) && lcl_IsContent(pFirst))
                    {
                        if(((SwOutlineContent*)pFirst->GetUserData())->GetPos() == nCurrPos)
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

void    SwContentTree::ShowTree()
{
    aUpdTimer.Start();
    SvTreeListBox::Show();
}

// folded together will not be geidled

void    SwContentTree::HideTree()
{
    aUpdTimer.Stop();
    SvTreeListBox::Hide();
}

// No idle with focus or while dragging.

IMPL_LINK_NOARG(SwContentTree, TimerUpdate)
{
    // No update while drag and drop.
    // Query view because the Navigator is cleared too late.
    SwView* pView = GetParentWindow()->GetCreateView();
    if( (!HasFocus() || bViewHasChanged) &&
         !bIsInDrag && !bIsInternalDrag && pView &&
         pView->GetWrtShellPtr() && !pView->GetWrtShellPtr()->ActionPend() )
    {
        bViewHasChanged = false;
        bIsIdleClear = false;
        SwWrtShell* pActShell = pView->GetWrtShellPtr();
        if( bIsConstant && !lcl_FindShell( pActiveShell ) )
        {
            SetActiveShell(pActShell);
            GetParentWindow()->UpdateListBox();
        }

        if(bIsActive && pActShell != GetWrtShell())
            SetActiveShell(pActShell);
        else if( (bIsActive || (bIsConstant && pActShell == GetWrtShell())) &&
                    HasContentChanged())
        {
            FindActiveTypeAndRemoveUserData();
            Display(true);
        }
    }
    else if(!pView && bIsActive && !bIsIdleClear)
    {
        if(pActiveShell)
            SetActiveShell(0);
        Clear();
        bIsIdleClear = true;
    }
    return 0;
}

DragDropMode SwContentTree::NotifyStartDrag(
                TransferDataContainer& rContainer,
                SvTreeListEntry* pEntry )
{
    DragDropMode eMode = (DragDropMode)0;
    if( bIsActive && nRootType == CONTENT_TYPE_OUTLINE &&
        GetModel()->GetAbsPos( pEntry ) > 0
        && !GetWrtShell()->GetView().GetDocShell()->IsReadOnly())
        eMode = GetDragDropMode();
    else if(!bIsActive && GetWrtShell()->GetView().GetDocShell()->HasName())
        eMode = SV_DRAGDROP_APP_COPY;

    sal_Int8 nDragMode;
    FillTransferData( rContainer, nDragMode );
    bDocChgdInDragging = false;
    bIsInternalDrag = true;
    return eMode;
}
// After the drag the current paragraph will be moved  w i t h  the children.

sal_Bool  SwContentTree::NotifyMoving( SvTreeListEntry*  pTarget,
        SvTreeListEntry*  pEntry, SvTreeListEntry*& , sal_uLong& )
{
    if(!bDocChgdInDragging)
    {
        sal_uInt16 nTargetPos = 0;
        sal_uInt16 nSourcePos = (( SwOutlineContent* )pEntry->GetUserData())->GetPos();
        if(!lcl_IsContent(pTarget))
            nTargetPos = USHRT_MAX;
        else
            nTargetPos = (( SwOutlineContent* )pTarget->GetUserData())->GetPos();
        if( MAXLEVEL > nOutlineLevel && // Not all layers are displayed.
                        nTargetPos != USHRT_MAX)
        {
            SvTreeListEntry* pNext = Next(pTarget);
            if(pNext)
                nTargetPos = (( SwOutlineContent* )pNext->GetUserData())->GetPos() -1;
            else
                nTargetPos = static_cast<sal_uInt16>(GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount())- 1;

        }

        OSL_ENSURE( pEntry &&
            lcl_IsContent(pEntry),"Source == 0 or Source has no Content" );
        GetParentWindow()->MoveOutline( nSourcePos,
                                    nTargetPos,
                                    true);

        aActiveContentArr[CONTENT_TYPE_OUTLINE]->Invalidate();
        Display(true);
    }
    //TreeListBox will be reloaded from the document
    return sal_False;
}

// After the drag the current paragraph will be moved  w i t h o u t  the children.

sal_Bool  SwContentTree::NotifyCopying( SvTreeListEntry*  pTarget,
        SvTreeListEntry*  pEntry, SvTreeListEntry*& , sal_uLong& )
{
    if(!bDocChgdInDragging)
    {
        sal_uInt16 nTargetPos = 0;
        sal_uInt16 nSourcePos = (( SwOutlineContent* )pEntry->GetUserData())->GetPos();
        if(!lcl_IsContent(pTarget))
            nTargetPos = USHRT_MAX;
        else
            nTargetPos = (( SwOutlineContent* )pTarget->GetUserData())->GetPos();

        if( MAXLEVEL > nOutlineLevel && // Not all layers are displayed.
                        nTargetPos != USHRT_MAX)
        {
            SvTreeListEntry* pNext = Next(pTarget);
            if(pNext)
                nTargetPos = (( SwOutlineContent* )pNext->GetUserData())->GetPos() - 1;
            else
                nTargetPos = static_cast<sal_uInt16>(GetWrtShell()->getIDocumentOutlineNodesAccess()->getOutlineNodesCount()) - 1;

        }

        OSL_ENSURE( pEntry &&
            lcl_IsContent(pEntry),"Source == 0 or Source has no Content" );
        GetParentWindow()->MoveOutline( nSourcePos, nTargetPos, false);

        //TreeListBox will be reloaded from the document
        aActiveContentArr[CONTENT_TYPE_OUTLINE]->Invalidate();
        Display(true);
    }
    return sal_False;
}

// No drop before the first entry - it's a SwContentType

sal_Bool  SwContentTree::NotifyAcceptDrop( SvTreeListEntry* pEntry)
{
    return pEntry != 0;
}

// If a Ctrl + DoubleClick are executed in an open area,
// then the base function of the control is to be called.

void  SwContentTree::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aPos( rMEvt.GetPosPixel());
    SvTreeListEntry* pEntry = GetEntry( aPos, sal_True );
    if( !pEntry && rMEvt.IsLeft() && rMEvt.IsMod1() && (rMEvt.GetClicks() % 2) == 0)
        Control::MouseButtonDown( rMEvt );
    else
        SvTreeListBox::MouseButtonDown( rMEvt );
}

// Update immediately

void  SwContentTree::GetFocus()
{
    SwView* pActView = GetParentWindow()->GetCreateView();
    if(pActView)
    {
        SwWrtShell* pActShell = pActView->GetWrtShellPtr();
        if(bIsConstant && !lcl_FindShell(pActiveShell))
        {
            SetActiveShell(pActShell);
        }

        if(bIsActive && pActShell != GetWrtShell())
            SetActiveShell(pActShell);
        else if( (bIsActive || (bIsConstant && pActShell == GetWrtShell())) &&
                    HasContentChanged())
        {
            Display(true);
        }
    }
    else if(bIsActive)
        Clear();
    SvTreeListBox::GetFocus();
}

void  SwContentTree::KeyInput(const KeyEvent& rEvent)
{
    const KeyCode aCode = rEvent.GetKeyCode();
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
                        IsExpanded(pEntry) ?
                            Collapse(pEntry) :
                                Expand(pEntry);
                    }
                    else
                        ContentDoubleClickHdl(0);
                break;
            }
        }
    }
    else if(aCode.GetCode() == KEY_DELETE && 0 == aCode.GetModifier())
    {
        SvTreeListEntry* pEntry = FirstSelected();
        if(pEntry &&
            lcl_IsContent(pEntry) &&
                ((SwContent*)pEntry->GetUserData())->GetParent()->IsDeletable() &&
                    !pActiveShell->GetView().GetDocShell()->IsReadOnly())
        {
            EditEntry(pEntry, EDIT_MODE_DELETE);
            bViewHasChanged = true;
            GetParentWindow()->UpdateListBox();
            TimerUpdate(&aUpdTimer);
            GrabFocus();
        }
    }
    else
        SvTreeListBox::KeyInput(rEvent);

}

void  SwContentTree::RequestHelp( const HelpEvent& rHEvt )
{
    bool bCallBase = true;
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            sal_uInt16 nType;
            bool bBalloon = false;
            bool bContent = false;
            void* pUserData = pEntry->GetUserData();
            if(lcl_IsContentType(pEntry))
                nType = ((SwContentType*)pUserData)->GetType();
            else
            {
                nType = ((SwContent*)pUserData)->GetParent()->GetType();
                bContent = true;
            }
            String sEntry;
            bool bRet = false;
            if(bContent)
            {
                switch( nType )
                {
                    case CONTENT_TYPE_URLFIELD:
                        sEntry = ((SwURLFieldContent*)pUserData)->GetURL();
                        bRet = true;
                    break;

                    case CONTENT_TYPE_POSTIT:
                        sEntry = ((SwPostItContent*)pUserData)->GetName();
                        bRet = true;
                        if(Help::IsBalloonHelpEnabled())
                            bBalloon = true;
                    break;
                    case CONTENT_TYPE_OUTLINE:
                        sEntry = ((SwOutlineContent*)pUserData)->GetName();
                        bRet = true;
                    break;
                    case CONTENT_TYPE_GRAPHIC:
                        sEntry = ((SwGraphicContent*)pUserData)->GetLink();
#if OSL_DEBUG_LEVEL > 1
                        sEntry += ' ';
                        sEntry += OUString::number(
                                    ((SwGraphicContent*)pUserData)->GetYPos());
#endif
                        bRet = true;
                    break;
#if OSL_DEBUG_LEVEL > 1
                    case CONTENT_TYPE_TABLE:
                    case CONTENT_TYPE_FRAME:
                        sEntry = OUString::number(
                                        ((SwContent*)pUserData)->GetYPos() );
                        bRet = true;
                    break;
#endif
                }
                if(((SwContent*)pUserData)->IsInvisible())
                {
                    if(sEntry.Len())
                        sEntry += OUString(", ");
                    sEntry += sInvisible;
                    bRet = true;
                }
            }
            else
            {
                sal_uInt16 nMemberCount = ((SwContentType*)pUserData)->GetMemberCount();
                sEntry = OUString::number(nMemberCount);
                sEntry += ' ';
                sEntry += nMemberCount == 1
                            ? ((SwContentType*)pUserData)->GetSingleName()
                            : ((SwContentType*)pUserData)->GetName();
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
                            QUICKHELP_LEFT|QUICKHELP_VCENTER );
                    bCallBase = false;
                }
            }
            else
            {
                Help::ShowQuickHelp( this, Rectangle(), aEmptyStr, 0 );
                bCallBase = false;
            }
        }
    }
    if( bCallBase )
        Window::RequestHelp( rHEvt );
}

void    SwContentTree::ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
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
            if(nOutlineLevel != nSelectedPopupEntry )
                SetOutlineLevel((sal_Int8)nSelectedPopupEntry);
        break;
        case 201:
        case 202:
        case 203:
            GetParentWindow()->SetRegionDropMode(nSelectedPopupEntry - 201);
        break;
        case 401:
        case 402:
            EditEntry(pFirst, nSelectedPopupEntry == 401 ? EDIT_MODE_RMV_IDX : EDIT_MODE_UPD_IDX);
        break;
        // Edit entry
        case 403:
            EditEntry(pFirst, EDIT_MODE_EDIT);
        break;
        case 404:
            EditEntry(pFirst, EDIT_UNPROTECT_TABLE);
        break;
        case 405 :
        {
            const SwTOXBase* pBase = ((SwTOXBaseContent*)pFirst->GetUserData())
                                                                ->GetTOXBase();
            pActiveShell->SetTOXBaseReadonly(*pBase, !pActiveShell->IsTOXBaseReadonly(*pBase));
        }
        break;
        case 4:
        break;
        case 501:
            EditEntry(pFirst, EDIT_MODE_DELETE);
        break;
        case 502 :
            EditEntry(pFirst, EDIT_MODE_RENAME);
        break;
        case 600:
            pActiveShell->GetView().GetPostItMgr()->Show();
            break;
        case 601:
            pActiveShell->GetView().GetPostItMgr()->Hide();
            break;
        case 602:
            {
                pActiveShell->GetView().GetPostItMgr()->SetActiveSidebarWin(0);
                pActiveShell->GetView().GetPostItMgr()->Delete();
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
                bViewHasChanged = bIsActive = nSelectedPopupEntry == 1;
                bIsConstant = false;
                Display(nSelectedPopupEntry == 1);
            }
        }
    }
    GetParentWindow()->UpdateListBox();
}

void SwContentTree::SetOutlineLevel(sal_uInt8 nSet)
{
    nOutlineLevel = nSet;
    pConfig->SetOutlineLevel( nOutlineLevel );
    SwContentType** ppContentT = bIsActive ?
                    &aActiveContentArr[CONTENT_TYPE_OUTLINE] :
                        &aHiddenContentArr[CONTENT_TYPE_OUTLINE];
    if(*ppContentT)
    {
        (*ppContentT)->SetOutlineLevel(nOutlineLevel);
        (*ppContentT)->Init();
    }
    Display(bIsActive);
}

// Mode Change: Show dropped Doc

void SwContentTree::ShowHiddenShell()
{
    if(pHiddenShell)
    {
        bIsConstant = false;
        bIsActive = false;
        Display(false);
    }
}

// Mode Change: Show active view

void SwContentTree::ShowActualView()
{
    bIsActive = true;
    bIsConstant = false;
    Display(true);
    GetParentWindow()->UpdateListBox();
}

// Here are the buttons for moving outlines are en-/disabled.

sal_Bool  SwContentTree::Select( SvTreeListEntry* pEntry, sal_Bool bSelect )
{
    if(!pEntry)
        return sal_False;
    sal_Bool bEnable = sal_False;
    SvTreeListEntry* pParentEntry = GetParent(pEntry);
    while(pParentEntry && (!lcl_IsContentType(pParentEntry)))
    {
        pParentEntry = GetParent(pParentEntry);
    }
    if(!bIsLastReadOnly && (!IsVisible() ||
        ( (bIsRoot && nRootType == CONTENT_TYPE_OUTLINE && pParentEntry) ||
            (lcl_IsContent(pEntry) && ((SwContentType*)pParentEntry->GetUserData())->GetType() == CONTENT_TYPE_OUTLINE)) ))
        bEnable = sal_True;
    SwNavigationPI* pNavi = GetParentWindow();
    pNavi->aContentToolBox.EnableItem(FN_ITEM_UP ,  bEnable);
    pNavi->aContentToolBox.EnableItem(FN_ITEM_DOWN, bEnable);
    pNavi->aContentToolBox.EnableItem(FN_ITEM_LEFT, bEnable);
    pNavi->aContentToolBox.EnableItem(FN_ITEM_RIGHT,bEnable);

    return SvTreeListBox::Select(pEntry, bSelect);
}

void SwContentTree::SetRootType(sal_uInt16 nType)
{
    nRootType = nType;
    bIsRoot = true;
    pConfig->SetRootType( nRootType );
}

void SwContentType::RemoveNewline(String& rEntry)
{
    sal_Unicode* pStr = rEntry.GetBufferAccess();
    for(xub_StrLen i = rEntry.Len(); i; --i, ++pStr )
    {
        if( *pStr == 10 || *pStr == 13 )
            *pStr = 0x20;
    }
}

void SwContentTree::EditEntry(SvTreeListEntry* pEntry, sal_uInt8 nMode)
{
    SwContent* pCnt = (SwContent*)pEntry->GetUserData();
    GotoContent(pCnt);
    sal_uInt16 nType = pCnt->GetParent()->GetType();
    sal_uInt16 nSlot = 0;

    uno::Reference< container::XNameAccess >  xNameAccess, xSecond, xThird;
    switch(nType)
    {
        case CONTENT_TYPE_TABLE     :
            if(nMode == EDIT_UNPROTECT_TABLE)
            {
                pActiveShell->GetView().GetDocShell()->
                        GetDoc()->UnProtectCells( pCnt->GetName());
            }
            else if(nMode == EDIT_MODE_DELETE)
            {
                pActiveShell->StartAction();
                String sTable = SW_RES(STR_TABLE_NAME);
                SwRewriter aRewriterTableName;
                aRewriterTableName.AddRule(UndoArg1, SW_RES(STR_START_QUOTE));
                aRewriterTableName.AddRule(UndoArg2, pCnt->GetName());
                aRewriterTableName.AddRule(UndoArg3, SW_RES(STR_END_QUOTE));
                sTable = aRewriterTableName.Apply(sTable);

                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, sTable);
                pActiveShell->StartUndo(UNDO_DELETE, &aRewriter);
                pActiveShell->GetView().GetViewFrame()->GetDispatcher()->Execute(FN_TABLE_SELECT_ALL);
                pActiveShell->DeleteRow();
                pActiveShell->EndUndo();
                pActiveShell->EndAction();
            }
            else if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextTablesSupplier >  xTables(xModel, uno::UNO_QUERY);
                xNameAccess = xTables->getTextTables();
            }
            else
                nSlot = FN_FORMAT_TABLE_DLG;
        break;

        case CONTENT_TYPE_GRAPHIC   :
            if(nMode == EDIT_MODE_DELETE)
            {
                pActiveShell->DelRight();
            }
            else if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
                xNameAccess = xGraphics->getGraphicObjects();
                uno::Reference< text::XTextFramesSupplier >  xFrms(xModel, uno::UNO_QUERY);
                xSecond = xFrms->getTextFrames();
                uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
                xThird = xObjs->getEmbeddedObjects();
            }
            else
                nSlot = FN_FORMAT_GRAFIC_DLG;
        break;

        case CONTENT_TYPE_FRAME     :
        case CONTENT_TYPE_OLE       :
            if(nMode == EDIT_MODE_DELETE)
            {
                pActiveShell->DelRight();
            }
            else if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextFramesSupplier >  xFrms(xModel, uno::UNO_QUERY);
                uno::Reference< text::XTextEmbeddedObjectsSupplier >  xObjs(xModel, uno::UNO_QUERY);
                if(CONTENT_TYPE_FRAME == nType)
                {
                    xNameAccess = xFrms->getTextFrames();
                    xSecond = xObjs->getEmbeddedObjects();
                }
                else
                {
                    xNameAccess = xObjs->getEmbeddedObjects();
                    xSecond = xFrms->getTextFrames();
                }
                uno::Reference< text::XTextGraphicObjectsSupplier >  xGraphics(xModel, uno::UNO_QUERY);
                xThird = xGraphics->getGraphicObjects();
            }
            else
                nSlot = FN_FORMAT_FRAME_DLG;
        break;
        case CONTENT_TYPE_BOOKMARK  :
            if(nMode == EDIT_MODE_DELETE)
            {
                IDocumentMarkAccess* const pMarkAccess = pActiveShell->getIDocumentMarkAccess();
                pMarkAccess->deleteMark( pMarkAccess->findMark(pCnt->GetName()) );
            }
            else if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XBookmarksSupplier >  xBkms(xModel, uno::UNO_QUERY);
                xNameAccess = xBkms->getBookmarks();
            }
            else
                nSlot = FN_INSERT_BOOKMARK;
        break;

        case CONTENT_TYPE_REGION    :
            if(nMode == EDIT_MODE_RENAME)
            {
                uno::Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                uno::Reference< text::XTextSectionsSupplier >  xSects(xModel, uno::UNO_QUERY);
                xNameAccess = xSects->getTextSections();
            }
            else
                nSlot = FN_EDIT_REGION;
        break;

        case CONTENT_TYPE_URLFIELD:
            nSlot = FN_EDIT_HYPERLINK;
        break;
        case CONTENT_TYPE_REFERENCE:
            nSlot = FN_EDIT_FIELD;
        break;

        case CONTENT_TYPE_POSTIT:
            pActiveShell->GetView().GetPostItMgr()->AssureStdModeAtShell();
            if(nMode == EDIT_MODE_DELETE)
            {
                if (((SwPostItContent*)pCnt)->IsPostIt())
                {
                    pActiveShell->GetView().GetPostItMgr()->SetActiveSidebarWin(0);
                    pActiveShell->DelRight();
                }
            }
            else
            {
                if (((SwPostItContent*)pCnt)->IsPostIt())
                    nSlot = FN_POSTIT;
                else
                    nSlot = FN_REDLINE_COMMENT;
            }
        break;
        case CONTENT_TYPE_INDEX:
        {
            const SwTOXBase* pBase = ((SwTOXBaseContent*)pCnt)->GetTOXBase();
            switch(nMode)
            {
                case EDIT_MODE_EDIT:
                    if(pBase)
                    {
                        SwPtrItem aPtrItem( FN_INSERT_MULTI_TOX, (void*)pBase);
                        pActiveShell->GetView().GetViewFrame()->
                            GetDispatcher()->Execute(FN_INSERT_MULTI_TOX,
                                            SFX_CALLMODE_ASYNCHRON, &aPtrItem, 0L);

                    }
                break;
                case EDIT_MODE_RMV_IDX:
                case EDIT_MODE_DELETE:
                {
                    if( pBase )
                        pActiveShell->DeleteTOX(*pBase, EDIT_MODE_DELETE == nMode);
                }
                break;
                case EDIT_MODE_UPD_IDX:
                case EDIT_MODE_RENAME:
                {
                    Reference< frame::XModel >  xModel = pActiveShell->GetView().GetDocShell()->GetBaseModel();
                    Reference< XDocumentIndexesSupplier >  xIndexes(xModel, UNO_QUERY);
                    Reference< XIndexAccess> xIdxAcc(xIndexes->getDocumentIndexes());
                    Reference< XNameAccess >xLocalNameAccess(xIdxAcc, UNO_QUERY);
                    if(EDIT_MODE_RENAME == nMode)
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
            }
        }
        break;
        case CONTENT_TYPE_DRAWOBJECT :
            if(EDIT_MODE_DELETE == nMode)
                nSlot = SID_DELETE;
        break;
    }
    if(nSlot)
        pActiveShell->GetView().GetViewFrame()->
                    GetDispatcher()->Execute(nSlot, SFX_CALLMODE_ASYNCHRON);
    else if(xNameAccess.is())
    {
        uno::Any aObj = xNameAccess->getByName(pCnt->GetName());
        uno::Reference< uno::XInterface >  xTmp;
        aObj >>= xTmp;
        uno::Reference< container::XNamed >  xNamed(xTmp, uno::UNO_QUERY);
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        AbstractSwRenameXNamedDlg* pDlg = pFact->CreateSwRenameXNamedDlg(this, xNamed, xNameAccess);
        OSL_ENSURE(pDlg, "Dialogdiet fail!");
        if(xSecond.is())
            pDlg->SetAlternativeAccess( xSecond, xThird);

        String sForbiddenChars;
        if(CONTENT_TYPE_BOOKMARK == nType)
        {
            sForbiddenChars = OUString("/\\@:*?\";,.#");
        }
        else if(CONTENT_TYPE_TABLE == nType)
        {
            sForbiddenChars = OUString(" .<>");
        }
        pDlg->SetForbiddenChars(sForbiddenChars);
        pDlg->Execute();
        delete pDlg;
    }
}

void SwContentTree::GotoContent(SwContent* pCnt)
{
    pActiveShell->EnterStdMode();

    bool bSel = false;
    sal_uInt16 nJumpType = pCnt->GetParent()->GetType();
    switch(nJumpType)
    {
        case CONTENT_TYPE_OUTLINE   :
        {
            pActiveShell->GotoOutline(((SwOutlineContent*)pCnt)->GetPos());
        }
        break;
        case CONTENT_TYPE_TABLE     :
        {
            pActiveShell->GotoTable(pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_FRAME     :
        case CONTENT_TYPE_GRAPHIC   :
        case CONTENT_TYPE_OLE       :
        {
            if(pActiveShell->GotoFly(pCnt->GetName()))
                bSel = true;
        }
        break;
        case CONTENT_TYPE_BOOKMARK:
        {
            pActiveShell->GotoMark(pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_REGION    :
        {
            pActiveShell->GotoRegion(pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_URLFIELD:
        {
            if(pActiveShell->GotoINetAttr(
                            *((SwURLFieldContent*)pCnt)->GetINetAttr() ))
            {
                pActiveShell->Right( CRSR_SKIP_CHARS, sal_True, 1, sal_False);
                pActiveShell->SwCrsrShell::SelectTxtAttr( RES_TXTATR_INETFMT, sal_True );
            }

        }
        break;
        case CONTENT_TYPE_REFERENCE:
        {
            pActiveShell->GotoRefMark(pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_INDEX:
        {
            if (!pActiveShell->GotoNextTOXBase(&pCnt->GetName()))
                pActiveShell->GotoPrevTOXBase(&pCnt->GetName());
        }
        break;
        case CONTENT_TYPE_POSTIT:
            pActiveShell->GetView().GetPostItMgr()->AssureStdModeAtShell();
            if (((SwPostItContent*)pCnt)->IsPostIt())
                pActiveShell->GotoFld(*((SwPostItContent*)pCnt)->GetPostIt());
            else
                pActiveShell->GetView().GetDocShell()->GetWrtShell()->GotoRedline(
                        pActiveShell->GetView().GetDocShell()->GetWrtShell()->FindRedlineOfData(((SwPostItContent*)pCnt)->GetRedline()->GetRedlineData()));

        break;
        case CONTENT_TYPE_DRAWOBJECT:
        {
            SwPosition aPos = *pActiveShell->GetCrsr()->GetPoint();
            SdrView* pDrawView = pActiveShell->GetDrawView();
            if (pDrawView)
            {
                pDrawView->SdrEndTextEdit();
                pDrawView->UnmarkAll();
                SdrModel* _pModel = pActiveShell->getIDocumentDrawModelAccess()->GetDrawModel();
                SdrPage* pPage = _pModel->GetPage(0);
                sal_uInt32 nCount = pPage->GetObjCount();
                for( sal_uInt32 i=0; i< nCount; i++ )
                {
                    SdrObject* pTemp = pPage->GetObj(i);
                    // #i51726# - all drawing objects can be named now
                    if (pTemp->GetName().equals(pCnt->GetName()))
                    {
                        SdrPageView* pPV = pDrawView->GetSdrPageView();
                        if( pPV )
                        {
                            pDrawView->MarkObj( pTemp, pPV );
                        }
                    }
                }
                pActiveShell->GetNavigationMgr().addEntry(aPos);
            }
        }
        break;
    }
    if(bSel)
    {
        pActiveShell->HideCrsr();
        pActiveShell->EnterSelFrmMode();
    }
    SwView& rView = pActiveShell->GetView();
    rView.StopShellTimer();
    rView.GetPostItMgr()->SetActiveSidebarWin(0);
    rView.GetEditWin().GrabFocus();
}

// Now even the matching text::Bookmark
NaviContentBookmark::NaviContentBookmark()
    :
    nDocSh(0),
    nDefDrag( REGION_MODE_NONE )
{
}

NaviContentBookmark::NaviContentBookmark( const String &rUrl,
                    const String& rDesc,
                    sal_uInt16 nDragType,
                    const SwDocShell* pDocSh ) :
    aUrl( rUrl ),
    aDescr(rDesc),
    nDocSh((sal_IntPtr)pDocSh),
    nDefDrag( nDragType )
{
}

void NaviContentBookmark::Copy( TransferDataContainer& rData ) const
{
    rtl_TextEncoding eSysCSet = osl_getThreadTextEncoding();

    OString sStrBuf(OUStringToOString(aUrl, eSysCSet) + OString(NAVI_BOOKMARK_DELIM) +
                    OUStringToOString(aDescr, eSysCSet) + OString(NAVI_BOOKMARK_DELIM) +
                    OString::number(nDefDrag) + OString(NAVI_BOOKMARK_DELIM) +
                    OString::number(nDocSh));
    rData.CopyByteString(SOT_FORMATSTR_ID_SONLK, sStrBuf);
}

sal_Bool NaviContentBookmark::Paste( TransferableDataHelper& rData )
{
    String sStr;
    sal_Bool bRet = rData.GetString( SOT_FORMATSTR_ID_SONLK, sStr );
    if( bRet )
    {
        sal_Int32 nPos = 0;
        aUrl    = sStr.GetToken(0, NAVI_BOOKMARK_DELIM, nPos );
        aDescr  = sStr.GetToken(0, NAVI_BOOKMARK_DELIM, nPos );
        nDefDrag= (sal_uInt16)sStr.GetToken(0, NAVI_BOOKMARK_DELIM, nPos ).ToInt32();
        nDocSh  = sStr.GetToken(0, NAVI_BOOKMARK_DELIM, nPos ).ToInt32();
    }
    return bRet;
}

class SwContentLBoxString : public SvLBoxString
{
public:
    SwContentLBoxString( SvTreeListEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr) {}

    virtual void Paint(
        const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView,
        const SvTreeListEntry* pEntry);
};

void SwContentTree::InitEntry(SvTreeListEntry* pEntry,
        const OUString& rStr ,const Image& rImg1,const Image& rImg2,
        SvLBoxButtonKind eButtonKind)
{
    sal_uInt16 nColToHilite = 1; //0==Bitmap;1=="Column1";2=="Column2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nColToHilite );
    SwContentLBoxString* pStr = new SwContentLBoxString( pEntry, 0, pCol->GetText() );
    pEntry->ReplaceItem( pStr, nColToHilite );
}

void SwContentLBoxString::Paint(
    const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView,
    const SvTreeListEntry* pEntry)
{
    if(lcl_IsContent(pEntry) &&
            ((SwContent *)pEntry->GetUserData())->IsInvisible())
    {
        Font aOldFont( rDev.GetFont());
        Font aFont(aOldFont);
        Color aCol( COL_LIGHTGRAY );
        aFont.SetColor( aCol );
        rDev.SetFont( aFont );
        rDev.DrawText( rPos, GetText() );
        rDev.SetFont( aOldFont );
    }
    else
        SvLBoxString::Paint( rPos, rDev, pView, pEntry);
}

void    SwContentTree::DataChanged( const DataChangedEvent& rDCEvt )
{
  if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        aEntryImages = ImageList(SW_RES(IMG_NAVI_ENTRYBMP));
        FindActiveTypeAndRemoveUserData();
        Display(true);
    }
    SvTreeListBox::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
