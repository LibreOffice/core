/*************************************************************************
 *
 *  $RCSfile: anchoreddrawobject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: od $ $Date: 2004-08-03 06:00:02 $
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
#ifndef _ANCHOREDDRAWOBJECT_HXX
#include <anchoreddrawobject.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _TOCNTNTANCHOROBJECTPOSITION_HXX
#include <tocntntanchoredobjectposition.hxx>
#endif
#ifndef _TOLAYOUTANCHOROBJECTPOSITION_HXX
#include <tolayoutanchoredobjectposition.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif

enum tLayoutDir
{
    HORI_L2R,
    HORI_R2L,
    VERT_R2L,
    VERT_L2R    // not supported yet
};

// ============================================================================
// helper class for correct notification due to the positioning of
// the anchored drawing object
// ============================================================================
class SwPosNotify
{
    private:
        SwAnchoredDrawObject* mpAnchoredDrawObj;
        SwRect maOldObjRect;
        SwPageFrm* mpOldPageFrm;

    public:
        SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj );
        ~SwPosNotify();
};

SwPosNotify::SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj ) :
    mpAnchoredDrawObj( _pAnchoredDrawObj )
{
    maOldObjRect = mpAnchoredDrawObj->GetObjRect();
    mpOldPageFrm = mpAnchoredDrawObj->AnchorFrm()->FindPageFrm();
}

SwPosNotify::~SwPosNotify()
{
    if ( maOldObjRect != mpAnchoredDrawObj->GetObjRect() )
    {
        if( maOldObjRect.HasArea() && mpOldPageFrm )
        {
            mpAnchoredDrawObj->NotifyBackground( mpOldPageFrm, maOldObjRect,
                                                 PREP_FLY_LEAVE );
        }
        SwRect aNewObjRect( mpAnchoredDrawObj->GetObjRect() );
        if( aNewObjRect.HasArea() )
        {
            SwPageFrm* pNewPageFrm = mpAnchoredDrawObj->AnchorFrm()->FindPageFrm();
            if( pNewPageFrm )
                mpAnchoredDrawObj->NotifyBackground( pNewPageFrm, aNewObjRect,
                                                     PREP_FLY_ARRIVE );
        }

        ::ClrContourCache( mpAnchoredDrawObj->GetDrawObj() );

        // indicate a restart of the layout process
        mpAnchoredDrawObj->SetRestartLayoutProcess( true );
    }
    else
    {
        // lock position
        mpAnchoredDrawObj->LockPosition();

        if ( !mpAnchoredDrawObj->ConsiderForTextWrap() )
        {
            // indicate that object has to be considered for text wrap
            mpAnchoredDrawObj->SetConsiderForTextWrap( true );
            // invalidate 'background' in order to allow its 'background'
            // to wrap around it.
            mpAnchoredDrawObj->NotifyBackground( mpAnchoredDrawObj->GetPageFrm(),
                                    mpAnchoredDrawObj->GetObjRectWithSpaces(),
                                    PREP_FLY_ARRIVE );
            // invalidate position of anchor frame in order to force
            // a re-format of the anchor frame, which also causes a
            // re-format of the invalid previous frames of the anchor frame.
            mpAnchoredDrawObj->AnchorFrm()->InvalidatePos();
        }
    }
}

// ============================================================================
// implementation of class <SwAnchoredDrawObject>
// ============================================================================
TYPEINIT1(SwAnchoredDrawObject,SwAnchoredObject);

SwAnchoredDrawObject::SwAnchoredDrawObject() :
    SwAnchoredObject(),
    mbValidPos( false ),
    maLastObjRect(),
    mbNotYetAttachedToAnchorFrame( true )
{
}

SwAnchoredDrawObject::~SwAnchoredDrawObject()
{
}

// =============================================================================
// OD 2004-03-25 #i26791# - implementation of pure virtual method declared in
// base class <SwAnchoredObject>
// =============================================================================
void SwAnchoredDrawObject::MakeObjPos()
{
    if ( IsPositioningInProgress() )
    {
        // nothind to do - positioning already in progress
        return;
    }

    if ( mbValidPos )
    {
        // nothing to do - position is valid
        return;
    }

    // indicate that positioning is in progress
    SwObjPositioningInProgress aObjPosInProgress( *this );

    // indicate that position will be valid after positioning is performed
    mbValidPos = true;

    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(::GetUserCall( GetDrawObj() ));

    // scope for created instance of <SwPosNotify>
    {
        // create instance of <SwPosNotify> for correct notification
        SwPosNotify aPosNotify( this );

        // determine relative position of drawing object and set it
        switch ( pDrawContact->GetAnchorId() )
        {
            case FLY_IN_CNTNT:
            {
                // nothing to do, because as-character anchored objects are positioned
                // during the format of its anchor frame - see <SwFlyCntPortion::SetBase(..)>
            }
            break;
            case FLY_AT_CNTNT:
            case FLY_AUTO_CNTNT:
            {
                objectpositioning::SwToCntntAnchoredObjectPosition
                        aObjPositioning( *DrawObj() );
                aObjPositioning.CalcPosition();
                SetVertPosOrientFrm ( aObjPositioning.GetVertPosOrientFrm() );
                _SetDrawObjAnchor( aObjPositioning.GetOffsetToFrmAnchorPos() );
            }
            break;
            case FLY_PAGE:
            case FLY_AT_FLY:
            {
                objectpositioning::SwToLayoutAnchoredObjectPosition
                        aObjPositioning( *DrawObj() );
                aObjPositioning.CalcPosition();
                Point aOffsetToFrmAnchorPos( aObjPositioning.GetOffsetToFrmAnchorPos() );
                _SetDrawObjAnchor( aOffsetToFrmAnchorPos );
                SetCurrRelPos( aObjPositioning.GetRelPos() );
                SetObjLeft( ( DrawObj()->GetAnchorPos().X() -
                              aOffsetToFrmAnchorPos.X() ) +
                            GetCurrRelPos().X() );
                SetObjTop( ( DrawObj()->GetAnchorPos().Y() -
                             aOffsetToFrmAnchorPos.Y() ) +
                           GetCurrRelPos().Y() );
            }
            break;
            default:
            {
                ASSERT( false, "<SwAnchoredDrawObject::MakeObjPos()> - unknown anchor type - please inform OD." );
            }
        }

        // keep, current object rectangle
        LastObjRect() = GetObjRect().SVRect();
    }

    // Assure for 'master' drawing object, that it's registered at the correct page.
    // Perform check not for as-character anchored drawing objects and only if
    // the anchor frame is valid.
    if ( !GetDrawObj()->ISA(SwDrawVirtObj) &&
         !pDrawContact->ObjAnchoredAsChar() &&
         GetAnchorFrm()->IsValid() )
    {
        pDrawContact->ChkPage();
    }
}

void SwAnchoredDrawObject::_SetDrawObjAnchor( const Point _aOffsetToFrmAnchorPos )
{
    // new anchor position
    Point aNewAnchorPos =
                GetAnchorFrm()->GetFrmAnchorPos( ::HasWrap( GetDrawObj() ) ) +
                _aOffsetToFrmAnchorPos;
    Point aCurrAnchorPos = GetDrawObj()->GetAnchorPos();
    if ( aNewAnchorPos != aCurrAnchorPos )
    {
        // determine movement to be applied after setting the new anchor position
        Size aMove( aCurrAnchorPos.X() - aNewAnchorPos.X(),
                    aCurrAnchorPos.Y() - aNewAnchorPos.Y() );
        // set new anchor position
        DrawObj()->SetAnchorPos( aNewAnchorPos );
        // correct object position, caused by setting new anchor position
        DrawObj()->Move( aMove );
    }
}

/** method to invalidate the given page frame

    OD 2004-07-02 #i28701#

    @author OD
*/
void SwAnchoredDrawObject::_InvalidatePage( SwPageFrm* _pPageFrm )
{
    if ( _pPageFrm && !_pPageFrm->GetFmt()->GetDoc()->IsInDtor() )
    {
        if ( _pPageFrm->GetUpper() )
        {
            _pPageFrm->InvalidateFlyLayout();

            SwRootFrm* pRootFrm = static_cast<SwRootFrm*>(_pPageFrm->GetUpper());
            pRootFrm->DisallowTurbo();
            if ( pRootFrm->GetTurbo() )
            {
                const SwCntntFrm* pTmpFrm = pRootFrm->GetTurbo();
                pRootFrm->ResetTurbo();
                pTmpFrm->InvalidatePage();
            }
            pRootFrm->SetIdleFlags();
        }
    }
}

void SwAnchoredDrawObject::InvalidateObjPos()
{
    // --> OD 2004-07-01 #i28701# - check, if invalidation is allowed
    if ( mbValidPos &&
         InvalidationOfPosAllowed() )
    {
        mbValidPos = false;

        SwPageFrm* pPageFrm = AnchorFrm()->FindPageFrm();
        _InvalidatePage( pPageFrm );
        SwPageFrm* pPageFrmOfAnchor = &GetPageFrmOfAnchor();
        if ( pPageFrmOfAnchor != pPageFrm )
        {
            _InvalidatePage( pPageFrmOfAnchor );
        }
    }
}

SwFrmFmt& SwAnchoredDrawObject::GetFrmFmt()
{
    ASSERT( static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt(),
            "<SwAnchoredDrawObject::GetFrmFmt()> - missing frame format -> crash." );
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt());
}
const SwFrmFmt& SwAnchoredDrawObject::GetFrmFmt() const
{
    ASSERT( static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt(),
            "<SwAnchoredDrawObject::GetFrmFmt()> - missing frame format -> crash." );
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt());
}

const SwRect SwAnchoredDrawObject::GetObjRect() const
{
    // use geometry of drawing object
    //return GetDrawObj()->GetCurrentBoundRect();
    return GetDrawObj()->GetSnapRect();
}
void SwAnchoredDrawObject::SetObjTop( const SwTwips _nTop )
{
    SwTwips nDiff = _nTop - GetObjRect().Top();
    DrawObj()->Move( Size( 0, nDiff ) );
}
void SwAnchoredDrawObject::SetObjLeft( const SwTwips _nLeft )
{
    SwTwips nDiff = _nLeft - GetObjRect().Left();
    DrawObj()->Move( Size( nDiff, 0 ) );
}

/** adjust positioning and alignment attributes for new anchor frame

    @author OD
*/
void SwAnchoredDrawObject::AdjustPositioningAttr( const SwFrm* _pNewAnchorFrm )
{
    SwTwips nHoriRelPos = 0;
    SwTwips nVertRelPos = 0;
    const Point aAnchorPos = _pNewAnchorFrm->GetFrmAnchorPos( ::HasWrap( GetDrawObj() ) );
    const SwRect aObjRect = GetObjRect();
    const bool bVert = _pNewAnchorFrm->IsVertical();
    const bool bR2L = _pNewAnchorFrm->IsRightToLeft();
    if ( bVert )
    {
        nHoriRelPos = aObjRect.Top() - aAnchorPos.Y();
        nVertRelPos = aAnchorPos.X() - aObjRect.Right();
    }
    else if ( bR2L )
    {
        nHoriRelPos = aAnchorPos.X() - aObjRect.Right();
        nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
    }
    else
    {
        nHoriRelPos = aObjRect.Left() - aAnchorPos.X();
        nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
    }

    GetFrmFmt().SetAttr( SwFmtHoriOrient( nHoriRelPos, HORI_NONE, FRAME ) );
    GetFrmFmt().SetAttr( SwFmtVertOrient( nVertRelPos, VERT_NONE, FRAME ) );
}

const Rectangle& SwAnchoredDrawObject::GetLastObjRect() const
{
    return maLastObjRect;
}

Rectangle& SwAnchoredDrawObject::LastObjRect()
{
    return maLastObjRect;
}

void SwAnchoredDrawObject::ObjectAttachedToAnchorFrame()
{
    if ( mbNotYetAttachedToAnchorFrame )
    {
        mbNotYetAttachedToAnchorFrame = false;

        // convert positioning attributes, if needed
        const SwFrm* pAnchorFrm = GetAnchorFrm();
        ASSERT( pAnchorFrm,
                "<SwAnchoredDrawObject::ObjectAttachedToAnchorFrame()> - missing anchor frame" );
        const bool bVert = pAnchorFrm->IsVertical();
        const bool bR2L = pAnchorFrm->IsRightToLeft();
        tLayoutDir nLayoutDir = HORI_L2R;
        if ( bVert )
        {
            nLayoutDir = VERT_R2L;
        }
        else if ( bR2L )
        {
            nLayoutDir = HORI_R2L;
        }
        if ( nLayoutDir != HORI_L2R )
        {
            SwRect aObjRect( GetObjRect() );
            const SwFmtHoriOrient& rHori = GetFrmFmt().GetHoriOrient();
            SwTwips nCurrHoriPos( rHori.GetPos() );
            const SwFmtVertOrient& rVert = GetFrmFmt().GetVertOrient();
            SwTwips nCurrVertPos( rVert.GetPos() );
            if ( rHori.GetHoriOrient() == HORI_NONE )
            {
                SwTwips nNewHoriPos( nCurrHoriPos );
                switch ( nLayoutDir )
                {
                    case HORI_R2L:
                    {
                        nNewHoriPos = -rHori.GetPos() - aObjRect.Width();
                    }
                    break;
                    case VERT_R2L:
                    {
                        nNewHoriPos = nCurrVertPos;
                    }
                    break;
                }
                if ( nNewHoriPos != nCurrHoriPos )
                {
                    GetFrmFmt().SetAttr( SwFmtHoriOrient( nNewHoriPos,
                                                  rHori.GetHoriOrient(),
                                                  rHori.GetRelationOrient() ) );
                }
            }
            if ( rVert.GetVertOrient() == VERT_NONE )
            {
                SwTwips nNewVertPos( nCurrVertPos );
                switch ( nLayoutDir )
                {
                    case VERT_R2L:
                    {
                        nNewVertPos = -nCurrHoriPos - aObjRect.Width();
                    }
                    break;
                }
                if ( nNewVertPos != nCurrVertPos )
                {
                    GetFrmFmt().SetAttr( SwFmtVertOrient( nNewVertPos,
                                                  rVert.GetVertOrient(),
                                                  rVert.GetRelationOrient() ) );
                }
            }
        }
    }
}

void SwAnchoredDrawObject::_SetPositioningAttr()
{
    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(GetUserCall( GetDrawObj() ));

    if ( !pDrawContact->ObjAnchoredAsChar() )
    {
        SwRect aObjRect( GetObjRect() );

        const SwFmtHoriOrient& rHori = GetFrmFmt().GetHoriOrient();
        GetFrmFmt().SetAttr( SwFmtHoriOrient( GetObjRect().Left(),
                                              rHori.GetHoriOrient(),
                                              rHori.GetRelationOrient() ) );

        const SwFmtVertOrient& rVert = GetFrmFmt().GetVertOrient();
        SwTwips nVertPos = GetObjRect().Top();
        if ( rVert.GetRelationOrient() == REL_CHAR ||
             rVert.GetRelationOrient() == REL_VERT_LINE )
        {
            nVertPos = -nVertPos;
        }
        GetFrmFmt().SetAttr( SwFmtVertOrient( nVertPos,
                                              rVert.GetVertOrient(),
                                              rVert.GetRelationOrient() ) );
    }
}

void SwAnchoredDrawObject::NotifyBackground( SwPageFrm* _pPageFrm,
                                             const SwRect& _rRect,
                                             PrepareHint _eHint )
{
    ::Notify_Background( GetDrawObj(), _pPageFrm, _rRect, _eHint, TRUE );
}

/** method to assure that anchored object is registered at the correct
    page frame

    OD 2004-07-02 #i28701#

    @author OD
*/
void SwAnchoredDrawObject::RegisterAtCorrectPage()
{
    SwPageFrm* pPageFrm( 0L );
    if ( GetVertPosOrientFrm() )
    {
        pPageFrm = const_cast<SwPageFrm*>(GetVertPosOrientFrm()->FindPageFrm());
    }
    if ( pPageFrm && GetPageFrm() != pPageFrm )
    {
        if ( GetPageFrm() )
            GetPageFrm()->RemoveDrawObjFromPage( *this );
        pPageFrm->AppendDrawObjToPage( *this );
    }
}

// =============================================================================
