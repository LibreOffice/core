#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include "MyProtocolHandler.h"
#include "MyListener.h"

namespace css = ::com::sun::star;

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

extern "C"
{
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(const sal_Char**        ppEnvTypeName,
                                                                                 uno_Environment** ppEnv        )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

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
