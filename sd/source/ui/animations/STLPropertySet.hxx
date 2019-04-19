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

#pragma once

#include <map>
#include <com/sun/star/uno/Any.hxx>

namespace sd
{

enum class STLPropertyState {
    Default = 0,
    Direct = 1,
    Ambiguous = 3
};

struct STLPropertyMapEntry
{
    css::uno::Any maValue;
    STLPropertyState mnState;

    STLPropertyMapEntry()
        : mnState( STLPropertyState::Ambiguous ) {}
    explicit STLPropertyMapEntry(css::uno::Any aValue)
        : maValue( aValue ), mnState( STLPropertyState::Default ) {}

};

typedef std::map<sal_Int32, STLPropertyMapEntry > PropertyMap;
typedef PropertyMap::iterator PropertyMapIter;
typedef PropertyMap::const_iterator PropertyMapConstIter;

class STLPropertySet
{
public:
    STLPropertySet();
    ~STLPropertySet();

    void setPropertyDefaultValue( sal_Int32 nHandle, const css::uno::Any& rValue );
    void setPropertyValue( sal_Int32 nHandle, const css::uno::Any& rValue );
    css::uno::Any getPropertyValue( sal_Int32 nHandle ) const;

    STLPropertyState getPropertyState( sal_Int32 nHandle ) const;
    void setPropertyState( sal_Int32 nHandle, STLPropertyState nState );

private:
    bool findProperty( sal_Int32 nHandle, PropertyMapIter& rIter );
    bool findProperty( sal_Int32 nHandle, PropertyMapConstIter& rIter ) const;

private:
    PropertyMap maPropertyMap;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
