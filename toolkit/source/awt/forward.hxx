/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#if defined (_MSC_VER) && (_MSC_VER <= 1310)
// Windows .Net2003 build fix
#define W3K_EXPLICIT_CAST(x) static_cast <XWindow2*> (&x)
#else // !(defined (_MSC_VER) && (_MSC_VER <= 1310))
#define W3K_EXPLICIT_CAST(x) x
#endif // !(defined (_MSC_VER) && (_MSC_VER <= 1310))

#endif /* AWT_FORWARD_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
