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

#include <osl/mutex.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/form/XLoadable.hpp>
#include <rtl/ref.hxx>


namespace bib
{


    class OComponentAdapterBase;

    class OComponentListener
    {
        friend class OComponentAdapterBase;

    private:
        rtl::Reference<OComponentAdapterBase>  m_xAdapter;
        ::osl::Mutex&                          m_rMutex;
    protected:
        explicit OComponentListener( ::osl::Mutex& _rMutex )
            :m_rMutex( _rMutex )
        {
        }

        virtual ~OComponentListener();

    protected:
        void setAdapter( OComponentAdapterBase* _pAdapter );
    };

    class OComponentAdapterBase
    {
        friend class OComponentListener;

    private:
        css::uno::Reference< css::lang::XComponent >  m_xComponent;
        OComponentListener*                 m_pListener;
        bool                                m_bListening    : 1;

        // impl method for dispose - virtual, 'cause you at least need to remove the listener from the broadcaster
        virtual void disposing() = 0;

    protected:
        // attribute access for derivees
        const css::uno::Reference< css::lang::XComponent >&
                                getComponent() const    { return m_xComponent; }
        OComponentListener*     getListener()           { return m_pListener; }

        // to be called by derivees which started listening at the component
        virtual void    startComponentListening() = 0;

        virtual ~OComponentAdapterBase();

    public:
        explicit OComponentAdapterBase(
            const css::uno::Reference< css::lang::XComponent >& _rxComp
        );

        // late construction
        // can be called from within you ctor, to have you're object fully initialized at the moment of
        // the call (which would not be the case when calling this ctor)
        void Init( OComponentListener* _pListener );

    // base for ref-counting, implemented by OComponentAdapter
        virtual void SAL_CALL acquire(  ) noexcept = 0;
        virtual void SAL_CALL release(  ) noexcept = 0;

        /// dispose the object - stop listening and such
        void dispose();

    protected:
    // XEventListener
        /// @throws css::uno::RuntimeException
        virtual void SAL_CALL disposing( const  css::lang::EventObject& Source );
    };

    class OLoadListener : public OComponentListener
    {
        friend class OLoadListenerAdapter;

    protected:
        explicit OLoadListener( ::osl::Mutex& _rMutex ) : OComponentListener( _rMutex ) { }

    // XLoadListener equivalents
        virtual void _loaded( const css::lang::EventObject& aEvent ) = 0;
        virtual void _unloading( const css::lang::EventObject& aEvent ) = 0;
        virtual void _reloading( const css::lang::EventObject& aEvent ) = 0;
        virtual void _reloaded( const css::lang::EventObject& aEvent ) = 0;
    };

    typedef ::cppu::WeakImplHelper< css::form::XLoadListener >    OLoadListenerAdapter_Base;
    class OLoadListenerAdapter
        :public OLoadListenerAdapter_Base
        ,public OComponentAdapterBase
    {
    protected:
        OLoadListener*      getLoadListener( )  { return static_cast< OLoadListener* >( getListener() ); }

    protected:
        virtual void    disposing() override;
        virtual void    startComponentListening() override;

    public:
        explicit OLoadListenerAdapter(
            const css::uno::Reference< css::form::XLoadable >& _rxLoadable
        );


        virtual void SAL_CALL acquire(  ) noexcept override;
        virtual void SAL_CALL release(  ) noexcept override;

    protected:
    // XEventListener
        virtual void SAL_CALL disposing( const  css::lang::EventObject& _rSource ) override;

    // XLoadListener
        virtual void SAL_CALL loaded( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL unloading( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL unloaded( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL reloading( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL reloaded( const css::lang::EventObject& aEvent ) override;
    };


}   // namespace bib


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
