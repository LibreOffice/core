/*************************************************************************
 *
 *  $RCSfile: ucbstore.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:52:48 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 UcbStore

 - Vergleiche der URL's case-insensitive -> UNIX ?

 PropertySetRegistry

  *************************************************************************/

#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef __LIST__
#include <stl/list>
#endif
#ifndef __HASH_MAP__
#include <stl/hash_map>
#endif
#ifndef __VECTOR__
#include <stl/vector>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _STORE_STORE_HXX_
#include <store/store.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSETINFOCHANGE_HPP_
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#endif

#include "ucbstore.hxx"

#ifndef _FLATANY_HXX
#include "flatany.hxx"
#endif
#if 0
#ifndef _CNTPPSET_HXX
#include "cntppset.hxx"
#endif
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

//=========================================================================
//
// stl hash map support.
//
//=========================================================================

struct equalStringIgnoreCase_Impl
{
    bool operator ()( const OUString& rKey1, const OUString& rKey2 ) const
    {
        return !!( rKey1.equalsIgnoreCase( rKey2 ) );
    }
};

struct hashStringIgnoreCase_Impl
{
    size_t operator()( const OUString& rKey ) const
    {
        const sal_Unicode* p = rKey.getStr();
        sal_uInt32     h = 0;
        for ( ; *p; ++p )
            h = 5 * h + rtl_char_toLowerCase( *p );

        return h;
    }
};

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

#define PROPERTYSET_STREAM_PREFIX  "propset."
#define PROPERTYSET_STREAM_MAGIC   19690713
#define PROPERTYSET_STREAM_VERSION 1

struct PropertySetStreamHeader_Impl
{
    sal_uInt32 m_nMagic;
    sal_uInt32 m_nVersion;
    sal_uInt32 m_nDataLen; // size of data, without header
    sal_uInt32 m_nCount;   // number of elements

    PropertySetStreamHeader_Impl()
    : m_nMagic( 0 ), m_nVersion( 0 ), m_nDataLen( 0 ), m_nCount( 0 ) {}

    PropertySetStreamHeader_Impl(
                sal_uInt32 M, sal_uInt32 V, sal_uInt32 D, sal_uInt32 C )
    : m_nMagic( M ), m_nVersion( V ), m_nDataLen( D ), m_nCount( C ) {}
};

#define PROPERTYSET_STREAM_HEADER_SIZE  sizeof( PropertySetStreamHeader_Impl )
#define PROPERTYSET_STREAM_ALIGNMENT    4 /* Bytes */

//=========================================================================
//
// class PropertySetStreamBuffer_Impl
//
//=========================================================================

#define ALIGN_POS( a ) \
    (( a*( PROPERTYSET_STREAM_ALIGNMENT-1))%PROPERTYSET_STREAM_ALIGNMENT)

class PropertySetStreamBuffer_Impl
{
    sal_uInt32 m_nSize;
    sal_uInt32 m_nGrow;
    sal_uInt8* m_pBuffer;
    sal_uInt8* m_pPos;

private:
    void ensureCapacity( sal_uInt8 nBytesNeeded );

public:
    PropertySetStreamBuffer_Impl( sal_uInt32 nInitSize,
                                  sal_uInt32 nGrowSize = 4096 );
    ~PropertySetStreamBuffer_Impl();

    operator sal_uInt8* () const { return m_pBuffer; }

    sal_uInt32 getDataLength() const { return ( m_pPos - m_pBuffer ); }

    sal_Bool readString( OUString& rValue );
    sal_Bool readInt32 ( sal_Int32& rValue );
    sal_Bool readAny   ( Any& rValue );

    sal_Bool writeString( const OUString& rValue );
    sal_Bool writeInt32 ( sal_Int32 nValue );
    sal_Bool writeAny   ( const Any& rValue );
};

//=========================================================================
//
// RegistryMap_Impl.
//
//=========================================================================

typedef std::hash_map
<
    OUString,
    PropertySetRegistry*,
    hashStringIgnoreCase_Impl,
    equalStringIgnoreCase_Impl
>
RegistryMap_Impl;

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

private:
    sal_Bool queryProperty( const OUString& aName, Property& rProp  );

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
    RegistryMap_Impl m_aRegistries;
    osl::Mutex       m_aMutex;
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
    if ( URL.getLength() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        RegistryMap_Impl& rRegs = m_pImpl->m_aRegistries;

        RegistryMap_Impl::const_iterator it = rRegs.find( URL );
        if ( it != rRegs.end() )
        {
            // Already instanciated.
            return Reference< XPropertySetRegistry >( (*it).second );
        }
        else
        {
            // Create new and remember, if valid.
            PropertySetRegistry* pNew =
                        PropertySetRegistry::create( m_xSMgr, *this, URL );
            if ( pNew )
            {
                rRegs[ URL ] = pNew;
                return Reference< XPropertySetRegistry >( pNew );
            }
        }
    }

    return Reference< XPropertySetRegistry >();
}

//=========================================================================
//
// New methods.
//
//=========================================================================

void UcbStore::removeRegistry( const OUString& URL )
{
    if ( URL.getLength() )
    {
        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        RegistryMap_Impl& rRegs = m_pImpl->m_aRegistries;

        RegistryMap_Impl::iterator it = rRegs.find( URL );
        if ( it != rRegs.end() )
        {
            // Found.
            rRegs.erase( it );
        }
    }
}

//=========================================================================
//
// PropertySetRegistry_Impl.
//
//=========================================================================

struct PropertySetRegistry_Impl
{
    UcbStore*           m_pCreator;
    OUString            m_aURL;
    store::OStoreFile   m_aStoreFile;
    PropertySetMap_Impl m_aPropSets;
    osl::Mutex          m_aMutex;

    PropertySetRegistry_Impl( UcbStore& rCreator,
                              const OUString& rURL,
                              const store::OStoreFile& rStoreFile )
    : m_pCreator( &rCreator ), m_aURL( rURL ), m_aStoreFile( rStoreFile )
    {
        m_pCreator->acquire();
    }

    ~PropertySetRegistry_Impl()
    {
        m_pCreator->removeRegistry( m_aURL );
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
                        UcbStore& rCreator,
                        const OUString& rURL,
                        const store::OStoreFile& rStoreFile )
: m_xSMgr( rXSMgr ),
  m_pImpl( new PropertySetRegistry_Impl( rCreator, rURL, rStoreFile ) )
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
            // Create new and remember, if valid.
            PersistentPropertySet* pNew =
                PersistentPropertySet::create( m_xSMgr, *this, key, create );
            if ( pNew )
                return Reference< XPersistentPropertySet >( pNew );
        }
    }

    return Reference< XPersistentPropertySet >();
}

//=========================================================================
// virtual
void SAL_CALL PropertySetRegistry::removePropertySet( const OUString& key )
    throw( RuntimeException )
{
    OUString aKey( OUString::createFromAscii( PROPERTYSET_STREAM_PREFIX ) );
    aKey += key;

    storeError nError = m_pImpl->m_aStoreFile.remove(
                                    OUString::createFromAscii( "/" ), aKey );

    VOS_ENSURE( ( nError == store_E_None ) || ( nError == store_E_NotExists ),
                "PropertySetRegistry::removePropertySet - error" );
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
    return getCppuType( ( Reference< XPersistentPropertySet >* ) 0 );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertySetRegistry::hasElements()
    throw( RuntimeException )
{
    Sequence< OUString > aSeq( getElementNames() );
    return ( aSeq.getLength() > 0 );
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
    Reference< XPersistentPropertySet > xSet(
        PersistentPropertySet::create( m_xSMgr, *this, aName, sal_False ) );
    if ( xSet.is() )
        return Any( &xSet, getCppuType( &xSet ) );
    else
        throw NoSuchElementException();

    return Any();
}

//=========================================================================
// virtual
Sequence< OUString > SAL_CALL PropertySetRegistry::getElementNames()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    store::OStoreDirectory aDir;
    storeError nError = aDir.create(
        m_pImpl->m_aStoreFile, OUString(), OUString(), store_AccessReadOnly );

    if ( nError == store_E_None )
    {
        /////////////////////////////////////////////////////////////
        // Collect names.
        /////////////////////////////////////////////////////////////

        std::list< OUString > aElements;

        store::OStoreDirectory::iterator iter;
        OUString       aKeyName;
        const OUString aPrefix( OUString::createFromAscii(
                                            PROPERTYSET_STREAM_PREFIX ) );
        sal_Int32      nPrefixLen = aPrefix.getLength();

        nError = aDir.first( iter );

        while ( nError == store_E_None )
        {
            aKeyName = OUString( iter.m_pszName );

            if ( aKeyName.compareTo( aPrefix, nPrefixLen ) == 0 )
                aElements.push_back(
                    aKeyName.copy( nPrefixLen,
                                   aKeyName.getLength() - nPrefixLen ) );

            nError = aDir.next( iter );
        }

        /////////////////////////////////////////////////////////////
        // Fill sequence.
        /////////////////////////////////////////////////////////////

        sal_uInt32 nCount = aElements.size();
        if ( nCount )
        {
            Sequence< OUString > aSeq( nCount );
            OUString* pNames = aSeq.getArray();
            sal_uInt32 nArrPos = 0;

            std::list < OUString >::const_iterator it  = aElements.begin();
            std::list < OUString >::const_iterator end = aElements.end();

            while ( it != end )
            {
                pNames[ nArrPos ] = (*it);
                it++;
                nArrPos++;
            }

            aDir.close();
            return aSeq;
        }
    }

    aDir.close();
    return Sequence< OUString >( 0 );
}

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertySetRegistry::hasByName( const OUString& aName )
    throw( RuntimeException )
{
    Reference< XPersistentPropertySet > xSet(
        PersistentPropertySet::create( m_xSMgr, *this, aName, sal_False ) );
    if ( xSet.is() )
        return sal_True;

    return sal_False;
}

//=========================================================================
//
// Non-interface methods
//
//=========================================================================

// static
PropertySetRegistry* PropertySetRegistry::create(
                            const Reference< XMultiServiceFactory >& rXSMgr,
                            UcbStore& rCreator,
                            const OUString& rURL )
{
    if ( !rURL.getLength() )
        return NULL;

    // Convert URL to system dependent file path.
    OUString aUNCPath;
    osl::FileBase::RC
    eErr = osl::FileBase::getNormalizedPathFromFileURL( rURL, aUNCPath );
    if ( eErr != osl::FileBase::E_None )
        return NULL;

    OUString aPath;
    eErr = osl::FileBase::getSystemPathFromNormalizedPath( aUNCPath, aPath );
    if ( eErr != osl::FileBase::E_None )
        return NULL;

    // Try to open/create storage file.
    store::OStoreFile aStoreFile;
    storeError nError = aStoreFile.create( aPath, store_AccessReadCreate );
    if ( nError != store_E_None )
        return NULL;

    // Root directory must be created explicitely!
    store::OStoreDirectory aRootDir;
    nError = aRootDir.create(
                aStoreFile, OUString(), OUString(), store_AccessReadCreate );

    return new PropertySetRegistry( rXSMgr, rCreator, rURL, aStoreFile );
}

//=========================================================================
osl::Mutex& PropertySetRegistry::getRegistryMutex() const
{
    return m_pImpl->m_aMutex;
}

//=========================================================================
store::OStoreFile& PropertySetRegistry::getStoreFile() const
{
    return m_pImpl->m_aStoreFile;
}

//=========================================================================
const OUString& PropertySetRegistry::getURL() const
{
    return m_pImpl->m_aURL;
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
    OUString aOldKey( OUString::createFromAscii( PROPERTYSET_STREAM_PREFIX ) );
    OUString aNewKey( aOldKey );
    aOldKey += rOldKey;
    aNewKey += rNewKey;

    storeError nError = m_pImpl->m_aStoreFile.rename(
                                OUString::createFromAscii( "/" ), aOldKey,
                                OUString::createFromAscii( "/" ), aNewKey );

    VOS_ENSURE( ( nError == store_E_None ) || ( nError == store_E_NotExists ),
                "PropertySetRegistry::renamePropertySet - error" );
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
// PropertyInfoList.
//
//=========================================================================

struct PropertyInfo : public com::sun::star::beans::PropertyValue
{
    sal_Int32 Attributes;

    PropertyInfo()
    : Attributes( 0 ) {}

    PropertyInfo( const rtl::OUString& Name,
                  sal_Int32 Handle,
                  const ::com::sun::star::uno::Any& Value,
                  const ::com::sun::star::beans::PropertyState& State,
                  sal_Int32 Attributes )
    : PropertyValue( Name, Handle, Value, State ), Attributes( Attributes ) {}
};

//=========================================================================
//
// PropertyInfoList_Impl.
//
//=========================================================================

typedef std::vector< PropertyInfo > PropertyInfos_Impl;

class PropertyInfoList_Impl : public PropertyInfos_Impl {};

//=========================================================================
//
// PersistentPropertySet_Impl.
//
//=========================================================================

struct PersistentPropertySet_Impl
{
    PropertySetRegistry*        m_pCreator;
    PropertyInfoList_Impl*      m_pValues;
    PropertySetInfo_Impl*       m_pInfo;
    OUString                    m_aKey;
    osl::Mutex                  m_aMutex;
    store::OStoreStream         m_aStream;
    OInterfaceContainerHelper*  m_pDisposeEventListeners;
    OInterfaceContainerHelper*  m_pPropSetChangeListeners;
    PropertyListeners_Impl*     m_pPropertyChangeListeners;

    PersistentPropertySet_Impl( PropertySetRegistry& rCreator,
                                const OUString& rKey,
                                const store::OStoreStream& rStream,
                                PropertyInfoList_Impl* pValues )
    : m_pCreator( &rCreator ), m_pValues( pValues ),
      m_pInfo( NULL ), m_aKey( rKey ), m_aStream( rStream ),
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

        delete m_pValues;
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
                        const OUString& rKey,
                        const store::OStoreStream& rStream )
: m_xSMgr( rXSMgr ),
  m_pImpl( new PersistentPropertySet_Impl( rCreator, rKey, rStream, NULL ) )
{
    // register at creator.
    rCreator.add( this );
}

//=========================================================================
PersistentPropertySet::PersistentPropertySet(
                        const Reference< XMultiServiceFactory >& rXSMgr,
                        PropertySetRegistry& rCreator,
                        const OUString& rKey,
                        const store::OStoreStream& rStream,
                        const PropertyInfoList_Impl& rValues )
: m_xSMgr( rXSMgr ),
  m_pImpl( new PersistentPropertySet_Impl(
                                  rCreator,
                                rKey,
                                rStream,
                                new PropertyInfoList_Impl( rValues ) ) )
{
    // Store properties.
    store();

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

    load();

    PropertyInfoList_Impl& rSeq = *m_pImpl->m_pValues;
    sal_uInt32 nCount = rSeq.size();

    if ( nCount )
    {
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            PropertyInfo& rValue = rSeq[ n ];

            if ( rValue.Name == aPropertyName )
            {
                // Check type.
                if ( rValue.Value.getValueType() != aValue.getValueType() )
                {
                    m_pImpl->m_aMutex.release();
                    throw IllegalArgumentException();
                }

                // Success.

                rValue.Value = aValue;
                rValue.State = PropertyState_DIRECT_VALUE;

                PropertyChangeEvent aEvt;
                if ( m_pImpl->m_pPropertyChangeListeners )
                {
                    aEvt.Source         = (OWeakObject*)this;
                    aEvt.PropertyName   = rValue.Name;
                    aEvt.PropertyHandle = rValue.Handle;
                    aEvt.Further        = sal_False;
                    aEvt.OldValue       = rValue.Value;
                    aEvt.NewValue       = aValue;
                }

                // Callback follows!
                m_pImpl->m_aMutex.release();

                if ( m_pImpl->m_pPropertyChangeListeners )
                    notifyPropertyChangeEvent( aEvt );

                store();
                return;
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

    load();

    const PropertyInfoList_Impl& rSeq = *m_pImpl->m_pValues;
    sal_uInt32 nCount = rSeq.size();

    if ( nCount )
    {
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            const PropertyInfo& rValue = rSeq[ n ];

            if ( rValue.Name == PropertyName )
            {
                // Found.
                return rValue.Value;
            }
        }
    }

    throw UnknownPropertyException();

    // Make MSC4 happy ;-)
    return Any();
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
//  VOS_ENSURE( sal_False,
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
//  VOS_ENSURE( sal_False,
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

    // Check type class ( Not all types can be written to storage )
    TypeClass eTypeClass = DefaultValue.getValueTypeClass();
    if ( eTypeClass == TypeClass_INTERFACE )
        throw IllegalTypeException();

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    load();

    PropertyInfoList_Impl& rSeq = *m_pImpl->m_pValues;
    sal_uInt32 nCount = rSeq.size();

    if ( nCount )
    {
        for ( sal_uInt32 n = 0; n < nCount; ++n )
        {
            PropertyInfo& rValue = rSeq[ n ];

            if ( rValue.Name == Name )
            {
                // Already in set.
                throw PropertyExistException();
            }
        }
    }

    // Property is always removeable.
    Attributes |= PropertyAttribute::REMOVEABLE;

    // Add property.
    rSeq.push_back( PropertyInfo( Name,
                                  -1,
                                  DefaultValue,
                                  PropertyState_DEFAULT_VALUE,
                                  Attributes ) );
    store();

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
}

//=========================================================================
// virtual
void SAL_CALL PersistentPropertySet::removeProperty( const OUString& Name )
    throw( UnknownPropertyException,
           NotRemoveableException,
           RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    load();

    PropertyInfoList_Impl& rSeq = *m_pImpl->m_pValues;
    sal_uInt32 nCount = rSeq.size();

    if ( !nCount )
        throw UnknownPropertyException();

    sal_Bool bFound = sal_False;

    for ( sal_uInt32 n = 0; n < nCount; ++n )
    {
        PropertyInfo& rValue = rSeq[ n ];

        if ( rValue.Name == Name )
        {
            // Found.

            PropertyInfo aValue( rValue );

            if ( !( rValue.Attributes & PropertyAttribute::REMOVEABLE ) )
            {
                // Not removeable!
                throw NotRemoveableException();
            }

            // Remove property from sequence.

            sal_uInt32 nNewCount = nCount - 1;

            if ( n == nNewCount )
            {
                // Remove last element.
                rSeq.pop_back();
            }
            else
            {
                PropertyInfoList_Impl* pNew = new PropertyInfoList_Impl;
                PropertyInfoList_Impl& rNew = *pNew;

                for ( sal_uInt32 k = 0, l = 0; k < nNewCount; ++k, ++l )
                {
                    if ( k == n )
                        l++;

                    rNew.push_back( rSeq[ l ] );
                }

                delete m_pImpl->m_pValues;
                m_pImpl->m_pValues = pNew;
            }

            store();

            // Property set info is invalid.
            if ( m_pImpl->m_pInfo )
                m_pImpl->m_pInfo->reset();

            // Notify propertyset info change listeners.
            if ( m_pImpl->m_pPropSetChangeListeners &&
                  m_pImpl->m_pPropSetChangeListeners->getLength() )
            {
                PropertySetInfoChangeEvent evt(
                                    static_cast< OWeakObject * >( this ),
                                    aValue.Name,
                                    aValue.Handle,
                                    PropertySetInfoChange::PROPERTY_REMOVED );
                notifyPropertySetInfoChange( evt );
            }

            return;
        }
    }

    throw UnknownPropertyException();
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
    const PropertyInfoList_Impl aInfo( getProperties() );
    sal_uInt32 nCount = aInfo.size();
    Sequence< PropertyValue > aValues( nCount );

    PropertyValue* pValues = aValues.getArray();

    for ( sal_uInt32 n = 0; n < nCount; ++n )
        pValues[ n ] = aInfo[ n ];

    return aValues;
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
    // Note: Unknown properties are ignored - UnknownPropertyExecption's
    //       will not be thrown! The specification of this method is buggy
    //       in my opinion. Refer to definition of XMultiProertySet, where
    //       exceptions are specified well.

    m_pImpl->m_aMutex.acquire();

    load();

    PropertyInfoList_Impl& rSeq = *m_pImpl->m_pValues;
    sal_uInt32 nCount = rSeq.size();

    if ( nCount )
    {
        const PropertyValue* pNewValues = aProps.getConstArray();
        sal_uInt32 nNewCount = aProps.getLength();

        typedef std::list< PropertyChangeEvent > Events;
        Events aEvents;

        // Iterate over new property value sequence.
        for ( sal_uInt32 n = 0; n < nNewCount; ++n )
        {
            const PropertyValue& rNewValue = pNewValues[ n ];
            const OUString& rName = rNewValue.Name;

#ifdef _DEBUG
            sal_Bool bFound = sal_False;
#endif
            // Iterate over property value sequence.
            for ( sal_uInt32 k = 0; k < nCount; ++k )
            {
                PropertyInfo& rValue = rSeq[ k ];
                if ( rValue.Name == rName )
                {
                    // type check ?

                    VOS_ENSURE( rNewValue.State == PropertyState_DIRECT_VALUE,
                                "PersistentPropertySet::setPropertyValues - "
                                "Wrong property state!" );

#ifdef _DEBUG
                    bFound = sal_True;
#endif

                    if ( m_pImpl->m_pPropertyChangeListeners )
                    {
                        PropertyChangeEvent aEvt;
                        aEvt.Source         = (OWeakObject*)this;
                        aEvt.PropertyName   = rNewValue.Name;
                        aEvt.PropertyHandle = rNewValue.Handle;
                        aEvt.Further        = sal_False;
                        aEvt.OldValue       = rValue.Value;
                        aEvt.NewValue       = rNewValue.Value;

                        aEvents.push_back( aEvt );
                    }

                    rValue.Name       = rNewValue.Name;
                    rValue.Handle     = rNewValue.Handle;
                    rValue.Value      = rNewValue.Value;
                    rValue.State      = PropertyState_DIRECT_VALUE;
//                  rValue.Attributes = <unchanged>

                    // Process next property to set.
                    break;
                }
            }

            VOS_ENSURE( bFound,
                        "PersistentPropertySet::setPropertyValues - "
                        "Unknown property!" );
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

        store();
        return;
    }

    m_pImpl->m_aMutex.release();

    VOS_ENSURE( sal_False,
                "PersistentPropertySet::setPropertyValues - Nothing set!" );
}

//=========================================================================
//
// Non-interface methods
//
//=========================================================================

// static
PersistentPropertySet* PersistentPropertySet::create(
                            const Reference< XMultiServiceFactory >& rXSMgr,
                            PropertySetRegistry& rCreator,
                            const OUString& rKey,
                            sal_Bool bCreate )
{
    if ( !rKey.getLength() )
        return NULL;

    osl::Guard< osl::Mutex > aGuard( rCreator.getRegistryMutex() );

    storeAccessMode eMode =
                bCreate ? store_AccessReadCreate : store_AccessReadWrite;

    store::OStoreFile& rStore = rCreator.getStoreFile();
    OUString aStreamName( OUString::createFromAscii(
                                            PROPERTYSET_STREAM_PREFIX ) );
    aStreamName += rKey;

    store::OStoreStream aStream;
    storeError nError = aStream.create( rStore,
                                        OUString::createFromAscii( "/" ),
                                        aStreamName,
                                        eMode );

    VOS_ENSURE( ( nError == store_E_None )
                    || ( ( nError == store_E_NotExists )
                                && ( eMode == store_AccessReadWrite ) ),
                "PersistentPropertySet::create - Error!" );

#if 0

    sal_Bool bLookForCHAOSViewProps = sal_False;

    const OUString& rURL = rCreator.getURL();
    if ( rURL.getLength() > 3 )
    {
        // Note: All View Storages ever written by CHAOS have the
        //       filename extension 'scc'.

        const OUString aExtension( rURL.copy( rURL.getLength() - 4 ) );
        if ( aExtension.equalsIgnoreCase(
                            OUString::createFromAscii( ".scc" ) ) )
        {
            if ( ( nError == store_E_NotExists ) &&
                   ( eMode == store_AccessReadWrite ) )
            {
                // Stream does not exist. But look for CHAOS view props.
                bLookForCHAOSViewProps = sal_True;
            }
            else if ( ( nError == store_E_None ) &&
                        ( eMode == store_AccessReadCreate ) )
            {
                sal_uInt32 nSize = 0;
                aStream.getSize( nSize );
                if ( nSize == 0 )
                {
                    // Stream was just created. Look for CHAOS view props.
                    bLookForCHAOSViewProps = sal_True;
                }
            }
        }
    }

    if ( bLookForCHAOSViewProps )
    {
        //////////////////////////////////////////////////////////////
        // Compatibility:
        //      Convert View-Properties from CHAOS-View-Storages.
        //////////////////////////////////////////////////////////////

        PropertyInfoList_Impl aSeq(
                        CntPersistentPropertySet::query( rURL, rKey ) );

        if ( nError == store_E_NotExists )
            nError = aStream.create( rStore,
                                     OUString::createFromAscii( "/" ),
                                     aStreamName,
                                     store_AccessReadCreate );

        if ( nError == store_E_None )
        {
            // Note: Pass the sequence to propset, even if it is empty!
            return new PersistentPropertySet( rXSMgr,
                                              rCreator,
                                              rKey,
                                              aStream,
                                              aSeq );
        }
    }
#endif

    if ( nError == store_E_None )
        return new PersistentPropertySet( rXSMgr,
                                          rCreator,
                                          rKey,
                                          aStream );
    return NULL;
}

//=========================================================================
const PropertyInfoList_Impl& PersistentPropertySet::getProperties()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    load();
    return *m_pImpl->m_pValues;
}

//=========================================================================
sal_Bool PersistentPropertySet::load()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( m_pImpl->m_pValues )
        return sal_True;

    osl::Guard< osl::Mutex > aRegistryGuard(
                                m_pImpl->m_pCreator->getRegistryMutex() );

    //////////////////////////////////////////////////////////////////////
    // Read header.
    //////////////////////////////////////////////////////////////////////

    PropertySetStreamHeader_Impl aHeader;

    sal_uInt32 nBytesRead = 0;
    storeError nError = m_pImpl->m_aStream.readAt(
                                        0,
                                        &aHeader,
                                        PROPERTYSET_STREAM_HEADER_SIZE,
                                        nBytesRead );

    if ( ( nError == store_E_None ) && ( nBytesRead == 0 ) )
    {
        // Not exists.
        m_pImpl->m_pValues = new PropertyInfoList_Impl;
        return sal_True;
    }

    if ( ( nError == store_E_None ) &&
         ( nBytesRead == PROPERTYSET_STREAM_HEADER_SIZE ) )
    {
        //////////////////////////////////////////////////////////////////
        // Check header.
        //////////////////////////////////////////////////////////////////

        if ( ( aHeader.m_nMagic == PROPERTYSET_STREAM_MAGIC ) &&
             ( aHeader.m_nVersion == PROPERTYSET_STREAM_VERSION ) )
        {
            if ( !aHeader.m_nDataLen )
            {
                // Empty.
                m_pImpl->m_pValues = new PropertyInfoList_Impl;
                return sal_True;
            }

            //////////////////////////////////////////////////////////////
            // Read data.
            //////////////////////////////////////////////////////////////

            PropertySetStreamBuffer_Impl aBuffer( aHeader.m_nDataLen );

            nBytesRead = 0;
            nError = m_pImpl->m_aStream.readAt(
                                        PROPERTYSET_STREAM_HEADER_SIZE,
                                        static_cast< sal_uInt8 * >( aBuffer ),
                                        aHeader.m_nDataLen,
                                        nBytesRead );

            if ( ( nError == store_E_None ) &&
                  ( nBytesRead == aHeader.m_nDataLen ) )
            {
                sal_Bool bSuccess = sal_True;

                PropertyInfoList_Impl* pSeq = new PropertyInfoList_Impl;

                for ( sal_uInt32 n = 0; n < aHeader.m_nCount; ++n )
                {
                    //////////////////////////////////////////////////////
                    // Read element.
                    //////////////////////////////////////////////////////

                    //////////////////////////////////////////////////////
                    // data format:
                    //
                    // sal_uInt32 nNameLen;
                    // sal_uInt8* Name;
                    //          ----> OUString      PropertyValue.Name
                    // sal_Int32  Handle;
                    //          ----> sal_Int32     PropertyValue.Handle
                    // sal_Int32  Attributes;
                    //          ----> sal_Int16     PropertyValue.Attributes
                    // sal_Int32  State;
                    //          ----> PropertyState PropertyValue.State
                    // sal_uInt32 nValueLen;
                    // sal_uInt8* Value;
                    //          ----> Any           PropertyValue.Value
                    //////////////////////////////////////////////////////

                    PropertyInfo aValue;

                    bSuccess = aBuffer.readString( aValue.Name );
                    if ( !bSuccess )
                        break;

                    bSuccess = aBuffer.readInt32( aValue.Handle );
                    if ( !bSuccess )
                        break;

                    sal_Int32 nAttributes;
                    bSuccess = aBuffer.readInt32( nAttributes );
                    if ( !bSuccess )
                        break;

                    aValue.Attributes = nAttributes; // sal_Int16 !

                    sal_Int32 nState;
                    bSuccess = aBuffer.readInt32( nState );
                    if ( !bSuccess )
                        break;

                    // enum !
                    aValue.State = static_cast< PropertyState >( nState );

                    bSuccess = aBuffer.readAny( aValue.Value );
                    if ( !bSuccess )
                        break;

                    pSeq->push_back( aValue );
                }

                if ( bSuccess )
                {
                    // Success!
                    m_pImpl->m_pValues = pSeq;
                    return sal_True;
                }
                else
                    delete pSeq;
            }
        }
    }

    VOS_ENSURE( sal_False, "PersistentPropertySet::load - error!" );
    m_pImpl->m_pValues = new PropertyInfoList_Impl;
    return sal_False;
}

//=========================================================================
sal_Bool PersistentPropertySet::store()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    load();

    osl::Guard< osl::Mutex > aRegistryGuard(
                                m_pImpl->m_pCreator->getRegistryMutex() );

    sal_Bool bSuccess = sal_True;

    //////////////////////////////////////////////////////////////////////
    // Create and fill buffer.
    //////////////////////////////////////////////////////////////////////

    const PropertyInfoList_Impl& rSeq = *m_pImpl->m_pValues;
    sal_uInt32 nElements = rSeq.size();

    PropertySetStreamBuffer_Impl aBuffer( 65535 /* Bytes - initial size */ );

    for ( sal_uInt32 n = 0; n < nElements; ++n )
    {
        const PropertyInfo& rValue = rSeq[ n ];

        //////////////////////////////////////////////////////////////////
        // Put element into buffer.
        //////////////////////////////////////////////////////////////////
        // data format: refer to PersistentPropertySet::load().
        //////////////////////////////////////////////////////////////////

        bSuccess = aBuffer.writeString( rValue.Name );
        if ( !bSuccess )
            break;

        bSuccess = aBuffer.writeInt32( rValue.Handle );
        if ( !bSuccess )
            break;

        bSuccess = aBuffer.writeInt32( rValue.Attributes );
        if ( !bSuccess )
            break;

        bSuccess = aBuffer.writeInt32( rValue.State );
        if ( !bSuccess )
            break;

        bSuccess = aBuffer.writeAny( rValue.Value );
        if ( !bSuccess )
            break;
    }

    if ( bSuccess )
    {
        sal_uInt32 nDataBytes = aBuffer.getDataLength();

        //////////////////////////////////////////////////////////////////
        // Write header.
        //////////////////////////////////////////////////////////////////

        PropertySetStreamHeader_Impl aHeader( PROPERTYSET_STREAM_MAGIC,
                                                  PROPERTYSET_STREAM_VERSION,
                                                  nDataBytes,
                                                  nElements );
        sal_uInt32 nBytesWritten;
        storeError nError = m_pImpl->m_aStream.writeAt(
                                        0,
                                        &aHeader,
                                        PROPERTYSET_STREAM_HEADER_SIZE,
                                        nBytesWritten );

        if ( ( nError == store_E_None ) &&
              ( nBytesWritten == PROPERTYSET_STREAM_HEADER_SIZE ) )
        {
            if ( !nDataBytes )
            {
                // Empty.
                return sal_True;
            }

            //////////////////////////////////////////////////////////////
            // Write data.
            //////////////////////////////////////////////////////////////

            nError = m_pImpl->m_aStream.writeAt(
                                    PROPERTYSET_STREAM_HEADER_SIZE,
                                    static_cast< sal_uInt8 * >( aBuffer ),
                                          nDataBytes,
                                    nBytesWritten );

            if ( ( nError == store_E_None ) && ( nBytesWritten == nDataBytes ) )
                return sal_True;
        }
    }

    VOS_ENSURE( sal_False, "PersistentPropertySet::store - error!" );
    return sal_False;
}

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
//=========================================================================
//
// PropertySetStreamBuffer_Impl Implementation.
//
//=========================================================================
//=========================================================================

PropertySetStreamBuffer_Impl::PropertySetStreamBuffer_Impl(
                            sal_uInt32 nInitSize, sal_uInt32 nGrowSize )
: m_nSize( nInitSize ),
  m_nGrow( nGrowSize )
{
  m_pBuffer = static_cast< sal_uInt8 * >( rtl_allocateMemory( m_nSize ) );
  m_pPos    = m_pBuffer;
}

//=========================================================================
PropertySetStreamBuffer_Impl::~PropertySetStreamBuffer_Impl()
{
    rtl_freeMemory( m_pBuffer );
}

//=========================================================================
sal_Bool PropertySetStreamBuffer_Impl::readString( OUString& rValue )
{
    // Read sal_Int32 -> data length.
    sal_Int32 nLen = 0;
    readInt32( nLen );

    // Read data bytes -> UTF8 encoded string as byte array.
    ensureCapacity( nLen );
    rValue = OUString( reinterpret_cast< const sal_Char * >( m_pPos ),
                       nLen,
                       RTL_TEXTENCODING_UTF8 );
    m_pPos += nLen;

    // Align buffer position.
    sal_uInt32 nAlignment = ALIGN_POS( nLen );
    ensureCapacity( nAlignment );
    m_pPos += nAlignment;

    return sal_True;
}

//=========================================================================
sal_Bool PropertySetStreamBuffer_Impl::readInt32( sal_Int32& rValue )
{
    // Read sal_Int32.
    ensureCapacity( sizeof( sal_Int32 ) );
    rtl_copyMemory( &rValue, m_pPos, sizeof( sal_Int32 ) );
    m_pPos += sizeof( sal_Int32 );

#ifdef OSL_BIGENDIAN
    rValue = VOS_SWAPDWORD( rValue );
#endif

    return sal_True;
}

//=========================================================================
sal_Bool PropertySetStreamBuffer_Impl::readAny( Any& rValue )
{
    // Read sal_Int32 -> data length.
    sal_Int32 nLen = 0;
    readInt32( nLen );

    if ( nLen )
    {
        // Read data bytes -> Any as byte array.
        ensureCapacity( nLen );

        Sequence< sal_Int8 > aSeq( nLen );
        sal_Int8* pData = aSeq.getArray();
        for ( sal_uInt32 n = 0; n < nLen; ++n )
        {
            pData[ n ] = *m_pPos;
            m_pPos++;
        }

        // Create Any from byte array.
        rValue = anyDeserialize( aSeq );

        // Align buffer position.
        sal_uInt32 nAlignment = ALIGN_POS( nLen );
        ensureCapacity( nAlignment );
        m_pPos += nAlignment;
    }

    return sal_True;
}

//=========================================================================
sal_Bool PropertySetStreamBuffer_Impl::writeString( const OUString& rValue )
{
    const OString aValue(
            rValue.getStr(), rValue.getLength(), RTL_TEXTENCODING_UTF8 );
    sal_uInt32 nLen = aValue.getLength();

    // Write sal_uInt32 -> data length.
    writeInt32( nLen );

    // Write data bytes -> UTF8 encoded string as byte array.
    ensureCapacity( nLen );
    rtl_copyMemory( m_pPos, aValue.getStr(), nLen );
    m_pPos += aValue.getLength();

    // Align buffer position.
    sal_uInt32 nAlignment = ALIGN_POS( nLen );
    ensureCapacity( nAlignment );
    m_pPos += nAlignment;

    return sal_True;
}

//=========================================================================
sal_Bool PropertySetStreamBuffer_Impl::writeInt32( sal_Int32 nValue )
{
    // Write sal_Int32.

#ifdef OSL_BIGENDIAN
    nValue = VOS_SWAPDWORD( nValue );
#endif

    ensureCapacity( sizeof( sal_Int32 ) );
    rtl_copyMemory( m_pPos, &nValue, sizeof( sal_Int32 ) );
    m_pPos += sizeof( sal_Int32 );

    return sal_True;
}

//=========================================================================
sal_Bool PropertySetStreamBuffer_Impl::writeAny( const Any& rValue )
{
    // Convert Any to byte sequence.
    Sequence< sal_Int8 > aSeq( anySerialize( rValue ) );

    sal_uInt32 nLen = aSeq.getLength();

    // Write sal_uInt32 -> data length.
    writeInt32( nLen );

    if ( nLen )
    {
        // Write data -> Any as byte array.
        ensureCapacity( nLen );

        const sal_Int8* pData = aSeq.getConstArray();
        for ( sal_uInt32 n = 0; n < nLen; ++n )
        {
            *m_pPos = pData[ n ];
            m_pPos++;
        }

        // Align buffer position.
        sal_uInt32 nAlignment = ALIGN_POS( nLen );
        ensureCapacity( nAlignment );
        m_pPos += nAlignment;
    }

    return sal_True;
}

//=========================================================================
void PropertySetStreamBuffer_Impl::ensureCapacity( sal_uInt8 nBytesNeeded )
{
    if ( ( m_pPos + nBytesNeeded ) > ( m_pBuffer + m_nSize ) )
    {
        sal_uInt32 nPosDelta = m_pPos - m_pBuffer;

        m_pBuffer = static_cast< sal_uInt8 * >(
                            rtl_reallocateMemory( m_pBuffer,
                                                  m_nSize + m_nGrow ) );
        m_pPos = m_pBuffer + nPosDelta;
        m_nSize += m_nGrow;
    }
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
        const PropertyInfoList_Impl& rSeq = m_pOwner->getProperties();
        sal_uInt32 nCount = rSeq.size();

        Sequence< Property >* pPropSeq = new Sequence< Property >( nCount );

        if ( nCount )
        {
            Property* pProps = pPropSeq->getArray();
            for ( sal_uInt32 n = 0; n < nCount; ++n )
            {
                const PropertyInfo& rValue = rSeq[ n ];
                Property& rProp = pProps[ n ];

                rProp.Name       = rValue.Name;
                rProp.Handle     = rValue.Handle;
                rProp.Type       = rValue.Value.getValueType();
                rProp.Attributes = rValue.Attributes;
            }
        }

        m_pProps = pPropSeq;
    }

    return *m_pProps;
}

//=========================================================================
// virtual
Property SAL_CALL PropertySetInfo_Impl::getPropertyByName(
                                                    const OUString& aName )
    throw( UnknownPropertyException, RuntimeException )
{
    Property aProp;
    if ( queryProperty( aName, aProp ) )
        return aProp;

    throw UnknownPropertyException();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL PropertySetInfo_Impl::hasPropertyByName(
                                                    const OUString& Name )
    throw( RuntimeException )
{
    Property aProp;
    return queryProperty( Name, aProp );
}

//=========================================================================
sal_Bool PropertySetInfo_Impl::queryProperty(
                                const OUString& aName, Property& rProp  )
{
    const PropertyInfoList_Impl& rSeq = m_pOwner->getProperties();
    sal_uInt32 nCount = rSeq.size();
    for ( sal_uInt32 n = 0; n < nCount; ++n )
    {
        const PropertyInfo& rValue = rSeq[ n ];
        if ( rValue.Name == aName )
        {
            rProp.Name       = rValue.Name;
            rProp.Handle     = rValue.Handle;
            rProp.Type       = rValue.Value.getValueType();
            rProp.Attributes = rValue.Attributes;

            return sal_True;
        }
    }

    return sal_False;
}

