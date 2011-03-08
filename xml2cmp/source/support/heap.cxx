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

#include <string.h>
#include "heap.hxx"


#include <iostream>
#include <stdlib.h>
#define AssertionOf(x)  {if (!(x)) {std::cerr << "Assertion failed: " << #x << __FILE__ << __LINE__ << std::endl; exit(3); }}

#ifdef UNX
#define stricmp strcasecmp
#endif



Heap::Heap(unsigned i_nWidth)
    :   dpColumnsArray(new Column[i_nWidth]),
        nColumnsArraySize(i_nWidth),
        nActiveColumn(nColumnsArraySize-1)
{
    for ( unsigned i = 0; i < nColumnsArraySize; i++)
    {
        dpColumnsArray[i] = 0;
    }  // end for
}

Heap::~Heap()
{
    for ( unsigned i = 0; i < nColumnsArraySize; i++)
    {
        HeapItem * & rColumn = dpColumnsArray[i];
        for ( HeapItem * pValue = rColumn; pValue != 0; pValue = rColumn )
        {
            rColumn = rColumn->Next();
            delete pValue;
        }
    }  // end for

    delete [] dpColumnsArray;
}

void
Heap::InsertValue( const char *     i_sKey,
                   const char *     i_sValue )
{
    HeapItem * pSearch1 = 0;
    HeapItem * pSearch2 = 0;
    HeapItem * pNew = new HeapItem(i_sKey, i_sValue);

    IncColumn();
    pSearch1 = ActiveColumn();

    if ( pSearch1 != 0 ? *pNew < *pSearch1 : true )
    {
        pNew->SetNext( pSearch1 );
        ActiveColumn() = pNew;

        if ( pNew->Next() != 0)
        {
            AssertionOf( *pNew <= *pNew->Next() );
        }

        return;
    }

    do
    {
        pSearch2 = pSearch1;
        pSearch1 = pSearch1->Next();

        if ( pSearch1 != 0 ? *pNew < *pSearch1 : true )
        {
            pNew->SetNext( pSearch1 );
            pSearch2->SetNext(pNew);


        AssertionOf( *pSearch2 <= *pNew );
        if ( pNew->Next() != 0)
        {
            AssertionOf( *pNew <= *pNew->Next() );
        }

        }
    } while (pSearch2->Next() != pNew);
}


Simstr sKey1;
Simstr sValue1;
Simstr sKey2;
Simstr sValue2;
int nCol1 = 0;
int nCol2 = 0;


HeapItem *
Heap::ReleaseTop()
{
    unsigned nRetColumn = 0;
    HeapItem * ret = dpColumnsArray[0];
    HeapItem * pSearch = 0;

    for ( unsigned i = 1; i < nColumnsArraySize; ++i )
    {
        pSearch = dpColumnsArray[i];
        if (pSearch != 0)
        {
            if ( ret == 0 ? true : *pSearch < *ret)
            {
                ret = pSearch;
                nRetColumn = i;
            }
        }
    }   // for

    if (ret != 0)
    {
        dpColumnsArray[nRetColumn] = ret->Next();
    }
    return ret;
}

void
Heap::IncColumn()
{
    if (++nActiveColumn >= nColumnsArraySize)
        nActiveColumn = 0;
}



HeapItem::HeapItem( const char *        i_sKey,
                    const char *        i_sValue )
    :   sValue(i_sValue),
        sKey(i_sKey),
        pNext(0)
{
}

HeapItem::~HeapItem()
{
}

bool
HeapItem::operator<( const HeapItem & i_rOther ) const
{
    int ret = stricmp(sKey.str(), i_rOther.sKey.str());
    if (ret == 0)
        ret = strcmp(sKey.str(), i_rOther.sKey.str());
    if (ret == 0)
        ret = stricmp(sValue.str(), i_rOther.sValue.str());
    if (ret == 0)
        ret = strcmp(sValue.str(), i_rOther.sValue.str());
    return ret < 0;
}

const Simstr &
HeapItem::Value() const
{
    return sValue;
}

const Simstr &
HeapItem::Key() const
{
    return sKey;
}

HeapItem *
HeapItem::Next() const
{
    return pNext;
}

void
HeapItem::SetNext( HeapItem * i_pNext )
{
    pNext = i_pNext;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
