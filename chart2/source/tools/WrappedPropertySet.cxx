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

#include <tools/solar.h>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

namespace chart
{

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

WrappedPropertySet::WrappedPropertySet()
                    : MutexContainer()
                    , m_xInfo(nullptr)
                    , m_pPropertyArrayHelper(nullptr)
                    , m_pWrappedPropertyMap(nullptr)
{
}
WrappedPropertySet::~WrappedPropertySet()
{
    clearWrappedPropertySet();
}

Reference< beans::XPropertyState > WrappedPropertySet::getInnerPropertyState()
{
    return Reference< beans::XPropertyState >( getInnerPropertySet(), uno::UNO_QUERY );
}

void WrappedPropertySet::clearWrappedPropertySet()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );//do not use different mutex than is already used for static property sequence

    m_pPropertyArrayHelper.reset();
    m_pWrappedPropertyMap.reset();

    m_xInfo = nullptr;
}

//XPropertySet
Reference< beans::XPropertySetInfo > SAL_CALL WrappedPropertySet::getPropertySetInfo(  )
{
    Reference< beans::XPropertySetInfo > xInfo = m_xInfo;
    if( !xInfo.is() )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );//do not use different mutex than is already used for static property sequence
        xInfo = m_xInfo;
        if( !xInfo.is() )
        {
            xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
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
    try
    {
        sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
        const WrappedProperty* pWrappedProperty = getWrappedProperty( nHandle );
        Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
        if( pWrappedProperty )
            pWrappedProperty->setPropertyValue( rValue, xInnerPropertySet );
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
        OSL_FAIL("invalid exception caught in WrappedPropertySet::setPropertyValue");
        throw lang::WrappedTargetException( ex.Message, nullptr, anyEx );
    }
}
Any SAL_CALL WrappedPropertySet::getPropertyValue( const OUString& rPropertyName )
{
    Any aRet;

    try
    {
        sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
        const WrappedProperty* pWrappedProperty = getWrappedProperty( nHandle );
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
        OSL_FAIL("invalid exception caught in WrappedPropertySet::setPropertyValue");
        throw lang::WrappedTargetException( ex.Message, nullptr, anyEx );
    }

    return aRet;
}

void SAL_CALL WrappedPropertySet::addPropertyChangeListener( const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& xListener )
{
    Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
    if( xInnerPropertySet.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
        if( pWrappedProperty )
            xInnerPropertySet->addPropertyChangeListener( pWrappedProperty->getInnerName(), xListener );
        else
            xInnerPropertySet->addPropertyChangeListener( rPropertyName, xListener );
    }
}
void SAL_CALL WrappedPropertySet::removePropertyChangeListener( const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& aListener )
{
    Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
    if( xInnerPropertySet.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
        if( pWrappedProperty )
            xInnerPropertySet->removePropertyChangeListener( pWrappedProperty->getInnerName(), aListener );
        else
            xInnerPropertySet->removePropertyChangeListener( rPropertyName, aListener );
    }
}
void SAL_CALL WrappedPropertySet::addVetoableChangeListener( const OUString& rPropertyName, const Reference< beans::XVetoableChangeListener >& aListener )
{
    Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
    if( xInnerPropertySet.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
        if( pWrappedProperty )
            xInnerPropertySet->addVetoableChangeListener( pWrappedProperty->getInnerName(), aListener );
        else
            xInnerPropertySet->addVetoableChangeListener( rPropertyName, aListener );
    }
}
void SAL_CALL WrappedPropertySet::removeVetoableChangeListener( const OUString& rPropertyName, const Reference< beans::XVetoableChangeListener >& aListener )
{
    Reference< beans::XPropertySet > xInnerPropertySet( getInnerPropertySet() );
    if( xInnerPropertySet.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
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
    if( rNameSeq.getLength() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            try
            {
                OUString aPropertyName( rNameSeq[nN] );
                aRetSeq[nN] = getPropertyValue( aPropertyName );
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
    beans::PropertyState aState( beans::PropertyState_DIRECT_VALUE );

    Reference< beans::XPropertyState > xInnerPropertyState( getInnerPropertyState() );
    if( xInnerPropertyState.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
        if( pWrappedProperty )
            aState = pWrappedProperty->getPropertyState( xInnerPropertyState );
        else
            aState = xInnerPropertyState->getPropertyState( rPropertyName );
    }
    return aState;
}

const WrappedProperty* WrappedPropertySet::getWrappedProperty( const OUString& rOuterName )
{
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rOuterName );
    return getWrappedProperty( nHandle );
}

const WrappedProperty* WrappedPropertySet::getWrappedProperty( sal_Int32 nHandle )
{
    tWrappedPropertyMap::const_iterator aFound( getWrappedPropertyMap().find( nHandle ) );
    if( aFound != getWrappedPropertyMap().end() )
        return (*aFound).second.get();
    return nullptr;
}

Sequence< beans::PropertyState > SAL_CALL WrappedPropertySet::getPropertyStates( const Sequence< OUString >& rNameSeq )
{
    Sequence< beans::PropertyState > aRetSeq;
    if( rNameSeq.getLength() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            aRetSeq[nN] = getPropertyState( aPropertyName );
        }
    }
    return aRetSeq;
}

void SAL_CALL WrappedPropertySet::setPropertyToDefault( const OUString& rPropertyName )
{
    Reference< beans::XPropertyState > xInnerPropertyState( getInnerPropertyState() );
    if( xInnerPropertyState.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
        if( pWrappedProperty )
            pWrappedProperty->setPropertyToDefault( xInnerPropertyState );
        else
            xInnerPropertyState->setPropertyToDefault( rPropertyName );
    }
}
Any SAL_CALL WrappedPropertySet::getPropertyDefault( const OUString& rPropertyName )
{
    Any aRet;
    Reference< beans::XPropertyState > xInnerPropertyState( getInnerPropertyState() );
    if( xInnerPropertyState.is() )
    {
        const WrappedProperty* pWrappedProperty = getWrappedProperty( rPropertyName );
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
    for(sal_Int32 nN=0; nN<rPropSeq.getLength(); nN++)
    {
        OUString aPropertyName( rPropSeq[nN].Name );
        setPropertyToDefault( aPropertyName );
    }
}
void SAL_CALL WrappedPropertySet::setPropertiesToDefault( const Sequence< OUString >& rNameSeq )
{
    for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
    {
        OUString aPropertyName( rNameSeq[nN] );
        setPropertyToDefault( aPropertyName );
    }
}
Sequence< Any > SAL_CALL WrappedPropertySet::getPropertyDefaults( const Sequence< OUString >& rNameSeq )
{
    Sequence< Any > aRetSeq;
    if( rNameSeq.getLength() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            aRetSeq[nN] = getPropertyDefault( aPropertyName );
        }
    }
    return aRetSeq;
}

::cppu::IPropertyArrayHelper& WrappedPropertySet::getInfoHelper()
{
    ::cppu::OPropertyArrayHelper* p = m_pPropertyArrayHelper.get();
    if(!p)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );//do not use different mutex than is already used for static property sequence
        p = m_pPropertyArrayHelper.get();
        if(!p)
        {
            p = new ::cppu::OPropertyArrayHelper( getPropertySequence(), true );
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            m_pPropertyArrayHelper.reset(p);
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return *m_pPropertyArrayHelper;
}

tWrappedPropertyMap& WrappedPropertySet::getWrappedPropertyMap()
{
    tWrappedPropertyMap* p = m_pWrappedPropertyMap.get();
    if(!p)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );//do not use different mutex than is already used for static property sequence
        p = m_pWrappedPropertyMap.get();
        if(!p)
        {
            std::vector< std::unique_ptr<WrappedProperty> > aPropList( createWrappedProperties() );
            p = new tWrappedPropertyMap;

            for (auto & elem : aPropList)
            {
                sal_Int32 nHandle = getInfoHelper().getHandleByName( elem->getOuterName() );

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

            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            m_pWrappedPropertyMap.reset(p);
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return *m_pWrappedPropertyMap;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
