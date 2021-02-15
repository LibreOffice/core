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

#include <ContentHelper.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/diagnose_ex.h>
#include <apitools.hxx>
#include <sdbcoretools.hxx>
#include <stringconstants.hxx>

#include <map>

namespace dbaccess
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::container;
using namespace ::cppu;

OContentHelper_Impl::OContentHelper_Impl()
    : m_pDataSource(nullptr)
{
}

OContentHelper_Impl::~OContentHelper_Impl()
{
}

OContentHelper::OContentHelper(const Reference< XComponentContext >& _xORB
                               ,const Reference< XInterface >&  _xParentContainer
                               ,const TContentPtr& _pImpl)
    : OContentHelper_COMPBASE(m_aMutex)
    ,m_aContentListeners(m_aMutex)
    ,m_aPropertyChangeListeners(m_aMutex)
    ,m_xParentContainer( _xParentContainer )
    ,m_aContext( _xORB )
    ,m_pImpl(_pImpl)
    ,m_nCommandId(0)
{
}

void SAL_CALL OContentHelper::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // say goodbye to our listeners
    EventObject aEvt(*this);
    m_aContentListeners.disposeAndClear(aEvt);

    m_xParentContainer = nullptr;
}

IMPLEMENT_SERVICE_INFO1(OContentHelper,"com.sun.star.comp.sdb.Content","com.sun.star.ucb.Content");

css::uno::Sequence<sal_Int8> OContentHelper::getUnoTunnelId()
{
    static cppu::OImplementationId aId;
    return aId.getImplementationId();
}

css::uno::Sequence<sal_Int8> OContentHelper::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XContent
Reference< XContentIdentifier > SAL_CALL OContentHelper::getIdentifier(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OUString aIdentifier( "private:" + impl_getHierarchicalName( true ) );
    return new ::ucbhelper::ContentIdentifier( aIdentifier );
}

OUString OContentHelper::impl_getHierarchicalName( bool _includingRootContainer ) const
{
    OUStringBuffer aHierarchicalName( m_pImpl->m_aProps.aTitle );
    Reference< XInterface > xParent = m_xParentContainer;
    while( xParent.is() )
    {
        Reference<XPropertySet> xProp( xParent, UNO_QUERY );
        Reference< XChild > xChild( xParent, UNO_QUERY );
        xParent.set( xChild.is() ? xChild->getParent() : Reference< XInterface >(), UNO_QUERY );
        if ( xProp.is() && xParent.is() )
        {
            OUString sName;
            xProp->getPropertyValue( PROPERTY_NAME ) >>= sName;

            OUString sPrevious = aHierarchicalName.makeStringAndClear();
            aHierarchicalName.append( sName ).append( "/" ).append( sPrevious );
        }
    }
    OUString sHierarchicalName( aHierarchicalName.makeStringAndClear() );
    if ( !_includingRootContainer )
        sHierarchicalName = sHierarchicalName.copy( sHierarchicalName.indexOf( '/' ) + 1 );
    return sHierarchicalName;
}

OUString SAL_CALL OContentHelper::getContentType()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( !m_pImpl->m_aProps.aContentType )
    {   // content type not yet retrieved
        m_pImpl->m_aProps.aContentType = determineContentType();
    }

    return *m_pImpl->m_aProps.aContentType;
}

void SAL_CALL OContentHelper::addContentEventListener( const Reference< XContentEventListener >& _rxListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( _rxListener.is() )
        m_aContentListeners.addInterface(_rxListener);
}

void SAL_CALL OContentHelper::removeContentEventListener( const Reference< XContentEventListener >& _rxListener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (_rxListener.is())
        m_aContentListeners.removeInterface(_rxListener);
}

// XCommandProcessor
sal_Int32 SAL_CALL OContentHelper::createCommandIdentifier(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    // Just increase counter on every call to generate an identifier.
    return ++m_nCommandId;
}

Any SAL_CALL OContentHelper::execute( const Command& aCommand, sal_Int32 /*CommandId*/, const Reference< XCommandEnvironment >& Environment )
{
    Any aRet;
    if ( aCommand.Name == "getPropertyValues" )
    {
        // getPropertyValues

        Sequence< Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        aRet <<= getPropertyValues( Properties);
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        // setPropertyValues

        Sequence< PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.hasElements() )
        {
            OSL_FAIL( "No properties!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
    {
        // getPropertySetInfo

        Reference<XPropertySet> xProp(*this,UNO_QUERY);
        if ( xProp.is() )
            aRet <<= xProp->getPropertySetInfo();
        //  aRet <<= getPropertySetInfo(); // TODO
    }
    else
    {
        // Unsupported command

        OSL_FAIL( "Content::execute - unsupported command!" );

        ucbhelper::cancelCommandExecution(
            makeAny( UnsupportedCommandException(
                            OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}

void SAL_CALL OContentHelper::abort( sal_Int32 /*CommandId*/ )
{
}

// XPropertiesChangeNotifier
void SAL_CALL OContentHelper::addPropertiesChangeListener( const Sequence< OUString >& PropertyNames, const Reference< XPropertiesChangeListener >& Listener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_aPropertyChangeListeners.addInterface(OUString(), Listener );
    }
    else
    {
        const OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const OUString& rName = pSeq[ n ];
            if ( !rName.isEmpty() )
                m_aPropertyChangeListeners.addInterface(rName, Listener );
        }
    }
}

void SAL_CALL OContentHelper::removePropertiesChangeListener( const Sequence< OUString >& PropertyNames, const Reference< XPropertiesChangeListener >& Listener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    sal_Int32 nCount = PropertyNames.getLength();
    if ( !nCount )
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_aPropertyChangeListeners.removeInterface( OUString(), Listener );
    }
    else
    {
        const OUString* pSeq = PropertyNames.getConstArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const OUString& rName = pSeq[ n ];
            if ( !rName.isEmpty() )
                m_aPropertyChangeListeners.removeInterface( rName, Listener );
        }
    }
}

// XPropertyContainer
void SAL_CALL OContentHelper::addProperty( const OUString& /*Name*/, sal_Int16 /*Attributes*/, const Any& /*DefaultValue*/ )
{
    OSL_FAIL( "OContentHelper::addProperty: not implemented!" );
}

void SAL_CALL OContentHelper::removeProperty( const OUString& /*Name*/ )
{
    OSL_FAIL( "OContentHelper::removeProperty: not implemented!" );
}

// XInitialization
void SAL_CALL OContentHelper::initialize( const Sequence< Any >& _aArguments )
{
    const Any* pBegin = _aArguments.getConstArray();
    const Any* pEnd = pBegin + _aArguments.getLength();
    PropertyValue aValue;
    for(;pBegin != pEnd;++pBegin)
    {
        *pBegin >>= aValue;
        if ( aValue.Name == "Parent" )
        {
            m_xParentContainer.set(aValue.Value,UNO_QUERY);
        }
        else if ( aValue.Name == PROPERTY_NAME )
        {
            aValue.Value >>= m_pImpl->m_aProps.aTitle;
        }
        else if ( aValue.Name == PROPERTY_PERSISTENT_NAME )
        {
            aValue.Value >>= m_pImpl->m_aProps.sPersistentName;
        }
    }
}

Sequence< Any > OContentHelper::setPropertyValues(const Sequence< PropertyValue >& rValues )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Sequence< Any > aRet( rValues.getLength() );
    Sequence< PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = false;
    aEvent.PropertyHandle = -1;

    const PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name == "ContentType" || rValue.Name == "IsDocument" || rValue.Name == "IsFolder" )
        {
            // Read-only property!
            aRet[ n ] <<= IllegalAccessException("Property is read-only!",
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name == "Title" )
        {
            OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                if ( aNewValue != m_pImpl->m_aProps.aTitle )
                {
                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue     <<= m_pImpl->m_aProps.aTitle;

                    try
                    {
                        impl_rename_throw( aNewValue ,false);
                        OSL_ENSURE( m_pImpl->m_aProps.aTitle == aNewValue, "OContentHelper::setPropertyValues('Title'): rename did not work!" );

                        aEvent.NewValue     <<= aNewValue;
                        aChanges.getArray()[ nChanged ] = aEvent;
                        nChanged++;
                    }
                    catch( const Exception& )
                    {
                        TOOLS_WARN_EXCEPTION( "dbaccess", "OContentHelper::setPropertyValues('Title'): caught an exception while renaming!" );
                    }
                }
                else
                {
                    // Old value equals new value. No error!
                }
            }
            else
            {
                aRet[ n ] <<= IllegalTypeException("Property value has wrong type!",
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }

        else
        {
            aRet[ n ] <<= Exception("No property set for storing the value!",
                            static_cast< cppu::OWeakObject * >( this ) );
        }
    }

    if ( nChanged > 0 )
    {
        notifyDataSourceModified();
        aGuard.clear();
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}

// static
Reference< XRow > OContentHelper::getPropertyValues( const Sequence< Property >& rProperties)
{
    // Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( m_aContext );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        const Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const Property& rProp = pProps[ n ];

            // Process Core properties.

            if ( rProp.Name == "ContentType" )
            {
                xRow->appendString ( rProp, getContentType() );
            }
            else if ( rProp.Name == "Title" )
            {
                xRow->appendString ( rProp, m_pImpl->m_aProps.aTitle );
            }
            else if ( rProp.Name == "IsDocument" )
            {
                xRow->appendBoolean( rProp, m_pImpl->m_aProps.bIsDocument );
            }
            else if ( rProp.Name == "IsFolder" )
            {
                xRow->appendBoolean( rProp, m_pImpl->m_aProps.bIsFolder );
            }
            else
                xRow->appendVoid(rProp);
        }
    }
    else
    {
        // Append all Core Properties.
        xRow->appendString (
            Property( "ContentType", -1,
                      cppu::UnoType<OUString>::get(),
                      PropertyAttribute::BOUND
                        | PropertyAttribute::READONLY ),
            getContentType() );
        xRow->appendString (
            Property( "Title", -1,
                      cppu::UnoType<OUString>::get(),
                      PropertyAttribute::BOUND ),
            m_pImpl->m_aProps.aTitle );
        xRow->appendBoolean(
            Property( "IsDocument", -1,
                      cppu::UnoType<bool>::get(),
                      PropertyAttribute::BOUND
                        | PropertyAttribute::READONLY ),
            m_pImpl->m_aProps.bIsDocument );
        xRow->appendBoolean(
            Property( "IsFolder", -1,
                      cppu::UnoType<bool>::get(),
                      PropertyAttribute::BOUND
                        | PropertyAttribute::READONLY ),
            m_pImpl->m_aProps.bIsFolder );

        // @@@ Append other properties supported directly.
    }

    return xRow;
}

void OContentHelper::notifyPropertiesChange( const Sequence< PropertyChangeEvent >& evt ) const
{

    sal_Int32 nCount = evt.getLength();
    if ( !nCount )
        return;

    // First, notify listeners interested in changes of every property.
    OInterfaceContainerHelper* pAllPropsContainer = m_aPropertyChangeListeners.getContainer( OUString() );
    if ( pAllPropsContainer )
    {
        OInterfaceIteratorHelper aIter( *pAllPropsContainer );
        while ( aIter.hasMoreElements() )
        {
            // Propagate event.
            Reference< XPropertiesChangeListener > xListener( aIter.next(), UNO_QUERY );
            if ( xListener.is() )
                xListener->propertiesChange( evt );
        }
    }

    typedef std::map< XPropertiesChangeListener*, Sequence< PropertyChangeEvent > > PropertiesEventListenerMap;
    PropertiesEventListenerMap aListeners;

    const PropertyChangeEvent* propertyChangeEvent = evt.getConstArray();

    for ( sal_Int32 n = 0; n < nCount; ++n, ++propertyChangeEvent )
    {
        const PropertyChangeEvent& rEvent = *propertyChangeEvent;
        const OUString& rName = rEvent.PropertyName;

        OInterfaceContainerHelper* pPropsContainer = m_aPropertyChangeListeners.getContainer( rName );
        if ( pPropsContainer )
        {
            OInterfaceIteratorHelper aIter( *pPropsContainer );
            while ( aIter.hasMoreElements() )
            {
                Sequence< PropertyChangeEvent >* propertyEvents;

                XPropertiesChangeListener* pListener = static_cast< XPropertiesChangeListener * >( aIter.next() );
                PropertiesEventListenerMap::iterator it = aListeners.find( pListener );
                if ( it == aListeners.end() )
                {
                    // Not in map - create and insert new entry.
                    auto pair = aListeners.emplace( pListener, Sequence< PropertyChangeEvent >( nCount ));
                    propertyEvents = &pair.first->second;
                }
                else
                    propertyEvents = &(*it).second;

                (*propertyEvents)[n] = rEvent;
            }
        }
    }

    // Notify listeners.
    for (auto & rPair : aListeners)
    {
        XPropertiesChangeListener* pListener = rPair.first;
        Sequence< PropertyChangeEvent >& rSeq = rPair.second;

        // Propagate event.
        pListener->propertiesChange( rSeq );
    }
}

// css::lang::XUnoTunnel
sal_Int64 OContentHelper::getSomething( const Sequence< sal_Int8 > & rId )
{
    if (isUnoTunnelId<OContentHelper>(rId))
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}

Reference< XInterface > SAL_CALL OContentHelper::getParent(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xParentContainer;
}

void SAL_CALL OContentHelper::setParent( const Reference< XInterface >& _xParent )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xParentContainer = _xParent;
}

void OContentHelper::impl_rename_throw(const OUString& _sNewName,bool _bNotify )
{
    osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
    if ( _sNewName == m_pImpl->m_aProps.aTitle )
        return;
    try
    {
        Sequence< PropertyChangeEvent > aChanges( 1 );

        aChanges[0].Source          = static_cast< cppu::OWeakObject * >( this );
        aChanges[0].Further         = false;
        aChanges[0].PropertyName    = PROPERTY_NAME;
        aChanges[0].PropertyHandle  = PROPERTY_ID_NAME;
        aChanges[0].OldValue        <<= m_pImpl->m_aProps.aTitle;
        aChanges[0].NewValue        <<= _sNewName;

        aGuard.clear();

        m_pImpl->m_aProps.aTitle = _sNewName;
        if ( _bNotify )
            notifyPropertiesChange( aChanges );
        notifyDataSourceModified();
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(_sNewName,*this);
    }
}

void SAL_CALL OContentHelper::rename( const OUString& newName )
{

    impl_rename_throw(newName);

}

void OContentHelper::notifyDataSourceModified()
{
    ::dbaccess::notifyDataSourceModified(m_xParentContainer);
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
