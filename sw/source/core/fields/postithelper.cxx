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

#include <postithelper.hxx>
#include <PostItMgr.hxx>
#include <AnnotationWin.hxx>

#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <redline.hxx>
#include <scriptinfo.hxx>
#include <calbck.hxx>
#include <IMark.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <fmtanchr.hxx>

class Point;

namespace
{
/// Checks if pAnnotationMark covers exactly rAnchorPos (the comment anchor).
bool AnnotationMarkCoversCommentAnchor(const sw::mark::IMark* pAnnotationMark,
                                       const SwPosition& rAnchorPos)
{
    if (!pAnnotationMark)
    {
        return false;
    }

    const SwPosition& rMarkStart = pAnnotationMark->GetMarkStart();
    const SwPosition& rMarkEnd = pAnnotationMark->GetMarkEnd();

    if (rMarkStart != rAnchorPos)
    {
        // This can be the as-char case: the comment placeholder character is exactly between the
        // annotation mark start and end.
        SwPosition aPosition(rMarkStart);
        ++aPosition.nContent;
        if (aPosition != rAnchorPos)
        {
            return false;
        }

        ++aPosition.nContent;
        if (aPosition != rMarkEnd)
        {
            return false;
        }

        return true;
    }

    if (rMarkStart.nNode != rMarkEnd.nNode)
    {
        return false;
    }

    return rMarkEnd.nContent.GetIndex() == rMarkStart.nContent.GetIndex() + 1;
}

/**
 * Finds the first draw object of rTextFrame which has the same anchor position as the start of
 * rAnnotationMark.
 */
SwAnchoredObject* GetAnchoredObjectOfAnnotationMark(const sw::mark::IMark& rAnnotationMark,
                                                    const SwTextFrame& rTextFrame)
{
    const SwSortedObjs* pAnchored = rTextFrame.GetDrawObjs();
    if (!pAnchored)
    {
        return nullptr;
    }

    for (SwAnchoredObject* pObject : *pAnchored)
    {
        SwFrameFormat& rFrameFormat = pObject->GetFrameFormat();
        const SwPosition* pFrameAnchor = rFrameFormat.GetAnchor().GetContentAnchor();
        if (!pFrameAnchor)
        {
            continue;
        }

        if (rAnnotationMark.GetMarkStart() == *pFrameAnchor)
        {
            return pObject;
        }
    }

    return nullptr;
}
}

SwSidebarItem::SwSidebarItem(const bool aFocus)
    : mpPostIt(nullptr)
    , mbShow(true)
    , mbFocus(aFocus)
    , mbPendingLayout(false)
    , mLayoutStatus(SwPostItHelper::INVISIBLE)
    , maLayoutInfo()
{
}

SwSidebarItem::~SwSidebarItem() {}

SwPostItHelper::SwLayoutStatus SwPostItHelper::getLayoutInfos(
    SwLayoutInfo& o_rInfo,
    const SwPosition& rAnchorPos,
    const sw::mark::IMark* pAnnotationMark )
{
    SwLayoutStatus aRet = INVISIBLE;
    SwTextNode* pTextNode = rAnchorPos.nNode.GetNode().GetTextNode();
    if ( pTextNode == nullptr )
        return aRet;

    SwIterator<SwTextFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aIter(*pTextNode);
    for( SwTextFrame* pTextFrame = aIter.First(); pTextFrame != nullptr; pTextFrame = aIter.Next() )
    {
        if( !pTextFrame->IsFollow() )
        {
            pTextFrame = pTextFrame->GetFrameAtPos( rAnchorPos );
            SwPageFrame *pPage = pTextFrame ? pTextFrame->FindPageFrame() : nullptr;
            if ( pPage != nullptr && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
            {
                aRet = VISIBLE;

                o_rInfo.mpAnchorFrame = pTextFrame;
                {
                    DisableCallbackAction a(*pTextFrame->getRootFrame());
                    bool bPositionFromCommentAnchor = true;
                    if (AnnotationMarkCoversCommentAnchor(pAnnotationMark, rAnchorPos))
                    {
                        SwAnchoredObject* pFrame
                            = GetAnchoredObjectOfAnnotationMark(*pAnnotationMark, *pTextFrame);
                        if (pFrame)
                        {
                            o_rInfo.mPosition = pFrame->GetObjRect();
                            bPositionFromCommentAnchor = false;
                        }
                    }
                    if (bPositionFromCommentAnchor)
                    {
                        pTextFrame->GetCharRect(o_rInfo.mPosition, rAnchorPos, nullptr, false);
                    }
                    o_rInfo.mPositionFromCommentAnchor = bPositionFromCommentAnchor;
                }
                if (pAnnotationMark != nullptr)
                {
                    const SwPosition& rAnnotationStartPos = pAnnotationMark->GetMarkStart();
                    o_rInfo.mnStartNodeIdx = rAnnotationStartPos.nNode.GetIndex();
                    o_rInfo.mnStartContent = rAnnotationStartPos.nContent.GetIndex();
                }
                else
                {
                    o_rInfo.mnStartNodeIdx = 0;
                    o_rInfo.mnStartContent = -1;
                }
                o_rInfo.mPageFrame = pPage->getFrameArea();
                o_rInfo.mPagePrtArea = pPage->getFramePrintArea();
                o_rInfo.mPagePrtArea.Pos() += o_rInfo.mPageFrame.Pos();
                o_rInfo.mnPageNumber = pPage->GetPhyPageNum();
                o_rInfo.meSidebarPosition = pPage->SidebarPosition();
                o_rInfo.mRedlineAuthor = 0;

                const IDocumentRedlineAccess& rIDRA = pTextNode->getIDocumentRedlineAccess();
                if( IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineFlags() ) )
                {
                    const SwRangeRedline* pRedline = rIDRA.GetRedline( rAnchorPos, nullptr );
                    if( pRedline )
                    {
                        if( RedlineType::Insert == pRedline->GetType() )
                            aRet = INSERTED;
                        else if( RedlineType::Delete == pRedline->GetType() )
                            aRet = DELETED;
                        o_rInfo.mRedlineAuthor = pRedline->GetAuthor();
                    }
                }
            }
        }
    }

    return ( (aRet==VISIBLE) && SwScriptInfo::IsInHiddenRange( *pTextNode , rAnchorPos.nContent.GetIndex()) )
             ? HIDDEN
             : aRet;
}

long SwPostItHelper::getLayoutHeight( const SwRootFrame* pRoot )
{
    long nRet = pRoot ? pRoot->getFrameArea().Height() : 0;
    return nRet;
}

void SwPostItHelper::setSidebarChanged( SwRootFrame* pRoot, bool bBrowseMode )
{
    if( pRoot )
    {
        pRoot->SetSidebarChanged();
        if( bBrowseMode )
            pRoot->InvalidateBrowseWidth();
    }
}

unsigned long SwPostItHelper::getPageInfo( SwRect& rPageFrame, const SwRootFrame* pRoot, const Point& rPoint )
{
    unsigned long nRet = 0;
    const SwFrame* pPage = pRoot->GetPageAtPos( rPoint, nullptr, true );
    if( pPage )
    {
        nRet = pPage->GetPhyPageNum();
        rPageFrame = pPage->getFrameArea();
    }
    return nRet;
}

SwPosition SwAnnotationItem::GetAnchorPosition() const
{
    SwTextField* pTextField = mrFormatField.GetTextField();
    SwTextNode* pTextNode = pTextField->GetpTextNode();

    SwPosition aPos( *pTextNode );
    aPos.nContent.Assign( pTextNode, pTextField->GetStart() );
    return aPos;
}

bool SwAnnotationItem::UseElement(SwRootFrame const& rLayout,
        IDocumentRedlineAccess const& rIDRA)
{
    return mrFormatField.IsFieldInDoc()
        && (!rLayout.IsHideRedlines()
            || !sw::IsFieldDeletedInModel(rIDRA, *mrFormatField.GetTextField()));
}

VclPtr<sw::annotation::SwAnnotationWin> SwAnnotationItem::GetSidebarWindow(
                                                            SwEditWin& rEditWin,
                                                            SwPostItMgr& aMgr)
{
    return VclPtr<sw::annotation::SwAnnotationWin>::Create( rEditWin,
                                                aMgr,
                                                *this,
                                                &mrFormatField );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
