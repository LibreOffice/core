/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>
#include <vcl/accel.hxx>
#include <cppuhelper/implbase1.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>

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
class MenuManager : public ThreadHelpBase           ,
                    public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XStatusListener >
{
    public:
        MenuManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            REFERENCE< XFRAME >& rFrame,
            Menu* pMenu,
            sal_Bool bDelete,
            sal_Bool bDeleteChildren );

        virtual ~MenuManager();

        // XStatusListener
        virtual void SAL_CALL statusChanged( const FEATURSTATEEVENT& Event ) throw ( RUNTIMEEXCEPTION );

        // XEventListener
        virtual void SAL_CALL disposing( const EVENTOBJECT& Source ) throw ( RUNTIMEEXCEPTION );

        DECL_LINK( Select, Menu * );

        Menu*   GetMenu() const { return m_pVCLMenu; }

        void    RemoveListener();

        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& getServiceFactory();

        static void UpdateSpecialWindowMenu( Menu* pMenu ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,IMutex& _rMutex);
        static void FillMenuImages(
            ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& xFrame,
            Menu* _pMenu,
            sal_Bool bShowMenuImages
        );

    protected:
        DECL_LINK(Highlight, void *);
        DECL_LINK( Activate, Menu * );
        DECL_LINK( Deactivate, Menu * );

    private:
        void UpdateSpecialFileMenu( Menu* pMenu );
        void UpdateSpecialWindowMenu( Menu* pMenu );
        void ClearMenuDispatch(const EVENTOBJECT& Source = EVENTOBJECT(),bool _bRemoveOnly = true);
        void SetHdl();
        void AddMenu(PopupMenu* _pPopupMenu,const ::rtl::OUString& _sItemCommand,sal_uInt16 _nItemId,sal_Bool _bDelete,sal_Bool _bDeleteChildren);
        sal_uInt16 FillItemCommand(::rtl::OUString& _rItemCommand,Menu* _pMenu,sal_uInt16 _nIndex) const;


        struct MenuItemHandler
        {
            MenuItemHandler( sal_uInt16 aItemId, MenuManager* pManager, REFERENCE< XDISPATCH >& rDispatch ) :
                nItemId( aItemId ), pSubMenuManager( pManager ), xMenuItemDispatch( rDispatch ) {}

            sal_uInt16                  nItemId;
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

        MenuItemHandler* GetMenuItemHandler( sal_uInt16 nItemId );

        sal_Bool                            m_bInitialized;
        sal_Bool                            m_bDeleteMenu;
        sal_Bool                            m_bDeleteChildren;
        sal_Bool                            m_bActive;
        sal_Bool                            m_bIsBookmarkMenu;
        sal_Bool                            m_bShowMenuImages;
        ::rtl::OUString                     m_aMenuItemCommand;
        Menu*                               m_pVCLMenu;
        REFERENCE< XFRAME >                 m_xFrame;
        ::std::vector< MenuItemHandler* >   m_aMenuItemHandlerVector;

        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& mxServiceFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >             m_xURLTransformer;
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
