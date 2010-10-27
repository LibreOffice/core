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
#ifndef _APPHELPER_SERVICEMACROS_HXX
#define _APPHELPER_SERVICEMACROS_HXX

/*
to use these macros the supported services and the implementation name needs to be static
especially you need to implement (declaration is contained in macro already):

static com::sun::star::uno::Sequence< rtl::OUString >
    Class::getSupportedServiceNames_Static();
*/

//=========================================================================
//
// XServiceInfo decl
//
//=========================================================================
namespace apphelper
{

#define APPHELPER_XSERVICEINFO_DECL()                                                   \
    virtual ::rtl::OUString SAL_CALL                                        \
        getImplementationName()                                             \
            throw( ::com::sun::star::uno::RuntimeException );               \
    virtual sal_Bool SAL_CALL                                               \
        supportsService( const ::rtl::OUString& ServiceName )               \
            throw( ::com::sun::star::uno::RuntimeException );               \
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL     \
        getSupportedServiceNames()                                          \
            throw( ::com::sun::star::uno::RuntimeException );               \
                                                                            \
    static ::rtl::OUString getImplementationName_Static();                  \
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >               \
        getSupportedServiceNames_Static();

//=========================================================================
//
// XServiceInfo impl
//
//=========================================================================

#define APPHELPER_XSERVICEINFO_IMPL( Class, ImplName )                              \
::rtl::OUString SAL_CALL Class::getImplementationName()                     \
    throw( ::com::sun::star::uno::RuntimeException )                        \
{                                                                           \
    return getImplementationName_Static();                                  \
}                                                                           \
                                                                            \
::rtl::OUString Class::getImplementationName_Static()                       \
{                                                                           \
    return ImplName;                                                        \
}                                                                           \
                                                                            \
sal_Bool SAL_CALL                                                           \
Class::supportsService( const ::rtl::OUString& ServiceName )                \
    throw( ::com::sun::star::uno::RuntimeException )                        \
{                                                                           \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSNL =               \
                                        getSupportedServiceNames();         \
    const ::rtl::OUString* pArray = aSNL.getArray();                        \
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )                       \
    {                                                                       \
        if( pArray[ i ] == ServiceName )                                    \
            return sal_True;                                                \
    }                                                                       \
                                                                            \
    return sal_False;                                                       \
}                                                                           \
                                                                            \
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL                 \
Class::getSupportedServiceNames()                                           \
    throw( ::com::sun::star::uno::RuntimeException )                        \
{                                                                           \
    return getSupportedServiceNames_Static();                               \
}

//=========================================================================
//
// Service factory helper decl+impl
//
//to use this macro you need to provide a constructor:
//class( Reference< XComponentContext > const & xContext )
//and implement OWeakObject
//=========================================================================

#define APPHELPER_SERVICE_FACTORY_HELPER(Class)                                     \
static ::com::sun::star::uno::Reference<                                    \
                            ::com::sun::star::uno::XInterface > SAL_CALL    \
    create( ::com::sun::star::uno::Reference<                               \
               ::com::sun::star::uno::XComponentContext > const & xContext) \
    throw(::com::sun::star::uno::Exception)                                 \
{                                                                           \
    return (::cppu::OWeakObject *)new Class( xContext );                    \
}

/** This macro contains the default implementation for getImplementationId().
    Note, that you have to include the header necessary for rtl_createUuid.
    Insert the following into your file:

    <code>
#include <rtl/uuid.h>
    </code>

    @param Class the Class-Name for which getImplementationId() should be
    implemented
 */
#define APPHELPER_GETIMPLEMENTATIONID_IMPL(Class) \
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL Class::getImplementationId() \
    throw (::com::sun::star::uno::RuntimeException) \
{ \
    static ::com::sun::star::uno::Sequence< sal_Int8 > aId; \
    if( aId.getLength() == 0 ) \
    { \
        aId.realloc( 16 ); \
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True ); \
    } \
    return aId; \
}

}//end namespace apphelper
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
