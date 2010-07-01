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

#ifndef SD_FRAMEWORK_RESOURCE_MANAGER_HXX
#define SD_FRAMEWORK_RESOURCE_MANAGER_HXX

#include "MutexOwner.hxx"
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <cppuhelper/compbase1.hxx>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper1 <
    ::com::sun::star::drawing::framework::XConfigurationChangeListener
    > ResourceManagerInterfaceBase;

} // end of anonymous namespace.


namespace sd { namespace framework {

/** Manage the activation state of one resource depending on the view in the
    center pane.  The ResourceManager remembers in which configuration to
    activate and in which to deactivate the resource.  When the resource is
    deactivated or activated manually by the user then the ResourceManager
    detects this and remembers it for the future.
*/
class ResourceManager
    : private sd::MutexOwner,
      public ResourceManagerInterfaceBase
{
public:
    ResourceManager (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XController>& rxController,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxResourceId);
    virtual ~ResourceManager (void);

    /** Remember the given URL as one of a center pane view for which to
        activate the resource managed by the called object.
    */
    void AddActiveMainView (const ::rtl::OUString& rsMainViewURL);

    virtual void SAL_CALL disposing (void);

    /** Allow the ResourceManager to make resource activation or
        deactivation requests.
    */
    void Enable (void);

    /** Disable the resource management.  When called, the ResourceManager
        requests the resource to be deactivated.  Until enabled again it
        does not make any further requests for resource activation or
        deactivation.

        Call this for example to hide resources in read-only mode.
    */
    void Disable (void);

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (com::sun::star::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);

protected:
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfigurationController>
        mxConfigurationController;

private:
    class MainViewContainer;
    ::boost::scoped_ptr<MainViewContainer> mpActiveMainViewContainer;

    /// The resource managed by this class.
    css::uno::Reference<css::drawing::framework::XResourceId> mxResourceId;

    /// The anchor of the main view.
    css::uno::Reference<css::drawing::framework::XResourceId> mxMainViewAnchorId;

    ::rtl::OUString msCurrentMainViewURL;
    bool mbIsEnabled;

    void HandleMainViewSwitch (
        const ::rtl::OUString& rsViewURL,
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XConfiguration>& rxConfiguration,
        const bool bIsActivated);
    void HandleResourceRequest(
        bool bActivation,
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XConfiguration>& rxConfiguration);
    void UpdateForMainViewShell (void);
};

} } // end of namespace sd::framework

#endif
