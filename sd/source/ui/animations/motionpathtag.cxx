/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


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
#include <svx/svdlegacy.hxx>

using ::rtl::OUString;
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
    PathDragMove(SdrView& rNewView,
        const rtl::Reference <MotionPathTag >& xTag,
        const basegfx::B2DPolyPolygon& rPathPolyPolygon)
    :   SdrDragMove(rNewView),
        maPathPolyPolygon(rPathPolyPolygon),
        mxTag( xTag )
    {}

    PathDragMove(SdrView& rNewView,
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
            DragStat().SetActionRange(pPathObj->getObjectRange(&DragStat().GetSdrViewFromSdrDragStat()));
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
// --------------------------------------------------------------------

class PathDragResize : public SdrDragResize
{
private:
    basegfx::B2DPolyPolygon         maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries();

public:
    PathDragResize(SdrView& rNewView,
        const rtl::Reference <MotionPathTag >& xTag,
        const basegfx::B2DPolyPolygon& rPathPolyPolygon)
    :   SdrDragResize(rNewView),
        maPathPolyPolygon(rPathPolyPolygon),
        mxTag( xTag )
    {}

    PathDragResize(SdrView& rNewView,
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
            basegfx::B2DHomMatrix aTrans(basegfx::tools::createTranslateB2DHomMatrix(-DragStat().GetRef1()));
            aTrans.scale(double(aXFact), double(aYFact));
            aTrans.translate(DragStat().GetRef1());
            basegfx::B2DPolyPolygon aDragPoly(pPathObj->getB2DPolyPolygonInObjectCoordinates());
            aDragPoly.transform(aTrans);
            pPathObj->setB2DPolyPolygonInObjectCoordinates( aDragPoly );
        }
    }
    return true;
}

// --------------------------------------------------------------------

class PathDragObjOwn : public SdrDragObjOwn
{
private:
    basegfx::B2DPolyPolygon         maPathPolyPolygon;

protected:
    virtual void createSdrDragEntries();

public:
    PathDragObjOwn(SdrView& rNewView,
        const basegfx::B2DPolyPolygon& rPathPolyPolygon)
    :   SdrDragObjOwn(rNewView),
        maPathPolyPolygon(rPathPolyPolygon)
    {}

    PathDragObjOwn(SdrView& rNewView)
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
    SdPathHdl(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        const SmartTagReference& xTag);
    virtual bool IsFocusHdl() const;
    virtual Pointer GetSdrDragPointer() const;
    virtual bool isMarkable() const;

protected:
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);
    virtual ~SdPathHdl();

private:
    rtl::Reference< MotionPathTag > mxTag;
};

// --------------------------------------------------------------------

SdPathHdl::SdPathHdl(
    SdrHdlList& rHdlList,
    const SdrObject& rSdrHdlObject,
    const SmartTagReference& xTag)
: SmartHdl( rHdlList, &rSdrHdlObject, xTag, HDL_SMARTTAG, rSdrHdlObject.getSdrObjectTranslate() )
, mxTag( dynamic_cast< MotionPathTag* >( xTag.get() ) )
{
}

// --------------------------------------------------------------------

SdPathHdl::~SdPathHdl()
{
}

// --------------------------------------------------------------------

void SdPathHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
    {
    if(GetObj())
            {
        const sdr::contact::ViewContact& rVC = GetObj()->GetViewContact();
                            const drawinglayer::primitive2d::Primitive2DSequence aSequence = rVC.getViewIndependentPrimitive2DSequence();
                            sdr::overlay::OverlayObject* pNew = new sdr::overlay::OverlayPrimitive2DSequenceObject(aSequence);

        rOverlayManager.add(*pNew);
                            maOverlayGroup.append(*pNew);
                        }
                    }

// --------------------------------------------------------------------

bool SdPathHdl::IsFocusHdl() const
{
    return false;
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
            if( !mxTag->getView().IsFrameHandles() && mxTag->getView().IsInsObjPointMode() )
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
, maSelectedPoints()
, msLastPath( pEffect->getPath() )
, mbInUpdatePath( false )
{
    mpPathObj = mpEffect->createSdrPathObjFromPath(rView.getSdrModelFromSdrView());
    mxPolyPoly = mpPathObj->getB2DPolyPolygonInObjectCoordinates();
    maOriginPos = mxOrigin->getPosition();

    SdrPage* pPage = mrView.GetSdrPageView() ? &mrView.GetSdrPageView()->getSdrPageFromSdrPageView() : 0;
    if( pPage )
    {
        // mpPathObj->SetPage( pPage );
        // mpPathObj->SetObjList( pPage );
    }

    XDash aDash( XDASH_RECT, 1, 80, 1, 80, 80);
    String aEmpty( RTL_CONSTASCII_USTRINGPARAM("?") );
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
    mpPathObj->SetMergedItem(XLineStartCenterItem(true));

    updatePathAttributes();

    mpPathObj->SetMergedItem(XLineTransparenceItem(50));
    StartListening(*mpPathObj);

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
    String aEmpty( RTL_CONSTASCII_USTRINGPARAM("?") );

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

// --------------------------------------------------------------------

void MotionPathTag::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if( mpPathObj && !mbInUpdatePath && dynamic_cast< const SdrBaseHint* >( &rHint ) && (mpEffect.get() != 0) )
    {
        if( mxPolyPoly != mpPathObj->getB2DPolyPolygonInObjectCoordinates() )
        {
            mbInUpdatePath = true;
            mxPolyPoly = mpPathObj->getB2DPolyPolygonInObjectCoordinates();
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
        sdr::legacy::MoveSdrObject(*mpPathObj, Size( nDX, nDY ) );
        mrView.SetMarkHandles();
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
            const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
            const basegfx::B2DPoint aMDPos(pOut->GetInverseViewTransformation() * aPixelPos);

            if( !mrView.IsFrameHandles() && mrView.IsInsObjPointMode() && (rHdl.GetObjHdlNum() == SMART_TAG_HDL_NUM) )
            {
                // insert a point in edit mode
                const bool bNewObj = rMEvt.IsMod1();

                mrView.BrkAction();

                basegfx::B2DPoint aPt(aMDPos);

                if(bNewObj)
                    aPt = mrView.GetSnapPos(aPt);

                bool bClosed0(mpPathObj->IsClosedObj());

                sal_uInt32 nInsPointNum = mpPathObj->InsPointOld(Point(basegfx::fround(aPt.getX()), basegfx::fround(aPt.getY())), bNewObj);

                if(bClosed0 != mpPathObj->IsClosedObj())
                {
                    // Obj was closed implicit
                    // object changed
                    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*mpPathObj);
                    mpPathObj->SetChanged();
                }

                if(0xffffffff != nInsPointNum)
                {
                    mrView.MarkPoints(0, true); // unmarkall
                    mrView.SetMarkHandles();

                    bool bRet = mrView.BegDragObj(aMDPos, mrView.GetHdlByIndex(nInsPointNum+1), 0.0, new PathDragObjOwn( mrView ) );

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
                        mrView.MarkPoints(0, true); // unmarkall
                        pHdl = dynamic_cast< SmartHdl* >( mrView.PickHandle(aMDPos) );
                    }
                    else
                    {
                        if (mrView.IsPointMarked(*pHdl) )
                        {
                            mrView.MarkPoint(*pHdl, true); // unmark
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
                    rtl::Reference< MotionPathTag > xTag( this );
                    SdrDragMethod* pDragMethod;

                    // #i95646# add DragPoly as geometry to each local SdrDragMethod to be able
                    // to create the needed local SdrDragEntry for it in createSdrDragEntries()
                    const basegfx::B2DPolyPolygon aDragPoly(mpPathObj->getB2DPolyPolygonInObjectCoordinates());

                    if( (pHdl->GetKind() == HDL_MOVE) || (pHdl->GetKind() == HDL_SMARTTAG) )
                    {
                        pDragMethod = new PathDragMove( mrView, xTag, aDragPoly );
                        pHdl->setPosition( aMDPos );
                    }
                    else if( pHdl->GetKind() == HDL_POLY )
                    {
                        pDragMethod = new PathDragObjOwn( mrView, aDragPoly );
                    }
                    else
                    {
                        pDragMethod = new PathDragResize( mrView, xTag, aDragPoly );
                    }

                    const double fTolerance(basegfx::B2DVector(pOut->GetInverseViewTransformation() * basegfx::B2DVector(DRGPIX, 0.0)).getLength());
                    mrView.BegDragObj(aMDPos, pHdl, fTolerance, pDragMethod );
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
                const basegfx::B2DRange aRange(
                    pHdl->getPosition() - basegfx::B2DPoint(100.0, 100.0),
                    pHdl->getPosition() + basegfx::B2DPoint(100.0, 100.0));

                mrView.MakeVisibleAtView(aRange, *pWindow);
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
                mrView.MarkPoint(*pHdl, true); // unmark
            }
        }
        else
        {
            if(!rKEvt.GetKeyCode().IsShift())
            {
                mrView.MarkPoints(0, true); // unmarkall
            }
            mrView.MarkPoint(*pHdl);
        }

        if(0L == rHdlList.GetFocusHdl())
        {
            // restore point with focus
            SdrHdl* pNewOne = 0L;

            for(sal_uInt32 a(0); !pNewOne && a < rHdlList.GetHdlCount(); a++)
            {
                SdrHdl* pAct = rHdlList.GetHdlByIndex(a);

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
            const basegfx::B2DPoint aStartPoint(pHdl->getPosition());
            const basegfx::B2DPoint aEndPoint(aStartPoint + basegfx::B2DPoint(nX, nY));

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

            mrView.BegDragObj(aStartPoint, pHdl, 0.0, pDragMethod);

            if(mrView.IsDragObj())
            {
                const bool bWasNoSnap(mrView.GetDragStat().IsNoSnap());
                const bool bWasSnapEnabled(mrView.IsSnapEnabled());

                // switch snapping off
                if(!bWasNoSnap)
                    ((SdrDragStat&)mrView.GetDragStat()).SetNoSnap(true);
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

// --------------------------------------------------------------------

sal_uInt32 MotionPathTag::GetMarkablePointCount() const
{
    if( mpPathObj && isSelected() )
    {
        return mpPathObj->GetObjectPointCount();
    }
    else
    {
        return 0;
    }
}

// --------------------------------------------------------------------

sal_uInt32 MotionPathTag::GetMarkedPointCount() const
{
    return maSelectedPoints.size();
}

// --------------------------------------------------------------------

bool MotionPathTag::MarkPoint(SdrHdl& rHdl, bool bUnmark )
{
    bool bRet(false);

    if( mpPathObj && mrView.IsPointMarkable( rHdl ) && (rHdl.GetKind() != HDL_SMARTTAG) )
    {
        const SmartHdl* pSmartHdl = dynamic_cast< const SmartHdl* >( &rHdl );

        if( pSmartHdl && pSmartHdl->getTag().get() == this )
        {
            if (mrView.MarkPointHelper(&rHdl, bUnmark))
            {
//              mrView.MarkListHasChanged();
                bRet = true;
            }
        }
    }

    return bRet;
}

// --------------------------------------------------------------------

bool MotionPathTag::MarkPoints(const basegfx::B2DRange* pRange, bool bUnmark)
{
    bool bChgd(false);

    if( mpPathObj && isSelected() )
    {
        sal_uInt32 nHdlNum(mrView.GetHdlList().GetHdlCount());

        while(nHdlNum)
        {
            nHdlNum--;
            SmartHdl* pHdl = dynamic_cast< SmartHdl* >(mrView.GetHdlByIndex(nHdlNum));

            if( pHdl && (pHdl->getTag().get() == this) && mrView.IsPointMarkable(*pHdl) && pHdl->IsSelected()==bUnmark)
            {
                const basegfx::B2DPoint aPosPoint(pHdl->getPosition());

                if( !pRange || pRange->isInside(aPosPoint))
                {
                    if( mrView.MarkPointHelper(pHdl, bUnmark) )
                    {
                        bChgd = true;
                    }
                }
            }
        }
    }

    return bChgd;
}

// --------------------------------------------------------------------

bool MotionPathTag::getContext( SdrViewContext& rContext )
{
    if( mpPathObj && isSelected() && !mrView.IsFrameHandles() )
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
            mrView.SetMoveAllowedOnSelection( true );
            mrView.SetMoveProtected( false );
            mrView.SetResizeFreeAllowed( true );
            mrView.SetResizePropAllowed( true );
            mrView.SetResizeProtected( false );

            if( !mrView.IsFrameHandles() && maSelectedPoints.size() )
            {
                bool b1stSmooth(true);
                bool b1stSegm(true);
                bool bCurve(false);
                bool bSmoothFuz(false);
                bool bSegmFuz(false);
                basegfx::B2VectorContinuity eSmooth = basegfx::CONTINUITY_NONE;

                mrView.CheckPolyPossibilitiesHelper( *mpPathObj, maSelectedPoints,
                    b1stSmooth, b1stSegm, bCurve, bSmoothFuz, bSegmFuz, eSmooth );
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
            mpPathObj->setB2DPolyPolygonInObjectCoordinates( mxPolyPoly );
            maOriginPos = aPos;
        }

        SmartTagReference xThis( this );
        SdPathHdl* pHdl = new SdPathHdl( rHandlerList, *mpPathObj, xThis );
        pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );

        if( isSelected() )
        {
            if( !mrView.IsFrameHandles() )
            {
                SdrHdlList aTemp( rHandlerList.GetViewFromSdrHdlList() );
                mpPathObj->AddToHdlList( aTemp );
                sal_uInt32 nHandle;
                for( nHandle = 0; nHandle < aTemp.GetHdlCount(); ++nHandle )
                {
                    SdrHdl* pTempHdl = aTemp.GetHdlByIndex( nHandle );
                    SmartHdl* pSmartHdl = new SmartHdl(rHandlerList, mpPathObj, xThis, nHandle, *pTempHdl);

                    const bool bSelected(maSelectedPoints.count(nHandle));
                    pSmartHdl->SetSelected(bSelected);

                    if( mrView.IsPlusHandlesAlwaysVisible() || bSelected )
                    {
                        sal_uInt32 nPlusAnz=mpPathObj->GetPlusHdlCount(*pSmartHdl);
                        for (sal_uInt32 nPlusNum=0; nPlusNum<nPlusAnz; nPlusNum++)
                        {
                            mpPathObj->GetPlusHdl(rHandlerList, *mpPathObj, *pSmartHdl, nPlusNum);
                        }
                    }
                }
            }
            else
            {
                const basegfx::B2DRange aRange(mpPathObj->getObjectRange(&mrView));

                if(!aRange.isEmpty())
                {
                    sal_uInt32 nCount(rHandlerList.GetHdlCount());
                    const bool bWdt0(basegfx::fTools::equalZero(aRange.getWidth()));
                    const bool bHgt0(basegfx::fTools::equalZero(aRange.getHeight()));

                    if (bWdt0 && bHgt0)
                    {
                        new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_UPLFT, aRange.getMinimum());
                    }
                    else if (bWdt0 || bHgt0)
                    {
                        new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_UPLFT, aRange.getMinimum());
                        new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_LWRGT, aRange.getMaximum());
                    }
                    else
                    {
                        if (!bWdt0 && !bHgt0) new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_UPLFT, aRange.getMinimum());
                        if (          !bHgt0) new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_UPPER, basegfx::B2DPoint(aRange.getCenterX(), aRange.getMinY()));
                        if (!bWdt0 && !bHgt0) new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_UPRGT, basegfx::B2DPoint(aRange.getMaxX(), aRange.getMinY()));
                        if (!bWdt0          ) new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_LEFT, basegfx::B2DPoint(aRange.getMinX(), aRange.getCenterY()));
                        if (!bWdt0          ) new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_RIGHT, basegfx::B2DPoint(aRange.getMaxX(), aRange.getCenterY()));
                        if (!bWdt0 && !bHgt0) new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_LWLFT, basegfx::B2DPoint(aRange.getMinX(), aRange.getMaxY()));
                        if (          !bHgt0) new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_LOWER, basegfx::B2DPoint(aRange.getCenterX(), aRange.getMaxY()));
                        if (!bWdt0 && !bHgt0) new SmartHdl(rHandlerList, mpPathObj, xThis, HDL_LWRGT, aRange.getMaximum());
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
        mrView.SetMarkHandles();
        deleteSdrObjectSafeAndClearPointer(pPathObj);
    }

    maSelectedPoints.clear();

    SmartTag::disposing();
}

// --------------------------------------------------------------------

void MotionPathTag::deselect()
{
    SmartTag::deselect();
    maSelectedPoints.clear();
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
// --------------------------------------------------------------------
// IPolyPolygonEditorController
// --------------------------------------------------------------------

void MotionPathTag::DeleteMarkedPoints()
{
    if( mpPathObj && IsDeleteMarkedPointsPossible() )
    {
        mrView.BrkAction();

        if(maSelectedPoints.size())
            {
            basegfx::B2DPolyPolygon aPolyPolygon(mpPathObj->getB2DPolyPolygonInObjectCoordinates());

            if( sdr::PolyPolygonEditor::DeletePoints(aPolyPolygon , maSelectedPoints ) )
                {
                if( aPolyPolygon.count() )
                {
                    mpPathObj->setB2DPolyPolygonInObjectCoordinates( aPolyPolygon );
                }

                mrView.MarkPoints(0, true); // unmarkall
                mrView.SetMarkHandles();
            }
        }
    }
}

bool MotionPathTag::IsDeleteMarkedPointsPossible() const
{
    return mpPathObj && isSelected() && GetMarkedPointCount();
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
        if(maSelectedPoints.size())
        {
            basegfx::B2DPolyPolygon aPolyPolygon(mpPathObj->getB2DPolyPolygonInObjectCoordinates());

            if(sdr::PolyPolygonEditor::SetSegmentsKind( aPolyPolygon, eKind, maSelectedPoints ) )
            {
                mpPathObj->setB2DPolyPolygonInObjectCoordinates(aPolyPolygon);
                mrView.SetMarkHandles();
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

    if(mpPathObj && isSelected() && (GetMarkedPointCount() != 0))
    {
        if(maSelectedPoints.size())
        {
            basegfx::B2DPolyPolygon aPolyPolygon(mpPathObj->getB2DPolyPolygonInObjectCoordinates());

            if(sdr::PolyPolygonEditor::SetPointsSmooth( aPolyPolygon, eFlags, maSelectedPoints ) )
            {
                mpPathObj->setB2DPolyPolygonInObjectCoordinates(aPolyPolygon);
                mrView.SetMarkHandles();
            }
        }
    }
}

void MotionPathTag::CloseMarkedObjects(bool /*bToggle*/, bool /*bOpen*/ )
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
        mrView.SetMarkHandles();
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

