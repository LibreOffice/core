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
#include "macros.hxx"
#include "DragMethod_PieSegment.hxx"
#include "DragMethod_RotateDiagram.hxx"
#include "ObjectHierarchy.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "RelativePositionHelper.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "RegressionCurveHelper.hxx"
#include "StatisticsHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "ContainerHelper.hxx"
#include "AxisHelper.hxx"
#include "LegendHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "DrawCommandDispatch.hxx"

#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>

#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>

#include <svtools/contextmenuhelper.hxx>
#include <toolkit/awt/vclxmenu.hxx>

#include <svx/svxids.hrc>
#include <svx/ActionDescriptionProvider.hxx>

#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/svddrgmt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/mutex.hxx>
#include <vcl/msgbox.hxx>
#include <rtl/math.hxx>
#include <svtools/acceleratorexecute.hxx>

#define DRGPIX    2     // Drag MinMove in Pixel

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace chart
{

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
        fRelativeGrowX, fRelativeGrowY );
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
        fRelativeShiftX, fRelativeShiftY );
}

void lcl_insertMenuCommand(
    const uno::Reference< awt::XPopupMenu > & xMenu,
    sal_Int16 nId, const OUString & rCommand )
{
    static OUString aEmptyString;
    xMenu->insertItem( nId, aEmptyString, 0, -1 );
    xMenu->setCommand( nId, rCommand );
}

OUString lcl_getFormatCommandForObjectCID( const OUString& rCID )
{
    OUString aDispatchCommand( ".uno:FormatSelection" );

    ObjectType eObjectType = ObjectIdentifier::getObjectType( rCID );

    switch(eObjectType)
    {
        case OBJECTTYPE_DIAGRAM:
        case OBJECTTYPE_DIAGRAM_WALL:
            aDispatchCommand = ".uno:FormatWall";
            break;
        case OBJECTTYPE_DIAGRAM_FLOOR:
            aDispatchCommand = ".uno:FormatFloor";
            break;
        case OBJECTTYPE_PAGE:
            aDispatchCommand = ".uno:FormatChartArea";
            break;
        case OBJECTTYPE_LEGEND:
            aDispatchCommand = ".uno:FormatLegend";
            break;
        case OBJECTTYPE_TITLE:
            aDispatchCommand = ".uno:FormatTitle";
            break;
        case OBJECTTYPE_LEGEND_ENTRY:
            aDispatchCommand = ".uno:FormatDataSeries";
            break;
        case OBJECTTYPE_AXIS:
        case OBJECTTYPE_AXIS_UNITLABEL:
            aDispatchCommand = ".uno:FormatAxis";
            break;
        case OBJECTTYPE_GRID:
            aDispatchCommand = ".uno:FormatMajorGrid";
            break;
        case OBJECTTYPE_SUBGRID:
            aDispatchCommand = ".uno:FormatMinorGrid";
            break;
        case OBJECTTYPE_DATA_LABELS:
            aDispatchCommand = ".uno:FormatDataLabels";
            break;
        case OBJECTTYPE_DATA_SERIES:
            aDispatchCommand = ".uno:FormatDataSeries";
            break;
        case OBJECTTYPE_DATA_LABEL:
            aDispatchCommand = ".uno:FormatDataLabel";
            break;
        case OBJECTTYPE_DATA_POINT:
            aDispatchCommand = ".uno:FormatDataPoint";
            break;
        case OBJECTTYPE_DATA_AVERAGE_LINE:
            aDispatchCommand = ".uno:FormatMeanValue";
            break;
        case OBJECTTYPE_DATA_ERRORS_X:
            aDispatchCommand = ".uno:FormatXErrorBars";
            break;
        case OBJECTTYPE_DATA_ERRORS_Y:
            aDispatchCommand = ".uno:FormatYErrorBars";
            break;
        case OBJECTTYPE_DATA_ERRORS_Z:
            aDispatchCommand = ".uno:FormatZErrorBars";
            break;
        case OBJECTTYPE_DATA_CURVE:
            aDispatchCommand = ".uno:FormatTrendline";
            break;
        case OBJECTTYPE_DATA_CURVE_EQUATION:
            aDispatchCommand = ".uno:FormatTrendlineEquation";
            break;
        case OBJECTTYPE_DATA_STOCK_RANGE:
            aDispatchCommand = ".uno:FormatSelection";
            break;
        case OBJECTTYPE_DATA_STOCK_LOSS:
            aDispatchCommand = ".uno:FormatStockLoss";
            break;
        case OBJECTTYPE_DATA_STOCK_GAIN:
            aDispatchCommand = ".uno:FormatStockGain";
            break;
        default: //OBJECTTYPE_UNKNOWN
            break;
    }
    return aDispatchCommand;
}

} // anonymous namespace

// awt::XWindow
void SAL_CALL ChartController::setPosSize(
    sal_Int32 X,
    sal_Int32 Y,
    sal_Int32 Width,
    sal_Int32 Height,
    sal_Int16 Flags )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is() && m_pChartWindow)
    {
        Size aLogicSize = m_pChartWindow->PixelToLogic( Size( Width, Height ), MapMode( MAP_100TH_MM )  );

        //todo: for standalone chart: detect whether we are standalone
        //change map mode to fit new size
        awt::Size aModelPageSize = ChartModelHelper::getPageSize( getModel() );
        sal_Int32 nScaleXNumerator = aLogicSize.Width();
        sal_Int32 nScaleXDenominator = aModelPageSize.Width;
        sal_Int32 nScaleYNumerator = aLogicSize.Height();
        sal_Int32 nScaleYDenominator = aModelPageSize.Height;
        MapMode aNewMapMode(
                    MAP_100TH_MM,
                    Point(0,0),
                    Fraction(nScaleXNumerator, nScaleXDenominator),
                    Fraction(nScaleYNumerator, nScaleYDenominator) );
        m_pChartWindow->SetMapMode(aNewMapMode);
        m_pChartWindow->setPosSizePixel( X, Y, Width, Height, static_cast<PosSizeFlags>(Flags) );

        //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
        uno::Reference< beans::XPropertySet > xProp( m_xChartView, uno::UNO_QUERY );
        if( xProp.is() )
        {
            auto aZoomFactors(::comphelper::InitPropertySequence({
                { "ScaleXNumerator", uno::makeAny( nScaleXNumerator ) },
                { "ScaleXDenominator", uno::makeAny( nScaleXDenominator ) },
                { "ScaleYNumerator", uno::makeAny( nScaleYNumerator ) },
                { "ScaleYDenominator", uno::makeAny( nScaleYDenominator ) }
            }));
            xProp->setPropertyValue( "ZoomFactors", uno::makeAny( aZoomFactors ));
        }

        //a correct work area is at least necessary for correct values in the position and  size dialog and for dragging area
        if(m_pDrawViewWrapper)
        {
            Rectangle aRect(Point(0,0), m_pChartWindow->GetOutputSize());
            m_pDrawViewWrapper->SetWorkArea( aRect );
        }
        m_pChartWindow->Invalidate();
    }
}

awt::Rectangle SAL_CALL ChartController::getPosSize()
    throw (uno::RuntimeException, std::exception)
{
    //@todo
    awt::Rectangle aRet(0, 0, 0, 0);

    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;
    if(xWindow.is())
        aRet = xWindow->getPosSize();

    return aRet;
}

void SAL_CALL ChartController::setVisible( sal_Bool Visible )
    throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setVisible( Visible );
}

void SAL_CALL ChartController::setEnable( sal_Bool Enable )
    throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setEnable( Enable );
}

void SAL_CALL ChartController::setFocus()
    throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setFocus();
}

void SAL_CALL ChartController::addWindowListener(
    const uno::Reference< awt::XWindowListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addWindowListener( xListener );
}

void SAL_CALL ChartController::removeWindowListener(
    const uno::Reference< awt::XWindowListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeWindowListener( xListener );
}

void SAL_CALL ChartController::addFocusListener(
    const uno::Reference< awt::XFocusListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addFocusListener( xListener );
}

void SAL_CALL ChartController::removeFocusListener(
    const uno::Reference< awt::XFocusListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeFocusListener( xListener );
}

void SAL_CALL ChartController::addKeyListener(
    const uno::Reference< awt::XKeyListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addKeyListener( xListener );
}

void SAL_CALL ChartController::removeKeyListener(
    const uno::Reference< awt::XKeyListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeKeyListener( xListener );
}

void SAL_CALL ChartController::addMouseListener(
    const uno::Reference< awt::XMouseListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addMouseListener( xListener );
}

void SAL_CALL ChartController::removeMouseListener(
    const uno::Reference< awt::XMouseListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeMouseListener( xListener );
}

void SAL_CALL ChartController::addMouseMotionListener(
    const uno::Reference< awt::XMouseMotionListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addMouseMotionListener( xListener );
}

void SAL_CALL ChartController::removeMouseMotionListener(
    const uno::Reference< awt::XMouseMotionListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeMouseMotionListener( xListener );
}

void SAL_CALL ChartController::addPaintListener(
    const uno::Reference< awt::XPaintListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addPaintListener( xListener );
}

void SAL_CALL ChartController::removePaintListener(
    const uno::Reference< awt::XPaintListener >& xListener )
        throw (uno::RuntimeException, std::exception)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removePaintListener( xListener );
}

// impl vcl window controller methods
void ChartController::PrePaint(vcl::RenderContext& /*rRenderContext*/)
{
    // forward VCLs PrePaint window event to DrawingLayer
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;

    if (pDrawViewWrapper)
    {
        pDrawViewWrapper->PrePaint();
    }
}

void ChartController::execute_Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    try
    {
        uno::Reference<frame::XModel> xModel(getModel());
        //OSL_ENSURE( xModel.is(), "ChartController::execute_Paint: have no model to paint");
        if (!xModel.is())
            return;

        //better performance for big data
        uno::Reference<beans::XPropertySet> xProp(m_xChartView, uno::UNO_QUERY);
        if (xProp.is())
        {
            awt::Size aResolution(1000, 1000);
            {
                SolarMutexGuard aGuard;
                if (m_pChartWindow)
                {
                    aResolution.Width = m_pChartWindow->GetSizePixel().Width();
                    aResolution.Height = m_pChartWindow->GetSizePixel().Height();
                }
            }
            xProp->setPropertyValue( "Resolution", uno::makeAny( aResolution ));
        }

        uno::Reference< util::XUpdatable > xUpdatable( m_xChartView, uno::UNO_QUERY );
        if (xUpdatable.is())
            xUpdatable->update();

        {
            SolarMutexGuard aGuard;
            DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
            if (pDrawViewWrapper)
                pDrawViewWrapper->CompleteRedraw(&rRenderContext, vcl::Region(rRect));
        }
    }
    catch( const uno::Exception & ex )
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

void ChartController::startDoubleClickWaiting()
{
    SolarMutexGuard aGuard;

    m_bWaitingForDoubleClick = true;

    sal_uInt64 nDblClkTime = 500;
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

IMPL_LINK_NOARG_TYPED(ChartController, DoubleClickWaitingHdl, Timer *, void)
{
    m_bWaitingForDoubleClick = false;

    if( !m_bWaitingForMouseUp && m_aSelection.maybeSwitchSelectionAfterSingleClickWasEnsured() )
    {
        this->impl_selectObjectAndNotiy();
        SolarMutexGuard aGuard;
        if( m_pChartWindow )
        {
            vcl::Window::PointerState aPointerState( m_pChartWindow->GetPointerState() );
            MouseEvent aMouseEvent(
                            aPointerState.maPos,
                            1/*nClicks*/,
                            MouseEventModifiers::NONE,
                            static_cast< sal_uInt16 >( aPointerState.mnState )/*nButtons*/,
                            0/*nModifier*/ );
            impl_SetMousePointer( aMouseEvent );
        }
    }
}

void ChartController::execute_MouseButtonDown( const MouseEvent& rMEvt )
{
    SolarMutexGuard aGuard;

    m_bWaitingForMouseUp = true;

    if( isDoubleClick(rMEvt) )
        stopDoubleClickWaiting();
    else
        startDoubleClickWaiting();

    m_aSelection.remindSelectionBeforeMouseDown();

    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if(!m_pChartWindow || !pDrawViewWrapper )
        return;

    Point   aMPos   = m_pChartWindow->PixelToLogic(rMEvt.GetPosPixel());

    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        m_pChartWindow->GrabFocus();
        m_pChartWindow->CaptureMouse();
    }

    if( pDrawViewWrapper->IsTextEdit() )
    {
        SdrViewEvent aVEvt;
        if ( pDrawViewWrapper->IsTextEditHit( aMPos ) ||
             // #i12587# support for shapes in chart
             ( rMEvt.IsRight() && pDrawViewWrapper->PickAnything( rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt ) == SDRHIT_MARKEDOBJECT ) )
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

    SdrHdl* pHitSelectionHdl = nullptr;
    //switch from move to resize if handle is hit on a resizable object
    if( m_aSelection.isResizeableObjectSelected() )
        pHitSelectionHdl = pDrawViewWrapper->PickHandle( aMPos );
    //only change selection if no selection handles are hit
    if( !pHitSelectionHdl )
    {
        // #i12587# support for shapes in chart
        if ( m_eDrawMode == CHARTDRAW_INSERT &&
             ( !pDrawViewWrapper->IsMarkedHit( aMPos ) || !m_aSelection.isDragableObjectSelected() ) )
        {
            if ( m_aSelection.hasSelection() )
            {
                m_aSelection.clearSelection();
            }
            if ( !pDrawViewWrapper->IsAction() )
            {
                if ( pDrawViewWrapper->GetCurrentObjIdentifier() == OBJ_CAPTION )
                {
                    Size aCaptionSize( 2268, 1134 );
                    pDrawViewWrapper->BegCreateCaptionObj( aMPos, aCaptionSize );
                }
                else
                {
                    pDrawViewWrapper->BegCreateObj( aMPos);
                }
                SdrObject* pObj = pDrawViewWrapper->GetCreateObj();
                DrawCommandDispatch* pDrawCommandDispatch = m_aDispatchContainer.getDrawCommandDispatch();
                if ( pObj && m_pDrawModelWrapper && pDrawCommandDispatch )
                {
                    SfxItemSet aSet( m_pDrawModelWrapper->GetItemPool() );
                    pDrawCommandDispatch->setAttributes( pObj );
                    pDrawCommandDispatch->setLineEnds( aSet );
                    pObj->SetMergedItemSet( aSet );
                }
            }
            impl_SetMousePointer( rMEvt );
            return;
        }

        m_aSelection.adaptSelectionToNewPos(
                        aMPos,
                        pDrawViewWrapper,
                        rMEvt.IsRight(),
                        m_bWaitingForDoubleClick );

        if( !m_aSelection.isRotateableObjectSelected( getModel() ) )
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
        sal_uInt16  nDrgLog = (sal_uInt16)m_pChartWindow->PixelToLogic(Size(DRGPIX,0)).Width();
        SdrDragMethod* pDragMethod = nullptr;

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
                pDragMethod = new DragMethod_RotateDiagram( *pDrawViewWrapper, m_aSelection.getSelectedCID(), getModel(), eRotationDirection );
            }
        }
        else
        {
            OUString aDragMethodServiceName( ObjectIdentifier::getDragMethodServiceName( m_aSelection.getSelectedCID() ) );
            if( aDragMethodServiceName.equals( ObjectIdentifier::getPieSegmentDragMethodServiceName() ) )
                pDragMethod = new DragMethod_PieSegment( *pDrawViewWrapper, m_aSelection.getSelectedCID(), getModel() );
        }
        pDrawViewWrapper->SdrView::BegDragObj(aMPos, nullptr, pHitSelectionHdl, nDrgLog, pDragMethod);
    }

    impl_SetMousePointer( rMEvt );
}

void ChartController::execute_MouseMove( const MouseEvent& rMEvt )
{
    SolarMutexGuard aGuard;

    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if(!m_pChartWindow || !pDrawViewWrapper)
        return;

    if( m_pDrawViewWrapper->IsTextEdit() )
    {
        if( m_pDrawViewWrapper->MouseMove(rMEvt,m_pChartWindow) )
            return;
    }

    if(pDrawViewWrapper->IsAction())
    {
        pDrawViewWrapper->MovAction( m_pChartWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    }

    impl_SetMousePointer( rMEvt );
}

void ChartController::execute_MouseButtonUp( const MouseEvent& rMEvt )
{
    ControllerLockGuardUNO aCLGuard( getModel() );
    bool bMouseUpWithoutMouseDown = !m_bWaitingForMouseUp;
    m_bWaitingForMouseUp = false;
    bool bNotifySelectionChange = false;
    {
        SolarMutexGuard aGuard;

        DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
        if(!m_pChartWindow || !pDrawViewWrapper)
            return;

        Point   aMPos   = m_pChartWindow->PixelToLogic(rMEvt.GetPosPixel());

        if(pDrawViewWrapper->IsTextEdit())
        {
            if( pDrawViewWrapper->MouseButtonUp(rMEvt,m_pChartWindow) )
                return;
        }

        // #i12587# support for shapes in chart
        if ( m_eDrawMode == CHARTDRAW_INSERT && pDrawViewWrapper->IsCreateObj() )
        {
            pDrawViewWrapper->EndCreateObj( SDRCREATE_FORCEEND );
            {
                HiddenUndoContext aUndoContext( m_xUndoManager );
                    // don't want the positioning Undo action to appear in the UI
                impl_switchDiagramPositioningToExcludingPositioning();
            }
            if ( pDrawViewWrapper->AreObjectsMarked() )
            {
                if ( pDrawViewWrapper->GetCurrentObjIdentifier() == OBJ_TEXT )
                {
                    executeDispatch_EditText();
                }
                else
                {
                    SdrObject* pObj = pDrawViewWrapper->getSelectedObject();
                    if ( pObj )
                    {
                        uno::Reference< drawing::XShape > xShape( pObj->getUnoShape(), uno::UNO_QUERY );
                        if ( xShape.is() )
                        {
                            m_aSelection.setSelection( xShape );
                            m_aSelection.applySelection( pDrawViewWrapper );
                        }
                    }
                }
            }
            else
            {
                m_aSelection.adaptSelectionToNewPos( aMPos, pDrawViewWrapper, rMEvt.IsRight(), m_bWaitingForDoubleClick );
                m_aSelection.applySelection( pDrawViewWrapper );
                setDrawMode( CHARTDRAW_SELECT );
            }
        }
        else if ( pDrawViewWrapper->IsDragObj() )
        {
            bool bDraggingDone = false;
            SdrDragMethod* pDragMethod = pDrawViewWrapper->SdrView::GetDragMethod();
            bool bIsMoveOnly = pDragMethod && pDragMethod->getMoveOnly();
            DragMethod_Base* pChartDragMethod = dynamic_cast< DragMethod_Base* >(pDragMethod);
            if( pChartDragMethod )
            {
                UndoGuard aUndoGuard( pChartDragMethod->getUndoDescription(),
                        m_xUndoManager );

                if( pDrawViewWrapper->EndDragObj() )
                {
                    bDraggingDone = true;
                    aUndoGuard.commit();
                }
            }

            if( !bDraggingDone && pDrawViewWrapper->EndDragObj() )
            {
                try
                {
                    //end move or size
                    SdrObject* pObj = pDrawViewWrapper->getSelectedObject();
                    if( pObj )
                    {
                        Rectangle aObjectRect = pObj->GetSnapRect();
                        awt::Size aPageSize( ChartModelHelper::getPageSize( getModel() ) );
                        Rectangle aPageRect( 0,0,aPageSize.Width,aPageSize.Height );

                        const E3dObject* pE3dObject = dynamic_cast< const E3dObject*>( pObj );
                        if( pE3dObject )
                            aObjectRect = pE3dObject->GetScene()->GetSnapRect();

                        ActionDescriptionProvider::ActionType eActionType(ActionDescriptionProvider::MOVE);
                        if( !bIsMoveOnly && m_aSelection.isResizeableObjectSelected() )
                            eActionType = ActionDescriptionProvider::RESIZE;

                        ObjectType eObjectType = ObjectIdentifier::getObjectType( m_aSelection.getSelectedCID() );

                        UndoGuard aUndoGuard(
                            ActionDescriptionProvider::createDescription( eActionType, ObjectNameProvider::getName( eObjectType)),
                            m_xUndoManager );

                        bool bChanged = false;
                        ChartModel* pModel = dynamic_cast<ChartModel*>(getModel().get());
                        if ( eObjectType == OBJECTTYPE_LEGEND )
                            bChanged = DiagramHelper::switchDiagramPositioningToExcludingPositioning( *pModel, false , true );

                        bool bMoved = PositionAndSizeHelper::moveObject( m_aSelection.getSelectedCID()
                                        , getModel()
                                        , awt::Rectangle(aObjectRect.getX(),aObjectRect.getY(),aObjectRect.getWidth(),aObjectRect.getHeight())
                                        , awt::Rectangle(aPageRect.getX(),aPageRect.getY(),aPageRect.getWidth(),aPageRect.getHeight()) );

                        if( bMoved || bChanged )
                        {
                            bDraggingDone = true;
                            aUndoGuard.commit();
                        }
                    }
                }
                catch( const uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }
                //all wanted model changes will take effect
                //and all unwanted view modifications are cleaned
            }

            if( !bDraggingDone ) //mouse wasn't moved while dragging
            {
                bool bClickedTwiceOnDragableObject = SelectionHelper::isDragableObjectHitTwice( aMPos, m_aSelection.getSelectedCID(), *pDrawViewWrapper );
                bool bIsRotateable = m_aSelection.isRotateableObjectSelected( getModel() );

                //toggle between move and rotate
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
        else if( isDoubleClick(rMEvt) && !bMouseUpWithoutMouseDown /*#i106966#*/ )
        {
            Point aMousePixel = rMEvt.GetPosPixel();
            execute_DoubleClick( &aMousePixel );
        }

        //@todo ForcePointer(&rMEvt);
        m_pChartWindow->ReleaseMouse();

        if( m_aSelection.isSelectionDifferentFromBeforeMouseDown() )
            bNotifySelectionChange = true;
    }

    impl_SetMousePointer( rMEvt );

    if(bNotifySelectionChange)
        impl_notifySelectionChangeListeners();
}

void ChartController::execute_DoubleClick( const Point* pMousePixel )
{
    bool bEditText = false;
    if ( m_aSelection.hasSelection() )
    {
        OUString aCID( m_aSelection.getSelectedCID() );
        if ( !aCID.isEmpty() )
        {
            ObjectType eObjectType = ObjectIdentifier::getObjectType( aCID );
            if ( OBJECTTYPE_TITLE == eObjectType )
            {
                bEditText = true;
            }
        }
        else
        {
            // #i12587# support for shapes in chart
            SdrObject* pObj = DrawViewWrapper::getSdrObject( m_aSelection.getSelectedAdditionalShape() );
            if ( dynamic_cast< const SdrTextObj* >(pObj) !=  nullptr )
            {
                bEditText = true;
            }
        }
    }

    if ( bEditText )
    {
        executeDispatch_EditText( pMousePixel );
    }
    else
    {
        executeDispatch_ObjectProperties();
    }
}

void ChartController::execute_Resize()
{
    SolarMutexGuard aGuard;
    if(m_pChartWindow)
        m_pChartWindow->Invalidate();
}

void ChartController::execute_Command( const CommandEvent& rCEvt )
{
    bool bIsAction = false;
    {
        SolarMutexGuard aGuard;
        DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
        if(!m_pChartWindow || !pDrawViewWrapper)
            return;
        bIsAction = m_pDrawViewWrapper->IsAction();
    }

    // pop-up menu
    if(rCEvt.GetCommand() == CommandEventId::ContextMenu && !bIsAction)
    {
        {
            SolarMutexGuard aGuard;
            if(m_pChartWindow)
                m_pChartWindow->ReleaseMouse();
        }

        if( m_aSelection.isSelectionDifferentFromBeforeMouseDown() )
            impl_notifySelectionChangeListeners();

        css::uno::Reference< css::awt::XPopupMenu > xPopupMenu( m_xCC->getServiceManager()->createInstanceWithContext(
            "com.sun.star.awt.PopupMenu", m_xCC ), css::uno::UNO_QUERY );

        Point aPos( rCEvt.GetMousePosPixel() );
        if( !rCEvt.IsMouseEvent() )
        {
            SolarMutexGuard aGuard;
            if(m_pChartWindow)
                aPos = m_pChartWindow->GetPointerState().maPos;
        }

        if ( isShapeContext() )
        {
            // #i12587# support for shapes in chart
            OUString aMenuName = m_pDrawViewWrapper->IsTextEdit() ? OUString( "drawtext" ) : OUString( "draw" );
            css::uno::Sequence< css::uno::Any > aArgs( 3 );
            aArgs[0] <<= comphelper::makePropertyValue( "Value", aMenuName );
            aArgs[1] <<= comphelper::makePropertyValue( "Frame", m_xFrame );
            aArgs[2] <<= comphelper::makePropertyValue( "IsContextMenu", true );

            css::uno::Reference< css::frame::XPopupMenuController > xPopupController(
                m_xCC->getServiceManager()->createInstanceWithArgumentsAndContext(
                "com.sun.star.comp.framework.ResourceMenuController", aArgs, m_xCC ), css::uno::UNO_QUERY );

            if ( !xPopupController.is() || !xPopupMenu.is() )
                return;

            xPopupController->setPopupMenu( xPopupMenu );
            xPopupMenu->execute( css::uno::Reference< css::awt::XWindowPeer >( m_xFrame->getContainerWindow(), css::uno::UNO_QUERY ),
                                 css::awt::Rectangle( aPos.X(), aPos.Y(), 0, 0 ),
                                 css::awt::PopupMenuDirection::EXECUTE_DEFAULT );

            css::uno::Reference< css::lang::XComponent > xComponent( xPopupController, css::uno::UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        else
        {
            // todo: the context menu should be specified by an xml file in uiconfig
            if( xPopupMenu.is())
            {
                sal_Int16 nUniqueId = 1;
                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:Cut" );
                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:Copy" );
                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:Paste" );
                xPopupMenu->insertSeparator( -1 );

                ObjectType eObjectType = ObjectIdentifier::getObjectType( m_aSelection.getSelectedCID() );
                Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( getModel() );

                OUString aFormatCommand( lcl_getFormatCommandForObjectCID( m_aSelection.getSelectedCID() ) );
                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, aFormatCommand );

                //some commands for dataseries and points:

                if( OBJECTTYPE_DATA_SERIES == eObjectType || OBJECTTYPE_DATA_POINT == eObjectType )
                {
                    bool bIsPoint = ( OBJECTTYPE_DATA_POINT == eObjectType );
                    uno::Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( m_aSelection.getSelectedCID(), getModel() );
                    uno::Reference< chart2::XRegressionCurveContainer > xCurveCnt( xSeries, uno::UNO_QUERY );
                    Reference< chart2::XRegressionCurve > xTrendline( RegressionCurveHelper::getFirstCurveNotMeanValueLine( xCurveCnt ) );
                    bool bHasEquation = RegressionCurveHelper::hasEquation( xTrendline );
                    Reference< chart2::XRegressionCurve > xMeanValue( RegressionCurveHelper::getMeanValueLine( xCurveCnt ) );
                    bool bHasYErrorBars = StatisticsHelper::hasErrorBars( xSeries );
                    bool bHasXErrorBars = StatisticsHelper::hasErrorBars( xSeries, false );
                    bool bHasDataLabelsAtSeries = DataSeriesHelper::hasDataLabelsAtSeries( xSeries );
                    bool bHasDataLabelsAtPoints = DataSeriesHelper::hasDataLabelsAtPoints( xSeries );
                    bool bHasDataLabelAtPoint = false;
                    sal_Int32 nPointIndex = -1;
                    if( bIsPoint )
                    {
                        nPointIndex = ObjectIdentifier::getIndexFromParticleOrCID( m_aSelection.getSelectedCID() );
                        bHasDataLabelAtPoint = DataSeriesHelper::hasDataLabelAtPoint( xSeries, nPointIndex );
                    }
                    bool bSelectedPointIsFormatted = false;
                    bool bHasFormattedDataPointsOtherThanSelected = false;

                    Reference< beans::XPropertySet > xSeriesProperties( xSeries, uno::UNO_QUERY );
                    if( xSeriesProperties.is() )
                    {
                        uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
                        if( xSeriesProperties->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
                        {
                            if( aAttributedDataPointIndexList.hasElements() )
                            {
                                if( bIsPoint )
                                {
                                    ::std::vector< sal_Int32 > aIndices( ContainerHelper::SequenceToVector( aAttributedDataPointIndexList ) );
                                    ::std::vector< sal_Int32 >::iterator aIt = ::std::find( aIndices.begin(), aIndices.end(), nPointIndex );
                                    if( aIt != aIndices.end())
                                        bSelectedPointIsFormatted = true;
                                    else
                                        bHasFormattedDataPointsOtherThanSelected = true;
                                }
                                else
                                    bHasFormattedDataPointsOtherThanSelected = true;
                            }
                        }
                    }

                    if( bIsPoint )
                    {
                        if( bHasDataLabelAtPoint )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatDataLabel" );
                        if( !bHasDataLabelAtPoint )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertDataLabel" );
                        else
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteDataLabel" );
                        if( bSelectedPointIsFormatted )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:ResetDataPoint" );

                        xPopupMenu->insertSeparator( -1 );

                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatDataSeries" );
                    }

                    Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeOfSeries( xDiagram, xSeries ) );
                    if( xChartType->getChartType() == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
                    {
                        try
                        {
                            Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY );
                            if( xChartTypeProp.is() )
                            {
                                bool bJapaneseStyle = false;
                                xChartTypeProp->getPropertyValue( "Japanese" ) >>= bJapaneseStyle;

                                if( bJapaneseStyle )
                                {
                                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatStockLoss" );
                                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatStockGain" );
                                }
                            }
                        }
                        catch( const uno::Exception & ex )
                        {
                            ASSERT_EXCEPTION( ex );
                        }
                    }

                    if( bHasDataLabelsAtSeries )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatDataLabels" );
                    if( bHasEquation )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatTrendlineEquation" );
                    if( xMeanValue.is() )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatMeanValue" );
                    if( bHasXErrorBars )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatXErrorBars" );
                    if( bHasYErrorBars )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatYErrorBars" );

                    xPopupMenu->insertSeparator( -1 );

                    if( !bHasDataLabelsAtSeries )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertDataLabels" );

                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertTrendline" );

                    if( !xMeanValue.is() )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertMeanValue" );
                    if( !bHasXErrorBars )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertXErrorBars" );
                    if( !bHasYErrorBars )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertYErrorBars" );
                    if( bHasDataLabelsAtSeries || ( bHasDataLabelsAtPoints && bHasFormattedDataPointsOtherThanSelected ) )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteDataLabels" );
                    if( bHasEquation )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteTrendlineEquation" );
                    if( xMeanValue.is() )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteMeanValue" );
                    if( bHasXErrorBars )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteXErrorBars" );
                    if( bHasYErrorBars )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteYErrorBars" );

                    if( bHasFormattedDataPointsOtherThanSelected )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:ResetAllDataPoints" );

                    xPopupMenu->insertSeparator( -1 );

                    lcl_insertMenuCommand( xPopupMenu, nUniqueId, ".uno:ArrangeRow" );
                    uno::Reference< awt::XPopupMenu > xArrangePopupMenu(
                        m_xCC->getServiceManager()->createInstanceWithContext(
                            "com.sun.star.awt.PopupMenu", m_xCC ), uno::UNO_QUERY );
                    if( xArrangePopupMenu.is() )
                    {
                        sal_Int16 nSubId = nUniqueId + 1;
                        lcl_insertMenuCommand( xArrangePopupMenu, nSubId++, ".uno:Forward" );
                        lcl_insertMenuCommand( xArrangePopupMenu, nSubId, ".uno:Backward" );
                        xPopupMenu->setPopupMenu( nUniqueId, xArrangePopupMenu );
                        nUniqueId = nSubId;
                    }
                    ++nUniqueId;
                }
                else if( OBJECTTYPE_DATA_CURVE == eObjectType )
                {
                    lcl_insertMenuCommand( xPopupMenu,  nUniqueId++, ".uno:DeleteTrendline" );
                    lcl_insertMenuCommand( xPopupMenu,  nUniqueId++, ".uno:FormatTrendlineEquation" );
                    lcl_insertMenuCommand( xPopupMenu,  nUniqueId++, ".uno:InsertTrendlineEquation" );
                    lcl_insertMenuCommand( xPopupMenu,  nUniqueId++, ".uno:InsertTrendlineEquationAndR2" );
                    lcl_insertMenuCommand( xPopupMenu,  nUniqueId++, ".uno:InsertR2Value" );
                    lcl_insertMenuCommand( xPopupMenu,  nUniqueId++, ".uno:DeleteTrendlineEquation" );
                    lcl_insertMenuCommand( xPopupMenu,  nUniqueId++, ".uno:DeleteR2Value" );
                }
                else if( OBJECTTYPE_DATA_CURVE_EQUATION == eObjectType )
                {
                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertR2Value" );
                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteR2Value" );
                }

                //some commands for axes: and grids

                else if( OBJECTTYPE_AXIS  == eObjectType || OBJECTTYPE_GRID == eObjectType || OBJECTTYPE_SUBGRID == eObjectType )
                {
                    Reference< XAxis > xAxis = ObjectIdentifier::getAxisForCID( m_aSelection.getSelectedCID(), getModel() );
                    if( xAxis.is() && xDiagram.is() )
                    {
                        sal_Int32 nDimensionIndex = -1;
                        sal_Int32 nCooSysIndex = -1;
                        sal_Int32 nAxisIndex = -1;
                        AxisHelper::getIndicesForAxis( xAxis, xDiagram, nCooSysIndex, nDimensionIndex, nAxisIndex );
                        bool bIsSecondaryAxis = nAxisIndex!=0;
                        bool bIsAxisVisible = AxisHelper::isAxisVisible( xAxis );
                        bool bIsMajorGridVisible = AxisHelper::isGridShown( nDimensionIndex, nCooSysIndex, true /*bMainGrid*/, xDiagram );
                        bool bIsMinorGridVisible = AxisHelper::isGridShown( nDimensionIndex, nCooSysIndex, false /*bMainGrid*/, xDiagram );
                        bool bHasTitle = false;
                        uno::Reference< XTitled > xTitled( xAxis, uno::UNO_QUERY );
                        if( xTitled.is())
                            bHasTitle = !TitleHelper::getCompleteString( xTitled->getTitleObject() ).isEmpty();

                        if( OBJECTTYPE_AXIS  != eObjectType && bIsAxisVisible )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatAxis" );
                        if( OBJECTTYPE_GRID != eObjectType && bIsMajorGridVisible && !bIsSecondaryAxis )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatMajorGrid" );
                        if( OBJECTTYPE_SUBGRID != eObjectType && bIsMinorGridVisible && !bIsSecondaryAxis )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatMinorGrid" );

                        xPopupMenu->insertSeparator( -1 );

                        if( OBJECTTYPE_AXIS  != eObjectType && !bIsAxisVisible )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertAxis" );
                        if( OBJECTTYPE_GRID != eObjectType && !bIsMajorGridVisible && !bIsSecondaryAxis )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertMajorGrid" );
                        if( OBJECTTYPE_SUBGRID != eObjectType && !bIsMinorGridVisible && !bIsSecondaryAxis )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertMinorGrid" );
                        if( !bHasTitle )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertAxisTitle" );

                        if( bIsAxisVisible )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteAxis" );
                        if( bIsMajorGridVisible && !bIsSecondaryAxis )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteMajorGrid" );
                        if( bIsMinorGridVisible && !bIsSecondaryAxis )
                            lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteMinorGrid" );
                    }
                }

                if( OBJECTTYPE_DATA_STOCK_LOSS == eObjectType )
                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatStockGain" );
                else if( OBJECTTYPE_DATA_STOCK_GAIN == eObjectType )
                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:FormatStockLoss" );

                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:TransformDialog" );

                if( OBJECTTYPE_PAGE == eObjectType || OBJECTTYPE_DIAGRAM == eObjectType
                    || OBJECTTYPE_DIAGRAM_WALL == eObjectType
                    || OBJECTTYPE_DIAGRAM_FLOOR == eObjectType
                    || OBJECTTYPE_UNKNOWN == eObjectType )
                {
                    if( OBJECTTYPE_UNKNOWN != eObjectType )
                        xPopupMenu->insertSeparator( -1 );
                    bool bHasLegend = LegendHelper::hasLegend( xDiagram );
                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertTitles" );
                    if( !bHasLegend )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertLegend" );
                    lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:InsertRemoveAxes" );
                    if( bHasLegend )
                        lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DeleteLegend" );
                }

                xPopupMenu->insertSeparator( -1 );
                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DiagramType" );
                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DataRanges" );
                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:DiagramData" );
                lcl_insertMenuCommand( xPopupMenu, nUniqueId++, ".uno:View3D" );

                ::svt::ContextMenuHelper aContextMenuHelper( m_xFrame );
                aContextMenuHelper.completeAndExecute( aPos, xPopupMenu );
            }
        }
    }
    else if( ( rCEvt.GetCommand() == CommandEventId::StartExtTextInput ) ||
             ( rCEvt.GetCommand() == CommandEventId::ExtTextInput ) ||
             ( rCEvt.GetCommand() == CommandEventId::EndExtTextInput ) ||
             ( rCEvt.GetCommand() == CommandEventId::InputContextChange ) )
    {
        //#i84417# enable editing with IME
        SolarMutexGuard aGuard;
        if( m_pDrawViewWrapper )
            m_pDrawViewWrapper->Command( rCEvt, m_pChartWindow );
    }
}

bool ChartController::execute_KeyInput( const KeyEvent& rKEvt )
{
    bool bReturn=false;

    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if(!m_pChartWindow || !pDrawViewWrapper)
        return bReturn;

    // handle accelerators
    if( ! m_apAccelExecute.get() && m_xFrame.is() && m_xCC.is() )
    {
        m_apAccelExecute = ::svt::AcceleratorExecute::createAcceleratorHelper();
        OSL_ASSERT( m_apAccelExecute.get());
        if( m_apAccelExecute.get() )
            m_apAccelExecute->init( m_xCC, m_xFrame );
    }

    vcl::KeyCode aKeyCode( rKEvt.GetKeyCode());
    sal_uInt16 nCode = aKeyCode.GetCode();
    bool bAlternate = aKeyCode.IsMod2();
    bool bCtrl = aKeyCode.IsMod1();

    if( m_apAccelExecute.get() )
        bReturn = m_apAccelExecute->execute( aKeyCode );
    if( bReturn )
        return bReturn;

    {
        SolarMutexGuard aGuard;
        if( pDrawViewWrapper->IsTextEdit() )
        {
            if( pDrawViewWrapper->KeyInput(rKEvt,m_pChartWindow) )
            {
                bReturn = true;
                if( nCode == KEY_ESCAPE )
                {
                    this->EndTextEdit();
                }
            }
        }
    }

    // keyboard accessibility
    ObjectType eObjectType = ObjectIdentifier::getObjectType( m_aSelection.getSelectedCID() );
    if( ! bReturn )
    {
        // Natvigation (Tab/F3/Home/End)
        uno::Reference< XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
        ObjectKeyNavigation aObjNav( m_aSelection.getSelectedOID(), xChartDoc, ExplicitValueProvider::getExplicitValueProvider( m_xChartView ));
        awt::KeyEvent aKeyEvent( ::svt::AcceleratorExecute::st_VCLKey2AWTKey( aKeyCode ));
        bReturn = aObjNav.handleKeyEvent( aKeyEvent );
        if( bReturn )
        {
            ObjectIdentifier aNewOID = aObjNav.getCurrentSelection();
            uno::Any aNewSelection;
            if ( aNewOID.isValid() && !ObjectHierarchy::isRootNode( aNewOID ) )
            {
                aNewSelection = aNewOID.getAny();
            }
            if ( m_eDragMode == SDRDRAG_ROTATE && !SelectionHelper::isRotateableObject( aNewOID.getObjectCID(), getModel() ) )
            {
                m_eDragMode = SDRDRAG_MOVE;
            }
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
                OUString aParameter( ObjectIdentifier::getDragParameterString( m_aSelection.getSelectedCID() ));
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
                    if( bAlternate && m_pChartWindow )
                    {
                        // together with Alt-key: 1 px in each direction
                        SolarMutexGuard aGuard;
                        if( m_pChartWindow )
                        {
                            Size aPixelSize = m_pChartWindow->PixelToLogic( Size( 2, 2 ));
                            fGrowAmountX = static_cast< double >( aPixelSize.Width());
                            fGrowAmountY = static_cast< double >( aPixelSize.Height());
                        }
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
                    if( bAlternate && m_pChartWindow )
                    {
                        // together with Alt-key: 1 px
                        SolarMutexGuard aGuard;
                        if(m_pChartWindow)
                        {
                            Size aPixelSize = m_pChartWindow->PixelToLogic( Size( 1, 1 ));
                            fShiftAmountX = static_cast< double >( aPixelSize.Width());
                            fShiftAmountY = static_cast< double >( aPixelSize.Height());
                        }
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
                    if( !m_aSelection.getSelectedCID().isEmpty() )
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
                            awt::Size aPageSize( ChartModelHelper::getPageSize( getModel() ) );
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

    // dumping the shape
    if( !bReturn && bCtrl && nCode == KEY_F12)
    {
        uno::Reference< qa::XDumper > xChartModel( getModel(), uno::UNO_QUERY );
        if(xChartModel.is())
        {
            OUString aDump = xChartModel->dump();
            SAL_WARN("chart2", aDump);
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
        uno::Reference< frame::XDispatchHelper > xDispatchHelper( frame::DispatchHelper::create(m_xCC) );
        uno::Sequence< beans::PropertyValue > aArgs;
        xDispatchHelper->executeDispatch(
            uno::Reference< frame::XDispatchProvider >( m_xFrame, uno::UNO_QUERY ),
            ".uno:TerminateInplaceActivation",
            "_parent",
            frame::FrameSearchFlag::PARENT,
            aArgs );
        bReturn = true;
    }

    if( ! bReturn &&
        (nCode == KEY_DELETE || nCode == KEY_BACKSPACE ))
    {
        bReturn = executeDispatch_Delete();
        if( ! bReturn )
        {
            SolarMutexGuard aGuard;
            ScopedVclPtrInstance<InfoBox>(m_pChartWindow, SCH_RESSTR(STR_ACTION_NOTPOSSIBLE))->Execute();
        }
    }

    return bReturn;
}

bool ChartController::requestQuickHelp(
    ::Point aAtLogicPosition,
    bool bIsBalloonHelp,
    OUString & rOutQuickHelpText,
    awt::Rectangle & rOutEqualRect )
{
    uno::Reference< frame::XModel > xChartModel;
    if( m_aModel.is())
        xChartModel.set( getModel() );
    if( !xChartModel.is())
        return false;

    // help text
    OUString aCID;
    if( m_pDrawViewWrapper )
    {
        aCID = SelectionHelper::getHitObjectCID(
            aAtLogicPosition, *m_pDrawViewWrapper );
    }
    bool bResult( !aCID.isEmpty());

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

// XSelectionSupplier (optional interface)
sal_Bool SAL_CALL ChartController::select( const uno::Any& rSelection )
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    bool bSuccess = false;

    if ( rSelection.hasValue() )
    {
        const uno::Type& rType = rSelection.getValueType();
        if ( rType == cppu::UnoType< OUString >::get() )
        {
            OUString aNewCID;
            if ( ( rSelection >>= aNewCID ) && m_aSelection.setSelection( aNewCID ) )
            {
                bSuccess = true;
            }
        }
        else if ( rType == cppu::UnoType<drawing::XShape>::get() )
        {
            uno::Reference< drawing::XShape > xShape;
            if ( ( rSelection >>= xShape ) && m_aSelection.setSelection( xShape ) )
            {
                bSuccess = true;
            }
        }
    }
    else
    {
        if ( m_aSelection.hasSelection() )
        {
            m_aSelection.clearSelection();
            bSuccess = true;
        }
    }

    if ( bSuccess )
    {
        SolarMutexGuard aGuard;
        if ( m_pDrawViewWrapper && m_pDrawViewWrapper->IsTextEdit() )
        {
            this->EndTextEdit();
        }
        this->impl_selectObjectAndNotiy();
        if ( m_pChartWindow )
        {
            m_pChartWindow->Invalidate();
        }
        return true;
    }

    return false;
}

uno::Any SAL_CALL ChartController::getSelection()
    throw(uno::RuntimeException, std::exception)
{
    uno::Any aReturn;
    if ( m_aSelection.hasSelection() )
    {
        OUString aCID( m_aSelection.getSelectedCID() );
        if ( !aCID.isEmpty() )
        {
            aReturn = uno::makeAny( aCID );
        }
        else
        {
            // #i12587# support for shapes in chart
            aReturn = uno::makeAny( m_aSelection.getSelectedAdditionalShape() );
        }
    }
    return aReturn;
}

void SAL_CALL ChartController::addSelectionChangeListener( const uno::Reference<view::XSelectionChangeListener> & xListener )
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( impl_isDisposedOrSuspended() )//@todo? allow adding of listeners in suspend mode?
        return; //behave passive if already disposed or suspended

    //--add listener
    m_aLifeTimeManager.m_aListenerContainer.addInterface( cppu::UnoType<view::XSelectionChangeListener>::get(), xListener );
}

void SAL_CALL ChartController::removeSelectionChangeListener( const uno::Reference<view::XSelectionChangeListener> & xListener )
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( impl_isDisposedOrSuspended() ) //@todo? allow removing of listeners in suspend mode?
        return; //behave passive if already disposed or suspended

    //--remove listener
    m_aLifeTimeManager.m_aListenerContainer.removeInterface( cppu::UnoType<view::XSelectionChangeListener>::get(), xListener );
}

void ChartController::impl_notifySelectionChangeListeners()
{
    ::cppu::OInterfaceContainerHelper* pIC = m_aLifeTimeManager.m_aListenerContainer
        .getContainer( cppu::UnoType<view::XSelectionChangeListener>::get() );
    if( pIC )
    {
        uno::Reference< view::XSelectionSupplier > xSelectionSupplier(this);
        lang::EventObject aEvent( xSelectionSupplier );
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            uno::Reference< view::XSelectionChangeListener > xListener( aIt.next(), uno::UNO_QUERY );
            if( xListener.is() )
                xListener->selectionChanged( aEvent );
        }
    }
}

void ChartController::impl_selectObjectAndNotiy()
{
    {
        SolarMutexGuard aGuard;
        DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
        if( pDrawViewWrapper )
        {
            pDrawViewWrapper->SetDragMode( m_eDragMode );
            m_aSelection.applySelection( m_pDrawViewWrapper );
        }
    }
    impl_notifySelectionChangeListeners();
}

bool ChartController::impl_moveOrResizeObject(
    const OUString & rCID,
    eMoveOrResizeType eType,
    double fAmountLogicX,
    double fAmountLogicY )
{
    bool bResult = false;
    bool bNeedResize = ( eType == CENTERED_RESIZE_OBJECT );

    uno::Reference< frame::XModel > xChartModel( getModel() );
    uno::Reference< beans::XPropertySet > xObjProp(
        ObjectIdentifier::getObjectPropertySet( rCID, xChartModel ));
    if( xObjProp.is())
    {
        awt::Size aRefSize = ChartModelHelper::getPageSize( xChartModel );

        chart2::RelativePosition aRelPos;
        chart2::RelativeSize     aRelSize;
        bool bDeterminePos  = !(xObjProp->getPropertyValue( "RelativePosition") >>= aRelPos);
        bool bDetermineSize = !bNeedResize || !(xObjProp->getPropertyValue( "RelativeSize") >>= aRelSize);

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

            ObjectType eObjectType = ObjectIdentifier::getObjectType( rCID );
            UndoGuard aUndoGuard( ActionDescriptionProvider::createDescription(
                    eActionType, ObjectNameProvider::getName( eObjectType )), m_xUndoManager );
            {
                ControllerLockGuardUNO aCLGuard( xChartModel );
                xObjProp->setPropertyValue( "RelativePosition", uno::makeAny( aRelPos ));
                if( bNeedResize || (eObjectType == OBJECTTYPE_DIAGRAM) )//Also set an explicit size at the diagram when an explicit position is set
                    xObjProp->setPropertyValue( "RelativeSize", uno::makeAny( aRelSize ));
            }
            aUndoGuard.commit();
        }
    }
    return bResult;
}

bool ChartController::impl_DragDataPoint( const OUString & rCID, double fAdditionalOffset )
{
    bool bResult = false;
    if( fAdditionalOffset < -1.0 || fAdditionalOffset > 1.0 || fAdditionalOffset == 0.0 )
        return bResult;

    sal_Int32 nDataPointIndex = ObjectIdentifier::getIndexFromParticleOrCID( rCID );
    uno::Reference< chart2::XDataSeries > xSeries(
        ObjectIdentifier::getDataSeriesForCID( rCID, getModel() ));
    if( xSeries.is())
    {
        try
        {
            uno::Reference< beans::XPropertySet > xPointProp( xSeries->getDataPointByIndex( nDataPointIndex ));
            double fOffset = 0.0;
            if( xPointProp.is() &&
                (xPointProp->getPropertyValue( "Offset" ) >>= fOffset ) &&
                (( fAdditionalOffset > 0.0 && fOffset < 1.0 ) || (fOffset > 0.0)) )
            {
                fOffset += fAdditionalOffset;
                if( fOffset > 1.0 )
                    fOffset = 1.0;
                else if( fOffset < 0.0 )
                    fOffset = 0.0;
                xPointProp->setPropertyValue( "Offset", uno::makeAny( fOffset ));
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
    SolarMutexGuard aGuard;

    if (!m_pDrawViewWrapper || !m_pChartWindow)
        return;

    Point aMousePos( m_pChartWindow->PixelToLogic( rEvent.GetPosPixel()));
    sal_uInt16 nModifier = rEvent.GetModifier();
    bool bLeftDown = rEvent.IsLeft();

    if ( m_pDrawViewWrapper->IsTextEdit() )
    {
        if( m_pDrawViewWrapper->IsTextEditHit( aMousePos ) )
        {
            m_pChartWindow->SetPointer( m_pDrawViewWrapper->GetPreferredPointer(
                aMousePos, m_pChartWindow, nModifier, bLeftDown ) );
            return;
        }
    }
    else if( m_pDrawViewWrapper->IsAction() )
    {
        return;//don't change pointer during running action
    }

    SdrHdl* pHitSelectionHdl = nullptr;
    if( m_aSelection.isResizeableObjectSelected() )
        pHitSelectionHdl = m_pDrawViewWrapper->PickHandle( aMousePos );

    if( pHitSelectionHdl )
    {
        Pointer aPointer = m_pDrawViewWrapper->GetPreferredPointer(
            aMousePos, m_pChartWindow, nModifier, bLeftDown );
        bool bForceArrowPointer = false;

        ObjectIdentifier aOID( m_aSelection.getSelectedOID() );

        switch( aPointer.GetStyle())
        {
            case PointerStyle::NSize:
            case PointerStyle::SSize:
            case PointerStyle::WSize:
            case PointerStyle::ESize:
            case PointerStyle::NWSize:
            case PointerStyle::NESize:
            case PointerStyle::SWSize:
            case PointerStyle::SESize:
                if( ! m_aSelection.isResizeableObjectSelected() )
                    bForceArrowPointer = true;
                break;
            case PointerStyle::Move:
                if ( !aOID.isDragableObject() )
                    bForceArrowPointer = true;
                break;
            case PointerStyle::MovePoint:
            case PointerStyle::MoveBezierWeight:
                // there is no point-editing in a chart
                // the PointerStyle::MoveBezierWeight appears in 3d data points
                bForceArrowPointer = true;
                break;
            default:
                break;
        }

        if( bForceArrowPointer )
            m_pChartWindow->SetPointer( Pointer( PointerStyle::Arrow ));
        else
            m_pChartWindow->SetPointer( aPointer );

        return;
    }

    // #i12587# support for shapes in chart
    if ( m_eDrawMode == CHARTDRAW_INSERT &&
         ( !m_pDrawViewWrapper->IsMarkedHit( aMousePos ) || !m_aSelection.isDragableObjectSelected() ) )
    {
        PointerStyle ePointerStyle = PointerStyle::DrawRect;
        SdrObjKind eKind = static_cast< SdrObjKind >( m_pDrawViewWrapper->GetCurrentObjIdentifier() );
        switch ( eKind )
        {
            case OBJ_LINE:
                {
                    ePointerStyle = PointerStyle::DrawLine;
                }
                break;
            case OBJ_RECT:
            case OBJ_CUSTOMSHAPE:
                {
                    ePointerStyle = PointerStyle::DrawRect;
                }
                break;
            case OBJ_CIRC:
                {
                    ePointerStyle = PointerStyle::DrawEllipse;
                }
                break;
            case OBJ_FREELINE:
                {
                    ePointerStyle = PointerStyle::DrawPolygon;
                }
                break;
            case OBJ_TEXT:
                {
                    ePointerStyle = PointerStyle::DrawText;
                }
                break;
            case OBJ_CAPTION:
                {
                    ePointerStyle = PointerStyle::DrawCaption;
                }
                break;
            default:
                {
                    ePointerStyle = PointerStyle::DrawRect;
                }
                break;
        }
        m_pChartWindow->SetPointer( Pointer( ePointerStyle ) );
        return;
    }

    OUString aHitObjectCID(
        SelectionHelper::getHitObjectCID(
            aMousePos, *m_pDrawViewWrapper, true /*bGetDiagramInsteadOf_Wall*/ ));

    if( m_pDrawViewWrapper->IsTextEdit() )
    {
        if( aHitObjectCID.equals(m_aSelection.getSelectedCID()) )
        {
            m_pChartWindow->SetPointer( Pointer( PointerStyle::Arrow ));
            return;
        }
    }

    if( aHitObjectCID.isEmpty() )
    {
        //additional shape was hit
        m_pChartWindow->SetPointer( PointerStyle::Move );
    }
    else if( ObjectIdentifier::isDragableObject( aHitObjectCID ) )
    {
        if( (m_eDragMode == SDRDRAG_ROTATE)
            && SelectionHelper::isRotateableObject( aHitObjectCID
                , getModel() ) )
            m_pChartWindow->SetPointer( Pointer( PointerStyle::Rotate ) );
        else
        {
            ObjectType eHitObjectType = ObjectIdentifier::getObjectType( aHitObjectCID );
            if( eHitObjectType == OBJECTTYPE_DATA_POINT )
            {
                if( !ObjectIdentifier::areSiblings(aHitObjectCID,m_aSelection.getSelectedCID())
                    && !ObjectIdentifier::areIdenticalObjects(aHitObjectCID,m_aSelection.getSelectedCID()) )
                {
                    m_pChartWindow->SetPointer( Pointer( PointerStyle::Arrow ));
                    return;
                }
            }
            m_pChartWindow->SetPointer( PointerStyle::Move );
        }
    }
    else
        m_pChartWindow->SetPointer( Pointer( PointerStyle::Arrow ));
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
