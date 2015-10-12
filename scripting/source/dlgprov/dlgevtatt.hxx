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

#ifndef INCLUDED_SCRIPTING_SOURCE_DLGPROV_DLGEVTATT_HXX
#define INCLUDED_SCRIPTING_SOURCE_DLGPROV_DLGEVTATT_HXX

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

namespace dlgprov
{
    // class DialogEventsAttacherImpl
    typedef std::unordered_map< OUString,
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >,
        OUStringHash,
        std::equal_to< OUString > > ListenerHash;

    typedef ::cppu::WeakImplHelper<
        ::com::sun::star::script::XScriptEventsAttacher > DialogEventsAttacherImpl_BASE;


    class DialogEventsAttacherImpl : public DialogEventsAttacherImpl_BASE
    {
    private:
        bool mbUseFakeVBAEvents;
        ListenerHash listernersForTypes;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacher > m_xEventAttacher;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener > getScriptListenerForKey( const OUString& sScriptName ) throw ( ::com::sun::star::uno::RuntimeException );
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptEventsSupplier > getFakeVbaEventsSupplier( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl, OUString& sCodeName );
        void nestedAttachEvents( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& Objects, const ::com::sun::star::uno::Any& Helper, OUString& sDialogCodeName );
        void attachEventsToControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& xControl, const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptEventsSupplier >& events, const ::com::sun::star::uno::Any& Helper  );
    public:
        DialogEventsAttacherImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
             const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel,
             const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& xControl,
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xHandler,
             const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess >& xIntrospect,
             bool bProviderMode,
             const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& xRTLListener ,const OUString& sDialogLibName );
        virtual ~DialogEventsAttacherImpl();

        // XScriptEventsAttacher
        virtual void SAL_CALL attachEvents( const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& Objects,
            const com::sun::star::uno::Reference<com::sun::star::script::XScriptListener>&,
            const ::com::sun::star::uno::Any& Helper )
            throw (::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::beans::IntrospectionException,
                   ::com::sun::star::script::CannotCreateAdapterException,
                   ::com::sun::star::lang::ServiceNotRegisteredException,
                   ::com::sun::star::uno::RuntimeException, std::exception) override;
    };



    // class DialogAllListenerImpl


    typedef ::cppu::WeakImplHelper<
        ::com::sun::star::script::XAllListener > DialogAllListenerImpl_BASE;


    class DialogAllListenerImpl : public DialogAllListenerImpl_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener > m_xScriptListener;
        OUString m_sScriptType;
        OUString m_sScriptCode;

        void firing_impl( const ::com::sun::star::script::AllEventObject& Event, ::com::sun::star::uno::Any* pRet );

    public:
        DialogAllListenerImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& rxListener,
            const OUString& rScriptType, const OUString& rScriptCode );
        virtual ~DialogAllListenerImpl();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XAllListener
        virtual void SAL_CALL firing( const ::com::sun::star::script::AllEventObject& Event )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL approveFiring( const ::com::sun::star::script::AllEventObject& Event )
            throw (::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    };



    // class DialogScriptListenerImpl


    typedef ::cppu::WeakImplHelper<
        ::com::sun::star::script::XScriptListener > DialogScriptListenerImpl_BASE;


    class DialogScriptListenerImpl : public DialogScriptListenerImpl_BASE
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        virtual void firing_impl( const ::com::sun::star::script::ScriptEvent& aScriptEvent, ::com::sun::star::uno::Any* pRet ) = 0;
    public:
        DialogScriptListenerImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext ) : m_xContext( rxContext ) {}
        virtual ~DialogScriptListenerImpl();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XScriptListener
        virtual void SAL_CALL firing( const ::com::sun::star::script::ScriptEvent& aScriptEvent )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL approveFiring( const ::com::sun::star::script::ScriptEvent& aScriptEvent )
            throw (::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    };



}   // namespace dlgprov


#endif // SCRIPTING_DLGEVT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
