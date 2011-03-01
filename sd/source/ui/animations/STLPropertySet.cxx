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
#include "precompiled_sd.hxx"
#include <tools/debug.hxx>
#include "STLPropertySet.hxx"

using namespace com::sun::star::beans;

using rtl::OUString;
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
