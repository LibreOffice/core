/*************************************************************************
 *
 *  $RCSfile: xserviceinfo.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:23 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#define __FRAMEWORK_MACROS_XSERVICEINFO_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_UNO_TYPE_H_
#include <com/sun/star/uno/Type.h>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

/*_________________________________________________________________________________________________________________

    macros for declaration and definition of XServiceInfo
    Please use follow public macros only!

    1)  DECLARE_XSERVICEINFO                                                                    => use it to declare XServiceInfo in your header
    2)  DEFINE_XSERVICEINFO_MULTISERVICE( CLASS, SERVICENAME, IMPLEMENTATIONNAME )              => use it to define XServiceInfo for multi service mode
    3)  DEFINE_XSERVICEINFO_ONEINSTANCESERVICE( CLASS, SERVICENAME, IMPLEMENTATIONNAME )        => use it to define XServiceInfo for one instance service mode

_________________________________________________________________________________________________________________*/

//*****************************************************************************************************************
//  private
//  implementation of XServiceInfo and helper functions
//*****************************************************************************************************************
#define PRIVATE_DEFINE_XSERVICEINFO( CLASS, SERVICENAME, IMPLEMENTATIONNAME )                                                                       \
    /*===========================================================================================================*/                                 \
    /* XServiceInfo                                                                                              */                                 \
    /*===========================================================================================================*/                                 \
    ::rtl::OUString SAL_CALL CLASS::getImplementationName() throw( ::com::sun::star::uno::RuntimeException )                                        \
    {                                                                                                                                               \
        return impl_getStaticImplementationName();                                                                                                  \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* XServiceInfo                                                                                              */                                 \
    /*===========================================================================================================*/                                 \
    sal_Bool SAL_CALL CLASS::supportsService( const ::rtl::OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException )                \
    {                                                                                                                                               \
        /* Set default return value. */                                                                                                             \
        sal_Bool bReturn = sal_False ;                                                                                                              \
        /* Get names of all supported servicenames. */                                                                                              \
        ::com::sun::star::uno::Sequence< ::rtl::OUString >  seqServiceNames =   getSupportedServiceNames();                                         \
        const ::rtl::OUString*                              pArray          =   seqServiceNames.getConstArray();                                    \
        sal_Int32                                           nCounter        =   0;                                                                  \
        sal_Int32                                           nLength         =   seqServiceNames.getLength();                                        \
        /* Search for right name in list. */                                                                                                        \
        while   (                                                                                                                                   \
                    ( nCounter  <   nLength     )   &&                                                                                              \
                    ( bReturn   ==  sal_False   )                                                                                                   \
                )                                                                                                                                   \
        {                                                                                                                                           \
            /* Is name was found, say "YES, SERVICE IS SUPPORTED." and break loop. */                                                               \
            if ( pArray[nCounter] == sServiceName )                                                                                                 \
            {                                                                                                                                       \
                bReturn = sal_True ;                                                                                                                \
            }                                                                                                                                       \
            /* Else step to next element in list. */                                                                                                \
            ++nCounter;                                                                                                                             \
        }                                                                                                                                           \
        /* Return state of search. */                                                                                                               \
        return bReturn;                                                                                                                             \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* XServiceInfo                                                                                              */                                 \
    /*===========================================================================================================*/                                 \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL CLASS::getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException )  \
    {                                                                                                                                               \
        return impl_getStaticSupportedServiceNames();                                                                                               \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* Helper for XServiceInfo */                                                                                                                   \
    /*===========================================================================================================*/                                 \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > CLASS::impl_getStaticSupportedServiceNames()                                                 \
    {                                                                                                                                               \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seqServiceNames( 1 );                                                                    \
        seqServiceNames.getArray() [0] = SERVICENAME ;                                                                                              \
        return seqServiceNames;                                                                                                                     \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* Helper for XServiceInfo                                                                                   */                                 \
    /*===========================================================================================================*/                                 \
    ::rtl::OUString CLASS::impl_getStaticImplementationName()                                                                                       \
    {                                                                                                                                               \
        return IMPLEMENTATIONNAME ;                                                                                                                 \
    }                                                                                                                                               \
                                                                                                                                                    \
    /*===========================================================================================================*/                                 \
    /* Helper for registry                                                                                       */                                 \
    /*===========================================================================================================*/                                 \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL CLASS::impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( ::com::sun::star::uno::Exception )  \
    {                                                                                                                                                                                                                                                               \
        return ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >( *new CLASS( xServiceManager ) );                                                                                                                                              \
    }

//*****************************************************************************************************************
//  private
//  definition of helper function createFactory() for multiple services
//*****************************************************************************************************************
#define PRIVATE_DEFINE_SINGLEFACTORY( CLASS )                                                                                                                                                                               \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > CLASS::impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager )  \
    {                                                                                                                                                                                                                       \
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xReturn   ( cppu::createSingleFactory (   xServiceManager                             ,                                           \
                                                                                                                                    CLASS::impl_getStaticImplementationName()   ,                                           \
                                                                                                                                    CLASS::impl_createInstance                  ,                                           \
                                                                                                                                    CLASS::impl_getStaticSupportedServiceNames()                                            \
                                                                                                                                )                                                                                           \
                                                                                                    );                                                                                                                      \
        return xReturn;                                                                                                                                                                                                     \
    }

//*****************************************************************************************************************
//  private
//  definition of helper function createFactory() for one instance services
//*****************************************************************************************************************
#define PRIVATE_DEFINE_ONEINSTANCEFACTORY( CLASS )                                                                                                                                                                          \
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > CLASS::impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager )  \
    {                                                                                                                                                                                                                       \
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xReturn   ( cppu::createOneInstanceFactory    (   xServiceManager                             ,                                   \
                                                                                                                                            CLASS::impl_getStaticImplementationName()   ,                                   \
                                                                                                                                            CLASS::impl_createInstance                  ,                                   \
                                                                                                                                            CLASS::impl_getStaticSupportedServiceNames()                                    \
                                                                                                                                        )                                                                                   \
                                                                                                    );                                                                                                                      \
        return xReturn;                                                                                                                                                                                                     \
    }

//*****************************************************************************************************************
//  public
//  declaration of XServiceInfo and helper functions
//*****************************************************************************************************************
#define DECLARE_XSERVICEINFO                                                                                                                                                                                                                                        \
    /* XServiceInfo */                                                                                                                                                                                                                                              \
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );                                                                                                                                                      \
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException );                                                                                                                              \
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );                                                                                                                \
    /* Helper for XServiceInfo */                                                                                                                                                                                                                                   \
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();                                                                                                                                                                \
    static ::rtl::OUString impl_getStaticImplementationName();                                                                                                                                                                                                      \
    /* Helper for registry */                                                                                                                                                                                                                                       \
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( ::com::sun::star::uno::Exception ); \
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

//*****************************************************************************************************************
//  public
//  implementation of XServiceInfo
//*****************************************************************************************************************
#define DEFINE_XSERVICEINFO_MULTISERVICE( CLASS, SERVICENAME, IMPLEMENTATIONNAME )                              \
    PRIVATE_DEFINE_XSERVICEINFO( CLASS, SERVICENAME, IMPLEMENTATIONNAME )                                       \
    PRIVATE_DEFINE_SINGLEFACTORY( CLASS )

#define DEFINE_XSERVICEINFO_ONEINSTANCESERVICE( CLASS, SERVICENAME, IMPLEMENTATIONNAME )                        \
    PRIVATE_DEFINE_XSERVICEINFO( CLASS, SERVICENAME, IMPLEMENTATIONNAME )                                       \
    PRIVATE_DEFINE_ONEINSTANCEFACTORY( CLASS )

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
