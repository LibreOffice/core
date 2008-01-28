/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationController.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 16:36:37 $
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

#include "precompiled_sd.hxx"

#include "framework/ConfigurationController.hxx"

#include "framework/Configuration.hxx"
#include "framework/FrameworkHelper.hxx"
#include "ConfigurationUpdater.hxx"
#include "ConfigurationControllerBroadcaster.hxx"
#include "ConfigurationTracer.hxx"
#include "GenericConfigurationChangeRequest.hxx"
#include "UpdateRequest.hxx"
#include "ChangeRequestQueueProcessor.hxx"
#include "ConfigurationClassifier.hxx"
#include "ViewShellBase.hxx"
#include "UpdateLockManager.hxx"
#include "DrawController.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONTROLLERMANAGER_HPP_
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#include <comphelper/stl_types.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using rtl::OUString;
using ::sd::framework::FrameworkHelper;

#undef VERBOSE
//#define VERBOSE 2


namespace sd { namespace framework {

Reference<XInterface> SAL_CALL ConfigurationController_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(ConfigurationController::Create(rxContext), UNO_QUERY);
}




OUString ConfigurationController_getImplementationName (void) throw(RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Draw.framework.configuration.ConfigurationController"));
}




Sequence<rtl::OUString> SAL_CALL ConfigurationController_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const OUString sServiceName(OUString::createFromAscii(
        "com.sun.star.drawing.framework.ConfigurationController"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== ConfigurationController::Lock =========================================

ConfigurationController::Lock::Lock (const Reference<XConfigurationController>& rxController)
    : mxController(rxController)
{
    OSL_ASSERT(mxController.is());

    if (mxController.is())
        mxController->lock();
}




ConfigurationController::Lock::~Lock (void)
{
    if (mxController.is())
        mxController->unlock();
}




//===== ConfigurationController ===============================================

Reference<XConfigurationController> ConfigurationController::Create (
    const Reference<XComponentContext>& rxContext) // NOTHROW
{
    (void)rxContext;
    ::rtl::Reference<ConfigurationController> pConfigurationController (
        new ConfigurationController());
    pConfigurationController->Initialize();
    return Reference<XConfigurationController>(pConfigurationController.get());
}




ConfigurationController::ConfigurationController (void) throw()
    : ConfigurationControllerInterfaceBase(MutexOwner::maMutex),
      mpQueueProcessor(),
      mpBroadcaster(),
      mxRequestedConfiguration(),
      mpBase(NULL),
      mbIsInitialized(false),
      mpConfigurationUpdater(),
      mpConfigurationUpdaterLock(),
      mnLockCount(0)
{
}




void ConfigurationController::Initialize (void)
{
    mpBroadcaster.reset(new ConfigurationControllerBroadcaster(this));
    mpConfigurationUpdater.reset(new ConfigurationUpdater(this));
    mpQueueProcessor.reset(new ChangeRequestQueueProcessor(this,mpConfigurationUpdater));
    mxRequestedConfiguration = Reference<XConfiguration>(new Configuration(this, true));
    mpQueueProcessor->SetConfiguration(mxRequestedConfiguration);
}




ConfigurationController::~ConfigurationController (void) throw()
{
}




void SAL_CALL ConfigurationController::disposing (void)
{
    // Release the listeners.
    lang::EventObject aEvent;
    aEvent.Source = uno::Reference<uno::XInterface>((cppu::OWeakObject*)this);

    {
        const ::vos::OGuard aSolarGuard (Application::GetSolarMutex());
        mpBroadcaster->DisposeAndClear();
    }

    mpQueueProcessor.reset();
    mpConfigurationUpdater.reset();
    mxRequestedConfiguration = NULL;
}




//----- XConfigurationControllerBroadcaster -----------------------------------

void SAL_CALL ConfigurationController::addConfigurationChangeListener (
    const Reference<XConfigurationChangeListener>& rxListener,
    const ::rtl::OUString& rsEventType,
    const Any& rUserData)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    ThrowIfDisposed();
    mpBroadcaster->AddListener(rxListener, rsEventType, rUserData);
}




void SAL_CALL ConfigurationController::removeConfigurationChangeListener (
    const Reference<XConfigurationChangeListener>& rxListener)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    ThrowIfDisposed();
    mpBroadcaster->RemoveListener(rxListener);
}




void SAL_CALL ConfigurationController::notifyEvent (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    mpBroadcaster->NotifyListeners(rEvent);
}





//----- XConfigurationController ----------------------------------------------

void SAL_CALL ConfigurationController::lock (void)
    throw (RuntimeException)
{
    OSL_ASSERT(mpConfigurationUpdater.get()!=NULL);

    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();


    ++mnLockCount;
    if (mpConfigurationUpdaterLock.get()==NULL)
        mpConfigurationUpdaterLock = mpConfigurationUpdater->GetLock();
}




void SAL_CALL ConfigurationController::unlock (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    OSL_ASSERT(mnLockCount>0);
    --mnLockCount;
    if (mnLockCount == 0)
        mpConfigurationUpdaterLock.reset();
}




void SAL_CALL ConfigurationController::requestResourceActivation (
    const Reference<XResourceId>& rxResourceId,
    ResourceActivationMode eMode)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
       ThrowIfDisposed();

#if defined VERBOSE && VERBOSE>=2
    OSL_TRACE("ConfigurationController::requestResourceActivation() %s",
        OUStringToOString(
            FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif

    if (rxResourceId.is())
    {
        if (eMode == ResourceActivationMode_REPLACE)
        {
            // Get a list of the matching resources and create deactivation
            // requests for them.
            Sequence<Reference<XResourceId> > aResourceList (
                mxRequestedConfiguration->getResources(
                    rxResourceId->getAnchor(),
                    rxResourceId->getResourceTypePrefix(),
                    AnchorBindingMode_DIRECT));

            for (sal_Int32 nIndex=0; nIndex<aResourceList.getLength(); ++nIndex)
            {
                Reference<XConfigurationChangeRequest> xRequest(
                    new GenericConfigurationChangeRequest(
                        aResourceList[nIndex],
                        GenericConfigurationChangeRequest::Deactivation));
                postChangeRequest(xRequest);
            }
        }

        Reference<XConfigurationChangeRequest> xRequest(
            new GenericConfigurationChangeRequest(
                rxResourceId,
                GenericConfigurationChangeRequest::Activation));
        postChangeRequest(xRequest);
    }
}




void SAL_CALL ConfigurationController::requestResourceDeactivation (
    const Reference<XResourceId>& rxResourceId)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

#if defined VERBOSE && VERBOSE>=2
    OSL_TRACE("ConfigurationController::requestResourceDeactivation() %s",
            OUStringToOString(
                FrameworkHelper::ResourceIdToString(rxResourceId), RTL_TEXTENCODING_UTF8).getStr());
#endif

    if (rxResourceId.is())
    {
        // Request deactivation of all resources linked to the specified one
        // as well.
        const Sequence<Reference<XResourceId> > aLinkedResources (
            mxRequestedConfiguration->getResources(
                rxResourceId,
                OUString(),
                AnchorBindingMode_INDIRECT));
        const sal_Int32 nCount (aLinkedResources.getLength());
        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            // We do not add deactivation requests directly but call this
            // method recursively, so that when one time there are resources
            // linked to linked resources, these are handled correctly, too.
            requestResourceDeactivation(aLinkedResources[nIndex]);
        }

        // Add a deactivation request for the specified resource.
        Reference<XConfigurationChangeRequest> xRequest(
            new GenericConfigurationChangeRequest(
                rxResourceId,
                GenericConfigurationChangeRequest::Deactivation));
        postChangeRequest(xRequest);
    }
}




void SAL_CALL ConfigurationController::update (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    if (mpQueueProcessor->IsEmpty())
    {
        // The queue is empty.  Add another request that does nothing but
        // asynchronously trigger a request for an update.
        mpQueueProcessor->AddRequest(new UpdateRequest());
    }
    else
    {
        // The queue is not empty, so we rely on the queue processor to
        // request an update automatically when the queue becomes empty.
    }
}




sal_Bool SAL_CALL ConfigurationController::hasPendingRequests (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    return ! mpQueueProcessor->IsEmpty();
}





void SAL_CALL ConfigurationController::postChangeRequest (
    const Reference<XConfigurationChangeRequest>& rxRequest)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    mpQueueProcessor->AddRequest(rxRequest);
}




Reference<XConfiguration> SAL_CALL ConfigurationController::getConfiguration (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    if (mxRequestedConfiguration.is())
        return Reference<XConfiguration>(mxRequestedConfiguration->createClone(), UNO_QUERY);
    else
        return Reference<XConfiguration>();
}




/** The given configuration is restored by generating the appropriate set of
    activation and deactivation requests.
*/
void SAL_CALL ConfigurationController::restoreConfiguration (
    const Reference<XConfiguration>& rxNewConfiguration)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    // We will probably be making a couple of activation and deactivation
    // requests so lock the configuration controller and let it later update
    // all changes at once.
    ::boost::shared_ptr<ConfigurationUpdaterLock> pLock (mpConfigurationUpdater->GetLock());

    // Get lists of resources that are to be activated or deactivated.
    Reference<XConfiguration> xCurrentConfiguration (
        mpConfigurationUpdater->GetCurrentConfiguration());
#if defined VERBOSE && VERBOSE>=1
    OSL_TRACE("ConfigurationController::restoreConfiguration(");
    ConfigurationTracer::TraceConfiguration(rxNewConfiguration, "requested configuration");
    ConfigurationTracer::TraceConfiguration(xCurrentConfiguration, "current configuration");
#endif
    ConfigurationClassifier aClassifier (rxNewConfiguration, xCurrentConfiguration);
    aClassifier.Partition();
#if defined VERBOSE && VERBOSE>=3
    aClassifier.TraceResourceIdVector(
        "requested but not current resources:", aClassifier.GetC1minusC2());
    aClassifier.TraceResourceIdVector(
        "current but not requested resources:", aClassifier.GetC2minusC1());
    aClassifier.TraceResourceIdVector(
        "requested and current resources:", aClassifier.GetC1andC2());
#endif

    ConfigurationClassifier::ResourceIdVector::const_iterator iResource;

    // Request the deactivation of resources that are not requested in the
    // new configuration.
    const ConfigurationClassifier::ResourceIdVector& rResourcesToDeactivate (
        aClassifier.GetC2minusC1());
    for (iResource=rResourcesToDeactivate.begin();
         iResource!=rResourcesToDeactivate.end();
         ++iResource)
    {
        requestResourceDeactivation(*iResource);
    }

    // Request the activation of resources that are requested in the
    // new configuration but are not part of the current configuration.
    const ConfigurationClassifier::ResourceIdVector& rResourcesToActivate (
        aClassifier.GetC1minusC2());
    for (iResource=rResourcesToActivate.begin();
         iResource!=rResourcesToActivate.end();
         ++iResource)
    {
        requestResourceActivation(*iResource, ResourceActivationMode_ADD);
    }

    pLock.reset();
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL ConfigurationController::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    if (aArguments.getLength() > 0)
    {
        const ::vos::OGuard aSolarGuard (Application::GetSolarMutex());

        mpConfigurationUpdater->SetControllerManager(
            Reference<XControllerManager>(aArguments[0], UNO_QUERY));

        // Tunnel through the controller to obtain a ViewShellBase.
        Reference<lang::XUnoTunnel> xTunnel (aArguments[0], UNO_QUERY);
        if (xTunnel.is())
        {
            ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
                xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
            if (pController != NULL)
                mpBase = pController->GetViewShellBase();
        }
    }
}




//-----------------------------------------------------------------------------

void ConfigurationController::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "ConfigurationController object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




} } // end of namespace sd::framework
