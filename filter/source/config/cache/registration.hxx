/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registration.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:24:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __COMPHELPER_REGISTRATION_HXX_
#define __COMPHELPER_REGISTRATION_HXX_

//_______________________________________________
// includes

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_INVALIDREGISTRYEXCEPTION_HPP_
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

//_______________________________________________
// namespace

namespace comphelper{

//_______________________________________________
// declaration

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_COMPONENT_GETIMPLEMENTATIONENVIRONMENT                                                          \
    extern "C" void SAL_CALL component_getImplementationEnvironment(const sal_Char**        ppEnvironmentTypeName,  \
                                                                          uno_Environment** /* ppEnvironment */ )   \
    {                                                                                                               \
        *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;                                                \
    }

//_______________________________________________

/** @short  creates a new key inside component registry.

    @descr  using: a) _COMPHELPER_COMPONENTINFO( MyClass,
                                                 MyClass::st_getImplName(),
                                                 MyClass::st_getServNames())

                   b) _COMPHELPER_COMPONENTINFO( MyClass,
                                                 ::rtl::OUString::createFromAscii("css.MyClass"),
                                                 lServiceNames)

    @param  CLASS
            must be the class name of the implementation
            of an uno service, which should be registered here.

    @param  IMPLEMENTATIONNAME
            can be an uno implementation name from type [::rtl::OUString]
            directly or any possible method call, which returns such
            name.

    @param  SERVICENAME
            a list of supported uno service names from type
            [css::uno::Sequence< ::rtl::OUString >]
            or any possible method call, which returns such
            list.
 */
#define _COMPHELPER_COMPONENTINFO(CLASS, IMPLEMENTATIONNAME, SERVICENAMES)                                          \
    /*define new scope to prevent multiple using of the same variables ... */                                       \
    {                                                                                                               \
        /* build new key name */                                                                                    \
        ::rtl::OUStringBuffer sKeyBuf(256);                                                                         \
        sKeyBuf.appendAscii("/"               );                                                                    \
        sKeyBuf.append     (IMPLEMENTATIONNAME);                                                                    \
        sKeyBuf.appendAscii("/UNO/SERVICES"   );                                                                    \
        ::rtl::OUString sKey = sKeyBuf.makeStringAndClear();                                                        \
                                                                                                                    \
        /* try to register this service ... thrown exception will be catched by COMPONENT_WRITEINFO! */             \
        css::uno::Reference< css::registry::XRegistryKey > xKey = xRoot->createKey(sKey);                           \
        if (!xKey.is())                                                                                             \
            throw css::registry::InvalidRegistryException(sKey, css::uno::Reference< css::uno::XInterface >());     \
                                                                                                                    \
        /* dont optimize it! it must work for simple types and function calls! */                                   \
        const css::uno::Sequence< ::rtl::OUString > lServiceNames = SERVICENAMES;                                   \
        const ::rtl::OUString*                      pServiceNames = lServiceNames.getConstArray();                  \
        sal_Int32                                   nCount        = lServiceNames.getLength();                      \
                                                                                                                    \
        for (sal_Int32 i=0; i<nCount; ++i)                                                                          \
            xKey->createKey(pServiceNames[i]);                                                                      \
    }

//_______________________________________________

/** @short  implments extern C function component_writeInfo

    @descr  using: _COMPHELPER_COMPONENT_WRITEINFO
                        (
                            _COMPHELPER_COMPONENTINFO(...)
                            ..
                            _COMPHELPER_COMPONENTINFO(...)
                        )

    @param  INFOLIST
            list of macros of type COMPONENTINFO without(!) any
            seperator signs between two elements.
 */
#define _COMPHELPER_COMPONENT_WRITEINFO(INFOLIST)                                                                                   \
    extern "C" sal_Bool SAL_CALL component_writeInfo(void* pServiceManager,                                                         \
                                                     void* pRegistryKey   )                                                         \
    {                                                                                                                               \
        if (!pServiceManager || !pRegistryKey)                                                                                      \
            return sal_False;                                                                                                       \
                                                                                                                                    \
        css::uno::Reference< css::registry::XRegistryKey > xRoot = reinterpret_cast< css::registry::XRegistryKey* >(pRegistryKey);  \
                                                                                                                                    \
        /*if one of following registration will fail ... an exception is thrown! */                                                 \
        try                                                                                                                         \
        {                                                                                                                           \
            /* This parameter will expand to: */                                                                                    \
            /*   _COMPHELPER_COMPONENTINFO(1) */                                                                                    \
            /*   ...                          */                                                                                    \
            /*   _COMPHELPER_COMPONENTINFO(n) */                                                                                    \
            INFOLIST                                                                                                                \
        }                                                                                                                           \
        catch(const css::registry::InvalidRegistryException&)                                                                       \
        {                                                                                                                           \
            return sal_False;                                                                                                       \
        }                                                                                                                           \
        return sal_True;                                                                                                            \
    }

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_MULTIINSTANCEFACTORY(IMPLEMENTATIONNAME, SERVICENAMES, FACTORYMETHOD)   \
    if (IMPLEMENTATIONNAME == sImplName)                                                    \
        xFactory = ::cppu::createSingleFactory(xSMGR             ,                          \
                                               IMPLEMENTATIONNAME,                          \
                                               FACTORYMETHOD     ,                          \
                                               SERVICENAMES      );

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_ONEINSTANCEFACTORY(IMPLEMENTATIONNAME, SERVICENAMES, FACTORYMETHOD)     \
    if (IMPLEMENTATIONNAME == sImplName)                                                    \
        xFactory = ::cppu::createOneInstanceFactory(xSMGR             ,                     \
                                                    IMPLEMENTATIONNAME,                     \
                                                    FACTORYMETHOD     ,                     \
                                                    SERVICENAMES      );

//_______________________________________________

/** TODO    doc
 */
#define _COMPHELPER_COMPONENT_GETFACTORY(FACTORYLIST)                                                                                               \
    extern "C" void* SAL_CALL component_getFactory(const sal_Char* pImplementationName,                                                             \
                                                         void*     pServiceManager    ,                                                             \
                                                         void*     /* pRegistryKey */ )                                                             \
    {                                                                                                                                               \
        if (                                                                                                                                        \
            (!pImplementationName) ||                                                                                                               \
            (!pServiceManager    )                                                                                                                  \
           )                                                                                                                                        \
        return NULL;                                                                                                                                \
                                                                                                                                                    \
        css::uno::Reference< css::lang::XMultiServiceFactory >  xSMGR     = reinterpret_cast< css::lang::XMultiServiceFactory* >(pServiceManager);  \
        css::uno::Reference< css::lang::XSingleServiceFactory > xFactory  ;                                                                         \
        rtl::OUString                                           sImplName = ::rtl::OUString::createFromAscii(pImplementationName);                  \
                                                                                                                                                    \
        /* This parameter will expand to: */                                                                                                        \
        /*  _COMPHELPER_xxxFACTORY(1)     */                                                                                                        \
        /*  else                          */                                                                                                        \
        /*  ...                           */                                                                                                        \
        /*  else                          */                                                                                                        \
        /*  _COMPHELPER_xxxFACTORY(n)     */                                                                                                        \
        FACTORYLIST                                                                                                                                 \
                                                                                                                                                    \
        /* And if one of these checks was successfully => xFactory was set! */                                                                      \
        if (xFactory.is())                                                                                                                          \
        {                                                                                                                                           \
            xFactory->acquire();                                                                                                                    \
            return xFactory.get();                                                                                                                  \
        }                                                                                                                                           \
                                                                                                                                                    \
        return NULL;                                                                                                                                \
    }

} // namespace comphelper

#endif  //  #ifndef __COMPHELPER_REGISTRATION_HXX_
