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

#ifndef __FRAMEWORK_PATTERN_CONFIGURATION_HXX_
#define __FRAMEWORK_PATTERN_CONFIGURATION_HXX_

#include <services.h>
#include <general.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <rtl/ustrbuf.hxx>

namespace framework{
    namespace pattern{
        namespace configuration{

//-----------------------------------------------
class ConfigurationHelper
{

    public:

        //---------------------------------------
        /** @short  allow opening of a configuration access
                    in different working modes.

            @descr  All enum values must be useable as flags
                    mapped into a int32 value!
         */
        enum EOpenMode
        {
            /// open it readonly (default=readwrite!)
            E_READONLY = 1,
            /// disable fallback handling for localized cfg nodes
            E_ALL_LOCALES = 2
        };

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /**
            @short  opens a configuration access.

            @descr  TODO

            @param  rxContext
                    this method need an uno service manager for internal work.

            @param  sPackage
                    name the configuration file.
                    e.g. "/.org.openoffice.Setup"
                    Note: It must start with "/" but end without(!) "/"!

            @param  sRelPath
                    describe the relativ path of the requested key inside
                    the specified package.
                    e.g. "Office/Factories"
                    Note: Its not allowed to start or end with a "/"!
                    Further you must use encoded path elements if
                    e.g. set nodes are involved.

            @param  nOpenFlags
                    force opening of the configuration access in special mode.
                    see enum EOpenMode for further information.
         */
        static css::uno::Reference< css::uno::XInterface > openConfig(const css::uno::Reference< css::uno::XComponentContext >&     rxContext,
                                                                      const OUString&                                        sPackage  ,
                                                                      const OUString&                                        sRelPath  ,
                                                                            sal_Int32                                               nOpenFlags)
        {
            css::uno::Reference< css::uno::XInterface > xCFG;

            try
            {
                css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider =
                    css::configuration::theDefaultProvider::get( rxContext );

                OUStringBuffer sPath(1024);
                sPath.append(sPackage      );
                sPath.append(static_cast<sal_Unicode>('/'));
                sPath.append(sRelPath      );

                sal_Bool bReadOnly   = ((nOpenFlags & ConfigurationHelper::E_READONLY   ) == ConfigurationHelper::E_READONLY   );
                sal_Bool bAllLocales = ((nOpenFlags & ConfigurationHelper::E_ALL_LOCALES) == ConfigurationHelper::E_ALL_LOCALES);

                sal_Int32 c = 1;
                if (bAllLocales)
                    c = 2;

                css::uno::Sequence< css::uno::Any > lParams(c);
                css::beans::PropertyValue           aParam;

                aParam.Name    = OUString("nodepath");
                aParam.Value <<= sPath.makeStringAndClear();
                lParams[0]   <<= aParam;

                if (bAllLocales)
                {
                    aParam.Name    = OUString("*");
                    aParam.Value <<= sal_True;
                    lParams[1]   <<= aParam;
                }

                if (bReadOnly)
                    xCFG = xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGREADACCESS, lParams);
                else
                    xCFG = xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGUPDATEACCESS, lParams);
            }
            catch(const css::uno::RuntimeException& exRun)
                { throw exRun; }
            catch(const css::uno::Exception&)
                { xCFG.clear(); }

            return xCFG;
        }
};

        } // namespace configuration
    } // namespace pattern
} // namespace framework

#endif // __FRAMEWORK_PATTERN_CONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
