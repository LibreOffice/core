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

#include <com/sun/star/util/XChangesNotifier.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdopath.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/svxids.hrc>
#include <svx/polypolygoneditor.hxx>
#include <svx/svddrgmt.hxx>

#include "CustomAnimationPane.hxx"
#include <View.hxx>
#include "motionpathtag.hxx"
#include <ViewShell.hxx>
#include <Window.hxx>

#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>

using sdr::PolyPolygonEditor;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::drawing;

namespace sd
{

const sal_uInt32 SMART_TAG_HDL_NUM = SAL_MAX_UINT32;
static const int DRGPIX     = 2;                               // Drag MinMove in Pixel

class PathDragMove : public SdrDragMove
{
private:
    basegfx::B2DPolyPolygon const   maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries() override;

public:
    PathDragMove(SdrDragView& rNewView,
        const rtl::Reference <MotionPathTag >& xTag,
        const basegfx::B2DPolyPolygon& rPathPolyPolygon)
    :   SdrDragMove(rNewView),
        maPathPolyPolygon(rPathPolyPolygon),
        mxTag( xTag )
    {}

    PathDragMove(SdrDragView& rNewView,
        const rtl::Reference <MotionPathTag >& xTag)
    :   SdrDragMove(rNewView),
        maPathPolyPolygon(),
        mxTag( xTag )
    {}

    virtual bool BeginSdrDrag() override;
    virtual bool EndSdrDrag(bool bCopy) override;

    rtl::Reference <MotionPathTag > mxTag;
};

void PathDragMove::createSdrDragEntries()
{
    // call parent
    SdrDragMove::createSdrDragEntries();

    if(maPathPolyPolygon.count())
    {
        addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPolyPolygon(maPathPolyPolygon)));
    }
}

bool PathDragMove::BeginSdrDrag()
{
    if( mxTag.is() )
    {
        SdrPathObj* pPathObj = mxTag->getPathObj();
        if( pPathObj )
        {
            DragStat().SetActionRect(pPathObj->GetCurrentBoundRect());
        }
    }
    Show();
    return true;
}

bool PathDragMove::EndSdrDrag(bool /*bCopy*/)
{
    Hide();
    if( mxTag.is() )
        mxTag->MovePath( DragStat().GetDX(), DragStat().GetDY() );
    return true;
}

class PathDragResize : public SdrDragResize
{
private:
    basegfx::B2DPolyPolygon const   maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries() override;

public:
    PathDragResize(SdrDragView& rNewView,
        const rtl::Reference <MotionPathTag >& xTag,
        const basegfx::B2DPolyPolygon& rPathPolyPolygon)
    :   SdrDragResize(rNewView),
        maPathPolyPolygon(rPathPolyPolygon),
        mxTag( xTag )
    {}

    PathDragResize(SdrDragView& rNewView,
        const rtl::Reference <MotionPathTag >& xTag)
    :   SdrDragResize(rNewView),
        maPathPolyPolygon(),
        mxTag( xTag )
    {}

    virtual bool EndSdrDrag(bool bCopy) override;
    rtl::Reference <MotionPathTag > mxTag;
};

void PathDragResize::createSdrDragEntries()
{
    // call parent
    SdrDragResize::createSdrDragEntries();

    if(maPathPolyPolygon.count())
    {
        addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPolyPolygon(maPathPolyPolygon)));
    }
}

bool PathDragResize::EndSdrDrag(bool /*bCopy*/)
{
    Hide();
    if( mxTag.is() )
    {
        SdrPathObj* pPathObj = mxTag->getPathObj();
        if( pPathObj )
        {
            const Point aRef( DragStat().GetRef1() );
            basegfx::B2DHomMatrix aTrans(basegfx::utils::createTranslateB2DHomMatrix(-aRef.X(), -aRef.Y()));
            aTrans.scale(double(aXFact), double(aYFact));
            aTrans.translate(aRef.X(), aRef.Y());
            basegfx::B2DPolyPolygon aDragPoly(pPathObj->GetPathPoly());
            aDragPoly.transform(aTrans);
            pPathObj->SetPathPoly( aDragPoly );
        }
    }
    return true;
}

class PathDragObjOwn : public SdrDragObjOwn
{
private:
    basegfx::B2DPolyPolygon const   maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries() override;

public:
    PathDragObjOwn(SdrDragView& rNewView,
        const basegfx::B2DPolyPolygon& rPathPolyPolygon)
    :   SdrDragObjOwn(rNewView),
        maPathPolyPolygon(rPathPolyPolygon)
    {}

    explicit PathDragObjOwn(SdrDragView& rNewView)
    :   SdrDragObjOwn(rNewView),
        maPathPolyPolygon()
    {}

    virtual bool EndSdrDrag(bool bCopy) override;
};

void PathDragObjOwn::createSdrDragEntries()
{
    // call parent
    SdrDragObjOwn::createSdrDragEntries();

    if(maPathPolyPolygon.count())
    {
        addSdrDragEntry(std::unique_ptr<SdrDragEntry>(new SdrDragEntryPolyPolygon(maPathPolyPolygon)));
    }
}

bool PathDragObjOwn::EndSdrDrag(bool /*bCopy*/)
{
    Hide();

    SdrObject* pObj = GetDragObj();

    if(pObj && pObj->applySpecialDrag(DragStat()))
    {
        pObj->SetChanged();
        pObj->BroadcastObjectChange();
        return true;
    }
    else
    {
        return false;
    }
}

class SdPathHdl : public SmartHdl
{
public:
    SdPathHdl( const SmartTagReference& xTag, SdrPathObj* mpPathObj );

    virtual void CreateB2dIAObject() override;
    virtual bool IsFocusHdl() const override;
    virtual bool isMarkable() const override;

private:
    SdrPathObj* const mpPathObj;
};

SdPathHdl::SdPathHdl( const SmartTagReference& xTag, SdrPathObj* pPathObj )
: SmartHdl( xTag, pPathObj->GetCurrentBoundRect().TopLeft(), SdrHdlKind::SmartTag )
, mpPathObj( pPathObj )
{
}

void SdPathHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!(pView && !pView->areMarkHandlesHidden()))
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is() && mpPathObj)
            {
                const sdr::contact::ViewContact& rVC = mpPathObj->GetViewContact();
                const drawinglayer::primitive2d::Primitive2DContainer& aSequence = rVC.getViewIndependentPrimitive2DContainer();
                std::unique_ptr<sdr::overlay::OverlayObject> pNew(new sdr::overlay::OverlayPrimitive2DSequenceObject(aSequence));

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNew),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

bool SdPathHdl::IsFocusHdl() const
{
    return false;
}

bool SdPathHdl::isMarkable() const
{
    return false;
}

MotionPathTag::MotionPathTag( CustomAnimationPane& rPane, ::sd::View& rView, const CustomAnimationEffectPtr& pEffect )
: SmartTag( rView )
, mrPane( rPane )
, mpEffect( pEffect )
, mxOrigin( pEffect->getTargetShape() )
, msLastPath( pEffect->getPath() )
, mbInUpdatePath( false )
{
    mpPathObj = mpEffect->createSdrPathObjFromPath(rView.getSdrModelFromSdrView());
    mxPolyPoly = mpPathObj->GetPathPoly();
    if (mxOrigin.is())
        maOriginPos = mxOrigin->getPosition();

    XDash aDash( css::drawing::DashStyle_RECT, 1, 80, 1, 80, 80);
    OUString aEmpty( "?" );
    mpPathObj->SetMergedItem( XLineDashItem( aEmpty, aDash ) );
    mpPathObj->SetMergedItem( XLineStyleItem( drawing::LineStyle_DASH ) );
    mpPathObj->SetMergedItem( XLineColorItem(aEmpty, COL_GRAY) );
    mpPathObj->SetMergedItem( XFillStyleItem( drawing::FillStyle_NONE ) );

    ::basegfx::B2DPolygon aStartArrow;
    aStartArrow.append(::basegfx::B2DPoint(20.0, 0.0));
    aStartArrow.append(::basegfx::B2DPoint(0.0,  0.0));
    aStartArrow.append(::basegfx::B2DPoint(10.0, 30.0));
    aStartArrow.setClosed(true);
    mpPathObj->SetMergedItem(XLineStartItem(aEmpty,::basegfx::B2DPolyPolygon(aStartArrow)));
    mpPathObj->SetMergedItem(XLineStartWidthItem(400));
    mpPathObj->SetMergedItem(XLineStartCenterItem(true));

    updatePathAttributes();

    mpPathObj->SetMergedItem(XLineTransparenceItem(50));

    mpMark.reset(new SdrMark( mpPathObj, mrView.GetSdrPageView() ));

    mpPathObj->AddListener( *this );

    Reference< XChangesNotifier > xNotifier( mpEffect->getNode(), UNO_QUERY );
    if( xNotifier.is() )
    {
        xNotifier->addChangesListener( this );
    }
}

MotionPathTag::~MotionPathTag()
{
    DBG_ASSERT( mpPathObj == nullptr, "sd::MotionPathTag::~MotionPathTag(), dispose me first!" );
    Dispose();
}

void MotionPathTag::updatePathAttributes()
{
    ::basegfx::B2DPolygon aCandidate;
    if( mxPolyPoly.count() )
    {
        aCandidate = mxPolyPoly.getB2DPolygon(0);
        ::basegfx::utils::checkClosed( aCandidate );
    }

    if( !aCandidate.isClosed() )
    {
        ::basegfx::B2DPolygon aEndArrow;
        aEndArrow.append(::basegfx::B2DPoint(10.0, 0.0));
        aEndArrow.append(::basegfx::B2DPoint(0.0, 30.0));
        aEndArrow.append(::basegfx::B2DPoint(20.0, 30.0));
        aEndArrow.setClosed(true);
        mpPathObj->SetMergedItem(XLineEndItem("?",::basegfx::B2DPolyPolygon(aEndArrow)));
        mpPathObj->SetMergedItem(XLineEndWidthItem(400));
        mpPathObj->SetMergedItem(XLineEndCenterItem(true));
    }
    else
    {
        mpPathObj->SetMergedItem(XLineEndItem());
    }
}

void MotionPathTag::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if( !(mpPathObj && !mbInUpdatePath && dynamic_cast< const SdrHint* >( &rHint ) && (mpEffect.get() != nullptr)) )
        return;

    if( mxPolyPoly != mpPathObj->GetPathPoly() )
    {
        mbInUpdatePath = true;
        mxPolyPoly = mpPathObj->GetPathPoly();
        rtl::Reference< MotionPathTag > xTag( this );
        mrPane.updatePathFromMotionPathTag( xTag );
        msLastPath = mpEffect->getPath();
        updatePathAttributes();
        mbInUpdatePath = false;
    }
}

void MotionPathTag::MovePath( int nDX, int nDY )
{
    if( mpPathObj )
    {
        mpPathObj->Move( Size( nDX, nDY ) );
        mrView.updateHandles();
    }
}

/** returns true if the MotionPathTag handled the event. */
bool MotionPathTag::MouseButtonDown( const MouseEvent& rMEvt, SmartHdl& rHdl )
{
    if( !mpPathObj )
        return false;

    if( !isSelected() )
    {
        SmartTagReference xTag( this );
        mrView.getSmartTags().select( xTag );
        selectionChanged();
        return true;
    }
    else
    {
        if( rMEvt.IsLeft() && (rMEvt.GetClicks() == 2) )
        {
            mrView.GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(SID_BEZIER_EDIT, SfxCallMode::ASYNCHRON);
            return true;
        }
        else if( rMEvt.IsLeft() )
        {
            OutputDevice* pOut = mrView.GetViewShell()->GetActiveWindow();
            Point aMDPos( pOut->PixelToLogic( rMEvt.GetPosPixel() ) );

            if( !mrView.IsFrameDragSingles() && mrView.IsInsObjPointMode() && (rHdl.GetObjHdlNum() == SMART_TAG_HDL_NUM) )
            {
                // insert a point in edit mode
                const bool bNewObj = rMEvt.IsMod1();

                mrView.BrkAction();

                Point aPt(aMDPos); // - pMarkedPV->GetOffset());

                if(bNewObj)
                    aPt = mrView.GetSnapPos(aPt,mrView.GetSdrPageView());

                bool bClosed0(mpPathObj->IsClosedObj());

                sal_uInt32 nInsPointNum = mpPathObj->NbcInsPointOld(aPt, bNewObj);

                if(bClosed0 != mpPathObj->IsClosedObj())
                {
                    // Obj was closed implicit
                    // object changed
                    mpPathObj->SetChanged();
                    mpPathObj->BroadcastObjectChange();
                }

                if(0xffffffff != nInsPointNum)
                {
                    mrView.UnmarkAllPoints();
                    mrView.updateHandles();

                    bool bRet = mrView.BegDragObj(aMDPos, pOut, mrView.GetHdl(nInsPointNum+1), 0, new PathDragObjOwn( mrView ) );

                    if (bRet)
                    {
                        const_cast< SdrDragStat* >( &mrView.GetDragStat() )->SetMinMoved();
                        mrView.MovDragObj(aMDPos);
                    }
                }
                return true;
            }
            else
            {
                SmartHdl* pHdl = &rHdl;
                if (!mrView.IsPointMarked(*pHdl) || rMEvt.IsShift())
                {
                    if (!rMEvt.IsShift())
                    {
                        mrView.UnmarkAllPoints();
                        pHdl = dynamic_cast< SmartHdl* >( mrView.PickHandle(aMDPos) );
                    }
                    else
                    {
                        if (mrView.IsPointMarked(*pHdl) )
                        {
                            mrView.UnmarkPoint(*pHdl);
                            pHdl = nullptr;
                        }
                        else
                        {
                            pHdl = dynamic_cast< SmartHdl* >( mrView.PickHandle(aMDPos) );
                        }
                    }

                    if (pHdl)
                        mrView.MarkPoint(*pHdl);
                }

                if( pHdl && !rMEvt.IsRight() )
                {
                    mrView.BrkAction();
                    const sal_uInt16 nDrgLog = static_cast<sal_uInt16>(pOut->PixelToLogic(Size(DRGPIX,0)).Width());

                    rtl::Reference< MotionPathTag > xTag( this );
                    SdrDragMethod* pDragMethod;

                    // #i95646# add DragPoly as geometry to each local SdrDragMethod to be able
                    // to create the needed local SdrDragEntry for it in createSdrDragEntries()
                    const basegfx::B2DPolyPolygon aDragPoly(mpPathObj->GetPathPoly());

                    if( (pHdl->GetKind() == SdrHdlKind::Move) || (pHdl->GetKind() == SdrHdlKind::SmartTag) )
                    {
                        pDragMethod = new PathDragMove( mrView, xTag, aDragPoly );
                        pHdl->SetPos( aMDPos );
                    }
                    else if( pHdl->GetKind() == SdrHdlKind::Poly )
                    {
                        pDragMethod = new PathDragObjOwn( mrView, aDragPoly );
                    }
                    else
                    {
                        pDragMethod = new PathDragResize( mrView, xTag, aDragPoly );
                    }

                    mrView.BegDragObj(aMDPos, nullptr, pHdl, nDrgLog, pDragMethod );
                }
                return true;
            }
        }
    }

    return false;
}

/** returns true if the SmartTag consumes this event. */
bool MotionPathTag::KeyInput( const KeyEvent& rKEvt )
{
    if( !mpPathObj )
        return false;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    switch( nCode )
    {
    case KEY_DELETE:
        return OnDelete();

    case KEY_DOWN:
    case KEY_UP:
    case KEY_LEFT:
    case KEY_RIGHT:
        return OnMove( rKEvt );

    case KEY_ESCAPE:
    {
        SmartTagReference xThis( this );
        mrView.getSmartTags().deselect();
        return true;
    }

    case KEY_TAB:
        return OnTabHandles( rKEvt );

    case KEY_SPACE:
        return OnMarkHandle( rKEvt );

    default:
        break;
    }
    return false;
}

bool MotionPathTag::OnDelete()
{
    mrPane.remove( mpEffect );
    return true;
}

bool MotionPathTag::OnTabHandles( const KeyEvent& rKEvt )
{
    if(rKEvt.GetKeyCode().IsMod1() || rKEvt.GetKeyCode().IsMod2())
    {
        const SdrHdlList& rHdlList = mrView.GetHdlList();
        bool bForward(!rKEvt.GetKeyCode().IsShift());

        const_cast<SdrHdlList&>(rHdlList).TravelFocusHdl(bForward);

        // guarantee visibility of focused handle
        SdrHdl* pHdl = rHdlList.GetFocusHdl();

        if(pHdl)
        {
            Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
            if( pWindow )
            {
                Point aHdlPosition(pHdl->GetPos());
                ::tools::Rectangle aVisRect(aHdlPosition - Point(100, 100), Size(200, 200));
                mrView.MakeVisible(aVisRect, *pWindow);
            }
        }

        return true;
    }

    return false;
}

bool MotionPathTag::OnMarkHandle( const KeyEvent& rKEvt )
{
    const SdrHdlList& rHdlList = mrView.GetHdlList();
    SdrHdl* pHdl = rHdlList.GetFocusHdl();

    if(pHdl && pHdl->GetKind() == SdrHdlKind::Poly )
    {
        // rescue ID of point with focus
        sal_uInt32 nPol(pHdl->GetPolyNum());
        sal_uInt32 nPnt(pHdl->GetPointNum());

        if(mrView.IsPointMarked(*pHdl))
        {
            if(rKEvt.GetKeyCode().IsShift())
            {
                mrView.UnmarkPoint(*pHdl);
            }
        }
        else
        {
            if(!rKEvt.GetKeyCode().IsShift())
            {
                mrView.UnmarkAllPoints();
            }
            mrView.MarkPoint(*pHdl);
        }

        if(nullptr == rHdlList.GetFocusHdl())
        {
            // restore point with focus
            SdrHdl* pNewOne = nullptr;

            for(size_t a = 0; !pNewOne && a < rHdlList.GetHdlCount(); ++a)
            {
                SdrHdl* pAct = rHdlList.GetHdl(a);

                if(pAct && pAct->GetKind() == SdrHdlKind::Poly && pAct->GetPolyNum() == nPol && pAct->GetPointNum() == nPnt)
                    pNewOne = pAct;
            }

            if(pNewOne)
                const_cast<SdrHdlList&>(rHdlList).SetFocusHdl(pNewOne);
        }
    }

    return true;
}

bool MotionPathTag::OnMove( const KeyEvent& rKEvt )
{
    long nX = 0;
    long nY = 0;

    switch( rKEvt.GetKeyCode().GetCode() )
    {
    case KEY_UP:    nY = -1; break;
    case KEY_DOWN:  nY =  1; break;
    case KEY_LEFT:  nX = -1; break;
    case KEY_RIGHT: nX =  1; break;
    default: break;
    }

    if(rKEvt.GetKeyCode().IsMod2())
    {
        OutputDevice* pOut = mrView.GetViewShell()->GetActiveWindow();
        Size aLogicSizeOnePixel = pOut ? pOut->PixelToLogic(Size(1,1)) : Size(100, 100);
        nX *= aLogicSizeOnePixel.Width();
        nY *= aLogicSizeOnePixel.Height();
    }
    else
    {
        // old, fixed move distance
        nX *= 100;
        nY *= 100;
    }

    if( nX || nY )
    {
        // in point edit mode move the handle with the focus
        const SdrHdlList& rHdlList = mrView.GetHdlList();
        SdrHdl* pHdl = rHdlList.GetFocusHdl();

        if(pHdl)
        {
            // now move the Handle (nX, nY)
            Point aStartPoint(pHdl->GetPos());
            Point aEndPoint(pHdl->GetPos() + Point(nX, nY));

            // start dragging
            rtl::Reference< MotionPathTag > xTag( this );
            SdrDragMethod* pDragMethod = nullptr;
            if( (pHdl->GetKind() == SdrHdlKind::Move) || (pHdl->GetKind() == SdrHdlKind::SmartTag) )
            {
                pDragMethod = new PathDragMove( mrView, xTag );
            }
            else if( pHdl->GetKind() == SdrHdlKind::Poly )
            {
                pDragMethod = new PathDragObjOwn( mrView );
            }
            else if( pHdl->GetKind() != SdrHdlKind::BezierWeight )
            {
                pDragMethod = new PathDragResize( mrView, xTag );
            }
            mrView.BegDragObj(aStartPoint, nullptr, pHdl, 0, pDragMethod);

            if(mrView.IsDragObj())
            {
                bool bWasNoSnap = mrView.GetDragStat().IsNoSnap();
                bool bWasSnapEnabled = mrView.IsSnapEnabled();

                // switch snapping off
                if(!bWasNoSnap)
                    const_cast<SdrDragStat&>(mrView.GetDragStat()).SetNoSnap();
                if(bWasSnapEnabled)
                    mrView.SetSnapEnabled(false);

                mrView.MovAction(aEndPoint);
                mrView.EndDragObj();

                // restore snap
                if(!bWasNoSnap)
                    const_cast<SdrDragStat&>(mrView.GetDragStat()).SetNoSnap(bWasNoSnap);
                if(bWasSnapEnabled)
                    mrView.SetSnapEnabled(bWasSnapEnabled);
            }
        }
        else
        {
            // move the path
            MovePath( nX, nY );
        }
    }

    return true;
}

sal_Int32 MotionPathTag::GetMarkablePointCount() const
{
    if( mpPathObj && isSelected() )
    {
        return mpPathObj->GetPointCount();
    }
    else
    {
        return 0;
    }
}

sal_Int32 MotionPathTag::GetMarkedPointCount() const
{
    if( mpMark )
    {
        const SdrUShortCont& rPts = mpMark->GetMarkedPoints();
        return rPts.size();
    }
    else
    {
        return 0;
    }
}

bool MotionPathTag::MarkPoint(SdrHdl& rHdl, bool bUnmark )
{
    bool bRet=false;
    if( mpPathObj && mrView.IsPointMarkable( rHdl ) && (rHdl.GetKind() != SdrHdlKind::SmartTag) )
    {
        SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( &rHdl );
        if( pSmartHdl && pSmartHdl->getTag().get() == this )
        {
            if (mrView.MarkPointHelper(&rHdl,mpMark.get(),bUnmark))
            {
                mrView.MarkListHasChanged();
                bRet=true;
            }
        }
    }
    return bRet;
}

bool MotionPathTag::MarkPoints(const ::tools::Rectangle* pRect, bool bUnmark )
{
    bool bChgd=false;

    if( mpPathObj && isSelected() )
    {
        size_t nHdlNum = mrView.GetHdlList().GetHdlCount();
        if ( nHdlNum <= 1 )
            return false;

        while( --nHdlNum > 0 )
        {
            SmartHdl* pHdl = dynamic_cast< SmartHdl* >( mrView.GetHdl( nHdlNum ) );

            if( pHdl && (pHdl->getTag().get() == this) && mrView.IsPointMarkable(*pHdl) && pHdl->IsSelected() == bUnmark)
            {
                Point aPos(pHdl->GetPos());
                if( pRect==nullptr || pRect->IsInside(aPos))
                {
                    if( mrView.MarkPointHelper(pHdl,mpMark.get(),bUnmark) )
                        bChgd=true;
                }
            }
        }

        if(bChgd)
            mrView.MarkListHasChanged();
    }

    return bChgd;
}

bool MotionPathTag::getContext( SdrViewContext& rContext )
{
    if( mpPathObj && isSelected() && !mrView.IsFrameDragSingles() )
    {
        rContext = SdrViewContext::PointEdit;
        return true;
    }
    else
    {
        return false;
    }
}

void MotionPathTag::CheckPossibilities()
{
    if( !(mpPathObj && isSelected()) )
        return;

    mrView.SetMoveAllowed( true );
    mrView.SetMoveProtected( false );
    mrView.SetResizeFreeAllowed( true );
    mrView.SetResizePropAllowed( true );
    mrView.SetResizeProtected( false );

    if( !mrView.IsFrameDragSingles() )
    {
        bool b1stSmooth(true);
        bool b1stSegm(true);
        bool bCurve(false);
        bool bSmoothFuz(false);
        bool bSegmFuz(false);
        basegfx::B2VectorContinuity eSmooth = basegfx::B2VectorContinuity::NONE;

        mrView.CheckPolyPossibilitiesHelper( mpMark.get(), b1stSmooth, b1stSegm, bCurve, bSmoothFuz, bSegmFuz, eSmooth );
    }
}

void MotionPathTag::addCustomHandles( SdrHdlList& rHandlerList )
{
    if( !mpPathObj )
        return;

    css::awt::Point aPos;
    if (mxOrigin.is())
        aPos = mxOrigin->getPosition();
    if( (aPos.X != maOriginPos.X) || (aPos.Y != maOriginPos.Y) )
    {
        const basegfx::B2DHomMatrix aTransform(basegfx::utils::createTranslateB2DHomMatrix(
            aPos.X - maOriginPos.X, aPos.Y - maOriginPos.Y));
        mxPolyPoly.transform( aTransform );
        mpPathObj->SetPathPoly( mxPolyPoly );
        maOriginPos = aPos;
    }

    SmartTagReference xThis( this );
    std::unique_ptr<SdPathHdl> pHdl(new SdPathHdl( xThis, mpPathObj ));
    pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
    pHdl->SetPageView( mrView.GetSdrPageView() );
    pHdl->SetObj(mpPathObj);
    rHandlerList.AddHdl( std::move(pHdl) );

    if( !isSelected() )
        return;

    mrView.GetSdrPageView()->SetHasMarkedObj(true);

    if( !mrView.IsFrameDragSingles() )
    {
        SdrHdlList aTemp( rHandlerList.GetView() );
        mpPathObj->AddToHdlList( aTemp );
        const SdrUShortCont& rMrkPnts = mpMark->GetMarkedPoints();

        for( size_t nHandle = 0; nHandle < aTemp.GetHdlCount(); ++nHandle )
        {
            SdrHdl* pTempHdl = aTemp.GetHdl( nHandle );

            SmartHdl* pSmartHdl = new SmartHdl( xThis, mpPathObj, pTempHdl->GetPos(), pTempHdl->GetKind() );
            pSmartHdl->SetObjHdlNum( static_cast<sal_uInt32>(nHandle) );
            pSmartHdl->SetPolyNum( pTempHdl->GetPolyNum() );
            pSmartHdl->SetPointNum( pTempHdl->GetPointNum() );
            pSmartHdl->SetPlusHdl(  pTempHdl->IsPlusHdl() );
            pSmartHdl->SetSourceHdlNum( pTempHdl->GetSourceHdlNum() );
            pSmartHdl->SetPageView( mrView.GetSdrPageView() );

            rHandlerList.AddHdl( std::unique_ptr<SmartHdl>(pSmartHdl) );

            const bool bSelected = rMrkPnts.find( sal_uInt16(nHandle) ) != rMrkPnts.end();
            pSmartHdl->SetSelected(bSelected);

            if( mrView.IsPlusHandlesAlwaysVisible() || bSelected )
            {
                SdrHdlList plusList(nullptr);
                mpPathObj->AddToPlusHdlList(plusList, *pSmartHdl);
                sal_uInt32 nPlusHdlCnt=plusList.GetHdlCount();
                for (sal_uInt32 nPlusNum=0; nPlusNum<nPlusHdlCnt; nPlusNum++)
                {
                    SdrHdl* pPlusHdl = plusList.GetHdl(nPlusNum);
                    pPlusHdl->SetObj(mpPathObj);
                    pPlusHdl->SetPageView(mrView.GetSdrPageView());
                    pPlusHdl->SetPlusHdl(true);
                }
                plusList.MoveTo(rHandlerList);
            }
        }
    }
    else
    {
        ::tools::Rectangle aRect(mpPathObj->GetCurrentBoundRect());

        if(!aRect.IsEmpty())
        {
            size_t nCount = rHandlerList.GetHdlCount();

            bool bWdt0=aRect.Left()==aRect.Right();
            bool bHgt0=aRect.Top()==aRect.Bottom();
            if (bWdt0 && bHgt0)
            {
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.TopLeft(),SdrHdlKind::UpperLeft));
            }
            else if (bWdt0 || bHgt0)
            {
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.TopLeft()    ,SdrHdlKind::UpperLeft));
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.BottomRight(),SdrHdlKind::LowerRight));
            }
            else // !bWdt0 && !bHgt0
            {
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.TopLeft()     ,SdrHdlKind::UpperLeft));
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.TopCenter()   ,SdrHdlKind::Upper));
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.TopRight()    ,SdrHdlKind::UpperRight));
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.LeftCenter()  ,SdrHdlKind::Left ));
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.RightCenter() ,SdrHdlKind::Right));
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.BottomLeft()  ,SdrHdlKind::LowerLeft));
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.BottomCenter(),SdrHdlKind::Lower));
                rHandlerList.AddHdl(std::make_unique<SmartHdl>( xThis, mpPathObj, aRect.BottomRight() ,SdrHdlKind::LowerRight));
            }

            while( nCount < rHandlerList.GetHdlCount() )
            {
                rHandlerList.GetHdl(nCount++)->SetPageView( mrView.GetSdrPageView() );
            }
        }
    }
}

void MotionPathTag::disposing()
{
    Reference< XChangesNotifier > xNotifier( mpEffect->getNode(), UNO_QUERY );
    if( xNotifier.is() )
    {
        xNotifier->removeChangesListener( this );
    }

    if( mpPathObj )
    {
        SdrObject* pTemp(mpPathObj);
        mpPathObj = nullptr;
        mrView.updateHandles();

        // always use SdrObject::Free(...) for SdrObjects (!)
        SdrObject::Free(pTemp);
    }

    mpMark.reset();

    SmartTag::disposing();
}

void MotionPathTag::deselect()
{
    SmartTag::deselect();

    if( mpMark )
    {
        SdrUShortCont& rPts = mpMark->GetMarkedPoints();
        rPts.clear();
    }

    selectionChanged();
}

void MotionPathTag::selectionChanged()
{
    if( mrView.GetViewShell() && mrView.GetViewShell()->GetViewFrame() )
    {
        SfxBindings& rBindings = mrView.GetViewShell()->GetViewFrame()->GetBindings();
        rBindings.InvalidateAll(true);
    }
}

// IPolyPolygonEditorController

void MotionPathTag::DeleteMarkedPoints()
{
    if( !(mpPathObj && IsDeleteMarkedPointsPossible()) )
        return;

    mrView.BrkAction();

    SdrUShortCont& rPts = mpMark->GetMarkedPoints();
    PolyPolygonEditor aEditor( mpPathObj->GetPathPoly());
    if (aEditor.DeletePoints(rPts))
    {
        if( aEditor.GetPolyPolygon().count() )
        {
            mpPathObj->SetPathPoly( aEditor.GetPolyPolygon() );
        }

        mrView.UnmarkAllPoints();
        mrView.MarkListHasChanged();
        mrView.updateHandles();
    }
}

bool MotionPathTag::IsDeleteMarkedPointsPossible() const
{
    return mpPathObj && isSelected() && (GetMarkedPointCount() != 0);
}

void MotionPathTag::RipUpAtMarkedPoints()
{
    // not supported for motion path
}

bool MotionPathTag::IsRipUpAtMarkedPointsPossible() const
{
    // not supported for motion path
    return false;
}

bool MotionPathTag::IsSetMarkedSegmentsKindPossible() const
{
    if( mpPathObj )
        return mrView.IsSetMarkedSegmentsKindPossible();
    else
        return false;
}

SdrPathSegmentKind MotionPathTag::GetMarkedSegmentsKind() const
{
    if( mpPathObj )
        return mrView.GetMarkedSegmentsKind();
    else
        return SdrPathSegmentKind::Line;
}

void MotionPathTag::SetMarkedSegmentsKind(SdrPathSegmentKind eKind)
{
    if(mpPathObj && isSelected() && (GetMarkedPointCount() != 0))
    {
        SdrUShortCont& rPts = mpMark->GetMarkedPoints();
        PolyPolygonEditor aEditor( mpPathObj->GetPathPoly() );
        if (aEditor.SetSegmentsKind(eKind, rPts))
        {
            mpPathObj->SetPathPoly(aEditor.GetPolyPolygon());
            mrView.MarkListHasChanged();
            mrView.updateHandles();
        }
    }
}

bool MotionPathTag::IsSetMarkedPointsSmoothPossible() const
{
    if( mpPathObj )
        return mrView.IsSetMarkedPointsSmoothPossible();
    else
        return false;
}

SdrPathSmoothKind MotionPathTag::GetMarkedPointsSmooth() const
{
    if( mpPathObj )
        return mrView.GetMarkedPointsSmooth();
    else
        return SdrPathSmoothKind::Angular;
}

void MotionPathTag::SetMarkedPointsSmooth(SdrPathSmoothKind eKind)
{
    basegfx::B2VectorContinuity eFlags;

    if(SdrPathSmoothKind::Angular == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::NONE;
    }
    else if(SdrPathSmoothKind::Asymmetric == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::C1;
    }
    else if(SdrPathSmoothKind::Symmetric == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::C2;
    }
    else
    {
        return;
    }

    if(mpPathObj && mpMark && isSelected() && (GetMarkedPointCount() != 0))
    {
        SdrUShortCont& rPts = mpMark->GetMarkedPoints();
        PolyPolygonEditor aEditor( mpPathObj->GetPathPoly());
        if (aEditor.SetPointsSmooth(eFlags, rPts))
        {
            mpPathObj->SetPathPoly(aEditor.GetPolyPolygon());
            mrView.MarkListHasChanged();
            mrView.updateHandles();
        }
    }
}

bool MotionPathTag::IsOpenCloseMarkedObjectsPossible() const
{
    // not supported for motion path
    return false;
}

SdrObjClosedKind MotionPathTag::GetMarkedObjectsClosedState() const
{
    // not supported for motion path
    return SdrObjClosedKind::Open;
}

// XChangesListener
void SAL_CALL MotionPathTag::changesOccurred( const ChangesEvent& /*Event*/ )
{
    if( mpPathObj && !mbInUpdatePath && (mpEffect->getPath() != msLastPath) )
    {
        mbInUpdatePath =true;
        msLastPath = mpEffect->getPath();
        mpEffect->updateSdrPathObjFromPath( *mpPathObj );
        mbInUpdatePath = false;
        updatePathAttributes();
        mrView.updateHandles();
    }
}

void SAL_CALL MotionPathTag::disposing( const EventObject& /*Source*/ )
{
    if( mpPathObj )
        Dispose();
}

Any SAL_CALL MotionPathTag::queryInterface( const css::uno::Type& aType )
{
    if( aType == cppu::UnoType<XChangesListener>::get() )
        return Any( Reference< XChangesListener >( this ) );
    if( aType == cppu::UnoType<XEventListener>::get() )
        return Any( Reference< XEventListener >( this ) );
    if( aType == cppu::UnoType<XInterface>::get() )
        return Any( Reference< XInterface >( this ) );

    return Any();
}

void SAL_CALL MotionPathTag::acquire() throw ()
{
    SimpleReferenceComponent::acquire();
}

void SAL_CALL MotionPathTag::release(  ) throw ()
{
    SimpleReferenceComponent::release();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
