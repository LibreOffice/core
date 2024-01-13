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

#include <optional>
#include <unordered_map>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/propertysequence.hxx>
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
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weak.hxx>
#include <utility>
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

static OUString makeHierarchalNameSegment( std::u16string_view rIn  )
{
    OUStringBuffer aBuffer( "['" );

    size_t nCount = rIn.size();
    for ( size_t n = 0; n < nCount; ++n )
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
    return aBuffer.makeStringAndClear();
}

constexpr OUString STORE_CONTENTPROPERTIES_KEY = u"/org.openoffice.ucb.Store/ContentProperties"_ustr;

// describe path of cfg entry
constexpr OUString CFGPROPERTY_NODEPATH = u"nodepath"_ustr;

class PropertySetInfo_Impl : public cppu::WeakImplHelper < XPropertySetInfo >
{
    std::optional<Sequence< Property >>
                                      m_xProps;
    PersistentPropertySet*            m_pOwner;

public:
    explicit PropertySetInfo_Impl(PersistentPropertySet* pOwner);

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties() override;
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) override;

    // Non-interface methods.
    void reset() { m_xProps.reset(); }
};


// UcbStore Implementation.


UcbStore::UcbStore( const Reference< XComponentContext >& xContext )
: m_xContext( xContext )
{
}


// virtual
UcbStore::~UcbStore()
{
}

OUString SAL_CALL UcbStore::getImplementationName()
{
    return "com.sun.star.comp.ucb.UcbStore";
}
sal_Bool SAL_CALL UcbStore::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}
css::uno::Sequence< OUString > SAL_CALL UcbStore::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.Store" };
}

// Service factory implementation.

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_UcbStore_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new UcbStore(context));
}


// XPropertySetRegistryFactory methods.


// virtual
Reference< XPropertySetRegistry > SAL_CALL
UcbStore::createPropertySetRegistry( const OUString& )
{
    // The URL parameter is ignored by this interface implementation. It always
    // uses the configuration server as storage medium.

    if ( !m_xTheRegistry.is() )
    {
        std::unique_lock aGuard( m_aMutex );
        if ( !m_xTheRegistry.is() )
            m_xTheRegistry = new PropertySetRegistry( m_xContext, m_aInitArgs );
    }

    return m_xTheRegistry;
}


// XInitialization methods.


// virtual
void SAL_CALL UcbStore::initialize( const Sequence< Any >& aArguments )
{
    std::unique_lock aGuard( m_aMutex );
    m_aInitArgs = aArguments;
}



// PropertySetRegistry Implementation.


PropertySetRegistry::PropertySetRegistry(
                        const Reference< XComponentContext >& xContext,
                        const Sequence< Any > &rInitArgs )
: m_xContext( xContext )
, m_aInitArgs(rInitArgs)
, m_bTriedToGetRootReadAccess(false)
, m_bTriedToGetRootWriteAccess(false)
{
}


// virtual
PropertySetRegistry::~PropertySetRegistry()
{
}


// XServiceInfo methods.


OUString SAL_CALL PropertySetRegistry::getImplementationName()
{
    return "com.sun.star.comp.ucb.PropertySetRegistry";
}

sal_Bool SAL_CALL PropertySetRegistry::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL PropertySetRegistry::getSupportedServiceNames()
{
    return {  "com.sun.star.ucb.PropertySetRegistry" };
}


// XPropertySetRegistry methods.


// virtual
Reference< XPersistentPropertySet > SAL_CALL
PropertySetRegistry::openPropertySet( const OUString& key, sal_Bool create )
{
    if ( key.isEmpty() )
        return Reference< XPersistentPropertySet >();

    std::unique_lock aGuard( m_aMutex );

    PropertySetMap_Impl& rSets = m_aPropSets;

    PropertySetMap_Impl::const_iterator it = rSets.find( key );
    if ( it != rSets.end() )
        // Already instantiated.
        return Reference< XPersistentPropertySet >( (*it).second );

    // Create new instance.
    Reference< XNameAccess > xRootNameAccess(
                            getRootConfigReadAccessImpl(aGuard), UNO_QUERY );
    if ( !xRootNameAccess.is() )
    {
        SAL_WARN( "ucb", "no root access" );
        return Reference< XPersistentPropertySet >();
    }

    // Propertyset in registry?
    if ( xRootNameAccess->hasByName( key ) )
    {
        // Yep!
        return Reference< XPersistentPropertySet >(
                                new PersistentPropertySet(
                                        aGuard, *this, key ) );
    }
    else if ( create )
    {
        // No. Create entry for propertyset.

        Reference< XSingleServiceFactory > xFac(
                getConfigWriteAccessImpl( aGuard, OUString() ), UNO_QUERY );
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
                            key, Any( xNameReplace ) );
                    // Commit changes.
                    xBatch->commitChanges();

                    return Reference< XPersistentPropertySet >(
                                new PersistentPropertySet(
                                        aGuard, *this, key ) );
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

    SAL_WARN( "ucb", "no root access" );

    return Reference< XPersistentPropertySet >();
}


// virtual
void SAL_CALL PropertySetRegistry::removePropertySet( const OUString& key )
{
    if ( key.isEmpty() )
        return;

    std::unique_lock aGuard( m_aMutex );

    Reference< XNameAccess > xRootNameAccess(
                                    getRootConfigReadAccessImpl(aGuard), UNO_QUERY );
    if ( xRootNameAccess.is() )
    {
        // Propertyset in registry?
        if ( !xRootNameAccess->hasByName( key ) )
            return;
        Reference< XChangesBatch > xBatch(
                            getConfigWriteAccessImpl( aGuard, OUString() ), UNO_QUERY );
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
    Reference< XNameAccess > xNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        return xNameAccess->hasByName( aName );
    }

    return false;
}


void PropertySetRegistry::add(
        std::unique_lock<std::mutex>& /*rCreatorGuard*/,
        PersistentPropertySet* pSet )
{
    OUString key( pSet->getKey() );

    if ( !key.isEmpty() )
    {
        m_aPropSets[ key ] = pSet;
    }
}


void PropertySetRegistry::remove( PersistentPropertySet* pSet )
{
    OUString key( pSet->getKey() );

    if ( key.isEmpty() )
        return;

    std::unique_lock aGuard( m_aMutex );

    PropertySetMap_Impl& rSets = m_aPropSets;

    PropertySetMap_Impl::iterator it = rSets.find( key );
    if ( it != rSets.end() )
    {
        // Found.
        rSets.erase( it );
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
                                    rNewKey, Any( xNameReplace ) );
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
                        = makeHierarchalNameSegment( rOldKey ) + "/Values";

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
                    const Sequence< OUString > aElems
                                    = xOldNameAccess->getElementNames();
                    if ( aElems.hasElements() )
                    {
                        OUString aNewValuesKey
                            = makeHierarchalNameSegment( rNewKey ) + "/Values";

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

                        for ( const OUString& rPropName : aElems )
                        {
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
                            OUString aKey = aOldValuesKey + makeHierarchalNameSegment( rPropName );

                            // ... handle
                            OUString aNewKey1 = aKey + "/Handle";
                            Any aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName( "Handle", aAny );

                            // ... value
                            aNewKey1 = aKey + "/Value";
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName( "Value", aAny );

                            // ... state
                            aNewKey1 = aKey + "/State";
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName( "State", aAny );

                            // ... attributes
                            aNewKey1 = aKey + "/Attributes";
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName( "Attributes", aAny );

                            // Insert new item.
                            xNewContainer->insertByName(
                                rPropName, Any( xNewPropNameReplace ) );

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


Reference< XMultiServiceFactory > PropertySetRegistry::getConfigProvider(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if ( !m_xConfigProvider.is() )
    {
        const Sequence< Any >& rInitArgs = m_aInitArgs;

        if ( rInitArgs.hasElements() )
        {
            // Extract config provider from service init args.
            rInitArgs[ 0 ] >>= m_xConfigProvider;

            OSL_ENSURE( m_xConfigProvider.is(),
                        "PropertySetRegistry::getConfigProvider - "
                        "No config provider!" );
        }
        else
        {
            try
            {
                m_xConfigProvider = theDefaultProvider::get( m_xContext );
            }
            catch (const Exception&)
            {
                TOOLS_WARN_EXCEPTION( "ucb", "");
            }
        }
    }

    return m_xConfigProvider;
}


Reference< XInterface > PropertySetRegistry::getRootConfigReadAccess()
{
    std::unique_lock aGuard( m_aMutex );
    return getRootConfigReadAccessImpl(aGuard);
}

Reference< XInterface > PropertySetRegistry::getRootConfigReadAccessImpl(std::unique_lock<std::mutex>& rGuard)
{
    try
    {
        if ( !m_xRootReadAccess.is() )
        {
            if ( m_bTriedToGetRootReadAccess )
            {
                OSL_FAIL( "PropertySetRegistry::getRootConfigReadAccess - "
                            "Unable to read any config data! -> #82494#" );
                return Reference< XInterface >();
            }

            getConfigProvider(rGuard);

            if ( m_xConfigProvider.is() )
            {
                Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
                {
                    {CFGPROPERTY_NODEPATH,  Any(STORE_CONTENTPROPERTIES_KEY)}
                }));

                m_bTriedToGetRootReadAccess = true;

                m_xRootReadAccess =
                    m_xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationAccess",
                        aArguments );

                if ( m_xRootReadAccess.is() )
                    return m_xRootReadAccess;
            }
        }
        else
            return m_xRootReadAccess;
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
        // createInstance, createInstanceWithArguments

        TOOLS_WARN_EXCEPTION("ucb", "");
        return Reference< XInterface >();
    }

    SAL_WARN( "ucb", "Error!" );
    return Reference< XInterface >();
}


Reference< XInterface > PropertySetRegistry::getConfigWriteAccess(
                                                    const OUString& rPath )
{
    std::unique_lock aGuard( m_aMutex );
    return getConfigWriteAccessImpl(aGuard, rPath);
}

Reference< XInterface > PropertySetRegistry::getConfigWriteAccessImpl(std::unique_lock<std::mutex>& rGuard,
                                                    const OUString& rPath )
{
    try
    {
        if ( !m_xRootWriteAccess.is() )
        {
            if ( m_bTriedToGetRootWriteAccess )
            {
                OSL_FAIL( "PropertySetRegistry::getConfigWriteAccess - "
                            "Unable to write any config data! -> #82494#" );
                return Reference< XInterface >();
            }

            getConfigProvider(rGuard);

            if ( m_xConfigProvider.is() )
            {
                Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
                {
                    {CFGPROPERTY_NODEPATH,  Any(STORE_CONTENTPROPERTIES_KEY)}
                }));

                m_bTriedToGetRootWriteAccess = true;

                m_xRootWriteAccess =
                    m_xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationUpdateAccess",
                        aArguments );

                OSL_ENSURE( m_xRootWriteAccess.is(),
                            "PropertySetRegistry::getConfigWriteAccess - "
                            "No config update access!" );
            }
        }

        if ( m_xRootWriteAccess.is() )
        {
            if ( !rPath.isEmpty() )
            {
                Reference< XHierarchicalNameAccess > xNA(
                                m_xRootWriteAccess, UNO_QUERY );
                if ( xNA.is() )
                {
                    Reference< XInterface > xInterface;
                    xNA->getByHierarchicalName( rPath ) >>= xInterface;

                    if ( xInterface.is() )
                        return xInterface;
                }
            }
            else
                return m_xRootWriteAccess;
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


// PersistentPropertySet Implementation.


PersistentPropertySet::PersistentPropertySet(
                        std::unique_lock<std::mutex>& rCreatorGuard,
                        PropertySetRegistry& rCreator,
                        OUString aKey )
: m_pCreator( &rCreator ), m_aKey(std::move( aKey ))
{
    // register at creator.
    rCreator.add( rCreatorGuard, this );
}


// virtual
PersistentPropertySet::~PersistentPropertySet()
{
    // deregister at creator.
    m_pCreator->remove( this );
}

// XServiceInfo methods.

OUString SAL_CALL PersistentPropertySet::getImplementationName()
{
    return "com.sun.star.comp.ucb.PersistentPropertySet";
}

sal_Bool SAL_CALL PersistentPropertySet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL PersistentPropertySet::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.PersistentPropertySet" };
}


// XComponent methods.


// virtual
void SAL_CALL PersistentPropertySet::dispose()
{
    std::unique_lock l(m_aMutex);
    if ( m_aDisposeEventListeners.getLength(l) )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this  );
        m_aDisposeEventListeners.disposeAndClear( l, aEvt );
    }

    if ( m_aPropSetChangeListeners.getLength(l) )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySetInfoChangeNotifier * >( this  );
        m_aPropSetChangeListeners.disposeAndClear( l, aEvt );
    }

    if ( m_aPropertyChangeListeners.hasContainedTypes(l) )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this  );
        m_aPropertyChangeListeners.disposeAndClear( l, aEvt );
    }
}


// virtual
void SAL_CALL PersistentPropertySet::addEventListener(
                            const Reference< XEventListener >& Listener )
{
    std::unique_lock l(m_aMutex);

    m_aDisposeEventListeners.addInterface( l, Listener );
}


// virtual
void SAL_CALL PersistentPropertySet::removeEventListener(
                            const Reference< XEventListener >& Listener )
{
    std::unique_lock l(m_aMutex);
    m_aDisposeEventListeners.removeInterface( l, Listener );

    // Note: Don't want to delete empty container here -> performance.
}


// XPropertySet methods.


// virtual
Reference< XPropertySetInfo > SAL_CALL PersistentPropertySet::getPropertySetInfo()
{
    std::unique_lock l(m_aMutex);

    if ( !m_pInfo.is() )
    {
        m_pInfo = new PropertySetInfo_Impl( this );
    }
    return m_pInfo;
}


// virtual
void SAL_CALL PersistentPropertySet::setPropertyValue( const OUString& aPropertyName,
                                                       const Any& aValue )
{
    std::unique_lock aCGuard(m_aMutex);

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullPropName( getFullKeyImpl(aCGuard) + "/" +
            makeHierarchalNameSegment( aPropertyName ) );

        // Does property exist?
        if ( xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
        {
            Reference< XNameReplace > xNameReplace(
                    m_pCreator->getConfigWriteAccess(
                                            aFullPropName ), UNO_QUERY );
            Reference< XChangesBatch > xBatch(
                    m_pCreator->getConfigWriteAccess(
                                            OUString() ), UNO_QUERY );

            if ( xNameReplace.is() && xBatch.is() )
            {
                try
                {
                    // Obtain old value
                    OUString aValueName = aFullPropName + "/Value";
                    Any aOldValue
                        = xRootHierNameAccess->getByHierarchicalName(
                                                                aValueName );
                    // Check value type.
                    if ( aOldValue.getValueType() != aValue.getValueType() )
                    {
                        throw IllegalArgumentException();
                    }

                    // Write value
                    xNameReplace->replaceByName( "Value", aValue );

                    // Write state ( Now it is a directly set value )
                    xNameReplace->replaceByName(
                                    "State",
                                    Any(
                                        sal_Int32(
                                            PropertyState_DIRECT_VALUE ) ) );

                    // Commit changes.
                    xBatch->commitChanges();

                    PropertyChangeEvent aEvt;
                    if ( m_aPropertyChangeListeners.hasContainedTypes(aCGuard) )
                    {
                        // Obtain handle
                        aValueName = aFullPropName + "/Handle";
                        sal_Int32 nHandle = -1;
                        xRootHierNameAccess->getByHierarchicalName( aValueName )
                            >>= nHandle;

                        aEvt.Source         = getXWeak();
                        aEvt.PropertyName   = aPropertyName;
                        aEvt.PropertyHandle = nHandle;
                        aEvt.Further        = false;
                        aEvt.OldValue       = aOldValue;
                        aEvt.NewValue       = aValue;

                        notifyPropertyChangeEvent( aCGuard, aEvt );
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

    throw UnknownPropertyException(aPropertyName);
}


// virtual
Any SAL_CALL PersistentPropertySet::getPropertyValue(
                                            const OUString& PropertyName )
{
    std::unique_lock aGuard(m_aMutex);

    Reference< XHierarchicalNameAccess > xNameAccess(
                m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        OUString aFullPropName( getFullKeyImpl(aGuard) + "/" +
            makeHierarchalNameSegment( PropertyName ) + "/Value" );
        try
        {
            return xNameAccess->getByHierarchicalName( aFullPropName );
        }
        catch (const NoSuchElementException&)
        {
            throw UnknownPropertyException(aFullPropName);
        }
    }

    throw UnknownPropertyException(PropertyName);
}


// virtual
void SAL_CALL PersistentPropertySet::addPropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& xListener )
{
//  load();

    std::unique_lock aGuard(m_aMutex);

    m_aPropertyChangeListeners.addInterface(aGuard, aPropertyName, xListener );
}


// virtual
void SAL_CALL PersistentPropertySet::removePropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& aListener )
{
//  load();

    std::unique_lock aGuard(m_aMutex);

    m_aPropertyChangeListeners.removeInterface(aGuard,
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
    return m_pCreator;
}


// virtual
OUString SAL_CALL PersistentPropertySet::getKey()
{
    return m_aKey;
}


// XNamed methods.


// virtual
OUString SAL_CALL PersistentPropertySet::getName()
{
    // same as getKey()
    return m_aKey;
}


// virtual
void SAL_CALL PersistentPropertySet::setName( const OUString& aName )
{
    if ( aName != m_aKey )
        m_pCreator->renamePropertySet( m_aKey, aName );
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

    std::unique_lock aGuard(m_aMutex);

    // Property already in set?

    OUString aFullValuesName;

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        aFullValuesName = getFullKeyImpl(aGuard);
        OUString aFullPropName = aFullValuesName + "/" +
            makeHierarchalNameSegment( Name );

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
                m_pCreator->getConfigWriteAccess( aFullValuesName ),
                UNO_QUERY );
    Reference< XNameContainer > xContainer( xFac, UNO_QUERY );
    Reference< XChangesBatch >  xBatch(
                m_pCreator->getConfigWriteAccess( OUString() ),
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
                                    Any( sal_Int32( -1 ) ) );

                // Set default value
                xNameReplace->replaceByName(
                                    "Value",
                                    DefaultValue );

                // Set state ( always "default" )
                xNameReplace->replaceByName(
                                    "State",
                                    Any(
                                        sal_Int32(
                                            PropertyState_DEFAULT_VALUE ) ) );

                // Set attributes
                xNameReplace->replaceByName(
                                    "Attributes",
                                    Any( sal_Int32( Attributes ) ) );

                // Insert new item.
                xContainer->insertByName( Name, Any( xNameReplace ) );

                // Commit changes.
                xBatch->commitChanges();

                // Property set info is invalid.
                if ( m_pInfo.is() )
                    m_pInfo->reset();

                // Notify propertyset info change listeners.
                if ( m_aPropSetChangeListeners.getLength(aGuard) )
                {
                    PropertySetInfoChangeEvent evt(
                                    getXWeak(),
                                    Name,
                                    -1,
                                    PropertySetInfoChange::PROPERTY_INSERTED );
                    notifyPropertySetInfoChange(aGuard, evt);
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
    std::unique_lock aGuard(m_aMutex);

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullValuesName = getFullKeyImpl(aGuard);
        OUString aFullPropName   = aFullValuesName + "/" +
            makeHierarchalNameSegment( Name );

        // Property in set?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            throw UnknownPropertyException(aFullPropName);

        // Property removable?
        try
        {
            OUString aFullAttrName = aFullPropName + "/Attributes";

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
                m_pCreator->getConfigWriteAccess( aFullValuesName ),
                UNO_QUERY );
        Reference< XChangesBatch > xBatch(
                m_pCreator->getConfigWriteAccess( OUString() ),
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

                if ( m_aPropSetChangeListeners.getLength(aGuard) )
                {
                    // Obtain property handle ( needed for propertysetinfo
                    // change event )...

                    try
                    {
                        OUString aFullHandleName = aFullPropName + "/Handle";

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
                if ( m_pInfo.is() )
                    m_pInfo->reset();

                // Notify propertyset info change listeners.
                if (  m_aPropSetChangeListeners.getLength(aGuard) )
                {
                    PropertySetInfoChangeEvent evt(
                                    getXWeak(),
                                    Name,
                                    nHandle,
                                    PropertySetInfoChange::PROPERTY_REMOVED );
                    notifyPropertySetInfoChange( aGuard, evt );
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
    std::unique_lock aGuard(m_aMutex);

    m_aPropSetChangeListeners.addInterface( aGuard, Listener );
}


// virtual
void SAL_CALL PersistentPropertySet::removePropertySetInfoChangeListener(
                const Reference< XPropertySetInfoChangeListener >& Listener )
{
    std::unique_lock aGuard(m_aMutex);
    m_aPropSetChangeListeners.removeInterface( aGuard, Listener );
}


// XPropertyAccess methods.


// virtual
Sequence< PropertyValue > SAL_CALL PersistentPropertySet::getPropertyValues()
{
    std::unique_lock aGuard(m_aMutex);

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        try
        {
            Reference< XNameAccess > xNameAccess;
            xRootHierNameAccess->getByHierarchicalName(getFullKeyImpl(aGuard))
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
                        auto pValues = aValues.getArray();

                        static constexpr OUStringLiteral aHandleName(u"/Handle");
                        static constexpr OUStringLiteral aValueName(u"/Value");
                        static constexpr OUStringLiteral aStateName(u"/State");

                        for ( sal_Int32 n = 0; n < nCount; ++n )
                        {
                            PropertyValue& rValue = pValues[ n ];
                            OUString rName    = aElems[ n ];
                            OUString aXMLName
                                        = makeHierarchalNameSegment( rName );

                            // Set property name.

                            rValue.Name = rName;

                            try
                            {
                                // Obtain and set property handle
                                OUString aHierName = aXMLName + aHandleName;
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
                                OUString aHierName = aXMLName + aValueName;
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
                                OUString aHierName = aXMLName +aStateName;
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
    if ( !aProps.hasElements() )
        return;

    std::unique_lock aCGuard(m_aMutex);

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        std::vector< PropertyChangeEvent > aEvents;

        OUString aFullPropNamePrefix( getFullKeyImpl(aCGuard) + "/" );

        // Iterate over given property value sequence.
        for ( const PropertyValue& rNewValue : aProps )
        {
            const OUString& rName = rNewValue.Name;

            OUString aFullPropName = aFullPropNamePrefix +
                makeHierarchalNameSegment( rName );

            // Does property exist?
            if ( xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            {
                Reference< XNameReplace > xNameReplace(
                    m_pCreator->getConfigWriteAccess(
                                            aFullPropName ), UNO_QUERY );
                Reference< XChangesBatch > xBatch(
                    m_pCreator->getConfigWriteAccess(
                                            OUString() ), UNO_QUERY );

                if ( xNameReplace.is() && xBatch.is() )
                {
                    try
                    {
                        // Write handle
                        xNameReplace->replaceByName(
                                    "Handle",
                                    Any( rNewValue.Handle ) );

                        // Save old value
                        OUString aValueName = aFullPropName +"/Value";
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
                                    Any(
                                        sal_Int32(
                                            PropertyState_DIRECT_VALUE ) ) );

                        // Commit changes.
                        xBatch->commitChanges();

                        if ( m_aPropertyChangeListeners.hasContainedTypes(aCGuard) )
                        {
                            PropertyChangeEvent aEvt;
                            aEvt.Source         = getXWeak();
                            aEvt.PropertyName   = rNewValue.Name;
                            aEvt.PropertyHandle = rNewValue.Handle;
                            aEvt.Further        = false;
                            aEvt.OldValue       = std::move(aOldValue);
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

        if ( m_aPropertyChangeListeners.hasContainedTypes(aCGuard) )
        {
            // Notify property changes.
            for (auto const& event : aEvents)
            {
                notifyPropertyChangeEvent( aCGuard, event );
            }
        }

        return;
    }

    OSL_FAIL( "PersistentPropertySet::setPropertyValues - Nothing set!" );
}


// Non-interface methods


void PersistentPropertySet::notifyPropertyChangeEvent(
                                    std::unique_lock<std::mutex>& rGuard,
                                    const PropertyChangeEvent& rEvent ) const
{
    // Get "normal" listeners for the property.
    OInterfaceContainerHelper4<XPropertyChangeListener>* pContainer =
            m_aPropertyChangeListeners.getContainer( rGuard, rEvent.PropertyName );
    if ( pContainer && pContainer->getLength(rGuard) )
    {
        pContainer->notifyEach( rGuard, &XPropertyChangeListener::propertyChange, rEvent );
    }

    // Get "normal" listeners for all properties.
    OInterfaceContainerHelper4<XPropertyChangeListener>* pNoNameContainer =
            m_aPropertyChangeListeners.getContainer( rGuard, OUString() );
    if ( pNoNameContainer && pNoNameContainer->getLength(rGuard) )
    {
        pNoNameContainer->notifyEach( rGuard, &XPropertyChangeListener::propertyChange, rEvent );
    }
}


void PersistentPropertySet::notifyPropertySetInfoChange(
                                std::unique_lock<std::mutex>& rGuard,
                                const PropertySetInfoChangeEvent& evt ) const
{
    // Notify event listeners.
    m_aPropSetChangeListeners.notifyEach( rGuard, &XPropertySetInfoChangeListener::propertySetInfoChange, evt );
}


OUString PersistentPropertySet::getFullKey()
{
    std::unique_lock aGuard(m_aMutex);
    return getFullKeyImpl(aGuard);
}

const OUString& PersistentPropertySet::getFullKeyImpl(std::unique_lock<std::mutex>& )
{
    if ( m_aFullKey.isEmpty() )
    {
        m_aFullKey = makeHierarchalNameSegment( m_aKey );
        m_aFullKey += "/Values";
    }

    return m_aFullKey;
}


PropertySetRegistry& PersistentPropertySet::getPropertySetRegistry()
{
    return *m_pCreator;
}


// PropertySetInfo_Impl Implementation.


PropertySetInfo_Impl::PropertySetInfo_Impl(
                        PersistentPropertySet* pOwner )
: m_pOwner( pOwner )
{
}


// XPropertySetInfo methods.


// virtual
Sequence< Property > SAL_CALL PropertySetInfo_Impl::getProperties()
{
    if ( !m_xProps )
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
                    Sequence< Property > aPropSeq( nCount );

                    if ( nCount )
                    {
                        Reference< XHierarchicalNameAccess > xHierNameAccess(
                                                    xNameAccess, UNO_QUERY );

                        OSL_ENSURE( xHierNameAccess.is(),
                                    "PropertySetInfo_Impl::getProperties - "
                                    "No hierarchical name access!" );

                        if ( xHierNameAccess.is() )
                        {
                            static constexpr OUStringLiteral aHandleName(u"/Handle");
                            static constexpr OUStringLiteral aValueName(u"/Value");
                            static constexpr OUStringLiteral aAttrName(u"/Attributes");

                            Property* pProps = aPropSeq.getArray();

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
                                    OUString aHierName = aXMLName + aHandleName;
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
                                    OUString aHierName = aXMLName + aValueName;
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
                                    OUString aHierName = aXMLName + aAttrName;
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
                    m_xProps = std::move(aPropSeq);
                    return *m_xProps;
                }
            }
            catch (const NoSuchElementException&)
            {
                // getByHierarchicalName
            }
        }

        OSL_FAIL( "PropertySetInfo_Impl::getProperties - Error!" );
        m_xProps.emplace();
    }

    return *m_xProps;
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
        OUString aFullPropName( m_pOwner->getFullKey() + "/" +
            makeHierarchalNameSegment( aName ) );

        // Does property exist?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            throw UnknownPropertyException(aFullPropName);

        try
        {
            Property aProp;

            // Obtain handle.
            OUString aKey = aFullPropName + "/Handle";

            if ( !( xRootHierNameAccess->getByHierarchicalName( aKey )
                    >>= aProp.Handle ) )
            {
                OSL_FAIL( "PropertySetInfo_Impl::getPropertyByName - "
                            "No handle!" );
                return Property();
            }

            // Obtain Value and extract type.
            aKey = aFullPropName + "/Value";

            Any aValue = xRootHierNameAccess->getByHierarchicalName( aKey );
            if ( !aValue.hasValue() )
            {
                OSL_FAIL( "PropertySetInfo_Impl::getPropertyByName - "
                            "No Value!" );
                return Property();
            }

            aProp.Type = aValue.getValueType();

            // Obtain Attributes.
            aKey = aFullPropName + "/Attributes";

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
        OUString aFullPropName( m_pOwner->getFullKey() + "/" +
            makeHierarchalNameSegment( Name ) );

        return xRootHierNameAccess->hasByHierarchicalName( aFullPropName );
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
