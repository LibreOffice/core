/*************************************************************************
 *
 *  $RCSfile: factory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-12 16:52:03 $
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
#ifndef _CPPUHELPER_FACTORY_HXX_
#define _CPPUHELPER_FACTORY_HXX_

#ifndef _RTL_STRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

//##################################################################################################

#define COMPONENT_GETENV            "component_getImplementationEnvironment"
#define COMPONENT_GETDESCRIPTION    "component_getDescription"
#define COMPONENT_WRITEINFO         "component_writeInfo"
#define COMPONENT_GETFACTORY        "component_getFactory"

typedef struct _uno_Environment uno_Environment;

/***************************************************************************************************
 * Function to determine the environment of the implementation.
 * If the environment is NOT session specific (needs no additional context), then this function
 * should return the environment type name and leave ppEnv (0).
 *<BR>
 * @param       ppEnvTypeName   environment type name; string must be constant
 * @param       ppEnv           function returns its environment if the environment is
 *                              session specific, i.e. has special context
 */
typedef void (SAL_CALL * component_getImplementationEnvironmentFunc)(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv );

/***************************************************************************************************
 * Optional function to retrieve a component description.
 *<BR>
 * @return                      an XML formatted string containing a short component description
 */
typedef const sal_Char * (SAL_CALL * component_getDescriptionFunc)(void);

/***************************************************************************************************
 * Writes component registry info, at least writing the supported service names.
 *<BR>
 * @param       pServiceManager a service manager
 *                              (the type is XMultiServiceFactory to be used by the environment
 *                              returned by component_getImplementationEnvironment)
 * @param       pRegistryKey    a registry key
 *                              (the type is XRegistryKey to be used by the environment
 *                              returned by component_getImplementationEnvironment)
 * @return                      true if everything went fine
 */
typedef sal_Bool (SAL_CALL * component_writeInfoFunc)(
    void * pServiceManager, void * pRegistryKey );

/***************************************************************************************************
 * Retrieves a factory to create component instances.
 *<BR>
 * @param       pImplName       desired implementation name
 * @param       pServiceManager a service manager
 *                              (the type is XMultiServiceFactory to be used by the environment
 *                              returned by component_getImplementationEnvironment)
 * @param       pRegistryKey    a registry key
 *                              (the type is XRegistryKey to be used by the environment
 *                              returned by component_getImplementationEnvironment)
 * @return                      acquired component factory
 *                              (the type is XSingleServiceFactory to be used by the environment
 *                              returned by component_getImplementationEnvironment)
 */
typedef void * (SAL_CALL * component_getFactoryFunc)(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey );

//##################################################################################################

#define WRITE_COMPONENT_INFO_FUNCTION       "writeComponentInfo"
#define CREATE_COMPONENT_FACTORY_FUNCTION   "createComponentFactory"

/** This function pointer describes a function to write needed administrativ information
 *  about a component into the registry.
 */
typedef sal_Bool (SAL_CALL * WriteComponentInfoFunc)( uno_Interface * pXKey );

/** This function pointer describes a function to create a factory for one or more components.
 */
typedef uno_Interface* (SAL_CALL * CreateComponentFactoryFunc)(
    const sal_Unicode *, uno_Interface * pXSMgr, uno_Interface * pXKey );


/** */ //for docpp
namespace cppu
{

/**
 * The type of the instanciate function used as argument of the create*Fcatory functions.
 * @see createSingleFactory
 * @see createOneInstanceFactory
 */
typedef ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(SAL_CALL * ComponentInstantiation)(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rServiceManager );

/**
 * Create a single service factory.<BR>
 * Note: The function pointer ComponentInstantiation points to a function throws Exception.
 *
 * @param rServiceManager       the service manager used by the implementation.
 * @param rImplementationName   the implementation name. An empty string is possible.
 * @param ComponentInstantiation the function pointer to create an object.
 * @param rServiceNames         the service supported by the implementation.
 * @return a factory that support the interfaces XServiceProvider, XServiceInfo
 *          XSingleServiceFactory and XComponent.
 *
 * @see createOneInstanceFactory
 */
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
createSingleFactory(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rServiceManager,
    const ::rtl::OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rServiceNames )
    SAL_THROW( () );

/**
 * Create a factory, that wrappes another one.<BR>
 * This means the methods of the interfaces XServiceProvider, XServiceInfo and
 * XSingleServiceFactory are forwarded.
 * <B>It is not possible to put a factory into two service managers!<BR>
 * The XComponent interface is not supported!</B>
 *
 * @param rServiceManager       the service manager used by the implementation.
 * @param xSingleServiceFactory the wrapped service factory.
 * @return a factory that support the interfaces XServiceProvider, XServiceInfo
 *          XSingleServiceFactory.
 *
 * @see createSingleFactory
 */
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
createFactoryProxy(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rServiceManager,
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > & rFactory )
    SAL_THROW( () );

/**
 * Create a single service factory which hold the instance created. If the
 * "XSingleServiceFactory::createFactoryWithArguments" method is called with arguments
 * new objects are created.
 * Note: The function pointer ComponentInstantiation points to a function throws Exception.
 *
 * @param rServiceManager       the service manager used by the implementation.
 * @param rImplementationName   the implementation name. An empty string is possible.
 * @param ComponentInstantiation the function pointer to create an object.
 * @param rServiceNames         the service supported by the implementation.
 * @return a factory that support the interfaces XServiceProvider, XServiceInfo
 *          XSingleServiceFactory and XComponent.
 *
 * @see createSingleFactory
 */
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
createOneInstanceFactory(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rServiceManager,
    const ::rtl::OUString & rComponentName,
    ComponentInstantiation pCreateFunction,
    const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rServiceNames )
    SAL_THROW( () );

/**
 * Create a single service factory.<BR>
 * Note: The function pointer ComponentInstantiation points to a function throws Exception.
 *
 * @param rServiceManager       the service manager used by the implementation.
 * @param rImplementationName   the implementation name. An empty string is possible.
 * @param ComponentInstantiation the function pointer to create an object.
 * @param rImplementationKey    the registry key of the implementation section.
 * @return a factory that support the interfaces XServiceProvider, XServiceInfo
 *          XSingleServiceFactory and XComponent.
 */
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL createSingleRegistryFactory(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rServiceManager,
    const ::rtl::OUString & rImplementationName,
    const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > & rImplementationKey )
    SAL_THROW( () );

/**
 * Create a single service factory which hold the instance created. If the
 * "XSingleServiceFactory::createFactoryWithArguments" method is called with arguments
 * new objects are created.
 * Note: The function pointer ComponentInstantiation points to a function throws Exception.
 *
 * @param rServiceManager       the service manager used by the implementation.
 * @param rImplementationName   the implementation name. An empty string is possible.
 * @param ComponentInstantiation the function pointer to create an object.
 * @param rImplementationKey    the registry key of the implementation section.
 * @return a factory that support the interfaces XServiceProvider, XServiceInfo
 *          XSingleServiceFactory and XComponent.
 *
 * @see createSingleRegistryFactory
 */
::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL createOneInstanceRegistryFactory(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rServiceManager,
    const ::rtl::OUString & rComponentName,
    const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > & rImplementationKey )
    SAL_THROW( () );

}

#endif

