/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: menumanager.hxx,v $
 * $Revision: 1.18 $
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

#ifndef __FRAMEWORK_CLASSES_MENUMANAGER_HXX_
#define __FRAMEWORK_CLASSES_MENUMANAGER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>
#include <vcl/accel.hxx>
#include <cppuhelper/weak.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>

// #110897#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#define REFERENCE                                       ::com::sun::star::uno::Reference
#define XFRAME                                          ::com::sun::star::frame::XFrame
#define XDISPATCH                                       ::com::sun::star::frame::XDispatch
#define XDISPATCHPROVIDER                               ::com::sun::star::frame::XDispatchProvider
#define XSTATUSLISTENER                                 ::com::sun::star::frame::XStatusListener
#define XEVENTLISTENER                                  ::com::sun::star::lang::XEventListener
#define FEATURSTATEEVENT                                ::com::sun::star::frame::FeatureStateEvent
#define RUNTIMEEXCEPTION                                ::com::sun::star::uno::RuntimeException
#define EVENTOBJECT                                     ::com::sun::star::lang::EventObject

namespace framework
{

class BmkMenu;
class AddonMenu;
class AddonPopupMenu;
class MenuManager : public XSTATUSLISTENER      ,
                    public ThreadHelpBase           ,
                    public ::cppu::OWeakObject
{
    public:
        // #110897#
        MenuManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            REFERENCE< XFRAME >& rFrame,
            Menu* pMenu,
            sal_Bool bDelete,
            sal_Bool bDeleteChildren );

        MenuManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            REFERENCE< XFRAME >& rFrame,
            BmkMenu*            pBmkMenu,
            sal_Bool            bDelete,
            sal_Bool            bDeleteChildren );

        MenuManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            REFERENCE< XFRAME >& rFrame,
            AddonMenu*          pAddonMenu,
            sal_Bool            bDelete,
            sal_Bool            bDeleteChildren );

        MenuManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            REFERENCE< XFRAME >& rFrame,
            AddonPopupMenu* pAddonMenu,
            sal_Bool            bDelete,
            sal_Bool            bDeleteChildren );

        virtual ~MenuManager();

        // XInterface
        virtual void SAL_CALL acquire() throw()
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw()
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type & rType ) throw( RUNTIMEEXCEPTION );

        // XStatusListener
        virtual void SAL_CALL statusChanged( const FEATURSTATEEVENT& Event ) throw ( RUNTIMEEXCEPTION );

        // XEventListener
        virtual void SAL_CALL disposing( const EVENTOBJECT& Source ) throw ( RUNTIMEEXCEPTION );

        DECL_LINK( Select, Menu * );

        Menu*   GetMenu() const { return m_pVCLMenu; }

        void    RemoveListener();

        // #110897#
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& getServiceFactory();

    protected:
        DECL_LINK( Highlight, Menu * );
        DECL_LINK( Activate, Menu * );
        DECL_LINK( Deactivate, Menu * );

    private:
        void UpdateSpecialFileMenu( Menu* pMenu );
        void UpdateSpecialWindowMenu( Menu* pMenu );


        struct MenuItemHandler
        {
            MenuItemHandler( USHORT aItemId, MenuManager* pManager, REFERENCE< XDISPATCH >& rDispatch ) :
                nItemId( aItemId ), pSubMenuManager( pManager ), xMenuItemDispatch( rDispatch ) {}

            USHORT                  nItemId;
            ::rtl::OUString         aTargetFrame;
            ::rtl::OUString         aMenuItemURL;
            ::rtl::OUString         aFilter;
            ::rtl::OUString         aPassword;
            ::rtl::OUString         aTitle;
            MenuManager*            pSubMenuManager;
            REFERENCE< XDISPATCH >  xMenuItemDispatch;
        };

        void             CreatePicklistArguments(
                            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgsList,
                            const MenuItemHandler* );

        MenuItemHandler* GetMenuItemHandler( USHORT nItemId );

        sal_Bool                            m_bInitialized;
        sal_Bool                            m_bDeleteMenu;
        sal_Bool                            m_bDeleteChildren;
        sal_Bool                            m_bActive;
        sal_Bool                            m_bIsBookmarkMenu;
        sal_Bool                            m_bWasHiContrast;
        sal_Bool                            m_bShowMenuImages;
        ::rtl::OUString                     m_aMenuItemCommand;
        Menu*                               m_pVCLMenu;
        REFERENCE< XFRAME >                 m_xFrame;
        ::std::vector< MenuItemHandler* >   m_aMenuItemHandlerVector;

        // #110897#
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& mxServiceFactory;
};

} // namespace

#endif
