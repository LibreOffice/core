/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationController.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:10:18 $
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

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCHANGEREQUEST_HPP_
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATION_HPP_
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONTROLLERMANAGER_HPP_
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_CONFIGURATIONCHANGEEVENT_HPP_
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XRESOURCEID_HPP_
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#include <tools/link.hxx>
#include <rtl/ref.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    ::com::sun::star::drawing::framework::XConfigurationController,
    ::com::sun::star::lang::XInitialization
    > ConfigurationControllerInterfaceBase;

} // end of anonymous namespace.


namespace sd { class ViewShellBase; }


namespace sd { namespace framework {

class ChangeRequestQueueProcessor;
class ConfigurationControllerBroadcaster;
class ConfigurationUpdater;
class ConfigurationUpdaterLock;

/** The configuration controller is responsible for the synchronization of
    other controllers for the management of resources like panes, views,
    tool bars, and command groups.

    @see com::sun::star::drawing::framework::XConfigurationController
        for an extended documentation.
*/
class ConfigurationController
    : private sd::MutexOwner,
      private boost::noncopyable,
      public ConfigurationControllerInterfaceBase
{
public:
    /** Create a new instance of this class for the given component
        context.
    */
    static ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::framework::XConfigurationController> Create (
        const ::com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& rxContext);
        // NOTHROW

    virtual void SAL_CALL disposing (void);


    // XConfigurationController

    virtual void SAL_CALL lock (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL unlock (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL requestResourceActivation (
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>&
            rxResourceId,
        ::com::sun::star::drawing::framework::ResourceActivationMode eMode)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL requestResourceDeactivation (
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>&
            rxResourceId)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL update (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual  ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfiguration>
        SAL_CALL getConfiguration (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL restoreConfiguration (
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfiguration>&
        rxConfiguration)
        throw (::com::sun::star::uno::RuntimeException);


    // XConfigurationControllerBroadcaster

    virtual void SAL_CALL addConfigurationChangeListener (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationChangeListener>& rxListener,
        const ::rtl::OUString& rsEventType,
        const ::com::sun::star::uno::Any& rUserData)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeConfigurationChangeListener (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationChangeListener>& rxListener)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL notifyEvent (
        const ::com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);


    // XConfigurationRequestQueue

    virtual sal_Bool SAL_CALL hasPendingRequests (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL postChangeRequest (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationChangeRequest>& rxRequest)
        throw (::com::sun::star::uno::RuntimeException);



    // XInitialization

    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence<com::sun::star::uno::Any>& rArguments)
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);


    /** Use this class instead of calling lock() and unlock() directly in
        order to be exception safe.
    */
    class Lock
    {
    public:
        Lock (const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationController>& rxController);
        ~Lock (void);
    private:
        ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XConfigurationController> mxController;
    };

private:

   /** The queue processor ownes the queue of configuration change request
        objects and processes the objects.
    */
    ::boost::scoped_ptr<ChangeRequestQueueProcessor> mpQueueProcessor;

    /** The Broadcaster class implements storing and calling of listeners.
    */
    ::boost::scoped_ptr<ConfigurationControllerBroadcaster> mpBroadcaster;

    /** The requested configuration which is modifed (asynchronously) by
        calls to requestResourceActivation() and
        requestResourceDeactivation().  The mpConfigurationUpdater makes the
        current configuration reflect the content of this one.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfiguration> mxRequestedConfiguration;

    ViewShellBase* mpBase;

    bool mbIsInitialized;

    ::boost::shared_ptr<ConfigurationUpdater> mpConfigurationUpdater;
    ::boost::shared_ptr<ConfigurationUpdaterLock> mpConfigurationUpdaterLock;

    sal_Int32 mnLockCount;

    ConfigurationController (void) throw();
    void Initialize (void);
    virtual ~ConfigurationController (void) throw();

    /** When the called object has already been disposed this method throws
        an exception and does not return.
    */
    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
