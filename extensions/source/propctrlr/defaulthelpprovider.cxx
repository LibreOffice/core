/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: defaulthelpprovider.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 11:58:16 $
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

#include "defaulthelpprovider.hxx"
#include "pcrcommon.hxx"
#include "modulepcr.hxx"

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UCB_ALREADYINITIALIZEDEXCEPTION_HPP_
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif
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
