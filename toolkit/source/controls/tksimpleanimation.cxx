/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tksimpleanimation.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:55:36 $
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

    using namespace ::com::sun::star;

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
    ::rtl::OUString UnoSimpleAnimationControlModel::getServiceName()
        throw( uno::RuntimeException )
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControlModel );
    }

    //--------------------------------------------------------------------
    uno::Any UnoSimpleAnimationControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControl ) );

        case BASEPROPERTY_STEP_TIME:
            return uno::makeAny( (sal_Int32) 100 );

        case BASEPROPERTY_REPEAT:
            return uno::makeAny( (sal_Bool)sal_True );

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
            uno::Sequence< sal_Int32 > aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    //--------------------------------------------------------------------
    uno::Reference< beans::XPropertySetInfo > UnoSimpleAnimationControlModel::getPropertySetInfo(  )
        throw( uno::RuntimeException )
    {
        static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoSimpleAnimationControlModel::getImplementationName()
        throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoSimpleAnimationControlModel" ) );
    }

    //--------------------------------------------------------------------
    uno::Sequence< ::rtl::OUString > SAL_CALL UnoSimpleAnimationControlModel::getSupportedServiceNames()
        throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aServices( UnoControlModel::getSupportedServiceNames() );
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
    uno::Any UnoSimpleAnimationControl::queryAggregation( const uno::Type & rType )
        throw( uno::RuntimeException )
    {
        uno::Any aRet = UnoControlBase::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoSimpleAnimationControl_Base::queryInterface( rType );
        return aRet;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoSimpleAnimationControl, UnoControlBase, UnoSimpleAnimationControl_Base )

    //--------------------------------------------------------------------
    void UnoSimpleAnimationControl::dispose() throw( uno::RuntimeException )
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        UnoControl::dispose();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoSimpleAnimationControl::getImplementationName()
        throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoSimpleAnimationControl" ) );
    }

    //--------------------------------------------------------------------
    uno::Sequence< ::rtl::OUString > SAL_CALL UnoSimpleAnimationControl::getSupportedServiceNames()
        throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aServices( UnoControlBase::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControl );
        return aServices;
    }

    //--------------------------------------------------------------------
    void UnoSimpleAnimationControl::createPeer( const uno::Reference< awt::XToolkit > &rxToolkit,
                                                const uno::Reference< awt::XWindowPeer >  &rParentPeer )
        throw( uno::RuntimeException )
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::start() throw ( uno::RuntimeException )
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        uno::Reference< XSimpleAnimation > xAnimation( getPeer(), uno::UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::stop() throw ( uno::RuntimeException )
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        uno::Reference< XSimpleAnimation > xAnimation( getPeer(), uno::UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->stop();
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::setImageList( const uno::Sequence< uno::Reference< graphic::XGraphic > >& ImageList )
        throw ( uno::RuntimeException )
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        uno::Reference< XSimpleAnimation > xAnimation( getPeer(), uno::UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->setImageList( ImageList );
    }

//........................................................................
}  // namespace toolkit
//........................................................................

