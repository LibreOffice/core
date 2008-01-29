/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ShellStackGuard.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:20:09 $
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

#ifndef SD_FRAMEWORK_SHELL_STACK_GUARD_HXX
#define SD_FRAMEWORK_SHELL_STACK_GUARD_HXX

#include <cppuhelper/basemutex.hxx>

#include "framework/ConfigurationController.hxx"

#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>

#include <vcl/timer.hxx>
#include <cppuhelper/compbase1.hxx>
#include <boost/scoped_ptr.hpp>


namespace css = ::com::sun::star;


namespace {

typedef ::cppu::WeakComponentImplHelper1 <
    css::drawing::framework::XConfigurationChangeListener
    > ShellStackGuardInterfaceBase;

} // end of anonymous namespace.

namespace sd {

class ViewShellBase;

}




namespace sd { namespace framework {

/** This module locks updates of the current configuration in situations
    when the shell stack must not be modified.

    On every start of a configuration update the ShellStackGuard checks the
    printer.  If it is printing the configuration update is locked.  It then
    polls the printer and unlocks updates when printing finishes.

    When in the future there are no resources left that use shells then this
    module can be removed.
*/
class ShellStackGuard
    : private ::cppu::BaseMutex,
      public ShellStackGuardInterfaceBase
{
public:
    ShellStackGuard (css::uno::Reference<css::frame::XController>& rxController);
    virtual ~ShellStackGuard (void);

    virtual void SAL_CALL disposing (void);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    ViewShellBase* mpBase;
    ::boost::scoped_ptr<ConfigurationController::Lock> mpUpdateLock;
    Timer maPrinterPollingTimer;

    DECL_LINK(TimeoutHandler, Timer*);

    /** Return <TRUE/> when the printer is printing.  Return <FALSE/> when
        the printer is not printing, or there is no printer, or someting
        else went wrong.
    */
    bool IsPrinting (void) const;
};

} } // end of namespace sd::framework

#endif
