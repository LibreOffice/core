/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartController_Window.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:58:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ChartController.hxx"
#include "PositionAndSizeHelper.hxx"
#include "ObjectIdentifier.hxx"
#include "ChartWindow.hxx"
#include "ResId.hxx"
#include "CommonConverters.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "TitleHelper.hxx"
#include "UndoGuard.hxx"
#include "ControllerLockGuard.hxx"
#include "ObjectNameProvider.hxx"
#include "Strings.hrc"
#include "SchSlotIds.hxx"
#include "macros.hxx"
#include "DragMethod_PieSegment.hxx"
#include "DragMethod_RotateDiagram.hxx"
#include "ObjectHierarchy.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "RelativePositionHelper.hxx"
#include "chartview/DrawModelWrapper.hxx"

#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>

#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>

#ifndef _COM_SUN_STAR_UTIL_XUPDATABLE_HPP_
#include <com/sun/star/util/XUpdatable.hpp>
#endif

#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#include <comphelper/InlineContainer.hxx>
#endif

#include <svtools/contextmenuhelper.hxx>
#include <toolkit/awt/vclxmenu.hxx>

#include <svx/svxids.hrc>
#include <svx/ActionDescriptionProvider.hxx>

// header for class E3dObject
#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif
// header for class E3dScene
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif
// header for class SdrDragMethod
#ifndef _SVDDRGMT_HXX
#include <svx/svddrgmt.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

// for InfoBox
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#include <rtl/math.hxx>

#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#include <svtools/acceleratorexecute.hxx>
#endif

/*
// header for class E3dScene
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif
// header for class XOutputDevice
#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif
// header for class ::basegfx::B3DPolygon
#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif
*/

#define DRGPIX    2     // Drag MinMove in Pixel

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace
{
bool lcl_GrowAndShiftLogic(
    RelativePosition &  rInOutRelPos,
    RelativeSize &      rInOutRelSize,
    const awt::Size &   rRefSize,
    double              fGrowLogicX,
    double              fGrowLogicY )
{
    if( rRefSize.Width == 0 ||
        rRefSize.Height == 0 )
        return false;

    double fRelativeGrowX = fGrowLogicX / rRefSize.Width;
    double fRelativeGrowY = fGrowLogicY / rRefSize.Height;

    return ::chart::RelativePositionHelper::centerGrow(
        rInOutRelPos, rInOutRelSize,
        fRelativeGrowX, fRelativeGrowY,
        /* bCheck = */ true );
}

bool lcl_MoveObjectLogic(
    RelativePosition &  rInOutRelPos,
    RelativeSize &      rObjectSize,
    const awt::Size &   rRefSize,
    double              fShiftLogicX,
    double              fShiftLogicY )
{
    if( rRefSize.Width == 0 ||
        rRefSize.Height == 0 )
        return false;

    double fRelativeShiftX = fShiftLogicX / rRefSize.Width;
    double fRelativeShiftY = fShiftLogicY / rRefSize.Height;

    return ::chart::RelativePositionHelper::moveObject(
        rInOutRelPos, rObjectSize,
        fRelativeShiftX, fRelativeShiftY,
        /* bCheck = */ true );
}

void lcl_insertMenuCommand(
    const uno::Reference< awt::XPopupMenu > & xMenu,
    const uno::Reference< awt::XMenuExtended > & xMenuEx,
    sal_Int16 nId, const ::rtl::OUString & rCommand )
{
    static ::rtl::OUString aEmptyString;
    xMenu->insertItem( nId, aEmptyString, 0, -1 );
    xMenuEx->setCommand( nId, rCommand );
}

} // anonymous namespace


//.............................................................................
namespace chart
{
//.............................................................................

const short HITPIX=2; //hit-tolerance in pixel

//-----------------------------------------------------------------
// awt::XWindow
//-----------------------------------------------------------------
    void SAL_CALL ChartController
::setPosSize( sal_Int32 X, sal_Int32 Y
            , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
            throw (uno::RuntimeException)
{
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;
    Window* pWindow = m_pChartWindow;

    if(xWindow.is() && pWindow)
    {
        Size aLogicSize = pWindow->PixelToLogic( Size( Width, Height ), MapMode( MAP_100TH_MM )  );

        bool bIsEmbedded = true;
        //todo: for standalone chart: detect wether we are standalone
        if( bIsEmbedded )
        {
            //change map mode to fit new size
            awt::Size aModelPageSize = ChartModelHelper::getPageSize( m_aModel->getModel() );
            sal_Int32 nScaleXNumerator = aLogicSize.Width();
            sal_Int32 nScaleXDenominator = aModelPageSize.Width;
            sal_Int32 nScaleYNumerator = aLogicSize.Height();
            sal_Int32 nScaleYDenominator = aModelPageSize.Height;
            MapMode aNewMapMode( MAP_100TH_MM, Point(0,0)
            , Fraction(nScaleXNumerator,nScaleXDenominator)
            , Fraction(nScaleYNumerator,nScaleYDenominator) );
            pWindow->SetMapMode(aNewMapMode);
            pWindow->SetPosSizePixel( X, Y, Width, Height, Flags );

            //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
            uno::Reference< beans::XPropertySet > xProp( m_xChartView, uno::UNO_QUERY );
            if( xProp.is() )
            {
                uno::Sequence< beans::PropertyValue > aZoomFactors(4);
                aZoomFactors[0].Name = C2U("ScaleXNumerator");
                aZoomFactors[0].Value = uno::makeAny( nScaleXNumerator );
                aZoomFactors[1].Name = C2U("ScaleXDenominator");
                aZoomFactors[1].Value = uno::makeAny( nScaleXDenominator );
                aZoomFactors[2].Name = C2U("ScaleYNumerator");
                aZoomFactors[2].Value = uno::makeAny( nScaleYNumerator );
                aZoomFactors[3].Name = C2U("ScaleYDenominator");
                aZoomFactors[3].Value = uno::makeAny( nScaleYDenominator );
                xProp->setPropertyValue( C2U("ZoomFactors"), uno::makeAny( aZoomFactors ));
            }

            //a correct work area is at least necessary for correct values in the position and  size dialog and for dragging area
            if(m_pDrawViewWrapper)
            {
                Rectangle aRect(Point(0,0), pWindow->GetOutputSize());
                m_pDrawViewWrapper->SetWorkArea( aRect );
            }
        }
        else
        {
            //change visarea
            ChartModelHelper::setPageSize( awt::Size( aLogicSize.Width(), aLogicSize.Height() ), m_aModel->getModel() );
            pWindow->SetPosSizePixel( X, Y, Width, Height, Flags );
        }
        pWindow->Invalidate();
    }
}

    awt::Rectangle SAL_CALL ChartController
::getPosSize()
            throw (uno::RuntimeException)
{
    //@todo
    awt::Rectangle aRet(0,0,0,0);

    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;
    if(xWindow.is())
        aRet = xWindow->getPosSize();

    return aRet;
}

    void SAL_CALL ChartController
::setVisible( sal_Bool Visible )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setVisible( Visible );
}

    void SAL_CALL ChartController
::setEnable( sal_Bool Enable )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setEnable( Enable );
}

    void SAL_CALL ChartController
::setFocus()    throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setFocus();
}

    void SAL_CALL ChartController
::addWindowListener( const uno::Reference<
            awt::XWindowListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addWindowListener( xListener );
}

    void SAL_CALL ChartController
::removeWindowListener( const uno::Reference<
            awt::XWindowListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeWindowListener( xListener );
}

    void SAL_CALL ChartController
::addFocusListener( const uno::Reference<
            awt::XFocusListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addFocusListener( xListener );
}

    void SAL_CALL ChartController
::removeFocusListener( const uno::Reference<
            awt::XFocusListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeFocusListener( xListener );
}

    void SAL_CALL ChartController
::addKeyListener( const uno::Reference<
            awt::XKeyListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addKeyListener( xListener );
}

    void SAL_CALL ChartController
::removeKeyListener( const uno::Reference<
            awt::XKeyListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeKeyListener( xListener );
}

    void SAL_CALL ChartController
::addMouseListener( const uno::Reference<
            awt::XMouseListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addMouseListener( xListener );
}

    void SAL_CALL ChartController
::removeMouseListener( const uno::Reference<
            awt::XMouseListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeMouseListener( xListener );
}

    void SAL_CALL ChartController
::addMouseMotionListener( const uno::Reference<
            awt::XMouseMotionListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addMouseMotionListener( xListener );
}

    void SAL_CALL ChartController
::removeMouseMotionListener( const uno::Reference<
            awt::XMouseMotionListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeMouseMotionListener( xListener );
}

    void SAL_CALL ChartController
::addPaintListener( const uno::Reference<
            awt::XPaintListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addPaintListener( xListener );
}

    void SAL_CALL ChartController
::removePaintListener( const uno::Reference<
            awt::XPaintListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removePaintListener( xListener );
}

//-----------------------------------------------------------------
// impl vcl window controller methods
//-----------------------------------------------------------------
void ChartController::execute_Paint( const Rectangle& rRect )
{
    try
    {
        //better performance for big data
        uno::Reference< beans::XPropertySet > xProp( m_xChartView, uno::UNO_QUERY );
        if( xProp.is() )
        {
            awt::Size aResolution(1000,1000);
            {
                ::vos::OGuard aGuard( Application::GetSolarMutex());
                if( m_pChartWindow )
                {
                    aResolution.Width = m_pChartWindow->GetSizePixel().Width();
                    aResolution.Height = m_pChartWindow->GetSizePixel().Height();
                }
            }
            xProp->setPropertyValue( C2U("Resolution"), uno::makeAny( aResolution ));
        }
        //

        uno::Reference< util::XUpdatable > xUpdatable( m_xChartView, uno::UNO_QUERY );
        if( xUpdatable.is() )
            xUpdatable->update();

        Window* pWindow = m_pChartWindow;
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex());
            DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
            if(pDrawViewWrapper)
                pDrawViewWrapper->CompleteRedraw(pWindow, Region(rRect) );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    catch( ... )
    {
    }
}

bool isDoubleClick( const MouseEvent& rMEvt )
{
    return rMEvt.GetClicks() == 2 && rMEvt.IsLeft() &&
        !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsShift();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void ChartController::startDoubleClickWaiting()
{
    m_bWaitingForDoubleClick = true;

    ULONG nDblClkTime = 500;
    if( m_pChartWindow )
    {
        const MouseSettings& rMSettings = m_pChartWindow->GetSettings().GetMouseSettings();
        nDblClkTime = rMSettings.GetDoubleClickTime();
    }
    m_aDoubleClickTimer.SetTimeout( nDblClkTime );
    m_aDoubleClickTimer.Start();
}

void ChartController::stopDoubleClickWaiting()
{
    m_aDoubleClickTimer.Stop();
    m_bWaitingForDoubleClick = false;
}

IMPL_LINK( ChartController, DoubleClickWaitingHdl, void*, EMPTYARG )
{
    m_bWaitingForDoubleClick = false;

    if( !m_bWaitingForMouseUp && m_aSelection.maybeSwitchSelectionAfterSingleClickWasEnsured() )
    {
        this->impl_selectObjectAndNotiy();
        if( m_pChartWindow )
        {
            Window::PointerState aPointerState( m_pChartWindow->GetPointerState() );
            MouseEvent aMouseEvent( aPointerState.maPos,1/*nClicks*/,
                                    0/*nMode*/, static_cast< USHORT >( aPointerState.mnState )/*nButtons*/,
                                    0/*nModifier*/ );
            impl_SetMousePointer( aMouseEvent );
        }
    }

    return 0;
}

//------------------------------------------------------------------------

void ChartController::execute_MouseButtonDown( const MouseEvent& rMEvt )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());

    m_bWaitingForMouseUp = true;

    if( isDoubleClick(rMEvt) )
        stopDoubleClickWaiting();
    else
        startDoubleClickWaiting();

    m_aSelection.remindSelectionBeforeMouseDown();

    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if(!pWindow || !pDrawViewWrapper )
        return;

    Point   aMPos   = pWindow->PixelToLogic(rMEvt.GetPosPixel());

    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        pWindow->GrabFocus();
        pWindow->CaptureMouse();
    }

    if( pDrawViewWrapper->IsTextEdit() )
    {
        if( pDrawViewWrapper->IsTextEditHit( aMPos, HITPIX) )
        {
            pDrawViewWrapper->MouseButtonDown(rMEvt,m_pChartWindow);
            return;
        }
        else
        {
            this->EndTextEdit();
        }
    }

    //abort running action
    if( pDrawViewWrapper->IsAction() )
    {
        if( rMEvt.IsRight() )
            pDrawViewWrapper->BckAction();
        return;
    }

    if( isDoubleClick(rMEvt) ) //do not change selection if double click
        return;//double click is handled further in mousebutton up

    SdrHdl* pHitSelectionHdl = 0;
    //switch from move to resize if handle is hit on a resizeable object
    if( m_aSelection.isResizeableObjectSelected() )
        pHitSelectionHdl = pDrawViewWrapper->PickHandle( aMPos );
    //only change selection if no selection handles are hit
    if( !pHitSelectionHdl )
    {
        m_aSelection.adaptSelectionToNewPos( aMPos, pDrawViewWrapper
            , rMEvt.IsRight(), m_bWaitingForDoubleClick );

        if( !m_aSelection.isRotateableObjectSelected( m_aModel->getModel() ) )
        {
                m_eDragMode = SDRDRAG_MOVE;
                pDrawViewWrapper->SetDragMode(m_eDragMode);
        }

        m_aSelection.applySelection(pDrawViewWrapper);
    }
    if( m_aSelection.isDragableObjectSelected()
         && !rMEvt.IsRight() )
    {
        //start drag
        USHORT  nDrgLog = (USHORT)pWindow->PixelToLogic(Size(DRGPIX,0)).Width();
        SdrDragMethod* pDragMethod = NULL;

        //change selection to 3D scene if rotate mode
        SdrDragMode eDragMode = pDrawViewWrapper->GetDragMode();
        if( SDRDRAG_ROTATE==eDragMode )
        {
            E3dScene* pScene = SelectionHelper::getSceneToRotate( pDrawViewWrapper->getNamedSdrObject( m_aSelection.getSelectedCID() ) );
            if( pScene )
            {
                DragMethod_RotateDiagram::RotationDirection eRotationDirection(DragMethod_RotateDiagram::ROTATIONDIRECTION_FREE);
                if(pHitSelectionHdl)
                {
                    SdrHdlKind eKind = pHitSelectionHdl->GetKind();
                    if( eKind==HDL_UPPER || eKind==HDL_LOWER )
                        eRotationDirection = DragMethod_RotateDiagram::ROTATIONDIRECTION_X;
                    else if( eKind==HDL_LEFT || eKind==HDL_RIGHT )
                        eRotationDirection = DragMethod_RotateDiagram::ROTATIONDIRECTION_Y;
                    else if( eKind==HDL_UPLFT || eKind==HDL_UPRGT || eKind==HDL_LWLFT || eKind==HDL_LWRGT )
                        eRotationDirection = DragMethod_RotateDiagram::ROTATIONDIRECTION_Z;
                }
                pDragMethod = new DragMethod_RotateDiagram( *pDrawViewWrapper, m_aSelection.getSelectedCID(), m_aModel->getModel(), eRotationDirection );
            }
        }
        else
        {
            rtl::OUString aDragMethodServiceName( ObjectIdentifier::getDragMethodServiceName( m_aSelection.getSelectedCID() ) );
            if( aDragMethodServiceName.equals( ObjectIdentifier::getPieSegmentDragMethodServiceName() ) )
                pDragMethod = new DragMethod_PieSegment( *pDrawViewWrapper, m_aSelection.getSelectedCID(), m_aModel->getModel() );
        }
        pDrawViewWrapper->SdrView::BegDragObj(aMPos, NULL, pHitSelectionHdl, nDrgLog, pDragMethod);
    }

    impl_SetMousePointer( rMEvt );
}

void ChartController::execute_MouseMove( const MouseEvent& rMEvt )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());

    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if(!pWindow || !pDrawViewWrapper)
        return;

    if( m_pDrawViewWrapper->IsTextEdit() )
    {
        if( m_pDrawViewWrapper->MouseMove(rMEvt,m_pChartWindow) )
            return;
    }

    if(pDrawViewWrapper->IsAction())
    {
        pDrawViewWrapper->MovAction( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    }

    //??    pDrawViewWrapper->GetPageView()->DragPoly();

    impl_SetMousePointer( rMEvt );
}
void ChartController::execute_Tracking( const TrackingEvent& /* rTEvt */ )
{
}

//-----------------

void ChartController::execute_MouseButtonUp( const MouseEvent& rMEvt )
{
    ControllerLockGuard aCLGuard( m_aModel->getModel());
    m_bWaitingForMouseUp = false;
    bool bNotifySelectionChange = false;
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex());

        Window* pWindow = m_pChartWindow;
        DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
        if(!pWindow || !pDrawViewWrapper)
            return;

        Point   aMPos   = pWindow->PixelToLogic(rMEvt.GetPosPixel());

        if(pDrawViewWrapper->IsTextEdit())
        {
            if( pDrawViewWrapper->MouseButtonUp(rMEvt,m_pChartWindow) )
                return;
        }

        if(pDrawViewWrapper->IsDragObj())
        {
            bool bDraggingDone = false;
            SdrDragMethod* pDragMethod = pDrawViewWrapper->SdrView::GetDragMethod();
            bool bIsMoveOnly = pDragMethod ? pDragMethod->IsMoveOnly() : false;
            DragMethod_Base* pChartDragMethod = dynamic_cast< DragMethod_Base* >(pDragMethod);
            if( pChartDragMethod )
            {
                UndoGuard aUndoGuard( pChartDragMethod->getUndoDescription(),
                        m_xUndoManager, m_aModel->getModel() );

                if( pDrawViewWrapper->EndDragObj(false) )
                {
                    bDraggingDone = true;
                    aUndoGuard.commitAction();
                }
            }

            if( !bDraggingDone && pDrawViewWrapper->EndDragObj(false) )
            {
                try
                {
                    //end move or size
                    SdrObject* pObj = pDrawViewWrapper->getSelectedObject();
                    if( pObj )
                    {
                        Rectangle aObjectRect = pObj->GetSnapRect();
                        awt::Size aPageSize( ChartModelHelper::getPageSize( m_aModel->getModel() ) );
                        Rectangle aPageRect( 0,0,aPageSize.Width,aPageSize.Height );

                        const E3dObject* pE3dObject = dynamic_cast< const E3dObject*>( pObj );
                        if( pE3dObject )
                            aObjectRect = pE3dObject->GetScene()->GetSnapRect();

                        ActionDescriptionProvider::ActionType eActionType(ActionDescriptionProvider::MOVE);
                        if( !bIsMoveOnly && m_aSelection.isResizeableObjectSelected() )
                            eActionType = ActionDescriptionProvider::RESIZE;

                        UndoGuard aUndoGuard(
                            ActionDescriptionProvider::createDescription(
                                eActionType,
                                ObjectNameProvider::getName( ObjectIdentifier::getObjectType( m_aSelection.getSelectedCID() ))),
                            m_xUndoManager, m_aModel->getModel() );
                        bool bChanged = PositionAndSizeHelper::moveObject( m_aSelection.getSelectedCID()
                                        , m_aModel->getModel()
                                        , awt::Rectangle(aObjectRect.getX(),aObjectRect.getY(),aObjectRect.getWidth(),aObjectRect.getHeight())
                                        , awt::Rectangle(aPageRect.getX(),aPageRect.getY(),aPageRect.getWidth(),aPageRect.getHeight())
                                        , m_xChartView );
                        if( bChanged )
                        {
                            bDraggingDone = true;
                            aUndoGuard.commitAction();
                        }
                    }
                }
                catch( uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
                //all wanted model changes will take effect
                //and all unwanted view modifications are cleaned
            }

            if( !bDraggingDone ) //mouse wasn't moved while dragging
            {
                bool bClickedTwiceOnDragableObject = SelectionHelper::isDragableObjectHitTwice( aMPos, m_aSelection.getSelectedCID(), *pDrawViewWrapper );
                bool bIsRotateable = m_aSelection.isRotateableObjectSelected( m_aModel->getModel() );

                //toggel between move and rotate
                if( bIsRotateable && bClickedTwiceOnDragableObject && SDRDRAG_MOVE==m_eDragMode )
                    m_eDragMode=SDRDRAG_ROTATE;
                else
                    m_eDragMode=SDRDRAG_MOVE;

                pDrawViewWrapper->SetDragMode(m_eDragMode);

                if( !m_bWaitingForDoubleClick && m_aSelection.maybeSwitchSelectionAfterSingleClickWasEnsured() )
                {
                    this->impl_selectObjectAndNotiy();
                }
            }
            else
                m_aSelection.resetPossibleSelectionAfterSingleClickWasEnsured();
        }
        else if( isDoubleClick(rMEvt) )
            execute_DoubleClick();

        //@todo ForcePointer(&rMEvt);
        pWindow->ReleaseMouse();

        if( m_aSelection.isSelectionDifferentFromBeforeMouseDown() )
            bNotifySelectionChange = true;
    }

    impl_SetMousePointer( rMEvt );

    if(bNotifySelectionChange)
        impl_notifySelectionChangeListeners();
}

void ChartController::execute_DoubleClick()
{
    ObjectType eObjectType = ObjectIdentifier::getObjectType( m_aSelection.getSelectedCID() );
    if( OBJECTTYPE_TITLE==eObjectType )
        executeDispatch_EditText();
    else
        executeDispatch_ObjectProperties();
}

void ChartController::execute_Resize()
{
    m_pChartWindow->Invalidate();
}
void ChartController::execute_Activate()
{
    ///// pDrawViewWrapper->SetEditMode(TRUE);
}
void ChartController::execute_Deactivate()
{
    /*
    pDrawViewWrapper->SetEditMode(FALSE);
    this->ReleaseMouse();
    */
}
void ChartController::execute_GetFocus()
{
}
void ChartController::execute_LoseFocus()
{
    //this->ReleaseMouse();
}

void ChartController::execute_Command( const CommandEvent& rCEvt )
{
    Window* pWindow = m_pChartWindow;

    bool bIsAction = false;
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex());
        DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
        if(!pWindow || !pDrawViewWrapper)
            return;
        bIsAction = m_pDrawViewWrapper->IsAction();
    }

    // pop-up menu
    if(rCEvt.GetCommand() == COMMAND_CONTEXTMENU && !bIsAction)
    {
        m_pChartWindow->ReleaseMouse();

        if( m_aSelection.isSelectionDifferentFromBeforeMouseDown() )
            impl_notifySelectionChangeListeners();

        // todo: the context menu should be specified by an xml file in uiconfig
        uno::Reference< awt::XPopupMenu > xPopupMenu( new VCLXPopupMenu());
        uno::Reference< awt::XMenuExtended > xMenuEx( xPopupMenu, uno::UNO_QUERY );
        if( xPopupMenu.is() && xMenuEx.is())
        {
            sal_Int16 nUniqueId = 1;
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:DiagramObjects"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:TransformDialog"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId, C2U(".uno:ArrangeRow"));
            uno::Reference< awt::XPopupMenu > xArrangePopupMenu( new VCLXPopupMenu());
            uno::Reference< awt::XMenuExtended > xArrangeMenuEx( xArrangePopupMenu, uno::UNO_QUERY );
            if( xArrangePopupMenu.is() && xArrangeMenuEx.is())
            {
                sal_Int16 nSubId = nUniqueId + 1;
                lcl_insertMenuCommand( xArrangePopupMenu, xArrangeMenuEx, nSubId++, C2U(".uno:Forward"));
                lcl_insertMenuCommand( xArrangePopupMenu, xArrangeMenuEx, nSubId, C2U(".uno:Backward"));
                xPopupMenu->setPopupMenu( nUniqueId, xArrangePopupMenu );
                nUniqueId = nSubId;
            }
            ++nUniqueId;
            xPopupMenu->insertSeparator( -1 );
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:DiagramType"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:DataRanges"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:View3D"));
            xPopupMenu->insertSeparator( -1 );
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:DiagramData"));
            xPopupMenu->insertSeparator( -1 );
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:InsertYErrorbar"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:DeleteYErrorbar"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:InsertMeanValue"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:DeleteMeanValue"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:InsertTrendline"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:DeleteTrendline"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:InsertTrendlineEquation"));
            xPopupMenu->insertSeparator( -1 );
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:Cut"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:Copy"));
            lcl_insertMenuCommand( xPopupMenu, xMenuEx, nUniqueId++, C2U(".uno:Paste"));

            ::svt::ContextMenuHelper aContextMenuHelper( m_xFrame );
            Point aPos( rCEvt.GetMousePosPixel() );
            if( !rCEvt.IsMouseEvent() )
                aPos = m_pChartWindow->GetPointerState().maPos;
            aContextMenuHelper.completeAndExecute( aPos, xPopupMenu );
        }
    }
}

bool ChartController::execute_KeyInput( const KeyEvent& rKEvt )
{
    bool bReturn=false;

    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if(!pWindow || !pDrawViewWrapper)
        return bReturn;

    // handle accelerators
    if( ! m_apAccelExecute.get() && m_xFrame.is() && m_xCC.is() && m_xCC->getServiceManager().is() )
    {
        m_apAccelExecute.reset( ::svt::AcceleratorExecute::createAcceleratorHelper());
        OSL_ASSERT( m_apAccelExecute.get());
        if( m_apAccelExecute.get() )
            m_apAccelExecute->init(
                uno::Reference< lang::XMultiServiceFactory >( m_xCC->getServiceManager(), uno::UNO_QUERY ), m_xFrame );
    }

    KeyCode aKeyCode( rKEvt.GetKeyCode());
    sal_uInt16 nCode = aKeyCode.GetCode();
//     bool bShift = aKeyCode.IsShift();
    bool bAlternate = aKeyCode.IsMod2();

    if( m_apAccelExecute.get() )
        bReturn = m_apAccelExecute->execute( aKeyCode );
    if( bReturn )
        return bReturn;

    if( pDrawViewWrapper->IsTextEdit() )
    {
        if( pDrawViewWrapper->KeyInput(rKEvt,pWindow) )
        {
            bReturn = true;
            if( nCode == KEY_ESCAPE )
            {
                this->EndTextEdit();
            }
        }
    }

    //if( m_pDrawViewWrapper->IsAction() );

    // keyboard accessibility
    ObjectType eObjectType = ObjectIdentifier::getObjectType( m_aSelection.getSelectedCID() );
    if( ! bReturn )
    {
        // Natvigation (Tab/F3/Home/End)
        uno::Reference< XChartDocument > xChartDoc( m_aModel->getModel(), uno::UNO_QUERY );
        ObjectKeyNavigation aObjNav( m_aSelection.getSelectedCID(), xChartDoc, ExplicitValueProvider::getExplicitValueProvider( m_xChartView ));
        awt::KeyEvent aKeyEvent( ::svt::AcceleratorExecute::st_VCLKey2AWTKey( aKeyCode ));
        bReturn = aObjNav.handleKeyEvent( aKeyEvent );
        if( bReturn )
        {
            ::rtl::OUString aNewCID = aObjNav.getCurrentSelection();
            uno::Any aNewSelection;
            if( aNewCID.getLength()>0 && !ObjectHierarchy::isRootNode( aNewCID ))
                aNewSelection <<= aNewCID;
            bReturn = select( aNewSelection );
        }
    }

    // Position and Size (+/-/arrow-keys) or pie segment dragging
    if( ! bReturn  )
    {
        // pie segment dragging
        // note: could also be done for data series
        if( eObjectType == OBJECTTYPE_DATA_POINT &&
            ObjectIdentifier::getDragMethodServiceName( m_aSelection.getSelectedCID() ).equals(
                ObjectIdentifier::getPieSegmentDragMethodServiceName()))
        {
            bool bDrag = false;
            bool bDragInside = false;
            if( nCode == KEY_ADD ||
                nCode == KEY_SUBTRACT )
            {
                bDrag = true;
                bDragInside = ( nCode == KEY_SUBTRACT );
            }
            else if(
                nCode == KEY_LEFT ||
                nCode == KEY_RIGHT ||
                nCode == KEY_UP ||
                nCode == KEY_DOWN )
            {
                bDrag = true;
                rtl::OUString aParameter( ObjectIdentifier::getDragParameterString( m_aSelection.getSelectedCID() ));
                sal_Int32 nOffsetPercentDummy( 0 );
                awt::Point aMinimumPosition( 0, 0 );
                awt::Point aMaximumPosition( 0, 0 );
                ObjectIdentifier::parsePieSegmentDragParameterString(
                    aParameter, nOffsetPercentDummy, aMinimumPosition, aMaximumPosition );
                aMaximumPosition.Y -= aMinimumPosition.Y;
                aMaximumPosition.X -= aMinimumPosition.X;

                bDragInside =
                    (nCode == KEY_RIGHT && (aMaximumPosition.X < 0)) ||
                    (nCode == KEY_LEFT  && (aMaximumPosition.X > 0)) ||
                    (nCode == KEY_DOWN  && (aMaximumPosition.Y < 0)) ||
                    (nCode == KEY_UP    && (aMaximumPosition.Y > 0));
            }

            if( bDrag )
            {
                double fAmount = bAlternate ? 0.01 : 0.05;
                if( bDragInside )
                    fAmount *= -1.0;

                bReturn = impl_DragDataPoint( m_aSelection.getSelectedCID(), fAmount );
            }
        }
        else
        {
            // size
            if( nCode == KEY_ADD ||
                nCode == KEY_SUBTRACT )
            {
                if( eObjectType == OBJECTTYPE_DIAGRAM )
                {
                    // default 1 mm in each direction
                    double fGrowAmountX = 200.0;
                    double fGrowAmountY = 200.0;
                    if( bAlternate && pWindow )
                    {
                        // together with Alt-key: 1 px in each direction
                        Size aPixelSize = pWindow->PixelToLogic( Size( 2, 2 ));
                        fGrowAmountX = static_cast< double >( aPixelSize.Width());
                        fGrowAmountY = static_cast< double >( aPixelSize.Height());
                    }
                    if( nCode == KEY_SUBTRACT )
                    {
                        fGrowAmountX = -fGrowAmountX;
                        fGrowAmountY = -fGrowAmountY;
                    }
                    bReturn = impl_moveOrResizeObject(
                        m_aSelection.getSelectedCID(), CENTERED_RESIZE_OBJECT, fGrowAmountX, fGrowAmountY );
                }
            }
            // position
            else if( nCode == KEY_LEFT  ||
                     nCode == KEY_RIGHT ||
                     nCode == KEY_UP ||
                     nCode == KEY_DOWN )
            {
                if( m_aSelection.isDragableObjectSelected() )
                {
                    // default 1 mm
                    double fShiftAmountX = 100.0;
                    double fShiftAmountY = 100.0;
                    if( bAlternate && pWindow )
                    {
                        // together with Alt-key: 1 px
                        Size aPixelSize = pWindow->PixelToLogic( Size( 1, 1 ));
                        fShiftAmountX = static_cast< double >( aPixelSize.Width());
                        fShiftAmountY = static_cast< double >( aPixelSize.Height());
                    }
                    switch( nCode )
                    {
                        case KEY_LEFT:
                            fShiftAmountX = -fShiftAmountX;
                            fShiftAmountY = 0.0;
                            break;
                        case KEY_RIGHT:
                            fShiftAmountY = 0.0;
                            break;
                        case KEY_UP:
                            fShiftAmountX = 0.0;
                            fShiftAmountY = -fShiftAmountY;
                            break;
                        case KEY_DOWN:
                            fShiftAmountX = 0.0;
                            break;
                    }
                    if( m_aSelection.getSelectedCID().getLength() )
                    {
                        //move chart objects
                        bReturn = impl_moveOrResizeObject(
                            m_aSelection.getSelectedCID(), MOVE_OBJECT, fShiftAmountX, fShiftAmountY );
                    }
                    else
                    {
                        //move additional shapes
                        uno::Reference< drawing::XShape > xShape( m_aSelection.getSelectedAdditionalShape() );
                        if( xShape.is() )
                        {
                            awt::Point aPos( xShape->getPosition() );
                            awt::Size aSize( xShape->getSize() );
                            awt::Size aPageSize( ChartModelHelper::getPageSize( m_aModel->getModel() ) );
                            aPos.X = static_cast< long >( static_cast< double >( aPos.X ) + fShiftAmountX );
                            aPos.Y = static_cast< long >( static_cast< double >( aPos.Y ) + fShiftAmountY );
                            if( aPos.X + aSize.Width > aPageSize.Width )
                                aPos.X = aPageSize.Width - aSize.Width;
                            if( aPos.X < 0 )
                                aPos.X = 0;
                            if( aPos.Y + aSize.Height > aPageSize.Height )
                                aPos.Y = aPageSize.Height - aSize.Height;
                            if( aPos.Y < 0 )
                                aPos.Y = 0;

                            xShape->setPosition( aPos );
                        }
                    }
                }
            }
        }
    }

    // text edit
    if( ! bReturn &&
        nCode == KEY_F2 )
    {
        if( OBJECTTYPE_TITLE == eObjectType )
        {
            executeDispatch_EditText();
            bReturn = true;
        }
    }

    // deactivate inplace mode (this code should be unnecessary, but
    // unfortunately is not)
    if( ! bReturn &&
        nCode == KEY_ESCAPE )
    {
        uno::Reference< frame::XDispatchHelper > xDispatchHelper(
            m_xCC->getServiceManager()->createInstanceWithContext(
                C2U("com.sun.star.frame.DispatchHelper"), m_xCC ), uno::UNO_QUERY );
        if( xDispatchHelper.is())
        {
            uno::Sequence< beans::PropertyValue > aArgs;
            xDispatchHelper->executeDispatch(
                uno::Reference< frame::XDispatchProvider >( m_xFrame, uno::UNO_QUERY ),
                C2U(".uno:TerminateInplaceActivation"),
                C2U("_parent"),
                frame::FrameSearchFlag::PARENT,
                aArgs );
            bReturn = true;
        }
    }

    if( ! bReturn &&
        (nCode == KEY_DELETE || nCode == KEY_BACKSPACE ))
    {
        bReturn = executeDispatch_Delete();
        if( ! bReturn )
        {
            InfoBox( m_pChartWindow, String(SchResId( STR_ACTION_NOTPOSSIBLE ))).Execute();
        }
    }

    /* old chart:
    // Ctrl-Shift-R: Repaint
    if (!bReturn && GetWindow())
    {
        KeyCode aKeyCode = rKEvt.GetKeyCode();

        if (aKeyCode.IsMod1() && aKeyCode.IsShift()
            && aKeyCode.GetCode() == KEY_R)
        {
                // 3D-Kontext wieder zerstoeren
            Base3D* pBase3D = (Base3D*) GetWindow()->Get3DContext();

            if (pBase3D)
            {
                pBase3D->Destroy(GetWindow());
            }
            GetWindow()->Invalidate();
            bReturn = TRUE;
        }
    }
    */
    return bReturn;
}

bool ChartController::requestQuickHelp(
    ::Point aAtLogicPosition,
    bool bIsBalloonHelp,
    ::rtl::OUString & rOutQuickHelpText,
    awt::Rectangle & rOutEqualRect )
{
    uno::Reference< frame::XModel > xChartModel;
    if( m_aModel.is())
        xChartModel.set( m_aModel->getModel());
    if( !xChartModel.is())
        return false;

    // help text
    ::rtl::OUString aCID;
    if( m_pDrawViewWrapper )
    {
        aCID = SelectionHelper::getHitObjectCID(
            aAtLogicPosition, *m_pDrawViewWrapper );
    }
    bool bResult( aCID.getLength());

    if( bResult )
    {
        // get help text
        rOutQuickHelpText = ObjectNameProvider::getHelpText( aCID, xChartModel, bIsBalloonHelp /* bVerbose */ );

        // set rectangle
        ExplicitValueProvider * pValueProvider(
            ExplicitValueProvider::getExplicitValueProvider( m_xChartView ));
        if( pValueProvider )
            rOutEqualRect = pValueProvider->getRectangleOfObject( aCID, true );
    }

    return bResult;
}

//-----------------------------------------------------------------
// XSelectionSupplier (optional interface)
//-----------------------------------------------------------------
        sal_Bool SAL_CALL ChartController
::select( const uno::Any& rSelection )
        throw( lang::IllegalArgumentException )
{
    rtl::OUString aNewCID;
    if( rSelection.hasValue() &&
        ! (rSelection >>= aNewCID))
        return sal_False;


    if( m_aSelection.setSelection( aNewCID ) )
    {
        this->impl_selectObjectAndNotiy();
        return sal_True;
    }
    return sal_False;
}

        uno::Any SAL_CALL ChartController
::getSelection() throw(uno::RuntimeException)
{
    return uno::makeAny(m_aSelection.getSelectedCID());
}

        void SAL_CALL ChartController
::addSelectionChangeListener( const uno::Reference<
        view::XSelectionChangeListener > & xListener )
        throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());
    if( impl_isDisposedOrSuspended() )//@todo? allow adding of listeners in suspend mode?
        return; //behave passive if already disposed or suspended

    //--add listener
    m_aLifeTimeManager.m_aListenerContainer.addInterface( ::getCppuType((const uno::Reference< view::XSelectionChangeListener >*)0), xListener );
}

        void SAL_CALL ChartController
::removeSelectionChangeListener( const uno::Reference<
        view::XSelectionChangeListener > & xListener )
        throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());
    if( impl_isDisposedOrSuspended() ) //@todo? allow removing of listeners in suspend mode?
        return; //behave passive if already disposed or suspended

    //--remove listener
    m_aLifeTimeManager.m_aListenerContainer.removeInterface( ::getCppuType((const uno::Reference< view::XSelectionChangeListener >*)0), xListener );
}

        void ChartController
::impl_notifySelectionChangeListeners()
{
    ::cppu::OInterfaceContainerHelper* pIC = m_aLifeTimeManager.m_aListenerContainer
        .getContainer( ::getCppuType((const uno::Reference< view::XSelectionChangeListener >*)0) );
    if( pIC )
    {
        uno::Reference< view::XSelectionSupplier > xSelectionSupplier(this);
        lang::EventObject aEvent( xSelectionSupplier );
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
            (static_cast< view::XSelectionChangeListener*>(aIt.next()))->selectionChanged( aEvent );
    }
}

void ChartController::impl_selectObjectAndNotiy()
{
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if( pDrawViewWrapper )
    {
        pDrawViewWrapper->SetDragMode( m_eDragMode );
        m_aSelection.applySelection( m_pDrawViewWrapper );
    }
    impl_notifySelectionChangeListeners();
}

bool ChartController::impl_moveOrResizeObject(
    const ::rtl::OUString & rCID,
    eMoveOrResizeType eType,
    double fAmountLogicX,
    double fAmountLogicY )
{
    bool bResult = false;
    bool bNeedShift = true;
    bool bNeedResize = ( eType == CENTERED_RESIZE_OBJECT );

    uno::Reference< frame::XModel > xChartModel( m_aModel->getModel());
    uno::Reference< beans::XPropertySet > xObjProp(
        ObjectIdentifier::getObjectPropertySet( rCID, xChartModel ));
    if( xObjProp.is())
    {
        awt::Size aRefSize = ChartModelHelper::getPageSize( xChartModel );

        chart2::RelativePosition aRelPos;
        chart2::RelativeSize     aRelSize;
        bool bDeterminePos  = !(xObjProp->getPropertyValue( C2U("RelativePosition")) >>= aRelPos);
        bool bDetermineSize = !bNeedResize || !(xObjProp->getPropertyValue( C2U("RelativeSize")) >>= aRelSize);

        if( ( bDeterminePos || bDetermineSize ) &&
            ( aRefSize.Width > 0 && aRefSize.Height > 0 ) )
        {
            ExplicitValueProvider * pValueProvider(
                ExplicitValueProvider::getExplicitValueProvider( m_xChartView ));
            if( pValueProvider )
            {
                awt::Rectangle aRect( pValueProvider->getRectangleOfObject( rCID ));
                double fWidth = static_cast< double >( aRefSize.Width );
                double fHeight = static_cast< double >( aRefSize.Height );
                if( bDetermineSize )
                {
                    aRelSize.Primary   = static_cast< double >( aRect.Width ) / fWidth;
                    aRelSize.Secondary = static_cast< double >( aRect.Height ) / fHeight;
                }
                if( bDeterminePos )
                {
                    if( bNeedResize && aRelSize.Primary > 0.0 && aRelSize.Secondary > 0.0 )
                    {
                        aRelPos.Primary   = (static_cast< double >( aRect.X ) / fWidth) +
                            (aRelSize.Primary / 2.0);
                        aRelPos.Secondary = (static_cast< double >( aRect.Y ) / fHeight) +
                            (aRelSize.Secondary / 2.0);
                        aRelPos.Anchor = drawing::Alignment_CENTER;
                    }
                    else
                    {
                        aRelPos.Primary   = static_cast< double >( aRect.X ) / fWidth;
                        aRelPos.Secondary = static_cast< double >( aRect.Y ) / fHeight;
                        aRelPos.Anchor = drawing::Alignment_TOP_LEFT;
                    }
                }
            }
        }

        if( eType == CENTERED_RESIZE_OBJECT )
            bResult = lcl_GrowAndShiftLogic( aRelPos, aRelSize, aRefSize, fAmountLogicX, fAmountLogicY );
        else if( eType == MOVE_OBJECT )
            bResult = lcl_MoveObjectLogic( aRelPos, aRelSize, aRefSize, fAmountLogicX, fAmountLogicY );

        if( bResult )
        {
            ActionDescriptionProvider::ActionType eActionType(ActionDescriptionProvider::MOVE);
            if( bNeedResize )
                eActionType = ActionDescriptionProvider::RESIZE;

            UndoGuard aUndoGuard(
                ActionDescriptionProvider::createDescription(
                    eActionType,
                    ObjectNameProvider::getName( ObjectIdentifier::getObjectType( rCID ))),
                m_xUndoManager, xChartModel );
            {
                ControllerLockGuard aCLGuard( xChartModel );
                if( bNeedShift )
                    xObjProp->setPropertyValue( C2U("RelativePosition"), uno::makeAny( aRelPos ));
                if( bNeedResize )
                    xObjProp->setPropertyValue( C2U("RelativeSize"), uno::makeAny( aRelSize ));
            }
            aUndoGuard.commitAction();
        }
    }
    return bResult;
}

bool ChartController::impl_DragDataPoint( const ::rtl::OUString & rCID, double fAdditionalOffset )
{
    bool bResult = false;
    if( fAdditionalOffset < -1.0 || fAdditionalOffset > 1.0 || fAdditionalOffset == 0.0 )
        return bResult;

    sal_Int32 nDataPointIndex = ObjectIdentifier::getIndexFromParticleOrCID( rCID );
    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( rCID, m_aModel->getModel()));
    if( xSeries.is())
    {
        try
        {
            uno::Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex( nDataPointIndex ));
            double fOffset = 0.0;
            if( xPointProp.is() &&
                (xPointProp->getPropertyValue( C2U("Offset" )) >>= fOffset ) &&
                (( fAdditionalOffset > 0.0 && fOffset < 1.0 ) || (fOffset > 0.0)) )
            {
                fOffset += fAdditionalOffset;
                if( fOffset > 1.0 )
                    fOffset = 1.0;
                else if( fOffset < 0.0 )
                    fOffset = 0.0;
                xPointProp->setPropertyValue( C2U("Offset"), uno::makeAny( fOffset ));
                bResult = true;
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return bResult;
}

void ChartController::impl_SetMousePointer( const MouseEvent & rEvent )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());
    Window* pWindow = m_pChartWindow;
    if( m_pDrawViewWrapper && pWindow )
    {
        Point aMousePos( pWindow->PixelToLogic( rEvent.GetPosPixel()));
        sal_uInt16 nModifier = rEvent.GetModifier();
        BOOL bLeftDown = rEvent.IsLeft();

        if( m_pDrawViewWrapper->IsTextEdit() )
        {
            if( m_pDrawViewWrapper->IsTextEditHit( aMousePos, HITPIX) )
            {
                pWindow->SetPointer( m_pDrawViewWrapper->GetPreferedPointer(
                    aMousePos, pWindow, nModifier, bLeftDown ) );
                return;
            }
        }
        else if( m_pDrawViewWrapper->IsAction() )
        {
            return;//don't change pointer during running action
        }

        SdrHdl* pHitSelectionHdl = 0;
        if( m_aSelection.isResizeableObjectSelected() )
            pHitSelectionHdl = m_pDrawViewWrapper->PickHandle( aMousePos );

        if( pHitSelectionHdl )
        {

            Pointer aPointer = m_pDrawViewWrapper->GetPreferedPointer(
                aMousePos, pWindow, nModifier, bLeftDown );
            bool bForceArrowPointer = false;

            ::rtl::OUString aObjectCID = m_aSelection.getSelectedCID();

            switch( aPointer.GetStyle())
            {
                case POINTER_NSIZE:
                case POINTER_SSIZE:
                case POINTER_WSIZE:
                case POINTER_ESIZE:
                case POINTER_NWSIZE:
                case POINTER_NESIZE:
                case POINTER_SWSIZE:
                case POINTER_SESIZE:
                    if( ! m_aSelection.isResizeableObjectSelected() )
                        bForceArrowPointer = true;
                    break;
                case POINTER_MOVE:
                    if( ! ObjectIdentifier::isDragableObject( aObjectCID ))
                        bForceArrowPointer = true;
                    break;
                case POINTER_MOVEPOINT:
                case POINTER_MOVEBEZIERWEIGHT:
                    // there is no point-editing in a chart
                    // the POINTER_MOVEBEZIERWEIGHT appears in 3d data points
                    bForceArrowPointer = true;
                    break;
                default:
                    break;
            }

            if( bForceArrowPointer )
                pWindow->SetPointer( Pointer( POINTER_ARROW ));
            else
                pWindow->SetPointer( aPointer );
        }
        else
        {
            ::rtl::OUString aHitObjectCID(
                SelectionHelper::getHitObjectCID(
                    aMousePos, *m_pDrawViewWrapper, true /*bGetDiagramInsteadOf_Wall*/ ));

            if( m_pDrawViewWrapper->IsTextEdit() )
            {
                if( aHitObjectCID.equals(m_aSelection.getSelectedCID()) )
                {
                    pWindow->SetPointer( Pointer( POINTER_ARROW ));
                    return;
                }
            }

            if( !aHitObjectCID.getLength() )
            {
                //additional shape was hit
                pWindow->SetPointer( POINTER_MOVE );
            }
            else if( ObjectIdentifier::isDragableObject( aHitObjectCID ) )
            {
                if( (m_eDragMode == SDRDRAG_ROTATE)
                    && SelectionHelper::isRotateableObject( aHitObjectCID
                        , m_aModel->getModel() ) )
                    pWindow->SetPointer( Pointer( POINTER_ROTATE ) );
                else
                {
                    ObjectType eHitObjectType = ObjectIdentifier::getObjectType( aHitObjectCID );
                    if( eHitObjectType == OBJECTTYPE_DATA_POINT )
                    {
                        if( !ObjectIdentifier::areSiblings(aHitObjectCID,m_aSelection.getSelectedCID())
                            && !ObjectIdentifier::areIdenticalObjects(aHitObjectCID,m_aSelection.getSelectedCID()) )
                        {
                            pWindow->SetPointer( Pointer( POINTER_ARROW ));
                            return;
                        }
                    }
                    pWindow->SetPointer( POINTER_MOVE );
                }
            }
            else
                pWindow->SetPointer( Pointer( POINTER_ARROW ));
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
