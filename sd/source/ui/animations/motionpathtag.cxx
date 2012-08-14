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

using ::rtl::OUString;
using ::sdr::PolyPolygonEditor;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::drawing;

namespace sd
{

const sal_uInt32 SMART_TAG_HDL_NUM = SAL_MAX_UINT32;
static const int DRGPIX     = 2;                               // Drag MinMove in Pixel

// --------------------------------------------------------------------

class PathDragMove : public SdrDragMove
{
private:
    basegfx::B2DPolyPolygon         maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries();

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

    virtual bool BeginSdrDrag();
    virtual bool EndSdrDrag(bool bCopy);

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
    return sal_True;
}

bool PathDragMove::EndSdrDrag(bool /*bCopy*/)
{
    Hide();
    if( mxTag.is() )
        mxTag->MovePath( DragStat().GetDX(), DragStat().GetDY() );
    return sal_True;
}
// --------------------------------------------------------------------

class PathDragResize : public SdrDragResize
{
private:
    basegfx::B2DPolyPolygon         maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries();

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

    virtual bool EndSdrDrag(bool bCopy);
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
    return sal_True;
}

// --------------------------------------------------------------------

class PathDragObjOwn : public SdrDragObjOwn
{
private:
    basegfx::B2DPolyPolygon         maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries();

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

    virtual bool EndSdrDrag(bool bCopy);
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

// --------------------------------------------------------------------

class SdPathHdl : public SmartHdl
{
public:
    SdPathHdl( const SmartTagReference& xTag, SdrPathObj* mpPathObj );
    virtual ~SdPathHdl();
    virtual void CreateB2dIAObject();
    virtual sal_Bool IsFocusHdl() const;
    virtual Pointer GetSdrDragPointer() const;
    virtual bool isMarkable() const;

private:
    SdrPathObj* mpPathObj;
    rtl::Reference< MotionPathTag > mxTag;
};

// --------------------------------------------------------------------

SdPathHdl::SdPathHdl( const SmartTagReference& xTag, SdrPathObj* pPathObj )
: SmartHdl( xTag, pPathObj->GetCurrentBoundRect().TopLeft() )
, mpPathObj( pPathObj )
, mxTag( dynamic_cast< MotionPathTag* >( xTag.get() ) )
{
}

// --------------------------------------------------------------------

SdPathHdl::~SdPathHdl()
{
}

// --------------------------------------------------------------------

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
                        rtl::Reference< ::sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
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

// --------------------------------------------------------------------

sal_Bool SdPathHdl::IsFocusHdl() const
{
    return sal_False;
}

// --------------------------------------------------------------------

bool SdPathHdl::isMarkable() const
{
    return false;
}

// --------------------------------------------------------------------

Pointer SdPathHdl::GetSdrDragPointer() const
{
    PointerStyle eStyle = POINTER_NOTALLOWED;
    if( mxTag.is() )
    {
        if( mxTag->isSelected() )
        {
            if( !mxTag->getView().IsFrameDragSingles() && mxTag->getView().IsInsObjPointMode() )
                eStyle = POINTER_CROSS;
            else
                eStyle = POINTER_MOVE;
        }
        else
        {
            eStyle = POINTER_ARROW;

        }
    }
    return Pointer( eStyle );
}

// ====================================================================

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
    maOriginPos = mxOrigin->getPosition();

    SdrPage* pPage = mrView.GetSdrPageView()->GetPage();
    if( pPage )
    {
        mpPathObj->SetPage( pPage );
        mpPathObj->SetObjList( pPage );
    }

    XDash aDash( XDASH_RECT, 1, 80, 1, 80, 80);
    rtl::OUString aEmpty( "?" );
    mpPathObj->SetMergedItem( XLineDashItem( aEmpty, aDash ) );
    mpPathObj->SetMergedItem( XLineStyleItem( XLINE_DASH ) );
    mpPathObj->SetMergedItem( XLineColorItem(aEmpty, ::Color(COL_GRAY)) );
    mpPathObj->SetMergedItem( XFillStyleItem( XFILL_NONE ) );

    ::basegfx::B2DPolygon aStartArrow;
    aStartArrow.append(::basegfx::B2DPoint(20.0, 0.0));
    aStartArrow.append(::basegfx::B2DPoint(0.0,  0.0));
    aStartArrow.append(::basegfx::B2DPoint(10.0, 30.0));
    aStartArrow.setClosed(true);
    mpPathObj->SetMergedItem(XLineStartItem(aEmpty,::basegfx::B2DPolyPolygon(aStartArrow)));
    mpPathObj->SetMergedItem(XLineStartWidthItem(400));
    mpPathObj->SetMergedItem(XLineStartCenterItem(sal_True));

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

// --------------------------------------------------------------------

MotionPathTag::~MotionPathTag()
{
    DBG_ASSERT( mpPathObj == 0, "sd::MotionPathTag::~MotionPathTag(), dispose me first!" );
    Dispose();
}

// --------------------------------------------------------------------

void MotionPathTag::updatePathAttributes()
{
    rtl::OUString aEmpty( "?" );

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
        mpPathObj->SetMergedItem(XLineEndCenterItem(sal_True));
    }
    else
    {
        mpPathObj->SetMergedItem(XLineEndItem());
    }
}

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

void MotionPathTag::MovePath( int nDX, int nDY )
{
    if( mpPathObj )
    {
        mpPathObj->Move( Size( nDX, nDY ) );
        mrView.updateHandles();
    }
}

// --------------------------------------------------------------------

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
            mrView.GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(SID_BEZIER_EDIT, SFX_CALLMODE_ASYNCHRON);
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

                sal_uInt32 nInsPointNum = mpPathObj->NbcInsPointOld(aPt, bNewObj, sal_True);

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

// --------------------------------------------------------------------

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
        sal_Bool bForward(!rKEvt.GetKeyCode().IsShift());

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

            for(sal_uInt32 a(0); !pNewOne && a < rHdlList.GetHdlCount(); a++)
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
                sal_Bool bWasSnapEnabled = mrView.IsSnapEnabled();

                // switch snapping off
                if(!bWasNoSnap)
                    ((SdrDragStat&)mrView.GetDragStat()).SetNoSnap(sal_True);
                if(bWasSnapEnabled)
                    mrView.SetSnapEnabled(sal_False);

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

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

sal_Bool MotionPathTag::MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark )
{
    sal_Bool bRet=sal_False;
    if( mpPathObj && mrView.IsPointMarkable( rHdl ) && (rHdl.GetKind() != HDL_SMARTTAG) )
    {
        SmartHdl* pSmartHdl = dynamic_cast< SmartHdl* >( &rHdl );
        if( pSmartHdl && pSmartHdl->getTag().get() == this )
        {
            mpMark->ForceMarkedPoints();
            if (mrView.MarkPointHelper(&rHdl,mpMark,bUnmark))
            {
                mrView.MarkListHasChanged();
                bRet=sal_True;
            }
        }
    }
    return bRet;
}

// --------------------------------------------------------------------

sal_Bool MotionPathTag::MarkPoints(const Rectangle* pRect, sal_Bool bUnmark )
{
    sal_Bool bChgd=sal_False;

    if( mpPathObj && isSelected() )
    {
        sal_Int32 nHdlNum = mrView.GetHdlList().GetHdlCount() - 1;
        while( nHdlNum > 0 )
        {
            SmartHdl* pHdl = dynamic_cast< SmartHdl* >( mrView.GetHdl( sal::static_int_cast< sal_uLong >( nHdlNum-- ) ) );

            if( pHdl && (pHdl->getTag().get() == this) && mrView.IsPointMarkable(*pHdl) && pHdl->IsSelected()==bUnmark)
            {
                Point aPos(pHdl->GetPos());
                if( pRect==NULL || pRect->IsInside(aPos))
                {
                    if( mrView.MarkPointHelper(pHdl,mpMark,bUnmark) )
                        bChgd=sal_True;
                }
            }
        }

        if(bChgd)
            mrView.MarkListHasChanged();
    }

    return bChgd;
}

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

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
                basegfx::B2VectorContinuity eSmooth = basegfx::CONTINUITY_NONE;

                mrView.CheckPolyPossibilitiesHelper( mpMark, b1stSmooth, b1stSegm, bCurve, bSmoothFuz, bSegmFuz, eSmooth );
            }
        }
    }
}

// --------------------------------------------------------------------

void MotionPathTag::addCustomHandles( SdrHdlList& rHandlerList )
{
    if( mpPathObj )
    {
        ::com::sun::star::awt::Point aPos( mxOrigin->getPosition() );
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
            mrView.GetSdrPageView()->SetHasMarkedObj(sal_True);

            if( !mrView.IsFrameDragSingles() )
            {
                SdrHdlList aTemp( rHandlerList.GetView() );
                mpPathObj->AddToHdlList( aTemp );
                const SdrUShortCont* pMrkPnts=mpMark->GetMarkedPoints();

                sal_uInt32 nHandle;
                for( nHandle = 0; nHandle < aTemp.GetHdlCount(); ++nHandle )
                {
                    SdrHdl* pTempHdl = aTemp.GetHdl( nHandle );

                    SmartHdl* pSmartHdl = new SmartHdl( xThis, mpPathObj, pTempHdl->GetPos(), pTempHdl->GetKind() );
                    pSmartHdl->SetObjHdlNum( nHandle );
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
                                pPlusHdl->SetPlusHdl(sal_True);
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
                    sal_uLong nCount = rHandlerList.GetHdlCount();

                    sal_Bool bWdt0=aRect.Left()==aRect.Right();
                    sal_Bool bHgt0=aRect.Top()==aRect.Bottom();
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

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

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
        rBindings.InvalidateAll(sal_True);
    }
}
// --------------------------------------------------------------------
// IPolyPolygonEditorController
// --------------------------------------------------------------------

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

sal_Bool MotionPathTag::IsDeleteMarkedPointsPossible() const
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

sal_Bool MotionPathTag::IsSetMarkedSegmentsKindPossible() const
{
    if( mpPathObj )
        return mrView.IsSetMarkedSegmentsKindPossible();
    else
        return sal_False;
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

sal_Bool MotionPathTag::IsSetMarkedPointsSmoothPossible() const
{
    if( mpPathObj )
        return mrView.IsSetMarkedPointsSmoothPossible();
    else
        return sal_False;
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
        eFlags = basegfx::CONTINUITY_NONE;
    }
    else if(SDRPATHSMOOTH_ASYMMETRIC == eKind)
    {
        eFlags = basegfx::CONTINUITY_C1;
    }
    else if(SDRPATHSMOOTH_SYMMETRIC == eKind)
    {
        eFlags = basegfx::CONTINUITY_C2;
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

void MotionPathTag::CloseMarkedObjects(sal_Bool /*bToggle*/, sal_Bool /*bOpen*/ )
{
    // not supported for motion path
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
void SAL_CALL MotionPathTag::changesOccurred( const ChangesEvent& /*Event*/ ) throw (RuntimeException)
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

void SAL_CALL MotionPathTag::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
{
    if( mpPathObj )
        Dispose();
}

Any SAL_CALL MotionPathTag::queryInterface( const ::com::sun::star::uno::Type& aType ) throw (RuntimeException)
{
    if( aType == XChangesListener::static_type() )
        return Any( Reference< XChangesListener >( this ) );
    if( aType == XEventListener::static_type() )
        return Any( Reference< XEventListener >( this ) );
    if( aType == XInterface::static_type() )
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
