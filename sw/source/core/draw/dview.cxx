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

#include <hintids.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/fmmodel.hxx>
#include <sot/exchange.hxx>
#include <svx/sdrundomanager.hxx>
#include <tools/globname.hxx>
#include <editeng/outliner.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <notxtfrm.hxx>
#include <flyfrm.hxx>
#include <frmfmt.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <textboxhelper.hxx>
#include <viewsh.hxx>
#include <viewimp.hxx>
#include <dview.hxx>
#include <doc.hxx>
#include <mdiexp.hxx>
#include <ndole.hxx>
#include <ndgrf.hxx>
#include <fmtanchr.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentLayoutAccess.hxx>

#include <com/sun/star/embed/Aspects.hpp>

#include <vector>

#include <sortedobjs.hxx>
#include <UndoManager.hxx>

using namespace com::sun::star;

namespace {

class SwSdrHdl : public SdrHdl
{
public:
    SwSdrHdl(const Point& rPnt, bool bTopRight ) :
        SdrHdl( rPnt, bTopRight ? SdrHdlKind::Anchor_TR : SdrHdlKind::Anchor ) {}
    virtual bool IsFocusHdl() const override;
};

}

bool SwSdrHdl::IsFocusHdl() const
{
    if( SdrHdlKind::Anchor == eKind || SdrHdlKind::Anchor_TR == eKind )
        return true;
    return SdrHdl::IsFocusHdl();
}

static const SwFrame *lcl_FindAnchor( const SdrObject *pObj, bool bAll )
{
    const SwVirtFlyDrawObj *pVirt = dynamic_cast< const SwVirtFlyDrawObj *>( pObj );
    if ( pVirt )
    {
        if ( bAll || !pVirt->GetFlyFrame()->IsFlyInContentFrame() )
            return pVirt->GetFlyFrame()->GetAnchorFrame();
    }
    else
    {
        const SwDrawContact *pCont = static_cast<const SwDrawContact*>(GetUserCall(pObj));
        if ( pCont )
            return pCont->GetAnchorFrame( pObj );
    }
    return nullptr;
}

SwDrawView::SwDrawView(
    SwViewShellImp& rI,
    FmFormModel& rFmFormModel,
    OutputDevice* pOutDev)
:   FmFormView(rFmFormModel, pOutDev),
    m_rImp( rI )
{
    SetPageVisible( false );
    SetBordVisible( false );
    SetGridVisible( false );
    SetHlplVisible( false );
    SetGlueVisible( false );
    SetFrameDragSingles();
    SetSwapAsynchron();

    EnableExtendedKeyInputDispatcher( false );
    EnableExtendedMouseEventDispatcher( false );

    SetHitTolerancePixel( GetMarkHdlSizePixel()/2 );

    SetPrintPreview( rI.GetShell()->IsPreview() );

    // #i73602# Use default from the configuration
    SetBufferedOverlayAllowed(SvtOptionsDrawinglayer::IsOverlayBuffer_Writer());

    // #i74769#, #i75172# Use default from the configuration
    SetBufferedOutputAllowed(SvtOptionsDrawinglayer::IsPaintBuffer_Writer());
}

// #i99665#
bool SwDrawView::IsAntiAliasing()
{
    return SvtOptionsDrawinglayer::IsAntiAliasing();
}

static SdrObject* impLocalHitCorrection(SdrObject* pRetval, const Point& rPnt, sal_uInt16 nTol, const SdrMarkList &rMrkList)
{
    if(!nTol)
    {
        // the old method forced back to outer bounds test when nTol == 0, so
        // do not try to correct when nTol is not set (used from HelpContent)
    }
    else
    {
        // rebuild logic from former SwVirtFlyDrawObj::CheckSdrObjectHit. This is needed since
        // the SdrObject-specific CheckHit implementations are now replaced with primitives and
        // 'tricks' like in the old implementation (e.g. using a view from a model-data class to
        // detect if object is selected) are no longer valid.
        // The standard primitive hit-test for SwVirtFlyDrawObj now is the outer bound. The old
        // implementation reduced this excluding the inner bound when the object was not selected.
        SwVirtFlyDrawObj* pSwVirtFlyDrawObj = dynamic_cast< SwVirtFlyDrawObj* >(pRetval);

        if(pSwVirtFlyDrawObj)
        {
            if(pSwVirtFlyDrawObj->GetFlyFrame()->Lower() && pSwVirtFlyDrawObj->GetFlyFrame()->Lower()->IsNoTextFrame())
            {
                // the old method used IsNoTextFrame (should be for SW's own OLE and
                // graphic's) to accept hit only based on outer bounds; nothing to do
            }
            else
            {
                // check if the object is selected in this view
                const size_t nMarkCount(rMrkList.GetMarkCount());
                bool bSelected(false);

                for(size_t a = 0; !bSelected && a < nMarkCount; ++a)
                {
                    if(pSwVirtFlyDrawObj == rMrkList.GetMark(a)->GetMarkedSdrObj())
                    {
                        bSelected = true;
                    }
                }

                if(!bSelected)
                {
                    // when not selected, the object is not hit when hit position is inside
                    // inner range. Get and shrink inner range
                    basegfx::B2DRange aInnerBound(pSwVirtFlyDrawObj->getInnerBound());

                    aInnerBound.grow(-1.0 * nTol);

                    if(aInnerBound.isInside(basegfx::B2DPoint(rPnt.X(), rPnt.Y())))
                    {
                        // exclude this hit
                        pRetval = nullptr;
                    }
                }
            }
        }
    }

    return pRetval;
}

SdrObject* SwDrawView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, SdrSearchOptions nOptions, const SdrLayerIDSet* pMVisLay) const
{
    // call parent
    SdrObject* pRetval = FmFormView::CheckSingleSdrObjectHit(rPnt, nTol, pObj, pPV, nOptions, pMVisLay);

    if(pRetval)
    {
        // override to allow extra handling when picking SwVirtFlyDrawObj's
        pRetval = impLocalHitCorrection(pRetval, rPnt, nTol, GetMarkedObjectList());
    }

    return pRetval;
}

/// Gets called every time the handles need to be build
void SwDrawView::AddCustomHdl()
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();

    if(rMrkList.GetMarkCount() != 1 || !GetUserCall(rMrkList.GetMark( 0 )->GetMarkedSdrObj()))
        return;

    SdrObject *pObj = rMrkList.GetMark(0)->GetMarkedSdrObj();
    // make code robust
    SwFrameFormat* pFrameFormat( ::FindFrameFormat( pObj ) );
    if ( !pFrameFormat )
    {
        OSL_FAIL( "<SwDrawView::AddCustomHdl()> - missing frame format!" );
        return;
    }
    const SwFormatAnchor &rAnchor = pFrameFormat->GetAnchor();

    if (RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId())
        return;

    const SwFrame* pAnch = CalcAnchor();
    if(nullptr == pAnch)
        return;

    Point aPos(m_aAnchorPoint);

    if ( RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId() )
    {
        // #i28701# - use last character rectangle saved at object
        // in order to avoid a format of the anchor frame
        SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );

        // Invalidate/recalc LastCharRect which can contain invalid frame offset because
        // of later frame changes
        pAnchoredObj->CheckCharRectAndTopOfLine(false);

        SwRect aAutoPos = pAnchoredObj->GetLastCharRect();
        if ( aAutoPos.Height() )
        {
            aPos = aAutoPos.Pos();
        }
    }

    // add anchor handle:
    std::unique_ptr<SdrHdl> hdl = std::make_unique<SwSdrHdl>( aPos, ( pAnch->IsVertical() && !pAnch->IsVertLR() ) ||
                                     pAnch->IsRightToLeft() );
    hdl->SetObjHdlNum(maHdlList.GetHdlCount());
    maHdlList.AddHdl(std::move(hdl));
}

SdrObject* SwDrawView::GetMaxToTopObj( SdrObject* pObj ) const
{
    if ( GetUserCall(pObj) )
    {
        const SwFrame *pAnch = ::lcl_FindAnchor( pObj, false );
        if ( pAnch )
        {
            //The topmost Obj within the anchor must not be overtaken.
            const SwFlyFrame *pFly = pAnch->FindFlyFrame();
            if ( pFly )
            {
                const SwPageFrame *pPage = pFly->FindPageFrame();
                if ( pPage->GetSortedObjs() )
                {
                    size_t nOrdNum = 0;
                    for (SwAnchoredObject* i : *pPage->GetSortedObjs())
                    {
                        const SdrObject *pO = i->GetDrawObj();

                        if ( pO->GetOrdNumDirect() > nOrdNum )
                        {
                            const SwFrame *pTmpAnch = ::lcl_FindAnchor( pO, false );
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
    return nullptr;
}

SdrObject* SwDrawView::GetMaxToBtmObj(SdrObject* pObj) const
{
    if ( GetUserCall(pObj) )
    {
        const SwFrame *pAnch = ::lcl_FindAnchor( pObj, false );
        if ( pAnch )
        {
            //The Fly of the anchor must not be "flying under".
            const SwFlyFrame *pFly = pAnch->FindFlyFrame();
            if ( pFly )
            {
                SdrObject *pRet = const_cast<SdrObject*>(static_cast<SdrObject const *>(pFly->GetVirtDrawObj()));
                return pRet != pObj ? pRet : nullptr;
            }
        }
    }
    return nullptr;
}

/// determine maximal order number for a 'child' object of given 'parent' object
sal_uInt32 SwDrawView::GetMaxChildOrdNum( const SwFlyFrame& _rParentObj,
                                           const SdrObject* _pExclChildObj )
{
    sal_uInt32 nMaxChildOrdNum = _rParentObj.GetDrawObj()->GetOrdNum();

    const SdrPage* pDrawPage = _rParentObj.GetDrawObj()->getSdrPageFromSdrObject();
    OSL_ENSURE( pDrawPage,
            "<SwDrawView::GetMaxChildOrdNum(..) - missing drawing page at parent object - crash!" );

    const size_t nObjCount = pDrawPage->GetObjCount();
    for ( size_t i = nObjCount-1; i > _rParentObj.GetDrawObj()->GetOrdNum() ; --i )
    {
        const SdrObject* pObj = pDrawPage->GetObj( i );

        // Don't consider 'child' object <_pExclChildObj>
        if ( pObj == _pExclChildObj )
        {
            continue;
        }

        if ( pObj->GetOrdNum() > nMaxChildOrdNum &&
             _rParentObj.IsAnLower( lcl_FindAnchor( pObj, true ) ) )
        {
            nMaxChildOrdNum = pObj->GetOrdNum();
            break;
        }
    }

    return nMaxChildOrdNum;
}

/// method to move 'repeated' objects of the given moved object to the according level
void SwDrawView::MoveRepeatedObjs( const SwAnchoredObject& _rMovedAnchoredObj,
                                    const std::vector<SdrObject*>& _rMovedChildObjs ) const
{
    // determine 'repeated' objects of already moved object <_rMovedAnchoredObj>
    std::vector<SwAnchoredObject*> aAnchoredObjs;
    {
        const SwContact* pContact = ::GetUserCall( _rMovedAnchoredObj.GetDrawObj() );
        assert(pContact && "SwDrawView::MoveRepeatedObjs(..) - missing contact object -> crash.");
        pContact->GetAnchoredObjs( aAnchoredObjs );
    }

    // check, if 'repeated' objects exists.
    if ( aAnchoredObjs.size() <= 1 )
        return;

    SdrPage* pDrawPage = GetModel()->GetPage( 0 );

    // move 'repeated' ones to the same order number as the already moved one.
    const size_t nNewPos = _rMovedAnchoredObj.GetDrawObj()->GetOrdNum();
    while ( !aAnchoredObjs.empty() )
    {
        SwAnchoredObject* pAnchoredObj = aAnchoredObjs.back();
        if ( pAnchoredObj != &_rMovedAnchoredObj )
        {
            pDrawPage->SetObjectOrdNum( pAnchoredObj->GetDrawObj()->GetOrdNum(),
                                        nNewPos );
            pDrawPage->RecalcObjOrdNums();
            // adjustments for accessibility API
            if ( auto pTmpFlyFrame = pAnchoredObj->DynCastFlyFrame() )
            {
                m_rImp.DisposeAccessibleFrame( pTmpFlyFrame );
                m_rImp.AddAccessibleFrame( pTmpFlyFrame );
            }
            else
            {
                m_rImp.DisposeAccessibleObj(pAnchoredObj->GetDrawObj(), true);
                m_rImp.AddAccessibleObj( pAnchoredObj->GetDrawObj() );
            }
        }
        aAnchoredObjs.pop_back();
    }

    // move 'repeated' ones of 'child' objects
    for ( SdrObject* pChildObj : _rMovedChildObjs )
    {
        {
            const SwContact* pContact = ::GetUserCall( pChildObj );
            assert(pContact && "SwDrawView::MoveRepeatedObjs(..) - missing contact object -> crash.");
            pContact->GetAnchoredObjs( aAnchoredObjs );
        }
        // move 'repeated' ones to the same order number as the already moved one.
        const size_t nTmpNewPos = pChildObj->GetOrdNum();
        while ( !aAnchoredObjs.empty() )
        {
            SwAnchoredObject* pAnchoredObj = aAnchoredObjs.back();
            if ( pAnchoredObj->GetDrawObj() != pChildObj )
            {
                pDrawPage->SetObjectOrdNum( pAnchoredObj->GetDrawObj()->GetOrdNum(),
                                            nTmpNewPos );
                pDrawPage->RecalcObjOrdNums();
                // adjustments for accessibility API
                if ( auto pTmpFlyFrame = pAnchoredObj->DynCastFlyFrame() )
                {
                    m_rImp.DisposeAccessibleFrame( pTmpFlyFrame );
                    m_rImp.AddAccessibleFrame( pTmpFlyFrame );
                }
                else
                {
                    m_rImp.DisposeAccessibleObj(pAnchoredObj->GetDrawObj(), true);
                    m_rImp.AddAccessibleObj( pAnchoredObj->GetDrawObj() );
                }
            }
            aAnchoredObjs.pop_back();
        }
    }
}

// --> adjustment and re-factoring of method
void SwDrawView::ObjOrderChanged( SdrObject* pObj, size_t nOldPos,
                                          size_t nNewPos )
{
    // nothing to do for group members
    if ( pObj->getParentSdrObjectFromSdrObject() )
    {
        return;
    }

    // determine drawing page and assure that the order numbers are correct.
    SdrPage* pDrawPage = GetModel()->GetPage( 0 );
    if ( pDrawPage->IsObjOrdNumsDirty() )
        pDrawPage->RecalcObjOrdNums();
    const size_t nObjCount = pDrawPage->GetObjCount();

    SwAnchoredObject* pMovedAnchoredObj =
                                ::GetUserCall( pObj )->GetAnchoredObj( pObj );
    const SwFlyFrame* pParentAnchoredObj =
                                pMovedAnchoredObj->GetAnchorFrame()->FindFlyFrame();

    const bool bMovedForward = nOldPos < nNewPos;

    // assure for a 'child' object, that it doesn't exceed the limits of its 'parent'
    if ( pParentAnchoredObj )
    {
        if ( bMovedForward )
        {
            const size_t nMaxChildOrdNumWithoutMoved =
                    GetMaxChildOrdNum( *pParentAnchoredObj, pMovedAnchoredObj->GetDrawObj() );
            if ( nNewPos > nMaxChildOrdNumWithoutMoved+1 )
            {
                // set position to the top of the 'child' object group
                pDrawPage->SetObjectOrdNum( nNewPos, nMaxChildOrdNumWithoutMoved+1 );
                nNewPos = nMaxChildOrdNumWithoutMoved+1;
            }
        }
        else
        {
            const size_t nParentOrdNum = pParentAnchoredObj->GetDrawObj()->GetOrdNum();
            if ( nNewPos < nParentOrdNum )
            {
                // set position to the bottom of the 'child' object group
                pDrawPage->SetObjectOrdNum( nNewPos, nParentOrdNum );
                nNewPos = nParentOrdNum;
            }
        }
        if ( pDrawPage->IsObjOrdNumsDirty() )
            pDrawPage->RecalcObjOrdNums();
    }

    // Assure, that object isn't positioned between 'repeated' ones
    if ( ( bMovedForward && nNewPos < nObjCount - 1 ) ||
         ( !bMovedForward && nNewPos > 0 ) )
    {
        const SdrObject* pTmpObj =
                pDrawPage->GetObj( bMovedForward ? nNewPos - 1 : nNewPos + 1 );
        if ( pTmpObj )
        {
            size_t nTmpNewPos( nNewPos );
            if ( bMovedForward )
            {
                // move before the top 'repeated' object
                const sal_uInt32 nTmpMaxOrdNum =
                                    ::GetUserCall( pTmpObj )->GetMaxOrdNum();
                if ( nTmpMaxOrdNum > nNewPos )
                    nTmpNewPos = nTmpMaxOrdNum;
            }
            else
            {
                // move behind the bottom 'repeated' object
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

    // On move forward, assure that object is moved before its own children.
    // Only Writer fly frames can have children.
    if ( pMovedAnchoredObj->DynCastFlyFrame() &&
         bMovedForward && nNewPos < nObjCount - 1 )
    {
        sal_uInt32 nMaxChildOrdNum =
                    GetMaxChildOrdNum( *static_cast<const SwFlyFrame*>(pMovedAnchoredObj) );
        if ( nNewPos < nMaxChildOrdNum )
        {
            // determine position before the object before its top 'child' object
            const SdrObject* pTmpObj = pDrawPage->GetObj( nMaxChildOrdNum );
            size_t nTmpNewPos = ::GetUserCall( pTmpObj )->GetMaxOrdNum() + 1;
            if ( nTmpNewPos >= nObjCount )
            {
                --nTmpNewPos;
            }
            // assure, that determined position isn't between 'repeated' objects
            pTmpObj = pDrawPage->GetObj( nTmpNewPos );
            nTmpNewPos = ::GetUserCall( pTmpObj )->GetMaxOrdNum();
            // apply new position
            pDrawPage->SetObjectOrdNum( nNewPos, nTmpNewPos );
            nNewPos = nTmpNewPos;
            pDrawPage->RecalcObjOrdNums();
        }
    }

    // Assure, that object isn't positioned between nested objects
    if ( ( bMovedForward && nNewPos < nObjCount - 1 ) ||
         ( !bMovedForward && nNewPos > 0 ) )
    {
        size_t nTmpNewPos( nNewPos );
        const SwFrameFormat* pParentFrameFormat =
                pParentAnchoredObj ? &(pParentAnchoredObj->GetFrameFormat()) : nullptr;
        const SdrObject* pTmpObj = pDrawPage->GetObj( nNewPos + 1 );
        while ( pTmpObj )
        {
            // #i38563# - assure, that anchor frame exists.
            // If object is anchored inside an invisible part of the document
            // (e.g. page header, whose page style isn't applied, or hidden
            // section), no anchor frame exists.
            const SwFrame* pTmpAnchorFrame = lcl_FindAnchor( pTmpObj, true );
            const SwFlyFrame* pTmpParentObj = pTmpAnchorFrame
                                            ? pTmpAnchorFrame->FindFlyFrame() : nullptr;
            if ( pTmpParentObj &&
                 &(pTmpParentObj->GetFrameFormat()) != pParentFrameFormat )
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

    // setup collection of moved 'child' objects to move its 'repeated' objects.
    std::vector< SdrObject* > aMovedChildObjs;

    // move 'children' accordingly
    if ( auto pFlyFrame = pMovedAnchoredObj->DynCastFlyFrame() )
    {
        // adjustments for accessibility API
        m_rImp.DisposeAccessibleFrame( pFlyFrame );
        m_rImp.AddAccessibleFrame( pFlyFrame );

        const sal_uInt32 nChildNewPos = bMovedForward ? nNewPos : nNewPos+1;
        size_t i = bMovedForward ? nOldPos : nObjCount-1;
        do
        {
            SdrObject* pTmpObj = pDrawPage->GetObj( i );
            if ( pTmpObj == pObj )
                break;

            // #i38563# - assure, that anchor frame exists.
            // If object is anchored inside an invisible part of the document
            // (e.g. page header, whose page style isn't applied, or hidden
            // section), no anchor frame exists.
            const SwFrame* pTmpAnchorFrame = lcl_FindAnchor( pTmpObj, true );
            const SwFlyFrame* pTmpParentObj = pTmpAnchorFrame
                                            ? pTmpAnchorFrame->FindFlyFrame() : nullptr;
            if ( pTmpParentObj &&
                 ( ( pTmpParentObj == pFlyFrame ) ||
                   ( pFlyFrame->IsUpperOf( *pTmpParentObj ) ) ) )
            {
                // move child object.,
                pDrawPage->SetObjectOrdNum( i, nChildNewPos );
                pDrawPage->RecalcObjOrdNums();
                // collect 'child' object
                aMovedChildObjs.push_back( pTmpObj );
                // adjustments for accessibility API
                if ( auto pFlyDrawObj = dynamic_cast<SwVirtFlyDrawObj *>( pTmpObj ) )
                {
                    const SwFlyFrame *pTmpFlyFrame = pFlyDrawObj->GetFlyFrame();
                    m_rImp.DisposeAccessibleFrame( pTmpFlyFrame );
                    m_rImp.AddAccessibleFrame( pTmpFlyFrame );
                }
                else
                {
                    m_rImp.DisposeAccessibleObj(pTmpObj, true);
                    m_rImp.AddAccessibleObj( pTmpObj );
                }
            }
            else
            {
                // adjust loop counter
                if ( bMovedForward )
                    ++i;
                else if (i > 0)
                    --i;
            }

        } while ( ( bMovedForward && i < ( nObjCount - aMovedChildObjs.size() ) ) ||
                  ( !bMovedForward && i > ( nNewPos + aMovedChildObjs.size() ) ) );
    }
    else
    {
        // adjustments for accessibility API
        m_rImp.DisposeAccessibleObj(pObj, true);
        m_rImp.AddAccessibleObj( pObj );
    }

    MoveRepeatedObjs( *pMovedAnchoredObj, aMovedChildObjs );
}

bool SwDrawView::TakeDragLimit( SdrDragMode eMode,
                                            tools::Rectangle& rRect ) const
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    bool bRet = false;
    if( 1 == rMrkList.GetMarkCount() )
    {
        const SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        SwRect aRect;
        if( ::CalcClipRect( pObj, aRect, eMode == SdrDragMode::Move ) )
        {
            rRect = aRect.SVRect();
            bRet = true;
        }
    }
    return bRet;
}

const SwFrame* SwDrawView::CalcAnchor()
{
    const SdrMarkList &rMrkList = GetMarkedObjectList();
    if ( rMrkList.GetMarkCount() != 1 )
        return nullptr;

    SdrObject* pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();

    //Search for paragraph bound objects, otherwise only the
    //current anchor. Search only if we currently drag.
    const SwFrame* pAnch;
    tools::Rectangle aMyRect;
    auto pFlyDrawObj = dynamic_cast<SwVirtFlyDrawObj *>( pObj );
    if ( pFlyDrawObj )
    {
        pAnch = pFlyDrawObj->GetFlyFrame()->GetAnchorFrame();
        aMyRect = pFlyDrawObj->GetFlyFrame()->getFrameArea().SVRect();
    }
    else
    {
        SwDrawContact *pC = static_cast<SwDrawContact*>(GetUserCall(pObj));
        // determine correct anchor position for 'virtual' drawing objects.
        // #i26791#
        pAnch = pC->GetAnchorFrame( pObj );
        if( !pAnch )
        {
            pC->ConnectToLayout();
            // determine correct anchor position for 'virtual' drawing objects.
            // #i26791#
            pAnch = pC->GetAnchorFrame( pObj );
        }
        aMyRect = pObj->GetSnapRect();
    }

    const bool bTopRight = pAnch && ( ( pAnch->IsVertical() &&
                                            !pAnch->IsVertLR() ) ||
                                             pAnch->IsRightToLeft() );
    const Point aMyPt = bTopRight ? aMyRect.TopRight() : aMyRect.TopLeft();

    Point aPt;
    if ( IsAction() )
    {
        if ( !TakeDragObjAnchorPos( aPt, bTopRight ) )
            return nullptr;
    }
    else
    {
        tools::Rectangle aRect = pObj->GetSnapRect();
        aPt = bTopRight ? aRect.TopRight() : aRect.TopLeft();
    }

    if ( aPt != aMyPt )
    {
        if ( pAnch && pAnch->IsContentFrame() )
        {
            // allow drawing objects in header/footer,
            // but exclude control objects.
            bool bBodyOnly = CheckControlLayer( pObj );
            pAnch = ::FindAnchor( static_cast<const SwContentFrame*>(pAnch), aPt, bBodyOnly );
        }
        else if ( !pFlyDrawObj )
        {
            const SwRect aRect( aPt.getX(), aPt.getY(), 1, 1 );

            SwDrawContact* pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
            if ( pContact->GetAnchorFrame( pObj ) &&
                 pContact->GetAnchorFrame( pObj )->IsPageFrame() )
                pAnch = pContact->GetPageFrame();
            else
                pAnch = pContact->FindPage( aRect );
        }
    }
    if( pAnch && !pAnch->IsProtected() )
        m_aAnchorPoint = pAnch->GetFrameAnchorPos( ::HasWrap( pObj ) );
    else
        pAnch = nullptr;
    return pAnch;
}

void SwDrawView::ShowDragAnchor()
{
    SdrHdl* pHdl = maHdlList.GetHdl(SdrHdlKind::Anchor);
    if ( ! pHdl )
        pHdl = maHdlList.GetHdl(SdrHdlKind::Anchor_TR);

    if(pHdl)
    {
        CalcAnchor();
        pHdl->SetPos(m_aAnchorPoint);
    }
}

void SwDrawView::MarkListHasChanged()
{
    Imp().GetShell()->DrawSelChanged();
    FmFormView::MarkListHasChanged();
}

// #i7672#
void SwDrawView::ModelHasChanged()
{
    // The ModelHasChanged() call in DrawingLayer also updates
    // an eventually active text edit view (OutlinerView). This also leads
    // to newly setting the background color for that edit view. Thus,
    // this method rescues the current background color if an OutlinerView
    // exists and re-establishes it then. To be more safe, the OutlinerView
    // will be fetched again (maybe textedit has ended).
    OutlinerView* pView = GetTextEditOutlinerView();
    Color aBackColor;
    bool bColorWasSaved(false);

    if(pView)
    {
        aBackColor = pView->GetBackgroundColor();
        bColorWasSaved = true;
    }

    // call parent
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

void SwDrawView::MakeVisible( const tools::Rectangle &rRect, vcl::Window & )
{
    OSL_ENSURE( m_rImp.GetShell()->GetWin(), "MakeVisible, unknown Window");
    m_rImp.GetShell()->MakeVisible( SwRect( rRect ) );
}

void SwDrawView::CheckPossibilities()
{
    FmFormView::CheckPossibilities();

    //In addition to the existing flags of the objects themselves,
    //which are evaluated by the DrawingEngine, other circumstances
    //lead to a protection.
    //Objects that are anchored in frames need to be protected
    //if the content of the frame is protected.
    //OLE-Objects may themselves wish a resize protection (StarMath)

    const SdrMarkList &rMrkList = GetMarkedObjectList();
    bool bProtect = false;
    bool bSzProtect = false;
    bool bRotate(false);

    for ( size_t i = 0; !bProtect && i < rMrkList.GetMarkCount(); ++i )
    {
        const SdrObject *pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
        const SwFrame *pFrame = nullptr;
        if ( auto pVirtFlyDrawObj = dynamic_cast< const SwVirtFlyDrawObj *>( pObj ) )
        {
            const SwFlyFrame *pFly = pVirtFlyDrawObj->GetFlyFrame();
            if ( pFly  )
            {
                pFrame = pFly->GetAnchorFrame();
                if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
                {
                    const SwNoTextFrame *const pNTF(static_cast<const SwNoTextFrame*>(pFly->Lower()));
                    const SwOLENode *const pOLENd = pNTF->GetNode()->GetOLENode();
                    const SwGrfNode *const pGrfNd = pNTF->GetNode()->GetGrfNode();

                    if ( pOLENd )
                    {
                        const uno::Reference < embed::XEmbeddedObject > xObj = const_cast< SwOLEObj& >(pOLENd->GetOLEObj()).GetOleRef();

                        if ( xObj.is() )
                        {
                            // --> improvement for the future, when more
                            // than one Writer fly frame can be selected.

                            // TODO/LATER: retrieve Aspect - from where?!
                            bSzProtect |= ( embed::EmbedMisc::EMBED_NEVERRESIZE & xObj->getStatus( embed::Aspects::MSOLE_CONTENT ) ) != 0;

                            // #i972: protect position if it is a Math object anchored 'as char' and baseline alignment is activated
                            SwDoc* pDoc = Imp().GetShell()->GetDoc();
                            const bool bProtectMathPos = SotExchange::IsMath( xObj->getClassID() )
                                    && RndStdIds::FLY_AS_CHAR == pFly->GetFormat()->GetAnchor().GetAnchorId()
                                    && pDoc->GetDocumentSettingManager().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT );
                            if (bProtectMathPos)
                                m_bMoveProtect = true;
                        }
                    }
                    else if(pGrfNd)
                    {
                        // RotGrfFlyFrame: GraphicNode allows rotation(s). The loop ew are in stops
                        // as soon as bMoveProtect is set, but since rotation is valid only with
                        // a single object selected this makes no difference
                        bRotate = true;
                    }
                }
            }
        }
        else
        {
            SwDrawContact *pC = static_cast<SwDrawContact*>(GetUserCall(pObj));
            if ( pC )
                pFrame = pC->GetAnchorFrame( pObj );
        }
        if ( pFrame )
            bProtect = pFrame->IsProtected(); //Frames, areas etc.
        {
            SwFrameFormat* pFrameFormat( ::FindFrameFormat( const_cast<SdrObject*>(pObj) ) );
            if ( !pFrameFormat )
            {
                OSL_FAIL( "<SwDrawView::CheckPossibilities()> - missing frame format" );
                bProtect = true;
            }
            else if ((RndStdIds::FLY_AS_CHAR == pFrameFormat->GetAnchor().GetAnchorId()) &&
                      rMrkList.GetMarkCount() > 1 )
            {
                bProtect = true;
            }
        }
    }
    m_bMoveProtect    |= bProtect;
    m_bResizeProtect  |= bProtect || bSzProtect;

    // RotGrfFlyFrame: allow rotation when SwGrfNode is selected and not size protected
    m_bRotateFreeAllowed |= bRotate && !bProtect;
    m_bRotate90Allowed |= m_bRotateFreeAllowed;
}

/// replace marked <SwDrawVirtObj>-objects by its reference object for delete marked objects.
void SwDrawView::ReplaceMarkedDrawVirtObjs( SdrMarkView& _rMarkView )
{
    SdrPageView* pDrawPageView = _rMarkView.GetSdrPageView();
    const SdrMarkList& rMarkList = _rMarkView.GetMarkedObjectList();

    if( !rMarkList.GetMarkCount() )
        return;

    // collect marked objects in a local data structure
    std::vector<SdrObject*> aMarkedObjs;
    for( size_t i = 0; i < rMarkList.GetMarkCount(); ++i )
    {
        SdrObject* pMarkedObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        aMarkedObjs.push_back( pMarkedObj );
    }
    // unmark all objects
    _rMarkView.UnmarkAllObj();
    // re-mark objects, but for marked <SwDrawVirtObj>-objects marked its
    // reference object.
    while ( !aMarkedObjs.empty() )
    {
        SdrObject* pMarkObj = aMarkedObjs.back();
        if ( auto pVirtObj = dynamic_cast<SwDrawVirtObj *>( pMarkObj ) )
        {
            SdrObject* pRefObj = &(pVirtObj->ReferencedObj());
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
    // sort marked list in order to assure consistent state in drawing layer
    _rMarkView.SortMarkedObjects();
}

SfxViewShell* SwDrawView::GetSfxViewShell() const
{
    return m_rImp.GetShell()->GetSfxViewShell();
}

void SwDrawView::DeleteMarked()
{
    SwDoc* pDoc = Imp().GetShell()->GetDoc();
    SwRootFrame *pTmpRoot = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    if ( pTmpRoot )
        pTmpRoot->StartAllAction();
    pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);
    // replace marked <SwDrawVirtObj>-objects by its reference objects.
    if (SdrPageView* pDrawPageView = m_rImp.GetPageView())
    {
        ReplaceMarkedDrawVirtObjs(pDrawPageView->GetView());
    }

    // Check what textboxes have to be deleted afterwards.
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    std::vector<SwFrameFormat*> aTextBoxesToDelete;
    for (size_t i = 0; i < rMarkList.GetMarkCount(); ++i)
    {
        SdrObject *pObject = rMarkList.GetMark(i)->GetMarkedSdrObj();
        SwContact* pContact = GetUserCall(pObject);
        SwFrameFormat* pFormat = pContact->GetFormat();
        if (pObject->getChildrenOfSdrObject())
        {
            auto pChildTextBoxes = SwTextBoxHelper::CollectTextBoxes(pObject, pFormat);
            for (auto& rChildTextBox : pChildTextBoxes)
                aTextBoxesToDelete.push_back(rChildTextBox);
        }
        else
            if (SwFrameFormat* pTextBox = SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT))
                aTextBoxesToDelete.push_back(pTextBox);
    }

    if ( pDoc->DeleteSelection( *this ) )
    {
        FmFormView::DeleteMarked();
        ::FrameNotify( Imp().GetShell(), FLY_DRAG_END );
    }

    // Only delete these now: earlier deletion would clear the mark list as well.
    // Delete in reverse order, assuming that the container is sorted by anchor positions.
    for (int i = aTextBoxesToDelete.size() - 1; i >= 0; --i)
    {
        SwFrameFormat*& rpTextBox = aTextBoxesToDelete[i];
        pDoc->getIDocumentLayoutAccess().DelLayoutFormat(rpTextBox);
    }

    pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
    if( pTmpRoot )
        pTmpRoot->EndAllAction();
}

// Create a new view-local UndoManager manager for Writer
std::unique_ptr<SdrUndoManager> SwDrawView::createLocalTextUndoManager()
{
    std::unique_ptr<SdrUndoManager> pUndoManager(new SdrUndoManager);
    pUndoManager->SetDocShell(SfxObjectShell::Current());
    return pUndoManager;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
