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



#include "ring.hxx"


/*************************************************************************
|*
|*    Ring::Ring()
|*
|*    Ersterstellung    VB  02.07.91
|*    Letzte Aenderung  JP 10.10.97
|*
*************************************************************************/

Ring::Ring( Ring *pObj )
{
    if( !pObj )
        pNext = this, pPrev = this;
    else
    {
        pNext = pObj;
        pPrev = pObj->pPrev;
        pObj->pPrev = this;
        pPrev->pNext = this;
    }
}

/*************************************************************************
|*
|*    Ring::~Ring()
|*
|*    Ersterstellung    VB  02.07.91
|*    Letzte Aenderung  JP 10.10.97
|*
*************************************************************************/

Ring::~Ring()
{
    pNext->pPrev = pPrev;
    pPrev->pNext = pNext;
}

/*************************************************************************
|*
|*    Ring::MoveTo
|*
|*    Ersterstellung    VB 4.3.91
|*    Letzte Aenderung  JP 10.10.97
|*
*************************************************************************/

void Ring::MoveTo(Ring *pDestRing)
{
    // loeschen aus dem alten
    pNext->pPrev = pPrev;
    pPrev->pNext = pNext;

    // im neuen einfuegen
    if( pDestRing )
    {
        pNext = pDestRing;
        pPrev = pDestRing->pPrev;
        pDestRing->pPrev = this;
        pPrev->pNext = this;
    }
    else
        pNext = pPrev = this;

}

void Ring::MoveRingTo(Ring *pDestRing)
{
    // den gesamten Ring in den DestRing einfuegen
    Ring* pMyPrev = pPrev;
    Ring* pDestPrev = pDestRing->pPrev;

    pMyPrev->pNext = pDestRing;
    pDestPrev->pNext = this;
    pDestRing->pPrev = pMyPrev;
    pPrev = pDestPrev;
}

sal_uInt32 Ring::numberOf() const
{
    sal_uInt32 nRet = 1;
    const Ring* pNxt = pNext;
    while( pNxt != this )
    {
        ++nRet;
        pNxt = pNxt->GetNext();
    }
    return nRet;
}


