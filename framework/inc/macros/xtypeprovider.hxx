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

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#define __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

namespace framework{

/*_________________________________________________________________________________________________________________

    macros for declaration and definition of XTypeProvider
    Please use follow public macros only!

    1)  DEFINE_XTYPEPROVIDER                                                            => use it in header to declare XTypeProvider and his methods
    2)  DECLARE_TYPEPROVIDER_0( CLASS )                                                 => use it to define implementation of XTypeProvider for 0 supported type
        DECLARE_TYPEPROVIDER_1( CLASS, TYPE1 )                                          => use it to define implementation of XTypeProvider for 1 supported type
        ...
        DECLARE_TYPEPROVIDER_16( CLASS, TYPE1, ... , TYPE16 )
    3)  DEFINE_XTYPEPROVIDER_1_WITH_BASECLASS( CLASS, BASECLASS, TYPE1 )                => use it to define implementation of XTypeProvider for 1 additional supported type to baseclass
        ...
        DEFINE_XTYPEPROVIDER_5_WITH_BASECLASS( CLASS, BASECLASS, TYPE1, ..., TYPE5 )

_________________________________________________________________________________________________________________*/

//*****************************************************************************************************************
//  private
//  implementation of XTypeProvider::getImplementationId()
//*****************************************************************************************************************
#define PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                               \
    ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL CLASS::getImplementationId() throw( ::com::sun::star::uno::RuntimeException )          \
    {                                                                                                                                           \
        /* Create one Id for all instances of this class.                                               */                                      \
        /* Use ethernet address to do this! (sal_True)                                                  */                                      \
        /* Optimize this method                                                                         */                                      \
        /* We initialize a static variable only one time. And we don't must use a mutex at every call!  */                                      \
        /* For the first call; pID is NULL - for the second call pID is different from NULL!            */                                      \
        static ::cppu::OImplementationId* pID = NULL ;                                                                                          \
        if ( pID == NULL )                                                                                                                      \
        {                                                                                                                                       \
            /* Ready for multithreading; get global mutex for first call of this method only! see before   */                                   \
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );                                                                         \
            /* Control these pointer again ... it can be, that another instance will be faster then these! */                                   \
            if ( pID == NULL )                                                                                                                  \
            {                                                                                                                                   \
                /* Create a new static ID ... */                                                                                                \
                static ::cppu::OImplementationId aID( sal_False );                                                                              \
                /* ... and set his address to static pointer! */                                                                                \
                pID = &aID ;                                                                                                                    \
            }                                                                                                                                   \
        }                                                                                                                                       \
        return pID->getImplementationId();                                                                                                      \
    }

//*****************************************************************************************************************
//  private
//  implementation of XTypeProvider::getTypes() with max. 12 interfaces!
//*****************************************************************************************************************
#define PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES( CLASS, TYPES )                                                                                   \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL CLASS::getTypes() throw( ::com::sun::star::uno::RuntimeException )  \
    {                                                                                                                                           \
        /* Optimize this method !                                       */                                                                      \
        /* We initialize a static variable only one time.               */                                                                      \
        /* And we don't must use a mutex at every call!                 */                                                                      \
        /* For the first call; pTypeCollection is NULL -                */                                                                      \
        /* for the second call pTypeCollection is different from NULL!  */                                                                      \
        static ::cppu::OTypeCollection* pTypeCollection = NULL ;                                                                                \
        if ( pTypeCollection == NULL )                                                                                                          \
        {                                                                                                                                       \
            /* Ready for multithreading; get global mutex for first call of this method only! see before   */                                   \
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );                                                                         \
            /* Control these pointer again ... it can be, that another instance will be faster then these! */                                   \
            if ( pTypeCollection == NULL )                                                                                                      \
            {                                                                                                                                   \
                /* Create a static typecollection ...           */                                                                              \
                /* Attention: "TYPES" will expand to "(...)"!   */                                                                              \
                static ::cppu::OTypeCollection aTypeCollection TYPES ;                                                                          \
                /* ... and set his address to static pointer! */                                                                                \
                pTypeCollection = &aTypeCollection ;                                                                                            \
            }                                                                                                                                   \
        }                                                                                                                                       \
        return pTypeCollection->getTypes();                                                                                                     \
    }

//*****************************************************************************************************************
//  private
//  implementation of XTypeProvider::getTypes() with more then 12 interfaces!
//*****************************************************************************************************************
#define PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES_LARGE( CLASS, TYPES_FIRST, TYPES_SECOND )                                                         \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL CLASS::getTypes() throw( ::com::sun::star::uno::RuntimeException )  \
    {                                                                                                                                           \
        /* Optimize this method !                                       */                                                                      \
        /* We initialize a static variable only one time.               */                                                                      \
        /* And we don't must use a mutex at every call!                 */                                                                      \
        /* For the first call; pTypeCollection is NULL -                */                                                                      \
        /* for the second call pTypeCollection is different from NULL!  */                                                                      \
        static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >* pTypeCollection = NULL ;                                         \
        if ( pTypeCollection == NULL )                                                                                                          \
        {                                                                                                                                       \
            /* Ready for multithreading; get global mutex for first call of this method only! see before   */                                   \
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );                                                                         \
            /* Control these pointer again ... it can be, that another instance will be faster then these! */                                   \
            if ( pTypeCollection == NULL )                                                                                                      \
            {                                                                                                                                   \
                /* Create two typecollections                           */                                                                      \
                /* (cppuhelper support 12 items per collection only!)   */                                                                      \
                ::cppu::OTypeCollection aTypeCollection1 TYPES_FIRST    ;                                                                       \
                ::cppu::OTypeCollection aTypeCollection2 TYPES_SECOND   ;                                                                       \
                /* Copy all items from both sequences to one result list! */                                                                    \
                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >          seqTypes1   = aTypeCollection1.getTypes();              \
                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >          seqTypes2   = aTypeCollection2.getTypes();              \
                sal_Int32                                                               nCount1     = seqTypes1.getLength();                    \
                sal_Int32                                                               nCount2     = seqTypes2.getLength();                    \
                static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >   seqResult   ( nCount1+nCount2 );                        \
                sal_Int32                                                               nSource     = 0;                                        \
                sal_Int32                                                               nDestination= 0;                                        \
                while( nSource<nCount1 )                                                                                                        \
                {                                                                                                                               \
                    seqResult[nDestination] = seqTypes1[nSource];                                                                               \
                    ++nSource;                                                                                                                  \
                    ++nDestination;                                                                                                             \
                }                                                                                                                               \
                nSource = 0;                                                                                                                    \
                while( nSource<nCount2 )                                                                                                        \
                {                                                                                                                               \
                    seqResult[nDestination] = seqTypes2[nSource];                                                                               \
                    ++nSource;                                                                                                                  \
                    ++nDestination;                                                                                                             \
                }                                                                                                                               \
                /* ... and set his address to static pointer! */                                                                                \
                pTypeCollection = &seqResult;                                                                                                   \
            }                                                                                                                                   \
        }                                                                                                                                       \
        return *pTypeCollection;                                                                                                                \
    }

//*****************************************************************************************************************
//  private
//  implementation of XTypeProvider::getTypes() with using max. 12 interfaces + baseclass!
//*****************************************************************************************************************
#define PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES_BASECLASS( CLASS, BASECLASS, TYPES )                                                              \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL CLASS::getTypes() throw( ::com::sun::star::uno::RuntimeException )  \
    {                                                                                                                                           \
        /* Optimize this method !                                       */                                                                      \
        /* We initialize a static variable only one time.               */                                                                      \
        /* And we don't must use a mutex at every call!                 */                                                                      \
        /* For the first call; pTypeCollection is NULL -                */                                                                      \
        /* for the second call pTypeCollection is different from NULL!  */                                                                      \
        static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >* pTypeCollection = NULL ;                                         \
        if ( pTypeCollection == NULL )                                                                                                          \
        {                                                                                                                                       \
            /* Ready for multithreading; get global mutex for first call of this method only! see before   */                                   \
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );                                                                         \
            /* Control these pointer again ... it can be, that another instance will be faster then these! */                                   \
            if ( pTypeCollection == NULL )                                                                                                      \
            {                                                                                                                                   \
                /* Create static typecollection for my own interfaces!  */                                                                      \
                static ::cppu::OTypeCollection aTypeCollection TYPES ;                                                                          \
                /* Copy all items from my list sequences and from my baseclass  */                                                              \
                /* to one result list!                                          */                                                              \
                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >          seqTypes1   = aTypeCollection.getTypes();               \
                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >          seqTypes2   = BASECLASS::getTypes();                    \
                sal_Int32                                                               nCount1     = seqTypes1.getLength();                    \
                sal_Int32                                                               nCount2     = seqTypes2.getLength();                    \
                static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >   seqResult   ( nCount1+nCount2 );                        \
                sal_Int32                                                               nSource     = 0;                                        \
                sal_Int32                                                               nDestination= 0;                                        \
                while( nSource<nCount1 )                                                                                                        \
                {                                                                                                                               \
                    seqResult[nDestination] = seqTypes1[nSource];                                                                               \
                    ++nSource;                                                                                                                  \
                    ++nDestination;                                                                                                             \
                }                                                                                                                               \
                nSource = 0;                                                                                                                    \
                while( nSource<nCount2 )                                                                                                        \
                {                                                                                                                               \
                    seqResult[nDestination] = seqTypes2[nSource];                                                                               \
                    ++nSource;                                                                                                                  \
                    ++nDestination;                                                                                                             \
                }                                                                                                                               \
                /* ... and set his address to static pointer! */                                                                                \
                pTypeCollection = &seqResult;                                                                                                   \
            }                                                                                                                                   \
        }                                                                                                                                       \
        return *pTypeCollection;                                                                                                                \
    }

//*****************************************************************************************************************
//  private
//  help macros to replace TYPES in getTypes() [see before]
//*****************************************************************************************************************
#define PRIVATE_DEFINE_TYPE_1( TYPE1 )                                                                                                          \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE1 >*)NULL )

#define PRIVATE_DEFINE_TYPE_2( TYPE1, TYPE2 )                                                                                                   \
    PRIVATE_DEFINE_TYPE_1( TYPE1 ),                                                                                                             \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE2 >*)NULL )

#define PRIVATE_DEFINE_TYPE_3( TYPE1, TYPE2, TYPE3 )                                                                                            \
    PRIVATE_DEFINE_TYPE_2( TYPE1, TYPE2 ),                                                                                                      \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE3 >*)NULL )

#define PRIVATE_DEFINE_TYPE_4( TYPE1, TYPE2, TYPE3, TYPE4 )                                                                                     \
    PRIVATE_DEFINE_TYPE_3( TYPE1, TYPE2, TYPE3 ),                                                                                               \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE4 >*)NULL )

#define PRIVATE_DEFINE_TYPE_5( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 )                                                                              \
    PRIVATE_DEFINE_TYPE_4( TYPE1, TYPE2, TYPE3, TYPE4 ),                                                                                        \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE5 >*)NULL )

#define PRIVATE_DEFINE_TYPE_6( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 )                                                                       \
    PRIVATE_DEFINE_TYPE_5( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 ),                                                                                 \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE6 >*)NULL )

#define PRIVATE_DEFINE_TYPE_7( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 )                                                                \
    PRIVATE_DEFINE_TYPE_6( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 ),                                                                          \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE7 >*)NULL )

#define PRIVATE_DEFINE_TYPE_8( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 )                                                         \
    PRIVATE_DEFINE_TYPE_7( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 ),                                                                   \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE8 >*)NULL )

#define PRIVATE_DEFINE_TYPE_9( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 )                                                  \
    PRIVATE_DEFINE_TYPE_8( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 ),                                                            \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE9 >*)NULL )

#define PRIVATE_DEFINE_TYPE_10( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 )                                         \
    PRIVATE_DEFINE_TYPE_9( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 ),                                                     \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE10 >*)NULL )

#define PRIVATE_DEFINE_TYPE_11( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 )                                 \
    PRIVATE_DEFINE_TYPE_10( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 ),                                            \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE11 >*)NULL )

#define PRIVATE_DEFINE_TYPE_12( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12 )                         \
    PRIVATE_DEFINE_TYPE_11( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 ),                                    \
    ::getCppuType(( const ::com::sun::star::uno::Reference< TYPE12 >*)NULL )

//*****************************************************************************************************************
//  private
//  complete implementation of XTypeProvider
//*****************************************************************************************************************
#define PRIVATE_DEFINE_XTYPEPROVIDER_PURE( CLASS )                                                                                                          \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                                               \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES( CLASS, ::getCppuType(( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >*)NULL ) )

#define PRIVATE_DEFINE_XTYPEPROVIDER( CLASS, TYPES )                                                                                                        \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                                               \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES( CLASS, TYPES )

#define PRIVATE_DEFINE_XTYPEPROVIDER_LARGE( CLASS, TYPES_FIRST, TYPES_SECOND )                                                                              \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                                               \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES_LARGE( CLASS, TYPES_FIRST, TYPES_SECOND )

#define PRIVATE_DEFINE_XTYPEPROVIDER_BASECLASS( CLASS, BASECLASS, TYPES )                                                                                   \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                                               \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES_BASECLASS( CLASS, BASECLASS, TYPES )

//*****************************************************************************************************************
//  public
//  declaration of XTypeProvider
//*****************************************************************************************************************
#define FWK_DECLARE_XTYPEPROVIDER                                                                                                                               \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes           () throw( ::com::sun::star::uno::RuntimeException );\
    virtual ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw( ::com::sun::star::uno::RuntimeException );

//*****************************************************************************************************************
//  public
//  implementation of XTypeProvider
//*****************************************************************************************************************
//  implementation of XTypeProvider without additional interface for getTypes()
//  XTypeProvider is used as the only one interface automaticly.
//  Following defines don't use XTypeProvider automaticly!!!!
#define DEFINE_XTYPEPROVIDER_0( CLASS )                                                                             \
    PRIVATE_DEFINE_XTYPEPROVIDER_PURE(  CLASS )

//  implementation of XTypeProvider with 1 additional interface for getTypes()
#define DEFINE_XTYPEPROVIDER_1( CLASS, TYPE1 )                                                                      \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_1  (   TYPE1                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 2 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_2( CLASS, TYPE1, TYPE2 )                                                               \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_2  (   TYPE1   ,                                       \
                                                                    TYPE2                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 3 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_3( CLASS, TYPE1, TYPE2, TYPE3 )                                                        \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_3  (   TYPE1   ,                                       \
                                                                    TYPE2   ,                                       \
                                                                    TYPE3                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 4 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_4( CLASS, TYPE1, TYPE2, TYPE3, TYPE4 )                                                 \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_4  (   TYPE1   ,                                       \
                                                                    TYPE2   ,                                       \
                                                                    TYPE3   ,                                       \
                                                                    TYPE4                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 5 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_5( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 )                                          \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_5  (   TYPE1   ,                                       \
                                                                    TYPE2   ,                                       \
                                                                    TYPE3   ,                                       \
                                                                    TYPE4   ,                                       \
                                                                    TYPE5                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 6 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_6( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 )                                   \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_6  (   TYPE1   ,                                       \
                                                                    TYPE2   ,                                       \
                                                                    TYPE3   ,                                       \
                                                                    TYPE4   ,                                       \
                                                                    TYPE5   ,                                       \
                                                                    TYPE6                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 7 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_7( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 )                            \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_7  (   TYPE1   ,                                       \
                                                                    TYPE2   ,                                       \
                                                                    TYPE3   ,                                       \
                                                                    TYPE4   ,                                       \
                                                                    TYPE5   ,                                       \
                                                                    TYPE6   ,                                       \
                                                                    TYPE7                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 8 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_8( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 )                     \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_8  (   TYPE1   ,                                       \
                                                                    TYPE2   ,                                       \
                                                                    TYPE3   ,                                       \
                                                                    TYPE4   ,                                       \
                                                                    TYPE5   ,                                       \
                                                                    TYPE6   ,                                       \
                                                                    TYPE7   ,                                       \
                                                                    TYPE8                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 9 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_9( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 )              \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_9  (   TYPE1   ,                                       \
                                                                    TYPE2   ,                                       \
                                                                    TYPE3   ,                                       \
                                                                    TYPE4   ,                                       \
                                                                    TYPE5   ,                                       \
                                                                    TYPE6   ,                                       \
                                                                    TYPE7   ,                                       \
                                                                    TYPE8   ,                                       \
                                                                    TYPE9                                           \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 10 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_10( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 )     \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                      \
                                        (PRIVATE_DEFINE_TYPE_10 (   TYPE1   ,                                       \
                                                                    TYPE2   ,                                       \
                                                                    TYPE3   ,                                       \
                                                                    TYPE4   ,                                       \
                                                                    TYPE5   ,                                       \
                                                                    TYPE6   ,                                       \
                                                                    TYPE7   ,                                       \
                                                                    TYPE8   ,                                       \
                                                                    TYPE9   ,                                       \
                                                                    TYPE10                                          \
                                                                ))                                                  \
                                    )

//  implementation of XTypeProvider with 11 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_11( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                          \
                                        (PRIVATE_DEFINE_TYPE_11 (   TYPE1   ,                                           \
                                                                    TYPE2   ,                                           \
                                                                    TYPE3   ,                                           \
                                                                    TYPE4   ,                                           \
                                                                    TYPE5   ,                                           \
                                                                    TYPE6   ,                                           \
                                                                    TYPE7   ,                                           \
                                                                    TYPE8   ,                                           \
                                                                    TYPE9   ,                                           \
                                                                    TYPE10  ,                                           \
                                                                    TYPE11                                              \
                                                                ))                                                      \
                                    )

//  implementation of XTypeProvider with 12 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_12( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER    (   CLASS,                                                                                  \
                                        (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                   \
                                                                    TYPE2   ,                                                   \
                                                                    TYPE3   ,                                                   \
                                                                    TYPE4   ,                                                   \
                                                                    TYPE5   ,                                                   \
                                                                    TYPE6   ,                                                   \
                                                                    TYPE7   ,                                                   \
                                                                    TYPE8   ,                                                   \
                                                                    TYPE9   ,                                                   \
                                                                    TYPE10  ,                                                   \
                                                                    TYPE11  ,                                                   \
                                                                    TYPE12                                                      \
                                                                ))                                                              \
                                    )

//  implementation of XTypeProvider with 13 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_13( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER_LARGE  (   CLASS,                                                                                      \
                                            (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                       \
                                                                        TYPE2   ,                                                       \
                                                                        TYPE3   ,                                                       \
                                                                        TYPE4   ,                                                       \
                                                                        TYPE5   ,                                                       \
                                                                        TYPE6   ,                                                       \
                                                                        TYPE7   ,                                                       \
                                                                        TYPE8   ,                                                       \
                                                                        TYPE9   ,                                                       \
                                                                        TYPE10  ,                                                       \
                                                                        TYPE11  ,                                                       \
                                                                        TYPE12                                                          \
                                                                    )),                                                                 \
                                            (PRIVATE_DEFINE_TYPE_1  (   TYPE13                                                          \
                                                                    ))                                                                  \
                                        )

//  implementation of XTypeProvider with 14 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_14( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13, TYPE14 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER_LARGE  (   CLASS,                                                                                              \
                                            (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                               \
                                                                        TYPE2   ,                                                               \
                                                                        TYPE3   ,                                                               \
                                                                        TYPE4   ,                                                               \
                                                                        TYPE5   ,                                                               \
                                                                        TYPE6   ,                                                               \
                                                                        TYPE7   ,                                                               \
                                                                        TYPE8   ,                                                               \
                                                                        TYPE9   ,                                                               \
                                                                        TYPE10  ,                                                               \
                                                                        TYPE11  ,                                                               \
                                                                        TYPE12                                                                  \
                                                                    )),                                                                         \
                                            (PRIVATE_DEFINE_TYPE_2  (   TYPE13  ,                                                               \
                                                                        TYPE14                                                                  \
                                                                    ))                                                                          \
                                        )

//  implementation of XTypeProvider with 15 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_15( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13, TYPE14, TYPE15 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER_LARGE  (   CLASS,                                                                                                      \
                                            (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                                       \
                                                                        TYPE2   ,                                                                       \
                                                                        TYPE3   ,                                                                       \
                                                                        TYPE4   ,                                                                       \
                                                                        TYPE5   ,                                                                       \
                                                                        TYPE6   ,                                                                       \
                                                                        TYPE7   ,                                                                       \
                                                                        TYPE8   ,                                                                       \
                                                                        TYPE9   ,                                                                       \
                                                                        TYPE10  ,                                                                       \
                                                                        TYPE11  ,                                                                       \
                                                                        TYPE12                                                                          \
                                                                    )),                                                                                 \
                                            (PRIVATE_DEFINE_TYPE_3  (   TYPE13  ,                                                                       \
                                                                        TYPE14  ,                                                                       \
                                                                        TYPE15                                                                          \
                                                                    ))                                                                                  \
                                        )

//  implementation of XTypeProvider with 16 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_16( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13, TYPE14, TYPE15, TYPE16 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER_LARGE  (   CLASS,                                                                                                              \
                                            (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                                               \
                                                                        TYPE2   ,                                                                               \
                                                                        TYPE3   ,                                                                               \
                                                                        TYPE4   ,                                                                               \
                                                                        TYPE5   ,                                                                               \
                                                                        TYPE6   ,                                                                               \
                                                                        TYPE7   ,                                                                               \
                                                                        TYPE8   ,                                                                               \
                                                                        TYPE9   ,                                                                               \
                                                                        TYPE10  ,                                                                               \
                                                                        TYPE11  ,                                                                               \
                                                                        TYPE12                                                                                  \
                                                                    )),                                                                                         \
                                            (PRIVATE_DEFINE_TYPE_4  (   TYPE13  ,                                                                               \
                                                                        TYPE14  ,                                                                               \
                                                                        TYPE15  ,                                                                               \
                                                                        TYPE16                                                                                  \
                                                                    ))                                                                                          \
                                        )

//  implementation of XTypeProvider with 17 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_17( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13, TYPE14, TYPE15, TYPE16, TYPE17 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER_LARGE  (   CLASS,                                                                                                              \
                                            (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                                               \
                                                                        TYPE2   ,                                                                               \
                                                                        TYPE3   ,                                                                               \
                                                                        TYPE4   ,                                                                               \
                                                                        TYPE5   ,                                                                               \
                                                                        TYPE6   ,                                                                               \
                                                                        TYPE7   ,                                                                               \
                                                                        TYPE8   ,                                                                               \
                                                                        TYPE9   ,                                                                               \
                                                                        TYPE10  ,                                                                               \
                                                                        TYPE11  ,                                                                               \
                                                                        TYPE12                                                                                  \
                                                                    )),                                                                                         \
                                            (PRIVATE_DEFINE_TYPE_5  (   TYPE13  ,                                                                               \
                                                                        TYPE14  ,                                                                               \
                                                                        TYPE15  ,                                                                               \
                                                                        TYPE16  ,                                                                               \
                                                                        TYPE17                                                                                  \
                                                                    ))                                                                                          \
                                        )

//  implementation of XTypeProvider with 18 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_18( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13, TYPE14, TYPE15, TYPE16, TYPE17, TYPE18 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER_LARGE  (   CLASS,                                                                                                              \
                                            (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                                               \
                                                                        TYPE2   ,                                                                               \
                                                                        TYPE3   ,                                                                               \
                                                                        TYPE4   ,                                                                               \
                                                                        TYPE5   ,                                                                               \
                                                                        TYPE6   ,                                                                               \
                                                                        TYPE7   ,                                                                               \
                                                                        TYPE8   ,                                                                               \
                                                                        TYPE9   ,                                                                               \
                                                                        TYPE10  ,                                                                               \
                                                                        TYPE11  ,                                                                               \
                                                                        TYPE12                                                                                  \
                                                                    )),                                                                                         \
                                            (PRIVATE_DEFINE_TYPE_6  (   TYPE13  ,                                                                               \
                                                                        TYPE14  ,                                                                               \
                                                                        TYPE15  ,                                                                               \
                                                                        TYPE16  ,                                                                               \
                                                                        TYPE17  ,                                                                               \
                                                                        TYPE18                                                                                  \
                                                                    ))                                                                                          \
                                        )

//  implementation of XTypeProvider with 19 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_19( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13, TYPE14, TYPE15, TYPE16, TYPE17, TYPE18, TYPE19 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER_LARGE  (   CLASS,                                                                                                              \
                                            (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                                               \
                                                                        TYPE2   ,                                                                               \
                                                                        TYPE3   ,                                                                               \
                                                                        TYPE4   ,                                                                               \
                                                                        TYPE5   ,                                                                               \
                                                                        TYPE6   ,                                                                               \
                                                                        TYPE7   ,                                                                               \
                                                                        TYPE8   ,                                                                               \
                                                                        TYPE9   ,                                                                               \
                                                                        TYPE10  ,                                                                               \
                                                                        TYPE11  ,                                                                               \
                                                                        TYPE12                                                                                  \
                                                                    )),                                                                                         \
                                            (PRIVATE_DEFINE_TYPE_7  (   TYPE13  ,                                                                               \
                                                                        TYPE14  ,                                                                               \
                                                                        TYPE15  ,                                                                               \
                                                                        TYPE16  ,                                                                               \
                                                                        TYPE17  ,                                                                               \
                                                                        TYPE18  ,                                                                               \
                                                                        TYPE19                                                                                  \
                                                                    ))                                                                                          \
                                        )

//  implementation of XTypeProvider with 20 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_20( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13, TYPE14, TYPE15, TYPE16, TYPE17, TYPE18, TYPE19, TYPE20 ) \
    PRIVATE_DEFINE_XTYPEPROVIDER_LARGE  (   CLASS,                                                                                                              \
                                            (PRIVATE_DEFINE_TYPE_12 (   TYPE1   ,                                                                               \
                                                                        TYPE2   ,                                                                               \
                                                                        TYPE3   ,                                                                               \
                                                                        TYPE4   ,                                                                               \
                                                                        TYPE5   ,                                                                               \
                                                                        TYPE6   ,                                                                               \
                                                                        TYPE7   ,                                                                               \
                                                                        TYPE8   ,                                                                               \
                                                                        TYPE9   ,                                                                               \
                                                                        TYPE10  ,                                                                               \
                                                                        TYPE11  ,                                                                               \
                                                                        TYPE12                                                                                  \
                                                                    )),                                                                                         \
                                            (PRIVATE_DEFINE_TYPE_8  (   TYPE13  ,                                                                               \
                                                                        TYPE14  ,                                                                               \
                                                                        TYPE15  ,                                                                               \
                                                                        TYPE16  ,                                                                               \
                                                                        TYPE17  ,                                                                               \
                                                                        TYPE18  ,                                                                               \
                                                                        TYPE19  ,                                                                               \
                                                                        TYPE20                                                                                  \
                                                                    ))                                                                                          \
                                        )

//  implementation of XTypeProvider with 1 additional interface for getTypes() AND using 1 baseclass
#define DEFINE_XTYPEPROVIDER_1_WITH_BASECLASS( CLASS, BASECLASS, TYPE1 )                                            \
    PRIVATE_DEFINE_XTYPEPROVIDER_BASECLASS  (   CLASS,                                                              \
                                                BASECLASS,                                                          \
                                                (PRIVATE_DEFINE_TYPE_1  (   TYPE1                                   \
                                                                        ))                                          \
                                            )

//  implementation of XTypeProvider with 2 additional interface for getTypes() AND using 1 baseclass
#define DEFINE_XTYPEPROVIDER_2_WITH_BASECLASS( CLASS, BASECLASS, TYPE1, TYPE2 )                                     \
    PRIVATE_DEFINE_XTYPEPROVIDER_BASECLASS  (   CLASS,                                                              \
                                                BASECLASS,                                                          \
                                                (PRIVATE_DEFINE_TYPE_2  (   TYPE1   ,                               \
                                                                            TYPE2                                   \
                                                                        ))                                          \
                                            )

//  implementation of XTypeProvider with 3 additional interface for getTypes() AND using 1 baseclass
#define DEFINE_XTYPEPROVIDER_3_WITH_BASECLASS( CLASS, BASECLASS, TYPE1, TYPE2, TYPE3 )                              \
    PRIVATE_DEFINE_XTYPEPROVIDER_BASECLASS  (   CLASS,                                                              \
                                                BASECLASS,                                                          \
                                                (PRIVATE_DEFINE_TYPE_3  (   TYPE1   ,                               \
                                                                            TYPE2   ,                               \
                                                                            TYPE3                                   \
                                                                        ))                                          \
                                            )
//  implementation of XTypeProvider with 4 additional interface for getTypes() AND using 1 baseclass
#define DEFINE_XTYPEPROVIDER_4_WITH_BASECLASS( CLASS, BASECLASS, TYPE1, TYPE2, TYPE3, TYPE4 )                       \
    PRIVATE_DEFINE_XTYPEPROVIDER_BASECLASS  (   CLASS,                                                              \
                                                BASECLASS,                                                          \
                                                (PRIVATE_DEFINE_TYPE_4  (   TYPE1   ,                               \
                                                                            TYPE2   ,                               \
                                                                            TYPE3   ,                               \
                                                                            TYPE4                                   \
                                                                        ))                                          \
                                            )
//  implementation of XTypeProvider with 5 additional interface for getTypes() AND using 1 baseclass
#define DEFINE_XTYPEPROVIDER_5_WITH_BASECLASS( CLASS, BASECLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 )                \
    PRIVATE_DEFINE_XTYPEPROVIDER_BASECLASS  (   CLASS,                                                              \
                                                BASECLASS,                                                          \
                                                (PRIVATE_DEFINE_TYPE_5  (   TYPE1   ,                               \
                                                                            TYPE2   ,                               \
                                                                            TYPE3   ,                               \
                                                                            TYPE4   ,                               \
                                                                            TYPE5                                   \
                                                                        ))                                          \
                                            )

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
