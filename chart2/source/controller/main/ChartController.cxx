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

#include <memory>
#include <sal/config.h>

#include <config_wasm_strip.h>
#include <ChartController.hxx>
#include <ChartView.hxx>
#include <servicenames.hxx>
#include <ResId.hxx>
#include <dlg_DataSource.hxx>
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <ChartType.hxx>
#include "ControllerCommandDispatch.hxx"
#include <DataSeries.hxx>
#include <Diagram.hxx>
#include <strings.hrc>
#include <ChartViewHelper.hxx>

#include <ChartWindow.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <DrawViewWrapper.hxx>
#include <ObjectIdentifier.hxx>
#include <ControllerLockGuard.hxx>
#include "UndoGuard.hxx"
#include "ChartDropTargetHelper.hxx"

#include <dlg_ChartType.hxx>
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
#include <AccessibleChartView.hxx>
#endif
#include "DrawCommandDispatch.hxx"
#include "ShapeController.hxx"
#include "UndoActions.hxx"
#include <ViewElementListProvider.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <BaseCoordinateSystem.hxx>

#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/frame/XLayoutManagerEventBroadcaster.hpp>
#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/chart2/XDataProviderAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <svx/sidebar/SelectionChangeHandler.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <osl/mutex.hxx>
#include <comphelper/lok.hxx>

#include <sfx2/sidebar/SidebarController.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

// this is needed to properly destroy the unique_ptr to the AcceleratorExecute
// object in the DTOR
#include <svtools/acceleratorexecute.hxx>
#include <svx/ActionDescriptionProvider.hxx>
#include <comphelper/diagnose_ex.hxx>

// enable the following define to let the controller listen to model changes and
// react on this by rebuilding the view
#define TEST_ENABLE_MODIFY_LISTENER

namespace chart
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

ChartController::ChartController(uno::Reference<uno::XComponentContext> xContext) :
    m_aLifeTimeManager( nullptr ),
    m_bSuspended( false ),
    m_xCC(std::move(xContext)),
    m_aModel( nullptr, m_aModelMutex ),
    m_eDragMode(SdrDragMode::Move),
    m_aDoubleClickTimer("chart2 ChartController m_aDoubleClickTimer"),
    m_bWaitingForDoubleClick(false),
    m_bWaitingForMouseUp(false),
    m_bFieldButtonDown(false),
    m_bConnectingToView(false),
    m_bDisposed(false),
    m_aDispatchContainer( m_xCC ),
    m_eDrawMode( CHARTDRAW_SELECT ),
    mpSelectionChangeHandler(new svx::sidebar::SelectionChangeHandler(
            [this]() { return this->GetContextName(); },
                this, vcl::EnumContext::Context::Cell))
{
    m_aDoubleClickTimer.SetInvokeHandler( LINK( this, ChartController, DoubleClickWaitingHdl ) );
}

ChartController::~ChartController()
{
    stopDoubleClickWaiting();
}

ChartController::TheModel::TheModel( rtl::Reference<::chart::ChartModel> xModel ) :
    m_xModel(std::move( xModel )),
    m_bOwnership( true )
{
}

ChartController::TheModel::~TheModel()
{
}

void ChartController::TheModel::addListener( ChartController* pController )
{
    if(m_xModel)
    {
        //if you need to be able to veto against the destruction of the model
        // you must add as a close listener

        //otherwise you 'can' add as closelistener or 'must' add as dispose event listener

        m_xModel->addCloseListener(
            static_cast<util::XCloseListener*>(pController) );
    }
}

void ChartController::TheModel::removeListener(  ChartController* pController )
{
    if(m_xModel)
        m_xModel->removeCloseListener(
            static_cast<util::XCloseListener*>(pController) );
}

void ChartController::TheModel::tryTermination()
{
    if(!m_bOwnership)
        return;

    try
    {
        if(m_xModel.is())
        {
            try
            {
                //@todo ? are we allowed to use sal_True here if we have the explicit ownership?
                //I think yes, because there might be other CloseListeners later in the list which might be interested still
                //but make sure that we do not throw the CloseVetoException here ourselves
                //so stop listening before trying to terminate or check the source of queryclosing event
                m_xModel->close(true);

                m_bOwnership                = false;
            }
            catch( const util::CloseVetoException& )
            {
                //since we have indicated to give up the ownership with parameter true in close call
                //the one who has thrown the CloseVetoException is the new owner

                SAL_WARN_IF( m_bOwnership, "chart2.main", "a well known owner has caught a CloseVetoException after calling close(true)");
                m_bOwnership                = false;
                return;
            }

        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION( "chart2", "Termination of model failed" );
    }
}

ChartController::TheModelRef::TheModelRef( TheModel* pTheModel, osl::Mutex& rMutex ) :
    m_rModelMutex(rMutex)
{
    osl::Guard< osl::Mutex > aGuard( m_rModelMutex );
    m_xTheModel = pTheModel;
}
ChartController::TheModelRef::TheModelRef( const TheModelRef& rTheModel, ::osl::Mutex& rMutex ) :
    m_rModelMutex(rMutex)
{
    osl::Guard< osl::Mutex > aGuard( m_rModelMutex );
    m_xTheModel = rTheModel.m_xTheModel;
}
ChartController::TheModelRef& ChartController::TheModelRef::operator=(TheModel* pTheModel)
{
    osl::Guard< osl::Mutex > aGuard( m_rModelMutex );
    m_xTheModel = pTheModel;
    return *this;
}
ChartController::TheModelRef& ChartController::TheModelRef::operator=(const TheModelRef& rTheModel)
{
    osl::Guard< osl::Mutex > aGuard( m_rModelMutex );
    m_xTheModel = rTheModel.operator->();
    return *this;
}
ChartController::TheModelRef::~TheModelRef()
{
    osl::Guard< osl::Mutex > aGuard( m_rModelMutex );
    m_xTheModel.clear();
}
bool ChartController::TheModelRef::is() const
{
    return m_xTheModel.is();
}

namespace {

rtl::Reference<ChartType> getChartType(const rtl::Reference<ChartModel>& xChartDoc)
{
    rtl::Reference<Diagram > xDiagram = xChartDoc->getFirstChartDiagram();
    if (!xDiagram.is())
        return nullptr;

    const std::vector< rtl::Reference< BaseCoordinateSystem > > & xCooSysSequence( xDiagram->getBaseCoordinateSystems());
    if (xCooSysSequence.empty())
        return nullptr;

    return xCooSysSequence[0]->getChartTypes2()[0];
}

}

OUString ChartController::GetContextName()
{
    if (m_bDisposed)
        return OUString();

    uno::Any aAny = getSelection();
    if (!aAny.hasValue())
        return u"Chart"_ustr;

    OUString aCID;
    aAny >>= aCID;

    if (aCID.isEmpty())
        return u"Chart"_ustr;

    ObjectType eObjectID = ObjectIdentifier::getObjectType(aCID);
    switch (eObjectID)
    {
        case OBJECTTYPE_DATA_SERIES:
            return u"Series"_ustr;
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
            return u"ErrorBar"_ustr;
        case OBJECTTYPE_AXIS:
            return u"Axis"_ustr;
        case OBJECTTYPE_GRID:
            return u"Grid"_ustr;
        case OBJECTTYPE_DIAGRAM:
            {
                rtl::Reference<ChartType> xChartType = getChartType(getChartModel());
                if (xChartType.is() && xChartType->getChartType() == "com.sun.star.chart2.PieChartType")
                    return u"ChartElements"_ustr;
                break;
            }
        case OBJECTTYPE_DATA_CURVE:
        case OBJECTTYPE_DATA_AVERAGE_LINE:
            return u"Trendline"_ustr;
        default:
        break;
    }

    return u"Chart"_ustr;
}

// private methods

bool ChartController::impl_isDisposedOrSuspended() const
{
    if( m_aLifeTimeManager.impl_isDisposed() )
        return true;

    if( m_bSuspended )
    {
        OSL_FAIL( "This Controller is suspended" );
        return true;
    }
    return false;
}

namespace {

uno::Reference<ui::XSidebar> getSidebarFromModel(const uno::Reference<frame::XModel>& xModel)
{
    uno::Reference<container::XChild> xChild(xModel, uno::UNO_QUERY);
    if (!xChild.is())
        return nullptr;

    uno::Reference<frame::XModel> xParent (xChild->getParent(), uno::UNO_QUERY);
    if (!xParent.is())
        return nullptr;

    uno::Reference<frame::XController2> xController(xParent->getCurrentController(), uno::UNO_QUERY);
    if (!xController.is())
        return nullptr;

    uno::Reference<ui::XSidebarProvider> xSidebarProvider  = xController->getSidebar();
    if (!xSidebarProvider.is())
        return nullptr;

    return xSidebarProvider->getSidebar();
}

}

// XController

void SAL_CALL ChartController::attachFrame(
    const uno::Reference<frame::XFrame>& xFrame )
{
    SolarMutexGuard aGuard;

    if( impl_isDisposedOrSuspended() ) //@todo? allow attaching the frame while suspended?
        return; //behave passive if already disposed or suspended

    if(m_xFrame.is()) //what happens, if we do have a Frame already??
    {
        //@todo? throw exception?
        OSL_FAIL( "there is already a frame attached to the controller" );
        return;
    }

    //--attach frame
    m_xFrame = xFrame; //the frameloader is responsible to call xFrame->setComponent

    // Only notify after setting the frame, otherwise notification will fail
    mpSelectionChangeHandler->Connect();

    uno::Reference<ui::XSidebar> xSidebar = getSidebarFromModel(getChartModel());
    if (xSidebar.is())
    {
        auto pSidebar = dynamic_cast<sfx2::sidebar::SidebarController*>(xSidebar.get());
        assert(pSidebar);
        pSidebar->registerSidebarForFrame(this);
        pSidebar->updateModel(getChartModel());
        css::lang::EventObject aEvent;
        mpSelectionChangeHandler->selectionChanged(aEvent);
    }

    //add as disposelistener to the frame (due to persistent reference) ??...:

    //the frame is considered to be owner of this controller and will live longer than we do
    //the frame or the disposer of the frame has the duty to call suspend and dispose on this object
    //so we do not need to add as lang::XEventListener for DisposingEvents right?

    //@todo nothing right???

    //create view @todo is this the correct place here??

    vcl::Window* pParent = nullptr;
    //get the window parent from the frame to use as parent for our new window
    if(xFrame.is())
    {
        uno::Reference<awt::XWindow> xContainerWindow = xFrame->getContainerWindow();
        if (xContainerWindow)
            xContainerWindow->setVisible(true);
        pParent = VCLUnoHelper::GetWindow( xContainerWindow );
    }

    {
        // calls to VCL
        SolarMutexGuard aSolarGuard;
        auto pChartWindow = VclPtr<ChartWindow>::Create(this,pParent,pParent?pParent->GetStyle():0);
        pChartWindow->SetBackground();//no Background
        m_xViewWindow.set( pChartWindow->GetComponentInterface(), uno::UNO_QUERY );
        pChartWindow->Show();
        m_apDropTargetHelper.reset(
            new ChartDropTargetHelper( pChartWindow->GetDropTarget(), getChartModel()));

        impl_createDrawViewController();
    }

    //create the menu
    {
        uno::Reference< beans::XPropertySet > xPropSet( xFrame, uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            try
            {
                uno::Reference< css::frame::XLayoutManager > xLayoutManager;
                xPropSet->getPropertyValue( u"LayoutManager"_ustr ) >>= xLayoutManager;
                if ( xLayoutManager.is() )
                {
                    xLayoutManager->lock();
                    xLayoutManager->requestElement( u"private:resource/menubar/menubar"_ustr );
                    //@todo: createElement should become unnecessary, remove when #i79198# is fixed
                    xLayoutManager->createElement( u"private:resource/toolbar/standardbar"_ustr );
                    xLayoutManager->requestElement( u"private:resource/toolbar/standardbar"_ustr );
                    //@todo: createElement should become unnecessary, remove when #i79198# is fixed
                    xLayoutManager->createElement( u"private:resource/toolbar/toolbar"_ustr );
                    xLayoutManager->requestElement( u"private:resource/toolbar/toolbar"_ustr );

                    // #i12587# support for shapes in chart
                    xLayoutManager->createElement( u"private:resource/toolbar/drawbar"_ustr );
                    xLayoutManager->requestElement( u"private:resource/toolbar/drawbar"_ustr );

                    xLayoutManager->requestElement( u"private:resource/statusbar/statusbar"_ustr );
                    xLayoutManager->unlock();

                    // add as listener to get notified when
                    m_xLayoutManagerEventBroadcaster.set( xLayoutManager, uno::UNO_QUERY );
                    if( m_xLayoutManagerEventBroadcaster.is())
                        m_xLayoutManagerEventBroadcaster->addLayoutManagerEventListener( this );
                }
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }
}

//XModeChangeListener
void SAL_CALL ChartController::modeChanged( const util::ModeChangeEvent& rEvent )
{
    SolarMutexGuard aGuard;
    auto pChartWindow(GetChartWindow());
    //adjust controller to view status changes

    if( rEvent.NewMode == "dirty" )
    {
        //the view has become dirty, we should repaint it if we have a window
        if( pChartWindow )
            pChartWindow->ForceInvalidate();
    }
    else if( rEvent.NewMode == "invalid" )
    {
        //the view is about to become invalid so end all actions on it
        impl_invalidateAccessible();
        if( m_pDrawViewWrapper && m_pDrawViewWrapper->IsTextEdit() )
            this->EndTextEdit();
        if( m_pDrawViewWrapper )
        {
            m_pDrawViewWrapper->UnmarkAll();
            m_pDrawViewWrapper->HideSdrPage();
        }
    }
    else
    {
        //the view was rebuild so we can start some actions on it again
        if( !m_bConnectingToView )
        {
            if(pChartWindow && m_aModel.is() )
            {
                m_bConnectingToView = true;

                GetDrawModelWrapper();
                if(m_pDrawModelWrapper)
                {
                    {
                        if( m_pDrawViewWrapper )
                            m_pDrawViewWrapper->ReInit();
                    }

                    //reselect object
                    if( m_aSelection.hasSelection() )
                        this->impl_selectObjectAndNotiy();
                    else
                        ChartModelHelper::triggerRangeHighlighting( getChartModel() );

                    impl_initializeAccessible();

                    {
                        if( pChartWindow )
                            pChartWindow->Invalidate();
                    }
                }

                m_bConnectingToView = false;
            }
        }
    }
}

sal_Bool SAL_CALL ChartController::attachModel( const uno::Reference< frame::XModel > & xModel )
{
    impl_invalidateAccessible();

    //is called to attach the controller to a new model.
    //return true if attach was successfully, false otherwise (e.g. if you do not work with a model)

    SolarMutexResettableGuard aGuard;
    if( impl_isDisposedOrSuspended() ) //@todo? allow attaching a new model while suspended?
        return false; //behave passive if already disposed or suspended
    aGuard.clear();

    ::chart::ChartModel* pChartModel = dynamic_cast<::chart::ChartModel*>(xModel.get());
    assert(!xModel || pChartModel);

    TheModelRef aNewModelRef( new TheModel(pChartModel), m_aModelMutex);
    TheModelRef aOldModelRef(m_aModel,m_aModelMutex);
    m_aModel = aNewModelRef;

    //--handle relations to the old model if any
    if( aOldModelRef.is() )
    {
        if( m_xChartView.is() )
            m_xChartView->removeModeChangeListener(this);
        m_pDrawModelWrapper.reset();

        aOldModelRef->removeListener( this );
 #ifdef TEST_ENABLE_MODIFY_LISTENER
        if( aOldModelRef->getModel().is())
            aOldModelRef->getModel()->removeModifyListener( this );
#endif
    }

    //--handle relations to the new model
    aNewModelRef->addListener( this );

    aGuard.reset(); // lock for m_aDispatchContainer access
    // set new model at dispatchers
    m_aDispatchContainer.setModel( aNewModelRef->getModel());
    rtl::Reference<ControllerCommandDispatch> pDispatch = new ControllerCommandDispatch( m_xCC, this, &m_aDispatchContainer );
    pDispatch->initialize();

    // the dispatch container will return "this" for all commands returned by
    // impl_getAvailableCommands().  That means, for those commands dispatch()
    // is called here at the ChartController.
    m_aDispatchContainer.setChartDispatch( pDispatch, impl_getAvailableCommands() );

    rtl::Reference<DrawCommandDispatch> pDrawDispatch = new DrawCommandDispatch( m_xCC, this );
    pDrawDispatch->initialize();
    m_aDispatchContainer.setDrawCommandDispatch( pDrawDispatch.get() );

    rtl::Reference<ShapeController> pShapeController = new ShapeController( m_xCC, this );
    pShapeController->initialize();
    m_aDispatchContainer.setShapeController( pShapeController.get() );
    aGuard.clear();

#ifdef TEST_ENABLE_MODIFY_LISTENER
    if( aNewModelRef->getModel().is())
        aNewModelRef->getModel()->addModifyListener( this );
#endif

    // #i119999# Do not do this per default to allow the user to deselect the chart OLE with a single press to ESC
    // select chart area per default:
    // select( uno::Any( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, OUString() ) ) );

    rtl::Reference< ChartModel > xFact = getChartModel();
    if( xFact.is())
    {
        m_xChartView = dynamic_cast<::chart::ChartView*>(xFact->createInstance( CHART_VIEW_SERVICE_NAME ).get());
        GetDrawModelWrapper();
        m_xChartView->addModeChangeListener(this);
    }

    //the frameloader is responsible to call xModel->connectController
    {
        SolarMutexGuard aGuard2;
        auto pChartWindow(GetChartWindow());
        if( pChartWindow )
            pChartWindow->Invalidate();
    }

    m_xUndoManager.set( getChartModel()->getUndoManager(), uno::UNO_SET_THROW );

    return true;
}

uno::Reference< frame::XFrame > SAL_CALL ChartController::getFrame()
{
    //provides access to owner frame of this controller
    //return the frame containing this controller

    return m_xFrame;
}

uno::Reference< frame::XModel > SAL_CALL ChartController::getModel()
{
    return getChartModel();
}

rtl::Reference<::chart::ChartModel> ChartController::getChartModel()
{
    //provides access to currently attached model
    //returns the currently attached model

    //return nothing, if you do not have a model
    TheModelRef aModelRef( m_aModel, m_aModelMutex);
    if(aModelRef.is())
        return aModelRef->getModel();

    return nullptr;
}

rtl::Reference<::chart::Diagram> ChartController::getFirstDiagram()
{
    return getChartModel()->getFirstChartDiagram();
}

uno::Any SAL_CALL ChartController::getViewData()
{
    //provides access to current view status
    //set of data that can be used to restore the current view status at later time
    //  by using XController::restoreViewData()

    SolarMutexGuard aGuard;
    if( impl_isDisposedOrSuspended() )
        return uno::Any(); //behave passive if already disposed or suspended //@todo? or throw an exception??

    //-- collect current view state
    uno::Any aRet;
    //// @todo integrate specialized implementation

    return aRet;
}

void SAL_CALL ChartController::restoreViewData(
    const uno::Any& /* Value */ )
{
    //restores the view status using the data gotten from a previous call to XController::getViewData()

    SolarMutexGuard aGuard;
    if( impl_isDisposedOrSuspended() )
        return; //behave passive if already disposed or suspended //@todo? or throw an exception??

    //// @todo integrate specialized implementation
}

sal_Bool SAL_CALL ChartController::suspend( sal_Bool bSuspend )
{
    //is called to prepare the controller for closing the view
    //bSuspend==true: force the controller to suspend his work
    //bSuspend==false try to reactivate the controller
    //returns true if request was accepted and of course successfully finished, false otherwise

    //we may show dialogs here to ask the user for saving changes ... @todo?

    SolarMutexGuard aGuard;
    if( m_aLifeTimeManager.impl_isDisposed() )
        return false; //behave passive if already disposed, return false because request was not accepted //@todo? correct

    if(bool(bSuspend) == m_bSuspended)
    {
        OSL_FAIL( "new suspend mode equals old suspend mode" );
        return true;
    }

    //change suspend mode
    m_bSuspended = bSuspend;
    return true;
}

// css::frame::XController2

css::uno::Reference<css::awt::XWindow> SAL_CALL ChartController::getComponentWindow()
{
    // it is a special characteristic of ChartController
    // that it simultaneously provides the XWindow functionality
    return this;
}

OUString SAL_CALL ChartController::getViewControllerName() { return {}; }

css::uno::Sequence<css::beans::PropertyValue> SAL_CALL ChartController::getCreationArguments()
{
    return {};
}

css::uno::Reference<css::ui::XSidebarProvider> SAL_CALL ChartController::getSidebar() { return {}; }

void ChartController::impl_createDrawViewController()
{
    SolarMutexGuard aGuard;
    if(!m_pDrawViewWrapper)
    {
        if( m_pDrawModelWrapper )
        {
            bool bLokCalcGlobalRTL = false;
            if(comphelper::LibreOfficeKit::isActive() && AllSettings::GetLayoutRTL())
            {
                rtl::Reference< ChartModel > xChartModel = getChartModel();
                if (xChartModel.is())
                {
                    uno::Reference<css::sheet::XSpreadsheetDocument> xSSDoc(xChartModel->getParent(), uno::UNO_QUERY);
                    if (xSSDoc.is())
                        bLokCalcGlobalRTL = true;
                }
            }

            m_pDrawViewWrapper.reset( new DrawViewWrapper(m_pDrawModelWrapper->getSdrModel(),GetChartWindow()->GetOutDev()) );
            m_pDrawViewWrapper->SetNegativeX(bLokCalcGlobalRTL);
            m_pDrawViewWrapper->attachParentReferenceDevice( getChartModel() );
        }
    }
}

void ChartController::impl_deleteDrawViewController()
{
    if( m_pDrawViewWrapper )
    {
        SolarMutexGuard aGuard;
        if( m_pDrawViewWrapper->IsTextEdit() )
            this->EndTextEdit();
        m_pDrawViewWrapper.reset();
    }
}

// XComponent (base of XController)

void SAL_CALL ChartController::dispose()
{
    m_bDisposed = true;

    mpSelectionChangeHandler->selectionChanged(css::lang::EventObject());
    mpSelectionChangeHandler->Disconnect();

    if (getModel().is())
    {
        uno::Reference<ui::XSidebar> xSidebar = getSidebarFromModel(getChartModel());
        if (sfx2::sidebar::SidebarController* pSidebar = dynamic_cast<sfx2::sidebar::SidebarController*>(xSidebar.get()))
        {
            pSidebar->unregisterSidebarForFrame(this);
        }
    }

    try
    {
        //This object should release all resources and references in the
        //easiest possible manner
        //This object must notify all registered listeners using the method
        //<member>XEventListener::disposing</member>

        //hold no mutex
        if( !m_aLifeTimeManager.dispose() )
            return;

//  OSL_ENSURE( m_bSuspended, "dispose was called but controller is not suspended" );

        this->stopDoubleClickWaiting();

        //end range highlighting
        if( m_aModel.is())
        {
            uno::Reference< view::XSelectionChangeListener > xSelectionChangeListener;
            rtl::Reference< ChartModel > xDataReceiver = getChartModel();
            if( xDataReceiver.is() )
                xSelectionChangeListener.set( xDataReceiver->getRangeHighlighter(), uno::UNO_QUERY );
            if( xSelectionChangeListener.is() )
            {
                uno::Reference< frame::XController > xController( this );
                lang::EventObject aEvent( xController );
                xSelectionChangeListener->disposing( aEvent );
            }
        }

        //--release all resources and references
        {
            if( m_xChartView.is() )
                m_xChartView->removeModeChangeListener(this);

            impl_invalidateAccessible();
            SolarMutexGuard aSolarGuard;
            impl_deleteDrawViewController();
            m_pDrawModelWrapper.reset();

            m_apDropTargetHelper.reset();

            //the accessible view is disposed within window destructor of m_pChartWindow
            if(m_xViewWindow.is())
                m_xViewWindow->dispose(); //ChartWindow is deleted via UNO due to dispose of m_xViewWindow (triggered by Framework (Controller pretends to be XWindow also))
            m_xChartView.clear();
        }

        // remove as listener to layout manager events
        if( m_xLayoutManagerEventBroadcaster.is())
        {
            m_xLayoutManagerEventBroadcaster->removeLayoutManagerEventListener( this );
            m_xLayoutManagerEventBroadcaster.clear();
        }

        m_xFrame.clear();
        m_xUndoManager.clear();

        TheModelRef aModelRef( m_aModel, m_aModelMutex);
        m_aModel = nullptr;

        if( aModelRef.is())
        {
            uno::Reference< frame::XModel > xModel( aModelRef->getModel() );
            if(xModel.is())
                xModel->disconnectController( uno::Reference< frame::XController >( this ));

            aModelRef->removeListener( this );
#ifdef TEST_ENABLE_MODIFY_LISTENER
            try
            {
                if( aModelRef->getModel().is())
                    aModelRef->getModel()->removeModifyListener( this );
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
#endif
            aModelRef->tryTermination();
        }

        //// @todo integrate specialized implementation
        //e.g. release further resources and references

        SolarMutexGuard g;
        m_aDispatchContainer.DisposeAndClear();
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
        assert(!m_xChartView.is());
    }
 }

void SAL_CALL ChartController::addEventListener(
    const uno::Reference<lang::XEventListener>& xListener )
{
    if( impl_isDisposedOrSuspended() )//@todo? allow adding of listeners in suspend mode?
        return; //behave passive if already disposed or suspended

    //--add listener
    std::unique_lock aGuard2(m_aLifeTimeManager.m_aAccessMutex);
    m_aLifeTimeManager.m_aEventListeners.addInterface( aGuard2, xListener );
}

void SAL_CALL ChartController::removeEventListener(
    const uno::Reference<lang::XEventListener>& xListener )
{
    SolarMutexGuard aGuard;
    if( m_aLifeTimeManager.impl_isDisposed(false) )
        return; //behave passive if already disposed or suspended

    //--remove listener
    std::unique_lock aGuard2(m_aLifeTimeManager.m_aAccessMutex);
    m_aLifeTimeManager.m_aEventListeners.removeInterface( aGuard2, xListener );
}

// util::XCloseListener
void SAL_CALL ChartController::queryClosing(
    const lang::EventObject& rSource,
    sal_Bool /*bGetsOwnership*/ )
{
    //do not use the m_aControllerMutex here because this call is not allowed to block

    TheModelRef aModelRef( m_aModel, m_aModelMutex);

    if( !aModelRef.is() )
        return;

    if( uno::Reference<XInterface>(static_cast<cppu::OWeakObject*>(aModelRef->getModel().get())) != rSource.Source )
    {
        OSL_FAIL( "queryClosing was called on a controller from an unknown source" );
        return;
    }

    //@ todo prepare to closing model -> don't start any further hindering actions
}

void SAL_CALL ChartController::notifyClosing(
    const lang::EventObject& rSource )
{
    //Listener should deregister himself and release all references to the closing object.

    TheModelRef aModelRef( m_aModel, m_aModelMutex);
    if( !impl_releaseThisModel( rSource.Source ) )
        return;

    //--stop listening to the closing model
    aModelRef->removeListener( this );

    // #i79087# If the model using this controller is closed, the frame is
    // expected to be closed as well
    Reference< util::XCloseable > xFrameCloseable( m_xFrame, uno::UNO_QUERY );
    if( xFrameCloseable.is())
    {
        try
        {
            xFrameCloseable->close( false /* DeliverOwnership */ );
            m_xFrame.clear();
        }
        catch( const util::CloseVetoException & )
        {
            // closing was vetoed
        }
    }
}

bool ChartController::impl_releaseThisModel(
    const uno::Reference< uno::XInterface > & xModel )
{
    bool bReleaseModel = false;
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aModelMutex );
        if( m_aModel.is() && uno::Reference< uno::XInterface >(static_cast<cppu::OWeakObject*>(m_aModel->getModel().get())) == xModel )
        {
            m_aModel = nullptr;
            m_xUndoManager.clear();
            bReleaseModel = true;
        }
    }
    if( bReleaseModel )
    {
        SolarMutexGuard g;
        m_aDispatchContainer.setModel( nullptr );
    }
    return bReleaseModel;
}

// util::XEventListener (base of XCloseListener)
void SAL_CALL ChartController::disposing(
    const lang::EventObject& rSource )
{
    if( !impl_releaseThisModel( rSource.Source ))
    {
        if( rSource.Source == m_xLayoutManagerEventBroadcaster )
            m_xLayoutManagerEventBroadcaster.clear();
    }
}

void SAL_CALL ChartController::layoutEvent(
    const lang::EventObject& aSource,
    sal_Int16 eLayoutEvent,
    const uno::Any& /* aInfo */ )
{
    if( eLayoutEvent == frame::LayoutManagerEvents::MERGEDMENUBAR )
    {
        Reference< frame::XLayoutManager > xLM( aSource.Source, uno::UNO_QUERY );
        if( xLM.is())
        {
            xLM->createElement(  u"private:resource/statusbar/statusbar"_ustr );
            xLM->requestElement( u"private:resource/statusbar/statusbar"_ustr );
        }
    }
}

// XDispatchProvider (required interface)

namespace
{

bool lcl_isFormatObjectCommand( std::u16string_view aCommand )
{
    return aCommand == u"MainTitle"
        || aCommand == u"SubTitle"
        || aCommand == u"XTitle"
        || aCommand == u"YTitle"
        || aCommand == u"ZTitle"
        || aCommand == u"SecondaryXTitle"
        || aCommand == u"SecondaryYTitle"
        || aCommand == u"AllTitles"
        || aCommand == u"DiagramAxisX"
        || aCommand == u"DiagramAxisY"
        || aCommand == u"DiagramAxisZ"
        || aCommand == u"DiagramAxisA"
        || aCommand == u"DiagramAxisB"
        || aCommand == u"DiagramAxisAll"
        || aCommand == u"DiagramGridXMain"
        || aCommand == u"DiagramGridYMain"
        || aCommand == u"DiagramGridZMain"
        || aCommand == u"DiagramGridXHelp"
        || aCommand == u"DiagramGridYHelp"
        || aCommand == u"DiagramGridZHelp"
        || aCommand == u"DiagramGridAll"

        || aCommand == u"DiagramWall"
        || aCommand == u"DiagramFloor"
        || aCommand == u"DiagramArea"
        || aCommand == u"Legend"

        || aCommand == u"FormatWall"
        || aCommand == u"FormatFloor"
        || aCommand == u"FormatChartArea"
        || aCommand == u"FormatLegend"

        || aCommand == u"FormatTitle"
        || aCommand == u"FormatAxis"
        || aCommand == u"FormatDataSeries"
        || aCommand == u"FormatDataPoint"
        || aCommand == u"FormatDataLabels"
        || aCommand == u"FormatDataLabel"
        || aCommand == u"FormatXErrorBars"
        || aCommand == u"FormatYErrorBars"
        || aCommand == u"FormatMeanValue"
        || aCommand == u"FormatTrendline"
        || aCommand == u"FormatTrendlineEquation"
        || aCommand == u"FormatStockLoss"
        || aCommand == u"FormatStockGain"
        || aCommand == u"FormatMajorGrid"
        || aCommand == u"FormatMinorGrid";
}

} // anonymous namespace

uno::Reference<frame::XDispatch> SAL_CALL
    ChartController::queryDispatch(
        const util::URL& rURL,
        const OUString& rTargetFrameName,
        sal_Int32 /* nSearchFlags */)
{
    SolarMutexGuard aGuard;

    if ( !m_aLifeTimeManager.impl_isDisposed() && getModel().is() )
    {
        if( !rTargetFrameName.isEmpty() && rTargetFrameName == "_self" )
            return m_aDispatchContainer.getDispatchForURL( rURL );
    }
    return uno::Reference< frame::XDispatch > ();
}

uno::Sequence<uno::Reference<frame::XDispatch > >
    ChartController::queryDispatches(
        const uno::Sequence<frame::DispatchDescriptor>& xDescripts )
{
    SolarMutexGuard g;

    if ( !m_aLifeTimeManager.impl_isDisposed() )
    {
        return m_aDispatchContainer.getDispatchesForURLs( xDescripts );
    }
    return uno::Sequence<uno::Reference<frame::XDispatch > > ();
}

// frame::XDispatch

void SAL_CALL ChartController::dispatch(
    const util::URL& rURL,
    const uno::Sequence< beans::PropertyValue >& rArgs )
{
    OUString aCommand = rURL.Path;

    if(aCommand == "LOKSetTextSelection")
    {
        if (rArgs.getLength() == 3)
        {
            sal_Int32 nType = -1;
            rArgs[0].Value >>= nType;
            sal_Int32 nX = 0;
            rArgs[1].Value >>= nX;
            sal_Int32 nY = 0;
            rArgs[2].Value >>= nY;
            executeDispatch_LOKSetTextSelection(nType, nX, nY);
        }
    }
    else if (aCommand == "LOKTransform")
    {
        if (rArgs[0].Name == "Action")
        {
            OUString sAction;
            if ((rArgs[0].Value >>= sAction) && sAction == "PieSegmentDragging")
            {
                if (rArgs[1].Name == "Offset")
                {
                    sal_Int32 nOffset;
                    if (rArgs[1].Value >>= nOffset)
                    {
                        this->executeDispatch_LOKPieSegmentDragging(nOffset);
                    }
                }
            }
        }
        else
        {
            this->executeDispatch_PositionAndSize(&rArgs);
        }
    }
    else if(aCommand == "FillColor")
    {
        if (rArgs.getLength() > 0)
        {
            sal_uInt32 nColor;
            if (rArgs[0].Value >>= nColor)
                this->executeDispatch_FillColor(nColor);
        }
    }
    else if(aCommand == "XLineColor")
    {
        if (rArgs.getLength() > 0)
        {
            sal_Int32 nColor = -1;
            rArgs[0].Value >>= nColor;
            this->executeDispatch_LineColor(nColor);
        }
    }
    else if(aCommand == "LineWidth")
    {
        if (rArgs.getLength() > 0)
        {
            sal_Int32 nWidth = -1;
            rArgs[0].Value >>= nWidth;
            this->executeDispatch_LineWidth(nWidth);
        }
    }
    else if(aCommand.startsWith("FillGradient"))
    {
        this->executeDispatch_FillGradient(aCommand.subView(aCommand.indexOf('=') + 1));
    }
    else if(aCommand == "Paste")
        this->executeDispatch_Paste();
    else if(aCommand == "Copy" )
        this->executeDispatch_Copy();
    else if(aCommand == "Cut" )
        this->executeDispatch_Cut();
    else if(aCommand == "DataRanges" )
        this->executeDispatch_SourceData();
    else if(aCommand == "Update" ) //Update Chart
    {
        ChartViewHelper::setViewToDirtyState( getChartModel() );
        SolarMutexGuard aGuard;
        auto pChartWindow(GetChartWindow());
        if( pChartWindow )
            pChartWindow->Invalidate();
    }
    else if(aCommand == "DiagramData" )
        this->executeDispatch_EditData();
    //insert objects
    else if( aCommand == "InsertTitles"
        || aCommand == "InsertMenuTitles")
        this->executeDispatch_InsertTitles();
    else if( aCommand == "InsertMenuLegend" )
        this->executeDispatch_OpenLegendDialog();
    else if( aCommand == "InsertLegend" )
        this->executeDispatch_InsertLegend();
    else if( aCommand == "DeleteLegend" )
        this->executeDispatch_DeleteLegend();
    else if( aCommand == "InsertMenuDataLabels" )
        this->executeDispatch_InsertMenu_DataLabels();
    else if( aCommand == "InsertMenuAxes"
        || aCommand == "InsertRemoveAxes" )
        this->executeDispatch_InsertAxes();
    else if( aCommand == "InsertMenuGrids" )
        this->executeDispatch_InsertGrid();
    else if( aCommand == "InsertMenuTrendlines" )
        this->executeDispatch_InsertMenu_Trendlines();
    else if( aCommand == "InsertMenuMeanValues" )
        this->executeDispatch_InsertMenu_MeanValues();
    else if( aCommand == "InsertMenuXErrorBars" )
        this->executeDispatch_InsertErrorBars(false);
    else if( aCommand == "InsertMenuYErrorBars" )
        this->executeDispatch_InsertErrorBars(true);
    else if( aCommand == "InsertMenuDataTable" )
        this->executeDispatch_OpenInsertDataTableDialog();
    else if( aCommand == "InsertSymbol" )
         this->executeDispatch_InsertSpecialCharacter();
    else if( aCommand == "InsertTrendline" )
         this->executeDispatch_InsertTrendline();
    else if( aCommand == "DeleteTrendline" )
         this->executeDispatch_DeleteTrendline();
    else if( aCommand == "InsertMeanValue" )
        this->executeDispatch_InsertMeanValue();
    else if( aCommand == "DeleteMeanValue" )
        this->executeDispatch_DeleteMeanValue();
    else if( aCommand == "InsertXErrorBars" )
        this->executeDispatch_InsertErrorBars(false);
    else if( aCommand == "InsertYErrorBars" )
        this->executeDispatch_InsertErrorBars(true);
    else if( aCommand == "DeleteXErrorBars" )
        this->executeDispatch_DeleteErrorBars(false);
    else if( aCommand == "DeleteYErrorBars" )
        this->executeDispatch_DeleteErrorBars(true);
    else if( aCommand == "InsertTrendlineEquation" )
         this->executeDispatch_InsertTrendlineEquation();
    else if( aCommand == "DeleteTrendlineEquation" )
         this->executeDispatch_DeleteTrendlineEquation();
    else if( aCommand == "InsertTrendlineEquationAndR2" )
         this->executeDispatch_InsertTrendlineEquation( true );
    else if( aCommand == "InsertR2Value" )
         this->executeDispatch_InsertR2Value();
    else if( aCommand == "DeleteR2Value")
         this->executeDispatch_DeleteR2Value();
    else if( aCommand == "InsertDataLabels" )
        this->executeDispatch_InsertDataLabels();
    else if( aCommand == "InsertDataLabel" )
        this->executeDispatch_InsertDataLabel();
    else if( aCommand == "DeleteDataLabels")
        this->executeDispatch_DeleteDataLabels();
    else if( aCommand == "DeleteDataLabel" )
        this->executeDispatch_DeleteDataLabel();
    else if( aCommand == "ResetAllDataPoints" )
        this->executeDispatch_ResetAllDataPoints();
    else if( aCommand == "ResetDataPoint" )
        this->executeDispatch_ResetDataPoint();
    else if( aCommand == "InsertAxis" )
        this->executeDispatch_InsertAxis();
    else if( aCommand == "InsertMajorGrid" )
        this->executeDispatch_InsertMajorGrid();
    else if( aCommand == "InsertMinorGrid" )
        this->executeDispatch_InsertMinorGrid();
    else if( aCommand == "InsertAxisTitle" )
        this->executeDispatch_InsertAxisTitle();
    else if( aCommand == "DeleteAxis" )
        this->executeDispatch_DeleteAxis();
    else if( aCommand == "DeleteMajorGrid")
        this->executeDispatch_DeleteMajorGrid();
    else if( aCommand == "DeleteMinorGrid" )
        this->executeDispatch_DeleteMinorGrid();
    else if( aCommand == "InsertDataTable" )
         this->executeDispatch_InsertDataTable();
    else if( aCommand == "DeleteDataTable" )
         this->executeDispatch_DeleteDataTable();
    //format objects
    else if( aCommand == "FormatSelection" )
        this->executeDispatch_ObjectProperties();
    else if( aCommand == "TransformDialog" )
    {
        if ( isShapeContext() )
        {
            this->impl_ShapeControllerDispatch( rURL, rArgs );
        }
        else
        {
            this->executeDispatch_PositionAndSize();
        }
    }
    else if ( aCommand == "FontDialog" )
        this->impl_ShapeControllerDispatch(rURL, rArgs);
    else if (lcl_isFormatObjectCommand(aCommand))
        this->executeDispatch_FormatObject(rURL.Path);
    //more format
    else if( aCommand == "DiagramType" )
        this->executeDispatch_ChartType();
    else if( aCommand == "View3D" )
        this->executeDispatch_View3D();
    else if ( aCommand == "Forward" )
    {
        if ( isShapeContext() )
        {
            this->impl_ShapeControllerDispatch( rURL, rArgs );
        }
        else
        {
            this->executeDispatch_MoveSeries( true );
        }
    }
    else if ( aCommand == "Backward" )
    {
        if ( isShapeContext() )
        {
            this->impl_ShapeControllerDispatch( rURL, rArgs );
        }
        else
        {
            this->executeDispatch_MoveSeries( false );
        }
    }
    else if( aCommand == "NewArrangement")
        this->executeDispatch_NewArrangement();
    else if( aCommand == "ToggleLegend" )
        this->executeDispatch_ToggleLegend();
    else if( aCommand == "ToggleGridHorizontal" )
        this->executeDispatch_ToggleGridHorizontal();
    else if( aCommand == "ToggleGridVertical" )
        this->executeDispatch_ToggleGridVertical();
    else if( aCommand == "ScaleText" )
        this->executeDispatch_ScaleText();
    else if( aCommand == "StatusBarVisible" )
    {
        // workaround: this should not be necessary.
        uno::Reference< beans::XPropertySet > xPropSet( m_xFrame, uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            uno::Reference< css::frame::XLayoutManager > xLayoutManager;
            xPropSet->getPropertyValue( u"LayoutManager"_ustr ) >>= xLayoutManager;
            if ( xLayoutManager.is() )
            {
                bool bIsVisible( xLayoutManager->isElementVisible( u"private:resource/statusbar/statusbar"_ustr ));
                if( bIsVisible )
                {
                    xLayoutManager->hideElement( u"private:resource/statusbar/statusbar"_ustr );
                    xLayoutManager->destroyElement( u"private:resource/statusbar/statusbar"_ustr );
                }
                else
                {
                    xLayoutManager->createElement( u"private:resource/statusbar/statusbar"_ustr );
                    xLayoutManager->showElement( u"private:resource/statusbar/statusbar"_ustr );
                }
                // @todo: update menu state (checkmark next to "Statusbar").
            }
        }
    }
    else if( aCommand == "ChangeTheme" )
        comphelper::dispatchCommand(u".uno:ChangeTheme"_ustr, getFrame(), rArgs);
}

void SAL_CALL ChartController::addStatusListener(
    const uno::Reference<frame::XStatusListener >& /* xControl */,
    const util::URL& /* aURL */ )
{
    //@todo
}

void SAL_CALL ChartController::removeStatusListener(
    const uno::Reference<frame::XStatusListener >& /* xControl */,
    const util::URL& /* aURL */ )
{
    //@todo
}

// XContextMenuInterception (optional interface)
void SAL_CALL ChartController::registerContextMenuInterceptor(
    const uno::Reference< ui::XContextMenuInterceptor >& /* xInterceptor */)
{
    //@todo
}

void SAL_CALL ChartController::releaseContextMenuInterceptor(
    const uno::Reference< ui::XContextMenuInterceptor > & /* xInterceptor */)
{
    //@todo
}

// ____ XEmbeddedClient ____
// implementation see: ChartController_EditData.cxx

void ChartController::executeDispatch_ChartType()
{
    auto xUndoGuard = std::make_shared<UndoLiveUpdateGuard>(SchResId(STR_ACTION_EDIT_CHARTTYPE),
                                                            m_xUndoManager);

    SolarMutexGuard aSolarGuard;
    //prepare and open dialog
    auto aDlg =  std::make_shared<ChartTypeDialog>(GetChartFrame(), getChartModel());
    weld::DialogController::runAsync(aDlg, [this, xUndoGuard=std::move(xUndoGuard)](int nResult) {
        if (nResult == RET_OK)
        {
            impl_adaptDataSeriesAutoResize();
            xUndoGuard->commit();
        }
    });
}

void ChartController::executeDispatch_SourceData()
{
    //convert properties to ItemSet
    rtl::Reference< ::chart::ChartModel >  xChartDoc = getChartModel();
    OSL_ENSURE( xChartDoc.is(), "Invalid XChartDocument" );
    if( !xChartDoc.is() )
        return;

    // If there is a data table we should ask user if we really want to destroy it
    // and switch to data ranges.
    ChartModel& rModel = *xChartDoc;
    if ( rModel.hasInternalDataProvider() )
    {
        // Check if we will able to create data provider later
        css::uno::Reference< com::sun::star::chart2::XDataProviderAccess > xCreatorDoc(
            rModel.getParent(), uno::UNO_QUERY);
        if (!xCreatorDoc.is())
            return;

        SolarMutexGuard aSolarGuard;

        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetChartFrame(),
                                                       VclMessageType::Question, VclButtonsType::YesNo, SchResId(STR_DLG_REMOVE_DATA_TABLE)));
        // If "No" then just return
        if (xQueryBox->run() == RET_NO)
            return;

        // Remove data table
        rModel.removeDataProviders();

        // Ask parent document to create new data provider

        uno::Reference< data::XDataProvider > xDataProvider = xCreatorDoc->createDataProvider();
        SAL_WARN_IF( !xDataProvider.is(), "chart2.main", "Data provider was not created" );
        if (xDataProvider.is())
        {
            rModel.attachDataProvider(xDataProvider);
        }
    }
    auto xUndoGuard = std::make_shared<UndoLiveUpdateGuard>(SchResId(STR_ACTION_EDIT_DATA_RANGES),
                                                            m_xUndoManager);
    SolarMutexGuard aSolarGuard;
    auto aDlg = std::make_shared<DataSourceDialog>(GetChartFrame(), xChartDoc);
    weld::DialogController::runAsync(aDlg, [this, xUndoGuard=std::move(xUndoGuard)](int nResult) {
        if (nResult == RET_OK)
        {
            impl_adaptDataSeriesAutoResize();
            xUndoGuard->commit();
        }
    });
}

void ChartController::executeDispatch_MoveSeries( bool bForward )
{
    ControllerLockGuardUNO aCLGuard( getChartModel() );

    //get selected series
    OUString aObjectCID(m_aSelection.getSelectedCID());
    rtl::Reference< DataSeries > xGivenDataSeries = ObjectIdentifier::getDataSeriesForCID( //yyy todo also legend entries and labels?
            aObjectCID, getChartModel() );

    UndoGuardWithSelection aUndoGuard(
        ActionDescriptionProvider::createDescription(
            (bForward ? ActionDescriptionProvider::ActionType::MoveToTop : ActionDescriptionProvider::ActionType::MoveToBottom),
            SchResId(STR_OBJECT_DATASERIES)),
        m_xUndoManager );

    bool bChanged = getFirstDiagram()->moveSeries( xGivenDataSeries, bForward );
    if( bChanged )
    {
        m_aSelection.setSelection( ObjectIdentifier::getMovedSeriesCID( aObjectCID, bForward ) );
        aUndoGuard.commit();
    }
}

// ____ XModifyListener ____
void SAL_CALL ChartController::modified(
    const lang::EventObject& /* aEvent */ )
{
    // the source can also be a subobject of the ChartModel
    // @todo: change the source in ChartModel to always be the model itself ?
    //todo? update menu states ?
}

void ChartController::NotifyUndoActionHdl( std::unique_ptr<SdrUndoAction> pUndoAction )
{
    ENSURE_OR_RETURN_VOID( pUndoAction, "invalid Undo action" );

    OUString aObjectCID = m_aSelection.getSelectedCID();
    if ( !aObjectCID.isEmpty() )
        return;

    try
    {
        rtl::Reference< ChartModel > xSuppUndo = getChartModel();
        const Reference< document::XUndoManager > xUndoManager( xSuppUndo->getUndoManager(), uno::UNO_SET_THROW );
        const Reference< document::XUndoAction > xAction( new impl::ShapeUndoElement( std::move(pUndoAction) ) );
        xUndoManager->addUndoAction( xAction );
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

DrawModelWrapper* ChartController::GetDrawModelWrapper()
{
    if( !m_pDrawModelWrapper )
    {
        if( m_xChartView )
            m_pDrawModelWrapper = m_xChartView->getDrawModelWrapper();
        if ( m_pDrawModelWrapper )
        {
            m_pDrawModelWrapper->getSdrModel().SetNotifyUndoActionHdl(
                std::bind(&ChartController::NotifyUndoActionHdl, this, std::placeholders::_1) );
        }
    }
    return m_pDrawModelWrapper.get();
}

DrawViewWrapper* ChartController::GetDrawViewWrapper()
{
    if ( !m_pDrawViewWrapper )
    {
        impl_createDrawViewController();
    }
    return m_pDrawViewWrapper.get();
}


ChartWindow* ChartController::GetChartWindow() const
{
    // clients getting the naked VCL Window from UNO should always have the
    // solar mutex (and keep it over the lifetime of this ptr), as VCL might
    // might deinit otherwise
    DBG_TESTSOLARMUTEX();
    if(!m_xViewWindow.is())
        return nullptr;
    return dynamic_cast<ChartWindow*>(VCLUnoHelper::GetWindow(m_xViewWindow));
}

weld::Window* ChartController::GetChartFrame()
{
    // clients getting the naked VCL Window from UNO should always have the
    // solar mutex (and keep it over the lifetime of this ptr), as VCL might
    // might deinit otherwise
    DBG_TESTSOLARMUTEX();
    return Application::GetFrameWeld(m_xViewWindow);
}

bool ChartController::isAdditionalShapeSelected() const
{
    return m_aSelection.isAdditionalShapeSelected();
}

void ChartController::SetAndApplySelection(const Reference<drawing::XShape>& rxShape)
{
    if(rxShape.is())
    {
        m_aSelection.setSelection(rxShape);
        m_aSelection.applySelection(GetDrawViewWrapper());
    }
}



uno::Reference< XAccessible > ChartController::CreateAccessible()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    rtl::Reference< AccessibleChartView > xResult = new AccessibleChartView( GetDrawViewWrapper() );
    impl_initializeAccessible( *xResult );
    return xResult;
#else
    return uno::Reference< XAccessible >();
#endif
}

void ChartController::impl_invalidateAccessible()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    SolarMutexGuard aGuard;
    auto pChartWindow(GetChartWindow());
    if( pChartWindow )
    {
        Reference< XInterface > xInit( pChartWindow->GetAccessible(false) );
        if(xInit.is())
        {
            //empty arguments -> invalid accessible
            dynamic_cast<AccessibleChartView&>(*xInit).initialize();
        }
    }
#endif
}
void ChartController::impl_initializeAccessible()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    SolarMutexGuard aGuard;
    auto pChartWindow(GetChartWindow());
    if( !pChartWindow )
        return;
    Reference<XAccessible> xInit = pChartWindow->GetAccessible(false);
    if(xInit.is())
        impl_initializeAccessible( dynamic_cast<AccessibleChartView&>(*xInit) );
#endif
}
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
void ChartController::impl_initializeAccessible( AccessibleChartView& rAccChartView )
{
    uno::Reference< XAccessible > xParent;
    {
        SolarMutexGuard aGuard;
        auto pChartWindow(GetChartWindow());
        if( pChartWindow )
        {
            vcl::Window* pParentWin( pChartWindow->GetAccessibleParentWindow());
            if( pParentWin )
                xParent.set( pParentWin->GetAccessible());
        }
    }

    rAccChartView.initialize(*this, getChartModel(), m_xChartView, xParent, m_xViewWindow);
}
#else
void ChartController::impl_initializeAccessible( AccessibleChartView& /* rAccChartView */) {}
#endif

const o3tl::sorted_vector< std::u16string_view >& ChartController::impl_getAvailableCommands()
{
    static const o3tl::sorted_vector< std::u16string_view > s_AvailableCommands {
        // commands for container forward
        u"AddDirect",           u"NewDoc",                u"Open",
        u"Save",                u"SaveAs",                u"SendMail",
        u"EditDoc",             u"ExportDirectToPDF",     u"PrintDefault",

        // own commands
        u"Cut",                u"Copy",                 u"Paste",
        u"DataRanges",         u"DiagramData",
        // insert objects
        u"InsertMenuTitles",   u"InsertTitles",
        u"InsertMenuLegend",   u"InsertLegend",         u"DeleteLegend",
        u"InsertMenuDataLabels",
        u"InsertMenuAxes",     u"InsertRemoveAxes",         u"InsertMenuGrids",
        u"InsertSymbol",
        u"InsertTrendlineEquation",  u"InsertTrendlineEquationAndR2",
        u"InsertR2Value",      u"DeleteR2Value",
        u"InsertMenuTrendlines",  u"InsertTrendline",
        u"InsertMenuMeanValues", u"InsertMeanValue",
        u"InsertMenuXErrorBars",  u"InsertXErrorBars",
        u"InsertMenuYErrorBars",   u"InsertYErrorBars",
        u"InsertDataLabels",   u"InsertDataLabel",
        u"DeleteTrendline",    u"DeleteMeanValue",      u"DeleteTrendlineEquation",
        u"DeleteXErrorBars",   u"DeleteYErrorBars",
        u"DeleteDataLabels",   u"DeleteDataLabel",
        u"InsertMenuDataTable",
        u"InsertDataTable", u"DeleteDataTable",
        //format objects
        u"FormatSelection",    u"FontDialog",           u"TransformDialog",
        u"DiagramType",        u"View3D",
        u"Forward",            u"Backward",
        u"MainTitle",          u"SubTitle",
        u"XTitle",             u"YTitle",               u"ZTitle",
        u"SecondaryXTitle",    u"SecondaryYTitle",
        u"AllTitles",          u"Legend",
        u"DiagramAxisX",       u"DiagramAxisY",         u"DiagramAxisZ",
        u"DiagramAxisA",       u"DiagramAxisB",         u"DiagramAxisAll",
        u"DiagramGridXMain",   u"DiagramGridYMain",     u"DiagramGridZMain",
        u"DiagramGridXHelp",   u"DiagramGridYHelp",     u"DiagramGridZHelp",
        u"DiagramGridAll",
        u"DiagramWall",        u"DiagramFloor",         u"DiagramArea",

        //context menu - format objects entries
        u"FormatWall",        u"FormatFloor",         u"FormatChartArea",
        u"FormatLegend",

        u"FormatAxis",           u"FormatTitle",
        u"FormatDataSeries",     u"FormatDataPoint",
        u"ResetAllDataPoints",   u"ResetDataPoint",
        u"FormatDataLabels",     u"FormatDataLabel",
        u"FormatMeanValue",      u"FormatTrendline",      u"FormatTrendlineEquation",
        u"FormatXErrorBars",     u"FormatYErrorBars",
        u"FormatStockLoss",      u"FormatStockGain",

        u"FormatMajorGrid",      u"InsertMajorGrid",      u"DeleteMajorGrid",
        u"FormatMinorGrid",      u"InsertMinorGrid",      u"DeleteMinorGrid",
        u"InsertAxis",           u"DeleteAxis",           u"InsertAxisTitle",

        // toolbar commands
        u"ToggleGridHorizontal", u"ToggleGridVertical", u"ToggleLegend",         u"ScaleText",
        u"NewArrangement",     u"Update",
        u"DefaultColors",      u"BarWidth",             u"NumberOfLines",
        u"ArrangeRow",
        u"StatusBarVisible",
        u"ChartElementSelector"};
    return s_AvailableCommands;
}

ViewElementListProvider ChartController::getViewElementListProvider()
{
    return ViewElementListProvider(m_pDrawModelWrapper.get());
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
