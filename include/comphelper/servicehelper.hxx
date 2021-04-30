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

#ifndef INCLUDED_COMPHELPER_SERVICEHELPER_HXX
#define INCLUDED_COMPHELPER_SERVICEHELPER_HXX

#include <rtl/uuid.h>
#include <rtl/instance.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/Sequence.hxx>

class UnoTunnelIdInit
{
private:
    css::uno::Sequence< sal_Int8 > m_aSeq;
public:
    UnoTunnelIdInit() : m_aSeq(16)
    {
        rtl_createUuid( reinterpret_cast<sal_uInt8*>(m_aSeq.getArray()), nullptr, true );
    }
    const css::uno::Sequence< sal_Int8 >& getSeq() const { return m_aSeq; }
};

namespace comphelper {

    template<class T>
    T* getUnoTunnelImplementation( const css::uno::Reference< css::uno::XInterface >& xIface )
    {
        css::uno::Reference< css::lang::XUnoTunnel > xUT( xIface, css::uno::UNO_QUERY );
        if (!xUT.is())
            return nullptr;

        return reinterpret_cast<T*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( T::getUnoTunnelId() )));
    }

}

template <typename T>
inline bool isUnoTunnelId(const css::uno::Sequence< sal_Int8 >& rId)
{
    return rId.getLength() == 16
        && memcmp( T::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) == 0;
}

/** the UNO3_GETIMPLEMENTATION_* macros  implement a static helper function
    that gives access to your implementation for a given interface reference,
    if possible.

    Example:
        MyClass* pClass = comphelper::getUnoTunnelImplementation<MyClass>( xRef );

    Usage:
        Put a UNO3_GETIMPLEMENTATION_DECL( classname ) inside your class
        definition and UNO3_GETIMPLEMENTATION_IMPL( classname ) inside
        your cxx file. Your class must inherit css::lang::XUnoTunnel
        and export it with queryInterface. Implementation of XUnoTunnel is
        done by this macro.
*/
#define UNO3_GETIMPLEMENTATION_DECL( classname ) \
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() noexcept; \
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

#define UNO3_GETIMPLEMENTATION_BASE_IMPL( classname ) \
const css::uno::Sequence< sal_Int8 > & classname::getUnoTunnelId() noexcept \
{ \
    static const UnoTunnelIdInit aId; \
    return aId.getSeq(); \
}

#define UNO3_GETIMPLEMENTATION_IMPL( classname )\
UNO3_GETIMPLEMENTATION_BASE_IMPL(classname)\
sal_Int64 SAL_CALL classname::getSomething( const css::uno::Sequence< sal_Int8 >& rId ) \
{ \
    if( isUnoTunnelId<classname>(rId) ) \
    { \
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this)); \
    } \
    return 0; \
}

#define UNO3_GETIMPLEMENTATION2_IMPL( classname, baseclass )\
UNO3_GETIMPLEMENTATION_BASE_IMPL(classname)\
sal_Int64 SAL_CALL classname::getSomething( const css::uno::Sequence< sal_Int8 >& rId ) \
{ \
    if( isUnoTunnelId<classname>(rId) ) \
    { \
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this)); \
    } \
    else \
    { \
        return baseclass::getSomething( rId ); \
    } \
}


#endif // INCLUDED_COMPHELPER_SERVICEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
