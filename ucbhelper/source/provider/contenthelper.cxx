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

#include <boost/unordered_map.hpp>
#include <com/sun/star/ucb/ContentAction.hpp>
#include <com/sun/star/ucb/CommandInfoChange.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contentinfo.hxx>
#include <ucbhelper/providerhelper.hxx>

#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"

using namespace com::sun::star;

namespace ucbhelper_impl
{

class PropertyEventSequence
{
    uno::Sequence< beans::PropertyChangeEvent > m_aSeq;
    sal_uInt32                                  m_nPos;

public:
    PropertyEventSequence( sal_uInt32 nSize )
    : m_aSeq( nSize ), m_nPos( 0 ) {};

    void append( const beans::PropertyChangeEvent& rEvt )
    { m_aSeq.getArray()[ m_nPos ] = rEvt; ++m_nPos; }

    const uno::Sequence< beans::PropertyChangeEvent >& getEvents()
    { m_aSeq.realloc( m_nPos ); return m_aSeq; }
};

typedef void* XPropertiesChangeListenerPtr; 

struct equalPtr
{
    bool operator()( const XPropertiesChangeListenerPtr& rp1,
                     const XPropertiesChangeListenerPtr& rp2 ) const
    {
        return ( rp1 == rp2 );
    }
};

struct hashPtr
{
    size_t operator()( const XPropertiesChangeListenerPtr& rp ) const
    {
        return (size_t)rp;
    }
};

typedef boost::unordered_map
<
    XPropertiesChangeListenerPtr,
    PropertyEventSequence*,
    hashPtr,
    equalPtr
>
PropertiesEventListenerMap;

typedef cppu::OMultiTypeInterfaceContainerHelperVar
<
    OUString,
    OUStringHash
> PropertyChangeListeners;

struct ContentImplHelper_Impl
{
    rtl::Reference< ::ucbhelper::PropertySetInfo >      m_xPropSetInfo;
    rtl::Reference< ::ucbhelper::CommandProcessorInfo > m_xCommandsInfo;
    cppu::OInterfaceContainerHelper*              m_pDisposeEventListeners;
    cppu::OInterfaceContainerHelper*              m_pContentEventListeners;
    cppu::OInterfaceContainerHelper*              m_pPropSetChangeListeners;
    cppu::OInterfaceContainerHelper*              m_pCommandChangeListeners;
    PropertyChangeListeners*                      m_pPropertyChangeListeners;

    ContentImplHelper_Impl()
    : m_pDisposeEventListeners( 0 ),
        m_pContentEventListeners( 0 ),
      m_pPropSetChangeListeners( 0 ),
        m_pCommandChangeListeners( 0 ),
      m_pPropertyChangeListeners( 0 ) {}

    ~ContentImplHelper_Impl()
    {
        delete m_pDisposeEventListeners;
        delete m_pContentEventListeners;
        delete m_pPropSetChangeListeners;
        delete m_pCommandChangeListeners;
        delete m_pPropertyChangeListeners;
    }
};

} 

using namespace ucbhelper_impl;

namespace ucbhelper {

ContentImplHelper::ContentImplHelper(
            const uno::Reference< uno::XComponentContext >& rxContext,
            const rtl::Reference< ContentProviderImplHelper >& rxProvider,
            const uno::Reference<
            com::sun::star::ucb::XContentIdentifier >& Identifier )
: m_pImpl( new ContentImplHelper_Impl ),
  m_xContext( rxContext ),
  m_xIdentifier( Identifier ),
  m_xProvider( rxProvider ),
  m_nCommandId( 0 )
{
}


ContentImplHelper::~ContentImplHelper()
{
    delete m_pImpl;
}

void SAL_CALL ContentImplHelper::acquire()
    throw()
{
    cppu::OWeakObject::acquire();
}

void SAL_CALL ContentImplHelper::release()
    throw()
{
    
    
    rtl::Reference< ContentProviderImplHelper > xKeepProviderAlive(
        m_xProvider );

    {
        osl::MutexGuard aGuard( m_xProvider->m_aMutex );
        OWeakObject::release();
    }
}

uno::Any SAL_CALL ContentImplHelper::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    com::sun::star::uno::Any aRet = cppu::queryInterface( rType,
            static_cast< lang::XTypeProvider * >(this),
            static_cast< lang::XServiceInfo * >(this),
            static_cast< lang::XComponent * >(this),
            static_cast< com::sun::star::ucb::XContent * >(this),
            static_cast< com::sun::star::ucb::XCommandProcessor * >(this),
            static_cast< beans::XPropertiesChangeNotifier * >(this),
            static_cast< com::sun::star::ucb::XCommandInfoChangeNotifier * >(this),
            static_cast< beans::XPropertyContainer * >(this),
            static_cast< beans::XPropertySetInfoChangeNotifier * >(this),
            static_cast< container::XChild * >(this));
    return aRet.hasValue() ? aRet : cppu::OWeakObject::queryInterface( rType );
}

XTYPEPROVIDER_IMPL_10( ContentImplHelper,
                       lang::XTypeProvider,
                       lang::XServiceInfo,
                       lang::XComponent,
                       com::sun::star::ucb::XContent,
                       com::sun::star::ucb::XCommandProcessor,
                       beans::XPropertiesChangeNotifier,
                       com::sun::star::ucb::XCommandInfoChangeNotifier,
                       beans::XPropertyContainer,
                       beans::XPropertySetInfoChangeNotifier,
                       container::XChild );


sal_Bool SAL_CALL ContentImplHelper::supportsService(
                                            const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}


void SAL_CALL ContentImplHelper::dispose()
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners &&
         m_pImpl->m_pDisposeEventListeners->getLength() )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< lang::XComponent * >( this );
        m_pImpl->m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pContentEventListeners &&
         m_pImpl->m_pContentEventListeners->getLength() )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< com::sun::star::ucb::XContent * >( this );
        m_pImpl->m_pContentEventListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pPropSetChangeListeners &&
         m_pImpl->m_pPropSetChangeListeners->getLength() )
    {
        lang::EventObject aEvt;
        aEvt.Source
            = static_cast< beans::XPropertySetInfoChangeNotifier * >( this );
        m_pImpl->m_pPropSetChangeListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pCommandChangeListeners &&
         m_pImpl->m_pCommandChangeListeners->getLength() )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast<  com::sun::star::ucb::XCommandInfoChangeNotifier * >( this );
        m_pImpl->m_pCommandChangeListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pPropertyChangeListeners )
    {
        lang::EventObject aEvt;
        aEvt.Source
            = static_cast< beans::XPropertiesChangeNotifier * >( this );
        m_pImpl->m_pPropertyChangeListeners->disposeAndClear( aEvt );
    }
}


void SAL_CALL ContentImplHelper::addEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners
            = new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL ContentImplHelper::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners->removeInterface( Listener );
}


uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
ContentImplHelper::getIdentifier()
    throw( uno::RuntimeException )
{
    return m_xIdentifier;
}


void SAL_CALL ContentImplHelper::addContentEventListener(
        const uno::Reference< com::sun::star::ucb::XContentEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_pContentEventListeners )
        m_pImpl->m_pContentEventListeners
            = new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pContentEventListeners->addInterface( Listener );
}


void SAL_CALL ContentImplHelper::removeContentEventListener(
        const uno::Reference< com::sun::star::ucb::XContentEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pContentEventListeners )
        m_pImpl->m_pContentEventListeners->removeInterface( Listener );
}


sal_Int32 SAL_CALL ContentImplHelper::createCommandIdentifier()
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    
    return ++m_nCommandId;
}


void SAL_CALL ContentImplHelper::addPropertiesChangeListener(
        const uno::Sequence< OUString >& PropertyNames,
        const uno::Reference< beans::XPropertiesChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners
            = new PropertyChangeListeners( m_aMutex );

    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        
        m_pImpl->m_pPropertyChangeListeners->addInterface(
            OUString(), Listener );
    }
    else
    {
        const OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const OUString& rName = pSeq[ n ];
            if ( !rName.isEmpty() )
                m_pImpl->m_pPropertyChangeListeners->addInterface(
                    rName, Listener );
        }
    }
}


void SAL_CALL ContentImplHelper::removePropertiesChangeListener(
        const uno::Sequence< OUString >& PropertyNames,
        const uno::Reference< beans::XPropertiesChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        
        m_pImpl->m_pPropertyChangeListeners->removeInterface(
            OUString(), Listener );
    }
    else
    {
        const OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const OUString& rName = pSeq[ n ];
            if ( !rName.isEmpty() )
                m_pImpl->m_pPropertyChangeListeners->removeInterface(
                    rName, Listener );
        }
    }
}


void SAL_CALL ContentImplHelper::addCommandInfoChangeListener(
        const uno::Reference< com::sun::star::ucb::XCommandInfoChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_pCommandChangeListeners )
        m_pImpl->m_pCommandChangeListeners
            = new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pCommandChangeListeners->addInterface( Listener );
}


void SAL_CALL ContentImplHelper::removeCommandInfoChangeListener(
        const uno::Reference< com::sun::star::ucb::XCommandInfoChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pCommandChangeListeners )
        m_pImpl->m_pCommandChangeListeners->removeInterface( Listener );
}


void SAL_CALL ContentImplHelper::addProperty(
        const OUString& Name,
        sal_Int16 Attributes,
        const uno::Any& DefaultValue )
    throw( beans::PropertyExistException,
           beans::IllegalTypeException,
           lang::IllegalArgumentException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    
    

    
    
    
    uno::Reference< com::sun::star::ucb::XCommandEnvironment > xEnv;

    if ( getPropertySetInfo( xEnv )->hasPropertyByName( Name ) )
    {
        
        throw beans::PropertyExistException();
    }

    
    
    uno::Reference< com::sun::star::ucb::XPersistentPropertySet > xSet(
                                    getAdditionalPropertySet( true ) );

    OSL_ENSURE( xSet.is(),
                "ContentImplHelper::addProperty - No property set!" );

    if ( xSet.is() )
    {
        uno::Reference< beans::XPropertyContainer > xContainer(
            xSet, uno::UNO_QUERY );

        OSL_ENSURE(
            xContainer.is(),
            "ContentImplHelper::addProperty - No property container!" );

        if ( xContainer.is() )
        {
            
            Attributes |= beans::PropertyAttribute::REMOVABLE;

            try
            {
                xContainer->addProperty( Name, Attributes, DefaultValue );
            }
            catch ( beans::PropertyExistException const & )
            {
                OSL_FAIL( "ContentImplHelper::addProperty - Exists!" );
                throw;
            }
            catch ( beans::IllegalTypeException const & )
            {
                OSL_FAIL( "ContentImplHelper::addProperty - Wrong Type!" );
                throw;
            }
            catch ( lang::IllegalArgumentException const & )
            {
                OSL_FAIL( "ContentImplHelper::addProperty - Illegal Arg!" );
                throw;
            }

            

            if ( m_pImpl->m_xPropSetInfo.is() )
            {
                
                m_pImpl->m_xPropSetInfo->reset();
            }

            
            if ( m_pImpl->m_pPropSetChangeListeners &&
                 m_pImpl->m_pPropSetChangeListeners->getLength() )
            {
                beans::PropertySetInfoChangeEvent evt(
                            static_cast< cppu::OWeakObject * >( this ),
                            Name,
                            -1, 
                            beans::PropertySetInfoChange::PROPERTY_INSERTED );
                notifyPropertySetInfoChange( evt );
            }
        }
    }
}


void SAL_CALL ContentImplHelper::removeProperty( const OUString& Name )
    throw( beans::UnknownPropertyException,
           beans::NotRemoveableException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    try
    {
        
        
        
        uno::Reference< com::sun::star::ucb::XCommandEnvironment > xEnv;

        beans::Property aProp
            = getPropertySetInfo( xEnv )->getPropertyByName( Name );

        if ( !( aProp.Attributes & beans::PropertyAttribute::REMOVABLE ) )
        {
            
            throw beans::NotRemoveableException();
        }
    }
    catch ( beans::UnknownPropertyException const & )
    {
        OSL_FAIL( "ContentImplHelper::removeProperty - Unknown!" );
        throw;
    }

    
    
    uno::Reference< com::sun::star::ucb::XPersistentPropertySet > xSet(
        getAdditionalPropertySet( false ) );
    if ( xSet.is() )
    {
        uno::Reference< beans::XPropertyContainer > xContainer(
            xSet, uno::UNO_QUERY );

        OSL_ENSURE(
            xContainer.is(),
            "ContentImplHelper::removeProperty - No property container!" );

        if ( xContainer.is() )
        {
            try
            {
                xContainer->removeProperty( Name );
            }
            catch ( beans::UnknownPropertyException const & )
            {
                OSL_FAIL( "ContentImplHelper::removeProperty - Unknown!" );
                throw;
            }
            catch ( beans::NotRemoveableException const & )
            {
                OSL_FAIL(
                    "ContentImplHelper::removeProperty - Unremovable!" );
                throw;
            }

            xContainer = 0;

            

            if ( xSet->getPropertySetInfo()->getProperties().getLength() == 0 )
            {
                
                uno::Reference< com::sun::star::ucb::XPropertySetRegistry >
                    xReg = xSet->getRegistry();
                if ( xReg.is() )
                {
                    OUString aKey( xSet->getKey() );
                    xSet = 0;
                    xReg->removePropertySet( aKey );
                }
            }

            if ( m_pImpl->m_xPropSetInfo.is() )
            {
                
                m_pImpl->m_xPropSetInfo->reset();
            }

            
            if ( m_pImpl->m_pPropSetChangeListeners &&
                 m_pImpl->m_pPropSetChangeListeners->getLength() )
            {
                beans::PropertySetInfoChangeEvent evt(
                            static_cast< cppu::OWeakObject * >( this ),
                            Name,
                            -1, 
                            beans::PropertySetInfoChange::PROPERTY_REMOVED );
                notifyPropertySetInfoChange( evt );
            }
        }
    }
}


void SAL_CALL ContentImplHelper::addPropertySetInfoChangeListener(
        const uno::Reference< beans::XPropertySetInfoChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners
            = new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pImpl->m_pPropSetChangeListeners->addInterface( Listener );
}


void SAL_CALL ContentImplHelper::removePropertySetInfoChangeListener(
        const uno::Reference< beans::XPropertySetInfoChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners->removeInterface( Listener );
}


uno::Reference< uno::XInterface > SAL_CALL ContentImplHelper::getParent()
    throw( uno::RuntimeException )
{
    uno::Reference< uno::XInterface > xParent;
    OUString aURL = getParentURL();

    if ( !aURL.isEmpty() )
    {
        uno::Reference< com::sun::star::ucb::XContentIdentifier > xId(
            new ContentIdentifier( aURL ) );
        try
        {
            xParent.set( m_xProvider->queryContent( xId ) );
        }
        catch ( com::sun::star::ucb::IllegalIdentifierException const & )
        {
        }
    }

    return xParent;
}


void SAL_CALL ContentImplHelper::setParent(
                                    const uno::Reference< uno::XInterface >& )
    throw( lang::NoSupportException, uno::RuntimeException )
{
    throw lang::NoSupportException();
}

uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
ContentImplHelper::getAdditionalPropertySet( bool bCreate )
{
    
    return m_xProvider->getAdditionalPropertySet(
                            m_xIdentifier->getContentIdentifier(), bCreate );
}

bool ContentImplHelper::renameAdditionalPropertySet(
    const OUString& rOldKey,
    const OUString& rNewKey,
    bool bRecursive )
{
    return m_xProvider->renameAdditionalPropertySet(
                                            rOldKey, rNewKey, bRecursive );
}

bool ContentImplHelper::copyAdditionalPropertySet(
    const OUString& rSourceKey,
    const OUString& rTargetKey,
    bool bRecursive )
{
    return m_xProvider->copyAdditionalPropertySet(
                                        rSourceKey, rTargetKey, bRecursive );
}

bool ContentImplHelper::removeAdditionalPropertySet( bool bRecursive )
{
    return m_xProvider->removeAdditionalPropertySet(
                    m_xIdentifier->getContentIdentifier(), bRecursive );
}

void ContentImplHelper::notifyPropertiesChange(
    const uno::Sequence< beans::PropertyChangeEvent >& evt ) const
{
    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    sal_Int32 nCount = evt.getLength();
    if ( nCount )
    {
        
        cppu::OInterfaceContainerHelper* pAllPropsContainer
            = m_pImpl->m_pPropertyChangeListeners->getContainer(
                OUString() );
        if ( pAllPropsContainer )
        {
            cppu::OInterfaceIteratorHelper aIter( *pAllPropsContainer );
            while ( aIter.hasMoreElements() )
            {
                
                uno::Reference< beans::XPropertiesChangeListener > xListener(
                    aIter.next(), uno::UNO_QUERY );
                if ( xListener.is() )
                    xListener->propertiesChange( evt );
            }
        }

        PropertiesEventListenerMap aListeners;

        const beans::PropertyChangeEvent* pEvents = evt.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::PropertyChangeEvent& rEvent = pEvents[ n ];
            const OUString& rName = rEvent.PropertyName;

            cppu::OInterfaceContainerHelper* pPropsContainer
                = m_pImpl->m_pPropertyChangeListeners->getContainer( rName );
            if ( pPropsContainer )
            {
                cppu::OInterfaceIteratorHelper aIter( *pPropsContainer );
                while ( aIter.hasMoreElements() )
                {
                    PropertyEventSequence* p = NULL;

                    beans::XPropertiesChangeListener* pListener =
                        static_cast< beans::XPropertiesChangeListener * >(
                                                            aIter.next() );
                    PropertiesEventListenerMap::iterator it =
                            aListeners.find( pListener );
                    if ( it == aListeners.end() )
                    {
                        
                        p = new PropertyEventSequence( nCount );
                        aListeners[ pListener ] = p;
                    }
                    else
                        p = (*it).second;

                    if ( p )
                        p->append( rEvent );
                }
            }
        }

        
        PropertiesEventListenerMap::iterator it = aListeners.begin();
        while ( !aListeners.empty() )
        {
            beans::XPropertiesChangeListener* pListener =
                static_cast< beans::XPropertiesChangeListener * >( (*it).first );
            PropertyEventSequence* pSeq = (*it).second;

            
            aListeners.erase( it );

            
            pListener->propertiesChange( pSeq->getEvents() );

            delete pSeq;

            it = aListeners.begin();
        }
    }
}

void ContentImplHelper::notifyPropertySetInfoChange(
    const beans::PropertySetInfoChangeEvent& evt ) const
{
    if ( !m_pImpl->m_pPropSetChangeListeners )
        return;

    
    cppu::OInterfaceIteratorHelper aIter( *m_pImpl->m_pPropSetChangeListeners );
    while ( aIter.hasMoreElements() )
    {
        
        uno::Reference< beans::XPropertySetInfoChangeListener >
            xListener( aIter.next(), uno::UNO_QUERY );
        if ( xListener.is() )
            xListener->propertySetInfoChange( evt );
    }
}

void ContentImplHelper::notifyContentEvent(
    const com::sun::star::ucb::ContentEvent& evt ) const
{
    if ( !m_pImpl->m_pContentEventListeners )
        return;

    
    cppu::OInterfaceIteratorHelper aIter( *m_pImpl->m_pContentEventListeners );
    while ( aIter.hasMoreElements() )
    {
        
        uno::Reference<
            com::sun::star::ucb::XContentEventListener > xListener(
                aIter.next(), uno::UNO_QUERY );
        if ( xListener.is() )
            xListener->contentEvent( evt );
    }
}

void ContentImplHelper::inserted()
{
    
    m_xProvider->registerNewContent( this );

    
    

    rtl::Reference< ContentImplHelper > xParent
                = m_xProvider->queryExistingContent( getParentURL() );

    if ( xParent.is() )
    {
        com::sun::star::ucb::ContentEvent aEvt(
            static_cast< cppu::OWeakObject * >( xParent.get() ), 
            com::sun::star::ucb::ContentAction::INSERTED,        
            this,                                                
            xParent->getIdentifier() );                          
        xParent->notifyContentEvent( aEvt );
    }
}

void ContentImplHelper::deleted()
{
    uno::Reference< com::sun::star::ucb::XContent > xThis = this;

    rtl::Reference< ContentImplHelper > xParent
                    = m_xProvider->queryExistingContent( getParentURL() );

    if ( xParent.is() )
    {
        
        com::sun::star::ucb::ContentEvent aEvt(
            static_cast< cppu::OWeakObject * >( xParent.get() ),
            com::sun::star::ucb::ContentAction::REMOVED,
            this,
            xParent->getIdentifier()    );
        xParent->notifyContentEvent( aEvt );
    }

    
    com::sun::star::ucb::ContentEvent aEvt1(
        static_cast< cppu::OWeakObject * >( this ),
        com::sun::star::ucb::ContentAction::DELETED,
        this,
        getIdentifier() );
    notifyContentEvent( aEvt1 );

    m_xProvider->removeContent( this );
}

bool ContentImplHelper::exchange(
    const uno::Reference< com::sun::star::ucb::XContentIdentifier >& rNewId )
{
    uno::Reference< com::sun::star::ucb::XContent > xThis = this;

    osl::ClearableMutexGuard aGuard( m_aMutex );

    rtl::Reference< ContentImplHelper > xContent
        = m_xProvider->queryExistingContent( rNewId );
    if ( xContent.is() )
    {
        
        
        
        return false;
    }

    uno::Reference< com::sun::star::ucb::XContentIdentifier > xOldId
        = getIdentifier();

    
    m_xProvider->removeContent( this );
    m_xIdentifier = rNewId;
    m_xProvider->registerNewContent( this );

    aGuard.clear();

    
    com::sun::star::ucb::ContentEvent aEvt(
        static_cast< cppu::OWeakObject * >( this ),
        com::sun::star::ucb::ContentAction::EXCHANGED,
        this,
        xOldId );
    notifyContentEvent( aEvt );
    return true;
}

uno::Reference< com::sun::star::ucb::XCommandInfo >
ContentImplHelper::getCommandInfo(
    const uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv,
    bool bCache )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_xCommandsInfo.is() )
        m_pImpl->m_xCommandsInfo
            = new CommandProcessorInfo( xEnv, this );
    else if ( !bCache )
        m_pImpl->m_xCommandsInfo->reset();

    return uno::Reference< com::sun::star::ucb::XCommandInfo >(
        m_pImpl->m_xCommandsInfo.get() );
}

uno::Reference< beans::XPropertySetInfo >
ContentImplHelper::getPropertySetInfo(
    const uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv,
    bool bCache )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_xPropSetInfo.is() )
        m_pImpl->m_xPropSetInfo
            = new PropertySetInfo( xEnv, this );
    else if ( !bCache )
        m_pImpl->m_xPropSetInfo->reset();

    return uno::Reference< beans::XPropertySetInfo >(
                                    m_pImpl->m_xPropSetInfo.get() );
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
