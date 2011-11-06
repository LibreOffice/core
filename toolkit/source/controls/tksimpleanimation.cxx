/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include "toolkit/controls/tksimpleanimation.hxx"
#include "toolkit/helper/property.hxx"
#include "toolkit/helper/unopropertyarrayhelper.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star;

    //====================================================================
    //= UnoSimpleAnimationControlModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoSimpleAnimationControlModel::UnoSimpleAnimationControlModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
        :UnoControlModel( i_factory )
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
        aServices.realloc( aServices.getLength() + 2 );
        aServices[ aServices.getLength() - 2 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControlModel );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName2_UnoSimpleAnimationControlModel );
        return aServices;
    }

    //====================================================================
    //= UnoSimpleAnimationControl
    //====================================================================
    //--------------------------------------------------------------------
    UnoSimpleAnimationControl::UnoSimpleAnimationControl( const uno::Reference< lang::XMultiServiceFactory >& i_factory )
        :UnoSimpleAnimationControl_Base( i_factory )
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoSimpleAnimationControl::GetComponentServiceName()
    {
        return ::rtl::OUString::createFromAscii( "SimpleAnimation" );
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
        uno::Sequence< ::rtl::OUString > aServices( UnoSimpleAnimationControl_Base::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSimpleAnimationControl );
        return aServices;
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::start() throw ( uno::RuntimeException )
    {
        uno::Reference< XSimpleAnimation > xAnimation;
        {
            ::osl::MutexGuard aGuard( GetMutex() );
            xAnimation.set( getPeer(), uno::UNO_QUERY );
        }
        if ( xAnimation.is() )
            xAnimation->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::stop() throw ( uno::RuntimeException )
    {
        uno::Reference< XSimpleAnimation > xAnimation;
        {
            ::osl::MutexGuard aGuard( GetMutex() );
            xAnimation.set( getPeer(), uno::UNO_QUERY );
        }
        if ( xAnimation.is() )
            xAnimation->stop();
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSimpleAnimationControl::setImageList( const uno::Sequence< uno::Reference< graphic::XGraphic > >& ImageList )
        throw ( uno::RuntimeException )
    {
        uno::Reference< XSimpleAnimation > xAnimation;
        {
            ::osl::MutexGuard aGuard( GetMutex() );
            xAnimation.set( getPeer(), uno::UNO_QUERY );
        }
        if ( xAnimation.is() )
            xAnimation->setImageList( ImageList );
    }

//........................................................................
}  // namespace toolkit
//........................................................................

