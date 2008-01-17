/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FastTokenHandlerService.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:06 $
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
#ifndef OOX_SHAPE_FAST_TOKEN_HANDLER_SERVICE_HXX
#define OOX_SHAPE_FAST_TOKEN_HANDLER_SERVICE_HXX

#include <oox/core/fasttokenhandler.hxx>

#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/implbase2.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/xml/sax/XFastTokenHandler.hpp"

namespace css = ::com::sun::star;

namespace oox {
namespace shape {

class FastTokenHandlerService:
    public ::cppu::WeakImplHelper2<
        css::lang::XServiceInfo,
        css::xml::sax::XFastTokenHandler>
{
public:
    explicit FastTokenHandlerService(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::xml::sax::XFastTokenHandler:
    virtual ::sal_Int32 SAL_CALL getToken(const ::rtl::OUString & Identifier) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getIdentifier(::sal_Int32 Token) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8(const css::uno::Sequence< ::sal_Int8 > & Identifier) throw (css::uno::RuntimeException);

private:
    FastTokenHandlerService(FastTokenHandlerService &); // not defined
    void operator =(FastTokenHandlerService &); // not defined

    virtual ~FastTokenHandlerService() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    FastTokenHandler mFastTokenHandler;
};

::rtl::OUString SAL_CALL FastTokenHandlerService_getImplementationName();

css::uno::Sequence< ::rtl::OUString > SAL_CALL FastTokenHandlerService_getSupportedServiceNames();

css::uno::Reference< css::uno::XInterface > SAL_CALL _FastTokenHandlerService_create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
    SAL_THROW((css::uno::Exception));

}}
#endif // OOX_SHAPE_FAST_TOKEN_HANDLER_SERVICE_HXX
