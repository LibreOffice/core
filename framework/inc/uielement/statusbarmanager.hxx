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

#pragma once

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusbarController.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <vcl/status.hxx>
#include <vcl/vclptr.hxx>
#include <map>

namespace framework
{

class FrameworkStatusBar;
class StatusBarManager final: public ::cppu::WeakImplHelper<
                                   css::frame::XFrameActionListener,
                                   css::lang::XComponent,
                                   css::ui::XUIConfigurationListener >

{
    friend class FrameworkStatusBar;

    public:
        StatusBarManager( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                          const css::uno::Reference< css::frame::XFrame >& rFrame,
                          StatusBar* pStatusBar );
        virtual ~StatusBarManager() override;

        StatusBar* GetStatusBar() const;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& Action ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& Event ) override;
        virtual void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& Event ) override;
        virtual void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& Event ) override;

        // XComponent
        void SAL_CALL dispose() override;
        void SAL_CALL addEventListener( const css::uno::Reference< XEventListener >& xListener ) override;
        void SAL_CALL removeEventListener( const css::uno::Reference< XEventListener >& xListener ) override;

        void FillStatusBar( const css::uno::Reference< css::container::XIndexAccess >& rStatusBarData );

    private:
        void DataChanged( const DataChangedEvent& rDCEvt );
        void UserDraw( const UserDrawEvent& rUDEvt );
        void Command( const CommandEvent& rEvt );
        void MouseMove( const MouseEvent& rMEvt );
        void MouseButtonDown( const MouseEvent& rMEvt );
        void MouseButtonUp( const MouseEvent& rMEvt );
        DECL_LINK(Click, StatusBar*, void);
        DECL_LINK(DoubleClick, StatusBar*, void);

        void RemoveControllers();
        void CreateControllers();
        void UpdateControllers();
        void MouseButton( const MouseEvent& rMEvt ,sal_Bool ( SAL_CALL css::frame::XStatusbarController::*_pMethod )(const css::awt::MouseEvent&));

        typedef std::map< sal_uInt16, css::uno::Reference< css::frame::XStatusbarController > > StatusBarControllerMap;

        bool                                                                  m_bDisposed : 1,
                                                                              m_bFrameActionRegistered : 1,
                                                                              m_bUpdateControllers : 1;
        VclPtr<StatusBar>                                                     m_pStatusBar;
        css::uno::Reference< css::frame::XFrame >                             m_xFrame;
        StatusBarControllerMap                                                m_aControllerMap;
        osl::Mutex                                                            m_mutex;
        ::cppu::OMultiTypeInterfaceContainerHelper                            m_aListenerContainer;   /// container for ALL Listener
        css::uno::Reference< css::uno::XComponentContext >                    m_xContext;
        css::uno::Reference< css::frame::XUIControllerFactory >               m_xStatusbarControllerFactory;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
