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

    prop.Name = rtl::OUString("Huhu");
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
