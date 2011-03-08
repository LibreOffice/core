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
#include "precompiled_xmloff.hxx"
#include "unointerfacetouniqueidentifiermapper.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::rtl::OUString;

namespace comphelper
{

UnoInterfaceToUniqueIdentifierMapper::UnoInterfaceToUniqueIdentifierMapper()
: mnNextId( 1 )
{
}

/** returns a unique identifier for the given uno object. IF a uno object is
    registered more than once, the returned identifier is always the same.
*/
const OUString& UnoInterfaceToUniqueIdentifierMapper::registerReference( const Reference< XInterface >& rInterface )
{
    IdMap_t::const_iterator aIter;
    if( findReference( rInterface, aIter ) )
    {
        return (*aIter).first;
    }
    else
    {
        OUString aId( RTL_CONSTASCII_USTRINGPARAM( "id" ) );
        aId += OUString::valueOf( mnNextId++ );
        return (*maEntries.insert( IdMap_t::value_type( aId, rInterface ) ).first).first;
    }
}

/** registers the given uno object with the given identifier.

    @returns
        false, if the given identifier already exists and is not associated with the given interface
*/
bool UnoInterfaceToUniqueIdentifierMapper::registerReference( const OUString& rIdentifier, const Reference< XInterface >& rInterface )
{
    IdMap_t::const_iterator aIter;
    if( findReference( rInterface, aIter ) )
    {
        return rIdentifier != (*aIter).first;
    }
    else if( findIdentifier( rIdentifier, aIter ) )
    {
        return false;
    }
    else
    {
        maEntries.insert( IdMap_t::value_type( rIdentifier, rInterface ) );

        // see if this is a reference like something we would generate in the future
        const sal_Unicode *p = rIdentifier.getStr();
        sal_Int32 nLength = rIdentifier.getLength();

        // see if the identifier is 'id' followed by a pure integer value
        if( nLength < 2 || p[0] != 'i' || p[1] != 'd' )
            return true;

        nLength -= 2;
        p += 2;

        while(nLength--)
        {
            if( (*p < '0') || (*p > '9') )
                return true; // a custom id, that will never conflict with genereated id's

            p++;
        }

        // the identifier is a pure integer value
        // so we make sure we will never generate
        // an integer value like this one
        sal_Int32 nId = rIdentifier.copy(2).toInt32();
        if( mnNextId <= nId )
            mnNextId = nId + 1;

        return true;
    }
}

/** @returns
        the identifier for the given uno object. If this uno object is not already
        registered, an empty string is returned
*/
const OUString& UnoInterfaceToUniqueIdentifierMapper::getIdentifier( const Reference< XInterface >& rInterface ) const
{
    IdMap_t::const_iterator aIter;
    if( findReference( rInterface, aIter ) )
    {
        return (*aIter).first;
    }
    else
    {
        static const OUString aEmpty;
        return aEmpty;
    }
}

/** @returns
    the uno object that is registered with the given identifier. If no uno object
    is registered with the given identifier, an empty reference is returned.
*/
const Reference< XInterface >& UnoInterfaceToUniqueIdentifierMapper::getReference( const OUString& rIdentifier ) const
{
    IdMap_t::const_iterator aIter;
    if( findIdentifier( rIdentifier, aIter ) )
    {
        return (*aIter).second;
    }
    else
    {
        static const Reference< XInterface > aEmpty;
        return aEmpty;
    }
}

bool UnoInterfaceToUniqueIdentifierMapper::findReference( const Reference< XInterface >& rInterface, IdMap_t::const_iterator& rIter ) const
{
    rIter = maEntries.begin();
    const IdMap_t::const_iterator aEnd( maEntries.end() );
    while( rIter != aEnd )
    {
        if( (*rIter).second == rInterface )
            return true;

        rIter++;
    }

    return false;
}

bool UnoInterfaceToUniqueIdentifierMapper::findIdentifier( const OUString& rIdentifier, IdMap_t::const_iterator& rIter ) const
{
    rIter = maEntries.find( rIdentifier );
    return rIter != maEntries.end();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
