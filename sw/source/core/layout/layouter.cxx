/*************************************************************************
 *
 *  $RCSfile: layouter.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "layouter.hxx"
#include "doc.hxx"
#include "sectfrm.hxx"
#include "ftnboss.hxx"
#include "cntfrm.hxx"
#include "pagefrm.hxx"
#include "ftnfrm.hxx"

#define LOOP_DETECT 200

class SwLooping
{
    USHORT nMinPage;
    USHORT nMaxPage;
    USHORT nCount;
public:
    SwLooping( SwPageFrm* pPage );
    void Control( SwPageFrm* pPage );
    static void Drastic( SwFrm* pFrm );
};

class SwEndnoter
{
    SwLayouter* pMaster;
    SwSectionFrm* pSect;
    SvPtrarr* pEndArr;
public:
    SwEndnoter( SwLayouter* pLay )
        : pMaster( pLay ), pSect( NULL ), pEndArr( NULL ) {}
    ~SwEndnoter() { delete pEndArr; }
    void CollectEndnotes( SwSectionFrm* pSct );
    void CollectEndnote( SwFtnFrm* pFtn );
    void Collect( SwFtnFrm* pFtn );
    const SwSectionFrm* GetSect() { return pSect; }
    void InsertEndnotes();
    BOOL HasEndnotes() const { return pEndArr && pEndArr->Count(); }
};

void SwEndnoter::CollectEndnotes( SwSectionFrm* pSct )
{
    ASSERT( pSct, "CollectEndnotes: Which section?" );
    if( !pSect )
        pSect = pSct;
    else if( pSct != pSect )
        return;
    pSect->CollectEndnotes( pMaster );
}

void SwEndnoter::CollectEndnote( SwFtnFrm* pFtn )
{
    if( pEndArr && USHRT_MAX != pEndArr->GetPos( (VoidPtr)pFtn ) )
        return;

    if( pFtn->GetUpper() )
    {
        // pFtn is the master, he incorporates its follows
        SwFtnFrm *pNxt = pFtn->GetFollow();
        while ( pNxt )
        {
            SwFrm *pCnt = pNxt->ContainsAny();
            if ( pCnt )
            {
                do
                {   SwFrm *pNxtCnt = pCnt->GetNext();
                    pCnt->Cut();
                    pCnt->Paste( pFtn );
                    pCnt = pNxtCnt;
                } while ( pCnt );
            }
            else
            {   ASSERT( pNxt->Lower() && pNxt->Lower()->IsSctFrm(),
                        "Endnote without content?" );
                pNxt->Cut();
                delete pNxt;
            }
            pNxt = pFtn->GetFollow();
        }
        if( pFtn->GetMaster() )
            return;
        pFtn->Cut();
    }
    else if( pEndArr )
    {
        for ( USHORT i = 0; i < pEndArr->Count(); ++i )
        {
            SwFtnFrm *pEndFtn = (SwFtnFrm*)((*pEndArr)[i]);
            if( pEndFtn->GetAttr() == pFtn->GetAttr() )
            {
                delete pFtn;
                return;
            }
        }
    }
    if( !pEndArr )
        pEndArr = new SvPtrarr( 5, 5 );  // deleted from the SwLayouter
    pEndArr->Insert( (VoidPtr)pFtn, pEndArr->Count() );
}

void SwEndnoter::Collect( SwFtnFrm* pFtn )
{
}

void SwEndnoter::InsertEndnotes()
{
    if( !pSect )
        return;
    if( !pEndArr || !pEndArr->Count() )
    {
        pSect = NULL;
        return;
    }
    ASSERT( pSect->Lower() && pSect->Lower()->IsFtnBossFrm(),
            "InsertEndnotes: Where's my column?" );
    SwFrm* pRef = pSect->FindLastCntnt( FINDMODE_MYLAST );
    SwFtnBossFrm *pBoss = pRef ? pRef->FindFtnBossFrm()
                               : (SwFtnBossFrm*)pSect->Lower();
    pBoss->_MoveFtns( *pEndArr );
    delete pEndArr;
    pEndArr = NULL;
    pSect = NULL;
}

SwLooping::SwLooping( SwPageFrm* pPage )
{
    ASSERT( pPage, "Where's my page?" );
    nMinPage = pPage->GetPhyPageNum();
    nMaxPage = nMinPage;
    nCount = 0;
}

void SwLooping::Drastic( SwFrm* pFrm )
{
    while( pFrm )
    {
        if( pFrm->IsLayoutFrm() )
            Drastic( ((SwLayoutFrm*)pFrm)->Lower() );
        pFrm->bValidPos = TRUE;
        pFrm->bValidSize = TRUE;
        pFrm->bValidPrtArea = TRUE;
        pFrm = pFrm->GetNext();
    }
}

void SwLooping::Control( SwPageFrm* pPage )
{
    if( !pPage )
        return;
    USHORT nNew = pPage->GetPhyPageNum();
    if( nNew > nMaxPage )
        nMaxPage = nNew;
    if( nNew < nMinPage )
    {
        nMinPage = nNew;
        nMaxPage = nNew;
        nCount = 0;
    }
    else if( nNew > nMinPage + 2 )
    {
        nMinPage = nNew - 2;
        nMaxPage = nNew;
        nCount = 0;
    }
    else if( ++nCount > LOOP_DETECT )
    {
#ifndef PRODUCT
#ifdef DEBUG
        static BOOL bNoLouie = FALSE;
        if( bNoLouie )
            return;
#endif
#endif
        ASSERT( FALSE, "Looping Louie" );
        nCount = 0;
        Drastic( pPage->Lower() );
        if( nNew > nMinPage && pPage->GetPrev() )
            Drastic( ((SwPageFrm*)pPage->GetPrev())->Lower() );
        if( nNew < nMaxPage && pPage->GetNext() )
            Drastic( ((SwPageFrm*)pPage->GetNext())->Lower() );
    }
}

/*************************************************************************
|*
|*  SwLayouter::SwLayouter()
|*
|*  Ersterstellung      AMA 02. Nov. 99
|*  Letzte Aenderung    AMA 02. Nov. 99
|*
|*************************************************************************/

SwLayouter::SwLayouter() : pEndnoter( NULL ), pLooping( NULL )
{
}

SwLayouter::~SwLayouter()
{
    delete pEndnoter;
    delete pLooping;
}

void SwLayouter::_CollectEndnotes( SwSectionFrm* pSect )
{
    if( !pEndnoter )
        pEndnoter = new SwEndnoter( this );
    pEndnoter->CollectEndnotes( pSect );
}

BOOL SwLayouter::HasEndnotes() const
{
    return pEndnoter->HasEndnotes();
}

void SwLayouter::CollectEndnote( SwFtnFrm* pFtn )
{
    pEndnoter->CollectEndnote( pFtn );
}

void SwLayouter::InsertEndnotes( SwSectionFrm* pSect )
{
    if( !pEndnoter || pEndnoter->GetSect() != pSect )
        return;
    pEndnoter->InsertEndnotes();
}

void SwLayouter::LoopControl( SwPageFrm* pPage, BYTE nLoop )
{
    ASSERT( pLooping, "Looping: Lost control" );
    pLooping->Control( pPage );
}

BOOL SwLayouter::StartLooping( SwPageFrm* pPage )
{
    if( pLooping )
        return FALSE;
    pLooping = new SwLooping( pPage );
    return TRUE;
}

void SwLayouter::EndLoopControl()
{
    delete pLooping;
    pLooping = NULL;
}

void SwLayouter::CollectEndnotes( SwDoc* pDoc, SwSectionFrm* pSect )
{
    ASSERT( pDoc, "No doc, no fun" );
    if( !pDoc->GetLayouter() )
        pDoc->SetLayouter( new SwLayouter() );
    pDoc->GetLayouter()->_CollectEndnotes( pSect );
}

BOOL SwLayouter::Collecting( SwDoc* pDoc, SwSectionFrm* pSect, SwFtnFrm* pFtn )
{
    if( !pDoc->GetLayouter() )
        return FALSE;
    SwLayouter *pLayouter = pDoc->GetLayouter();
    if( pLayouter->pEndnoter && pLayouter->pEndnoter->GetSect() && pSect &&
        ( pLayouter->pEndnoter->GetSect()->IsAnFollow( pSect ) ||
          pSect->IsAnFollow( pLayouter->pEndnoter->GetSect() ) ) )
    {
        if( pFtn )
            pLayouter->CollectEndnote( pFtn );
        return TRUE;
    }
    return FALSE;
}

BOOL SwLayouter::StartLoopControl( SwDoc* pDoc, SwPageFrm *pPage )
{
    ASSERT( pDoc, "No doc, no fun" );
    if( !pDoc->GetLayouter() )
        pDoc->SetLayouter( new SwLayouter() );
    return !pDoc->GetLayouter()->pLooping &&
            pDoc->GetLayouter()->StartLooping( pPage );
}


