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

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XEventAttacher.hpp>
#include <com/sun/star/script/XScriptEventsAttacher.hpp>
#include <com/sun/star/script/XScriptListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/script/XScriptEventsSupplier.hpp>

#include <unordered_map>
#include <utility>

namespace dlgprov
{
    typedef std::unordered_map< OUString,
        css::uno::Reference< css::script::XScriptListener > > ListenerHash;

    typedef ::cppu::WeakImplHelper<
        css::script::XScriptEventsAttacher > DialogEventsAttacherImpl_BASE;


    class DialogEventsAttacherImpl : public DialogEventsAttacherImpl_BASE
    {
    private:
        bool mbUseFakeVBAEvents;
        ListenerHash listenersForTypes;
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
        css::uno::Reference< css::script::XEventAttacher > m_xEventAttacher;
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::script::XScriptListener > const & getScriptListenerForKey( const OUString& sScriptName );
        css::uno::Reference< css::script::XScriptEventsSupplier > getFakeVbaEventsSupplier( const css::uno::Reference< css::awt::XControl>& xControl, OUString const & sCodeName );
        void nestedAttachEvents( const css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >& Objects, const css::uno::Any& Helper, OUString& sDialogCodeName );
        void nestedAttachEvents( const css::uno::Reference< css::awt::XControl >& xControl, const css::uno::Any& Helper, OUString& sDialogCodeName );
        void attachEventsToControl( const css::uno::Reference< css::awt::XControl>& xControl, const css::uno::Reference< css::script::XScriptEventsSupplier >& events, const css::uno::Any& Helper  );
    public:
        DialogEventsAttacherImpl( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             const css::uno::Reference< css::frame::XModel >& xModel,
             const css::uno::Reference< css::awt::XControl >& xControl,
             const css::uno::Reference< css::uno::XInterface >& xHandler,
             const css::uno::Reference< css::beans::XIntrospectionAccess >& xIntrospect,
             bool bProviderMode,
             const css::uno::Reference< css::script::XScriptListener >& xRTLListener ,const OUString& sDialogLibName );
        virtual ~DialogEventsAttacherImpl() override;

        // XScriptEventsAttacher
        virtual void SAL_CALL attachEvents( const css::uno::Sequence<
            css::uno::Reference< css::uno::XInterface > >& Objects,
            const css::uno::Reference<css::script::XScriptListener>&,
            const css::uno::Any& Helper ) override;
    };




    typedef ::cppu::WeakImplHelper<
        css::script::XAllListener > DialogAllListenerImpl_BASE;


    class DialogAllListenerImpl : public DialogAllListenerImpl_BASE
    {
    private:
        css::uno::Reference< css::script::XScriptListener > m_xScriptListener;
        OUString m_sScriptType;
        OUString m_sScriptCode;

        void firing_impl( const css::script::AllEventObject& Event, css::uno::Any* pRet );

    public:
        DialogAllListenerImpl( const css::uno::Reference< css::script::XScriptListener >& rxListener,
            OUString sScriptType, OUString sScriptCode );
        virtual ~DialogAllListenerImpl() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XAllListener
        virtual void SAL_CALL firing( const css::script::AllEventObject& Event ) override;
        virtual css::uno::Any SAL_CALL approveFiring( const css::script::AllEventObject& Event ) override;
    };




    typedef ::cppu::WeakImplHelper<
        css::script::XScriptListener > DialogScriptListenerImpl_BASE;


    class DialogScriptListenerImpl : public DialogScriptListenerImpl_BASE
    {
    protected:
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
        virtual void firing_impl( const css::script::ScriptEvent& aScriptEvent, css::uno::Any* pRet ) = 0;
    public:
        explicit DialogScriptListenerImpl( css::uno::Reference< css::uno::XComponentContext > xContext ) : m_xContext(std::move( xContext )) {}
        virtual ~DialogScriptListenerImpl() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XScriptListener
        virtual void SAL_CALL firing( const css::script::ScriptEvent& aScriptEvent ) override;
        virtual css::uno::Any SAL_CALL approveFiring( const css::script::ScriptEvent& aScriptEvent ) override;
    };


}   // namespace dlgprov

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
