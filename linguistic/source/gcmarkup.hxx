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

#ifndef INCLUDED_LINGUISTIC_SOURCE_GCMARKUP_HXX
#define INCLUDED_LINGUISTIC_SOURCE_GCMARKUP_HXX

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include "com/sun/star/uno/Any.hxx"
#include "cppu/unotype.hxx"

#include <cppuhelper/implbase.hxx>

#include <map>

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

/** Implementation of the css::container::XStringKeyMap interface
 */
class SwXStringKeyMap:
    public ::cppu::WeakImplHelper<
        css::container::XStringKeyMap>
{
public:
    SwXStringKeyMap();

    virtual css::uno::Any SAL_CALL getValue(const OUString & aKey) override;
    virtual sal_Bool SAL_CALL hasValue(const OUString & aKey) override;
    virtual void SAL_CALL insertValue(const OUString & aKey, const css::uno::Any & aValue) override;
    virtual ::sal_Int32 SAL_CALL getCount() override;
    virtual OUString SAL_CALL getKeyByIndex(::sal_Int32 nIndex) override;
    virtual css::uno::Any SAL_CALL getValueByIndex(::sal_Int32 nIndex) override;

private:
    SwXStringKeyMap(SwXStringKeyMap &) = delete;
    void operator =(SwXStringKeyMap &) = delete;

    ~SwXStringKeyMap() override {};

    std::map< OUString, css::uno::Any > maMap;
};

SwXStringKeyMap::SwXStringKeyMap() {
}

// SwXStringKeyMap::~SwXStringKeyMap() {
// }

void SAL_CALL SwXStringKeyMap::insertValue(const OUString & aKey, const css::uno::Any & aValue)
{
    std::map< OUString, css::uno::Any >::const_iterator aIter = maMap.find( aKey );
    if ( aIter != maMap.end() )
    throw css::container::ElementExistException();

    maMap[ aKey ] = aValue;
}

css::uno::Any SAL_CALL SwXStringKeyMap::getValue(const OUString & aKey)
{
    std::map< OUString, css::uno::Any >::const_iterator aIter = maMap.find( aKey );
    if ( aIter == maMap.end() )
        throw css::container::NoSuchElementException();

    return (*aIter).second;
}

sal_Bool SAL_CALL SwXStringKeyMap::hasValue(const OUString & aKey)
{
    return maMap.find( aKey ) != maMap.end();
}

::sal_Int32 SAL_CALL SwXStringKeyMap::getCount()
{
    return maMap.size();
}

OUString SAL_CALL SwXStringKeyMap::getKeyByIndex(::sal_Int32 nIndex)
{
    if ( static_cast<sal_uInt32>(nIndex) >= maMap.size() )
        throw css::lang::IndexOutOfBoundsException();

    return OUString();
}

css::uno::Any SAL_CALL SwXStringKeyMap::getValueByIndex(::sal_Int32 nIndex)
{
    if ( static_cast<sal_uInt32>(nIndex) >= maMap.size() )
        throw css::lang::IndexOutOfBoundsException();

    return css::uno::Any();
}


#endif
