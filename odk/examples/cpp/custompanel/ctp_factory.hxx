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



#ifndef SD_WORKBENCH_CTP_FACTORY_HXX
#define SD_WORKBENCH_CTP_FACTORY_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/basemutex.hxx>

//......................................................................................................................
namespace sd { namespace colortoolpanel
{
//......................................................................................................................

    class FactoryGuard;

    //==================================================================================================================
    //= ToolPanelFactory
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::ui::XUIElementFactory
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    >   ToolPanelFactory_Base;
    class ToolPanelFactory   :public ::cppu::BaseMutex
                            ,public ToolPanelFactory_Base
    {
    public:
        ToolPanelFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext
        );
        ~ToolPanelFactory();

        // XUIElementFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > SAL_CALL createUIElement( const ::rtl::OUString& i_rResourceURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& i_rArgs ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static versions
        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext ) throw (::com::sun::star::uno::RuntimeException);

    private:
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                m_xContext;
    };

//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................

#endif // SD_WORKBENCH_CTP_FACTORY_HXX
