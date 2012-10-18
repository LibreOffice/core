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

#ifndef _SFX_SFXUNO_HXX
#define _SFX_SFXUNO_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>


//________________________________________________________________________________________________________________________
//  fix uno header
//________________________________________________________________________________________________________________________
#include <com/sun/star/uno/Type.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/factory.hxx>

//________________________________________________________________________________________________________________________
//  something else ... header
//________________________________________________________________________________________________________________________
#include <osl/mutex.hxx>

#include <rtl/ustring.hxx>

//________________________________________________________________________________________________________________________
//  defines for namespaces !
//________________________________________________________________________________________________________________________

#define UNOANY                          ::com::sun::star::uno::Any
#define UNOEXCEPTION                    ::com::sun::star::uno::Exception
#define UNOMUTEX                        ::osl::Mutex
#define UNOMUTEXGUARD                   ::osl::MutexGuard
#define UNOOIMPLEMENTATIONID            ::cppu::OImplementationId
#define UNOOTYPECOLLECTION              ::cppu::OTypeCollection
#define UNOPROPERTYVALUE                ::com::sun::star::beans::PropertyValue
#define UNOREFERENCE                    ::com::sun::star::uno::Reference
#define UNORUNTIMEEXCEPTION             ::com::sun::star::uno::RuntimeException
#define UNOSEQUENCE                     ::com::sun::star::uno::Sequence
#define UNOTYPE                         ::com::sun::star::uno::Type
#define UNOURL                          ::com::sun::star::util::URL
#define UNOXINTERFACE                   ::com::sun::star::uno::XInterface
#define UNOXMULTISERVICEFACTORY         ::com::sun::star::lang::XMultiServiceFactory
#define UNOXSINGLESERVICEFACTORY        ::com::sun::star::lang::XSingleServiceFactory
#define UNOXTYPEPROVIDER                ::com::sun::star::lang::XTypeProvider

//________________________________________________________________________________________________________________________
//  declarations and defines for sfx
//________________________________________________________________________________________________________________________

inline sal_Bool operator==( const UNOURL& aURL1, const UNOURL& aURL2 )
{
    return aURL1.Complete == aURL2.Complete;
}

class SfxAllItemSet ;
class SfxItemSet    ;
class SfxSlot       ;

SFX2_DLLPUBLIC void TransformParameters(            sal_uInt16                          nSlotId     ,
                            const   UNOSEQUENCE< UNOPROPERTYVALUE >&    seqArgs     ,
                                    SfxAllItemSet&                      aSet        ,
                            const   SfxSlot*                            pSlot = 0   );

SFX2_DLLPUBLIC void TransformItems(         sal_uInt16                          nSlotId     ,
                            const   SfxItemSet&                         aSet        ,
                                    UNOSEQUENCE< UNOPROPERTYVALUE >&    seqArgs     ,
                            const   SfxSlot*                            pSlot = 0   );

bool GetEncryptionData_Impl( const SfxItemSet* pSet, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aEncryptionData );

#define FrameSearchFlags            sal_Int32

// Macros to convert string -> unicode and unicode -> string.
// We use UTF8 everytime. Its the best way to do this!
#define S2U(STRING)                             ::rtl::OStringToOUString(STRING, RTL_TEXTENCODING_UTF8)
#define U2S(STRING)                             ::rtl::OUStringToOString(STRING, RTL_TEXTENCODING_UTF8)

// Macro to define const unicode a'la "..."
// It's better then "OUString::createFromAscii(...)" !!!
#define DEFINE_CONST_UNICODE(CONSTASCII)        UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))
#define DEFINE_CONST_OUSTRING(CONSTASCII)       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))

//________________________________________________________________________________________________________________________
//  macros for declaration and definition of uno-services
//________________________________________________________________________________________________________________________

//************************************************************************************************************************
//  declaration of      XInterface::queryInterface()
//                      XInterface::aquire()
//                      XInterface::release()
//                      XTypeProvider::getTypes()
//                      XTypeProvider::getImplementationId()
//************************************************************************************************************************
#define SFX_DECL_XINTERFACE \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw( ::com::sun::star::uno::RuntimeException ); \
    virtual void SAL_CALL acquire() throw(); \
    virtual void SAL_CALL release() throw();

#define SFX_DECL_XTYPEPROVIDER                                                                                                      \
    /* XTypeProvider */                                                                                                                             \
    virtual UNOSEQUENCE< UNOTYPE >  SAL_CALL getTypes() throw( UNORUNTIMEEXCEPTION );                                                               \
    virtual UNOSEQUENCE< sal_Int8 > SAL_CALL getImplementationId() throw( UNORUNTIMEEXCEPTION );

#define SFX_DECL_XINTERFACE_XTYPEPROVIDER                                                                                            \
    SFX_DECL_XINTERFACE                                                                                                               \
    SFX_DECL_XTYPEPROVIDER

//************************************************************************************************************************
//  declaration of      XInterface::queryInterface()
//                      XInterface::aquire()
//                      XInterface::release()
//                      XTypeProvider::getTypes()
//                      XTypeProvider::getImplementationId()
//                      XServiceInfo::getImplementationName()
//                      XServiceInfo::supportsService()
//                      XServiceInfo::getSupportedServiceNames()
//                      static xxx::impl_getStaticSupportedServiceNames()
//                      static xxx::impl_getStaticImplementationName()
//                      static xxx::impl_createInstance()
//************************************************************************************************************************
#define SFX_DECL_XSERVICEINFO_NOFACTORY                                                                                   \
    /* XServiceInfo */                                                                                                                              \
    virtual rtl::OUString SAL_CALL getImplementationName() throw( UNORUNTIMEEXCEPTION );                                                            \
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& sServiceName ) throw( UNORUNTIMEEXCEPTION );                                    \
    virtual UNOSEQUENCE< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION );                                          \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    static UNOSEQUENCE< rtl::OUString > impl_getStaticSupportedServiceNames();                                                                      \
    static rtl::OUString impl_getStaticImplementationName();                                                                                        \
                                                                                                                                                    \
    /* Helper for registry */                                                                                                                       \
    static UNOREFERENCE< UNOXINTERFACE > SAL_CALL impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION );

//************************************************************************************************************************
//  declaration of      XInterface::queryInterface()
//                      XInterface::aquire()
//                      XInterface::release()
//                      XTypeProvider::getTypes()
//                      XTypeProvider::getImplementationId()
//                      XServiceInfo::getImplementationName()
//                      XServiceInfo::supportsService()
//                      XServiceInfo::getSupportedServiceNames()
//                      static xxx::impl_getStaticSupportedServiceNames()
//                      static xxx::impl_getStaticImplementationName()
//                      static xxx::impl_createInstance()
//                      static xxx::impl_createFactory()
//************************************************************************************************************************
#define SFX_DECL_XSERVICEINFO                                                                                             \
    SFX_DECL_XSERVICEINFO_NOFACTORY                                                                                       \
    static UNOREFERENCE< UNOXSINGLESERVICEFACTORY > impl_createFactory( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager );

#define SFX_DECL_XINTERFACE_XTYPEPROVIDER_XSERVICEINFO                                                                                              \
                                                                                                                                                    \
    /* XInterface + XTypeProvider */                                                                                                                \
    SFX_DECL_XINTERFACE_XTYPEPROVIDER                                                                                                               \
    SFX_DECL_XSERVICEINFO

//************************************************************************************************************************
//  implementation of   XInterface::queryInterface()
//                      XInterface::aquire()
//                      XInterface::release()
//************************************************************************************************************************
/*_________________________________________________________________________________________________________________________*/
// DON'T USE FOLLOW MACROS DIRECTLY!!!
#define SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, IMPLINTERFACES )                                                                            \
                                                                                                                                                    \
    void SAL_CALL IMPLCLASS::acquire() throw()                                                                                  \
    {                                                                                                                                               \
        /* Don't use mutex in methods of XInterface! */                                                                                             \
        BASECLASS::acquire();                                                                                                                       \
    }                                                                                                                                               \
                                                                                                                                                    \
    void SAL_CALL IMPLCLASS::release() throw()                                                                                  \
    {                                                                                                                                               \
        /* Don't use mutex in methods of XInterface! */                                                                                             \
        BASECLASS::release();                                                                                                                       \
    }                                                                                                                                               \
                                                                                                                                                    \
    UNOANY SAL_CALL IMPLCLASS::queryInterface( const UNOTYPE& rType ) throw( UNORUNTIMEEXCEPTION )                                                  \
    {                                                                                                                                               \
        /* Attention: Don't use mutex or guard in this method!!! Is a method of XInterface. */                                                      \
        /* Ask for my own supported interfaces ...                                          */                                                      \
        UNOANY aReturn  ( ::cppu::queryInterface(   rType,                                                                                          \
                                                    IMPLINTERFACES                                                                              \
                                                )                                                                                                   \
                        );                                                                                                                          \
        /* If searched interface supported by this class ... */                                                                                     \
        if ( aReturn.hasValue() == sal_True )                                                                                                       \
        {                                                                                                                                           \
            /* ... return this information. */                                                                                                      \
            return aReturn ;                                                                                                                        \
        }                                                                                                                                           \
        else                                                                                                                                        \
        {                                                                                                                                           \
            /* Else; ... ask baseclass for interfaces! */                                                                                           \
            return BASECLASS::queryInterface( rType );                                                                                              \
        }                                                                                                                                           \
    }

#define SFX_IMPL_INTERFACE_0                                                                                                                        \
    static_cast< UNOXTYPEPROVIDER* >( this )

#define SFX_IMPL_INTERFACE_1( INTERFACE1 )                                                                                                          \
    SFX_IMPL_INTERFACE_0,                                                                                                                           \
    static_cast< INTERFACE1* >( this )

#define SFX_IMPL_INTERFACE_2( INTERFACE1, INTERFACE2 )                                                                                              \
    SFX_IMPL_INTERFACE_1( INTERFACE1 ),                                                                                                         \
    static_cast< INTERFACE2* >( this )

#define SFX_IMPL_INTERFACE_3( INTERFACE1, INTERFACE2, INTERFACE3 )                                                                                  \
    SFX_IMPL_INTERFACE_2( INTERFACE1, INTERFACE2 ),                                                                                             \
    static_cast< INTERFACE3* >( this )

#define SFX_IMPL_INTERFACE_4( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4 )                                                                      \
    SFX_IMPL_INTERFACE_3( INTERFACE1, INTERFACE2, INTERFACE3 ),                                                                             \
    static_cast< INTERFACE4* >( this )

#define SFX_IMPL_INTERFACE_5( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5 )                                                          \
    SFX_IMPL_INTERFACE_4( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4 ),                                                                 \
    static_cast< INTERFACE5* >( this )

#define SFX_IMPL_INTERFACE_6( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6 )                                              \
    SFX_IMPL_INTERFACE_5( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5 ),                                                 \
    static_cast< INTERFACE6* >( this )

#define SFX_IMPL_INTERFACE_7( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7 )                                                              \
    SFX_IMPL_INTERFACE_6( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6 ),                                                                 \
    static_cast< INTERFACE7* >( this )

#define SFX_IMPL_INTERFACE_8( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8 )                                                  \
    SFX_IMPL_INTERFACE_7( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7 ),                                                 \
    static_cast< INTERFACE8* >( this )

#define SFX_IMPL_INTERFACE_9( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9 )                                      \
    SFX_IMPL_INTERFACE_8( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8 ),                                     \
    static_cast< INTERFACE9* >( this )

#define SFX_IMPL_INTERFACE_10( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10 )                        \
    SFX_IMPL_INTERFACE_9( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9 ),                     \
    static_cast< INTERFACE10* >( this )

#define SFX_IMPL_INTERFACE_11( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11 )           \
    SFX_IMPL_INTERFACE_10( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10 ),       \
    static_cast< INTERFACE11* >( this )

#define SFX_IMPL_INTERFACE_12( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11, INTERFACE12 )          \
    SFX_IMPL_INTERFACE_11( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11 ),      \
    static_cast< INTERFACE12* >( this )

#define SFX_IMPL_INTERFACE_13( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11, INTERFACE12, INTERFACE13 )         \
    SFX_IMPL_INTERFACE_12( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11, INTERFACE12 ),     \
    static_cast< INTERFACE13* >( this )
/*_________________________________________________________________________________________________________________________*/

// Follow macros can used directly :-)
//  implementation of XInterface with 0 additional interface for queryInterface()
#define SFX_IMPL_XINTERFACE_0( IMPLCLASS, BASECLASS )                                                                                               \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_0 )

//  implementation of XInterface with 1 additional interface for queryInterface()
#define SFX_IMPL_XINTERFACE_1( IMPLCLASS, BASECLASS, INTERFACE1 )                                                                                   \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_1( INTERFACE1 ) )

//  implementation of XInterface with 2 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_2( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2 )                                                                       \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_2( INTERFACE1, INTERFACE2 ) )

//  implementation of XInterface with 3 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_3( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3 )                                                           \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_3( INTERFACE1, INTERFACE2, INTERFACE3 ) )

//  implementation of XInterface with 4 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_4( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4 )                                               \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_4( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4 ) )

//  implementation of XInterface with 5 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_5( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5 )                                                                                                                               \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_5( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5 ) )

//  implementation of XInterface with 6 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_6( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6 )                                                                                                                   \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_6( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6 ) )

//  implementation of XInterface with 7 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_7( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7 )                                                                                                       \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_7( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7 ) )

//  implementation of XInterface with 8 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_8( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8 )                                                                                           \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_8( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8 ) )

//  implementation of XInterface with 9 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_9( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9 )                                                                               \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_9( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9 ) )

//  implementation of XInterface with 10 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_10( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10 )                                                                 \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_10( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10 ) )

//  implementation of XInterface with 11 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_11( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11 )                                                    \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_11( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11 ) )

//  implementation of XInterface with 11 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_12( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11, INTERFACE12 )                                                   \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_12( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11, INTERFACE12 ) )

//  implementation of XInterface with 11 additional interfaces for queryInterface()
#define SFX_IMPL_XINTERFACE_13( IMPLCLASS, BASECLASS, INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11, INTERFACE12, INTERFACE13 )                                                  \
    SFX_IMPL_XINTERFACE_BASE( IMPLCLASS, BASECLASS, SFX_IMPL_INTERFACE_13( INTERFACE1, INTERFACE2, INTERFACE3, INTERFACE4, INTERFACE5, INTERFACE6, INTERFACE7, INTERFACE8, INTERFACE9, INTERFACE10, INTERFACE11, INTERFACE12, INTERFACE13 ) )

//************************************************************************************************************************
//  implementation of   XTypeProvider::getTypes()
//                      XTypeProvider::getImplementationId()
//************************************************************************************************************************
/*_________________________________________________________________________________________________________________________*/
// DON'T USE FOLLOW MACROS DIRECTLY!!!
#define SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, IMPLTYPES )                                                                                         \
                                                                                                                                                    \
    UNOSEQUENCE< sal_Int8 > SAL_CALL IMPLCLASS::getImplementationId() throw( UNORUNTIMEEXCEPTION )                                              \
    {                                                                                                                                               \
        /* Create one Id for all instances of this class.                                               */                                          \
        /* Use ethernet address to do this! (sal_True)                                                  */                                          \
        /* Optimize this method                                                                         */                                          \
        /* We initialize a static variable only one time. And we don't must use a mutex at every call!  */                                          \
        /* For the first call; pID is NULL - for the second call pID is different from NULL!            */                                          \
        static UNOOIMPLEMENTATIONID* pID = NULL ;                                                                                                   \
        if ( pID == NULL )                                                                                                                          \
        {                                                                                                                                           \
            /* Ready for multithreading; get global mutex for first call of this method only! see before */                                         \
            UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );                                                                                     \
            /* Control these pointer again ... it can be, that another instance will be faster then these! */                                       \
            if ( pID == NULL )                                                                                                                      \
            {                                                                                                                                       \
                /* Create a new static ID ... */                                                                                                    \
                static UNOOIMPLEMENTATIONID aID( sal_False );                                                                                       \
                /* ... and set his address to static pointer! */                                                                                    \
                pID = &aID ;                                                                                                                        \
            }                                                                                                                                       \
        }                                                                                                                                           \
        return pID->getImplementationId();                                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    UNOSEQUENCE< UNOTYPE > SAL_CALL IMPLCLASS::getTypes() throw( UNORUNTIMEEXCEPTION )                                                          \
    {                                                                                                                                               \
        /* Optimize this method !                                                                                   */                              \
        /* We initialize a static variable only one time. And we don't must use a mutex at every call!              */                              \
        /* For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!*/                              \
        static UNOOTYPECOLLECTION* pTypeCollection = NULL ;                                                                                         \
        if ( pTypeCollection == NULL )                                                                                                              \
        {                                                                                                                                           \
            /* Ready for multithreading; get global mutex for first call of this method only! see before */                                         \
            UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );                                                                                     \
            /* Control these pointer again ... it can be, that another instance will be faster then these! */                                       \
            if ( pTypeCollection == NULL )                                                                                                          \
            {                                                                                                                                       \
                /* Create a static typecollection ... */                                                                                            \
                static UNOOTYPECOLLECTION aTypeCollection(  IMPLTYPES );                                                                            \
                /* ... and set his address to static pointer! */                                                                                    \
                pTypeCollection = &aTypeCollection ;                                                                                                \
            }                                                                                                                                       \
        }                                                                                                                                           \
        return pTypeCollection->getTypes();                                                                                                         \
    }

#define SFX_IMPL_TYPE_0                                                                                                                             \
    ::getCppuType(( const UNOREFERENCE< UNOXTYPEPROVIDER >*)NULL )

#define SFX_IMPL_TYPE_1( TYPE1 )                                                                                                                    \
    SFX_IMPL_TYPE_0,                                                                                                                                \
    ::getCppuType(( const UNOREFERENCE< TYPE1 >*)NULL )

#define SFX_IMPL_TYPE_2( TYPE1, TYPE2 )                                                                                                             \
    SFX_IMPL_TYPE_1( TYPE1 ),                                                                                                                       \
    ::getCppuType(( const UNOREFERENCE< TYPE2 >*)NULL )

#define SFX_IMPL_TYPE_3( TYPE1, TYPE2, TYPE3 )                                                                                                      \
    SFX_IMPL_TYPE_2( TYPE1, TYPE2 ),                                                                                                            \
    ::getCppuType(( const UNOREFERENCE< TYPE3 >*)NULL )

#define SFX_IMPL_TYPE_4( TYPE1, TYPE2, TYPE3, TYPE4 )                                                                                               \
    SFX_IMPL_TYPE_3( TYPE1, TYPE2, TYPE3 ),                                                                                                 \
    ::getCppuType(( const UNOREFERENCE< TYPE4 >*)NULL )

#define SFX_IMPL_TYPE_5( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 )                                                                                        \
    SFX_IMPL_TYPE_4( TYPE1, TYPE2, TYPE3, TYPE4 ),                                                                                          \
    ::getCppuType(( const UNOREFERENCE< TYPE5 >*)NULL )

#define SFX_IMPL_TYPE_6( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 )                                                                                 \
    SFX_IMPL_TYPE_5( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 ),                                                                                   \
    ::getCppuType(( const UNOREFERENCE< TYPE6 >*)NULL )

#define SFX_IMPL_TYPE_7( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 )                                                                          \
    SFX_IMPL_TYPE_6( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 ),                                                                        \
    ::getCppuType(( const UNOREFERENCE< TYPE7 >*)NULL )

#define SFX_IMPL_TYPE_8( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 )                                                                   \
    SFX_IMPL_TYPE_7( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 ),                                                             \
    ::getCppuType(( const UNOREFERENCE< TYPE8 >*)NULL )

#define SFX_IMPL_TYPE_9( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 )                                                            \
    SFX_IMPL_TYPE_8( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 ),                                                      \
    ::getCppuType(( const UNOREFERENCE< TYPE9 >*)NULL )

#define SFX_IMPL_TYPE_10( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 )                                                   \
    SFX_IMPL_TYPE_9( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 ),                                               \
    ::getCppuType(( const UNOREFERENCE< TYPE10 >*)NULL )

#define SFX_IMPL_TYPE_11( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 )                                           \
    SFX_IMPL_TYPE_10( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 ),                                  \
    ::getCppuType(( const UNOREFERENCE< TYPE11 >*)NULL )

#define SFX_IMPL_TYPE_12( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12 )                                           \
    SFX_IMPL_TYPE_11( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 ),                                  \
    ::getCppuType(( const UNOREFERENCE< TYPE12 >*)NULL )

#define SFX_IMPL_TYPE_13( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13 )                                           \
    SFX_IMPL_TYPE_12( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12 ),                                  \
    ::getCppuType(( const UNOREFERENCE< TYPE13 >*)NULL )
/*_________________________________________________________________________________________________________________________*/

// FOLLOW MACROS CAN USED DIRECTLY :-)

//  implementation of XInterface with 1 additional interface for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_0( IMPLCLASS )                                                                                                       \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_0 )

//  implementation of XInterface with 1 additional interface for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_1( IMPLCLASS, TYPE1 )                                                                                                \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_1( TYPE1 ) )

//  implementation of XInterface with 2 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_2( IMPLCLASS, TYPE1, TYPE2 )                                                                                         \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_2( TYPE1, TYPE2 ) )

//  implementation of XInterface with 3 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_3( IMPLCLASS, TYPE1, TYPE2, TYPE3 )                                                                                  \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_3( TYPE1, TYPE2, TYPE3 ) )

//  implementation of XInterface with 4 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_4( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4 )                                                                           \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_4( TYPE1, TYPE2, TYPE3, TYPE4 ) )

//  implementation of XInterface with 5 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_5( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 )                                                                    \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_5( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 ) )

//  implementation of XInterface with 6 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_6( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 )                                                             \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_6( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6 ) )

//  implementation of XInterface with 7 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_7( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 )                                                      \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_7( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7 ) )

//  implementation of XInterface with 8 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_8( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 )                                               \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_8( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8 ) )

//  implementation of XInterface with 9 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_9( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 )                                                                \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_9( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9 ) )

//  implementation of XInterface with 10 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_10( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 )                                                       \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_10( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10 ) )

//  implementation of XInterface with 11 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_11( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 )                                               \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_11( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11 ) )

//  implementation of XInterface with 11 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_12( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12 )                                               \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_12( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12 ) )

//  implementation of XInterface with 11 additional interfaces for queryInterface()
#define SFX_IMPL_XTYPEPROVIDER_13( IMPLCLASS, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13 )                                               \
    SFX_IMPL_XTYPEPROVIDER_BASE( IMPLCLASS, SFX_IMPL_TYPE_13( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9, TYPE10, TYPE11, TYPE12, TYPE13 ) )

//************************************************************************************************************************
//  implementation of   XServiceInfo::getImplementationName()
//                      XServiceInfo::supportsService()
//                      XServiceInfo::getSupportedServiceNames()
//                      static xxx::impl_getStaticSupportedServiceNames()
//                      static xxx::impl_getStaticImplementationName()
//                      static xxx::impl_createInstance()
//************************************************************************************************************************
#define SFX_IMPL_XSERVICEINFO( IMPLCLASS, IMPLSERVICENAME, IMPLNAME )                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    rtl::OUString SAL_CALL IMPLCLASS::getImplementationName() throw( UNORUNTIMEEXCEPTION )                                                          \
    {                                                                                                                                               \
        return impl_getStaticImplementationName();                                                                                                  \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    sal_Bool SAL_CALL IMPLCLASS::supportsService( const rtl::OUString& sServiceName ) throw( UNORUNTIMEEXCEPTION )                                  \
    {                                                                                                                                               \
        UNOSEQUENCE< rtl::OUString > seqServiceNames = getSupportedServiceNames();                                                                  \
        const rtl::OUString*         pArray          = seqServiceNames.getConstArray();                                                             \
        for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )                                                              \
        {                                                                                                                                           \
            if ( pArray[nCounter] == sServiceName )                                                                                                 \
            {                                                                                                                                       \
                return sal_True ;                                                                                                                   \
            }                                                                                                                                       \
        }                                                                                                                                           \
        return sal_False ;                                                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    UNOSEQUENCE< rtl::OUString > SAL_CALL IMPLCLASS::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )                                        \
    {                                                                                                                                               \
        return impl_getStaticSupportedServiceNames();                                                                                               \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    UNOSEQUENCE< rtl::OUString > IMPLCLASS::impl_getStaticSupportedServiceNames()                                                                   \
    {                                                                                                                                               \
        UNOSEQUENCE< rtl::OUString > seqServiceNames( 1 );                                                                                          \
        seqServiceNames.getArray() [0] = rtl::OUString::createFromAscii( IMPLSERVICENAME );                                                         \
        return seqServiceNames ;                                                                                                                    \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    rtl::OUString IMPLCLASS::impl_getStaticImplementationName()                                                                                     \
    {                                                                                                                                               \
        return rtl::OUString::createFromAscii( IMPLNAME );                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for registry */                                                                                                                       \
    UNOREFERENCE< UNOXINTERFACE > SAL_CALL IMPLCLASS::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )       \
    {                                                                                                                                               \
        return UNOREFERENCE< UNOXINTERFACE >( *new IMPLCLASS( xServiceManager ) );                                                              \
    }

//************************************************************************************************************************
//  implementation of   XServiceInfo::getImplementationName()
//                      XServiceInfo::supportsService()
//                      XServiceInfo::getSupportedServiceNames()
//                      static xxx::impl_getStaticSupportedServiceNames()
//                      static xxx::impl_getStaticImplementationName()
//                      static xxx::impl_createInstance()
//************************************************************************************************************************
#define SFX_IMPL_XSERVICEINFO_CTX( IMPLCLASS, IMPLSERVICENAME, IMPLNAME )                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    rtl::OUString SAL_CALL IMPLCLASS::getImplementationName() throw( UNORUNTIMEEXCEPTION )                                                          \
    {                                                                                                                                               \
        return impl_getStaticImplementationName();                                                                                                  \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    sal_Bool SAL_CALL IMPLCLASS::supportsService( const rtl::OUString& sServiceName ) throw( UNORUNTIMEEXCEPTION )                                  \
    {                                                                                                                                               \
        UNOSEQUENCE< rtl::OUString > seqServiceNames = getSupportedServiceNames();                                                                  \
        const rtl::OUString*         pArray          = seqServiceNames.getConstArray();                                                             \
        for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )                                                              \
        {                                                                                                                                           \
            if ( pArray[nCounter] == sServiceName )                                                                                                 \
            {                                                                                                                                       \
                return sal_True ;                                                                                                                   \
            }                                                                                                                                       \
        }                                                                                                                                           \
        return sal_False ;                                                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* XServiceInfo */                                                                                                                              \
    UNOSEQUENCE< rtl::OUString > SAL_CALL IMPLCLASS::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )                                        \
    {                                                                                                                                               \
        return impl_getStaticSupportedServiceNames();                                                                                               \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    UNOSEQUENCE< rtl::OUString > IMPLCLASS::impl_getStaticSupportedServiceNames()                                                                   \
    {                                                                                                                                               \
        UNOSEQUENCE< rtl::OUString > seqServiceNames( 1 );                                                                                          \
        seqServiceNames.getArray() [0] = rtl::OUString::createFromAscii( IMPLSERVICENAME );                                                         \
        return seqServiceNames ;                                                                                                                    \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for XServiceInfo */                                                                                                                   \
    rtl::OUString IMPLCLASS::impl_getStaticImplementationName()                                                                                     \
    {                                                                                                                                               \
        return rtl::OUString::createFromAscii( IMPLNAME );                                                                                          \
    }                                                                                                                                               \
                                                                                                                                                    \
    /* Helper for registry */                                                                                                                       \
    UNOREFERENCE< UNOXINTERFACE > SAL_CALL IMPLCLASS::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )       \
    {                                                                                                                                               \
        return UNOREFERENCE< UNOXINTERFACE >( *new IMPLCLASS( comphelper::getComponentContext(xServiceManager) ) );                                                              \
    }

//************************************************************************************************************************
//  definition of createFactory() for MultiServices
//************************************************************************************************************************
#define SFX_IMPL_SINGLEFACTORY( IMPLCLASS )                                                                                                         \
                                                                                                                                                    \
    UNOREFERENCE< UNOXSINGLESERVICEFACTORY > IMPLCLASS::impl_createFactory( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager )        \
    {                                                                                                                                               \
        UNOREFERENCE< UNOXSINGLESERVICEFACTORY > xReturn    (                                                                                       \
                                                    cppu::createSingleFactory(  xServiceManager                                     ,               \
                                                                                IMPLCLASS::impl_getStaticImplementationName()       ,               \
                                                                                IMPLCLASS::impl_createInstance                  ,               \
                                                                                IMPLCLASS::impl_getStaticSupportedServiceNames()    )               \
                                                            );                                                                                      \
        return xReturn ;                                                                                                                            \
    }

//************************************************************************************************************************
//  definition of createFactory() for OneInstance-Services
//************************************************************************************************************************
#define SFX_IMPL_ONEINSTANCEFACTORY( IMPLCLASS )                                                                                                    \
                                                                                                                                                    \
    UNOREFERENCE< UNOXSINGLESERVICEFACTORY > IMPLCLASS::impl_createFactory( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager )        \
    {                                                                                                                                               \
        UNOREFERENCE< UNOXSINGLESERVICEFACTORY > xReturn(                                                                                           \
                                                    cppu::createOneInstanceFactory( xServiceManager                                     ,           \
                                                                                    IMPLCLASS::impl_getStaticImplementationName()       ,           \
                                                                                    IMPLCLASS::impl_createInstance                  ,           \
                                                                                    IMPLCLASS::impl_getStaticSupportedServiceNames()    )           \
                                                        );                                                                                          \
        return xReturn ;                                                                                                                            \
    }

//************************************************************************************************************************
//  definition for "extern c sfx_component_getFactory()"
//************************************************************************************************************************
#define CREATEFACTORY(CLASS)                                                                                            \
                                                                                                                        \
    /* Create right factory ... */                                                                                      \
    xFactory = CLASS::impl_createFactory( xServiceManager );

//************************************************************************************************************************
//  definition for "extern c sfx_component_getFactory()"
//************************************************************************************************************************
#define IF_NAME_CREATECOMPONENTFACTORY(CLASS)                                                                           \
                                                                                                                        \
    if ( CLASS::impl_getStaticImplementationName().equals( rtl::OUString::createFromAscii( pImplementationName ) ) )    \
    {                                                                                                                   \
        CREATEFACTORY ( CLASS )                                                                                     \
    }

#endif // _SFX_SFXUNO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
