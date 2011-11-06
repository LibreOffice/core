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


