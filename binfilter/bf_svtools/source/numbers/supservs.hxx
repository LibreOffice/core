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

#ifndef _SVTOOLS_NUMBERS_SUPPLIERSERVICE_HXX_
#define _SVTOOLS_NUMBERS_SUPPLIERSERVICE_HXX_

#ifndef _NUMUNO_HXX
#include "numuno.hxx"
#endif
#ifndef _ZFORLIST_HXX
#include <bf_svtools/zforlist.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif

namespace binfilter
{

//=========================================================================
//= SvNumberFormatsSupplierServiceObject - a number formats supplier which
//=				- can be instantiated as an service
//=				- supports the ::com::sun::star::io::XPersistObject interface
//=				- works with it's own SvNumberFormatter instance
//=				- can be initialized (::com::sun::star::lang::XInitialization)
//=					with a specific language (i.e. ::com::sun::star::lang::Locale)
//=========================================================================
class SvNumberFormatsSupplierServiceObject
            :protected SvNumberFormatsSupplierObj
            ,public ::com::sun::star::lang::XInitialization
            ,public ::com::sun::star::io::XPersistObject
            ,public ::com::sun::star::lang::XServiceInfo
{	// don't want the Set-/GetNumberFormatter to be accessable from outside

    friend ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL SvNumberFormatsSupplierServiceObject_CreateInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

protected:
    SvNumberFormatter*	m_pOwnFormatter;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;

public:
    SvNumberFormatsSupplierServiceObject(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
    ~SvNumberFormatsSupplierServiceObject();

    // XInterface
    virtual void SAL_CALL acquire() throw() { SvNumberFormatsSupplierObj::acquire(); }
    virtual void SAL_CALL release() throw() { SvNumberFormatsSupplierObj::release(); }
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException)
        { return SvNumberFormatsSupplierObj::queryInterface(_rType); }

    // XAggregation
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XNumberFormatsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
                getNumberFormatSettings() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > SAL_CALL
                getNumberFormats() throw(::com::sun::star::uno::RuntimeException);

    // XUnoTunnler
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);

protected:
    void implEnsureFormatter();
};

}

#endif // _SVTOOLS_NUMBERS_SUPPLIERSERVICE_HXX_

