/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TaskPanelFactory.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_FRAMEWORK_TASK_PANEL_FACTORY_HXX
#define SD_FRAMEWORK_TASK_PANEL_FACTORY_HXX

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase2.hxx>

#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    css::lang::XInitialization,
    css::drawing::framework::XResourceFactory
    > TaskPanelFactoryInterfaceBase;

} // end of anonymous namespace.


namespace sd { class ViewShellBase; }

namespace sd { namespace framework {

/** This class creates panels for the task pane.
*/
class TaskPanelFactory
    : private ::cppu::BaseMutex,
      public TaskPanelFactoryInterfaceBase
{
public:
    TaskPanelFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~TaskPanelFactory (void);

    virtual void SAL_CALL disposing (void);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxResourcesId)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL releaseResource (
        const css::uno::Reference<
            css::drawing::framework::XResource>& rxResource)
        throw (css::uno::RuntimeException);

private:
    ViewShellBase* mpViewShellBase;

    void ThrowIfDisposed (void) const throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
