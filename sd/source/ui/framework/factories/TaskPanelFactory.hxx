/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TaskPanelFactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:35:09 $
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
