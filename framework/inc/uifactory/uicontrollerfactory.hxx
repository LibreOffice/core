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

#ifndef __FRAMEWORK_UICONTROLLERFACTORY_HXX_
#define __FRAMEWORK_UICONTROLLERFACTORY_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>

#include <cppuhelper/implbase2.hxx>

namespace framework
{

class ConfigurationAccess_ControllerFactory;
class UIControllerFactory :  protected ThreadHelpBase, // Struct for right initalization of mutex member! Must be first of baseclasses.
                             public ::cppu::WeakImplHelper2<
                                 com::sun::star::lang::XServiceInfo,
                                 com::sun::star::frame::XUIControllerFactory >
{
    public:
        virtual ~UIControllerFactory();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

        // XMultiComponentFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithContext( const ::rtl::OUString& aServiceSpecifier, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames() throw (::com::sun::star::uno::RuntimeException);

        // XUIControllerRegistration
        virtual sal_Bool SAL_CALL hasController( const ::rtl::OUString& aCommandURL, const rtl::OUString& aModuleName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL registerController( const ::rtl::OUString& aCommandURL, const rtl::OUString& aModuleName, const ::rtl::OUString& aControllerImplementationName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deregisterController( const ::rtl::OUString& aCommandURL, const rtl::OUString& aModuleName ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        UIControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager, const rtl::OUString &rUINode  );
        sal_Bool                                                                         m_bConfigRead;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
        ConfigurationAccess_ControllerFactory*                                           m_pConfigAccess;
};

class PopupMenuControllerFactory :  public UIControllerFactory
{
    public:
        PopupMenuControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO
};

class ToolbarControllerFactory :  public UIControllerFactory
{
    public:
        ToolbarControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO
};

class StatusbarControllerFactory :  public UIControllerFactory
{
    public:
        StatusbarControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO
};

}

#endif // __FRAMEWORK_UICONTROLLERFACTORY_HXX_
