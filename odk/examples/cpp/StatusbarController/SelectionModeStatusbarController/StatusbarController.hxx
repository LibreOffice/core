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

#ifndef STATUSBARCONTROLLER_HXX
#define STATUSBARCONTROLLER_HXX

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/ui/XStatusbarItem.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusbarController.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <map>

namespace framework
{
    namespace statusbar_controller_selectionmode
    {
        typedef cppu::WeakComponentImplHelper1 < com::sun::star::frame::XStatusbarController > StatusbarController_Base;

        class StatusbarController : protected cppu::BaseMutex,
            public StatusbarController_Base
        {
            public:
                explicit StatusbarController();
                virtual ~StatusbarController();

                virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::frame::XStatusbarController Methods
                virtual ::sal_Bool SAL_CALL mouseButtonDown( const ::com::sun::star::awt::MouseEvent &aMouseEvent ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual ::sal_Bool SAL_CALL mouseMove( const ::com::sun::star::awt::MouseEvent &aMouseEvent ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual ::sal_Bool SAL_CALL mouseButtonUp( const ::com::sun::star::awt::MouseEvent &aMouseEvent ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL command( const ::com::sun::star::awt::Point &aPos, ::sal_Int32 nCommand, ::sal_Bool bMouseEvent, const ::com::sun::star::uno::Any &aData ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > &xGraphics, const ::com::sun::star::awt::Rectangle &rOutputRectangle, ::sal_Int32 nStyle ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL click( const ::com::sun::star::awt::Point &aPos ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL doubleClick( const ::com::sun::star::awt::Point &aPos ) throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::frame::XStatusListener Methods
                virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent &State ) throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::lang::XEventListener Methods
                virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject &Source ) throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::util::XUpdatable Methods
                virtual void SAL_CALL update() throw ( ::com::sun::star::uno::RuntimeException );

                // com::sun::star::lang::XInitialization Methods
                virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &aArguments ) throw ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

            protected:

                struct Listener
                {

                    explicit Listener(
                        const ::com::sun::star::util::URL &rURL,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > &rDispatch )
                        : aURL( rURL )
                        , xDispatch( rDispatch )
                    {
                    }

                    ::com::sun::star::util::URL aURL;
                    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDispatch;
                };

                typedef ::std::map< ::rtl::OUString, com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > > URLToDispatchMap;

                void ThrowIfDisposed() throw ( com::sun::star::uno::RuntimeException );
                void BindListeners();
                void UnbindListeners();
                bool IsBound() const;
                ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > GetURLTransformer();
                // execute methods
                void Execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > &aArgs );
                void Execute( const rtl::OUString &rCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > &aArgs );
                void UpdateStatus();
                void UpdateStatus( const rtl::OUString &rCommand );

                bool m_bInitialized;
                bool m_bOwnerDraw;
                rtl::OUString m_sModuleIdentifier;
                rtl::OUString m_sCommandURL;
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xStatusbarWindow;
                ::com::sun::star::uno::Reference< ::com::sun::star::ui::XStatusbarItem > m_xStatusbarItem;
                ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLTransformer;
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

                URLToDispatchMap m_aListenerMap;
        };
    }
}

#endif  /* STATUSBARCONTROLLER_HXX */

