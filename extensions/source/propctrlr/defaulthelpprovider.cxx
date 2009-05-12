/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: defaulthelpprovider.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "defaulthelpprovider.hxx"
#include "pcrcommon.hxx"
#include "modulepcr.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
/** === end UNO includes === **/

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <tools/diagnose_ex.h>

//------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_DefaultHelpProvider()
{
    ::pcr::OAutoRegistration< ::pcr::DefaultHelpProvider > aAutoRegistration;
}

//........................................................................
namespace pcr
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::inspection::XObjectInspectorUI;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::awt::XVclWindowPeer;
    /** === end UNO using === **/

    //====================================================================
    //= DefaultHelpProvider
    //====================================================================
    //--------------------------------------------------------------------
    DefaultHelpProvider::DefaultHelpProvider( const Reference< XComponentContext >& _rxContext )
        :m_aContext( _rxContext )
        ,m_bConstructed( false )
    {
    }

    //--------------------------------------------------------------------
    DefaultHelpProvider::~DefaultHelpProvider()
    {
    }

    //------------------------------------------------------------------------
    ::rtl::OUString DefaultHelpProvider::getImplementationName_static(  ) throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( "org.openoffice.comp.extensions.DefaultHelpProvider");
    }

    //------------------------------------------------------------------------
    Sequence< ::rtl::OUString > DefaultHelpProvider::getSupportedServiceNames_static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString::createFromAscii( "com.sun.star.inspection.DefaultHelpProvider" );
        return aSupported;
    }

    //------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL DefaultHelpProvider::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *new DefaultHelpProvider( _rxContext );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DefaultHelpProvider::focusGained( const Reference< XPropertyControl >& _Control ) throw (RuntimeException)
    {
        if ( !m_xInspectorUI.is() )
            throw RuntimeException( ::rtl::OUString(), *this );

        try
        {
            m_xInspectorUI->setHelpSectionText( impl_getHelpText_nothrow( _Control ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL DefaultHelpProvider::valueChanged( const Reference< XPropertyControl >& /*_Control*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL DefaultHelpProvider::initialize( const Sequence< Any >& _arguments ) throw (Exception, RuntimeException)
    {
        if ( m_bConstructed )
            throw AlreadyInitializedException();

        StlSyntaxSequence< Any > arguments( _arguments );
        if ( arguments.size() == 1 )
        {   // constructor: "create( XObjectInspectorUI )"
            Reference< XObjectInspectorUI > xUI( arguments[0], UNO_QUERY );
            create( xUI );
            return;
        }

        throw IllegalArgumentException( ::rtl::OUString(), *this, 0 );
    }

    //--------------------------------------------------------------------
    void DefaultHelpProvider::create( const Reference< XObjectInspectorUI >& _rxUI )
    {
        if ( !_rxUI.is() )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

        try
        {
            m_xInspectorUI = _rxUI;
            m_xInspectorUI->registerControlObserver( this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        m_bConstructed = true;
    }

    //--------------------------------------------------------------------
    Window* DefaultHelpProvider::impl_getVclControlWindow_nothrow( const Reference< XPropertyControl >& _rxControl )
    {
        Window* pControlWindow = NULL;
        OSL_PRECOND( _rxControl.is(), "DefaultHelpProvider::impl_getVclControlWindow_nothrow: illegal control!" );
        if ( !_rxControl.is() )
            return pControlWindow;

        try
        {
            Reference< XWindow > xControlWindow( _rxControl->getControlWindow(), UNO_QUERY_THROW );
            pControlWindow = VCLUnoHelper::GetWindow( xControlWindow );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return pControlWindow;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString DefaultHelpProvider::impl_getHelpText_nothrow( const Reference< XPropertyControl >& _rxControl )
    {
        ::rtl::OUString sHelpText;
        OSL_PRECOND( _rxControl.is(), "DefaultHelpProvider::impl_getHelpText_nothrow: illegal control!" );
        if ( !_rxControl.is() )
            return sHelpText;

        Window* pControlWindow( impl_getVclControlWindow_nothrow( _rxControl ) );
        OSL_ENSURE( pControlWindow, "DefaultHelpProvider::impl_getHelpText_nothrow: could not determine the VCL window!" );
        if ( !pControlWindow )
            return sHelpText;

        sHelpText = pControlWindow->GetHelpText();
        return sHelpText;
    }
//........................................................................
} // namespace pcr
//........................................................................
