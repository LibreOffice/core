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

#ifndef PROTOCOL_HANDLER_HXX
#define PROTOCOL_HANDLER_HXX

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace frame
            {
                class XFrame;
            }
        }
    }
}

namespace framework
{
    namespace statusbar_controller_wordcount
    {

        namespace
        {
            typedef cppu::WeakComponentImplHelper3 <
            com::sun::star::frame::XDispatchProvider,
                com::sun::star::lang::XInitialization,
                com::sun::star::lang::XServiceInfo > ProtocolHandler_Base;
        }

        class ProtocolHandler :
            protected ::cppu::BaseMutex,
            public ProtocolHandler_Base
        {
            public:
                explicit ProtocolHandler( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > &rxContext );
                virtual ~ProtocolHandler();

                static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > Create( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > &rxContext ) throw( com::sun::star::uno::Exception );
                static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static();
                static ::rtl::OUString getImplementationName_static();

                // called from WeakComponentImplHelper when XComponent::dispose() was
                // called from outside
                virtual void SAL_CALL disposing();

                // ::com::sun::star::lang::XInitialization:
                virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &aArguments ) throw ( com::sun::star::uno::RuntimeException, com::sun::star::uno::Exception );

                // ::com::sun::star::frame::XDispatchProvider:
                virtual com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const com::sun::star::util::URL &URL, const ::rtl::OUString &TargetFrameName, sal_Int32 SearchFlags ) throw ( com::sun::star::uno::RuntimeException );
                virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const com::sun::star::uno::Sequence< com::sun::star::frame::DispatchDescriptor > &Requests ) throw ( com::sun::star::uno::RuntimeException );

                // ::com::sun::star::lang::XServiceInfo:
                virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( com::sun::star::uno::RuntimeException );
                virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString &ServiceName ) throw ( com::sun::star::uno::RuntimeException );
                virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( com::sun::star::uno::RuntimeException );

            private:
                void ThrowIfDisposed() throw ( com::sun::star::uno::RuntimeException );

                com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xContext;
                com::sun::star::uno::Reference< com::sun::star::frame::XFrame > m_xFrame;
                rtl::OUString m_sModuleIdentifier;
        };

    }
}

#endif
