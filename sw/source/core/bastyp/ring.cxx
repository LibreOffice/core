/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ring.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:29:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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


