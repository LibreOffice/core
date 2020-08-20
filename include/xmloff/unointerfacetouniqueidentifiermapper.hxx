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

#ifndef INCLUDED_XMLOFF_UNOINTERFACETOUNIQUEIDENTIFIERMAPPER_HXX
#define INCLUDED_XMLOFF_UNOINTERFACETOUNIQUEIDENTIFIERMAPPER_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>

#include <deque>
#include <map>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XInterface.hpp>

namespace comphelper
{

typedef ::std::map< OUString, css::uno::Reference< css::uno::XInterface > > IdMap_t;

class XMLOFF_DLLPUBLIC UnoInterfaceToUniqueIdentifierMapper
{
    typedef std::deque< OUString > Reserved_t;

public:
    UnoInterfaceToUniqueIdentifierMapper();

    /** returns a unique identifier for the given uno object. If a UNO object is
        registered more than once, the returned identifier is always the same.
    */
    const OUString& registerReference( const css::uno::Reference< css::uno::XInterface >& rInterface );

    /** registers the given uno object with the given identifier.

        @returns
            false, if the given identifier already exists and is not associated with the given interface
    */
    bool registerReference( const OUString& rIdentifier, const css::uno::Reference< css::uno::XInterface >& rInterface );

    /** reserves an identifier for later registration.

        @returns
            false, if the identifier already exists
      */
    bool reserveIdentifier( const OUString& rIdentifier );

    /** registers the given uno object with reserved identifier.
      */
    bool registerReservedReference( const OUString& rIdentifier, const css::uno::Reference< css::uno::XInterface >& rInterface );

    /** @returns
            the identifier for the given uno object. If this uno object is not already
            registered, an empty string is returned
    */
    const OUString& getIdentifier( const css::uno::Reference< css::uno::XInterface >& rInterface ) const;

    /** @returns
        the uno object that is registered with the given identifier. If no uno object
        is registered with the given identifier, an empty reference is returned.
    */
    const css::uno::Reference< css::uno::XInterface >& getReference( const OUString& rIdentifier ) const;

private:
    bool findReference( const css::uno::Reference< css::uno::XInterface >& rInterface, IdMap_t::const_iterator& rIter ) const;
    bool findIdentifier( const OUString& rIdentifier, IdMap_t::const_iterator& rIter ) const;
    bool findReserved( const OUString& rIdentifier ) const;
    bool findReserved( const OUString& rIdentifier, Reserved_t::const_iterator& rIter ) const;

    IdMap_t maEntries;
    sal_uInt32 mnNextId;
    Reserved_t maReserved;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
