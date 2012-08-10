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

#ifndef _NAMEDCOLLECTION_HXX
#define _NAMEDCOLLECTION_HXX

#include <collection.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

#include <algorithm>

template<class T>
class NamedCollection : public cppu::ImplInheritanceHelper1<
                            Collection<T>,
                            com::sun::star::container::XNameAccess>
{
    using Collection<T>::maItems;
    using Collection<T>::getItem;
    using Collection<T>::hasItem;

public:
    NamedCollection() {}
    virtual ~NamedCollection() {}

    const T& getItem( const rtl::OUString& rName ) const
    {
        OSL_ENSURE( hasItem( rName ), "invalid name" );
        return *findItem( rName );
    }

    bool hasItem( const rtl::OUString& rName ) const
    {
        return findItem( rName ) != maItems.end();
    }

    typedef com::sun::star::uno::Sequence<rtl::OUString> Names_t;
    Names_t getNames() const
    {
        // iterate over members, and collect all those that have names
        std::vector<rtl::OUString> aNames;
        for( typename std::vector<T>::const_iterator aIter = maItems.begin();
             aIter != maItems.end();
             ++aIter )
        {
            com::sun::star::uno::Reference<com::sun::star::container::XNamed>
                xNamed( *aIter, com::sun::star::uno::UNO_QUERY );
            if( xNamed.is() )
                aNames.push_back( xNamed->getName() );
        }

        // copy names to Sequence and return
        Names_t aResult( aNames.size() );
        rtl::OUString* pStrings = aResult.getArray();
        std::copy( aNames.begin(), aNames.end(), pStrings );

        return aResult;
    }

protected:
    typename std::vector<T>::const_iterator findItem( const rtl::OUString& rName ) const
    {
        for( typename std::vector<T>::const_iterator aIter = maItems.begin();
             aIter != maItems.end();
             ++aIter )
        {
            com::sun::star::uno::Reference<com::sun::star::container::XNamed>
                xNamed( *aIter, com::sun::star::uno::UNO_QUERY );
            if( xNamed.is()  &&  xNamed->getName() == rName )
                return aIter;
        }
        return maItems.end();
    }

public:

    // XElementAccess
    virtual typename Collection<T>::Type_t SAL_CALL getElementType()
        throw( typename Collection<T>::RuntimeException_t )
    {
        return Collection<T>::getElementType();
    }

    virtual sal_Bool SAL_CALL hasElements()
        throw( typename Collection<T>::RuntimeException_t )
    {
        return Collection<T>::hasElements();
    }

    // XNameAccess : XElementAccess
    virtual typename Collection<T>::Any_t SAL_CALL getByName(
        const rtl::OUString& aName )
        throw( typename Collection<T>::NoSuchElementException_t,
               typename Collection<T>::WrappedTargetException_t,
               typename Collection<T>::RuntimeException_t )
    {
        if( hasItem( aName ) )
            return com::sun::star::uno::makeAny( getItem( aName ) );
        else
            throw typename Collection<T>::NoSuchElementException_t();

    }

    virtual Names_t SAL_CALL getElementNames()
        throw( typename Collection<T>::RuntimeException_t )
    {
        return getNames();
    }

    virtual sal_Bool SAL_CALL hasByName(
        const rtl::OUString& aName )
        throw( typename Collection<T>::RuntimeException_t )
    {
        return hasItem( aName ) ? sal_True : sal_False;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
