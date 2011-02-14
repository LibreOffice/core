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
    AnnotationDragMove(SdrDragView& rNewView, const rtl::Reference <AnnotationTag >& xTag);
    virtual bool BeginSdrDrag();
    virtual bool EndSdrDrag(bool bCopy);
    virtual void MoveSdrDrag(const Point& rNoSnapPnt);
    virtual void CancelSdrDrag();

private:
    rtl::Reference <AnnotationTag > mxTag;
    Point maOrigin;
};

AnnotationDragMove::AnnotationDragMove(SdrDragView& rNewView, const rtl::Reference <AnnotationTag >& xTag)
: SdrDragMove(rNewView)
, mxTag( xTag )
{
}

bool AnnotationDragMove::BeginSdrDrag()
{
    DragStat().Ref1()=GetDragHdl()->GetPos();
    DragStat().SetShown(!DragStat().IsShown());

    maOrigin = GetDragHdl()->GetPos();
    DragStat().SetActionRect(Rectangle(maOrigin,maOrigin));

    return true;
}

void AnnotationDragMove::MoveSdrDrag(const Point& rNoSnapPnt)
{
    Point aPnt(rNoSnapPnt);

    if (DragStat().CheckMinMoved(rNoSnapPnt))
    {
        if (aPnt!=DragStat().GetNow())
        {
            Hide();
            DragStat().NextMove(aPnt);
            GetDragHdl()->SetPos( maOrigin + Point( DragStat().GetDX(), DragStat().GetDY() ) );
            Show();
            DragStat().SetActionRect(Rectangle(aPnt,aPnt));
        }
    }
}

bool AnnotationDragMove::EndSdrDrag(bool /*bCopy*/)
{
    Hide();
    if( mxTag.is() )
        mxTag->Move( DragStat().GetDX(), DragStat().GetDY() );
    return sal_True;
}

void AnnotationDragMove::CancelSdrDrag()
{
    Hide();
}

// --------------------------------------------------------------------

class AnnotationHdl : public SmartHdl
{
public:
    AnnotationHdl( const SmartTagReference& xTag, const Reference< XAnnotation >& xAnnotation, const Point& rPnt );
    virtual ~AnnotationHdl();
    virtual void CreateB2dIAObject();
    virtual sal_Bool IsFocusHdl() const;
    virtual Pointer GetSdrDragPointer() const;
    virtual bool isMarkable() const;


private:
    Reference< XAnnotation > mxAnnotation;
    rtl::Reference< AnnotationTag > mxTag;
};

// --------------------------------------------------------------------

AnnotationHdl::AnnotationHdl( const SmartTagReference& xTag, const Reference< XAnnotation >& xAnnotation, const Point& rPnt )
: SmartHdl( xTag, rPnt )
, mxAnnotation( xAnnotation )
, mxTag( dynamic_cast< AnnotationTag* >( xTag.get() ) )
{
}

// --------------------------------------------------------------------

AnnotationHdl::~AnnotationHdl()
{
}

// --------------------------------------------------------------------

void AnnotationHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if( mxAnnotation.is() )
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        const Point aTagPos( GetPos() );
        basegfx::B2DPoint aPosition( aTagPos.X(), aTagPos.Y() );

        const bool bFocused = IsFocusHdl() && pHdlList && (pHdlList->GetFocusHdl() == this);

        BitmapEx aBitmapEx( mxTag->CreateAnnotationBitmap(mxTag->isSelected()) );
        BitmapEx aBitmapEx2;
        if( bFocused )
            aBitmapEx2 = mxTag->CreateAnnotationBitmap(!mxTag->isSelected() );

        if(pHdlList)
        {
            SdrMarkView* pView = pHdlList->GetView();

            if(pView && !pView->areMarkHandlesHidden())
            {
                SdrPageView* pPageView = pView->GetSdrPageView();

                if(pPageView)
                {
                    for(sal_uInt32 b = 0; b < pPageView->PageWindowCount(); b++)
                    {
                        // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
                        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                        SdrPaintWindow& rPaintWindow = rPageWindow.GetPaintWindow();
                        if(rPaintWindow.OutputToWindow() && rPageWindow.GetOverlayManager() )
                        {
                            ::sdr::overlay::OverlayObject* pOverlayObject = 0;

                            // animate focused handles
                            if(bFocused)
                            {
                                const sal_uInt32 nBlinkTime = sal::static_int_cast<sal_uInt32>(rStyleSettings.GetCursorBlinkTime());

                                pOverlayObject = new ::sdr::overlay::OverlayAnimatedBitmapEx(aPosition, aBitmapEx, aBitmapEx2, nBlinkTime, 0, 0, 0, 0 );
/*
                                    (sal_uInt16)(aBitmapEx.GetSizePixel().Width() - 1) >> 1,
                                    (sal_uInt16)(aBitmapEx.GetSizePixel().Height() - 1) >> 1,
                                    (sal_uInt16)(aBitmapEx2.GetSizePixel().Width() - 1) >> 1,
                                    (sal_uInt16)(aBitmapEx2.GetSizePixel().Height() - 1) >> 1);
*/
                            }
                            else
                            {
                                pOverlayObject = new ::sdr::overlay::OverlayBitmapEx( aPosition, aBitmapEx, 0, 0 );
                            }

                            rPageWindow.GetOverlayManager()->add(*pOverlayObject);
                            maOverlayGroup.append(*pOverlayObject);
                        }
                    }
                }
            }
        }
    }
}

// --------------------------------------------------------------------

sal_Bool AnnotationHdl::IsFocusHdl() const
{
    return sal_True;
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
            maMouseDownPos = pWindow->PixelToLogic( rMEvt.GetPosPixel() );

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

        mrView.updateHandles();
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

sal_uLong AnnotationTag::GetMarkablePointCount() const
{
    return 0;
}

// --------------------------------------------------------------------

sal_uLong AnnotationTag::GetMarkedPointCount() const
{
    return 0;
}

// --------------------------------------------------------------------

sal_Bool AnnotationTag::MarkPoint(SdrHdl& /*rHdl*/, sal_Bool /*bUnmark*/ )
{
    sal_Bool bRet=sal_False;
    return bRet;
}

// --------------------------------------------------------------------

sal_Bool AnnotationTag::MarkPoints(const Rectangle* /*pRect*/, sal_Bool /*bUnmark*/ )
{
    sal_Bool bChgd=sal_False;
    return bChgd;
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
        Point aPoint;
        AnnotationHdl* pHdl = new AnnotationHdl( xThis, mxAnnotation, aPoint );
        pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
        pHdl->SetPageView( mrView.GetSdrPageView() );

        RealPoint2D aPosition( mxAnnotation->getPosition() );
        Point aBasePos( static_cast<long>(aPosition.X * 100.0), static_cast<long>(aPosition.Y * 100.0) );
        pHdl->SetPos( aBasePos );

        rHandlerList.AddHdl( pHdl );
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
        RealPoint2D aPosition( mxAnnotation->getPosition() );
        Point aPos( static_cast<long>(aPosition.X * 100.0), static_cast<long>(aPosition.Y * 100.0) );

        Rectangle aVisRect( aPos, pWindow->PixelToLogic(maSize) );
        mrView.MakeVisible(aVisRect, *pWindow);
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
    aVDev.SetOutputSizePixel( maSize, sal_False );

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
                            const sal_uInt16 nDrgLog = (sal_uInt16)pWindow->PixelToLogic(Size(DRGPIX,0)).Width();

                            rtl::Reference< AnnotationTag > xTag( this );

                            SdrDragMethod* pDragMethod = new AnnotationDragMove( mrView, xTag );
                            mrView.BegDragObj(maMouseDownPos, NULL, pHdl, nDrgLog, pDragMethod );
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
    return sal_True;
}

IMPL_LINK( AnnotationTag, ClosePopupHdl, void *, EMPTYARG )
{
    mnClosePopupEvent = 0;
    ClosePopup();
    return 0;
}

} // end of namespace sd

