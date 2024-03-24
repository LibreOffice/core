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

#include <propertybaghelper.hxx>

#include <property.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/NotRemoveableException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>

#include <comphelper/diagnose_ex.hxx>

#include <comphelper/sequence.hxx>


#define NEW_HANDLE_BASE 10000


namespace frm
{


    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::beans::PropertyExistException;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::beans::XMultiPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::beans::NotRemoveableException;
    using ::com::sun::star::beans::UnknownPropertyException;

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;


    //= helper

    namespace
    {

        ::comphelper::IPropertyInfoService& lcl_getPropertyInfos()
        {
            static ConcreteInfoService s_aPropInfos;
            return s_aPropInfos;
        }
    }

    PropertyBagHelper::PropertyBagHelper( IPropertyBagHelperContext& _rContext )
        :m_rContext( _rContext )
        ,m_bDisposed( false )
    {
    }


    PropertyBagHelper::~PropertyBagHelper()
    {
    }


    void PropertyBagHelper::dispose()
    {
        m_bDisposed = true;
    }


    void PropertyBagHelper::impl_nts_checkDisposed_throw() const
    {
        if ( m_bDisposed )
            throw DisposedException();
    }


    void PropertyBagHelper::impl_nts_invalidatePropertySetInfo()
    {
        m_pPropertyArrayHelper.reset();
    }


    sal_Int32 PropertyBagHelper::impl_findFreeHandle( const OUString& _rPropertyName )
    {
        ::comphelper::OPropertyArrayAggregationHelper& rPropInfo( impl_ts_getArrayHelper() );

        // check the preferred handle
        sal_Int32 nHandle = lcl_getPropertyInfos().getPreferredPropertyId( _rPropertyName );
        if ( ( nHandle != -1 ) && rPropInfo.fillPropertyMembersByHandle( nullptr, nullptr, nHandle ) )
            nHandle = -1;

        // search a free handle in <math>F_1009</math>
        if ( nHandle == -1 )
        {
            sal_Int32 const nPrime = 1009;
            sal_Int32 nFactor = 11;
            sal_Int32 nNum = nFactor;
            while ( nNum != 1 )
            {
                if ( !rPropInfo.fillPropertyMembersByHandle( nullptr, nullptr, nNum + NEW_HANDLE_BASE ) )
                {
                    // handle not used, yet
                    nHandle = nNum + NEW_HANDLE_BASE;
                    break;
                }
                nNum = ( nNum * nFactor ) % nPrime;
            }
        }

        // search a free handle greater NEW_HANDLE_BASE
        if ( nHandle == -1 )
        {
            nHandle = NEW_HANDLE_BASE + 1009;
            while ( rPropInfo.fillPropertyMembersByHandle( nullptr, nullptr, nHandle ) )
                ++nHandle;
        }

        return nHandle;
    }


    ::comphelper::OPropertyArrayAggregationHelper& PropertyBagHelper::impl_ts_getArrayHelper() const
    {
        OPropertyArrayAggregationHelper* p = m_pPropertyArrayHelper.get();
        if ( !p )
        {
            ::osl::MutexGuard aGuard( m_rContext.getMutex() );
            p = m_pPropertyArrayHelper.get();
            if ( !p )
            {
                // our own fixed and our aggregate's properties
                Sequence< Property > aFixedProps;
                Sequence< Property > aAggregateProps;
                m_rContext.describeFixedAndAggregateProperties( aFixedProps, aAggregateProps );

                // our dynamic properties
                Sequence< Property > aDynamicProps;
                m_aDynamicProperties.describeProperties( aDynamicProps );

                Sequence< Property > aOwnProps(
                    ::comphelper::concatSequences( aFixedProps, aDynamicProps ) );

                p = new OPropertyArrayAggregationHelper( aOwnProps, aAggregateProps, &lcl_getPropertyInfos(), NEW_HANDLE_BASE );
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                const_cast< PropertyBagHelper* >( this )->m_pPropertyArrayHelper.reset( p );
            }
        } // if ( !p )
        else
        {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
        return *p;
    }


    void PropertyBagHelper::addProperty( const OUString& _rName, ::sal_Int16 _nAttributes, const Any& _rInitialValue )
    {
        ::osl::MutexGuard aGuard( m_rContext.getMutex() );
        impl_nts_checkDisposed_throw();


        // check name sanity
        ::comphelper::OPropertyArrayAggregationHelper& aPropInfo( impl_ts_getArrayHelper() );
        if ( aPropInfo.hasPropertyByName( _rName ) )
            throw PropertyExistException( _rName, m_rContext.getPropertiesInterface() );


        // normalize the REMOVABLE attribute - the FormComponent service
        // requires that all dynamic properties are REMOVABLE
        _nAttributes |= PropertyAttribute::REMOVABLE;


        // find a free handle
        sal_Int32 nHandle = impl_findFreeHandle( _rName );


        // register the property, and invalidate our property meta data
        m_aDynamicProperties.addProperty( _rName, nHandle, _nAttributes, _rInitialValue );
        impl_nts_invalidatePropertySetInfo();
    }


    void PropertyBagHelper::removeProperty( const OUString& _rName )
    {
        ::osl::MutexGuard aGuard( m_rContext.getMutex() );
        impl_nts_checkDisposed_throw();

        // check whether it's removable at all
        Reference< XMultiPropertySet > xMe( m_rContext.getPropertiesInterface(), css::uno::UNO_SET_THROW );
        Reference< XPropertySetInfo > xPSI( xMe->getPropertySetInfo(), css::uno::UNO_SET_THROW );
        Property aProperty( xPSI->getPropertyByName( _rName ) );
        if ( ( aProperty.Attributes & PropertyAttribute::REMOVABLE ) == 0 )
            throw NotRemoveableException( _rName, xMe );

        m_aDynamicProperties.removeProperty( _rName );
        impl_nts_invalidatePropertySetInfo();
    }


    namespace
    {

        struct SelectNameOfProperty
        {
            const OUString& operator()( const Property& _rProp ) const { return _rProp.Name; }
        };


        struct SelectNameOfPropertyValue
        {
            const OUString& operator()( const PropertyValue& _rProp ) const { return _rProp.Name; }
        };


        struct SelectValueOfPropertyValue
        {
            const Any& operator()( const PropertyValue& _rProp ) const { return _rProp.Value; }
        };


        struct PropertyValueLessByName
        {
            bool operator()( const PropertyValue& _lhs, const PropertyValue& _rhs ) const
            {
                return _lhs.Name < _rhs.Name;
            }
        };
    }


    Sequence< PropertyValue > PropertyBagHelper::getPropertyValues()
    {
        ::osl::MutexGuard aGuard( m_rContext.getMutex() );
        impl_nts_checkDisposed_throw();

        Reference< XMultiPropertySet > xMe( m_rContext.getPropertiesInterface(), css::uno::UNO_SET_THROW );
        Reference< XPropertySetInfo > xPSI( xMe->getPropertySetInfo(), css::uno::UNO_SET_THROW );

        const Sequence< Property > aProperties( xPSI->getProperties() );
        Sequence< OUString > aPropertyNames( aProperties.getLength() );
        ::std::transform( aProperties.begin(), aProperties.end(),
            aPropertyNames.getArray(), SelectNameOfProperty() );

        Sequence< Any > aValues;
        try
        {
            aValues = xMe->getPropertyValues( aPropertyNames );

            if ( aValues.getLength() != aPropertyNames.getLength() )
                throw RuntimeException("property name and value counts out of sync");
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("forms.component");
        }
        Sequence< PropertyValue > aPropertyValues( aValues.getLength() );
        PropertyValue* pPropertyValue = aPropertyValues.getArray();

        const OUString* pName = aPropertyNames.getConstArray();
        const OUString* pNameEnd = aPropertyNames.getConstArray() + aPropertyNames.getLength();
        const Any* pValue = aValues.getConstArray();
        for ( ; pName != pNameEnd; ++pName, ++pValue, ++pPropertyValue )
        {
            pPropertyValue->Name = *pName;
            pPropertyValue->Value = *pValue;
        }

        return aPropertyValues;
    }


    void PropertyBagHelper::setPropertyValues( const Sequence< PropertyValue >& _rProps )
    {
        ::osl::ClearableMutexGuard aGuard( m_rContext.getMutex() );
        impl_nts_checkDisposed_throw();

        sal_Int32 nPropertyValues = _rProps.getLength();

        // XMultiPropertySet::setPropertyValues expects its arguments to be sorted by name
        // while XPropertyAccess::setPropertyValues doesn't. So first of all, sort.
        Sequence< PropertyValue > aSortedProps( _rProps );
        ::std::sort( aSortedProps.getArray(), aSortedProps.getArray() + nPropertyValues, PropertyValueLessByName() );

        // also, XPropertyAccess::setPropertyValues is expected to throw an UnknownPropertyException
        // for unsupported properties, while XMultiPropertySet::setPropertyValues is expected to ignore
        // those. So, check for unsupported properties first.
        ::comphelper::OPropertyArrayAggregationHelper& rArrayHelper( impl_ts_getArrayHelper() );
        for (   const PropertyValue* pProperties = aSortedProps.getConstArray();
                pProperties != aSortedProps.getConstArray() + nPropertyValues;
                ++pProperties
            )
        {
            if ( !rArrayHelper.hasPropertyByName( pProperties->Name ) )
                throw UnknownPropertyException( pProperties->Name, m_rContext.getPropertiesInterface() );
        }

        // Now finally split into a Name and a Value sequence, and forward to
        // XMultiPropertySet::setPropertyValues
        Sequence< OUString > aNames( nPropertyValues );
        ::std::transform( aSortedProps.getConstArray(), aSortedProps.getConstArray() + nPropertyValues,
            aNames.getArray(), SelectNameOfPropertyValue() );

        Sequence< Any > aValues( nPropertyValues );
        ::std::transform( aSortedProps.getConstArray(), aSortedProps.getConstArray() + nPropertyValues,
            aValues.getArray(), SelectValueOfPropertyValue() );

        Reference< XMultiPropertySet > xMe( m_rContext.getPropertiesInterface(), css::uno::UNO_SET_THROW );

        aGuard.clear();
        xMe->setPropertyValues( aNames, aValues );
    }


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
