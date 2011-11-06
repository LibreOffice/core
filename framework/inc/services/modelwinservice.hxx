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



#ifndef __FRAMEWORK_SERVICES_MODELWINSERVICE_HXX_
#define __FRAMEWORK_SERVICES_MODELWINSERVICE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <cppuhelper/weak.hxx>
#include <vcl/window.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

class IModelWin
{
    public:
        virtual void registerModelForXWindow( const css::uno::Reference< css::awt::XWindow >& rWindow, const css::uno::Reference< css::awt::XControlModel >& rModel ) = 0;
        virtual void deregisterModelForXWindow( const css::uno::Reference< css::awt::XWindow >& rWindow ) = 0;
};

class ModelWinService : public css::lang::XTypeProvider
                     ,  public css::lang::XServiceInfo
                     ,  public css::container::XNameAccess
                     ,  public IModelWin
                     ,  public ::cppu::OWeakObject
{
    public:
        ModelWinService(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager );
        virtual ~ModelWinService();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  IModelWin
        //---------------------------------------------------------------------------------------------------------
        virtual void registerModelForXWindow( const css::uno::Reference< css::awt::XWindow >& rWindow, const css::uno::Reference< css::awt::XControlModel >& rModel );
        virtual void deregisterModelForXWindow( const css::uno::Reference< css::awt::XWindow >& rWindow );

        //---------------------------------------------------------------------------------------------------------
        //  XNameAccess
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& sName ) throw( css::container::NoSuchElementException  ,
                                                                                        css::lang::WrappedTargetException       ,
                                                                                        css::uno::RuntimeException              );
        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw( css::uno::RuntimeException );
        virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& sName ) throw( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XElementAccess
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Type SAL_CALL getElementType() throw( css::uno::RuntimeException );
        virtual sal_Bool SAL_CALL hasElements() throw( css::uno::RuntimeException );

    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xServiceManager;
};

}

#endif // __FRAMEWORK_SERVICES_MODELWINSERVICE_HXX_
