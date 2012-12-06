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

#ifndef BASE_DISPATCH_HXX
#define BASE_DISPATCH_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <map>

namespace framework
{
    namespace statusbar_controller_wordcount
    {
        typedef cppu::WeakComponentImplHelper3 <
            com::sun::star::frame::XDispatch,
            com::sun::star::util::XModifyListener,
            com::sun::star::view::XSelectionChangeListener > BaseDispatch_Base;

        class BaseDispatch :
            protected ::cppu::BaseMutex,
            public BaseDispatch_Base
        {
            public:
                virtual void SAL_CALL disposing();

                // ::com::sun::star::frame::XDispatch:
                virtual void SAL_CALL dispatch( const com::sun::star::util::URL &aURL, const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &lArguments ) throw ( com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL addStatusListener( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > &xControl, const com::sun::star::util::URL &aURL ) throw ( com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL removeStatusListener( const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > &xControl, const com::sun::star::util::URL &aURL ) throw ( com::sun::star::uno::RuntimeException );

                // com::sun::star::util::XModifyListener Methods
                virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject &aEvent ) throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::view::XSelectionChangeListener Methods
                virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject &aEvent ) throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::lang::XEventListener Methods
                virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject &aEvent ) throw ( ::com::sun::star::uno::RuntimeException );

            protected:
                BaseDispatch( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > &rxContext,
                              const com::sun::star::uno::Reference< com::sun::star::frame::XFrame > &rxFrame,
                              const rtl::OUString &rModuleIdentifier );
                virtual ~BaseDispatch();

                virtual com::sun::star::frame::FeatureStateEvent GetState( const com::sun::star::util::URL &rURL ) = 0;
                virtual rtl::OUString GetCommand() = 0;
                virtual void ExecuteCommand( const com::sun::star::util::URL &aURL, const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > &lArguments ) = 0;

                void ThrowIfDisposed() throw ( com::sun::star::uno::RuntimeException );
                void FireStatusEvent( const com::sun::star::util::URL &aURL, const com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > &xControl = com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener >() );
                void FireStatusEvent();

                com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xContext;
                com::sun::star::uno::Reference< com::sun::star::frame::XFrame > m_xFrame;
                com::sun::star::uno::Reference< com::sun::star::frame::XModel > m_xModel;
                com::sun::star::uno::Reference< com::sun::star::util::XURLTransformer > m_xURLTransformer;
                rtl::OUString m_sModuleIdentifier;

            private:
                typedef ::std::map< ::rtl::OUString, ::cppu::OInterfaceContainerHelper * > tListenerMap;
                tListenerMap m_aListeners;
        };
    }
}

#endif
