/*************************************************************************
 *
 *  $RCSfile: anchoredobject.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: od $ $Date: 2004-08-03 06:05:14 $
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
#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif

#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
// --> OD 2004-06-29 #i28701#
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
// <--
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

// ============================================================================
// --> OD 2004-06-30 #i28701# -
// implementation of helper class <SwObjPositioningInProgress>
// ============================================================================
SwObjPositioningInProgress::SwObjPositioningInProgress( SdrObject& _rSdrObj ) :
    mpAnchoredObj( 0L )
{
    mpAnchoredObj = ::GetUserCall( &_rSdrObj )->GetAnchoredObj( &_rSdrObj );
    mpAnchoredObj->SetPositioningInProgress( true );
}

SwObjPositioningInProgress::SwObjPositioningInProgress( SwAnchoredObject& _rAnchoredObj ) :
    mpAnchoredObj( &_rAnchoredObj )
{
    mpAnchoredObj->SetPositioningInProgress( true );
}

SwObjPositioningInProgress::~SwObjPositioningInProgress()
{
    if ( mpAnchoredObj )
    {
        mpAnchoredObj->SetPositioningInProgress( false );
    }
}

// ============================================================================

TYPEINIT0(SwAnchoredObject);

SwAnchoredObject::SwAnchoredObject() :
    mpDrawObj( 0L ),
    mpAnchorFrm( 0L ),
    // --> OD 2004-06-30 #i28701#
    mpPageFrm( 0L ),
    // <--
    maLastCharRect(),
    mnLastTopOfLine( 0L ),
    mpVertPosOrientFrm( 0L ),
    // --> OD 2004-06-29 #i28701#
    mbPositioningInProgress( false ),
    mbConsiderForTextWrap( false ),
    mbPositionLocked( false ),
    mbRestartLayoutProcess( false )
    // <--
{
}

SwAnchoredObject::~SwAnchoredObject()
{
}

// =============================================================================
// accessors for member <mpDrawObj>
// =============================================================================
void SwAnchoredObject::SetDrawObj( SdrObject& _rDrawObj )
{
    mpDrawObj = &_rDrawObj;
}

const SdrObject* SwAnchoredObject::GetDrawObj() const
{
    return mpDrawObj;
}

SdrObject* SwAnchoredObject::DrawObj()
{
    return mpDrawObj;
}

// =============================================================================
// accessors for member <mpAnchorFrm>
// =============================================================================
const SwFrm* SwAnchoredObject::GetAnchorFrm() const
{
    return mpAnchorFrm;
}

SwFrm* SwAnchoredObject::AnchorFrm()
{
    return mpAnchorFrm;
}

void SwAnchoredObject::ChgAnchorFrm( SwFrm* _pNewAnchorFrm )
{
    mpAnchorFrm = _pNewAnchorFrm;

    ObjectAttachedToAnchorFrame();
}

// =============================================================================
// OD 2004-06-30 #i28701# accessors for member <mpPageFrm>
// =============================================================================
SwPageFrm* SwAnchoredObject::GetPageFrm()
{
    return mpPageFrm;
}

const SwPageFrm* SwAnchoredObject::GetPageFrm() const
{
    return mpPageFrm;
}

void SwAnchoredObject::SetPageFrm( SwPageFrm* _pNewPageFrm )
{
    mpPageFrm = _pNewPageFrm;
}

// =============================================================================
// accessors for member <maLastCharRect>
// =============================================================================
const SwRect& SwAnchoredObject::GetLastCharRect() const
{
    return maLastCharRect;
}

SwTwips SwAnchoredObject::GetLastCharX() const
{
    return maLastCharRect.Left() - GetObjRect().Left();
}

SwTwips SwAnchoredObject::GetRelCharX( const SwFrm* pFrm ) const
{
    return maLastCharRect.Left() - pFrm->Frm().Left();
}

SwTwips SwAnchoredObject::GetRelCharY( const SwFrm* pFrm ) const
{
    return maLastCharRect.Bottom() - pFrm->Frm().Top();
}

void SwAnchoredObject::AddLastCharY( long nDiff )
{
    maLastCharRect.Pos().Y() += nDiff;
}

void SwAnchoredObject::ResetLastCharRectHeight()
{
    maLastCharRect.Height( 0 );
}
// =============================================================================
// accessors for member <mpVertPosOrientFrm>
// =============================================================================
const SwLayoutFrm* SwAnchoredObject::GetVertPosOrientFrm() const
{
    return mpVertPosOrientFrm;
}

void SwAnchoredObject::SetVertPosOrientFrm( const SwLayoutFrm& _rVertPosOrientFrm )
{
    mpVertPosOrientFrm = &_rVertPosOrientFrm;

    // --> OD 2004-07-02 #i28701# - take over functionality of deleted method
    // <SwFlyAtCntFrm::AssertPage()>: assure for at-paragraph and at-character
    // an anchored object, that it is registered at the correct page frame
    RegisterAtCorrectPage();
}

// =============================================================================
// accessors for member <mnLastTopOfLine>
// =============================================================================
const SwTwips SwAnchoredObject::GetLastTopOfLine() const
{
    return mnLastTopOfLine;
}

// OD 2004-05-18 #i28701# - follow-up of #i22341#
void SwAnchoredObject::AddLastTopOfLineY( SwTwips _nDiff )
{
    mnLastTopOfLine += _nDiff;
}

/** check anchor character rectangle and top of line

    OD 2004-03-24 #i26791
    For to-character anchored Writer fly frames the members <maLastCharRect>
    and <maLastTopOfLine> are updated. These are checked for change and
    depending on the applied positioning, it's decided, if the Writer fly
    frame has to be invalidated.
    OD 2004-07-15 #117380#
    add parameter <_bCheckForParaPorInf>, default value <true>

    @author OD
*/
void SwAnchoredObject::CheckCharRectAndTopOfLine(
                                        const bool _bCheckForParaPorInf )
{
    if ( GetAnchorFrm() &&
         GetAnchorFrm()->IsTxtFrm() )
    {
        const SwFmtAnchor& rAnch = GetFrmFmt().GetAnchor();
        if ( rAnch.GetAnchorId() == FLY_AUTO_CNTNT &&
             rAnch.GetCntntAnchor() )
        {
            // --> OD 2004-07-14 #117380# - if requested, assure that anchor frame,
            // which contains the anchor character, has a paragraph portion information.
            // The paragraph portion information is needed to determine the
            // anchor character rectangle respectively the top of the line.
            // Thus, a format of this frame is avoided to determine the
            // paragraph portion information.
            xub_StrLen nOffset = rAnch.GetCntntAnchor()->nContent.GetIndex();
            const SwTxtFrm& aAnchorCharFrm =
                    static_cast<SwTxtFrm*>(AnchorFrm())->GetFrmAtOfst( nOffset );
            if ( !_bCheckForParaPorInf || aAnchorCharFrm.HasPara() )
            {
                _CheckCharRect( rAnch, aAnchorCharFrm );
                _CheckTopOfLine( rAnch, aAnchorCharFrm );
            }
            // <--
        }
    }
}

/** check anchor character rectangle

    OD 11.11.2003 #i22341#
    helper method for method <CheckCharRectAndTopOfLine()>
    For to-character anchored Writer fly frames the member <maLastCharRect>
    is updated. This is checked for change and depending on the applied
    positioning, it's decided, if the Writer fly frame has to be invalidated.
    OD 2004-07-14 #117380#
    improvement - add second parameter <_rAnchorCharFrm>

    @author OD
*/
void SwAnchoredObject::_CheckCharRect( const SwFmtAnchor& _rAnch,
                                       const SwTxtFrm& _rAnchorCharFrm )
{
    // determine rectangle of anchor character. If not exist, abort operation
    SwRect aCharRect;
    if ( !_rAnchorCharFrm.GetAutoPos( aCharRect, *_rAnch.GetCntntAnchor() ) )
    {
        return;
    }
    // check, if anchor character rectangle has changed
    if ( aCharRect != maLastCharRect )
    {
        // check positioning and alignment for invalidation of position
        {
            SWRECTFN( (&_rAnchorCharFrm) );
            // determine positioning and alignment
            SwFmtVertOrient aVert( GetFrmFmt().GetVertOrient() );
            SwFmtHoriOrient aHori( GetFrmFmt().GetHoriOrient() );
            // check for anchor character rectangle changes for certain
            // positionings and alignments
            // OD 07.10.2003 #110978# - add condition to invalidate position,
            // if vertical aligned at frame/page area and vertical position
            // of anchor character has changed.
            const SwRelationOrient eVertRelOrient = aVert.GetRelationOrient();
            if ( ( aHori.GetRelationOrient() == REL_CHAR &&
                   (aCharRect.*fnRect->fnGetLeft)() !=
                        (maLastCharRect.*fnRect->fnGetLeft)() ) ||
                 ( eVertRelOrient == REL_CHAR &&
                   ( (aCharRect.*fnRect->fnGetTop)() !=
                        (maLastCharRect.*fnRect->fnGetTop)() ||
                     (aCharRect.*fnRect->fnGetHeight)() !=
                        (maLastCharRect.*fnRect->fnGetHeight)() ) ) ||
                 ( ( ( eVertRelOrient == FRAME ) ||
                     ( eVertRelOrient == PRTAREA ) ||
                     ( eVertRelOrient == REL_PG_FRAME ) ||
                     ( eVertRelOrient == REL_PG_PRTAREA ) ) &&
                   ( (aCharRect.*fnRect->fnGetTop)() !=
                        (maLastCharRect.*fnRect->fnGetTop)() ) ) )
            {
                InvalidateObjPos();
            }
        }
        // keep new anchor character rectangle
        maLastCharRect = aCharRect;
    }
}

/** check top of line

    OD 11.11.2003 #i22341#
    helper method for method <CheckCharRectAndTopOfLine()>
    For to-character anchored Writer fly frames the member <mnLastTopOfLine>
    is updated. This is checked for change and depending on the applied
    positioning, it's decided, if the Writer fly frame has to be invalidated.
    OD 2004-07-14 #117380#
    improvement - add second parameter <_rAnchorCharFrm>

    @author OD
*/
void SwAnchoredObject::_CheckTopOfLine( const SwFmtAnchor& _rAnch,
                                        const SwTxtFrm& _rAnchorCharFrm )
{
    SwTwips nTopOfLine = 0L;
    if ( _rAnchorCharFrm.GetTopOfLine( nTopOfLine, *_rAnch.GetCntntAnchor() ) )
    {
        if ( nTopOfLine != mnLastTopOfLine )
        {
            // check alignment for invalidation of position
            if ( GetFrmFmt().GetVertOrient().GetRelationOrient() == REL_VERT_LINE )
            {
                InvalidateObjPos();
            }
            // keep new top of line value
            mnLastTopOfLine = nTopOfLine;
        }
    }
}

void SwAnchoredObject::ClearCharRectAndTopOfLine()
{
    maLastCharRect.Clear();
    mnLastTopOfLine = 0;
}

void SwAnchoredObject::SetPositioningInProgress( const bool _bPosInProgress )
{
    mbPositioningInProgress = _bPosInProgress;
}

bool SwAnchoredObject::IsPositioningInProgress() const
{
    return mbPositioningInProgress;
}

const Point SwAnchoredObject::GetCurrRelPos() const
{
    return maRelPos;
}
void SwAnchoredObject::SetCurrRelPos( Point _aRelPos )
{
    maRelPos = _aRelPos;
}

void SwAnchoredObject::ObjectAttachedToAnchorFrame()
{
    // default behaviour: nothing to do
}

/** method to perform necessary invalidations for the positioning of
    objects, for whose the wrapping style influence has to be considered
    on the object positioning.

    OD 2004-06-30 #i28701#

    @author OD
*/
void SwAnchoredObject::InvalidateObjPosForConsiderWrapInfluence(
                                                    const bool _bNotifyBackgrd )
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
    {
        // indicate that object has not to be considered for text wrap
        SetConsiderForTextWrap( false );
        // unlock position
        UnlockPosition();
        // invalidate position
        InvalidateObjPos();
        // invalidate 'background', if requested
        if ( _bNotifyBackgrd )
        {
            NotifyBackground( GetPageFrm(), GetObjRectWithSpaces(), PREP_FLY_LEAVE );
        }
    }
}

/** method to determine, if wrapping style influence of the anchored
    object has to be considered on the object positioning

    OD 2004-06-30 #i28701#
    Note: result of this method also decides, if the boolean for the
    layout process are of relevance.

    @author OD
*/
bool SwAnchoredObject::ConsiderObjWrapInfluenceOnObjPos() const
{
    bool bRet( false );

    const SwFrmFmt& rObjFmt = GetFrmFmt();
    if ( rObjFmt.GetDoc()->ConsiderWrapOnObjPos() )
    {
        const SwFmtAnchor& rAnchor = rObjFmt.GetAnchor();
        if ( ( rAnchor.GetAnchorId() == FLY_AUTO_CNTNT ||
               rAnchor.GetAnchorId() == FLY_AT_CNTNT ) &&
             rObjFmt.GetSurround().GetSurround() != SURROUND_THROUGHT )
        {
            SdrLayerID nHellId = rObjFmt.GetDoc()->GetHellId();
            SdrLayerID nInvisibleHellId = rObjFmt.GetDoc()->GetInvisibleHellId();
            if ( GetDrawObj()->GetLayer() != nHellId &&
                 GetDrawObj()->GetLayer() != nInvisibleHellId )
            {
                bRet = true;
            }
        }
    }

    return bRet;
}

// =============================================================================
// --> OD 2004-06-29 #i28701# - accessors to booleans for layout process
// =============================================================================
bool SwAnchoredObject::ConsiderForTextWrap() const
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
        return mbConsiderForTextWrap;
    else
        return true;
}

void SwAnchoredObject::SetConsiderForTextWrap( const bool _bConsiderForTextWrap )
{
    mbConsiderForTextWrap = _bConsiderForTextWrap;
}

bool SwAnchoredObject::PositionLocked() const
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
        return mbPositionLocked;
    else
        return false;
}

void SwAnchoredObject::LockPosition()
{
    mbPositionLocked = true;
}

void SwAnchoredObject::UnlockPosition()
{
    mbPositionLocked = false;
}

bool SwAnchoredObject::RestartLayoutProcess() const
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
        return mbRestartLayoutProcess;
    else
        return false;
}

void SwAnchoredObject::SetRestartLayoutProcess( const bool _bRestartLayoutProcess )
{
    mbRestartLayoutProcess = _bRestartLayoutProcess;
}

/** method to add spacing to object area

    OD 2004-06-30 #i28701#

    @author OD
*/
const SwRect SwAnchoredObject::GetObjRectWithSpaces() const
{
    SwRect aRet( GetObjRect() );
    const SwFrmFmt& rFmt = GetFrmFmt();
    const SvxULSpaceItem& rUL = rFmt.GetULSpace();
    const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
    {
        aRet.Top ( Max( aRet.Top() - long(rUL.GetUpper()), 0L ));
        aRet.Left( Max( aRet.Left()- long(rLR.GetLeft()),  0L ));
        aRet.SSize().Height() += rUL.GetLower();
        aRet.SSize().Width()  += rLR.GetRight();
    }
    return aRet;
}

/** method to update anchored object in the <SwSortedObjs> lists

    OD 2004-07-01 #i28701#
    If document compatibility option 'Consider wrapping style influence
    on object positioning' is ON, additionally all anchored objects
    at the anchor frame and all following anchored objects on the page
    frame are invalidated.

    @author OD
*/
void SwAnchoredObject::UpdateObjInSortedList()
{
    if ( GetAnchorFrm() )
    {
        if ( GetFrmFmt().GetDoc()->ConsiderWrapOnObjPos() )
        {
            // invalidate position of all anchored objects at anchor frame
            if ( GetAnchorFrm()->GetDrawObjs() )
            {
                const SwSortedObjs* pObjs = GetAnchorFrm()->GetDrawObjs();
                // determine start index
                sal_uInt32 i = 0;
                for ( ; i < pObjs->Count(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                    if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
                        pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                    else
                        pAnchoredObj->InvalidateObjPos();
                }
            }
            // invalidate all following anchored objects on the page frame
            if ( GetPageFrm() && GetPageFrm()->GetSortedObjs() )
            {
                const SwSortedObjs* pObjs = GetPageFrm()->GetSortedObjs();
                // determine start index
                sal_uInt32 i = pObjs->ListPosOf( *this ) + 1;
                for ( ; i < pObjs->Count(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                    if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
                        pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                    else
                        pAnchoredObj->InvalidateObjPos();
                }
            }
        }
        // update its position in the sorted object list of its anchor frame
        AnchorFrm()->GetDrawObjs()->Update( *this );
        // update its position in the sorted object list of its page frame
        // note: as-character anchored object aren't registered at a page frame
        if ( GetFrmFmt().GetAnchor().GetAnchorId() != FLY_IN_CNTNT )
        {
            GetPageFrm()->GetSortedObjs()->Update( *this );
        }
    }
}

/** method to determine, if invalidation of position is allowed

    OD 2004-07-01 #i28701#

    @author OD
*/
bool SwAnchoredObject::InvalidationOfPosAllowed() const
{
    bool bAllowed( true );

    if ( ConsiderObjWrapInfluenceOnObjPos() )
    {
        const SwPageFrm* pPageFrm = GetPageFrm();
        if ( pPageFrm && pPageFrm->IsLayoutInProgress() &&
             PositionLocked() )
        {
            bAllowed = false;
        }
    }

    return bAllowed;
}

/** method to determine the page frame, on which the 'anchor' of
    the given anchored object is.

    OD 2004-07-02 #i28701#

    @author OD
*/
SwPageFrm& SwAnchoredObject::GetPageFrmOfAnchor()
{
    SwPageFrm* pRetPageFrm = 0L;

    const SwFmtAnchor& rAnch = GetFrmFmt().GetAnchor();
    if ( rAnch.GetAnchorId() == FLY_AUTO_CNTNT ||
         rAnch.GetAnchorId() == FLY_IN_CNTNT )
    {
        SwTxtFrm& aAnchorCharFrm = static_cast<SwTxtFrm*>(AnchorFrm())->
                    GetFrmAtOfst( rAnch.GetCntntAnchor()->nContent.GetIndex() );
        pRetPageFrm = aAnchorCharFrm.FindPageFrm();
    }
    else if ( rAnch.GetAnchorId() == FLY_AT_CNTNT ||
              rAnch.GetAnchorId() == FLY_AT_FLY ||
              rAnch.GetAnchorId() == FLY_PAGE )
    {
        pRetPageFrm = AnchorFrm()->FindPageFrm();
    }
    else
    {
        ASSERT( false,
                "<SwAnchoredObject::_GetPageFrmOfAnchor(..)> - unexcepted anchor type" );
    }

    ASSERT( pRetPageFrm,
            "<SwAnchoredObject::_GetPageFrmOfAnchor(..)> - missing page frame -> crash." );
    return *pRetPageFrm;
}

/** method to determine, if a format on the anchored object is possible

    OD 2004-07-23 #i28701#
    A format is possible, if anchored object is in an invisible layer.
    Note: method is virtual to refine the conditions for the sub-classes.

    @author OD
*/
bool SwAnchoredObject::IsFormatPossible() const
{
    return GetFrmFmt().GetDoc()->IsVisibleLayerId( GetDrawObj()->GetLayer() );
}
