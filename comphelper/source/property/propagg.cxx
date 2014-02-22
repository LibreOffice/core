/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "comphelper/propagg.hxx"
#include "comphelper/property.hxx"
#include <cppuhelper/queryinterface.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#if OSL_DEBUG_LEVEL > 0
#include <typeinfo>
#include <rtl/strbuf.hxx>
#endif

#include <algorithm>
#include <set>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    using namespace internal;

    
    namespace
    {
        const Property* lcl_findPropertyByName( const Sequence< Property >& _rProps, const OUString& _rName )
        {
            sal_Int32 nLen = _rProps.getLength();
            const Property* pProperties = _rProps.getConstArray();
            Property aNameProp(_rName, 0, Type(), 0);
            const Property* pResult = ::std::lower_bound(pProperties, pProperties + nLen, aNameProp, PropertyCompareByName());
            if ( pResult && ( pResult == pProperties + nLen || pResult->Name != _rName) )
                pResult = NULL;

            return pResult;
        }
    }





OPropertyArrayAggregationHelper::OPropertyArrayAggregationHelper(
        const  Sequence< Property >& _rProperties, const  Sequence< Property >& _rAggProperties,
        IPropertyInfoService* _pInfoService, sal_Int32 _nFirstAggregateId )
    :m_aProperties( _rProperties )
{
    sal_Int32 nDelegatorProps = _rProperties.getLength();
    sal_Int32 nAggregateProps = _rAggProperties.getLength();

    
    sal_Int32 nMergedProps = nDelegatorProps + nAggregateProps;
    m_aProperties.realloc( nMergedProps );

    const   Property* pAggregateProps   = _rAggProperties.getConstArray();
    const   Property* pDelegateProps    = _rProperties.getConstArray();
            Property* pMergedProps = m_aProperties.getArray();

    
    
    ::std::set< OUString > aDelegatorProps;

    
    sal_Int32 nMPLoop = 0;
    for ( ; nMPLoop < nDelegatorProps; ++nMPLoop, ++pDelegateProps )
    {
        m_aPropertyAccessors[ pDelegateProps->Handle ] = OPropertyAccessor( -1, nMPLoop, false );
        OSL_ENSURE( aDelegatorProps.find( pDelegateProps->Name ) == aDelegatorProps.end(),
            "OPropertyArrayAggregationHelper::OPropertyArrayAggregationHelper: duplicate delegatee property!" );
        aDelegatorProps.insert( pDelegateProps->Name );
    }

    
    sal_Int32 nAggregateHandle = _nFirstAggregateId;
    pMergedProps += nDelegatorProps;
    for ( ; nMPLoop < nMergedProps; ++pAggregateProps )
    {
        
        if ( aDelegatorProps.find( pAggregateProps->Name ) != aDelegatorProps.end() )
        {
            --nMergedProps;
            continue;
        }

        
        *pMergedProps = *pAggregateProps;

        
        sal_Int32 nHandle = -1;
        
        if ( _pInfoService )
            nHandle = _pInfoService->getPreferredPropertyId( pMergedProps->Name );

        if ( -1 == nHandle )
            
            nHandle = nAggregateHandle++;
        else
        {   
            const  Property* pPropsTilNow = m_aProperties.getConstArray();
            for ( sal_Int32 nCheck = 0; nCheck < nMPLoop; ++nCheck, ++pPropsTilNow )
                if ( pPropsTilNow->Handle == nHandle )
                {   
                    nHandle = nAggregateHandle++;
                    break;
                }
        }

        
        m_aPropertyAccessors[ nHandle ] = OPropertyAccessor( pMergedProps->Handle, nMPLoop, true );
        pMergedProps->Handle = nHandle;

        ++nMPLoop;
        ++pMergedProps;
    }
    m_aProperties.realloc( nMergedProps );
    pMergedProps = m_aProperties.getArray();    

    
    ::std::sort( pMergedProps, pMergedProps+nMergedProps, PropertyCompareByName());

    pMergedProps = m_aProperties.getArray();

    
    for ( nMPLoop = 0; nMPLoop < nMergedProps; ++nMPLoop, ++pMergedProps )
        m_aPropertyAccessors[ pMergedProps->Handle ].nPos = nMPLoop;
}


OPropertyArrayAggregationHelper::PropertyOrigin OPropertyArrayAggregationHelper::classifyProperty( const OUString& _rName )
{
    PropertyOrigin eOrigin = UNKNOWN_PROPERTY;
    
    const Property* pPropertyDescriptor = lcl_findPropertyByName( m_aProperties, _rName );
    if ( pPropertyDescriptor )
    {
        
        ConstPropertyAccessorMapIterator aPos = m_aPropertyAccessors.find( pPropertyDescriptor->Handle );
        OSL_ENSURE( m_aPropertyAccessors.end() != aPos, "OPropertyArrayAggregationHelper::classifyProperty: should have this handle in my map!" );
        if ( m_aPropertyAccessors.end() != aPos )
        {
            eOrigin = aPos->second.bAggregate ? AGGREGATE_PROPERTY : DELEGATOR_PROPERTY;
        }
    }
    return eOrigin;
}


Property OPropertyArrayAggregationHelper::getPropertyByName( const OUString& _rPropertyName ) throw( UnknownPropertyException )
{
    const Property* pProperty = findPropertyByName( _rPropertyName );

    if ( !pProperty )
        throw  UnknownPropertyException();

    return *pProperty;
}


sal_Bool OPropertyArrayAggregationHelper::hasPropertyByName(const OUString& _rPropertyName)
{
    return NULL != findPropertyByName( _rPropertyName );
}


const Property* OPropertyArrayAggregationHelper::findPropertyByName(const :: OUString& _rName ) const
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
        const  ::com::sun::star::beans::Property& rProperty = m_aProperties.getConstArray()[(*i).second.nPos];
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
            OSL_ENSURE((*i).second.nPos < m_aProperties.getLength(),"Invalid index for sequence!");
            const  ::com::sun::star::beans::Property& rProperty = m_aProperties.getConstArray()[(*i).second.nPos];
            *_pPropName = rProperty.Name;
        }
    }
    return bRet;
}



 ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> OPropertyArrayAggregationHelper::getProperties()
{
    return m_aProperties;
}



sal_Int32 OPropertyArrayAggregationHelper::fillHandles(
        sal_Int32* _pHandles, const  ::com::sun::star::uno::Sequence< OUString >& _rPropNames )
{
    sal_Int32 nHitCount = 0;
    const OUString* pReqProps = _rPropNames.getConstArray();
    sal_Int32 nReqLen = _rPropNames.getLength();

#if OSL_DEBUG_LEVEL > 0
    
    {
        const OUString* pLookup = _rPropNames.getConstArray();
        const OUString* pEnd = _rPropNames.getConstArray() + _rPropNames.getLength() - 1;
        for (; pLookup < pEnd; ++pLookup)
        {
            const OUString* pCompare = pLookup + 1;
            const OUString* pCompareEnd = pEnd + 1;
            for (; pCompare < pCompareEnd; ++pCompare)
            {
                OSL_ENSURE(pLookup->compareTo(*pCompare) < 0, "OPropertyArrayAggregationHelper::fillHandles : property names are not sorted!");
            }
        }
    }
#endif

    const  ::com::sun::star::beans::Property* pCur = m_aProperties.getConstArray();
    const  ::com::sun::star::beans::Property* pEnd = m_aProperties.getConstArray() + m_aProperties.getLength();

    for( sal_Int32 i = 0; i < nReqLen; ++i )
    {
        
        sal_uInt32 n = (sal_uInt32)(pEnd - pCur);
        sal_Int32 nLog = 0;
        while( n )
        {
            nLog += 1;
            n = n >> 1;
        }

        
        if( (nReqLen - i) * nLog >= pEnd - pCur )
        {
            
            while( pCur < pEnd && pReqProps[i] > pCur->Name )
            {
                pCur++;
            }
            if( pCur < pEnd && pReqProps[i] == pCur->Name )
            {
                _pHandles[i] = pCur->Handle;
                nHitCount++;
            }
            else
                _pHandles[i] = -1;
        }
        else
        {
            
            sal_Int32   nCompVal = 1;
            const  ::com::sun::star::beans::Property*  pOldEnd = pEnd--;
            const  ::com::sun::star::beans::Property*  pMid = pCur;

            while( nCompVal != 0 && pCur <= pEnd )
            {
                pMid = (pEnd - pCur) / 2 + pCur;

                nCompVal = pReqProps[i].compareTo( pMid->Name );

                if( nCompVal > 0 )
                    pCur = pMid + 1;
                else
                    pEnd = pMid - 1;
            }

            if( nCompVal == 0 )
            {
                _pHandles[i] = pMid->Handle;
                nHitCount++;
                pCur = pMid +1;
            }
            else if( nCompVal > 0 )
            {
                _pHandles[i] = -1;
                pCur = pMid + 1;
            }
            else
            {
                _pHandles[i] = -1;
                pCur = pMid;
            }
            pEnd = pOldEnd;
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
        ::std::set< sal_Int32 >         m_aProperties;
        sal_Int32                       m_nCurrentlyForwarding;

    public:
        PropertyForwarder( OPropertySetAggregationHelper& _rAggregationHelper );
        ~PropertyForwarder();

        /** declares that the forwarder should be responsible for the given property

        @param _nHandle
            the public handle (<em>not</em> the original handle!) of the property
        */
        void    takeResponsibilityFor( sal_Int32 _nHandle );

        /** checks whether the forwarder is responsible for the given property
        */
        bool    isResponsibleFor( sal_Int32 _nHandle );

        
        void    doForward( sal_Int32 _nHandle, const Any& _rValue ) throw ( Exception );

        sal_Int32 getCurrentlyForwardedProperty( ) const { return m_nCurrentlyForwarding; }
    };

    
    PropertyForwarder::PropertyForwarder( OPropertySetAggregationHelper& _rAggregationHelper )
        :m_rAggregationHelper( _rAggregationHelper )
        ,m_nCurrentlyForwarding( -1 )
    {
    }

    
    PropertyForwarder::~PropertyForwarder()
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

    
    void PropertyForwarder::doForward( sal_Int32 _nHandle, const Any& _rValue ) throw ( Exception )
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
                    
            }
            catch( const Exception& )
            {
                m_rAggregationHelper.forwardedPropertyValue( _nHandle, false );
                throw;
            }

            m_nCurrentlyForwarding = -1;

            m_rAggregationHelper.forwardedPropertyValue( _nHandle, true );
        }
    }
}






OPropertySetAggregationHelper::OPropertySetAggregationHelper( ::cppu::OBroadcastHelper& rBHlp )
    :OPropertyStateHelper( rBHlp )
    ,m_bListening( false )
{
    m_pForwarder = new PropertyForwarder( *this );
}


OPropertySetAggregationHelper::~OPropertySetAggregationHelper()
{
    delete m_pForwarder;
}


 ::com::sun::star::uno::Any SAL_CALL OPropertySetAggregationHelper::queryInterface(const  ::com::sun::star::uno::Type& _rType) throw( ::com::sun::star::uno::RuntimeException)
{
     ::com::sun::star::uno::Any aReturn = OPropertyStateHelper::queryInterface(_rType);

    if ( !aReturn.hasValue() )
        aReturn = cppu::queryInterface(_rType
        ,static_cast< ::com::sun::star::beans::XPropertiesChangeListener*>(this)
        ,static_cast< ::com::sun::star::beans::XVetoableChangeListener*>(this)
        ,static_cast< ::com::sun::star::lang::XEventListener*>(static_cast< ::com::sun::star::beans::XPropertiesChangeListener*>(this))
        );

    return aReturn;
}


void OPropertySetAggregationHelper::disposing()
{
    osl::MutexGuard aGuard(rBHelper.rMutex);

    if ( m_xAggregateSet.is() && m_bListening )
    {
        
        m_xAggregateMultiSet->removePropertiesChangeListener(this);
        m_xAggregateSet->removeVetoableChangeListener(OUString(), this);
        m_bListening = false;
    }

    OPropertyStateHelper::disposing();
}


void SAL_CALL OPropertySetAggregationHelper::disposing(const  ::com::sun::star::lang::EventObject& _rSource) throw ( ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::disposing : don't have an aggregate anymore !");
    if (_rSource.Source == m_xAggregateSet)
        m_bListening = false;
}


void SAL_CALL OPropertySetAggregationHelper::propertiesChange(const  ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent>& _rEvents) throw( ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::propertiesChange : have no aggregate !");

    sal_Int32 nLen = _rEvents.getLength();
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();

    if (1 == nLen)
    {
        const  ::com::sun::star::beans::PropertyChangeEvent& evt = _rEvents.getConstArray()[0];
        OSL_ENSURE(!evt.PropertyName.isEmpty(), "OPropertySetAggregationHelper::propertiesChange : invalid event !");
            
        sal_Int32 nHandle = rPH.getHandleByName( evt.PropertyName );

        
        
        
        

        if ( ( nHandle != -1 ) && !isCurrentlyForwardingProperty( nHandle ) )
            fire(&nHandle, &evt.NewValue, &evt.OldValue, 1, sal_False);
    }
    else
    {
        sal_Int32* pHandles = new sal_Int32[nLen];
         ::com::sun::star::uno::Any* pNewValues = new  ::com::sun::star::uno::Any[nLen];
         ::com::sun::star::uno::Any* pOldValues = new  ::com::sun::star::uno::Any[nLen];

        const  ::com::sun::star::beans::PropertyChangeEvent* pEvents = _rEvents.getConstArray();
        sal_Int32 nDest = 0;
        for (sal_Int32 nSource=0; nSource<nLen; ++nSource, ++pEvents)
        {
            sal_Int32 nHandle = rPH.getHandleByName(pEvents->PropertyName);
            if ( ( nHandle != -1 ) && !isCurrentlyForwardingProperty( nHandle ) )
            {   
                pHandles[nDest] = nHandle;
                pNewValues[nDest] = pEvents->NewValue;
                pOldValues[nDest] = pEvents->OldValue;
                ++nDest;
            }
        }

        if (nDest)
            fire(pHandles, pNewValues, pOldValues, nDest, sal_False);

        delete[] pHandles;
        delete[] pNewValues;
        delete[] pOldValues;
    }
}


void SAL_CALL OPropertySetAggregationHelper::vetoableChange(const  ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::beans::PropertyVetoException,  ::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::vetoableChange : have no aggregate !");

    cppu::IPropertyArrayHelper& rPH = getInfoHelper();

    sal_Int32 nHandle = rPH.getHandleByName(_rEvent.PropertyName);
    fire(&nHandle, &_rEvent.NewValue, &_rEvent.OldValue, 1, sal_True);
}


void OPropertySetAggregationHelper::setAggregation(const  ::com::sun::star::uno::Reference<  ::com::sun::star::uno::XInterface >& _rxDelegate)
        throw(  ::com::sun::star::lang::IllegalArgumentException )
{
    osl::MutexGuard aGuard(rBHelper.rMutex);

    if (m_bListening && m_xAggregateSet.is())
    {
        m_xAggregateMultiSet->removePropertiesChangeListener(this);
        m_xAggregateSet->removeVetoableChangeListener(OUString(), this);
        m_bListening = false;
    }

    m_xAggregateState       =  m_xAggregateState.query( _rxDelegate );
    m_xAggregateSet         =  m_xAggregateSet.query( _rxDelegate );
    m_xAggregateMultiSet    =  m_xAggregateMultiSet.query( _rxDelegate );
    m_xAggregateFastSet     =  m_xAggregateFastSet.query( _rxDelegate );

    
    if ( m_xAggregateSet.is() && !m_xAggregateMultiSet.is() )
        throw  ::com::sun::star::lang::IllegalArgumentException();
}


void OPropertySetAggregationHelper::startListening()
{
    osl::MutexGuard aGuard(rBHelper.rMutex);

    if (!m_bListening && m_xAggregateSet.is())
    {
        
         ::com::sun::star::uno::Sequence< OUString > aPropertyNames;
        m_xAggregateMultiSet->addPropertiesChangeListener(aPropertyNames, this);
        m_xAggregateSet->addVetoableChangeListener(OUString(), this);

        m_bListening = true;
    }
}


void SAL_CALL OPropertySetAggregationHelper::addVetoableChangeListener(const OUString& _rPropertyName,
                                                                       const  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener>& _rxListener)
                                                                       throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::lang::WrappedTargetException,  ::com::sun::star::uno::RuntimeException)
{
    OPropertySetHelper::addVetoableChangeListener(_rPropertyName, _rxListener);
    if (!m_bListening)
        startListening();
}


void SAL_CALL OPropertySetAggregationHelper::addPropertyChangeListener(const OUString& _rPropertyName,
                                                                       const  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>& _rxListener)
                                                                       throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::lang::WrappedTargetException,  ::com::sun::star::uno::RuntimeException)
{
    OPropertySetHelper::addPropertyChangeListener(_rPropertyName, _rxListener);
    if (!m_bListening)
        startListening();
}


void SAL_CALL OPropertySetAggregationHelper::addPropertiesChangeListener(const  ::com::sun::star::uno::Sequence< OUString >& _rPropertyNames,
                                                                         const  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener>& _rxListener)
                                                                         throw( ::com::sun::star::uno::RuntimeException)
{
    OPropertySetHelper::addPropertiesChangeListener(_rPropertyNames, _rxListener);
    if (!m_bListening)
        startListening();
}


sal_Int32 OPropertySetAggregationHelper::getOriginalHandle(sal_Int32 nHandle) const
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)const_cast<OPropertySetAggregationHelper*>(this)->getInfoHelper();
    sal_Int32 nOriginalHandle = -1;
    rPH.fillAggregatePropertyInfoByHandle(NULL, &nOriginalHandle, nHandle);
    return nOriginalHandle;
}


OUString OPropertySetAggregationHelper::getPropertyName( sal_Int32 _nHandle ) const
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( const_cast<OPropertySetAggregationHelper*>(this)->getInfoHelper() );
    Property aProperty;
    OSL_VERIFY( rPH.getPropertyByHandle( _nHandle, aProperty ) );
    return aProperty.Name;
}


void SAL_CALL OPropertySetAggregationHelper::setFastPropertyValue(sal_Int32 _nHandle, const  ::com::sun::star::uno::Any& _rValue)
        throw(   ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::beans::PropertyVetoException,
                 ::com::sun::star::lang::IllegalArgumentException,  ::com::sun::star::lang::WrappedTargetException,
                 ::com::sun::star::uno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;

    
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, _nHandle))
        if (m_xAggregateFastSet.is())
            m_xAggregateFastSet->setFastPropertyValue(nOriginalHandle, _rValue);
        else
            m_xAggregateSet->setPropertyValue(aPropName, _rValue);
    else
        OPropertySetHelper::setFastPropertyValue(_nHandle, _rValue);
}


void OPropertySetAggregationHelper::getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)const_cast<OPropertySetAggregationHelper*>(this)->getInfoHelper();
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
        
        
        rValue = m_xAggregateSet->getPropertyValue( getPropertyName( nHandle ) );
    }
}


 ::com::sun::star::uno::Any SAL_CALL OPropertySetAggregationHelper::getFastPropertyValue(sal_Int32 nHandle)
        throw(   ::com::sun::star::beans::UnknownPropertyException,
                 ::com::sun::star::lang::WrappedTargetException,
                 ::com::sun::star::uno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
     ::com::sun::star::uno::Any  aValue;

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
    throw ( PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    OSL_ENSURE( !rBHelper.bInDispose, "OPropertySetAggregationHelper::setPropertyValues : do not use within the dispose call !");
    OSL_ENSURE( !rBHelper.bDisposed, "OPropertySetAggregationHelper::setPropertyValues : object is disposed" );

    
    if (!m_xAggregateSet.is())
        OPropertySetHelper::setPropertyValues(_rPropertyNames, _rValues);
    else if (_rPropertyNames.getLength() == 1) 
    {
        try
        {
            setPropertyValue( _rPropertyNames[0], _rValues[0] );
        }
        catch( const UnknownPropertyException& )
        {
            
        #if OSL_DEBUG_LEVEL > 0
            OStringBuffer aMessage;
            aMessage.append( "OPropertySetAggregationHelper::setPropertyValues: unknown property '" );
            aMessage.append( OUStringToOString( _rPropertyNames[0], RTL_TEXTENCODING_ASCII_US ) );
            aMessage.append( "'" );
            aMessage.append( "\n(implementation " );
            aMessage.append( typeid( *this ).name() );
            aMessage.append( ")" );
            OSL_FAIL( aMessage.getStr() );
        #endif
        }
    }
    else
    {
        OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );

        
        const OUString* pNames = _rPropertyNames.getConstArray();
        sal_Int32 nAggCount(0);
        sal_Int32 nLen(_rPropertyNames.getLength());

        for ( sal_Int32 i = 0; i < nLen; ++i, ++pNames )
        {
            OPropertyArrayAggregationHelper::PropertyOrigin ePropOrg = rPH.classifyProperty( *pNames );
            if ( OPropertyArrayAggregationHelper::UNKNOWN_PROPERTY == ePropOrg )
                throw WrappedTargetException( OUString(), static_cast< XMultiPropertySet* >( this ), makeAny( UnknownPropertyException( ) ) );
                
                

            if ( OPropertyArrayAggregationHelper::AGGREGATE_PROPERTY == ePropOrg )
                ++nAggCount;
        }

        pNames = _rPropertyNames.getConstArray();   

        
        if (nAggCount == nLen)
            m_xAggregateMultiSet->setPropertyValues(_rPropertyNames, _rValues);

        
        else if (nAggCount == 0)
            OPropertySetHelper::setPropertyValues(_rPropertyNames, _rValues);

        
        else
        {
            const  ::com::sun::star::uno::Any* pValues = _rValues.getConstArray();
             ::com::sun::star::uno::Any* pConvertedValues = NULL;
             ::com::sun::star::uno::Any* pOldValues = NULL;
            sal_Int32*  pHandles = NULL;

            try
            {
                

                
                Sequence< OUString > AggPropertyNames( nAggCount );
                OUString* pAggNames = AggPropertyNames.getArray();
                
                Sequence< Any >  AggValues( nAggCount );
                Any* pAggValues = AggValues.getArray();

                
                Sequence< OUString > DelPropertyNames( nLen - nAggCount );
                OUString* pDelNames = DelPropertyNames.getArray();

                
                Sequence< Any > DelValues( nLen - nAggCount );
                Any* pDelValues = DelValues.getArray();

                for ( sal_Int32 i = 0; i < nLen; ++i, ++pNames, ++pValues )
                {
                    if ( OPropertyArrayAggregationHelper::AGGREGATE_PROPERTY == rPH.classifyProperty( *pNames ) )
                    {
                        *pAggNames++ = *pNames;
                        *pAggValues++ = *pValues;
                    }
                    else
                    {
                        *pDelNames++ = *pNames;
                        *pDelValues++ = *pValues;
                    }
                }

                
                pDelValues = DelValues.getArray();

                pHandles = new sal_Int32[ nLen - nAggCount ];

                
                cppu::IPropertyArrayHelper& rPH2 = getInfoHelper();

                
                sal_Int32 nHitCount = rPH2.fillHandles( pHandles, DelPropertyNames );
                if (nHitCount != 0)
                {

                     pConvertedValues = new  ::com::sun::star::uno::Any[ nHitCount ];
                     pOldValues = new  ::com::sun::star::uno::Any[ nHitCount ];
                    nHitCount = 0;
                    sal_Int32 i;

                    {
                    
                        osl::MutexGuard aGuard( rBHelper.rMutex );
                        for( i = 0; i < (nLen - nAggCount); ++i )
                        {
                            if( pHandles[i] != -1 )
                            {
                                sal_Int16 nAttributes;
                                rPH2.fillPropertyMembersByHandle( NULL, &nAttributes, pHandles[i] );
                                if( nAttributes &  ::com::sun::star::beans::PropertyAttribute::READONLY )
                                    throw  ::com::sun::star::beans::PropertyVetoException();
                                
                                if( convertFastPropertyValue( pConvertedValues[ nHitCount ], pOldValues[nHitCount],
                                                            pHandles[i], pDelValues[i] ) )
                                {
                                    
                                    pHandles[nHitCount]         = pHandles[i];
                                    nHitCount++;
                                }
                            }
                        }
                    
                    }

                    
                    fire( pHandles, pConvertedValues, pOldValues, nHitCount, sal_True );

                    
                    m_xAggregateMultiSet->setPropertyValues(AggPropertyNames, AggValues);

                    {
                    
                        osl::MutexGuard aGuard( rBHelper.rMutex );
                        
                        for( i = 0; i < nHitCount; i++ )
                        {
                            
                            setFastPropertyValue_NoBroadcast( pHandles[i], pConvertedValues[i] );
                        }
                    
                    }

                    
                    fire( pHandles, pConvertedValues, pOldValues, nHitCount, sal_False );
                }
                else
                    m_xAggregateMultiSet->setPropertyValues(AggPropertyNames, AggValues);

            }
            catch(::com::sun::star::uno::Exception&)
            {
                delete [] pHandles;
                delete [] pOldValues;
                delete [] pConvertedValues;
                throw;
            }

            delete [] pHandles;
            delete [] pOldValues;
            delete [] pConvertedValues;
        }
    }
}



 ::com::sun::star::beans::PropertyState SAL_CALL OPropertySetAggregationHelper::getPropertyState(const OUString& _rPropertyName)
            throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    sal_Int32 nHandle = rPH.getHandleByName( _rPropertyName );

    if (nHandle == -1)
    {
        throw  ::com::sun::star::beans::UnknownPropertyException();
    }

    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateState.is())
            return m_xAggregateState->getPropertyState(_rPropertyName);
        else
            return  ::com::sun::star::beans::PropertyState_DIRECT_VALUE;
    }
    else
        return getPropertyStateByHandle(nHandle);
}


void SAL_CALL OPropertySetAggregationHelper::setPropertyToDefault(const OUString& _rPropertyName)
        throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    sal_Int32 nHandle = rPH.getHandleByName(_rPropertyName);
    if (nHandle == -1)
    {
        throw  ::com::sun::star::beans::UnknownPropertyException();
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


 ::com::sun::star::uno::Any SAL_CALL OPropertySetAggregationHelper::getPropertyDefault(const OUString& aPropertyName)
        throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::lang::WrappedTargetException,  ::com::sun::star::uno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
    sal_Int32 nHandle = rPH.getHandleByName( aPropertyName );

    if ( nHandle == -1 )
        throw  ::com::sun::star::beans::UnknownPropertyException();

    OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateState.is())
            return m_xAggregateState->getPropertyDefault(aPropertyName);
        else
            return  ::com::sun::star::uno::Any();
    }
    else
        return getPropertyDefaultByHandle(nHandle);
}


sal_Bool SAL_CALL OPropertySetAggregationHelper::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw(IllegalArgumentException)
{
    bool bModified = false;

    OSL_ENSURE( m_pForwarder->isResponsibleFor( _nHandle ), "OPropertySetAggregationHelper::convertFastPropertyValue: this is no forwarded property - did you use declareForwardedProperty for it?" );
    if ( m_pForwarder->isResponsibleFor( _nHandle ) )
    {
        
        OPropertyArrayAggregationHelper& rPH = static_cast< OPropertyArrayAggregationHelper& >( getInfoHelper() );
        Property aProperty;
        OSL_VERIFY( rPH.getPropertyByHandle( _nHandle, aProperty ) );

        Any aCurrentValue;
        getFastPropertyValue( aCurrentValue, _nHandle );
        bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, aCurrentValue, aProperty.Type );
    }

    return bModified;
}


void SAL_CALL OPropertySetAggregationHelper::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw ( Exception )
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


void SAL_CALL OPropertySetAggregationHelper::forwardingPropertyValue( sal_Int32 )
{
    
}


void SAL_CALL OPropertySetAggregationHelper::forwardedPropertyValue( sal_Int32, bool )
{
    
}


bool OPropertySetAggregationHelper::isCurrentlyForwardingProperty( sal_Int32 _nHandle ) const
{
    return m_pForwarder->getCurrentlyForwardedProperty() == _nHandle;
}


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
