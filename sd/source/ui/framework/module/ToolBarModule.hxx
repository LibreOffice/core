/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ToolBarModule.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:41:15 $
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

#ifndef SD_FRAMEWORK_TOOL_BAR_MODULE_HXX
#define SD_FRAMEWORK_TOOL_BAR_MODULE_HXX

#include "ToolBarManager.hxx"
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sd {
class ViewShellBase;
}



namespace sd { namespace framework {

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        ::css::drawing::framework::XConfigurationChangeListener
        > ToolBarModuleInterfaceBase;
}


/** This module is responsible for locking the ToolBarManager during
    configuration updates and for triggering ToolBarManager updates.
*/
class ToolBarModule
    : private ::cppu::BaseMutex,
      public ToolBarModuleInterfaceBase
{
public:
    /** Create a new module.
        @param rxController
            This is the access point to the drawing framework.
    */
    ToolBarModule (
        const css::uno::Reference<css::frame::XController>& rxController);
    virtual ~ToolBarModule (void);

    virtual void SAL_CALL disposing (void);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<
        css::drawing::framework::XConfigurationController> mxConfigurationController;
    ViewShellBase* mpBase;
    ::boost::scoped_ptr<ToolBarManager::UpdateLock> mpToolBarManagerLock;
    bool mbMainViewSwitchUpdatePending;

    void HandleUpdateStart (void);
    void HandleUpdateEnd (void);
};

} } // end of namespace sd::framework

#endif
