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

#ifndef OOX_SHAPE_FASTTOKENHANDLERSERVICE_HXX
#define OOX_SHAPE_FASTTOKENHANDLERSERVICE_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include "oox/core/fasttokenhandler.hxx"

namespace oox {
namespace shape {

// ============================================================================

typedef ::cppu::WeakImplHelper2< ::com::sun::star::lang::XServiceInfo, ::com::sun::star::xml::sax::XFastTokenHandler > FastTokenHandlerServiceBase;

class FastTokenHandlerService : public FastTokenHandlerServiceBase
{
public:
    explicit FastTokenHandlerService( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxCcontext );
    virtual ~FastTokenHandlerService();

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::xml::sax::XFastTokenHandler:
    virtual ::sal_Int32 SAL_CALL getToken(const ::rtl::OUString & Identifier) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getIdentifier(::sal_Int32 Token) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getUTF8Identifier( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8(const ::com::sun::star::uno::Sequence< ::sal_Int8 > & Identifier) throw (::com::sun::star::uno::RuntimeException);

private:
    FastTokenHandlerService(FastTokenHandlerService &); // not defined
    void operator =(FastTokenHandlerService &); // not defined

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::oox::core::FastTokenHandler mFastTokenHandler;
};

// ============================================================================

} // namspace shape
} // namspace oox

#endif
