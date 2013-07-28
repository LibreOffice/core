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

#include "WrappedPropertySet.hxx"
#include "macros.hxx"

// header for define DELETEZ
#include <tools/solar.h>

namespace chart
{

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

WrappedPropertySet::WrappedPropertySet()
                    : MutexContainer()
                    , m_xInfo(0)
                    , m_pPropertyArrayHelper(0)
                    , m_pWrappedPropertyMap(0)
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

    //delete all wrapped properties
    if(m_pWrappedPropertyMap)
    {
        for( tWrappedPropertyMap::iterator aIt = m_pWrappedPropertyMap->begin()
            ; aIt!= m_pWrappedPropertyMap->end(); ++aIt )
        {
            const WrappedProperty* pWrappedProperty = (*aIt).second;
            DELETEZ(pWrappedProperty);
        }
    }

    DELETEZ(m_pPropertyArrayHelper);
    DELETEZ(m_pWrappedPropertyMap);

    m_xInfo = NULL;
}

//XPropertySet
Reference< beans::XPropertySetInfo > SAL_CALL WrappedPropertySet::getPropertySetInfo(  )
                                    throw (uno::RuntimeException)
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
                                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
        const WrappedProperty* pWrappedProperty = getWrappedProperty( nHandle );
        Reference< beans::XPropertySet > xInnerPropertySet( this->getInnerPropertySet() );
        if( pWrappedProperty )
            pWrappedProperty->setPropertyValue( rValue, xInnerPropertySet );
        else if( xInnerPropertySet.is() )
            xInnerPropertySet->setPropertyValue( rPropertyName, rValue );
        else
        {
#if OSL_DEBUG_LEVEL > 1
            OSL_FAIL("found no inner property set to map to");
#endif
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
        OSL_FAIL("invalid exception caught in WrappedPropertySet::setPropertyValue");
        lang::WrappedTargetException aWrappedException;
        aWrappedException.TargetException = uno::makeAny( ex );
        throw aWrappedException;
    }
}
Any SAL_CALL WrappedPropertySet::getPropertyValue( const OUString& rPropertyName )
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;

    try
    {
        sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
        const WrappedProperty* pWrappedProperty = getWrappedProperty( nHandle );
        Reference< beans::XPropertySet > xInnerPropertySet( this->getInnerPropertySet() );
        if( pWrappedProperty )
            aRet = pWrappedProperty->getPropertyValue( xInnerPropertySet );
        else if( xInnerPropertySet.is() )
            aRet = xInnerPropertySet->getPropertyValue( rPropertyName );
        else
        {
#if OSL_DEBUG_LEVEL > 1
            OSL_FAIL("found no inner property set to map to");
#endif
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
        OSL_FAIL("invalid exception caught in WrappedPropertySet::setPropertyValue");
        lang::WrappedTargetException aWrappedException;
        aWrappedException.TargetException = uno::makeAny( ex );
        throw aWrappedException;
    }

    return aRet;
}

void SAL_CALL WrappedPropertySet::addPropertyChangeListener( const OUString& rPropertyName, const Reference< beans::XPropertyChangeListener >& xListener )
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< beans::XPropertySet > xInnerPropertySet( this->getInnerPropertySet() );
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
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< beans::XPropertySet > xInnerPropertySet( this->getInnerPropertySet() );
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
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< beans::XPropertySet > xInnerPropertySet( this->getInnerPropertySet() );
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
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< beans::XPropertySet > xInnerPropertySet( this->getInnerPropertySet() );
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
                                    throw (beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    bool bUnknownProperty = false;
    sal_Int32 nMinCount = std::min( rValueSeq.getLength(), rNameSeq.getLength() );
    for(sal_Int32 nN=0; nN<nMinCount; nN++)
    {
        OUString aPropertyName( rNameSeq[nN] );
        try
        {
            this->setPropertyValue( aPropertyName, rValueSeq[nN] );
        }
        catch( const beans::UnknownPropertyException& ex )
        {
            ASSERT_EXCEPTION( ex );
            bUnknownProperty = true;
        }
    }
    //todo: store unknown properties elsewhere
    OSL_ENSURE(!bUnknownProperty,"unknown property");
    (void)bUnknownProperty;
//    if( bUnknownProperty )
//        throw beans::UnknownPropertyException();
}
Sequence< Any > SAL_CALL WrappedPropertySet::getPropertyValues( const Sequence< OUString >& rNameSeq )
                                    throw (uno::RuntimeException)
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
                aRetSeq[nN] = this->getPropertyValue( aPropertyName );
            }
            catch( const beans::UnknownPropertyException& ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            catch( const lang::WrappedTargetException& ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
    return aRetSeq;
}
void SAL_CALL WrappedPropertySet::addPropertiesChangeListener( const Sequence< OUString >& /* rNameSeq */, const Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented yet");
    //todo
}
void SAL_CALL WrappedPropertySet::removePropertiesChangeListener( const Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented yet");
    //todo
}
void SAL_CALL WrappedPropertySet::firePropertiesChangeEvent( const Sequence< OUString >& /* rNameSeq */, const Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented yet");
    //todo
}

//XPropertyState
beans::PropertyState SAL_CALL WrappedPropertySet::getPropertyState( const OUString& rPropertyName )
                                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    beans::PropertyState aState( beans::PropertyState_DIRECT_VALUE );

    Reference< beans::XPropertyState > xInnerPropertyState( this->getInnerPropertyState() );
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
        return (*aFound).second;
    return 0;
}

Sequence< beans::PropertyState > SAL_CALL WrappedPropertySet::getPropertyStates( const Sequence< OUString >& rNameSeq )
                                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    Sequence< beans::PropertyState > aRetSeq;
    if( rNameSeq.getLength() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            aRetSeq[nN] = this->getPropertyState( aPropertyName );
        }
    }
    return aRetSeq;
}

void SAL_CALL WrappedPropertySet::setPropertyToDefault( const OUString& rPropertyName )
                                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    Reference< beans::XPropertyState > xInnerPropertyState( this->getInnerPropertyState() );
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
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    Reference< beans::XPropertyState > xInnerPropertyState( this->getInnerPropertyState() );
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
                                    throw (uno::RuntimeException)
{
    const Sequence< beans::Property >&  rPropSeq = getPropertySequence();
    for(sal_Int32 nN=0; nN<rPropSeq.getLength(); nN++)
    {
        OUString aPropertyName( rPropSeq[nN].Name );
        this->setPropertyToDefault( aPropertyName );
    }
}
void SAL_CALL WrappedPropertySet::setPropertiesToDefault( const Sequence< OUString >& rNameSeq )
                                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
    {
        OUString aPropertyName( rNameSeq[nN] );
        this->setPropertyToDefault( aPropertyName );
    }
}
Sequence< Any > SAL_CALL WrappedPropertySet::getPropertyDefaults( const Sequence< OUString >& rNameSeq )
                                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Sequence< Any > aRetSeq;
    if( rNameSeq.getLength() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            aRetSeq[nN] = this->getPropertyDefault( aPropertyName );
        }
    }
    return aRetSeq;
}

::cppu::IPropertyArrayHelper& WrappedPropertySet::getInfoHelper()
{
    ::cppu::OPropertyArrayHelper* p = m_pPropertyArrayHelper;
    if(!p)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );//do not use different mutex than is already used for static property sequence
        p = m_pPropertyArrayHelper;
        if(!p)
        {
            p = new ::cppu::OPropertyArrayHelper( getPropertySequence(), sal_True );
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            m_pPropertyArrayHelper = p;
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
    tWrappedPropertyMap* p = m_pWrappedPropertyMap;
    if(!p)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );//do not use different mutex than is already used for static property sequence
        p = m_pWrappedPropertyMap;
        if(!p)
        {
            std::vector< WrappedProperty* > aPropList( createWrappedProperties() );
            p = new tWrappedPropertyMap();

            for( std::vector< WrappedProperty* >::const_iterator aIt = aPropList.begin(); aIt!=aPropList.end(); ++aIt )
            {
                WrappedProperty* pProperty = *aIt;
                if(pProperty)
                {
                    sal_Int32 nHandle = getInfoHelper().getHandleByName( pProperty->getOuterName() );

                    if( nHandle == -1 )
                    {
                        OSL_FAIL( "missing property in property list" );
                        delete pProperty;//we are owner or the created WrappedProperties
                    }
                    else if( p->find( nHandle ) != p->end() )
                    {
                        //duplicate Wrapped property
                        OSL_FAIL( "duplicate Wrapped property" );
                        delete pProperty;//we are owner or the created WrappedProperties
                    }
                    else
                        (*p)[ nHandle ] = pProperty;
                }
            }

            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            m_pWrappedPropertyMap = p;
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
