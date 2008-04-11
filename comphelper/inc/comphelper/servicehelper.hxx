/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: servicehelper.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _UTL_SERVICEHELPER_HXX_
#define _UTL_SERVICEHELPER_HXX_

/** the UNO3_GETIMPLEMENTATION_* macros  implement a static helper function
    that gives access to your implementation for a given interface reference,
    if possible.

    Example:
        MyClass* pClass = MyClass::getImplementation( xRef );

    Usage:
        Put a UNO3_GETIMPLEMENTATION_DECL( classname ) inside your class
        definitian and UNO3_GETIMPLEMENTATION_IMPL( classname ) inside
        your cxx file. Your class must inherit ::com::sun::star::uno::XUnoTunnel
        and export it with queryInterface. Implementation of XUnoTunnel is
        done by this macro.
*/
#define UNO3_GETIMPLEMENTATION_DECL( classname ) \
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw(); \
    static classname* getImplementation( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt ) throw(); \
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

#define UNO3_GETIMPLEMENTATION_BASE_IMPL( classname ) \
const ::com::sun::star::uno::Sequence< sal_Int8 > & classname::getUnoTunnelId() throw() \
{ \
    static ::com::sun::star::uno::Sequence< sal_Int8 > * pSeq = 0; \
    if( !pSeq ) \
    { \
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if( !pSeq ) \
        { \
            static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( 16 ); \
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True ); \
            pSeq = &aSeq; \
        } \
    } \
    return *pSeq; \
} \
\
classname* classname::getImplementation( uno::Reference< uno::XInterface > xInt ) throw() \
{ \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUT( xInt, ::com::sun::star::uno::UNO_QUERY ); \
    if( xUT.is() ) \
        return reinterpret_cast<classname*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( classname::getUnoTunnelId() ))); \
    else \
        return NULL; \
}

#define UNO3_GETIMPLEMENTATION_IMPL( classname )\
UNO3_GETIMPLEMENTATION_BASE_IMPL(classname)\
sal_Int64 SAL_CALL classname::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), \
                                                         rId.getConstArray(), 16 ) ) \
    { \
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this)); \
    } \
    return 0; \
}

#define UNO3_GETIMPLEMENTATION2_IMPL( classname, baseclass )\
UNO3_GETIMPLEMENTATION_BASE_IMPL(classname)\
sal_Int64 SAL_CALL classname::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw(::com::sun::star::uno::RuntimeException) \
{ \
    if( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), \
                                                         rId.getConstArray(), 16 ) ) \
    { \
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this)); \
    } \
    else \
    { \
        return baseclass::getSomething( rId ); \
    } \
}


#endif // _UTL_SERVICEHELPER_HXX_

