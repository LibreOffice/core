/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include <cellfrm.hxx>
#include <rowfrm.hxx>
#include <swtable.hxx>

#include "tabfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrms.hxx"
#include "ftnfrm.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"
#include <txtfrm.hxx>
#include <switerator.hxx>


SwLayoutFrm *SwFtnBossFrm::FindBodyCont()
{
    SwFrm *pLay = Lower();
    while ( pLay && !pLay->IsBodyFrm() )
        pLay = pLay->GetNext();
    return (SwLayoutFrm*)pLay;
}


SwCntntFrm *SwPageFrm::FindLastBodyCntnt()
{
    SwCntntFrm *pRet = FindFirstBodyCntnt();
    SwCntntFrm *pNxt = pRet;
    while ( pNxt && pNxt->IsInDocBody() && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNextCnt();
    }
    return pRet;
}

/**
 * Checks if the frame contains one or more CntntFrm's anywhere in his
 * subsidiary structure; if so the first found CntntFrm is returned.
 */
const SwCntntFrm *SwLayoutFrm::ContainsCntnt() const
{
    
    
    
    
    

    const SwLayoutFrm *pLayLeaf = this;
    do
    {
        while ( (!pLayLeaf->IsSctFrm() || pLayLeaf == this ) &&
                pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrm() )
            pLayLeaf = (SwLayoutFrm*)pLayLeaf->Lower();

        if( pLayLeaf->IsSctFrm() && pLayLeaf != this )
        {
            const SwCntntFrm *pCnt = pLayLeaf->ContainsCntnt();
            if( pCnt )
                return pCnt;
            if( pLayLeaf->GetNext() )
            {
                if( pLayLeaf->GetNext()->IsLayoutFrm() )
                {
                    pLayLeaf = (SwLayoutFrm*)pLayLeaf->GetNext();
                    continue;
                }
                else
                    return (SwCntntFrm*)pLayLeaf->GetNext();
            }
        }
        else if ( pLayLeaf->Lower() )
            return (SwCntntFrm*)pLayLeaf->Lower();

        pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        if( !IsAnLower( pLayLeaf) )
            return 0;
    } while( pLayLeaf );
    return 0;
}

/**
 * Calls ContainsAny first to reach the innermost cell. From there we walk back
 * up to the first SwCellFrm. Since we use SectionFrms, ContainsCntnt()->GetUpper()
 * is not enough anymore.
 */
const SwCellFrm *SwLayoutFrm::FirstCell() const
{
    const SwFrm* pCnt = ContainsAny();
    while( pCnt && !pCnt->IsCellFrm() )
        pCnt = pCnt->GetUpper();
    return (const SwCellFrm*)pCnt;
}

/** return CntntFrms, sections, and tables.
 *
 * @param _bInvestigateFtnForSections controls investigation of content of footnotes for sections.
 * @see ContainsCntnt
 */
const SwFrm *SwLayoutFrm::ContainsAny( const bool _bInvestigateFtnForSections ) const
{
    
    
    

    const SwLayoutFrm *pLayLeaf = this;
    
    const bool bNoFtn = IsSctFrm() && !_bInvestigateFtnForSections;
    do
    {
        while ( ( (!pLayLeaf->IsSctFrm() && !pLayLeaf->IsTabFrm())
                 || pLayLeaf == this ) &&
                pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrm() )
            pLayLeaf = (SwLayoutFrm*)pLayLeaf->Lower();

        if( ( pLayLeaf->IsTabFrm() || pLayLeaf->IsSctFrm() )
            && pLayLeaf != this )
        {
            
            
            return pLayLeaf;
        }
        else if ( pLayLeaf->Lower() )
            return (SwCntntFrm*)pLayLeaf->Lower();

        pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        if( bNoFtn && pLayLeaf && pLayLeaf->IsInFtn() )
        {
            do
            {
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
            } while( pLayLeaf && pLayLeaf->IsInFtn() );
        }
        if( !IsAnLower( pLayLeaf) )
            return 0;
    } while( pLayLeaf );
    return 0;
}

const SwFrm* SwFrm::GetLower() const
{
    return IsLayoutFrm() ? ((SwLayoutFrm*)this)->Lower() : 0;
}

SwFrm* SwFrm::GetLower()
{
    return IsLayoutFrm() ? ((SwLayoutFrm*)this)->Lower() : 0;
}

sal_Bool SwLayoutFrm::IsAnLower( const SwFrm *pAssumed ) const
{
    const SwFrm *pUp = pAssumed;
    while ( pUp )
    {
        if ( pUp == this )
            return sal_True;
        if ( pUp->IsFlyFrm() )
            pUp = ((SwFlyFrm*)pUp)->GetAnchorFrm();
        else
            pUp = pUp->GetUpper();
    }
    return sal_False;
}

/** method to check relative position of layout frame to
    a given layout frame.

    OD 08.11.2002 - refactoring of pseudo-local method <lcl_Apres(..)> in
    <txtftn.cxx> for #104840#.

    @param _aCheckRefLayFrm
    constant reference of an instance of class <SwLayoutFrm> which
    is used as the reference for the relative position check.

    @return true, if <this> is positioned before the layout frame <p>
*/
bool SwLayoutFrm::IsBefore( const SwLayoutFrm* _pCheckRefLayFrm ) const
{
    OSL_ENSURE( !IsRootFrm() , "<IsBefore> called at a <SwRootFrm>.");
    OSL_ENSURE( !_pCheckRefLayFrm->IsRootFrm() , "<IsBefore> called with a <SwRootFrm>.");

    bool bReturn;

    
    const SwPageFrm *pMyPage = FindPageFrm();
    const SwPageFrm *pCheckRefPage = _pCheckRefLayFrm->FindPageFrm();
    if( pMyPage != pCheckRefPage )
    {
        
        bReturn = pMyPage->GetPhyPageNum() < pCheckRefPage->GetPhyPageNum();
    }
    else
    {
        
        
        const SwLayoutFrm* pUp = this;
        while ( pUp->GetUpper() &&
                !pUp->GetUpper()->IsAnLower( _pCheckRefLayFrm )
              )
            pUp = pUp->GetUpper();
        if( !pUp->GetUpper() )
        {
            
            bReturn = false;
        }
        else
        {
            
            
            SwLayoutFrm* pUpNext = (SwLayoutFrm*)pUp->GetNext();
            while ( pUpNext &&
                    !pUpNext->IsAnLower( _pCheckRefLayFrm ) )
            {
                pUpNext = (SwLayoutFrm*)pUpNext->GetNext();
            }
            bReturn = pUpNext != 0;
        }
    }

    return bReturn;
}



static const SwFrm* lcl_FindLayoutFrame( const SwFrm* pFrm, bool bNext )
{
    const SwFrm* pRet = 0;
    if ( pFrm->IsFlyFrm() )
        pRet = bNext ? ((SwFlyFrm*)pFrm)->GetNextLink() : ((SwFlyFrm*)pFrm)->GetPrevLink();
    else
        pRet = bNext ? pFrm->GetNext() : pFrm->GetPrev();

    return pRet;
}

static const SwFrm* lcl_GetLower( const SwFrm* pFrm, bool bFwd )
{
    if ( !pFrm->IsLayoutFrm() )
        return 0;

    return bFwd ?
           static_cast<const SwLayoutFrm*>(pFrm)->Lower() :
           static_cast<const SwLayoutFrm*>(pFrm)->GetLastLower();
}

/**
 * Finds the next layout leaf. This is a layout frame, which does not
 * have a lower which is a LayoutFrame. That means, pLower can be 0 or a
 * content frame.
 *
 * However, pLower may be a TabFrm
 */
const SwLayoutFrm *SwFrm::ImplGetNextLayoutLeaf( bool bFwd ) const
{
    const SwFrm       *pFrm = this;
    const SwLayoutFrm *pLayoutFrm = 0;
    const SwFrm       *p = 0;
    bool bGoingUp = !bFwd;          
    do {

         bool bGoingFwdOrBwd = false;

         bool bGoingDown = ( !bGoingUp && ( 0 != (p = lcl_GetLower( pFrm, bFwd ) ) ) );
         if ( !bGoingDown )
         {
             
             
             
             bGoingFwdOrBwd = (0 != (p = lcl_FindLayoutFrame( pFrm, bFwd ) ) );
             if ( !bGoingFwdOrBwd )
             {
                 
                 
                 bGoingUp = (0 != (p = pFrm->GetUpper() ) );
                 if ( !bGoingUp )
                 {
                    
                    return 0;
                 }
             }
         }

        
        bGoingUp = !bGoingFwdOrBwd && !bGoingDown;

        pFrm = p;
        p = lcl_GetLower( pFrm, true );

    } while( ( p && !p->IsFlowFrm() ) ||
             pFrm == this ||
             0 == ( pLayoutFrm = pFrm->IsLayoutFrm() ? (SwLayoutFrm*)pFrm : 0 ) ||
             pLayoutFrm->IsAnLower( this ) );

    return pLayoutFrm;
}

/**
 * Walk back inside the tree: grab the subordinate Frm if one exists and the
 * last step was not moving up a level (this would lead to an infinite up/down
 * loop!). With this we ensure that during walking back we search through all
 * sub trees. If we walked downwards we have to go to the end of the chain first
 * because we go backwards from the last Frm inside another Frm. Walking
 * forward works the same.
 *
 * @warning fixes here may also need to be applied to the @{lcl_NextFrm} method above
 */
const SwCntntFrm* SwCntntFrm::ImplGetNextCntntFrm( bool bFwd ) const
{
    const SwFrm *pFrm = this;
    
    SwCntntFrm *pCntntFrm = 0;
    bool bGoingUp = false;
    do {
        const SwFrm *p = 0;
        bool bGoingFwdOrBwd = false;

        bool bGoingDown = ( !bGoingUp && ( 0 != ( p = lcl_GetLower( pFrm, true ) ) ) );
        if ( !bGoingDown )
        {
            bGoingFwdOrBwd = ( 0 != ( p = lcl_FindLayoutFrame( pFrm, bFwd ) ) );
            if ( !bGoingFwdOrBwd )
            {
                bGoingUp = ( 0 != ( p = pFrm->GetUpper() ) );
                if ( !bGoingUp )
                {
                    return 0;
                }
            }
        }

        bGoingUp = !(bGoingFwdOrBwd || bGoingDown);

        if ( !bFwd )
        {
            if( bGoingDown && p )
                while ( p->GetNext() )
                    p = p->GetNext();
        }

        pFrm = p;
    } while ( 0 == (pCntntFrm = (pFrm->IsCntntFrm() ? (SwCntntFrm*)pFrm:0) ));

    return pCntntFrm;
}

SwPageFrm* SwFrm::FindPageFrm()
{
    SwFrm *pRet = this;
    while ( pRet && !pRet->IsPageFrm() )
    {
        if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrm() )
        {
            
            if ( static_cast<SwFlyFrm*>(pRet)->GetPageFrm() )
                pRet = static_cast<SwFlyFrm*>(pRet)->GetPageFrm();
            else
                pRet = static_cast<SwFlyFrm*>(pRet)->AnchorFrm();
        }
        else
            return 0;
    }
    return (SwPageFrm*)pRet;
}

SwFtnBossFrm* SwFrm::FindFtnBossFrm( sal_Bool bFootnotes )
{
    SwFrm *pRet = this;
    
    
    if( pRet->IsInTab() )
        pRet = pRet->FindTabFrm();
    while ( pRet && !pRet->IsFtnBossFrm() )
    {
        if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrm() )
        {
            
            if ( static_cast<SwFlyFrm*>(pRet)->GetPageFrm() )
                pRet = static_cast<SwFlyFrm*>(pRet)->GetPageFrm();
            else
                pRet = static_cast<SwFlyFrm*>(pRet)->AnchorFrm();
        }
        else
            return 0;
    }
    if( bFootnotes && pRet && pRet->IsColumnFrm() &&
        !pRet->GetNext() && !pRet->GetPrev() )
    {
        SwSectionFrm* pSct = pRet->FindSctFrm();
        OSL_ENSURE( pSct, "FindFtnBossFrm: Single column outside section?" );
        if( !pSct->IsFtnAtEnd() )
            return pSct->FindFtnBossFrm( sal_True );
    }
    return (SwFtnBossFrm*)pRet;
}

SwTabFrm* SwFrm::ImplFindTabFrm()
{
    SwFrm *pRet = this;
    while ( !pRet->IsTabFrm() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return 0;
    }
    return (SwTabFrm*)pRet;
}

SwSectionFrm* SwFrm::ImplFindSctFrm()
{
    SwFrm *pRet = this;
    while ( !pRet->IsSctFrm() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return 0;
    }
    return (SwSectionFrm*)pRet;
}

SwFtnFrm *SwFrm::ImplFindFtnFrm()
{
    SwFrm *pRet = this;
    while ( !pRet->IsFtnFrm() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return 0;
    }
    return (SwFtnFrm*)pRet;
}

SwFlyFrm *SwFrm::ImplFindFlyFrm()
{
    const SwFrm *pRet = this;
    do
    {
        if ( pRet->IsFlyFrm() )
            return (SwFlyFrm*)pRet;
        else
            pRet = pRet->GetUpper();
    } while ( pRet );
    return 0;
}

SwFrm *SwFrm::FindColFrm()
{
    SwFrm *pFrm = this;
    do
    {   pFrm = pFrm->GetUpper();
    } while ( pFrm && !pFrm->IsColumnFrm() );
    return pFrm;
}

SwRowFrm *SwFrm::FindRowFrm()
{
    SwFrm *pFrm = this;
    do
    {   pFrm = pFrm->GetUpper();
    } while ( pFrm && !pFrm->IsRowFrm() );
    return dynamic_cast< SwRowFrm* >( pFrm );
}

SwFrm* SwFrm::FindFooterOrHeader()
{
    SwFrm* pRet = this;
    do
    {   if ( pRet->GetType() & 0x0018 ) 
            return pRet;
        else if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrm() )
            pRet = ((SwFlyFrm*)pRet)->AnchorFrm();
        else
            return 0;
    } while ( pRet );
    return pRet;
}

const SwFtnFrm* SwFtnContFrm::FindFootNote() const
{
    const SwFtnFrm* pRet = (SwFtnFrm*)Lower();
    if( pRet && !pRet->GetAttr()->GetFtn().IsEndNote() )
        return pRet;
    return NULL;
}

const SwPageFrm* SwRootFrm::GetPageAtPos( const Point& rPt, const Size* pSize, bool bExtend ) const
{
    const SwPageFrm* pRet = 0;

    SwRect aRect;
    if ( pSize )
    {
        aRect.Pos()  = rPt;
        aRect.SSize() = *pSize;
    }

    const SwFrm* pPage = Lower();

    if ( !bExtend )
    {
        if( !Frm().IsInside( rPt ) )
            return 0;

        
        while( pPage && rPt.Y() > pPage->Frm().Bottom() )
            pPage = pPage->GetNext();
    }

    OSL_ENSURE( GetPageNum() <= maPageRects.size(), "number of pages differes from page rect array size" );
    sal_uInt16 nPageIdx = 0;

    while ( pPage && !pRet )
    {
        const SwRect& rBoundRect = bExtend ? maPageRects[ nPageIdx++ ] : pPage->Frm();

        if ( (!pSize && rBoundRect.IsInside(rPt)) ||
              (pSize && rBoundRect.IsOver(aRect)) )
        {
            pRet = static_cast<const SwPageFrm*>(pPage);
        }

        pPage = pPage->GetNext();
    }

    return pRet;
}

const SwAttrSet* SwFrm::GetAttrSet() const
{
    if ( IsCntntFrm() )
        return &((const SwCntntFrm*)this)->GetNode()->GetSwAttrSet();
    else
        return &((const SwLayoutFrm*)this)->GetFmt()->GetAttrSet();
}

/*************************************************************************
|*
|*  SwFrm::_FindNext(), _FindPrev(), InvalidateNextPos()
|*         _FindNextCnt() visits tables and sections and only returns SwCntntFrms.
|*
|*  Description         Invalidates the position of the next frame.
|*      This is the direct successor or in case of CntntFrms the next
|*      CntntFrm which sits in the same flow as I do:
|*      - body,
|*      - footnote,
|*      - in headers/footers the notification only needs to be forwarded
|*        inside the section
|*      - same for Flys
|*      - Cntnts in tabs remain only inside their cell
|*      - in principle tables behave exactly like the Cntnts
|*      - sections also
|*************************************************************************/



static SwFrm* lcl_NextFrm( SwFrm* pFrm )
{
    SwFrm *pRet = 0;
    bool bGoingUp = false;
    do {
        SwFrm *p = 0;

        bool bGoingFwd = false;
        bool bGoingDown = (!bGoingUp && ( 0 != (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)));

        if( !bGoingDown )
        {
            bGoingFwd = (0 != (p = ( pFrm->IsFlyFrm() ? ((SwFlyFrm*)pFrm)->GetNextLink() : pFrm->GetNext())));
            if ( !bGoingFwd )
            {
                bGoingUp = (0 != (p = pFrm->GetUpper()));
                if ( !bGoingUp )
                {
                    return 0;
                }
            }
        }
        bGoingUp = !(bGoingFwd || bGoingDown);
        pFrm = p;
    } while ( 0 == (pRet = ( ( pFrm->IsCntntFrm() || ( !bGoingUp &&
            ( pFrm->IsTabFrm() || pFrm->IsSctFrm() ) ) )? pFrm : 0 ) ) );
    return pRet;
}

SwFrm *SwFrm::_FindNext()
{
    bool bIgnoreTab = false;
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        
        
        
        if ( ((SwTabFrm*)this)->GetFollow() )
            return ((SwTabFrm*)this)->GetFollow();

        pThis = ((SwTabFrm*)this)->FindLastCntnt();
        if ( !pThis )
            pThis = this;
        bIgnoreTab = true;
    }
    else if ( IsSctFrm() )
    {
        
        if ( ((SwSectionFrm*)this)->GetFollow() )
            return ((SwSectionFrm*)this)->GetFollow();

        pThis = ((SwSectionFrm*)this)->FindLastCntnt();
        if ( !pThis )
            pThis = this;
    }
    else if ( IsCntntFrm() )
    {
        if( ((SwCntntFrm*)this)->GetFollow() )
            return ((SwCntntFrm*)this)->GetFollow();
    }
    else if ( IsRowFrm() )
    {
        SwFrm* pMyUpper = GetUpper();
        if ( pMyUpper->IsTabFrm() && ((SwTabFrm*)pMyUpper)->GetFollow() )
            return ((SwTabFrm*)pMyUpper)->GetFollow()->GetLower();
        else return NULL;
    }
    else
        return NULL;

    SwFrm* pRet = NULL;
    const sal_Bool bFtn  = pThis->IsInFtn();
    if ( !bIgnoreTab && pThis->IsInTab() )
    {
        SwLayoutFrm *pUp = pThis->GetUpper();
        while ( !pUp->IsCellFrm() )
            pUp = pUp->GetUpper();
        OSL_ENSURE( pUp, "Cntnt in Tabelle aber nicht in Zelle." );
        SwFrm* pNxt = ((SwCellFrm*)pUp)->GetFollowCell();
        if ( pNxt )
            pNxt = ((SwCellFrm*)pNxt)->ContainsCntnt();
        if ( !pNxt )
        {
            pNxt = lcl_NextFrm( pThis );
            if ( pUp->IsAnLower( pNxt ) )
                pRet = pNxt;
        }
        else
            pRet = pNxt;
    }
    else
    {
        const bool bBody = pThis->IsInDocBody();
        SwFrm *pNxtCnt = lcl_NextFrm( pThis );
        if ( pNxtCnt )
        {
            if ( bBody || bFtn )
            {
                while ( pNxtCnt )
                {
                    
                    
                    
                    bool bEndn = IsInSct() && !IsSctFrm() &&
                                 ( !pNxtCnt->IsInSct() ||
                                   !pNxtCnt->FindSctFrm()->IsEndnAtEnd()
                                 );
                    if ( ( bBody && pNxtCnt->IsInDocBody() ) ||
                         ( pNxtCnt->IsInFtn() &&
                           ( bFtn ||
                             ( bEndn && pNxtCnt->FindFtnFrm()->GetAttr()->GetFtn().IsEndNote() )
                           )
                         )
                       )
                    {
                        pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
                                                    : (SwFrm*)pNxtCnt;
                        break;
                    }
                    pNxtCnt = lcl_NextFrm( pNxtCnt );
                }
            }
            else if ( pThis->IsInFly() )
            {
                pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
                                            : (SwFrm*)pNxtCnt;
            }
            else    
            {
                const SwFrm *pUp = pThis->GetUpper();
                const SwFrm *pCntUp = pNxtCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() &&
                        !pCntUp->IsHeaderFrm() && !pCntUp->IsFooterFrm() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                {
                    pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
                                                : (SwFrm*)pNxtCnt;
                }
            }
        }
    }
    if( pRet && pRet->IsInSct() )
    {
        SwSectionFrm* pSct = pRet->FindSctFrm();
        
        
        if( !pSct->IsAnLower( this ) &&
            (!bFtn || pSct->IsInFtn() ) )
            return pSct;
    }
    return pRet;
}


SwCntntFrm *SwFrm::_FindNextCnt( const bool _bInSameFtn )
{
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        if ( ((SwTabFrm*)this)->GetFollow() )
        {
            pThis = ((SwTabFrm*)this)->GetFollow()->ContainsCntnt();
            if( pThis )
                return (SwCntntFrm*)pThis;
        }
        pThis = ((SwTabFrm*)this)->FindLastCntnt();
        if ( !pThis )
            return 0;
    }
    else if ( IsSctFrm() )
    {
        if ( ((SwSectionFrm*)this)->GetFollow() )
        {
            pThis = ((SwSectionFrm*)this)->GetFollow()->ContainsCntnt();
            if( pThis )
                return (SwCntntFrm*)pThis;
        }
        pThis = ((SwSectionFrm*)this)->FindLastCntnt();
        if ( !pThis )
            return 0;
    }
    else if ( IsCntntFrm() && ((SwCntntFrm*)this)->GetFollow() )
        return ((SwCntntFrm*)this)->GetFollow();

    if ( pThis->IsCntntFrm() )
    {
        const bool bBody = pThis->IsInDocBody();
        const sal_Bool bFtn  = pThis->IsInFtn();
        SwCntntFrm *pNxtCnt = ((SwCntntFrm*)pThis)->GetNextCntntFrm();
        if ( pNxtCnt )
        {
            
            if ( bBody || ( bFtn && !_bInSameFtn ) )
            {
                
                while ( pNxtCnt )
                {
                    if ( (bBody && pNxtCnt->IsInDocBody()) ||
                         (bFtn  && pNxtCnt->IsInFtn()) )
                        return pNxtCnt;
                    pNxtCnt = pNxtCnt->GetNextCntntFrm();
                }
            }
            
            else if ( bFtn && _bInSameFtn )
            {
                
                
                const SwFtnFrm* pFtnFrmOfNext( pNxtCnt->FindFtnFrm() );
                const SwFtnFrm* pFtnFrmOfCurr( pThis->FindFtnFrm() );
                OSL_ENSURE( pFtnFrmOfCurr,
                        "<SwFrm::_FindNextCnt() - unknown layout situation: current frame has to have an upper footnote frame." );
                if ( pFtnFrmOfNext == pFtnFrmOfCurr )
                {
                    return pNxtCnt;
                }
                else if ( pFtnFrmOfCurr->GetFollow() )
                {
                    
                    
                    SwFtnFrm* pFollowFtnFrmOfCurr(
                                        const_cast<SwFtnFrm*>(pFtnFrmOfCurr) );
                    pNxtCnt = 0L;
                    do {
                        pFollowFtnFrmOfCurr = pFollowFtnFrmOfCurr->GetFollow();
                        pNxtCnt = pFollowFtnFrmOfCurr->ContainsCntnt();
                    } while ( !pNxtCnt && pFollowFtnFrmOfCurr->GetFollow() );
                    return pNxtCnt;
                }
                else
                {
                    
                    
                    return 0L;
                }
            }
            else if ( pThis->IsInFly() )
                
                
                return pNxtCnt;
            else
            {
                
                const SwFrm *pUp = pThis->GetUpper();
                const SwFrm *pCntUp = pNxtCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() &&
                        !pCntUp->IsHeaderFrm() && !pCntUp->IsFooterFrm() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                    return pNxtCnt;
            }
        }
    }
    return 0;
}

/** method to determine previous content frame in the same environment
    for a flow frame (content frame, table frame, section frame)

    OD 2005-11-30 #i27138#
*/
SwCntntFrm* SwFrm::_FindPrevCnt( const bool _bInSameFtn )
{
    if ( !IsFlowFrm() )
    {
        
        return 0L;
    }

    SwCntntFrm* pPrevCntntFrm( 0L );

    
    
    SwCntntFrm* pCurrCntntFrm = dynamic_cast<SwCntntFrm*>(this);

    
    
    if ( pCurrCntntFrm && pCurrCntntFrm->IsFollow() )
    {
        
        pPrevCntntFrm = pCurrCntntFrm->FindMaster();
    }
    else if ( IsTabFrm() )
    {
        SwTabFrm* pTabFrm( static_cast<SwTabFrm*>(this) );
        if ( pTabFrm->IsFollow() )
        {
            
            pPrevCntntFrm = pTabFrm->FindMaster()->FindLastCntnt();
        }
        else
        {
            
            
            pCurrCntntFrm = pTabFrm->ContainsCntnt();
        }
    }
    else if ( IsSctFrm() )
    {
        SwSectionFrm* pSectFrm( static_cast<SwSectionFrm*>(this) );
        if ( pSectFrm->IsFollow() )
        {
            
            pPrevCntntFrm = pSectFrm->FindMaster()->FindLastCntnt();
        }
        else
        {
            
            
            pCurrCntntFrm = pSectFrm->ContainsCntnt();
        }
    }

    
    
    if ( !pPrevCntntFrm && pCurrCntntFrm )
    {
        pPrevCntntFrm = pCurrCntntFrm->GetPrevCntntFrm();
        if ( pPrevCntntFrm )
        {
            if ( pCurrCntntFrm->IsInFly() )
            {
                
                
                
            }
            else
            {
                const bool bInDocBody = pCurrCntntFrm->IsInDocBody();
                const bool bInFtn  = pCurrCntntFrm->IsInFtn();
                if ( bInDocBody || ( bInFtn && !_bInSameFtn ) )
                {
                    
                    
                    
                    while ( pPrevCntntFrm )
                    {
                        if ( ( bInDocBody && pPrevCntntFrm->IsInDocBody() ) ||
                             ( bInFtn && pPrevCntntFrm->IsInFtn() ) )
                        {
                            break;
                        }
                        pPrevCntntFrm = pPrevCntntFrm->GetPrevCntntFrm();
                    }
                }
                else if ( bInFtn && _bInSameFtn )
                {
                    
                    
                    const SwFtnFrm* pFtnFrmOfPrev( pPrevCntntFrm->FindFtnFrm() );
                    const SwFtnFrm* pFtnFrmOfCurr( pCurrCntntFrm->FindFtnFrm() );
                    if ( pFtnFrmOfPrev != pFtnFrmOfCurr )
                    {
                        if ( pFtnFrmOfCurr->GetMaster() )
                        {
                            SwFtnFrm* pMasterFtnFrmOfCurr(
                                        const_cast<SwFtnFrm*>(pFtnFrmOfCurr) );
                            pPrevCntntFrm = 0L;
                            
                            
                            do {
                                pMasterFtnFrmOfCurr = pMasterFtnFrmOfCurr->GetMaster();
                                pPrevCntntFrm = pMasterFtnFrmOfCurr->FindLastCntnt();
                            } while ( !pPrevCntntFrm &&
                                      pMasterFtnFrmOfCurr->GetMaster() );
                        }
                        else
                        {
                            
                            
                            pPrevCntntFrm = 0L;;
                        }
                    }
                }
                else
                {
                    
                    
                    
                    
                    
                    
                    
                    
                    OSL_ENSURE( pCurrCntntFrm->FindFooterOrHeader(),
                            "<SwFrm::_FindPrevCnt()> - unknown layout situation: current frame should be in page header or page footer" );
                    OSL_ENSURE( !pPrevCntntFrm->IsInFly(),
                            "<SwFrm::_FindPrevCnt()> - unknown layout situation: found previous frame should *not* be inside a fly frame." );
                    if ( pPrevCntntFrm->FindFooterOrHeader() !=
                                            pCurrCntntFrm->FindFooterOrHeader() )
                    {
                        pPrevCntntFrm = 0L;
                    }
                }
            }
        }
    }

    return pPrevCntntFrm;
}

SwFrm *SwFrm::_FindPrev()
{
    bool bIgnoreTab = false;
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        
        
        
        if ( ((SwTabFrm*)this)->IsFollow() )
            return ((SwTabFrm*)this)->FindMaster();
        else
            pThis = ((SwTabFrm*)this)->ContainsCntnt();
        bIgnoreTab = true;
    }

    if ( pThis && pThis->IsCntntFrm() )
    {
        SwCntntFrm *pPrvCnt = ((SwCntntFrm*)pThis)->GetPrevCntntFrm();
        if( !pPrvCnt )
            return 0;
        if ( !bIgnoreTab && pThis->IsInTab() )
        {
            SwLayoutFrm *pUp = pThis->GetUpper();
            while ( !pUp->IsCellFrm() )
                pUp = pUp->GetUpper();
            OSL_ENSURE( pUp, "Cntnt in table but not in cell." );
            if ( pUp->IsAnLower( pPrvCnt ) )
                return pPrvCnt;
        }
        else
        {
            SwFrm* pRet;
            const bool bBody = pThis->IsInDocBody();
            const sal_Bool bFtn  = bBody ? sal_False : pThis->IsInFtn();
            if ( bBody || bFtn )
            {
                while ( pPrvCnt )
                {
                    if ( (bBody && pPrvCnt->IsInDocBody()) ||
                            (bFtn   && pPrvCnt->IsInFtn()) )
                    {
                        pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
                                                  : (SwFrm*)pPrvCnt;
                        return pRet;
                    }
                    pPrvCnt = pPrvCnt->GetPrevCntntFrm();
                }
            }
            else if ( pThis->IsInFly() )
            {
                pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
                                            : (SwFrm*)pPrvCnt;
                return pRet;
            }
            else 
            {
                const SwFrm *pUp = pThis->GetUpper();
                const SwFrm *pCntUp = pPrvCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                {
                    pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
                                                : (SwFrm*)pPrvCnt;
                    return pRet;
                }
            }
        }
    }
    return 0;
}

void SwFrm::ImplInvalidateNextPos( sal_Bool bNoFtn )
{
    SwFrm *pFrm;
    if ( 0 != (pFrm = _FindNext()) )
    {
        if( pFrm->IsSctFrm() )
        {
            while( pFrm && pFrm->IsSctFrm() )
            {
                if( ((SwSectionFrm*)pFrm)->GetSection() )
                {
                    SwFrm* pTmp = ((SwSectionFrm*)pFrm)->ContainsAny();
                    if( pTmp )
                        pTmp->InvalidatePos();
                    else if( !bNoFtn )
                        ((SwSectionFrm*)pFrm)->InvalidateFtnPos();
                    if( !IsInSct() || FindSctFrm()->GetFollow() != pFrm )
                        pFrm->InvalidatePos();
                    return;
                }
                pFrm = pFrm->FindNext();
            }
            if( pFrm )
            {
                if ( pFrm->IsSctFrm())
                {
                    
                    
                    SwFrm* pTmp = ((SwSectionFrm*)pFrm)->ContainsAny();
                    if( pTmp )
                        pTmp->InvalidatePos();
                    if( !IsInSct() || FindSctFrm()->GetFollow() != pFrm )
                        pFrm->InvalidatePos();
                }
                else
                    pFrm->InvalidatePos();
            }
        }
        else
            pFrm->InvalidatePos();
    }
}

/** method to invalidate printing area of next frame

    OD 09.01.2004 #i11859#

    FME 2004-04-19 #i27145# Moved function from SwTxtFrm to SwFrm
*/
void SwFrm::InvalidateNextPrtArea()
{
    
    SwFrm* pNextFrm = FindNext();
    
    {
        while ( pNextFrm &&
                ( ( pNextFrm->IsSctFrm() &&
                    !static_cast<SwSectionFrm*>(pNextFrm)->GetSection() ) ||
                  ( pNextFrm->IsTxtFrm() &&
                    static_cast<SwTxtFrm*>(pNextFrm)->IsHiddenNow() ) ) )
        {
            pNextFrm = pNextFrm->FindNext();
        }
    }

    
    if ( pNextFrm )
    {
        if ( pNextFrm->IsSctFrm() )
        {
            
            
            
            
            if ( !IsInSct() || FindSctFrm()->GetFollow() != pNextFrm )
            {
                pNextFrm->InvalidatePrt();
            }

            
            SwFrm* pFstCntntOfSctFrm =
                    static_cast<SwSectionFrm*>(pNextFrm)->ContainsAny();
            if ( pFstCntntOfSctFrm )
            {
                pFstCntntOfSctFrm->InvalidatePrt();
            }
        }
        else
        {
            pNextFrm->InvalidatePrt();
        }
    }
}



static bool lcl_IsInColSct( const SwFrm *pUp )
{
    bool bRet = false;
    while( pUp )
    {
        if( pUp->IsColumnFrm() )
            bRet = true;
        else if( pUp->IsSctFrm() )
            return bRet;
        else if( pUp->IsTabFrm() )
            return false;
        pUp = pUp->GetUpper();
    }
    return false;
}

/** determine, if frame is moveable in given environment

    OD 08.08.2003 #110978#
    method replaced 'old' method <sal_Bool IsMoveable() const>.
    Determines, if frame is moveable in given environment. if no environment
    is given (parameter _pLayoutFrm == 0L), the movability in the actual
    environment (<this->GetUpper()) is checked.
*/
bool SwFrm::IsMoveable( const SwLayoutFrm* _pLayoutFrm ) const
{
    bool bRetVal = false;

    if ( !_pLayoutFrm )
    {
        _pLayoutFrm = GetUpper();
    }

    if ( _pLayoutFrm && IsFlowFrm() )
    {
        if ( _pLayoutFrm->IsInSct() && lcl_IsInColSct( _pLayoutFrm ) )
        {
            bRetVal = true;
        }
        else if ( _pLayoutFrm->IsInFly() ||
                  _pLayoutFrm->IsInDocBody() ||
                  _pLayoutFrm->IsInFtn() )
        {
            if ( _pLayoutFrm->IsInTab() && !IsTabFrm() &&
                 ( !IsCntntFrm() || !const_cast<SwFrm*>(this)->GetNextCellLeaf( MAKEPAGE_NONE ) ) )
            {
                bRetVal = false;
            }
            else
            {
                if ( _pLayoutFrm->IsInFly() )
                {
                    
                    
                    if ( const_cast<SwLayoutFrm*>(_pLayoutFrm)->FindFlyFrm()->GetNextLink() )
                    {
                        bRetVal = true;
                    }
                    else
                    {
                        
                        
                        
                        const SwFrm* pCol = _pLayoutFrm;
                        while ( pCol && !pCol->IsColumnFrm() )
                        {
                            pCol = pCol->GetUpper();
                        }
                        
                        if ( pCol && pCol->GetNext() )
                        {
                            bRetVal = true;
                        }
                    }
                }
                else
                {
                    bRetVal = true;
                }
            }
        }
    }

    return bRetVal;
}

void SwFrm::SetInfFlags()
{
    if ( !IsFlyFrm() && !GetUpper() ) 
        return;

    mbInfInvalid = mbInfBody = mbInfTab = mbInfFly = mbInfFtn = mbInfSct = sal_False;

    SwFrm *pFrm = this;
    if( IsFtnContFrm() )
        mbInfFtn = sal_True;
    do
    {
        
        if ( pFrm->IsBodyFrm() && !mbInfFtn && pFrm->GetUpper()
             && pFrm->GetUpper()->IsPageFrm() )
            mbInfBody = sal_True;
        else if ( pFrm->IsTabFrm() || pFrm->IsCellFrm() )
        {
            mbInfTab = sal_True;
        }
        else if ( pFrm->IsFlyFrm() )
            mbInfFly = sal_True;
        else if ( pFrm->IsSctFrm() )
            mbInfSct = sal_True;
        else if ( pFrm->IsFtnFrm() )
            mbInfFtn = sal_True;

        pFrm = pFrm->GetUpper();

    } while ( pFrm && !pFrm->IsPageFrm() ); 
}

/** Updates the vertical or the righttoleft-flags.
 *
 * If the property is derived, it's from the upper or (for fly frames) from
 * the anchor. Otherwise we've to call a virtual method to check the property.
 */
void SwFrm::SetDirFlags( sal_Bool bVert )
{
    if( bVert )
    {
        
        
        if( mbDerivedVert )
        {
            const SwFrm* pAsk = IsFlyFrm() ?
                          ((SwFlyFrm*)this)->GetAnchorFrm() : GetUpper();

            OSL_ENSURE( pAsk != this, "Autsch! Stack overflow is about to happen" );

            if( pAsk )
            {
                mbVertical = pAsk->IsVertical() ? 1 : 0;
                mbReverse  = pAsk->IsReverse()  ? 1 : 0;

                mbVertLR  = pAsk->IsVertLR() ? 1 : 0;
                
                if ( !pAsk->mbInvalidVert )
                    mbInvalidVert = sal_False;
            }
        }
        else
            CheckDirection( bVert );
    }
    else
    {
        sal_Bool bInv = 0;
        if( !mbDerivedR2L ) 
            CheckDirection( bVert );
        if( mbDerivedR2L )
        {
            const SwFrm* pAsk = IsFlyFrm() ?
                          ((SwFlyFrm*)this)->GetAnchorFrm() : GetUpper();

            OSL_ENSURE( pAsk != this, "Oops! Stack overflow is about to happen" );

            if( pAsk )
                mbRightToLeft = pAsk->IsRightToLeft() ? 1 : 0;
            if( !pAsk || pAsk->mbInvalidR2L )
                bInv = mbInvalidR2L;
        }
        mbInvalidR2L = bInv;
    }
}

SwLayoutFrm* SwFrm::GetNextCellLeaf( MakePageType )
{
    SwFrm* pTmpFrm = this;
    while ( !pTmpFrm->IsCellFrm() )
        pTmpFrm = pTmpFrm->GetUpper();

    OSL_ENSURE( pTmpFrm, "SwFrm::GetNextCellLeaf() without cell" );
    return ((SwCellFrm*)pTmpFrm)->GetFollowCell();
}

SwLayoutFrm* SwFrm::GetPrevCellLeaf( MakePageType )
{
    SwFrm* pTmpFrm = this;
    while ( !pTmpFrm->IsCellFrm() )
        pTmpFrm = pTmpFrm->GetUpper();

    OSL_ENSURE( pTmpFrm, "SwFrm::GetNextPreviousLeaf() without cell" );
    return ((SwCellFrm*)pTmpFrm)->GetPreviousCell();
}

static SwCellFrm* lcl_FindCorrespondingCellFrm( const SwRowFrm& rOrigRow,
                                         const SwCellFrm& rOrigCell,
                                         const SwRowFrm& rCorrRow,
                                         bool bInFollow )
{
    SwCellFrm* pRet = NULL;
    SwCellFrm* pCell = (SwCellFrm*)rOrigRow.Lower();
    SwCellFrm* pCorrCell = (SwCellFrm*)rCorrRow.Lower();

    while ( pCell != &rOrigCell && !pCell->IsAnLower( &rOrigCell ) )
    {
        pCell = (SwCellFrm*)pCell->GetNext();
        pCorrCell = (SwCellFrm*)pCorrCell->GetNext();
    }

    OSL_ENSURE( pCell && pCorrCell, "lcl_FindCorrespondingCellFrm does not work" );

    if ( pCell != &rOrigCell )
    {
        
        OSL_ENSURE( pCell->Lower() && pCell->Lower()->IsRowFrm(),
                "lcl_FindCorrespondingCellFrm does not work" );

        SwRowFrm* pRow = (SwRowFrm*)pCell->Lower();
        while ( !pRow->IsAnLower( &rOrigCell ) )
            pRow = (SwRowFrm*)pRow->GetNext();

        SwRowFrm* pCorrRow = 0;
        if ( bInFollow )
            pCorrRow = pRow->GetFollowRow();
        else
        {
            SwRowFrm* pTmpRow = static_cast<SwRowFrm*>(pCorrCell->GetLastLower());

            if ( pTmpRow && pTmpRow->GetFollowRow() == pRow )
                pCorrRow = pTmpRow;
        }

        if ( pCorrRow )
            pRet = lcl_FindCorrespondingCellFrm( *pRow, rOrigCell, *pCorrRow, bInFollow );
    }
    else
        pRet = pCorrCell;

    return pRet;
}


SwCellFrm* SwCellFrm::GetFollowCell() const
{
    SwCellFrm* pRet = NULL;

    
    
    const long nRowSpan = GetLayoutRowSpan();
    if ( nRowSpan < 1 )
        return NULL;

    
    const SwFrm* pRow = GetUpper();
    while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
        pRow = pRow->GetUpper();

    if ( !pRow )
        return NULL;

    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( pRow->GetUpper() );
    if ( !pRow || !pTabFrm->GetFollow() || !pTabFrm->HasFollowFlowLine() )
        return NULL;

    const SwCellFrm* pThisCell = this;

    
    if ( nRowSpan > 1 )
    {
        
        long nMax = 0;
        while ( pRow->GetNext() && ++nMax < nRowSpan )
            pRow = pRow->GetNext();

        if ( !pRow->GetNext() )
        {
            pThisCell = &pThisCell->FindStartEndOfRowSpanCell( false, true );
            pRow = pThisCell->GetUpper();
        }
    }

    const SwRowFrm* pFollowRow = NULL;
    if ( !pRow->GetNext() &&
         NULL != ( pFollowRow = pRow->IsInSplitTableRow() ) &&
         ( !pFollowRow->IsRowSpanLine() || nRowSpan > 1 ) )
         pRet = lcl_FindCorrespondingCellFrm( *((SwRowFrm*)pRow), *pThisCell, *pFollowRow, true );

    return pRet;
}


SwCellFrm* SwCellFrm::GetPreviousCell() const
{
    SwCellFrm* pRet = NULL;

    
    
    if ( GetLayoutRowSpan() < 1 )
        return NULL;

    
    const SwFrm* pRow = GetUpper();
    while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
        pRow = pRow->GetUpper();

    OSL_ENSURE( pRow->GetUpper() && pRow->GetUpper()->IsTabFrm(), "GetPreviousCell without Table" );

    SwTabFrm* pTab = (SwTabFrm*)pRow->GetUpper();

    if ( pTab->IsFollow() )
    {
        const SwFrm* pTmp = pTab->GetFirstNonHeadlineRow();
        const bool bIsInFirstLine = ( pTmp == pRow );

        if ( bIsInFirstLine )
        {
            SwTabFrm *pMaster = (SwTabFrm*)pTab->FindMaster();
            if ( pMaster && pMaster->HasFollowFlowLine() )
            {
                SwRowFrm* pMasterRow = static_cast<SwRowFrm*>(pMaster->GetLastLower());
                if ( pMasterRow )
                    pRet = lcl_FindCorrespondingCellFrm( *((SwRowFrm*)pRow), *this, *pMasterRow, false );
                if ( pRet && pRet->GetTabBox()->getRowSpan() < 1 )
                    pRet = &const_cast<SwCellFrm&>(pRet->FindStartEndOfRowSpanCell( true, true ));
            }
        }
    }

    return pRet;
}


const SwCellFrm& SwCellFrm::FindStartEndOfRowSpanCell( bool bStart, bool bCurrentTableOnly ) const
{
    const SwCellFrm* pRet = 0;

    const SwTabFrm* pTableFrm = dynamic_cast<const SwTabFrm*>(GetUpper()->GetUpper());

    if ( !bStart && pTableFrm->IsFollow() && pTableFrm->IsInHeadline( *this ) )
        return *this;

    OSL_ENSURE( pTableFrm &&
            (  (bStart && GetTabBox()->getRowSpan() < 1) ||
              (!bStart && GetLayoutRowSpan() > 1) ),
            "SwCellFrm::FindStartRowSpanCell: No rowspan, no table, no cookies" );

    if ( pTableFrm )
    {
        const SwTable* pTable = pTableFrm->GetTable();

        sal_uInt16 nMax = USHRT_MAX;
        if ( bCurrentTableOnly )
        {
            const SwFrm* pCurrentRow = GetUpper();
            const bool bDoNotEnterHeadline = bStart && pTableFrm->IsFollow() &&
                                        !pTableFrm->IsInHeadline( *pCurrentRow );

            
            
            nMax = 0;
            while ( bStart ? pCurrentRow->GetPrev() : pCurrentRow->GetNext() )
            {
                if ( bStart )
                {
                    
                    if ( bDoNotEnterHeadline && pTableFrm->IsFollow() &&
                         pTableFrm->IsInHeadline( *pCurrentRow->GetPrev() ) )
                        break;

                    pCurrentRow = pCurrentRow->GetPrev();
                }
                else
                    pCurrentRow = pCurrentRow->GetNext();

                ++nMax;
            }
        }

        
        
        
        const SwTableBox& rMasterBox = bStart ?
                                       GetTabBox()->FindStartOfRowSpan( *pTable, nMax ) :
                                       GetTabBox()->FindEndOfRowSpan( *pTable, nMax );

        SwIterator<SwCellFrm,SwFmt> aIter( *rMasterBox.GetFrmFmt() );

        for ( SwCellFrm* pMasterCell = aIter.First(); pMasterCell; pMasterCell = aIter.Next() )
        {
            if ( pMasterCell->GetTabBox() == &rMasterBox )
            {
                const SwTabFrm* pMasterTable = static_cast<const SwTabFrm*>(pMasterCell->GetUpper()->GetUpper());

                if ( bCurrentTableOnly )
                {
                    if ( pMasterTable == pTableFrm )
                    {
                        pRet = pMasterCell;
                        break;
                    }
                }
                else
                {
                    if ( pMasterTable == pTableFrm ||
                         (  (bStart && pMasterTable->IsAnFollow(pTableFrm)) ||
                           (!bStart && pTableFrm->IsAnFollow(pMasterTable)) ) )
                    {
                        pRet = pMasterCell;
                        break;
                    }
                }
            }
        }
    }

    OSL_ENSURE( pRet, "SwCellFrm::FindStartRowSpanCell: No result" );

    return *pRet;
}


const SwRowFrm* SwFrm::IsInSplitTableRow() const
{
    OSL_ENSURE( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" );

    const SwFrm* pRow = this;

    
    while( pRow && ( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return NULL;

    OSL_ENSURE( pRow->GetUpper()->IsTabFrm(), "Confusion in table layout" );

    const SwTabFrm* pTab = (SwTabFrm*)pRow->GetUpper();
    //
    
    
    if ( pRow->GetNext() ||
         pTab->GetTable()->IsHeadline(
                    *(static_cast<const SwRowFrm*>(pRow)->GetTabLine()) ) ||
         !pTab->HasFollowFlowLine() ||
         !pTab->GetFollow() )
        return NULL;

    
    const SwRowFrm* pFollowRow = pTab->GetFollow()->GetFirstNonHeadlineRow();

    OSL_ENSURE( pFollowRow, "SwFrm::IsInSplitTableRow() does not work" );

    return pFollowRow;
}

const SwRowFrm* SwFrm::IsInFollowFlowRow() const
{
    OSL_ENSURE( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" );

    
    const SwFrm* pRow = this;
    while( pRow && ( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return NULL;

    OSL_ENSURE( pRow->GetUpper()->IsTabFrm(), "Confusion in table layout" );

    const SwTabFrm* pTab = (SwTabFrm*)pRow->GetUpper();

    const SwTabFrm* pMaster = pTab->IsFollow() ? pTab->FindMaster() : 0;

    if ( !pMaster || !pMaster->HasFollowFlowLine() )
        return NULL;

    const SwFrm* pTmp = pTab->GetFirstNonHeadlineRow();
    const bool bIsInFirstLine = ( pTmp == pRow );

    if ( !bIsInFirstLine )
        return NULL;

    const SwRowFrm* pMasterRow = static_cast<const SwRowFrm*>(pMaster->GetLastLower());
    return pMasterRow;
}

bool SwFrm::IsInBalancedSection() const
{
    bool bRet = false;

    if ( IsInSct() )
    {
        const SwSectionFrm* pSectionFrm = FindSctFrm();
        if ( pSectionFrm )
            bRet = pSectionFrm->IsBalancedSection();
    }
    return bRet;
}

const SwFrm* SwLayoutFrm::GetLastLower() const
{
    const SwFrm* pRet = Lower();
    if ( !pRet )
        return 0;
    while ( pRet->GetNext() )
        pRet = pRet->GetNext();
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
