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

#ifndef _SD_STLPROPERTYSET_HXX
#define _SD_STLPROPERTYSET_HXX

#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/stl_types.hxx>

#include <list>
#include <map>

namespace sd
{

const sal_Int32 STLPropertyState_DEFAULT = 0;
const sal_Int32 STLPropertyState_DIRECT = 1;
const sal_Int32 STLPropertyState_AMBIGUOUS = 3;

struct STLPropertyMapEntry
{
    ::com::sun::star::uno::Any maValue;
    sal_Int32 mnState;

    STLPropertyMapEntry()
        : mnState( STLPropertyState_AMBIGUOUS ) {}
    STLPropertyMapEntry( ::com::sun::star::uno::Any aValue, sal_Int32 nState = STLPropertyState_DEFAULT )
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

    void setPropertyDefaultValue( sal_Int32 nHandle, const com::sun::star::uno::Any& rValue );
    void setPropertyValue( sal_Int32 nHandle, const com::sun::star::uno::Any& rValue, sal_Int32 nState = STLPropertyState_DIRECT );
    ::com::sun::star::uno::Any getPropertyValue( sal_Int32 nHandle ) const;

    sal_Int32 getPropertyState( sal_Int32 nHandle ) const;
    void setPropertyState( sal_Int32 nHandle, sal_Int32 nState );

private:
    bool findProperty( sal_Int32 nHandle, PropertyMapIter& rIter );
    bool findProperty( sal_Int32 nHandle, PropertyMapConstIter& rIter ) const;

private:
    PropertyMap maPropertyMap;
};

}

#endif // _SD_STLPROPERTYSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
