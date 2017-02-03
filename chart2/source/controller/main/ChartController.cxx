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

#include <sal/config.h>

#include <set>

#include "ChartController.hxx"
#include "servicenames.hxx"
#include "ResId.hxx"
#include "dlg_DataSource.hxx"
#include "ChartModelHelper.hxx"
#include "ControllerCommandDispatch.hxx"
#include "Strings.hrc"
#include "chartview/ExplicitValueProvider.hxx"
#include "ChartViewHelper.hxx"

#include "ChartWindow.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "DrawViewWrapper.hxx"
#include "ObjectIdentifier.hxx"
#include "DiagramHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "UndoGuard.hxx"
#include "ChartDropTargetHelper.hxx"

#include "macros.hxx"
#include "dlg_CreationWizard.hxx"
#include "dlg_ChartType.hxx"
#include "AccessibleChartView.hxx"
#include "DrawCommandDispatch.hxx"
#include "ShapeController.hxx"
#include "UndoActions.hxx"
#include "ViewElementListProvider.hxx"

#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/document/XUndoAction.hpp>
#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include <svx/sidebar/SelectionChangeHandler.hxx>
#include <vcl/msgbox.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <sfx2/sidebar/SidebarController.hxx>

#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>

// this is needed to properly destroy the unique_ptr to the AcceleratorExecute
// object in the DTOR
#include <svtools/acceleratorexecute.hxx>
#include <svx/ActionDescriptionProvider.hxx>
#include <tools/diagnose_ex.h>

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

ChartController::ChartController(uno::Reference<uno::XComponentContext> const & xContext) :
    m_aLifeTimeManager( nullptr ),
    m_bSuspended( false ),
    m_xCC(xContext), //@todo is it allowed to hold this context??
    m_xFrame( nullptr ),
    m_aModelMutex(),
    m_aModel( nullptr, m_aModelMutex ),
    m_pChartWindow( nullptr ),
    m_xViewWindow(),
    m_xChartView(),
    m_pDrawModelWrapper(),
    m_pDrawViewWrapper(nullptr),
    m_eDragMode(SdrDragMode::Move),
    m_bWaitingForDoubleClick(false),
    m_bWaitingForMouseUp(false),
    m_bConnectingToView(false),
    m_bDisposed(false),
    m_xUndoManager( nullptr ),
    m_aDispatchContainer( m_xCC, this ),
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

ChartController::TheModel::TheModel( const uno::Reference< frame::XModel > & xModel ) :
    m_xModel( xModel ),
    m_xCloseable( nullptr ),
    m_bOwnership( true )
{
    m_xCloseable =
        uno::Reference< util::XCloseable >( xModel, uno::UNO_QUERY );
}

ChartController::TheModel::~TheModel()
{
}

void ChartController::TheModel::addListener( ChartController* pController )
{
    if(m_xCloseable.is())
    {
        //if you need to be able to veto against the destruction of the model
        // you must add as a close listener

        //otherwise you 'can' add as closelistener or 'must' add as dispose event listener

        m_xCloseable->addCloseListener(
            static_cast<util::XCloseListener*>(pController) );
    }
    else if( m_xModel.is() )
    {
        //we need to add as dispose event listener
        m_xModel->addEventListener(
            static_cast<util::XCloseListener*>(pController) );
    }

}

void ChartController::TheModel::removeListener(  ChartController* pController )
{
    if(m_xCloseable.is())
        m_xCloseable->removeCloseListener(
            static_cast<util::XCloseListener*>(pController) );

    else if( m_xModel.is() )
        m_xModel->removeEventListener(
            static_cast<util::XCloseListener*>(pController) );
}

void ChartController::TheModel::tryTermination()
{
    if(!m_bOwnership)
        return;

    try
    {
        if(m_xCloseable.is())
        {
            try
            {
                //@todo ? are we allowed to use sal_True here if we have the explicit ownership?
                //I think yes, because there might be other closelistners later in the list which might be interested still
                //but make sure that we do not throw the CloseVetoException here ourselves
                //so stop listening before trying to terminate or check the source of queryclosing event
                m_xCloseable->close(true);

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
        else if( m_xModel.is() )
        {
            //@todo correct??
            m_xModel->dispose();
            return;
        }
    }
    catch(const uno::Exception& ex)
    {
        (void)(ex); // no warning in non-debug builds
        OSL_FAIL( OString( OString("Termination of model failed: ")
            + OUStringToOString( ex.Message, RTL_TEXTENCODING_ASCII_US ) ).getStr() );
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

css::uno::Reference<css::chart2::XChartType> getChartType(
        const css::uno::Reference<css::chart2::XChartDocument>& xChartDoc)
{
    Reference <chart2::XDiagram > xDiagram = xChartDoc->getFirstDiagram();
    if (!xDiagram.is()) {
        return css::uno::Reference<css::chart2::XChartType>();
    }

    Reference< chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY_THROW );

    Sequence< Reference< chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());

    Reference< chart2::XChartTypeContainer > xChartTypeContainer( xCooSysSequence[0], uno::UNO_QUERY_THROW );

    Sequence< Reference< chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );

    return xChartTypeSequence[0];
}

}

OUString ChartController::GetContextName()
{
    if (m_bDisposed)
        return OUString();

    uno::Any aAny = getSelection();
    if (!aAny.hasValue())
        return OUString("Chart");

    OUString aCID;
    aAny >>= aCID;

    if (aCID.isEmpty())
        return OUString("Chart");

    ObjectType eObjectID = ObjectIdentifier::getObjectType(aCID);
    switch (eObjectID)
    {
        case OBJECTTYPE_DATA_SERIES:
            return OUString("Series");
        break;
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
            return OUString("ErrorBar");
        case OBJECTTYPE_AXIS:
            return OUString("Axis");
        case OBJECTTYPE_GRID:
            return OUString("Grid");
        case OBJECTTYPE_DIAGRAM:
            {
                css::uno::Reference<css::chart2::XChartType> xChartType = getChartType(css::uno::Reference<css::chart2::XChartDocument>(getModel(), uno::UNO_QUERY));
                if (xChartType.is() && xChartType->getChartType() == "com.sun.star.chart2.PieChartType")
                    return OUString("ChartElements");
                break;
            }
        case OBJECTTYPE_DATA_CURVE:
        case OBJECTTYPE_DATA_AVERAGE_LINE:
            return OUString("Trendline");
        default:
        break;
    }

    return OUString("Chart");
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

// lang::XServiceInfo

OUString SAL_CALL ChartController::getImplementationName()
{
    return OUString(CHART_CONTROLLER_SERVICE_IMPLEMENTATION_NAME);
}

sal_Bool SAL_CALL ChartController::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ChartController::getSupportedServiceNames()
{
    return {
        CHART_CONTROLLER_SERVICE_NAME,
        "com.sun.star.frame.Controller"
        //// @todo : add additional services if you support any further
    };
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

    uno::Reference<ui::XSidebarProvider> xSidebarProvider (xController->getSidebar(), uno::UNO_QUERY);
    if (!xSidebarProvider.is())
        return nullptr;

    uno::Reference<ui::XSidebar> xSidebar(xSidebarProvider->getSidebar(), uno::UNO_QUERY);

    return xSidebar;
}

}

// XController

void SAL_CALL ChartController::attachFrame(
    const uno::Reference<frame::XFrame>& xFrame )
{
    SolarMutexGuard aGuard;

    if( impl_isDisposedOrSuspended() ) //@todo? allow attaching the frame while suspended?
        return; //behave passive if already disposed or suspended

    mpSelectionChangeHandler->Connect();

    uno::Reference<ui::XSidebar> xSidebar = getSidebarFromModel(getModel());
    if (xSidebar.is())
    {
        sfx2::sidebar::SidebarController* pSidebar = dynamic_cast<sfx2::sidebar::SidebarController*>(xSidebar.get());
        sfx2::sidebar::SidebarController::registerSidebarForFrame(pSidebar, this);
        pSidebar->updateModel(getModel());
        css::lang::EventObject aEvent;
        mpSelectionChangeHandler->selectionChanged(aEvent);
    }

    if(m_xFrame.is()) //what happens, if we do have a Frame already??
    {
        //@todo? throw exception?
        OSL_FAIL( "there is already a frame attached to the controller" );
        return;
    }

    //--attach frame
    m_xFrame = xFrame; //the frameloader is responsible to call xFrame->setComponent

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
        uno::Reference< awt::XWindow > xContainerWindow = xFrame->getContainerWindow();
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(xContainerWindow);
        assert(pParentComponent);
        if (pParentComponent)
            pParentComponent->setVisible(true);

        pParent = VCLUnoHelper::GetWindow( xContainerWindow ).get();
    }

    if(m_pChartWindow)
    {
        //@todo delete ...
        m_pChartWindow->clear();
        m_apDropTargetHelper.reset();
    }
    {
        // calls to VCL
        SolarMutexGuard aSolarGuard;
        m_pChartWindow = VclPtr<ChartWindow>::Create(this,pParent,pParent?pParent->GetStyle():0);
        m_pChartWindow->SetBackground();//no Background
        m_xViewWindow.set( m_pChartWindow->GetComponentInterface(), uno::UNO_QUERY );
        m_pChartWindow->Show();
        m_apDropTargetHelper.reset(
            new ChartDropTargetHelper( m_pChartWindow->GetDropTarget(),
                                       uno::Reference< chart2::XChartDocument >( getModel(), uno::UNO_QUERY )));

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
                xPropSet->getPropertyValue( "LayoutManager" ) >>= xLayoutManager;
                if ( xLayoutManager.is() )
                {
                    xLayoutManager->lock();
                    xLayoutManager->requestElement( "private:resource/menubar/menubar" );
                    //@todo: createElement should become unnecessary, remove when #i79198# is fixed
                    xLayoutManager->createElement( "private:resource/toolbar/standardbar" );
                    xLayoutManager->requestElement( "private:resource/toolbar/standardbar" );
                    //@todo: createElement should become unnecessary, remove when #i79198# is fixed
                    xLayoutManager->createElement( "private:resource/toolbar/toolbar" );
                    xLayoutManager->requestElement( "private:resource/toolbar/toolbar" );

                    // #i12587# support for shapes in chart
                    xLayoutManager->createElement( "private:resource/toolbar/drawbar" );
                    xLayoutManager->requestElement( "private:resource/toolbar/drawbar" );

                    xLayoutManager->requestElement( "private:resource/statusbar/statusbar" );
                    xLayoutManager->unlock();

                    // add as listener to get notified when
                    m_xLayoutManagerEventBroadcaster.set( xLayoutManager, uno::UNO_QUERY );
                    if( m_xLayoutManagerEventBroadcaster.is())
                        m_xLayoutManagerEventBroadcaster->addLayoutManagerEventListener( this );
                }
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

//XModeChangeListener
void SAL_CALL ChartController::modeChanged( const util::ModeChangeEvent& rEvent )
{
    //adjust controller to view status changes

    if( rEvent.NewMode == "dirty" )
    {
        //the view has become dirty, we should repaint it if we have a window
        SolarMutexGuard aGuard;
        if( m_pChartWindow )
            m_pChartWindow->ForceInvalidate();
    }
    else if( rEvent.NewMode == "invalid" )
    {
        //the view is about to become invalid so end all actions on it
        impl_invalidateAccessible();
        SolarMutexGuard aGuard;
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
            if(m_pChartWindow && m_aModel.is() )
            {
                m_bConnectingToView = true;

                GetDrawModelWrapper();
                if(m_pDrawModelWrapper)
                {
                    {
                        SolarMutexGuard aGuard;
                        if( m_pDrawViewWrapper )
                            m_pDrawViewWrapper->ReInit();
                    }

                    //reselect object
                    if( m_aSelection.hasSelection() )
                        this->impl_selectObjectAndNotiy();
                    else
                        ChartModelHelper::triggerRangeHighlighting( getModel() );

                    impl_initializeAccessible();

                    {
                        SolarMutexGuard aGuard;
                        if( m_pChartWindow )
                            m_pChartWindow->Invalidate();
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

    TheModelRef aNewModelRef( new TheModel( xModel), m_aModelMutex);
    TheModelRef aOldModelRef(m_aModel,m_aModelMutex);
    m_aModel = aNewModelRef;

    //--handle relations to the old model if any
    if( aOldModelRef.is() )
    {
        uno::Reference< util::XModeChangeBroadcaster > xViewBroadcaster( m_xChartView, uno::UNO_QUERY );
        if( xViewBroadcaster.is() )
            xViewBroadcaster->removeModeChangeListener(this);
        m_pDrawModelWrapper.reset();

        aOldModelRef->removeListener( this );
 #ifdef TEST_ENABLE_MODIFY_LISTENER
        uno::Reference< util::XModifyBroadcaster > xMBroadcaster( aOldModelRef->getModel(),uno::UNO_QUERY );
        if( xMBroadcaster.is())
            xMBroadcaster->removeModifyListener( this );
#endif
    }

    //--handle relations to the new model
    aNewModelRef->addListener( this );

    aGuard.reset(); // lock for m_aDispatchContainer access
    // set new model at dispatchers
    m_aDispatchContainer.setModel( aNewModelRef->getModel());
    ControllerCommandDispatch * pDispatch = new ControllerCommandDispatch( m_xCC, this, &m_aDispatchContainer );
    pDispatch->initialize();

    // the dispatch container will return "this" for all commands returned by
    // impl_getAvailableCommands().  That means, for those commands dispatch()
    // is called here at the ChartController.
    m_aDispatchContainer.setChartDispatch( pDispatch, impl_getAvailableCommands() );

    DrawCommandDispatch* pDrawDispatch = new DrawCommandDispatch( m_xCC, this );
    pDrawDispatch->initialize();
    m_aDispatchContainer.setDrawCommandDispatch( pDrawDispatch );

    ShapeController* pShapeController = new ShapeController( m_xCC, this );
    pShapeController->initialize();
    m_aDispatchContainer.setShapeController( pShapeController );
    aGuard.clear();

#ifdef TEST_ENABLE_MODIFY_LISTENER
    uno::Reference< util::XModifyBroadcaster > xMBroadcaster( aNewModelRef->getModel(),uno::UNO_QUERY );
    if( xMBroadcaster.is())
        xMBroadcaster->addModifyListener( this );
#endif

    // #i119999# Do not do this per default to allow the user to deselect the chart OLE with a single press to ESC
    // select chart area per default:
    // select( uno::Any( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, OUString() ) ) );

    uno::Reference< lang::XMultiServiceFactory > xFact( getModel(), uno::UNO_QUERY );
    if( xFact.is())
    {
        m_xChartView = xFact->createInstance( CHART_VIEW_SERVICE_NAME );
        GetDrawModelWrapper();
        uno::Reference< util::XModeChangeBroadcaster > xViewBroadcaster( m_xChartView, uno::UNO_QUERY );
        if( xViewBroadcaster.is() )
            xViewBroadcaster->addModeChangeListener(this);
    }

    //the frameloader is responsible to call xModel->connectController
    {
        SolarMutexGuard aGuard2;
        if( m_pChartWindow )
            m_pChartWindow->Invalidate();
    }

    uno::Reference< document::XUndoManagerSupplier > xSuppUndo( getModel(), uno::UNO_QUERY_THROW );
    m_xUndoManager.set( xSuppUndo->getUndoManager(), uno::UNO_QUERY_THROW );

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
    //provides access to currently attached model
    //returns the currently attached model

    //return nothing, if you do not have a model
    TheModelRef aModelRef( m_aModel, m_aModelMutex);
    if(aModelRef.is())
        return aModelRef->getModel();

    return uno::Reference< frame::XModel > ();
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
    if(bSuspend)
    {
        m_bSuspended = bSuspend;
        return true;
    }
    else
    {
        m_bSuspended = bSuspend;
    }
    return true;
}

void ChartController::impl_createDrawViewController()
{
    SolarMutexGuard aGuard;
    if(!m_pDrawViewWrapper)
    {
        if( m_pDrawModelWrapper )
        {
            m_pDrawViewWrapper = new DrawViewWrapper(&m_pDrawModelWrapper->getSdrModel(),m_pChartWindow);
            m_pDrawViewWrapper->attachParentReferenceDevice( getModel() );
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
        DELETEZ( m_pDrawViewWrapper );
    }
}

// XComponent (base of XController)

void SAL_CALL ChartController::dispose()
{
    m_bDisposed = true;

    if (getModel().is())
    {
        uno::Reference<ui::XSidebar> xSidebar = getSidebarFromModel(getModel());
        if (sfx2::sidebar::SidebarController* pSidebar = dynamic_cast<sfx2::sidebar::SidebarController*>(xSidebar.get()))
        {
            sfx2::sidebar::SidebarController::unregisterSidebarForFrame(pSidebar, this);
        }
    }
    mpSelectionChangeHandler->selectionChanged(css::lang::EventObject());
    mpSelectionChangeHandler->Disconnect();

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
            uno::Reference< chart2::data::XDataReceiver > xDataReceiver( getModel(), uno::UNO_QUERY );
            if( xDataReceiver.is() )
                xSelectionChangeListener.set( xDataReceiver->getRangeHighlighter(), uno::UNO_QUERY );
            if( xSelectionChangeListener.is() )
            {
                uno::Reference< frame::XController > xController( this );
                uno::Reference< lang::XComponent > xComp( xController, uno::UNO_QUERY );
                lang::EventObject aEvent( xComp );
                xSelectionChangeListener->disposing( aEvent );
            }
        }

        //--release all resources and references
        {
            uno::Reference< chart2::X3DChartWindowProvider > x3DWindowProvider(getModel(), uno::UNO_QUERY_THROW);
            x3DWindowProvider->setWindow(0);

            uno::Reference< util::XModeChangeBroadcaster > xViewBroadcaster( m_xChartView, uno::UNO_QUERY );
            if( xViewBroadcaster.is() )
                xViewBroadcaster->removeModeChangeListener(this);

            impl_invalidateAccessible();
            SolarMutexGuard aSolarGuard;
            impl_deleteDrawViewController();
            m_pDrawModelWrapper.reset();

            m_apDropTargetHelper.reset();

            //the accessible view is disposed within window destructor of m_pChartWindow
            m_pChartWindow->clear();
            m_pChartWindow = nullptr;//m_pChartWindow is deleted via UNO due to dispose of m_xViewWindow (triggered by Framework (Controller pretends to be XWindow also))
            m_xViewWindow->dispose();
            m_xChartView.clear();
        }

        // remove as listener to layout manager events
        if( m_xLayoutManagerEventBroadcaster.is())
        {
            m_xLayoutManagerEventBroadcaster->removeLayoutManagerEventListener( this );
            m_xLayoutManagerEventBroadcaster.set( nullptr );
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
                uno::Reference< util::XModifyBroadcaster > xMBroadcaster( aModelRef->getModel(),uno::UNO_QUERY );
                if( xMBroadcaster.is())
                    xMBroadcaster->removeModifyListener( this );
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
#endif
            aModelRef->tryTermination();
        }

        //// @todo integrate specialized implementation
        //e.g. release further resources and references

        SolarMutexGuard g;
        m_aDispatchContainer.DisposeAndClear();
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
 }

void SAL_CALL ChartController::addEventListener(
    const uno::Reference<lang::XEventListener>& xListener )
{
    SolarMutexGuard aGuard;
    if( impl_isDisposedOrSuspended() )//@todo? allow adding of listeners in suspend mode?
        return; //behave passive if already disposed or suspended

    //--add listener
    m_aLifeTimeManager.m_aListenerContainer.addInterface( cppu::UnoType<lang::XEventListener>::get(), xListener );
}

void SAL_CALL ChartController::removeEventListener(
    const uno::Reference<lang::XEventListener>& xListener )
{
    SolarMutexGuard aGuard;
    if( m_aLifeTimeManager.impl_isDisposed(false) )
        return; //behave passive if already disposed or suspended

    //--remove listener
    m_aLifeTimeManager.m_aListenerContainer.removeInterface( cppu::UnoType<lang::XEventListener>::get(), xListener );
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

    if( !(aModelRef->getModel() == rSource.Source) )
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
    if( impl_releaseThisModel( rSource.Source ) )
    {
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
}

bool ChartController::impl_releaseThisModel(
    const uno::Reference< uno::XInterface > & xModel )
{
    bool bReleaseModel = false;
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aModelMutex );
        if( m_aModel.is() && m_aModel->getModel() == xModel )
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
            m_xLayoutManagerEventBroadcaster.set( nullptr );
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
            xLM->createElement(  "private:resource/statusbar/statusbar" );
            xLM->requestElement( "private:resource/statusbar/statusbar" );
        }
    }
}

// XDispatchProvider (required interface)

namespace
{

bool lcl_isFormatObjectCommand( const OUString& aCommand )
{
    if(    aCommand == "MainTitle"
        || aCommand == "SubTitle"
        || aCommand == "XTitle"
        || aCommand == "YTitle"
        || aCommand == "ZTitle"
        || aCommand == "SecondaryXTitle"
        || aCommand == "SecondaryYTitle"
        || aCommand == "AllTitles"
        || aCommand == "DiagramAxisX"
        || aCommand == "DiagramAxisY"
        || aCommand == "DiagramAxisZ"
        || aCommand == "DiagramAxisA"
        || aCommand == "DiagramAxisB"
        || aCommand == "DiagramAxisAll"
        || aCommand == "DiagramGridXMain"
        || aCommand == "DiagramGridYMain"
        || aCommand == "DiagramGridZMain"
        || aCommand == "DiagramGridXHelp"
        || aCommand == "DiagramGridYHelp"
        || aCommand == "DiagramGridZHelp"
        || aCommand == "DiagramGridAll"

        || aCommand == "DiagramWall"
        || aCommand == "DiagramFloor"
        || aCommand == "DiagramArea"
        || aCommand == "Legend"

        || aCommand == "FormatWall"
        || aCommand == "FormatFloor"
        || aCommand == "FormatChartArea"
        || aCommand == "FormatLegend"

        || aCommand == "FormatTitle"
        || aCommand == "FormatAxis"
        || aCommand == "FormatDataSeries"
        || aCommand == "FormatDataPoint"
        || aCommand == "FormatDataLabels"
        || aCommand == "FormatDataLabel"
        || aCommand == "FormatXErrorBars"
        || aCommand == "FormatYErrorBars"
        || aCommand == "FormatMeanValue"
        || aCommand == "FormatTrendline"
        || aCommand == "FormatTrendlineEquation"
        || aCommand == "FormatStockLoss"
        || aCommand == "FormatStockGain"
        || aCommand == "FormatMajorGrid"
        || aCommand == "FormatMinorGrid"
        )
        return true;

    // else
    return false;
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

    if(aCommand == "Paste")
        this->executeDispatch_Paste();
    else if(aCommand == "Copy" )
        this->executeDispatch_Copy();
    else if(aCommand == "Cut" )
        this->executeDispatch_Cut();
    else if(aCommand == "DataRanges" )
        this->executeDispatch_SourceData();
    else if(aCommand == "Update" ) //Update Chart
    {
        ChartViewHelper::setViewToDirtyState( getModel() );
        SolarMutexGuard aGuard;
        if( m_pChartWindow )
            m_pChartWindow->Invalidate();
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
    else if( lcl_isFormatObjectCommand(aCommand) )
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
            xPropSet->getPropertyValue( "LayoutManager" ) >>= xLayoutManager;
            if ( xLayoutManager.is() )
            {
                bool bIsVisible( xLayoutManager->isElementVisible( "private:resource/statusbar/statusbar" ));
                if( bIsVisible )
                {
                    xLayoutManager->hideElement( "private:resource/statusbar/statusbar" );
                    xLayoutManager->destroyElement( "private:resource/statusbar/statusbar" );
                }
                else
                {
                    xLayoutManager->createElement( "private:resource/statusbar/statusbar" );
                    xLayoutManager->showElement( "private:resource/statusbar/statusbar" );
                }
                // @todo: update menu state (checkmark next to "Statusbar").
            }
        }
    }
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
    UndoLiveUpdateGuard aUndoGuard(
        SCH_RESSTR( STR_ACTION_EDIT_CHARTTYPE ), m_xUndoManager );

    SolarMutexGuard aSolarGuard;
    //prepare and open dialog
    ScopedVclPtrInstance< ChartTypeDialog > aDlg( m_pChartWindow, getModel() );
    if( aDlg->Execute() == RET_OK )
    {
        impl_adaptDataSeriesAutoResize();
        aUndoGuard.commit();
    }
}

void ChartController::executeDispatch_SourceData()
{
    //convert properties to ItemSet
    uno::Reference< XChartDocument >   xChartDoc( getModel(), uno::UNO_QUERY );
    OSL_ENSURE( xChartDoc.is(), "Invalid XChartDocument" );
    if( !xChartDoc.is())
        return;

    UndoLiveUpdateGuard aUndoGuard(
        SCH_RESSTR(STR_ACTION_EDIT_DATA_RANGES), m_xUndoManager );
    if( xChartDoc.is())
    {
        SolarMutexGuard aSolarGuard;
        ScopedVclPtrInstance< ::chart::DataSourceDialog > aDlg( m_pChartWindow, xChartDoc, m_xCC );
        if( aDlg->Execute() == RET_OK )
        {
            impl_adaptDataSeriesAutoResize();
            aUndoGuard.commit();
        }
    }
}

void ChartController::executeDispatch_MoveSeries( bool bForward )
{
    ControllerLockGuardUNO aCLGuard( getModel() );

    //get selected series
    OUString aObjectCID(m_aSelection.getSelectedCID());
    uno::Reference< XDataSeries > xGivenDataSeries( ObjectIdentifier::getDataSeriesForCID( //yyy todo also legend entries and labels?
            aObjectCID, getModel() ) );

    UndoGuardWithSelection aUndoGuard(
        ActionDescriptionProvider::createDescription(
            (bForward ? ActionDescriptionProvider::ActionType::MoveToTop : ActionDescriptionProvider::ActionType::MoveToBottom),
            SCH_RESSTR(STR_OBJECT_DATASERIES)),
        m_xUndoManager );

    bool bChanged = DiagramHelper::moveSeries( ChartModelHelper::findDiagram( getModel() ), xGivenDataSeries, bForward );
    if( bChanged )
    {
        m_aSelection.setSelection( ObjectIdentifier::getMovedSeriesCID( aObjectCID, bForward ) );
        aUndoGuard.commit();
    }
}

// ____ XMultiServiceFactory ____
uno::Reference< uno::XInterface > SAL_CALL
    ChartController::createInstance( const OUString& aServiceSpecifier )
{
    uno::Reference< uno::XInterface > xResult;

    if( aServiceSpecifier == CHART_ACCESSIBLE_TEXT_SERVICE_NAME )
        xResult.set( impl_createAccessibleTextContext());
    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL
    ChartController::createInstanceWithArguments(
        const OUString& ServiceSpecifier,
        const uno::Sequence< uno::Any >& /* Arguments */ )
{
    // ignore Arguments
    return createInstance( ServiceSpecifier );
}

uno::Sequence< OUString > SAL_CALL
    ChartController::getAvailableServiceNames()
{
    uno::Sequence< OUString > aServiceNames { CHART_ACCESSIBLE_TEXT_SERVICE_NAME };
    return aServiceNames;
}

// ____ XModifyListener ____
void SAL_CALL ChartController::modified(
    const lang::EventObject& /* aEvent */ )
{
    // the source can also be a subobject of the ChartModel
    // @todo: change the source in ChartModel to always be the model itself ?
    //todo? update menu states ?
}

IMPL_LINK( ChartController, NotifyUndoActionHdl, SdrUndoAction*, pUndoAction, void )
{
    ENSURE_OR_RETURN_VOID( pUndoAction, "invalid Undo action" );

    OUString aObjectCID = m_aSelection.getSelectedCID();
    if ( aObjectCID.isEmpty() )
    {
        try
        {
            const Reference< document::XUndoManagerSupplier > xSuppUndo( getModel(), uno::UNO_QUERY_THROW );
            const Reference< document::XUndoManager > xUndoManager( xSuppUndo->getUndoManager(), uno::UNO_QUERY_THROW );
            const Reference< document::XUndoAction > xAction( new impl::ShapeUndoElement( *pUndoAction ) );
            xUndoManager->addUndoAction( xAction );
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

DrawModelWrapper* ChartController::GetDrawModelWrapper()
{
    if( !m_pDrawModelWrapper.get() )
    {
        ExplicitValueProvider* pProvider = ExplicitValueProvider::getExplicitValueProvider( m_xChartView );
        if( pProvider )
            m_pDrawModelWrapper = pProvider->getDrawModelWrapper();
        if ( m_pDrawModelWrapper.get() )
        {
            m_pDrawModelWrapper->getSdrModel().SetNotifyUndoActionHdl( LINK( this, ChartController, NotifyUndoActionHdl ) );
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
    return m_pDrawViewWrapper;
}

uno::Reference< XAccessible > ChartController::CreateAccessible()
{
    uno::Reference< XAccessible > xResult = new AccessibleChartView( GetDrawViewWrapper() );
    impl_initializeAccessible( uno::Reference< lang::XInitialization >( xResult, uno::UNO_QUERY ) );
    return xResult;
}

void ChartController::impl_invalidateAccessible()
{
    SolarMutexGuard aGuard;
    if( m_pChartWindow )
    {
        Reference< lang::XInitialization > xInit( m_pChartWindow->GetAccessible(false), uno::UNO_QUERY );
        if(xInit.is())
        {
            uno::Sequence< uno::Any > aArguments(3);//empty arguments -> invalid accessible
            xInit->initialize(aArguments);
        }
    }
}
void ChartController::impl_initializeAccessible()
{
    SolarMutexGuard aGuard;
    if( m_pChartWindow )
        this->impl_initializeAccessible( Reference< lang::XInitialization >( m_pChartWindow->GetAccessible(false), uno::UNO_QUERY ) );
}
void ChartController::impl_initializeAccessible( const uno::Reference< lang::XInitialization >& xInit )
{
    if(xInit.is())
    {
        uno::Sequence< uno::Any > aArguments(5);
        aArguments[0] <<= uno::Reference<view::XSelectionSupplier>(this);
        aArguments[1] <<= uno::Reference<frame::XModel>(getModel());
        aArguments[2] <<= m_xChartView;
        uno::Reference< XAccessible > xParent;
        {
            SolarMutexGuard aGuard;
            if( m_pChartWindow )
            {
                vcl::Window* pParentWin( m_pChartWindow->GetAccessibleParentWindow());
                if( pParentWin )
                    xParent.set( pParentWin->GetAccessible());
            }
        }
        aArguments[3] <<= xParent;
        aArguments[4] <<= m_xViewWindow;

        xInit->initialize(aArguments);
    }
}

const ::std::set< OUString >& ChartController::impl_getAvailableCommands()
{
    static ::std::set< OUString > s_AvailableCommands {
        // commands for container forward
        "AddDirect",           "NewDoc",                "Open",
        "Save",                "SaveAs",                "SendMail",
        "EditDoc",             "ExportDirectToPDF",     "PrintDefault",

        // own commands
        "Cut",                "Copy",                 "Paste",
        "DataRanges",         "DiagramData",
        // insert objects
        "InsertMenuTitles",   "InsertTitles",
        "InsertMenuLegend",   "InsertLegend",         "DeleteLegend",
        "InsertMenuDataLabels",
        "InsertMenuAxes",     "InsertRemoveAxes",         "InsertMenuGrids",
        "InsertSymbol",
        "InsertTrendlineEquation",  "InsertTrendlineEquationAndR2",
        "InsertR2Value",      "DeleteR2Value",
        "InsertMenuTrendlines",  "InsertTrendline",
        "InsertMenuMeanValues", "InsertMeanValue",
        "InsertMenuXErrorBars",  "InsertXErrorBars",
        "InsertMenuYErrorBars",   "InsertYErrorBars",
        "InsertDataLabels",   "InsertDataLabel",
        "DeleteTrendline",    "DeleteMeanValue",      "DeleteTrendlineEquation",
        "DeleteXErrorBars",   "DeleteYErrorBars",
        "DeleteDataLabels",   "DeleteDataLabel",
        //format objects
        "FormatSelection",     "TransformDialog",
        "DiagramType",        "View3D",
        "Forward",            "Backward",
        "MainTitle",          "SubTitle",
        "XTitle",             "YTitle",               "ZTitle",
        "SecondaryXTitle",    "SecondaryYTitle",
        "AllTitles",          "Legend",
        "DiagramAxisX",       "DiagramAxisY",         "DiagramAxisZ",
        "DiagramAxisA",       "DiagramAxisB",         "DiagramAxisAll",
        "DiagramGridXMain",   "DiagramGridYMain",     "DiagramGridZMain",
        "DiagramGridXHelp",   "DiagramGridYHelp",     "DiagramGridZHelp",
        "DiagramGridAll",
        "DiagramWall",        "DiagramFloor",         "DiagramArea",

        //context menu - format objects entries
        "FormatWall",        "FormatFloor",         "FormatChartArea",
        "FormatLegend",

        "FormatAxis",           "FormatTitle",
        "FormatDataSeries",     "FormatDataPoint",
        "ResetAllDataPoints",   "ResetDataPoint",
        "FormatDataLabels",     "FormatDataLabel",
        "FormatMeanValue",      "FormatTrendline",      "FormatTrendlineEquation",
        "FormatXErrorBars",     "FormatYErrorBars",
        "FormatStockLoss",      "FormatStockGain",

        "FormatMajorGrid",      "InsertMajorGrid",      "DeleteMajorGrid",
        "FormatMinorGrid",      "InsertMinorGrid",      "DeleteMinorGrid",
        "InsertAxis",           "DeleteAxis",           "InsertAxisTitle",

        // toolbar commands
        "ToggleGridHorizontal", "ToggleGridVertical", "ToggleLegend",         "ScaleText",
        "NewArrangement",     "Update",
        "DefaultColors",      "BarWidth",             "NumberOfLines",
        "ArrangeRow",
        "StatusBarVisible",
        "ChartElementSelector"};
    return s_AvailableCommands;
}

ViewElementListProvider ChartController::getViewElementListProvider()
{
    return ViewElementListProvider(m_pDrawModelWrapper.get());
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_ChartController_get_implementation(css::uno::XComponentContext *context,
                                                            css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::ChartController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
