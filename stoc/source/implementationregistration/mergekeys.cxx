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


#include <utility>
#include <vector>

#include <osl/diagnose.h>

#include <com/sun/star/registry/XRegistryKey.hpp>

#include "mergekeys.hxx"

using namespace css::uno;
using namespace ::com::sun::star;

namespace stoc_impreg
{

namespace {

struct Link
{
    OUString m_name;
    OUString m_target;

    Link( OUString name, OUString target )
        : m_name(std::move( name ))
        , m_target(std::move( target ))
        {}
};

}

typedef ::std::vector< Link > t_links;


static void mergeKeys(
    Reference< registry::XRegistryKey > const & xDest,
    Reference< registry::XRegistryKey > const & xSource,
    t_links & links )
    // throw( registry::InvalidRegistryException, registry::MergeConflictException, RuntimeException )
{
    if (!xSource.is() || !xSource->isValid()) {
        throw registry::InvalidRegistryException(
            u"source key is null or invalid!"_ustr );
    }
    if (!xDest.is() || !xDest->isValid()) {
        throw registry::InvalidRegistryException(
            u"destination key is null or invalid!"_ustr );
    }

    // write value
    switch (xSource->getValueType())
    {
    case registry::RegistryValueType_NOT_DEFINED:
        break;
    case registry::RegistryValueType_LONG:
        xDest->setLongValue( xSource->getLongValue() );
        break;
    case registry::RegistryValueType_ASCII:
        xDest->setAsciiValue( xSource->getAsciiValue() );
        break;
    case registry::RegistryValueType_STRING:
        xDest->setStringValue( xSource->getStringValue() );
        break;
    case registry::RegistryValueType_BINARY:
        xDest->setBinaryValue( xSource->getBinaryValue() );
        break;
    case registry::RegistryValueType_LONGLIST:
        xDest->setLongListValue( xSource->getLongListValue() );
        break;
    case registry::RegistryValueType_ASCIILIST:
        xDest->setAsciiListValue( xSource->getAsciiListValue() );
        break;
    case registry::RegistryValueType_STRINGLIST:
        xDest->setStringListValue( xSource->getStringListValue() );
        break;
    default:
        OSL_ASSERT(false);
        break;
    }

    // sub keys
    Sequence< OUString > sourceKeys( xSource->getKeyNames() );
    for ( sal_Int32 nPos = sourceKeys.getLength(); nPos--; )
    {
        // key name
        OUString name( sourceKeys[ nPos ] );
        sal_Int32 nSlash = name.lastIndexOf( '/' );
        if (nSlash >= 0)
        {
            name = name.copy( nSlash +1 );
        }

        if (xSource->getKeyType( name ) == registry::RegistryKeyType_KEY)
        {
            // try to open existing dest key or create new one
            Reference< registry::XRegistryKey > xDestKey( xDest->createKey( name ) );
            Reference< registry::XRegistryKey > xSourceKey( xSource->openKey( name ) );
            mergeKeys( xDestKey, xSourceKey, links );
            xSourceKey->closeKey();
            xDestKey->closeKey();
        }
        else // link
        {
            // remove existing key
            Reference< registry::XRegistryKey > xDestKey( xDest->openKey( name ) );
            if (xDestKey.is() && xDestKey->isValid()) // something to remove
            {
                xDestKey->closeKey();
                if (xDest->getKeyType( name ) == registry::RegistryKeyType_LINK)
                {
                    xDest->deleteLink( name );
                }
                else
                {
                    xDest->deleteKey( name );
                }
            }

            links.push_back( Link(
                sourceKeys[ nPos ], // abs path
                xSource->getResolvedName( name ) // abs resolved name
                ) );
        }
    }
}


void mergeKeys(
    Reference< registry::XRegistryKey > const & xDest,
    Reference< registry::XRegistryKey > const & xSource )
    // throw( registry::InvalidRegistryException, registry::MergeConflictException, RuntimeException )
{
    if (!xDest.is() || !xDest->isValid()) {
        throw registry::InvalidRegistryException(
            u"destination key is null or invalid!"_ustr );
    }
    if (xDest->isReadOnly())
    {
        throw registry::InvalidRegistryException(
            u"destination registry is read-only!  cannot merge!"_ustr );
    }

    t_links links;
    links.reserve( 16 );
    mergeKeys( xDest, xSource, links );

    for ( size_t nPos = links.size(); nPos--; )
    {
        Link const & r = links[ nPos ];
        OSL_VERIFY( xDest->createLink( r.m_name, r.m_target ) );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
