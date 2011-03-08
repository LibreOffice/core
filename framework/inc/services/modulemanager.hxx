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

#ifndef __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_
#define __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_

//_______________________________________________
// own includes

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <general.h>
#include <stdtypes.h>

//_______________________________________________
// interface includes
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>

//_______________________________________________
// other includes
#include <cppuhelper/weak.hxx>

//_______________________________________________
// definition

namespace framework
{

//_______________________________________________
/**
    implements the service com.sun.star.frame.ModuleManager
 */
class ModuleManager : public  css::lang::XTypeProvider
                    , public  css::lang::XServiceInfo
                    , public  css::frame::XModuleManager
                    , public  css::container::XNameReplace // => XNameAccess, XElementAccess
                    , public  css::container::XContainerQuery
                    // attention! Must be the first base class to guarentee right initialize lock ...
                    , private ThreadHelpBase
                    , public  ::cppu::OWeakObject
{
    //___________________________________________
    // member

    private:

        //---------------------------------------
        /** the global uno service manager.
            Must be used to create own needed services.
         */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------
        /** points to the underlying configuration.
            This ModuleManager does not cache - it calls directly the
            configuration API!
          */
        css::uno::Reference< css::container::XNameAccess > m_xCFG;

    //___________________________________________
    // interface

    public:

                 ModuleManager(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~ModuleManager(                                                                   );

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XModuleManager
        virtual ::rtl::OUString SAL_CALL identify(const css::uno::Reference< css::uno::XInterface >& xModule)
            throw(css::lang::IllegalArgumentException,
                  css::frame::UnknownModuleException,
                  css::uno::RuntimeException         );

        // XNameReplace
        virtual void SAL_CALL replaceByName(const ::rtl::OUString& sName ,
                                            const css::uno::Any&   aValue)
            throw (css::lang::IllegalArgumentException   ,
                   css::container::NoSuchElementException,
                   css::lang::WrappedTargetException     ,
                   css::uno::RuntimeException            );

        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName(const ::rtl::OUString& sName)
            throw(css::container::NoSuchElementException,
                  css::lang::WrappedTargetException     ,
                  css::uno::RuntimeException            );

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
            throw(css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasByName(const ::rtl::OUString& sName)
            throw(css::uno::RuntimeException);

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType()
            throw(css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasElements()
            throw(css::uno::RuntimeException);

        // XContainerQuery
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByQuery(const ::rtl::OUString& sQuery)
            throw(css::uno::RuntimeException);

        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByProperties(const css::uno::Sequence< css::beans::NamedValue >& lProperties)
            throw(css::uno::RuntimeException);
    //___________________________________________
    // helper

    private:

        //---------------------------------------
        /** @short  open the underlying configuration.

            @descr  This method must be called everytimes
                    a (reaonly!) configuration is needed. Because
                    method works together with the member
                    m_xCFG, open it on demand and cache it
                    afterwards.

                    Note: A writable configuration access
                    must be created explicitly. Otherwise
                    we cant make sure that broken write requests
                    wont affect our read access !

            @return [com.sun.star.container.XNameAccess]
                    the configuration object

            @throw  [com.sun.star.uno.RuntimeException]
                    if config could not be opened successfully!

            @threadsafe
          */
        css::uno::Reference< css::container::XNameAccess > implts_getConfig()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        /** @short  makes the real identification of the module.

            @descr  It checks for the optional but preferred interface
                    XModule first. If this module does not exists at the
                    given component it tries to use XServiceInfo instead.

                    Note: This method try to locate a suitable module name.
                    Nothing else. Selecting the right component and throwing suitable
                    exceptions must be done outside.

            @see    identify()

            @param  xComponent
                    the module for identification.

            @return The identifier of the given module.
                    Can be empty if given component is not a real module !

            @threadsafe
         */
        ::rtl::OUString implts_identify(const css::uno::Reference< css::uno::XInterface >& xComponent);
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
