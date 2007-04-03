/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DrawController.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:25:16 $
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
#include "precompiled_sd.hxx"

#include "DrawController.hxx"

#include "DrawSubController.hxx"
#include "sdpage.hxx"
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#ifndef SD_VIEW_SHELL_MANAGER_HXX
#include "ViewShellManager.hxx"
#endif
#include "framework/PaneController.hxx"

#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/bootstrap.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_PANECONTROLLER_HPP_
#include <com/sun/star/drawing/framework/PaneController.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::std;
using ::rtl::OUString;
using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace {
static const ::com::sun::star::uno::Type saComponentTypeIdentifier (
    ::getCppuType( (Reference<lang::XEventListener > *)0 ));
static const ::com::sun::star::uno::Type saSelectionTypeIdentifier (
    ::getCppuType( (Reference<view::XSelectionChangeListener > *)0 ));

} // end of anonymous namespace

namespace sd {

class DrawController::ControllerContainer
    : public ::std::hash_map<
          rtl::OUString,
          Reference<XInterface>,
          ::comphelper::UStringHash,
          ::comphelper::UStringEqual>
{
public:
    ControllerContainer (void) {};
};



DrawController::DrawController (ViewShellBase& rBase) throw()
    : DrawControllerInterfaceBase(&rBase),
      BroadcastHelperOwner(SfxBaseController::m_aMutex),
      OPropertySetHelper( static_cast<OBroadcastHelperVar<
          OMultiTypeInterfaceContainerHelper,
          OMultiTypeInterfaceContainerHelper::keyType>& >(
              BroadcastHelperOwner::maBroadcastHelper)),
      mpBase(&rBase),
      maLastVisArea(),
      mpCurrentPage(NULL),
      mbMasterPageMode(false),
      mbLayerMode(false),
      mbDisposing(false),
      mpPropertyArrayHelper(NULL),
      mpSubController(),
      mpControllerContainer(new ControllerContainer()),
      mxConfigurationController(),
      mxPaneController(),
      mxViewController(),
      maResourceControllerList()
{
    ProvideSubControllers();
}




DrawController::~DrawController (void) throw()
{
}




void DrawController::SetSubController (::std::auto_ptr<DrawSubController> pSubController)
{
    // Update the internal state.
    mpSubController = pSubController;
    mpPropertyArrayHelper.reset();
    maLastVisArea = Rectangle();

    // Inform listeners about the changed state.
    FireSelectionChangeListener();
}



// XInterface

IMPLEMENT_FORWARD_XINTERFACE2(
    DrawController,
    DrawControllerInterfaceBase,
    OPropertySetHelper);


// XTypeProvider

Sequence<Type> SAL_CALL DrawController::getTypes (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed();
    // OPropertySetHelper does not provide getTypes, so we have to
    // implement this method manually and list its three interfaces.
    OTypeCollection aTypeCollection (
        ::getCppuType (( const Reference<beans::XMultiPropertySet>*)NULL),
        ::getCppuType (( const Reference<beans::XFastPropertySet>*)NULL),
        ::getCppuType (( const Reference<beans::XPropertySet>*)NULL));

    return ::comphelper::concatSequences(
        SfxBaseController::getTypes(),
        aTypeCollection.getTypes(),
        DrawControllerInterfaceBase::getTypes());
}

IMPLEMENT_GET_IMPLEMENTATION_ID(DrawController);



// XComponent


void SAL_CALL DrawController::dispose()
    throw( RuntimeException )
{
    if( !mbDisposing )
    {
        OGuard aGuard( Application::GetSolarMutex() );

        if( !mbDisposing )
        {
            mbDisposing = true;

            // When the controller has not been detached from its view
            // shell, i.e. mpViewShell is not NULL, then tell PaneManager
            // and ViewShellManager to clear the shell stack.
            if (mpSubController.get()!=NULL && mpBase!=NULL)
            {
                mpBase->DisconnectAllClients();
                mpBase->GetViewShellManager().Shutdown();
            }

            OPropertySetHelper::disposing();

            DisposeSubControllers();

            SfxBaseController::dispose();
        }
    }
}




void SAL_CALL DrawController::addEventListener(
    const Reference<lang::XEventListener >& xListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    SfxBaseController::addEventListener( xListener );
}




void SAL_CALL DrawController::removeEventListener (
    const Reference<lang::XEventListener >& aListener)
    throw (RuntimeException)
{
    if(!rBHelper.bDisposed && !rBHelper.bInDispose && !mbDisposing)
        SfxBaseController::removeEventListener( aListener );
}




// XServiceInfo

OUString SAL_CALL DrawController::getImplementationName(  ) throw(RuntimeException)
{
    // Do not throw an excepetion at the moment.  This leads to a crash
    // under Solaris on relead.  See issue i70929 for details.
    //    ThrowIfDisposed();
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "DrawController" ) );
}



static OUString ssServiceName (OUString::createFromAscii(
    "com.sun.star.drawing.DrawingDocumentDrawView"));

sal_Bool SAL_CALL DrawController::supportsService (
    const OUString& rsServiceName)
    throw(RuntimeException)
{
    // Do not throw an excepetion at the moment.  This leads to a crash
    // under Solaris on relead.  See issue i70929 for details.
    //    ThrowIfDisposed();
    return rsServiceName.equals(ssServiceName);
}




Sequence<OUString> SAL_CALL DrawController::getSupportedServiceNames (void)
    throw(RuntimeException)
{
    ThrowIfDisposed();
    Sequence<OUString> aSupportedServices (1);
    OUString* pServices = aSupportedServices.getArray();
    pServices[0] = ssServiceName;
    return aSupportedServices;
}




//------ XSelectionSupplier --------------------------------------------

sal_Bool SAL_CALL DrawController::select (const Any& aSelection)
    throw(lang::IllegalArgumentException, RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    if (mpSubController.get() != NULL)
        return mpSubController->select(aSelection);
    else
        return false;
}




Any SAL_CALL DrawController::getSelection()
    throw(RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    if (mpSubController.get() != NULL)
        return mpSubController->getSelection();
    else
        return Any();
}




void SAL_CALL DrawController::addSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener)
    throw(RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    BroadcastHelperOwner::maBroadcastHelper.addListener (saSelectionTypeIdentifier, xListener);
}




void SAL_CALL DrawController::removeSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener )
    throw(RuntimeException)
{
    if( mbDisposing )
        throw lang::DisposedException();

    BroadcastHelperOwner::maBroadcastHelper.removeListener (saSelectionTypeIdentifier, xListener);
}





//=====  lang::XEventListener  ================================================

void SAL_CALL
    DrawController::disposing (const lang::EventObject& )
    throw (uno::RuntimeException)
{
}




//=====  view::XSelectionChangeListener  ======================================

void  SAL_CALL
    DrawController::selectionChanged (const lang::EventObject& rEvent)
        throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    // Have to forward the event to our selection change listeners.
    OInterfaceContainerHelper* pListeners = BroadcastHelperOwner::maBroadcastHelper.getContainer(
        ::getCppuType((Reference<view::XSelectionChangeListener>*)0));
    if (pListeners)
    {
        // Re-send the event to all of our listeners.
        OInterfaceIteratorHelper aIterator (*pListeners);
        while (aIterator.hasMoreElements())
        {
            try
            {
                view::XSelectionChangeListener* pListener =
                    static_cast<view::XSelectionChangeListener*>(
                        aIterator.next());
                if (pListener != NULL)
                    pListener->selectionChanged (rEvent);
            }
            catch (RuntimeException aException)
            {
            }
        }
    }
}




::awt::Rectangle DrawController::GetVisArea (void) const
{
    return awt::Rectangle(
        maLastVisArea.Left(),
        maLastVisArea.Top(),
        maLastVisArea.GetWidth(),
        maLastVisArea.GetHeight());
}




// XDrawView

void SAL_CALL DrawController::setCurrentPage( const Reference< drawing::XDrawPage >& xPage )
    throw(RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    if (mpSubController.get() != NULL)
        mpSubController->setCurrentPage(xPage);
}




Reference< drawing::XDrawPage > SAL_CALL DrawController::getCurrentPage (void)
    throw(RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Reference<drawing::XDrawPage> xPage;

    if (mpSubController.get() != NULL)
        xPage = mpSubController->getCurrentPage();

    return xPage;
}




void DrawController::FireVisAreaChanged (const Rectangle& rVisArea) throw()
{
    if( maLastVisArea != rVisArea )
    {
        Any aNewValue;
        aNewValue <<= awt::Rectangle(
            rVisArea.Left(),
            rVisArea.Top(),
            rVisArea.GetWidth(),
            rVisArea.GetHeight() );

        Any aOldValue;
        aOldValue <<= awt::Rectangle(
            maLastVisArea.Left(),
            maLastVisArea.Top(),
            maLastVisArea.GetWidth(),
            maLastVisArea.GetHeight() );

        FirePropertyChange (PROPERTY_WORKAREA, aNewValue, aOldValue);

        maLastVisArea = rVisArea;
    }
}




void DrawController::FireSelectionChangeListener() throw()
{
    OInterfaceContainerHelper * pLC = BroadcastHelperOwner::maBroadcastHelper.getContainer(
        saSelectionTypeIdentifier);
    if( pLC )
    {
        Reference< XInterface > xSource( (XWeak*)this );
        const lang::EventObject aEvent( xSource );

        // Ueber alle Listener iterieren und Events senden
        OInterfaceIteratorHelper aIt( *pLC);
        while( aIt.hasMoreElements() )
        {
            try
            {
                view::XSelectionChangeListener * pL =
                    static_cast<view::XSelectionChangeListener*>(aIt.next());
                if (pL != NULL)
                    pL->selectionChanged( aEvent );
            }
            catch (RuntimeException aException)
            {
            }
        }
    }
}




void DrawController::FireChangeEditMode (bool bMasterPageMode) throw()
{
    if (bMasterPageMode != mbMasterPageMode )
    {
        FirePropertyChange(
            PROPERTY_MASTERPAGEMODE,
            makeAny(bMasterPageMode),
            makeAny(mbMasterPageMode));

        mbMasterPageMode = bMasterPageMode;
    }
}




void DrawController::FireChangeLayerMode (bool bLayerMode) throw()
{
    if (bLayerMode != mbLayerMode)
    {
        FirePropertyChange(
            PROPERTY_LAYERMODE,
            makeAny(bLayerMode),
            makeAny(mbLayerMode));

        mbLayerMode = bLayerMode;
    }
}




void DrawController::FireSwitchCurrentPage (SdPage* pNewCurrentPage) throw()
{
    SdrPage* pCurrentPage  = mpCurrentPage.get();
    if (pNewCurrentPage != pCurrentPage)
    {
        try
        {
            Any aNewValue (
                makeAny(Reference<drawing::XDrawPage>(pNewCurrentPage->getUnoPage(), UNO_QUERY)));

            Any aOldValue;
            if (pCurrentPage != NULL)
            {
                Reference<drawing::XDrawPage> xOldPage (pCurrentPage->getUnoPage(), UNO_QUERY);
                aOldValue <<= xOldPage;
            }

            FirePropertyChange(PROPERTY_CURRENTPAGE, aNewValue, aOldValue);

            mpCurrentPage.reset(pNewCurrentPage);
        }
        catch( uno::Exception& e )
        {
            (void)e;
            DBG_ERROR(
                (::rtl::OString("sd::SdUnoDrawView::FireSwitchCurrentPage(), "
                    "exception caught: ") +
                    ::rtl::OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }
}




void DrawController::FirePropertyChange (
    sal_Int32 nHandle,
    const Any& rNewValue,
    const Any& rOldValue)
{
    try
    {
        fire (&nHandle, &rNewValue, &rOldValue, 1, sal_False);
    }
    catch (RuntimeException aException)
    {
        // Ignore this exception.  Exceptions should be handled in the
        // fire() function so that all listeners are called.  This is
        // not the case at the moment, so we simply ignore the
        // exception.
    }

}




ViewShellBase* DrawController::GetViewShellBase (void)
{
    return mpBase;
}




void DrawController::ReleaseViewShellBase (void)
{
    DisposeSubControllers();
    mpBase = NULL;
}




//===== XControllerManager ==============================================================

void SAL_CALL DrawController::registerResourceController (
    const ::rtl::OUString& sServiceName,
    const Reference<XResourceController>& rxResourceController)
    throw (RuntimeException)
{
    (void)sServiceName;

    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    sal_uInt32 nResourceControllerCount (maResourceControllerList.getLength());
    maResourceControllerList.realloc(nResourceControllerCount+1);
    maResourceControllerList[nResourceControllerCount] = rxResourceController;
}




void SAL_CALL DrawController::removeResourceController (
    const Reference<XResourceController>& rxResourceController)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    sal_uInt32 nResourceControllerCount (maResourceControllerList.getLength());
    for (sal_uInt32 nIndex=0; nIndex<nResourceControllerCount; ++nIndex)
    {
        if (maResourceControllerList[nIndex] == rxResourceController)
        {
            nResourceControllerCount -= 1;
            for ( ; nIndex<nResourceControllerCount; ++nIndex)
                maResourceControllerList[nIndex] = maResourceControllerList[nIndex+1];
            maResourceControllerList.realloc(nResourceControllerCount);
            break;
        }
    }
}




Sequence<Reference<XResourceController> > SAL_CALL
    DrawController::getResourceControllers (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return maResourceControllerList;
}




Reference<XInterface> SAL_CALL DrawController::getController (const ::rtl::OUString& sServiceName)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    Reference<XInterface> xController;
    ControllerContainer::const_iterator iController (mpControllerContainer->find(sServiceName));
    if (iController != mpControllerContainer->end())
        xController = iController->second;

    if ( ! xController.is())
    {
        try
        {
            // We do the creation and initialization manually so that we can
            // store the newly created instance before it is initialized.
            // This prevents a second creation when during the
            // initialization the service is requested a second time.
            Reference<lang::XMultiServiceFactory> xFactory (
                ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW);
            xController = xFactory->createInstance(sServiceName);
            (*mpControllerContainer)[sServiceName] = xController;
            Reference<lang::XInitialization> xInit (xController, UNO_QUERY_THROW);
            Sequence<Any> aArguments(1);
            aArguments[0] = makeAny(Reference<XController>(this));
            xInit->initialize(aArguments);
        }
        catch (RuntimeException&)
        {
        }
    }
    return xController;
}




void DrawController::ProvideSubControllers (void)
{
    ::vos::OGuard aGuard (Application::GetSolarMutex());
    try
    {
        Reference<lang::XMultiServiceFactory> xFactory (
            ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW);

        Sequence<Any> aInitializationArguments(1);
        aInitializationArguments[0] = makeAny(Reference<XController>(this));
        OUString sServiceName;
        Reference<XInterface> xController;

        // First, create the module controller whose services may be used by
        // the resource controllers to find the right resource factories.
        if ( ! mxModuleController.is())
        {
            sServiceName = ::rtl::OUString::createFromAscii(
                "com.sun.star.drawing.framework.ModuleController");
            xController = xFactory->createInstance(sServiceName);
            (*mpControllerContainer)[sServiceName] = xController;
            mxModuleController = Reference<XModuleController>(xController, UNO_QUERY);
            Reference<lang::XInitialization> xInit (xController, UNO_QUERY_THROW);
            xInit->initialize(aInitializationArguments);
        }

        // Create the configuration controller that synchronizes the
        // resource controllers.
        if ( ! mxConfigurationController.is())
        {
            sServiceName = ::rtl::OUString::createFromAscii(
                "com.sun.star.drawing.framework.ConfigurationController");
            xController = xFactory->createInstance(sServiceName);
            (*mpControllerContainer)[sServiceName] = xController;
            mxConfigurationController
                = Reference<XConfigurationController>(xController, UNO_QUERY);
            Reference<lang::XInitialization> xInit (xController, UNO_QUERY_THROW);
            xInit->initialize(aInitializationArguments);
        }

        // Create the resource controllers.
        if ( ! mxPaneController.is())
        {
            sServiceName = ::rtl::OUString::createFromAscii(
                "com.sun.star.drawing.framework.PaneController");
            xController = xFactory->createInstance(sServiceName);
            (*mpControllerContainer)[sServiceName] = xController;
            mxPaneController = Reference<XPaneController>(xController, UNO_QUERY);
            Reference<lang::XInitialization> xInit (xController, UNO_QUERY_THROW);
            xInit->initialize(aInitializationArguments);
        }
        if ( ! mxViewController.is())
        {
            sServiceName = ::rtl::OUString::createFromAscii(
                "com.sun.star.drawing.framework.ViewController");
            xController = xFactory->createInstance(sServiceName);
            (*mpControllerContainer)[sServiceName] = xController;
            mxViewController = Reference<XViewController>(xController, UNO_QUERY);
            Reference<lang::XInitialization> xInit (xController, UNO_QUERY_THROW);
            xInit->initialize(aInitializationArguments);
        }
        if ( ! mxToolBarController.is())
        {
            sServiceName = ::rtl::OUString::createFromAscii(
                "com.sun.star.drawing.framework.ToolbarController");
            xController = xFactory->createInstance(sServiceName);
            (*mpControllerContainer)[sServiceName] = xController;
            mxToolBarController = Reference<XToolBarController>(xController, UNO_QUERY);
            Reference<lang::XInitialization> xInit (xController, UNO_QUERY_THROW);
            xInit->initialize(aInitializationArguments);
        }
        if ( ! mxCommandController.is())
        {
            sServiceName = ::rtl::OUString::createFromAscii(
                "com.sun.star.drawing.framework.CommandController");
            xController = xFactory->createInstance(sServiceName);
            (*mpControllerContainer)[sServiceName] = xController;
            mxCommandController = Reference<XCommandController>(xController, UNO_QUERY);
            Reference<lang::XInitialization> xInit (xController, UNO_QUERY_THROW);
            xInit->initialize(aInitializationArguments);
        }

        // Put the controllers into the resource controller list.
        maResourceControllerList.realloc(4);
        maResourceControllerList[0]
            = Reference<XResourceController>(mxToolBarController, UNO_QUERY_THROW);
        maResourceControllerList[1]
            = Reference<XResourceController>(mxViewController, UNO_QUERY_THROW);
        maResourceControllerList[2]
            = Reference<XResourceController>(mxPaneController, UNO_QUERY_THROW);
        maResourceControllerList[3]
            = Reference<XResourceController>(mxCommandController, UNO_QUERY_THROW);
    }
    catch (RuntimeException&)
    {
        mxModuleController = NULL;
        mxPaneController = NULL;
        mxViewController = NULL;
        mxToolBarController = NULL;
        mxCommandController = NULL;
        mxConfigurationController = NULL;
        mpControllerContainer->clear();
        maResourceControllerList.realloc(0);
    }
}




Reference<XConfigurationController> SAL_CALL DrawController::getConfigurationController (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());
    if ( ! mxConfigurationController.is())
    {
        mxConfigurationController = Reference<XConfigurationController>(
            getController(
                ::rtl::OUString::createFromAscii(
                    "com.sun.star.drawing.framework.ConfigurationController")),
            UNO_QUERY);
    }
    return mxConfigurationController;
}




Reference<XModuleController> SAL_CALL DrawController::getModuleController (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    if ( ! mxModuleController.is())
    {
        mxModuleController = Reference<XModuleController>(
            getController(
                ::rtl::OUString::createFromAscii("com.sun.star.drawing.framework.ModuleController")),
            UNO_QUERY);
    }

    return mxModuleController;
}




Reference<XPaneController> SAL_CALL DrawController::getPaneController (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());
    if ( ! mxPaneController.is())
    {
        mxPaneController = Reference<XPaneController>(
            getController(
                ::rtl::OUString::createFromAscii("com.sun.star.drawing.framework.PaneController")),
            UNO_QUERY);
    }
    return mxPaneController;
}




Reference<XViewController> SAL_CALL DrawController::getViewController (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());
    if ( ! mxViewController.is())
    {
        mxViewController = Reference<XViewController>(
            getController(
                ::rtl::OUString::createFromAscii("com.sun.star.drawing.framework.ViewController")),
            UNO_QUERY);
    }
    return mxViewController;
}




Reference<XToolBarController> SAL_CALL DrawController::getToolBarController (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());
    if ( ! mxToolBarController.is())
    {
        mxToolBarController = Reference<XToolBarController>(
            getController(
                ::rtl::OUString::createFromAscii(
                    "com.sun.star.drawing.framework.ToolbarController")),
            UNO_QUERY);
    }
    return mxToolBarController;
}




Reference<XCommandController> SAL_CALL DrawController::getCommandController (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());
    if ( ! mxCommandController.is())
    {
        mxCommandController = Reference<XCommandController>(
            getController(
                ::rtl::OUString::createFromAscii(
                    "com.sun.star.drawing.framework.CommandController")),
            UNO_QUERY);
    }
    return mxCommandController;
}




void SAL_CALL DrawController::releaseController (
    const Reference<XInterface>& xController)
    throw (RuntimeException)
{
    (void)xController;
    throw RuntimeException(
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "DrawController::releaseController is not yet implemented")),
        const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
}




//===== XUnoTunnel ============================================================

const Sequence<sal_Int8>& DrawController::getUnoTunnelId (void)
{
    static ::com::sun::star::uno::Sequence<sal_Int8>* pSequence = NULL;
    if (pSequence == NULL)
    {
        ::osl::Guard< ::osl::Mutex > aGuard (::osl::Mutex::getGlobalMutex());
        if (pSequence == NULL)
        {
            static ::com::sun::star::uno::Sequence<sal_Int8> aSequence (16);
            rtl_createUuid((sal_uInt8*)aSequence.getArray(), 0, sal_True);
            pSequence = &aSequence;
        }
    }
    return *pSequence;
}




sal_Int64 SAL_CALL DrawController::getSomething (const Sequence<sal_Int8>& rId)
    throw (RuntimeException)
{
    sal_Int64 nResult = 0;

    if (rId.getLength() == 16
        && rtl_compareMemory(getUnoTunnelId().getConstArray(), rId.getConstArray(), 16) == 0)
    {
        nResult = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }

    return nResult;
}




//===== Properties ============================================================

void DrawController::FillPropertyTable (
    ::std::vector<beans::Property>& rProperties)
{
    rProperties.push_back(
        beans::Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM("VisibleArea") ),
            PROPERTY_WORKAREA,
            ::getCppuType((const ::com::sun::star::awt::Rectangle*)0),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY));
    rProperties.push_back(
        beans::Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM("CurrentPage") ),
            PROPERTY_CURRENTPAGE,
            ::getCppuType((const Reference< drawing::XDrawPage > *)0),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("IsLayerMode") ),
            PROPERTY_LAYERMODE,
            ::getCppuBooleanType(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("IsMasterPageMode") ),
            PROPERTY_MASTERPAGEMODE,
            ::getCppuBooleanType(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ActiveLayer") ),
            PROPERTY_ACTIVE_LAYER,
            ::getCppuBooleanType(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ZoomValue") ),
            PROPERTY_ZOOMVALUE,
            ::getCppuType((const sal_Int16*)0),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ZoomType") ),
            PROPERTY_ZOOMTYPE,
            ::getCppuType((const sal_Int16*)0),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("ViewOffset") ),
            PROPERTY_VIEWOFFSET,
            ::getCppuType((const ::com::sun::star::awt::Point*)0),
            beans::PropertyAttribute::BOUND ));

    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("PaneController") ),
            PROPERTY_PANE_CONTROLLER,
            ::getCppuType((const Reference<XInterface>*)0),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY));
}




IPropertyArrayHelper & DrawController::getInfoHelper()
{
    OGuard aGuard( Application::GetSolarMutex() );

    if (mpPropertyArrayHelper.get() == NULL)
    {
        ::std::vector<beans::Property> aProperties;
        FillPropertyTable (aProperties);
        Sequence<beans::Property> aPropertySequence (aProperties.size());
        for (unsigned int i=0; i<aProperties.size(); i++)
            aPropertySequence[i] = aProperties[i];
        mpPropertyArrayHelper.reset(new OPropertyArrayHelper(aPropertySequence, sal_False));
    }

    return *mpPropertyArrayHelper.get();
}




Reference < beans::XPropertySetInfo >  DrawController::getPropertySetInfo()
        throw ( ::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    static Reference < beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



sal_Bool DrawController::convertFastPropertyValue (
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bResult = sal_False;

    if (mpSubController.get() != NULL)
        bResult = mpSubController->convertFastPropertyValue(
            rConvertedValue,
            rOldValue,
            nHandle,
            rValue);

    return bResult;
}




void DrawController::setFastPropertyValue_NoBroadcast (
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( com::sun::star::uno::Exception)
{
    OGuard aGuard( Application::GetSolarMutex() );
    if (mpSubController.get() != NULL)
        mpSubController->setFastPropertyValue_NoBroadcast(nHandle, rValue);
}




void DrawController::getFastPropertyValue (
    Any & rRet,
    sal_Int32 nHandle ) const
{
    OGuard aGuard( Application::GetSolarMutex() );

    switch( nHandle )
    {
        case PROPERTY_WORKAREA:
            rRet <<= awt::Rectangle(
                maLastVisArea.Left(),
                maLastVisArea.Top(),
                maLastVisArea.GetWidth(),
                maLastVisArea.GetHeight());
            break;

        case PROPERTY_PANE_CONTROLLER:
            //            rRet <<= mxPaneController;
            break;

        default:
            if (mpSubController.get() != NULL)
                mpSubController->getFastPropertyValue(rRet,nHandle);
            break;
    }
}



void DrawController::DisposeSubControllers (void)
{
    // Dispose the named sub controllers in a defined order.
    DisposeSubController(mxConfigurationController);
    mxConfigurationController = NULL;

    DisposeSubController(mxViewController);
    mxViewController = NULL;

    DisposeSubController(mxPaneController);
    mxPaneController = NULL;

    DisposeSubController(mxToolBarController);
    mxToolBarController = NULL;

    DisposeSubController(mxCommandController);
    mxCommandController = NULL;

    // Dispose the other sub controllers.
    ControllerContainer::iterator iController;
    for (iController=mpControllerContainer->begin();
         iController!=mpControllerContainer->end();
         ++iController)
    {
        DisposeSubController(iController->second);
    }
}




void DrawController::DisposeSubController (const Reference<XInterface>& rxController)
{
    if (rxController.is())
    {
        try
        {
            Reference<XComponent> xComponent (rxController, UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            RemoveSubController(mxViewController);
        }
        catch (RuntimeException&)
        {
            DBG_ASSERT(false, "caught exception while disposing sub controller");
        }
    }
}




void DrawController::RemoveSubController (const Reference<XInterface>& rxController)
{
    ControllerContainer::iterator iController;
    for (iController=mpControllerContainer->begin();
         iController!=mpControllerContainer->end();
         ++iController)
    {
        if (iController->second == rxController)
        {
            mpControllerContainer->erase(iController);
            break;
        }
    }
}




void DrawController::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose || mbDisposing)
    {
        OSL_TRACE ("Calling disposed DrawController object. Throwing exception:");
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "DrawController object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}





} // end of namespace sd

