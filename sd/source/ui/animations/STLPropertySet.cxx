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

using namespace com::sun::star::beans;

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
    STLPropertyMapEntry aEntry( rValue, STLPropertyState_DEFAULT );
    maPropertyMap[ nHandle ] = aEntry;
}

void STLPropertySet::setPropertyValue( sal_Int32 nHandle, const Any& rValue, sal_Int32 /* nState = STLPropertyState_DIRECT */ )
{
    PropertyMapIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        (*aIter).second.mnState = STLPropertyState_DIRECT;
        (*aIter).second.maValue = rValue;
    }
    else
    {
        OSL_FAIL( "sd::STLPropertySet::setPropertyValue(), unknown property!" );
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
        OSL_FAIL( "sd::STLPropertySet::setPropertyValue(), unknown property!" );

        Any aAny;
        return aAny;
    }
}

sal_Int32 STLPropertySet::getPropertyState( sal_Int32 nHandle ) const
{
    PropertyMapConstIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        return (*aIter).second.mnState;
    }
    else
    {
        OSL_FAIL( "sd::STLPropertySet::setPropertyState(), unknown property!" );
        return STLPropertyState_AMBIGUOUS;
    }
}

void STLPropertySet::setPropertyState( sal_Int32 nHandle, sal_Int32 nState )
{
    PropertyMapIter aIter;
    if( findProperty( nHandle, aIter ) )
    {
        (*aIter).second.mnState = nState;
    }
    else
    {
        OSL_FAIL( "sd::STLPropertySet::setPropertyState(), unknown property!" );
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
