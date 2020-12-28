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

#include <string.h>
#include <string_view>
#include <vector>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/ustring.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/registry/XImplementationRegistration2.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "mergekeys.hxx"

#if defined(_WIN32)
#include <io.h>
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

constexpr OUStringLiteral slash_UNO_slash_REGISTRY_LINKS
        = u"/UNO/REGISTRY_LINKS";
constexpr OUStringLiteral slash_IMPLEMENTATIONS
        =  u"/IMPLEMENTATIONS";
constexpr OUStringLiteral slash_UNO
        = u"/UNO";
constexpr OUStringLiteral slash_UNO_slash_SERVICES
        = u"/UNO/SERVICES";
constexpr OUStringLiteral slash_UNO_slash_SINGLETONS
        = u"/UNO/SINGLETONS";
constexpr OUStringLiteral slash_SERVICES
        = u"/SERVICES/";
constexpr OUStringLiteral slash_UNO_slash_LOCATION
        = u"/UNO/LOCATION";
constexpr OUStringLiteral slash_UNO_slash_ACTIVATOR
        = u"/UNO/ACTIVATOR";
constexpr OUStringLiteral colon_old
        = u":old";
constexpr OUStringLiteral com_sun_star_registry_SimpleRegistry
        = u"com.sun.star.registry.SimpleRegistry";
constexpr OUStringLiteral Registry
        = u"Registry";

//  static deleteAllLinkReferences()

void deleteAllLinkReferences(const Reference < XSimpleRegistry >& xReg,
                                    const Reference < XRegistryKey >& xSource)
    // throw ( InvalidRegistryException, RuntimeException )
{
    Reference < XRegistryKey > xKey = xSource->openKey(
        slash_UNO_slash_REGISTRY_LINKS );

    if (!(xKey.is() && (xKey->getValueType() == RegistryValueType_ASCIILIST)))
        return;

    const Sequence<OUString> linkNames = xKey->getAsciiListValue();

    if (!linkNames.hasElements())
        return;

    OUString            aLinkName;
    OUString            aLinkParent;
    Reference < XRegistryKey >  xLinkParent;
    const sal_Unicode*  pTmpName = nullptr;
    const sal_Unicode*  pShortName = nullptr;
    sal_Int32           sEnd = 0;

    for (const OUString& rLinkName : linkNames)
    {
        aLinkName = rLinkName;

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

        sEnd = aLinkName.lastIndexOf( '/' );

        aLinkParent = aLinkName.copy(0, sEnd);

        while(!aLinkParent.isEmpty())
        {
            xLinkParent = xReg->getRootKey()->openKey(aLinkParent);

            if (xLinkParent.is() && !xLinkParent->getKeyNames().hasElements())
            {
                aLinkName = aLinkParent;

                xReg->getRootKey()->deleteKey(aLinkParent);

                sEnd = aLinkName.lastIndexOf( '/' );

                aLinkParent = aLinkName.copy(0, sEnd);
            }
            else
            {
                break;
            }
        }
    }
}


//  static prepareLink

void prepareLink( const Reference < XSimpleRegistry > & xDest,
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
        linkRefName += link.subView(pShortName - pTmpName + 1);
        linkName = link.copy(0, pShortName - pTmpName);
    }

    if (isRelativ)
        xSource->createLink(linkName, linkRefName);
    else
        xDest->getRootKey()->createLink(linkName, linkRefName);
}


//  static searchImplForLink

OUString searchImplForLink(
    const Reference < XRegistryKey > & xRootKey,
    std::u16string_view linkName,
    std::u16string_view implName )
    // throw ( InvalidRegistryException, RuntimeException )
{
    Reference < XRegistryKey > xKey = xRootKey->openKey( slash_IMPLEMENTATIONS );
    if (xKey.is())
    {
        const Sequence< Reference < XRegistryKey > > subKeys( xKey->openKeys() );
        OUString key_name( slash_UNO + linkName );

        for (const Reference < XRegistryKey >& xImplKey : subKeys)
        {
            try
            {
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

OUString searchLinkTargetForImpl(const Reference < XRegistryKey >& xRootKey,
                                        std::u16string_view linkName,
                                        const OUString& implName)
{
    Reference < XRegistryKey > xKey = xRootKey->openKey( slash_IMPLEMENTATIONS );

    if (xKey.is())
    {
        Sequence< Reference < XRegistryKey > > subKeys = xKey->openKeys();

        OUString qualifiedLinkName( slash_UNO + linkName );

        auto pSubKey = std::find_if(subKeys.begin(), subKeys.end(),
            [&implName, &qualifiedLinkName](const Reference<XRegistryKey>& rSubKey) {
                OUString tmpImplName = rSubKey->getKeyName().copy(strlen("/IMPLEMENTATIONS/"));
                return tmpImplName == implName
                    && rSubKey->getKeyType( qualifiedLinkName ) == RegistryKeyType_LINK;
            });
        if (pSubKey != subKeys.end())
            return (*pSubKey)->getLinkTarget( qualifiedLinkName );
    }

    return OUString();
}


//  static createUniqueSubEntry

void createUniqueSubEntry(const Reference < XRegistryKey > & xSuperKey,
                                 const OUString& value)
    // throw ( InvalidRegistryException, RuntimeException )
{
    if (!xSuperKey.is())
        return;

    if (xSuperKey->getValueType() == RegistryValueType_ASCIILIST)
    {
        Sequence<OUString> implEntries = xSuperKey->getAsciiListValue();
        sal_Int32 length = implEntries.getLength();

        bool bReady = comphelper::findValue(implEntries, value) != -1;

        if (bReady)
        {
            Sequence<OUString> implEntriesNew(length);
            implEntriesNew.getArray()[0] = value;

            std::copy_if(implEntries.begin(), implEntries.end(), std::next(implEntriesNew.begin()),
                [&value](const OUString& rEntry) { return rEntry != value; });
            xSuperKey->setAsciiListValue(implEntriesNew);
        } else
        {
            Sequence<OUString> implEntriesNew(length+1);
            implEntriesNew.getArray()[0] = value;

            std::copy(implEntries.begin(), implEntries.end(), std::next(implEntriesNew.begin()));
            xSuperKey->setAsciiListValue(implEntriesNew);
        }
    } else
    {
        Sequence<OUString> implEntriesNew { value };

        xSuperKey->setAsciiListValue(implEntriesNew);
    }
}


//  static deleteSubEntry

bool deleteSubEntry(const Reference < XRegistryKey >& xSuperKey, const OUString& value)
    // throw ( InvalidRegistryException, RuntimeException )
{
    if (xSuperKey->getValueType() == RegistryValueType_ASCIILIST)
    {
        Sequence<OUString> implEntries = xSuperKey->getAsciiListValue();
        sal_Int32 length = implEntries.getLength();
        sal_Int32 equals = static_cast<sal_Int32>(std::count(implEntries.begin(), implEntries.end(), value));
        bool hasNoImplementations = false;

        if (equals == length)
        {
            hasNoImplementations = true;
        } else
        {
            Sequence<OUString> implEntriesNew(length - equals);

            std::copy_if(implEntries.begin(), implEntries.end(), implEntriesNew.begin(),
                [&value](const OUString& rEntry) { return rEntry != value; });
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

void prepareUserLink(const Reference < XSimpleRegistry >& xDest,
                                const OUString& linkName,
                                const OUString& linkTarget,
                                std::u16string_view implName)
{
    Reference < XRegistryKey > xRootKey = xDest->getRootKey();

    if (xRootKey->getKeyType(linkName) == RegistryKeyType_LINK)
    {
        OUString oldImplName(searchImplForLink(xRootKey, linkName, implName));

        if (!oldImplName.isEmpty())
        {
            createUniqueSubEntry(xDest->getRootKey()->createKey(
                linkName + colon_old ), oldImplName);
        }
    }

    if (xRootKey->isValid())
        xRootKey->createLink(linkName, linkTarget);
}


//  static deleteUserLink

void deletePathIfPossible(const Reference < XRegistryKey >& xRootKey,
                                 const OUString& path)
{
    try
    {
        Sequence<OUString> keyNames(xRootKey->openKey(path)->getKeyNames());

        if (!keyNames.hasElements() &&
            xRootKey->openKey(path)->getValueType() == RegistryValueType_NOT_DEFINED)
        {
            xRootKey->deleteKey(path);

            OUString newPath = path.copy(0, path.lastIndexOf('/'));

            if (newPath.getLength() > 1)
                deletePathIfPossible(xRootKey, newPath);
        }
    }
    catch(InvalidRegistryException&)
    {
    }
}


//  static deleteUserLink

void deleteUserLink(const Reference < XRegistryKey >& xRootKey,
                               const OUString& linkName,
                               std::u16string_view linkTarget,
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
        linkName + colon_old );
    if (xOldKey.is())
    {
        if (xOldKey->getValueType() == RegistryValueType_ASCIILIST)
        {
            Sequence<OUString> implEntries = xOldKey->getAsciiListValue();
            sal_Int32 length = implEntries.getLength();
            sal_Int32 equals = static_cast<sal_Int32>(std::count(implEntries.begin(), implEntries.end(), implName));
            bool hasNoImplementations = false;

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
        OUString path = linkName.copy(0, linkName.lastIndexOf('/'));
        deletePathIfPossible(xRootKey, path);
    }
}


//  static prepareUserKeys

void prepareUserKeys(const Reference < XSimpleRegistry >& xDest,
                                const Reference < XRegistryKey >& xUnoKey,
                                const Reference < XRegistryKey >& xKey,
                                const OUString& implName,
                                bool bRegister)
{
    bool hasSubKeys = false;

    Sequence<OUString> keyNames = xKey->getKeyNames();

    OUString relativKey;
    if (keyNames.hasElements())
        relativKey = keyNames.getConstArray()[0].copy(xKey->getKeyName().getLength()+1);

    if (keyNames.getLength() == 1 &&
        xKey->getKeyType(relativKey) == RegistryKeyType_LINK)
    {
        hasSubKeys = true;

        OUString linkTarget = xKey->getLinkTarget(relativKey);
        OUString linkName(xKey->getKeyName().copy(xUnoKey->getKeyName().getLength()));

        linkName += "/" + relativKey;

        if (bRegister)
        {
            prepareUserLink(xDest, linkName, linkTarget, implName);
        } else
        {
            deleteUserLink(xDest->getRootKey(), linkName, linkTarget, implName);
        }
    } else
    {
        const Sequence< Reference < XRegistryKey> > subKeys = xKey->openKeys();

        if (subKeys.hasElements())
        {
            hasSubKeys = true;

            for (const Reference < XRegistryKey > & rSubKey : subKeys)
            {
                prepareUserKeys(xDest, xUnoKey, rSubKey, implName, bRegister);
            }
        }
    }

    if (hasSubKeys)
        return;

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


//  static deleteAllImplementations

void deleteAllImplementations(   const Reference < XSimpleRegistry >& xReg,
                                        const Reference < XRegistryKey >& xSource,
                                        std::u16string_view locationUrl,
                                        std::vector<OUString> & implNames)
    // throw (InvalidRegistryException, RuntimeException)
{
    Sequence < Reference < XRegistryKey > > subKeys = xSource->openKeys();

    if (subKeys.hasElements())
    {
        bool hasLocationUrl = false;

        for (const Reference < XRegistryKey> & xImplKey : std::as_const(subKeys))
        {
            Reference < XRegistryKey > xKey = xImplKey->openKey(
                slash_UNO_slash_LOCATION );

            if (xKey.is() && (xKey->getValueType() == RegistryValueType_ASCII))
            {
                if (xKey->getAsciiValue() == locationUrl)
                {
                    hasLocationUrl = true;

                    OUString implName(xImplKey->getKeyName().copy(1));
                    sal_Int32 firstDot = implName.indexOf('/');

                    if (firstDot >= 0)
                        implName = implName.copy(firstDot + 1);

                    implNames.push_back(implName);

                    deleteAllLinkReferences(xReg, xImplKey);

                    xKey = xImplKey->openKey( slash_UNO );
                    if (xKey.is())
                    {
                        const Sequence< Reference < XRegistryKey > > subKeys2 = xKey->openKeys();

                        for (const Reference < XRegistryKey > & rSubKey2 : subKeys2)
                        {
                            if (rSubKey2->getKeyName() != (xImplKey->getKeyName() + slash_UNO_slash_SERVICES ) &&
                                rSubKey2->getKeyName() != (xImplKey->getKeyName() + slash_UNO_slash_REGISTRY_LINKS ) &&
                                rSubKey2->getKeyName() != (xImplKey->getKeyName() + slash_UNO_slash_ACTIVATOR ) &&
                                rSubKey2->getKeyName() != (xImplKey->getKeyName() + slash_UNO_slash_SINGLETONS ) &&
                                rSubKey2->getKeyName() != (xImplKey->getKeyName() + slash_UNO_slash_LOCATION) )
                            {
                                prepareUserKeys(xReg, xKey, rSubKey2, implName, false);
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
        if (!subKeys.hasElements())
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


void delete_all_singleton_entries(
    Reference < registry::XRegistryKey > const & xSingletons_section,
    ::std::vector< OUString > const & impl_names )
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

                for (auto const& impl_name : impl_names)
                {
                    if (impl_name == registered_implname)
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

void deleteAllServiceEntries(    const Reference < XSimpleRegistry >& xReg,
                                        const Reference < XRegistryKey >& xSource,
                                        const OUString& implName)
    // throw ( InvalidRegistryException, RuntimeException )
{
    Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

    if (subKeys.hasElements())
    {
        bool hasNoImplementations = false;

        for (const Reference < XRegistryKey > & xServiceKey : std::as_const(subKeys))
        {
            if (xServiceKey->getValueType() == RegistryValueType_ASCIILIST)
            {
                Sequence<OUString> implEntries = xServiceKey->getAsciiListValue();
                sal_Int32 length = implEntries.getLength();
                sal_Int32 equals = static_cast<sal_Int32>(std::count(implEntries.begin(), implEntries.end(), implName));

                if (equals == length)
                {
                    hasNoImplementations = true;
                } else
                {
                    if (equals > 0)
                    {
                        Sequence<OUString> implEntriesNew(length-equals);

                        std::copy_if(implEntries.begin(), implEntries.end(), implEntriesNew.begin(),
                            [&implName](const OUString& rEntry) { return rEntry != implName; });

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
        if (!subKeys.hasElements())
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


bool is_supported_service(
    OUString const & service_name,
    Reference< reflection::XServiceTypeDescription > const & xService_td )
{
    if (xService_td->getName() == service_name)
        return true;
    Sequence< Reference< reflection::XServiceTypeDescription > > seq(
        xService_td->getMandatoryServices() );
    return std::any_of(seq.begin(), seq.end(), [&service_name](const auto& rService) {
        return is_supported_service( service_name, rService ); });
}


void insert_singletons(
    Reference< registry::XSimpleRegistry > const & xDest,
    Reference< registry::XRegistryKey > const & xImplKey,
    Reference< XComponentContext > const & xContext )
    // throw( registry::InvalidRegistryException, registry::CannotRegisterImplementationException, RuntimeException )
{
    // singletons
    Reference< registry::XRegistryKey > xKey( xImplKey->openKey( "UNO/SINGLETONS" ) );
    if (!(xKey.is() && xKey->isValid()))
        return;

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
                if ( existing_name != service_name )
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
                            throw registry::CannotRegisterImplementationException(
                                "existing singleton service (" + singleton_name + "=" + existing_name + ") "
                                " does not support given one: " + service_name);
                        }
                    }
                    catch (const container::NoSuchElementException & exc)
                    {
                        css::uno::Any anyEx = cppu::getCaughtException();
                        throw css::lang::WrappedTargetRuntimeException(
                                "cannot get service type description: " + exc.Message,
                                nullptr, anyEx );
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
        if (comphelper::findValue(implnames, implname) == -1)
        {
            // append and write back
            implnames.realloc( implnames.getLength() +1 );
            implnames[ implnames.getLength() -1 ] = implname;
            xRegisteredImplNames->setAsciiListValue( implnames );
        }
    }
}


//  static prepareRegistry

void prepareRegistry(
    const Reference < XSimpleRegistry >& xDest,
    const Reference < XRegistryKey >& xSource,
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    Reference< XComponentContext > const & xContext )
    // throw ( InvalidRegistryException, CannotRegisterImplementationException, RuntimeException )
{
    const Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

    if (!subKeys.hasElements())
    {
        throw InvalidRegistryException(
            "prepareRegistry(): source registry is empty" );
    }

    for (const Reference < XRegistryKey >& xImplKey : subKeys)
    {
        Reference < XRegistryKey >  xKey = xImplKey->openKey(
            slash_UNO_slash_SERVICES );

        if (xKey.is())
        {
            // update entries in SERVICES section
            const Sequence< Reference < XRegistryKey > > serviceKeys = xKey->openKeys();

            OUString implName = xImplKey->getKeyName().copy(1);
            sal_Int32 firstDot = implName.indexOf('/');

            if (firstDot >= 0)
                implName = implName.copy(firstDot + 1);

            sal_Int32 offset = xKey->getKeyName().getLength() + 1;

            for (const Reference < XRegistryKey >& rServiceKey : serviceKeys)
            {
                OUString serviceName = rServiceKey->getKeyName().copy(offset);

                createUniqueSubEntry(
                    xDest->getRootKey()->createKey(
                        slash_SERVICES + serviceName ),
                    implName);
            }

            xKey = xImplKey->openKey( slash_UNO );
            if (xKey.is())
            {
                const Sequence< Reference < XRegistryKey > > subKeys2 = xKey->openKeys();

                for (const Reference < XRegistryKey >& rSubKey2 : subKeys2)
                {
                    if (rSubKey2->getKeyName() != (xImplKey->getKeyName() + slash_UNO_slash_SERVICES) &&
                        rSubKey2->getKeyName() != (xImplKey->getKeyName() + slash_UNO_slash_REGISTRY_LINKS ) &&
                        rSubKey2->getKeyName() != (xImplKey->getKeyName() + slash_UNO_slash_SINGLETONS ))
                    {
                        prepareUserKeys(xDest, xKey, rSubKey2, implName, true);
                    }
                }
            }
        }

        // update LOCATION entry
        xKey = xImplKey->createKey( slash_UNO_slash_LOCATION );

        if (xKey.is())
        {
            xKey->setAsciiValue(locationUrl);
        }

        // update ACTIVATOR entry
        xKey = xImplKey->createKey( slash_UNO_slash_ACTIVATOR );

        if (xKey.is())
        {
            xKey->setAsciiValue(implementationLoaderUrl);
        }

        xKey = xImplKey->openKey( slash_UNO_slash_SERVICES );

        if (xKey.is() && (xKey->getValueType() == RegistryValueType_ASCIILIST))
        {
            // update link entries in REGISTRY_LINKS section
            const Sequence<OUString> linkNames = xKey->getAsciiListValue();

            for (const OUString& rLinkName : linkNames)
            {
                prepareLink(xDest, xImplKey, rLinkName);
            }
        }

        insert_singletons( xDest, xImplKey, xContext );
    }
}


void findImplementations(    const Reference < XRegistryKey > & xSource,
                                    std::vector<OUString>& implNames)
{
    bool isImplKey = false;

    try
    {
        Reference < XRegistryKey > xKey = xSource->openKey(
            slash_UNO_slash_SERVICES );

        if (xKey.is() && xKey->getKeyNames().hasElements())
        {
            isImplKey = true;

            OUString implName = xSource->getKeyName().copy(1).replace('/', '.');
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
        const Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

        for (const Reference < XRegistryKey >& rSubKey : subKeys)
        {
            findImplementations(rSubKey, implNames);
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

    // XServiceInfo
    OUString                        SAL_CALL getImplementationName() override;
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) override;
    Sequence< OUString >            SAL_CALL getSupportedServiceNames() override;

    // XImplementationRegistration
    virtual void SAL_CALL registerImplementation(
        const OUString& implementationLoader,
        const OUString& location,
        const Reference < XSimpleRegistry > & xReg) override;

    virtual sal_Bool SAL_CALL revokeImplementation(
        const OUString& location,
        const Reference < XSimpleRegistry >& xReg) override;

    virtual Sequence< OUString > SAL_CALL getImplementations(
        const OUString& implementationLoader,
        const OUString& location) override;
    virtual Sequence< OUString > SAL_CALL checkInstantiation(
        const OUString& implementationName) override;

    // XImplementationRegistration2
    virtual void SAL_CALL registerImplementationWithLocation(
        const OUString& implementationLoader,
        const OUString& location,
        const OUString& registeredLocation,
        const Reference < XSimpleRegistry > & xReg) override;

    // XInitialization
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments ) override;

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
                          std::u16string_view locationUrl );
        // throw( InvalidRegistryException, RuntimeException )
    Reference< XSimpleRegistry > getRegistryFromServiceManager() const;

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

// XServiceInfo
OUString ImplementationRegistration::getImplementationName()
{
    return "com.sun.star.comp.stoc.ImplementationRegistration";
}

// XServiceInfo
sal_Bool ImplementationRegistration::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > ImplementationRegistration::getSupportedServiceNames()
{
    return { "com.sun.star.registry.ImplementationRegistration" };
}

Reference< XSimpleRegistry > ImplementationRegistration::getRegistryFromServiceManager() const
{
    Reference < XPropertySet > xPropSet( m_xSMgr, UNO_QUERY );
    Reference < XSimpleRegistry > xRegistry;

    if( xPropSet.is() ) {

        try {  // the implementation does not support XIntrospectionAccess !

            Any aAny = xPropSet->getPropertyValue( Registry );

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
{

    if( aArgs.getLength() != 4 ) {
        throw IllegalArgumentException(
            "ImplementationRegistration::initialize() expects 4 parameters, got "  + OUString::number( aArgs.getLength() ),
            Reference<XInterface > (), 0 );
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
        throw IllegalArgumentException(
            "ImplementationRegistration::initialize() invalid first parameter,"
            "expected " + cppu::UnoType<decltype(rLoader)>::get().getTypeName() +
            ", got " + aArgs.getConstArray()[0].getValueTypeName(),
            Reference< XInterface > (), 0 );
    }

    // 2nd argument : The service name of the loader. This name is written into the registry
    if( aArgs.getConstArray()[1].getValueType().getTypeClass() == TypeClass_STRING ) {
        aArgs.getConstArray()[1] >>= loaderServiceName;
    }
    if( loaderServiceName.isEmpty() ) {
        throw IllegalArgumentException(
            "ImplementationRegistration::initialize() invalid second parameter,"
            "expected string, got " + aArgs.getConstArray()[1].getValueTypeName(),
            Reference< XInterface > (), 0 );
    }

    // 3rd argument : The file name of the dll, that contains the loader
    if( aArgs.getConstArray()[2].getValueType().getTypeClass() == TypeClass_STRING ) {
        aArgs.getConstArray()[2] >>= locationUrl;
    }
    if( locationUrl.isEmpty() ) {
        throw IllegalArgumentException(
            "ImplementationRegistration::initialize() invalid third parameter,"
            "expected string, got " + aArgs.getConstArray()[2].getValueTypeName(),
            Reference< XInterface > (), 0 );
    }

    // 4th argument : The registry, the service should be written to
    if( aArgs.getConstArray()[3].getValueType().getTypeClass() == TypeClass_INTERFACE ) {
        aArgs.getConstArray()[3] >>= rReg;
    }

    if( !rReg.is() ) {
        rReg = getRegistryFromServiceManager();
        if( !rReg.is() ) {
            throw IllegalArgumentException(
                "ImplementationRegistration::initialize() invalid fourth parameter,"
                "expected " + cppu::UnoType<decltype(rReg)>::get().getTypeName() +
                ", got " + aArgs.getConstArray()[3].getValueTypeName(),
                Reference< XInterface > (), 0 );
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
    OUString activatorName;

    if (!implementationLoaderUrl.isEmpty())
    {
        activatorName = implementationLoaderUrl.getToken(0, ':');
    } else
    {
        // check locationUrl to find out what kind of loader is needed
        // set implLoaderUrl
    }

    if( !m_xSMgr.is() )    {
        throw CannotRegisterImplementationException(
                "ImplementationRegistration::registerImplementation() "
                "no componentcontext available to instantiate loader" );
    }

    try
    {
        Reference < XImplementationLoader > xAct(
            m_xSMgr->createInstanceWithContext(activatorName, m_xCtx) , UNO_QUERY );
        if (!xAct.is())
        {
            throw CannotRegisterImplementationException(
                "ImplementationRegistration::registerImplementation() - The service "
                + activatorName + " cannot be instantiated" );
        }

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
            doRegister(m_xSMgr, m_xCtx, xAct, xRegistry, implementationLoaderUrl,
                       locationUrl, registeredLocationUrl);
        }

    }
    catch( CannotRegisterImplementationException & )
    {
        throw;
    }
    catch( const InvalidRegistryException & e )
    {
        throw CannotRegisterImplementationException(
            "ImplementationRegistration::registerImplementation() "
            "InvalidRegistryException during registration (" + e.Message + ")" );
    }
    catch( const MergeConflictException & e )
    {
        throw CannotRegisterImplementationException(
            "ImplementationRegistration::registerImplementation() "
            "MergeConflictException during registration (" + e.Message + ")" );
    }

}


// virtual function registerImplementation of XImplementationRegistration

void ImplementationRegistration::registerImplementation(
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    const Reference < XSimpleRegistry > & xReg)
{
    prepareRegister(implementationLoaderUrl, locationUrl, locationUrl, xReg);
}


// virtual function revokeImplementation of XImplementationRegistration

sal_Bool ImplementationRegistration::revokeImplementation(const OUString& location,
                                                      const Reference < XSimpleRegistry >& xReg)
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
                Any aAny = xPropSet->getPropertyValue( Registry );

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
            TOOLS_WARN_EXCEPTION( "stoc", "InvalidRegistryException during revokeImplementation" );
        }
    }

    return ret;
}


// virtual function getImplementations of XImplementationRegistration

Sequence< OUString > ImplementationRegistration::getImplementations(
    const OUString & implementationLoaderUrl,
    const OUString & locationUrl)
{
    OUString activatorName;

    if (!implementationLoaderUrl.isEmpty())
    {
        activatorName = implementationLoaderUrl.getToken(0, ':');
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
                    xReg->open(OUString() /* in mem */, false, true);
                    Reference < XRegistryKey > xImpl;

                    { // only necessary for deleting the temporary variable of rootkey
                        xImpl = xReg->getRootKey()->createKey( slash_IMPLEMENTATIONS );
                    }
                    if (xAct->writeRegistryInfo(xImpl, implementationLoaderUrl, locationUrl))
                    {
                        std::vector<OUString> implNames;

                        findImplementations(xImpl, implNames);

                        if (!implNames.empty())
                        {
                            Sequence<OUString> seqImpl(comphelper::containerToSequence(implNames));
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
{
    OSL_FAIL( "ImplementationRegistration::checkInstantiation not implemented" );
    return Sequence<OUString>();
}


// helper function doRegistration


void ImplementationRegistration::doRevoke(
    const Reference < XSimpleRegistry >& xDest,
    std::u16string_view locationUrl)
    // throw ( InvalidRegistryException, RuntimeException )
{
    if( !xDest.is() )
        return;

    std::vector<OUString> aNames;

    Reference < XRegistryKey > xRootKey( xDest->getRootKey() );

    Reference < XRegistryKey > xKey =
        xRootKey->openKey( slash_IMPLEMENTATIONS );
    if (xKey.is() && xKey->isValid())
    {
        deleteAllImplementations(xDest, xKey, locationUrl, aNames);
    }

    xKey = xRootKey->openKey( slash_SERVICES );
    if (xKey.is())
    {
        for (auto const& name : aNames)
        {
            deleteAllServiceEntries(xDest, xKey, name);
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

    if (!(xAct.is() && xReg.is() && xDest.is()))
        return;

    try
    {
        xReg->open(OUString() /* in mem */, false, true);

        { // only necessary for deleting the temporary variable of rootkey
            xSourceKey = xReg->getRootKey()->createKey( slash_IMPLEMENTATIONS );
        }

        bool bSuccess =
            xAct->writeRegistryInfo(xSourceKey, implementationLoaderUrl, locationUrl);
        if ( !bSuccess )
        {
            throw CannotRegisterImplementationException(
                "ImplementationRegistration::doRegistration() component registration signaled failure" );
        }

        prepareRegistry(xDest, xSourceKey, implementationLoaderUrl, registeredLocationUrl, xCtx);

        xSourceKey->closeKey();

        xSourceKey = xReg->getRootKey();
        Reference < XRegistryKey > xDestKey = xDest->getRootKey();
        stoc_impreg::mergeKeys( xDestKey, xSourceKey );
        xDestKey->closeKey();
        xSourceKey->closeKey();


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


Reference< XSimpleRegistry > ImplementationRegistration::createTemporarySimpleRegistry(
    const Reference< XMultiComponentFactory > &rSMgr,
    const Reference < XComponentContext > & xCtx)
{

    Reference < XSimpleRegistry > xReg(
        rSMgr->createInstanceWithContext(
            com_sun_star_registry_SimpleRegistry,   xCtx ),
        UNO_QUERY);
    OSL_ASSERT( xReg.is() );
    return xReg;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_stoc_ImplementationRegistration_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ImplementationRegistration(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
