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
#include <boost/unordered_map.hpp>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/processfactory.hxx>
#include "ucbstore.hxx"

using namespace com::sun::star::beans;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

//=========================================================================
OUString makeHierarchalNameSegment( const OUString & rIn  )
{
    OUStringBuffer aBuffer;
    aBuffer.appendAscii( "['" );

    sal_Int32 nCount = rIn.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const sal_Unicode c = rIn.getStr()[ n ];
        switch ( c )
        {
            case '&':
                aBuffer.appendAscii( "&amp;" );
                break;

            case '"':
                aBuffer.appendAscii( "&quot;" );
                break;

            case '\'':
                aBuffer.appendAscii( "&apos;" );
                break;

            case '<':
                aBuffer.appendAscii( "&lt;" );
                break;

            case '>':
                aBuffer.appendAscii( "&gt;" );
                break;

            default:
                aBuffer.append( c );
                break;
        }
    }

    aBuffer.appendAscii( "']" );
    return OUString( aBuffer.makeStringAndClear() );
}

//=========================================================================

#define STORE_CONTENTPROPERTIES_KEY "/org.openoffice.ucb.Store/ContentProperties"

// describe path of cfg entry
#define CFGPROPERTY_NODEPATH        "nodepath"
// true->async. update; false->sync. update
#define CFGPROPERTY_LAZYWRITE       "lazywrite"

//=========================================================================

struct equalString_Impl
{
  bool operator()( const OUString& s1, const OUString& s2 ) const
  {
        return !!( s1 == s2 );
  }
};

struct hashString_Impl
{
    size_t operator()( const OUString & rName ) const
    {
        return rName.hashCode();
    }
};

//=========================================================================
//
// PropertySetMap_Impl.
//
//=========================================================================

typedef boost::unordered_map
<
    OUString,
    PersistentPropertySet*,
    hashString_Impl,
    equalString_Impl
>
PropertySetMap_Impl;

//=========================================================================
//
// class PropertySetInfo_Impl
//
//=========================================================================

class PropertySetInfo_Impl :
        public OWeakObject, public XTypeProvider, public XPropertySetInfo
{
    Reference< XMultiServiceFactory > m_xSMgr;
    Sequence< Property >*             m_pProps;
    PersistentPropertySet*            m_pOwner;

public:
    PropertySetInfo_Impl( const Reference< XMultiServiceFactory >& rxSMgr,
                          PersistentPropertySet* pOwner );
    virtual ~PropertySetInfo_Impl();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties()
        throw( RuntimeException );
    virtual Property SAL_CALL getPropertyByName( const OUString& aName )
        throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name )
        throw( RuntimeException );

    // Non-interface methods.
    void reset() { delete m_pProps; m_pProps = 0; }
};

//=========================================================================
//
// UcbStore_Impl.
//
//=========================================================================

struct UcbStore_Impl
{
    osl::Mutex                        m_aMutex;
    Sequence< Any >                   m_aInitArgs;
    Reference< XPropertySetRegistry > m_xTheRegistry;
};

//=========================================================================
//=========================================================================
//=========================================================================
//
// UcbStore Implementation.
//
//=========================================================================
//=========================================================================
//=========================================================================

UcbStore::UcbStore( const Reference< XMultiServiceFactory >& rXSMgr )
: m_xSMgr( rXSMgr ),
  m_pImpl( new UcbStore_Impl() )
{
}

//=========================================================================
// virtual
UcbStore::~UcbStore()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_4( UcbStore,
                   XTypeProvider,
                   XServiceInfo,
                   XPropertySetRegistryFactory,
                   XInitialization );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_4( UcbStore,
                         XTypeProvider,
                         XServiceInfo,
                      XPropertySetRegistryFactory,
                      XInitialization );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( UcbStore,
                     OUString( "com.sun.star.comp.ucb.UcbStore" ),
                     OUString( STORE_SERVICE_NAME ) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( UcbStore );

//=========================================================================
//
// XPropertySetRegistryFactory methods.
//
//=========================================================================

// virtual
Reference< XPropertySetRegistry > SAL_CALL
UcbStore::createPropertySetRegistry( const OUString& )
    throw( RuntimeException )
{
    // The URL parameter is ignored by this interface implementation. It always
    // uses the configuration server as storage medium.

    if ( !m_pImpl->m_xTheRegistry.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
        if ( !m_pImpl->m_xTheRegistry.is() )
            m_pImpl->m_xTheRegistry = new PropertySetRegistry( m_xSMgr, getInitArgs() );
    }

    return m_pImpl->m_xTheRegistry;
}

//=========================================================================
//
// XInitialization methods.
//
//=========================================================================

// virtual
void SAL_CALL UcbStore::initialize( const Sequence< Any >& aArguments )
    throw( Exception, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
    m_pImpl->m_aInitArgs = aArguments;
}

//=========================================================================
const Sequence< Any >& UcbStore::getInitArgs() const
{
    return m_pImpl->m_aInitArgs;
}

//=========================================================================
//
// PropertySetRegistry_Impl.
//
//=========================================================================

struct PropertySetRegistry_Impl
{
    const Sequence< Any >             m_aInitArgs;
    PropertySetMap_Impl               m_aPropSets;
    Reference< XMultiServiceFactory > m_xConfigProvider;
    Reference< XInterface >           m_xRootReadAccess;
    Reference< XInterface >           m_xRootWriteAccess;
    osl::Mutex                        m_aMutex;
    sal_Bool                          m_bTriedToGetRootReadAccess;  // #82494#
    sal_Bool                          m_bTriedToGetRootWriteAccess; // #82494#

    PropertySetRegistry_Impl( const Sequence< Any > &rInitArgs )
    : m_aInitArgs( rInitArgs ),
      m_bTriedToGetRootReadAccess( sal_False ),
      m_bTriedToGetRootWriteAccess( sal_False )
    {
    }
};

//=========================================================================
//=========================================================================
//=========================================================================
//
// PropertySetRegistry Implementation.
//
//=========================================================================
//=========================================================================
//=========================================================================

PropertySetRegistry::PropertySetRegistry(
                        const Reference< XMultiServiceFactory >& rXSMgr,
                        const Sequence< Any > &rInitArgs )
: m_xSMgr( rXSMgr ),
  m_pImpl( new PropertySetRegistry_Impl( rInitArgs ) )
{
}

//=========================================================================
// virtual
PropertySetRegistry::~PropertySetRegistry()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_5( PropertySetRegistry,
                   XTypeProvider,
                   XServiceInfo,
                   XPropertySetRegistry,
                   XElementAccess, /* base of XNameAccess */
                   XNameAccess );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_4( PropertySetRegistry,
                         XTypeProvider,
                      XServiceInfo,
                      XPropertySetRegistry,
                      XNameAccess );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_NOFACTORY_IMPL_1( PropertySetRegistry,
                                OUString( "com.sun.star.comp.ucb.PropertySetRegistry" ),
                                OUString( PROPSET_REG_SERVICE_NAME ) );

//=========================================================================
//
// XPropertySetRegistry methods.
//
//=========================================================================

// virtual
Reference< XPersistentPropertySet > SAL_CALL
PropertySetRegistry::openPropertySet( const OUString& key, sal_Bool create )
    throw( RuntimeException )
{
    if ( !key.isEmpty() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        PropertySetMap_Impl& rSets = m_pImpl->m_aPropSets;

        PropertySetMap_Impl::const_iterator it = rSets.find( key );
        if ( it != rSets.end() )
        {
            // Already instanciated.
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
                                                    m_xSMgr, *this, key ) );
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
                                "No conteiner!" );

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

//                              // Set Values
//                              xNameReplace->replaceByName(
//                                      OUString("Values"),
//                                      makeAny( ... ) );

                                // Insert new item.
                                xContainer->insertByName(
                                        key, makeAny( xNameReplace ) );
                                // Commit changes.
                                xBatch->commitChanges();

                                return Reference< XPersistentPropertySet >(
                                            new PersistentPropertySet(
                                                    m_xSMgr, *this, key ) );
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

            OSL_TRACE( "PropertySetRegistry::openPropertySet no root access" );
        }
    }

    return Reference< XPersistentPropertySet >();
}

//=========================================================================
// virtual
void SAL_CALL PropertySetRegistry::removePropertySet( const OUString& key )
    throw( RuntimeException )
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

    OSL_TRACE( "PropertySetRegistry::removePropertySet - no root access" );
}

//=========================================================================
//
// XElementAccess methods.
//
//=========================================================================

// virtual
com::sun::star::uno::Type SAL_CALL PropertySetRegistry::getElementType()
    throw( RuntimeException )
{
    return getCppuType( ( Reference< XPersistentPropertySet > * ) 0 );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertySetRegistry::hasElements()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XElementAccess > xElemAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xElemAccess.is() )
        return xElemAccess->hasElements();

    return sal_False;
}

//=========================================================================
//
// XNameAccess methods.
//
//=========================================================================

// virtual
Any SAL_CALL PropertySetRegistry::getByName( const OUString& aName )
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
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

//=========================================================================
// virtual
Sequence< OUString > SAL_CALL PropertySetRegistry::getElementNames()
    throw( RuntimeException )
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

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertySetRegistry::hasByName( const OUString& aName )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XNameAccess > xNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        return xNameAccess->hasByName( aName );
    }

    return sal_False;
}

//=========================================================================
void PropertySetRegistry::add( PersistentPropertySet* pSet )
{
    OUString key( pSet->getKey() );

    if ( !key.isEmpty() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
        m_pImpl->m_aPropSets[ key ] = pSet;
    }
}

//=========================================================================
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

//=========================================================================
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
                //////////////////////////////////////////////////////
                // Create new "Properties" config item.
                //////////////////////////////////////////////////////

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

                //////////////////////////////////////////////////////
                // Copy data...
                //////////////////////////////////////////////////////

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
                    aOldValuesKey += OUString("/Values");

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
                        aNewValuesKey += OUString("/Values");

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

                        aOldValuesKey += OUString("/");

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
                            xNewPropNameReplace->replaceByName(
                                OUString("Handle"),
                                aAny );

                            // ... value
                            aNewKey1 = aKey;
                            aNewKey1 += aValueKey;
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName(
                                OUString("Value"),
                                aAny );

                            // ... state
                            aNewKey1 = aKey;
                            aNewKey1 += aStateKey;
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName(
                                OUString("State"),
                                aAny );

                            // ... attributes
                            aNewKey1 = aKey;
                            aNewKey1 += aAttrKey;
                            aAny =
                                xRootHierNameAccess->getByHierarchicalName(
                                    aNewKey1 );
                            xNewPropNameReplace->replaceByName(
                                OUString("Attributes"),
                                aAny );

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

                //////////////////////////////////////////////////////
                // Remove old entry...
                //////////////////////////////////////////////////////

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

//=========================================================================
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
                    m_pImpl->m_xConfigProvider
                        = theDefaultProvider::get(
                            comphelper::getComponentContext( m_xSMgr ) );
                }
                catch (const Exception&)
                {
                    OSL_TRACE( "PropertySetRegistry::getConfigProvider - "
                                "caught exception!" );
                }
            }
        }
    }

    return m_pImpl->m_xConfigProvider;
}

//=========================================================================
Reference< XInterface > PropertySetRegistry::getRootConfigReadAccess()
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( !m_pImpl->m_xRootReadAccess.is() )
        {
            if ( m_pImpl->m_bTriedToGetRootReadAccess ) // #82494#
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
                aProperty.Name
                    = OUString( CFGPROPERTY_NODEPATH  );
                aProperty.Value
                    <<= OUString( STORE_CONTENTPROPERTIES_KEY  );
                aArguments[ 0 ] <<= aProperty;

                m_pImpl->m_bTriedToGetRootReadAccess = sal_True;

                m_pImpl->m_xRootReadAccess =
                    m_pImpl->m_xConfigProvider->createInstanceWithArguments(
                        OUString( "com.sun.star.configuration.ConfigurationAccess" ),
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

    OSL_TRACE( "PropertySetRegistry::getRootConfigReadAccess - Error!" );
    return Reference< XInterface >();
}

//=========================================================================
Reference< XInterface > PropertySetRegistry::getConfigWriteAccess(
                                                    const OUString& rPath )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( !m_pImpl->m_xRootWriteAccess.is() )
        {
            if ( m_pImpl->m_bTriedToGetRootWriteAccess ) // #82494#
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

                aProperty.Name = OUString( CFGPROPERTY_NODEPATH  );
                aProperty.Value <<= OUString( STORE_CONTENTPROPERTIES_KEY  );
                aArguments[ 0 ] <<= aProperty;

                aProperty.Name = OUString( CFGPROPERTY_LAZYWRITE  );
                aProperty.Value <<= sal_True;
                aArguments[ 1 ] <<= aProperty;

                m_pImpl->m_bTriedToGetRootWriteAccess = sal_True;

                m_pImpl->m_xRootWriteAccess =
                    m_pImpl->m_xConfigProvider->createInstanceWithArguments(
                        OUString( "com.sun.star.configuration.ConfigurationUpdateAccess" ),
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

//=========================================================================
//
// PropertyListeners_Impl.
//
//=========================================================================

typedef OMultiTypeInterfaceContainerHelperVar
<
    OUString,
    hashString_Impl,
    equalString_Impl
> PropertyListeners_Impl;

//=========================================================================
//
// PersistentPropertySet_Impl.
//
//=========================================================================

struct PersistentPropertySet_Impl
{
    PropertySetRegistry*        m_pCreator;
    PropertySetInfo_Impl*       m_pInfo;
    OUString                    m_aKey;
    OUString                    m_aFullKey;
    osl::Mutex                  m_aMutex;
    OInterfaceContainerHelper*  m_pDisposeEventListeners;
    OInterfaceContainerHelper*  m_pPropSetChangeListeners;
    PropertyListeners_Impl*     m_pPropertyChangeListeners;

    PersistentPropertySet_Impl( PropertySetRegistry& rCreator,
                                const OUString& rKey )
    : m_pCreator( &rCreator ), m_pInfo( NULL ), m_aKey( rKey ),
      m_pDisposeEventListeners( NULL ), m_pPropSetChangeListeners( NULL ),
      m_pPropertyChangeListeners( NULL )
    {
        m_pCreator->acquire();
    }

    ~PersistentPropertySet_Impl()
    {
        m_pCreator->release();

        if ( m_pInfo )
            m_pInfo->release();

        delete m_pDisposeEventListeners;
        delete m_pPropSetChangeListeners;
        delete m_pPropertyChangeListeners;
    }
};

//=========================================================================
//=========================================================================
//=========================================================================
//
// PersistentPropertySet Implementation.
//
//=========================================================================
//=========================================================================
//=========================================================================

PersistentPropertySet::PersistentPropertySet(
                        const Reference< XMultiServiceFactory >& rXSMgr,
                        PropertySetRegistry& rCreator,
                        const OUString& rKey )
: m_xSMgr( rXSMgr ),
  m_pImpl( new PersistentPropertySet_Impl( rCreator, rKey ) )
{
    // register at creator.
    rCreator.add( this );
}

//=========================================================================
// virtual
PersistentPropertySet::~PersistentPropertySet()
{
    // deregister at creator.
    m_pImpl->m_pCreator->remove( this );

    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_9( PersistentPropertySet,
                   XTypeProvider,
                   XServiceInfo,
                   XComponent,
                   XPropertySet, /* base of XPersistentPropertySet */
                   XNamed,
                   XPersistentPropertySet,
                   XPropertyContainer,
                   XPropertySetInfoChangeNotifier,
                   XPropertyAccess );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_8( PersistentPropertySet,
                         XTypeProvider,
                         XServiceInfo,
                      XComponent,
                      XPersistentPropertySet,
                      XNamed,
                      XPropertyContainer,
                      XPropertySetInfoChangeNotifier,
                      XPropertyAccess );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_NOFACTORY_IMPL_1( PersistentPropertySet,
                                OUString( "com.sun.star.comp.ucb.PersistentPropertySet" ),
                                OUString( PERS_PROPSET_SERVICE_NAME ) );

//=========================================================================
//
// XComponent methods.
//
//=========================================================================

// virtual
void SAL_CALL PersistentPropertySet::dispose()
    throw( RuntimeException )
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

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::addEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    if ( !m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners =
                    new OInterfaceContainerHelper( m_pImpl->m_aMutex );

    m_pImpl->m_pDisposeEventListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    if ( m_pImpl->m_pDisposeEventListeners )
        m_pImpl->m_pDisposeEventListeners->removeInterface( Listener );

    // Note: Don't want to delete empty container here -> performance.
}

//=========================================================================
//
// XPropertySet methods.
//
//=========================================================================

// virtual
Reference< XPropertySetInfo > SAL_CALL
                                PersistentPropertySet::getPropertySetInfo()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    PropertySetInfo_Impl*& rpInfo = m_pImpl->m_pInfo;
    if ( !rpInfo )
    {
        rpInfo = new PropertySetInfo_Impl( m_xSMgr, this );
        rpInfo->acquire();
    }
    return Reference< XPropertySetInfo >( rpInfo );
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::setPropertyValue(
                        const OUString& aPropertyName, const Any& aValue )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
{
    if ( aPropertyName.isEmpty() )
        throw UnknownPropertyException();

    osl::ClearableGuard< osl::Mutex > aCGuard( m_pImpl->m_aMutex );

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullPropName( getFullKey() );
        aFullPropName += OUString("/");
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
                    aValueName += OUString("/Value");
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
                    xNameReplace->replaceByName(
                                    OUString("Value"),
                                    aValue );

                    // Write state ( Now it is a directly set value )
                    xNameReplace->replaceByName(
                                    OUString("State"),
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
                        aValueName += OUString("/Handle");
                        sal_Int32 nHandle = -1;
                        xRootHierNameAccess->getByHierarchicalName( aValueName )
                            >>= nHandle;

                        aEvt.Source         = (OWeakObject*)this;
                        aEvt.PropertyName   = aPropertyName;
                        aEvt.PropertyHandle = nHandle;
                        aEvt.Further        = sal_False;
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

//=========================================================================
// virtual
Any SAL_CALL PersistentPropertySet::getPropertyValue(
                                            const OUString& PropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    if ( PropertyName.isEmpty() )
        throw UnknownPropertyException();

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XHierarchicalNameAccess > xNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        OUString aFullPropName( getFullKey() );
        aFullPropName += OUString("/");
        aFullPropName += makeHierarchalNameSegment( PropertyName );
        aFullPropName += OUString("/Value");
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

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::addPropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
//  load();

    if ( !m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners =
                    new PropertyListeners_Impl( m_pImpl->m_aMutex );

    m_pImpl->m_pPropertyChangeListeners->addInterface(
                                                aPropertyName, xListener );
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::removePropertyChangeListener(
                    const OUString& aPropertyName,
                    const Reference< XPropertyChangeListener >& aListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
//  load();

    if ( m_pImpl->m_pPropertyChangeListeners )
        m_pImpl->m_pPropertyChangeListeners->removeInterface(
                                                aPropertyName, aListener );

    // Note: Don't want to delete empty container here -> performance.
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::addVetoableChangeListener(
                    const OUString&,
                    const Reference< XVetoableChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
//  load();
//  OSL_FAIL( //                "PersistentPropertySet::addVetoableChangeListener - N.Y.I." );
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::removeVetoableChangeListener(
                    const OUString&,
                    const Reference< XVetoableChangeListener >& )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
//  load();
//  OSL_FAIL( //                "PersistentPropertySet::removeVetoableChangeListener - N.Y.I." );
}

//=========================================================================
//
// XPersistentPropertySet methods.
//
//=========================================================================

// virtual
Reference< XPropertySetRegistry > SAL_CALL PersistentPropertySet::getRegistry()
    throw( RuntimeException )
{
    return Reference< XPropertySetRegistry >( m_pImpl->m_pCreator );
}

//=========================================================================
// virtual
OUString SAL_CALL PersistentPropertySet::getKey()
    throw( RuntimeException )
{
    return m_pImpl->m_aKey;
}

//=========================================================================
//
// XNamed methods.
//
//=========================================================================

// virtual
OUString SAL_CALL PersistentPropertySet::getName()
    throw( RuntimeException )
{
    // same as getKey()
    return m_pImpl->m_aKey;
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::setName( const OUString& aName )
    throw( RuntimeException )
{
    if ( aName != m_pImpl->m_aKey )
        m_pImpl->m_pCreator->renamePropertySet( m_pImpl->m_aKey, aName );
}

//=========================================================================
//
// XPropertyContainer methods.
//
//=========================================================================

// virtual
void SAL_CALL PersistentPropertySet::addProperty(
        const OUString& Name, sal_Int16 Attributes, const Any& DefaultValue )
    throw( PropertyExistException,
           IllegalTypeException,
           IllegalArgumentException,
           RuntimeException )
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
        aFullPropName += OUString("/");
        aFullPropName += makeHierarchalNameSegment( Name );

        if ( xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
        {
            // Already in set.
            throw PropertyExistException();
        }
    }

    // Property is always removeable.
    Attributes |= PropertyAttribute::REMOVEABLE;

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
                                    OUString("Handle"),
                                    makeAny( sal_Int32( -1 ) ) );

                // Set default value
                xNameReplace->replaceByName(
                                    OUString("Value"),
                                    DefaultValue );

                // Set state ( always "default" )
                xNameReplace->replaceByName(
                                    OUString("State"),
                                    makeAny(
                                        sal_Int32(
                                            PropertyState_DEFAULT_VALUE ) ) );

                // Set attributes
                xNameReplace->replaceByName(
                                    OUString("Attributes"),
                                    makeAny( sal_Int32( Attributes ) ) );

                // Insert new item.
                xContainer->insertByName( Name, makeAny( xNameReplace ) );

                // Commit changes.
                xBatch->commitChanges();

                // Property set info is invalid.
                if ( m_pImpl->m_pInfo )
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

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::removeProperty( const OUString& Name )
    throw( UnknownPropertyException,
           NotRemoveableException,
           RuntimeException )
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
        aFullPropName   += OUString("/");
        aFullPropName   += makeHierarchalNameSegment( Name );

        // Property in set?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            throw UnknownPropertyException();

        // Property removeable?
        try
        {
            OUString aFullAttrName = aFullPropName;
            aFullAttrName += OUString("/Attributes");

            sal_Int32 nAttribs = 0;
            if ( xRootHierNameAccess->getByHierarchicalName( aFullAttrName )
                    >>= nAttribs )
            {
                if ( !( nAttribs & PropertyAttribute::REMOVEABLE ) )
                {
                    // Not removeable!
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
                                += OUString("/Handle");

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
                if ( m_pImpl->m_pInfo )
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

//=========================================================================
//
// XPropertySetInfoChangeNotifier methods.
//
//=========================================================================

// virtual
void SAL_CALL PersistentPropertySet::addPropertySetInfoChangeListener(
                const Reference< XPropertySetInfoChangeListener >& Listener )
    throw( RuntimeException )
{
    if ( !m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners =
                    new OInterfaceContainerHelper( m_pImpl->m_aMutex );

    m_pImpl->m_pPropSetChangeListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::removePropertySetInfoChangeListener(
                const Reference< XPropertySetInfoChangeListener >& Listener )
    throw( RuntimeException )
{
    if ( m_pImpl->m_pPropSetChangeListeners )
        m_pImpl->m_pPropSetChangeListeners->removeInterface( Listener );
}

//=========================================================================
//
// XPropertyAccess methods.
//
//=========================================================================

// virtual
Sequence< PropertyValue > SAL_CALL PersistentPropertySet::getPropertyValues()
    throw( RuntimeException )
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

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::setPropertyValues(
                                 const Sequence< PropertyValue >& aProps )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
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
        aFullPropNamePrefix += OUString("/");

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
                                    OUString("Handle"),
                                    makeAny( rNewValue.Handle ) );

                        // Save old value
                        OUString aValueName = aFullPropName;
                        aValueName += OUString("/Value");
                        Any aOldValue
                            = xRootHierNameAccess->getByHierarchicalName(
                                                                aValueName );
                        // Write value
                        xNameReplace->replaceByName(
                                    OUString("Value"),
                                    rNewValue.Value );

                        // Write state ( Now it is a directly set value )
                        xNameReplace->replaceByName(
                                    OUString("State"),
                                    makeAny(
                                        sal_Int32(
                                            PropertyState_DIRECT_VALUE ) ) );

                        // Commit changes.
                        xBatch->commitChanges();

                        if ( m_pImpl->m_pPropertyChangeListeners )
                        {
                            PropertyChangeEvent aEvt;
                            aEvt.Source         = (OWeakObject*)this;
                            aEvt.PropertyName   = rNewValue.Name;
                            aEvt.PropertyHandle = rNewValue.Handle;
                            aEvt.Further        = sal_False;
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

//=========================================================================
//
// Non-interface methods
//
//=========================================================================

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

//=========================================================================
void PersistentPropertySet::notifyPropertySetInfoChange(
                                const PropertySetInfoChangeEvent& evt ) const
{
    if ( !m_pImpl->m_pPropSetChangeListeners )
        return;

    // Notify event listeners.
    OInterfaceIteratorHelper aIter( *( m_pImpl->m_pPropSetChangeListeners ) );
    while ( aIter.hasMoreElements() )
    {
        // Propagate event.
        Reference< XPropertySetInfoChangeListener >
                            xListener( aIter.next(), UNO_QUERY );
        if ( xListener.is() )
            xListener->propertySetInfoChange( evt );
    }
}

//=========================================================================
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
                += OUString(  "/Values"  );
        }
    }

    return m_pImpl->m_aFullKey;
}

//=========================================================================
PropertySetRegistry& PersistentPropertySet::getPropertySetRegistry()
{
    return *m_pImpl->m_pCreator;
}

//=========================================================================
//=========================================================================
//
// PropertySetInfo_Impl Implementation.
//
//=========================================================================
//=========================================================================

PropertySetInfo_Impl::PropertySetInfo_Impl(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        PersistentPropertySet* pOwner )
: m_xSMgr( rxSMgr ),
  m_pProps( NULL ),
  m_pOwner( pOwner )
{
}

//=========================================================================
// virtual
PropertySetInfo_Impl::~PropertySetInfo_Impl()
{
    delete m_pProps;

    // !!! Do not delete m_pOwner !!!
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( PropertySetInfo_Impl,
                   XTypeProvider,
                   XPropertySetInfo );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( PropertySetInfo_Impl,
                         XTypeProvider,
                         XPropertySetInfo );

//=========================================================================
//
// XPropertySetInfo methods.
//
//=========================================================================

// virtual
Sequence< Property > SAL_CALL PropertySetInfo_Impl::getProperties()
    throw( RuntimeException )
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
                    m_pProps = pPropSeq;
                    return *m_pProps;
                }
            }
            catch (const NoSuchElementException&)
            {
                // getByHierarchicalName
            }
        }

        OSL_FAIL( "PropertySetInfo_Impl::getProperties - Error!" );
        m_pProps = new Sequence< Property >( 0 );
    }

    return *m_pProps;
}

//=========================================================================
// virtual
Property SAL_CALL PropertySetInfo_Impl::getPropertyByName(
                                                    const OUString& aName )
    throw( UnknownPropertyException, RuntimeException )
{
    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
            m_pOwner->getPropertySetRegistry().getRootConfigReadAccess(),
            UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullPropName( m_pOwner->getFullKey() );
        aFullPropName += OUString("/");
        aFullPropName += makeHierarchalNameSegment( aName );

        // Does property exist?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            throw UnknownPropertyException();

        try
        {
            Property aProp;

            // Obtain handle.
            OUString aKey = aFullPropName;
            aKey += OUString("/Handle");

            if ( !( xRootHierNameAccess->getByHierarchicalName( aKey )
                    >>= aProp.Handle ) )
            {
                OSL_FAIL( "PropertySetInfo_Impl::getPropertyByName - "
                            "No handle!" );
                return Property();
            }

            // Obtain Value and extract type.
            aKey = aFullPropName;
            aKey += OUString("/Value");

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
            aKey += OUString("/Attributes");

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

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertySetInfo_Impl::hasPropertyByName(
                                                    const OUString& Name )
    throw( RuntimeException )
{
    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
            m_pOwner->getPropertySetRegistry().getRootConfigReadAccess(),
            UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullPropName( m_pOwner->getFullKey() );
        aFullPropName += OUString("/");
        aFullPropName += makeHierarchalNameSegment( Name );

        return xRootHierNameAccess->hasByHierarchicalName( aFullPropName );
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
