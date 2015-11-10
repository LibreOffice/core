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

#include <rtl/ustring.hxx>
#include "xmlitmap.hxx"

using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_TOKEN_INVALID;

// TODO: optimize this!
class SvXMLItemMapEntries_impl
{
public:
    SvXMLItemMapEntry* mpEntries;
    sal_uInt16 mnCount;
};

SvXMLItemMapEntries::SvXMLItemMapEntries( SvXMLItemMapEntry* pEntries )
{
    mpImpl = new SvXMLItemMapEntries_impl;
    mpImpl->mpEntries = pEntries;

    mpImpl->mnCount = 0;
    while( pEntries->eLocalName != XML_TOKEN_INVALID )
    {
        pEntries++;
        mpImpl->mnCount++;
    }
}

SvXMLItemMapEntries::~SvXMLItemMapEntries()
{
    delete mpImpl;
}

SvXMLItemMapEntry* SvXMLItemMapEntries::getByName(  sal_uInt16 nNameSpace,
                                                    const OUString& rString,
                                                    SvXMLItemMapEntry* pStartAt /* = NULL */  ) const
{
    SvXMLItemMapEntry* pMap =
        (pStartAt && (pStartAt->eLocalName!=XML_TOKEN_INVALID)) ?
                                           &(pStartAt[1]) : mpImpl->mpEntries;
    while( pMap && (pMap->eLocalName != XML_TOKEN_INVALID) )
    {
        if( pMap->nNameSpace == nNameSpace &&
            IsXMLToken( rString, pMap->eLocalName ) )
            break;
        pMap++;
    }

    return (pMap && (pMap->eLocalName != XML_TOKEN_INVALID)) ? pMap : nullptr;
}

SvXMLItemMapEntry& SvXMLItemMapEntries::getByIndex( sal_uInt16 nIndex ) const
{
    return mpImpl->mpEntries[nIndex];
}

sal_uInt16 SvXMLItemMapEntries::getCount() const
{
    return mpImpl->mnCount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
