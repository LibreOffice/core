/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"
/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <boost/unordered_map.hpp>
#include <com/sun/star/ucb/ContentAction.hpp>
#include <com/sun/star/ucb/CommandInfoChange.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <cppuhelper/interfacecontainer.hxx>

#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <ucbhelper/contentinfo.hxx>

using namespace com::sun::star;

namespace ucbhelper_impl
{

//=========================================================================
//
// class PropertyEventSequence.
//
//=========================================================================

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

//=========================================================================
//
// PropertiesEventListenerMap.
//
//=========================================================================

typedef void* XPropertiesChangeListenerPtr; // -> Compiler problems!

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

//=========================================================================
//
// PropertyChangeListenerContainer.
//
//=========================================================================

struct equalStr
{
    bool operator()( const rtl::OUString& s1, const rtl::OUString& s2 ) const
      {
        return !!( s1 == s2 );
    }
};

struct hashStr
{
    size_t operator()( const rtl::OUString& rName ) const
    {
        return rName.hashCode();
    }
};

typedef cppu::OMultiTypeInterfaceContainerHelperVar
<
    rtl::OUString,
    hashStr,
    equalStr
> PropertyChangeListeners;

//=========================================================================
//
// struct ContentImplHelper_Impl
//
//=========================================================================

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

} // namespace ucbhelper_impl

using namespace ucbhelper_impl;

//=========================================================================
//=========================================================================
//
// ContentImplHelper Implementation.
//
//=========================================================================
//=========================================================================

namespace ucbhelper {

ContentImplHelper::ContentImplHelper(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            const rtl::Reference< ContentProviderImplHelper >& rxProvider,
            const uno::Reference<
            com::sun::star::ucb::XContentIdentifier >& Identifier )
: m_pImpl( new ContentImplHelper_Impl ),
  m_xSMgr( rxSMgr ),
  m_xIdentifier( Identifier ),
  m_xProvider( rxProvider ),
  m_nCommandId( 0 )
{
}

//=========================================================================
// virtual
ContentImplHelper::~ContentImplHelper()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

void SAL_CALL ContentImplHelper::acquire()
    throw()
{
    cppu::OWeakObject::acquire();
}

void SAL_CALL ContentImplHelper::release()
    throw()
{
    // #144882# - Call to OWeakObject::release may destroy m_xProvider.
    //            Prevent this.
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

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

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

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
sal_Bool SAL_CALL ContentImplHelper::supportsService(
                                            const rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSNL = getSupportedServiceNames();
    const rtl::OUString* pArray = aSNL.getConstArray();
    for ( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if ( pArray[ i ] == ServiceName )
            return sal_True;
    }

    return sal_False;
}

//=========================================================================
//
// XComponent methods.
//
//=========================================================================

// virtual
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

//=========================================================================
// virtual
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

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners->removeInterface( Listener );
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
ContentImplHelper::getIdentifier()
    throw( uno::RuntimeException )
{
    return m_xIdentifier;
}

//=========================================================================
// virtual
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

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removeContentEventListener(
        const uno::Reference< com::sun::star::ucb::XContentEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pContentEventListeners )
        m_pImpl->m_pContentEventListeners->removeInterface( Listener );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
sal_Int32 SAL_CALL ContentImplHelper::createCommandIdentifier()
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    // Just increase counter on every call to generate an identifier.
    return ++m_nCommandId;
}

//=========================================================================
//
// XPropertiesChangeNotifier methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentImplHelper::addPropertiesChangeListener(
        const uno::Sequence< rtl::OUString >& PropertyNames,
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
        // Note: An empty sequence means a listener for "all" properties.
        m_pImpl->m_pPropertyChangeListeners->addInterface(
            rtl::OUString(), Listener );
    }
    else
    {
        const rtl::OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const rtl::OUString& rName = pSeq[ n ];
            if ( rName.getLength() )
                m_pImpl->m_pPropertyChangeListeners->addInterface(
                    rName, Listener );
        }
    }
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removePropertiesChangeListener(
        const uno::Sequence< rtl::OUString >& PropertyNames,
        const uno::Reference< beans::XPropertiesChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_pImpl->m_pPropertyChangeListeners->removeInterface(
            rtl::OUString(), Listener );
    }
    else
    {
        const rtl::OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const rtl::OUString& rName = pSeq[ n ];
            if ( rName.getLength() )
                m_pImpl->m_pPropertyChangeListeners->removeInterface(
                    rName, Listener );
        }
    }
}

//=========================================================================
//
// XCommandInfoChangeNotifier methods.
//
//=========================================================================

// virtual
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

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removeCommandInfoChangeListener(
        const uno::Reference< com::sun::star::ucb::XCommandInfoChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pCommandChangeListeners )
        m_pImpl->m_pCommandChangeListeners->removeInterface( Listener );
}

//=========================================================================
//
// XPropertyContainer methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentImplHelper::addProperty(
        const rtl::OUString& Name,
        sal_Int16 Attributes,
        const uno::Any& DefaultValue )
    throw( beans::PropertyExistException,
           beans::IllegalTypeException,
           lang::IllegalArgumentException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    //////////////////////////////////////////////////////////////////////
    // Make sure a property with the requested name does not already
    // exist in dynamic and static(!) properties.
    //////////////////////////////////////////////////////////////////////

    // @@@ Need real command environment here, but where to get it from?
    //     XPropertyContainer interface should be replaced by
    //     XCommandProcessor commands!
    uno::Reference< com::sun::star::ucb::XCommandEnvironment > xEnv;

    if ( getPropertySetInfo( xEnv )->hasPropertyByName( Name ) )
    {
        // Property does already exist.
        throw beans::PropertyExistException();
    }

    //////////////////////////////////////////////////////////////////////
    // Add a new dynamic property.
    //////////////////////////////////////////////////////////////////////

    // Open/create persistent property set.
    uno::Reference< com::sun::star::ucb::XPersistentPropertySet > xSet(
                                    getAdditionalPropertySet( sal_True ) );

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
            // Property is always removeable.
            Attributes |= beans::PropertyAttribute::REMOVEABLE;

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

            // Success!

            if ( m_pImpl->m_xPropSetInfo.is() )
            {
                // Info cached in propertyset info is invalid now!
                m_pImpl->m_xPropSetInfo->reset();
            }

            // Notify propertyset info change listeners.
            if ( m_pImpl->m_pPropSetChangeListeners &&
                 m_pImpl->m_pPropSetChangeListeners->getLength() )
            {
                beans::PropertySetInfoChangeEvent evt(
                            static_cast< cppu::OWeakObject * >( this ),
                            Name,
                            -1, // No handle available
                            beans::PropertySetInfoChange::PROPERTY_INSERTED );
                notifyPropertySetInfoChange( evt );
            }
        }
    }
}

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removeProperty( const rtl::OUString& Name )
    throw( beans::UnknownPropertyException,
           beans::NotRemoveableException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    try
    {
        // @@@ Need real command environment here, but where to get it from?
        //     XPropertyContainer interface should be replaced by
        //     XCommandProcessor commands!
        uno::Reference< com::sun::star::ucb::XCommandEnvironment > xEnv;

        beans::Property aProp
            = getPropertySetInfo( xEnv )->getPropertyByName( Name );

        if ( !( aProp.Attributes & beans::PropertyAttribute::REMOVEABLE ) )
        {
            // Not removeable!
            throw beans::NotRemoveableException();
        }
    }
    catch ( beans::UnknownPropertyException const & )
    {
        OSL_FAIL( "ContentImplHelper::removeProperty - Unknown!" );
        throw;
    }

    //////////////////////////////////////////////////////////////////////
    // Try to remove property from dynamic property set.
    //////////////////////////////////////////////////////////////////////

    // Open persistent property set, if exists.
    uno::Reference< com::sun::star::ucb::XPersistentPropertySet > xSet(
        getAdditionalPropertySet( sal_False ) );
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
                    "ContentImplHelper::removeProperty - Unremoveable!" );
                throw;
            }

            xContainer = 0;

            // Success!

            if ( xSet->getPropertySetInfo()->getProperties().getLength() == 0 )
            {
                // Remove empty propertyset from registry.
                uno::Reference< com::sun::star::ucb::XPropertySetRegistry >
                    xReg = xSet->getRegistry();
                if ( xReg.is() )
                {
                    rtl::OUString aKey( xSet->getKey() );
                    xSet = 0;
                    xReg->removePropertySet( aKey );
                }
            }

            if ( m_pImpl->m_xPropSetInfo.is() )
            {
                // Info cached in propertyset info is invalid now!
                m_pImpl->m_xPropSetInfo->reset();
            }

            // Notify propertyset info change listeners.
            if ( m_pImpl->m_pPropSetChangeListeners &&
                 m_pImpl->m_pPropSetChangeListeners->getLength() )
            {
                beans::PropertySetInfoChangeEvent evt(
                            static_cast< cppu::OWeakObject * >( this ),
                            Name,
                            -1, // No handle available
                            beans::PropertySetInfoChange::PROPERTY_REMOVED );
                notifyPropertySetInfoChange( evt );
            }
        }
    }
}

//=========================================================================
//
// XPropertySetInfoChangeNotifier methods.
//
//=========================================================================

// virtual
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

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::removePropertySetInfoChangeListener(
        const uno::Reference< beans::XPropertySetInfoChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners->removeInterface( Listener );
}

//=========================================================================
//
// XChild methods.
//
//=========================================================================

// virtual
uno::Reference< uno::XInterface > SAL_CALL ContentImplHelper::getParent()
    throw( uno::RuntimeException )
{
    uno::Reference< uno::XInterface > xParent;
    rtl::OUString aURL = getParentURL();

    if ( aURL.getLength() )
    {
        uno::Reference< com::sun::star::ucb::XContentIdentifier > xId(
            new ContentIdentifier( m_xSMgr, aURL ) );
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

//=========================================================================
// virtual
void SAL_CALL ContentImplHelper::setParent(
                                    const uno::Reference< uno::XInterface >& )
    throw( lang::NoSupportException, uno::RuntimeException )
{
    throw lang::NoSupportException();
}

//=========================================================================
//
// Non-interface methods
//
//=========================================================================

uno::Reference< com::sun::star::ucb::XPersistentPropertySet >
ContentImplHelper::getAdditionalPropertySet( sal_Bool bCreate )
{
    // Get propertyset from provider.
    return m_xProvider->getAdditionalPropertySet(
                            m_xIdentifier->getContentIdentifier(), bCreate );
}

//=========================================================================
sal_Bool ContentImplHelper::renameAdditionalPropertySet(
    const rtl::OUString& rOldKey,
    const rtl::OUString& rNewKey,
    sal_Bool bRecursive )
{
    return m_xProvider->renameAdditionalPropertySet(
                                            rOldKey, rNewKey, bRecursive );
}

//=========================================================================
sal_Bool ContentImplHelper::copyAdditionalPropertySet(
    const rtl::OUString& rSourceKey,
    const rtl::OUString& rTargetKey,
    sal_Bool bRecursive )
{
    return m_xProvider->copyAdditionalPropertySet(
                                        rSourceKey, rTargetKey, bRecursive );
}

//=========================================================================
sal_Bool ContentImplHelper::removeAdditionalPropertySet( sal_Bool bRecursive )
{
    return m_xProvider->removeAdditionalPropertySet(
                    m_xIdentifier->getContentIdentifier(), bRecursive );
}

//=========================================================================
void ContentImplHelper::notifyPropertiesChange(
    const uno::Sequence< beans::PropertyChangeEvent >& evt ) const
{
    if ( !m_pImpl->m_pPropertyChangeListeners )
        return;

    sal_Int32 nCount = evt.getLength();
    if ( nCount )
    {
        // First, notify listeners interested in changes of every property.
        cppu::OInterfaceContainerHelper* pAllPropsContainer
            = m_pImpl->m_pPropertyChangeListeners->getContainer(
                rtl::OUString() );
        if ( pAllPropsContainer )
        {
            cppu::OInterfaceIteratorHelper aIter( *pAllPropsContainer );
            while ( aIter.hasMoreElements() )
            {
                // Propagate event.
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
            const rtl::OUString& rName = rEvent.PropertyName;

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
                        // Not in map - create and insert new entry.
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

        // Notify listeners.
        PropertiesEventListenerMap::iterator it = aListeners.begin();
        while ( !aListeners.empty() )
        {
            beans::XPropertiesChangeListener* pListener =
                static_cast< beans::XPropertiesChangeListener * >( (*it).first );
            PropertyEventSequence* pSeq = (*it).second;

            // Remove current element.
            aListeners.erase( it );

            // Propagate event.
            pListener->propertiesChange( pSeq->getEvents() );

            delete pSeq;

            it = aListeners.begin();
        }
    }
}

//=========================================================================
void ContentImplHelper::notifyPropertySetInfoChange(
    const beans::PropertySetInfoChangeEvent& evt ) const
{
    if ( !m_pImpl->m_pPropSetChangeListeners )
        return;

    // Notify event listeners.
    cppu::OInterfaceIteratorHelper aIter( *m_pImpl->m_pPropSetChangeListeners );
    while ( aIter.hasMoreElements() )
    {
        // Propagate event.
        uno::Reference< beans::XPropertySetInfoChangeListener >
            xListener( aIter.next(), uno::UNO_QUERY );
        if ( xListener.is() )
            xListener->propertySetInfoChange( evt );
    }
}

//=========================================================================
void ContentImplHelper::notifyCommandInfoChange(
    const com::sun::star::ucb::CommandInfoChangeEvent& evt ) const
{
    if ( !m_pImpl->m_pCommandChangeListeners )
        return;

    // Notify event listeners.
    cppu::OInterfaceIteratorHelper aIter(
        *m_pImpl->m_pCommandChangeListeners );
    while ( aIter.hasMoreElements() )
    {
        // Propagate event.
        uno::Reference< com::sun::star::ucb::XCommandInfoChangeListener >
            xListener( aIter.next(), uno::UNO_QUERY );
        if ( xListener.is() )
            xListener->commandInfoChange( evt );
    }
}

//=========================================================================
void ContentImplHelper::notifyContentEvent(
    const com::sun::star::ucb::ContentEvent& evt ) const
{
    if ( !m_pImpl->m_pContentEventListeners )
        return;

    // Notify event listeners.
    cppu::OInterfaceIteratorHelper aIter( *m_pImpl->m_pContentEventListeners );
    while ( aIter.hasMoreElements() )
    {
        // Propagate event.
        uno::Reference<
            com::sun::star::ucb::XContentEventListener > xListener(
                aIter.next(), uno::UNO_QUERY );
        if ( xListener.is() )
            xListener->contentEvent( evt );
    }
}

//=========================================================================
void ContentImplHelper::inserted()
{
    // Content is not yet registered at provider.
    m_xProvider->registerNewContent( this );

    // If the parent content is currently not instanciated, there can be
    // no listeners interested in changes ;-)

    rtl::Reference< ContentImplHelper > xParent
                = m_xProvider->queryExistingContent( getParentURL() );

    if ( xParent.is() )
    {
        com::sun::star::ucb::ContentEvent aEvt(
            static_cast< cppu::OWeakObject * >( xParent.get() ), // Source
            com::sun::star::ucb::ContentAction::INSERTED,        // Action
            this,                                                // Content
            xParent->getIdentifier() );                          // Id
        xParent->notifyContentEvent( aEvt );
    }
}

//=========================================================================
void ContentImplHelper::deleted()
{
    uno::Reference< com::sun::star::ucb::XContent > xThis = this;

    rtl::Reference< ContentImplHelper > xParent
                    = m_xProvider->queryExistingContent( getParentURL() );

    if ( xParent.is() )
    {
        // Let parent notify "REMOVED" event.
        com::sun::star::ucb::ContentEvent aEvt(
            static_cast< cppu::OWeakObject * >( xParent.get() ),
            com::sun::star::ucb::ContentAction::REMOVED,
            this,
            xParent->getIdentifier()    );
        xParent->notifyContentEvent( aEvt );
    }

    // Notify "DELETED" event.
    com::sun::star::ucb::ContentEvent aEvt1(
        static_cast< cppu::OWeakObject * >( this ),
        com::sun::star::ucb::ContentAction::DELETED,
        this,
        getIdentifier() );
    notifyContentEvent( aEvt1 );

    m_xProvider->removeContent( this );
}

//=========================================================================
sal_Bool ContentImplHelper::exchange(
    const uno::Reference< com::sun::star::ucb::XContentIdentifier >& rNewId )
{
    uno::Reference< com::sun::star::ucb::XContent > xThis = this;

    osl::ClearableMutexGuard aGuard( m_aMutex );

    rtl::Reference< ContentImplHelper > xContent
        = m_xProvider->queryExistingContent( rNewId );
    if ( xContent.is() )
    {
        // @@@
        // Big trouble. Another object with the new identity exists.
        // How shall I mutate to / merge with the other object?
        return sal_False;
    }

    uno::Reference< com::sun::star::ucb::XContentIdentifier > xOldId
        = getIdentifier();

    // Re-insert at provider.
    m_xProvider->removeContent( this );
    m_xIdentifier = rNewId;
    m_xProvider->registerNewContent( this );

    aGuard.clear();

    // Notify "EXCHANGED" event.
    com::sun::star::ucb::ContentEvent aEvt(
        static_cast< cppu::OWeakObject * >( this ),
        com::sun::star::ucb::ContentAction::EXCHANGED,
        this,
        xOldId );
    notifyContentEvent( aEvt );
    return sal_True;
}

//=========================================================================
uno::Reference< com::sun::star::ucb::XCommandInfo >
ContentImplHelper::getCommandInfo(
    const uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv,
    sal_Bool bCache )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_xCommandsInfo.is() )
        m_pImpl->m_xCommandsInfo
            = new CommandProcessorInfo( m_xSMgr, xEnv, this );
    else if ( !bCache )
        m_pImpl->m_xCommandsInfo->reset();

    return uno::Reference< com::sun::star::ucb::XCommandInfo >(
        m_pImpl->m_xCommandsInfo.get() );
}

//=========================================================================
uno::Reference< beans::XPropertySetInfo >
ContentImplHelper::getPropertySetInfo(
    const uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv,
    sal_Bool bCache )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pImpl->m_xPropSetInfo.is() )
        m_pImpl->m_xPropSetInfo
            = new PropertySetInfo( m_xSMgr, xEnv, this );
    else if ( !bCache )
        m_pImpl->m_xPropSetInfo->reset();

    return uno::Reference< beans::XPropertySetInfo >(
                                    m_pImpl->m_xPropSetInfo.get() );
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
