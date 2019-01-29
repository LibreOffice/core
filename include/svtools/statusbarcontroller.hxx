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

#ifndef INCLUDED_SVTOOLS_STATUSBARCONTROLLER_HXX
#define INCLUDED_SVTOOLS_STATUSBARCONTROLLER_HXX

#include <svtools/svtdllapi.h>
#include <com/sun/star/frame/XStatusbarController.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <tools/gen.hxx>
#include <unordered_map>

namespace com :: sun :: star :: awt { class XWindow; }
namespace com :: sun :: star :: beans { struct PropertyValue; }
namespace com :: sun :: star :: frame { class XDispatch; }
namespace com :: sun :: star :: frame { class XFrame; }
namespace com :: sun :: star :: ui { class XStatusbarItem; }
namespace com :: sun :: star :: uno { class XComponentContext; }
namespace com :: sun :: star :: util { class XURLTransformer; }

namespace svt
{

class SVT_DLLPUBLIC StatusbarController :
                            public css::frame::XStatusbarController,
                            public ::cppu::BaseMutex,
                            public ::cppu::OWeakObject
{
    public:
        StatusbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                             const css::uno::Reference< css::frame::XFrame >& xFrame,
                             const OUString& aCommandURL,
                             unsigned short       nID );
        StatusbarController();
        virtual ~StatusbarController() override;

        css::uno::Reference< css::frame::XFrame > getFrameInterface() const;
        css::uno::Reference< css::util::XURLTransformer > getURLTransformer() const;

        ::tools::Rectangle getControlRect() const;

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XUpdatable
        virtual void SAL_CALL update() override;

        // XComponent
        virtual void SAL_CALL dispose() override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        // XStatusbarController
        virtual sal_Bool SAL_CALL mouseButtonDown( const css::awt::MouseEvent& aMouseEvent ) override;
        virtual sal_Bool SAL_CALL mouseMove( const css::awt::MouseEvent& aMouseEvent ) override;
        virtual sal_Bool SAL_CALL mouseButtonUp( const css::awt::MouseEvent& aMouseEvent ) override;
        virtual void SAL_CALL command( const css::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       sal_Bool bMouseEvent,
                                       const css::uno::Any& aData ) override;
        virtual void SAL_CALL paint( const css::uno::Reference< css::awt::XGraphics >& xGraphics,
                                     const css::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nStyle ) override;
        virtual void SAL_CALL click( const css::awt::Point& aPos ) override;
        virtual void SAL_CALL doubleClick( const css::awt::Point& aPos ) override;

    protected:
        struct Listener
        {
            Listener( const css::util::URL& rURL, const css::uno::Reference< css::frame::XDispatch >& rDispatch ) :
                aURL( rURL ), xDispatch( rDispatch ) {}

            css::util::URL const                         aURL;
            css::uno::Reference< css::frame::XDispatch > xDispatch;
        };

        typedef std::unordered_map< OUString,
                                    css::uno::Reference< css::frame::XDispatch > > URLToDispatchMap;

        // methods to support status forwarder, known by the old sfx2 toolbox controller implementation
        void addStatusListener( const OUString& aCommandURL );
        void bindListener();

        // execute methods
        // execute bound status bar controller command/execute various commands
        void execute( const css::uno::Sequence< css::beans::PropertyValue >& aArgs );
        void execute( const OUString& aCommand, const css::uno::Sequence< css::beans::PropertyValue >& aArgs );

        bool                                                      m_bInitialized : 1,
                                                                  m_bDisposed : 1;
        unsigned short                                            m_nID;
        css::uno::Reference< css::frame::XFrame >                 m_xFrame;
        css::uno::Reference< css::awt::XWindow >                  m_xParentWindow;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        OUString                                                  m_aCommandURL;
        URLToDispatchMap                                          m_aListenerMap;
        ::cppu::OMultiTypeInterfaceContainerHelper                m_aListenerContainer;   /// container for ALL Listener
        mutable css::uno::Reference< css::util::XURLTransformer > m_xURLTransformer;
        css::uno::Reference< css::ui::XStatusbarItem >            m_xStatusbarItem;
};

}

#endif // INCLUDED_SVTOOLS_STATUSBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
