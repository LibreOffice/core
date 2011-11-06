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



#ifndef __FRAMEWORK_LAYOUTMANAGER_PANELMANAGER_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_PANELMANAGER_HXX_

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
#include "panel.hxx"
#include <uielement/uielement.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <cppuhelper/weak.hxx>
#include <vcl/window.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

class PanelManager : private ThreadHelpBase // Struct for right initalization of mutex member! Must be first of baseclasses.
{
    public:
        PanelManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMGR,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
        virtual ~PanelManager();

        bool createPanels();
        ::com::sun::star::awt::Rectangle getPreferredSize() const;
        void layoutPanels( const ::com::sun::star::awt::Rectangle newSize );

        UIElement* findDockingWindow( const ::rtl::OUString& rResourceName );
        bool addDockingWindow( const ::rtl::OUString& rResourceName, const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& xUIElement );
        bool destroyDockingWindow( const ::rtl::OUString& rResourceName );

        //---------------------------------------------------------------------------------------------------------
        //  XDockableWindowListener
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL startDocking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::DockingData SAL_CALL docking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endDocking( const ::com::sun::star::awt::EndDockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL prepareToggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL toggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL closed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    private:
        Panel*                                                 m_aPanels[PANEL_COUNT];
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
        css::uno::Reference< css::frame::XFrame >              m_xFrame;
};

} // namespace framework

#endif // __FRAMEWORK_LAYOUTMANAGER_PANELMANAGER_HXX_
