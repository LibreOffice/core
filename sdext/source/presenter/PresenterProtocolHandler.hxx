/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterProtocolHandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:03:05 $
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

#ifndef SDEXT_PRESENTER_PRESENTER_PROTOCOL_HANDLER_HXX
#define SDEXT_PRESENTER_PRESENTER_PROTOCOL_HANDLER_HXX

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {


namespace {
    typedef ::cppu::WeakComponentImplHelper3 <
        css::lang::XInitialization,
        css::frame::XDispatchProvider,
        css::frame::XDispatch
    > PresenterProtocolHandlerInterfaceBase;
}


class PresenterProtocolHandler
    : protected ::cppu::BaseMutex,
      public PresenterProtocolHandlerInterfaceBase
{
public:
    PresenterProtocolHandler (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterProtocolHandler (void);

    void SAL_CALL disposing (void);

    static ::rtl::OUString getImplementationName_static (void);
    static css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static (void);
    static css::uno::Reference<css::uno::XInterface> Create(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        SAL_THROW((css::uno::Exception));


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // XDispatchProvider

    virtual css::uno::Reference<css::frame::XDispatch > SAL_CALL
        queryDispatch (
            const css::util::URL& aURL,
            const rtl::OUString& aTargetFrameName,
            sal_Int32 nSearchFlags )
        throw(css::uno::RuntimeException);

    virtual css::uno::Sequence<css::uno::Reference<css::frame::XDispatch> > SAL_CALL
        queryDispatches(
            const css::uno::Sequence< css::frame::DispatchDescriptor>& rDescriptors)
        throw(css::uno::RuntimeException);


    // XDispatch
    virtual void SAL_CALL dispatch(
        const css::util::URL& aURL,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL addStatusListener(
        const css::uno::Reference<css::frame::XStatusListener>& rxListener,
        const css::util::URL& rURL)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL removeStatusListener (
        const css::uno::Reference<css::frame::XStatusListener>& rxListener,
        const css::util::URL& rURL)
        throw(css::uno::RuntimeException);

private:

    void ThrowIfDisposed (void) const throw (css::lang::DisposedException);
};

} }

#endif
