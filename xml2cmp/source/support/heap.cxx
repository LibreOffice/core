/*************************************************************************
 *
 *  $RCSfile: heap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2001-03-23 13:23:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string.h>
#include "heap.hxx"


#include <iostream>
#include <stdlib.h>
#define AssertionOf(x)  {if (!(x)) {cerr << "Assertion failed: " << #x << __FILE__ << __LINE__ << std::endl; exit(3); }}

#ifdef UNX
#define stricmp strcasecmp
#endif


using std::cerr;

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


