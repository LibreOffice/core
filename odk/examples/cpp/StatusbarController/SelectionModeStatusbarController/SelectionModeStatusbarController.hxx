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

#ifndef SELECTIONMODESTATUSBARCONTROLLER_HXX
#define SELECTIONMODESTATUSBARCONTROLLER_HXX

#include "StatusbarController.hxx"

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

namespace framework
{
    namespace statusbar_controller_selectionmode
    {
        enum SelectionMode
        {
            SEL_MODE_STD = 0,
            SEL_MODE_EXT,
            SEL_MODE_ADD,
            SEL_MODE_BLK
        };

        typedef cppu::ImplInheritanceHelper1< StatusbarController, com::sun::star::lang::XServiceInfo >
        SelectionModeStatusbarController_Base;

        class SelectionModeStatusbarController : public SelectionModeStatusbarController_Base
        {
            public:
                explicit SelectionModeStatusbarController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext> &rxContext );
                ~SelectionModeStatusbarController();

                virtual void SAL_CALL command( const ::com::sun::star::awt::Point &aPos, ::sal_Int32 nCommand, ::sal_Bool bMouseEvent, const ::com::sun::star::uno::Any &aData ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL click(  const ::com::sun::star::awt::Point &aPos ) throw ( ::com::sun::star::uno::RuntimeException );

                virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent &aFeatureState ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > &aArguments ) throw ( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException );

                virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( ::com::sun::star::uno::RuntimeException );
                virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString &ServiceName ) throw ( ::com::sun::star::uno::RuntimeException );
                virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( ::com::sun::star::uno::RuntimeException );

                static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > Create( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > &rxContext ) throw ( com::sun::star::uno::Exception );
                static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static();
                static ::rtl::OUString getImplementationName_static();

            private:
                void ExecutePopupMenu( const com::sun::star::awt::Point &aPos );

                bool m_bEnabled;
                bool m_bSelectionModeEnabled;
                SelectionMode m_eSelectionMode;

        };
    }
}

#endif
