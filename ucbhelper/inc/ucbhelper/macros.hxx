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

#ifndef _UCBHELPER_MACROS_HXX
#define _UCBHELPER_MACROS_HXX

#include <sal/types.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/typeprovider.hxx>

#include "osl/mutex.hxx"

//=========================================================================

#define CPPU_TYPE( T )      getCppuType( static_cast< T * >( 0 ) )
#define CPPU_TYPE_REF( T )  CPPU_TYPE( com::sun::star::uno::Reference< T > )

//=========================================================================
//
// XInterface decl.
//
//=========================================================================

#define XINTERFACE_DECL()                                                   \
    virtual com::sun::star::uno::Any SAL_CALL                               \
    queryInterface( const com::sun::star::uno::Type & rType )               \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual void SAL_CALL                                                   \
    acquire()                                                               \
        throw();                                                            \
    virtual void SAL_CALL                                                   \
    release()                                                               \
        throw();

//=========================================================================
//
// XInterface impl. internals.
//
//=========================================================================

#define XINTERFACE_COMMON_IMPL( Class )                                     \
void SAL_CALL Class::acquire()                                              \
    throw()                                                                 \
{                                                                           \
    OWeakObject::acquire();                                                 \
}                                                                           \
                                                                            \
void SAL_CALL Class::release()                                              \
    throw()                                                                 \
{                                                                           \
    OWeakObject::release();                                                 \
}

#define QUERYINTERFACE_IMPL_START( Class )                                  \
com::sun::star::uno::Any SAL_CALL Class::queryInterface(                    \
                                const com::sun::star::uno::Type & rType )   \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    com::sun::star::uno::Any aRet = cppu::queryInterface( rType,

#define QUERYINTERFACE_IMPL_END                                             \
                    );                                                      \
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );   \
}

//=========================================================================
//
// XInterface impl.
//
//=========================================================================

// 1 interface implemented
#define XINTERFACE_IMPL_1( Class, Ifc1 )                                    \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< Ifc1* >(this))                                          \
QUERYINTERFACE_IMPL_END

// 2 interfaces implemented
#define XINTERFACE_IMPL_2( Class, Ifc1, Ifc2 )                              \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< Ifc1* >(this)),                                         \
    (static_cast< Ifc2* >(this))                                          \
QUERYINTERFACE_IMPL_END

// 3 interfaces implemented
#define XINTERFACE_IMPL_3( Class, Ifc1, Ifc2, Ifc3 )                        \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< Ifc1* >(this)),                                         \
    (static_cast< Ifc2* >(this)),                                         \
    (static_cast< Ifc3* >(this))                                          \
QUERYINTERFACE_IMPL_END

// 4 interfaces implemented
#define XINTERFACE_IMPL_4( Class, Ifc1, Ifc2, Ifc3, Ifc4 )                  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< Ifc1* >(this)),                                         \
    (static_cast< Ifc2* >(this)),                                         \
    (static_cast< Ifc3* >(this)),                                         \
    (static_cast< Ifc4* >(this))                                          \
QUERYINTERFACE_IMPL_END

// 5 interfaces implemented
#define XINTERFACE_IMPL_5( Class, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 )            \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< Ifc1* >(this)),                                         \
    (static_cast< Ifc2* >(this)),                                         \
    (static_cast< Ifc3* >(this)),                                         \
    (static_cast< Ifc4* >(this)),                                         \
    (static_cast< Ifc5* >(this))                                          \
QUERYINTERFACE_IMPL_END

// 6 interfaces implemented
#define XINTERFACE_IMPL_6( Class,I1,I2,I3,I4,I5,I6 )                        \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this))                                            \
QUERYINTERFACE_IMPL_END

// 7 interfaces implemented
#define XINTERFACE_IMPL_7( Class,I1,I2,I3,I4,I5,I6,I7 )                     \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this))                                            \
QUERYINTERFACE_IMPL_END

// 8 interfaces implemented
#define XINTERFACE_IMPL_8( Class,I1,I2,I3,I4,I5,I6,I7,I8 )                  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this)),                                           \
    (static_cast< I8* >(this))                                            \
QUERYINTERFACE_IMPL_END

// 9 interfaces implemented
#define XINTERFACE_IMPL_9( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9 )               \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this)),                                           \
    (static_cast< I8* >(this)),                                           \
    (static_cast< I9* >(this))                                            \
QUERYINTERFACE_IMPL_END

// 10 interfaces implemented
#define XINTERFACE_IMPL_10( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10 )          \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this)),                                           \
    (static_cast< I8* >(this)),                                           \
    (static_cast< I9* >(this)),                                           \
    (static_cast< I10* >(this))                                           \
QUERYINTERFACE_IMPL_END

// 11 interfaces implemented
#define XINTERFACE_IMPL_11( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11 )      \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this)),                                           \
    (static_cast< I8* >(this)),                                           \
    (static_cast< I9* >(this)),                                           \
    (static_cast< I10* >(this)),                                          \
    (static_cast< I11* >(this))                                           \
QUERYINTERFACE_IMPL_END

// 12 interfaces implemented
#define XINTERFACE_IMPL_12( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12 )  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this)),                                           \
    (static_cast< I8* >(this)),                                           \
    (static_cast< I9* >(this)),                                           \
    (static_cast< I10* >(this)),                                          \
    (static_cast< I11* >(this)),                                          \
    (static_cast< I12* >(this))                                           \
QUERYINTERFACE_IMPL_END

// 13 interfaces implemented
#define XINTERFACE_IMPL_13( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13 ) \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this)),                                           \
    (static_cast< I8* >(this)),                                           \
    (static_cast< I9* >(this)),                                           \
    (static_cast< I10* >(this)),                                          \
    (static_cast< I11* >(this)),                                          \
    (static_cast< I12* >(this)),                                          \
    (static_cast< I13* >(this))                                           \
QUERYINTERFACE_IMPL_END

// 14 interfaces implemented
#define XINTERFACE_IMPL_14( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14 )  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this)),                                           \
    (static_cast< I8* >(this)),                                           \
    (static_cast< I9* >(this)),                                           \
    (static_cast< I10* >(this)),                                          \
    (static_cast< I11* >(this)),                                          \
    (static_cast< I12* >(this)),                                          \
    (static_cast< I13* >(this)),                                          \
    (static_cast< I14* >(this))                                           \
QUERYINTERFACE_IMPL_END

// 15 interfaces implemented
#define XINTERFACE_IMPL_15( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15 )  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    (static_cast< I1* >(this)),                                           \
    (static_cast< I2* >(this)),                                           \
    (static_cast< I3* >(this)),                                           \
    (static_cast< I4* >(this)),                                           \
    (static_cast< I5* >(this)),                                           \
    (static_cast< I6* >(this)),                                           \
    (static_cast< I7* >(this)),                                           \
    (static_cast< I8* >(this)),                                           \
    (static_cast< I9* >(this)),                                           \
    (static_cast< I10* >(this)),                                          \
    (static_cast< I11* >(this)),                                          \
    (static_cast< I12* >(this)),                                          \
    (static_cast< I13* >(this)),                                          \
    (static_cast< I14* >(this)),                                          \
    (static_cast< I15* >(this))                                           \
QUERYINTERFACE_IMPL_END

//=========================================================================
//
// XTypeProvider decl.
//
//=========================================================================

#define XTYPEPROVIDER_DECL()                                                \
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL              \
    getImplementationId()                                                   \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL \
    getTypes()                                                              \
        throw( com::sun::star::uno::RuntimeException );

//=========================================================================
//
// XTypeProvider impl. internals
//
//=========================================================================

#define XTYPEPROVIDER_COMMON_IMPL( Class )                                  \
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL                          \
Class::getImplementationId()                                                \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    static cppu::OImplementationId* pId = NULL;                             \
      if ( !pId )                                                             \
      {                                                                       \
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );    \
          if ( !pId )                                                         \
          {                                                                   \
              static cppu::OImplementationId id( sal_False );                 \
              pId = &id;                                                      \
          }                                                                   \
      }                                                                       \
      return (*pId).getImplementationId();                                    \
}

#define GETTYPES_IMPL_START( Class )                                        \
com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL         \
Class::getTypes()                                                           \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    static cppu::OTypeCollection* pCollection = NULL;                       \
      if ( !pCollection )                                                     \
      {                                                                       \
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );    \
        if ( !pCollection )                                                 \
        {                                                                   \
            static cppu::OTypeCollection collection(

#define GETTYPES_IMPL_END                                                   \
                );                                                          \
            pCollection = &collection;                                      \
        }                                                                   \
    }                                                                       \
    return (*pCollection).getTypes();                                       \
}

//=========================================================================
//
// XTypeProvider impl.
//
//=========================================================================

// 1 interface supported
#define XTYPEPROVIDER_IMPL_1( Class, I1 )                                   \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 )                                                 \
GETTYPES_IMPL_END

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

// 6 interfaces supported
#define XTYPEPROVIDER_IMPL_6( Class, I1,I2,I3,I4,I5,I6 )                    \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 )                                                     \
GETTYPES_IMPL_END

// 7 interfaces supported
#define XTYPEPROVIDER_IMPL_7( Class, I1,I2,I3,I4,I5,I6,I7 )                 \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 )                                                     \
GETTYPES_IMPL_END

// 8 interfaces supported
#define XTYPEPROVIDER_IMPL_8( Class, I1,I2,I3,I4,I5,I6,I7,I8 )              \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 )                                                     \
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

// 12 interfaces supported
#define XTYPEPROVIDER_IMPL_12( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12 )  \
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
    CPPU_TYPE_REF( I11 ),                                                   \
    CPPU_TYPE_REF( I12 )                                                    \
GETTYPES_IMPL_END

// 13 interfaces supported
#define XTYPEPROVIDER_IMPL_13( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13 )  \
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
    CPPU_TYPE_REF( I11 ),                                                   \
    CPPU_TYPE_REF( I12 ),                                                   \
    CPPU_TYPE_REF( I13 )                                                    \
GETTYPES_IMPL_END

// 14 interfaces supported
#define XTYPEPROVIDER_IMPL_14( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14 )  \
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
    CPPU_TYPE_REF( I11 ),                                                   \
    CPPU_TYPE_REF( I12 ),                                                   \
    CPPU_TYPE_REF( I13 ),                                                   \
    CPPU_TYPE_REF( I14 )                                                    \
GETTYPES_IMPL_END

// 15 interfaces supported
#define XTYPEPROVIDER_IMPL_15( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15 )  \
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
    CPPU_TYPE_REF( I11 ),                                                   \
    CPPU_TYPE_REF( I12 ),                                                   \
    CPPU_TYPE_REF( I13 ),                                                   \
    CPPU_TYPE_REF( I14 ),                                                   \
    CPPU_TYPE_REF( I15 )                                                    \
GETTYPES_IMPL_END

//=========================================================================
//
// XServiceInfo decl.
//
//=========================================================================

#define XSERVICEINFO_NOFACTORY_DECL()                                       \
    virtual rtl::OUString SAL_CALL                                          \
    getImplementationName()                                                 \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual sal_Bool SAL_CALL                                               \
    supportsService( const rtl::OUString& ServiceName )                     \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL         \
    getSupportedServiceNames()                                              \
        throw( com::sun::star::uno::RuntimeException );                     \
                                                                            \
    static rtl::OUString                                                    \
    getImplementationName_Static();                                         \
    static com::sun::star::uno::Sequence< rtl::OUString >                   \
    getSupportedServiceNames_Static();

#define XSERVICEINFO_DECL()                                                 \
    XSERVICEINFO_NOFACTORY_DECL()                                           \
                                                                            \
    static com::sun::star::uno::Reference<                                  \
            com::sun::star::lang::XSingleServiceFactory >                   \
    createServiceFactory( const com::sun::star::uno::Reference<             \
            com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr );

//=========================================================================
//
// XServiceInfo impl. internals
//
//=========================================================================

#define XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                         \
rtl::OUString SAL_CALL Class::getImplementationName()                       \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    return getImplementationName_Static();                                  \
}                                                                           \
                                                                            \
rtl::OUString Class::getImplementationName_Static()                         \
{                                                                           \
    return ImplName;                                                        \
}                                                                           \
                                                                            \
sal_Bool SAL_CALL                                                           \
Class::supportsService( const rtl::OUString& ServiceName )                  \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    com::sun::star::uno::Sequence< rtl::OUString > aSNL =                   \
                                        getSupportedServiceNames();         \
    const rtl::OUString* pArray = aSNL.getArray();                          \
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )                       \
    {                                                                       \
        if( pArray[ i ] == ServiceName )                                    \
            return sal_True;                                                \
    }                                                                       \
                                                                            \
    return sal_False;                                                       \
}                                                                           \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL                     \
Class::getSupportedServiceNames()                                           \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    return getSupportedServiceNames_Static();                               \
}

#define XSERVICEINFO_CREATE_INSTANCE_IMPL( Class )                          \
static com::sun::star::uno::Reference<                                      \
                                com::sun::star::uno::XInterface > SAL_CALL  \
Class##_CreateInstance( const com::sun::star::uno::Reference<               \
                com::sun::star::lang::XMultiServiceFactory> & rSMgr )       \
    throw( com::sun::star::uno::Exception )                                 \
{                                                                           \
    com::sun::star::lang::XServiceInfo* pX =                                \
                (com::sun::star::lang::XServiceInfo*)new Class( rSMgr );    \
    return com::sun::star::uno::Reference<                                  \
                            com::sun::star::uno::XInterface >::query( pX ); \
}

#define XSERVICEINFO_CREATE_INSTANCE_IMPL_CTX( Class )                          \
static com::sun::star::uno::Reference<                                      \
                                com::sun::star::uno::XInterface > SAL_CALL  \
Class##_CreateInstance( const com::sun::star::uno::Reference<               \
                com::sun::star::lang::XMultiServiceFactory> & rSMgr )       \
    throw( com::sun::star::uno::Exception )                                 \
{                                                                           \
    com::sun::star::lang::XServiceInfo* pX =                                \
                (com::sun::star::lang::XServiceInfo*)new Class( comphelper::getComponentContext(rSMgr) );    \
    return com::sun::star::uno::Reference<                                  \
                            com::sun::star::uno::XInterface >::query( pX ); \
}

//=========================================================================
//
// XServiceInfo impl.
//
//=========================================================================

#define ONE_INSTANCE_SERVICE_FACTORY_IMPL( Class )                          \
com::sun::star::uno::Reference<                                             \
                        com::sun::star::lang::XSingleServiceFactory >       \
Class::createServiceFactory( const com::sun::star::uno::Reference<          \
            com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr )    \
{                                                                           \
    return com::sun::star::uno::Reference<                                  \
        com::sun::star::lang::XSingleServiceFactory >(                      \
            cppu::createOneInstanceFactory(                                 \
                rxServiceMgr,                                               \
                Class::getImplementationName_Static(),                      \
                Class##_CreateInstance,                                     \
                Class::getSupportedServiceNames_Static() ) );               \
}

// Service without service factory.

// Own implementation of getSupportedServiceNames_Static().
#define XSERVICEINFO_NOFACTORY_IMPL_0( Class, ImplName )                    \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()

// 1 service name
#define XSERVICEINFO_NOFACTORY_IMPL_1( Class, ImplName, Service1 )          \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()                                    \
{                                                                           \
    com::sun::star::uno::Sequence< rtl::OUString > aSNS( 1 );               \
    aSNS.getArray()[ 0 ] = Service1;                                        \
    return aSNS;                                                            \
}

// Service with service factory.

// Own implementation of getSupportedServiceNames_Static().
#define XSERVICEINFO_IMPL_0( Class, ImplName )                              \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
XSERVICEINFO_CREATE_INSTANCE_IMPL( Class )                                  \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()

// Own implementation of getSupportedServiceNames_Static().
#define XSERVICEINFO_IMPL_0_CTX( Class, ImplName )                              \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
XSERVICEINFO_CREATE_INSTANCE_IMPL_CTX( Class )                                  \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()

// 1 service name
#define XSERVICEINFO_IMPL_1( Class, ImplName, Service1 )                    \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
XSERVICEINFO_CREATE_INSTANCE_IMPL( Class )                                  \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()                                    \
{                                                                           \
    com::sun::star::uno::Sequence< rtl::OUString > aSNS( 1 );               \
    aSNS.getArray()[ 0 ] = Service1;                                        \
    return aSNS;                                                            \
}

#endif /* !_UCBHELPER_MACROS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
