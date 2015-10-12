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
                            public ::com::sun::star::frame::XStatusbarController,
                            public ::comphelper::OBaseMutex,
                            public ::cppu::OWeakObject
{
    public:
        StatusbarController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                             const OUString& aCommandURL,
                             unsigned short       nID );
        StatusbarController();
        virtual ~StatusbarController();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > getFrameInterface() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > getURLTransformer() const;

        ::Rectangle getControlRect() const;

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XUpdatable
        virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XStatusbarController
        virtual sal_Bool SAL_CALL mouseButtonDown( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL mouseMove( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL mouseButtonUp( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL command( const ::com::sun::star::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       sal_Bool bMouseEvent,
                                       const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
                                     const ::com::sun::star::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nStyle ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL click( const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL doubleClick( const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    protected:
        struct Listener
        {
            Listener( const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& rDispatch ) :
                aURL( rURL ), xDispatch( rDispatch ) {}

            ::com::sun::star::util::URL aURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDispatch;
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
        void execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );
        void execute( const OUString& aCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );

        bool                                                                                m_bInitialized : 1,
                                                                                            m_bDisposed : 1;
        unsigned short                                                                      m_nID;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                  m_xParentWindow;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        OUString                                                                       m_aCommandURL;
        URLToDispatchMap                                                                    m_aListenerMap;
        ::cppu::OMultiTypeInterfaceContainerHelper                                          m_aListenerContainer;   /// container for ALL Listener
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLTransformer;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XStatusbarItem >           m_xStatusbarItem;
};

}

#endif // INCLUDED_SVTOOLS_STATUSBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
