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

#include "hintids.hxx"
#include <editeng/protitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/fmmodel.hxx>
#include <sot/exchange.hxx>
#include <svx/sdrundomanager.hxx>
#include <editeng/outliner.hxx>
#include <com/sun/star/embed/EmbedMisc.hpp>

#include "swtypes.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"
#include "frmfmt.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmatr.hxx"
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "dview.hxx"
#include "dpage.hxx"
#include "doc.hxx"
#include "mdiexp.hxx"
#include <ndole.hxx>
#include <fmtanchr.hxx>
#include "shellres.hxx"
#include <IDocumentUndoRedo.hxx>


#include <com/sun/star/embed/Aspects.hpp>

#include <vector>

#include <sortedobjs.hxx>
#include <flyfrms.hxx>
#include <UndoManager.hxx>

using namespace com::sun::star;

class SwSdrHdl : public SdrHdl
{
public:
    SwSdrHdl(const Point& rPnt, bool bTopRight ) :
        SdrHdl( rPnt, bTopRight ? HDL_ANCHOR_TR : HDL_ANCHOR ) {}
    virtual sal_Bool IsFocusHdl() const;
};

sal_Bool SwSdrHdl::IsFocusHdl() const
{
    if( HDL_ANCHOR == eKind || HDL_ANCHOR_TR == eKind )
        return sal_True;
    return SdrHdl::IsFocusHdl();
}

static const SwFrm *lcl_FindAnchor( const SdrObject *pObj, sal_Bool bAll )
{
    const SwVirtFlyDrawObj *pVirt = pObj->ISA(SwVirtFlyDrawObj) ?
                                            (SwVirtFlyDrawObj*)pObj : 0;
    if ( pVirt )
    {
        if ( bAll || !pVirt->GetFlyFrm()->IsFlyInCntFrm() )
            return pVirt->GetFlyFrm()->GetAnchorFrm();
    }
    else
    {
        const SwDrawContact *pCont = (const SwDrawContact*)GetUserCall(pObj);
        if ( pCont )
            return pCont->GetAnchorFrm( pObj );
    }
    return 0;
}



SwDrawView::SwDrawView( SwViewImp &rI, SdrModel *pMd, OutputDevice *pOutDev) :
    FmFormView( (FmFormModel*)pMd, pOutDev ),
    rImp( rI )
{
    SetPageVisible( false );
    SetBordVisible( false );
    SetGridVisible( false );
    SetHlplVisible( false );
    SetGlueVisible( false );
    SetFrameDragSingles( sal_True );
    SetVirtualObjectBundling( sal_True );
    SetSwapAsynchron( sal_True );

    EnableExtendedKeyInputDispatcher( sal_False );
    EnableExtendedMouseEventDispatcher( sal_False );
    EnableExtendedCommandEventDispatcher( sal_False );

    SetHitTolerancePixel( GetMarkHdlSizePixel()/2 );

    SetPrintPreview( rI.GetShell()->IsPreview() );

    
    SetBufferedOverlayAllowed(getOptionsDrawinglayer().IsOverlayBuffer_Writer());

    
    SetBufferedOutputAllowed(getOptionsDrawinglayer().IsPaintBuffer_Writer());
}


sal_Bool SwDrawView::IsAntiAliasing() const
{
    return getOptionsDrawinglayer().IsAntiAliasing();
}

SdrObject* impLocalHitCorrection(SdrObject* pRetval, const Point& rPnt, sal_uInt16 nTol, const SdrMarkList &rMrkList)
{
    if(!nTol)
    {
        
        
    }
    else
    {
        
        
        
        
        
        
        SwVirtFlyDrawObj* pSwVirtFlyDrawObj = dynamic_cast< SwVirtFlyDrawObj* >(pRetval);

        if(pSwVirtFlyDrawObj)
        {
            if(pSwVirtFlyDrawObj->GetFlyFrm()->Lower() && pSwVirtFlyDrawObj->GetFlyFrm()->Lower()->IsNoTxtFrm())
            {
                
                
            }
            else
            {
                
                const sal_uInt32 nMarkCount(rMrkList.GetMarkCount());
                bool bSelected(false);

                for(sal_uInt32 a(0); !bSelected && a < nMarkCount; a++)
                {
                    if(pSwVirtFlyDrawObj == rMrkList.GetMark(a)->GetMarkedSdrObj())
                    {
                        bSelected = true;
                    }
                }

                if(!bSelected)
                {
                    
                    
                    basegfx::B2DRange aInnerBound(pSwVirtFlyDrawObj->getInnerBound());

                    aInnerBound.grow(-1.0 * nTol);

                    if(aInnerBound.isInside(basegfx::B2DPoint(rPnt.X(), rPnt.Y())))
                    {
                        
                        pRetval = 0;
                    }
                }
            }
        }
    }

    return pRetval;
}

SdrObject* SwDrawView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, sal_uLong nOptions, const SetOfByte* pMVisLay) const
{
    
    SdrObject* pRetval = FmFormView::CheckSingleSdrObjectHit(rPnt, nTol, pObj, pPV, nOptions, pMVisLay);

    if(pRetval)
    {
        
        pRetval = impLocalHitCorrection(pRetval, rPnt, nTol, GetMarkedObjectList());
    }

    return pRetval;
}


void SwDrawView::AddCustomHdl()
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();

    if(rMrkList.GetMarkCount() != 1 || !GetUserCall(rMrkList.GetMark( 0 )->GetMarkedSdrObj()))
        return;

    SdrObject *pObj = rMrkList.GetMark(0)->GetMarkedSdrObj();
    
    SwFrmFmt* pFrmFmt( ::FindFrmFmt( pObj ) );
    if ( !pFrmFmt )
    {
        OSL_FAIL( "<SwDrawView::AddCustomHdl()> - missing frame format!" );
        return;
    }
    const SwFmtAnchor &rAnchor = pFrmFmt->GetAnchor();

    if (FLY_AS_CHAR == rAnchor.GetAnchorId())
        return;

    const SwFrm* pAnch;
    if(0 == (pAnch = CalcAnchor()))
        return;

    Point aPos(aAnchorPoint);

    if ( FLY_AT_CHAR == rAnchor.GetAnchorId() )
    {
        
        
        SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
        SwRect aAutoPos = pAnchoredObj->GetLastCharRect();
        if ( aAutoPos.Height() )
        {
            aPos = aAutoPos.Pos();
        }
    }

    
    
    aHdl.AddHdl( new SwSdrHdl( aPos, ( pAnch->IsVertical() && !pAnch->IsVertLR() ) ||
                                     pAnch->IsRightToLeft() ) );
}

SdrObject* SwDrawView::GetMaxToTopObj( SdrObject* pObj ) const
{
    if ( GetUserCall(pObj) )
    {
        const SwFrm *pAnch = ::lcl_FindAnchor( pObj, sal_False );
        if ( pAnch )
        {
            
            const SwFlyFrm *pFly = pAnch->FindFlyFrm();
            if ( pFly )
            {
                const SwPageFrm *pPage = pFly->FindPageFrm();
                if ( pPage->GetSortedObjs() )
                {
                    sal_uInt32 nOrdNum = 0;
                    for ( sal_uInt16 i = 0; i < pPage->GetSortedObjs()->Count(); ++i )
                    {
                        const SdrObject *pO =
                                    (*pPage->GetSortedObjs())[i]->GetDrawObj();

                        if ( pO->GetOrdNumDirect() > nOrdNum )
                        {
                            const SwFrm *pTmpAnch = ::lcl_FindAnchor( pO, sal_False );
                            if ( pFly->IsAnLower( pTmpAnch ) )
                            {
                                nOrdNum = pO->GetOrdNumDirect();
                            }
                        }
                    }
                    if ( nOrdNum )
                    {
                        SdrPage *pTmpPage = GetModel()->GetPage( 0 );
                        ++nOrdNum;
                        if ( nOrdNum < pTmpPage->GetObjCount() )
                        {
                            return pTmpPage->GetObj( nOrdNum );
                        }
                    }
                }
            }
        }
    }
    return 0;
}

SdrObject* SwDrawView::GetMaxToBtmObj(SdrObject* pObj) const
{
    if ( GetUserCall(pObj) )
    {
        const SwFrm *pAnch = ::lcl_FindAnchor( pObj, sal_False );
        if ( pAnch )
        {
            
            const SwFlyFrm *pFly = pAnch->FindFlyFrm();
            if ( pFly )
            {
                SdrObject *pRet = (SdrObject*)pFly->GetVirtDrawObj();
                return pRet != pObj ? pRet : 0;
            }
        }
    }
    return 0;
}


sal_uInt32 SwDrawView::_GetMaxChildOrdNum( const SwFlyFrm& _rParentObj,
                                           const SdrObject* _pExclChildObj ) const
{
    sal_uInt32 nMaxChildOrdNum = _rParentObj.GetDrawObj()->GetOrdNum();

    const SdrPage* pDrawPage = _rParentObj.GetDrawObj()->GetPage();
    OSL_ENSURE( pDrawPage,
            "<SwDrawView::_GetMaxChildOrdNum(..) - missing drawing page at parent object - crash!" );

    sal_uInt32 nObjCount = pDrawPage->GetObjCount();
    for ( sal_uInt32 i = nObjCount-1; i > _rParentObj.GetDrawObj()->GetOrdNum() ; --i )
    {
        const SdrObject* pObj = pDrawPage->GetObj( i );

        
        if ( pObj == _pExclChildObj )
        {
            continue;
        }

        if ( pObj->GetOrdNum() > nMaxChildOrdNum &&
             _rParentObj.IsAnLower( lcl_FindAnchor( pObj, sal_True ) ) )
        {
            nMaxChildOrdNum = pObj->GetOrdNum();
            break;
        }
    }

    return nMaxChildOrdNum;
}


void SwDrawView::_MoveRepeatedObjs( const SwAnchoredObject& _rMovedAnchoredObj,
                                    const std::vector<SdrObject*>& _rMovedChildObjs ) const
{
    
    std::list<SwAnchoredObject*> aAnchoredObjs;
    {
        const SwContact* pContact = ::GetUserCall( _rMovedAnchoredObj.GetDrawObj() );
        OSL_ENSURE( pContact,
                "SwDrawView::_MoveRepeatedObjs(..) - missing contact object -> crash." );
        pContact->GetAnchoredObjs( aAnchoredObjs );
    }

    
    if ( aAnchoredObjs.size() > 1 )
    {
        SdrPage* pDrawPage = GetModel()->GetPage( 0 );

        
        sal_uInt32 nNewPos = _rMovedAnchoredObj.GetDrawObj()->GetOrdNum();
        while ( !aAnchoredObjs.empty() )
        {
            SwAnchoredObject* pAnchoredObj = aAnchoredObjs.back();
            if ( pAnchoredObj != &_rMovedAnchoredObj )
            {
                pDrawPage->SetObjectOrdNum( pAnchoredObj->GetDrawObj()->GetOrdNum(),
                                            nNewPos );
                pDrawPage->RecalcObjOrdNums();
                
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pTmpFlyFrm = static_cast<SwFlyFrm*>(pAnchoredObj);
                    rImp.DisposeAccessibleFrm( pTmpFlyFrm );
                    rImp.AddAccessibleFrm( pTmpFlyFrm );
                }
                else
                {
                    rImp.DisposeAccessibleObj( pAnchoredObj->GetDrawObj() );
                    rImp.AddAccessibleObj( pAnchoredObj->GetDrawObj() );
                }
            }
            aAnchoredObjs.pop_back();
        }

        
        for ( std::vector<SdrObject*>::const_iterator aObjIter = _rMovedChildObjs.begin();
              aObjIter != _rMovedChildObjs.end(); ++aObjIter )
        {
            SdrObject* pChildObj = (*aObjIter);
            {
                const SwContact* pContact = ::GetUserCall( pChildObj );
                OSL_ENSURE( pContact,
                        "SwDrawView::_MoveRepeatedObjs(..) - missing contact object -> crash." );
                pContact->GetAnchoredObjs( aAnchoredObjs );
            }
            
            const sal_uInt32 nTmpNewPos = pChildObj->GetOrdNum();
            while ( !aAnchoredObjs.empty() )
            {
                SwAnchoredObject* pAnchoredObj = aAnchoredObjs.back();
                if ( pAnchoredObj->GetDrawObj() != pChildObj )
                {
                    pDrawPage->SetObjectOrdNum( pAnchoredObj->GetDrawObj()->GetOrdNum(),
                                                nTmpNewPos );
                    pDrawPage->RecalcObjOrdNums();
                    
                    if ( pAnchoredObj->ISA(SwFlyFrm) )
                    {
                        const SwFlyFrm *pTmpFlyFrm = static_cast<SwFlyFrm*>(pAnchoredObj);
                        rImp.DisposeAccessibleFrm( pTmpFlyFrm );
                        rImp.AddAccessibleFrm( pTmpFlyFrm );
                    }
                    else
                    {
                        rImp.DisposeAccessibleObj( pAnchoredObj->GetDrawObj() );
                        rImp.AddAccessibleObj( pAnchoredObj->GetDrawObj() );
                    }
                }
                aAnchoredObjs.pop_back();
            }
        }
    }
}


void SwDrawView::ObjOrderChanged( SdrObject* pObj, sal_uLong nOldPos,
                                          sal_uLong nNewPos )
{
    
    if ( pObj->GetUpGroup() )
    {
        return;
    }

    
    SdrPage* pDrawPage = GetModel()->GetPage( 0 );
    if ( pDrawPage->IsObjOrdNumsDirty() )
        pDrawPage->RecalcObjOrdNums();
    const sal_uInt32 nObjCount = pDrawPage->GetObjCount();

    SwAnchoredObject* pMovedAnchoredObj =
                                ::GetUserCall( pObj )->GetAnchoredObj( pObj );
    const SwFlyFrm* pParentAnchoredObj =
                                pMovedAnchoredObj->GetAnchorFrm()->FindFlyFrm();

    const bool bMovedForward = nOldPos < nNewPos;

    
    if ( pParentAnchoredObj )
    {
        if ( bMovedForward )
        {
            sal_uInt32 nMaxChildOrdNumWithoutMoved =
                    _GetMaxChildOrdNum( *pParentAnchoredObj, pMovedAnchoredObj->GetDrawObj() );
            if ( nNewPos > nMaxChildOrdNumWithoutMoved+1 )
            {
                
                pDrawPage->SetObjectOrdNum( nNewPos, nMaxChildOrdNumWithoutMoved+1 );
                nNewPos = nMaxChildOrdNumWithoutMoved+1;
            }
        }
        else
        {
            const sal_uInt32 nParentOrdNum = pParentAnchoredObj->GetDrawObj()->GetOrdNum();
            if ( nNewPos < nParentOrdNum )
            {
                
                pDrawPage->SetObjectOrdNum( nNewPos, nParentOrdNum );
                nNewPos = nParentOrdNum;
            }
        }
        if ( pDrawPage->IsObjOrdNumsDirty() )
            pDrawPage->RecalcObjOrdNums();
    }

    
    if ( ( bMovedForward && nNewPos < nObjCount - 1 ) ||
         ( !bMovedForward && nNewPos > 0 ) )
    {
        const SdrObject* pTmpObj =
                pDrawPage->GetObj( bMovedForward ? nNewPos - 1 : nNewPos + 1 );
        if ( pTmpObj )
        {
            sal_uInt32 nTmpNewPos( nNewPos );
            if ( bMovedForward )
            {
                
                const sal_uInt32 nTmpMaxOrdNum =
                                    ::GetUserCall( pTmpObj )->GetMaxOrdNum();
                if ( nTmpMaxOrdNum > nNewPos )
                    nTmpNewPos = nTmpMaxOrdNum;
            }
            else
            {
                
                const sal_uInt32 nTmpMinOrdNum =
                                    ::GetUserCall( pTmpObj )->GetMinOrdNum();
                if ( nTmpMinOrdNum < nNewPos )
                    nTmpNewPos = nTmpMinOrdNum;
            }
            if ( nTmpNewPos != nNewPos )
            {
                pDrawPage->SetObjectOrdNum( nNewPos, nTmpNewPos );
                nNewPos = nTmpNewPos;
                pDrawPage->RecalcObjOrdNums();
            }
        }
    }

    
    
    if ( pMovedAnchoredObj->ISA(SwFlyFrm) &&
         bMovedForward && nNewPos < nObjCount - 1 )
    {
        sal_uInt32 nMaxChildOrdNum =
                    _GetMaxChildOrdNum( *(static_cast<const SwFlyFrm*>(pMovedAnchoredObj)) );
        if ( nNewPos < nMaxChildOrdNum )
        {
            
            const SdrObject* pTmpObj = pDrawPage->GetObj( nMaxChildOrdNum );
            sal_uInt32 nTmpNewPos = ::GetUserCall( pTmpObj )->GetMaxOrdNum() + 1;
            if ( nTmpNewPos >= nObjCount )
            {
                --nTmpNewPos;
            }
            
            pTmpObj = pDrawPage->GetObj( nTmpNewPos );
            nTmpNewPos = ::GetUserCall( pTmpObj )->GetMaxOrdNum();
            
            pDrawPage->SetObjectOrdNum( nNewPos, nTmpNewPos );
            nNewPos = nTmpNewPos;
            pDrawPage->RecalcObjOrdNums();
        }
    }

    
    if ( ( bMovedForward && nNewPos < nObjCount - 1 ) ||
         ( !bMovedForward && nNewPos > 0 ) )
    {
        sal_uInt32 nTmpNewPos( nNewPos );
        const SwFrmFmt* pParentFrmFmt =
                pParentAnchoredObj ? &(pParentAnchoredObj->GetFrmFmt()) : 0L;
        const SdrObject* pTmpObj = pDrawPage->GetObj( nNewPos + 1 );
        while ( pTmpObj )
        {
            
            
            
            
            const SwFrm* pTmpAnchorFrm = lcl_FindAnchor( pTmpObj, sal_True );
            const SwFlyFrm* pTmpParentObj = pTmpAnchorFrm
                                            ? pTmpAnchorFrm->FindFlyFrm() : 0L;
            if ( pTmpParentObj &&
                 &(pTmpParentObj->GetFrmFmt()) != pParentFrmFmt )
            {
                if ( bMovedForward )
                {
                    nTmpNewPos = ::GetUserCall( pTmpObj )->GetMaxOrdNum();
                    pTmpObj = pDrawPage->GetObj( nTmpNewPos + 1 );
                }
                else
                {
                    nTmpNewPos = ::GetUserCall( pTmpParentObj->GetDrawObj() )
                                                            ->GetMinOrdNum();
                    pTmpObj = pTmpParentObj->GetDrawObj();
                }
            }
            else
                break;
        }
        if ( nTmpNewPos != nNewPos )
        {
            pDrawPage->SetObjectOrdNum( nNewPos, nTmpNewPos );
            nNewPos = nTmpNewPos;
            pDrawPage->RecalcObjOrdNums();
        }
    }

    
    std::vector< SdrObject* > aMovedChildObjs;

    
    if ( pMovedAnchoredObj->ISA(SwFlyFrm) )
    {
        const SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pMovedAnchoredObj);

        
        rImp.DisposeAccessibleFrm( pFlyFrm );
        rImp.AddAccessibleFrm( pFlyFrm );

        const sal_uInt32 nChildNewPos = bMovedForward ? nNewPos : nNewPos+1;
        sal_uInt32 i = bMovedForward ? nOldPos : nObjCount-1;
        do
        {
            SdrObject* pTmpObj = pDrawPage->GetObj( i );
            if ( pTmpObj == pObj )
                break;

            
            
            
            
            const SwFrm* pTmpAnchorFrm = lcl_FindAnchor( pTmpObj, sal_True );
            const SwFlyFrm* pTmpParentObj = pTmpAnchorFrm
                                            ? pTmpAnchorFrm->FindFlyFrm() : 0L;
            if ( pTmpParentObj &&
                 ( ( pTmpParentObj == pFlyFrm ) ||
                   ( pFlyFrm->IsUpperOf( *pTmpParentObj ) ) ) )
            {
                
                pDrawPage->SetObjectOrdNum( i, nChildNewPos );
                pDrawPage->RecalcObjOrdNums();
                
                aMovedChildObjs.push_back( pTmpObj );
                
                if ( pTmpObj->ISA(SwVirtFlyDrawObj) )
                {
                    const SwFlyFrm *pTmpFlyFrm =
                        static_cast<SwVirtFlyDrawObj*>(pTmpObj)->GetFlyFrm();
                    rImp.DisposeAccessibleFrm( pTmpFlyFrm );
                    rImp.AddAccessibleFrm( pTmpFlyFrm );
                }
                else
                {
                    rImp.DisposeAccessibleObj( pTmpObj );
                    rImp.AddAccessibleObj( pTmpObj );
                }
            }
            else
            {
                
                if ( bMovedForward )
                    ++i;
                else if ( !bMovedForward && i > 0 )
                    --i;
            }

        } while ( ( bMovedForward && i < ( nObjCount - aMovedChildObjs.size() ) ) ||
                  ( !bMovedForward && i > ( nNewPos + aMovedChildObjs.size() ) ) );
    }
    else
    {
        
        rImp.DisposeAccessibleObj( pObj );
        rImp.AddAccessibleObj( pObj );
    }

    _MoveRepeatedObjs( *pMovedAnchoredObj, aMovedChildObjs );
}

sal_Bool SwDrawView::TakeDragLimit( SdrDragMode eMode,
                                            Rectangle& rRect ) const
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    sal_Bool bRet = sal_False;
    if( 1 == rMrkList.GetMarkCount() )
    {
        const SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        SwRect aRect;
        if( ::CalcClipRect( pObj, aRect, eMode == SDRDRAG_MOVE ) )
        {
            rRect = aRect.SVRect();
             bRet = sal_True;
        }
    }
    return bRet;
}

const SwFrm* SwDrawView::CalcAnchor()
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    if ( rMrkList.GetMarkCount() != 1 )
        return NULL;

    SdrObject* pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();

    
    
    const SwFrm* pAnch;
    Rectangle aMyRect;
    const sal_Bool bFly = pObj->ISA(SwVirtFlyDrawObj);
    if ( bFly )
    {
        pAnch = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetAnchorFrm();
        aMyRect = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Frm().SVRect();
    }
    else
    {
        SwDrawContact *pC = (SwDrawContact*)GetUserCall(pObj);
        
        
        pAnch = pC->GetAnchorFrm( pObj );
        if( !pAnch )
        {
            pC->ConnectToLayout();
            
            
            pAnch = pC->GetAnchorFrm( pObj );
        }
        aMyRect = pObj->GetSnapRect();
    }

    
    const sal_Bool bTopRight = pAnch && ( ( pAnch->IsVertical() &&
                                            !pAnch->IsVertLR() ) ||
                                             pAnch->IsRightToLeft() );
    const Point aMyPt = bTopRight ? aMyRect.TopRight() : aMyRect.TopLeft();

    Point aPt;
    if ( IsAction() )
    {
        if ( !TakeDragObjAnchorPos( aPt, bTopRight ) )
            return NULL;
    }
    else
    {
        Rectangle aRect = pObj->GetSnapRect();
        aPt = bTopRight ? aRect.TopRight() : aRect.TopLeft();
    }

    if ( aPt != aMyPt )
    {
        if ( pAnch && pAnch->IsCntntFrm() )
        {
            
            
            bool bBodyOnly = CheckControlLayer( pObj );
            pAnch = ::FindAnchor( (SwCntntFrm*)pAnch, aPt, bBodyOnly );
        }
        else if ( !bFly )
        {
            const SwRect aRect( aPt.getX(), aPt.getY(), 1, 1 );

            SwDrawContact* pContact = (SwDrawContact*)GetUserCall(pObj);
            if ( pContact->GetAnchorFrm( pObj ) &&
                 pContact->GetAnchorFrm( pObj )->IsPageFrm() )
                pAnch = pContact->GetPageFrm();
            else
                pAnch = pContact->FindPage( aRect );
        }
    }
    if( pAnch && !pAnch->IsProtected() )
        aAnchorPoint = pAnch->GetFrmAnchorPos( ::HasWrap( pObj ) );
    else
        pAnch = 0;
    return pAnch;
}

void SwDrawView::ShowDragAnchor()
{
    SdrHdl* pHdl = aHdl.GetHdl(HDL_ANCHOR);
    if ( ! pHdl )
        pHdl = aHdl.GetHdl(HDL_ANCHOR_TR);

    if(pHdl)
    {
        CalcAnchor();
        pHdl->SetPos(aAnchorPoint);
    }
}

void SwDrawView::MarkListHasChanged()
{
    Imp().GetShell()->DrawSelChanged();
    FmFormView::MarkListHasChanged();
}


void SwDrawView::ModelHasChanged()
{
    
    
    
    
    
    
    OutlinerView* pView = GetTextEditOutlinerView();
    Color aBackColor;
    bool bColorWasSaved(false);

    if(pView)
    {
        aBackColor = pView->GetBackgroundColor();
        bColorWasSaved = true;
    }

    
    FmFormView::ModelHasChanged();

    if(bColorWasSaved)
    {
        pView = GetTextEditOutlinerView();

        if(pView)
        {
            pView->SetBackgroundColor(aBackColor);
        }
    }
}

void SwDrawView::MakeVisible( const Rectangle &rRect, Window & )
{
    OSL_ENSURE( rImp.GetShell()->GetWin(), "MakeVisible, unknown Window");
    rImp.GetShell()->MakeVisible( SwRect( rRect ) );
}

void SwDrawView::CheckPossibilities()
{
    FmFormView::CheckPossibilities();

    
    
    
    
    
    

    const SdrMarkList &rMrkList = GetMarkedObjectList();
    bool bProtect = false;
    bool bSzProtect = false;
    for ( sal_uInt16 i = 0; !bProtect && i < rMrkList.GetMarkCount(); ++i )
    {
        const SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        const SwFrm *pFrm = NULL;
        if ( pObj->ISA(SwVirtFlyDrawObj) )
        {
            const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            if ( pFly  )
            {
                pFrm = pFly->GetAnchorFrm();
                if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
                {
                    SwOLENode *pNd = ((SwCntntFrm*)pFly->Lower())->GetNode()->GetOLENode();
                    if ( pNd )
                    {
                        uno::Reference < embed::XEmbeddedObject > xObj = pNd->GetOLEObj().GetOleRef();
                        if ( xObj.is() )
                        {
                            
                            

                            
                            bSzProtect |= ( embed::EmbedMisc::EMBED_NEVERRESIZE & xObj->getStatus( embed::Aspects::MSOLE_CONTENT ) ) != 0;

                            
                            SwDoc* pDoc = Imp().GetShell()->GetDoc();
                            const bool bProtectMathPos = SotExchange::IsMath( xObj->getClassID() )
                                    && FLY_AS_CHAR == pFly->GetFmt()->GetAnchor().GetAnchorId()
                                    && pDoc->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT );
                            if (bProtectMathPos)
                                bMoveProtect = true;
                        }
                    }
                }
            }
        }
        else
        {
            SwDrawContact *pC = (SwDrawContact*)GetUserCall(pObj);
            if ( pC )
                pFrm = pC->GetAnchorFrm( pObj );
        }
        if ( pFrm )
            bProtect = pFrm->IsProtected(); 
        {
            SwFrmFmt* pFrmFmt( ::FindFrmFmt( const_cast<SdrObject*>(pObj) ) );
            if ( !pFrmFmt )
            {
                OSL_FAIL( "<SwDrawView::CheckPossibilities()> - missing frame format" );
                bProtect = true;
            }
            else if ((FLY_AS_CHAR == pFrmFmt->GetAnchor().GetAnchorId()) &&
                      rMrkList.GetMarkCount() > 1 )
            {
                bProtect = true;
            }
        }
    }
    bMoveProtect    |= bProtect;
    bResizeProtect  |= bProtect || bSzProtect;
}


void SwDrawView::ReplaceMarkedDrawVirtObjs( SdrMarkView& _rMarkView )
{
    SdrPageView* pDrawPageView = _rMarkView.GetSdrPageView();
    const SdrMarkList& rMarkList = _rMarkView.GetMarkedObjectList();

    if( rMarkList.GetMarkCount() )
    {
        
        std::vector<SdrObject*> aMarkedObjs;
        for( sal_uInt32 i = 0; i < rMarkList.GetMarkCount(); ++i )
        {
            SdrObject* pMarkedObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
            aMarkedObjs.push_back( pMarkedObj );
        }
        
        _rMarkView.UnmarkAllObj();
        
        
        while ( !aMarkedObjs.empty() )
        {
            SdrObject* pMarkObj = aMarkedObjs.back();
            if ( pMarkObj->ISA(SwDrawVirtObj) )
            {
                SdrObject* pRefObj = &(static_cast<SwDrawVirtObj*>(pMarkObj)->ReferencedObj());
                if ( !_rMarkView.IsObjMarked( pRefObj )  )
                {
                    _rMarkView.MarkObj( pRefObj, pDrawPageView );
                }
            }
            else
            {
                _rMarkView.MarkObj( pMarkObj, pDrawPageView );
            }

            aMarkedObjs.pop_back();
        }
        
        _rMarkView.SortMarkedObjects();
    }
}

void SwDrawView::DeleteMarked()
{
    SwDoc* pDoc = Imp().GetShell()->GetDoc();
    SwRootFrm *pTmpRoot = pDoc->GetCurrentLayout();
    if ( pTmpRoot )
        pTmpRoot->StartAllAction();
    pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);
    
    {
        SdrPageView* pDrawPageView = rImp.GetPageView();
        if ( pDrawPageView )
        {
            SdrMarkView* pMarkView = PTR_CAST( SdrMarkView, &(pDrawPageView->GetView()) );
            if ( pMarkView )
            {
                ReplaceMarkedDrawVirtObjs( *pMarkView );
            }
        }
    }
    if ( pDoc->DeleteSelection( *this ) )
    {
        FmFormView::DeleteMarked();
        ::FrameNotify( Imp().GetShell(), FLY_DRAG_END );
    }
    pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
    if( pTmpRoot )
        pTmpRoot->EndAllAction();
}


SdrUndoManager* SwDrawView::getSdrUndoManagerForEnhancedTextEdit() const
{
    SwDoc* pDoc = Imp().GetShell()->GetDoc();

    return pDoc ? dynamic_cast< SdrUndoManager* >(&(pDoc->GetUndoManager())) : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
