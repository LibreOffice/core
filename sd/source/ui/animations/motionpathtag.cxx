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

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
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
#include "View.hxx"
#include "motionpathtag.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"
#include "app.hrc"
#include "Window.hxx"

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
    basegfx::B2DPolyPolygon         maPathPolyPolygon;

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
        addSdrDragEntry(new SdrDragEntryPolyPolygon(maPathPolyPolygon));
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
    basegfx::B2DPolyPolygon         maPathPolyPolygon;

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
        addSdrDragEntry(new SdrDragEntryPolyPolygon(maPathPolyPolygon));
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
            const Point aRef( DragStat().Ref1() );
            basegfx::B2DHomMatrix aTrans(basegfx::tools::createTranslateB2DHomMatrix(-aRef.X(), -aRef.Y()));
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
    basegfx::B2DPolyPolygon         maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries() override;

public:
    PathDragObjOwn(SdrDragView& rNewView,
        const basegfx::B2DPolyPolygon& rPathPolyPolygon)
    :   SdrDragObjOwn(rNewView),
        maPathPolyPolygon(rPathPolyPolygon)
    {}

    PathDragObjOwn(SdrDragView& rNewView)
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
        addSdrDragEntry(new SdrDragEntryPolyPolygon(maPathPolyPolygon));
    }
}

bool PathDragObjOwn::EndSdrDrag(bool /*bCopy*/)
{
    Hide();

    SdrObject* pObj = GetDragObj();

    if(pObj)
    {
        return pObj->applySpecialDrag(DragStat());
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
    virtual ~SdPathHdl();
    virtual void CreateB2dIAObject() override;
    virtual bool IsFocusHdl() const override;
    virtual bool isMarkable() const override;

private:
    SdrPathObj* mpPathObj;
    rtl::Reference< MotionPathTag > mxTag;
};

SdPathHdl::SdPathHdl( const SmartTagReference& xTag, SdrPathObj* pPathObj )
: SmartHdl( xTag, pPathObj->GetCurrentBoundRect().TopLeft() )
, mpPathObj( pPathObj )
, mxTag( dynamic_cast< MotionPathTag* >( xTag.get() ) )
{
}

SdPathHdl::~SdPathHdl()
{
}

void SdPathHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden())
        {
            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is() && mpPathObj)
                        {
                            const sdr::contact::ViewContact& rVC = mpPathObj->GetViewContact();
                            const drawinglayer::primitive2d::Primitive2DSequence aSequence = rVC.getViewIndependentPrimitive2DSequence();
                            sdr::overlay::OverlayObject* pNew = new sdr::overlay::OverlayPrimitive2DSequenceObject(aSequence);

                            xManager->add(*pNew);
                            maOverlayGroup.append(*pNew);
                        }
                    }
                }
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
    mpPathObj = mpEffect->createSdrPathObjFromPath();
    mxPolyPoly = mpPathObj->GetPathPoly();
    if (mxOrigin.is())
        maOriginPos = mxOrigin->getPosition();

    SdrPage* pPage = mrView.GetSdrPageView()->GetPage();
    if( pPage )
    {
        mpPathObj->SetPage( pPage );
        mpPathObj->SetObjList( pPage );
    }

    XDash aDash( css::drawing::DashStyle_RECT, 1, 80, 1, 80, 80);
    OUString aEmpty( "?" );
    mpPathObj->SetMergedItem( XLineDashItem( aEmpty, aDash ) );
    mpPathObj->SetMergedItem( XLineStyleItem( drawing::LineStyle_DASH ) );
    mpPathObj->SetMergedItem( XLineColorItem(aEmpty, ::Color(COL_GRAY)) );
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

    mpMark = new SdrMark( mpPathObj, mrView.GetSdrPageView() );

    mpPathObj->AddListener( *this );

    Reference< XChangesNotifier > xNotifier( mpEffect->getNode(), UNO_QUERY );
    if( xNotifier.is() )
    {
        Reference< XChangesListener > xListener( this );
        xNotifier->addChangesListener( this );
    }
}

MotionPathTag::~MotionPathTag()
{
    DBG_ASSERT( mpPathObj == 0, "sd::MotionPathTag::~MotionPathTag(), dispose me first!" );
    Dispose();
}

void MotionPathTag::updatePathAttributes()
{
    OUString aEmpty( "?" );

    ::basegfx::B2DPolygon aCandidate;
    if( mxPolyPoly.count() )
    {
        aCandidate = mxPolyPoly.getB2DPolygon(0);
        ::basegfx::tools::checkClosed( aCandidate );
    }

    if( !aCandidate.isClosed() )
    {
        ::basegfx::B2DPolygon aEndArrow;
        aEndArrow.append(::basegfx::B2DPoint(10.0, 0.0));
        aEndArrow.append(::basegfx::B2DPoint(0.0, 30.0));
        aEndArrow.append(::basegfx::B2DPoint(20.0, 30.0));
        aEndArrow.setClosed(true);
        mpPathObj->SetMergedItem(XLineEndItem(aEmpty,::basegfx::B2DPolyPolygon(aEndArrow)));
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
    if( mpPathObj && !mbInUpdatePath && dynamic_cast< const SdrHint* >( &rHint ) && (mpEffect.get() != 0) )
    {
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

                sal_uInt32 nInsPointNum = mpPathObj->NbcInsPointOld(aPt, bNewObj, true);

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
                            pHdl = NULL;
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
                    const sal_uInt16 nDrgLog = (sal_uInt16)pOut->PixelToLogic(Size(DRGPIX,0)).Width();

                    rtl::Reference< MotionPathTag > xTag( this );
                    SdrDragMethod* pDragMethod;

                    // #i95646# add DragPoly as geometry to each local SdrDragMethod to be able
                    // to create the needed local SdrDragEntry for it in createSdrDragEntries()
                    const basegfx::B2DPolyPolygon aDragPoly(mpPathObj->GetPathPoly());

                    if( (pHdl->GetKind() == HDL_MOVE) || (pHdl->GetKind() == HDL_SMARTTAG) )
                    {
                        pDragMethod = new PathDragMove( mrView, xTag, aDragPoly );
                        pHdl->SetPos( aMDPos );
                    }
                    else if( pHdl->GetKind() == HDL_POLY )
                    {
                        pDragMethod = new PathDragObjOwn( mrView, aDragPoly );
                    }
                    else
                    {
                        pDragMethod = new PathDragResize( mrView, xTag, aDragPoly );
                    }

                    mrView.BegDragObj(aMDPos, NULL, pHdl, nDrgLog, pDragMethod );
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

        ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

        // guarantee visibility of focused handle
        SdrHdl* pHdl = rHdlList.GetFocusHdl();

        if(pHdl)
        {
            Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
            if( pWindow )
            {
                Point aHdlPosition(pHdl->GetPos());
                Rectangle aVisRect(aHdlPosition - Point(100, 100), Size(200, 200));
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

    if(pHdl && pHdl->GetKind() == HDL_POLY )
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

        if(0L == rHdlList.GetFocusHdl())
        {
            // restore point with focus
            SdrHdl* pNewOne = 0L;

            for(size_t a = 0; !pNewOne && a < rHdlList.GetHdlCount(); ++a)
            {
                SdrHdl* pAct = rHdlList.GetHdl(a);

                if(pAct && pAct->GetKind() == HDL_POLY && pAct->GetPolyNum() == nPol && pAct->GetPointNum() == nPnt)
                    pNewOne = pAct;
            }

            if(pNewOne)
                ((SdrHdlList&)rHdlList).SetFocusHdl(pNewOne);
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
        Size aLogicSizeOnePixel = (pOut) ? pOut->PixelToLogic(Size(1,1)) : Size(100, 100);
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
            SdrDragMethod* pDragMethod = 0;
            if( (pHdl->GetKind() == HDL_MOVE) || (pHdl->GetKind() == HDL_SMARTTAG) )
            {
                pDragMethod = new PathDragMove( mrView, xTag );
            }
            else if( pHdl->GetKind() == HDL_POLY )
            {
                pDragMethod = new PathDragObjOwn( mrView );
            }
            else if( pHdl->GetKind() != HDL_BWGT )
            {
                pDragMethod = new PathDragResize( mrView, xTag );
            }
            mrView.BegDragObj(aStartPoint, 0, pHdl, 0, pDragMethod);

            if(mrView.IsDragObj())
            {
                bool bWasNoSnap = mrView.GetDragStat().IsNoSnap();
                bool bWasSnapEnabled = mrView.IsSnapEnabled();

                // switch snapping off
                if(!bWasNoSnap)
                    ((SdrDragStat&)mrView.GetDragStat()).SetNoSnap();
                if(bWasSnapEnabled)
                    mrView.SetSnapEnabled(false);

                mrView.MovAction(aEndPoint);
                mrView.EndDragObj();

                // restore snap
                if(!bWasNoSnap)
                    ((SdrDragStat&)mrView.GetDragStat()).SetNoSnap(bWasNoSnap);
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

sal_uLong MotionPathTag::GetMarkablePointCount() const
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

sal_uLong MotionPathTag::GetMarkedPointCount() const
{
    if( mpMark )
    {
        const SdrUShortCont* pPts=mpMark->GetMarkedPoints();
        return pPts ? pPts->size() : 0;
    }
    else
    {
        return 0;
    }
}

bool MotionPathTag::MarkPoint(SdrHdl& rHdl, bool bUnmark )
{
    bool bRet=false;
    if( mpPathObj && mrView.IsPointMarkable( rHdl ) && (rHdl.GetKind() != HDL_SMARTTAG) )
    {
        SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( &rHdl );
        if( pSmartHdl && pSmartHdl->getTag().get() == this )
        {
            mpMark->ForceMarkedPoints();
            if (mrView.MarkPointHelper(&rHdl,mpMark,bUnmark))
            {
                mrView.MarkListHasChanged();
                bRet=true;
            }
        }
    }
    return bRet;
}

bool MotionPathTag::MarkPoints(const Rectangle* pRect, bool bUnmark )
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
                if( pRect==NULL || pRect->IsInside(aPos))
                {
                    if( mrView.MarkPointHelper(pHdl,mpMark,bUnmark) )
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
        rContext = SDRCONTEXT_POINTEDIT;
        return true;
    }
    else
    {
        return false;
    }
}

void MotionPathTag::CheckPossibilities()
{
    if( mpPathObj )
    {
        if( isSelected() )
        {
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

                mrView.CheckPolyPossibilitiesHelper( mpMark, b1stSmooth, b1stSegm, bCurve, bSmoothFuz, bSegmFuz, eSmooth );
            }
        }
    }
}

void MotionPathTag::addCustomHandles( SdrHdlList& rHandlerList )
{
    if( mpPathObj )
    {
        ::com::sun::star::awt::Point aPos;
        if (mxOrigin.is())
            aPos = mxOrigin->getPosition();
        if( (aPos.X != maOriginPos.X) || (aPos.Y != maOriginPos.Y) )
        {
            const basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(
                aPos.X - maOriginPos.X, aPos.Y - maOriginPos.Y));
            mxPolyPoly.transform( aTransform );
            mpPathObj->SetPathPoly( mxPolyPoly );
            maOriginPos = aPos;
        }

        SmartTagReference xThis( this );
        SdPathHdl* pHdl = new SdPathHdl( xThis, mpPathObj );
        pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
        pHdl->SetPageView( mrView.GetSdrPageView() );

        pHdl->SetObj(mpPathObj);
        rHandlerList.AddHdl( pHdl );

        if( isSelected() )
        {
            mrView.GetSdrPageView()->SetHasMarkedObj(true);

            if( !mrView.IsFrameDragSingles() )
            {
                SdrHdlList aTemp( rHandlerList.GetView() );
                mpPathObj->AddToHdlList( aTemp );
                const SdrUShortCont* pMrkPnts=mpMark->GetMarkedPoints();

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

                    rHandlerList.AddHdl( pSmartHdl );

                    const bool bSelected= pMrkPnts && pMrkPnts->find( sal_uInt16(nHandle) ) != pMrkPnts->end();
                    pSmartHdl->SetSelected(bSelected);

                    if( mrView.IsPlusHandlesAlwaysVisible() || bSelected )
                    {
                        sal_uInt32 nPlusAnz=mpPathObj->GetPlusHdlCount(*pSmartHdl);
                        for (sal_uInt32 nPlusNum=0; nPlusNum<nPlusAnz; nPlusNum++)
                        {
                            SdrHdl* pPlusHdl = mpPathObj->GetPlusHdl(*pSmartHdl,nPlusNum);
                            if (pPlusHdl!=NULL)
                            {
                                pPlusHdl->SetObj(mpPathObj);
                                pPlusHdl->SetPageView(mrView.GetSdrPageView());
                                pPlusHdl->SetPlusHdl(true);
                                rHandlerList.AddHdl(pPlusHdl);
                            }
                        }
                    }
                }
            }
            else
            {
                Rectangle aRect(mpPathObj->GetCurrentBoundRect());

                if(!aRect.IsEmpty())
                {
                    size_t nCount = rHandlerList.GetHdlCount();

                    bool bWdt0=aRect.Left()==aRect.Right();
                    bool bHgt0=aRect.Top()==aRect.Bottom();
                    if (bWdt0 && bHgt0)
                    {
                        rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.TopLeft(),HDL_UPLFT));
                    }
                    else if (bWdt0 || bHgt0)
                    {
                        rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.TopLeft()    ,HDL_UPLFT));
                        rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.BottomRight(),HDL_LWRGT));
                    }
                    else
                    {
                        if (!bWdt0 && !bHgt0) rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.TopLeft()     ,HDL_UPLFT));
                        if (          !bHgt0) rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.TopCenter()   ,HDL_UPPER));
                        if (!bWdt0 && !bHgt0) rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.TopRight()    ,HDL_UPRGT));
                        if (!bWdt0          ) rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.LeftCenter()  ,HDL_LEFT ));
                        if (!bWdt0          ) rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.RightCenter() ,HDL_RIGHT));
                        if (!bWdt0 && !bHgt0) rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.BottomLeft()  ,HDL_LWLFT));
                        if (          !bHgt0) rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.BottomCenter(),HDL_LOWER));
                        if (!bWdt0 && !bHgt0) rHandlerList.AddHdl(new SmartHdl( xThis, mpPathObj, aRect.BottomRight() ,HDL_LWRGT));
                    }

                    while( nCount < rHandlerList.GetHdlCount() )
                    {
                        rHandlerList.GetHdl(nCount++)->SetPageView( mrView.GetSdrPageView() );
                    }
                }
            }
        }
    }
}

void MotionPathTag::disposing()
{
    Reference< XChangesNotifier > xNotifier( mpEffect->getNode(), UNO_QUERY );
    if( xNotifier.is() )
    {
        Reference< XChangesListener > xListener( this );
        xNotifier->removeChangesListener( this );
    }

    if( mpPathObj )
    {
        SdrPathObj* pPathObj = mpPathObj;
        mpPathObj = 0;
        mrView.updateHandles();
        delete pPathObj;
    }

    if( mpMark )
    {
        delete mpMark;
        mpMark = 0;
    }

    SmartTag::disposing();
}

void MotionPathTag::deselect()
{
    SmartTag::deselect();

    if( mpMark )
    {
        SdrUShortCont* pPts = mpMark->GetMarkedPoints();

        if( pPts )
            pPts->clear();
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
    if( mpPathObj && IsDeleteMarkedPointsPossible() )
    {
        mrView.BrkAction();

        SdrUShortCont* pPts = mpMark->GetMarkedPoints();

        if( pPts )
        {
            PolyPolygonEditor aEditor( mpPathObj->GetPathPoly(), mpPathObj->IsClosed() );
            if( aEditor.DeletePoints( *pPts ) )
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
        return SDRPATHSEGMENT_LINE;
}

void MotionPathTag::SetMarkedSegmentsKind(SdrPathSegmentKind eKind)
{
    if(mpPathObj && isSelected() && (GetMarkedPointCount() != 0))
    {
        SdrUShortCont* pPts = mpMark->GetMarkedPoints();
        if(pPts)
        {
            PolyPolygonEditor aEditor( mpPathObj->GetPathPoly(), mpPathObj->IsClosed() );
            if(aEditor.SetSegmentsKind( eKind, *pPts ) )
            {
                mpPathObj->SetPathPoly(aEditor.GetPolyPolygon());
                mrView.MarkListHasChanged();
                mrView.updateHandles();
            }
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
        return SDRPATHSMOOTH_ANGULAR;
}

void MotionPathTag::SetMarkedPointsSmooth(SdrPathSmoothKind eKind)
{
    basegfx::B2VectorContinuity eFlags;

    if(SDRPATHSMOOTH_ANGULAR == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::NONE;
    }
    else if(SDRPATHSMOOTH_ASYMMETRIC == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::C1;
    }
    else if(SDRPATHSMOOTH_SYMMETRIC == eKind)
    {
        eFlags = basegfx::B2VectorContinuity::C2;
    }
    else
    {
        return;
    }

    if(mpPathObj && mpMark && isSelected() && (GetMarkedPointCount() != 0))
    {
        SdrUShortCont* pPts = mpMark->GetMarkedPoints();
        if(pPts)
        {
            PolyPolygonEditor aEditor( mpPathObj->GetPathPoly(), mpPathObj->IsClosed() );
            if(aEditor.SetPointsSmooth( eFlags, *pPts ) )
            {
                mpPathObj->SetPathPoly(aEditor.GetPolyPolygon());
                mrView.MarkListHasChanged();
                mrView.updateHandles();
            }
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
    return SDROBJCLOSED_OPEN;
}

// XChangesListener
void SAL_CALL MotionPathTag::changesOccurred( const ChangesEvent& /*Event*/ ) throw (RuntimeException, std::exception)
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

void SAL_CALL MotionPathTag::disposing( const EventObject& /*Source*/ ) throw (RuntimeException, std::exception)
{
    if( mpPathObj )
        Dispose();
}

Any SAL_CALL MotionPathTag::queryInterface( const ::com::sun::star::uno::Type& aType ) throw (RuntimeException, std::exception)
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
