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


/**************************************************************************
                                TODO
 **************************************************************************

  *************************************************************************/

#include <list>
#include <unordered_map>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/NotRemoveableException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include "ucbstore.hxx"

using namespace com::sun::star::beans;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace comphelper;
using namespace cppu;


OUString makeHierarchalNameSegment( const OUString & rIn  )
{
    OUStringBuffer aBuffer;
    aBuffer.append( "['" );

    sal_Int32 nCount = rIn.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const sal_Unicode c = rIn[ n ];
        switch ( c )
        {
            case '&':
                aBuffer.append( "&amp;" );
                break;

            case '"':
                aBuffer.append( "&quot;" );
                break;

            case '\'':
                aBuffer.append( "&apos;" );
                break;

            case '<':
                aBuffer.append( "&lt;" );
                break;

            case '>':
                aBuffer.append( "&gt;" );
                break;

            default:
                aBuffer.append( c );
                break;
        }
    }

    aBuffer.append( "']" );
    return OUString( aBuffer.makeStringAndClear() );
}

#define STORE_CONTENTPROPERTIES_KEY "/org.openoffice.ucb.Store/ContentProperties"

// describe path of cfg entry
#define CFGPROPERTY_NODEPATH        "nodepath"
// true->async. update; false->sync. update
#define CFGPROPERTY_LAZYWRITE       "lazywrite"

// PropertySetMap_Impl.
typedef std::unordered_map
<
    OUString,
    PersistentPropertySet*,
    OUStringHash
>
PropertySetMap_Impl;

// class PropertySetInfo_Impl
class PropertySetInfo_Impl : public cppu::WeakImplHelper < XPropertySetInfo >
{
    std::unique_ptr<Sequence< Property >>
                                      m_pProps;
    PersistentPropertySet*            m_pOwner;

public:
    explicit PropertySetInfo_Impl(PersistentPropertySet* pOwner);

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties() override;
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;

    // Non-interface methods.
    void reset() { m_pProps.reset(); }
};


// UcbStore_Impl.


struct UcbStore_Impl
{
    osl::Mutex                        m_aMutex;
    Sequence< Any >                   m_aInitArgs;
    Reference< XPropertySetRegistry > m_xTheRegistry;
};


// UcbStore Implementation.


UcbStore::UcbStore( const Reference< XComponentContext >& xContext )
: m_xContext( xContext ),
  m_pImpl( new UcbStore_Impl() )
{
}


// virtual
UcbStore::~UcbStore()
{
}

XSERVICEINFO_COMMOM_IMPL( UcbStore,
                          OUString( "com.sun.star.comp.ucb.UcbStore" ) )
/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface > SAL_CALL
UcbStore_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX =
        static_cast<css::lang::XServiceInfo*>(new UcbStore( ucbhelper::getComponentContext(rSMgr) ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}

css::uno::Sequence< OUString >
UcbStore::getSupportedServiceNames_Static()
{
    css::uno::Sequence< OUString > aSNS { STORE_SERVICE_NAME };
    return aSNS;
}

// Service factory implementation.


ONE_INSTANCE_SERVICE_FACTORY_IMPL( UcbStore );


// XPropertySetRegistryFactory methods.


// virtual
Reference< XPropertySetRegistry > SAL_CALL
UcbStore::createPropertySetRegistry( const OUString& )
{
    // The URL parameter is ignored by this interface implementation. It always
    // uses the configuration server as storage medium.

    if ( !m_pImpl->m_xTheRegistry.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
        if ( !m_pImpl->m_xTheRegistry.is() )
            m_pImpl->m_xTheRegistry = new PropertySetRegistry( m_xContext, m_pImpl->m_aInitArgs );
    }

    return m_pImpl->m_xTheRegistry;
}


// XInitialization methods.


// virtual
void SAL_CALL UcbStore::initialize( const Sequence< Any >& aArguments )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
    m_pImpl->m_aInitArgs = aArguments;
}



// PropertySetRegistry_Impl.


struct PropertySetRegistry_Impl
{
    const Sequence< Any >             m_aInitArgs;
    PropertySetMap_Impl               m_aPropSets;
    Reference< XMultiServiceFactory > m_xConfigProvider;
    Reference< XInterface >           m_xRootReadAccess;
    Reference< XInterface >           m_xRootWriteAccess;
    osl::Mutex                        m_aMutex;
    bool                              m_bTriedToGetRootReadAccess;
    bool                              m_bTriedToGetRootWriteAccess;

    explicit PropertySetRegistry_Impl(const Sequence<Any> &rInitArgs)
        : m_aInitArgs(rInitArgs)
        , m_bTriedToGetRootReadAccess(false)
        , m_bTriedToGetRootWriteAccess(false)
    {
    }
};

// PropertySetRegistry Implementation.


PropertySetRegistry::PropertySetRegistry(
                        const Reference< XComponentContext >& xContext,
                        const Sequence< Any > &rInitArgs )
: m_xContext( xContext ),
  m_pImpl( new PropertySetRegistry_Impl( rInitArgs ) )
{
}


// virtual
PropertySetRegistry::~PropertySetRegistry()
{
}


// XServiceInfo methods.


OUString SAL_CALL PropertySetRegistry::getImplementationName()
{
    return OUString( "com.sun.star.comp.ucb.PropertySetRegistry" );
}

sal_Bool SAL_CALL PropertySetRegistry::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL PropertySetRegistry::getSupportedServiceNames()
{
    return { PROPSET_REG_SERVICE_NAME };
}


// XPropertySetRegistry methods.


// virtual
Reference< XPersistentPropertySet > SAL_CALL
PropertySetRegistry::openPropertySet( const OUString& key, sal_Bool create )
{
    if ( !key.isEmpty() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        PropertySetMap_Impl& rSets = m_pImpl->m_aPropSets;

        PropertySetMap_Impl::const_iterator it = rSets.find( key );
        if ( it != rSets.end() )
        {
            // Already instantiated.
            return Reference< XPersistentPropertySet >( (*it).second );
        }
        else
        {
            // Create new instance.
            Reference< XNameAccess > xRootNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
            if ( xRootNameAccess.is() )
            {
                // Propertyset in registry?
                if ( xRootNameAccess->hasByName( key ) )
                {
                    // Yep!
                    return Reference< XPersistentPropertySet >(
                                            new PersistentPropertySet(
                                                    *this, key ) );
                }
                else if ( create )
                {
                    // No. Create entry for propertyset.

                    Reference< XSingleServiceFactory > xFac(
                            getConfigWriteAccess( OUString() ), UNO_QUERY );
                    Reference< XChangesBatch >  xBatch( xFac, UNO_QUERY );
                    Reference< XNameContainer > xContainer( xFac, UNO_QUERY );

                    OSL_ENSURE( xFac.is(),
                                "PropertySetRegistry::openPropertySet - "
                                "No factory!" );

                    OSL_ENSURE( xBatch.is(),
                                "PropertySetRegistry::openPropertySet - "
                                "No batch!" );

                    OSL_ENSURE( xContainer.is(),
                                "PropertySetRegistry::openPropertySet - "
                                "No container!" );

                    if ( xFac.is() && xBatch.is() && xContainer.is() )
                    {
                        try
                        {
                            // Create new "Properties" config item.
                            Reference< XNameReplace > xNameReplace(
                                        xFac->createInstance(), UNO_QUERY );

                            if ( xNameReplace.is() )
                            {
                                // Fill new item...

                                // Insert new item.
                                xContainer->insertByName(
                                        key, makeAny( xNameReplace ) );
                                // Commit changes.
                                xBatch->commitChanges();

                                return Reference< XPersistentPropertySet >(
                                            new PersistentPropertySet(
                                                    *this, key ) );
                            }
                        }
                        catch (const IllegalArgumentException&)
                        {
                            // insertByName

                            OSL_FAIL( "PropertySetRegistry::openPropertySet - "
                                        "caught IllegalArgumentException!" );
                        }
                        catch (const ElementExistException&)
                        {
                            // insertByName

                            OSL_FAIL( "PropertySetRegistry::openPropertySet - "
                                        "caught ElementExistException!" );
                        }
                        catch (const WrappedTargetException&)
                        {
                            // insertByName, commitChanges

                            OSL_FAIL( "PropertySetRegistry::openPropertySet - "
                                        "caught WrappedTargetException!" );
                        }
                        catch (const RuntimeException&)
                        {
                            OSL_FAIL( "PropertySetRegistry::openPropertySet - "
                                        "caught RuntimeException!" );
                        }
                        catch (const Exception&)
                        {
                            // createInstance

                            OSL_FAIL( "PropertySetRegistry::openPropertySet - "
                                        "caught Exception!" );
                        }
                    }
                }
                else
                {
                    // No entry. Fail, but no error.
                    return Reference< XPersistentPropertySet >();
                }
            }

            SAL_WARN( "ucb", "no root access" );
        }
    }

    return Reference< XPersistentPropertySet >();
}


// virtual
void SAL_CALL PropertySetRegistry::removePropertySet( const OUString& key )
{
    if ( key.isEmpty() )
        return;

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XNameAccess > xRootNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootNameAccess.is() )
    {
        // Propertyset in registry?
        if ( !xRootNameAccess->hasByName( key ) )
            return;
        Reference< XChangesBatch > xBatch(
                            getConfigWriteAccess( OUString() ), UNO_QUERY );
        Reference< XNameContainer > xContainer( xBatch, UNO_QUERY );

        if ( xBatch.is() && xContainer.is() )
        {
            try
            {
                // Remove item.
                xContainer->removeByName( key );
                // Commit changes.
                xBatch->commitChanges();

                // Success.
                return;
            }
            catch (const NoSuchElementException&)
            {
                // removeByName

                OSL_FAIL( "PropertySetRegistry::removePropertySet - "
                            "caught NoSuchElementException!" );
                return;
            }
            catch (const WrappedTargetException&)
            {
                // commitChanges

                OSL_FAIL( "PropertySetRegistry::removePropertySet - "
                            "caught WrappedTargetException!" );
                return;
            }
        }

        return;
    }

    SAL_WARN( "ucb", "no root access" );
}


// XElementAccess methods.


// virtual
css::uno::Type SAL_CALL PropertySetRegistry::getElementType()
{
    return cppu::UnoType<XPersistentPropertySet>::get();
}


// virtual
sal_Bool SAL_CALL PropertySetRegistry::hasElements()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XElementAccess > xElemAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xElemAccess.is() )
        return xElemAccess->hasElements();

    return false;
}


// XNameAccess methods.


// virtual
Any SAL_CALL PropertySetRegistry::getByName( const OUString& aName )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XNameAccess > xNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {

        try
        {
            return xNameAccess->getByName( aName );
        }
        catch (const NoSuchElementException&)
        {
            // getByName
        }
        catch (const WrappedTargetException&)
        {
            // getByName
        }
    }

    return Any();
}


// virtual
Sequence< OUString > SAL_CALL PropertySetRegistry::getElementNames()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XNameAccess > xNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        return xNameAccess->getElementNames();
    }
    return Sequence< OUString >( 0 );
}


// virtual
sal_Bool SAL_CALL PropertySetRegistry::hasByName( const OUString& aName )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XNameAccess > xNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        return xNameAccess->hasByName( aName );
    }

    return false;
}


void PropertySetRegistry::add( PersistentPropertySet* pSet )
{
    OUString key( pSet->getKey() );

    if ( !key.isEmpty() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
        m_pImpl->m_aPropSets[ key ] = pSet;
    }
}


void PropertySetRegistry::remove( PersistentPropertySet* pSet )
{
    OUString key( pSet->getKey() );

    if ( !key.isEmpty() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        PropertySetMap_Impl& rSets = m_pImpl->m_aPropSets;

        PropertySetMap_Impl::iterator it = rSets.find( key );
        if ( it != rSets.end() )
        {
            // Found.
            rSets.erase( it );
        }
    }
}


void PropertySetRegistry::renamePropertySet( const OUString& rOldKey,
                                             const OUString& rNewKey )
{
    if ( rOldKey == rNewKey )
        return;

    Reference< XNameAccess > xRootNameAccess(
                            getConfigWriteAccess( OUString() ), UNO_QUERY );
    if ( xRootNameAccess.is() )
    {
        // Old key present?
        if ( xRootNameAccess->hasByName( rOldKey ) )
        {
            // New key not present?
            if ( xRootNameAccess->hasByName( rNewKey ) )
            {
                OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                            "New key exists!" );
                return;
            }
            Reference< XSingleServiceFactory > xFac(
                                                xRootNameAccess, UNO_QUERY );
            Reference< XChangesBatch >  xBatch( xFac, UNO_QUERY );
            Reference< XNameContainer > xContainer( xFac, UNO_QUERY );

            OSL_ENSURE( xFac.is(),
                        "PropertySetRegistry::renamePropertySet - "
                        "No factory!" );

            OSL_ENSURE( xBatch.is(),
                        "PropertySetRegistry::renamePropertySet - "
                        "No batch!" );

            OSL_ENSURE( xContainer.is(),
                        "PropertySetRegistry::renamePropertySet - "
                        "No container!" );

            if ( xFac.is() && xBatch.is() && xContainer.is() )
            {

                // Create new "Properties" config item.


                try
                {
                    Reference< XNameReplace > xNameReplace(
                                    xFac->createInstance(), UNO_QUERY );

                    if ( xNameReplace.is() )
                    {
                        // Insert new item.
                        xContainer->insertByName(
                                    rNewKey, makeAny( xNameReplace ) );
                        // Commit changes.
                        xBatch->commitChanges();
                    }
                }
                catch (const IllegalArgumentException&)
                {
                    // insertByName

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught IllegalArgumentException!" );
                    return;
                }
                catch (const ElementExistException&)
                {
                    // insertByName

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught ElementExistException!" );
                    return;
                }
                catch (const WrappedTargetException&)
                {
                    // insertByName, commitChanges

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught WrappedTargetException!" );
                    return;
                }
                catch (const RuntimeException&)
                {
                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught RuntimeException!" );
                    return;
                }
                catch (const Exception&)
                {
                    // createInstance

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught Exception!" );
                    return;
                }


                // Copy data...


                Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                                                xRootNameAccess, UNO_QUERY );
                if ( !xRootHierNameAccess.is() )
                {
                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "No hierarchical name access!" );
                    return;
                }

                try
                {
                    OUString aOldValuesKey
                        = makeHierarchalNameSegment( rOldKey );
                    aOldValuesKey += "/Values";

                    Reference< XNameAccess > xOldNameAccess;
                    xRootHierNameAccess->getByHierarchicalName(
                                                        aOldValuesKey )
                        >>= xOldNameAccess;
                    if ( !xOldNameAccess.is() )
                    {
                        OSL_FAIL( "PersistentPropertySet::renamePropertySet - "
                            "No old name access!" );
                        return;
                    }

                    // Obtain property names.
                    Sequence< OUString > aElems
                                    = xOldNameAccess->getElementNames();
                    sal_Int32 nCount = aElems.getLength();
                    if ( nCount )
                    {
                        OUString aNewValuesKey
                            = makeHierarchalNameSegment( rNewKey );
                        aNewValuesKey += "/Values";

                        Reference< XSingleServiceFactory > xNewFac;
                        xRootHierNameAccess->getByHierarchicalName(
                                                        aNewValuesKey )
                            >>= xNewFac;
                        if ( !xNewFac.is() )
                        {
                            OSL_FAIL( "PersistentPropertySet::renamePropertySet - "
                                "No new factory!" );
                            return;
                        }

                        Reference< XNameContainer > xNewContainer(
                                                    xNewFac, UNO_QUERY );
                        if ( !xNewContainer.is() )
                        {
                            OSL_FAIL( "PersistentPropertySet::renamePropertySet - "
                                "No new container!" );
                            return;
                        }

                        aOldValuesKey += "/";

                        OUString aHandleKey("/Handle");
                        OUString aValueKey("/Value");
                        OUString aStateKey("/State");
                        OUString aAttrKey("/Attributes");

                        for ( sal_Int32 n = 0; n < nCount; ++n )
                        {
                            const OUString& rPropName = aElems[ n ];

                            // Create new item.
                            Reference< XNameReplace > xNewPropNameReplace(
                                xNewFac->createInstance(), UNO_QUERY );

                            if ( !xNewPropNameReplace.is() )
                            {
                                OSL_FAIL( "PersistentPropertySet::renamePropertySet - "
                                    "No new prop name replace!" );
                                return;
                            }

                            // Fill new item...

                            // Set Values
                            OUString aKey = aOldValuesKey;
                            aKey += makeHierarchalNameSegment( rPropName );

                            // ... handle
                            OUString aNewKey1 = aKey;
                            aNewKey1 += aHandleKey;
                            Any aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName( "Handle", aAny );

                            // ... value
                            aNewKey1 = aKey;
                            aNewKey1 += aValueKey;
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName( "Value", aAny );

                            // ... state
                            aNewKey1 = aKey;
                            aNewKey1 += aStateKey;
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName( "State", aAny );

                            // ... attributes
                            aNewKey1 = aKey;
                            aNewKey1 += aAttrKey;
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName( "Attributes", aAny );

                            // Insert new item.
                            xNewContainer->insertByName(
                                rPropName, makeAny( xNewPropNameReplace ) );

                            // Commit changes.
                            xBatch->commitChanges();
                        }
                    }
                }
                catch (const IllegalArgumentException&)
                {
                    // insertByName, replaceByName

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught IllegalArgumentException!" );
                    return;
                }
                catch (const ElementExistException&)
                {
                    // insertByName

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught ElementExistException!" );
                    return;
                }
                catch (const WrappedTargetException&)
                {
                    // insertByName, replaceByName, commitChanges

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught WrappedTargetException!" );
                    return;
                }
                catch (const NoSuchElementException&)
                {
                    // getByHierarchicalName, replaceByName

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught NoSuchElementException!" );
                    return;
                }
                catch (const RuntimeException&)
                {
                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught RuntimeException!" );
                    return;
                }
                catch (const Exception&)
                {
                    // createInstance

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught Exception!" );
                    return;
                }


                // Remove old entry...


                try
                {
                    // Remove item.
                    xContainer->removeByName( rOldKey );
                    // Commit changes.
                    xBatch->commitChanges();

                    // Success.
                    return;
                }
                catch (const NoSuchElementException&)
                {
                    // removeByName

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught NoSuchElementException!" );
                    return;
                }
                catch (const WrappedTargetException&)
                {
                    // commitChanges

                    OSL_FAIL( "PropertySetRegistry::renamePropertySet - "
                                "caught WrappedTargetException!" );
                    return;
                }
            }
        }
    }

    OSL_FAIL( "PropertySetRegistry::renamePropertySet - Error!" );
}


Reference< XMultiServiceFactory > PropertySetRegistry::getConfigProvider()
{
    if ( !m_pImpl->m_xConfigProvider.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
        if ( !m_pImpl->m_xConfigProvider.is() )
        {
            const Sequence< Any >& rInitArgs = m_pImpl->m_aInitArgs;

            if ( rInitArgs.getLength() > 0 )
            {
                // Extract config provider from service init args.
                rInitArgs[ 0 ] >>= m_pImpl->m_xConfigProvider;

                OSL_ENSURE( m_pImpl->m_xConfigProvider.is(),
                            "PropertySetRegistry::getConfigProvider - "
                            "No config provider!" );
            }
            else
            {
                try
                {
                    m_pImpl->m_xConfigProvider = theDefaultProvider::get( m_xContext );
                }
                catch (const Exception&)
                {
                    SAL_WARN( "ucb", "caught exception!" );
                }
            }
        }
    }

    return m_pImpl->m_xConfigProvider;
}


Reference< XInterface > PropertySetRegistry::getRootConfigReadAccess()
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( !m_pImpl->m_xRootReadAccess.is() )
        {
            if ( m_pImpl->m_bTriedToGetRootReadAccess )
            {
                OSL_FAIL( "PropertySetRegistry::getRootConfigReadAccess - "
                            "Unable to read any config data! -> #82494#" );
                return Reference< XInterface >();
            }

            getConfigProvider();

            if ( m_pImpl->m_xConfigProvider.is() )
            {
                Sequence< Any > aArguments( 1 );
                PropertyValue aProperty;
                aProperty.Name = CFGPROPERTY_NODEPATH;
                aProperty.Value
                    <<= OUString( STORE_CONTENTPROPERTIES_KEY  );
                aArguments[ 0 ] <<= aProperty;

                m_pImpl->m_bTriedToGetRootReadAccess = true;

                m_pImpl->m_xRootReadAccess =
                    m_pImpl->m_xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationAccess",
                        aArguments );

                if ( m_pImpl->m_xRootReadAccess.is() )
                    return m_pImpl->m_xRootReadAccess;
            }
        }
        else
            return m_pImpl->m_xRootReadAccess;
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
        // createInstance, createInstanceWithArguments

        OSL_FAIL( "PropertySetRegistry::getRootConfigReadAccess - caught Exception!" );
        return Reference< XInterface >();
    }

    SAL_WARN( "ucb", "Error!" );
    return Reference< XInterface >();
}


Reference< XInterface > PropertySetRegistry::getConfigWriteAccess(
                                                    const OUString& rPath )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( !m_pImpl->m_xRootWriteAccess.is() )
        {
            if ( m_pImpl->m_bTriedToGetRootWriteAccess )
            {
                OSL_FAIL( "PropertySetRegistry::getConfigWriteAccess - "
                            "Unable to write any config data! -> #82494#" );
                return Reference< XInterface >();
            }

            getConfigProvider();

            if ( m_pImpl->m_xConfigProvider.is() )
            {
                Sequence< Any > aArguments( 2 );
                PropertyValue   aProperty;

                aProperty.Name = CFGPROPERTY_NODEPATH;
                aProperty.Value <<= OUString( STORE_CONTENTPROPERTIES_KEY  );
                aArguments[ 0 ] <<= aProperty;

                aProperty.Name = CFGPROPERTY_LAZYWRITE;
                aProperty.Value <<= true;
                aArguments[ 1 ] <<= aProperty;

                m_pImpl->m_bTriedToGetRootWriteAccess = true;

                m_pImpl->m_xRootWriteAccess =
                    m_pImpl->m_xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationUpdateAccess",
                        aArguments );

                OSL_ENSURE( m_pImpl->m_xRootWriteAccess.is(),
                            "PropertySetRegistry::getConfigWriteAccess - "
                            "No config update access!" );
            }
        }

        if ( m_pImpl->m_xRootWriteAccess.is() )
        {
            if ( !rPath.isEmpty() )
            {
                Reference< XHierarchicalNameAccess > xNA(
                                m_pImpl->m_xRootWriteAccess, UNO_QUERY );
                if ( xNA.is() )
                {
                    Reference< XInterface > xInterface;
                    xNA->getByHierarchicalName( rPath ) >>= xInterface;

                    if ( xInterface.is() )
                        return xInterface;
                }
            }
            else
                return m_pImpl->m_xRootWriteAccess;
        }
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const NoSuchElementException&)
    {
        // getByHierarchicalName

        OSL_FAIL( "PropertySetRegistry::getConfigWriteAccess - "
            "caught NoSuchElementException!" );
        return Reference< XInterface >();
    }
    catch (const Exception&)
    {
        // createInstance, createInstanceWithArguments

        OSL_FAIL( "PropertySetRegistry::getConfigWriteAccess - "
                    "caught Exception!" );
        return Reference< XInterface >();
    }

    OSL_FAIL( "PropertySetRegistry::getConfigWriteAccess - Error!" );
    return Reference< XInterface >();
}

typedef OMultiTypeInterfaceContainerHelperVar<OUString> PropertyListeners_Impl;

struct PersistentPropertySet_Impl
{
    rtl::Reference<PropertySetRegistry>  m_pCreator;
    rtl::Reference<PropertySetInfo_Impl> m_pInfo;
    OUString                    m_aKey;
    OUString                    m_aFullKey;
    osl::Mutex                  m_aMutex;
    OInterfaceContainerHelper2*  m_pDisposeEventListeners;
    OInterfaceContainerHelper2*  m_pPropSetChangeListeners;
    PropertyListeners_Impl*     m_pPropertyChangeListeners;

    PersistentPropertySet_Impl( PropertySetRegistry& rCreator,
                                const OUString& rKey )
    : m_pCreator( &rCreator ), m_pInfo( nullptr ), m_aKey( rKey ),
      m_pDisposeEventListeners( nullptr ), m_pPropSetChangeListeners( nullptr ),
      m_pPropertyChangeListeners( nullptr )
    {
    }

    ~PersistentPropertySet_Impl()
    {
        delete m_pDisposeEventListeners;
        delete m_pPropSetChangeListeners;
        delete m_pPropertyChangeListeners;
    }
};


// PersistentPropertySet Implementation.


PersistentPropertySet::PersistentPropertySet(
                        PropertySetRegistry& rCreator,
                        const OUString& rKey )
: m_pImpl( new PersistentPropertySet_Impl( rCreator, rKey ) )
{
    // register at creator.
    rCreator.add( this );
}


// virtual
PersistentPropertySet::~PersistentPropertySet()
{
    // deregister at creator.
    m_pImpl->m_pCreator->remove( this );
}

// XServiceInfo methods.

OUString SAL_CALL PersistentPropertySet::getImplementationName()
{
    return OUString( "com.sun.star.comp.ucb.PersistentPropertySet" );
}

sal_Bool SAL_CALL PersistentPropertySet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL
PersistentPropertySet::getSupportedServiceNames()
{
    return { PERS_PROPSET_SERVICE_NAME };
}


// XComponent methods.


// virtual
void SAL_CALL PersistentPropertySet::dispose()
{
    if ( m_pImpl->m_pDisposeEventListeners &&
         m_pImpl->m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this  );
        m_pImpl->m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pPropSetChangeListeners &&
         m_pImpl->m_pPropSetChangeListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySetInfoChangeNotifier * >( this  );
        m_pImpl->m_pPropSetChangeListeners->disposeAndClear( aEvt );
    }

    if ( m_pImpl->m_pPropertyChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this  );
        m_pImpl->m_pPropertyChangeListeners->disposeAndClear( aEvt );
    }
}


// virtual
void SAL_CALL PersistentPropertySet::addEventListener(
                            const Reference< XEventListener >& Listener )
{
    if ( !m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners =
                    new OInterfaceContainerHelper2( m_pImpl->m_aMutex );

    m_pImpl->m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL PersistentPropertySet::removeEventListener(
                            const Reference< XEventListener >& Listener )
{
    if ( m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners->removeInterface( Listener );

    // Note: Don't want to delete empty container here -> performance.
}


// XPropertySet methods.


// virtual
Reference< XPropertySetInfo > SAL_CALL PersistentPropertySet::getPropertySetInfo()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_pInfo.is() )
    {
        m_pImpl->m_pInfo = new PropertySetInfo_Impl( this );
    }
    return Reference< XPropertySetInfo >( m_pImpl->m_pInfo.get() );
}


// virtual
void SAL_CALL PersistentPropertySet::setPropertyValue( const OUString& aPropertyName,
                                                       const Any& aValue )
{
    if ( aPropertyName.isEmpty() )
        throw UnknownPropertyException();

    osl::ClearableGuard< osl::Mutex > aCGuard( m_pImpl->m_aMutex );

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullPropName( getFullKey() );
        aFullPropName += "/";
        aFullPropName += makeHierarchalNameSegment( aPropertyName );

        // Does property exist?
        if ( xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
        {
            Reference< XNameReplace > xNameReplace(
                    m_pImpl->m_pCreator->getConfigWriteAccess(
                                            aFullPropName ), UNO_QUERY );
            Reference< XChangesBatch > xBatch(
                    m_pImpl->m_pCreator->getConfigWriteAccess(
                                            OUString() ), UNO_QUERY );

            if ( xNameReplace.is() && xBatch.is() )
            {
                try
                {
                    // Obtain old value
                    OUString aValueName = aFullPropName;
                    aValueName += "/Value";
                    Any aOldValue
                        = xRootHierNameAccess->getByHierarchicalName(
                                                                aValueName );
                    // Check value type.
                    if ( aOldValue.getValueType() != aValue.getValueType() )
                    {
                        aCGuard.clear();
                        throw IllegalArgumentException();
                    }

                    // Write value
                    xNameReplace->replaceByName( "Value", aValue );

                    // Write state ( Now it is a directly set value )
                    xNameReplace->replaceByName(
                                    "State",
                                    makeAny(
                                        sal_Int32(
                                            PropertyState_DIRECT_VALUE ) ) );

                    // Commit changes.
                    xBatch->commitChanges();

                    PropertyChangeEvent aEvt;
                    if ( m_pImpl->m_pPropertyChangeListeners )
                    {
                        // Obtain handle
                        aValueName = aFullPropName;
                        aValueName += "/Handle";
                        sal_Int32 nHandle = -1;
                        xRootHierNameAccess->getByHierarchicalName( aValueName )
                            >>= nHandle;

                        aEvt.Source         = static_cast<OWeakObject*>(this);
                        aEvt.PropertyName   = aPropertyName;
                        aEvt.PropertyHandle = nHandle;
                        aEvt.Further        = false;
                        aEvt.OldValue       = aOldValue;
                        aEvt.NewValue       = aValue;

                        // Callback follows!
                        aCGuard.clear();

                        notifyPropertyChangeEvent( aEvt );
                    }
                    return;
                }
                catch (const IllegalArgumentException&)
                {
                    // replaceByName
                }
                catch (const NoSuchElementException&)
                {
                    // getByHierarchicalName, replaceByName
                }
                catch (const WrappedTargetException&)
                {
                    // replaceByName, commitChanges
                }
            }
        }
    }

    throw UnknownPropertyException();
}


// virtual
Any SAL_CALL PersistentPropertySet::getPropertyValue(
                                            const OUString& PropertyName )
{
    if ( PropertyName.isEmpty() )
        throw UnknownPropertyException();

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XHierarchicalNameAccess > xNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        OUString aFullPropName( getFullKey() );
        aFullPropName += "/";
        aFullPropName += makeHierarchalNameSegment( PropertyName );
        aFullPropName += "/Value";
        try
        {
            return xNameAccess->getByHierarchicalName( aFullPropName );
        }
        catch (const NoSuchElementException&)
        {
            throw UnknownPropertyException();
        }
    }

    throw UnknownPropertyException();
}


// virtual
void SAL_CALL PersistentPropertySet::addPropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& xListener )
{
//  load();

    if ( !m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners =
                    new PropertyListeners_Impl( m_pImpl->m_aMutex );

    m_pImpl->m_pPropertyChangeListeners->addInterface(
                                                aPropertyName, xListener );
}


// virtual
void SAL_CALL PersistentPropertySet::removePropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& aListener )
{
//  load();

    if ( m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners->removeInterface(
                                                aPropertyName, aListener );

    // Note: Don't want to delete empty container here -> performance.
}


// virtual
void SAL_CALL PersistentPropertySet::addVetoableChangeListener(
                    const OUString&,
                    const Reference< XVetoableChangeListener >& )
{
//  load();
//  OSL_FAIL( //                "PersistentPropertySet::addVetoableChangeListener - N.Y.I." );
}


// virtual
void SAL_CALL PersistentPropertySet::removeVetoableChangeListener(
                    const OUString&,
                    const Reference< XVetoableChangeListener >& )
{
//  load();
//  OSL_FAIL( //                "PersistentPropertySet::removeVetoableChangeListener - N.Y.I." );
}


// XPersistentPropertySet methods.


// virtual
Reference< XPropertySetRegistry > SAL_CALL PersistentPropertySet::getRegistry()
{
    return Reference< XPropertySetRegistry >( m_pImpl->m_pCreator.get() );
}


// virtual
OUString SAL_CALL PersistentPropertySet::getKey()
{
    return m_pImpl->m_aKey;
}


// XNamed methods.


// virtual
OUString SAL_CALL PersistentPropertySet::getName()
{
    // same as getKey()
    return m_pImpl->m_aKey;
}


// virtual
void SAL_CALL PersistentPropertySet::setName( const OUString& aName )
{
    if ( aName != m_pImpl->m_aKey )
        m_pImpl->m_pCreator->renamePropertySet( m_pImpl->m_aKey, aName );
}


// XPropertyContainer methods.


// virtual
void SAL_CALL PersistentPropertySet::addProperty(
        const OUString& Name, sal_Int16 Attributes, const Any& DefaultValue )
{
    if ( Name.isEmpty() )
        throw IllegalArgumentException();

    // @@@ What other types can't be written to config server?

    // Check type class ( Not all types can be written to storage )
    TypeClass eTypeClass = DefaultValue.getValueTypeClass();
    if ( eTypeClass == TypeClass_INTERFACE )
        throw IllegalTypeException();

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    // Property already in set?

    OUString aFullValuesName;

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        aFullValuesName = getFullKey();
        OUString aFullPropName = aFullValuesName;
        aFullPropName += "/";
        aFullPropName += makeHierarchalNameSegment( Name );

        if ( xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
        {
            // Already in set.
            throw PropertyExistException();
        }
    }

    // Property is always removable.
    Attributes |= PropertyAttribute::REMOVABLE;

    // Add property.

    Reference< XSingleServiceFactory > xFac(
                m_pImpl->m_pCreator->getConfigWriteAccess( aFullValuesName ),
                UNO_QUERY );
    Reference< XNameContainer > xContainer( xFac, UNO_QUERY );
    Reference< XChangesBatch >  xBatch(
                m_pImpl->m_pCreator->getConfigWriteAccess( OUString() ),
                UNO_QUERY );

    OSL_ENSURE( xFac.is(),
                "PersistentPropertySet::addProperty - No factory!" );

    OSL_ENSURE( xBatch.is(),
                "PersistentPropertySet::addProperty - No batch!" );

    OSL_ENSURE( xContainer.is(),
                "PersistentPropertySet::addProperty - No container!" );

    if ( xFac.is() && xBatch.is() && xContainer.is() )
    {
        try
        {
            // Create new "PropertyValue" config item.
            Reference< XNameReplace > xNameReplace(
                                        xFac->createInstance(), UNO_QUERY );

            if ( xNameReplace.is() )
            {
                // Fill new item...

                // Set handle
                xNameReplace->replaceByName(
                                    "Handle",
                                    makeAny( sal_Int32( -1 ) ) );

                // Set default value
                xNameReplace->replaceByName(
                                    "Value",
                                    DefaultValue );

                // Set state ( always "default" )
                xNameReplace->replaceByName(
                                    "State",
                                    makeAny(
                                        sal_Int32(
                                            PropertyState_DEFAULT_VALUE ) ) );

                // Set attributes
                xNameReplace->replaceByName(
                                    "Attributes",
                                    makeAny( sal_Int32( Attributes ) ) );

                // Insert new item.
                xContainer->insertByName( Name, makeAny( xNameReplace ) );

                // Commit changes.
                xBatch->commitChanges();

                // Property set info is invalid.
                if ( m_pImpl->m_pInfo.is() )
                    m_pImpl->m_pInfo->reset();

                // Notify propertyset info change listeners.
                if ( m_pImpl->m_pPropSetChangeListeners &&
                     m_pImpl->m_pPropSetChangeListeners->getLength() )
                {
                    PropertySetInfoChangeEvent evt(
                                    static_cast< OWeakObject * >( this ),
                                    Name,
                                    -1,
                                    PropertySetInfoChange::PROPERTY_INSERTED );
                    notifyPropertySetInfoChange( evt );
                }

                // Success.
                return;
            }
        }
        catch (const IllegalArgumentException&)
        {
            // insertByName

            OSL_FAIL( "PersistentPropertySet::addProperty - "
                        "caught IllegalArgumentException!" );
            return;
        }
        catch (const ElementExistException&)
        {
            // insertByName

            OSL_FAIL( "PersistentPropertySet::addProperty - "
                        "caught ElementExistException!" );
            return;
        }
        catch (const WrappedTargetException&)
        {
            // replaceByName, insertByName, commitChanges

            OSL_FAIL( "PersistentPropertySet::addProperty - "
                        "caught WrappedTargetException!" );
            return;
        }
        catch (const RuntimeException&)
        {
            throw;
        }
        catch (const Exception&)
        {
            // createInstance

            OSL_FAIL( "PersistentPropertySet::addProperty - "
                        "caught Exception!" );
            return;
        }
    }

    OSL_FAIL( "PersistentPropertySet::addProperty - Error!" );
}


// virtual
void SAL_CALL PersistentPropertySet::removeProperty( const OUString& Name )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    OUString aFullValuesName;
    OUString aFullPropName;

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        aFullValuesName = getFullKey();
        aFullPropName   = aFullValuesName;
        aFullPropName   += "/";
        aFullPropName   += makeHierarchalNameSegment( Name );

        // Property in set?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            throw UnknownPropertyException();

        // Property removable?
        try
        {
            OUString aFullAttrName = aFullPropName;
            aFullAttrName += "/Attributes";

            sal_Int32 nAttribs = 0;
            if ( xRootHierNameAccess->getByHierarchicalName( aFullAttrName )
                    >>= nAttribs )
            {
                if ( !( nAttribs & PropertyAttribute::REMOVABLE ) )
                {
                    // Not removable!
                    throw NotRemoveableException();
                }
            }
            else
            {
                OSL_FAIL( "PersistentPropertySet::removeProperty - "
                            "No attributes!" );
                return;
            }
        }
        catch (const NoSuchElementException&)
        {
            // getByHierarchicalName

            OSL_FAIL( "PersistentPropertySet::removeProperty - "
                        "caught NoSuchElementException!" );
        }

        // Remove property...

        Reference< XNameContainer > xContainer(
                m_pImpl->m_pCreator->getConfigWriteAccess( aFullValuesName ),
                UNO_QUERY );
        Reference< XChangesBatch > xBatch(
                m_pImpl->m_pCreator->getConfigWriteAccess( OUString() ),
                UNO_QUERY );

        OSL_ENSURE( xBatch.is(),
                    "PersistentPropertySet::removeProperty - No batch!" );

        OSL_ENSURE( xContainer.is(),
                    "PersistentPropertySet::removeProperty - No container!" );

        if ( xBatch.is() && xContainer.is() )
        {
            try
            {
                sal_Int32 nHandle = -1;

                if ( m_pImpl->m_pPropSetChangeListeners &&
                       m_pImpl->m_pPropSetChangeListeners->getLength() )
                {
                    // Obtain property handle ( needed for propertysetinfo
                    // change event )...

                    try
                    {
                        OUString aFullHandleName = aFullPropName;
                        aFullHandleName
                                += "/Handle";

                        if ( ! ( xRootHierNameAccess->getByHierarchicalName(
                                        aFullHandleName ) >>= nHandle ) )
                            nHandle = -1;

                    }
                    catch (const NoSuchElementException&)
                    {
                        // getByHierarchicalName

                        OSL_FAIL( "PersistentPropertySet::removeProperty - "
                                    "caught NoSuchElementException!" );
                        nHandle = -1;
                    }
                }

                xContainer->removeByName( Name );
                xBatch->commitChanges();

                // Property set info is invalid.
                if ( m_pImpl->m_pInfo.is() )
                    m_pImpl->m_pInfo->reset();

                // Notify propertyset info change listeners.
                if ( m_pImpl->m_pPropSetChangeListeners &&
                      m_pImpl->m_pPropSetChangeListeners->getLength() )
                {
                    PropertySetInfoChangeEvent evt(
                                    static_cast< OWeakObject * >( this ),
                                    Name,
                                    nHandle,
                                    PropertySetInfoChange::PROPERTY_REMOVED );
                    notifyPropertySetInfoChange( evt );
                }

                // Success.
                return;
            }
            catch (const NoSuchElementException&)
            {
                // removeByName

                OSL_FAIL( "PersistentPropertySet::removeProperty - "
                            "caught NoSuchElementException!" );
                return;
            }
            catch (const WrappedTargetException&)
            {
                // commitChanges

                OSL_FAIL( "PersistentPropertySet::removeProperty - "
                            "caught WrappedTargetException!" );
                return;
            }
        }
    }

    OSL_FAIL( "PersistentPropertySet::removeProperty - Error!" );
}


// XPropertySetInfoChangeNotifier methods.


// virtual
void SAL_CALL PersistentPropertySet::addPropertySetInfoChangeListener(
                const Reference< XPropertySetInfoChangeListener >& Listener )
{
    if ( !m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners =
                    new OInterfaceContainerHelper2( m_pImpl->m_aMutex );

    m_pImpl->m_pPropSetChangeListeners->addInterface( Listener );
}


// virtual
void SAL_CALL PersistentPropertySet::removePropertySetInfoChangeListener(
                const Reference< XPropertySetInfoChangeListener >& Listener )
{
    if ( m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners->removeInterface( Listener );
}


// XPropertyAccess methods.


// virtual
Sequence< PropertyValue > SAL_CALL PersistentPropertySet::getPropertyValues()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        try
        {
            Reference< XNameAccess > xNameAccess;
            xRootHierNameAccess->getByHierarchicalName(getFullKey())
                >>= xNameAccess;
            if ( xNameAccess.is() )
            {
                // Obtain property names.

                Sequence< OUString > aElems = xNameAccess->getElementNames();

                sal_Int32 nCount = aElems.getLength();
                if ( nCount )
                {
                    Reference< XHierarchicalNameAccess > xHierNameAccess(
                                                    xNameAccess, UNO_QUERY );

                    OSL_ENSURE( xHierNameAccess.is(),
                                "PersistentPropertySet::getPropertyValues - "
                                "No hierarchical name access!" );

                    if ( xHierNameAccess.is() )
                    {
                        Sequence< PropertyValue > aValues( nCount );

                        const OUString aHandleName("/Handle");
                        const OUString aValueName("/Value");
                        const OUString aStateName("/State");

                        for ( sal_Int32 n = 0; n < nCount; ++n )
                        {
                            PropertyValue& rValue = aValues[ n ];
                            OUString rName    = aElems[ n ];
                            OUString aXMLName
                                        = makeHierarchalNameSegment( rName );

                            // Set property name.

                            rValue.Name = rName;

                            try
                            {
                                // Obtain and set property handle
                                OUString aHierName = aXMLName;
                                aHierName += aHandleName;
                                Any aKeyValue
                                    = xHierNameAccess->getByHierarchicalName(
                                        aHierName );

                                if ( !( aKeyValue >>= rValue.Handle ) )
                                    OSL_FAIL( "PersistentPropertySet::getPropertyValues - "
                                      "Error getting property handle!" );
                            }
                            catch (const NoSuchElementException&)
                            {
                                // getByHierarchicalName

                                OSL_FAIL( "PersistentPropertySet::getPropertyValues - "
                                  "NoSuchElementException!" );
                            }

                            try
                            {
                                // Obtain and set property value
                                OUString aHierName = aXMLName;
                                aHierName += aValueName;
                                rValue.Value
                                    = xHierNameAccess->getByHierarchicalName(
                                        aHierName );

                                // Note: The value may be void if addProperty
                                //       was called with a default value
                                //       of type void.
                            }
                            catch (const NoSuchElementException&)
                            {
                                // getByHierarchicalName

                                OSL_FAIL( "PersistentPropertySet::getPropertyValues - "
                                  "NoSuchElementException!" );
                            }

                            try
                            {
                                // Obtain and set property state
                                OUString aHierName = aXMLName;
                                aHierName += aStateName;
                                Any aKeyValue
                                    = xHierNameAccess->getByHierarchicalName(
                                        aHierName );

                                sal_Int32 nState = 0;
                                if ( !( aKeyValue >>= nState ) )
                                    OSL_FAIL( "PersistentPropertySet::getPropertyValues - "
                                      "Error getting property state!" );
                                else
                                    rValue.State = PropertyState( nState );
                            }
                            catch (const NoSuchElementException&)
                            {
                                // getByHierarchicalName

                                OSL_FAIL( "PersistentPropertySet::getPropertyValues - "
                                  "NoSuchElementException!" );
                            }
                        }

                        return aValues;
                    }
                }
            }
        }
        catch (const NoSuchElementException&)
        {
            // getByHierarchicalName
        }
    }

    return Sequence< PropertyValue >( 0 );
}


// virtual
void SAL_CALL PersistentPropertySet::setPropertyValues(
                                 const Sequence< PropertyValue >& aProps )
{
    sal_Int32 nCount = aProps.getLength();
    if ( !nCount )
        return;

    osl::ClearableGuard< osl::Mutex > aCGuard( m_pImpl->m_aMutex );

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        const PropertyValue* pNewValues = aProps.getConstArray();

        typedef std::list< PropertyChangeEvent > Events;
        Events aEvents;

        OUString aFullPropNamePrefix( getFullKey() );
        aFullPropNamePrefix += "/";

        // Iterate over given property value sequence.
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const PropertyValue& rNewValue = pNewValues[ n ];
            const OUString& rName = rNewValue.Name;

            OUString aFullPropName = aFullPropNamePrefix;
            aFullPropName += makeHierarchalNameSegment( rName );

            // Does property exist?
            if ( xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            {
                Reference< XNameReplace > xNameReplace(
                    m_pImpl->m_pCreator->getConfigWriteAccess(
                                            aFullPropName ), UNO_QUERY );
                Reference< XChangesBatch > xBatch(
                    m_pImpl->m_pCreator->getConfigWriteAccess(
                                            OUString() ), UNO_QUERY );

                if ( xNameReplace.is() && xBatch.is() )
                {
                    try
                    {
                        // Write handle
                        xNameReplace->replaceByName(
                                    "Handle",
                                    makeAny( rNewValue.Handle ) );

                        // Save old value
                        OUString aValueName = aFullPropName;
                        aValueName += "/Value";
                        Any aOldValue
                            = xRootHierNameAccess->getByHierarchicalName(
                                                                aValueName );
                        // Write value
                        xNameReplace->replaceByName(
                                    "Value",
                                    rNewValue.Value );

                        // Write state ( Now it is a directly set value )
                        xNameReplace->replaceByName(
                                    "State",
                                    makeAny(
                                        sal_Int32(
                                            PropertyState_DIRECT_VALUE ) ) );

                        // Commit changes.
                        xBatch->commitChanges();

                        if ( m_pImpl->m_pPropertyChangeListeners )
                        {
                            PropertyChangeEvent aEvt;
                            aEvt.Source         = static_cast<OWeakObject*>(this);
                            aEvt.PropertyName   = rNewValue.Name;
                            aEvt.PropertyHandle = rNewValue.Handle;
                            aEvt.Further        = false;
                            aEvt.OldValue       = aOldValue;
                            aEvt.NewValue       = rNewValue.Value;

                            aEvents.push_back( aEvt );
                        }
                    }
                    catch (const IllegalArgumentException&)
                    {
                        // replaceByName
                    }
                    catch (const NoSuchElementException&)
                    {
                        // getByHierarchicalName, replaceByName
                    }
                    catch (const WrappedTargetException&)
                    {
                        // replaceByName, commitChanges
                    }
                }
            }
        }

        // Callback follows!
        aCGuard.clear();

        if ( m_pImpl->m_pPropertyChangeListeners )
        {
            // Notify property changes.
            Events::const_iterator it  = aEvents.begin();
            Events::const_iterator end = aEvents.end();

            while ( it != end )
            {
                notifyPropertyChangeEvent( (*it) );
                ++it;
            }
        }

        return;
    }

    OSL_FAIL( "PersistentPropertySet::setPropertyValues - Nothing set!" );
}


// Non-interface methods


void PersistentPropertySet::notifyPropertyChangeEvent(
                                    const PropertyChangeEvent& rEvent ) const
{
    // Get "normal" listeners for the property.
    OInterfaceContainerHelper* pContainer =
            m_pImpl->m_pPropertyChangeListeners->getContainer(
                                                    rEvent.PropertyName );
    if ( pContainer && pContainer->getLength() )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while ( aIter.hasMoreElements() )
        {
            // Propagate event.
            Reference< XPropertyChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if ( xListener.is() )
                xListener->propertyChange( rEvent );
        }
    }

    // Get "normal" listeners for all properties.
    OInterfaceContainerHelper* pNoNameContainer =
            m_pImpl->m_pPropertyChangeListeners->getContainer( OUString() );
    if ( pNoNameContainer && pNoNameContainer->getLength() )
    {
        OInterfaceIteratorHelper aIter( *pNoNameContainer );
        while ( aIter.hasMoreElements() )
        {
            // Propagate event.
            Reference< XPropertyChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if ( xListener.is() )
                xListener->propertyChange( rEvent );
        }
    }
}


void PersistentPropertySet::notifyPropertySetInfoChange(
                                const PropertySetInfoChangeEvent& evt ) const
{
    if ( !m_pImpl->m_pPropSetChangeListeners )
        return;

    // Notify event listeners.
    OInterfaceIteratorHelper2 aIter( *( m_pImpl->m_pPropSetChangeListeners ) );
    while ( aIter.hasMoreElements() )
    {
        // Propagate event.
        Reference< XPropertySetInfoChangeListener >
                            xListener( aIter.next(), UNO_QUERY );
        if ( xListener.is() )
            xListener->propertySetInfoChange( evt );
    }
}


const OUString& PersistentPropertySet::getFullKey()
{
    if ( m_pImpl->m_aFullKey.isEmpty() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
        if ( m_pImpl->m_aFullKey.isEmpty() )
        {
            m_pImpl->m_aFullKey
                = makeHierarchalNameSegment( m_pImpl->m_aKey );
            m_pImpl->m_aFullKey
                += "/Values";
        }
    }

    return m_pImpl->m_aFullKey;
}


PropertySetRegistry& PersistentPropertySet::getPropertySetRegistry()
{
    return *m_pImpl->m_pCreator;
}


// PropertySetInfo_Impl Implementation.


PropertySetInfo_Impl::PropertySetInfo_Impl(
                        PersistentPropertySet* pOwner )
: m_pProps( nullptr ),
  m_pOwner( pOwner )
{
}


// XPropertySetInfo methods.


// virtual
Sequence< Property > SAL_CALL PropertySetInfo_Impl::getProperties()
{
    if ( !m_pProps )
    {
        Reference< XHierarchicalNameAccess > xRootHierNameAccess(
            m_pOwner->getPropertySetRegistry().getRootConfigReadAccess(),
            UNO_QUERY );
        if ( xRootHierNameAccess.is() )
        {
            try
            {
                Reference< XNameAccess > xNameAccess;
                xRootHierNameAccess->getByHierarchicalName(
                            m_pOwner->getFullKey() )
                    >>= xNameAccess;
                if ( xNameAccess.is() )
                {
                    // Obtain property names.

                    Sequence< OUString > aElems
                                            = xNameAccess->getElementNames();

                    sal_uInt32 nCount = aElems.getLength();
                    Sequence< Property >* pPropSeq
                                        = new Sequence< Property >( nCount );

                    if ( nCount )
                    {
                        Reference< XHierarchicalNameAccess > xHierNameAccess(
                                                    xNameAccess, UNO_QUERY );

                        OSL_ENSURE( xHierNameAccess.is(),
                                    "PropertySetInfo_Impl::getProperties - "
                                    "No hierarchical name access!" );

                        if ( xHierNameAccess.is() )
                        {
                            const OUString aHandleName("/Handle");
                            const OUString aValueName("/Value");
                            const OUString aAttrName("/Attributes");

                            Property* pProps = pPropSeq->getArray();

                            for ( sal_uInt32 n = 0; n < nCount; ++n )
                            {
                                Property& rProp = pProps[ n ];
                                OUString  rName = aElems[ n ];
                                OUString aXMLName
                                    = makeHierarchalNameSegment( rName );

                                // Set property name.

                                rProp.Name = rName;

                                try
                                {
                                    // Obtain and set property handle
                                    OUString aHierName = aXMLName;
                                    aHierName += aHandleName;
                                    Any aKeyValue
                                        = xHierNameAccess->getByHierarchicalName(
                                            aHierName );

                                    if ( !( aKeyValue >>= rProp.Handle ) )
                                        OSL_FAIL( "PropertySetInfo_Impl::getProperties - "
                                          "Error getting property handle!" );
                                }
                                catch (const NoSuchElementException&)
                                {
                                    // getByHierarchicalName

                                    OSL_FAIL( "PropertySetInfo_Impl::getProperties - "
                                      "NoSuchElementException!" );
                                }

                                try
                                {
                                    // Obtain and set property type
                                    OUString aHierName = aXMLName;
                                    aHierName += aValueName;
                                    Any aKeyValue
                                        = xHierNameAccess->getByHierarchicalName(
                                            aHierName );

                                    // Note: The type may be void if addProperty
                                    //       was called with a default value
                                    //       of type void.

                                    rProp.Type = aKeyValue.getValueType();
                                }
                                catch (const NoSuchElementException&)
                                {
                                    // getByHierarchicalName

                                    OSL_FAIL( "PropertySetInfo_Impl::getProperties - "
                                      "NoSuchElementException!" );
                                }

                                try
                                {
                                    // Obtain and set property attributes
                                    OUString aHierName = aXMLName;
                                    aHierName += aAttrName;
                                    Any aKeyValue
                                        = xHierNameAccess->getByHierarchicalName(
                                            aHierName );

                                    sal_Int32 nAttribs = 0;
                                    if ( aKeyValue >>= nAttribs )
                                        rProp.Attributes
                                            = sal_Int16( nAttribs );
                                    else
                                        OSL_FAIL( "PropertySetInfo_Impl::getProperties - "
                                          "Error getting property attributes!" );
                                }
                                catch (const NoSuchElementException&)
                                {
                                    // getByHierarchicalName

                                    OSL_FAIL( "PropertySetInfo_Impl::getProperties - "
                                      "NoSuchElementException!" );
                                }
                            }
                        }
                    }

                    // Success.
                    m_pProps.reset( pPropSeq );
                    return *m_pProps;
                }
            }
            catch (const NoSuchElementException&)
            {
                // getByHierarchicalName
            }
        }

        OSL_FAIL( "PropertySetInfo_Impl::getProperties - Error!" );
        m_pProps.reset( new Sequence< Property >( 0 ) );
    }

    return *m_pProps;
}


// virtual
Property SAL_CALL PropertySetInfo_Impl::getPropertyByName(
                                                    const OUString& aName )
{
    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
            m_pOwner->getPropertySetRegistry().getRootConfigReadAccess(),
            UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullPropName( m_pOwner->getFullKey() );
        aFullPropName += "/";
        aFullPropName += makeHierarchalNameSegment( aName );

        // Does property exist?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            throw UnknownPropertyException();

        try
        {
            Property aProp;

            // Obtain handle.
            OUString aKey = aFullPropName;
            aKey += "/Handle";

            if ( !( xRootHierNameAccess->getByHierarchicalName( aKey )
                    >>= aProp.Handle ) )
            {
                OSL_FAIL( "PropertySetInfo_Impl::getPropertyByName - "
                            "No handle!" );
                return Property();
            }

            // Obtain Value and extract type.
            aKey = aFullPropName;
            aKey += "/Value";

            Any aValue = xRootHierNameAccess->getByHierarchicalName( aKey );
            if ( !aValue.hasValue() )
            {
                OSL_FAIL( "PropertySetInfo_Impl::getPropertyByName - "
                            "No Value!" );
                return Property();
            }

            aProp.Type = aValue.getValueType();

            // Obtain Attributes.
            aKey = aFullPropName;
            aKey += "/Attributes";

            sal_Int32 nAttribs = 0;
            if ( xRootHierNameAccess->getByHierarchicalName( aKey )
                    >>= nAttribs )
                aProp.Attributes = sal_Int16( nAttribs );
            else
            {
                OSL_FAIL( "PropertySetInfo_Impl::getPropertyByName - "
                            "No attributes!" );
                return Property();
            }

            // set name.
            aProp.Name = aName;

            // Success.
            return aProp;
        }
        catch (const NoSuchElementException&)
        {
            // getByHierarchicalName

            OSL_FAIL( "PropertySetInfo_Impl::getPropertyByName - "
                        "caught NoSuchElementException!" );
        }

    }

    OSL_FAIL( "PropertySetInfo_Impl::getPropertyByName - Error!" );
    return Property();
}


// virtual
sal_Bool SAL_CALL PropertySetInfo_Impl::hasPropertyByName(
                                                    const OUString& Name )
{
    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
            m_pOwner->getPropertySetRegistry().getRootConfigReadAccess(),
            UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullPropName( m_pOwner->getFullKey() );
        aFullPropName += "/";
        aFullPropName += makeHierarchalNameSegment( Name );

        return xRootHierNameAccess->hasByHierarchicalName( aFullPropName );
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
