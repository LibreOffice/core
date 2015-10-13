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

#ifndef INCLUDED_FRAMEWORK_INC_MACROS_XTYPEPROVIDER_HXX
#define INCLUDED_FRAMEWORK_INC_MACROS_XTYPEPROVIDER_HXX

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

//  private
//  implementation of XTypeProvider::getImplementationId()

#define PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                               \
    css::uno::Sequence< sal_Int8 > SAL_CALL CLASS::getImplementationId() throw( css::uno::RuntimeException, std::exception )          \
    {                                                                                                                                           \
        return css::uno::Sequence<sal_Int8>();                                                                                                  \
    }

//  private
//  implementation of XTypeProvider::getTypes() with max. 12 interfaces!

#define PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES( CLASS, TYPES )                                                                                   \
    css::uno::Sequence< css::uno::Type > SAL_CALL CLASS::getTypes() throw( css::uno::RuntimeException, std::exception )  \
    {                                                                                                                                           \
        /* Optimize this method !                                       */                                                                      \
        /* We initialize a static variable only one time.               */                                                                      \
        /* And we don't must use a mutex at every call!                 */                                                                      \
        /* For the first call; pTypeCollection is NULL -                */                                                                      \
        /* for the second call pTypeCollection is different from NULL!  */                                                                      \
        static ::cppu::OTypeCollection* pTypeCollection = NULL;                                                                                \
        if ( pTypeCollection == NULL )                                                                                                          \
        {                                                                                                                                       \
            /* Ready for multithreading; get global mutex for first call of this method only! see before   */                                   \
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );                                                                         \
            /* Control these pointer again ... it can be, that another instance will be faster then these! */                                   \
            if ( pTypeCollection == NULL )                                                                                                      \
            {                                                                                                                                   \
                /* Create a static typecollection ...           */                                                                              \
                /* Attention: "TYPES" will expand to "(...)"!   */                                                                              \
                static ::cppu::OTypeCollection aTypeCollection TYPES;                                                                          \
                /* ... and set his address to static pointer! */                                                                                \
                pTypeCollection = &aTypeCollection;                                                                                            \
            }                                                                                                                                   \
        }                                                                                                                                       \
        return pTypeCollection->getTypes();                                                                                                     \
    }

//  private
//  implementation of XTypeProvider::getTypes() with more than 12 interfaces!

#define PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES_LARGE( CLASS, TYPES_FIRST, TYPES_SECOND )                                                         \
    css::uno::Sequence< css::uno::Type > SAL_CALL CLASS::getTypes() throw( css::uno::RuntimeException, std::exception )  \
    {                                                                                                                                           \
        /* Optimize this method !                                       */                                                                      \
        /* We initialize a static variable only one time.               */                                                                      \
        /* And we don't must use a mutex at every call!                 */                                                                      \
        /* For the first call; pTypeCollection is NULL -                */                                                                      \
        /* for the second call pTypeCollection is different from NULL!  */                                                                      \
        static css::uno::Sequence< css::uno::Type >* pTypeCollection = NULL;                                         \
        if ( pTypeCollection == NULL )                                                                                                          \
        {                                                                                                                                       \
            /* Ready for multithreading; get global mutex for first call of this method only! see before   */                                   \
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );                                                                         \
            /* Control these pointer again ... it can be, that another instance will be faster then these! */                                   \
            if ( pTypeCollection == NULL )                                                                                                      \
            {                                                                                                                                   \
                /* Create two typecollections                           */                                                                      \
                /* (cppuhelper support 12 items per collection only!)   */                                                                      \
                ::cppu::OTypeCollection aTypeCollection1 TYPES_FIRST;                                                                       \
                ::cppu::OTypeCollection aTypeCollection2 TYPES_SECOND;                                                                       \
                /* Copy all items from both sequences to one result list! */                                                                    \
                css::uno::Sequence< css::uno::Type >          seqTypes1   = aTypeCollection1.getTypes();              \
                css::uno::Sequence< css::uno::Type >          seqTypes2   = aTypeCollection2.getTypes();              \
                sal_Int32                                     nCount1     = seqTypes1.getLength();                    \
                sal_Int32                                     nCount2     = seqTypes2.getLength();                    \
                static css::uno::Sequence< css::uno::Type >   seqResult   ( nCount1+nCount2 );                        \
                sal_Int32                                     nSource     = 0;                                        \
                sal_Int32                                     nDestination= 0;                                        \
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

//  private
//  help macros to replace TYPES in getTypes() [see before]

#define PRIVATE_DEFINE_TYPE_1( TYPE1 )                                                                                                          \
    cppu::UnoType<TYPE1>::get()

#define PRIVATE_DEFINE_TYPE_2( TYPE1, TYPE2 )                                                                                                   \
    PRIVATE_DEFINE_TYPE_1( TYPE1 ),                                                                                                             \
    cppu::UnoType<TYPE2>::get()

#define PRIVATE_DEFINE_TYPE_3( TYPE1, TYPE2, TYPE3 )                                                                                            \
    PRIVATE_DEFINE_TYPE_2( TYPE1, TYPE2 ),                                                                                                      \
    cppu::UnoType<TYPE3>::get()

#define PRIVATE_DEFINE_TYPE_4( TYPE1, TYPE2, TYPE3, TYPE4 )                                                                                     \
    PRIVATE_DEFINE_TYPE_3( TYPE1, TYPE2, TYPE3 ),                                                                                               \
    cppu::UnoType<TYPE4>::get()

#define PRIVATE_DEFINE_TYPE_5( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 )                                                                              \
    PRIVATE_DEFINE_TYPE_4( TYPE1, TYPE2, TYPE3, TYPE4 ),                                                                                        \
    cppu::UnoType<TYPE5>::get()

#define PRIVATE_DEFINE_TYPE_6( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 )                                                                       \
    PRIVATE_DEFINE_TYPE_5( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 ),                                                                                 \
    cppu::UnoType<TYPE6>::get()

#define PRIVATE_DEFINE_TYPE_7( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 )                                                                \
    PRIVATE_DEFINE_TYPE_6( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 ),                                                                          \
    cppu::UnoType<TYPE7>::get()

#define PRIVATE_DEFINE_TYPE_8( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 )                                                         \
    PRIVATE_DEFINE_TYPE_7( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 ),                                                                   \
    cppu::UnoType<TYPE8>::get()

#define PRIVATE_DEFINE_TYPE_9( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 )                                                  \
    PRIVATE_DEFINE_TYPE_8( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 ),                                                            \
    cppu::UnoType<TYPE9>::get()

#define PRIVATE_DEFINE_TYPE_10( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 )                                         \
    PRIVATE_DEFINE_TYPE_9( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 ),                                                     \
    cppu::UnoType<TYPE10>::get()

#define PRIVATE_DEFINE_TYPE_11( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 )                                 \
    PRIVATE_DEFINE_TYPE_10( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 ),                                            \
    cppu::UnoType<TYPE11>::get()

#define PRIVATE_DEFINE_TYPE_12( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12 )                         \
    PRIVATE_DEFINE_TYPE_11( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 ),                                    \
    cppu::UnoType<TYPE12>::get()

//  private
//  complete implementation of XTypeProvider

#define PRIVATE_DEFINE_XTYPEPROVIDER( CLASS, TYPES )                                                                                                        \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                                               \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES( CLASS, TYPES )

#define PRIVATE_DEFINE_XTYPEPROVIDER_LARGE( CLASS, TYPES_FIRST, TYPES_SECOND )                                                                              \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETIMPLEMENTATIONID( CLASS )                                                                                               \
    PRIVATE_DEFINE_XTYPEPROVIDER_GETTYPES_LARGE( CLASS, TYPES_FIRST, TYPES_SECOND )

//  public
//  declaration of XTypeProvider

#define FWK_DECLARE_XTYPEPROVIDER                                                                                                                               \
    virtual css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes           () throw( css::uno::RuntimeException, std::exception ) override;\
    virtual css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw( css::uno::RuntimeException, std::exception ) override;

//  public
//  implementation of XTypeProvider

//  implementation of XTypeProvider without additional interface for getTypes()
//  XTypeProvider is used as the only one interface automatically.


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

//  implementation of XTypeProvider with 20 additional interfaces for getTypes()
#define DEFINE_XTYPEPROVIDER_21( CLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13, TYPE14, TYPE15, TYPE16, TYPE17, TYPE18, TYPE19, TYPE20, TYPE21 ) \
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
                                            (PRIVATE_DEFINE_TYPE_9  (   TYPE13  ,                                                                               \
                                                                        TYPE14  ,                                                                               \
                                                                        TYPE15  ,                                                                               \
                                                                        TYPE16  ,                                                                               \
                                                                        TYPE17  ,                                                                               \
                                                                        TYPE18  ,                                                                               \
                                                                        TYPE19  ,                                                                               \
                                                                        TYPE20  ,                                                                               \
                                                                        TYPE21                                                                                  \
                                                                    ))                                                                                          \
                                        )


}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_MACROS_XTYPEPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
