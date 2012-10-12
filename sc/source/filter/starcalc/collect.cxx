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

#include "collect.hxx"

#include <string.h>

#define MAXCOLLECTIONSIZE       16384
#define MAXDELTA                1024

// -----------------------------------------------------------------------

ScDataObject::~ScDataObject()
{
}

//------------------------------------------------------------------------
// Collection
//------------------------------------------------------------------------

static void lcl_DeleteScDataObjects( ScDataObject** p, sal_uInt16 nCount )
{
    if ( p )
    {
        for (sal_uInt16 i = 0; i < nCount; i++) delete p[i];
        delete[] p;
        p = NULL;
    }
}

ScCollection::ScCollection(sal_uInt16 nLim, sal_uInt16 nDel) :
    nCount ( 0 ),
    nLimit ( nLim ),
    nDelta ( nDel ),
    pItems ( NULL )
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
        pItems ( NULL )
{
    *this = rCollection;
}

//------------------------------------------------------------------------

ScCollection::~ScCollection()
{
    lcl_DeleteScDataObjects( pItems, nCount );
}

//------------------------------------------------------------------------
sal_uInt16 ScCollection::GetCount() const { return nCount; }

//------------------------------------------------------------------------

sal_Bool ScCollection::AtInsert(sal_uInt16 nIndex, ScDataObject* pScDataObject)
{
    if ((nCount < MAXCOLLECTIONSIZE) && (nIndex <= nCount) && pItems)
    {
        if (nCount == nLimit)
        {
            ScDataObject** pNewItems = new ScDataObject*[nLimit + nDelta];
            if (!pNewItems)
                return false;
            nLimit = sal::static_int_cast<sal_uInt16>( nLimit + nDelta );
            memmove(pNewItems, pItems, nCount * sizeof(ScDataObject*));
            delete[] pItems;
            pItems = pNewItems;
        }
        if (nCount > nIndex)
            memmove(&pItems[nIndex + 1], &pItems[nIndex], (nCount - nIndex) * sizeof(ScDataObject*));
        pItems[nIndex] = pScDataObject;
        nCount++;
        return sal_True;
    }
    return false;
}

//------------------------------------------------------------------------

sal_Bool ScCollection::Insert(ScDataObject* pScDataObject)
{
    return AtInsert(nCount, pScDataObject);
}

//------------------------------------------------------------------------

ScDataObject* ScCollection::At(sal_uInt16 nIndex) const
{
    if (nIndex < nCount)
        return pItems[nIndex];
    else
        return NULL;
}

//------------------------------------------------------------------------

sal_uInt16 ScCollection::IndexOf(ScDataObject* pScDataObject) const
{
    sal_uInt16 nIndex = 0xffff;
    for (sal_uInt16 i = 0; ((i < nCount) && (nIndex == 0xffff)); i++)
    {
        if (pItems[i] == pScDataObject) nIndex = i;
    }
    return nIndex;
}

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

ScDataObject*   ScCollection::Clone() const
{
    return new ScCollection(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
