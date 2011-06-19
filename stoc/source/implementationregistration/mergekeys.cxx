/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include <vector>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/MergeConflictException.hpp>

#include "mergekeys.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace stoc_impreg
{

struct Link
{
    OUString m_name;
    OUString m_target;

    inline Link( OUString const & name, OUString const & target )
        : m_name( name )
        , m_target( target )
        {}
};
typedef ::std::vector< Link > t_links;

//==================================================================================================
static void mergeKeys(
    Reference< registry::XRegistryKey > const & xDest,
    Reference< registry::XRegistryKey > const & xSource,
    t_links & links )
    // throw( registry::InvalidRegistryException, registry::MergeConflictException, RuntimeException )
{
    if (!xSource.is() || !xSource->isValid()) {
        throw registry::InvalidRegistryException(
            OUSTR("source key is null or invalid!"),
            Reference<XInterface>() );
    }
    if (!xDest.is() || !xDest->isValid()) {
        throw registry::InvalidRegistryException(
            OUSTR("destination key is null or invalid!"),
            Reference<XInterface>() );
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
    OUString const * pSourceKeys = sourceKeys.getConstArray();
    for ( sal_Int32 nPos = sourceKeys.getLength(); nPos--; )
    {
        // key name
        OUString name( pSourceKeys[ nPos ] );
        sal_Int32 nSlash = name.lastIndexOf( '/' );
        if (nSlash >= 0)
        {
            name = name.copy( nSlash +1 );
        }

        if (xSource->getKeyType( name ) == registry::RegistryKeyType_KEY)
        {
            // try to open exisiting dest key or create new one
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
                pSourceKeys[ nPos ], // abs path
                xSource->getResolvedName( name ) // abs resolved name
                ) );
        }
    }
}

//==================================================================================================
void mergeKeys(
    Reference< registry::XRegistryKey > const & xDest,
    Reference< registry::XRegistryKey > const & xSource )
    // throw( registry::InvalidRegistryException, registry::MergeConflictException, RuntimeException )
{
    if (!xDest.is() || !xDest->isValid()) {
        throw registry::InvalidRegistryException(
            OUSTR("destination key is null or invalid!"),
            Reference<XInterface>() );
    }
    if (xDest->isReadOnly())
    {
        throw registry::InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                          "destination registry is read-only!  cannot merge!") ),
            Reference< XInterface >() );
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
