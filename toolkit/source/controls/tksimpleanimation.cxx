/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tksimpleanimation.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 13:53:28 $
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

#ifndef TOOLKIT_CONTROLS_TKSIMPLEANIMATION_HXX
#include "toolkit/controls/tksimpleanimation.hxx"
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
    //= UnoSimpleAnimationControlModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoSimpleAnimationControlModel::UnoSimpleAnimationControlModel()
    {
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_REPEAT );
        ImplRegisterProperty( BASEPROPERTY_STEP_TIME );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoSimpleAnimationControlModel::getServiceName( ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControlModel );
    }

    //--------------------------------------------------------------------
    Any UnoSimpleAnimationControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControl ) );

        case BASEPROPERTY_STEP_TIME:
            return makeAny( (sal_Int32) 100 );

        case BASEPROPERTY_REPEAT:
            return makeAny( (sal_Bool)sal_True );

        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
        }
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoSimpleAnimationControlModel::getInfoHelper()
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
    Reference< XPropertySetInfo > UnoSimpleAnimationControlModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoSimpleAnimationControlModel::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoSimpleAnimationControlModel" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoSimpleAnimationControlModel::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( UnoControlModel::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControlModel );
        return aServices;
    }

    //====================================================================
    //= UnoSimpleAnimationControl
    //====================================================================
    //--------------------------------------------------------------------
    UnoSimpleAnimationControl::UnoSimpleAnimationControl()
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoSimpleAnimationControl::GetComponentServiceName()
    {
        return ::rtl::OUString::createFromAscii( "SimpleAnimation" );
    }

    //--------------------------------------------------------------------
    Any UnoSimpleAnimationControl::queryAggregation( const Type & rType ) throw(RuntimeException)
    {
        Any aRet = UnoControlBase::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoSimpleAnimationControl_Base::queryInterface( rType );
        return aRet;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoSimpleAnimationControl, UnoControlBase, UnoSimpleAnimationControl_Base )

    //--------------------------------------------------------------------
    void UnoSimpleAnimationControl::dispose() throw(RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        UnoControl::dispose();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoSimpleAnimationControl::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoSimpleAnimationControl" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoSimpleAnimationControl::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( UnoControlBase::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControl );
        return aServices;
    }

    //--------------------------------------------------------------------
    void UnoSimpleAnimationControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::start() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        Reference< XSimpleAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::stop() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        Reference< XSimpleAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->stop();
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::setImageList( const Sequence< Reference< XGraphic > >& ImageList )
        throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        Reference< XSimpleAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->setImageList( ImageList );
    }

//........................................................................
}  // namespace toolkit
//........................................................................

