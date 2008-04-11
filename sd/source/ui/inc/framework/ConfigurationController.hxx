/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConfigurationController.hxx,v $
 * $Revision: 1.5 $
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

#ifndef SD_FRAMEWORK_CONFIGURATION_CONTROLLER_HXX
#define SD_FRAMEWORK_CONFIGURATION_CONTROLLER_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XResourceFactoryManager.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/compbase2.hxx>
#include <tools/link.hxx>
#include <rtl/ref.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    ::css::drawing::framework::XConfigurationController,
    ::css::lang::XInitialization
    > ConfigurationControllerInterfaceBase;

} // end of anonymous namespace.


namespace sd { class ViewShellBase; }


namespace sd { namespace framework {

class ChangeRequestQueueProcessor;
class ConfigurationControllerBroadcaster;
class ConfigurationUpdater;
class ConfigurationUpdaterLock;

/** The configuration controller is responsible for maintaining the current
    configuration.

    @see css::drawing::framework::XConfigurationController
        for an extended documentation.
*/
class ConfigurationController
    : private sd::MutexOwner,
      private boost::noncopyable,
      public ConfigurationControllerInterfaceBase
{
public:
    ConfigurationController (void) throw();
    virtual ~ConfigurationController (void) throw();

    virtual void SAL_CALL disposing (void);

    void ProcessEvent (void);


    // XConfigurationController

    virtual void SAL_CALL lock (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL unlock (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL requestResourceActivation (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        css::drawing::framework::ResourceActivationMode eMode)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL requestResourceDeactivation (
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL getResource (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL update (void)
        throw (css::uno::RuntimeException);

    virtual  css::uno::Reference<
        css::drawing::framework::XConfiguration>
        SAL_CALL getRequestedConfiguration (void)
        throw (css::uno::RuntimeException);

    virtual  css::uno::Reference<
        css::drawing::framework::XConfiguration>
        SAL_CALL getCurrentConfiguration (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL restoreConfiguration (
        const css::uno::Reference<css::drawing::framework::XConfiguration>&
        rxConfiguration)
        throw (css::uno::RuntimeException);


    // XConfigurationControllerBroadcaster

    virtual void SAL_CALL addConfigurationChangeListener (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener>& rxListener,
        const ::rtl::OUString& rsEventType,
        const css::uno::Any& rUserData)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeConfigurationChangeListener (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener>& rxListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL notifyEvent (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XConfigurationRequestQueue

    virtual sal_Bool SAL_CALL hasPendingRequests (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL postChangeRequest (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeRequest>& rxRequest)
        throw (css::uno::RuntimeException);


    // XResourceFactoryManager

    virtual void SAL_CALL addResourceFactory(
        const ::rtl::OUString& sResourceURL,
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxResourceFactory)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeResourceFactoryForURL(
        const ::rtl::OUString& sResourceURL)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removeResourceFactoryForReference(
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxResourceFactory)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::framework::XResourceFactory>
        SAL_CALL getResourceFactory (
        const ::rtl::OUString& sResourceURL)
        throw (css::uno::RuntimeException);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    /** Use this class instead of calling lock() and unlock() directly in
        order to be exception safe.
    */
    class Lock
    {
    public:
        Lock (const css::uno::Reference<
            css::drawing::framework::XConfigurationController>& rxController);
        ~Lock (void);
    private:
        css::uno::Reference<
            css::drawing::framework::XConfigurationController> mxController;
    };

private:
    class Implementation;
    ::boost::scoped_ptr<Implementation> mpImplementation;
    bool mbIsDisposed;

    /** When the called object has already been disposed this method throws
        an exception and does not return.
    */
    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
