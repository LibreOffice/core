/*************************************************************************
 *
 *  $RCSfile: modulemanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 17:35:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_
#define __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

//_______________________________________________
// definition

#ifndef css
namespace css = ::com::sun::star;
#endif

#ifndef dcss
namespace dcss = ::drafts::com::sun::star;
#endif

namespace framework
{

//_______________________________________________
/**
    implements the service drafts.com.sun.star.frame.ModuleManager
 */
class ModuleManager : public  css::lang::XTypeProvider
                    , public  css::lang::XServiceInfo
                    , public  dcss::frame::XModuleManager
                    , public  css::container::XNameAccess // => XElementAccess
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
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XModuleManager
        virtual ::rtl::OUString SAL_CALL identify(const css::uno::Reference< css::uno::XInterface >& xModule)
            throw(css::lang::IllegalArgumentException,
                  dcss::frame::UnknownModuleException,
                  css::uno::RuntimeException         );

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

    //___________________________________________
    // helper

    private:

        //---------------------------------------
        /** @short  open the underlying configuration.

            @descr  This method must be called everytimes
                    a configuartion call is needed. Because
                    method works together with the member
                    m_xCFG, open it on demand and cache it
                    afterwards.

            @return [com.sun.star.container.XNameAccess]
                    the configuration object

            @throw  [com.sun.star.uno.RuntimeException]
                    if config could not be opened successfully!

            @threadsafe
          */
        css::uno::Reference< css::container::XNameAccess > implts_openConfig()
            throw(css::uno::RuntimeException);

        //---------------------------------------
        /** @short  makes the real identification of the module.

            @descr  The interface method identify() tries to full fill the
                    requirements (means it check for the right interfaces and throw
                    all supported exceptions). So this method impl_identify() is called
                    with a valid parameter everytimes and can be concentrated to the real
                    detection.

            @param  xModule
                    the module for identification.

            @return The identifier (uno service name) of the given module.

            @throw  dcss::frame::UnknownModuleException
                    if the module has no valid configuration.
         */
        ::rtl::OUString impl_identify(const css::uno::Reference< css::lang::XServiceInfo >& xModule)
            throw(dcss::frame::UnknownModuleException);
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_MODULEMANAGER_HXX_
