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

#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XWeak.hpp>
#include <cppuhelper/weakref.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{



    //= OWeakListenerAdapterBase

    /** (the base for) an adapter which allows to add as listener to a foreign component, without
        being held hard.

        <p>The idea is that this adapter is added as listener to a foreign component, which usually
        holds it's listener hard. The adapter itself knows the real listener as weak reference,
        thus not affecting its life time.</p>
    */
    class OWeakListenerAdapterBase : public OBaseMutex
    {
    private:
        ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >
                m_aListener;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                m_xBroadcaster;

    protected:
        inline ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                getListener( ) const
        {
            return m_aListener.get();
        }

        inline const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&
                getBroadcaster( ) const
        {
            return m_xBroadcaster;
        }

        inline void resetListener( )
        {
            m_aListener.clear();
        }


    protected:
        inline OWeakListenerAdapterBase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak >& _rxListener,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxBroadcaster
        )
            :m_aListener    (  _rxListener )
            ,m_xBroadcaster ( _rxBroadcaster )
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
            :public ::cppu::WeakComponentImplHelper< LISTENER >
            ,public OWeakListenerAdapterBase
    {
    protected:
        /** ctor
            <p>Note that derived classes still need to add themself as listener to the broadcaster,
            as this can't be done in a generic way</p>
        */
        OWeakListenerAdapter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak >& _rxListener,
            const ::com::sun::star::uno::Reference< BROADCASTER >& _rxBroadcaster
        );

    protected:
        inline  ::com::sun::star::uno::Reference< LISTENER > getListener( ) const
        {
            return  ::com::sun::star::uno::Reference< LISTENER >( OWeakListenerAdapterBase::getListener(), ::com::sun::star::uno::UNO_QUERY );
        }

        // XEventListener overridables
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException) override;

    protected:
        // OComponentHelper overridables
        // to be overridden, again - the derived class should revoke the listener from the broadcaster
        virtual void SAL_CALL disposing( ) override = 0;
    };


    //= OWeakEventListenerAdapter

    typedef OWeakListenerAdapter    <   ::com::sun::star::lang::XComponent
                                    ,   ::com::sun::star::lang::XEventListener
                                    >   OWeakEventListenerAdapter_Base;
    /** the most simple listener adapter: for XEventListeners at XComponents
    */
    class COMPHELPER_DLLPUBLIC OWeakEventListenerAdapter : public OWeakEventListenerAdapter_Base
    {
    public:
        OWeakEventListenerAdapter(
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak > _rxListener,
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > _rxBroadcaster
        );

        // nothing to do except an own ctor - the forwarding of the "disposing" is already done
        // in the base class

    protected:
        using OWeakEventListenerAdapter_Base::disposing;
        virtual void SAL_CALL disposing( ) override;
    };


    //= OWeakListenerAdapter


    template< class BROADCASTER, class LISTENER >
    OWeakListenerAdapter< BROADCASTER, LISTENER >::OWeakListenerAdapter(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XWeak >& _rxListener,
        const ::com::sun::star::uno::Reference< BROADCASTER >& _rxBroadcaster
    )
        : ::cppu::WeakComponentImplHelper< LISTENER >( m_aMutex )
        , OWeakListenerAdapterBase( _rxListener, _rxBroadcaster )
    {
    }


    template< class BROADCASTER, class LISTENER >
    void SAL_CALL OWeakListenerAdapter< BROADCASTER, LISTENER >::disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException)
    {
        ::com::sun::star::uno::Reference< LISTENER > xListener( getListener() );
        if ( xListener.is() )
            xListener->disposing( _rSource );
    }


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_WEAKEVENTLISTENER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
