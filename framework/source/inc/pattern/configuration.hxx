/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configuration.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:33:09 $
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

#ifndef __FRAMEWORK_PATTERN_CONFIGURATION_HXX_
#define __FRAMEWORK_PATTERN_CONFIGURATION_HXX_

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HXX_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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
