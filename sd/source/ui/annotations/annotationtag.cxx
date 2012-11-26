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

#include <vcl/help.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svddrgmt.hxx>

#include "View.hxx"
#include "sdresid.hxx"
#include "annotations.hrc"
#include "annotationmanagerimpl.hxx"
#include "annotationwindow.hxx"
#include "annotationtag.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"
#include "app.hrc"
#include "Window.hxx"
#include "drawdoc.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
//using namespace ::com::sun::star::util;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::geometry;

namespace sd
{

const sal_uInt32 SMART_TAG_HDL_NUM = SAL_MAX_UINT32;
static const int DRGPIX     = 2;                               // Drag MinMove in Pixel

// --------------------------------------------------------------------

static OUString getInitials( const OUString& rName )
{
    OUString sInitials;

    const sal_Unicode * pStr = rName.getStr();
    sal_Int32 nLength = rName.getLength();

    while( nLength )
    {
        // skip whitespace
        while( nLength && (*pStr <= ' ') )
        {
            nLength--; pStr++;
        }

        // take letter
        if( nLength )
        {
            sInitials += OUString( *pStr );
            nLength--; pStr++;
        }

        // skip letters until whitespace
        while( nLength && (*pStr > ' ') )
        {
            nLength--; pStr++;
        }
    }

    return sInitials;
}

// --------------------------------------------------------------------

// --------------------------------------------------------------------

class AnnotationDragMove : public SdrDragMove
{
public:
    AnnotationDragMove(SdrView& rNewView, const rtl::Reference <AnnotationTag >& xTag);
    virtual bool BeginSdrDrag();
    virtual bool EndSdrDrag(bool bCopy);
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rNoSnapPnt);
    virtual void CancelSdrDrag();

private:
    rtl::Reference <AnnotationTag > mxTag;
    basegfx::B2DPoint maOrigin;
};

AnnotationDragMove::AnnotationDragMove(SdrView& rNewView, const rtl::Reference <AnnotationTag >& xTag)
: SdrDragMove(rNewView)
, mxTag( xTag )
{
}

bool AnnotationDragMove::BeginSdrDrag()
{
    DragStat().SetRef1(GetDragHdl()->getPosition());
    DragStat().SetShown(!DragStat().IsShown());

    maOrigin = GetDragHdl()->getPosition();
    DragStat().SetActionRange(basegfx::B2DRange(maOrigin));

    return true;
}

void AnnotationDragMove::MoveSdrDrag(const basegfx::B2DPoint& rNoSnapPnt)
{
    if (DragStat().CheckMinMoved(rNoSnapPnt))
    {
        basegfx::B2DPoint aPnt(rNoSnapPnt);

        if(!aPnt.equal(DragStat().GetNow()))
        {
            Hide();
            DragStat().NextMove(aPnt);
            GetDragHdl()->setPosition(maOrigin + (DragStat().GetNow() - DragStat().GetPrev()));
            Show();
            DragStat().SetActionRange(basegfx::B2DRange(aPnt));
        }
    }
}

bool AnnotationDragMove::EndSdrDrag(bool /*bCopy*/)
{
    Hide();
    if( mxTag.is() )
        mxTag->Move( DragStat().GetDX(), DragStat().GetDY() );
    return true;
}

void AnnotationDragMove::CancelSdrDrag()
{
    Hide();
}

// --------------------------------------------------------------------

class AnnotationHdl : public SmartHdl
{
public:
    AnnotationHdl(
        SdrHdlList& rHdlList,
        const SmartTagReference& xTag,
        const Reference< XAnnotation >& xAnnotation,
        const basegfx::B2DPoint& rPnt );
    virtual bool IsFocusHdl() const;
    virtual Pointer GetSdrDragPointer() const;
    virtual bool isMarkable() const;

protected:
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);
    virtual ~AnnotationHdl();

private:
    Reference< XAnnotation > mxAnnotation;
    rtl::Reference< AnnotationTag > mxTag;
};

// --------------------------------------------------------------------

AnnotationHdl::AnnotationHdl(
    SdrHdlList& rHdlList,
    const SmartTagReference& xTag,
    const Reference< XAnnotation >& xAnnotation,
    const basegfx::B2DPoint& rPnt )
: SmartHdl( rHdlList, 0, xTag, HDL_SMARTTAG, rPnt )
, mxAnnotation( xAnnotation )
, mxTag( dynamic_cast< AnnotationTag* >( xTag.get() ) )
{
}

// --------------------------------------------------------------------

AnnotationHdl::~AnnotationHdl()
{
}

// --------------------------------------------------------------------

void AnnotationHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    if( mxAnnotation.is() )
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const bool bFocused = IsFocusHdl() && (mrHdlList.GetFocusHdl() == this);

        BitmapEx aBitmapEx( mxTag->CreateAnnotationBitmap(mxTag->isSelected()) );
        BitmapEx aBitmapEx2;
        if( bFocused )
            aBitmapEx2 = mxTag->CreateAnnotationBitmap(!mxTag->isSelected() );

        ::sdr::overlay::OverlayObject* pOverlayObject = 0;

        // animate focused handles
        if(bFocused)
        {
            const sal_uInt32 nBlinkTime = sal::static_int_cast<sal_uInt32>(rStyleSettings.GetCursorBlinkTime());

            pOverlayObject = new ::sdr::overlay::OverlayAnimatedBitmapEx(getPosition(), aBitmapEx, aBitmapEx2, nBlinkTime, 0, 0, 0, 0 );
        }
        else
        {
            pOverlayObject = new ::sdr::overlay::OverlayBitmapEx( getPosition(), aBitmapEx, 0, 0 );
        }

        rOverlayManager.add(*pOverlayObject);
        maOverlayGroup.append(*pOverlayObject);
    }
}

// --------------------------------------------------------------------

bool AnnotationHdl::IsFocusHdl() const
{
    return true;
}

// --------------------------------------------------------------------

bool AnnotationHdl::isMarkable() const
{
    return false;
}

// --------------------------------------------------------------------

Pointer AnnotationHdl::GetSdrDragPointer() const
{
    PointerStyle eStyle = POINTER_NOTALLOWED;
    if( mxTag.is() )
    {
        if( mxTag->isSelected() )
        {
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

AnnotationTag::AnnotationTag( AnnotationManagerImpl& rManager, ::sd::View& rView, const Reference< XAnnotation >& xAnnotation, Color& rColor, int nIndex, const Font& rFont )
: SmartTag( rView )
, mrManager( rManager )
, mxAnnotation( xAnnotation )
, maColor( rColor )
, mnIndex( nIndex )
, mrFont( rFont )
, mnClosePopupEvent( 0 )
, mpListenWindow( 0 )
{
}

// --------------------------------------------------------------------

AnnotationTag::~AnnotationTag()
{
    DBG_ASSERT( !mxAnnotation.is(), "sd::AnnotationTag::~AnnotationTag(), dispose me first!" );
    Dispose();
}

// --------------------------------------------------------------------

/** returns true if the AnnotationTag handled the event. */
bool AnnotationTag::MouseButtonDown( const MouseEvent& rMEvt, SmartHdl& /*rHdl*/ )
{
    if( !mxAnnotation.is() )
        return false;

    bool bRet = false;
    if( !isSelected() )
    {
        SmartTagReference xTag( this );
        mrView.getSmartTags().select( xTag );
        bRet = true;
    }
    /*
    if( rMEvt.IsLeft() && (rMEvt.GetClicks() == 2) )
    {
        // double click;
        return true;
    }
    else */
    if( rMEvt.IsLeft() && !rMEvt.IsRight() )
    {
        Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
        if( pWindow )
        {
            maMouseDownPos = pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());

            if( mpListenWindow )
                mpListenWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));

            mpListenWindow = pWindow;
            mpListenWindow->AddEventListener( LINK(this, AnnotationTag, WindowEventHandler));
        }

        bRet = true;
    }

    return bRet;
}

// --------------------------------------------------------------------

/** returns true if the SmartTag consumes this event. */
bool AnnotationTag::KeyInput( const KeyEvent& rKEvt )
{
    if( !mxAnnotation.is() )
        return false;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    switch( nCode )
    {
    case KEY_DELETE:
        mrManager.DeleteAnnotation( mxAnnotation );
        return true;

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
        mrManager.SelectNextAnnotation(!rKEvt.GetKeyCode().IsShift());
        return true;

    case KEY_RETURN:
       case KEY_SPACE:
           OpenPopup( true );
           return true;

    default:
        return false;
    }
}

/** returns true if the SmartTag consumes this event. */
bool AnnotationTag::RequestHelp( const HelpEvent& /*rHEvt*/ )
{
/*
    ::Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
    if( mxAnnotation.is() && pWindow )
    {
        OUString aHelpText( mrManager.GetHelpText( mxAnnotation ) );

        RealPoint2D aPosition( mxAnnotation->getPosition() );
        Point aPos( pWindow->LogicToPixel( Point( static_cast<long>(aPosition.X * 100.0), static_cast<long>(aPosition.Y * 100.0) ) ) );

        Rectangle aRect( aPos, maSize );

        if (Help::IsBalloonHelpEnabled())
            Help::ShowBalloon( pWindow, aPos, aRect, aHelpText);
        else if (Help::IsQuickHelpEnabled())
            Help::ShowQuickHelp( pWindow, aRect, aHelpText);

        return true;
   }
*/
   return false;
}

/** returns true if the SmartTag consumes this event. */
bool AnnotationTag::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        ::Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
        if( pWindow )
        {
            Rectangle aContextRect(rCEvt.GetMousePosPixel(),Size(1,1));
               mrManager.ExecuteAnnotationContextMenu( mxAnnotation, pWindow, aContextRect );
            return true;
        }
    }

    return false;
}

void AnnotationTag::Move( int nDX, int nDY )
{
    if( mxAnnotation.is() )
    {
        if( mrManager.GetDoc()->IsUndoEnabled() )
            mrManager.GetDoc()->BegUndo( String( SdResId( STR_ANNOTATION_UNDO_MOVE ) ) );

        RealPoint2D aPosition( mxAnnotation->getPosition() );
        aPosition.X += (double)nDX / 100.0;
        aPosition.Y += (double)nDY / 100.0;
        mxAnnotation->setPosition( aPosition );

        if( mrManager.GetDoc()->IsUndoEnabled() )
            mrManager.GetDoc()->EndUndo();

        mrView.SetMarkHandles();
    }
}

bool AnnotationTag::OnMove( const KeyEvent& rKEvt )
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
        // move the annotation
        Move( nX, nY );
    }

    return true;
}

// --------------------------------------------------------------------

void AnnotationTag::CheckPossibilities()
{
}

// --------------------------------------------------------------------

sal_uInt32 AnnotationTag::GetMarkablePointCount() const
{
    return 0;
}

// --------------------------------------------------------------------

sal_uInt32 AnnotationTag::GetMarkedPointCount() const
{
    return 0;
}

// --------------------------------------------------------------------

bool AnnotationTag::MarkPoint(SdrHdl& /*rHdl*/, bool /*bUnmark*/ )
{
    return false;
}

// --------------------------------------------------------------------

bool AnnotationTag::MarkPoints(const basegfx::B2DRange* /*pRange*/, bool /*bUnmark*/ )
{
    return false;
}

// --------------------------------------------------------------------

bool AnnotationTag::getContext( SdrViewContext& /*rContext*/ )
{
    return false;
}

// --------------------------------------------------------------------

void AnnotationTag::addCustomHandles( SdrHdlList& rHandlerList )
{
    if( mxAnnotation.is() )
    {
        SmartTagReference xThis( this );
        const RealPoint2D aPosition( mxAnnotation->getPosition() );
        const basegfx::B2DPoint aB2DPosition(aPosition.X * 100.0, aPosition.Y * 100.0);
        AnnotationHdl* pHdl = new AnnotationHdl(rHandlerList, xThis, mxAnnotation, aB2DPosition);

        pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
    }
}

// --------------------------------------------------------------------

void AnnotationTag::disposing()
{
    if( mpListenWindow )
    {
        mpListenWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));
    }

    if( mnClosePopupEvent )
    {
        Application::RemoveUserEvent( mnClosePopupEvent );
        mnClosePopupEvent = 0;
    }

    mxAnnotation.clear();
    ClosePopup();
    SmartTag::disposing();
}

// --------------------------------------------------------------------

void AnnotationTag::select()
{
    SmartTag::select();

    mrManager.onTagSelected( *this );

    Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
    if( pWindow )
    {
        const RealPoint2D aPosition( mxAnnotation->getPosition() );
        const basegfx::B2DPoint aB2DPosition(aPosition.X * 100.0, aPosition.Y * 100.0);
        const basegfx::B2DRange aRange(
            aB2DPosition,
            aB2DPosition + (pWindow->GetInverseViewTransformation() * basegfx::B2DPoint(maSize.getWidth(), maSize.getHeight())));

        mrView.MakeVisibleAtView(aRange, *pWindow);
    }
}

// --------------------------------------------------------------------

void AnnotationTag::deselect()
{
    SmartTag::deselect();

    ClosePopup();

    mrManager.onTagDeselected( *this );
}

// --------------------------------------------------------------------

BitmapEx AnnotationTag::CreateAnnotationBitmap( bool bSelected )
{
    VirtualDevice aVDev;

    OUString sAuthor( getInitials( mxAnnotation->getAuthor() ) );
    sAuthor += OUString( sal_Unicode( ' ' ) );
    sAuthor += OUString::valueOf( (sal_Int32)mnIndex );

    aVDev.SetFont( mrFont );

    const int BORDER_X = 4; // pixels
    const int BORDER_Y = 4; // pixels

    maSize = Size( aVDev.GetTextWidth( sAuthor ) + 2*BORDER_X, aVDev.GetTextHeight() + 2*BORDER_Y );
    aVDev.SetOutputSizePixel( maSize, false );

    Color aBorderColor( maColor );

    if( bSelected )
    {
        aBorderColor.Invert();
    }
    else
    {
        if( maColor.IsDark() )
        {
            aBorderColor.IncreaseLuminance( 32 );
        }
        else
        {
            aBorderColor.DecreaseLuminance( 32 );
        }
    }

    Point aPos;
    Rectangle aBorderRect( aPos, maSize );
    aVDev.SetLineColor(aBorderColor);
    aVDev.SetFillColor(maColor);
    aVDev.DrawRect( aBorderRect );

    aVDev.SetTextColor( maColor.IsDark() ? COL_WHITE : COL_BLACK );
    aVDev.DrawText( Point( BORDER_X, BORDER_Y ), sAuthor );

    return aVDev.GetBitmapEx( aPos, maSize );
}

void AnnotationTag::OpenPopup( bool bEdit )
{
    if( !mxAnnotation.is() )
        return;

    if( !mpAnnotationWindow.get() )
    {
           ::Window* pWindow = dynamic_cast< ::Window* >( getView().GetFirstOutputDevice() );
           if( pWindow )
           {
            RealPoint2D aPosition( mxAnnotation->getPosition() );
            Point aPos( pWindow->OutputToScreenPixel( pWindow->LogicToPixel( Point( static_cast<long>(aPosition.X * 100.0), static_cast<long>(aPosition.Y * 100.0) ) ) ) );

            aPos.X() += 4; // magic!
            aPos.Y() += 1;

            Rectangle aRect( aPos, maSize );

            mpAnnotationWindow.reset( new AnnotationWindow( mrManager, mrView.GetDocSh(), pWindow->GetWindow(WINDOW_FRAME) ) );
            mpAnnotationWindow->InitControls();
            mpAnnotationWindow->setAnnotation(mxAnnotation);

            sal_uInt16 nArrangeIndex = 0;
            Point aPopupPos( FloatingWindow::CalcFloatingPosition( mpAnnotationWindow.get(), aRect, FLOATWIN_POPUPMODE_RIGHT, nArrangeIndex ) );
            Size aPopupSize( 320, 240 );

            mpAnnotationWindow->SetPosSizePixel( aPopupPos, aPopupSize );
            mpAnnotationWindow->DoResize();

            mpAnnotationWindow->Show();
            mpAnnotationWindow->GrabFocus();
            mpAnnotationWindow->AddEventListener( LINK(this, AnnotationTag, WindowEventHandler));
        }
    }

    if( bEdit && mpAnnotationWindow.get() )
        mpAnnotationWindow->StartEdit();
}

void AnnotationTag::ClosePopup()
{
    if( mpAnnotationWindow.get() )
    {
        mpAnnotationWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));
        mpAnnotationWindow->Deactivate();
        mpAnnotationWindow.reset();
    }
}

IMPL_LINK(AnnotationTag, WindowEventHandler, VclWindowEvent*, pEvent)
{
    if( pEvent != NULL )
    {
        ::Window* pWindow = pEvent->GetWindow();

        if( pWindow )
        {
            if( pWindow == mpAnnotationWindow.get() )
            {
                if( pEvent->GetId() == VCLEVENT_WINDOW_DEACTIVATE )
                {
                    if( mnClosePopupEvent )
                        Application::RemoveUserEvent( mnClosePopupEvent );

                    mnClosePopupEvent = Application::PostUserEvent( LINK( this, AnnotationTag, ClosePopupHdl ) );
                }
            }
            else if( pWindow == mpListenWindow )
            {
                switch( pEvent->GetId() )
                {
                case VCLEVENT_WINDOW_MOUSEBUTTONUP:
                    {
                        // if we stop pressing the button without a mouse move we open the popup
                        mpListenWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));
                        mpListenWindow = 0;
                        if( mpAnnotationWindow.get() == 0 )
                            OpenPopup(false);
                    }
                    break;
                case VCLEVENT_WINDOW_MOUSEMOVE:
                    {
                        // if we move the mouse after a button down we wan't to start draging
                        mpListenWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));
                        mpListenWindow = 0;

                        SdrHdl* pHdl = mrView.PickHandle(maMouseDownPos);
                        if( pHdl )
                        {
                            mrView.BrkAction();
                            rtl::Reference< AnnotationTag > xTag( this );

                            SdrDragMethod* pDragMethod = new AnnotationDragMove( mrView, xTag );
                            const double fTolerance(basegfx::B2DVector(pWindow->GetInverseViewTransformation() * basegfx::B2DVector(DRGPIX, 0.0)).getLength());
                            mrView.BegDragObj(maMouseDownPos, pHdl, fTolerance, pDragMethod );
                        }
                    }
                    break;
                case VCLEVENT_OBJECT_DYING:
                    mpListenWindow = 0;
                    break;
                }
            }
        }
    }
    return true;
}

IMPL_LINK( AnnotationTag, ClosePopupHdl, void *, EMPTYARG )
{
    mnClosePopupEvent = 0;
    ClosePopup();
    return 0;
}

} // end of namespace sd

