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

#ifndef _COMPHELPER_PROPERTYINFOHASH_HXX_
#define _COMPHELPER_PROPERTYINFOHASH_HXX_

#include <rtl/ustring.hxx>
#include <comphelper/TypeGeneration.hxx>
#include <boost/unordered_map.hpp>
namespace comphelper
{
    struct PropertyInfo
    {
        const sal_Char* mpName;
        sal_uInt16 mnNameLen;
        sal_Int32 mnHandle;
        CppuTypes meCppuType;
        sal_Int16 mnAttributes;
        sal_uInt8 mnMemberId;
    };
    struct PropertyData
    {
        sal_uInt8 mnMapId;
        PropertyInfo *mpInfo;
        PropertyData ( sal_uInt8 nMapId, PropertyInfo *pInfo )
        : mnMapId ( nMapId )
        , mpInfo ( pInfo ) {}
    };
    struct eqFunc
    {
        sal_Bool operator()( const OUString &r1,
                             const OUString &r2) const
        {
            return r1 == r2;
        }
    };
}

typedef boost::unordered_map < OUString,
                        ::comphelper::PropertyInfo*,
                        OUStringHash,
                        ::comphelper::eqFunc > PropertyInfoHash;
typedef boost::unordered_map < OUString,
                        ::comphelper::PropertyData*,
                        OUStringHash,
                        ::comphelper::eqFunc > PropertyDataHash;
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
