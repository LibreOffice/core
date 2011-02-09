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
#include "precompiled_tools.hxx"

#define _TOOLS_TABLE_CXX

// -----------------------------------------------------------------------
#include <tools/debug.hxx>
#include <impcont.hxx>
#include <tools/table.hxx>

// =======================================================================

ULONG Table::ImplGetIndex( ULONG nKey, ULONG* pIndex ) const
{
    // Abpruefen, ob der erste Key groesser als der Vergleichskey ist
    if ( !nCount || (nKey < (ULONG)Container::ImpGetObject(0)) )
        return TABLE_ENTRY_NOTFOUND;

    ULONG   nLow;
    ULONG   nHigh;
    ULONG   nMid;
    ULONG   nCompareKey;
    void**  pNodes = Container::ImpGetOnlyNodes();

    // Binaeres Suchen
    nLow  = 0;
    nHigh = nCount-1;
    if ( pNodes )
    {
        do
        {
            nMid = (nLow + nHigh) / 2;
            nCompareKey = (ULONG)pNodes[nMid*2];
            if ( nKey < nCompareKey )
                nHigh = nMid-1;
            else
            {
                if ( nKey > nCompareKey )
                    nLow = nMid + 1;
                else
                    return nMid*2;
            }
        }
        while ( nLow <= nHigh );
    }
    else
    {
        do
        {
            nMid = (nLow + nHigh) / 2;
            nCompareKey = (ULONG)Container::ImpGetObject( nMid*2 );
            if ( nKey < nCompareKey )
                nHigh = nMid-1;
            else
            {
                if ( nKey > nCompareKey )
                    nLow = nMid + 1;
                else
                    return nMid*2;
            }
        }
        while ( nLow <= nHigh );
    }

    if ( pIndex )
    {
        if ( nKey > nCompareKey )
            *pIndex = (nMid+1)*2;
        else
            *pIndex = nMid*2;
    }

    return TABLE_ENTRY_NOTFOUND;
}

// =======================================================================

Table::Table( USHORT _nInitSize, USHORT _nReSize ) :
           Container( CONTAINER_MAXBLOCKSIZE, _nInitSize*2, _nReSize*2 )
{
    DBG_ASSERT( _nInitSize <= 32767, "Table::Table(): InitSize > 32767" );
    DBG_ASSERT( _nReSize <= 32767, "Table::Table(): ReSize > 32767" );
    nCount = 0;
}

// -----------------------------------------------------------------------

BOOL Table::Insert( ULONG nKey, void* p )
{
    // Tabellenelement einsortieren
    ULONG i;
    if ( nCount )
    {
        if ( nCount <= 24 )
        {
            USHORT n = 0;
            USHORT nTempCount = (USHORT)nCount * 2;

            if( void** pNodes = Container::ImpGetOnlyNodes() )
            {
                ULONG  nCompareKey = (ULONG)(*pNodes);
                while ( nKey > nCompareKey )
                {
                    n += 2;
                    pNodes += 2;
                    if ( n < nTempCount )
                        nCompareKey = (ULONG)(*pNodes);
                    else
                    {
                        nCompareKey = 0;
                        break;
                    }
                }

                // Testen, ob sich der Key schon in der Tabelle befindet
                if ( nKey == nCompareKey )
                    return FALSE;

                i = n;
            }
            else
            {
                i = 0;
                if ( ImplGetIndex( nKey, &i ) != TABLE_ENTRY_NOTFOUND )
                    return FALSE;
            }
        }
        else
        {
            i = 0;
            if ( ImplGetIndex( nKey, &i ) != TABLE_ENTRY_NOTFOUND )
                return FALSE;
        }
    }
    else
        i = 0;

    // Eintrag einfuegen (Key vor Pointer)
    Container::Insert( (void*)nKey, i );
    Container::Insert( p, i+1 );

    // Ein neuer Eintrag
    nCount++;

    return TRUE;
}

// -----------------------------------------------------------------------

void* Table::Remove( ULONG nKey )
{
    // Index besorgen
    ULONG nIndex = ImplGetIndex( nKey );

    // Testen, ob sich der Key in der Tabelle befindet
    if ( nIndex == TABLE_ENTRY_NOTFOUND )
        return NULL;

    // Itemanzahl erniedrigen
    nCount--;

    // Key entfernen
    Container::Remove( nIndex );

    // Pointer entfernen und zurueckgeben
    return Container::Remove( nIndex );
}

// -----------------------------------------------------------------------

void* Table::Replace( ULONG nKey, void* p )
{
    // Index abfragen
    ULONG nIndex = ImplGetIndex( nKey );

    // Existiert kein Eintrag mit dem Schluessel
    if ( nIndex == TABLE_ENTRY_NOTFOUND )
        return NULL;
    else
        return Container::Replace( p, nIndex+1 );
}

// -----------------------------------------------------------------------

void* Table::Get( ULONG nKey ) const
{
    // Index besorgen
    ULONG nIndex = ImplGetIndex( nKey );

    // Testen, ob sich der Key in der Tabelle befindet
    if ( nIndex == TABLE_ENTRY_NOTFOUND )
        return NULL;
    else
        return Container::ImpGetObject( nIndex+1 );
}

// -----------------------------------------------------------------------

void* Table::GetCurObject() const
{
    return Container::ImpGetObject( Container::GetCurPos()+1 );
}

// -----------------------------------------------------------------------

ULONG Table::GetKey( const void* p ) const
{
    ULONG nIndex = 0;

    // Solange noch Eintraege Vorhanden sind
    while ( nIndex < nCount )
    {
        // Stimmt der Pointer ueberein, wird der Key zurueckgegeben
        if ( p == Container::ImpGetObject( (nIndex*2)+1 ) )
            return (ULONG)Container::ImpGetObject( nIndex*2 );

        nIndex++;
    }

    return TABLE_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

BOOL Table::IsKeyValid( ULONG nKey ) const
{
    return (ImplGetIndex( nKey ) != TABLE_ENTRY_NOTFOUND) ? TRUE : FALSE;
}

// -----------------------------------------------------------------------

ULONG Table::GetUniqueKey( ULONG nStartKey ) const
{
    DBG_ASSERT( (nStartKey > 1) && (nStartKey < 0xFFFFFFFF),
                "Table::GetUniqueKey() - nStartKey == 0 or nStartKey >= 0xFFFFFFFF" );

    if ( !nCount )
        return nStartKey;

    ULONG nLastKey = (ULONG)Container::GetObject( (nCount*2)-2 );
    if ( nLastKey < nStartKey )
        return nStartKey;
    else
    {
        if ( nLastKey < 0xFFFFFFFE )
            return nLastKey+1;
        else
        {
            ULONG nPos;
            ULONG nTempPos = ImplGetIndex( nStartKey, &nPos );
            if ( nTempPos != TABLE_ENTRY_NOTFOUND )
                nPos = nTempPos;
            nLastKey = (ULONG)Container::GetObject( nPos );
            if ( nStartKey < nLastKey )
                return nStartKey;
            while ( nLastKey < 0xFFFFFFFE )
            {
                nPos += 2;
                nLastKey++;
                if ( nLastKey != (ULONG)Container::GetObject( nPos ) )
                    return nLastKey;
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

ULONG Table::SearchKey( ULONG nKey, ULONG* pPos ) const
{
    *pPos = 0;
    ULONG nPos = ImplGetIndex( nKey, pPos );
    if ( nPos != TABLE_ENTRY_NOTFOUND )
    {
        nPos /= 2;
        *pPos = nPos;
    }
    else
        *pPos /= 2;
    return nPos;
}

// -----------------------------------------------------------------------

void* Table::Seek( ULONG nKey )
{
    // Testen, ob ein Eintrag vorhanden ist
    if ( nCount )
    {
        ULONG nIndex = ImplGetIndex( nKey );

        // Ist Key nicht enthalten
        if ( nIndex == TABLE_ENTRY_NOTFOUND )
            return NULL;
        else
        {
            // Index setzen
            Container::Seek( nIndex );

            // Pointer zurueckgeben
            return Container::ImpGetObject( Container::GetCurPos() + 1 );
        }
    }
    else
        return NULL;
}

// -----------------------------------------------------------------------

void* Table::Seek( void* p )
{
    ULONG nKey = GetKey( p );

    // Ist Key vorhanden, dann als aktuellen Eintrag setzen
    if ( nKey != TABLE_ENTRY_NOTFOUND )
        return Seek( nKey );
    else
        return NULL;
}

// -----------------------------------------------------------------------

void* Table::First()
{
    // Testen, ob ein Eintrag vorhanden ist
    if ( nCount )
    {
        // Auf ersten Eintag setzen
        Container::First();

        // Pointer zurueckgeben
        return Container::ImpGetObject( 1 );
    }
    else
        return NULL;
}

// -----------------------------------------------------------------------

void* Table::Last()
{
    // Testen, ob ein Eintrag vorhanden ist
    if ( nCount )
    {
        // Last auf letzten Eintrag setzen
        void* p = Container::Last();
        Container::Prev();

        // Pointer zurueckgeben
        return p;
    }
    else
        return NULL;
}

// -----------------------------------------------------------------------

void* Table::Next()
{
    // Ueber den Pointer weiterschalten
    Container::Next();

    // Nachsten Eintag
    Container::Next();

    // Pointer vom naechsten Key zurueckgeben
    return Container::ImpGetObject( Container::GetCurPos() + 1 );
}

// -----------------------------------------------------------------------

void* Table::Prev()
{
    // Ueber den Pointer weiterschalten
    void* p = Container::Prev();

    // Nachsten Eintag
    Container::Prev();

    // Pointer vom vorherigen Key zurueckgeben
    return p;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
