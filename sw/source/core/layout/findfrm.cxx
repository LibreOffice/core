/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "node.hxx"
#include "doc.hxx"
#include "frmtool.hxx"
#include "flyfrm.hxx"
#include <frmfmt.hxx>
#include <cellfrm.hxx>
#include <rowfrm.hxx>
#include <swtable.hxx>

#include "tabfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrms.hxx"
#include "ftnfrm.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"
#include <txtfrm.hxx>   // SwTxtFrm
#include <switerator.hxx>

/*************************************************************************
|*
|*  FindBodyCont, FindLastBodyCntnt()
|*
|*  Description     Searches the first/last CntntFrm in BodyText below the page.
|*
|*************************************************************************/
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

/*************************************************************************
|*
|*  SwLayoutFrm::ContainsCntnt
|*
|*  Description     Checks if the frame contains one or more CntntFrm's
|*          anywhere in his subsidiary structure; if so the first found CntntFrm
|*          is returned
|*
|*************************************************************************/

const SwCntntFrm *SwLayoutFrm::ContainsCntnt() const
{
    //Search downwards the layout leaf and if there is no content, jump to the
    //next leaf until content is found or we leave "this".
    //Sections: Cntnt next to sections would not be found this way (empty
    //sections directly next to CntntFrm) therefore we need to recursively
    //search for them even if it's more complex.

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

/*************************************************************************
|*
|*  SwLayoutFrm::FirstCell
|*
|*  Description     Calls ContainsAny first to reach the innermost cell. From
|*                  there we walk back up to the first SwCellFrm. Since we use
|*                  SectionFrms ContainsCntnt()->GetUpper() is not enough any
|*                  more.
|*************************************************************************/

const SwCellFrm *SwLayoutFrm::FirstCell() const
{
    const SwFrm* pCnt = ContainsAny();
    while( pCnt && !pCnt->IsCellFrm() )
        pCnt = pCnt->GetUpper();
    return (const SwCellFrm*)pCnt;
}

/*************************************************************************
|*
|*  SwLayoutFrm::ContainsAny
|*
|*  like ComtainsCntnt, but does not only return CntntFrms but
|*  also sections and tables.
|*************************************************************************/

// #130797#
// New parameter <_bInvestigateFtnForSections> controls investigation of
// content of footnotes for sections.
const SwFrm *SwLayoutFrm::ContainsAny( const bool _bInvestigateFtnForSections ) const
{
    //Search downwards the layout leaf and if there is no content, jump to the
    //next leaf until content is found, we leave "this" or until we found
    //a SectionFrm or a TabFrm.

    const SwLayoutFrm *pLayLeaf = this;
    // #130797#
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
            // Now we also return "deleted" SectionFrms so they can be
            // maintained on SaveCntnt and RestoreCntnt
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


/*************************************************************************
|*
|*  SwFrm::GetLower()
|*
|*************************************************************************/
const SwFrm* SwFrm::GetLower() const
{
    return IsLayoutFrm() ? ((SwLayoutFrm*)this)->Lower() : 0;
}

SwFrm* SwFrm::GetLower()
{
    return IsLayoutFrm() ? ((SwLayoutFrm*)this)->Lower() : 0;
}

/*************************************************************************
|*
|*  SwLayoutFrm::IsAnLower()
|*
|*************************************************************************/
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

    @author OD

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

//
// Local helper functions for GetNextLayoutLeaf
//

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

/*************************************************************************
|*
|*  SwFrm::ImplGetNextLayoutLeaf
|*
|* Finds the next layout leaf. This is a layout frame, which does not
 * have a lower which is a LayoutFrame. That means, pLower can be 0 or a
 * content frame.
 *
 * However, pLower may be a TabFrm
 *
|*************************************************************************/

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
             0 == ( pLayoutFrm = pFrm->IsLayoutFrm() ? (SwLayoutFrm*)pFrm : 0 ) ||
             pLayoutFrm->IsAnLower( this ) );

    return pLayoutFrm;
}



/*************************************************************************
|*
|*    SwFrm::ImplGetNextCntntFrm( bool )
|*
|*      Walk back inside the tree: grab the subordinate Frm if one exists and
|*      the last step was not moving up a level (this would lead to an infinite
|*      up/down loop!). With this we ensure that during walking back we search
|*      through all sub trees. If we walked downwards we have to go to the end
|*      of the chain first because we go backwards from the last Frm inside
|*      another Frm. Walking forward works the same.
|*************************************************************************/

// Caution: fixes in ImplGetNextCntntFrm() may also need to be applied to the
// lcl_NextFrm(..) method above
const SwCntntFrm* SwCntntFrm::ImplGetNextCntntFrm( bool bFwd ) const
{
    const SwFrm *pFrm = this;
    // #100926#
    SwCntntFrm *pCntntFrm = 0;
    sal_Bool bGoingUp = sal_False;
    do {
        const SwFrm *p = 0;
        sal_Bool bGoingFwdOrBwd = sal_False;

        sal_Bool bGoingDown = ( !bGoingUp && ( 0 != ( p = lcl_GetLower( pFrm, true ) ) ) );
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




/*************************************************************************
|*
|*  SwFrm::FindRootFrm(), FindTabFrm(), FindFtnFrm(), FindFlyFrm(),
|*         FindPageFrm(), FindColFrm()
|*
|*************************************************************************/
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
    return (SwPageFrm*)pRet;
}

SwFtnBossFrm* SwFrm::FindFtnBossFrm( sal_Bool bFootnotes )
{
    SwFrm *pRet = this;
    // Footnote bosses can't exist inside a table; also sections with columns
    // don't contain footnote texts there
    if( pRet->IsInTab() )
        pRet = pRet->FindTabFrm();
    while ( pRet && !pRet->IsFtnBossFrm() )
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
    {   if ( pRet->GetType() & 0x0018 ) //header and footer
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

        // skip pages above point:
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

/*************************************************************************
|*
|*  SwFrmFrm::GetAttrSet()
|*
|*************************************************************************/
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

// This helper function is an equivalent to the ImplGetNextCntntFrm() method,
// besides ContentFrames this function also returns TabFrms and SectionFrms.
static SwFrm* lcl_NextFrm( SwFrm* pFrm )
{
    SwFrm *pRet = 0;
    sal_Bool bGoingUp = sal_False;
    do {
        SwFrm *p = 0;

        sal_Bool bGoingFwd = sal_False;
        sal_Bool bGoingDown = (!bGoingUp && ( 0 != (p = pFrm->IsLayoutFrm() ? ((SwLayoutFrm*)pFrm)->Lower() : 0)));

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
    sal_Bool bIgnoreTab = sal_False;
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        //The last Cntnt of the table gets picked up and his follower is
        //returned. To be able to deactivate the special case for tables
        //(see below) bIgnoreTab will be set.
        if ( ((SwTabFrm*)this)->GetFollow() )
            return ((SwTabFrm*)this)->GetFollow();

        pThis = ((SwTabFrm*)this)->FindLastCntnt();
        if ( !pThis )
            pThis = this;
        bIgnoreTab = sal_True;
    }
    else if ( IsSctFrm() )
    {
        //The last Cntnt of the section gets picked and his follower is returned.
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
        const sal_Bool bBody = pThis->IsInDocBody();
        SwFrm *pNxtCnt = lcl_NextFrm( pThis );
        if ( pNxtCnt )
        {
            if ( bBody || bFtn )
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
                                                : (SwFrm*)pNxtCnt;
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
            (!bFtn || pSct->IsInFtn() ) )
            return pSct;
    }
    return pRet;
}

// #i27138# - add parameter <_bInSameFtn>
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
        const sal_Bool bBody = pThis->IsInDocBody();
        const sal_Bool bFtn  = pThis->IsInFtn();
        SwCntntFrm *pNxtCnt = ((SwCntntFrm*)pThis)->GetNextCntntFrm();
        if ( pNxtCnt )
        {
            // #i27138#
            if ( bBody || ( bFtn && !_bInSameFtn ) )
            {
                // handling for environments 'footnotes' and 'document body frames':
                while ( pNxtCnt )
                {
                    if ( (bBody && pNxtCnt->IsInDocBody()) ||
                         (bFtn  && pNxtCnt->IsInFtn()) )
                        return pNxtCnt;
                    pNxtCnt = pNxtCnt->GetNextCntntFrm();
                }
            }
            // #i27138#
            else if ( bFtn && _bInSameFtn )
            {
                // handling for environments 'each footnote':
                // Assure that found next content frame belongs to the same footnotes
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
                    // next content frame has to be the first content frame
                    // in the follow footnote, which contains a content frame.
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

    @author OD
*/
SwCntntFrm* SwFrm::_FindPrevCnt( const bool _bInSameFtn )
{
    if ( !IsFlowFrm() )
    {
        // nothing to do, if current frame isn't a flow frame.
        return 0L;
    }

    SwCntntFrm* pPrevCntntFrm( 0L );

    // Because method <SwCntntFrm::GetPrevCntntFrm()> is used to travel
    // through the layout, a content frame, at which the travel starts, is needed.
    SwCntntFrm* pCurrCntntFrm = dynamic_cast<SwCntntFrm*>(this);

    // perform shortcut, if current frame is a follow, and
    // determine <pCurrCntntFrm>, if current frame is a table or section frame
    if ( pCurrCntntFrm && pCurrCntntFrm->IsFollow() )
    {
        // previous content frame is its master content frame
        pPrevCntntFrm = pCurrCntntFrm->FindMaster();
    }
    else if ( IsTabFrm() )
    {
        SwTabFrm* pTabFrm( static_cast<SwTabFrm*>(this) );
        if ( pTabFrm->IsFollow() )
        {
            // previous content frame is the last content of its master table frame
            pPrevCntntFrm = pTabFrm->FindMaster()->FindLastCntnt();
        }
        else
        {
            // start content frame for the search is the first content frame of
            // the table frame.
            pCurrCntntFrm = pTabFrm->ContainsCntnt();
        }
    }
    else if ( IsSctFrm() )
    {
        SwSectionFrm* pSectFrm( static_cast<SwSectionFrm*>(this) );
        if ( pSectFrm->IsFollow() )
        {
            // previous content frame is the last content of its master section frame
            pPrevCntntFrm = pSectFrm->FindMaster()->FindLastCntnt();
        }
        else
        {
            // start content frame for the search is the first content frame of
            // the section frame.
            pCurrCntntFrm = pSectFrm->ContainsCntnt();
        }
    }

    // search for next content frame, depending on the environment, in which
    // the current frame is in.
    if ( !pPrevCntntFrm && pCurrCntntFrm )
    {
        pPrevCntntFrm = pCurrCntntFrm->GetPrevCntntFrm();
        if ( pPrevCntntFrm )
        {
            if ( pCurrCntntFrm->IsInFly() )
            {
                // handling for environments 'unlinked fly frame' and
                // 'group of linked fly frames':
                // Nothing to do, <pPrevCntntFrm> is the one
            }
            else
            {
                const bool bInDocBody = pCurrCntntFrm->IsInDocBody();
                const bool bInFtn  = pCurrCntntFrm->IsInFtn();
                if ( bInDocBody || ( bInFtn && !_bInSameFtn ) )
                {
                    // handling for environments 'footnotes' and 'document body frames':
                    // Assure that found previous frame is also in one of these
                    // environments. Otherwise, travel further
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
                    // handling for environments 'each footnote':
                    // Assure that found next content frame belongs to the same footnotes
                    const SwFtnFrm* pFtnFrmOfPrev( pPrevCntntFrm->FindFtnFrm() );
                    const SwFtnFrm* pFtnFrmOfCurr( pCurrCntntFrm->FindFtnFrm() );
                    if ( pFtnFrmOfPrev != pFtnFrmOfCurr )
                    {
                        if ( pFtnFrmOfCurr->GetMaster() )
                        {
                            SwFtnFrm* pMasterFtnFrmOfCurr(
                                        const_cast<SwFtnFrm*>(pFtnFrmOfCurr) );
                            pPrevCntntFrm = 0L;
                            // #146872#
                            // correct wrong loop-condition
                            do {
                                pMasterFtnFrmOfCurr = pMasterFtnFrmOfCurr->GetMaster();
                                pPrevCntntFrm = pMasterFtnFrmOfCurr->FindLastCntnt();
                            } while ( !pPrevCntntFrm &&
                                      pMasterFtnFrmOfCurr->GetMaster() );
                        }
                        else
                        {
                            // current content frame is the first content in the
                            // footnote - no previous content exists.
                            pPrevCntntFrm = 0L;;
                        }
                    }
                }
                else
                {
                    // handling for environments 'page header' and 'page footer':
                    // Assure that found previous frame is also in the same
                    // page header respectively page footer as <pCurrCntntFrm>
                    // Note: At this point its clear, that <pCurrCntntFrm> has
                    //       to be inside a page header or page footer and that
                    //       neither <pCurrCntntFrm> nor <pPrevCntntFrm> are
                    //       inside a fly frame.
                    //       Thus, method <FindFooterOrHeader()> can be used.
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
    sal_Bool bIgnoreTab = sal_False;
    SwFrm *pThis = this;

    if ( IsTabFrm() )
    {
        //The first Cntnt of the table gets picked up and his predecessor is
        //returnd. To be able to deactivate the special case for tables
        //(see below) bIgnoreTab will be set.
        if ( ((SwTabFrm*)this)->IsFollow() )
            return ((SwTabFrm*)this)->FindMaster();
        else
            pThis = ((SwTabFrm*)this)->ContainsCntnt();
        bIgnoreTab = sal_True;
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
            const sal_Bool bBody = pThis->IsInDocBody();
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
                    // We need to invalidate the section's content so it gets
                    // the chance to flow to a different page.
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

    @author OD

    FME 2004-04-19 #i27145# Moved function from SwTxtFrm to SwFrm
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
                  ( pNextFrm->IsTxtFrm() &&
                    static_cast<SwTxtFrm*>(pNextFrm)->IsHiddenNow() ) ) )
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

/*************************************************************************
|*
|*    lcl_IsInColSect()
|*
|* returns true if the frame _directly_ sits in a section with columns
|* but not if it sits in a table which itself sits in a section with columns.
|*************************************************************************/

static sal_Bool lcl_IsInColSct( const SwFrm *pUp )
{
    sal_Bool bRet = sal_False;
    while( pUp )
    {
        if( pUp->IsColumnFrm() )
            bRet = sal_True;
        else if( pUp->IsSctFrm() )
            return bRet;
        else if( pUp->IsTabFrm() )
            return sal_False;
        pUp = pUp->GetUpper();
    }
    return sal_False;
}

/*************************************************************************
|*
|*    SwFrm::IsMoveable();
|*
|*************************************************************************/
/** determine, if frame is moveable in given environment

    OD 08.08.2003 #110978#
    method replaced 'old' method <sal_Bool IsMoveable() const>.
    Determines, if frame is moveable in given environment. if no environment
    is given (parameter _pLayoutFrm == 0L), the movability in the actual
    environment (<this->GetUpper()) is checked.

    @author OD
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

/*************************************************************************
|*
|*    SwFrm::SetInfFlags();
|*
|*************************************************************************/
void SwFrm::SetInfFlags()
{
    if ( !IsFlyFrm() && !GetUpper() ) //not yet pasted, no information available
        return;

    bInfInvalid = bInfBody = bInfTab = bInfFly = bInfFtn = bInfSct = sal_False;

    SwFrm *pFrm = this;
    if( IsFtnContFrm() )
        bInfFtn = sal_True;
    do
    {
        // bInfBody is only set in the page body, but not in the column body
        if ( pFrm->IsBodyFrm() && !bInfFtn && pFrm->GetUpper()
             && pFrm->GetUpper()->IsPageFrm() )
            bInfBody = sal_True;
        else if ( pFrm->IsTabFrm() || pFrm->IsCellFrm() )
        {
            bInfTab = sal_True;
        }
        else if ( pFrm->IsFlyFrm() )
            bInfFly = sal_True;
        else if ( pFrm->IsSctFrm() )
            bInfSct = sal_True;
        else if ( pFrm->IsFtnFrm() )
            bInfFtn = sal_True;

        pFrm = pFrm->GetUpper();

    } while ( pFrm && !pFrm->IsPageFrm() ); //there is nothing above the page
}

/*
 * SwFrm::SetDirFlags( sal_Bool )
 * actualizes the vertical or the righttoleft-flags.
 * If the property is derived, it's from the upper or (for fly frames) from
 * the anchor. Otherwise we've to call a virtual method to check the property.
 */

void SwFrm::SetDirFlags( sal_Bool bVert )
{
    if( bVert )
    {
        // OD 2004-01-21 #114969# - if derived, valid vertical flag only if
        // vertical flag of upper/anchor is valid.
        if( bDerivedVert )
        {
            const SwFrm* pAsk = IsFlyFrm() ?
                          ((SwFlyFrm*)this)->GetAnchorFrm() : GetUpper();

            OSL_ENSURE( pAsk != this, "Autsch! Stack overflow is about to happen" );

            if( pAsk )
            {
                bVertical = pAsk->IsVertical() ? 1 : 0;
                bReverse  = pAsk->IsReverse()  ? 1 : 0;

                bVertLR  = pAsk->IsVertLR() ? 1 : 0;
                //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
                if ( !pAsk->bInvalidVert )
                    bInvalidVert = sal_False;
            }
        }
        else
            CheckDirection( bVert );
    }
    else
    {
        sal_Bool bInv = 0;
        if( !bDerivedR2L ) // CheckDirection is able to set bDerivedR2L!
            CheckDirection( bVert );
        if( bDerivedR2L )
        {
            const SwFrm* pAsk = IsFlyFrm() ?
                          ((SwFlyFrm*)this)->GetAnchorFrm() : GetUpper();

            OSL_ENSURE( pAsk != this, "Oops! Stack overflow is about to happen" );

            if( pAsk )
                bRightToLeft = pAsk->IsRightToLeft() ? 1 : 0;
            if( !pAsk || pAsk->bInvalidR2L )
                bInv = bInvalidR2L;
        }
        bInvalidR2L = bInv;
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
        // rOrigCell must be a lower of pCell. We need to recurse into the rows:
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
    while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
        pRow = pRow->GetUpper();

    if ( !pRow )
        return NULL;

    const SwTabFrm* pTabFrm = static_cast<const SwTabFrm*>( pRow->GetUpper() );
    if ( !pRow || !pTabFrm->GetFollow() || !pTabFrm->HasFollowFlowLine() )
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
         pRet = lcl_FindCorrespondingCellFrm( *((SwRowFrm*)pRow), *pThisCell, *pFollowRow, true );

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

// --> NEW TABLES
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

    const SwTabFrm* pTab = (SwTabFrm*)pRow->GetUpper();
    //
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

/*
 * SwLayoutFrm::GetLastLower()
 */
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
