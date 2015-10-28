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

#ifndef INCLUDED_SD_SOURCE_UI_ANIMATIONS_STLPROPERTYSET_HXX
#define INCLUDED_SD_SOURCE_UI_ANIMATIONS_STLPROPERTYSET_HXX

#include <com/sun/star/beans/PropertyValue.hpp>

#include <list>
#include <map>

namespace sd
{

const sal_Int32 STLPropertyState_DEFAULT = 0;
const sal_Int32 STLPropertyState_DIRECT = 1;
const sal_Int32 STLPropertyState_AMBIGUOUS = 3;

struct STLPropertyMapEntry
{
    css::uno::Any maValue;
    sal_Int32 mnState;

    STLPropertyMapEntry()
        : mnState( STLPropertyState_AMBIGUOUS ) {}
    STLPropertyMapEntry( css::uno::Any aValue, sal_Int32 nState = STLPropertyState_DEFAULT )
        : maValue( aValue ), mnState( nState ) {}

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
    void setPropertyValue( sal_Int32 nHandle, const css::uno::Any& rValue, sal_Int32 nState = STLPropertyState_DIRECT );
    css::uno::Any getPropertyValue( sal_Int32 nHandle ) const;

    sal_Int32 getPropertyState( sal_Int32 nHandle ) const;
    void setPropertyState( sal_Int32 nHandle, sal_Int32 nState );

private:
    bool findProperty( sal_Int32 nHandle, PropertyMapIter& rIter );
    bool findProperty( sal_Int32 nHandle, PropertyMapConstIter& rIter ) const;

private:
    PropertyMap maPropertyMap;
};

}

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_STLPROPERTYSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
