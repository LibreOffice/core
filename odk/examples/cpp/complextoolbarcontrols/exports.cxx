/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <cppuhelper/factory.hxx>

#include "MyProtocolHandler.h"
#include "MyListener.h"

namespace css = ::com::sun::star;

#if 0
static void writeInfo(const css::uno::Reference< css::registry::XRegistryKey >& xRegistryKey       ,
                      const char*                                               pImplementationName,
                      const char*                                               pServiceName       )
{
    ::rtl::OUStringBuffer sKey(256);
    sKey.append     (::rtl::OUString::createFromAscii(pImplementationName));
    sKey.appendAscii("/UNO/SERVICES/");
    sKey.append     (::rtl::OUString::createFromAscii(pServiceName));

    xRegistryKey->createKey(sKey.makeStringAndClear());
}
#endif

extern "C"
{
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(const sal_Char**        ppEnvTypeName,
                                                                                 uno_Environment** ppEnv        )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

#if 0
/**
 * This method not longer necessary since OOo 3.4 where the component registration was
 * was changed to passive component registration. For more details see
 * http://wiki.services.openoffice.org/wiki/Passive_Component_Registration
 */
//==================================================================================================
SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(void* pServiceManager,
                                                            void* pRegistryKey   )
{
    if (!pRegistryKey)
        return sal_False;

    try
    {
        css::uno::Reference< css::registry::XRegistryKey > xKey(reinterpret_cast< css::registry::XRegistryKey* >(pRegistryKey), css::uno::UNO_QUERY);

        writeInfo( xKey, MYLISTENER_IMPLEMENTATIONNAME       , MYLISTENER_SERVICENAME        );
        writeInfo( xKey, MYPROTOCOLHANDLER_IMPLEMENTATIONNAME, MYPROTOCOLHANDLER_SERVICENAME );

        return sal_True;
    }
    catch(const css::registry::InvalidRegistryException&)
        { OSL_ENSURE( sal_False, "### InvalidRegistryException!" ); }

    return sal_False;
}
#endif

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

    if (sImplName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MYLISTENER_IMPLEMENTATIONNAME ) ) )
    {
        css::uno::Sequence< ::rtl::OUString > lNames(1);
        lNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MYLISTENER_IMPLEMENTATIONNAME ) );
        xFactory = ::cppu::createSingleFactory(xSMGR, sImplName, MyListener::st_createInstance, lNames);
    }
    else
    if (sImplName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MYPROTOCOLHANDLER_IMPLEMENTATIONNAME ) ) )
    {
        css::uno::Sequence< ::rtl::OUString > lNames(1);
        lNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MYPROTOCOLHANDLER_SERVICENAME ) );
        xFactory = ::cppu::createSingleFactory(xSMGR, sImplName, MyProtocolHandler_createInstance, lNames);
    }

    if (!xFactory.is())
        return 0;

    xFactory->acquire();
    return xFactory.get();
}

} // extern C
