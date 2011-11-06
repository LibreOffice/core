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


#ifndef EXTENSIONS_OOOIMPROVEMENT_ONLOGROTATEJOB_HXX
#define EXTENSIONS_OOOIMPROVEMENT_ONLOGROTATEJOB_HXX

#include <com/sun/star/task/XAsyncJob.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>


namespace oooimprovement
{
    #ifdef css
        #error css defined globally
    #endif
    #define css ::com::sun::star
    class OnLogRotateJob : public ::cppu::WeakImplHelper2<
        ::com::sun::star::task::XAsyncJob,
        ::com::sun::star::lang::XServiceInfo>
    {
        public:
            // XServiceInfo - static version
            static ::rtl::OUString SAL_CALL getImplementationName_static();
            static ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames_static();
            static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL Create(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& context);

            static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL Create(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& sf);

        protected:
            OnLogRotateJob(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& context);
            OnLogRotateJob(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& sm);
            virtual ~OnLogRotateJob();

            // XAsyncJob
            virtual void SAL_CALL executeAsync(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue>& args,
                const ::com::sun::star::uno::Reference< ::com::sun::star::task::XJobListener>& listener)
                throw(::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName()
                throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& service_name)
                throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames()
                throw(::com::sun::star::uno::RuntimeException);

        private:
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_ServiceFactory;
    };
    #undef css
}
#endif
