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

#include "STLPropertySet.hxx"
#include <sal/log.hxx>

using com::sun::star::uno::Any;

namespace sd
{

STLPropertySet::STLPropertySet()
{
}

STLPropertySet::~STLPropertySet()
{
}

void STLPropertySet::setPropertyDefaultValue( sal_Int32 nHandle, const Any& rValue )
{
    STLPropertyMapEntry aEntry( rValue );
    maPropertyMap[ nHandle ] = aEntry;
}

void STLPropertySet::setPropertyValue( sal_Int32 nHandle, const Any& rValue )
{
    PropertyMapIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        (*aIter).second.mnState = STLPropertyState::Direct;
        (*aIter).second.maValue = rValue;
    }
    else
    {
        SAL_WARN("sd", "sd::STLPropertySet::setPropertyValue(), unknown property!");
    }
}

Any STLPropertySet::getPropertyValue( sal_Int32 nHandle ) const
{
    PropertyMapConstIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        return (*aIter).second.maValue;
    }
    else
    {
        SAL_WARN("sd", "sd::STLPropertySet::getPropertyValue(), unknown property!");

        Any aAny;
        return aAny;
    }
}

STLPropertyState STLPropertySet::getPropertyState( sal_Int32 nHandle ) const
{
    PropertyMapConstIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        return (*aIter).second.mnState;
    }
    else
    {
        SAL_WARN("sd", "sd::STLPropertySet::getPropertyState(), unknown property!");
        return STLPropertyState::Ambiguous;
    }
}

void STLPropertySet::setPropertyState( sal_Int32 nHandle, STLPropertyState nState )
{
    PropertyMapIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        (*aIter).second.mnState = nState;
    }
    else
    {
        SAL_WARN("sd","sd::STLPropertySet::setPropertyState(), unknown property!");
    }
}

bool STLPropertySet::findProperty( sal_Int32 nHandle, PropertyMapIter& rIter )
{
    rIter = maPropertyMap.find(nHandle);
    return( rIter != maPropertyMap.end() );
}

bool STLPropertySet::findProperty( sal_Int32 nHandle, PropertyMapConstIter& rIter ) const
{
    rIter = maPropertyMap.find(nHandle);
    return( rIter != maPropertyMap.end() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
