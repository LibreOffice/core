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

#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/office/XAnnotation.hpp>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svddrgmt.hxx>

#include <View.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include "annotationmanagerimpl.hxx"
#include "annotationwindow.hxx"
#include "annotationtag.hxx"
#include <ViewShell.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::geometry;

namespace sd
{

const sal_uInt32 SMART_TAG_HDL_NUM = SAL_MAX_UINT32;
static const int DRGPIX     = 2;                               // Drag MinMove in Pixel

static OUString getInitials( const OUString& rName )
{
    OUStringBuffer sInitials;

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
            sInitials.append(*pStr);
            nLength--; pStr++;
        }

        // skip letters until whitespace
        while( nLength && (*pStr > ' ') )
        {
            nLength--; pStr++;
        }
    }

    return sInitials.makeStringAndClear();
}

class AnnotationDragMove : public SdrDragMove
{
public:
    AnnotationDragMove(SdrDragView& rNewView, const rtl::Reference <AnnotationTag >& xTag);
    virtual bool BeginSdrDrag() override;
    virtual bool EndSdrDrag(bool bCopy) override;
    virtual void MoveSdrDrag(const Point& rNoSnapPnt) override;
    virtual void CancelSdrDrag() override;

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
    DragStat().SetRef1(GetDragHdl()->GetPos());
    DragStat().SetShown(!DragStat().IsShown());

    maOrigin = GetDragHdl()->GetPos();
    DragStat().SetActionRect(::tools::Rectangle(maOrigin,maOrigin));

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
            DragStat().SetActionRect(::tools::Rectangle(aPnt,aPnt));
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

class AnnotationHdl : public SmartHdl
{
public:
    AnnotationHdl( const SmartTagReference& xTag, const Reference< XAnnotation >& xAnnotation, const Point& rPnt );

    virtual void CreateB2dIAObject() override;
    virtual bool IsFocusHdl() const override;
    virtual bool isMarkable() const override;

private:
    Reference< XAnnotation > mxAnnotation;
    rtl::Reference< AnnotationTag > mxTag;
};

AnnotationHdl::AnnotationHdl( const SmartTagReference& xTag, const Reference< XAnnotation >& xAnnotation, const Point& rPnt )
: SmartHdl( xTag, rPnt, SdrHdlKind::SmartTag )
, mxAnnotation( xAnnotation )
, mxTag( dynamic_cast< AnnotationTag* >( xTag.get() ) )
{
}

void AnnotationHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if( !mxAnnotation.is() )
        return;

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    const Point aTagPos( GetPos() );
    basegfx::B2DPoint aPosition( aTagPos.X(), aTagPos.Y() );

    const bool bFocused = IsFocusHdl() && pHdlList && (pHdlList->GetFocusHdl() == this);

    BitmapEx aBitmapEx( mxTag->CreateAnnotationBitmap(mxTag->isSelected()) );
    BitmapEx aBitmapEx2;
    if( bFocused )
        aBitmapEx2 = mxTag->CreateAnnotationBitmap(!mxTag->isSelected() );

    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!(pView && !pView->areMarkHandlesHidden()))
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b = 0; b < pPageView->PageWindowCount(); b++)
    {
        // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        SdrPaintWindow& rPaintWindow = rPageWindow.GetPaintWindow();
        const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
        if(rPaintWindow.OutputToWindow() && xManager.is() )
        {
            std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject;

            // animate focused handles
            if(bFocused)
            {
                const sal_uInt64 nBlinkTime = rStyleSettings.GetCursorBlinkTime();

                pOverlayObject.reset(new sdr::overlay::OverlayAnimatedBitmapEx(aPosition, aBitmapEx, aBitmapEx2, nBlinkTime, 0, 0, 0, 0 ));
            }
            else
            {
                pOverlayObject.reset(new sdr::overlay::OverlayBitmapEx( aPosition, aBitmapEx, 0, 0 ));
            }

            // OVERLAYMANAGER
            insertNewlyCreatedOverlayObjectForSdrHdl(
                std::move(pOverlayObject),
                rPageWindow.GetObjectContact(),
                *xManager);
        }
    }
}

bool AnnotationHdl::IsFocusHdl() const
{
    return true;
}

bool AnnotationHdl::isMarkable() const
{
    return false;
}

AnnotationTag::AnnotationTag( AnnotationManagerImpl& rManager, ::sd::View& rView, const Reference< XAnnotation >& xAnnotation, Color const & rColor, int nIndex, const vcl::Font& rFont )
: SmartTag( rView )
, mrManager( rManager )
, mxAnnotation( xAnnotation )
, maColor( rColor )
, mnIndex( nIndex )
, mrFont( rFont )
, mnClosePopupEvent( nullptr )
, mpListenWindow( nullptr )
{
}

AnnotationTag::~AnnotationTag()
{
    DBG_ASSERT( !mxAnnotation.is(), "sd::AnnotationTag::~AnnotationTag(), dispose me first!" );
    Dispose();
}

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

    if( rMEvt.IsLeft() && !rMEvt.IsRight() )
    {
        vcl::Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
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

   return false;
}

/** returns true if the SmartTag consumes this event. */
bool AnnotationTag::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        vcl::Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
        if( pWindow )
        {
            ::tools::Rectangle aContextRect(rCEvt.GetMousePosPixel(),Size(1,1));
            mrManager.ExecuteAnnotationContextMenu( mxAnnotation, pWindow, aContextRect );
            return true;
        }
    }

    return false;
}

void AnnotationTag::Move( int nDX, int nDY )
{
    if( !mxAnnotation.is() )
        return;

    if( mrManager.GetDoc()->IsUndoEnabled() )
        mrManager.GetDoc()->BegUndo( SdResId( STR_ANNOTATION_UNDO_MOVE ) );

    RealPoint2D aPosition( mxAnnotation->getPosition() );
    aPosition.X += static_cast<double>(nDX) / 100.0;
    aPosition.Y += static_cast<double>(nDY) / 100.0;
    mxAnnotation->setPosition( aPosition );

    if( mrManager.GetDoc()->IsUndoEnabled() )
        mrManager.GetDoc()->EndUndo();

    mrView.updateHandles();
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
        // move the annotation
        Move( nX, nY );
    }

    return true;
}

void AnnotationTag::CheckPossibilities()
{
}

sal_Int32 AnnotationTag::GetMarkablePointCount() const
{
    return 0;
}

sal_Int32 AnnotationTag::GetMarkedPointCount() const
{
    return 0;
}

bool AnnotationTag::MarkPoint(SdrHdl& /*rHdl*/, bool /*bUnmark*/ )
{
    return false;
}

bool AnnotationTag::MarkPoints(const ::tools::Rectangle* /*pRect*/, bool /*bUnmark*/ )
{
    return false;
}

bool AnnotationTag::getContext( SdrViewContext& /*rContext*/ )
{
    return false;
}

void AnnotationTag::addCustomHandles( SdrHdlList& rHandlerList )
{
    if( !mxAnnotation.is() )
        return;

    SmartTagReference xThis( this );
    std::unique_ptr<AnnotationHdl> pHdl(new AnnotationHdl( xThis, mxAnnotation, Point() ));
    pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
    pHdl->SetPageView( mrView.GetSdrPageView() );

    RealPoint2D aPosition( mxAnnotation->getPosition() );
    Point aBasePos( static_cast<long>(aPosition.X * 100.0), static_cast<long>(aPosition.Y * 100.0) );
    pHdl->SetPos( aBasePos );

    rHandlerList.AddHdl( std::move(pHdl) );
}

void AnnotationTag::disposing()
{
    if( mpListenWindow )
    {
        mpListenWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));
    }

    if( mnClosePopupEvent )
    {
        Application::RemoveUserEvent( mnClosePopupEvent );
        mnClosePopupEvent = nullptr;
    }

    mxAnnotation.clear();
    ClosePopup();
    SmartTag::disposing();
}

void AnnotationTag::select()
{
    SmartTag::select();

    mrManager.onTagSelected( *this );

    vcl::Window* pWindow = mrView.GetViewShell()->GetActiveWindow();
    if( pWindow )
    {
        RealPoint2D aPosition( mxAnnotation->getPosition() );
        Point aPos( static_cast<long>(aPosition.X * 100.0), static_cast<long>(aPosition.Y * 100.0) );

        ::tools::Rectangle aVisRect( aPos, pWindow->PixelToLogic(maSize) );
        mrView.MakeVisible(aVisRect, *pWindow);
    }
}

void AnnotationTag::deselect()
{
    SmartTag::deselect();

    ClosePopup();

    mrManager.onTagDeselected( *this );
}

BitmapEx AnnotationTag::CreateAnnotationBitmap( bool bSelected )
{
    ScopedVclPtrInstance< VirtualDevice > pVDev;

    OUString sInitials(mxAnnotation->getInitials());
    if (sInitials.isEmpty())
        sInitials = getInitials(mxAnnotation->getAuthor());

    OUString sAuthor(sInitials + " " + OUString::number(mnIndex));

    pVDev->SetFont( mrFont );

    const int BORDER_X = 4; // pixels
    const int BORDER_Y = 4; // pixels

    maSize = Size( pVDev->GetTextWidth( sAuthor ) + 2*BORDER_X, pVDev->GetTextHeight() + 2*BORDER_Y );
    pVDev->SetOutputSizePixel( maSize, false );

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
    ::tools::Rectangle aBorderRect( aPos, maSize );
    pVDev->SetLineColor(aBorderColor);
    pVDev->SetFillColor(maColor);
    pVDev->DrawRect( aBorderRect );

    pVDev->SetTextColor( maColor.IsDark() ? COL_WHITE : COL_BLACK );
    pVDev->DrawText( Point( BORDER_X, BORDER_Y ), sAuthor );

    return pVDev->GetBitmapEx( aPos, maSize );
}

void AnnotationTag::OpenPopup( bool bEdit )
{
    if( !mxAnnotation.is() )
        return;

    if( !mpAnnotationWindow.get() )
    {
        vcl::Window* pWindow = dynamic_cast< vcl::Window* >( getView().GetFirstOutputDevice() );
        if( pWindow )
        {
            RealPoint2D aPosition( mxAnnotation->getPosition() );
            Point aPos( pWindow->OutputToScreenPixel( pWindow->LogicToPixel( Point( static_cast<long>(aPosition.X * 100.0), static_cast<long>(aPosition.Y * 100.0) ) ) ) );

            aPos.AdjustX(4 ); // magic!
            aPos.AdjustY(1 );

            ::tools::Rectangle aRect( aPos, maSize );

            mpAnnotationWindow.reset( VclPtr<AnnotationWindow>::Create( mrManager, mrView.GetDocSh(), pWindow->GetWindow(GetWindowType::Frame) ) );
            mpAnnotationWindow->InitControls();
            mpAnnotationWindow->setAnnotation(mxAnnotation);

            sal_uInt16 nArrangeIndex = 0;
            Point aPopupPos( FloatingWindow::CalcFloatingPosition( mpAnnotationWindow.get(), aRect, FloatWinPopupFlags::Right, nArrangeIndex ) );
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
    if( mpAnnotationWindow.get())
    {
        mpAnnotationWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));
        mpAnnotationWindow->Deactivate();
        mpAnnotationWindow.disposeAndClear();
    }
}

IMPL_LINK(AnnotationTag, WindowEventHandler, VclWindowEvent&, rEvent, void)
{
        vcl::Window* pWindow = rEvent.GetWindow();

        if( !pWindow )
            return;

        if( pWindow == mpAnnotationWindow.get() )
        {
            if( rEvent.GetId() == VclEventId::WindowDeactivate )
            {
                // tdf#99388 and tdf#99712 if PopupMenu is active, suppress
                // deletion of the AnnotationWindow which is triggered by
                // it losing focus
                if (!mrManager.getPopupMenuActive())
                {
                    if( mnClosePopupEvent )
                        Application::RemoveUserEvent( mnClosePopupEvent );

                    mnClosePopupEvent = Application::PostUserEvent( LINK( this, AnnotationTag, ClosePopupHdl ) );
                }
            }
        }
        else if( pWindow == mpListenWindow )
        {
            switch( rEvent.GetId() )
            {
            case VclEventId::WindowMouseButtonUp:
                {
                    // if we stop pressing the button without a mouse move we open the popup
                    mpListenWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));
                    mpListenWindow = nullptr;
                    if( mpAnnotationWindow.get() == nullptr )
                        OpenPopup(false);
                }
                break;
            case VclEventId::WindowMouseMove:
                {
                    // if we move the mouse after a button down we want to start dragging
                    mpListenWindow->RemoveEventListener( LINK(this, AnnotationTag, WindowEventHandler));
                    mpListenWindow = nullptr;

                    SdrHdl* pHdl = mrView.PickHandle(maMouseDownPos);
                    if( pHdl )
                    {
                        mrView.BrkAction();
                        const sal_uInt16 nDrgLog = static_cast<sal_uInt16>(pWindow->PixelToLogic(Size(DRGPIX,0)).Width());

                        rtl::Reference< AnnotationTag > xTag( this );

                        SdrDragMethod* pDragMethod = new AnnotationDragMove( mrView, xTag );
                        mrView.BegDragObj(maMouseDownPos, nullptr, pHdl, nDrgLog, pDragMethod );
                    }
                }
                break;
            case VclEventId::ObjectDying:
                mpListenWindow = nullptr;
                break;
            default: break;
            }
        }
}

IMPL_LINK_NOARG(AnnotationTag, ClosePopupHdl, void*, void)
{
    mnClosePopupEvent = nullptr;
    ClosePopup();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
