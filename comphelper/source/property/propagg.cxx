/*************************************************************************
 *
 *  $RCSfile: propagg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#include <comphelper/propagg.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif


//.........................................................................
namespace comphelper
{
//.........................................................................

//==================================================================
//= OPropertyArrayAggregationHelper
//==================================================================

//------------------------------------------------------------------------------
OPropertyArrayAggregationHelper::OPropertyArrayAggregationHelper(
                                    const staruno::Sequence<starbeans::Property>& _rProperties,
                                    const staruno::Sequence<starbeans::Property>& _rAggProperties,
                                    IPropertyInfoService* _pInfoService,
                                    sal_Int32 _nFirstAggregateId)
                                :m_aProperties(_rProperties)
{
    m_aProperties.realloc(_rProperties.getLength() + _rAggProperties.getLength());

    const starbeans::Property* pAggregateProps = _rAggProperties.getConstArray();
    const starbeans::Property* pDelegateProps   = _rProperties.getConstArray();
    starbeans::Property* pProps = m_aProperties.getArray();

    // create the map for the delegator properties
    sal_Int32 i = 0;
    for (; i < _rProperties.getLength(); ++i, ++pDelegateProps)
        m_aPropertyAccessors[pDelegateProps->Handle] = internal::OPropertyAccessor(-1, i, sal_False);

    // create the map for the aggregate properties
    sal_Int32 nAggregateHandle = _nFirstAggregateId;
    sal_Int32 j = 0;
    pProps += _rProperties.getLength();
    for (; i < m_aProperties.getLength(); ++i, ++pProps)
    {
        *pProps = pAggregateProps[j++];

        sal_Int32 nHandle(-1);
        if (_pInfoService)
            nHandle = _pInfoService->getPreferedPropertyId(pProps->Name);

        if (-1 == nHandle)
            nHandle = nAggregateHandle++;
        else
        {   // check if we alread have a property with the given handle
            const starbeans::Property* pPropsTilNow = m_aProperties.getConstArray();
            for (sal_Int32 k=0; k<i; ++k, ++pPropsTilNow)
                if (pPropsTilNow->Handle == nHandle)
                {   // conflicts -> use another one (which we don't check anymore, assuming _nFirstAggregateId was large enough)
                    nHandle = nAggregateHandle++;
                    break;
                }
        }

        m_aPropertyAccessors[nHandle] = internal::OPropertyAccessor(pProps->Handle, i, sal_True);
        pProps->Handle = nHandle;
    }
    pProps = m_aProperties.getArray();  // reset, needed again below

    // sortieren der Properties nach Namen
    qsort((void*) pProps, m_aProperties.getLength(), sizeof(starbeans::Property), &PropertyCompare);

    // Positionen in der Map abgleichen
    for (i = 0; i < m_aProperties.getLength(); ++i, ++pProps)
        m_aPropertyAccessors[pProps->Handle].nPos = i;
}

//------------------------------------------------------------------
starbeans::Property OPropertyArrayAggregationHelper::getPropertyByName(const ::rtl::OUString& _rPropertyName)
                                throw(starbeans::UnknownPropertyException)
{
    starbeans::Property* pProperty = findPropertyByName(_rPropertyName);

    if (!pProperty)
        throw starbeans::UnknownPropertyException();

    return *pProperty;
}

//------------------------------------------------------------------------------
sal_Bool OPropertyArrayAggregationHelper::hasPropertyByName(const ::rtl::OUString& _rPropertyName)
{
    return findPropertyByName(_rPropertyName) != NULL;
}

//------------------------------------------------------------------------------
starbeans::Property* OPropertyArrayAggregationHelper::findPropertyByName(const ::rtl::OUString& _rName) const
{
    starbeans::Property aSearch;
    aSearch.Name = _rName;

    const starbeans::Property* pAry = m_aProperties.getConstArray();
    return (starbeans::Property*) bsearch(&aSearch,
                                (void*)pAry,
                                 m_aProperties.getLength(),
                                 sizeof(starbeans::Property),
                                 &PropertyCompare);
}

//------------------------------------------------------------------------------
sal_Int32 OPropertyArrayAggregationHelper::getHandleByName(const ::rtl::OUString& _rPropertyName)
{
    starbeans::Property* pProperty = findPropertyByName(_rPropertyName);
    return pProperty ? pProperty->Handle : -1;
}

//------------------------------------------------------------------------------
sal_Bool OPropertyArrayAggregationHelper::fillPropertyMembersByHandle(
            ::rtl::OUString* _pPropName, sal_Int16* _pAttributes, sal_Int32 _nHandle)
{
    internal::ConstPropertyAccessorMapIterator i = m_aPropertyAccessors.find(_nHandle);
    sal_Bool bRet = i != m_aPropertyAccessors.end();
    if (bRet)
    {
        const starbeans::Property& rProperty = m_aProperties.getConstArray()[(*i).second.nPos];
        if (_pPropName)
            *_pPropName = rProperty.Name;
        if (_pAttributes)
            *_pAttributes = rProperty.Attributes;
    }
    return bRet;
}

//------------------------------------------------------------------------------
sal_Bool OPropertyArrayAggregationHelper::fillAggregatePropertyInfoByHandle(
            ::rtl::OUString* _pPropName, sal_Int32* _pOriginalHandle, sal_Int32 _nHandle) const
{
    internal::ConstPropertyAccessorMapIterator i = m_aPropertyAccessors.find(_nHandle);
    sal_Bool bRet = i != m_aPropertyAccessors.end() && (*i).second.bAggregate;
    if (bRet)
    {
        if (_pOriginalHandle)
            *_pOriginalHandle = (*i).second.nOriginalHandle;
        if (_pPropName)
        {
            const starbeans::Property& rProperty = m_aProperties.getConstArray()[(*i).second.nPos];
            *_pPropName = rProperty.Name;
        }
    }
    return bRet;
}


//------------------------------------------------------------------------------
staruno::Sequence<starbeans::Property> OPropertyArrayAggregationHelper::getProperties()
{
    return m_aProperties;
}


//------------------------------------------------------------------------------
sal_Int32 OPropertyArrayAggregationHelper::fillHandles(
        sal_Int32* _pHandles, const staruno::Sequence< ::rtl::OUString >& _rPropNames )
{
    sal_Int32 nHitCount = 0;
    const ::rtl::OUString* pReqProps = _rPropNames.getConstArray();
    sal_Int32 nReqLen = _rPropNames.getLength();

    const starbeans::Property* pCur = m_aProperties.getConstArray();
    const starbeans::Property* pEnd = m_aProperties.getConstArray() + m_aProperties.getLength();

    for( sal_Int32 i = 0; i < nReqLen; ++i )
    {
        // Logarithmus ermitteln
        sal_uInt32 n = (sal_uInt32)(pEnd - pCur);
        sal_Int32 nLog = 0;
        while( n )
        {
            nLog += 1;
            n = n >> 1;
        }

        // Anzahl der noch zu suchenden Properties * dem Log2 der verbleibenden
        // zu dursuchenden Properties.
        if( (nReqLen - i) * nLog >= pEnd - pCur )
        {
            // linear search is better
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
            // binary search is better
            sal_Int32   nCompVal = 1;
            const starbeans::Property*  pOldEnd = pEnd--;
            const starbeans::Property*  pMid = pCur;

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

//==================================================================
//= OPropertySetAggregationHelper
//==================================================================

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OPropertySetAggregationHelper::queryInterface(const staruno::Type& _rType) throw(staruno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = OPropertyStateHelper::queryInterface(_rType);

    if (!aReturn.hasValue())
        aReturn = cppu::queryInterface(_rType
        ,static_cast<starbeans::XPropertiesChangeListener*>(this)
        ,static_cast<starbeans::XVetoableChangeListener*>(this)
        ,static_cast<starlang::XEventListener*>(static_cast<starbeans::XPropertiesChangeListener*>(this))
        );

    return aReturn;
}

//------------------------------------------------------------------------------
void OPropertySetAggregationHelper::disposing()
{
    osl::MutexGuard aGuard(rBHelper.rMutex);

    if (m_xAggregateSet.is() && m_bListening)
    {
        // als einziger Listener anmelden
        m_xAggregateMultiSet->removePropertiesChangeListener(this);
        m_xAggregateSet->removeVetoableChangeListener(::rtl::OUString(), this);
        m_bListening = sal_False;
    }

    OPropertyStateHelper::disposing();
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::disposing(const starlang::EventObject& _rSource) throw (staruno::RuntimeException)
{
    OSL_ENSHURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::disposing : don't have an aggregate anymore !");
    if (_rSource.Source == m_xAggregateSet)
        m_bListening = sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::propertiesChange(const staruno::Sequence<starbeans::PropertyChangeEvent>& _rEvents) throw(staruno::RuntimeException)
{
    OSL_ENSHURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::propertiesChange : have no aggregate !");

    sal_Int32 nLen = _rEvents.getLength();
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();

    if (1 == nLen)
    {
        const starbeans::PropertyChangeEvent& evt = _rEvents.getConstArray()[0];
        OSL_ENSHURE(evt.PropertyName.getLength() > 0, "OPropertySetAggregationHelper::propertiesChange : invalid event !");
            // we had a bug where this assertion would have us saved a whole day :) (72514)
        sal_Int32 nHandle = rPH.getHandleByName( evt.PropertyName );
        if (nHandle != -1)
            fire(&nHandle, &evt.NewValue, &evt.OldValue, 1, sal_False);
        // if nHandle is -1 the event marks for a (aggregate) property which we hide to callers
    }
    else
    {
        sal_Int32* pHandles = new sal_Int32[nLen];
        staruno::Any* pNewValues = new staruno::Any[nLen];
        staruno::Any* pOldValues = new staruno::Any[nLen];

        const starbeans::PropertyChangeEvent* pEvents = _rEvents.getConstArray();
        sal_Int32 nDest = 0;
        for (sal_Int32 nSource=0; nSource<nLen; ++nSource, ++pEvents)
        {
            sal_Int32 nHandle = rPH.getHandleByName(pEvents->PropertyName);
            if (nHandle != -1)
            {   // same as above : -1 is valid (73247) ...
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

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::vetoableChange(const starbeans::PropertyChangeEvent& _rEvent) throw(starbeans::PropertyVetoException, staruno::RuntimeException)
{
    OSL_ENSHURE(m_xAggregateSet.is(), "OPropertySetAggregationHelper::vetoableChange : have no aggregate !");

    cppu::IPropertyArrayHelper& rPH = getInfoHelper();

    sal_Int32 nHandle = rPH.getHandleByName(_rEvent.PropertyName);
    fire(&nHandle, &_rEvent.NewValue, &_rEvent.OldValue, 1, sal_True);
}

//------------------------------------------------------------------------------
void OPropertySetAggregationHelper::setAggregation(const staruno::Reference< staruno::XInterface >& _rxDelegate)
        throw( starlang::IllegalArgumentException )
{
    osl::MutexGuard aGuard(rBHelper.rMutex);

    if (m_bListening && m_xAggregateSet.is())
    {
        m_xAggregateMultiSet->removePropertiesChangeListener(this);
        m_xAggregateSet->removeVetoableChangeListener(::rtl::OUString(), this);
        m_bListening = sal_False;
    }

    m_xAggregateState       = staruno::Reference<starbeans::XPropertyState>(_rxDelegate, staruno::UNO_QUERY);
    m_xAggregateSet         = staruno::Reference<starbeans::XPropertySet>(_rxDelegate, staruno::UNO_QUERY);
    m_xAggregateMultiSet    = staruno::Reference<starbeans::XMultiPropertySet>(_rxDelegate, staruno::UNO_QUERY);
    m_xAggregateFastSet     = staruno::Reference<starbeans::XFastPropertySet>(_rxDelegate, staruno::UNO_QUERY);

    // must support XPropertySet and XMultiPropertySet
    if (m_xAggregateSet.is() && !m_xAggregateMultiSet.is())
    {
        throw starlang::IllegalArgumentException();
    }
}

//------------------------------------------------------------------------------
void OPropertySetAggregationHelper::startListening()
{
    if (!m_bListening && m_xAggregateSet.is())
    {
        osl::MutexGuard aGuard(rBHelper.rMutex);

        // als einziger Listener anmelden
        staruno::Sequence< ::rtl::OUString > aPropertyNames;
        m_xAggregateMultiSet->addPropertiesChangeListener(aPropertyNames, this);
        m_xAggregateSet->addVetoableChangeListener(::rtl::OUString(), this);

        m_bListening = sal_True;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::addVetoableChangeListener(const ::rtl::OUString& _rPropertyName,
                                                                       const staruno::Reference<starbeans::XVetoableChangeListener>& _rxListener)
                                                                       throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    OPropertySetHelper::addVetoableChangeListener(_rPropertyName, _rxListener);
    if (!m_bListening)
        startListening();
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::addPropertyChangeListener(const ::rtl::OUString& _rPropertyName,
                                                                       const staruno::Reference<starbeans::XPropertyChangeListener>& _rxListener)
                                                                       throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    OPropertySetHelper::addPropertyChangeListener(_rPropertyName, _rxListener);
    if (!m_bListening)
        startListening();
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::addPropertiesChangeListener(const staruno::Sequence< ::rtl::OUString >& _rPropertyNames,
                                                                         const staruno::Reference<starbeans::XPropertiesChangeListener>& _rxListener)
                                                                         throw(staruno::RuntimeException)
{
    OPropertySetHelper::addPropertiesChangeListener(_rPropertyNames, _rxListener);
    if (!m_bListening)
        startListening();
}

//------------------------------------------------------------------------------
sal_Int32 OPropertySetAggregationHelper::getOriginalHandle(sal_Int32 nHandle) const
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)const_cast<OPropertySetAggregationHelper*>(this)->getInfoHelper();
    sal_Int32 nOriginalHandle = -1;
    rPH.fillAggregatePropertyInfoByHandle(NULL, &nOriginalHandle, nHandle);
    return nOriginalHandle;
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::setFastPropertyValue(sal_Int32 _nHandle, const staruno::Any& _rValue)
        throw(  starbeans::UnknownPropertyException, starbeans::PropertyVetoException,
                starlang::IllegalArgumentException, starlang::WrappedTargetException,
                staruno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)getInfoHelper();
    ::rtl::OUString aPropName;
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

//------------------------------------------------------------------------------
void OPropertySetAggregationHelper::getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle) const
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)const_cast<OPropertySetAggregationHelper*>(this)->getInfoHelper();
    ::rtl::OUString aPropName;
    sal_Int32   nOriginalHandle = -1;

    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateFastSet.is())
            rValue = m_xAggregateFastSet->getFastPropertyValue(nOriginalHandle);
        else
            rValue = m_xAggregateSet->getPropertyValue(aPropName);
    }
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OPropertySetAggregationHelper::getFastPropertyValue(sal_Int32 nHandle)
        throw(  starbeans::UnknownPropertyException,
                starlang::WrappedTargetException,
                staruno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)getInfoHelper();
    ::rtl::OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    staruno::Any  aValue;

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

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::setPropertyValues(
                const staruno::Sequence< ::rtl::OUString >& _rPropertyNames,
                const staruno::Sequence<staruno::Any>& _rValues)
                throw(  starbeans::PropertyVetoException,
                        starlang::IllegalArgumentException,
                        starlang::WrappedTargetException,
                        staruno::RuntimeException)
{
    OSL_ENSHURE( !rBHelper.bInDispose, "OPropertySetAggregationHelper::setPropertyValues : do not use within the dispose call !");
    OSL_ENSHURE( !rBHelper.bDisposed, "OPropertySetAggregationHelper::setPropertyValues : object is disposed" );

    // check where the properties come from
    if (!m_xAggregateSet.is())
        OPropertySetHelper::setPropertyValues(_rPropertyNames, _rValues);
    else if (_rPropertyNames.getLength() == 1) // use the more efficient way
        setPropertyValue(_rPropertyNames.getConstArray()[0], _rValues.getConstArray()[0]);
    else
    {
        // gehoeren die Properties alle zum aggregierten Object ?
        staruno::Reference<starbeans::XPropertySetInfo> xInfo(m_xAggregateSet->getPropertySetInfo());
        const ::rtl::OUString* pNames = _rPropertyNames.getConstArray();
        sal_Int32 nAggCount(0);
        sal_Int32 nLen(_rPropertyNames.getLength());

        for (sal_Int32 i = 0; i < nLen; ++i, ++pNames)
            if (xInfo->hasPropertyByName(*pNames))
                nAggCount++;
        pNames = _rPropertyNames.getConstArray();   // reset, we'll need it again below ...

        // all properties belong to the aggregate
        if (nAggCount == nLen)
            m_xAggregateMultiSet->setPropertyValues(_rPropertyNames, _rValues);

        // all properties belong to the aggregating object
        else if (nAggCount == 0)
            OPropertySetHelper::setPropertyValues(_rPropertyNames, _rValues);

        // mixed
        else
        {
            const staruno::Any* pValues = _rValues.getConstArray();
            staruno::Any* pConvertedValues = NULL;
            staruno::Any* pOldValues = NULL;
            sal_Int32*  pHandles = NULL;

            try
            {
                // dividing the Names and _rValues
                staruno::Sequence< ::rtl::OUString > AggPropertyNames(nAggCount);
                ::rtl::OUString* pAggNames = AggPropertyNames.getArray();
                staruno::Sequence<staruno::Any>  AggValues(nAggCount);
                staruno::Any* pAggValues = AggValues.getArray();
                staruno::Sequence< ::rtl::OUString > OwnPropertyNames(nLen - nAggCount);
                ::rtl::OUString* pOwnNames = AggPropertyNames.getArray();
                staruno::Sequence<staruno::Any>  OwnValues(nLen - nAggCount);
                staruno::Any* pOwnValues = OwnValues.getArray();

                for (sal_Int32 i = 0, a = 0, o = 0; i < nLen; ++i)
                {
                    if (xInfo->hasPropertyByName(pNames[i]))
                    {
                        pAggNames[a] = pNames[i];
                        pAggValues[a++] = pValues[i];
                    }
                    else
                    {
                        pOwnNames[o] = pNames[i];
                        pOwnValues[o++] = pValues[i];
                    }
                }

                pHandles = new sal_Int32[ nLen - nAggCount ];

                // get the map table
                cppu::IPropertyArrayHelper& rPH = getInfoHelper();

                // fill the handle array
                sal_Int32 nHitCount = rPH.fillHandles( pHandles, OwnPropertyNames );
                if (nHitCount != 0)
                {

                    staruno::Any * pConvertedValues = new staruno::Any[ nHitCount ];
                    staruno::Any * pOldValues = new staruno::Any[ nHitCount ];
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
                                rPH.fillPropertyMembersByHandle( NULL, &nAttributes, pHandles[i] );
                                if( nAttributes & starbeans::PropertyAttribute::READONLY )
                                    throw starbeans::PropertyVetoException();
                                // Will the property change?
                                if( convertFastPropertyValue( pConvertedValues[ nHitCount ], pOldValues[nHitCount],
                                                            pHandles[i], pOwnValues[i] ) )
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
                    fire( pHandles, pConvertedValues, pOldValues, nHitCount, sal_True );

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
                    fire( pHandles, pConvertedValues, pOldValues, nHitCount, sal_False );
                }
                else
                    m_xAggregateMultiSet->setPropertyValues(AggPropertyNames, AggValues);

            }
            catch(...)
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

// XPropertyState
//------------------------------------------------------------------------------
starbeans::PropertyState SAL_CALL OPropertySetAggregationHelper::getPropertyState(const ::rtl::OUString& _rPropertyName)
            throw(starbeans::UnknownPropertyException, staruno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName( _rPropertyName );

    if (nHandle == -1)
    {
        throw starbeans::UnknownPropertyException();
    }

    ::rtl::OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateState.is())
            return m_xAggregateState->getPropertyState(_rPropertyName);
        else
            return starbeans::PropertyState_DIRECT_VALUE;
    }
    else
        return getPropertyStateByHandle(nHandle);
}

//------------------------------------------------------------------------------
void SAL_CALL OPropertySetAggregationHelper::setPropertyToDefault(const ::rtl::OUString& _rPropertyName)
        throw(starbeans::UnknownPropertyException, staruno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName(_rPropertyName);
    if (nHandle == -1)
    {
        throw starbeans::UnknownPropertyException();
    }

    ::rtl::OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateState.is())
            m_xAggregateState->setPropertyToDefault(_rPropertyName);
    }
    else
        setPropertyToDefaultByHandle(nHandle);
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OPropertySetAggregationHelper::getPropertyDefault(const ::rtl::OUString& aPropertyName)
        throw(starbeans::UnknownPropertyException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName( aPropertyName );

    if (nHandle == -1)
    {
        throw starbeans::UnknownPropertyException();
    }

    ::rtl::OUString aPropName;
    sal_Int32   nOriginalHandle = -1;
    if (rPH.fillAggregatePropertyInfoByHandle(&aPropName, &nOriginalHandle, nHandle))
    {
        if (m_xAggregateState.is())
            return m_xAggregateState->getPropertyDefault(aPropertyName);
        else
            return staruno::Any();
    }
    else
        return getPropertyDefaultByHandle(nHandle);
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

