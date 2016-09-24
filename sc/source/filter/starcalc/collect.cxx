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

#include "collect.hxx"

#include <string.h>

#define MAXCOLLECTIONSIZE       16384
#define MAXDELTA                1024

ScDataObject::~ScDataObject()
{
}

// Collection

static void lcl_DeleteScDataObjects( ScDataObject** p, sal_uInt16 nCount )
{
    if ( p )
    {
        for (sal_uInt16 i = 0; i < nCount; i++) delete p[i];
        delete[] p;
    }
}

ScCollection::ScCollection(sal_uInt16 nLim, sal_uInt16 nDel) :
    nCount ( 0 ),
    nLimit ( nLim ),
    nDelta ( nDel ),
    pItems ( nullptr )
{
    if (nDelta > MAXDELTA)
        nDelta = MAXDELTA;
    else if (nDelta == 0)
        nDelta = 1;
    if (nLimit > MAXCOLLECTIONSIZE)
        nLimit = MAXCOLLECTIONSIZE;
    else if (nLimit < nDelta)
        nLimit = nDelta;
    pItems = new ScDataObject*[nLimit];
}

ScCollection::ScCollection(const ScCollection& rCollection)
    :   ScDataObject(),
        nCount ( 0 ),
        nLimit ( 0 ),
        nDelta ( 0 ),
        pItems ( nullptr )
{
    *this = rCollection;
}

ScCollection::~ScCollection()
{
    lcl_DeleteScDataObjects( pItems, nCount );
}

bool ScCollection::AtInsert(sal_uInt16 nIndex, ScDataObject* pScDataObject)
{
    if ((nCount < MAXCOLLECTIONSIZE) && (nIndex <= nCount) && pItems)
    {
        if (nCount == nLimit)
        {
            ScDataObject** pNewItems = new ScDataObject*[nLimit + nDelta];
            if (!pNewItems)
                return false;
            nLimit = sal::static_int_cast<sal_uInt16>( nLimit + nDelta );
            memcpy(pNewItems, pItems, nCount * sizeof(ScDataObject*));
            delete[] pItems;
            pItems = pNewItems;
        }
        if (nCount > nIndex)
            memmove(&pItems[nIndex + 1], &pItems[nIndex], (nCount - nIndex) * sizeof(ScDataObject*));
        pItems[nIndex] = pScDataObject;
        nCount++;
        return true;
    }
    return false;
}

bool ScCollection::Insert(ScDataObject* pScDataObject)
{
    return AtInsert(nCount, pScDataObject);
}

ScDataObject* ScCollection::At(sal_uInt16 nIndex) const
{
    if (nIndex < nCount)
        return pItems[nIndex];
    else
        return nullptr;
}

ScCollection& ScCollection::operator=( const ScCollection& r )
{
    // Check for self-assignment
    if (this == &r)
       return *this;

    lcl_DeleteScDataObjects( pItems, nCount );

    nCount = r.nCount;
    nLimit = r.nLimit;
    nDelta = r.nDelta;
    pItems = new ScDataObject*[nLimit];
    for ( sal_uInt16 i=0; i<nCount; i++ )
        pItems[i] = r.pItems[i]->Clone();

    return *this;
}

ScDataObject*   ScCollection::Clone() const
{
    return new ScCollection(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
