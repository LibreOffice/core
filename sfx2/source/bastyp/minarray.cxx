/*************************************************************************
 *
 *  $RCSfile: minarray.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:28 $
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

#pragma hdrstop

#include "minarray.hxx"

// -----------------------------------------------------------------------

SfxPtrArr::SfxPtrArr( BYTE nInitSize, BYTE nGrowSize ):
    nUsed( 0 ),
    nGrow( nGrowSize ? nGrowSize : 1 ),
    nUnused( nInitSize )
{
    DBG_MEMTEST();
    USHORT nMSCBug = nInitSize;

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
    __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
}

// -----------------------------------------------------------------------

SfxPtrArr& SfxPtrArr::operator=( const SfxPtrArr& rOrig )
{
    DBG_MEMTEST();

    __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;

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
    DBG_ASSERT( (nUsed+1) < ( USHRT_MAX / sizeof(void*) ), "array too large" );
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        USHORT nNewSize = (nUsed == 1) ? (nGrow==1 ? 2 : nGrow) : nUsed+nGrow;
        void** pNewData = new void*[nNewSize];
        if ( pData )
        {
            DBG_ASSERT( nUsed <= nNewSize, "" );
            memmove( pNewData, pData, sizeof(void*)*nUsed );
            __DELETE (DEL_ARRAY (nUsed)) pData;
        }
        nUnused = nNewSize-nUsed;
        pData = pNewData;
    }

    // jetzt hinten in den freien Raum schreiben
    pData[nUsed] = aElem;
    ++nUsed;
    --nUnused;
}

// -----------------------------------------------------------------------

USHORT SfxPtrArr::Remove( USHORT nPos, USHORT nLen )
{
    DBG_MEMTEST();
    // nLen adjustieren, damit nicht ueber das Ende hinaus geloescht wird
    nLen = Min( (USHORT)(nUsed-nPos), nLen );

    // einfache Aufgaben erfordern einfache Loesungen!
    if ( nLen == 0 )
        return 0;

    // bleibt vielleicht keiner uebrig
    if ( (nUsed-nLen) == 0 )
    {
        __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
        pData = 0;
        nUsed = 0;
        nUnused = 0;
        return nLen;
    }

    // feststellen, ob das Array dadurch physikalisch schrumpft...
    if ( (nUnused+nLen) >= nGrow )
    {
        // auf die naechste Grow-Grenze aufgerundet verkleinern
        USHORT nNewUsed = nUsed-nLen;
        USHORT nNewSize = (nNewUsed+nGrow-1)/nGrow; nNewSize *= nGrow;
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
        __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
        pData = pNewData;
        nUsed = nNewUsed;
        nUnused = nNewSize - nNewUsed;
        return nLen;
    }

    // in allen anderen Faellen nur zusammenschieben
    if ( nUsed-nPos-nLen > 0 )
        memmove( pData+nPos, pData+nPos+nLen, (nUsed-nPos-nLen)*sizeof(void*) );
    nUsed -= nLen;
    nUnused += nLen;
    return nLen;
}

// -----------------------------------------------------------------------

BOOL SfxPtrArr::Remove( void* aElem )
{
    DBG_MEMTEST();
    // einfache Aufgaben ...
    if ( nUsed == 0 )
        return FALSE;

    // rueckwaerts, da meist der letzte zuerst wieder entfernt wird
    void* *pIter = pData + nUsed - 1;
    for ( USHORT n = 0; n < nUsed; ++n, --pIter )
        if ( *pIter == aElem )
        {
            Remove(nUsed-n-1, 1);
            return TRUE;
        }
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL SfxPtrArr::Replace( void* aOldElem, void* aNewElem )
{
    DBG_MEMTEST();
    // einfache Aufgaben ...
    if ( nUsed == 0 )
        return FALSE;

    // rueckwaerts, da meist der letzte zuerst wieder entfernt wird
    void* *pIter = pData + nUsed - 1;
    for ( USHORT n = 0; n < nUsed; ++n, --pIter )
        if ( *pIter == aOldElem )
        {
            pData[nUsed-n-1] = aNewElem;
            return TRUE;
        }
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL SfxPtrArr::Contains( const void* rItem ) const
{
    DBG_MEMTEST();
    if ( !nUsed )
        return FALSE;

    for ( USHORT n = 0; n < nUsed; ++n )
    {
        void* p = GetObject(n);
        if ( p == rItem )
            return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void SfxPtrArr::Insert( USHORT nPos, void* rElem )
{
    DBG_MEMTEST();
    DBG_ASSERT( (nUsed+1) < ( USHRT_MAX / sizeof(void*) ), "array too large" );
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        // auf die naechste Grow-Grenze aufgerundet vergroeszern
        USHORT nNewSize = nUsed+nGrow;
        void** pNewData = new void*[nNewSize];

        if ( pData )
        {
            DBG_ASSERT( nUsed < nNewSize, "" );
            memmove( pNewData, pData, sizeof(void*)*nUsed );
            __DELETE (DEL_ARRAY (nUsed)) pData;
        }
        nUnused = nNewSize-nUsed;
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

ByteArr::ByteArr( BYTE nInitSize, BYTE nGrowSize ):
    nUsed( 0 ),
    nGrow( nGrowSize ? nGrowSize : 1 ),
    nUnused( nInitSize )
{
    DBG_MEMTEST();
    USHORT nMSCBug = nInitSize;

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
    __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
}

// -----------------------------------------------------------------------

ByteArr& ByteArr::operator=( const ByteArr& rOrig )
{
    DBG_MEMTEST();

    __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;

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
        USHORT nNewSize = (nUsed == 1) ? (nGrow==1 ? 2 : nGrow) : nUsed+nGrow;
        char* pNewData = new char[nNewSize];
        if ( pData )
        {
            DBG_ASSERT( nUsed <= nNewSize, "" );
            memmove( pNewData, pData, sizeof(char)*nUsed );
            __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
        }
        nUnused = nNewSize-nUsed;
        pData = pNewData;
    }

    // jetzt hinten in den freien Raum schreiben
    pData[nUsed] = aElem;
    ++nUsed;
    --nUnused;
}

// -----------------------------------------------------------------------

USHORT ByteArr::Remove( USHORT nPos, USHORT nLen )
{
    DBG_MEMTEST();
    // nLen adjustieren, damit nicht ueber das Ende hinaus geloescht wird
    nLen = Min( (USHORT)(nUsed-nPos), nLen );

    // einfache Aufgaben erfordern einfache Loesungen!
    if ( nLen == 0 )
        return 0;

    // bleibt vielleicht keiner uebrig
    if ( (nUsed-nLen) == 0 )
    {
        __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
        pData = 0;
        nUsed = 0;
        nUnused = 0;
        return nLen;
    }

    // feststellen, ob das Array dadurch physikalisch schrumpft...
    if ( (nUnused+nLen) >= nGrow )
    {
        // auf die naechste Grow-Grenze aufgerundet verkleinern
        USHORT nNewUsed = nUsed-nLen;
        USHORT nNewSize = (nNewUsed+nGrow-1)/nGrow; nNewSize *= nGrow;
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
        __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
        pData = pNewData;
        nUsed = nNewUsed;
        nUnused = nNewSize - nNewUsed;
        return nLen;
    }

    // in allen anderen Faellen nur zusammenschieben
    if ( nUsed-nPos-nLen > 0 )
        memmove( pData+nPos, pData+nPos+nLen, (nUsed-nPos-nLen)*sizeof(char) );
    nUsed -= nLen;
    nUnused += nLen;
    return nLen;
}

// -----------------------------------------------------------------------

BOOL ByteArr::Remove( char aElem )
{
    DBG_MEMTEST();
    // einfache Aufgaben ...
    if ( nUsed == 0 )
        return FALSE;

    // rueckwaerts, da meist der letzte zuerst wieder entfernt wird
    char *pIter = pData + nUsed - 1;
    for ( USHORT n = 0; n < nUsed; ++n, --pIter )
        if ( *pIter == aElem )
        {
            Remove(nUsed-n-1, 1);
            return TRUE;
        }
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ByteArr::Contains( const char rItem ) const
{
    DBG_MEMTEST();
    if ( !nUsed )
        return FALSE;

    for ( USHORT n = 0; n < nUsed; ++n )
    {
        char p = GetObject(n);
        if ( p == rItem )
            return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void ByteArr::Insert( USHORT nPos, char rElem )
{
    DBG_MEMTEST();
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        // auf die naechste Grow-Grenze aufgerundet vergroeszern
        USHORT nNewSize = nUsed+nGrow;
        char* pNewData = new char[nNewSize];

        if ( pData )
        {
            DBG_ASSERT( nUsed < nNewSize, "" );
            memmove( pNewData, pData, sizeof(char)*nUsed );
            __DELETE (DEL_ARRAY (nUsed)) pData;
        }
        nUnused = nNewSize-nUsed;
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

char ByteArr::operator[]( USHORT nPos ) const
{
    DBG_MEMTEST();
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}

// -----------------------------------------------------------------------

char& ByteArr::operator [] (USHORT nPos)
{
    DBG_MEMTEST();
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}

// class WordArr ---------------------------------------------------------

WordArr::WordArr( BYTE nInitSize, BYTE nGrowSize ):
    nUsed( 0 ),
    nGrow( nGrowSize ? nGrowSize : 1 ),
    nUnused( nInitSize )
{
    DBG_MEMTEST();
    USHORT nMSCBug = nInitSize;

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
    __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
}

// -----------------------------------------------------------------------

WordArr& WordArr::operator=( const WordArr& rOrig )
{
    DBG_MEMTEST();

    __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;

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
        USHORT nNewSize = (nUsed == 1) ? (nGrow==1 ? 2 : nGrow) : nUsed+nGrow;
        short* pNewData = new short[nNewSize];
        if ( pData )
        {
            DBG_ASSERT( nUsed <= nNewSize, " " );
            memmove( pNewData, pData, sizeof(short)*nUsed );
            __DELETE (DEL_ARRAY (nUsed)) pData;
        }
        nUnused = nNewSize-nUsed;
        pData = pNewData;
    }

    // jetzt hinten in den freien Raum schreiben
    pData[nUsed] = aElem;
    ++nUsed;
    --nUnused;
}

// -----------------------------------------------------------------------

USHORT WordArr::Remove( USHORT nPos, USHORT nLen )
{
    DBG_MEMTEST();
    // nLen adjustieren, damit nicht ueber das Ende hinaus geloescht wird
    nLen = Min( (USHORT)(nUsed-nPos), nLen );

    // einfache Aufgaben erfordern einfache Loesungen!
    if ( nLen == 0 )
        return 0;

    // bleibt vielleicht keiner uebrig
    if ( (nUsed-nLen) == 0 )
    {
        __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
        pData = 0;
        nUsed = 0;
        nUnused = 0;
        return nLen;
    }

    // feststellen, ob das Array dadurch physikalisch schrumpft...
    if ( (nUnused+nLen) >= nGrow )
    {
        // auf die naechste Grow-Grenze aufgerundet verkleinern
        USHORT nNewUsed = nUsed-nLen;
        USHORT nNewSize = (nNewUsed+nGrow-1)/nGrow; nNewSize *= nGrow;
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
            __DELETE (DEL_ARRAY (nUsed+nUnused)) pData;
        pData = pNewData;
        nUsed = nNewUsed;
        nUnused = nNewSize - nNewUsed;
        return nLen;
    }

    // in allen anderen Faellen nur zusammenschieben
    if ( nUsed-nPos-nLen > 0 )
        memmove( pData+nPos, pData+nPos+nLen, (nUsed-nPos-nLen)*sizeof(short) );
    nUsed -= nLen;
    nUnused += nLen;
    return nLen;
}

// -----------------------------------------------------------------------

BOOL WordArr::Remove( short aElem )
{
    DBG_MEMTEST();
    // einfache Aufgaben ...
    if ( nUsed == 0 )
        return FALSE;

    // rueckwaerts, da meist der letzte zuerst wieder entfernt wird
    short *pIter = pData + nUsed - 1;
    for ( USHORT n = 0; n < nUsed; ++n, --pIter )
        if ( *pIter == aElem )
        {
            Remove(nUsed-n-1, 1);
            return TRUE;
        }
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL WordArr::Contains( const short rItem ) const
{
    DBG_MEMTEST();
    if ( !nUsed )
        return FALSE;

    for ( USHORT n = 0; n < nUsed; ++n )
    {
        short p = GetObject(n);
        if ( p == rItem )
            return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void WordArr::Insert( USHORT nPos, short rElem )
{
    DBG_MEMTEST();
    // musz das Array umkopiert werden?
    if ( nUnused == 0 )
    {
        // auf die naechste Grow-Grenze aufgerundet vergroeszern
        USHORT nNewSize = nUsed+nGrow;
        short* pNewData = new short[nNewSize];

        if ( pData )
        {
            DBG_ASSERT( nUsed < nNewSize, "" );
            memmove( pNewData, pData, sizeof(short)*nUsed );
            __DELETE (DEL_ARRAY (nUsed)) pData;
        }
        nUnused = nNewSize-nUsed;
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

short WordArr::operator[]( USHORT nPos ) const
{
    DBG_MEMTEST();
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}

// -----------------------------------------------------------------------

short& WordArr::operator [] (USHORT nPos)
{
    DBG_MEMTEST();
    DBG_ASSERT( nPos < nUsed, "" );
    return *(pData+nPos);
}


