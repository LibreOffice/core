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
#include <comphelper/servicehelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sdbcoretools.hxx>
#include <stringconstants.hxx>
#include <strings.hxx>

#include <map>
#include <utility>

namespace dbaccess
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::io;
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
                               ,TContentPtr _pImpl)
    : OContentHelper_COMPBASE(m_aMutex)
    ,m_aContentListeners(m_aMutex)
    ,m_aPropertyChangeListeners(m_aMutex)
    ,m_xParentContainer( _xParentContainer )
    ,m_aContext( _xORB )
    ,m_pImpl(std::move(_pImpl))
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

OUString SAL_CALL OContentHelper::getImplementationName()
    {
        return u"com.sun.star.comp.sdb.Content"_ustr;
    }
sal_Bool SAL_CALL OContentHelper::supportsService(const OUString& _rServiceName)
    {
        const css::uno::Sequence< OUString > aSupported(getSupportedServiceNames());
        for (const OUString& s : aSupported)
            if (s == _rServiceName)
                return true;

        return false;
    }
css::uno::Sequence< OUString > SAL_CALL OContentHelper::getSupportedServiceNames()
{
    return { u"com.sun.star.ucb.Content"_ustr };
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
            aHierarchicalName.append( sName + "/" + sPrevious );
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
                Any( IllegalArgumentException(
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
                Any( IllegalArgumentException(
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
                Any( IllegalArgumentException(
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
            Any( UnsupportedCommandException(
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
    if (!PropertyNames.hasElements())
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_aPropertyChangeListeners.addInterface(OUString(), Listener );
    }
    else
    {
        for (auto& rName : PropertyNames)
            if ( !rName.isEmpty() )
                m_aPropertyChangeListeners.addInterface(rName, Listener );
    }
}

void SAL_CALL OContentHelper::removePropertiesChangeListener( const Sequence< OUString >& PropertyNames, const Reference< XPropertiesChangeListener >& Listener )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (!PropertyNames.hasElements())
    {
        // Note: An empty sequence means a listener for "all" properties.
        m_aPropertyChangeListeners.removeInterface( OUString(), Listener );
    }
    else
    {
        for (auto& rName : PropertyNames)
            if ( !rName.isEmpty() )
                m_aPropertyChangeListeners.removeInterface( rName, Listener );
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
    for (auto& arg : _aArguments)
    {
        PropertyValue aValue;
        arg >>= aValue;
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
    auto aRetRange = asNonConstRange(aRet);
    Sequence< PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = false;
    aEvent.PropertyHandle = -1;

    for (sal_Int32 n = 0; n < rValues.getLength(); ++n)
    {
        const PropertyValue& rValue = rValues[n];

        if ( rValue.Name == "ContentType" || rValue.Name == "IsDocument" || rValue.Name == "IsFolder" )
        {
            // Read-only property!
            aRetRange[ n ] <<= IllegalAccessException(u"Property is read-only!"_ustr,
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
                aRetRange[ n ] <<= IllegalTypeException(u"Property value has wrong type!"_ustr,
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }

        else
        {
            aRetRange[ n ] <<= Exception(u"No property set for storing the value!"_ustr,
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

    if (rProperties.hasElements())
    {
        for (auto& rProp : rProperties)
        {
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
            Property( u"ContentType"_ustr, -1,
                      cppu::UnoType<OUString>::get(),
                      PropertyAttribute::BOUND
                        | PropertyAttribute::READONLY ),
            getContentType() );
        xRow->appendString (
            Property( u"Title"_ustr, -1,
                      cppu::UnoType<OUString>::get(),
                      PropertyAttribute::BOUND ),
            m_pImpl->m_aProps.aTitle );
        xRow->appendBoolean(
            Property( u"IsDocument"_ustr, -1,
                      cppu::UnoType<bool>::get(),
                      PropertyAttribute::BOUND
                        | PropertyAttribute::READONLY ),
            m_pImpl->m_aProps.bIsDocument );
        xRow->appendBoolean(
            Property( u"IsFolder"_ustr, -1,
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
    comphelper::OInterfaceContainerHelper3<XPropertiesChangeListener>* pAllPropsContainer = m_aPropertyChangeListeners.getContainer( OUString() );
    if ( pAllPropsContainer )
        pAllPropsContainer->notifyEach( &XPropertiesChangeListener::propertiesChange, evt );

    typedef std::map< XPropertiesChangeListener*, Sequence< PropertyChangeEvent > > PropertiesEventListenerMap;
    PropertiesEventListenerMap aListeners;

    for (sal_Int32 n = 0; n < nCount; ++n)
    {
        const PropertyChangeEvent& rEvent = evt[n];
        const OUString& rName = rEvent.PropertyName;

        comphelper::OInterfaceContainerHelper3<XPropertiesChangeListener>* pPropsContainer = m_aPropertyChangeListeners.getContainer( rName );
        if ( pPropsContainer )
        {
            comphelper::OInterfaceIteratorHelper3 aIter( *pPropsContainer );
            while ( aIter.hasMoreElements() )
            {
                Sequence< PropertyChangeEvent >* propertyEvents;

                XPropertiesChangeListener* pListener = aIter.next().get();
                PropertiesEventListenerMap::iterator it = aListeners.find( pListener );
                if ( it == aListeners.end() )
                {
                    // Not in map - create and insert new entry.
                    auto pair = aListeners.emplace( pListener, Sequence< PropertyChangeEvent >( nCount ));
                    propertyEvents = &pair.first->second;
                }
                else
                    propertyEvents = &(*it).second;

                asNonConstRange(*propertyEvents)[n] = rEvent;
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
        Sequence<PropertyChangeEvent> aChanges{
            { /* Source         */ static_cast<cppu::OWeakObject*>(this),
              /* PropertyName   */ PROPERTY_NAME,
              /* Further        */ false,
              /* PropertyHandle */ PROPERTY_ID_NAME,
              /* OldValue       */ Any(m_pImpl->m_aProps.aTitle),
              /* NewValue       */ Any(_sNewName) }
        };

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
