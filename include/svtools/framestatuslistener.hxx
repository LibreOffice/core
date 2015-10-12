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

#ifndef INCLUDED_SVTOOLS_FRAMESTATUSLISTENER_HXX
#define INCLUDED_SVTOOLS_FRAMESTATUSLISTENER_HXX

#include <svtools/svtdllapi.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <unordered_map>

namespace svt
{

class SVT_DLLPUBLIC FrameStatusListener : public ::com::sun::star::frame::XStatusListener,
                            public ::com::sun::star::frame::XFrameActionListener,
                            public ::com::sun::star::lang::XComponent,
                            public ::comphelper::OBaseMutex,
                            public ::cppu::OWeakObject
{
    public:
        FrameStatusListener( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );
        virtual ~FrameStatusListener();

        // methods to support status forwarder, known by the old sfx2 toolbox controller implementation
        void addStatusListener( const OUString& aCommandURL );
        void bindListener();
        void unbindListener();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override = 0;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const com::sun::star::frame::FrameActionEvent& Action ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

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

        bool                                                                                m_bInitialized : 1,
                                                                                            m_bDisposed : 1;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        URLToDispatchMap                                                                    m_aListenerMap;
};

}

#endif // INCLUDED_SVTOOLS_FRAMESTATUSLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
