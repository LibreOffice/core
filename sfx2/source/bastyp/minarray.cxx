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


#include <sfx2/minarray.hxx>
#include <algorithm>

// -----------------------------------------------------------------------

SfxPtrArr::SfxPtrArr( sal_uInt8 nInitSize, sal_uInt8 nGrowSize ):
    nUsed( 0 ),
    nGrow( nGrowSize ? nGrowSize : 1 ),
    nUnused( nInitSize )
{
    sal_uInt16 nMSCBug = nInitSize;

    if ( nMSCBug > 0 )
        pData = new void*[nMSCBug];
    else
        pData = 0;
}

// -----------------------------------------------------------------------

SfxPtrArr::SfxPtrArr( const SfxPtrArr& rOrig )
{
    nUsed = rOrig.nUsed;
    nGrow = rOrig.nGrow;
    nUnused = rOrig.nUnused;

    if ( rOrig.pData != 0 )
    {
        pData = new void*[nUsed+nUnused];
        memcpy( pData, rOrig.pData, nUsed*sizeof(void*) );
    }
    else
        pData = 0;
}

// -----------------------------------------------------------------------

SfxPtrArr::~SfxPtrArr()
{
    delete [] pData;
}

// -----------------------------------------------------------------------

SfxPtrArr& SfxPtrArr::operator=( const SfxPtrArr& rOrig )
{

    delete [] pData;

    nUsed = rOrig.nUsed;
    nGrow = rOrig.nGrow;
    nUnused = rOrig.nUnused;

    if ( rOrig.pData != 0 )
    {
        pData = new void*[nUsed+nUnused];
        memcpy( pData, rOrig.pData, nUsed*sizeof(void*) );
    }
    else
        pData = 0;
    return *this;
}

// -----------------------------------------------------------------------

void SfxPtrArr::Append( void* aElem )
{
    DBG_ASSERT( sal::static_int_cast< unsigned >(nUsed+1) < ( USHRT_MAX / sizeof(void*) ), "array too large" );
    // Does the Array need to be copied?
    if ( nUnused == 0 )
    {
        sal_uInt16 nNewSize = (nUsed == 1) ? (nGrow==1 ? 2 : nGrow) : nUsed+nGrow;
        void** pNewData = new void*[nNewSize];
        if ( pData )
        {
            DBG_ASSERT( nUsed <= nNewSize, "" );
            memcpy( pNewData, pData, sizeof(void*)*nUsed );
            delete [] pData;
        }
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize-nUsed);
        pData = pNewData;
    }

    // now write at the back in the open space
    pData[nUsed] = aElem;
    ++nUsed;
    --nUnused;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxPtrArr::Remove( sal_uInt16 nPos, sal_uInt16 nLen )
{
    // Adjust nLen, thus to avoid deleting beyond the end
    nLen = std::min( (sal_uInt16)(nUsed-nPos), nLen );

    // simple problems require simple solutions!
    if ( nLen == 0 )
        return 0;

    // Maybe no one will remain
    if ( (nUsed-nLen) == 0 )
    {
        delete [] pData;
        pData = 0;
        nUsed = 0;
        nUnused = 0;
        return nLen;
    }

    // Determine whether the array has physically shrunk...
    if ( (nUnused+nLen) >= nGrow )
    {
        // reduce (rounded up) to the next Grow-border
        sal_uInt16 nNewUsed = nUsed-nLen;
        sal_uInt16 nNewSize = ((nNewUsed+nGrow-1)/nGrow) * nGrow;
        DBG_ASSERT( nNewUsed <= nNewSize && nNewUsed+nGrow > nNewSize,
                    "shrink size computation failed" );
        void** pNewData = new void*[nNewSize];
        if ( nPos > 0 )
        {
            DBG_ASSERT( nPos <= nNewSize, "" );
            memcpy( pNewData, pData, sizeof(void*)*nPos );
        }
        if ( nNewUsed != nPos )
            memcpy( pNewData+nPos, pData+nPos+nLen,
                    sizeof(void*)*(nNewUsed-nPos) );
        delete [] pData;
        pData = pNewData;
        nUsed = nNewUsed;
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize - nNewUsed);
        return nLen;
    }

    // in all other cases, only push together
    if ( nUsed-nPos-nLen > 0 )
        memmove( pData+nPos, pData+nPos+nLen, (nUsed-nPos-nLen)*sizeof(void*) );
    nUsed = nUsed - nLen;
    nUnused = sal::static_int_cast< sal_uInt8 >(nUnused + nLen);
    return nLen;
}

// -----------------------------------------------------------------------

sal_Bool SfxPtrArr::Remove( void* aElem )
{
    // simple tasks ...
    if ( nUsed == 0 )
        return sal_False;

    // backwards, since most of the last is first removed
    void* *pIter = pData + nUsed - 1;
    for ( sal_uInt16 n = 0; n < nUsed; ++n, --pIter )
        if ( *pIter == aElem )
        {
            Remove(nUsed-n-1, 1);
            return sal_True;
        }
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SfxPtrArr::Contains( const void* rItem ) const
{
    if ( !nUsed )
        return sal_False;

    for ( sal_uInt16 n = 0; n < nUsed; ++n )
    {
        void* p = GetObject(n);
        if ( p == rItem )
            return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void SfxPtrArr::Insert( sal_uInt16 nPos, void* rElem )
{
    DBG_ASSERT( sal::static_int_cast< unsigned >(nUsed+1) < ( USHRT_MAX / sizeof(void*) ), "array too large" );
    // Does the Array have to be copied?
    if ( nUnused == 0 )
    {
        // increase (rounded up ) to the next Grow-border
        sal_uInt16 nNewSize = nUsed+nGrow;
        void** pNewData = new void*[nNewSize];

        if ( pData )
        {
            DBG_ASSERT( nUsed < nNewSize, "" );
            memcpy( pNewData, pData, sizeof(void*)*nUsed );
            delete [] pData;
        }
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize-nUsed);
        pData = pNewData;
    }

    // Now move the rear part
    if ( nPos < nUsed )
        memmove( pData+nPos+1, pData+nPos, (nUsed-nPos)*sizeof(void*) );

    // Now write into the free space.
    memmove( pData+nPos, &rElem, sizeof(void*) );
    nUsed += 1;
    nUnused -= 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
