/*************************************************************************
 *
 *  $RCSfile: ServiceMacros.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
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
