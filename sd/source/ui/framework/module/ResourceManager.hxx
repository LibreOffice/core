/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResourceManager.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:39:54 $
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

    void Trace (void) const;
};

} } // end of namespace sd::framework

#endif
