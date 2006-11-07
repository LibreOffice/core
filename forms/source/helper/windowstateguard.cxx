/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: windowstateguard.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:47:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#ifndef FORMS_WINDOWSTATEGUARD_HXX
#include "windowstateguard.hxx"
#endif
#ifndef FRM_STRINGS_HXX
#include "frm_strings.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER2_HPP_
#include <com/sun/star/awt/XWindowListener2.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

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
        void    impl_ensureEnabledState_nothrow() const;
    };

    //--------------------------------------------------------------------
    WindowStateGuard_Impl::WindowStateGuard_Impl( const Reference< XWindow2 >& _rxWindow, const Reference< XPropertySet >& _rxMdelProps )
        :m_xWindow( _rxWindow )
        ,m_xModelProps( _rxMdelProps )
    {
        if ( !m_xWindow.is() || !m_xModelProps.is() )
            throw RuntimeException();

        osl_incrementInterlockedCount( &m_refCount );
        {
            m_xWindow->addWindowListener( this );
        }
        osl_decrementInterlockedCount( &m_refCount );
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
    void WindowStateGuard_Impl::impl_ensureEnabledState_nothrow() const
    {
        try
        {
            sal_Bool bEnabled = m_xWindow->isEnabled();
            sal_Bool bShouldBeEnabled = sal_False;
            OSL_VERIFY( m_xModelProps->getPropertyValue( PROPERTY_ENABLED ) >>= bShouldBeEnabled );

            if ( !bShouldBeEnabled && bEnabled )
                m_xWindow->setEnable( sal_False );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::windowEnabled( const EventObject& /*e*/ ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensureEnabledState_nothrow();
    }

    //--------------------------------------------------------------------
    void SAL_CALL WindowStateGuard_Impl::windowDisabled( const EventObject& /*e*/ ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_ensureEnabledState_nothrow();
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

