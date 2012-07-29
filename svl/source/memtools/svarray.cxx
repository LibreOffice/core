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

#include <svl/svarray.hxx>

SvPtrarr::SvPtrarr( sal_uInt16 nInit )
    : pData (0),
      nFree (nInit),
      nA    (0)
{
    if( nInit )
    {
        pData = (VoidPtr*)(rtl_allocateMemory(sizeof(VoidPtr) * nInit));
        OSL_ENSURE( pData, "CTOR, allocate");
    }
}

void SvPtrarr::_resize (size_t n)
{
    sal_uInt16 nL = ((n < USHRT_MAX) ? sal_uInt16(n) : USHRT_MAX);
    VoidPtr* pE = (VoidPtr*)(rtl_reallocateMemory (pData, sizeof(VoidPtr) * nL));
    if ((pE != 0) || (nL == 0))
    {
        pData = pE;
        nFree = nL - nA;
    }
}

void SvPtrarr::Insert( const VoidPtr& aE, sal_uInt16 nP )
{
    OSL_ENSURE(nP <= nA && nA < USHRT_MAX, "Ins 1");
    if (nFree < 1)
        _resize (nA + ((nA > 1) ? nA : 1));
    if( pData && nP < nA )
        memmove( pData+nP+1, pData+nP, (nA-nP) * sizeof( VoidPtr ));
    *(pData+nP) = (VoidPtr&)aE;
    ++nA; --nFree;
}

void SvPtrarr::Insert( const VoidPtr* pE, sal_uInt16 nL, sal_uInt16 nP )
{
    OSL_ENSURE(nP<=nA && ((long)nA+nL)<USHRT_MAX,"Ins n");
    if (nFree < nL)
        _resize (nA + ((nA > nL) ? nA : nL));
    if( pData && nP < nA )
        memmove( pData+nP+nL, pData+nP, (nA-nP) * sizeof( VoidPtr ));
    if( pE )
        memcpy( pData+nP, pE, nL * sizeof( VoidPtr ));
    nA = nA + nL; nFree = nFree - nL;
}

void SvPtrarr::Remove( sal_uInt16 nP, sal_uInt16 nL )
{
    if( !nL )
        return;
    OSL_ENSURE( nP < nA && nP + nL <= nA,"Del");
    if( pData && nP+1 < nA )
        memmove( pData+nP, pData+nP+nL, (nA-nP-nL) * sizeof( VoidPtr ));
    nA = nA - nL; nFree = nFree + nL;
    if (nFree > nA)
        _resize (nA);
}

sal_uInt16 SvPtrarr::GetPos( const VoidPtr& aElement ) const
{
    sal_uInt16 n;
    for( n=0; n < nA && *(GetData()+n) != aElement; ) n++;
    return ( n >= nA ? USHRT_MAX : n );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
