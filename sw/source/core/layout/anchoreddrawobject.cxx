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

#include <dcontact.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <tocntntanchoredobjectposition.hxx>
#include <tolayoutanchoredobjectposition.hxx>
#include <frmtool.hxx>
#include <fmtornt.hxx>

#include <txtfrm.hxx>

#include <vector>
#include <svx/svdogrp.hxx>

using namespace ::com::sun::star;



class SwPosNotify
{
    private:
        SwAnchoredDrawObject* mpAnchoredDrawObj;
        SwRect maOldObjRect;
        SwPageFrm* mpOldPageFrm;

    public:
        SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj );
        ~SwPosNotify();
        
        Point LastObjPos() const;
};

SwPosNotify::SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj ) :
    mpAnchoredDrawObj( _pAnchoredDrawObj )
{
    maOldObjRect = mpAnchoredDrawObj->GetObjRect();
    
    mpOldPageFrm = mpAnchoredDrawObj->GetPageFrm();
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
            
            SwPageFrm* pNewPageFrm = mpAnchoredDrawObj->GetPageFrm();
            if( pNewPageFrm )
                mpAnchoredDrawObj->NotifyBackground( pNewPageFrm, aNewObjRect,
                                                     PREP_FLY_ARRIVE );
        }

        ::ClrContourCache( mpAnchoredDrawObj->GetDrawObj() );

        
        
        
        
        
        if ( mpAnchoredDrawObj->GetAnchorFrm()->IsTxtFrm() &&
             mpOldPageFrm == mpAnchoredDrawObj->GetAnchorFrm()->FindPageFrm() )
        {
            mpAnchoredDrawObj->AnchorFrm()->Prepare( PREP_FLY_LEAVE );
        }

        
        mpAnchoredDrawObj->SetRestartLayoutProcess( true );
    }
    else
    {
        
        mpAnchoredDrawObj->LockPosition();

        if ( !mpAnchoredDrawObj->ConsiderForTextWrap() )
        {
            
            mpAnchoredDrawObj->SetConsiderForTextWrap( true );
            
            
            mpAnchoredDrawObj->NotifyBackground( mpAnchoredDrawObj->GetPageFrm(),
                                    mpAnchoredDrawObj->GetObjRectWithSpaces(),
                                    PREP_FLY_ARRIVE );
            
            
            
            mpAnchoredDrawObj->AnchorFrm()->InvalidatePos();
        }
    }
}


Point SwPosNotify::LastObjPos() const
{
    return maOldObjRect.Pos();
}



class SwObjPosOscillationControl
{
    private:
        sal_uInt8 mnPosStackSize;

        const SwAnchoredDrawObject* mpAnchoredDrawObj;

        std::vector<Point*> maObjPositions;

    public:
        SwObjPosOscillationControl( const SwAnchoredDrawObject& _rAnchoredDrawObj );
        ~SwObjPosOscillationControl();

        bool OscillationDetected();
};

SwObjPosOscillationControl::SwObjPosOscillationControl(
                                const SwAnchoredDrawObject& _rAnchoredDrawObj )
    : mnPosStackSize( 20 ),
      mpAnchoredDrawObj( &_rAnchoredDrawObj )
{
}

SwObjPosOscillationControl::~SwObjPosOscillationControl()
{
    while ( !maObjPositions.empty() )
    {
        Point* pPos = maObjPositions.back();
        delete pPos;

        maObjPositions.pop_back();
    }
}

bool SwObjPosOscillationControl::OscillationDetected()
{
    bool bOscillationDetected = false;

    if ( maObjPositions.size() == mnPosStackSize )
    {
        
        bOscillationDetected = true;
    }
    else
    {
        Point* pNewObjPos = new Point( mpAnchoredDrawObj->GetObjRect().Pos() );
        for ( std::vector<Point*>::iterator aObjPosIter = maObjPositions.begin();
              aObjPosIter != maObjPositions.end();
              ++aObjPosIter )
        {
            if ( *(pNewObjPos) == *(*aObjPosIter) )
            {
                
                bOscillationDetected = true;
                delete pNewObjPos;
                break;
            }
        }
        if ( !bOscillationDetected )
        {
            maObjPositions.push_back( pNewObjPos );
        }
    }

    return bOscillationDetected;
}



TYPEINIT1(SwAnchoredDrawObject,SwAnchoredObject);

SwAnchoredDrawObject::SwAnchoredDrawObject() :
    SwAnchoredObject(),
    mbValidPos( false ),
    
    mpLastObjRect( 0L ),
    mbNotYetAttachedToAnchorFrame( true ),
    
    mbNotYetPositioned( true ),
    
    mbCaptureAfterLayoutDirChange( false )
{
}

SwAnchoredDrawObject::~SwAnchoredDrawObject()
{
    
    delete mpLastObjRect;
}


void SwAnchoredDrawObject::UpdateLayoutDir()
{
    SwFrmFmt::tLayoutDir nOldLayoutDir( GetFrmFmt().GetLayoutDir() );

    SwAnchoredObject::UpdateLayoutDir();

    if ( !NotYetPositioned() &&
         GetFrmFmt().GetLayoutDir() != nOldLayoutDir &&
         GetFrmFmt().GetDoc()->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         !IsOutsidePage() )
    {
        mbCaptureAfterLayoutDirChange = true;
    }
}


bool SwAnchoredDrawObject::IsOutsidePage() const
{
    bool bOutsidePage( false );

    if ( !NotYetPositioned() && GetPageFrm() )
    {
        SwRect aTmpRect( GetObjRect() );
        bOutsidePage =
            ( aTmpRect.Intersection( GetPageFrm()->Frm() ) != GetObjRect() );
    }

    return bOutsidePage;
}

void SwAnchoredDrawObject::MakeObjPos()
{
    if ( IsPositioningInProgress() )
    {
        
        return;
    }

    if ( mbValidPos )
    {
        
        return;
    }

    
    
    if ( mbNotYetAttachedToAnchorFrame )
    {
        OSL_FAIL( "<SwAnchoredDrawObject::MakeObjPos() - drawing object not yet attached to anchor frame -> no positioning" );
        return;
    }

    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(::GetUserCall( GetDrawObj() ));

    
    
    
    
    
    
    
    
    if ( mbNotYetPositioned )
    {
        
        pDrawContact->MoveObjToVisibleLayer( DrawObj() );
        
        
        
        
        if ( !GetDrawObj()->ISA(SwDrawVirtObj) &&
             !static_cast<SwDrawFrmFmt&>(GetFrmFmt()).IsPosAttrSet() )
        {
            _SetPositioningAttr();
        }
        
        
        
        mbNotYetPositioned = false;
    }

    
    {
        SwObjPositioningInProgress aObjPosInProgress( *this );

        
        switch ( pDrawContact->GetAnchorId() )
        {
            case FLY_AS_CHAR:
            {
                
                mbValidPos = true;
                
                
            }
            break;
            case FLY_AT_PARA:
            case FLY_AT_CHAR:
            {
                
                
                _MakeObjPosAnchoredAtPara();
            }
            break;
            case FLY_AT_PAGE:
            case FLY_AT_FLY:
            {
                
                
                _MakeObjPosAnchoredAtLayout();
            }
            break;
            default:
            {
                OSL_FAIL( "<SwAnchoredDrawObject::MakeObjPos()> - unknown anchor type - please inform OD." );
            }
        }

        
        
        SetLastObjRect( GetObjRect().SVRect() );

        
        
        
        if ( !GetDrawObj()->ISA(SwDrawVirtObj) &&
             !pDrawContact->ObjAnchoredAsChar() &&
             GetAnchorFrm()->IsValid() )
        {
            pDrawContact->ChkPage();
        }
    }

    
    if ( mbCaptureAfterLayoutDirChange &&
         GetPageFrm() )
    {
        SwRect aPageRect( GetPageFrm()->Frm() );
        SwRect aObjRect( GetObjRect() );
        if ( aObjRect.Right() >= aPageRect.Right() + 10 )
        {
            Size aSize( aPageRect.Right() - aObjRect.Right(), 0 );
            DrawObj()->Move( aSize );
            aObjRect = GetObjRect();
        }

        if ( aObjRect.Left() + 10 <= aPageRect.Left() )
        {
            Size aSize( aPageRect.Left() - aObjRect.Left(), 0 );
            DrawObj()->Move( aSize );
        }

        mbCaptureAfterLayoutDirChange = false;
    }
}

/** method for the intrinsic positioning of a at-paragraph|at-character
    anchored drawing object

    #i32795# - helper method for method <MakeObjPos>
*/
void SwAnchoredDrawObject::_MakeObjPosAnchoredAtPara()
{
    
    

    
    
    
    
    
    
    
    
    
    const bool bFormatAnchor =
            !static_cast<const SwTxtFrm*>( GetAnchorFrmContainingAnchPos() )->IsAnyJoinLocked() &&
            !ConsiderObjWrapInfluenceOnObjPos() &&
            !ConsiderObjWrapInfluenceOfOtherObjs();

    if ( bFormatAnchor )
    {
        
        GetAnchorFrmContainingAnchPos()->Calc();
    }

    bool bOscillationDetected = false;
    SwObjPosOscillationControl aObjPosOscCtrl( *this );
    
    
    
    bool bConsiderWrapInfluenceDueToOverlapPrevCol( false );
    do {
        
        mbValidPos = true;

        
        {
            
            SwPosNotify aPosNotify( this );

            
            objectpositioning::SwToCntntAnchoredObjectPosition
                    aObjPositioning( *DrawObj() );
            aObjPositioning.CalcPosition();

            
            SetVertPosOrientFrm ( aObjPositioning.GetVertPosOrientFrm() );
            _SetDrawObjAnchor();

            
            if ( GetObjRect().Pos() != aPosNotify.LastObjPos() )
            {
                bOscillationDetected = aObjPosOscCtrl.OscillationDetected();
            }
        }
        
        
        
        if ( bFormatAnchor )
        {
            
            GetAnchorFrmContainingAnchPos()->Calc();
        }

        
        if ( !ConsiderObjWrapInfluenceOnObjPos() &&
             OverlapsPrevColumn() )
        {
            bConsiderWrapInfluenceDueToOverlapPrevCol = true;
        }
    } while ( !mbValidPos && !bOscillationDetected &&
              !bConsiderWrapInfluenceDueToOverlapPrevCol );

    
    
    
    if ( bOscillationDetected || bConsiderWrapInfluenceDueToOverlapPrevCol )
    {
        SetTmpConsiderWrapInfluence( true );
        SetRestartLayoutProcess( true );
    }
}

/** method for the intrinsic positioning of a at-page|at-frame anchored
    drawing object

    #i32795# - helper method for method <MakeObjPos>
*/
void SwAnchoredDrawObject::_MakeObjPosAnchoredAtLayout()
{
    
    mbValidPos = true;

    
    SwPosNotify aPosNotify( this );

    
    objectpositioning::SwToLayoutAnchoredObjectPosition
            aObjPositioning( *DrawObj() );
    aObjPositioning.CalcPosition();

    

    
    
    
    {
        const Point aNewAnchorPos =
                    GetAnchorFrm()->GetFrmAnchorPos( ::HasWrap( GetDrawObj() ) );
        DrawObj()->SetAnchorPos( aNewAnchorPos );
        
        InvalidateObjRectWithSpaces();
    }
    SetCurrRelPos( aObjPositioning.GetRelPos() );
    const SwFrm* pAnchorFrm = GetAnchorFrm();
    SWRECTFN( pAnchorFrm );
    const Point aAnchPos( (pAnchorFrm->Frm().*fnRect->fnGetPos)() );
    SetObjLeft( aAnchPos.X() + GetCurrRelPos().X() );
    SetObjTop( aAnchPos.Y() + GetCurrRelPos().Y() );
}

void SwAnchoredDrawObject::_SetDrawObjAnchor()
{
    
    
    Point aNewAnchorPos =
                GetAnchorFrm()->GetFrmAnchorPos( ::HasWrap( GetDrawObj() ) );
    Point aCurrAnchorPos = GetDrawObj()->GetAnchorPos();
    if ( aNewAnchorPos != aCurrAnchorPos )
    {
        
        Size aMove( aCurrAnchorPos.getX() - aNewAnchorPos.getX(),
                    aCurrAnchorPos.getY() - aNewAnchorPos.getY() );
        
        DrawObj()->SetAnchorPos( aNewAnchorPos );
        
        DrawObj()->Move( aMove );
        
        InvalidateObjRectWithSpaces();
    }
}

/** method to invalidate the given page frame

    #i28701#
*/
void SwAnchoredDrawObject::_InvalidatePage( SwPageFrm* _pPageFrm )
{
    if ( _pPageFrm && !_pPageFrm->GetFmt()->GetDoc()->IsInDtor() )
    {
        if ( _pPageFrm->GetUpper() )
        {
            
            
            if ( GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR )
            {
                _pPageFrm->InvalidateFlyInCnt();
            }
            else
            {
                _pPageFrm->InvalidateFlyLayout();
            }

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
    
    if ( mbValidPos &&
         InvalidationOfPosAllowed() )
    {
        mbValidPos = false;
        
        InvalidateObjRectWithSpaces();

        
        if ( GetAnchorFrm() )
        {
            
            
            
            
            if ( GetAnchorFrm()->ISA(SwTxtFrm) &&
                 (GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
            {
                SwTxtFrm* pAnchorTxtFrm( static_cast<SwTxtFrm*>(AnchorFrm()) );
                if ( pAnchorTxtFrm->GetTxtNode()->GetpSwpHints() &&
                     pAnchorTxtFrm->CalcFlyPos( &GetFrmFmt() ) != COMPLETE_STRING )
                {
                    AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG, &GetFrmFmt() );
                }
            }

            SwPageFrm* pPageFrm = AnchorFrm()->FindPageFrm();
            _InvalidatePage( pPageFrm );

            
            
            SwPageFrm* pPageFrmRegisteredAt = GetPageFrm();
            if ( pPageFrmRegisteredAt &&
                 pPageFrmRegisteredAt != pPageFrm )
            {
                _InvalidatePage( pPageFrmRegisteredAt );
            }
            
            
            
            SwPageFrm* pPageFrmOfAnchor = FindPageFrmOfAnchor();
            if ( pPageFrmOfAnchor &&
                 pPageFrmOfAnchor != pPageFrm &&
                 pPageFrmOfAnchor != pPageFrmRegisteredAt )
            {
                _InvalidatePage( pPageFrmOfAnchor );
            }
        }
    }
}

SwFrmFmt& SwAnchoredDrawObject::GetFrmFmt()
{
    OSL_ENSURE( static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt(),
            "<SwAnchoredDrawObject::GetFrmFmt()> - missing frame format -> crash." );
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt());
}
const SwFrmFmt& SwAnchoredDrawObject::GetFrmFmt() const
{
    OSL_ENSURE( static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt(),
            "<SwAnchoredDrawObject::GetFrmFmt()> - missing frame format -> crash." );
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt());
}

const SwRect SwAnchoredDrawObject::GetObjRect() const
{
    
    
    return GetDrawObj()->GetSnapRect();
}


const SwRect SwAnchoredDrawObject::GetObjBoundRect() const
{
    bool bGroupShape = PTR_CAST(SdrObjGroup, GetDrawObj());
    
    if ( !bGroupShape && GetPageFrm( ) && ( GetDrawObj( )->GetRelativeWidth( ) || GetDrawObj()->GetRelativeHeight( ) ) )
    {
        Rectangle aCurrObjRect = GetDrawObj()->GetCurrentBoundRect();

        long nTargetWidth = aCurrObjRect.GetWidth( );
        if ( GetDrawObj( )->GetRelativeWidth( ) )
        {
            Rectangle aPageRect;
            if (GetDrawObj()->GetRelativeWidthRelation() == text::RelOrientation::FRAME)
                
                aPageRect = GetPageFrm()->Prt().SVRect();
            else
                aPageRect = GetPageFrm( )->GetBoundRect( ).SVRect();
            nTargetWidth = aPageRect.GetWidth( ) * GetDrawObj( )->GetRelativeWidth( ).get( );
        }

        long nTargetHeight = aCurrObjRect.GetHeight( );
        if ( GetDrawObj( )->GetRelativeHeight( ) )
        {
            Rectangle aPageRect;
            if (GetDrawObj()->GetRelativeHeightRelation() == text::RelOrientation::FRAME)
                
                aPageRect = GetPageFrm()->Prt().SVRect();
            else
                aPageRect = GetPageFrm( )->GetBoundRect( ).SVRect();
            nTargetHeight = aPageRect.GetHeight( ) * GetDrawObj( )->GetRelativeHeight( ).get( );
        }

        if ( nTargetWidth != aCurrObjRect.GetWidth( ) || nTargetHeight != aCurrObjRect.GetHeight( ) )
        {
            const_cast< SdrObject* >( GetDrawObj() )->Resize( aCurrObjRect.TopLeft(),
                    Fraction( nTargetWidth, aCurrObjRect.GetWidth() ),
                    Fraction( nTargetHeight, aCurrObjRect.GetHeight() ), false );
        }
    }
    return GetDrawObj()->GetCurrentBoundRect();
}


bool SwAnchoredDrawObject::_SetObjTop( const SwTwips _nTop )
{
    SwTwips nDiff = _nTop - GetObjRect().Top();
    DrawObj()->Move( Size( 0, nDiff ) );

    return nDiff != 0;
}
bool SwAnchoredDrawObject::_SetObjLeft( const SwTwips _nLeft )
{
    SwTwips nDiff = _nLeft - GetObjRect().Left();
    DrawObj()->Move( Size( nDiff, 0 ) );

    return nDiff != 0;
}

/** adjust positioning and alignment attributes for new anchor frame

    #i33313# - add second optional parameter <_pNewObjRect>
*/
void SwAnchoredDrawObject::AdjustPositioningAttr( const SwFrm* _pNewAnchorFrm,
                                                  const SwRect* _pNewObjRect )
{
    SwTwips nHoriRelPos = 0;
    SwTwips nVertRelPos = 0;
    const Point aAnchorPos = _pNewAnchorFrm->GetFrmAnchorPos( ::HasWrap( GetDrawObj() ) );
    
    const SwRect aObjRect( _pNewObjRect ? *_pNewObjRect : GetObjRect() );
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

    GetFrmFmt().SetFmtAttr( SwFmtHoriOrient( nHoriRelPos, text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
    GetFrmFmt().SetFmtAttr( SwFmtVertOrient( nVertRelPos, text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
}


const Rectangle* SwAnchoredDrawObject::GetLastObjRect() const
{
    return mpLastObjRect;
}



void SwAnchoredDrawObject::SetLastObjRect( const Rectangle& _rNewLastRect )
{
    if ( !mpLastObjRect )
    {
        mpLastObjRect = new Rectangle;
    }
    *(mpLastObjRect) = _rNewLastRect;
}

void SwAnchoredDrawObject::ObjectAttachedToAnchorFrame()
{
    
    SwAnchoredObject::ObjectAttachedToAnchorFrame();

    if ( mbNotYetAttachedToAnchorFrame )
    {
        mbNotYetAttachedToAnchorFrame = false;
    }
}

/** method to set positioning attributes

    #i35798#
    During load the positioning attributes aren't set.
    Thus, the positioning attributes are set by the current object geometry.
    This method is also used for the conversion for drawing objects
    (not anchored as-character) imported from OpenOffice.org file format
    once and directly before the first positioning.
*/
void SwAnchoredDrawObject::_SetPositioningAttr()
{
    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(GetUserCall( GetDrawObj() ));

    if ( !pDrawContact->ObjAnchoredAsChar() )
    {
        SwRect aObjRect( GetObjRect() );

        SwTwips nHoriPos = aObjRect.Left();
        SwTwips nVertPos = aObjRect.Top();
        
        
        if ( GetFrmFmt().GetPositionLayoutDir() ==
                text::PositionLayoutDir::PositionInHoriL2R )
        {
            SwFrmFmt::tLayoutDir eLayoutDir = GetFrmFmt().GetLayoutDir();
            switch ( eLayoutDir )
            {
                case SwFrmFmt::HORI_L2R:
                {
                    
                }
                break;
                case SwFrmFmt::HORI_R2L:
                {
                    nHoriPos = -aObjRect.Left() - aObjRect.Width();
                }
                break;
                case SwFrmFmt::VERT_R2L:
                {
                    nHoriPos = aObjRect.Top();
                    nVertPos = -aObjRect.Left() - aObjRect.Width();
                }
                break;
                default:
                {
                    OSL_FAIL( "<SwAnchoredDrawObject::_SetPositioningAttr()> - unsupported layout direction" );
                }
            }
        }

        
        
        SwFmtHoriOrient aHori( GetFrmFmt().GetHoriOrient() );
        aHori.SetPos( nHoriPos );
        GetFrmFmt().SetFmtAttr( aHori );

        SwFmtVertOrient aVert( GetFrmFmt().GetVertOrient() );

        aVert.SetPos( nVertPos );
        GetFrmFmt().SetFmtAttr( aVert );

        
        GetFrmFmt().SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
    }
    
    
    
    static_cast<SwDrawFrmFmt&>(GetFrmFmt()).PosAttrSet();
}

void SwAnchoredDrawObject::NotifyBackground( SwPageFrm* _pPageFrm,
                                             const SwRect& _rRect,
                                             PrepareHint _eHint )
{
    ::Notify_Background( GetDrawObj(), _pPageFrm, _rRect, _eHint, sal_True );
}

/** method to assure that anchored object is registered at the correct
    page frame

    #i28701#
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
