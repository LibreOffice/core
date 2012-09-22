/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "windowstateguard.hxx"
#include "frm_strings.hxx"

#include <com/sun/star/awt/XWindowListener2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase1.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace frm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::awt::XWindowListener2;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XWindow2;
    using ::com::sun::star::awt::WindowEvent;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::awt::XControlModel;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    /** === end UNO using === **/

    //====================================================================
    //= WindowStateGuard_Impl
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   XWindowListener2
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
        virtual void SAL_CALL windowEnabled( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowDisabled( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

        // XWindowListener
        virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** ensures that the window's Enabled state matches what is described at the model
            @precond
                our mutex is locked
        */
        void    impl_ensureEnabledState_nothrow_nolck();
    };

    //--------------------------------------------------------------------
    WindowStateGuard_Impl::WindowStateGuard_Impl( const Reference< XWindow2 >& _rxWindow, const Reference< XPropertySet >& _rxMdelProps )
        :m_xWindow( _rxWindow )
        ,m_xModelProps( _rxMdelProps )
    {
        if ( !m_xWindow.is() || !m_xModelProps.is() )
            throw RuntimeException();

        osl_atomic_increment( &m_refCount );
        {
            m_xWindow->addWindowListener( this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    //--------------------------------------------------------------------
    void WindowStateGuard_Impl::dispose()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xWindow.is() )
            // already disposed
            return;

        m_xWindow->removeWindowListener( this );
        m_xWindow.clear();
    }

    //--------------------------------------------------------------------
    void WindowStateGuard_Impl::impl_ensureEnabledState_nothrow_nolck()
    {
        try
        {
            Reference< XWindow2 > xWindow;
            Reference< XPropertySet > xModelProps;
            sal_Bool bShouldBeEnabled = sal_False;
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
                xWindow->setEnable( sal_False );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::windowEnabled( const EventObject& /*e*/ ) throw (RuntimeException)
    {
        impl_ensureEnabledState_nothrow_nolck();
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::windowDisabled( const EventObject& /*e*/ ) throw (RuntimeException)
    {
        impl_ensureEnabledState_nothrow_nolck();
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::windowResized( const WindowEvent& /*e*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::windowMoved( const WindowEvent& /*e*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::windowShown( const EventObject& /*e*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::windowHidden( const EventObject& /*e*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        OSL_ENSURE( Source.Source == m_xWindow, "WindowStateGuard_Impl::disposing: where does this come from?" );
        (void)Source;
        dispose();
    }

    //====================================================================
    //= WindowStateGuard
    //====================================================================
    //--------------------------------------------------------------------
    WindowStateGuard::WindowStateGuard()
    {
    }

    //--------------------------------------------------------------------
    WindowStateGuard::~WindowStateGuard()
    {
    }

    //--------------------------------------------------------------------
    void WindowStateGuard::attach( const Reference< XWindow2 >& _rxWindow, const Reference< XControlModel >& _rxModel )
    {
        if ( m_pImpl.is() )
        {
            m_pImpl->dispose();
            m_pImpl = NULL;
        }

        Reference< XPropertySet > xModelProps( _rxModel, UNO_QUERY );
        OSL_ENSURE( xModelProps.is() || !_rxModel.is(), "WindowStateGuard::attach: a model which is no XPropertySet?" );
        if ( _rxWindow.is() && xModelProps.is() )
            m_pImpl = new WindowStateGuard_Impl( _rxWindow, xModelProps );
    }

//........................................................................
} // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
