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

#include <WrappedPropertySet.hxx>
#include <cppuhelper/propshlp.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <o3tl/temporary.hxx>
#include <sal/log.hxx>

namespace chart
{

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

WrappedPropertySet::WrappedPropertySet()
{
}
WrappedPropertySet::~WrappedPropertySet()
{
    clearWrappedPropertySet(o3tl::temporary(std::unique_lock(m_aMutex)));
}

Reference< beans::XPropertyState > WrappedPropertySet::getInnerPropertyState()
{
    return Reference< beans::XPropertyState >( getInnerPropertySet(), uno::UNO_QUERY );
}

void WrappedPropertySet::clearWrappedPropertySet(std::unique_lock<std::mutex>& rGuard)
{
    assert(rGuard.owns_lock());
    // Take the members out under the lock and destroy them after releasing
    // it: the destructors may release UNO references that require SolarMutex,
    // and we must not hold m_aMutex across that. Re-lock before returning so
    // the caller's guard stays in the state it was in on entry.
    {
        auto pArr = std::move(m_pPropertyArrayHelper);
        auto pMap = std::move(m_pWrappedPropertyMap);
        auto xInfo = std::move(m_xInfo);
        rGuard.unlock();
    }
    rGuard.lock();
}

//XPropertySet
Reference< beans::XPropertySetInfo > SAL_CALL WrappedPropertySet::getPropertySetInfo(  )
{
    Reference< beans::XPropertySetInfo > xInfo = m_xInfo;
    if( !xInfo.is() )
    {
        std::unique_lock aGuard(m_aMutex);
        xInfo = m_xInfo;
        if( !xInfo.is() )
        {
            xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper(aGuard) );
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            m_xInfo = xInfo;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return m_xInfo;
}

void SAL_CALL WrappedPropertySet::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
{
    std::unique_lock aGuard(m_aMutex);
    setPropertyValue(aGuard, rPropertyName, rValue);
}

void WrappedPropertySet::setPropertyValue( std::unique_lock<std::mutex>& rGuard, const OUString& rPropertyName, const Any& rValue )
{
    try
    {
        sal_Int32 nHandle = getInfoHelper(rGuard).getHandleByName( rPropertyName );
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rGuard, nHandle );
        Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
        if( pWrappedProperty )
        {
            // some of these call into things that take the solarmutex
            rGuard.unlock();
            pWrappedProperty->setPropertyValue( rValue, xInnerPropertySet );
            rGuard.lock();
        }
        else if( xInnerPropertySet.is() )
            xInnerPropertySet->setPropertyValue( rPropertyName, rValue );
        else
        {
            SAL_WARN("chart2.tools", "found no inner property set to map to");
        }
    }
    catch( const beans::UnknownPropertyException& )
    {
        throw;
    }
    catch( const beans::PropertyVetoException& )
    {
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        throw;
    }
    catch( const lang::WrappedTargetException& )
    {
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        throw;
    }
    catch( const uno::Exception& ex )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        TOOLS_WARN_EXCEPTION( "chart2", "invalid exception caught in WrappedPropertySet::setPropertyValue");
        throw lang::WrappedTargetException( ex.Message, nullptr, anyEx );
    }
}

Any SAL_CALL WrappedPropertySet::getPropertyValue( const OUString& rPropertyName )
{
    std::unique_lock aGuard(m_aMutex);
    return getPropertyValue(aGuard, rPropertyName);
}

Any WrappedPropertySet::getPropertyValue( std::unique_lock<std::mutex>& rGuard, const OUString& rPropertyName )
{
    Any aRet;

    try
    {
        sal_Int32 nHandle = getInfoHelper(rGuard).getHandleByName( rPropertyName );
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rGuard, nHandle );
        Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
        if( pWrappedProperty )
            aRet = pWrappedProperty->getPropertyValue( xInnerPropertySet );
        else if( xInnerPropertySet.is() )
            aRet = xInnerPropertySet->getPropertyValue( rPropertyName );
        else
        {
            SAL_WARN("chart2.tools", "found no inner property set to map to");
        }
    }
    catch( const beans::UnknownPropertyException& )
    {
        throw;
    }
    catch( const lang::WrappedTargetException& )
    {
        throw;
    }
    catch( const uno::RuntimeException& )
    {
        throw;
    }
    catch( const uno::Exception& ex )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        TOOLS_WARN_EXCEPTION( "chart2", "invalid exception caught in WrappedPropertySet::setPropertyValue");
        throw lang::WrappedTargetException( ex.Message, nullptr, anyEx );
    }

    return aRet;
}

void SAL_CALL WrappedPropertySet::addPropertyChangeListener( const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& xListener )
{
    std::unique_lock aGuard(m_aMutex);
    addPropertyChangeListener(aGuard, rPropertyName, xListener);
}

void WrappedPropertySet::addPropertyChangeListener( std::unique_lock<std::mutex>& rGuard, const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& xListener )
{
    Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
    if( xInnerPropertySet.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rGuard, rPropertyName );
        if( pWrappedProperty )
            xInnerPropertySet->addPropertyChangeListener( pWrappedProperty->getInnerName(), xListener );
        else
            xInnerPropertySet->addPropertyChangeListener( rPropertyName, xListener );
    }
}

void SAL_CALL WrappedPropertySet::removePropertyChangeListener( const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& aListener )
{
    std::unique_lock aGuard(m_aMutex);
    removePropertyChangeListener(aGuard, rPropertyName, aListener);
}

void WrappedPropertySet::removePropertyChangeListener( std::unique_lock<std::mutex>& rGuard, const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& aListener )
{
    Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
    if( xInnerPropertySet.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rGuard, rPropertyName );
        if( pWrappedProperty )
            xInnerPropertySet->removePropertyChangeListener( pWrappedProperty->getInnerName(), aListener );
        else
            xInnerPropertySet->removePropertyChangeListener( rPropertyName, aListener );
    }
}
void SAL_CALL WrappedPropertySet::addVetoableChangeListener( const OUString& rPropertyName, const Reference< beans::XVetoableChangeListener >& aListener )
{
    std::unique_lock aGuard(m_aMutex);
    Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
    if( xInnerPropertySet.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( aGuard, rPropertyName );
        if( pWrappedProperty )
            xInnerPropertySet->addVetoableChangeListener( pWrappedProperty->getInnerName(), aListener );
        else
            xInnerPropertySet->addVetoableChangeListener( rPropertyName, aListener );
    }
}
void SAL_CALL WrappedPropertySet::removeVetoableChangeListener( const OUString& rPropertyName, const Reference< beans::XVetoableChangeListener >& aListener )
{
    std::unique_lock aGuard(m_aMutex);
    Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
    if( xInnerPropertySet.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( aGuard, rPropertyName );
        if( pWrappedProperty )
            xInnerPropertySet->removeVetoableChangeListener( pWrappedProperty->getInnerName(), aListener );
        else
            xInnerPropertySet->removeVetoableChangeListener( rPropertyName, aListener );
    }
}

//XMultiPropertySet
void SAL_CALL WrappedPropertySet::setPropertyValues( const Sequence< OUString >& rNameSeq, const Sequence< Any >& rValueSeq )
{
    bool bUnknownProperty = false;
    sal_Int32 nMinCount = std::min( rValueSeq.getLength(), rNameSeq.getLength() );
    for(sal_Int32 nN=0; nN<nMinCount; nN++)
    {
        OUString aPropertyName( rNameSeq[nN] );
        try
        {
            setPropertyValue( aPropertyName, rValueSeq[nN] );
        }
        catch( const beans::UnknownPropertyException& )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
            bUnknownProperty = true;
        }
    }
    //todo: store unknown properties elsewhere
    OSL_ENSURE(!bUnknownProperty,"unknown property");
//    if( bUnknownProperty )
//        throw beans::UnknownPropertyException();
}
Sequence< Any > SAL_CALL WrappedPropertySet::getPropertyValues( const Sequence< OUString >& rNameSeq )
{
    Sequence< Any > aRetSeq;
    if( rNameSeq.hasElements() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        auto pRetSeq = aRetSeq.getArray();
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            try
            {
                OUString aPropertyName( rNameSeq[nN] );
                pRetSeq[nN] = getPropertyValue( aPropertyName );
            }
            catch( const beans::UnknownPropertyException& )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
            catch( const lang::WrappedTargetException& )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }
    return aRetSeq;
}
void SAL_CALL WrappedPropertySet::addPropertiesChangeListener( const Sequence< OUString >& /* rNameSeq */, const Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented yet");
    //todo
}
void SAL_CALL WrappedPropertySet::removePropertiesChangeListener( const Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented yet");
    //todo
}
void SAL_CALL WrappedPropertySet::firePropertiesChangeEvent( const Sequence< OUString >& /* rNameSeq */, const Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented yet");
    //todo
}

//XPropertyState
beans::PropertyState SAL_CALL WrappedPropertySet::getPropertyState( const OUString& rPropertyName )
{
    std::unique_lock aGuard(m_aMutex);
    return getPropertyState(aGuard, rPropertyName);
}

beans::PropertyState WrappedPropertySet::getPropertyState( std::unique_lock<std::mutex>& rGuard, const OUString& rPropertyName )
{
    beans::PropertyState aState( beans::PropertyState_DIRECT_VALUE );

    Reference< beans::XPropertyState > xInnerPropertyState( getInnerPropertyState() );
    if( xInnerPropertyState.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rGuard, rPropertyName );
        if( pWrappedProperty )
            aState = pWrappedProperty->getPropertyState( xInnerPropertyState );
        else
            aState = xInnerPropertyState->getPropertyState( rPropertyName );
    }
    return aState;
}

const WrappedProperty* WrappedPropertySet::getWrappedProperty( std::unique_lock<std::mutex>& rGuard, const OUString& rOuterName )
{
    sal_Int32 nHandle = getInfoHelper(rGuard).getHandleByName( rOuterName );
    return getWrappedProperty( rGuard, nHandle );
}

const WrappedProperty* WrappedPropertySet::getWrappedProperty( std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle )
{
    tWrappedPropertyMap::const_iterator aFound( getWrappedPropertyMap(rGuard).find( nHandle ) );
    if( aFound != getWrappedPropertyMap(rGuard).end() )
        return (*aFound).second.get();
    return nullptr;
}

Sequence< beans::PropertyState > SAL_CALL WrappedPropertySet::getPropertyStates( const Sequence< OUString >& rNameSeq )
{
    Sequence< beans::PropertyState > aRetSeq;
    if( rNameSeq.hasElements() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        auto pRetSeq = aRetSeq.getArray();
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            pRetSeq[nN] = getPropertyState( aPropertyName );
        }
    }
    return aRetSeq;
}

void SAL_CALL WrappedPropertySet::setPropertyToDefault( const OUString& rPropertyName )
{
    std::unique_lock aGuard(m_aMutex);
    Reference< beans::XPropertyState > xInnerPropertyState( getInnerPropertyState() );
    if( xInnerPropertyState.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( aGuard, rPropertyName );
        if( pWrappedProperty )
            pWrappedProperty->setPropertyToDefault( xInnerPropertyState );
        else
            xInnerPropertyState->setPropertyToDefault( rPropertyName );
    }
}
Any SAL_CALL WrappedPropertySet::getPropertyDefault( const OUString& rPropertyName )
{
    std::unique_lock aGuard(m_aMutex);
    Any aRet;
    Reference< beans::XPropertyState > xInnerPropertyState( getInnerPropertyState() );
    if( xInnerPropertyState.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( aGuard, rPropertyName );
        if( pWrappedProperty )
            aRet = pWrappedProperty->getPropertyDefault(xInnerPropertyState);
        else
            aRet = xInnerPropertyState->getPropertyDefault( rPropertyName );
    }
    return aRet;
}

//XMultiPropertyStates
void SAL_CALL WrappedPropertySet::setAllPropertiesToDefault(  )
{
    const Sequence< beans::Property >&  rPropSeq = getPropertySequence();
    for(beans::Property const & prop : rPropSeq)
    {
        setPropertyToDefault( prop.Name );
    }
}
void SAL_CALL WrappedPropertySet::setPropertiesToDefault( const Sequence< OUString >& rNameSeq )
{
    for(OUString const & s : rNameSeq)
    {
        setPropertyToDefault( s );
    }
}
Sequence< Any > SAL_CALL WrappedPropertySet::getPropertyDefaults( const Sequence< OUString >& rNameSeq )
{
    Sequence< Any > aRetSeq;
    if( rNameSeq.hasElements() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        auto pRetSeq = aRetSeq.getArray();
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            pRetSeq[nN] = getPropertyDefault( aPropertyName );
        }
    }
    return aRetSeq;
}

::cppu::IPropertyArrayHelper& WrappedPropertySet::getInfoHelper(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if(!m_pPropertyArrayHelper)
    {
        m_pPropertyArrayHelper = std::make_unique<::cppu::OPropertyArrayHelper>( getPropertySequence(), true );
    }
    return *m_pPropertyArrayHelper;
}

tWrappedPropertyMap& WrappedPropertySet::getWrappedPropertyMap(std::unique_lock<std::mutex>& rGuard)
{
    if(!m_pWrappedPropertyMap)
    {
        std::vector< std::unique_ptr<WrappedProperty> > aPropList( createWrappedProperties() );
        auto p = new tWrappedPropertyMap;

        for (auto & elem : aPropList)
        {
            sal_Int32 nHandle = getInfoHelper(rGuard).getHandleByName( elem->getOuterName() );

            if( nHandle == -1 )
            {
                OSL_FAIL( "missing property in property list" );
            }
            else if( p->find( nHandle ) != p->end() )
            {
                //duplicate Wrapped property
                OSL_FAIL( "duplicate Wrapped property" );
            }
            else
                (*p)[ nHandle ] = std::move(elem);
        }

        m_pWrappedPropertyMap.reset(p);
    }
    return *m_pWrappedPropertyMap;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
