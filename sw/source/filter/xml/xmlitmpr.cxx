/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

