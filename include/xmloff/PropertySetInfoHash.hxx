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
#ifndef _XMLOFF_PROPERTYSETINFOHASH_HXX
#define _XMLOFF_PROPERTYSETINFOHASH_HXX

#include <xmloff/PropertySetInfoKey.hxx>

#include <string.h>
#include <memory>

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
    sal_Int32 nId32 =   pBytesAsInt32Array[0] ^
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
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
