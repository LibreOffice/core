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
#include "toolkit/controls/tkthrobber.hxx"
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
    //= UnoThrobberControlModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoThrobberControlModel::UnoThrobberControlModel( const uno::Reference< lang::XMultiServiceFactory >& i_factory )
        :UnoControlModel( i_factory )
    {
        ImplRegisterProperty( BASEPROPERTY_BORDER );
        ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
        ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
        ImplRegisterProperty( BASEPROPERTY_HELPURL );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoThrobberControlModel::getServiceName( ) throw ( uno::RuntimeException )
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoThrobberControlModel );
    }

    //--------------------------------------------------------------------
    uno::Any UnoThrobberControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoThrobberControl ) );
        case BASEPROPERTY_BORDER:
            return uno::makeAny( (sal_Int16) 0 );
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
            uno::Sequence< sal_Int32 > aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    //--------------------------------------------------------------------
    uno::Reference< beans::XPropertySetInfo > UnoThrobberControlModel::getPropertySetInfo()
        throw( uno::RuntimeException )
    {
        static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoThrobberControlModel::getImplementationName()
        throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoThrobberControlModel" ) );
    }

    //--------------------------------------------------------------------
    uno::Sequence< ::rtl::OUString > SAL_CALL UnoThrobberControlModel::getSupportedServiceNames()
        throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aServices( UnoControlModel::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 2 );
        aServices[ aServices.getLength() - 2 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoThrobberControlModel );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName2_UnoThrobberControlModel );
        return aServices;
    }

    //====================================================================
    //= UnoThrobberControl
    //====================================================================
    //--------------------------------------------------------------------
    UnoThrobberControl::UnoThrobberControl( const uno::Reference< lang::XMultiServiceFactory >& i_factory )
        :UnoControlBase( i_factory )
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoThrobberControl::GetComponentServiceName()
    {
        return ::rtl::OUString::createFromAscii( "Throbber" );
    }

    //--------------------------------------------------------------------
    uno::Any UnoThrobberControl::queryAggregation( const uno::Type & rType ) throw( uno::RuntimeException )
    {
        uno::Any aRet = UnoControlBase::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoThrobberControl_Base::queryInterface( rType );
        return aRet;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoThrobberControl, UnoControlBase, UnoThrobberControl_Base )

    //--------------------------------------------------------------------
    void UnoThrobberControl::dispose() throw( uno::RuntimeException )
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );

        UnoControl::dispose();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoThrobberControl::getImplementationName()
        throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoThrobberControl" ) );
    }

    //--------------------------------------------------------------------
    uno::Sequence< ::rtl::OUString > SAL_CALL UnoThrobberControl::getSupportedServiceNames()
        throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aServices( UnoControlBase::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 2 );
        aServices[ aServices.getLength() - 2 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoThrobberControl );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName2_UnoThrobberControl );
        return aServices;
    }

    //--------------------------------------------------------------------
    void UnoThrobberControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit,
                                         const uno::Reference< awt::XWindowPeer >  & rParentPeer )
        throw( uno::RuntimeException )
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoThrobberControl::start() throw ( uno::RuntimeException )
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        uno::Reference< XThrobber > xAnimation( getPeer(), uno::UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->start();
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoThrobberControl::stop() throw ( uno::RuntimeException )
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        uno::Reference< XThrobber > xAnimation( getPeer(), uno::UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->stop();
    }

//........................................................................
}  // namespace toolkit
//........................................................................

