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



// MARKER(update_precomp.py): autogen include statement, do not remove


#ifndef EXTENSIONS_OOOIMPROVEMENT_CORECONTROLLER_HXX
#define EXTENSIONS_OOOIMPROVEMENT_CORECONTROLLER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/oooimprovement/XCoreController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase2.hxx>


namespace oooimprovement
{
    #ifdef css
        #error css defined globally
    #endif
    #define css ::com::sun::star
    class CoreController : public ::cppu::WeakImplHelper2<
        css::oooimprovement::XCoreController,
        css::lang::XServiceInfo>
    {
        public:
            // css::lang::XServiceInfo - static version
            static ::rtl::OUString SAL_CALL getImplementationName_static();
            static css::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames_static();
            //static css::uno::Reference< css::uno::XInterface> SAL_CALL Create(
            //    const css::uno::Reference<css::uno::XComponentContext>& context);

            static css::uno::Reference< css::uno::XInterface> SAL_CALL Create(
                const css::uno::Reference< css::lang::XMultiServiceFactory>& sm);

        protected:
            CoreController(const css::uno::Reference< css::uno::XComponentContext>& context);
            CoreController(const css::uno::Reference< css::lang::XMultiServiceFactory>& sm);
            virtual ~CoreController();

            // css::oooimprovement::XCoreController
            virtual sal_Bool SAL_CALL enablingUiEventsLoggerAllowed(sal_Int16 version)
                throw(css::uno::RuntimeException);
            virtual sal_Bool SAL_CALL showBuiltinOptionsPage(sal_Int16 version)
                throw(css::uno::RuntimeException);

            // css::lang::XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName()
                throw(css::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString& service_name)
                throw(css::uno::RuntimeException);
            virtual css::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames()
                throw(css::uno::RuntimeException);

        private:
            css::uno::Reference< css::lang::XMultiServiceFactory> m_ServiceFactory;
    };
    #undef css
}
#endif
