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
#include <calbck.hxx>

/// Searches the first ContentFrm in BodyText below the page.
SwLayoutFrm *SwFootnoteBossFrm::FindBodyCont()
{
    SwFrm *pLay = Lower();
    while ( pLay && !pLay->IsBodyFrm() )
        pLay = pLay->GetNext();
    return static_cast<SwLayoutFrm*>(pLay);
}

/// Searches the last ContentFrm in BodyText below the page.
SwContentFrm *SwPageFrm::FindLastBodyContent()
{
    SwContentFrm *pRet = FindFirstBodyContent();
    SwContentFrm *pNxt = pRet;
    while ( pNxt && pNxt->IsInDocBody() && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNextCnt();
    }
    return pRet;
}

/**
 * Checks if the frame contains one or more ContentFrm's anywhere in his
 * subsidiary structure; if so the first found ContentFrm is returned.
 */
const SwContentFrm *SwLayoutFrm::ContainsContent() const
{
    //Search downwards the layout leaf and if there is no content, jump to the
    //next leaf until content is found or we leave "this".
    //Sections: Content next to sections would not be found this way (empty
    //sections directly next to ContentFrm) therefore we need to recursively
    //search for them even if it's more complex.

    const SwLayoutFrm *pLayLeaf = this;
    do
    {
        while ( (!pLayLeaf->IsSctFrm() || pLayLeaf == this ) &&
                pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrm() )
            pLayLeaf = static_cast<const SwLayoutFrm*>(pLayLeaf->Lower());

        if( pLayLeaf->IsSctFrm() && pLayLeaf != this )
        {
            const SwContentFrm *pCnt = pLayLeaf->ContainsContent();
            if( pCnt )
                return pCnt;
            if( pLayLeaf->GetNext() )
            {
                if( pLayLeaf->GetNext()->IsLayoutFrm() )
                {
                    pLayLeaf = static_cast<const SwLayoutFrm*>(pLayLeaf->GetNext());
                    continue;
                }
                else
                    return static_cast<const SwContentFrm*>(pLayLeaf->GetNext());
            }
        }
        else if ( pLayLeaf->Lower() )
            return static_cast<const SwContentFrm*>(pLayLeaf->Lower());

        pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        if( !IsAnLower( pLayLeaf) )
            return 0;
    } while( pLayLeaf );
    return 0;
}

/**
 * Calls ContainsAny first to reach the innermost cell. From there we walk back
 * up to the first SwCellFrm. Since we use SectionFrms, ContainsContent()->GetUpper()
 * is not enough anymore.
 */
const SwCellFrm *SwLayoutFrm::FirstCell() const
{
    const SwFrm* pCnt = ContainsAny();
    while( pCnt && !pCnt->IsCellFrm() )
        pCnt = pCnt->GetUpper();
    return static_cast<const SwCellFrm*>(pCnt);
}

/** return ContentFrms, sections, and tables.
 *
 * @param _bInvestigateFootnoteForSections controls investigation of content of footnotes for sections.
 * @see ContainsContent
 */
const SwFrm *SwLayoutFrm::ContainsAny( const bool _bInvestigateFootnoteForSections ) const
{
    //Search downwards the layout leaf and if there is no content, jump to the
    //next leaf until content is found, we leave "this" or until we found
    //a SectionFrm or a TabFrm.

    const SwLayoutFrm *pLayLeaf = this;
    // #130797#
    const bool bNoFootnote = IsSctFrm() && !_bInvestigateFootnoteForSections;
    do
    {
        while ( ( (!pLayLeaf->IsSctFrm() && !pLayLeaf->IsTabFrm())
                 || pLayLeaf == this ) &&
                pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrm() )
            pLayLeaf = static_cast<const SwLayoutFrm*>(pLayLeaf->Lower());

        if( ( pLayLeaf->IsTabFrm() || pLayLeaf->IsSctFrm() )
            && pLayLeaf != this )
        {
            // Now we also return "deleted" SectionFrms so they can be
            // maintained on SaveContent and RestoreContent
            return pLayLeaf;
        }
        else if ( pLayLeaf->Lower() )
            return static_cast<const SwContentFrm*>(pLayLeaf->Lower());

        pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        if( bNoFootnote && pLayLeaf && pLayLeaf->IsInFootnote() )
        {
            do
            {
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
            } while( pLayLeaf && pLayLeaf->IsInFootnote() );
        }
        if( !IsAnLower( pLayLeaf) )
            return 0;
    } while( pLayLeaf );
    return 0;
}

const SwFrm* SwFrm::GetLower() const
{
    return IsLayoutFrm() ? static_cast<const SwLayoutFrm*>(this)->Lower() : 0;
}

SwFrm* SwFrm::GetLower()
{
    return IsLayoutFrm() ? static_cast<SwLayoutFrm*>(this)->Lower() : 0;
}

SwContentFrm* SwFrm::FindPrevCnt( const bool _bInSameFootnote )
{
    if ( GetPrev() && GetPrev()->IsContentFrm() )
        return static_cast<SwContentFrm*>(GetPrev());
    else
        return _FindPrevCnt( _bInSameFootnote );
}

const SwContentFrm* SwFrm::FindPrevCnt( const bool _bInSameFootnote ) const
{
    if ( GetPrev() && GetPrev()->IsContentFrm() )
        return static_cast<const SwContentFrm*>(GetPrev());
    else
        return const_cast<SwFrm*>(this)->_FindPrevCnt( _bInSameFootnote );
}

SwContentFrm *SwFrm::FindNextCnt( const bool _bInSameFootnote )
{
    if ( mpNext && mpNext->IsContentFrm() )
        return static_cast<SwContentFrm*>(mpNext);
    else
        return _FindNextCnt( _bInSameFootnote );
}

const SwContentFrm *SwFrm::FindNextCnt( const bool _bInSameFootnote ) const
{
    if ( mpNext && mpNext->IsContentFrm() )
        return static_cast<SwContentFrm*>(mpNext);
    else
        return const_cast<SwFrm*>(this)->_FindNextCnt( _bInSameFootnote );
}

bool SwLayoutFrm::IsAnLower( const SwFrm *pAssumed ) const
{
    const SwFrm *pUp = pAssumed;
    while ( pUp )
    {
        if ( pUp == this )
            return true;
        if ( pUp->IsFlyFrm() )
            pUp = static_cast<const SwFlyFrm*>(pUp)->GetAnchorFrm();
        else
            pUp = pUp->GetUpper();
    }
    return false;
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

    // check, if on different pages
    const SwPageFrm *pMyPage = FindPageFrm();
    const SwPageFrm *pCheckRefPage = _pCheckRefLayFrm->FindPageFrm();
    if( pMyPage != pCheckRefPage )
    {
        // being on different page as check reference
        bReturn = pMyPage->GetPhyPageNum() < pCheckRefPage->GetPhyPageNum();
    }
    else
    {
        // being on same page as check reference
        // --> search my supreme parent <pUp>, which doesn't contain check reference.
        const SwLayoutFrm* pUp = this;
        while ( pUp->GetUpper() &&
                !pUp->GetUpper()->IsAnLower( _pCheckRefLayFrm )
              )
            pUp = pUp->GetUpper();
        if( !pUp->GetUpper() )
        {
            // can occur, if <this> is a fly frm
            bReturn = false;
        }
        else
        {
            // travel through the next's of <pUp> and check if one of these
            // contain the check reference.
            const SwLayoutFrm* pUpNext = static_cast<const SwLayoutFrm*>(pUp->GetNext());
            while ( pUpNext &&
                    !pUpNext->IsAnLower( _pCheckRefLayFrm ) )
            {
                pUpNext = static_cast<const SwLayoutFrm*>(pUpNext->GetNext());
            }
            bReturn = pUpNext != 0;
        }
    }

    return bReturn;
}

// Local helper functions for GetNextLayoutLeaf

static const SwFrm* lcl_FindLayoutFrame( const SwFrm* pFrm, bool bNext )
{
    const SwFrm* pRet = 0;
    if ( pFrm->IsFlyFrm() )
        pRet = bNext ? static_cast<const SwFlyFrm*>(pFrm)->GetNextLink() : static_cast<const SwFlyFrm*>(pFrm)->GetPrevLink();
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
    bool bGoingUp = !bFwd;          // false for forward, true for backward
    do {

         bool bGoingFwdOrBwd = false;

         bool bGoingDown = ( !bGoingUp && ( 0 != (p = lcl_GetLower( pFrm, bFwd ) ) ) );
         if ( !bGoingDown )
         {
             // I cannot go down, because either I'm currently going up or
             // because the is no lower.
             // I'll try to go forward:
             bGoingFwdOrBwd = (0 != (p = lcl_FindLayoutFrame( pFrm, bFwd ) ) );
             if ( !bGoingFwdOrBwd )
             {
                 // I cannot go forward, because there is no next frame.
                 // I'll try to go up:
                 bGoingUp = (0 != (p = pFrm->GetUpper() ) );
                 if ( !bGoingUp )
                 {
                    // I cannot go up, because there is no upper frame.
                    return 0;
                 }
             }
         }

        // If I could not go down or forward, I'll have to go up
        bGoingUp = !bGoingFwdOrBwd && !bGoingDown;

        pFrm = p;
        p = lcl_GetLower( pFrm, true );

    } while( ( p && !p->IsFlowFrm() ) ||
             pFrm == this ||
             0 == ( pLayoutFrm = pFrm->IsLayoutFrm() ? static_cast<const SwLayoutFrm*>(pFrm) : 0 ) ||
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
const SwContentFrm* SwContentFrm::ImplGetNextContentFrm( bool bFwd ) const
{
    const SwFrm *pFrm = this;
    // #100926#
    const SwContentFrm *pContentFrm = 0;
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
    } while ( 0 == (pContentFrm = (pFrm->IsContentFrm() ? static_cast<const SwContentFrm*>(pFrm) : 0) ));

    return pContentFrm;
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
            // #i28701# - use new method <GetPageFrm()>
            if ( static_cast<SwFlyFrm*>(pRet)->GetPageFrm() )
                pRet = static_cast<SwFlyFrm*>(pRet)->GetPageFrm();
            else
                pRet = static_cast<SwFlyFrm*>(pRet)->AnchorFrm();
        }
        else
            return 0;
    }
    return static_cast<SwPageFrm*>(pRet);
}

SwFootnoteBossFrm* SwFrm::FindFootnoteBossFrm( bool bFootnotes )
{
    SwFrm *pRet = this;
    // Footnote bosses can't exist inside a table; also sections with columns
    // don't contain footnote texts there
    if( pRet->IsInTab() )
        pRet = pRet->FindTabFrm();
    while ( pRet && !pRet->IsFootnoteBossFrm() )
    {
        if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrm() )
        {
            // #i28701# - use new method <GetPageFrm()>
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
        OSL_ENSURE( pSct, "FindFootnoteBossFrm: Single column outside section?" );
        if( !pSct->IsFootnoteAtEnd() )
            return pSct->FindFootnoteBossFrm( true );
    }
    return static_cast<SwFootnoteBossFrm*>(pRet);
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
    return static_cast<SwTabFrm*>(pRet);
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
    return static_cast<SwSectionFrm*>(pRet);
}

SwFootnoteFrm *SwFrm::ImplFindFootnoteFrm()
{
    SwFrm *pRet = this;
    while ( !pRet->IsFootnoteFrm() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return 0;
    }
    return static_cast<SwFootnoteFrm*>(pRet);
}

SwFlyFrm *SwFrm::ImplFindFlyFrm()
{
    SwFrm *pRet = this;
    do
    {
        if ( pRet->IsFlyFrm() )
            return static_cast<SwFlyFrm*>(pRet);
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
    {
        if (pRet->GetType() & FRM_HEADFOOT) //header and footer
            return pRet;
        else if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrm() )
            pRet = static_cast<SwFlyFrm*>(pRet)->AnchorFrm();
        else
            return 0;
    } while ( pRet );
    return pRet;
}

const SwFootnoteFrm* SwFootnoteContFrm::FindFootNote() const
{
    const SwFootnoteFrm* pRet = static_cast<const SwFootnoteFrm*>(Lower());
    if( pRet && !pRet->GetAttr()->GetFootnote().IsEndNote() )
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

        // skip pages above point:
        while( pPage && rPt.Y() > pPage->Frm().Bottom() )
            pPage = pPage->GetNext();
    }

    OSL_ENSURE( GetPageNum() <= maPageRects.size(), "number of pages differes from page rect array size" );
    size_t nPageIdx = 0;

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

bool SwRootFrm::IsBetweenPages(const Point& rPt) const
{
    if (!Frm().IsInside(rPt))
        return false;

    // top visible page
    const SwFrm* pPage = Lower();
    if (pPage == nullptr)
        return false;

    // skip pages above point:
    while (pPage && rPt.Y() > pPage->Frm().Bottom())
        pPage = pPage->GetNext();

    return (pPage && !pPage->Frm().IsInside(rPt) &&
        rPt.X() >= pPage->Frm().Left() &&
        rPt.X() <= pPage->Frm().Right());
}

const SwAttrSet* SwFrm::GetAttrSet() const
{
    if ( IsContentFrm() )
        return &static_cast<const SwContentFrm*>(this)->GetNode()->GetSwAttrSet();
    else
        return &static_cast<const SwLayoutFrm*>(this)->GetFormat()->GetAttrSet();
}

//UUUU
drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwFrm::getSdrAllFillAttributesHelper() const
{
    if(IsContentFrm())
    {
        return static_cast< const SwContentFrm* >(this)->GetNode()->getSdrAllFillAttributesHelper();
    }
    else
    {
        return static_cast< const SwLayoutFrm* >(this)->GetFormat()->getSdrAllFillAttributesHelper();
    }
}

bool SwFrm::supportsFullDrawingLayerFillAttributeSet() const
{
    if (IsContentFrm())
    {
        return true;
    }
    else
    {
        return static_cast< const SwLayoutFrm* >(this)->GetFormat()->supportsFullDrawingLayerFillAttributeSet();
    }
}

/*
 *  SwFrm::_FindNext(), _FindPrev(), InvalidateNextPos()
 *         _FindNextCnt() visits tables and sections and only returns SwContentFrms.
 *
 *  Description         Invalidates the position of the next frame.
 *      This is the direct successor or in case of ContentFrms the next
 *      ContentFrm which sits in the same flow as I do:
 *      - body,
 *      - footnote,
 *      - in headers/footers the notification only needs to be forwarded
 *        inside the section
 *      - same for Flys
 *      - Contents in tabs remain only inside their cell
 *      - in principle tables behave exactly like the Contents
 *      - sections also
 */
// This helper function is an equivalent to the ImplGetNextContentFrm() method,
// besides ContentFrames this function also returns TabFrms and SectionFrms.
static SwFrm* lcl_NextFrm( SwFrm* pFrm )
{
    SwFrm *pRet = 0;
    bool bGoingUp = false;
    do {
        SwFrm *p = 0;

        bool bGoingFwd = false;
        bool bGoingDown = (!bGoingUp && ( 0 != (p = pFrm->IsLayoutFrm() ? static_cast<SwLayoutFrm*>(pFrm)->Lower() : 0)));

        if( !bGoingDown )
        {
            bGoingFwd = (0 != (p = ( pFrm->IsFlyFrm() ? static_cast<SwFlyFrm*>(pFrm)->GetNextLink() : pFrm->GetNext())));
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
    } while ( 0 == (pRet = ( ( pFrm->IsContentFrm() || ( !bGoingUp &&
            ( pFrm->IsTabFrm() || pFrm->IsSctFrm() ) ) )? pFrm : 0 ) ) );
    return pRet;
}

SwFrm *SwFrm::_FindNext()
{
    bool bIgnoreTab = false;
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        //The last Content of the table gets picked up and his follower is
        //returned. To be able to deactivate the special case for tables
        //(see below) bIgnoreTab will be set.
        if ( static_cast<SwTabFrm*>(this)->GetFollow() )
            return static_cast<SwTabFrm*>(this)->GetFollow();

        pThis = static_cast<SwTabFrm*>(this)->FindLastContent();
        if ( !pThis )
            pThis = this;
        bIgnoreTab = true;
    }
    else if ( IsSctFrm() )
    {
        //The last Content of the section gets picked and his follower is returned.
        if ( static_cast<SwSectionFrm*>(this)->GetFollow() )
            return static_cast<SwSectionFrm*>(this)->GetFollow();

        pThis = static_cast<SwSectionFrm*>(this)->FindLastContent();
        if ( !pThis )
            pThis = this;
    }
    else if ( IsContentFrm() )
    {
        if( static_cast<SwContentFrm*>(this)->GetFollow() )
            return static_cast<SwContentFrm*>(this)->GetFollow();
    }
    else if ( IsRowFrm() )
    {
        SwFrm* pMyUpper = GetUpper();
        if ( pMyUpper->IsTabFrm() && static_cast<SwTabFrm*>(pMyUpper)->GetFollow() )
            return static_cast<SwTabFrm*>(pMyUpper)->GetFollow()->GetLower();
        else return NULL;
    }
    else
        return NULL;

    SwFrm* pRet = NULL;
    const bool bFootnote  = pThis->IsInFootnote();
    if ( !bIgnoreTab && pThis->IsInTab() )
    {
        SwLayoutFrm *pUp = pThis->GetUpper();
        while (pUp && !pUp->IsCellFrm())
            pUp = pUp->GetUpper();
        SAL_WARN_IF(!pUp, "sw.core", "Content in table but not in cell.");
        SwFrm* pNxt = pUp ? static_cast<SwCellFrm*>(pUp)->GetFollowCell() : NULL;
        if ( pNxt )
            pNxt = static_cast<SwCellFrm*>(pNxt)->ContainsContent();
        if ( !pNxt )
        {
            pNxt = lcl_NextFrm( pThis );
            if (pUp && pUp->IsAnLower(pNxt))
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
            if ( bBody || bFootnote )
            {
                while ( pNxtCnt )
                {
                    // OD 02.04.2003 #108446# - check for endnote, only if found
                    // next content isn't contained in a section, that collect its
                    // endnotes at its end.
                    bool bEndn = IsInSct() && !IsSctFrm() &&
                                 ( !pNxtCnt->IsInSct() ||
                                   !pNxtCnt->FindSctFrm()->IsEndnAtEnd()
                                 );
                    if ( ( bBody && pNxtCnt->IsInDocBody() ) ||
                         ( pNxtCnt->IsInFootnote() &&
                           ( bFootnote ||
                             ( bEndn && pNxtCnt->FindFootnoteFrm()->GetAttr()->GetFootnote().IsEndNote() )
                           )
                         )
                       )
                    {
                        pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
                                                    : pNxtCnt;
                        break;
                    }
                    pNxtCnt = lcl_NextFrm( pNxtCnt );
                }
            }
            else if ( pThis->IsInFly() )
            {
                pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrm()
                                            : pNxtCnt;
            }
            else    //footer-/or header section
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
                                                : pNxtCnt;
                }
            }
        }
    }
    if( pRet && pRet->IsInSct() )
    {
        SwSectionFrm* pSct = pRet->FindSctFrm();
        //Footnotes in frames with columns must not return the section which
        //contains the footnote
        if( !pSct->IsAnLower( this ) &&
            (!bFootnote || pSct->IsInFootnote() ) )
            return pSct;
    }
    return pRet;
}

// #i27138# - add parameter <_bInSameFootnote>
SwContentFrm *SwFrm::_FindNextCnt( const bool _bInSameFootnote )
{
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        if ( static_cast<SwTabFrm*>(this)->GetFollow() )
        {
            pThis = static_cast<SwTabFrm*>(this)->GetFollow()->ContainsContent();
            if( pThis )
                return static_cast<SwContentFrm*>(pThis);
        }
        pThis = static_cast<SwTabFrm*>(this)->FindLastContent();
        if ( !pThis )
            return 0;
    }
    else if ( IsSctFrm() )
    {
        if ( static_cast<SwSectionFrm*>(this)->GetFollow() )
        {
            pThis = static_cast<SwSectionFrm*>(this)->GetFollow()->ContainsContent();
            if( pThis )
                return static_cast<SwContentFrm*>(pThis);
        }
        pThis = static_cast<SwSectionFrm*>(this)->FindLastContent();
        if ( !pThis )
            return 0;
    }
    else if ( IsContentFrm() && static_cast<SwContentFrm*>(this)->GetFollow() )
        return static_cast<SwContentFrm*>(this)->GetFollow();

    if ( pThis->IsContentFrm() )
    {
        const bool bBody = pThis->IsInDocBody();
        const bool bFootnote  = pThis->IsInFootnote();
        SwContentFrm *pNxtCnt = static_cast<SwContentFrm*>(pThis)->GetNextContentFrm();
        if ( pNxtCnt )
        {
            // #i27138#
            if ( bBody || ( bFootnote && !_bInSameFootnote ) )
            {
                // handling for environments 'footnotes' and 'document body frames':
                while ( pNxtCnt )
                {
                    if ( (bBody && pNxtCnt->IsInDocBody()) ||
                         (bFootnote  && pNxtCnt->IsInFootnote()) )
                        return pNxtCnt;
                    pNxtCnt = pNxtCnt->GetNextContentFrm();
                }
            }
            // #i27138#
            else if ( bFootnote && _bInSameFootnote )
            {
                // handling for environments 'each footnote':
                // Assure that found next content frame belongs to the same footnotes
                const SwFootnoteFrm* pFootnoteFrmOfNext( pNxtCnt->FindFootnoteFrm() );
                const SwFootnoteFrm* pFootnoteFrmOfCurr( pThis->FindFootnoteFrm() );
                OSL_ENSURE( pFootnoteFrmOfCurr,
                        "<SwFrm::_FindNextCnt() - unknown layout situation: current frame has to have an upper footnote frame." );
                if ( pFootnoteFrmOfNext == pFootnoteFrmOfCurr )
                {
                    return pNxtCnt;
                }
                else if ( pFootnoteFrmOfCurr->GetFollow() )
                {
                    // next content frame has to be the first content frame
                    // in the follow footnote, which contains a content frame.
                    SwFootnoteFrm* pFollowFootnoteFrmOfCurr(
                                        const_cast<SwFootnoteFrm*>(pFootnoteFrmOfCurr) );
                    pNxtCnt = 0L;
                    do {
                        pFollowFootnoteFrmOfCurr = pFollowFootnoteFrmOfCurr->GetFollow();
                        pNxtCnt = pFollowFootnoteFrmOfCurr->ContainsContent();
                    } while ( !pNxtCnt && pFollowFootnoteFrmOfCurr->GetFollow() );
                    return pNxtCnt;
                }
                else
                {
                    // current content frame is the last content frame in the
                    // footnote - no next content frame exists.
                    return 0L;
                }
            }
            else if ( pThis->IsInFly() )
                // handling for environments 'unlinked fly frame' and
                // 'group of linked fly frames':
                return pNxtCnt;
            else
            {
                // handling for environments 'page header' and 'page footer':
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
SwContentFrm* SwFrm::_FindPrevCnt( const bool _bInSameFootnote )
{
    if ( !IsFlowFrm() )
    {
        // nothing to do, if current frame isn't a flow frame.
        return 0L;
    }

    SwContentFrm* pPrevContentFrm( 0L );

    // Because method <SwContentFrm::GetPrevContentFrm()> is used to travel
    // through the layout, a content frame, at which the travel starts, is needed.
    SwContentFrm* pCurrContentFrm = dynamic_cast<SwContentFrm*>(this);

    // perform shortcut, if current frame is a follow, and
    // determine <pCurrContentFrm>, if current frame is a table or section frame
    if ( pCurrContentFrm && pCurrContentFrm->IsFollow() )
    {
        // previous content frame is its master content frame
        pPrevContentFrm = pCurrContentFrm->FindMaster();
    }
    else if ( IsTabFrm() )
    {
        SwTabFrm* pTabFrm( static_cast<SwTabFrm*>(this) );
        if ( pTabFrm->IsFollow() )
        {
            // previous content frame is the last content of its master table frame
            pPrevContentFrm = pTabFrm->FindMaster()->FindLastContent();
        }
        else
        {
            // start content frame for the search is the first content frame of
            // the table frame.
            pCurrContentFrm = pTabFrm->ContainsContent();
        }
    }
    else if ( IsSctFrm() )
    {
        SwSectionFrm* pSectFrm( static_cast<SwSectionFrm*>(this) );
        if ( pSectFrm->IsFollow() )
        {
            // previous content frame is the last content of its master section frame
            pPrevContentFrm = pSectFrm->FindMaster()->FindLastContent();
        }
        else
        {
            // start content frame for the search is the first content frame of
            // the section frame.
            pCurrContentFrm = pSectFrm->ContainsContent();
        }
    }

    // search for next content frame, depending on the environment, in which
    // the current frame is in.
    if ( !pPrevContentFrm && pCurrContentFrm )
    {
        pPrevContentFrm = pCurrContentFrm->GetPrevContentFrm();
        if ( pPrevContentFrm )
        {
            if ( pCurrContentFrm->IsInFly() )
            {
                // handling for environments 'unlinked fly frame' and
                // 'group of linked fly frames':
                // Nothing to do, <pPrevContentFrm> is the one
            }
            else
            {
                const bool bInDocBody = pCurrContentFrm->IsInDocBody();
                const bool bInFootnote  = pCurrContentFrm->IsInFootnote();
                if ( bInDocBody || ( bInFootnote && !_bInSameFootnote ) )
                {
                    // handling for environments 'footnotes' and 'document body frames':
                    // Assure that found previous frame is also in one of these
                    // environments. Otherwise, travel further
                    while ( pPrevContentFrm )
                    {
                        if ( ( bInDocBody && pPrevContentFrm->IsInDocBody() ) ||
                             ( bInFootnote && pPrevContentFrm->IsInFootnote() ) )
                        {
                            break;
                        }
                        pPrevContentFrm = pPrevContentFrm->GetPrevContentFrm();
                    }
                }
                else if ( bInFootnote && _bInSameFootnote )
                {
                    // handling for environments 'each footnote':
                    // Assure that found next content frame belongs to the same footnotes
                    const SwFootnoteFrm* pFootnoteFrmOfPrev( pPrevContentFrm->FindFootnoteFrm() );
                    const SwFootnoteFrm* pFootnoteFrmOfCurr( pCurrContentFrm->FindFootnoteFrm() );
                    if ( pFootnoteFrmOfPrev != pFootnoteFrmOfCurr )
                    {
                        if ( pFootnoteFrmOfCurr->GetMaster() )
                        {
                            SwFootnoteFrm* pMasterFootnoteFrmOfCurr(
                                        const_cast<SwFootnoteFrm*>(pFootnoteFrmOfCurr) );
                            pPrevContentFrm = 0L;
                            // #146872#
                            // correct wrong loop-condition
                            do {
                                pMasterFootnoteFrmOfCurr = pMasterFootnoteFrmOfCurr->GetMaster();
                                pPrevContentFrm = pMasterFootnoteFrmOfCurr->FindLastContent();
                            } while ( !pPrevContentFrm &&
                                      pMasterFootnoteFrmOfCurr->GetMaster() );
                        }
                        else
                        {
                            // current content frame is the first content in the
                            // footnote - no previous content exists.
                            pPrevContentFrm = 0L;
                        }
                    }
                }
                else
                {
                    // handling for environments 'page header' and 'page footer':
                    // Assure that found previous frame is also in the same
                    // page header respectively page footer as <pCurrContentFrm>
                    // Note: At this point its clear, that <pCurrContentFrm> has
                    //       to be inside a page header or page footer and that
                    //       neither <pCurrContentFrm> nor <pPrevContentFrm> are
                    //       inside a fly frame.
                    //       Thus, method <FindFooterOrHeader()> can be used.
                    OSL_ENSURE( pCurrContentFrm->FindFooterOrHeader(),
                            "<SwFrm::_FindPrevCnt()> - unknown layout situation: current frame should be in page header or page footer" );
                    OSL_ENSURE( !pPrevContentFrm->IsInFly(),
                            "<SwFrm::_FindPrevCnt()> - unknown layout situation: found previous frame should *not* be inside a fly frame." );
                    if ( pPrevContentFrm->FindFooterOrHeader() !=
                                            pCurrContentFrm->FindFooterOrHeader() )
                    {
                        pPrevContentFrm = 0L;
                    }
                }
            }
        }
    }

    return pPrevContentFrm;
}

SwFrm *SwFrm::_FindPrev()
{
    bool bIgnoreTab = false;
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        //The first Content of the table gets picked up and his predecessor is
        //returned. To be able to deactivate the special case for tables
        //(see below) bIgnoreTab will be set.
        if ( static_cast<SwTabFrm*>(this)->IsFollow() )
            return static_cast<SwTabFrm*>(this)->FindMaster();
        else
            pThis = static_cast<SwTabFrm*>(this)->ContainsContent();
        bIgnoreTab = true;
    }

    if ( pThis && pThis->IsContentFrm() )
    {
        SwContentFrm *pPrvCnt = static_cast<SwContentFrm*>(pThis)->GetPrevContentFrm();
        if( !pPrvCnt )
            return 0;
        if ( !bIgnoreTab && pThis->IsInTab() )
        {
            SwLayoutFrm *pUp = pThis->GetUpper();
            while (pUp && !pUp->IsCellFrm())
                pUp = pUp->GetUpper();
            SAL_WARN_IF(!pUp, "sw.core", "Content in table but not in cell.");
            if (pUp && pUp->IsAnLower(pPrvCnt))
                return pPrvCnt;
        }
        else
        {
            SwFrm* pRet;
            const bool bBody = pThis->IsInDocBody();
            const bool bFootnote  = !bBody && pThis->IsInFootnote();
            if ( bBody || bFootnote )
            {
                while ( pPrvCnt )
                {
                    if ( (bBody && pPrvCnt->IsInDocBody()) ||
                            (bFootnote   && pPrvCnt->IsInFootnote()) )
                    {
                        pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
                                                  : static_cast<SwFrm*>(pPrvCnt);
                        return pRet;
                    }
                    pPrvCnt = pPrvCnt->GetPrevContentFrm();
                }
            }
            else if ( pThis->IsInFly() )
            {
                pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrm()
                                            : static_cast<SwFrm*>(pPrvCnt);
                return pRet;
            }
            else // footer or header or Fly
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
                                                : static_cast<SwFrm*>(pPrvCnt);
                    return pRet;
                }
            }
        }
    }
    return 0;
}

void SwFrm::ImplInvalidateNextPos( bool bNoFootnote )
{
    SwFrm *pFrm;
    if ( 0 != (pFrm = _FindNext()) )
    {
        if( pFrm->IsSctFrm() )
        {
            while( pFrm && pFrm->IsSctFrm() )
            {
                if( static_cast<SwSectionFrm*>(pFrm)->GetSection() )
                {
                    SwFrm* pTmp = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
                    if( pTmp )
                        pTmp->InvalidatePos();
                    else if( !bNoFootnote )
                        static_cast<SwSectionFrm*>(pFrm)->InvalidateFootnotePos();
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
                    // We need to invalidate the section's content so it gets
                    // the chance to flow to a different page.
                    SwFrm* pTmp = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
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

    FME 2004-04-19 #i27145# Moved function from SwTextFrm to SwFrm
*/
void SwFrm::InvalidateNextPrtArea()
{
    // determine next frame
    SwFrm* pNextFrm = FindNext();
    // skip empty section frames and hidden text frames
    {
        while ( pNextFrm &&
                ( ( pNextFrm->IsSctFrm() &&
                    !static_cast<SwSectionFrm*>(pNextFrm)->GetSection() ) ||
                  ( pNextFrm->IsTextFrm() &&
                    static_cast<SwTextFrm*>(pNextFrm)->IsHiddenNow() ) ) )
        {
            pNextFrm = pNextFrm->FindNext();
        }
    }

    // Invalidate printing area of found next frame
    if ( pNextFrm )
    {
        if ( pNextFrm->IsSctFrm() )
        {
            // Invalidate printing area of found section frame, if
            // (1) this text frame isn't in a section OR
            // (2) found section frame isn't a follow of the section frame this
            //     text frame is in.
            if ( !IsInSct() || FindSctFrm()->GetFollow() != pNextFrm )
            {
                pNextFrm->InvalidatePrt();
            }

            // Invalidate printing area of first content in found section.
            SwFrm* pFstContentOfSctFrm =
                    static_cast<SwSectionFrm*>(pNextFrm)->ContainsAny();
            if ( pFstContentOfSctFrm )
            {
                pFstContentOfSctFrm->InvalidatePrt();
            }
        }
        else
        {
            pNextFrm->InvalidatePrt();
        }
    }
}

/// @returns true if the frame _directly_ sits in a section with columns
///     but not if it sits in a table which itself sits in a section with columns.
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
                  _pLayoutFrm->IsInFootnote() )
        {
            if ( _pLayoutFrm->IsInTab() && !IsTabFrm() &&
                 ( !IsContentFrm() || !const_cast<SwFrm*>(this)->GetNextCellLeaf( MAKEPAGE_NONE ) ) )
            {
                bRetVal = false;
            }
            else
            {
                if ( _pLayoutFrm->IsInFly() )
                {
                    // if fly frame has a follow (next linked fly frame),
                    // frame is moveable.
                    if ( const_cast<SwLayoutFrm*>(_pLayoutFrm)->FindFlyFrm()->GetNextLink() )
                    {
                        bRetVal = true;
                    }
                    else
                    {
                        // if environment is columned, frame is moveable, if
                        // it isn't in last column.
                        // search for column frame
                        const SwFrm* pCol = _pLayoutFrm;
                        while ( pCol && !pCol->IsColumnFrm() )
                        {
                            pCol = pCol->GetUpper();
                        }
                        // frame is moveable, if found column frame isn't last one.
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
    if ( !IsFlyFrm() && !GetUpper() ) //not yet pasted, no information available
        return;

    mbInfInvalid = mbInfBody = mbInfTab = mbInfFly = mbInfFootnote = mbInfSct = false;

    SwFrm *pFrm = this;
    if( IsFootnoteContFrm() )
        mbInfFootnote = true;
    do
    {
        // mbInfBody is only set in the page body, but not in the column body
        if ( pFrm->IsBodyFrm() && !mbInfFootnote && pFrm->GetUpper()
             && pFrm->GetUpper()->IsPageFrm() )
            mbInfBody = true;
        else if ( pFrm->IsTabFrm() || pFrm->IsCellFrm() )
        {
            mbInfTab = true;
        }
        else if ( pFrm->IsFlyFrm() )
            mbInfFly = true;
        else if ( pFrm->IsSctFrm() )
            mbInfSct = true;
        else if ( pFrm->IsFootnoteFrm() )
            mbInfFootnote = true;

        pFrm = pFrm->GetUpper();

    } while ( pFrm && !pFrm->IsPageFrm() ); //there is nothing above the page
}

/** Updates the vertical or the righttoleft-flags.
 *
 * If the property is derived, it's from the upper or (for fly frames) from
 * the anchor. Otherwise we've to call a virtual method to check the property.
 */
void SwFrm::SetDirFlags( bool bVert )
{
    if( bVert )
    {
        // OD 2004-01-21 #114969# - if derived, valid vertical flag only if
        // vertical flag of upper/anchor is valid.
        if( mbDerivedVert )
        {
            const SwFrm* pAsk = IsFlyFrm() ?
                          static_cast<SwFlyFrm*>(this)->GetAnchorFrm() : GetUpper();

            OSL_ENSURE( pAsk != this, "Autsch! Stack overflow is about to happen" );

            if( pAsk )
            {
                mbVertical = pAsk->IsVertical();
                mbReverse  = pAsk->IsReverse();

                mbVertLR  = pAsk->IsVertLR();

                if ( !pAsk->mbInvalidVert )
                    mbInvalidVert = false;
            }
        }
        else
            CheckDirection( bVert );
    }
    else
    {
        bool bInv = false;
        if( !mbDerivedR2L ) // CheckDirection is able to set bDerivedR2L!
            CheckDirection( bVert );
        if( mbDerivedR2L )
        {
            const SwFrm* pAsk = IsFlyFrm() ?
                          static_cast<SwFlyFrm*>(this)->GetAnchorFrm() : GetUpper();

            OSL_ENSURE( pAsk != this, "Oops! Stack overflow is about to happen" );

            if( pAsk )
                mbRightToLeft = pAsk->IsRightToLeft();
            if( !pAsk || pAsk->mbInvalidR2L )
                bInv = mbInvalidR2L;
        }
        mbInvalidR2L = bInv;
    }
}

SwLayoutFrm* SwFrm::GetNextCellLeaf( MakePageType )
{
    SwFrm* pTmpFrm = this;
    while (pTmpFrm && !pTmpFrm->IsCellFrm())
        pTmpFrm = pTmpFrm->GetUpper();

    SAL_WARN_IF(!pTmpFrm, "sw.core", "SwFrm::GetNextCellLeaf() without cell");
    return pTmpFrm ? static_cast<SwCellFrm*>(pTmpFrm)->GetFollowCell() : NULL;
}

SwLayoutFrm* SwFrm::GetPrevCellLeaf( MakePageType )
{
    SwFrm* pTmpFrm = this;
    while ( !pTmpFrm->IsCellFrm() )
        pTmpFrm = pTmpFrm->GetUpper();

    OSL_ENSURE( pTmpFrm, "SwFrm::GetNextPreviousLeaf() without cell" );
    return static_cast<SwCellFrm*>(pTmpFrm)->GetPreviousCell();
}

static SwCellFrm* lcl_FindCorrespondingCellFrm( const SwRowFrm& rOrigRow,
                                         const SwCellFrm& rOrigCell,
                                         const SwRowFrm& rCorrRow,
                                         bool bInFollow )
{
    SwCellFrm* pRet = NULL;
    const SwCellFrm* pCell = static_cast<const SwCellFrm*>(rOrigRow.Lower());
    SwCellFrm* pCorrCell = const_cast<SwCellFrm*>(static_cast<const SwCellFrm*>(rCorrRow.Lower()));

    while ( pCell != &rOrigCell && !pCell->IsAnLower( &rOrigCell ) )
    {
        pCell = static_cast<const SwCellFrm*>(pCell->GetNext());
        pCorrCell = static_cast<SwCellFrm*>(pCorrCell->GetNext());
    }

    assert(pCell && pCorrCell && "lcl_FindCorrespondingCellFrm does not work");

    if ( pCell != &rOrigCell )
    {
        // rOrigCell must be a lower of pCell. We need to recurse into the rows:
        assert(pCell->Lower() && pCell->Lower()->IsRowFrm() &&
               "lcl_FindCorrespondingCellFrm does not work");

        const SwRowFrm* pRow = static_cast<const SwRowFrm*>(pCell->Lower());
        while ( !pRow->IsAnLower( &rOrigCell ) )
            pRow = static_cast<const SwRowFrm*>(pRow->GetNext());

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

// VERSION OF GetFollowCell() that assumes that we always have a follow flow line:
SwCellFrm* SwCellFrm::GetFollowCell() const
{
    SwCellFrm* pRet = NULL;

    // NEW TABLES
    // Covered cells do not have follow cells!
    const long nRowSpan = GetLayoutRowSpan();
    if ( nRowSpan < 1 )
        return NULL;

    // find most upper row frame
    const SwFrm* pRow = GetUpper();

    while (pRow && (!pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm()))
        pRow = pRow->GetUpper();

    if (!pRow)
        return NULL;

    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>(pRow->GetUpper());
    if (!pTabFrm || !pTabFrm->GetFollow() || !pTabFrm->HasFollowFlowLine())
        return NULL;

    const SwCellFrm* pThisCell = this;

    // Get last cell of the current table frame that belongs to the rowspan:
    if ( nRowSpan > 1 )
    {
        // optimization: Will end of row span be in last row or exceed row?
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
         pRet = lcl_FindCorrespondingCellFrm( *static_cast<const SwRowFrm*>(pRow), *pThisCell, *pFollowRow, true );

    return pRet;
}

// VERSION OF GetPreviousCell() THAT ASSUMES THAT WE ALWAYS HAVE A FFL
SwCellFrm* SwCellFrm::GetPreviousCell() const
{
    SwCellFrm* pRet = NULL;

    // NEW TABLES
    // Covered cells do not have previous cells!
    if ( GetLayoutRowSpan() < 1 )
        return NULL;

    // find most upper row frame
    const SwFrm* pRow = GetUpper();
    while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
        pRow = pRow->GetUpper();

    OSL_ENSURE( pRow->GetUpper() && pRow->GetUpper()->IsTabFrm(), "GetPreviousCell without Table" );

    const SwTabFrm* pTab = static_cast<const SwTabFrm*>(pRow->GetUpper());

    if ( pTab->IsFollow() )
    {
        const SwFrm* pTmp = pTab->GetFirstNonHeadlineRow();
        const bool bIsInFirstLine = ( pTmp == pRow );

        if ( bIsInFirstLine )
        {
            SwTabFrm *pMaster = pTab->FindMaster();
            if ( pMaster && pMaster->HasFollowFlowLine() )
            {
                SwRowFrm* pMasterRow = static_cast<SwRowFrm*>(pMaster->GetLastLower());
                if ( pMasterRow )
                    pRet = lcl_FindCorrespondingCellFrm( *static_cast<const SwRowFrm*>(pRow), *this, *pMasterRow, false );
                if ( pRet && pRet->GetTabBox()->getRowSpan() < 1 )
                    pRet = &const_cast<SwCellFrm&>(pRet->FindStartEndOfRowSpanCell( true, true ));
            }
        }
    }

    return pRet;
}

// --> NEW TABLES
const SwCellFrm& SwCellFrm::FindStartEndOfRowSpanCell( bool bStart, bool bCurrentTableOnly ) const
{
    const SwCellFrm* pRet = 0;

    const SwTabFrm* pTableFrm = dynamic_cast<const SwTabFrm*>(GetUpper()->GetUpper());

    if ( !bStart && pTableFrm && pTableFrm->IsFollow() && pTableFrm->IsInHeadline( *this ) )
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

            // check how many rows we are allowed to go up or down until we reach the end of
            // the current table frame:
            nMax = 0;
            while ( bStart ? pCurrentRow->GetPrev() : pCurrentRow->GetNext() )
            {
                if ( bStart )
                {
                    // do not enter a repeated headline:
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

        // By passing the nMax value for Find*OfRowSpan (in case of bCurrentTableOnly
        // is set) we assure that we find a rMasterBox that has a SwCellFrm in
        // the current table frame:
        const SwTableBox& rMasterBox = bStart ?
                                       GetTabBox()->FindStartOfRowSpan( *pTable, nMax ) :
                                       GetTabBox()->FindEndOfRowSpan( *pTable, nMax );

        SwIterator<SwCellFrm,SwFormat> aIter( *rMasterBox.GetFrameFormat() );

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

    assert(pRet && "SwCellFrm::FindStartRowSpanCell: No result");

    return *pRet;
}
// <-- NEW TABLES

const SwRowFrm* SwFrm::IsInSplitTableRow() const
{
    OSL_ENSURE( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" );

    const SwFrm* pRow = this;

    // find most upper row frame
    while( pRow && ( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return NULL;

    OSL_ENSURE( pRow->GetUpper()->IsTabFrm(), "Confusion in table layout" );

    const SwTabFrm* pTab = static_cast<const SwTabFrm*>(pRow->GetUpper());

    // If most upper row frame is a headline row, the current frame
    // can't be in a splitted table row. Thus, add corresponding condition.
    if ( pRow->GetNext() ||
         pTab->GetTable()->IsHeadline(
                    *(static_cast<const SwRowFrm*>(pRow)->GetTabLine()) ) ||
         !pTab->HasFollowFlowLine() ||
         !pTab->GetFollow() )
        return NULL;

    // skip headline
    const SwRowFrm* pFollowRow = pTab->GetFollow()->GetFirstNonHeadlineRow();

    OSL_ENSURE( pFollowRow, "SwFrm::IsInSplitTableRow() does not work" );

    return pFollowRow;
}

const SwRowFrm* SwFrm::IsInFollowFlowRow() const
{
    OSL_ENSURE( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" );

    // find most upper row frame
    const SwFrm* pRow = this;
    while( pRow && ( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return NULL;

    OSL_ENSURE( pRow->GetUpper()->IsTabFrm(), "Confusion in table layout" );

    const SwTabFrm* pTab = static_cast<const SwTabFrm*>(pRow->GetUpper());

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
