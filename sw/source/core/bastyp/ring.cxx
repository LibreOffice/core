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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
