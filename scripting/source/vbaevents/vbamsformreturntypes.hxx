/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009, 2010.
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

#ifndef VBA_MSFORM_RETURNTYPES_HXX
#define VBA_MSFORM_RETURNTYPES_HXX

#include <cppuhelper/implbase1.hxx>
#include <msforms/XReturnBoolean.hpp>
#include <msforms/XReturnInteger.hpp>


typedef ::cppu::WeakImplHelper1<msforms::XReturnBoolean> ReturnBoolean_BASE;

class SAL_DLLPUBLIC_EXPORT VbaReturnBoolean : public ReturnBoolean_BASE
{
public:
    sal_Bool Value;

public:
    VbaReturnBoolean() : Value(false) {} ;

    // XReturnBoolean
    virtual ::sal_Bool SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException) { return Value; }
    virtual void SAL_CALL setValue( ::sal_Bool _value ) throw (::com::sun::star::uno::RuntimeException) { Value = _value; }

    // XDefaultProperty
    ::rtl::OUString SAL_CALL getDefaultPropertyName() throw (com::sun::star::uno::RuntimeException) { return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Value")); }
};


typedef ::cppu::WeakImplHelper1<msforms::XReturnInteger> ReturnInteger_BASE;

class SAL_DLLPUBLIC_EXPORT VbaReturnInteger : public ReturnInteger_BASE
{
public:
    sal_Int32 Value;

public:
    VbaReturnInteger() : Value(0) {} ;

    // XReturnInteger
    virtual ::sal_Int32 SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException) { return Value; }
    virtual void SAL_CALL setValue( ::sal_Int32 _value ) throw (::com::sun::star::uno::RuntimeException) { Value = _value; }

    // XDefaultProperty
    ::rtl::OUString SAL_CALL getDefaultPropertyName() throw (com::sun::star::uno::RuntimeException) { return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Value")); }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
