/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkthrobber.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-18 14:44:21 $
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
#include "precompiled_toolkit.hxx"

#ifndef   TOOLKIT_CONTROLS_TKTHROBBER_HXX
#include "toolkit/controls/tkthrobber.hxx"
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include "toolkit/helper/property.hxx"
#endif
#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#include "toolkit/helper/unopropertyarrayhelper.hxx"
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::graphic;

    //====================================================================
    //= UnoThrobberControlModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoThrobberControlModel::UnoThrobberControlModel()
    {
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoThrobberControlModel::getServiceName( ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoThrobberControlModel );
    }

    //--------------------------------------------------------------------
    Any UnoThrobberControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoThrobberControl ) );
        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
        }
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoThrobberControlModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > UnoThrobberControlModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoThrobberControlModel::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoThrobberControlModel" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoThrobberControlModel::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( UnoControlModel::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoThrobberControlModel );
        return aServices;
    }

    //====================================================================
    //= UnoThrobberControl
    //====================================================================
    //--------------------------------------------------------------------
    UnoThrobberControl::UnoThrobberControl()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoThrobberControl::GetComponentServiceName()
    {
        return ::rtl::OUString::createFromAscii( "Throbber" );
    }

    //--------------------------------------------------------------------
    Any UnoThrobberControl::queryAggregation( const Type & rType ) throw(RuntimeException)
    {
        Any aRet = UnoControlBase::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoThrobberControl_Base::queryInterface( rType );
        return aRet;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoThrobberControl, UnoControlBase, UnoThrobberControl_Base )

    //--------------------------------------------------------------------
    void UnoThrobberControl::dispose() throw(RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        UnoControl::dispose();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoThrobberControl::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoThrobberControl" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoThrobberControl::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( UnoControlBase::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoThrobberControl );
        return aServices;
    }

    //--------------------------------------------------------------------
    void UnoThrobberControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoThrobberControl::start() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        Reference< XThrobber > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoThrobberControl::stop() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        Reference< XThrobber > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->stop();
    }

//........................................................................
}  // namespace toolkit
//........................................................................

