/*************************************************************************
 *
 *  $RCSfile: ucbstore.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-26 15:17:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

  *************************************************************************/

#ifndef __LIST__
#include <stl/list>
#endif
#ifndef __HASH_MAP__
#include <stl/hash_map>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSETINFOCHANGE_HPP_
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGESCAPE_HPP_
#include <com/sun/star/util/XStringEscape.hpp>
#endif

#ifndef _UCBSTORE_HXX
#include "ucbstore.hxx"
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;
using namespace rtl;

//=========================================================================

#define STORE_CONTENTPROPERTIES_KEY "/org.openoffice.ucb.Store/ContentProperties"

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

typedef std::hash_map
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

XINTERFACE_IMPL_3( UcbStore,
                   XTypeProvider,
                   XServiceInfo,
                   XPropertySetRegistryFactory );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( UcbStore,
                         XTypeProvider,
                         XServiceInfo,
                      XPropertySetRegistryFactory );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( UcbStore,
                     OUString::createFromAscii( "UcbStore" ),
                     OUString::createFromAscii( STORE_SERVICE_NAME ) );

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
UcbStore::createPropertySetRegistry( const OUString& URL )
    throw( RuntimeException )
{
    // URL is ignored by this interface implementation. It always uses
    // the configuration server as storage medium.

    return Reference< XPropertySetRegistry >(
                            new PropertySetRegistry( m_xSMgr, *this ) );
}

//=========================================================================
//
// New methods.
//
//=========================================================================

void UcbStore::removeRegistry( const OUString& URL )
{
    // nothing to do, since this implementation does not maintain
    // different regsitry instances.
}

//=========================================================================
//
// PropertySetRegistry_Impl.
//
//=========================================================================

struct PropertySetRegistry_Impl
{
    UcbStore*                         m_pCreator;
    PropertySetMap_Impl               m_aPropSets;
    Reference< XMultiServiceFactory > m_xConfigProvider;
    osl::Mutex                        m_aMutex;

    PropertySetRegistry_Impl( UcbStore& rCreator )
    : m_pCreator( &rCreator )
    {
        m_pCreator->acquire();
    }

    ~PropertySetRegistry_Impl()
    {
        m_pCreator->removeRegistry( OUString() );
        m_pCreator->release();
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
                        UcbStore& rCreator )
: m_xSMgr( rXSMgr ),
  m_pImpl( new PropertySetRegistry_Impl( rCreator ) )
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
                                OUString::createFromAscii(
                                               "PropertySetRegistry" ),
                                OUString::createFromAscii(
                                               PROPSET_REG_SERVICE_NAME ) );

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
    if ( key.getLength() )
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

            osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

            Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
            if ( xRootHierNameAccess.is() )
            {
                Reference< XStringEscape > xEsc(
                                    xRootHierNameAccess, UNO_QUERY );

                OSL_ENSURE( xEsc.is(),
                            "PropertySetRegistry::openPropertySet - "
                            "No string escape!" );

                OUString aEscapedKey;
                if ( xEsc.is() )
                {
                    try
                    {
                        aEscapedKey = xEsc->escapeString( key );
                    }
                    catch ( IllegalArgumentException& )
                    {
                        // escapeString

                        OSL_ENSURE( xEsc.is(),
                                    "PropertySetRegistry::openPropertySet - "
                                    "caught IllegalArgumentException!" );

                        aEscapedKey = key;
                    }
                }
                else
                    aEscapedKey = key;

                // Propertyset in registry?
                if ( xRootHierNameAccess->hasByHierarchicalName( aEscapedKey ) )
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
//                                      OUString::createFromAscii( "Values" ),
//                                      makeAny( ... ) );

                                // Insert new item.

                                xContainer->insertByName(
                                        aEscapedKey, makeAny( xNameReplace ) );

                                // Commit changes.
                                xBatch->commitChanges();

                                return Reference< XPersistentPropertySet >(
                                            new PersistentPropertySet(
                                                    m_xSMgr, *this, key ) );
                            }
                        }
                        catch ( IllegalArgumentException& )
                        {
                            // insertByName

                            OSL_ENSURE( sal_False,
                                        "PropertySetRegistry::openPropertySet - "
                                        "caught IllegalArgumentException!" );
                        }
                        catch ( ElementExistException& )
                        {
                            // insertByName

                            OSL_ENSURE( sal_False,
                                        "PropertySetRegistry::openPropertySet - "
                                        "caught ElementExistException!" );
                        }
                        catch ( WrappedTargetException& )
                        {
                            // insertByName, commitChanges

                            OSL_ENSURE( sal_False,
                                        "PropertySetRegistry::openPropertySet - "
                                        "caught WrappedTargetException!" );
                        }
                        catch ( RuntimeException& )
                        {
                            OSL_ENSURE( sal_False,
                                        "PropertySetRegistry::openPropertySet - "
                                        "caught RuntimeException!" );
                        }
                        catch ( Exception& )
                        {
                            // createInstance

                            OSL_ENSURE( sal_False,
                                        "PropertySetRegistry::openPropertySet - "
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

            OSL_ENSURE( sal_False,
                        "PropertySetRegistry::openPropertySet - Error!" );
        }
    }

    return Reference< XPersistentPropertySet >();
}

//=========================================================================
// virtual
void SAL_CALL PropertySetRegistry::removePropertySet( const OUString& key )
    throw( RuntimeException )
{
    if ( !key.getLength() )
        return;

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                                    getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        Reference< XStringEscape > xEsc( xRootHierNameAccess, UNO_QUERY );

        OSL_ENSURE( xEsc.is(),
                    "PropertySetRegistry::removePropertySet - "
                    "No string escape!" );

        OUString aEscapedKey;
        if ( xEsc.is() )
        {
            try
            {
                aEscapedKey = xEsc->escapeString( key );
            }
            catch ( IllegalArgumentException& )
            {
                // escapeString

                OSL_ENSURE( xEsc.is(),
                            "PropertySetRegistry::removePropertySet - "
                            "caught IllegalArgumentException!" );

                aEscapedKey = key;
            }
        }
        else
            aEscapedKey = key;

        // Propertyset in registry?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aEscapedKey ) )
            return;

        Reference< XChangesBatch > xBatch(
                            getConfigWriteAccess( OUString() ), UNO_QUERY );
        Reference< XNameContainer > xContainer( xBatch, UNO_QUERY );

        OSL_ENSURE( xBatch.is(),
                    "PropertySetRegistry::removePropertySet - "
                    "No batch!" );

        OSL_ENSURE( xContainer.is(),
                    "PropertySetRegistry::removePropertySet - "
                    "No conteiner!" );

        if ( xBatch.is() && xContainer.is() )
        {
            try
            {
                // Remove item.
                xContainer->removeByName( aEscapedKey );

                // Commit changes.
                xBatch->commitChanges();

                // Success.
                return;
            }
            catch ( NoSuchElementException& )
            {
                // removeByName

                OSL_ENSURE( sal_False,
                            "PropertySetRegistry::removePropertySet - "
                            "caught NoSuchElementException!" );
                return;
            }
            catch ( WrappedTargetException& )
            {
                // commitChanges

                OSL_ENSURE( sal_False,
                            "PropertySetRegistry::removePropertySet - "
                            "caught WrappedTargetException!" );
                return;
            }
        }

        return;
    }

    OSL_ENSURE( sal_False, "PropertySetRegistry::removePropertySet - Error!" );
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
        Reference< XStringEscape > xEscaper( xNameAccess, UNO_QUERY );

        OSL_ENSURE( xEscaper.is(),
                    "PropertySetRegistry::getByName - No string escaper!" );

        OUString aRealName( aName );

        if ( xEscaper.is() )
        {
            try
            {
                aRealName = xEscaper->escapeString( aName );
            }
            catch ( IllegalArgumentException& )
            {
                // escapeString
                aRealName = aName;
            }
        }

        try
        {
            return xNameAccess->getByName( aRealName );
        }
        catch ( NoSuchElementException& )
        {
            // getByName
        }
        catch ( WrappedTargetException& )
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
        Reference< XStringEscape > xEscaper( xNameAccess, UNO_QUERY );

        OSL_ENSURE( xEscaper.is(),
                "PropertySetRegistry::getElementNames - No string escaper!" );

        if ( !xEscaper.is() )
            return xNameAccess->getElementNames();

        // Decode names...
        Sequence< OUString > aEncodedNames = xNameAccess->getElementNames();

        sal_Int32 nCount = aEncodedNames.getLength();
        Sequence< OUString > aNames( nCount );

        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            try
            {
                aNames[ n ] = xEscaper->unescapeString( aEncodedNames[ n ] );
            }
            catch ( IllegalArgumentException& )
            {
                // unescapeString
                aNames[ n ] = aEncodedNames[ n ];
            }
        }

        return aNames;
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
        Reference< XStringEscape > xEscaper( xNameAccess, UNO_QUERY );

        OSL_ENSURE( xEscaper.is(),
                    "PropertySetRegistry::hasByName - No string escaper!" );

        OUString aRealName( aName );

        if ( xEscaper.is() )
        {
            try
            {
                aRealName = xEscaper->escapeString( aName );
            }
            catch ( IllegalArgumentException& )
            {
                // escapeString
                aRealName = aName;
            }
        }

         return xNameAccess->hasByName( aRealName );
    }

    return sal_False;
}

//=========================================================================
void PropertySetRegistry::add( PersistentPropertySet* pSet )
{
    OUString key( pSet->getKey() );

    if ( key.getLength() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        PropertySetMap_Impl& rSets = m_pImpl->m_aPropSets;

        PropertySetMap_Impl::const_iterator it = rSets.find( key );
        rSets[ key ] = pSet;
    }
}

//=========================================================================
void PropertySetRegistry::remove( PersistentPropertySet* pSet )
{
    OUString key( pSet->getKey() );

    if ( key.getLength() )
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

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                            getConfigWriteAccess( OUString() ), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        Reference< XStringEscape > xEscaper( xRootHierNameAccess, UNO_QUERY );

        OSL_ENSURE( xEscaper.is(),
                    "PropertySetRegistry::renamePropertySet - "
                    "No string escaper!" );

        if ( xEscaper.is() )
        {
            OUString aOldKey;
            try
            {
                aOldKey = xEscaper->escapeString( rOldKey );
            }
            catch ( IllegalArgumentException& )
            {
                // escapeString
                aOldKey = rOldKey;
            }

            OUString aNewKey;
            try
            {
                aNewKey = xEscaper->escapeString( rNewKey );
            }
            catch ( IllegalArgumentException& )
            {
                // escapeString
                aNewKey = rNewKey;
            }

            // Old key present?
            if ( xRootHierNameAccess->hasByHierarchicalName( aOldKey ) )
            {
                // New key not present?
                if ( xRootHierNameAccess->hasByHierarchicalName( aNewKey ) )
                {
                    OSL_ENSURE( sal_False,
                                "PropertySetRegistry::renamePropertySet - "
                                "New key exists!" );
                    return;
                }

                Reference< XSingleServiceFactory > xFac(
                                        xRootHierNameAccess, UNO_QUERY );
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
                                        aNewKey, makeAny( xNameReplace ) );

                            // Commit changes.
                            xBatch->commitChanges();
                        }
                    }
                    catch ( IllegalArgumentException& )
                    {
                        // insertByName

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught IllegalArgumentException!" );
                        return;
                    }
                    catch ( ElementExistException& )
                    {
                        // insertByName

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught ElementExistException!" );
                        return;
                    }
                    catch ( WrappedTargetException& )
                    {
                        // insertByName, commitChanges

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught WrappedTargetException!" );
                        return;
                    }
                    catch ( RuntimeException& )
                    {
                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught RuntimeException!" );
                        return;
                    }
                    catch ( Exception& )
                    {
                        // createInstance

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught Exception!" );
                        return;
                    }

                    //////////////////////////////////////////////////////
                    // Copy data...
                    //////////////////////////////////////////////////////

                    try
                    {
                        OUString aOldValuesKey( aOldKey );
                        aOldValuesKey += OUString::createFromAscii( "/Values" );

                        Reference< XNameAccess > xOldNameAccess;
                        xRootHierNameAccess->getByHierarchicalName(
                                                            aOldValuesKey )
                            >>= xOldNameAccess;
                        if ( !xOldNameAccess.is() )
                        {
                            OSL_ENSURE( sal_False,
                                "PersistentPropertySet::renamePropertySet - "
                                "No old name access!" );
                            return;
                        }

                        // Obtain property names.
                        Sequence< OUString > aElems
                                        = xOldNameAccess->getElementNames();
                        sal_uInt32 nCount = aElems.getLength();
                        if ( nCount )
                        {
                            OUString aNewValuesKey( aNewKey );
                            aNewValuesKey
                                    += OUString::createFromAscii( "/Values" );

                            Reference< XSingleServiceFactory > xNewFac;
                            xRootHierNameAccess->getByHierarchicalName(
                                                            aNewValuesKey )
                                >>= xNewFac;
                            if ( !xNewFac.is() )
                            {
                                OSL_ENSURE( sal_False,
                                    "PersistentPropertySet::renamePropertySet - "
                                    "No new factory!" );
                                return;
                            }

                            Reference< XNameContainer > xNewContainer(
                                                        xNewFac, UNO_QUERY );
                            if ( !xNewContainer.is() )
                            {
                                OSL_ENSURE( sal_False,
                                    "PersistentPropertySet::renamePropertySet - "
                                    "No new container!" );
                                return;
                            }

                            aOldValuesKey += OUString::createFromAscii( "/" );

                            OUString aHandleKey
                                = OUString::createFromAscii( "/Handle" );
                            OUString aValueKey
                                = OUString::createFromAscii( "/Value" );
                            OUString aStateKey
                                = OUString::createFromAscii( "/State" );
                            OUString aAttrKey
                                = OUString::createFromAscii( "/Attributes" );

                            for ( sal_Int32 n = 0; n < nCount; ++n )
                            {
                                const OUString& rPropName = aElems[ n ];

                                // Create new item.
                                Reference< XNameReplace > xNewPropNameReplace(
                                    xNewFac->createInstance(), UNO_QUERY );

                                if ( !xNewPropNameReplace.is() )
                                {
                                    OSL_ENSURE( sal_False,
                                        "PersistentPropertySet::renamePropertySet - "
                                        "No new prop name replace!" );
                                    return;
                                }

                                // Fill new item...

                                // Set Values
                                OUString aKey = aOldValuesKey;
                                aKey += rPropName;

                                // ... handle
                                OUString aNewKey = aKey;
                                aNewKey += aHandleKey;
                                Any aAny =
                                    xRootHierNameAccess->getByHierarchicalName(
                                        aNewKey );
                                xNewPropNameReplace->replaceByName(
                                    OUString::createFromAscii( "Handle" ),
                                    aAny );

                                // ... value
                                aNewKey = aKey;
                                aNewKey += aValueKey;
                                aAny =
                                    xRootHierNameAccess->getByHierarchicalName(
                                        aNewKey );
                                xNewPropNameReplace->replaceByName(
                                    OUString::createFromAscii( "Value" ),
                                    aAny );

                                // ... state
                                aNewKey = aKey;
                                aNewKey += aStateKey;
                                aAny =
                                    xRootHierNameAccess->getByHierarchicalName(
                                        aNewKey );
                                xNewPropNameReplace->replaceByName(
                                    OUString::createFromAscii( "State" ),
                                    aAny );

                                // ... attributes
                                aNewKey = aKey;
                                aNewKey += aAttrKey;
                                aAny =
                                    xRootHierNameAccess->getByHierarchicalName(
                                        aNewKey );
                                xNewPropNameReplace->replaceByName(
                                    OUString::createFromAscii( "Attributes" ),
                                    aAny );

                                // Insert new item.
                                xNewContainer->insertByName(
                                    rPropName, makeAny( xNewPropNameReplace ) );

                                // Commit changes.
                                xBatch->commitChanges();
                            }
                        }
                    }
                    catch ( IllegalArgumentException& )
                    {
                        // insertByName, replaceByName

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught IllegalArgumentException!" );
                        return;
                    }
                    catch ( ElementExistException& )
                    {
                        // insertByName

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught ElementExistException!" );
                        return;
                    }
                    catch ( WrappedTargetException& )
                    {
                        // insertByName, replaceByName, commitChanges

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught WrappedTargetException!" );
                        return;
                    }
                    catch ( NoSuchElementException& )
                    {
                        // getByHierarchicalName, replaceByName

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught NoSuchElementException!" );
                        return;
                    }
                    catch ( RuntimeException& )
                    {
                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught RuntimeException!" );
                        return;
                    }
                    catch ( Exception& )
                    {
                        // createInstance

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught Exception!" );
                        return;
                    }

                    //////////////////////////////////////////////////////
                    // Remove old entry...
                    //////////////////////////////////////////////////////

                    try
                    {
                        // Remove item.
                        xContainer->removeByName( aOldKey );

                        // Commit changes.
                        xBatch->commitChanges();

                        // Success.
                        return;
                    }
                    catch ( NoSuchElementException& )
                    {
                        // removeByName

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught NoSuchElementException!" );
                        return;
                    }
                    catch ( WrappedTargetException& )
                    {
                        // commitChanges

                        OSL_ENSURE( sal_False,
                                    "PropertySetRegistry::renamePropertySet - "
                                    "caught WrappedTargetException!" );
                        return;
                    }
                }
            }
        }
    }

    OSL_ENSURE( sal_False, "PropertySetRegistry::renamePropertySet - Error!" );
}

//=========================================================================
Reference< XInterface > PropertySetRegistry::getRootConfigReadAccess()
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( !m_pImpl->m_xConfigProvider.is() )
            m_pImpl->m_xConfigProvider = Reference< XMultiServiceFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                UNO_QUERY );

        if ( m_pImpl->m_xConfigProvider.is() )
        {
            Sequence< Any > aArguments( 1 );
            aArguments[ 0 ]
                <<= OUString::createFromAscii( STORE_CONTENTPROPERTIES_KEY );

            Reference< XInterface > xInterface(
                m_pImpl->m_xConfigProvider->createInstanceWithArguments(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationAccess" ),
                    aArguments ) );

            if ( xInterface.is() )
                return xInterface;
        }
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
        // createInstance, createInstanceWithArguments

        OSL_ENSURE( sal_False,
            "PropertySetRegistry::getRootConfigReadAccess - caught Exception!" );
        return Reference< XInterface >();
    }

    OSL_ENSURE( sal_False,
                "PropertySetRegistry::getRootConfigReadAccess - Error!" );
    return Reference< XInterface >();
}

//=========================================================================
Reference< XInterface > PropertySetRegistry::getConfigWriteAccess(
                                                    const OUString& rPath )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( !m_pImpl->m_xConfigProvider.is() )
            m_pImpl->m_xConfigProvider = Reference< XMultiServiceFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                UNO_QUERY );

        if ( m_pImpl->m_xConfigProvider.is() )
        {
            OUString aConfigPath
                = OUString::createFromAscii( STORE_CONTENTPROPERTIES_KEY );
            if ( rPath.getLength() )
            {
                if ( rPath.compareToAscii( "/", 1 ) != 0 )
                    aConfigPath += OUString::createFromAscii( "/" );

                aConfigPath += rPath;
            }

            Sequence< Any > aArguments( 1 );
            aArguments[ 0 ] <<= aConfigPath;

            Reference< XInterface > xInterface(
                m_pImpl->m_xConfigProvider->createInstanceWithArguments(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationUpdateAccess" ),
                    aArguments ) );

            if ( xInterface.is() )
                return xInterface;
        }
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
        // createInstance, createInstanceWithArguments

        OSL_ENSURE( sal_False,
            "PropertySetRegistry::getConfigWriteAccess - caught Exception!" );
        return Reference< XInterface >();
    }

    OSL_ENSURE( sal_False,
                "PropertySetRegistry::getConfigWriteAccess - Error!" );
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
                                OUString::createFromAscii(
                                               "PersistentPropertySet" ),
                                OUString::createFromAscii(
                                               PERS_PROPSET_SERVICE_NAME ) );

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
    if ( !aPropertyName.getLength() )
        throw UnknownPropertyException();

    m_pImpl->m_aMutex.acquire();

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        OUString aFullPropName( getFullKey( xRootHierNameAccess ) );
        aFullPropName += OUString::createFromAscii( "/" );
        aFullPropName += aPropertyName;

        // Does property exist?
        if ( xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
        {
            Reference< XNameReplace > xNameReplace(
                    m_pImpl->m_pCreator->getConfigWriteAccess(
                                            aFullPropName ), UNO_QUERY );
            Reference< XChangesBatch > xBatch( xNameReplace, UNO_QUERY );

            if ( xNameReplace.is() && xBatch.is() )
            {
                try
                {
                    // Obtain old value
                    OUString aValueName = aFullPropName;
                    aValueName += OUString::createFromAscii( "/Value" );
                    Any aOldValue
                        = xRootHierNameAccess->getByHierarchicalName(
                                                                aValueName );
                    // Check value type.
                    if ( aOldValue.getValueType() != aValue.getValueType() )
                    {
                        m_pImpl->m_aMutex.release();
                        throw IllegalArgumentException();
                    }

                    // Write value
                    xNameReplace->replaceByName(
                                    OUString::createFromAscii( "Value" ),
                                    aValue );

                    // Write state ( Now it is a directly set value )
                    xNameReplace->replaceByName(
                                    OUString::createFromAscii( "State" ),
                                    makeAny( PropertyState_DIRECT_VALUE ) );

                    // Commit changes.
                    xBatch->commitChanges();

                    PropertyChangeEvent aEvt;
                    if ( m_pImpl->m_pPropertyChangeListeners )
                    {
                        // Obtain handle
                        aValueName = aFullPropName;
                        aValueName += OUString::createFromAscii( "/Handle" );
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
                        m_pImpl->m_aMutex.release();

                        notifyPropertyChangeEvent( aEvt );
                    }
                    return;
                }
                catch ( IllegalArgumentException& )
                {
                    // replaceByName
                }
                catch ( NoSuchElementException& )
                {
                    // getByHierarchicalName, replaceByName
                }
                catch ( WrappedTargetException& )
                {
                    // replaceByName, commitChanges
                }
            }
        }
    }

    m_pImpl->m_aMutex.release();
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
    if ( !PropertyName.getLength() )
        throw UnknownPropertyException();

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    Reference< XHierarchicalNameAccess > xNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        OUString aFullPropName( getFullKey( xNameAccess ) );
        aFullPropName += OUString::createFromAscii( "/" );
        aFullPropName += PropertyName;
        aFullPropName += OUString::createFromAscii( "/Value" );

        try
        {
            return xNameAccess->getByHierarchicalName( aFullPropName );
        }
        catch ( NoSuchElementException& )
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
                    const OUString& PropertyName,
                    const Reference< XVetoableChangeListener >& aListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
//  load();
//  OSL_ENSURE( sal_False,
//              "PersistentPropertySet::addVetoableChangeListener - N.Y.I." );
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::removeVetoableChangeListener(
                    const OUString& PropertyName,
                    const Reference< XVetoableChangeListener >& aListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
//  load();
//  OSL_ENSURE( sal_False,
//              "PersistentPropertySet::removeVetoableChangeListener - N.Y.I." );
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
rtl::OUString SAL_CALL PersistentPropertySet::getName()
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
    if ( !Name.getLength() )
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
        aFullValuesName = getFullKey( xRootHierNameAccess );
        OUString aFullPropName = aFullValuesName;
        aFullPropName += OUString::createFromAscii( "/" );
        aFullPropName += Name;

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
    Reference< XChangesBatch >  xBatch( xFac, UNO_QUERY );
    Reference< XNameContainer > xContainer( xFac, UNO_QUERY );

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
                                    OUString::createFromAscii( "Handle" ),
                                    makeAny( sal_Int32( -1 ) ) );

                // Set default value
                xNameReplace->replaceByName(
                                    OUString::createFromAscii( "Value" ),
                                    DefaultValue );

                // Set state ( always "default" )
                xNameReplace->replaceByName(
                                    OUString::createFromAscii( "State" ),
                                    makeAny( PropertyState_DEFAULT_VALUE ) );

                // Set attributes
                xNameReplace->replaceByName(
                                    OUString::createFromAscii( "Attributes" ),
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
        catch ( IllegalArgumentException& )
        {
            // insertByName

            OSL_ENSURE( sal_False,
                        "PersistentPropertySet::addProperty - "
                        "caught IllegalArgumentException!" );
            return;
        }
        catch ( ElementExistException& )
        {
            // insertByName

            OSL_ENSURE( sal_False,
                        "PersistentPropertySet::addProperty - "
                        "caught ElementExistException!" );
            return;
        }
        catch ( WrappedTargetException& )
        {
            // replaceByName, insertByName, commitChanges

            OSL_ENSURE( sal_False,
                        "PersistentPropertySet::addProperty - "
                        "caught WrappedTargetException!" );
            return;
        }
        catch ( RuntimeException& )
        {
            throw;
        }
        catch ( Exception& )
        {
            // createInstance

            OSL_ENSURE( sal_False,
                        "PersistentPropertySet::addProperty - "
                        "caught Exception!" );
            return;
        }
    }

    OSL_ENSURE( sal_False,
                "PersistentPropertySet::addProperty - Error!" );
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
        aFullValuesName = getFullKey( xRootHierNameAccess );
        aFullPropName   = aFullValuesName;
        aFullPropName += OUString::createFromAscii( "/" );
        aFullPropName += Name;

        // Property in set?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            throw UnknownPropertyException();

        // Property removeable?
        try
        {
            OUString aFullAttrName = aFullPropName;
            aFullAttrName += OUString::createFromAscii( "/Attributes" );

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
                OSL_ENSURE( sal_False,
                            "PersistentPropertySet::removeProperty - "
                            "No attributes!" );
                return;
            }
        }
        catch ( NoSuchElementException& )
        {
            // getByHierarchicalName

            OSL_ENSURE( sal_False,
                        "PersistentPropertySet::removeProperty - "
                        "caught NoSuchElementException!" );
        }

        // Remove property...

        Reference< XChangesBatch > xBatch(
                m_pImpl->m_pCreator->getConfigWriteAccess( aFullValuesName ),
                UNO_QUERY );
        Reference< XNameContainer > xContainer( xBatch, UNO_QUERY );

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
                                += OUString::createFromAscii( "/Handle" );

                        if ( ! ( xRootHierNameAccess->getByHierarchicalName(
                                        aFullHandleName ) >>= nHandle ) )
                            nHandle = -1;

                    }
                    catch ( NoSuchElementException& )
                    {
                        // getByHierarchicalName

                        OSL_ENSURE( sal_False,
                                    "PersistentPropertySet::removeProperty - "
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
            catch ( NoSuchElementException& )
            {
                // removeByName

                OSL_ENSURE( sal_False,
                            "PersistentPropertySet::removeProperty - "
                            "caught NoSuchElementException!" );
                return;
            }
            catch ( WrappedTargetException& )
            {
                // commitChanges

                OSL_ENSURE( sal_False,
                            "PersistentPropertySet::removeProperty - "
                            "caught WrappedTargetException!" );
                return;
            }
        }
    }

    OSL_ENSURE( sal_False,
                "PersistentPropertySet::removeProperty - Error!" );
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
            xRootHierNameAccess->getByHierarchicalName(
                    getFullKey( xRootHierNameAccess ) ) >>= xNameAccess;
            if ( xNameAccess.is() )
            {
                // Obtain property names.

                Sequence< OUString > aElems = xNameAccess->getElementNames();

                sal_uInt32 nCount = aElems.getLength();
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

                        OUString aHandleName
                                    = OUString::createFromAscii( "/Handle" );
                        OUString aValueName
                                    = OUString::createFromAscii( "/Value" );
                        OUString aStateName
                                    = OUString::createFromAscii( "/State" );

                        for ( sal_Int32 n = 0; n < nCount; ++n )
                        {
                            PropertyValue& rValue = aValues[ n ];
                            OUString       rName  = aElems[ n ];

                            // Set property name.

                            rValue.Name = rName;

                            try
                            {
                                // Obtain and set property handle

                                OUString aHierName = rName;
                                aHierName += aHandleName;

                                Any aKeyValue
                                    = xHierNameAccess->getByHierarchicalName(
                                        aHierName );

                                if ( !( aKeyValue >>= rValue.Handle ) )
                                    OSL_ENSURE( sal_False,
                                      "PersistentPropertySet::getPropertyValues - "
                                      "Error getting property handle!" );
                            }
                            catch ( NoSuchElementException& )
                            {
                                // getByHierarchicalName

                                OSL_ENSURE( sal_False,
                                  "PersistentPropertySet::getPropertyValues - "
                                  "NoSuchElementException!" );
                            }

                            try
                            {
                                // Obtain and set property value

                                OUString aHierName = rName;
                                aHierName += aValueName;

                                rValue.Value
                                    = xHierNameAccess->getByHierarchicalName(
                                        aHierName );

                                if ( !rValue.Value.hasValue() )
                                    OSL_ENSURE( sal_False,
                                      "PersistentPropertySet::getPropertyValues - "
                                      "Error getting property value!" );
                            }
                            catch ( NoSuchElementException& )
                            {
                                // getByHierarchicalName

                                OSL_ENSURE( sal_False,
                                  "PersistentPropertySet::getPropertyValues - "
                                  "NoSuchElementException!" );
                            }

                            try
                            {
                                // Obtain and set property state

                                OUString aHierName = rName;
                                aHierName += aStateName;

                                Any aKeyValue
                                    = xHierNameAccess->getByHierarchicalName(
                                        aHierName );

                                sal_Int32 nState = 0;
                                if ( !( aKeyValue >>= nState ) )
                                    OSL_ENSURE( sal_False,
                                      "PersistentPropertySet::getPropertyValues - "
                                      "Error getting property state!" );
                                else
                                    rValue.State = PropertyState( nState );
                            }
                            catch ( NoSuchElementException& )
                            {
                                // getByHierarchicalName

                                OSL_ENSURE( sal_False,
                                  "PersistentPropertySet::getPropertyValues - "
                                  "NoSuchElementException!" );
                            }
                        }

                        return aValues;
                    }
                }
            }
        }
        catch ( NoSuchElementException& )
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

    m_pImpl->m_aMutex.acquire();

    Reference< XHierarchicalNameAccess > xRootHierNameAccess(
                m_pImpl->m_pCreator->getRootConfigReadAccess(), UNO_QUERY );
    if ( xRootHierNameAccess.is() )
    {
        const PropertyValue* pNewValues = aProps.getConstArray();

        typedef std::list< PropertyChangeEvent > Events;
        Events aEvents;

        OUString aFullPropNamePrefix( getFullKey( xRootHierNameAccess ) );
        aFullPropNamePrefix += OUString::createFromAscii( "/" );

        // Iterate over given property value sequence.
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            const PropertyValue& rNewValue = pNewValues[ n ];
            const OUString& rName = rNewValue.Name;

            OUString aFullPropName = aFullPropNamePrefix;
            aFullPropName += rName;

            // Does property exist?
            if ( xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            {
                Reference< XNameReplace > xNameReplace(
                    m_pImpl->m_pCreator->getConfigWriteAccess(
                                            aFullPropName ), UNO_QUERY );
                Reference< XChangesBatch > xBatch( xNameReplace, UNO_QUERY );

                if ( xNameReplace.is() && xBatch.is() )
                {
                    try
                    {
                        // Write handle
                        xNameReplace->replaceByName(
                                    OUString::createFromAscii( "Handle" ),
                                    makeAny( rNewValue.Handle ) );

                        // Save old value
                        OUString aValueName = aFullPropName;
                        aValueName += OUString::createFromAscii( "/Value" );
                        Any aOldValue
                            = xRootHierNameAccess->getByHierarchicalName(
                                                                aValueName );
                        // Write value
                        xNameReplace->replaceByName(
                                    OUString::createFromAscii( "Value" ),
                                    rNewValue.Value );

                        // Write state ( Now it is a directly set value )
                        xNameReplace->replaceByName(
                                    OUString::createFromAscii( "State" ),
                                    makeAny( PropertyState_DIRECT_VALUE ) );

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
                    catch ( IllegalArgumentException& )
                    {
                        // replaceByName
                    }
                    catch ( NoSuchElementException& )
                    {
                        // getByHierarchicalName, replaceByName
                    }
                    catch ( WrappedTargetException& )
                    {
                        // replaceByName, commitChanges
                    }
                }
            }
        }

        // Callback follows!
        m_pImpl->m_aMutex.release();

        if ( m_pImpl->m_pPropertyChangeListeners )
        {
            // Notify property changes.
            Events::const_iterator it  = aEvents.begin();
            Events::const_iterator end = aEvents.end();

            while ( it != end )
            {
                notifyPropertyChangeEvent( (*it) );
                it++;
            }
        }

        return;
    }

    m_pImpl->m_aMutex.release();

    OSL_ENSURE( sal_False,
                "PersistentPropertySet::setPropertyValues - Nothing set!" );
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
const OUString& PersistentPropertySet::getFullKey(
                                const Reference< XInterface >& xEscaper )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_aFullKey.getLength() )
    {
        Reference< XStringEscape > xEsc( xEscaper, UNO_QUERY );

        OSL_ENSURE( xEsc.is(),
                    "PersistentPropertySet::getFullKey - No string escaper!" );

        if ( xEsc.is() )
        {
            try
            {
                m_pImpl->m_aFullKey = xEsc->escapeString( m_pImpl->m_aKey );
            }
            catch ( IllegalArgumentException& )
            {
                // escapeString
                m_pImpl->m_aFullKey = m_pImpl->m_aKey;
            }
        }
        else
            m_pImpl->m_aFullKey = m_pImpl->m_aKey;

        m_pImpl->m_aFullKey += OUString::createFromAscii( "/Values" );
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
                            m_pOwner->getFullKey( xRootHierNameAccess ) )
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
                            OUString aHandleName
                                = OUString::createFromAscii( "/Handle" );
                            OUString aValueName
                                = OUString::createFromAscii( "/Value" );
                            OUString aAttrName
                                = OUString::createFromAscii( "/Attributes" );

                            Property* pProps = pPropSeq->getArray();

                            for ( sal_uInt32 n = 0; n < nCount; ++n )
                            {
                                Property& rProp = pProps[ n ];
                                OUString  rName = aElems[ n ];

                                // Set property name.

                                rProp.Name = rName;

                                try
                                {
                                    // Obtain and set property handle

                                    OUString aHierName = rName;
                                    aHierName += aHandleName;

                                    Any aKeyValue
                                        = xHierNameAccess->getByHierarchicalName(
                                            aHierName );

                                    if ( !( aKeyValue >>= rProp.Handle ) )
                                        OSL_ENSURE( sal_False,
                                          "PropertySetInfo_Impl::getProperties - "
                                          "Error getting property handle!" );
                                }
                                catch ( NoSuchElementException& )
                                {
                                    // getByHierarchicalName

                                    OSL_ENSURE( sal_False,
                                      "PropertySetInfo_Impl::getProperties - "
                                      "NoSuchElementException!" );
                                }

                                try
                                {
                                    // Obtain and set property type

                                    OUString aHierName = rName;
                                    aHierName += aValueName;

                                    Any aKeyValue
                                        = xHierNameAccess->getByHierarchicalName(
                                            aHierName );

                                    if ( !aKeyValue.hasValue() )
                                        OSL_ENSURE( sal_False,
                                          "PropertySetInfo_Impl::getProperties - "
                                          "Error getting property value!" );
                                    else
                                        rProp.Type = aKeyValue.getValueType();

                                }
                                catch ( NoSuchElementException& )
                                {
                                    // getByHierarchicalName

                                    OSL_ENSURE( sal_False,
                                      "PropertySetInfo_Impl::getProperties - "
                                      "NoSuchElementException!" );
                                }

                                try
                                {
                                    // Obtain and set property attributes

                                    OUString aHierName = rName;
                                    aHierName += aAttrName;

                                    Any aKeyValue
                                        = xHierNameAccess->getByHierarchicalName(
                                            aHierName );

                                    sal_Int32 nAttribs = 0;
                                    if ( aKeyValue >>= nAttribs )
                                        rProp.Attributes = nAttribs;
                                    else
                                        OSL_ENSURE( sal_False,
                                          "PropertySetInfo_Impl::getProperties - "
                                          "Error getting property attributes!" );
                                }
                                catch ( NoSuchElementException& )
                                {
                                    // getByHierarchicalName

                                    OSL_ENSURE( sal_False,
                                      "PropertySetInfo_Impl::getProperties - "
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
            catch ( NoSuchElementException& )
            {
                // getByHierarchicalName
            }
        }

        OSL_ENSURE( sal_False, "PropertySetInfo_Impl::getProperties - Error!" );
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
        OUString aFullPropName( m_pOwner->getFullKey( xRootHierNameAccess ) );
        aFullPropName += OUString::createFromAscii( "/" );
        aFullPropName += aName;

        // Does property exist?
        if ( !xRootHierNameAccess->hasByHierarchicalName( aFullPropName ) )
            throw UnknownPropertyException();

        try
        {
            Property aProp;

            // Obtain handle.
            OUString aKey = aFullPropName;
            aKey += OUString::createFromAscii( "/Handle" );

            if ( !( xRootHierNameAccess->getByHierarchicalName( aKey )
                    >>= aProp.Handle ) )
            {
                OSL_ENSURE( sal_False,
                            "PropertySetInfo_Impl::getPropertyByName - "
                            "No handle!" );
                return Property();
            }

            // Obtain Value and extract type.
            aKey = aFullPropName;
            aKey += OUString::createFromAscii( "/Value" );

            Any aValue = xRootHierNameAccess->getByHierarchicalName( aKey );
            if ( !aValue.hasValue() )
            {
                OSL_ENSURE( sal_False,
                            "PropertySetInfo_Impl::getPropertyByName - "
                            "No Value!" );
                return Property();
            }

            aProp.Type = aValue.getValueType();

            // Obtain Attributes.
            aKey = aFullPropName;
            aKey += OUString::createFromAscii( "/Attributes" );

            sal_Int32 nAttribs = 0;
            if ( xRootHierNameAccess->getByHierarchicalName( aKey )
                    >>= nAttribs )
                aProp.Attributes = nAttribs;
            else
            {
                OSL_ENSURE( sal_False,
                            "PropertySetInfo_Impl::getPropertyByName - "
                            "No attributes!" );
                return Property();
            }

            // set name.
            aProp.Name = aName;

            // Success.
            return aProp;
        }
        catch ( NoSuchElementException& )
        {
            // getByHierarchicalName

            OSL_ENSURE( sal_False,
                        "PropertySetInfo_Impl::getPropertyByName - "
                        "caught NoSuchElementException!" );
        }

    }

    OSL_ENSURE( sal_False, "PropertySetInfo_Impl::getPropertyByName - Error!" );
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
        OUString aFullPropName( m_pOwner->getFullKey( xRootHierNameAccess ) );
        aFullPropName += OUString::createFromAscii( "/" );
        aFullPropName += Name;

        return xRootHierNameAccess->hasByHierarchicalName( aFullPropName );
    }

    return sal_False;
}

