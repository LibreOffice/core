/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_
#define __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>

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
