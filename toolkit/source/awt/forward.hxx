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

#ifndef AWT_FORWARD_HXX
#define AWT_FORWARD_HXX

#include <comphelper/uno3.hxx>

#define IMPLEMENT_FORWARD_XTYPEPROVIDER1( classname, baseclass ) \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL classname::getTypes(  ) throw (::com::sun::star::uno::RuntimeException) \
    { return baseclass::getTypes(); } \
    IMPLEMENT_GET_IMPLEMENTATION_ID( classname )

#define IMPLEMENT_2_FORWARD_XINTERFACE1( classname, refcountbase1, refcountbase2 ) \
    void SAL_CALL classname::acquire() throw() { refcountbase1::acquire(); refcountbase2::acquire(); } \
    void SAL_CALL classname::release() throw() { refcountbase1::release(); refcountbase2::release(); } \
    ::com::sun::star::uno::Any SAL_CALL classname::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aReturn = refcountbase1::queryInterface( _rType ); \
        if ( !aReturn.hasValue() ) \
        { \
            aReturn = refcountbase2::queryInterface( _rType ); \
        } \
        return aReturn; \
    }

#define IMPLEMENT_2_FORWARD_XINTERFACE2( classname, refcountbase1, refcountbase2, baseclass3 ) \
    void SAL_CALL classname::acquire() throw() { refcountbase1::acquire(); refcountbase2::acquire(); } \
    void SAL_CALL classname::release() throw() { refcountbase1::release(); refcountbase2::release(); } \
    ::com::sun::star::uno::Any SAL_CALL classname::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aReturn = refcountbase1::queryInterface( _rType ); \
        if ( !aReturn.hasValue() ) \
        { \
            aReturn = refcountbase2::queryInterface( _rType ); \
            if ( !aReturn.hasValue() ) \
                aReturn = baseclass3::queryInterface( _rType ); \
        } \
        return aReturn; \
    }

#if (defined (_MSC_VER) && (_MSC_VER <= 1310)) || defined (__clang__)
// Windows .Net2003 build fix
#define W3K_EXPLICIT_CAST(x) static_cast <XWindow2*> (&x)
#else // !((defined (_MSC_VER) && (_MSC_VER <= 1310)) || defined (__clang__))
#define W3K_EXPLICIT_CAST(x) x
#endif // !((defined (_MSC_VER) && (_MSC_VER <= 1310)) || defined (__clang__))

#endif /* AWT_FORWARD_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
