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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"
#include <cppuhelper/proptypehlp.hxx>

#include <com/sun/star/beans/Property.hpp>

using namespace ::com::sun::star::uno;
using namespace ::cppu;

void testPropertyTypeHelper()
{
    Any a;

    a <<= ( sal_Int8 ) 25;

    sal_Int32 i;
    convertPropertyValue( i ,  a );
    OSL_ASSERT( 25 == i );

    sal_Int16 i16;
    convertPropertyValue( i16 ,  a );
    OSL_ASSERT( 25 == i16 );

    sal_Int8 i8;
    convertPropertyValue( i8 ,  a );
    OSL_ASSERT( 25 == i8 );

    sal_uInt32 i32;
    convertPropertyValue( i32 ,  a );
    OSL_ASSERT( 25 == i32 );

    double d;
    convertPropertyValue( d , a );
    OSL_ASSERT( 25. == d );

    float f;
    convertPropertyValue( f , a );
    OSL_ASSERT( 25. == f );

    ::com::sun::star::beans::Property prop;

    prop.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Huhu") );
    prop.Handle = 5;
    prop.Attributes = 3;

    a <<= prop;

    ::com::sun::star::beans::Property prop2;
    convertPropertyValue( prop2 , a );
     OSL_ASSERT( prop.Handle == prop2.Handle && prop.Name == prop2.Name && prop.Attributes == prop2.Attributes );


    ::rtl::OUString ow;
    a <<= prop.Name;
    convertPropertyValue( ow , a );
    OSL_ASSERT( ow == prop.Name );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
