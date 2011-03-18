/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <tools/gen.hxx>

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
#include <switerator.hxx>

namespace {

struct LayoutInfoOrder
{
    bool operator()( const SwLayoutInfo& rLayoutInfo,
                     const SwLayoutInfo& rNewLayoutInfo )
    {
        if ( rLayoutInfo.mnPageNumber != rNewLayoutInfo.mnPageNumber )
        {
            // corresponding <SwFrm> instances are on different pages
            return rLayoutInfo.mnPageNumber < rNewLayoutInfo.mnPageNumber;
        }
        else
        {
            // corresponding <SwFrm> instances are in different repeating table header rows
            ASSERT( rLayoutInfo.mpAnchorFrm->FindTabFrm(),
                    "<LayoutInfoOrder::operator()> - table frame not found" );
            ASSERT( rNewLayoutInfo.mpAnchorFrm->FindTabFrm(),
                    "<LayoutInfoOrder::operator()> - table frame not found" );
            const SwTabFrm* pLayoutInfoTabFrm( rLayoutInfo.mpAnchorFrm->FindTabFrm() );
            const SwTabFrm* pNewLayoutInfoTabFrm( rNewLayoutInfo.mpAnchorFrm->FindTabFrm() );
            const SwTabFrm* pTmpTabFrm( pNewLayoutInfoTabFrm );
            while ( pTmpTabFrm && pTmpTabFrm->GetFollow() )
            {
                pTmpTabFrm = static_cast<const SwTabFrm*>(pTmpTabFrm->GetFollow()->GetFrm());
                if ( pTmpTabFrm == pLayoutInfoTabFrm )
                {
                    return false;
                }
            }
            return true;
        }
    }
};

} // eof anonymous namespace

SwPostItHelper::SwLayoutStatus SwPostItHelper::getLayoutInfos( std::vector< SwLayoutInfo >& rInfo, SwPosition& rPos )
{
    SwLayoutStatus aRet = INVISIBLE;
    const SwTxtNode* pTxtNode = rPos.nNode.GetNode().GetTxtNode();
    SwCntntNode* pNode = rPos.nNode.GetNode().GetCntntNode();   // getfirstcontentnode // getnext...
    if( !pNode )
        return aRet;
    SwIterator<SwTxtFrm,SwCntntNode> aIter( *pNode );
    for( SwTxtFrm* pTxtFrm = aIter.First(); pTxtFrm; pTxtFrm = aIter.Next() )
    {
        if( !pTxtFrm->IsFollow() )
        {
            pTxtFrm = ((SwTxtFrm*)pTxtFrm)->GetFrmAtPos( rPos );
        SwPageFrm *pPage = pTxtFrm ? pTxtFrm->FindPageFrm() : 0;
        // #i103490#
            if ( pPage && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
            {
                SwLayoutInfo aInfo;
                pTxtFrm->GetCharRect( aInfo.mPosition, rPos, 0 );
                aInfo.mpAnchorFrm = pTxtFrm;
                aInfo.mPageFrame = pPage->Frm();
                aInfo.mPagePrtArea = pPage->Prt();
                aInfo.mPagePrtArea.Pos() += aInfo.mPageFrame.Pos();
                aInfo.mnPageNumber = pPage->GetPhyPageNum();
                aInfo.meSidebarPosition = pPage->SidebarPosition();
                aInfo.mRedlineAuthor = 0;

                if( aRet == INVISIBLE )
                {
                    aRet = VISIBLE;
                    const IDocumentRedlineAccess* pIDRA = pNode->getIDocumentRedlineAccess();
                    if( IDocumentRedlineAccess::IsShowChanges( pIDRA->GetRedlineMode() ) )
                    {
                        const SwRedline* pRedline = pIDRA->GetRedline( rPos, 0 );
                        if( pRedline )
                        {
                            if( nsRedlineType_t::REDLINE_INSERT == pRedline->GetType() )
                                aRet = INSERTED;
                            else if( nsRedlineType_t::REDLINE_DELETE == pRedline->GetType() )
                                aRet = DELETED;
                            aInfo.mRedlineAuthor = pRedline->GetAuthor();
                        }
                    }
                }

                {
                    std::vector< SwLayoutInfo >::iterator aInsPosIter =
                                std::lower_bound( rInfo.begin(), rInfo.end(),
                                                  aInfo, LayoutInfoOrder() );

                    rInfo.insert( aInsPosIter, aInfo );
                }
            }
        }
    }
    return ((aRet==VISIBLE) && SwScriptInfo::IsInHiddenRange( *pTxtNode , rPos.nContent.GetIndex()) ) ? HIDDEN : aRet;
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
    SwTxtFld* pFld = pFmtFld->GetTxtFld();
    //if( pFld )
    //{
        SwTxtNode* pTNd = pFld->GetpTxtNode();
    //  if( pTNd )
    //  {
            SwPosition aPos( *pTNd );
            aPos.nContent.Assign( pTNd, *pFld->GetStart() );
            return aPos;
    //  }
    //}
}

bool SwAnnotationItem::UseElement()
{
    return pFmtFld->IsFldInDoc();
}

sw::sidebarwindows::SwSidebarWin* SwAnnotationItem::GetSidebarWindow(
                                                            SwEditWin& rEditWin,
                                                            WinBits nBits,
                                                            SwPostItMgr& aMgr,
                                                            SwPostItBits aBits)
{
    return new sw::annotation::SwAnnotationWin( rEditWin, nBits,
                                                aMgr, aBits,
                                                *this,
                                                pFmtFld );
}

/*
SwPosition SwRedCommentItem::GetAnchorPosition()
{
    return *pRedline->Start();
}

SwSidebarWin* SwRedCommentItem::GetSidebarWindow(Window* pParent, WinBits nBits,SwPostItMgr* aMgr,SwPostItBits aBits)
{
    return new SwRedComment(pParent,nBits,aMgr,aBits,pRedline);
}

bool SwRedCommentItem::UseElement()
{
    return true;
}
*/
