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

#ifndef DISPATCH_POOL_HXX
#define DISPATCH_POOL_HXX

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

namespace framework
{
    namespace statusbar_controller_wordcount
    {
        namespace
        {
            typedef ::cppu::WeakImplHelper1 <
            com::sun::star::lang::XEventListener > DipatchPool_Base;
        }

        class DispatchPool : public DipatchPool_Base
        {
            private:
                DispatchPool();
                ~DispatchPool();
            public:
                virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject &Source ) throw ( ::com::sun::star::uno::RuntimeException );

                static DispatchPool *Create();
                static com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > GetDispatch( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext> &xContext, const com::sun::star::uno::Reference< com::sun::star::frame::XFrame > &xFrame, const com::sun::star::util::URL &rCommandURL, const rtl::OUString &rModuleIdentifier );
        };
    }
}

#endif
