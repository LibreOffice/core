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
#include "precompiled_sw.hxx"
#include <rtl/ustring.hxx>
#include "xmlitmap.hxx"

using rtl::OUString;
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

    return (pMap->eLocalName != XML_TOKEN_INVALID) ? pMap : NULL;
}

SvXMLItemMapEntry* SvXMLItemMapEntries::getByIndex( sal_uInt16 nIndex ) const
{
    return &mpImpl->mpEntries[nIndex];
}

sal_uInt16 SvXMLItemMapEntries::getCount() const
{
    return mpImpl->mnCount;
}

