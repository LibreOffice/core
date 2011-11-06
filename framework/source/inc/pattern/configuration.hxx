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



#ifndef __FRAMEWORK_PATTERN_CONFIGURATION_HXX_
#define __FRAMEWORK_PATTERN_CONFIGURATION_HXX_

//_______________________________________________
// own includes

#include <services.h>
#include <general.h>

//_______________________________________________
// interface includes
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HXX_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_______________________________________________
// other includes
#include <rtl/ustrbuf.hxx>

//_______________________________________________
// namespaces

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework{
    namespace pattern{
        namespace configuration{

//_______________________________________________
// definitions

//-----------------------------------------------
class ConfigurationHelper
{
    //-------------------------------------------
    // const
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

            @param  xSMGR
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
                    see enum EOpenMode for further informations.
         */
        static css::uno::Reference< css::uno::XInterface > openConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR     ,
                                                                      const ::rtl::OUString&                                        sPackage  ,
                                                                      const ::rtl::OUString&                                        sRelPath  ,
                                                                            sal_Int32                                               nOpenFlags)
        {
            static ::rtl::OUString PATH_SEPERATOR = ::rtl::OUString::createFromAscii("/");

            css::uno::Reference< css::uno::XInterface > xCFG;

            try
            {
                css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
                    xSMGR->createInstance(SERVICENAME_CFGPROVIDER), css::uno::UNO_QUERY_THROW);

                ::rtl::OUStringBuffer sPath(1024);
                sPath.append(sPackage      );
                sPath.append(PATH_SEPERATOR);
                sPath.append(sRelPath      );

                sal_Bool bReadOnly   = ((nOpenFlags & ConfigurationHelper::E_READONLY   ) == ConfigurationHelper::E_READONLY   );
                sal_Bool bAllLocales = ((nOpenFlags & ConfigurationHelper::E_ALL_LOCALES) == ConfigurationHelper::E_ALL_LOCALES);

                sal_Int32 c = 1;
                if (bAllLocales)
                    c = 2;

                css::uno::Sequence< css::uno::Any > lParams(c);
                css::beans::PropertyValue           aParam;

                aParam.Name    = ::rtl::OUString::createFromAscii("nodepath");
                aParam.Value <<= sPath.makeStringAndClear();
                lParams[0]   <<= aParam;

                if (bAllLocales)
                {
                    aParam.Name    = ::rtl::OUString::createFromAscii("*");
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
