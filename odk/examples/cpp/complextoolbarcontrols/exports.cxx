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


#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "MyProtocolHandler.h"
#include "MyListener.h"

namespace css = ::com::sun::star;

// static void writeInfo(const css::uno::Reference< css::registry::XRegistryKey >& xRegistryKey       ,
//                       const char*                                               pImplementationName,
//                       const char*                                               pServiceName       )
// {
//     ::rtl::OUStringBuffer sKey(256);
//  sKey.append     (::rtl::OUString::createFromAscii(pImplementationName));
//     sKey.appendAscii("/UNO/SERVICES/");
//     sKey.append     (::rtl::OUString::createFromAscii(pServiceName));

//     xRegistryKey->createKey(sKey.makeStringAndClear());
// }

extern "C"
{
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(const sal_Char**        ppEnvTypeName,
                                                                                 uno_Environment** ppEnv        )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// This method not longer necessary since OOo 3.4 where the component registration was
// was changed to passive component registration. For more details see
// http://wiki.services.openoffice.org/wiki/Passive_Component_Registration
//==================================================================================================
// SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(void* pServiceManager,
//                                                             void* pRegistryKey   )
// {
//     if (!pRegistryKey)
//         return sal_False;

//     try
//     {
//         css::uno::Reference< css::registry::XRegistryKey > xKey(reinterpret_cast< css::registry::XRegistryKey* >(pRegistryKey), css::uno::UNO_QUERY);

//         writeInfo( xKey, MYLISTENER_IMPLEMENTATIONNAME       , MYLISTENER_SERVICENAME        );
//         writeInfo( xKey, MYPROTOCOLHANDLER_IMPLEMENTATIONNAME, MYPROTOCOLHANDLER_SERVICENAME );

//         return sal_True;
//     }
//     catch(const css::registry::InvalidRegistryException&)
//         { OSL_ENSURE( sal_False, "### InvalidRegistryException!" ); }

//     return sal_False;
// }

//==================================================================================================
SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(const sal_Char* pImplName      ,
                                                                void*     pServiceManager,
                                                                void*     pRegistryKey   )
{
    if ( !pServiceManager || !pImplName )
        return 0;

    css::uno::Reference< css::lang::XSingleServiceFactory > xFactory  ;
    css::uno::Reference< css::lang::XMultiServiceFactory >  xSMGR     (reinterpret_cast< css::lang::XMultiServiceFactory* >(pServiceManager), css::uno::UNO_QUERY);
    ::rtl::OUString                                         sImplName = ::rtl::OUString::createFromAscii(pImplName);

    if (sImplName.equalsAscii(MYLISTENER_IMPLEMENTATIONNAME))
    {
        css::uno::Sequence< ::rtl::OUString > lNames(1);
        lNames[0] = ::rtl::OUString::createFromAscii(MYLISTENER_IMPLEMENTATIONNAME);
        xFactory = ::cppu::createSingleFactory(xSMGR, sImplName, MyListener::st_createInstance, lNames);
    }
    else
    if (sImplName.equalsAscii(MYPROTOCOLHANDLER_IMPLEMENTATIONNAME))
    {
        css::uno::Sequence< ::rtl::OUString > lNames(1);
        lNames[0] = ::rtl::OUString::createFromAscii(MYPROTOCOLHANDLER_SERVICENAME);
        xFactory = ::cppu::createSingleFactory(xSMGR, sImplName, MyProtocolHandler_createInstance, lNames);
    }

    if (!xFactory.is())
        return 0;

    xFactory->acquire();
    return xFactory.get();
}

} // extern C
