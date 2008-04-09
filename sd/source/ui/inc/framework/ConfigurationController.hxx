/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationController.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-09 16:27:42 $
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
