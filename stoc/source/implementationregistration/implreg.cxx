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

#include <stdlib.h>
#include <string.h>
#include <list>

#include <boost/noncopyable.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

#include <uno/mapping.hxx>
#include <osl/thread.h>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/process.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/registry/XImplementationRegistration2.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include "mergekeys.hxx"

#if defined(SAL_W32)
#include <io.h>
#else
#include <unistd.h>
#endif


using namespace com::sun::star;
using namespace css::uno;
using namespace css::loader;
using namespace css::beans;
using namespace css::lang;
using namespace css::registry;
using namespace cppu;
using namespace osl;

namespace {

struct StringPool: private boost::noncopyable
{
    OUString slash_UNO_slash_REGISTRY_LINKS;
    OUString slash_IMPLEMENTATIONS;
    OUString slash_UNO;
    OUString slash_UNO_slash_SERVICES;
    OUString slash_UNO_slash_SINGLETONS;
    OUString slash_SERVICES;
    OUString slash_UNO_slash_LOCATION;
    OUString slash_UNO_slash_ACTIVATOR;
    OUString colon_old;
    OUString com_sun_star_registry_SimpleRegistry;
    OUString Registry;
    StringPool()
        : slash_UNO_slash_REGISTRY_LINKS( "/UNO/REGISTRY_LINKS")
        , slash_IMPLEMENTATIONS( "/IMPLEMENTATIONS" )
        , slash_UNO( "/UNO")
        , slash_UNO_slash_SERVICES( "/UNO/SERVICES")
        , slash_UNO_slash_SINGLETONS( "/UNO/SINGLETONS")
        , slash_SERVICES( "/SERVICES/" )
        , slash_UNO_slash_LOCATION( "/UNO/LOCATION" )
        , slash_UNO_slash_ACTIVATOR( "/UNO/ACTIVATOR" )
        , colon_old( ":old")
        , com_sun_star_registry_SimpleRegistry("com.sun.star.registry.SimpleRegistry" )
        , Registry( "Registry" )
        {}
};

const StringPool &spool()
{
    static StringPool *pPool = nullptr;
    if( ! pPool )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pPool )
        {
            static StringPool pool;
            pPool = &pool;
        }
    }
    return *pPool;
}


//  static deleteAllLinkReferences()

static void deleteAllLinkReferences(const Reference < XSimpleRegistry >& xReg,
                                    const Reference < XRegistryKey >& xSource)
    // throw ( InvalidRegistryException, RuntimeException )
{
    Reference < XRegistryKey > xKey = xSource->openKey(
        spool().slash_UNO_slash_REGISTRY_LINKS );

    if (xKey.is() && (xKey->getValueType() == RegistryValueType_ASCIILIST))
    {
        Sequence<OUString> linkNames = xKey->getAsciiListValue();

        if (linkNames.getLength())
        {
            const OUString* pLinkNames = linkNames.getConstArray();

            OUString            aLinkName;
            OUString            aLinkParent;
            Reference < XRegistryKey >  xLinkParent;
            const sal_Unicode*  pTmpName = nullptr;
            const sal_Unicode*  pShortName = nullptr;
            sal_Int32           sEnd = 0;

            for (sal_Int32 i = 0; i < linkNames.getLength(); i++)
            {
                aLinkName = pLinkNames[i];

                pTmpName = aLinkName.getStr();

                if (pTmpName[0] != L'/')
                    continue;

                sal_Int32 nIndex = rtl_ustr_indexOfChar( pTmpName, '%' );
                if ( nIndex == -1 )
                    pShortName = nullptr;
                else
                    pShortName = pTmpName+nIndex;

                while (pShortName && pShortName[1] == L'%')
                {
                    nIndex = rtl_ustr_indexOfChar( pShortName+2, '%' );
                    if ( nIndex == -1 )
                        pShortName = nullptr;
                    else
                        pShortName += nIndex+2;
                }

                if (pShortName)
                {
                    aLinkName = aLinkName.copy(0, pShortName - pTmpName);
                }

                xReg->getRootKey()->deleteLink(aLinkName);

                sEnd = rtl_ustr_lastIndexOfChar( aLinkName.getStr(), '/' );

                aLinkParent = aLinkName.copy(0, sEnd);

                while(!aLinkParent.isEmpty())
                {
                    xLinkParent = xReg->getRootKey()->openKey(aLinkParent);

                    if (xLinkParent.is() && (xLinkParent->getKeyNames().getLength() == 0))
                    {
                        aLinkName = aLinkParent;

                        xReg->getRootKey()->deleteKey(aLinkParent);

                        sEnd = rtl_ustr_lastIndexOfChar( aLinkName.getStr(), '/' );

                        aLinkParent = aLinkName.copy(0, sEnd);
                    } else
                    {
                        break;
                    }
                }
            }
        }
    }
}


//  static prepareLink

static void prepareLink( const Reference < XSimpleRegistry > & xDest,
                         const Reference < XRegistryKey > & xSource,
                         const OUString& link)
    // throw ( InvalidRegistryException, RuntimeException )
{
    OUString linkRefName = xSource->getKeyName();
    OUString linkName(link);
    bool    isRelativ = false;

    const sal_Unicode*  pTmpName = link.getStr();
    const sal_Unicode*  pShortName;
    sal_Int32           nIndex = rtl_ustr_indexOfChar( pTmpName, '%' );
    if ( nIndex == -1 )
        pShortName = nullptr;
    else
        pShortName = pTmpName+nIndex;

    if (pTmpName[0] != L'/')
        isRelativ = true;

    while (pShortName && pShortName[1] == L'%')
    {
        nIndex = rtl_ustr_indexOfChar( pShortName+2, '%' );
        if ( nIndex == -1 )
            pShortName = nullptr;
        else
            pShortName += nIndex+2;
    }

    if (pShortName)
    {
        linkRefName = linkRefName + link.copy(pShortName - pTmpName + 1);
        linkName = link.copy(0, pShortName - pTmpName);
    }

    if (isRelativ)
        xSource->createLink(linkName, linkRefName);
    else
        xDest->getRootKey()->createLink(linkName, linkRefName);
}


//  static searchImplForLink

static OUString searchImplForLink(
    const Reference < XRegistryKey > & xRootKey,
    const OUString& linkName,
    const OUString& implName )
    // throw ( InvalidRegistryException, RuntimeException )
{
    const StringPool & pool = spool();
    Reference < XRegistryKey > xKey = xRootKey->openKey( pool.slash_IMPLEMENTATIONS );
    if (xKey.is())
    {
        Sequence< Reference < XRegistryKey > > subKeys( xKey->openKeys() );
        const Reference < XRegistryKey > * pSubKeys = subKeys.getConstArray();
        OUString key_name( pool.slash_UNO + linkName );

        for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
        {
            try
            {
                Reference < XRegistryKey > xImplKey( pSubKeys[i] );
                if (xImplKey->getKeyType( key_name ) == RegistryKeyType_LINK)
                {
                    OUString oldImplName = xImplKey->getKeyName().copy(strlen("/IMPLEMENTATIONS/"));
                    if (implName != oldImplName)
                    {
                        return oldImplName;
                    }
                }
            }
            catch(InvalidRegistryException&)
            {
            }
        }
    }

    return OUString();
}


//  static searchLinkTargetForImpl

static OUString searchLinkTargetForImpl(const Reference < XRegistryKey >& xRootKey,
                                        const OUString& linkName,
                                        const OUString& implName)
    // throw ( InvalidRegistryException, RuntimeException )
{
    OUString ret;

//      try
//      {
        const StringPool & pool = spool();
        Reference < XRegistryKey > xKey = xRootKey->openKey( pool.slash_IMPLEMENTATIONS );

        if (xKey.is())
        {
            Sequence< Reference < XRegistryKey > > subKeys = xKey->openKeys();

            const Reference < XRegistryKey >* pSubKeys = subKeys.getConstArray();
            Reference < XRegistryKey > xImplKey;

            for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
            {
                xImplKey = pSubKeys[i];

                OUString tmpImplName = xImplKey->getKeyName().copy(strlen("/IMPLEMENTATIONS/"));
                OUString qualifiedLinkName( pool.slash_UNO );
                qualifiedLinkName += linkName;
                if (tmpImplName == implName &&
                    xImplKey->getKeyType( qualifiedLinkName ) == RegistryKeyType_LINK)
                {
                    return xImplKey->getLinkTarget( qualifiedLinkName );
                }
            }
        }
//      }
//      catch(InvalidRegistryException&)
//      {
//      }

    return ret;
}


//  static createUniqueSubEntry

static void createUniqueSubEntry(const Reference < XRegistryKey > & xSuperKey,
                                 const OUString& value)
    // throw ( InvalidRegistryException, RuntimeException )
{
    if (xSuperKey.is())
    {
//          try
//          {
        if (xSuperKey->getValueType() == RegistryValueType_ASCIILIST)
        {
            sal_Int32 length = 0;
            bool bReady = false;

            Sequence<OUString> implEntries = xSuperKey->getAsciiListValue();
            length = implEntries.getLength();

            for (sal_Int32 i = 0; !bReady && (i < length); i++)
            {
                bReady = (implEntries.getConstArray()[i] == value);
            }

            if (bReady)
            {
                Sequence<OUString> implEntriesNew(length);
                implEntriesNew.getArray()[0] = value;

                for (sal_Int32 i=0, j=1; i < length; i++)
                {
                    if (implEntries.getConstArray()[i] != value)
                        implEntriesNew.getArray()[j++] = implEntries.getConstArray()[i];
                }
                xSuperKey->setAsciiListValue(implEntriesNew);
            } else
            {
                Sequence<OUString> implEntriesNew(length+1);
                implEntriesNew.getArray()[0] = value;

                for (sal_Int32 i = 0; i < length; i++)
                {
                    implEntriesNew.getArray()[i+1] = implEntries.getConstArray()[i];
                }
                xSuperKey->setAsciiListValue(implEntriesNew);
            }
        } else
        {
            Sequence<OUString> implEntriesNew { value };

            xSuperKey->setAsciiListValue(implEntriesNew);
        }
//          }
//          catch(InvalidRegistryException&)
//          {
//          }
    }
}


//  static deleteSubEntry

static bool deleteSubEntry(const Reference < XRegistryKey >& xSuperKey, const OUString& value)
    // throw ( InvalidRegistryException, RuntimeException )
{
    if (xSuperKey->getValueType() == RegistryValueType_ASCIILIST)
    {
        Sequence<OUString> implEntries = xSuperKey->getAsciiListValue();
        sal_Int32 length = implEntries.getLength();
        sal_Int32 equals = 0;
        bool hasNoImplementations = false;

        for (sal_Int32 i = 0; i < length; i++)
        {
            if (implEntries.getConstArray()[i] == value)
                equals++;
        }

        if (equals == length)
        {
            hasNoImplementations = true;
        } else
        {
            Sequence<OUString> implEntriesNew(length - equals);

            sal_Int32 j = 0;
            for (sal_Int32 i = 0; i < length; i++)
            {
                if (implEntries.getConstArray()[i] != value)
                {
                        implEntriesNew.getArray()[j++] = implEntries.getConstArray()[i];
                }
            }
            xSuperKey->setAsciiListValue(implEntriesNew);
        }

        if (hasNoImplementations)
        {
            return true;
        }
    }
    return false;
}


//  static prepareUserLink

static void prepareUserLink(const Reference < XSimpleRegistry >& xDest,
                                const OUString& linkName,
                                const OUString& linkTarget,
                                const OUString& implName)
{
    Reference < XRegistryKey > xRootKey;

    xRootKey = xDest->getRootKey();

    if (xRootKey->getKeyType(linkName) == RegistryKeyType_LINK)
    {
        OUString oldImplName(searchImplForLink(xRootKey, linkName, implName));

        if (!oldImplName.isEmpty())
        {
            createUniqueSubEntry(xDest->getRootKey()->createKey(
                linkName + spool().colon_old ), oldImplName);
        }
    }

    if (xRootKey->isValid())
        xRootKey->createLink(linkName, linkTarget);
}


//  static deleteUserLink

static void deletePathIfPossible(const Reference < XRegistryKey >& xRootKey,
                                 const OUString& path)
{
    try
    {
        Sequence<OUString> keyNames(xRootKey->openKey(path)->getKeyNames());

        if (keyNames.getLength() == 0 &&
            xRootKey->openKey(path)->getValueType() == RegistryValueType_NOT_DEFINED)
        {
            xRootKey->deleteKey(path);

            OUString tmpPath(path);
            OUString newPath = tmpPath.copy(0, tmpPath.lastIndexOf('/'));

            if (newPath.getLength() > 1)
                deletePathIfPossible(xRootKey, newPath);
        }
    }
    catch(InvalidRegistryException&)
    {
    }
}



//  static deleteUserLink

static void deleteUserLink(const Reference < XRegistryKey >& xRootKey,
                               const OUString& linkName,
                               const OUString& linkTarget,
                               const OUString& implName)
    // throw ( InvalidRegistryException, RuntimeException )
{
    bool bClean = false;

    if (xRootKey->getKeyType(linkName) == RegistryKeyType_LINK)
    {
        OUString tmpTarget = xRootKey->getLinkTarget(linkName);

        if (tmpTarget == linkTarget)
        {
            xRootKey->deleteLink(linkName);
        }
    }

    Reference < XRegistryKey > xOldKey = xRootKey->openKey(
        linkName + spool().colon_old );
    if (xOldKey.is())
    {
        if (xOldKey->getValueType() == RegistryValueType_ASCIILIST)
        {
            Sequence<OUString> implEntries = xOldKey->getAsciiListValue();
            sal_Int32 length = implEntries.getLength();
            sal_Int32 equals = 0;
            bool hasNoImplementations = false;

            for (sal_Int32 i = 0; i < length; i++)
            {
                if (implEntries.getConstArray()[i] == implName)
                    equals++;
            }

            if (equals == length)
            {
                hasNoImplementations = true;
            } else
            {
                OUString oldImpl;

                if (length > equals + 1)
                {
                    Sequence<OUString> implEntriesNew(length - equals - 1);

                    sal_Int32 j = 0;
                    bool first = true;
                    for (sal_Int32 i = 0; i < length; i++)
                    {
                        if (implEntries.getConstArray()[i] != implName)
                        {
                            if (first)
                            {
                                oldImpl = implEntries.getConstArray()[i];
                                first = false;
                            } else
                            {
                                implEntriesNew.getArray()[j++] = implEntries.getConstArray()[i];
                            }
                        }
                    }

                    xOldKey->setAsciiListValue(implEntriesNew);
                } else
                {
                    oldImpl = implEntries.getConstArray()[0];
                    OUString path(xOldKey->getKeyName());
                    xOldKey->closeKey();
                    xRootKey->deleteKey(path);
                }

                OUString oldTarget = searchLinkTargetForImpl(xRootKey, linkName, oldImpl);
                if (!oldTarget.isEmpty())
                {
                    xRootKey->createLink(linkName, oldTarget);
                }
            }

            if (hasNoImplementations)
            {
                bClean = true;
                OUString path(xOldKey->getKeyName());
                xOldKey->closeKey();
                xRootKey->deleteKey(path);
            }
        }
    } else
    {
        bClean = true;
    }

    if (bClean)
    {
        OUString tmpName(linkName);
        OUString path = tmpName.copy(0, tmpName.lastIndexOf('/'));
        deletePathIfPossible(xRootKey, path);
    }
}


//  static prepareUserKeys

static void prepareUserKeys(const Reference < XSimpleRegistry >& xDest,
                                const Reference < XRegistryKey >& xUnoKey,
                                const Reference < XRegistryKey >& xKey,
                                const OUString& implName,
                                bool bRegister)
{
    bool hasSubKeys = false;

    Sequence<OUString> keyNames = xKey->getKeyNames();

    OUString relativKey;
    if (keyNames.getLength())
        relativKey = keyNames.getConstArray()[0].copy(xKey->getKeyName().getLength()+1);

    if (keyNames.getLength() == 1 &&
        xKey->getKeyType(relativKey) == RegistryKeyType_LINK)
    {
        hasSubKeys = true;

        OUString linkTarget = xKey->getLinkTarget(relativKey);
        OUString linkName(xKey->getKeyName().copy(xUnoKey->getKeyName().getLength()));

        linkName = linkName + "/" + relativKey;

        if (bRegister)
        {
            prepareUserLink(xDest, linkName, linkTarget, implName);
        } else
        {
            deleteUserLink(xDest->getRootKey(), linkName, linkTarget, implName);
        }
    } else
    {
        Sequence< Reference < XRegistryKey> > subKeys = xKey->openKeys();

        if (subKeys.getLength())
        {
            hasSubKeys = true;
            const Reference < XRegistryKey > * pSubKeys = subKeys.getConstArray();

            for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
            {
                prepareUserKeys(xDest, xUnoKey, pSubKeys[i], implName, bRegister);
            }
        }
    }

    if (! hasSubKeys)
    {
        OUString keyName(xKey->getKeyName().copy(xUnoKey->getKeyName().getLength()));

        Reference < XRegistryKey > xRootKey = xDest->getRootKey();
        if (bRegister)
        {
            createUniqueSubEntry(xRootKey->createKey(keyName), implName);
        }
        else
        {
            Reference< XRegistryKey > rKey = xRootKey->openKey(keyName);
            if( rKey.is() )
            {
                deleteSubEntry(rKey, implName);
                xRootKey->deleteKey(keyName);
            }

            OUString path = keyName.copy(0, keyName.lastIndexOf('/'));
            if( !path.isEmpty() )
            {
                deletePathIfPossible(xRootKey, path);
            }
        }
    }
}


//  static deleteAllImplementations

static void deleteAllImplementations(   const Reference < XSimpleRegistry >& xReg,
                                        const Reference < XRegistryKey >& xSource,
                                        const OUString& locationUrl,
                                        std::list<OUString> & implNames)
    // throw (InvalidRegistryException, RuntimeException)
{
    Sequence < Reference < XRegistryKey > > subKeys = xSource->openKeys();

    if (subKeys.getLength() > 0)
    {
        const Reference < XRegistryKey> * pSubKeys = subKeys.getConstArray();
        Reference < XRegistryKey > xImplKey;
        bool hasLocationUrl = false;

        const StringPool &pool = spool();
        for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
        {
            xImplKey = pSubKeys[i];
            Reference < XRegistryKey > xKey = xImplKey->openKey(
                pool.slash_UNO_slash_LOCATION );

            if (xKey.is() && (xKey->getValueType() == RegistryValueType_ASCII))
            {
                if (xKey->getAsciiValue() == locationUrl)
                {
                    hasLocationUrl = true;

                    OUString implName(xImplKey->getKeyName().getStr() + 1);
                    sal_Int32 firstDot = implName.indexOf('/');

                    if (firstDot >= 0)
                        implName = implName.copy(firstDot + 1);

                    implNames.push_back(implName);

                    deleteAllLinkReferences(xReg, xImplKey);

                    xKey = xImplKey->openKey( pool.slash_UNO );
                    if (xKey.is())
                    {
                        Sequence< Reference < XRegistryKey > > subKeys2 = xKey->openKeys();

                        if (subKeys2.getLength())
                        {
                            const Reference < XRegistryKey > * pSubKeys2 = subKeys2.getConstArray();

                            for (sal_Int32 j = 0; j < subKeys2.getLength(); j++)
                            {
                                if (pSubKeys2[j]->getKeyName() != (xImplKey->getKeyName() + pool.slash_UNO_slash_SERVICES ) &&
                                    pSubKeys2[j]->getKeyName() != (xImplKey->getKeyName() + pool.slash_UNO_slash_REGISTRY_LINKS ) &&
                                    pSubKeys2[j]->getKeyName() != (xImplKey->getKeyName() + pool.slash_UNO_slash_ACTIVATOR ) &&
                                    pSubKeys2[j]->getKeyName() != (xImplKey->getKeyName() + pool.slash_UNO_slash_SINGLETONS ) &&
                                    pSubKeys2[j]->getKeyName() != (xImplKey->getKeyName() + pool.slash_UNO_slash_LOCATION) )
                                {
                                    prepareUserKeys(xReg, xKey, pSubKeys2[j], implName, false);
                                }
                            }
                        }
                    }
                }
            }

            if (hasLocationUrl)
            {
                hasLocationUrl = false;
                OUString path(xImplKey->getKeyName());
                xImplKey->closeKey();
                xReg->getRootKey()->deleteKey(path);
            }
        }

        subKeys = xSource->openKeys();
        if (subKeys.getLength() == 0)
        {
            OUString path(xSource->getKeyName());
            xSource->closeKey();
            xReg->getRootKey()->deleteKey(path);
        }
    } else
    {
        OUString path(xSource->getKeyName());
        xSource->closeKey();
        xReg->getRootKey()->deleteKey(path);
    }
}


static void delete_all_singleton_entries(
    Reference < registry::XRegistryKey > const & xSingletons_section,
    ::std::list< OUString > const & impl_names )
    // throw (InvalidRegistryException, RuntimeException)
{
    Sequence< Reference< registry::XRegistryKey > > singletons( xSingletons_section->openKeys() );
    Reference< registry::XRegistryKey > const * subkeys = singletons.getConstArray();
    for ( sal_Int32 nPos = singletons.getLength(); nPos--; )
    {
        Reference< registry::XRegistryKey > const & xSingleton = subkeys[ nPos ];
        Reference< registry::XRegistryKey > xRegisteredImplNames(
            xSingleton->openKey( "REGISTERED_BY" ) );
        if (xRegisteredImplNames.is() && xRegisteredImplNames->isValid())
        {
            Sequence< OUString > registered_implnames;
            try
            {
                registered_implnames = xRegisteredImplNames->getAsciiListValue();
            }
            catch (registry::InvalidValueException &)
            {
            }
            OUString const * p = registered_implnames.getConstArray();
            sal_Int32 nOrigRegLength = registered_implnames.getLength();
            sal_Int32 nNewLength = nOrigRegLength;
            for ( sal_Int32 n = nOrigRegLength; n--; )
            {
                OUString const & registered_implname = p[ n ];

                ::std::list< OUString >::const_iterator iPos( impl_names.begin() );
                ::std::list< OUString >::const_iterator const iEnd( impl_names.end() );
                for ( ; iPos != iEnd; ++iPos )
                {
                    if (iPos->equals( registered_implname ))
                    {
                        registered_implnames[ n ] = p[ nNewLength -1 ];
                        --nNewLength;
                    }
                }
            }

            if (nNewLength != nOrigRegLength)
            {
                if (0 == nNewLength)
                {
                    // remove whole entry
                    xRegisteredImplNames->closeKey();
                    xSingleton->deleteKey( "REGISTERED_BY" );
                    // registry key cannot provide its relative name, only absolute :(
                    OUString abs( xSingleton->getKeyName() );
                    xSingletons_section->deleteKey( abs.copy( abs.lastIndexOf( '/' ) +1 ) );
                }
                else
                {
                    registered_implnames.realloc( nNewLength );
                    xRegisteredImplNames->setAsciiListValue( registered_implnames );
                }
            }
        }
    }
}


//  static deleteAllServiceEntries

static void deleteAllServiceEntries(    const Reference < XSimpleRegistry >& xReg,
                                        const Reference < XRegistryKey >& xSource,
                                        const OUString& implName)
    // throw ( InvalidRegistryException, RuntimeException )
{
    Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

    if (subKeys.getLength() > 0)
    {
        const Reference < XRegistryKey > * pSubKeys = subKeys.getConstArray();
        Reference < XRegistryKey > xServiceKey;
        bool hasNoImplementations = false;

        for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
        {
            xServiceKey = pSubKeys[i];

            if (xServiceKey->getValueType() == RegistryValueType_ASCIILIST)
            {
                Sequence<OUString> implEntries = xServiceKey->getAsciiListValue();
                sal_Int32 length = implEntries.getLength();
                sal_Int32 equals = 0;

                for (sal_Int32 j = 0; j < length; j++)
                {
                    if (implEntries.getConstArray()[j] == implName)
                        equals++;
                }

                if (equals == length)
                {
                    hasNoImplementations = true;
                } else
                {
                    if (equals > 0)
                    {
                        Sequence<OUString> implEntriesNew(length-equals);

                        sal_Int32 j = 0;
                        for (sal_Int32 k = 0; k < length; k++)
                        {
                            if (implEntries.getConstArray()[k] != implName)
                            {
                                implEntriesNew.getArray()[j++] = implEntries.getConstArray()[k];
                            }
                        }

                        xServiceKey->setAsciiListValue(implEntriesNew);
                    }
                }
            }

            if (hasNoImplementations)
            {
                hasNoImplementations = false;
                OUString path(xServiceKey->getKeyName());
                xServiceKey->closeKey();
                xReg->getRootKey()->deleteKey(path);
            }
        }

        subKeys = xSource->openKeys();
        if (subKeys.getLength() == 0)
        {
            OUString path(xSource->getKeyName());
            xSource->closeKey();
            xReg->getRootKey()->deleteKey(path);
        }
    } else
    {
        OUString path(xSource->getKeyName());
        xSource->closeKey();
        xReg->getRootKey()->deleteKey(path);
    }
}


static bool is_supported_service(
    OUString const & service_name,
    Reference< reflection::XServiceTypeDescription > const & xService_td )
{
    if (xService_td->getName().equals( service_name ))
        return true;
    Sequence< Reference< reflection::XServiceTypeDescription > > seq(
        xService_td->getMandatoryServices() );
    Reference< reflection::XServiceTypeDescription > const * p = seq.getConstArray();
    for ( sal_Int32 nPos = seq.getLength(); nPos--; )
    {
        if (is_supported_service( service_name, p[ nPos ] ))
            return true;
    }
    return false;
}


static void insert_singletons(
    Reference< registry::XSimpleRegistry > const & xDest,
    Reference< registry::XRegistryKey > const & xImplKey,
    Reference< XComponentContext > const & xContext )
    // throw( registry::InvalidRegistryException, registry::CannotRegisterImplementationException, RuntimeException )
{
    // singletons
    Reference< registry::XRegistryKey > xKey( xImplKey->openKey( "UNO/SINGLETONS" ) );
    if (xKey.is() && xKey->isValid())
    {
        OUString implname( xImplKey->getKeyName().copy( sizeof ("/IMPLEMENTATIONS/") -1 ) );
        // singleton entries
        Sequence< Reference< registry::XRegistryKey > > xSingletons_section( xKey->openKeys() );
        Reference< registry::XRegistryKey > const * p = xSingletons_section.getConstArray();
        for ( sal_Int32 nPos = xSingletons_section.getLength(); nPos--; )
        {
            Reference< registry::XRegistryKey > const & xSingleton = p[ nPos ];
            OUString singleton_name(
                xSingleton->getKeyName().copy(
                    implname.getLength() + sizeof ("/IMPLEMENTATIONS//UNO/SINGLETONS/") -1 ) );
            OUString service_name( xSingleton->getStringValue() );

            OUString keyname( "/SINGLETONS/" + singleton_name );
            Reference< registry::XRegistryKey > xKey2( xDest->getRootKey()->openKey( keyname ) );
            if (xKey2.is() && xKey2->isValid())
            {
                try
                {
                    OUString existing_name( xKey2->getStringValue() );
                    if (! existing_name.equals( service_name ))
                    {
                        Reference< container::XHierarchicalNameAccess > xTDMgr;
                        OUString the_tdmgr =
                            "/singletons/com.sun.star.reflection.theTypeDescriptionManager";
                        xContext->getValueByName( the_tdmgr ) >>= xTDMgr;
                        if (! xTDMgr.is())
                        {
                            throw RuntimeException( "cannot get singleton " + the_tdmgr );
                        }
                        try
                        {
                            Reference< reflection::XServiceTypeDescription > xExistingService_td;
                            xTDMgr->getByHierarchicalName( existing_name ) >>= xExistingService_td;
                            if (! xExistingService_td.is())
                            {
                                throw RuntimeException( "cannot get service type description: " + existing_name );
                            }

                            // everything's fine if existing service entry supports the one
                            // to be registered
                            if (! is_supported_service( service_name, xExistingService_td ))
                            {
                                OUStringBuffer buf( 64 );
                                buf.append( "existing singleton service (" );
                                buf.append( singleton_name );
                                buf.append( '=' );
                                buf.append( existing_name );
                                buf.append( ") does not support given one: " );
                                buf.append( service_name );
                                throw registry::CannotRegisterImplementationException(
                                    buf.makeStringAndClear() );
                            }
                        }
                        catch (const container::NoSuchElementException & exc)
                        {
                            throw RuntimeException(
                                "cannot get service type description: " + exc.Message );
                        }
                    }
                }
                catch (registry::InvalidValueException &)
                {
                    // repair
                    xKey2->setStringValue( service_name );
                }
            }
            else
            {
                // insert singleton entry
                xKey2 = xDest->getRootKey()->createKey( keyname );
                xKey2->setStringValue( service_name );
            }

            Reference< registry::XRegistryKey > xRegisteredImplNames(
                xKey2->openKey( "REGISTERED_BY" ) );
            if (!xRegisteredImplNames.is() || !xRegisteredImplNames->isValid())
            {
                // create
                xRegisteredImplNames = xKey2->createKey( "REGISTERED_BY" );
            }

            Sequence< OUString > implnames;
            try
            {
                implnames = xRegisteredImplNames->getAsciiListValue();
            }
            catch (registry::InvalidValueException &)
            {
            }
            // check implname is already in
            sal_Int32 nPos_implnames = implnames.getLength();
            OUString const * pImplnames = implnames.getConstArray();
            while (nPos_implnames--)
            {
                if (implname.equals( pImplnames[ nPos_implnames ] ))
                    break;
            }
            if (nPos_implnames < 0)
            {
                // append and write back
                implnames.realloc( implnames.getLength() +1 );
                implnames[ implnames.getLength() -1 ] = implname;
                xRegisteredImplNames->setAsciiListValue( implnames );
            }
        }
    }
}



//  static prepareRegistry

static void prepareRegistry(
    const Reference < XSimpleRegistry >& xDest,
    const Reference < XRegistryKey >& xSource,
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    Reference< XComponentContext > const & xContext )
    // throw ( InvalidRegistryException, CannotRegisterImplementationException, RuntimeException )
{
    Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

    if (!subKeys.getLength())
    {
        throw InvalidRegistryException(
            "prepareRegistry(): source registry is empty" );
    }

    const StringPool & pool = spool();

    const Reference < XRegistryKey >* pSubKeys = subKeys.getConstArray();
    Reference < XRegistryKey > xImplKey;

    for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
    {
        xImplKey = pSubKeys[i];

        Reference < XRegistryKey >  xKey = xImplKey->openKey(
            pool.slash_UNO_slash_SERVICES );

        if (xKey.is())
        {
            // update entries in SERVICES section
            Sequence< Reference < XRegistryKey > > serviceKeys = xKey->openKeys();
            const Reference < XRegistryKey > * pServiceKeys = serviceKeys.getConstArray();

            OUString implName = OUString(xImplKey->getKeyName().getStr() + 1);
            sal_Int32 firstDot = implName.indexOf('/');

            if (firstDot >= 0)
                implName = implName.copy(firstDot + 1);

            sal_Int32 offset = xKey->getKeyName().getLength() + 1;

            for (sal_Int32 j = 0; j < serviceKeys.getLength(); j++)
            {
                OUString serviceName = pServiceKeys[j]->getKeyName().copy(offset);

                createUniqueSubEntry(
                    xDest->getRootKey()->createKey(
                        pool.slash_SERVICES + serviceName ),
                    implName);
            }

            xKey = xImplKey->openKey( pool.slash_UNO );
            if (xKey.is())
            {
                Sequence< Reference < XRegistryKey > > subKeys2 = xKey->openKeys();

                if (subKeys2.getLength())
                {
                    const Reference < XRegistryKey > * pSubKeys2 = subKeys2.getConstArray();

                    for (sal_Int32 j = 0; j < subKeys2.getLength(); j++)
                    {
                        if (pSubKeys2[j]->getKeyName() != (xImplKey->getKeyName() + pool.slash_UNO_slash_SERVICES) &&
                            pSubKeys2[j]->getKeyName() != (xImplKey->getKeyName() + pool.slash_UNO_slash_REGISTRY_LINKS ) &&
                            pSubKeys2[j]->getKeyName() != (xImplKey->getKeyName() + pool.slash_UNO_slash_SINGLETONS ))
                        {
                            prepareUserKeys(xDest, xKey, pSubKeys2[j], implName, true);
                        }
                    }
                }
            }
        }

        // update LOCATION entry
        xKey = xImplKey->createKey( pool.slash_UNO_slash_LOCATION );

        if (xKey.is())
        {
            xKey->setAsciiValue(locationUrl);
        }

        // update ACTIVATOR entry
        xKey = xImplKey->createKey( pool.slash_UNO_slash_ACTIVATOR );

        if (xKey.is())
        {
            xKey->setAsciiValue(implementationLoaderUrl);
        }

        xKey = xImplKey->openKey( pool.slash_UNO_slash_SERVICES );

        if (xKey.is() && (xKey->getValueType() == RegistryValueType_ASCIILIST))
        {
            // update link entries in REGISTRY_LINKS section
            Sequence<OUString> linkNames = xKey->getAsciiListValue();

            if (linkNames.getLength())
            {
                const OUString* pLinkNames = linkNames.getConstArray();

                for (sal_Int32 j = 0; j < linkNames.getLength(); j++)
                {
                    prepareLink(xDest, xImplKey, pLinkNames[j]);
                }
            }
        }

        insert_singletons( xDest, xImplKey, xContext );
    }
}


static void findImplementations(    const Reference < XRegistryKey > & xSource,
                                    std::list <OUString>& implNames)
{
    bool isImplKey = false;

    try
    {
        Reference < XRegistryKey > xKey = xSource->openKey(
            spool().slash_UNO_slash_SERVICES );

        if (xKey.is() && (xKey->getKeyNames().getLength() > 0))
        {
            isImplKey = true;

            OUString implName = OUString(xSource->getKeyName().getStr() + 1).replace('/', '.').getStr();
            sal_Int32 firstDot = implName.indexOf('.');

            if (firstDot >= 0)
                implName = implName.copy(firstDot + 1);

            implNames.push_back(implName);
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    if (isImplKey) return;

    try
    {
        Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

        if (subKeys.getLength() > 0)
        {
            const Reference < XRegistryKey >* pSubKeys = subKeys.getConstArray();

            for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
            {
                findImplementations(pSubKeys[i], implNames);
            }

        }
    }
    catch(InvalidRegistryException&)
    {
    }
}


class ImplementationRegistration
    : public WeakImplHelper< XImplementationRegistration2, XServiceInfo, XInitialization >
{
public:
    explicit ImplementationRegistration( const Reference < XComponentContext > & rSMgr );
    virtual ~ImplementationRegistration();

    // XServiceInfo
    OUString                        SAL_CALL getImplementationName() throw(RuntimeException, std::exception) override;
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw(RuntimeException, std::exception) override;
    Sequence< OUString >            SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) override;

    // XImplementationRegistration
    virtual void SAL_CALL registerImplementation(
        const OUString& implementationLoader,
        const OUString& location,
        const Reference < XSimpleRegistry > & xReg)
        throw(  CannotRegisterImplementationException, RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL revokeImplementation(
        const OUString& location,
        const Reference < XSimpleRegistry >& xReg)
        throw( RuntimeException, std::exception ) override;

    virtual Sequence< OUString > SAL_CALL getImplementations(
        const OUString& implementationLoader,
        const OUString& location)
        throw( RuntimeException, std::exception ) override;
    virtual Sequence< OUString > SAL_CALL checkInstantiation(
        const OUString& implementationName)
        throw( RuntimeException, std::exception ) override;

    // XImplementationRegistration2
    virtual void SAL_CALL registerImplementationWithLocation(
        const OUString& implementationLoader,
        const OUString& location,
        const OUString& registeredLocation,
        const Reference < XSimpleRegistry > & xReg)
        throw(  CannotRegisterImplementationException, RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments )
        throw(  css::uno::Exception, css::uno::RuntimeException, std::exception) override;

private: // helper methods
    void prepareRegister(
        const OUString& implementationLoader,
        const OUString& location,
        const OUString& registeredLocation,
        const Reference < XSimpleRegistry > & xReg);
    // throw( CannotRegisterImplementationException, RuntimeException )

    static void doRegister( const Reference < XMultiComponentFactory >& xSMgr,
                            const Reference < XComponentContext > &xCtx,
                            const Reference < XImplementationLoader >& xAct,
                            const Reference < XSimpleRegistry >& xDest,
                            const OUString& implementationLoaderUrl,
                            const OUString& locationUrl,
                            const OUString& registeredLocationUrl);
        /* throw ( InvalidRegistryException,
                   MergeConflictException,
                   CannotRegisterImplementationException, RuntimeException ) */

    static void doRevoke( const Reference < XSimpleRegistry >& xDest,
                          const OUString& locationUrl );
        // throw( InvalidRegistryException, RuntimeException )
    Reference< XSimpleRegistry > getRegistryFromServiceManager();

    static Reference< XSimpleRegistry > createTemporarySimpleRegistry(
        const Reference< XMultiComponentFactory > &rSMgr,
        const Reference < XComponentContext > & rCtx );

private: // members
    Reference < XMultiComponentFactory >    m_xSMgr;
    Reference < XComponentContext >         m_xCtx;
};


// ImplementationRegistration()

ImplementationRegistration::ImplementationRegistration( const Reference < XComponentContext > & xCtx )
    : m_xSMgr( xCtx->getServiceManager() )
    , m_xCtx( xCtx )
{}


// ~ImplementationRegistration()

ImplementationRegistration::~ImplementationRegistration() {}

// XServiceInfo
OUString ImplementationRegistration::getImplementationName() throw(RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.stoc.ImplementationRegistration");
}

// XServiceInfo
sal_Bool ImplementationRegistration::supportsService(const OUString& ServiceName) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > ImplementationRegistration::getSupportedServiceNames() throw(RuntimeException, std::exception)
{
    Sequence< OUString > seqNames { "com.sun.star.registry.ImplementationRegistration" };
    return seqNames;
}

Reference< XSimpleRegistry > ImplementationRegistration::getRegistryFromServiceManager()
{
    Reference < XPropertySet > xPropSet( m_xSMgr, UNO_QUERY );
    Reference < XSimpleRegistry > xRegistry;

    if( xPropSet.is() ) {

        try {  // the implementation does not support XIntrospectionAccess !

            Any aAny = xPropSet->getPropertyValue( spool().Registry );

            if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE ) {
                aAny >>= xRegistry;
            }
         }
         catch( UnknownPropertyException & ) {
             // empty reference is error signal !
        }
    }

    return xRegistry;
}



// XInitialization

void ImplementationRegistration::initialize(
    const css::uno::Sequence< css::uno::Any >& aArgs )
    throw(  css::uno::Exception, css::uno::RuntimeException, std::exception)
{

    if( aArgs.getLength() != 4 ) {
        OUStringBuffer buf;
        buf.append( "ImplementationRegistration::initialize() expects 4 parameters, got " );
        buf.append( (sal_Int32) aArgs.getLength() );
        throw IllegalArgumentException( buf.makeStringAndClear(),
                                        Reference<XInterface > (),
                                        0 );
    }

    Reference< XImplementationLoader > rLoader;
    OUString loaderServiceName;
    OUString locationUrl;
    Reference< XSimpleRegistry > rReg;

    // 1st argument : An instance of an implementation loader
    if( aArgs.getConstArray()[0].getValueType().getTypeClass() == TypeClass_INTERFACE ) {
        aArgs.getConstArray()[0] >>= rLoader;
    }
    if( !rLoader.is()) {
        OUStringBuffer buf;
        buf.append( "ImplementationRegistration::initialize() invalid first parameter,"
                    "expected " );
        buf.append( cppu::UnoType<decltype(rLoader)>::get().getTypeName() );
        buf.append( ", got " );
        buf.append( aArgs.getConstArray()[0].getValueTypeName() );
        throw IllegalArgumentException( buf.makeStringAndClear(),
                                        Reference< XInterface > (),
                                        0 );
    }

    // 2nd argument : The service name of the loader. This name is written into the registry
    if( aArgs.getConstArray()[1].getValueType().getTypeClass() == TypeClass_STRING ) {
        aArgs.getConstArray()[1] >>= loaderServiceName;
    }
    if( loaderServiceName.isEmpty() ) {
        OUStringBuffer buf;
        buf.append( "ImplementationRegistration::initialize() invalid second parameter,"
                    "expected string, got " );
        buf.append( aArgs.getConstArray()[1].getValueTypeName() );
        throw IllegalArgumentException( buf.makeStringAndClear(),
                                        Reference< XInterface > (),
                                        0 );
    }

    // 3rd argument : The file name of the dll, that contains the loader
    if( aArgs.getConstArray()[2].getValueType().getTypeClass() == TypeClass_STRING ) {
        aArgs.getConstArray()[2] >>= locationUrl;
    }
    if( locationUrl.isEmpty() ) {
        OUStringBuffer buf;
        buf.append( "ImplementationRegistration::initialize() invalid third parameter,"
                    "expected string, got " );
        buf.append( aArgs.getConstArray()[2].getValueTypeName() );
        throw IllegalArgumentException( buf.makeStringAndClear(),
                                        Reference< XInterface > (),
                                        0 );
    }

    // 4th argument : The registry, the service should be written to
    if( aArgs.getConstArray()[3].getValueType().getTypeClass() == TypeClass_INTERFACE ) {
        aArgs.getConstArray()[3] >>= rReg;
    }

    if( !rReg.is() ) {
        rReg = getRegistryFromServiceManager();
        if( !rReg.is() ) {
            OUStringBuffer buf;
            buf.append( "ImplementationRegistration::initialize() invalid fourth parameter,"
                        "expected " );
            buf.append( cppu::UnoType<decltype(rReg)>::get().getTypeName() );
            buf.append( ", got " );
            buf.append( aArgs.getConstArray()[3].getValueTypeName() );
            throw IllegalArgumentException( buf.makeStringAndClear(),
                                            Reference< XInterface > (),
                                            0 );
        }
    }

    doRegister(m_xSMgr, m_xCtx, rLoader , rReg, loaderServiceName , locationUrl, locationUrl);
}




// virtual function registerImplementationWithLocation of XImplementationRegistration2

void ImplementationRegistration::registerImplementationWithLocation(
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    const OUString& registeredLocationUrl,
    const Reference < XSimpleRegistry > & xReg)
    throw( CannotRegisterImplementationException, RuntimeException, std::exception )
{
    prepareRegister(
        implementationLoaderUrl, locationUrl, registeredLocationUrl, xReg);
}

// helper function
void ImplementationRegistration::prepareRegister(
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    const OUString& registeredLocationUrl,
    const Reference < XSimpleRegistry > & xReg)
    // throw( CannotRegisterImplementationException, RuntimeException )
{
    OUString implLoaderUrl(implementationLoaderUrl);
    OUString activatorName;

    if (!implementationLoaderUrl.isEmpty())
    {
        OUString tmpActivator(implementationLoaderUrl);
        sal_Int32 nIndex = 0;
        activatorName = tmpActivator.getToken(0, ':', nIndex );
    } else
    {
        // check locationUrl to find out what kind of loader is needed
        // set iimplLoaderUrl
    }

    if( m_xSMgr.is() ) {
        try
        {
            Reference < XImplementationLoader > xAct(
                m_xSMgr->createInstanceWithContext(activatorName, m_xCtx) , UNO_QUERY );
            if (xAct.is())
            {
                Reference < XSimpleRegistry > xRegistry;

                if (xReg.is())
                {
                    // registry supplied by user
                    xRegistry = xReg;
                }
                else
                {
                    xRegistry = getRegistryFromServiceManager();
                }

                if ( xRegistry.is())
                {
                    doRegister(m_xSMgr, m_xCtx, xAct, xRegistry, implLoaderUrl,
                               locationUrl, registeredLocationUrl);
                }
            }
            else
            {
                OUStringBuffer buf( 128 );
                buf.append( "ImplementationRegistration::registerImplementation() - The service " );
                buf.append( activatorName );
                buf.append( " cannot be instantiated\n" );
                throw CannotRegisterImplementationException(
                    buf.makeStringAndClear() );
            }
        }
        catch( CannotRegisterImplementationException & )
        {
            throw;
        }
        catch( const InvalidRegistryException & e )
        {
            OUStringBuffer buf;
            buf.append( "ImplementationRegistration::registerImplementation() "
                        "InvalidRegistryException during registration (" );
            buf.append( e.Message );
            buf.append( ")" );
            throw CannotRegisterImplementationException(
                buf.makeStringAndClear() );
        }
        catch( const MergeConflictException & e )
        {
            OUStringBuffer buf;
            buf.append( "ImplementationRegistration::registerImplementation() "
                        "MergeConflictException during registration (" );
            buf.append( e.Message );
            buf.append( ")" );
            throw CannotRegisterImplementationException(
                buf.makeStringAndClear() );
        }
    }
    else
    {
        throw CannotRegisterImplementationException(
                "ImplementationRegistration::registerImplementation() "
                "no componentcontext available to instantiate loader" );
    }
}


// virtual function registerImplementation of XImplementationRegistration

void ImplementationRegistration::registerImplementation(
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    const Reference < XSimpleRegistry > & xReg)
    throw( CannotRegisterImplementationException, RuntimeException, std::exception )
{
    prepareRegister(implementationLoaderUrl, locationUrl, locationUrl, xReg);
}



// virtual function revokeImplementation of XImplementationRegistration

sal_Bool ImplementationRegistration::revokeImplementation(const OUString& location,
                                                      const Reference < XSimpleRegistry >& xReg)
    throw ( RuntimeException, std::exception )
{
    bool ret = false;

    Reference < XSimpleRegistry > xRegistry;

    if (xReg.is()) {
        xRegistry = xReg;
    }
    else {
        Reference < XPropertySet > xPropSet( m_xSMgr, UNO_QUERY );
        if( xPropSet.is() ) {
            try {
                Any aAny = xPropSet->getPropertyValue( spool().Registry );

                if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE )
                {
                    aAny >>= xRegistry;
                }
            }
            catch ( UnknownPropertyException & ) {
            }
        }
    }

    if (xRegistry.is())
    {
        try
        {
            doRevoke(xRegistry, location);
            ret = true;
        }
        catch( InvalidRegistryException & )
        {
            // no way to transport the error, as no exception is specified and a runtime
            // exception is not appropriate.
            OSL_FAIL( "InvalidRegistryException during revokeImplementation" );
        }
    }

    return ret;
}


// virtual function getImplementations of XImplementationRegistration

Sequence< OUString > ImplementationRegistration::getImplementations(
    const OUString & implementationLoaderUrl,
    const OUString & locationUrl)
    throw ( RuntimeException, std::exception )
{
    OUString activatorName;

    if (!implementationLoaderUrl.isEmpty())
    {
        OUString tmpActivator(implementationLoaderUrl);
        sal_Int32 nIndex = 0;
        activatorName = tmpActivator.getToken(0, ':', nIndex );
    } else
    {
        // check locationUrl to find out what kind of loader is needed
        // set implementationLoaderUrl
    }

    if( m_xSMgr.is() ) {

        Reference < XImplementationLoader > xAct(
            m_xSMgr->createInstanceWithContext( activatorName, m_xCtx ), UNO_QUERY );

        if (xAct.is())
        {

            Reference < XSimpleRegistry > xReg =
                createTemporarySimpleRegistry( m_xSMgr, m_xCtx);

            if (xReg.is())
            {
                try
                {
                    xReg->open(OUString() /* in mem */, sal_False, sal_True);
                    Reference < XRegistryKey > xImpl;

                    { // only necessary for deleting the temporary variable of rootkey
                        xImpl = xReg->getRootKey()->createKey( spool().slash_IMPLEMENTATIONS );
                    }
                    if (xAct->writeRegistryInfo(xImpl, implementationLoaderUrl, locationUrl))
                    {
                        std::list <OUString> implNames;

                        findImplementations(xImpl, implNames);

                        if (!implNames.empty())
                        {
                            Sequence<OUString> seqImpl(comphelper::containerToSequence<OUString>(implNames));
                            xImpl->closeKey();
                            return seqImpl;
                        }
                    }

                    xImpl->closeKey();
                }
                catch(MergeConflictException&)
                {
                }
                catch(InvalidRegistryException&)
                {
                }
            }
        }
    }

    return Sequence<OUString>();
}


// virtual function checkInstantiation of XImplementationRegistration

Sequence< OUString > ImplementationRegistration::checkInstantiation(const OUString&)
    throw ( RuntimeException, std::exception )
{
    OSL_FAIL( "ImplementationRegistration::checkInstantiation not implemented" );
    return Sequence<OUString>();
}


// helper function doRegistration


void ImplementationRegistration::doRevoke(
    const Reference < XSimpleRegistry >& xDest,
    const OUString& locationUrl)
    // throw ( InvalidRegistryException, RuntimeException )
{
    if( xDest.is() )
    {
        std::list<OUString> aNames;

        const StringPool &pool = spool();
        Reference < XRegistryKey > xRootKey( xDest->getRootKey() );

        Reference < XRegistryKey > xKey =
            xRootKey->openKey( pool.slash_IMPLEMENTATIONS );
        if (xKey.is() && xKey->isValid())
        {
            deleteAllImplementations(xDest, xKey, locationUrl, aNames);
        }

        xKey = xRootKey->openKey( pool.slash_SERVICES );
        if (xKey.is())
        {
            std::list<OUString>::const_iterator iter = aNames.begin();

            while (iter != aNames.end())
            {
                deleteAllServiceEntries(xDest, xKey, *iter);
                ++iter;
            }
        }

        xKey = xRootKey->openKey( "/SINGLETONS" );
        if (xKey.is() && xKey->isValid())
        {
            delete_all_singleton_entries( xKey, aNames );
        }

        if (xRootKey.is())
            xRootKey->closeKey();
        if (xKey.is() && xKey->isValid() )
            xKey->closeKey();
    }
}

void ImplementationRegistration::doRegister(
    const Reference< XMultiComponentFactory > & xSMgr,
    const Reference< XComponentContext > &xCtx,
    const Reference < XImplementationLoader > & xAct,
    const Reference < XSimpleRegistry >& xDest,
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    const OUString& registeredLocationUrl)
    /* throw ( InvalidRegistryException,
               MergeConflictException,
               CannotRegisterImplementationException, RuntimeException ) */
{
    Reference < XSimpleRegistry >   xReg =
        createTemporarySimpleRegistry( xSMgr, xCtx );
    Reference < XRegistryKey >      xSourceKey;

    if (xAct.is() && xReg.is() && xDest.is())
    {
        try
        {
            xReg->open(OUString() /* in mem */, sal_False, sal_True);

            { // only necessary for deleting the temporary variable of rootkey
                xSourceKey = xReg->getRootKey()->createKey( spool().slash_IMPLEMENTATIONS );
            }

            bool bSuccess =
                xAct->writeRegistryInfo(xSourceKey, implementationLoaderUrl, locationUrl);
            if ( bSuccess )
            {
                prepareRegistry(xDest, xSourceKey, implementationLoaderUrl, registeredLocationUrl, xCtx);

                xSourceKey->closeKey();

                xSourceKey = xReg->getRootKey();
                Reference < XRegistryKey > xDestKey = xDest->getRootKey();
                stoc_impreg::mergeKeys( xDestKey, xSourceKey );
                xDestKey->closeKey();
                xSourceKey->closeKey();
            }
            else
            {
                throw CannotRegisterImplementationException(
                    "ImplementationRegistration::doRegistration() component registration signaled failure" );
            }

            // Cleanup Source registry.
            if ( xSourceKey->isValid() )
                xSourceKey->closeKey();
        }
        catch(CannotRegisterImplementationException&)
        {
            if ( xSourceKey->isValid() )
                xSourceKey->closeKey();
            // and throw again
            throw;
        }
    }
}



Reference< XSimpleRegistry > ImplementationRegistration::createTemporarySimpleRegistry(
    const Reference< XMultiComponentFactory > &rSMgr,
    const Reference < XComponentContext > & xCtx)
{

    Reference < XSimpleRegistry > xReg(
        rSMgr->createInstanceWithContext(
            spool().com_sun_star_registry_SimpleRegistry,   xCtx ),
        UNO_QUERY);
    OSL_ASSERT( xReg.is() );
    return xReg;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_stoc_ImplementationRegistration_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ImplementationRegistration(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
