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
#include <connectivity/paramwrapper.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <tools/diagnose_ex.h>
#include <comphelper/enumhelper.hxx>

#define PROPERTY_ID_VALUE   1000


namespace dbtools
{
namespace param
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::sdbc::XParameters;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::XWeak;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XFastPropertySet;
    using ::com::sun::star::beans::XMultiPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::lang::IndexOutOfBoundsException;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::sdb::XSingleSelectQueryAnalyzer;
    using ::com::sun::star::sdb::XParametersSupplier;
    using ::com::sun::star::lang::DisposedException;

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;
    namespace DataType = ::com::sun::star::sdbc::DataType;

    ParameterWrapper::ParameterWrapper( const Reference< XPropertySet >& _rxColumn )
        :PropertyBase( m_aBHelper )
        ,m_xDelegator( _rxColumn )
    {
        if ( m_xDelegator.is() )
            m_xDelegatorPSI = m_xDelegator->getPropertySetInfo();
        if ( !m_xDelegatorPSI.is() )
            throw RuntimeException();
    }


    ParameterWrapper::ParameterWrapper( const Reference< XPropertySet >& _rxColumn,
            const Reference< XParameters >& _rxAllParameters, const ::std::vector< sal_Int32 >& _rIndexes )
        :PropertyBase( m_aBHelper )
        ,m_aIndexes( _rIndexes )
        ,m_xDelegator( _rxColumn )
        ,m_xValueDestination( _rxAllParameters )
    {
        if ( m_xDelegator.is() )
            m_xDelegatorPSI = m_xDelegator->getPropertySetInfo();
        if ( !m_xDelegatorPSI.is() )
            throw RuntimeException();

        OSL_ENSURE( !m_aIndexes.empty(), "ParameterWrapper::ParameterWrapper: sure about the indexes?" );
    }


    ParameterWrapper::~ParameterWrapper()
    {
    }


    IMPLEMENT_FORWARD_REFCOUNT( ParameterWrapper, UnoBase )

    css::uno::Any ParameterWrapper::queryInterface(css::uno::Type const & aType)
    {
        css::uno::Any a(UnoBase::queryInterface(aType));
        if (!a.hasValue()) {
            a = PropertyBase::queryInterface(aType);
            if (!a.hasValue()
                && aType == cppu::UnoType<css::lang::XTypeProvider>::get())
            {
                a <<= css::uno::Reference<css::lang::XTypeProvider>(this);
            }
        }
        return a;
    }


    Sequence< Type > SAL_CALL ParameterWrapper::getTypes(   )
    {
        Sequence< Type > aTypes( 5 );
        aTypes[ 0 ] = cppu::UnoType<XWeak>::get();
        aTypes[ 1 ] = cppu::UnoType<XTypeProvider>::get();
        aTypes[ 2 ] = cppu::UnoType<XPropertySet>::get();
        aTypes[ 3 ] = cppu::UnoType<XFastPropertySet>::get();
        aTypes[ 4 ] = cppu::UnoType<XMultiPropertySet>::get();
        return aTypes;
    }


    IMPLEMENT_GET_IMPLEMENTATION_ID( ParameterWrapper )


    OUString ParameterWrapper::impl_getPseudoAggregatePropertyName( sal_Int32 _nHandle ) const
    {
        Reference< XPropertySetInfo >  xInfo = const_cast<ParameterWrapper*>( this )->getPropertySetInfo();
        Sequence< Property > aProperties = xInfo->getProperties();
        const Property* pProperties = aProperties.getConstArray();
        for ( sal_Int32 i = 0; i < aProperties.getLength(); ++i, ++pProperties )
        {
            if ( pProperties->Handle == _nHandle )
                return pProperties->Name;
        }

        OSL_FAIL( "ParameterWrapper::impl_getPseudoAggregatePropertyName: invalid argument!" );
        return OUString();
    }


    Reference< XPropertySetInfo > ParameterWrapper::getPropertySetInfo()
    {
        return createPropertySetInfo( getInfoHelper() );
    }


    ::cppu::IPropertyArrayHelper& ParameterWrapper::getInfoHelper()
    {
        if ( !m_pInfoHelper.get() )
        {
            Sequence< Property > aProperties;
            try
            {
                aProperties = m_xDelegatorPSI->getProperties();
                sal_Int32 nProperties( aProperties.getLength() );
                aProperties.realloc( nProperties + 1 );
                aProperties[ nProperties ] = Property(
                    "Value",
                    PROPERTY_ID_VALUE,
                    ::cppu::UnoType< Any >::get(),
                    PropertyAttribute::TRANSIENT | PropertyAttribute::MAYBEVOID
                );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            m_pInfoHelper.reset( new ::cppu::OPropertyArrayHelper( aProperties, false ) );
        }
        return *m_pInfoHelper;
    }


    sal_Bool ParameterWrapper::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
    {
        OSL_ENSURE( PROPERTY_ID_VALUE == nHandle, "ParameterWrapper::convertFastPropertyValue: the only non-readonly prop should be our PROPERTY_VALUE!" );
        (void)nHandle;

        // we're lazy here ...
        rOldValue = m_aValue.makeAny();
        rConvertedValue = rValue;
        return true;    // assume "modified" ...
    }


    void ParameterWrapper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
    {
        if ( nHandle == PROPERTY_ID_VALUE )
        {
            try
            {
                // TODO : aParamType & nScale can be obtained within the constructor ....
                sal_Int32 nParamType = DataType::VARCHAR;
                OSL_VERIFY( m_xDelegator->getPropertyValue("Type") >>= nParamType );

                sal_Int32 nScale = 0;
                if ( m_xDelegatorPSI->hasPropertyByName("Scale") )
                    OSL_VERIFY( m_xDelegator->getPropertyValue("Scale") >>= nScale );

                if ( m_xValueDestination.is() )
                {
                    for ( ::std::vector< sal_Int32 >::const_iterator aIter = m_aIndexes.begin(); aIter != m_aIndexes.end(); ++aIter )
                    {
                        m_xValueDestination->setObjectWithInfo( *aIter + 1, rValue, nParamType, nScale );
                            // (the index of the parameters is one-based)
                    }
                }

                m_aValue = rValue;
            }
            catch( SQLException& e )
            {
                WrappedTargetException aExceptionWrapper;
                aExceptionWrapper.Context = e.Context;
                aExceptionWrapper.Message = e.Message;
                aExceptionWrapper.TargetException <<= e;
                throw WrappedTargetException( aExceptionWrapper );
            }
        }
        else
        {
            OUString aName = impl_getPseudoAggregatePropertyName( nHandle );
            m_xDelegator->setPropertyValue( aName, rValue );
        }
    }


    void ParameterWrapper::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
    {
        if ( nHandle == PROPERTY_ID_VALUE )
        {
            rValue = m_aValue.makeAny();
        }
        else
        {
            OUString aName = impl_getPseudoAggregatePropertyName( nHandle );
            rValue = m_xDelegator->getPropertyValue( aName );
        }
    }


    void SAL_CALL ParameterWrapper::dispose()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_aValue.setNull();
        m_aIndexes.resize(0);
        m_xDelegator.clear();
        m_xDelegatorPSI.clear();
        m_xValueDestination.clear();

        m_aBHelper.bDisposed = true;
    }

    ParameterWrapperContainer::ParameterWrapperContainer()
        :ParameterWrapperContainer_Base( m_aMutex )
    {
    }


    ParameterWrapperContainer::ParameterWrapperContainer( const Reference< XSingleSelectQueryAnalyzer >& _rxComposer )
        :ParameterWrapperContainer_Base( m_aMutex )
    {
        Reference< XParametersSupplier > xSuppParams( _rxComposer, UNO_QUERY_THROW );
        Reference< XIndexAccess > xParameters( xSuppParams->getParameters(), UNO_QUERY_THROW );
        sal_Int32 nParamCount( xParameters->getCount() );
        m_aParameters.reserve( nParamCount );
        for ( sal_Int32 i=0; i<nParamCount; ++i )
        {
            m_aParameters.push_back( new ParameterWrapper( Reference< XPropertySet >( xParameters->getByIndex( i ), UNO_QUERY_THROW ) ) );
        }
    }


    ParameterWrapperContainer::~ParameterWrapperContainer()
    {
    }


    Type SAL_CALL ParameterWrapperContainer::getElementType()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkDisposed_throw();
        return cppu::UnoType<XPropertySet>::get();
    }


    sal_Bool SAL_CALL ParameterWrapperContainer::hasElements()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkDisposed_throw();
        return !m_aParameters.empty();
    }


    sal_Int32 SAL_CALL ParameterWrapperContainer::getCount()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkDisposed_throw();
        return m_aParameters.size();
    }


    Any SAL_CALL ParameterWrapperContainer::getByIndex( sal_Int32 _nIndex )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkDisposed_throw();

        if ( ( _nIndex < 0 ) || ( _nIndex >= (sal_Int32)m_aParameters.size() ) )
            throw IndexOutOfBoundsException();

        return makeAny( Reference< XPropertySet >( m_aParameters[ _nIndex ].get() ) );
    }


    Reference< XEnumeration > ParameterWrapperContainer::createEnumeration()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkDisposed_throw();

        return new ::comphelper::OEnumerationByIndex( static_cast< XIndexAccess* >( this ) );
    }


    void ParameterWrapperContainer::impl_checkDisposed_throw()
    {
        if ( rBHelper.bDisposed )
            throw DisposedException( OUString(), *this );
    }


    void SAL_CALL ParameterWrapperContainer::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkDisposed_throw();

        for (   Parameters::const_iterator param = m_aParameters.begin();
                param != m_aParameters.end();
                ++param
            )
        {
            (*param)->dispose();
        }

        Parameters aEmpty;
        m_aParameters.swap( aEmpty );
    }


} } // namespace dbtools::param


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
