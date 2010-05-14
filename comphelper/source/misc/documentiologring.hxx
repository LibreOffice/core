/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentiologring.hxx,v $
 * $Revision: 1.0 $
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

#ifndef __DOCUMENTIOLOGRING_HXX_
#define __DOCUMENTIOLOGRING_HXX_

#include <com/sun/star/logging/XSimpleLogRing.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase3.hxx>

#define SIMPLELOGRING_SIZE 256

namespace comphelper
{

class OSimpleLogRing : public ::cppu::WeakImplHelper3< ::com::sun::star::logging::XSimpleLogRing,
                                                           ::com::sun::star::lang::XInitialization,
                                                           ::com::sun::star::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > m_aMessages;

    sal_Bool m_bInitialized;
    sal_Bool m_bFull;
    sal_Int32 m_nPos;

public:
    OSimpleLogRing( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
    virtual ~OSimpleLogRing();

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
            impl_staticGetSupportedServiceNames();

    static ::rtl::OUString SAL_CALL impl_staticGetImplementationName();

    static ::rtl::OUString SAL_CALL impl_staticGetSingletonName();

    static ::rtl::OUString SAL_CALL impl_staticGetServiceName();

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

// XSimpleLogRing
    virtual void SAL_CALL logString( const ::rtl::OUString& aMessage ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCollectedLog() throw (::com::sun::star::uno::RuntimeException);

// XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

};

} // namespace comphelper

#endif

