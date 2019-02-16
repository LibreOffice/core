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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_NAMEDCOLLECTION_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_NAMEDCOLLECTION_HXX

#include "collection.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

#include <algorithm>

template<class T>
class NamedCollection : public cppu::ImplInheritanceHelper<
                            Collection<T>,
                            css::container::XNameAccess>
{
    using Collection<T>::maItems;
    using Collection<T>::getItem;
    using Collection<T>::hasItem;

public:
    NamedCollection() {}

    const T& getItem( const OUString& rName ) const
    {
        OSL_ENSURE( hasItem( rName ), "invalid name" );
        return *findItem( rName );
    }

    bool hasItem( const OUString& rName ) const
    {
        return findItem( rName ) != maItems.end();
    }

    css::uno::Sequence<OUString> getNames() const
    {
        // iterate over members, and collect all those that have names
        std::vector<OUString> aNames;
        for( const T& rItem : maItems )
        {
            css::uno::Reference<css::container::XNamed>
                xNamed( rItem, css::uno::UNO_QUERY );
            if( xNamed.is() )
                aNames.push_back( xNamed->getName() );
        }

        return comphelper::containerToSequence(aNames);
    }

protected:
    typename std::vector<T>::const_iterator findItem( const OUString& rName ) const
    {
        return std::find_if(maItems.begin(), maItems.end(), [&rName](const T& rItem) {
            css::uno::Reference<css::container::XNamed>
                xNamed( rItem, css::uno::UNO_QUERY );
            return xNamed.is() && xNamed->getName() == rName;
        });
    }

public:

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override
    {
        return Collection<T>::getElementType();
    }

    virtual sal_Bool SAL_CALL hasElements() override
    {
        return Collection<T>::hasElements();
    }

    // XNameAccess : XElementAccess
    virtual css::uno::Any SAL_CALL getByName(
        const OUString& aName ) override
    {
        if( !hasItem( aName ) )
            throw css::container::NoSuchElementException();
        return css::uno::makeAny( getItem( aName ) );
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getElementNames() override
    {
        return getNames();
    }

    virtual sal_Bool SAL_CALL hasByName(
        const OUString& aName ) override
    {
        return hasItem( aName );
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
