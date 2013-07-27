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



#ifndef __FRAMEWORK_UIELEMENT_NEWMENUCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_NEWMENUCONTROLLER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <vcl/accel.hxx>
#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <hash_map>

namespace framework
{
    struct NewDocument
    {
        ::com::sun::star::util::URL                                                 aTargetURL;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   aArgSeq;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >      xDispatch;
    };

    class NewMenuController :  public svt::PopupMenuControllerBase
    {
        using svt::PopupMenuControllerBase::disposing;

        public:
            NewMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
            virtual ~NewMenuController();

            // XServiceInfo
            DECLARE_XSERVICEINFO

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

            // XStatusListener
            virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

            // XMenuListener
            virtual void SAL_CALL itemSelected( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL itemActivated( const ::com::sun::star::awt::MenuEvent& rEvent ) throw (::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

            DECL_STATIC_LINK( NewMenuController, ExecuteHdl_Impl, NewDocument* );

        private:
            virtual void impl_setPopupMenu();
            struct AddInfo
            {
                rtl::OUString aTargetFrame;
                rtl::OUString aImageId;
            };

            typedef ::std::hash_map< int, AddInfo > AddInfoForId;

            void fillPopupMenu( com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >& rPopupMenu );
            void retrieveShortcutsFromConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >& rAccelCfg,
                                                     const ::com::sun::star::uno::Sequence< rtl::OUString >& rCommands,
                                                     std::vector< KeyCode >& aMenuShortCuts );
            void setAccelerators( PopupMenu* pPopupMenu );
            void determineAndSetNewDocAccel( PopupMenu* pPopupMenu, const KeyCode& rKeyCode );
            void setMenuImages( PopupMenu* pPopupMenu, sal_Bool bSetImages, sal_Bool bHiContrast );

        private:
            // members
            sal_Bool            m_bShowImages : 1,
                                m_bHiContrast : 1,
                                m_bNewMenu    : 1,
                                m_bModuleIdentified : 1,
                                m_bAcceleratorCfg : 1;
            AddInfoForId        m_aAddInfoForItem;
            rtl::OUString       m_aTargetFrame;
            rtl::OUString       m_aModuleIdentifier;
            rtl::OUString       m_aEmptyDocURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xDocAcceleratorManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xModuleAcceleratorManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration > m_xGlobalAcceleratorManager;
    };
}

#endif // __FRAMEWORK_UIELEMENT_NEWMENUCONTROLLER_HXX_
