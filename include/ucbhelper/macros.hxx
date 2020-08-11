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

#ifndef INCLUDED_UCBHELPER_MACROS_HXX
#define INCLUDED_UCBHELPER_MACROS_HXX

#include <sal/types.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

#define CPPU_TYPE( T )      cppu::UnoType<T>::get()
#define CPPU_TYPE_REF( T )  CPPU_TYPE( T )

// XTypeProvider impl. internals


#define XTYPEPROVIDER_COMMON_IMPL( Class )                                  \
css::uno::Sequence< sal_Int8 > SAL_CALL                          \
Class::getImplementationId()          \
{                                                                           \
      return css::uno::Sequence<sal_Int8>();                                \
}

#define GETTYPES_IMPL_START( Class )                                        \
css::uno::Sequence< css::uno::Type > SAL_CALL         \
Class::getTypes()          \
{                                                                           \
    static cppu::OTypeCollection collection(


#define GETTYPES_IMPL_END                                                   \
                );                                                          \
                                                                            \
    return collection.getTypes();                                           \
}


// XTypeProvider impl.


// 2 interfaces supported
#define XTYPEPROVIDER_IMPL_2( Class, I1,I2 )                                \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 )                                                     \
GETTYPES_IMPL_END

// 3 interfaces supported
#define XTYPEPROVIDER_IMPL_3( Class, I1,I2,I3 )                             \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 )                                                     \
GETTYPES_IMPL_END

// 4 interfaces supported
#define XTYPEPROVIDER_IMPL_4( Class, I1,I2,I3,I4 )                          \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 )                                                     \
GETTYPES_IMPL_END

// 5 interfaces supported
#define XTYPEPROVIDER_IMPL_5( Class, I1,I2,I3,I4,I5 )                       \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 )                                                     \
GETTYPES_IMPL_END

// 9 interfaces supported
#define XTYPEPROVIDER_IMPL_9( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9 )           \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 )                                                     \
GETTYPES_IMPL_END

// 10 interfaces supported
#define XTYPEPROVIDER_IMPL_10( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10 )      \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 ),                                                    \
    CPPU_TYPE_REF( I10 )                                                    \
GETTYPES_IMPL_END

// 11 interfaces supported
#define XTYPEPROVIDER_IMPL_11( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11 )  \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 ),                                                    \
    CPPU_TYPE_REF( I10 ),                                                   \
    CPPU_TYPE_REF( I11 )                                                    \
GETTYPES_IMPL_END

#endif /* ! INCLUDED_UCBHELPER_MACROS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
