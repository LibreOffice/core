/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <stdtypes.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusbarController.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <vcl/status.hxx>
#include <map>

namespace framework
{

class FrameworkStatusBar;
class StatusBarManager : public ThreadHelpBase,
                         public ::cppu::WeakImplHelper3<
                                   css::frame::XFrameActionListener,
                                   css::lang::XComponent,
                                   css::ui::XUIConfigurationListener >

{
    friend class FrameworkStatusBar;

    public:
        StatusBarManager( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                          const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                          const OUString& rResourceName,
                          StatusBar* pStatusBar );
        virtual ~StatusBarManager();

        StatusBar* GetStatusBar() const;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const com::sun::star::frame::FrameActionEvent& Action ) throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );
        void SAL_CALL addEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
        void SAL_CALL removeEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );

        void FillStatusBar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rStatusBarData );

    protected:
        void StateChanged( StateChangedType nType );
        void DataChanged( const DataChangedEvent& rDCEvt );
        void UserDraw( const UserDrawEvent& rUDEvt );
        void Command( const CommandEvent& rEvt );
        void MouseMove( const MouseEvent& rMEvt );
        void MouseButtonDown( const MouseEvent& rMEvt );
        void MouseButtonUp( const MouseEvent& rMEvt );
        DECL_LINK(Click, void *);
        DECL_LINK(DoubleClick, void *);

        void RemoveControllers();
        OUString RetrieveLabelFromCommand( const OUString& aCmdURL );
        void CreateControllers();
        void UpdateControllers();
        void AddFrameActionListener();
        void MouseButton( const MouseEvent& rMEvt ,sal_Bool ( SAL_CALL ::com::sun::star::frame::XStatusbarController::*_pMethod )(const ::com::sun::star::awt::MouseEvent&));

    protected:
        typedef std::map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::frame::XStatusbarController > > StatusBarControllerMap;

        sal_Bool                                                                                        m_bDisposed : 1,
                                                                                                        m_bFrameActionRegistered : 1,
                                                                                                        m_bUpdateControllers : 1;
        sal_Bool                                                                                        m_bModuleIdentified;
        StatusBar*                                                                                      m_pStatusBar;
        OUString                                                                                   m_aModuleIdentifier;
        OUString                                                                                   m_aResourceName;
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame >                                 m_xFrame;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >                        m_xUICommandLabels;
        StatusBarControllerMap                                                                          m_aControllerMap;
        ::cppu::OMultiTypeInterfaceContainerHelper                                                      m_aListenerContainer;   /// container for ALL Listener
        ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >                      m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUIControllerFactory >               m_xStatusbarControllerFactory;
};

}

#endif // __FRAMEWORK_UIELEMENT_STATUSBARMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
