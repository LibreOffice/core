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
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusbarController.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/XStatusbarItem.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <tools/gen.hxx>
#include <unordered_map>

namespace svt
{

class SVT_DLLPUBLIC StatusbarController :
                            public css::frame::XStatusbarController,
                            public ::comphelper::OBaseMutex,
                            public ::cppu::OWeakObject
{
    public:
        StatusbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                             const css::uno::Reference< css::frame::XFrame >& xFrame,
                             const OUString& aCommandURL,
                             unsigned short       nID );
        StatusbarController();
        virtual ~StatusbarController();

        css::uno::Reference< css::frame::XFrame > getFrameInterface() const;
        css::uno::Reference< css::util::XURLTransformer > getURLTransformer() const;

        ::Rectangle getControlRect() const;

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XUpdatable
        virtual void SAL_CALL update() throw (css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XStatusbarController
        virtual sal_Bool SAL_CALL mouseButtonDown( const css::awt::MouseEvent& aMouseEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL mouseMove( const css::awt::MouseEvent& aMouseEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL mouseButtonUp( const css::awt::MouseEvent& aMouseEvent ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL command( const css::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       sal_Bool bMouseEvent,
                                       const css::uno::Any& aData ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL paint( const css::uno::Reference< css::awt::XGraphics >& xGraphics,
                                     const css::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nStyle ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL click( const css::awt::Point& aPos ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL doubleClick( const css::awt::Point& aPos ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        struct Listener
        {
            Listener( const css::util::URL& rURL, const css::uno::Reference< css::frame::XDispatch >& rDispatch ) :
                aURL( rURL ), xDispatch( rDispatch ) {}

            css::util::URL                               aURL;
            css::uno::Reference< css::frame::XDispatch > xDispatch;
        };

        typedef std::unordered_map< OUString,
                                    css::uno::Reference< css::frame::XDispatch >,
                                    OUStringHash,
                                    std::equal_to< OUString > > URLToDispatchMap;

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
