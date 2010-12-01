/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layoutmanager.hxx,v $
 * $Revision: 1.34 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
