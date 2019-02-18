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

#include <comphelper/propagg.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#if OSL_DEBUG_LEVEL > 0
#include <typeinfo>
#include <rtl/strbuf.hxx>
#endif

#include <algorithm>
#include <set>
#include <unordered_set>
#include <memory>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    using namespace internal;


    namespace
    {
        const Property* lcl_findPropertyByName( const std::vector< Property >& _rProps, const OUString& _rName )
        {
            Property aNameProp(_rName, 0, Type(), 0);
            auto pResult = std::lower_bound(_rProps.begin(), _rProps.end(), aNameProp, PropertyCompareByName());
            if ( pResult == _rProps.end() || pResult->Name != _rName )
                return nullptr;

            return &*pResult;
        }
    }

OPropertyArrayAggregationHelper::OPropertyArrayAggregationHelper(
        const  Sequence< Property >& _rProperties, const  Sequence< Property >& _rAggProperties,
        IPropertyInfoService* _pInfoService, sal_Int32 _nFirstAggregateId )
{
    // if properties are present both at the delegatee and the aggregate, then the former are supposed to win
    // merge and sort properties by name, delete duplicates (stable sort ensures delegator properties win)
    m_aProperties.insert( m_aProperties.end(), _rProperties.begin(), _rProperties.end() );
    m_aProperties.insert( m_aProperties.end(), _rAggProperties.begin(), _rAggProperties.end() );
    std::stable_sort( m_aProperties.begin(), m_aProperties.end(), PropertyCompareByName() );
    m_aProperties.erase( std::unique(m_aProperties.begin(), m_aProperties.end(),
        []( const css::beans::Property& x, const css::beans::Property& y ) -> bool { return x.Name == y.Name; } ),
        m_aProperties.end() );
    m_aProperties.shrink_to_fit();

    // fill aDelegatorProps with names from _rProperties for a fast existence check
    // different kinds of properties are processed differently
    std::unordered_set< OUString > aDelegatorProps;
    aDelegatorProps.reserve( _rProperties.getLength() );
    for( auto &delegateProp: _rProperties )
    {
        const auto inserted = aDelegatorProps.insert( delegateProp.Name );
        OSL_ENSURE( inserted.second,
            "OPropertyArrayAggregationHelper::OPropertyArrayAggregationHelper: duplicate delegatee property!" );
    }

    std::unordered_set< sal_Int32 > existingHandles;
    existingHandles.reserve( m_aProperties.size() );
    sal_Int32 nAggregateHandle = _nFirstAggregateId;
    for ( sal_Int32 nMPLoop = 0; nMPLoop < static_cast< sal_Int32 >( m_aProperties.size() ); ++nMPLoop )
    {
        auto &prop = m_aProperties[ nMPLoop ];
        if ( aDelegatorProps.find( prop.Name ) != aDelegatorProps.end() )
        {
            m_aPropertyAccessors[ prop.Handle ] = OPropertyAccessor( -1, nMPLoop, false );
            existingHandles.insert( prop.Handle );
        }
        else
        {
            // determine the handle for the property which we will expose to the outside world
            sal_Int32 nHandle = -1;
            // ask the info service first
            if ( _pInfoService )
                nHandle = _pInfoService->getPreferredPropertyId( prop.Name );

            if ( ( -1 == nHandle ) || ( existingHandles.find( nHandle ) != existingHandles.end() ) )
            {
                // 1. no handle from the info service -> default
                // 2. conflicts -> use another one (which we don't check anymore, assuming _nFirstAggregateId was large enough)
                nHandle = nAggregateHandle++;
            }
            else
            {
                existingHandles.insert( nHandle );
            }

            // remember the accessor for this property
            m_aPropertyAccessors[ nHandle ] = OPropertyAccessor( prop.Handle, nMPLoop, true );
            prop.Handle = nHandle;
        }
    }
}


OPropertyArrayAggregationHelper::PropertyOrigin OPropertyArrayAggregationHelper::classifyProperty( const OUString& _rName )
{
    PropertyOrigin eOrigin = PropertyOrigin::Unknown;
    // look up the name
    const Property* pPropertyDescriptor = lcl_findPropertyByName( m_aProperties, _rName );
    if ( pPropertyDescriptor )
    {
        // look up the handle for this name
        ConstPropertyAccessorMapIterator aPos = m_aPropertyAccessors.find( pPropertyDescriptor->Handle );
        OSL_ENSURE( m_aPropertyAccessors.end() != aPos, "OPropertyArrayAggregationHelper::classifyProperty: should have this handle in my map!" );
        if ( m_aPropertyAccessors.end() != aPos )
        {
            eOrigin = aPos->second.bAggregate ? PropertyOrigin::Aggregate : PropertyOrigin::Delegator;
        }
    }
    return eOrigin;
}


Property OPropertyArrayAggregationHelper::getPropertyByName( const OUString& _rPropertyName )
{
    const Property* pProperty = findPropertyByName( _rPropertyName );

    if ( !pProperty )
        throw  UnknownPropertyException();

    return *pProperty;
}


sal_Bool OPropertyArrayAggregationHelper::hasPropertyByName(const OUString& _rPropertyName)
{
    return nullptr != findPropertyByName( _rPropertyName );
}


const Property* OPropertyArrayAggregationHelper::findPropertyByName(const OUString& _rName ) const
{
    return lcl_findPropertyByName( m_aProperties, _rName );
}


sal_Int32 OPropertyArrayAggregationHelper::getHandleByName(const OUString& _rPropertyName)
{
    const Property* pProperty = findPropertyByName( _rPropertyName );
    return pProperty ? pProperty->Handle : -1;
}


sal_Bool OPropertyArrayAggregationHelper::fillPropertyMembersByHandle(
            OUString* _pPropName, sal_Int16* _pAttributes, sal_Int32 _nHandle)
{
    ConstPropertyAccessorMapIterator i = m_aPropertyAccessors.find(_nHandle);
    bool bRet = i != m_aPropertyAccessors.end();
    if (bRet)
    {
        const css::beans::Property& rProperty = m_aProperties[(*i).second.nPos];
        if (_pPropName)
            *_pPropName = rProperty.Name;
        if (_pAttributes)
            *_pAttributes = rProperty.Attributes;
    }
    return bRet;
}


bool OPropertyArrayAggregationHelper::getPropertyByHandle( sal_Int32 _nHandle, Property& _rProperty ) const
{
    ConstPropertyAccessorMapIterator pos = m_aPropertyAccessors.find(_nHandle);
    if ( pos != m_aPropertyAccessors.end() )
    {
        _rProperty = m_aProperties[ pos->second.nPos ];
        return true;
    }
    return false;
}


bool OPropertyArrayAggregationHelper::fillAggregatePropertyInfoByHandle(
            OUString* _pPropName, sal_Int32* _pOriginalHandle, sal_Int32 _nHandle) const
{
    ConstPropertyAccessorMapIterator i = m_aPropertyAccessors.find(_nHandle);
    bool bRet = i != m_aPropertyAccessors.end() && (*i).second.bAggregate;
    if (bRet)
    {
        if (_pOriginalHandle)
            *_pOriginalHandle = (*i).second.nOriginalHandle;
        if (_pPropName)
        {
            OSL_ENSURE((*i).second.nPos < static_cast<sal_Int32>(m_aProperties.size()),"Invalid index for sequence!");
            const css::beans::Property& rProperty = m_aProperties[(*i).second.nPos];
            *_pPropName = rProperty.Name;
        }
    }
    return bRet;
}


css::uno::Sequence< css::beans::Property> OPropertyArrayAggregationHelper::getProperties()
{
    return comphelper::containerToSequence(m_aProperties);
}


sal_Int32 OPropertyArrayAggregationHelper::fillHandles(
        sal_Int32* _pHandles, const css::uno::Sequence< OUString >& _rPropNames )
{
    sal_Int32 nHitCount = 0;
    const OUString* pReqProps = _rPropNames.getConstArray();
    sal_Int32 nReqLen = _rPropNames.getLength();

    Property aNameProp;
    for( sal_Int32 i = 0; i < nReqLen; ++i )
    {
        aNameProp.Name = pReqProps[i];
        auto findIter = std::lower_bound(m_aProperties.begin(), m_aProperties.end(), aNameProp, PropertyCompareByName());
        if ( findIter != m_aProperties.end() && findIter->Name == pReqProps[i] )
        {
            _pHandles[i] = findIter->Handle;
            nHitCount++;
        }
    }
    return nHitCount;
}

namespace internal
{
    class PropertyForwarder
    {
    private:
        OPropertySetAggregationHelper&  m_rAggregationHelper;
        std::set< sal_Int32 >         m_aProperties;
        sal_Int32                       m_nCurrentlyForwarding;

    public:
        explicit PropertyForwarder( OPropertySetAggregationHelper& _rAggregationHelper );

        /** declares that the forwarder should be responsible for the given property

        @param _nHandle
            the public handle (<em>not</em> the original handle!) of the property
        */
        void    takeResponsibilityFor( sal_Int32 _nHandle );

        /** checks whether the forwarder is responsible for the given property
        */
        bool    isResponsibleFor( sal_Int32 _nHandle );

        /// actually forwards a property value to the aggregate
        ///
        /// @throws Exception
        void    doForward( sal_Int32 _nHandle, const Any& _rValue );

        sal_Int32 getCurrentlyForwardedProperty( ) const { return m_nCurrentlyForwarding; }
    };


    PropertyForwarder::PropertyForwarder( OPropertySetAggregationHelper& _rAggregationHelper )
        :m_rAggregationHelper( _rAggregationHelper )
        ,m_nCurrentlyForwarding( -1 )
    {
    }


    void PropertyForwarder::takeResponsibilityFor( sal_Int32 _nHandle )
    {
        m_aProperties.insert( _nHandle );
    }


    bool PropertyForwarder::isResponsibleFor( sal_Int32 _nHandle )
    {
        return m_aProperties.find( _nHandle ) != m_aProperties.end();
    }


    void PropertyForwarder::doForward( sal_Int32 _nHandle, const Any& _rValue )
    {
        OSL_ENSURE( m_rAggregationHelper.m_xAggregateSet.is(), "PropertyForwarder::doForward: no property set!" );
        if ( m_rAggregationHelper.m_xAggregateSet.is() )
        {
            m_rAggregationHelper.forwardingPropertyValue( _nHandle );

            OSL_ENSURE( m_nCurrentlyForwarding == -1, "PropertyForwarder::doForward: reentrance?" );
            m_nCurrentlyForwarding = _nHandle;

            try
            {
                m_rAggregationHelper.m_xAggregateSet->setPropertyValue( m_rAggregationHelper.getPropertyName( _nHandle ), _rValue );
                    // TODO: cache the property name? (it's a O(log n) search)
            }
            catch( const Exception& )
            {
                m_rAggregationHelper.forwardedPropertyValue( _nHandle );
                throw;
            }

            m_nCurrentlyForwarding = -1;

            m_rAggregationHelper.forwardedPropertyValue( _nHandle );
        }
    }
}

OPropertySetAggregationHelper::OPropertySetAggregationHelper( ::cppu::OBroadcastHelper& rBHlp )
    :OPropertyStateHelper( rBHlp )
    ,m_bListening( false )
{
    m_pForwarder.reset( new PropertyForwarder( *this ) );
}


OPropertySetAggregationHelper::~OPropertySetAggregationHelper()
{
}


css::uno::Any SAL_CALL OPropertySetAggregationHelper::queryInterface(const  css::uno::Type& _rType)
{
    css::uno::Any aReturn = OPropertyStateHelper::queryInterface(_rType);

    if ( !aReturn.hasValue() )
        aReturn = cppu::queryInterface(_rType
        ,static_cast< css::beans::XPropertiesChangeListener*>(this)
        ,static_cast< css::beans::XVetoableChangeListener*>(this)
        ,static_cast< css::lang::XEventListener*>(static_cast< css::beans::XPropertiesChangeListener*>(this))
        );

    return aReturn;
}


void OPropertySetAggregationHelper::disposing()
{
    osl::MutexGuard aGuard(rBHelper.rMutex);

    if ( m_xAggregateSet.is() && m_bListening )
    {
        // register as a single listener
        m_xAggregateMultiSet->removePropertiesChangeListener(this);
        m_xAggregateSet->removeVetoableChangeListener(OUString(), this);
        m_bListening = false;
    }

    OPropertyStateHelper::disposing();
}


void SAL_CALL OPropertySetAggregationHelper::disposing(const css::lang::EventObject& _rSource)
{
    OSL_ENSURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::disposing : don't have an aggregate anymore !");
    if (_rSource.Source == m_xAggregateSet)
        m_bListening = false;
}


void SAL_CALL OPropertySetAggregationHelper::propertiesChange(const css::uno::Sequence< css::beans::PropertyChangeEvent>& _rEvents)
{
    OSL_ENSURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::propertiesChange : have no aggregate !");

    sal_Int32 nLen = _rEvents.getLength();
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();

    if (1 == nLen)
    {
        const css::beans::PropertyChangeEvent& evt = _rEvents.getConstArray()[0];
        OSL_ENSURE(!evt.PropertyName.isEmpty(), "OPropertySetAggregationHelper::propertiesChange : invalid event !");
            // we had a bug where this assertion would have us saved a whole day :) (72514)
        sal_Int32 nHandle = rPH.getHandleByName( evt.PropertyName );

        // If nHandle is -1 the event marks a (aggregate) property which we hide to callers
        // If isCurrentlyForwardingProperty( nHandle ) is <TRUE/>, then we ourself triggered
        // setting this property. In this case, it will be notified later (by the OPropertySetHelper
        // implementation)

        if ( ( nHandle != -1 ) && !isCurrentlyForwardingProperty( nHandle ) )
            fire(&nHandle, &evt.NewValue, &evt.OldValue, 1, false);
    }
    else
    {
        std::unique_ptr<sal_Int32[]> pHandles(new sal_Int32[nLen]);
        std::unique_ptr< css::uno::Any[]> pNewValues(new css::uno::Any[nLen]);
        std::unique_ptr< css::uno::Any[]> pOldValues(new css::uno::Any[nLen]);

        sal_Int32 nDest = 0;
        for (const css::beans::PropertyChangeEvent& rEvent : _rEvents)
        {
            sal_Int32 nHandle = rPH.getHandleByName(rEvent.PropertyName);
            if ( ( nHandle != -1 ) && !isCurrentlyForwardingProperty( nHandle ) )
            {   // same as above : -1 is valid (73247) ...
                pHandles[nDest] = nHandle;
                pNewValues[nDest] = rEvent.NewValue;
                pOldValues[nDest] = rEvent.OldValue;
                ++nDest;
            }
        }

        if (nDest)
            fire(pHandles.get(), pNewValues.get(), pOldValues.get(), nDest, false);
    }
}


void SAL_CALL OPropertySetAggregationHelper::vetoableChange(const css::beans::PropertyChangeEvent& _rEvent)
{
    OSL_ENSURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::vetoableChange : have no aggregate !");

    cppu::IPropertyArrayHelper& rPH = getInfoHelper();

    sal_Int32 nHandle = rPH.getHandleByName(_rEvent.PropertyName);
    fire(&nHandle, &_rEvent.NewValue, &_rEvent.OldValue, 1, true);
}


void OPropertySetAggregationHelper::setAggregation(const css::uno::Reference<  css::uno::XInterface >& _rxDelegate)
{
    osl::MutexGuard aGuard(rBHelper.rMutex);

    if (m_bListening && m_xAggregateSet.is())
    {
        m_xAggregateMultiSet->removePropertiesChangeListener(this);
        m_xAggregateSet->removeVetoableChangeListener(OUString(), this);
        m_bListening = false;
    }

    m_xAggregateState.set(_rxDelegate, css::uno::UNO_QUERY);
    m_xAggregateSet.set(_rxDelegate, css::uno::UNO_QUERY);
    m_xAggregateMultiSet.set(_rxDelegate, css::uno::UNO_QUERY);
    m_xAggregateFastSet.set(_rxDelegate, css::uno::UNO_QUERY);

    // must support XPropertySet and XMultiPropertySet
    if ( m_xAggregateSet.is() && !m_xAggregateMultiSet.is() )
        throw  css::lang::IllegalArgumentException();
}


void OPropertySetAggregationHelper::startListening()
{
    osl::MutexGuard aGuard(rBHelper.rMutex);

    if (!m_bListening && m_xAggregateSet.is())
    {
        // register as a single listener
        css::uno::Sequence< OUString > aPropertyNames;
        m_xAggregateMultiSet->addPropertiesChangeListener(aPropertyNames, this);
        m_xAggregateSet->addVetoableChangeListener(OUString(), this);

        m_bListening = true;
    }
}


void SAL_CALL OPropertySetAggregationHelper::addVetoableChangeListener(const OUString& _rPropertyName,
                                                                       const  css::uno::Reference< css::beans::XVetoableChangeListener>& _rxListener)
{
    OPropertySetHelper::addVetoableChangeListener(_rPropertyName, _rxListener);
    if (!m_bListening)
        startListening();
}


void SAL_CALL OPropertySetAggregationHelper::addPropertyChangeListener(const OUString& _rPropertyName,
                                                                       const css::uno::Reference< css::beans::XPropertyChangeListener>& _rxListener)
{
    OPropertySetHelper::addPropertyChangeListener(_rPropertyName, _rxListener);
    if (!m_bListening)
        startListening();
}


void SAL_CALL OPropertySetAggregationHelper::addPropertiesChangeListener(const css::uno::Sequence< OUString >& _rPropertyNames,
                                                                         const css::uno::Reference< css::beans::XPropertiesChangeListener>& _rxListener)
{
    OPropertySetHelper::addPropertiesChangeListener(_rPropertyNames, _rxListener);
    if (!m_bListening)
        startListening();
}


sal_Int32 OPropertySetAggregationHelper::getOriginalHandle(sal_Int32 nHandle) const
{
    OPropertyArrayAggregationHelper& rPH = static_cast<OPropertyArrayAggregationHelper&>( const_cast<OPropertySetAggregationHelper*>(this)->getInfoHelper() );
    sal_Int32 nOriginalHandle = -1;
    (void)rPH.fillAggregatePropertyInfoByHandle(nullptr, &nOriginalHandle, nHandle);
    return nOriginalHandle;
}


OUString OPropertySetAggregationHelper::getPropertyName( sal_Int32 _nHandle ) const
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( const_cast<OPropertySetAggregationHelper*>(this)->getInfoHelper() );
    Property aProperty;
    OSL_VERIFY( rPH.getPropertyByHandle( _nHandle, aProperty ) );
    return aProperty.Name;
}


void SAL_CALL OPropertySetAggregationHelper::setFastPropertyValue(sal_Int32 _nHandle, const  css::uno::Any& _rValue)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;

    // does the handle belong to the aggregation ?
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, _nHandle))
        if (m_xAggregateFastSet.is())
            m_xAggregateFastSet->setFastPropertyValue(nOriginalHandle, _rValue);
        else
            m_xAggregateSet->setPropertyValue(aPropName, _rValue);
    else
        OPropertySetHelper::setFastPropertyValue(_nHandle, _rValue);
}


void OPropertySetAggregationHelper::getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle) const
{
    OPropertyArrayAggregationHelper& rPH = static_cast<OPropertyArrayAggregationHelper&>( const_cast<OPropertySetAggregationHelper*>(this)->getInfoHelper() );
    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;

    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateFastSet.is())
            rValue = m_xAggregateFastSet->getFastPropertyValue(nOriginalHandle);
        else
            rValue = m_xAggregateSet->getPropertyValue(aPropName);
    }
    else if ( m_pForwarder->isResponsibleFor( nHandle ) )
    {
        // this is a property which has been "overwritten" in our instance (thus
        // fillAggregatePropertyInfoByHandle didn't find it)
        rValue = m_xAggregateSet->getPropertyValue( getPropertyName( nHandle ) );
    }
}


css::uno::Any SAL_CALL OPropertySetAggregationHelper::getFastPropertyValue(sal_Int32 nHandle)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    css::uno::Any  aValue;

    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateFastSet.is())
            aValue = m_xAggregateFastSet->getFastPropertyValue(nOriginalHandle);
        else
            aValue = m_xAggregateSet->getPropertyValue(aPropName);
    }
    else
        aValue = OPropertySetHelper::getFastPropertyValue(nHandle);

    return aValue;
}


void SAL_CALL OPropertySetAggregationHelper::setPropertyValues(
        const Sequence< OUString >& _rPropertyNames, const Sequence< Any >& _rValues )
{
    OSL_ENSURE( !rBHelper.bInDispose, "OPropertySetAggregationHelper::setPropertyValues : do not use within the dispose call !");
    OSL_ENSURE( !rBHelper.bDisposed, "OPropertySetAggregationHelper::setPropertyValues : object is disposed" );

    // check where the properties come from
    if (!m_xAggregateSet.is())
        OPropertySetHelper::setPropertyValues(_rPropertyNames, _rValues);
    else if (_rPropertyNames.getLength() == 1) // use the more efficient way
    {
        try
        {
            setPropertyValue( _rPropertyNames[0], _rValues[0] );
        }
        catch( const UnknownPropertyException& )
        {
            // by definition of XMultiPropertySet::setPropertyValues, unknown properties are to be ignored
            SAL_WARN( "comphelper", "OPropertySetAggregationHelper::setPropertyValues: unknown property: '"
                    << _rPropertyNames[0] << "', implementation: " << typeid( *this ).name() );
        }
    }
    else
    {
        OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );

        // determine which properties belong to the aggregate, and which ones to the delegator
        sal_Int32 nAggCount(0);
        sal_Int32 nLen(_rPropertyNames.getLength());

        for ( const OUString& rName : _rPropertyNames )
        {
            OPropertyArrayAggregationHelper::PropertyOrigin ePropOrg = rPH.classifyProperty( rName );
            if ( OPropertyArrayAggregationHelper::PropertyOrigin::Unknown == ePropOrg )
                throw WrappedTargetException( OUString(), static_cast< XMultiPropertySet* >( this ), Any( UnknownPropertyException( ) ) );
                // due to a flaw in the API design, this method is not allowed to throw an UnknownPropertyException
                // so we wrap it into a WrappedTargetException

            if ( OPropertyArrayAggregationHelper::PropertyOrigin::Aggregate == ePropOrg )
                ++nAggCount;
        }

        // all properties belong to the aggregate
        if (nAggCount == nLen)
            m_xAggregateMultiSet->setPropertyValues(_rPropertyNames, _rValues);

        // all properties belong to the aggregating object
        else if (nAggCount == 0)
            OPropertySetHelper::setPropertyValues(_rPropertyNames, _rValues);

        // mixed
        else
        {
            const  css::uno::Any* pValues = _rValues.getConstArray();

            // dividing the Names and _rValues

            // aggregate's names
            Sequence< OUString > AggPropertyNames( nAggCount );
            OUString* pAggNames = AggPropertyNames.getArray();
            // aggregate's values
            Sequence< Any >  AggValues( nAggCount );
            Any* pAggValues = AggValues.getArray();

            // delegator names
            Sequence< OUString > DelPropertyNames( nLen - nAggCount );
            OUString* pDelNames = DelPropertyNames.getArray();

            // delegator values
            Sequence< Any > DelValues( nLen - nAggCount );
            Any* pDelValues = DelValues.getArray();

            for ( const OUString& rName : _rPropertyNames )
            {
                if ( OPropertyArrayAggregationHelper::PropertyOrigin::Aggregate == rPH.classifyProperty( rName ) )
                {
                    *pAggNames++ = rName;
                    *pAggValues++ = *pValues++;
                }
                else
                {
                    *pDelNames++ = rName;
                    *pDelValues++ = *pValues++;
                }
            }

            // reset, needed below
            pDelValues = DelValues.getArray();

            std::unique_ptr<sal_Int32[]> pHandles(new sal_Int32[ nLen - nAggCount ]);

            // get the map table
            cppu::IPropertyArrayHelper& rPH2 = getInfoHelper();

            // fill the handle array
            sal_Int32 nHitCount = rPH2.fillHandles( pHandles.get(), DelPropertyNames );
            if (nHitCount != 0)
            {
                std::unique_ptr< css::uno::Any[]> pConvertedValues(new  css::uno::Any[ nHitCount ]);
                std::unique_ptr< css::uno::Any[]> pOldValues(new  css::uno::Any[ nHitCount ]);
                nHitCount = 0;
                sal_Int32 i;

                {
                // must lock the mutex outside the loop. So all values are consistent.
                    osl::MutexGuard aGuard( rBHelper.rMutex );
                    for( i = 0; i < (nLen - nAggCount); ++i )
                    {
                        if( pHandles[i] != -1 )
                        {
                            sal_Int16 nAttributes;
                            rPH2.fillPropertyMembersByHandle( nullptr, &nAttributes, pHandles[i] );
                            if( nAttributes & css::beans::PropertyAttribute::READONLY )
                                throw css::beans::PropertyVetoException();
                            // Will the property change?
                            if( convertFastPropertyValue( pConvertedValues[ nHitCount ], pOldValues[nHitCount],
                                                        pHandles[i], pDelValues[i] ) )
                            {
                                // only increment if the property really change
                                pHandles[nHitCount]         = pHandles[i];
                                nHitCount++;
                            }
                        }
                    }
                // release guard to fire events
                }

                // fire vetoable events
                fire( pHandles.get(), pConvertedValues.get(), pOldValues.get(), nHitCount, true );

                // setting the agg Properties
                m_xAggregateMultiSet->setPropertyValues(AggPropertyNames, AggValues);

                {
                // must lock the mutex outside the loop.
                    osl::MutexGuard aGuard( rBHelper.rMutex );
                    // Loop over all changed properties
                    for( i = 0; i < nHitCount; i++ )
                    {
                        // Will the property change?
                        setFastPropertyValue_NoBroadcast( pHandles[i], pConvertedValues[i] );
                    }
                // release guard to fire events
                }

                // fire change events
                fire( pHandles.get(), pConvertedValues.get(), pOldValues.get(), nHitCount, false );
            }
            else
                m_xAggregateMultiSet->setPropertyValues(AggPropertyNames, AggValues);
        }
    }
}

// XPropertyState

css::beans::PropertyState SAL_CALL OPropertySetAggregationHelper::getPropertyState(const OUString& _rPropertyName)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    sal_Int32 nHandle = rPH.getHandleByName( _rPropertyName );

    if (nHandle == -1)
    {
        throw css::beans::UnknownPropertyException();
    }

    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateState.is())
            return m_xAggregateState->getPropertyState(_rPropertyName);
        else
            return css::beans::PropertyState_DIRECT_VALUE;
    }
    else
        return getPropertyStateByHandle(nHandle);
}


void SAL_CALL OPropertySetAggregationHelper::setPropertyToDefault(const OUString& _rPropertyName)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    sal_Int32 nHandle = rPH.getHandleByName(_rPropertyName);
    if (nHandle == -1)
    {
        throw css::beans::UnknownPropertyException();
    }

    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateState.is())
            m_xAggregateState->setPropertyToDefault(_rPropertyName);
    }
    else
    {
        try
        {
            setPropertyToDefaultByHandle( nHandle );
        }
        catch( const UnknownPropertyException& ) { throw; }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            OSL_FAIL( "OPropertySetAggregationHelper::setPropertyToDefault: caught an exception which is not allowed to leave here!" );
        }
    }
}


css::uno::Any SAL_CALL OPropertySetAggregationHelper::getPropertyDefault(const OUString& aPropertyName)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    sal_Int32 nHandle = rPH.getHandleByName( aPropertyName );

    if ( nHandle == -1 )
        throw css::beans::UnknownPropertyException();

    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateState.is())
            return m_xAggregateState->getPropertyDefault(aPropertyName);
        else
            return css::uno::Any();
    }
    else
        return getPropertyDefaultByHandle(nHandle);
}

sal_Bool SAL_CALL OPropertySetAggregationHelper::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
{
    bool bModified = false;

    OSL_ENSURE( m_pForwarder->isResponsibleFor( _nHandle ), "OPropertySetAggregationHelper::convertFastPropertyValue: this is no forwarded property - did you use declareForwardedProperty for it?" );
    if ( m_pForwarder->isResponsibleFor( _nHandle ) )
    {
        // need to determine the type of the property for conversion
        OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
        Property aProperty;
        OSL_VERIFY( rPH.getPropertyByHandle( _nHandle, aProperty ) );

        Any aCurrentValue;
        getFastPropertyValue( aCurrentValue, _nHandle );
        bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, aCurrentValue, aProperty.Type );
    }

    return bModified;
}

void SAL_CALL OPropertySetAggregationHelper::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
{
    OSL_ENSURE( m_pForwarder->isResponsibleFor( _nHandle ), "OPropertySetAggregationHelper::setFastPropertyValue_NoBroadcast: this is no forwarded property - did you use declareForwardedProperty for it?" );
    if ( m_pForwarder->isResponsibleFor( _nHandle ) )
        m_pForwarder->doForward( _nHandle, _rValue );
}


void OPropertySetAggregationHelper::declareForwardedProperty( sal_Int32 _nHandle )
{
    OSL_ENSURE( !m_pForwarder->isResponsibleFor( _nHandle ), "OPropertySetAggregationHelper::declareForwardedProperty: already declared!" );
    m_pForwarder->takeResponsibilityFor( _nHandle );
}


void OPropertySetAggregationHelper::forwardingPropertyValue( sal_Int32 )
{
    // not interested in
}


void OPropertySetAggregationHelper::forwardedPropertyValue( sal_Int32 )
{
    // not interested in
}


bool OPropertySetAggregationHelper::isCurrentlyForwardingProperty( sal_Int32 _nHandle ) const
{
    return m_pForwarder->getCurrentlyForwardedProperty() == _nHandle;
}


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
