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
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif

#include <sfx2/minarray.hxx>

// -----------------------------------------------------------------------

SfxPtrArr::SfxPtrArr( sal_uInt8 nInitSize, sal_uInt8 nGrowSize ):
    nUsed( 0 ),
    nGrow( nGrowSize ? nGrowSize : 1 ),
    nUnused( nInitSize )
{
    DBG_MEMTEST();
    sal_uInt16 nMSCBug = nInitSize;

    if ( nMSCBug > 0 )
        pData = new void*[nMSCBug];
    else
        pData = 0;
}

// -----------------------------------------------------------------------

SfxPtrArr::SfxPtrArr( const SfxPtrArr& rOrig )
{
    DBG_MEMTEST();
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
    DBG_MEMTEST();
    delete [] pData;
}

// -----------------------------------------------------------------------

SfxPtrArr& SfxPtrArr::operator=( const SfxPtrArr& rOrig )
{
    DBG_MEMTEST();

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
    DBG_MEMTEST();
    DBG_ASSERT( sal::static_int_cast< unsigned >(nUsed+1) < ( USHRT_MAX / sizeof(void*) ), "array too large" );
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        sal_uInt16 nNewSize = (nUsed == 1) ? (nGrow==1 ? 2 : nGrow) : nUsed+nGrow;
        void** pNewData = new void*[nNewSize];
        if ( pData )
        {
            DBG_ASSERT( nUsed <= nNewSize, "" );
            memmove( pNewData, pData, sizeof(void*)*nUsed );
            delete [] pData;
        }
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize-nUsed);
        pData = pNewData;
    }

    // jetzt hinten in den freien Raum schreiben
    pData[nUsed] = aElem;
    ++nUsed;
    --nUnused;
}

// -----------------------------------------------------------------------

sal_uInt16 SfxPtrArr::Remove( sal_uInt16 nPos, sal_uInt16 nLen )
{
    DBG_MEMTEST();
    // nLen adjustieren, damit nicht ueber das Ende hinaus geloescht wird
    nLen = Min( (sal_uInt16)(nUsed-nPos), nLen );

    // einfache Aufgaben erfordern einfache Loesungen!
    if ( nLen == 0 )
        return 0;

    // bleibt vielleicht keiner uebrig
    if ( (nUsed-nLen) == 0 )
    {
        delete [] pData;
        pData = 0;
        nUsed = 0;
        nUnused = 0;
        return nLen;
    }

    // feststellen, ob das Array dadurch physikalisch schrumpft...
    if ( (nUnused+nLen) >= nGrow )
    {
        // auf die naechste Grow-Grenze aufgerundet verkleinern
        sal_uInt16 nNewUsed = nUsed-nLen;
        sal_uInt16 nNewSize = ((nNewUsed+nGrow-1)/nGrow) * nGrow;
        DBG_ASSERT( nNewUsed <= nNewSize && nNewUsed+nGrow > nNewSize,
                    "shrink size computation failed" );
        void** pNewData = new void*[nNewSize];
        if ( nPos > 0 )
        {
            DBG_ASSERT( nPos <= nNewSize, "" );
            memmove( pNewData, pData, sizeof(void*)*nPos );
        }
        if ( nNewUsed != nPos )
            memmove( pNewData+nPos, pData+nPos+nLen,
                     sizeof(void*)*(nNewUsed-nPos) );
        delete [] pData;
        pData = pNewData;
        nUsed = nNewUsed;
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize - nNewUsed);
        return nLen;
    }

    // in allen anderen Faellen nur zusammenschieben
    if ( nUsed-nPos-nLen > 0 )
        memmove( pData+nPos, pData+nPos+nLen, (nUsed-nPos-nLen)*sizeof(void*) );
    nUsed = nUsed - nLen;
    nUnused = sal::static_int_cast< sal_uInt8 >(nUnused + nLen);
    return nLen;
}

// -----------------------------------------------------------------------

sal_Bool SfxPtrArr::Remove( void* aElem )
{
    DBG_MEMTEST();
    // einfache Aufgaben ...
    if ( nUsed == 0 )
        return sal_False;

    // rueckwaerts, da meist der letzte zuerst wieder entfernt wird
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

sal_Bool SfxPtrArr::Replace( void* aOldElem, void* aNewElem )
{
    DBG_MEMTEST();
    // einfache Aufgaben ...
    if ( nUsed == 0 )
        return sal_False;

    // rueckwaerts, da meist der letzte zuerst wieder entfernt wird
    void* *pIter = pData + nUsed - 1;
    for ( sal_uInt16 n = 0; n < nUsed; ++n, --pIter )
        if ( *pIter == aOldElem )
        {
            pData[nUsed-n-1] = aNewElem;
            return sal_True;
        }
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool SfxPtrArr::Contains( const void* rItem ) const
{
    DBG_MEMTEST();
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
    DBG_MEMTEST();
    DBG_ASSERT( sal::static_int_cast< unsigned >(nUsed+1) < ( USHRT_MAX / sizeof(void*) ), "array too large" );
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        // auf die naechste Grow-Grenze aufgerundet vergroeszern
        sal_uInt16 nNewSize = nUsed+nGrow;
        void** pNewData = new void*[nNewSize];

        if ( pData )
        {
            DBG_ASSERT( nUsed < nNewSize, "" );
            memmove( pNewData, pData, sizeof(void*)*nUsed );
            delete [] pData;
        }
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize-nUsed);
        pData = pNewData;
    }

    // jetzt den hinteren Teil verschieben
    if ( nPos < nUsed )
        memmove( pData+nPos+1, pData+nPos, (nUsed-nPos)*sizeof(void*) );

    // jetzt in den freien Raum schreiben
    memmove( pData+nPos, &rElem, sizeof(void*) );
    nUsed += 1;
    nUnused -= 1;
}

// class ByteArr ---------------------------------------------------------

ByteArr::ByteArr( sal_uInt8 nInitSize, sal_uInt8 nGrowSize ):
    nUsed( 0 ),
    nGrow( nGrowSize ? nGrowSize : 1 ),
    nUnused( nInitSize )
{
    DBG_MEMTEST();
    sal_uInt16 nMSCBug = nInitSize;

    if ( nInitSize > 0 )
        pData = new char[nMSCBug];
    else
        pData = 0;
}

// -----------------------------------------------------------------------

ByteArr::ByteArr( const ByteArr& rOrig )
{
    DBG_MEMTEST();
    nUsed = rOrig.nUsed;
    nGrow = rOrig.nGrow;
    nUnused = rOrig.nUnused;

    if ( rOrig.pData != 0 )
    {
        pData = new char[nUsed+nUnused];
        memcpy( pData, rOrig.pData, nUsed*sizeof(char) );
    }
    else
        pData = 0;
}

// -----------------------------------------------------------------------

ByteArr::~ByteArr()
{
    DBG_MEMTEST();
    delete [] pData;
}

// -----------------------------------------------------------------------

ByteArr& ByteArr::operator=( const ByteArr& rOrig )
{
    DBG_MEMTEST();

    delete [] pData;

    nUsed = rOrig.nUsed;
    nGrow = rOrig.nGrow;
    nUnused = rOrig.nUnused;

    if ( rOrig.pData != 0 )
    {
        pData = new char[nUsed+nUnused];
        memcpy( pData, rOrig.pData, nUsed*sizeof(char) );
    }
    else
        pData = 0;
    return *this;
}

// -----------------------------------------------------------------------

void ByteArr::Append( char aElem )
{
    DBG_MEMTEST();
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        sal_uInt16 nNewSize = (nUsed == 1) ? (nGrow==1 ? 2 : nGrow) : nUsed+nGrow;
        char* pNewData = new char[nNewSize];
        if ( pData )
        {
            DBG_ASSERT( nUsed <= nNewSize, "" );
            memmove( pNewData, pData, sizeof(char)*nUsed );
            delete [] pData;
        }
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize-nUsed);
        pData = pNewData;
    }

    // jetzt hinten in den freien Raum schreiben
    pData[nUsed] = aElem;
    ++nUsed;
    --nUnused;
}

// -----------------------------------------------------------------------

sal_uInt16 ByteArr::Remove( sal_uInt16 nPos, sal_uInt16 nLen )
{
    DBG_MEMTEST();
    // nLen adjustieren, damit nicht ueber das Ende hinaus geloescht wird
    nLen = Min( (sal_uInt16)(nUsed-nPos), nLen );

    // einfache Aufgaben erfordern einfache Loesungen!
    if ( nLen == 0 )
        return 0;

    // bleibt vielleicht keiner uebrig
    if ( (nUsed-nLen) == 0 )
    {
        delete [] pData;
        pData = 0;
        nUsed = 0;
        nUnused = 0;
        return nLen;
    }

    // feststellen, ob das Array dadurch physikalisch schrumpft...
    if ( (nUnused+nLen) >= nGrow )
    {
        // auf die naechste Grow-Grenze aufgerundet verkleinern
        sal_uInt16 nNewUsed = nUsed-nLen;
        sal_uInt16 nNewSize = ((nNewUsed+nGrow-1)/nGrow) * nGrow;
        DBG_ASSERT( nNewUsed <= nNewSize && nNewUsed+nGrow > nNewSize,
                    "shrink size computation failed" );
        char* pNewData = new char[nNewSize];
        if ( nPos > 0 )
        {
            DBG_ASSERT( nPos <= nNewSize, "" );
            memmove( pNewData, pData, sizeof(char)*nPos );
        }
        if ( nNewUsed != nPos )
            memmove( pNewData+nPos, pData+nPos+nLen,
                     sizeof(char)*(nNewUsed-nPos) );
        delete [] pData;
        pData = pNewData;
        nUsed = nNewUsed;
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize - nNewUsed);
        return nLen;
    }

    // in allen anderen Faellen nur zusammenschieben
    if ( nUsed-nPos-nLen > 0 )
        memmove( pData+nPos, pData+nPos+nLen, (nUsed-nPos-nLen)*sizeof(char) );
    nUsed = nUsed - nLen;
    nUnused = sal::static_int_cast< sal_uInt8 >(nUnused + nLen);
    return nLen;
}

// -----------------------------------------------------------------------

sal_Bool ByteArr::Remove( char aElem )
{
    DBG_MEMTEST();
    // einfache Aufgaben ...
    if ( nUsed == 0 )
        return sal_False;

    // rueckwaerts, da meist der letzte zuerst wieder entfernt wird
    char *pIter = pData + nUsed - 1;
    for ( sal_uInt16 n = 0; n < nUsed; ++n, --pIter )
        if ( *pIter == aElem )
        {
            Remove(nUsed-n-1, 1);
            return sal_True;
        }
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ByteArr::Contains( const char rItem ) const
{
    DBG_MEMTEST();
    if ( !nUsed )
        return sal_False;

    for ( sal_uInt16 n = 0; n < nUsed; ++n )
    {
        char p = GetObject(n);
        if ( p == rItem )
            return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void ByteArr::Insert( sal_uInt16 nPos, char rElem )
{
    DBG_MEMTEST();
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        // auf die naechste Grow-Grenze aufgerundet vergroeszern
        sal_uInt16 nNewSize = nUsed+nGrow;
        char* pNewData = new char[nNewSize];

        if ( pData )
        {
            DBG_ASSERT( nUsed < nNewSize, "" );
            memmove( pNewData, pData, sizeof(char)*nUsed );
            delete [] pData;
        }
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize-nUsed);
        pData = pNewData;
    }

    // jetzt den hinteren Teil verschieben
    if ( nPos < nUsed )
        memmove( pData+nPos+1, pData+nPos, (nUsed-nPos)*sizeof(char) );

    // jetzt in den freien Raum schreiben
    memmove( pData+nPos, &rElem, sizeof(char) );
    nUsed += 1;
    nUnused -= 1;
}

// -----------------------------------------------------------------------

char ByteArr::operator[]( sal_uInt16 nPos ) const
{
    DBG_MEMTEST();
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}

// -----------------------------------------------------------------------

char& ByteArr::operator [] (sal_uInt16 nPos)
{
    DBG_MEMTEST();
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}

// class WordArr ---------------------------------------------------------

WordArr::WordArr( sal_uInt8 nInitSize, sal_uInt8 nGrowSize ):
    nUsed( 0 ),
    nGrow( nGrowSize ? nGrowSize : 1 ),
    nUnused( nInitSize )
{
    DBG_MEMTEST();
    sal_uInt16 nMSCBug = nInitSize;

    if ( nInitSize > 0 )
        pData = new short[nMSCBug];
    else
        pData = 0;
}

// -----------------------------------------------------------------------

WordArr::WordArr( const WordArr& rOrig )
{
    DBG_MEMTEST();
    nUsed = rOrig.nUsed;
    nGrow = rOrig.nGrow;
    nUnused = rOrig.nUnused;

    if ( rOrig.pData != 0 )
    {
        pData = new short[nUsed+nUnused];
        memcpy( pData, rOrig.pData, nUsed*sizeof(short) );
    }
    else
        pData = 0;
}

// -----------------------------------------------------------------------

WordArr::~WordArr()
{
    DBG_MEMTEST();
    delete [] pData;
}

// -----------------------------------------------------------------------

WordArr& WordArr::operator=( const WordArr& rOrig )
{
    DBG_MEMTEST();

    delete [] pData;

    nUsed = rOrig.nUsed;
    nGrow = rOrig.nGrow;
    nUnused = rOrig.nUnused;

    if ( rOrig.pData != 0 )
    {
        pData = new short[nUsed+nUnused];
        memcpy( pData, rOrig.pData, nUsed*sizeof(short) );
    }
    else
        pData = 0;
    return *this;
}

// -----------------------------------------------------------------------

void WordArr::Append( short aElem )
{
    DBG_MEMTEST();
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        sal_uInt16 nNewSize = (nUsed == 1) ? (nGrow==1 ? 2 : nGrow) : nUsed+nGrow;
        short* pNewData = new short[nNewSize];
        if ( pData )
        {
            DBG_ASSERT( nUsed <= nNewSize, " " );
            memmove( pNewData, pData, sizeof(short)*nUsed );
            delete [] pData;
        }
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize-nUsed);
        pData = pNewData;
    }

    // jetzt hinten in den freien Raum schreiben
    pData[nUsed] = aElem;
    ++nUsed;
    --nUnused;
}

// -----------------------------------------------------------------------

sal_uInt16 WordArr::Remove( sal_uInt16 nPos, sal_uInt16 nLen )
{
    DBG_MEMTEST();
    // nLen adjustieren, damit nicht ueber das Ende hinaus geloescht wird
    nLen = Min( (sal_uInt16)(nUsed-nPos), nLen );

    // einfache Aufgaben erfordern einfache Loesungen!
    if ( nLen == 0 )
        return 0;

    // bleibt vielleicht keiner uebrig
    if ( (nUsed-nLen) == 0 )
    {
        delete [] pData;
        pData = 0;
        nUsed = 0;
        nUnused = 0;
        return nLen;
    }

    // feststellen, ob das Array dadurch physikalisch schrumpft...
    if ( (nUnused+nLen) >= nGrow )
    {
        // auf die naechste Grow-Grenze aufgerundet verkleinern
        sal_uInt16 nNewUsed = nUsed-nLen;
        sal_uInt16 nNewSize = ((nNewUsed+nGrow-1)/nGrow) * nGrow;
        DBG_ASSERT( nNewUsed <= nNewSize && nNewUsed+nGrow > nNewSize,
                    "shrink size computation failed" );
        short* pNewData = new short[nNewSize];
        if ( nPos > 0 )
        {
            DBG_ASSERT( nPos <= nNewSize, "" );
            memmove( pNewData, pData, sizeof(short)*nPos );
        }
        if ( nNewUsed != nPos )
            memmove( pNewData+nPos, pData+nPos+nLen,
                     sizeof(short)*(nNewUsed-nPos) );
            delete [] pData;
        pData = pNewData;
        nUsed = nNewUsed;
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize - nNewUsed);
        return nLen;
    }

    // in allen anderen Faellen nur zusammenschieben
    if ( nUsed-nPos-nLen > 0 )
        memmove( pData+nPos, pData+nPos+nLen, (nUsed-nPos-nLen)*sizeof(short) );
    nUsed = nUsed - nLen;
    nUnused = sal::static_int_cast< sal_uInt8 >(nUnused + nLen);
    return nLen;
}

// -----------------------------------------------------------------------

sal_Bool WordArr::Remove( short aElem )
{
    DBG_MEMTEST();
    // einfache Aufgaben ...
    if ( nUsed == 0 )
        return sal_False;

    // rueckwaerts, da meist der letzte zuerst wieder entfernt wird
    short *pIter = pData + nUsed - 1;
    for ( sal_uInt16 n = 0; n < nUsed; ++n, --pIter )
        if ( *pIter == aElem )
        {
            Remove(nUsed-n-1, 1);
            return sal_True;
        }
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool WordArr::Contains( const short rItem ) const
{
    DBG_MEMTEST();
    if ( !nUsed )
        return sal_False;

    for ( sal_uInt16 n = 0; n < nUsed; ++n )
    {
        short p = GetObject(n);
        if ( p == rItem )
            return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void WordArr::Insert( sal_uInt16 nPos, short rElem )
{
    DBG_MEMTEST();
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        // auf die naechste Grow-Grenze aufgerundet vergroeszern
        sal_uInt16 nNewSize = nUsed+nGrow;
        short* pNewData = new short[nNewSize];

        if ( pData )
        {
            DBG_ASSERT( nUsed < nNewSize, "" );
            memmove( pNewData, pData, sizeof(short)*nUsed );
            delete [] pData;
        }
        nUnused = sal::static_int_cast< sal_uInt8 >(nNewSize-nUsed);
        pData = pNewData;
    }

    // jetzt den hinteren Teil verschieben
    if ( nPos < nUsed )
        memmove( pData+nPos+1, pData+nPos, (nUsed-nPos)*sizeof(short) );

    // jetzt in den freien Raum schreiben
    memmove( pData+nPos, &rElem, sizeof(short) );
    nUsed += 1;
    nUnused -= 1;
}

// -----------------------------------------------------------------------

short WordArr::operator[]( sal_uInt16 nPos ) const
{
    DBG_MEMTEST();
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}

// -----------------------------------------------------------------------

short& WordArr::operator [] (sal_uInt16 nPos)
{
    DBG_MEMTEST();
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}


