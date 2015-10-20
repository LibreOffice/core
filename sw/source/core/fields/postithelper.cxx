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
#include <docufld.hxx>
#include <ndtxt.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <tabfrm.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <redline.hxx>
#include <scriptinfo.hxx>
#include <editeng/charhiddenitem.hxx>
#include <calbck.hxx>

class Point;

SwPostItHelper::SwLayoutStatus SwPostItHelper::getLayoutInfos(
    SwLayoutInfo& o_rInfo,
    const SwPosition& rAnchorPos,
    const SwPosition* pAnnotationStartPos )
{
    SwLayoutStatus aRet = INVISIBLE;
    SwTextNode* pTextNode = rAnchorPos.nNode.GetNode().GetTextNode();
    if ( pTextNode == NULL )
        return aRet;

    SwIterator<SwTextFrm,SwContentNode> aIter( *pTextNode );
    for( SwTextFrm* pTextFrm = aIter.First(); pTextFrm != NULL; pTextFrm = aIter.Next() )
    {
        if( !pTextFrm->IsFollow() )
        {
            pTextFrm = pTextFrm->GetFrmAtPos( rAnchorPos );
            SwPageFrm *pPage = pTextFrm ? pTextFrm->FindPageFrm() : 0;
            if ( pPage != NULL && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
            {
                aRet = VISIBLE;

                o_rInfo.mpAnchorFrm = pTextFrm;
                pTextFrm->GetCharRect( o_rInfo.mPosition, rAnchorPos );
                if ( pAnnotationStartPos != NULL )
                {
                    o_rInfo.mnStartNodeIdx = pAnnotationStartPos->nNode.GetIndex();
                    o_rInfo.mnStartContent = pAnnotationStartPos->nContent.GetIndex();
                }
                else
                {
                    o_rInfo.mnStartNodeIdx = 0;
                    o_rInfo.mnStartContent = -1;
                }
                o_rInfo.mPageFrame = pPage->Frm();
                o_rInfo.mPagePrtArea = pPage->Prt();
                o_rInfo.mPagePrtArea.Pos() += o_rInfo.mPageFrame.Pos();
                o_rInfo.mnPageNumber = pPage->GetPhyPageNum();
                o_rInfo.meSidebarPosition = pPage->SidebarPosition();
                o_rInfo.mRedlineAuthor = 0;

                const IDocumentRedlineAccess& rIDRA = pTextNode->getIDocumentRedlineAccess();
                if( IDocumentRedlineAccess::IsShowChanges( rIDRA.GetRedlineMode() ) )
                {
                    const SwRangeRedline* pRedline = rIDRA.GetRedline( rAnchorPos, 0 );
                    if( pRedline )
                    {
                        if( nsRedlineType_t::REDLINE_INSERT == pRedline->GetType() )
                            aRet = INSERTED;
                        else if( nsRedlineType_t::REDLINE_DELETE == pRedline->GetType() )
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

long SwPostItHelper::getLayoutHeight( const SwRootFrm* pRoot )
{
    long nRet = pRoot ? pRoot->Frm().Height() : 0;
    return nRet;
}

void SwPostItHelper::setSidebarChanged( SwRootFrm* pRoot, bool bBrowseMode )
{
    if( pRoot )
    {
        pRoot->SetSidebarChanged();
        if( bBrowseMode )
            pRoot->InvalidateBrowseWidth();
    }
}

unsigned long SwPostItHelper::getPageInfo( SwRect& rPageFrm, const SwRootFrm* pRoot, const Point& rPoint )
{
    unsigned long nRet = 0;
    const SwFrm* pPage = pRoot->GetPageAtPos( rPoint, 0, true );
    if( pPage )
    {
        nRet = pPage->GetPhyPageNum();
        rPageFrm = pPage->Frm();
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

bool SwAnnotationItem::UseElement()
{
    return mrFormatField.IsFieldInDoc();
}

VclPtr<sw::sidebarwindows::SwSidebarWin> SwAnnotationItem::GetSidebarWindow(
                                                            SwEditWin& rEditWin,
                                                            WinBits nBits,
                                                            SwPostItMgr& aMgr,
                                                            SwPostItBits aBits)
{
    return VclPtr<sw::annotation::SwAnnotationWin>::Create( rEditWin, nBits,
                                                aMgr, aBits,
                                                *this,
                                                &mrFormatField );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
