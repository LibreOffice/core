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

#ifndef INCLUDED_COMPHELPER_WEAKEVENTLISTENER_HXX
#define INCLUDED_COMPHELPER_WEAKEVENTLISTENER_HXX

#include <config_options.h>
#include <comphelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <comphelper/comphelperdllapi.h>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <utility>

namespace com::sun::star::uno { class XWeak; }

namespace comphelper
{


    //= OWeakListenerAdapterBase

    /** (the base for) an adapter which allows to add as listener to a foreign component, without
        being held hard.

        <p>The idea is that this adapter is added as listener to a foreign component, which usually
        holds it's listener hard. The adapter itself knows the real listener as weak reference,
        thus not affecting its life time.</p>
    */
    class OWeakListenerAdapterBase
    {
    private:
        css::uno::WeakReference< css::uno::XInterface >
                m_aListener;
        css::uno::Reference< css::uno::XInterface >
                m_xBroadcaster;

    protected:
        css::uno::Reference< css::uno::XInterface >
                getListener( ) const
        {
            return m_aListener.get();
        }

        const css::uno::Reference< css::uno::XInterface >&
                getBroadcaster( ) const
        {
            return m_xBroadcaster;
        }

        void resetListener( )
        {
            m_aListener.clear();
        }


    protected:
        OWeakListenerAdapterBase(
            const css::uno::Reference< css::uno::XWeak >& _rxListener,
            css::uno::Reference< css::uno::XInterface > _xBroadcaster
        )
            :m_aListener    (  _rxListener )
            ,m_xBroadcaster (std::move( _xBroadcaster ))
        {
        }

    protected:
        virtual ~OWeakListenerAdapterBase();
    };


    //= OWeakListenerAdapter

    template< class BROADCASTER, class LISTENER >
    /** yet another base for weak listener adapters, this time with some type safety

        <p>Note that derived classes need to overwrite all virtual methods of their interface
        except XEventListener::disposing, and forward it to their master listener.</p>

        <p>Additionally, derived classes need to add themself as listener to the broadcaster,
        as this can't be done in a generic way</p>
    */
    class OWeakListenerAdapter
            :public ::comphelper::WeakComponentImplHelper< LISTENER >
            ,public OWeakListenerAdapterBase
    {
    protected:
        /** ctor
            <p>Note that derived classes still need to add themself as listener to the broadcaster,
            as this can't be done in a generic way</p>
        */
        OWeakListenerAdapter(
            const css::uno::Reference< css::uno::XWeak >& _rxListener,
            const css::uno::Reference< BROADCASTER >& _rxBroadcaster
        );

    protected:
        css::uno::Reference< LISTENER > getListener( ) const
        {
            return  css::uno::Reference< LISTENER >( OWeakListenerAdapterBase::getListener(), css::uno::UNO_QUERY );
        }

        // XEventListener overridables
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override final;

    protected:
        // OComponentHelper overridables
        // to be overridden, again - the derived class should revoke the listener from the broadcaster
        virtual void disposing( std::unique_lock<std::mutex>& rGuard ) override = 0;
    };


    //= OWeakEventListenerAdapter

    typedef OWeakListenerAdapter    <   css::lang::XComponent
                                    ,   css::lang::XEventListener
                                    >   OWeakEventListenerAdapter_Base;
    /** the most simple listener adapter: for XEventListeners at XComponents
    */
    class UNLESS_MERGELIBS(COMPHELPER_DLLPUBLIC) OWeakEventListenerAdapter final : public OWeakEventListenerAdapter_Base
    {
    public:
        OWeakEventListenerAdapter(
            css::uno::Reference< css::uno::XWeak > const & _rxListener,
            css::uno::Reference< css::lang::XComponent > const & _rxBroadcaster
        );

        // nothing to do except an own ctor - the forwarding of the "disposing" is already done
        // in the base class

    private:
        using OWeakEventListenerAdapter_Base::disposing;
        virtual void disposing( std::unique_lock<std::mutex>& rGuard ) override;
    };


    //= OWeakListenerAdapter


    template< class BROADCASTER, class LISTENER >
    OWeakListenerAdapter< BROADCASTER, LISTENER >::OWeakListenerAdapter(
        const css::uno::Reference< css::uno::XWeak >& _rxListener,
        const css::uno::Reference< BROADCASTER >& _rxBroadcaster
    )
        : OWeakListenerAdapterBase( _rxListener, _rxBroadcaster )
    {
    }


    template< class BROADCASTER, class LISTENER >
    void SAL_CALL OWeakListenerAdapter< BROADCASTER, LISTENER >::disposing( const css::lang::EventObject& _rSource )
    {
        css::uno::Reference< LISTENER > xListener( getListener() );
        if ( xListener.is() )
            xListener->disposing( _rSource );
    }


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_WEAKEVENTLISTENER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
