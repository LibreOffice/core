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

#include "ring.hxx"

Ring::Ring( Ring *pObj )
{
    if( !pObj )
    {
        pNext = this, pPrev = this;
    }
    else
    {
        pNext = pObj;
        pPrev = pObj->pPrev;
        pObj->pPrev = this;
        pPrev->pNext = this;
    }
}

Ring::~Ring()
{
    pNext->pPrev = pPrev;
    pPrev->pNext = pNext;
}

void Ring::MoveTo(Ring *pDestRing)
{
    // delete from "old"
    pNext->pPrev = pPrev;
    pPrev->pNext = pNext;

    // insert into "new"
    if( pDestRing )
    {
        pNext = pDestRing;
        pPrev = pDestRing->pPrev;
        pDestRing->pPrev = this;
        pPrev->pNext = this;
    }
    else
    {
        pNext = pPrev = this;
    }

}

void Ring::MoveRingTo(Ring *pDestRing)
{
    // insert the whole ring into DestRing
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
