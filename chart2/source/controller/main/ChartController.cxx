/*************************************************************************
 *
 *  $RCSfile: ChartController.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-04 15:35:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ChartController.hxx"
#include "servicenames.hxx"
#include "SchItemPool.hxx"

#include "InlineContainer.hxx"
#include "Chart.hrc"
#include "ResId.hxx"
#include "SchSlotIds.hxx"

#include "chartview/ChartView.hxx"
#include "ChartWindow.hxx"
#include "DrawModelWrapper.hxx"
#include "DrawViewWrapper.hxx"
#include "DataSeriesTreeHelper.hxx"
#include "DiagramHelper.hxx"

#include "macros.hxx"

#include "chartview/NumberFormatterWrapper.hxx"

#include "dlg_ChartType.hxx"

//for SID_CHARMAP:
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XSTACKABLESCALEGROUP_HPP_
#include <drafts/com/sun/star/chart2/XStackableScaleGroup.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPETEMPLATE_HPP_
#include <drafts/com/sun/star/chart2/XChartTypeTemplate.hpp>
#endif

//-------
// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
//-------

//-------
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
//-------
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

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
    , m_xViewWindow( NULL )
    , m_pChartView( NULL )
    , m_pDrawModelWrapper( NULL )
    , m_pDrawViewWrapper(NULL)
    , m_bViewDirty( false )
    , m_pNumberFormatterWrapper(NULL)
{

    //@todo
    m_pNumberFormatterWrapper = new NumberFormatterWrapper();
}

ChartController::~ChartController()
{
    impl_deleteView();
    //m_pChartWindow is deleted via UNO by Window hierarchy or Framework
    if(m_pNumberFormatterWrapper)
        delete m_pNumberFormatterWrapper;

    //@todo ?
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
            static_cast<lang::XEventListener*>(pController) );
    }

}

void ChartController::TheModel::removeListener(  ChartController* pController )
{
    if(m_xCloseable.is())
        m_xCloseable->removeCloseListener(
            static_cast<util::XCloseListener*>(pController) );

    else if( m_xModel.is() )
        m_xModel->removeEventListener(
            static_cast<lang::XEventListener*>(pController) );
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

                OSL_ENSURE( !m_bOwnership,
                    "INFO: a well known owner has catched a CloseVetoException after calling close(true)" );

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
::impl_isDisposedOrSuspended()
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
    osl::Guard< osl::Mutex > aGuard( m_aControllerMutex );

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
    }
    {
        // calls to VCL
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        m_pChartWindow = new ChartWindow(this,pParent,pParent?pParent->GetStyle():0);
        m_xViewWindow = uno::Reference< awt::XWindow >( m_pChartWindow->GetComponentInterface(), uno::UNO_QUERY );
        m_pChartWindow->Show();
    }

    impl_tryInitializeView();

    {//create the menu
        util::URL aURL( SchResId(RID_MENU).getURL() );

        uno::Reference< lang::XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
        uno::Reference< util::XURLTransformer > xTrans(
                    xMgr->createInstance( ::rtl::OUString::createFromAscii(
                    "com.sun.star.util.URLTransformer") ), uno::UNO_QUERY );
        if( xTrans.is() )
        {
            // Datei laden
            xTrans->parseStrict( aURL );

            uno::Reference< frame::XDispatchProvider > xProv( xFrame, uno::UNO_QUERY );
            if ( xProv.is() )
            {
                uno::Reference< frame::XDispatch > aDisp =
                    xProv->queryDispatch( aURL,
                    ::rtl::OUString::createFromAscii("_menubar"), 12 );
                if ( aDisp.is() )
                    aDisp->dispatch( aURL, uno::Sequence<beans::PropertyValue>() );
            }
        }
    }
}

void SAL_CALL ChartController
::impl_rebuildView()
    throw(uno::RuntimeException)
{
    m_bViewDirty = false;
    impl_deleteView();
    impl_tryInitializeView();
    if( m_aSelectedObjectCID.getLength() )
    {
        //@todo reselect object
        m_aSelectedObjectCID = C2U("");
    }
}

void SAL_CALL ChartController
::impl_deleteView()
        throw(uno::RuntimeException)
{
    if( m_pDrawViewWrapper->IsTextEdit() )
        this->EndTextEdit();

    delete m_pChartView; m_pChartView = NULL;
    delete m_pDrawViewWrapper; m_pDrawViewWrapper = NULL;
    delete m_pDrawModelWrapper; m_pDrawModelWrapper = NULL;
}

        sal_Bool SAL_CALL ChartController
::impl_tryInitializeView()
        throw(uno::RuntimeException)
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aControllerMutex );
    if(!m_pChartWindow || !m_aModel.is() )
        return sal_False;

    m_pDrawModelWrapper = new DrawModelWrapper(m_xCC
        ,wrapper::SchItemPool::CreateSchItemPool());

    uno::Reference< frame::XModel > xDrawModel = m_pDrawModelWrapper->getUnoModel();
    if( xDrawModel.is())
        xDrawModel->lockControllers();

    m_pChartView = ChartView::createView( m_xCC, m_aModel->getModel()
                                , uno::Reference<drawing::XDrawPagesSupplier>::query( xDrawModel )
                                , m_pNumberFormatterWrapper );
    //OutputDevice* pOutDev = m_pDrawViewWrapper->GetWin(0);
    Size aPageSize = m_pChartWindow->GetOutputSize();
    m_pChartView->create( awt::Size( aPageSize.Width(), aPageSize.Height() ) );
//  m_pChartWindow->SetChartView(m_pChartView);//enable event flow from window to View (Window knows View)

    //create draw view:
    m_pDrawViewWrapper = new DrawViewWrapper(&m_pDrawModelWrapper->getSdrModel(),m_pChartWindow);
    //test:
    //Rectangle aTest = m_pDrawViewWrapper->GetWorkArea();
    //m_pDrawViewWrapper->SetWorkArea(pOutDev->PixelToLogic(Rectangle(rOfs, rSize)));

    if( xDrawModel.is())
    {
        xDrawModel->unlockControllers();
        m_pChartWindow->Invalidate();
    }
    return sal_True;
}

        sal_Bool SAL_CALL ChartController
::attachModel( const uno::Reference< frame::XModel > & xModel )
        throw(uno::RuntimeException)
{
    //is called to attach the controller to a new model.
    //return true if attach was successfully, false otherwise (e.g. if you do not work with a model)

    osl::ClearableGuard< osl::Mutex > aGuard( m_aControllerMutex );
    if( impl_isDisposedOrSuspended() ) //@todo? allow attaching a new model while suspended?
        return sal_False; //behave passive if already disposed or suspended
    aGuard.clear();


    TheModelRef aNewModelRef( new TheModel( xModel), m_aModelMutex);
    TheModelRef aOldModelRef(m_aModel,m_aModelMutex);
    m_aModel = aNewModelRef;

    //--handle relations to the old model if any
    if( aOldModelRef.is() )
    {
        aOldModelRef->removeListener( this );
        //@todo?? termination correct?
        aOldModelRef->tryTermination();
    }

    //--handle relations to the new model
    aNewModelRef->addListener( this );

    //the frameloader is responsible to call xModel->connectController

    {
        // Indirect calls to VCL
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        impl_tryInitializeView();
    }

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

    osl::ResettableGuard< osl::Mutex > aGuard( m_aControllerMutex );
    if( impl_isDisposedOrSuspended() )
        return uno::Any(); //behave passive if already disposed or suspended //@todo? or throw an exception??

    //-- collect current view state
    uno::Any aRet;
    //// @todo integrate specialized implementation

    return aRet;
}

        void SAL_CALL ChartController
::restoreViewData( const uno::Any& Value )
        throw(uno::RuntimeException)
{
    //restores the view status using the data gotten from a previous call to XController::getViewData()

    osl::ResettableGuard< osl::Mutex > aGuard( m_aControllerMutex );
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

    osl::ResettableGuard< osl::Mutex > aGuard( m_aControllerMutex );
    if( m_aLifeTimeManager.impl_isDisposed() )
        return sal_False; //behave passive if already disposed, return false because request was not accepted //@todo? correct

    if(bSuspend==m_bSuspended)
    {
        OSL_ENSURE( sal_False, "new suspend mode equals old suspend mode" );
        //@todo ??? or return true in this case?
        return sal_False;
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
        return sal_True;
    }


    /*
    if ( bSuspend )
        getFrame()->removeFrameActionListener( pImp );
    else
        getFrame()->addFrameActionListener( pImp );
        */
    return sal_True;
}

//-----------------------------------------------------------------
// XComponent (base of XController)
//-----------------------------------------------------------------

        void SAL_CALL ChartController
::dispose() throw(uno::RuntimeException)
{
    //This object should release all resources and references in the
    //easiest possible manner
    //This object must notify all registered listeners using the method
    //<member>XEventListener::disposing</member>

    //hold no mutex
    if( !m_aLifeTimeManager.dispose() )
        return;

    OSL_ENSURE( m_bSuspended, "dispose was called but controller is not suspended" );

    //--release all resources and references

    impl_deleteView();
    m_pChartWindow = NULL;//m_pChartWindow is deleted via UNO by Window hierarchy or Framework

    m_xFrame = NULL;
    TheModelRef aModelRef( m_aModel, m_aModelMutex);
    m_aModel = NULL;

    if(aModelRef.is())
    {
        aModelRef->removeListener( this );
        aModelRef->tryTermination();
    }

    //// @todo integrate specialized implementation
    //e.g. release further resources and references
 }

        void SAL_CALL ChartController
::addEventListener( const uno::Reference<lang::XEventListener>& xListener )
        throw(uno::RuntimeException)
{
    osl::Guard< osl::Mutex > aGuard( m_aControllerMutex );
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
    osl::Guard< osl::Mutex > aGuard( m_aControllerMutex );
    if( impl_isDisposedOrSuspended() ) //@todo? allow removing of listeners in suspend mode?
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

    TheModelRef aModelRef( 0, m_aModelMutex);
    bool bReleaseModel = sal_False;
    {
        ::osl::Guard< ::osl::Mutex > aGuard( m_aModelMutex );
        aModelRef = m_aModel;
        if( aModelRef.is() && aModelRef->getModel() == rSource.Source )
        {
            m_aModel = NULL;
            bReleaseModel = sal_True;
        }
    }
    //@todo ?? why remove the listener??? this could be handled by the closing object
    if( bReleaseModel )
    {
        //--stop listening to the closing model
        aModelRef->removeListener( this );
    }
}

//-----------------------------------------------------------------
// util::XEventListener (base of XCloseListener)
//-----------------------------------------------------------------
        void SAL_CALL ChartController
::disposing( const lang::EventObject& rSource )
        throw(uno::RuntimeException)
{
    notifyClosing(rSource);
}

//-----------------------------------------------------------------
// XDispatchProvider (required interface)
//-----------------------------------------------------------------

bool isFormatObjectSID( sal_Int32 nSlotID )
{
    if((sal_Int32)SID_DIAGRAM_TITLE_MAIN == nSlotID
    || (sal_Int32)SID_DIAGRAM_TITLE_SUB == nSlotID
    || (sal_Int32)SID_DIAGRAM_TITLE_X == nSlotID
    || (sal_Int32)SID_DIAGRAM_TITLE_Y == nSlotID
    || (sal_Int32)SID_DIAGRAM_TITLE_Z == nSlotID
    || (sal_Int32)SID_DIAGRAM_TITLE_ALL == nSlotID
    || (sal_Int32)SID_LEGEND == nSlotID
    || (sal_Int32)SID_DIAGRAM_AXIS_X == nSlotID
    || (sal_Int32)SID_DIAGRAM_AXIS_Y == nSlotID
    || (sal_Int32)SID_DIAGRAM_AXIS_Z == nSlotID
    || (sal_Int32)SID_DIAGRAM_AXIS_A == nSlotID //secondary x axis
    || (sal_Int32)SID_DIAGRAM_AXIS_B == nSlotID
    || (sal_Int32)SID_DIAGRAM_AXIS_ALL == nSlotID
    || (sal_Int32)SID_DIAGRAM_GRID_X_MAIN == nSlotID
    || (sal_Int32)SID_DIAGRAM_GRID_Y_MAIN == nSlotID
    || (sal_Int32)SID_DIAGRAM_GRID_Z_MAIN == nSlotID
    || (sal_Int32)SID_DIAGRAM_GRID_X_HELP == nSlotID
    || (sal_Int32)SID_DIAGRAM_GRID_Y_HELP == nSlotID
    || (sal_Int32)SID_DIAGRAM_GRID_Z_HELP == nSlotID
    || (sal_Int32)SID_DIAGRAM_GRID_ALL == nSlotID
    || (sal_Int32)SID_DIAGRAM_WALL == nSlotID
    || (sal_Int32)SID_DIAGRAM_FLOOR == nSlotID
    || (sal_Int32)SID_DIAGRAM_AREA == nSlotID
    )
        return true;
    return false;
}

        uno::Reference<frame::XDispatch> SAL_CALL ChartController
::queryDispatch( const util::URL& rURL
        , const rtl::OUString& rTargetFrameName
        , sal_Int32 nSearchFlags)
        throw(uno::RuntimeException)
{
    //// @todo integrate specialized implementation
    //decide which commands you can handle

    if ( !m_aLifeTimeManager.impl_isDisposed() )
    {
        //@todo avoid OString (see Mathias mail on bug #104387#)
        rtl::OString aCommand( rtl::OUStringToOString( rURL.Path, RTL_TEXTENCODING_ASCII_US ) );
        if( aCommand.equals("Save")
            || aCommand.equals("SaveAs")
            || aCommand.equals("SaveAll")

            || aCommand.equals("Undo")
            || aCommand.equals("Cut")
            || aCommand.equals("Copy")
            || aCommand.equals("Paste")
            || aCommand.equals("SelectAll")
            || aCommand.equals("Close")
            || aCommand.equals("TESTTEST")
            || aCommand.equals("slot:23")
            //|| aCommand.copy(0,4).EqualsAscii("Bib/")
            //|| aURL.Complete.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("slot:5503"))
            )
        {
            //@todo create a seperate dispatcher object or implement XDispatch interface by yourself
            //to return it here
            return static_cast< frame::XDispatch* >( this );
        }
        else if(rURL.Protocol.equalsIgnoreAsciiCase( C2U("slot:") ) )
        {
            sal_Int32 nSlotID = rURL.Path.toInt32();
            if( (sal_Int32)SID_DIAGRAM_OBJECTS == nSlotID
                || (sal_Int32)SID_DIAGRAM_TYPE == nSlotID
                || (sal_Int32)SID_INSERT_TITLE == nSlotID
                || (sal_Int32)SID_INSERT_CHART_LEGEND == nSlotID
                || (sal_Int32)SID_INSERT_DESCRIPTION == nSlotID
                || (sal_Int32)SID_INSERT_AXIS == nSlotID
                || (sal_Int32)SID_INSERT_GRIDS == nSlotID
                || (sal_Int32)SID_INSERT_STATISTICS == nSlotID
                || (sal_Int32)SID_CHARMAP == nSlotID
                || (sal_Int32)SID_TEXTEDIT == nSlotID
                || isFormatObjectSID(nSlotID)
                )
            {
                return static_cast< frame::XDispatch* >( this );
            }
        }
    }
    return uno::Reference< frame::XDispatch > ();
}
/*
typedef ::std::map< sal_Int32, ::rtl::OUString >    tSlotIdCommandMap;
typedef ::comphelper::MakeMap< sal_Int32, ::rtl::OUString > tMakeSlotIdCommandMap;
tMakeSlotIdCommandMap m_aSlotIdCommandMap =
    tMakeSlotIdCommandMap
    ( (sal_Int32)SID_DIAGRAM_TITLE_MAIN, C2U( "slot:" )+=rtl::OUString::valueOf( (sal_Int32)SID_DIAGRAM_TITLE_MAIN ) )
    ;
*/
        uno::Sequence<uno::Reference<frame::XDispatch > >   ChartController
::queryDispatches( const uno::Sequence<
        frame::DispatchDescriptor>& xDescripts)
        throw(uno::RuntimeException)
{
    sal_Int32 nCount = xDescripts.getLength();

    if( !nCount )
        return uno::Sequence<uno::Reference<frame::XDispatch > > ();

    uno::Sequence<uno::Reference<frame::XDispatch > > aRet( nCount );
    for( sal_Int32 nPos = 0; nPos<nCount; ++nPos )
    {
        aRet[ nPos ] = queryDispatch(
                              xDescripts[nPos].FeatureURL
                            , xDescripts[nPos].FrameName
                            , xDescripts[nPos].SearchFlags );
    }
    return aRet;
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
    //test only
    if(aCommand.equals("TESTTEST")
        || aCommand.equals("Undo") )
    {
        Window* pWindow = m_pChartWindow;
        Rectangle aRect( Point(0,0), pWindow->GetOutputSize() );
        Region aRegion( aRect );
        m_pDrawViewWrapper->InitRedraw(pWindow, aRegion );

        /*
        INVALIDATE_CHILDREN
        INVALIDATE_NOCHILDREN
        INVALIDATE_NOERASE
        INVALIDATE_UPDATE
        INVALIDATE_TRANSPARENT
        INVALIDATE_NOTRANSPARENT
        INVALIDATE_NOCLIPCHILDREN
        */
        //Invalidate(INVALIDATE_UPDATE);
    }

    else if(aCommand.equals("Save"))
    {
        //only test:
        impl_rebuildView();
    }
    else if(aCommand.equals("SaveAs"))
    {
        //only test:
        this->executeDispatch_ObjectToDefault();
    }
    //----------------------------------
    else if(rURL.Protocol.equalsIgnoreAsciiCase( C2U("slot:") ) )
    {
        sal_Int32 nSlotID = rURL.Path.toInt32();
        if((sal_Int32)SID_DIAGRAM_OBJECTS == nSlotID)
        {
            this->executeDispatch_ObjectProperties();
        }
        else if((sal_Int32)SID_DIAGRAM_TYPE == nSlotID)
        {
            this->executeDispatch_ChartType();
        }
        else if((sal_Int32)SID_INSERT_TITLE == nSlotID)
        {
            this->executeDispatch_InsertTitle();
        }
        else if((sal_Int32)SID_INSERT_CHART_LEGEND == nSlotID)
        {
            this->executeDispatch_InsertLegend();
        }
        else if((sal_Int32)SID_INSERT_DESCRIPTION == nSlotID)
        {
            this->executeDispatch_InsertDataLabel();
        }
        else if((sal_Int32)SID_INSERT_AXIS == nSlotID)
        {
            this->executeDispatch_InsertAxis();
        }
        else if((sal_Int32)SID_INSERT_GRIDS == nSlotID)
        {
            this->executeDispatch_InsertGrid();
        }
        else if((sal_Int32)SID_INSERT_STATISTICS == nSlotID)
        {
            this->executeDispatch_InsertStatistic();
        }
        else if((sal_Int32)SID_CHARMAP == nSlotID)
        {
            this->executeDispatch_InsertSpecialCharacter();
        }
        else if((sal_Int32)SID_TEXTEDIT == nSlotID)
        {
            this->executeDispatch_EditText();
        }
        else if( isFormatObjectSID(nSlotID) )
        {
            this->executeDispatch_FormatObject(nSlotID);
        }
    }
    else if(aCommand.equals("SaveAll"))
    {
        if( m_aModel.is())
        {
            uno::Reference< ::drafts::com::sun::star::chart2::XChartDocument > xDoc(
                m_aModel->getModel(), uno::UNO_QUERY );
            OSL_ASSERT( xDoc.is());

            uno::Reference< ::drafts::com::sun::star::chart2::XDataProvider > xDataProvider(
                m_xCC->getServiceManager()->createInstanceWithContext(
                    C2U( "com.sun.star.comp.chart.FileDataProvider" ),
                    m_xCC ), uno::UNO_QUERY );
            OSL_ASSERT( xDataProvider.is());

            ::rtl::OUString aFileName(
#if defined WNT
                RTL_CONSTASCII_USTRINGPARAM( "file:///D:/files/data.chd" )
#else
                RTL_CONSTASCII_USTRINGPARAM( "file:///work/data/data.chd" )
#endif
                );

            xDoc->attachDataProvider( xDataProvider );
            xDoc->setRangeRepresentation( aFileName );
            impl_rebuildView();
        }
    }
}

    void SAL_CALL ChartController
::addStatusListener( const uno::Reference<frame::XStatusListener >& xControl
            , const util::URL& aURL )
            throw (uno::RuntimeException)
{
}

    void SAL_CALL ChartController
::removeStatusListener( const uno::Reference<frame::XStatusListener >& xControl
            , const util::URL& aURL )
            throw (uno::RuntimeException)
{
}

//-----------------------------------------------------------------
// XContextMenuInterception (optional interface)
//-----------------------------------------------------------------
        void SAL_CALL ChartController
::registerContextMenuInterceptor( const uno::Reference<
        ui::XContextMenuInterceptor > & xInterceptor)
        throw(uno::RuntimeException)
{
    //@todo
}

        void SAL_CALL ChartController
::releaseContextMenuInterceptor( const uno::Reference<
        ui::XContextMenuInterceptor > & xInterceptor)
        throw(uno::RuntimeException)
{
    //@todo
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SAL_CALL ChartController::executeDispatch_ChartType()
{
    bool bChanged = false;

    //-------------------------------------------------------------
    //convert properties to ItemSet
    uno::Reference< XChartDocument >   xChartDoc( m_aModel->getModel(), uno::UNO_QUERY );
    DBG_ASSERT( xChartDoc.is(), "Invalid XChartDocument" );
    if( !xChartDoc.is())
        return;
    uno::Reference< XDiagram > xDia( xChartDoc->getDiagram() );
    DBG_ASSERT( xDia.is(), "No Diagram set!" );
    uno::Reference< XChartTypeTemplate > xTemplate;

    if( xChartDoc.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xCTManager( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
        //-------------------------------------------------------------
        //prepare and open dialog
        Window* pParent( NULL );
        SchDiagramTypeDlg aDlg( pParent, xDia, xCTManager );
        if( aDlg.Execute() == RET_OK &&
            aDlg.HasChanged() )
        {
            xTemplate.set( aDlg.getTemplate());
            bChanged = true;
        }
    }

    try
    {
        //make sure that all objects using  m_pDrawModelWrapper or m_pChartView are already deleted
        if( bChanged &&
            xTemplate.is() )
        {
            uno::Reference< XDiagram > xNewDia(
                xTemplate->createDiagram(
                    helper::DataSeriesTreeHelper::getDataSeriesFromDiagram( xDia )));
            helper::DiagramHelper::changeDiagram( xDia, xNewDia );
            xChartDoc->setDiagram( xNewDia );

            impl_rebuildView();
        }
    }
    catch( uno::Exception& e)
    {
        ASSERT_EXCEPTION( e );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
