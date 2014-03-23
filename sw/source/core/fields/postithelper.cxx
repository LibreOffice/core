/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

SwPostItHelper::SwLayoutStatus SwPostItHelper::getLayoutInfos(
    SwLayoutInfo& o_rInfo,
    const SwPosition& rAnchorPos,
    const SwPosition* pAnnotationStartPos )
{
    SwLayoutStatus aRet = INVISIBLE;
    SwTxtNode* pTxtNode = rAnchorPos.nNode.GetNode().GetTxtNode();
    if ( pTxtNode == NULL )
        return aRet;

    SwIterator<SwTxtFrm,SwCntntNode> aIter( *pTxtNode );
    for( SwTxtFrm* pTxtFrm = aIter.First(); pTxtFrm != NULL; pTxtFrm = aIter.Next() )
    {
        if( !pTxtFrm->IsFollow() )
        {
            pTxtFrm = pTxtFrm->GetFrmAtPos( rAnchorPos );
            SwPageFrm *pPage = pTxtFrm ? pTxtFrm->FindPageFrm() : 0;
            if ( pPage != NULL && !pPage->IsInvalid() && !pPage->IsInvalidFly() )
            {
                aRet = VISIBLE;

                o_rInfo.mpAnchorFrm = pTxtFrm;
                pTxtFrm->GetCharRect( o_rInfo.mPosition, rAnchorPos, 0 );
                if ( pAnnotationStartPos != NULL )
                {
                    o_rInfo.mnStartNodeIdx = pAnnotationStartPos->nNode.GetIndex();
                    o_rInfo.mnStartContent = pAnnotationStartPos->nContent.GetIndex();
                }
                else
                {
                    o_rInfo.mnStartNodeIdx = 0;
                    o_rInfo.mnStartContent = STRING_NOTFOUND;
                }
                o_rInfo.mPageFrame = pPage->Frm();
                o_rInfo.mPagePrtArea = pPage->Prt();
                o_rInfo.mPagePrtArea.Pos() += o_rInfo.mPageFrame.Pos();
                o_rInfo.mnPageNumber = pPage->GetPhyPageNum();
                o_rInfo.meSidebarPosition = pPage->SidebarPosition();
                o_rInfo.mRedlineAuthor = 0;

                const IDocumentRedlineAccess* pIDRA = pTxtNode->getIDocumentRedlineAccess();
                if( IDocumentRedlineAccess::IsShowChanges( pIDRA->GetRedlineMode() ) )
                {
                    const SwRedline* pRedline = pIDRA->GetRedline( rAnchorPos, 0 );
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

    return ( (aRet==VISIBLE) && SwScriptInfo::IsInHiddenRange( *pTxtNode , rAnchorPos.nContent.GetIndex()) )
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
    SwTxtFld* pTxtFld = mrFmtFld.GetTxtFld();
    //if( pTxtFld )
    //{
        SwTxtNode* pTxtNode = pTxtFld->GetpTxtNode();
        //if( pTxtNode )
        //{
            SwPosition aPos( *pTxtNode );
            aPos.nContent.Assign( pTxtNode, *(pTxtFld->GetStart()) );
            return aPos;
        //}
    //}
}

bool SwAnnotationItem::UseElement()
{
    return mrFmtFld.IsFldInDoc();
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
                                                &mrFmtFld );
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
