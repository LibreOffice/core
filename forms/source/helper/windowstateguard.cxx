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

#include <windowstateguard.hxx>
#include <frm_strings.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/awt/XWindowListener2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/diagnose_ex.hxx>


namespace frm
{


    using ::com::sun::star::awt::XWindowListener2;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XWindow2;
    using ::com::sun::star::awt::WindowEvent;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::awt::XControlModel;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;

    typedef ::cppu::WeakImplHelper <   XWindowListener2
                                    >   WindowStateGuard_Impl_Base;
    class WindowStateGuard_Impl : public WindowStateGuard_Impl_Base
    {
    private:
        ::osl::Mutex                m_aMutex;
        Reference< XWindow2 >       m_xWindow;
        Reference< XPropertySet >   m_xModelProps;

    public:
        /** constructs the instance
            @param _rxWindow
                the window at which to listen. Must not be <NULL/>.
            @param _rxModel
                the model which acts as the reference for the states to be enforced. Must not be <NULL/>.
        */
        WindowStateGuard_Impl( const Reference< XWindow2 >& _rxWindow, const Reference< XPropertySet >& _rxMdelProps );

        void dispose();

    protected:
        // XWindowListener2
        virtual void SAL_CALL windowEnabled( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowDisabled( const css::lang::EventObject& e ) override;

        // XWindowListener
        virtual void SAL_CALL windowResized( const css::awt::WindowEvent& e ) override;
        virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& e ) override;
        virtual void SAL_CALL windowShown( const css::lang::EventObject& e ) override;
        virtual void SAL_CALL windowHidden( const css::lang::EventObject& e ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    private:
        /** ensures that the window's Enabled state matches what is described at the model
            @precond
                our mutex is locked
        */
        void    impl_ensureEnabledState_nothrow_nolck();
    };


    WindowStateGuard_Impl::WindowStateGuard_Impl( const Reference< XWindow2 >& _rxWindow, const Reference< XPropertySet >& _rxMdelProps )
        :m_xWindow( _rxWindow )
        ,m_xModelProps( _rxMdelProps )
    {
        if ( !m_xWindow.is() )
            throw IllegalArgumentException(u"no window supplied"_ustr, *this, 0);
        if ( !m_xModelProps.is() )
            throw IllegalArgumentException(u"no property set supplied"_ustr, *this, 1);

        osl_atomic_increment( &m_refCount );
        {
            m_xWindow->addWindowListener( this );
        }
        osl_atomic_decrement( &m_refCount );
    }


    void WindowStateGuard_Impl::dispose()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xWindow.is() )
            // already disposed
            return;

        m_xWindow->removeWindowListener( this );
        m_xWindow.clear();
    }


    void WindowStateGuard_Impl::impl_ensureEnabledState_nothrow_nolck()
    {
        try
        {
            Reference< XWindow2 > xWindow;
            Reference< XPropertySet > xModelProps;
            bool bShouldBeEnabled = false;
            {
                ::osl::MutexGuard aGuard( m_aMutex );
                if ( !m_xWindow.is() || !m_xModelProps.is() )
                    return;
                xWindow = m_xWindow;
                xModelProps = m_xModelProps;
            }
            // fdo#42157: do not lock m_aMutex to prevent deadlock
            bool const bEnabled = xWindow->isEnabled();
            OSL_VERIFY( xModelProps->getPropertyValue( PROPERTY_ENABLED )
                        >>= bShouldBeEnabled );

            if ( !bShouldBeEnabled && bEnabled )
                xWindow->setEnable( false );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("forms.helper");
        }
    }


    void SAL_CALL WindowStateGuard_Impl::windowEnabled( const EventObject& /*e*/ )
    {
        impl_ensureEnabledState_nothrow_nolck();
    }


    void SAL_CALL WindowStateGuard_Impl::windowDisabled( const EventObject& /*e*/ )
    {
        impl_ensureEnabledState_nothrow_nolck();
    }


    void SAL_CALL WindowStateGuard_Impl::windowResized( const WindowEvent& /*e*/ )
    {
        // not interested in
    }


    void SAL_CALL WindowStateGuard_Impl::windowMoved( const WindowEvent& /*e*/ )
    {
        // not interested in
    }


    void SAL_CALL WindowStateGuard_Impl::windowShown( const EventObject& /*e*/ )
    {
        // not interested in
    }


    void SAL_CALL WindowStateGuard_Impl::windowHidden( const EventObject& /*e*/ )
    {
        // not interested in
    }


    void SAL_CALL WindowStateGuard_Impl::disposing( const EventObject& Source )
    {
        OSL_ENSURE( Source.Source == m_xWindow, "WindowStateGuard_Impl::disposing: where does this come from?" );
        dispose();
    }

    WindowStateGuard::WindowStateGuard()
    {
    }


    WindowStateGuard::~WindowStateGuard()
    {
    }


    void WindowStateGuard::attach( const Reference< XWindow2 >& _rxWindow, const Reference< XControlModel >& _rxModel )
    {
        if ( m_pImpl.is() )
        {
            m_pImpl->dispose();
            m_pImpl = nullptr;
        }

        Reference< XPropertySet > xModelProps( _rxModel, UNO_QUERY );
        OSL_ENSURE( xModelProps.is() || !_rxModel.is(), "WindowStateGuard::attach: a model which is no XPropertySet?" );
        if ( _rxWindow.is() && xModelProps.is() )
            m_pImpl = new WindowStateGuard_Impl( _rxWindow, xModelProps );
    }


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
