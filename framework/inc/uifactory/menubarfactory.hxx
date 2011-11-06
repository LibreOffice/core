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



#ifndef __FRAMEWORK_UIFACTORY_MENUBARFACTORY_HXX_
#define __FRAMEWORK_UIFACTORY_MENUBARFACTORY_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <stdtypes.h>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/implbase2.hxx>
#include <rtl/ustring.hxx>

namespace framework
{
    class MenuBarFactory :  protected ThreadHelpBase                                    ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                            public ::cppu::WeakImplHelper2< com::sun::star::lang::XServiceInfo,
                                                            ::com::sun::star::ui::XUIElementFactory>
    {
        public:
            MenuBarFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~MenuBarFactory();

            //  XInterface, XTypeProvider, XServiceInfo
            DECLARE_XSERVICEINFO

            // XUIElementFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > SAL_CALL createUIElement( const ::rtl::OUString& ResourceURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Args ) throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

            static void CreateUIElement(const ::rtl::OUString& ResourceURL
                        , const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Args
                        ,const char* _pExtraMode
                        ,const char* _pAsciiName
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& _xMenuBar
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager >& _xModuleManager
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceManager);

        protected:
            MenuBarFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager,bool );

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > m_xModuleManager;
    };
}

#endif // __FRAMEWORK_UIFACTORY_MENUBARFACTORY_HXX_
