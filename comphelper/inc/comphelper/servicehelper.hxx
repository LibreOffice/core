/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _COMPHELPER_SERVICEHELPER_HXX_
#define _COMPHELPER_SERVICEHELPER_HXX_

#include <rtl/uuid.h>
#include <rtl/instance.hxx>
#include <com/sun/star/uno/Sequence.hxx>

class UnoTunnelIdInit
{
private:
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aSeq;
public:
    UnoTunnelIdInit() : m_aSeq(16)
    {
        rtl_createUuid( (sal_uInt8*)m_aSeq.getArray(), 0, sal_True );
    }
    const ::com::sun::star::uno::Sequence< sal_Int8 >& getSeq() const { return m_aSeq; }
};

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
    static classname* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xInt ); \
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

#define UNO3_GETIMPLEMENTATION_BASE_IMPL( classname ) \
namespace \
{ \
    class the##classname##UnoTunnelId : public rtl::Static< UnoTunnelIdInit, the##classname##UnoTunnelId> {}; \
} \
const ::com::sun::star::uno::Sequence< sal_Int8 > & classname::getUnoTunnelId() throw() \
{ \
    return the##classname##UnoTunnelId::get().getSeq(); \
} \
\
classname* classname::getImplementation( const uno::Reference< uno::XInterface >& xInt ) \
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
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), \
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
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), \
                                                         rId.getConstArray(), 16 ) ) \
    { \
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this)); \
    } \
    else \
    { \
        return baseclass::getSomething( rId ); \
    } \
}


#endif // _COMPHELPER_SERVICEHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
