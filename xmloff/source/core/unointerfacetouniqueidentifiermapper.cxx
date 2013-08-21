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

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;

namespace comphelper
{

UnoInterfaceToUniqueIdentifierMapper::UnoInterfaceToUniqueIdentifierMapper()
: mnNextId( 1 )
{
}

const OUString& UnoInterfaceToUniqueIdentifierMapper::registerReference( const Reference< XInterface >& rInterface )
{
    // Be certain that the references we store in our table are to the
    // leading / primary XInterface - cf. findReference
    uno::Reference< uno::XInterface > xRef( rInterface, uno::UNO_QUERY );

    IdMap_t::const_iterator aIter;
    if( findReference( xRef, aIter ) )
    {
        return (*aIter).first;
    }
    else
    {
        OUString aId( "id" );
        aId += OUString::number( mnNextId++ );
        return (*maEntries.insert( IdMap_t::value_type( aId, xRef ) ).first).first;
    }
}

bool UnoInterfaceToUniqueIdentifierMapper::registerReference( const OUString& rIdentifier, const Reference< XInterface >& rInterface )
{
    IdMap_t::const_iterator aIter;

    // Be certain that the references we store in our table are to the
    // leading / primary XInterface - cf. findReference
    uno::Reference< uno::XInterface > xRef( rInterface, uno::UNO_QUERY );

    if( findReference( xRef, aIter ) )
    {
        return rIdentifier != (*aIter).first;
    }
    else if( findIdentifier( rIdentifier, aIter ) )
    {
        return false;
    }
    else
    {
        insertReference( rIdentifier, xRef );
    }

    return true;
}

void UnoInterfaceToUniqueIdentifierMapper::registerReferenceAlways( const OUString& rIdentifier, const Reference< XInterface >& rInterface )
{
    // Be certain that the references we store in our table are to the
    // leading / primary XInterface - cf. findReference
    uno::Reference< uno::XInterface > xRef( rInterface, uno::UNO_QUERY );

    insertReference( rIdentifier, xRef );
}

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
    uno::Reference< uno::XInterface > xRef( rInterface, uno::UNO_QUERY );

    rIter = maEntries.begin();

    const IdMap_t::const_iterator aEnd( maEntries.end() );
    while( rIter != aEnd )
    {
        // The Reference == operator, does a repeated queryInterface on
        // this to ensure we got the right XInterface base-class. However,
        // we can be sure that this has been done already by the time we
        // get to here.
        if( (*rIter).second.get() == xRef.get() )
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

void UnoInterfaceToUniqueIdentifierMapper::insertReference( const OUString& rIdentifier, const Reference< XInterface >& rInterface )
{
    maEntries[rIdentifier] = rInterface;

    // see if this is a reference like something we would generate in the future
    const sal_Unicode *p = rIdentifier.getStr();
    sal_Int32 nLength = rIdentifier.getLength();

    // see if the identifier is 'id' followed by a pure integer value
    if( nLength < 2 || p[0] != 'i' || p[1] != 'd' )
        return;

    nLength -= 2;
    p += 2;

    while(nLength--)
    {
        if( (*p < '0') || (*p > '9') )
            return; // a custom id, that will never conflict with genereated id's

        p++;
    }

    // the identifier is a pure integer value
    // so we make sure we will never generate
    // an integer value like this one
    sal_Int32 nId = rIdentifier.copy(2).toInt32();
    if( mnNextId <= nId )
        mnNextId = nId + 1;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
