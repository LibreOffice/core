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
#include "precompiled_chart2.hxx"
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

#include <comphelper/InlineContainer.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/document/XUndoAction.hpp>

//-------
// header for define RET_OK
#include <vcl/msgbox.hxx>
//-------

//-------
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
//-------
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>

// this is needed to properly destroy the auto_ptr to the AcceleratorExecute
// object in the DTOR
#include <svtools/acceleratorexecute.hxx>
#include <svx/ActionDescriptionProvider.hxx>
#include <tools/diagnose_ex.h>

// enable the following define to let the controller listen to model changes and
// react on this by rebuilding the view
#define TEST_ENABLE_MODIFY_LISTENER

/*
#include <vcl/svapp.hxx>
*/

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
DBG_NAME(ChartController)
//-----------------------------------------------------------------
// ChartController Constructor and Destructor
//-----------------------------------------------------------------

ChartController::ChartController(uno::Reference<uno::XComponentContext> const & xContext)
    : m_aLifeTimeManager( NULL )
    , m_bSuspended( sal_False )
    , m_bCanClose( sal_True )
    , m_xCC(xContext) //@todo is it allowed to hold this context??
    , m_xFrame( NULL )
    , m_aModelMutex()
    , m_aModel( NULL, m_aModelMutex )
    , m_pChartWindow( NULL )
    , m_xViewWindow()
    , m_xChartView()
    , m_pDrawModelWrapper()
    , m_pDrawViewWrapper(NULL)
    , m_eDragMode(SDRDRAG_MOVE)
    , m_bWaitingForDoubleClick(false)
    , m_bWaitingForMouseUp(false)
    , m_bConnectingToView(false)
    , m_xUndoManager( 0 )
    , m_aDispatchContainer( m_xCC, this )
    , m_eDrawMode( CHARTDRAW_SELECT )
{
    DBG_CTOR(ChartController,NULL);
    m_aDoubleClickTimer.SetTimeoutHdl( LINK( this, ChartController, DoubleClickWaitingHdl ) );
}

ChartController::~ChartController()
{
    DBG_DTOR(ChartController,NULL);
    stopDoubleClickWaiting();
}

//-----------------------------------------------------------------

ChartController::RefCountable::RefCountable() : m_nRefCount(0)
{
}

ChartController::RefCountable::~RefCountable()
{
}
void ChartController::RefCountable::acquire()
{
    m_nRefCount++;
}
void ChartController::RefCountable::release()
{
    m_nRefCount--;
    if(!m_nRefCount)
        delete this;
}

//-----------------------------------------------------------------

ChartController::TheModel::TheModel( const uno::Reference< frame::XModel > & xModel )
    : m_xModel( xModel )
    , m_xCloseable( NULL )
    , m_bOwnership( sal_True )
    , m_bOwnershipIsWellKnown( sal_False )
{
    m_xCloseable =
        uno::Reference< util::XCloseable >( xModel, uno::UNO_QUERY );
}

ChartController::TheModel::~TheModel()
{
}

void ChartController::TheModel::SetOwnerShip( sal_Bool bGetsOwnership )
{
    m_bOwnership                = bGetsOwnership;
    m_bOwnershipIsWellKnown = sal_True;
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
                //but make sure that we do not throw the CloseVetoException here ourselfs
                //so stop listening before trying to terminate or check the source of queryclosing event
                m_xCloseable->close(sal_True);

                m_bOwnership                = false;
                m_bOwnershipIsWellKnown = sal_True;
            }
            catch( util::CloseVetoException& )
            {
                //since we have indicated to give up the ownership with paramter true in close call
                //the one who has thrown the CloseVetoException is the new owner

#if OSL_DEBUG_LEVEL > 2
                OSL_ENSURE( !m_bOwnership,
                    "INFO: a well known owner has catched a CloseVetoException after calling close(true)" );
#endif

                m_bOwnership                = false;
                m_bOwnershipIsWellKnown = sal_True;
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
    catch( uno::Exception& ex)
    {
        (void)(ex); // no warning in non-debug builds
        OSL_ENSURE( sal_False, ( rtl::OString("Termination of model failed: ")
            + rtl::OUStringToOString( ex.Message, RTL_TEXTENCODING_ASCII_US ) ).getStr() );
    }
}

//-----------------------------------------------------------------

ChartController::TheModelRef::TheModelRef( TheModel* pTheModel, ::osl::Mutex& rMutex )
        : m_pTheModel(pTheModel), m_rModelMutex(rMutex)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_rModelMutex );
    if(m_pTheModel)
        m_pTheModel->acquire();
}
ChartController::TheModelRef::TheModelRef( const TheModelRef& rTheModel, ::osl::Mutex& rMutex )
        : m_rModelMutex(rMutex)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_rModelMutex );
    m_pTheModel=rTheModel.operator->();
    if(m_pTheModel)
        m_pTheModel->acquire();
}
ChartController::TheModelRef& ChartController::TheModelRef::operator=(TheModel* pTheModel)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_rModelMutex );
    if(m_pTheModel==pTheModel)
        return *this;
    if(m_pTheModel)
        m_pTheModel->release();
    m_pTheModel=pTheModel;
    if(m_pTheModel)
        m_pTheModel->acquire();
    return *this;
}
ChartController::TheModelRef& ChartController::TheModelRef::operator=(const TheModelRef& rTheModel)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_rModelMutex );
    TheModel* pNew=rTheModel.operator->();
    if(m_pTheModel==pNew)
        return *this;
    if(m_pTheModel)
        m_pTheModel->release();
    m_pTheModel=pNew;
    if(m_pTheModel)
        m_pTheModel->acquire();
    return *this;
}
ChartController::TheModelRef::~TheModelRef()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_rModelMutex );
    if(m_pTheModel)
        m_pTheModel->release();
}
sal_Bool ChartController::TheModelRef::is() const
{
    return (m_pTheModel != 0);
}


//-----------------------------------------------------------------
// private methods
//-----------------------------------------------------------------

    sal_Bool ChartController
::impl_isDisposedOrSuspended() const
{
    if( m_aLifeTimeManager.impl_isDisposed() )
        return sal_True;

    if( m_bSuspended )
    {
        OSL_ENSURE( sal_False, "This Controller is suspended" );
        return sal_True;
    }
    return sal_False;
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------

APPHELPER_XSERVICEINFO_IMPL(ChartController,CHART_CONTROLLER_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > ChartController
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 2 );
    aSNS.getArray()[ 0 ] = CHART_CONTROLLER_SERVICE_NAME;
    aSNS.getArray()[ 1 ] = ::rtl::OUString::createFromAscii("com.sun.star.frame.Controller");
    //// @todo : add additional services if you support any further
    return aSNS;
}

//-----------------------------------------------------------------
// XController
//-----------------------------------------------------------------

        void SAL_CALL ChartController
::attachFrame( const uno::Reference<frame::XFrame>& xFrame )
        throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());

    if( impl_isDisposedOrSuspended() ) //@todo? allow attaching the frame while suspended?
        return; //behave passive if already disposed or suspended

    if(m_xFrame.is()) //what happens, if we do have a Frame already??
    {
        //@todo? throw exception?
        OSL_ENSURE( sal_False, "there is already a frame attached to the controller" );
        return;
    }

    //--attach frame
    m_xFrame = xFrame; //the frameloader is responsible to call xFrame->setComponent

    //add as disposelistener to the frame (due to persistent reference) ??...:

    //the frame is considered to be owner of this controller and will live longer than we do
    //the frame or the disposer of the frame has the duty to call suspend and dispose on this object
    //so we do not need to add as lang::XEventListener for DisposingEvents right?

    //@todo nothing right???



    //--------------------------------------------------
    //create view @todo is this the correct place here??

    Window* pParent = NULL;
    //get the window parent from the frame to use as parent for our new window
    if(xFrame.is())
    {
        uno::Reference< awt::XWindow > xContainerWindow = xFrame->getContainerWindow();
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(xContainerWindow);
        pParentComponent->setVisible(sal_True);

        pParent = VCLUnoHelper::GetWindow( xContainerWindow );
    }

    if(m_pChartWindow)
    {
        //@todo delete ...
        m_pChartWindow->clear();
        m_apDropTargetHelper.reset();
    }
    {
        awt::Size aPageSize( ChartModelHelper::getPageSize(getModel()) );

        // calls to VCL
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        m_pChartWindow = new ChartWindow(this,pParent,pParent?pParent->GetStyle():0);
        m_pChartWindow->SetBackground();//no Background
        m_xViewWindow = uno::Reference< awt::XWindow >( m_pChartWindow->GetComponentInterface(), uno::UNO_QUERY );
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
                uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                xPropSet->getPropertyValue( C2U( "LayoutManager" ) ) >>= xLayoutManager;
                if ( xLayoutManager.is() )
                {
                    xLayoutManager->lock();
                    xLayoutManager->requestElement( C2U( "private:resource/menubar/menubar" ) );
                    //@todo: createElement should become unnecessary, remove when #i79198# is fixed
                    xLayoutManager->createElement(  C2U( "private:resource/toolbar/standardbar" ) );
                    xLayoutManager->requestElement( C2U( "private:resource/toolbar/standardbar" ) );
                    //@todo: createElement should become unnecessary, remove when #i79198# is fixed
                    xLayoutManager->createElement(  C2U( "private:resource/toolbar/toolbar" ) );
                    xLayoutManager->requestElement( C2U( "private:resource/toolbar/toolbar" ) );

                    // #i12587# support for shapes in chart
                    xLayoutManager->createElement(  C2U( "private:resource/toolbar/drawbar" ) );
                    xLayoutManager->requestElement( C2U( "private:resource/toolbar/drawbar" ) );

                    xLayoutManager->requestElement( C2U( "private:resource/statusbar/statusbar" ) );
                    xLayoutManager->unlock();

                    // add as listener to get notified when
                    m_xLayoutManagerEventBroadcaster.set( xLayoutManager, uno::UNO_QUERY );
                    if( m_xLayoutManagerEventBroadcaster.is())
                        m_xLayoutManagerEventBroadcaster->addLayoutManagerEventListener( this );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

//XModeChangeListener
void SAL_CALL ChartController::modeChanged( const util::ModeChangeEvent& rEvent )
    throw ( uno::RuntimeException )
{
    //adjust controller to view status changes

    if( rEvent.NewMode.equals(C2U("dirty")) )
    {
        //the view has become dirty, we should repaint it if we have a window
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if( m_pChartWindow )
            m_pChartWindow->ForceInvalidate();
    }
    else if( rEvent.NewMode.equals(C2U("invalid")) )
    {
        //the view is about to become invalid so end all actions on it
        impl_invalidateAccessible();
        ::vos::OGuard aGuard( Application::GetSolarMutex());
        if( m_pDrawViewWrapper && m_pDrawViewWrapper->IsTextEdit() )
            this->EndTextEdit();
        if( m_pDrawViewWrapper )
        {
            m_pDrawViewWrapper->UnmarkAll();
            //m_pDrawViewWrapper->hideMarkHandles(); todo??
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
                        ::vos::OGuard aGuard( Application::GetSolarMutex());
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
                        ::vos::OGuard aGuard( Application::GetSolarMutex() );
                        if( m_pChartWindow )
                            m_pChartWindow->Invalidate();
                    }
                }

                m_bConnectingToView = false;
            }
        }
    }
}

        sal_Bool SAL_CALL ChartController
::attachModel( const uno::Reference< frame::XModel > & xModel )
        throw(uno::RuntimeException)
{
    impl_invalidateAccessible();

    //is called to attach the controller to a new model.
    //return true if attach was successfully, false otherwise (e.g. if you do not work with a model)

    ::vos::OClearableGuard aClearableGuard( Application::GetSolarMutex());
    if( impl_isDisposedOrSuspended() ) //@todo? allow attaching a new model while suspended?
        return sal_False; //behave passive if already disposed or suspended
    aClearableGuard.clear();


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
        //@todo?? termination correct?
//      aOldModelRef->tryTermination();
#ifdef TEST_ENABLE_MODIFY_LISTENER
        uno::Reference< util::XModifyBroadcaster > xMBroadcaster( aOldModelRef->getModel(),uno::UNO_QUERY );
        if( xMBroadcaster.is())
            xMBroadcaster->removeModifyListener( this );
#endif
    }

    //--handle relations to the new model
    aNewModelRef->addListener( this );

    // set new model at dispatchers
    m_aDispatchContainer.setModel( aNewModelRef->getModel());
    ControllerCommandDispatch * pDispatch = new ControllerCommandDispatch( m_xCC, this, &m_aDispatchContainer );
    pDispatch->initialize();

    // the dispatch container will return "this" for all commands returned by
    // impl_getAvailableCommands().  That means, for those commands dispatch()
    // is called here at the ChartController.
    m_aDispatchContainer.setChartDispatch( pDispatch, impl_getAvailableCommands() );

    DrawCommandDispatch* pDrawDispatch = new DrawCommandDispatch( m_xCC, this );
    if ( pDrawDispatch )
    {
        pDrawDispatch->initialize();
        m_aDispatchContainer.setDrawCommandDispatch( pDrawDispatch );
    }

    ShapeController* pShapeController = new ShapeController( m_xCC, this );
    if ( pShapeController )
    {
        pShapeController->initialize();
        m_aDispatchContainer.setShapeController( pShapeController );
    }

#ifdef TEST_ENABLE_MODIFY_LISTENER
    uno::Reference< util::XModifyBroadcaster > xMBroadcaster( aNewModelRef->getModel(),uno::UNO_QUERY );
    if( xMBroadcaster.is())
        xMBroadcaster->addModifyListener( this );
#endif

    //select chart area per default:
    select( uno::makeAny( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, rtl::OUString() ) ) );

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
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if( m_pChartWindow )
            m_pChartWindow->Invalidate();
    }

    uno::Reference< document::XUndoManagerSupplier > xSuppUndo( getModel(), uno::UNO_QUERY_THROW );
    m_xUndoManager.set( xSuppUndo->getUndoManager(), uno::UNO_QUERY_THROW );

    return sal_True;
}

        uno::Reference< frame::XFrame > SAL_CALL ChartController
::getFrame()    throw(uno::RuntimeException)
{
    //provides access to owner frame of this controller
    //return the frame containing this controller

    return m_xFrame;
}

        uno::Reference< frame::XModel > SAL_CALL ChartController
::getModel()    throw(uno::RuntimeException)
{
    //provides access to currently attached model
    //returns the currently attached model

    //return nothing, if you do not have a model
    TheModelRef aModelRef( m_aModel, m_aModelMutex);
    if(aModelRef.is())
        return aModelRef->getModel();

    return uno::Reference< frame::XModel > ();
}

        uno::Any SAL_CALL ChartController
::getViewData() throw(uno::RuntimeException)
{
    //provides access to current view status
    //set of data that can be used to restore the current view status at later time
    //  by using XController::restoreViewData()

    ::vos::OGuard aGuard( Application::GetSolarMutex());
    if( impl_isDisposedOrSuspended() )
        return uno::Any(); //behave passive if already disposed or suspended //@todo? or throw an exception??

    //-- collect current view state
    uno::Any aRet;
    //// @todo integrate specialized implementation

    return aRet;
}

        void SAL_CALL ChartController
::restoreViewData( const uno::Any& /* Value */ )
        throw(uno::RuntimeException)
{
    //restores the view status using the data gotten from a previous call to XController::getViewData()

    ::vos::OGuard aGuard( Application::GetSolarMutex());
    if( impl_isDisposedOrSuspended() )
        return; //behave passive if already disposed or suspended //@todo? or throw an exception??

    //// @todo integrate specialized implementation
}

        sal_Bool SAL_CALL ChartController
::suspend( sal_Bool bSuspend )
        throw(uno::RuntimeException)
{
    //is called to prepare the controller for closing the view
    //bSuspend==true: force the controller to suspend his work
    //bSuspend==false try to reactivate the controller
    //returns true if request was accepted and of course successfully finished, false otherwise

    //we may show dialogs here to ask the user for saving changes ... @todo?

    ::vos::OGuard aGuard( Application::GetSolarMutex());
    if( m_aLifeTimeManager.impl_isDisposed() )
        return sal_False; //behave passive if already disposed, return false because request was not accepted //@todo? correct

    if(bSuspend==m_bSuspended)
    {
        OSL_ENSURE( sal_False, "new suspend mode equals old suspend mode" );
        return sal_True;
    }

    //change suspend mode
    if(bSuspend)
    {
        //aGuard.clear();
        //@todo ???  try to stop all what may prevent me from becoming disposed
        //aGuard.reset();

        m_bSuspended = bSuspend;
        return sal_True;
    }
    else
    {
        //aGuard.clear();
        //@todo ??? redo what was made in section bSuspend==true
        //aGuard.reset();

        m_bSuspended = bSuspend;
    }
    return sal_True;


    /*
    if ( bSuspend )
        getFrame()->removeFrameActionListener( pImp );
    else
        getFrame()->addFrameActionListener( pImp );
        */
}


void ChartController::impl_createDrawViewController()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());
    if(!m_pDrawViewWrapper)
    {
        if( m_pDrawModelWrapper )
        {
            m_pDrawViewWrapper = new DrawViewWrapper(&m_pDrawModelWrapper->getSdrModel(),m_pChartWindow,true);
            m_pDrawViewWrapper->attachParentReferenceDevice( getModel() );
        }
    }
}
void ChartController::impl_deleteDrawViewController()
{
    if( m_pDrawViewWrapper )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex());
        if( m_pDrawViewWrapper->IsTextEdit() )
            this->EndTextEdit();
        DELETEZ( m_pDrawViewWrapper );
    }
}

//-----------------------------------------------------------------
// XComponent (base of XController)
//-----------------------------------------------------------------

        void SAL_CALL ChartController
::dispose() throw(uno::RuntimeException)
{
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
                xSelectionChangeListener = uno::Reference< view::XSelectionChangeListener >( xDataReceiver->getRangeHighlighter(), uno::UNO_QUERY );
            if( xSelectionChangeListener.is() )
            {
                uno::Reference< frame::XController > xController( this );
                uno::Reference< lang::XComponent > xComp( xController, uno::UNO_QUERY );
                //lang::EventObject aEvent( static_cast< lang::XComponent* >( this ) );
                lang::EventObject aEvent( xComp );
                xSelectionChangeListener->disposing( aEvent );
            }
        }

        //--release all resources and references
        {
            uno::Reference< util::XModeChangeBroadcaster > xViewBroadcaster( m_xChartView, uno::UNO_QUERY );
            if( xViewBroadcaster.is() )
                xViewBroadcaster->removeModeChangeListener(this);
            // /--
            impl_invalidateAccessible();
            ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
            impl_deleteDrawViewController();
            m_pDrawModelWrapper.reset();

            m_apDropTargetHelper.reset();

            //the accessible view is disposed within window destructor of m_pChartWindow
            m_pChartWindow->clear();
            m_pChartWindow = NULL;//m_pChartWindow is deleted via UNO due to dispose of m_xViewWindow (trigerred by Framework (Controller pretends to be XWindow also))
            m_xViewWindow->dispose();
            m_xChartView.clear();
            // \--
        }

        // remove as listener to layout manager events
        if( m_xLayoutManagerEventBroadcaster.is())
        {
            m_xLayoutManagerEventBroadcaster->removeLayoutManagerEventListener( this );
            m_xLayoutManagerEventBroadcaster.set( 0 );
        }

        m_xFrame.clear();
        m_xUndoManager.clear();

        TheModelRef aModelRef( m_aModel, m_aModelMutex);
        m_aModel = NULL;

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

        m_aDispatchContainer.DisposeAndClear();
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
 }

        void SAL_CALL ChartController
::addEventListener( const uno::Reference<lang::XEventListener>& xListener )
        throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());
    if( impl_isDisposedOrSuspended() )//@todo? allow adding of listeners in suspend mode?
        return; //behave passive if already disposed or suspended

    //--add listener
    m_aLifeTimeManager.m_aListenerContainer.addInterface( ::getCppuType((const uno::Reference< lang::XEventListener >*)0), xListener );
}

        void SAL_CALL ChartController
::removeEventListener( const uno::Reference<
        lang::XEventListener>& xListener )
        throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());
    if( m_aLifeTimeManager.impl_isDisposed(false) )
        return; //behave passive if already disposed or suspended

    //--remove listener
    m_aLifeTimeManager.m_aListenerContainer.removeInterface( ::getCppuType((const uno::Reference< lang::XEventListener >*)0), xListener );
}


//-----------------------------------------------------------------
// util::XCloseListener
//-----------------------------------------------------------------
        void SAL_CALL ChartController
::queryClosing( const lang::EventObject& rSource, sal_Bool bGetsOwnership )
        throw(util::CloseVetoException, uno::RuntimeException)
{
    //do not use the m_aControllerMutex here because this call is not allowed to block

    TheModelRef aModelRef( m_aModel, m_aModelMutex);

    if( !aModelRef.is() )
        return;

    if( !(aModelRef->getModel() == rSource.Source) )
    {
        OSL_ENSURE( sal_False, "queryClosing was called on a controller from an unknown source" );
        return;
    }

    if( !m_bCanClose )//@todo tryaqcuire mutex
    {
        if( bGetsOwnership )
        {
            aModelRef->SetOwnerShip( bGetsOwnership );
        }

        throw util::CloseVetoException();
    }
    else
    {
        //@ todo prepare to to closing model -> don't start any further hindering actions
    }
}

        void SAL_CALL ChartController
::notifyClosing( const lang::EventObject& rSource )
        throw(uno::RuntimeException)
{
    //Listener should deregister himself and relaese all references to the closing object.

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
                xFrameCloseable->close( sal_False /* DeliverOwnership */ );
                m_xFrame.clear();
            }
            catch( util::CloseVetoException & )
            {
                // closing was vetoed
            }
        }
    }
}

bool ChartController::impl_releaseThisModel( const uno::Reference< uno::XInterface > & xModel )
{
    bool bReleaseModel = sal_False;
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aModelMutex );
        if( m_aModel.is() && m_aModel->getModel() == xModel )
        {
            m_aModel = NULL;
            m_xUndoManager.clear();
            bReleaseModel = true;
        }
    }
    if( bReleaseModel )
        m_aDispatchContainer.setModel( 0 );
    return bReleaseModel;
}

//-----------------------------------------------------------------
// util::XEventListener (base of XCloseListener)
//-----------------------------------------------------------------
        void SAL_CALL ChartController
::disposing( const lang::EventObject& rSource )
        throw(uno::RuntimeException)
{
    if( !impl_releaseThisModel( rSource.Source ))
    {
        if( rSource.Source == m_xLayoutManagerEventBroadcaster )
            m_xLayoutManagerEventBroadcaster.set( 0 );
    }
}

void SAL_CALL ChartController::layoutEvent( const lang::EventObject& aSource, ::sal_Int16 eLayoutEvent, const uno::Any& /* aInfo */ )
    throw (uno::RuntimeException)
{
    if( eLayoutEvent == frame::LayoutManagerEvents::MERGEDMENUBAR )
    {
        Reference< frame::XLayoutManager > xLM( aSource.Source, uno::UNO_QUERY );
        if( xLM.is())
        {
            xLM->createElement( C2U("private:resource/statusbar/statusbar"));
            xLM->requestElement( C2U("private:resource/statusbar/statusbar"));
        }
    }
}


//-----------------------------------------------------------------
// XDispatchProvider (required interface)
//-----------------------------------------------------------------

namespace
{
bool lcl_isFormatObjectCommand( const rtl::OString& aCommand )
{
    if(    aCommand.equals("MainTitle")
        || aCommand.equals("SubTitle")
        || aCommand.equals("XTitle")
        || aCommand.equals("YTitle")
        || aCommand.equals("ZTitle")
        || aCommand.equals("SecondaryXTitle")
        || aCommand.equals("SecondaryYTitle")
        || aCommand.equals("AllTitles")
        || aCommand.equals("DiagramAxisX")
        || aCommand.equals("DiagramAxisY")
        || aCommand.equals("DiagramAxisZ")
        || aCommand.equals("DiagramAxisA")
        || aCommand.equals("DiagramAxisB")
        || aCommand.equals("DiagramAxisAll")
        || aCommand.equals("DiagramGridXMain")
        || aCommand.equals("DiagramGridYMain")
        || aCommand.equals("DiagramGridZMain")
        || aCommand.equals("DiagramGridXHelp")
        || aCommand.equals("DiagramGridYHelp")
        || aCommand.equals("DiagramGridZHelp")
        || aCommand.equals("DiagramGridAll")

        || aCommand.equals("DiagramWall")
        || aCommand.equals("DiagramFloor")
        || aCommand.equals("DiagramArea")
        || aCommand.equals("Legend")

        || aCommand.equals("FormatWall")
        || aCommand.equals("FormatFloor")
        || aCommand.equals("FormatChartArea")
        || aCommand.equals("FormatLegend")

        || aCommand.equals("FormatTitle")
        || aCommand.equals("FormatAxis")
        || aCommand.equals("FormatDataSeries")
        || aCommand.equals("FormatDataPoint")
        || aCommand.equals("FormatDataLabels")
        || aCommand.equals("FormatDataLabel")
        || aCommand.equals("FormatYErrorBars")
        || aCommand.equals("FormatMeanValue")
        || aCommand.equals("FormatTrendline")
        || aCommand.equals("FormatTrendlineEquation")
        || aCommand.equals("FormatStockLoss")
        || aCommand.equals("FormatStockGain")
        || aCommand.equals("FormatMajorGrid")
        || aCommand.equals("FormatMinorGrid")
        )
    return true;

    // else
    return false;
}
} // anonymous namespace

        uno::Reference<frame::XDispatch> SAL_CALL ChartController
::queryDispatch( const util::URL& rURL
        , const rtl::OUString& rTargetFrameName
        , sal_Int32 /* nSearchFlags */)
        throw(uno::RuntimeException)
{
    if ( !m_aLifeTimeManager.impl_isDisposed() && getModel().is() )
    {
        if( rTargetFrameName.getLength() &&
            rTargetFrameName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("_self")))
            return m_aDispatchContainer.getDispatchForURL( rURL );
    }
    return uno::Reference< frame::XDispatch > ();
}

        uno::Sequence<uno::Reference<frame::XDispatch > >   ChartController
::queryDispatches( const uno::Sequence<
        frame::DispatchDescriptor>& xDescripts)
        throw(uno::RuntimeException)
{
    if ( !m_aLifeTimeManager.impl_isDisposed() )
    {
        return m_aDispatchContainer.getDispatchesForURLs( xDescripts );
    }
    return uno::Sequence<uno::Reference<frame::XDispatch > > ();
}

//-----------------------------------------------------------------
// frame::XDispatch
//-----------------------------------------------------------------

    void SAL_CALL ChartController
::dispatch( const util::URL& rURL
            , const uno::Sequence< beans::PropertyValue >& rArgs )
            throw (uno::RuntimeException)
{
    //@todo avoid OString (see Mathias mail on bug #104387#)
    rtl::OString aCommand( rtl::OUStringToOString( rURL.Path, RTL_TEXTENCODING_ASCII_US ) );

    if(aCommand.equals("Paste"))
        this->executeDispatch_Paste();
    else if(aCommand.equals("Copy"))
        this->executeDispatch_Copy();
    else if(aCommand.equals("Cut"))
        this->executeDispatch_Cut();
    else if(aCommand.equals("DataRanges"))
        this->executeDispatch_SourceData();
    //----------------------------------
    else if(aCommand.equals("Update")) //Update Chart
    {
        ChartViewHelper::setViewToDirtyState( getModel() );
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        if( m_pChartWindow )
            m_pChartWindow->Invalidate();
    }
    else if(aCommand.equals("DiagramData"))
        this->executeDispatch_EditData();
    //insert objects
    else if( aCommand.equals("InsertTitles")
        || aCommand.equals("InsertMenuTitles") )
        this->executeDispatch_InsertTitles();
    else if( aCommand.equals("InsertMenuLegend") )
        this->executeDispatch_OpenLegendDialog();
    else if( aCommand.equals("InsertLegend") )
        this->executeDispatch_InsertLegend();
    else if( aCommand.equals("DeleteLegend") )
        this->executeDispatch_DeleteLegend();
    else if( aCommand.equals("InsertMenuDataLabels"))
        this->executeDispatch_InsertMenu_DataLabels();
    else if( aCommand.equals("InsertMenuAxes")
        || aCommand.equals("InsertRemoveAxes") )
        this->executeDispatch_InsertAxes();
    else if( aCommand.equals("InsertMenuGrids"))
        this->executeDispatch_InsertGrid();
    else if( aCommand.equals("InsertMenuTrendlines"))
        this->executeDispatch_InsertMenu_Trendlines();
    else if( aCommand.equals("InsertMenuMeanValues"))
        this->executeDispatch_InsertMenu_MeanValues();
    else if( aCommand.equals("InsertMenuYErrorBars"))
        this->executeDispatch_InsertMenu_YErrorBars();
    else if( aCommand.equals("InsertSymbol"))
         this->executeDispatch_InsertSpecialCharacter();
    else if( aCommand.equals("InsertTrendline"))
         this->executeDispatch_InsertTrendline();
    else if( aCommand.equals("DeleteTrendline"))
         this->executeDispatch_DeleteTrendline();
    else if( aCommand.equals("InsertMeanValue"))
        this->executeDispatch_InsertMeanValue();
    else if( aCommand.equals("DeleteMeanValue"))
        this->executeDispatch_DeleteMeanValue();
    else if( aCommand.equals("InsertYErrorBars"))
        this->executeDispatch_InsertYErrorBars();
    else if( aCommand.equals("DeleteYErrorBars"))
        this->executeDispatch_DeleteYErrorBars();
    else if( aCommand.equals("InsertTrendlineEquation"))
         this->executeDispatch_InsertTrendlineEquation();
    else if( aCommand.equals("DeleteTrendlineEquation"))
         this->executeDispatch_DeleteTrendlineEquation();
    else if( aCommand.equals("InsertTrendlineEquationAndR2"))
         this->executeDispatch_InsertTrendlineEquation( true );
    else if( aCommand.equals("InsertR2Value"))
         this->executeDispatch_InsertR2Value();
    else if( aCommand.equals("DeleteR2Value"))
         this->executeDispatch_DeleteR2Value();
    else if( aCommand.equals("InsertDataLabels") )
        this->executeDispatch_InsertDataLabels();
    else if( aCommand.equals("InsertDataLabel") )
        this->executeDispatch_InsertDataLabel();
    else if( aCommand.equals("DeleteDataLabels") )
        this->executeDispatch_DeleteDataLabels();
    else if( aCommand.equals("DeleteDataLabel") )
        this->executeDispatch_DeleteDataLabel();
    else if( aCommand.equals("ResetAllDataPoints") )
        this->executeDispatch_ResetAllDataPoints();
    else if( aCommand.equals("ResetDataPoint") )
        this->executeDispatch_ResetDataPoint();
    else if( aCommand.equals("InsertAxis") )
        this->executeDispatch_InsertAxis();
    else if( aCommand.equals("InsertMajorGrid") )
        this->executeDispatch_InsertMajorGrid();
    else if( aCommand.equals("InsertMinorGrid") )
        this->executeDispatch_InsertMinorGrid();
    else if( aCommand.equals("InsertAxisTitle") )
        this->executeDispatch_InsertAxisTitle();
    else if( aCommand.equals("DeleteAxis") )
        this->executeDispatch_DeleteAxis();
    else if( aCommand.equals("DeleteMajorGrid") )
        this->executeDispatch_DeleteMajorGrid();
    else if( aCommand.equals("DeleteMinorGrid") )
        this->executeDispatch_DeleteMinorGrid();
    //format objects
    else if( aCommand.equals("FormatSelection") )
        this->executeDispatch_ObjectProperties();
    else if( aCommand.equals("TransformDialog"))
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
//MENUCHANGE    else if(aCommand.equals("SelectSourceRanges"))
//MENUCHANGE        this->executeDispatch_SourceData();
    else if( aCommand.equals("DiagramType"))
        this->executeDispatch_ChartType();
    else if( aCommand.equals("View3D"))
        this->executeDispatch_View3D();
    else if ( aCommand.equals( "Forward" ) )
    {
        if ( isShapeContext() )
        {
            this->impl_ShapeControllerDispatch( rURL, rArgs );
        }
        else
        {
            this->executeDispatch_MoveSeries( sal_True );
        }
    }
    else if ( aCommand.equals( "Backward" ) )
    {
        if ( isShapeContext() )
        {
            this->impl_ShapeControllerDispatch( rURL, rArgs );
        }
        else
        {
            this->executeDispatch_MoveSeries( sal_False );
        }
    }
    else if( aCommand.equals("NewArrangement"))
        this->executeDispatch_NewArrangement();
    else if( aCommand.equals("ToggleLegend"))
        this->executeDispatch_ToggleLegend();
    else if( aCommand.equals("ToggleGridHorizontal"))
        this->executeDispatch_ToggleGridHorizontal();
    else if( aCommand.equals("ScaleText"))
        this->executeDispatch_ScaleText();
    else if( aCommand.equals("StatusBarVisible"))
    {
        // workaround: this should not be necessary.
        uno::Reference< beans::XPropertySet > xPropSet( m_xFrame, uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
            xPropSet->getPropertyValue( C2U( "LayoutManager" ) ) >>= xLayoutManager;
            if ( xLayoutManager.is() )
            {
                bool bIsVisible( xLayoutManager->isElementVisible( C2U("private:resource/statusbar/statusbar")));
                if( bIsVisible )
                {
                    xLayoutManager->hideElement( C2U( "private:resource/statusbar/statusbar"));
                    xLayoutManager->destroyElement( C2U( "private:resource/statusbar/statusbar"));
                }
                else
                {
                    xLayoutManager->createElement( C2U( "private:resource/statusbar/statusbar"));
                    xLayoutManager->showElement( C2U( "private:resource/statusbar/statusbar"));
                }
                // @todo: update menu state (checkmark next to "Statusbar").
            }
        }
    }

    /*
    case SID_TEXTEDIT:
        this->executeDispatch_EditText();
    */
}

    void SAL_CALL ChartController
::addStatusListener( const uno::Reference<frame::XStatusListener >& /* xControl */
            , const util::URL& /* aURL */ )
            throw (uno::RuntimeException)
{
//     // TODO: add listener by URL !
//  ::vos::OGuard aGuard( Application::GetSolarMutex());
//  if( impl_isDisposedOrSuspended() )//@todo? allow adding of listeners in suspend mode?
//      return; //behave passive if already disposed or suspended

//  //--add listener
//      m_aLifeTimeManager.m_aListenerContainer.addInterface( ::getCppuType( & xControl ), xControl );
}

    void SAL_CALL ChartController
::removeStatusListener( const uno::Reference<frame::XStatusListener >& /* xControl */
            , const util::URL& /* aURL */ )
            throw (uno::RuntimeException)
{
//     // TODO: remove listener by URL !
//  ::vos::OGuard aGuard( Application::GetSolarMutex());
//     if( m_aLifeTimeManager.impl_isDisposed() )
//      return; //behave passive if already disposed or suspended

//  //--remove listener
//  m_aLifeTimeManager.m_aListenerContainer.removeInterface( ::getCppuType( & xControl ), xControl );
}

//-----------------------------------------------------------------
// XContextMenuInterception (optional interface)
//-----------------------------------------------------------------
        void SAL_CALL ChartController
::registerContextMenuInterceptor( const uno::Reference<
        ui::XContextMenuInterceptor > & /* xInterceptor */)
        throw(uno::RuntimeException)
{
    //@todo
}

        void SAL_CALL ChartController
::releaseContextMenuInterceptor( const uno::Reference<
        ui::XContextMenuInterceptor > & /* xInterceptor */)
        throw(uno::RuntimeException)
{
    //@todo
}

// ____ XEmbeddedClient ____
// implementation see: ChartController_EditData.cxx

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SAL_CALL ChartController::executeDispatch_ChartType()
{
    // using assignment for broken gcc 3.3
    UndoLiveUpdateGuard aUndoGuard = UndoLiveUpdateGuard(
        String( SchResId( STR_ACTION_EDIT_CHARTTYPE )), m_xUndoManager );

    // /--
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    //prepare and open dialog
    ChartTypeDialog aDlg( m_pChartWindow, getModel(), m_xCC );
    if( aDlg.Execute() == RET_OK )
    {
        impl_adaptDataSeriesAutoResize();
        aUndoGuard.commit();
    }
    // \--
}

void SAL_CALL ChartController::executeDispatch_SourceData()
{
    //-------------------------------------------------------------
    //convert properties to ItemSet
    uno::Reference< XChartDocument >   xChartDoc( getModel(), uno::UNO_QUERY );
    DBG_ASSERT( xChartDoc.is(), "Invalid XChartDocument" );
    if( !xChartDoc.is())
        return;

    // using assignment for broken gcc 3.3
    UndoLiveUpdateGuard aUndoGuard = UndoLiveUpdateGuard(
        String( SchResId( STR_ACTION_EDIT_DATA_RANGES )), m_xUndoManager );
    if( xChartDoc.is())
    {
        // /--
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        ::chart::DataSourceDialog aDlg( m_pChartWindow, xChartDoc, m_xCC );
        if( aDlg.Execute() == RET_OK )
        {
            impl_adaptDataSeriesAutoResize();
            aUndoGuard.commit();
        }
        // \--
    }
}

void SAL_CALL ChartController::executeDispatch_MoveSeries( sal_Bool bForward )
{
    ControllerLockGuard aCLGuard( getModel() );

    //get selected series
    ::rtl::OUString aObjectCID(m_aSelection.getSelectedCID());
    uno::Reference< XDataSeries > xGivenDataSeries( ObjectIdentifier::getDataSeriesForCID( //yyy todo also legendentries and labels?
            aObjectCID, getModel() ) );

    UndoGuardWithSelection aUndoGuard(
        ActionDescriptionProvider::createDescription(
            (bForward ? ActionDescriptionProvider::MOVE_TOTOP : ActionDescriptionProvider::MOVE_TOBOTTOM),
            String( SchResId( STR_OBJECT_DATASERIES ))),
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
    ChartController::createInstance( const ::rtl::OUString& aServiceSpecifier )
    throw (uno::Exception,
           uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xResult;

    if( aServiceSpecifier.equals( CHART_ACCESSIBLE_TEXT_SERVICE_NAME ))
        xResult.set( impl_createAccessibleTextContext());
    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL
    ChartController::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier,
                                 const uno::Sequence< uno::Any >& /* Arguments */ )
    throw (uno::Exception,
           uno::RuntimeException)
{
    // ignore Arguments
    return createInstance( ServiceSpecifier );
}

uno::Sequence< ::rtl::OUString > SAL_CALL
    ChartController::getAvailableServiceNames()
    throw (uno::RuntimeException)
{
    static uno::Sequence< ::rtl::OUString > aServiceNames;

    if( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc(1);
        aServiceNames[0] = CHART_ACCESSIBLE_TEXT_SERVICE_NAME;
    }

    return aServiceNames;
}

// ____ XModifyListener ____
void SAL_CALL ChartController::modified( const lang::EventObject& /* aEvent */ )
    throw (uno::RuntimeException)
{
    // the source can also be a subobject of the ChartModel
    // @todo: change the source in ChartModel to always be the model itself ?
//     if( getModel() == aEvent.Source )


    //todo? update menu states ?
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

IMPL_LINK( ChartController, NotifyUndoActionHdl, SdrUndoAction*, pUndoAction )
{
    ENSURE_OR_RETURN( pUndoAction, "invalid Undo action", 1L );

    ::rtl::OUString aObjectCID = m_aSelection.getSelectedCID();
    if ( aObjectCID.getLength() == 0 )
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
    return 0L;
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
    uno::Reference< XAccessible > xResult = new AccessibleChartView( m_xCC, GetDrawViewWrapper() );
    impl_initializeAccessible( uno::Reference< lang::XInitialization >( xResult, uno::UNO_QUERY ) );
    return xResult;
}

void ChartController::impl_invalidateAccessible()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if( m_pChartWindow )
        this->impl_initializeAccessible( Reference< lang::XInitialization >( m_pChartWindow->GetAccessible(false), uno::UNO_QUERY ) );
}
void ChartController::impl_initializeAccessible( const uno::Reference< lang::XInitialization >& xInit )
{
    if(xInit.is())
    {
        uno::Sequence< uno::Any > aArguments(5);
        uno::Reference<view::XSelectionSupplier> xSelectionSupplier(this);
        aArguments[0]=uno::makeAny(xSelectionSupplier);
        uno::Reference<frame::XModel> xModel(getModel());
        aArguments[1]=uno::makeAny(xModel);
        aArguments[2]=uno::makeAny(m_xChartView);
        uno::Reference< XAccessible > xParent;
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            if( m_pChartWindow )
            {
                Window* pParentWin( m_pChartWindow->GetAccessibleParentWindow());
                if( pParentWin )
                    xParent.set( pParentWin->GetAccessible());
            }
        }
        aArguments[3]=uno::makeAny(xParent);
        aArguments[4]=uno::makeAny(m_xViewWindow);

        xInit->initialize(aArguments);
    }
}

::std::set< ::rtl::OUString > ChartController::impl_getAvailableCommands()
{
    return ::comphelper::MakeSet< ::rtl::OUString >
        // commands for container forward
        ( C2U("AddDirect"))           ( C2U("NewDoc"))                ( C2U("Open"))
        ( C2U("Save"))                ( C2U("SaveAs"))                ( C2U("SendMail"))
        ( C2U("EditDoc"))             ( C2U("ExportDirectToPDF"))     ( C2U("PrintDefault"))

        // own commands
        ( C2U("Cut") )                ( C2U("Copy") )                 ( C2U("Paste") )
        ( C2U("DataRanges") )         ( C2U("DiagramData") )
        // insert objects
        ( C2U("InsertMenuTitles") )   ( C2U("InsertTitles") )
        ( C2U("InsertMenuLegend") )   ( C2U("InsertLegend") )         ( C2U("DeleteLegend") )
        ( C2U("InsertMenuDataLabels") )
        ( C2U("InsertMenuAxes") )     ( C2U("InsertRemoveAxes") )         ( C2U("InsertMenuGrids") )
        ( C2U("InsertSymbol") )
        ( C2U("InsertTrendlineEquation") )  ( C2U("InsertTrendlineEquationAndR2") )
        ( C2U("InsertR2Value") )      ( C2U("DeleteR2Value") )
        ( C2U("InsertMenuTrendlines") )  ( C2U("InsertTrendline") )
        ( C2U("InsertMenuMeanValues") ) ( C2U("InsertMeanValue") )
        ( C2U("InsertMenuYErrorBars") )   ( C2U("InsertYErrorBars") )
        ( C2U("InsertDataLabels") )   ( C2U("InsertDataLabel") )
        ( C2U("DeleteTrendline") )    ( C2U("DeleteMeanValue") )      ( C2U("DeleteTrendlineEquation") )
        ( C2U("DeleteYErrorBars") )
        ( C2U("DeleteDataLabels") )   ( C2U("DeleteDataLabel") )
        //format objects
//MENUCHANGE            ( C2U("SelectSourceRanges") )
        ( C2U("FormatSelection") )     ( C2U("TransformDialog") )
        ( C2U("DiagramType") )        ( C2U("View3D") )
        ( C2U("Forward") )            ( C2U("Backward") )
        ( C2U("MainTitle") )          ( C2U("SubTitle") )
        ( C2U("XTitle") )             ( C2U("YTitle") )               ( C2U("ZTitle") )
        ( C2U("SecondaryXTitle") )    ( C2U("SecondaryYTitle") )
        ( C2U("AllTitles") )          ( C2U("Legend") )
        ( C2U("DiagramAxisX") )       ( C2U("DiagramAxisY") )         ( C2U("DiagramAxisZ") )
        ( C2U("DiagramAxisA") )       ( C2U("DiagramAxisB") )         ( C2U("DiagramAxisAll") )
        ( C2U("DiagramGridXMain") )   ( C2U("DiagramGridYMain") )     ( C2U("DiagramGridZMain") )
        ( C2U("DiagramGridXHelp") )   ( C2U("DiagramGridYHelp") )     ( C2U("DiagramGridZHelp") )
        ( C2U("DiagramGridAll") )
        ( C2U("DiagramWall") )        ( C2U("DiagramFloor") )         ( C2U("DiagramArea") )

        //context menu - format objects entries
        ( C2U("FormatWall") )        ( C2U("FormatFloor") )         ( C2U("FormatChartArea") )
        ( C2U("FormatLegend") )

        ( C2U("FormatAxis") )           ( C2U("FormatTitle") )
        ( C2U("FormatDataSeries") )     ( C2U("FormatDataPoint") )
        ( C2U("ResetAllDataPoints") )   ( C2U("ResetDataPoint") )
        ( C2U("FormatDataLabels") )     ( C2U("FormatDataLabel") )
        ( C2U("FormatMeanValue") )      ( C2U("FormatTrendline") )      ( C2U("FormatTrendlineEquation") )
        ( C2U("FormatYErrorBars") )
        ( C2U("FormatStockLoss") )      ( C2U("FormatStockGain") )

        ( C2U("FormatMajorGrid") )      ( C2U("InsertMajorGrid") )      ( C2U("DeleteMajorGrid") )
        ( C2U("FormatMinorGrid") )      ( C2U("InsertMinorGrid") )      ( C2U("DeleteMinorGrid") )
        ( C2U("InsertAxis") )           ( C2U("DeleteAxis") )           ( C2U("InsertAxisTitle") )

        // toolbar commands
        ( C2U("ToggleGridHorizontal"))( C2U("ToggleLegend") )         ( C2U("ScaleText") )
        ( C2U("NewArrangement") )     ( C2U("Update") )
        ( C2U("DefaultColors") )      ( C2U("BarWidth") )             ( C2U("NumberOfLines") )
        ( C2U("ArrangeRow") )
        ( C2U("StatusBarVisible") )
        ( C2U("ChartElementSelector") )
        ;
}

//.............................................................................
} //namespace chart
//.............................................................................
