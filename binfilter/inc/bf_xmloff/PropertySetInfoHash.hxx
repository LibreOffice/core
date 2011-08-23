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
#ifndef _XMLOFF_PROPERTYSETINFOHASH_HXX
#define _XMLOFF_PROPERTYSETINFOHASH_HXX

#include <string.h>

#include <bf_xmloff/PropertySetInfoKey.hxx>
namespace binfilter {

struct PropertySetInfoHash
{
    inline size_t operator()( const PropertySetInfoKey& r ) const;
    inline bool operator()( const PropertySetInfoKey& r1,
                               const PropertySetInfoKey& r2 ) const;
};

inline size_t PropertySetInfoHash::operator()(
        const PropertySetInfoKey& r ) const
{
    const sal_Int32* pBytesAsInt32Array =
        (const sal_Int32*)r.aImplementationId.getConstArray();
    sal_Int32 nId32 =	pBytesAsInt32Array[0] ^
                        pBytesAsInt32Array[1] ^
                        pBytesAsInt32Array[2] ^
                        pBytesAsInt32Array[3];
    return (size_t)nId32 ^ (size_t)r.xPropInfo.get();
}

inline bool PropertySetInfoHash::operator()(
        const PropertySetInfoKey& r1,
        const PropertySetInfoKey& r2 ) const
{
    if( r1.xPropInfo != r2.xPropInfo )
        return sal_False;

    const sal_Int8* pId1 = r1.aImplementationId.getConstArray();
    const sal_Int8* pId2 = r2.aImplementationId.getConstArray();
    return memcmp( pId1, pId2, 16 * sizeof( sal_Int8 ) ) == 0;
}
}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
