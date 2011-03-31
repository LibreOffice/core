/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * A LibreOffice extension to send the menubar structure through DBusMenu
 *
 * Copyright 2011 Canonical, Ltd.
 * Authors:
 *     Alberto Ruiz <alberto.ruiz@codethink.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the the GNU Lesser General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR PURPOSE.  See the applicable
 * version of the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with this program. If not, see <http://www.gnu.org/licenses/>
 *
 */

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

#include "DesktopJob.hxx"
#include "FrameJob.hxx"

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
                                                                                 uno_Environment** /*ppEnv*/        )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


//==================================================================================================
SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(void* /*pServiceManager*/,
                                                            void* pRegistryKey   )
{
    if (!pRegistryKey)
        return sal_False;

    try
    {
        css::uno::Reference< css::registry::XRegistryKey > xKey(reinterpret_cast< css::registry::XRegistryKey* >(pRegistryKey), css::uno::UNO_QUERY);

        writeInfo( xKey, DESKTOPJOB_IMPLEMENTATION_NAME, DESKTOPJOB_SERVICE_NAME);
        writeInfo( xKey, FRAMEJOB_IMPLEMENTATION_NAME, FRAMEJOB_SERVICE_NAME);

        return sal_True;
    }
    catch(const css::registry::InvalidRegistryException&)
        { OSL_ENSURE( sal_False, "### InvalidRegistryException!" ); }

    return sal_False;
}

//==================================================================================================
SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(const sal_Char* pImplName      ,
                                                                void*     pServiceManager,
                                                                void*     /*pRegistryKey*/  )
{
    if ( !pServiceManager || !pImplName )
        return 0;

    css::uno::Reference< css::lang::XSingleServiceFactory > xFactory  ;
    css::uno::Reference< css::lang::XMultiServiceFactory >  xSMGR     (reinterpret_cast< css::lang::XMultiServiceFactory* >(pServiceManager), css::uno::UNO_QUERY);
    ::rtl::OUString                                         sImplName = ::rtl::OUString::createFromAscii(pImplName);

    if (sImplName.equalsAscii(DESKTOPJOB_IMPLEMENTATION_NAME))
    {
        css::uno::Sequence< ::rtl::OUString > lNames(1);
        lNames[0] = ::rtl::OUString::createFromAscii(DESKTOPJOB_IMPLEMENTATION_NAME);
        xFactory = ::cppu::createSingleFactory(xSMGR, sImplName,  DesktopJob_createInstance, lNames);
    }
    if (sImplName.equalsAscii(FRAMEJOB_IMPLEMENTATION_NAME))
    {
        css::uno::Sequence< ::rtl::OUString > lNames(1);
        lNames[0] = ::rtl::OUString::createFromAscii(FRAMEJOB_IMPLEMENTATION_NAME);
        xFactory = ::cppu::createSingleFactory(xSMGR, sImplName,  FrameJob_createInstance, lNames);
    }


    if (!xFactory.is())
        return 0;

    xFactory->acquire();
    return xFactory.get();
}

} // extern C
