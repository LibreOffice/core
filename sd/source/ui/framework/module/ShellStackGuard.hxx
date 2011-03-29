/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
