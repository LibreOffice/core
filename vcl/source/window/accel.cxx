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
#include "precompiled_vcl.hxx"

#include <tools/table.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/accel.h>
#include <vcl/accel.hxx>
#include <tools/rc.h>
#include <vector>

// =======================================================================

DECLARE_TABLE( ImplAccelTable, ImplAccelEntry* )
typedef ::std::vector< ImplAccelEntry* > ImplAccelList;

#define ACCELENTRY_NOTFOUND     ((USHORT)0xFFFF)

// =======================================================================

class ImplAccelData
{
public:
    ImplAccelTable  maKeyTable;     // Fuer KeyCodes, die mit einem Code erzeugt wurden
    ImplAccelList   maIdList;       // Id-List
};

// =======================================================================

DBG_NAME( Accelerator )

// =======================================================================

USHORT ImplAccelEntryGetIndex( ImplAccelList* pList, USHORT nId,
                               USHORT* pIndex = NULL )
{
    size_t  nLow;
    size_t  nHigh;
    size_t  nMid;
    size_t  nCount = pList->size();
    USHORT  nCompareId;

    // Abpruefen, ob der erste Key groesser als der Vergleichskey ist
    if ( !nCount || (nId < (*pList)[ 0 ]->mnId) )
    {
        if ( pIndex )
            *pIndex = 0;
        return ACCELENTRY_NOTFOUND;
    }

    // Binaeres Suchen
    nLow  = 0;
    nHigh = nCount-1;
    do
    {
        nMid = (nLow + nHigh) / 2;
        nCompareId = (*pList)[ nMid ]->mnId;
        if ( nId < nCompareId )
            nHigh = nMid-1;
        else
        {
            if ( nId > nCompareId )
                nLow = nMid + 1;
            else
                return (USHORT)nMid;
        }
    }
    while ( nLow <= nHigh );

    if ( pIndex )
    {
        if ( nId > nCompareId )
            *pIndex = (USHORT)(nMid+1);
        else
            *pIndex = (USHORT)nMid;
    }

    return ACCELENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

static void ImplAccelEntryInsert( ImplAccelList* pList, ImplAccelEntry* pEntry )
{
    USHORT  nInsIndex;
    USHORT  nIndex = ImplAccelEntryGetIndex( pList, pEntry->mnId, &nInsIndex );

    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        do
        {
            nIndex++;
            ImplAccelEntry* pTempEntry = (*pList)[ nIndex ];
            if ( !pTempEntry || (pTempEntry->mnId != pEntry->mnId) )
                break;
        }
        while ( nIndex < pList->size() );

        if ( nIndex < pList->size() ) {
            ImplAccelList::iterator it = pList->begin();
            ::std::advance( it, nIndex );
            pList->insert( it, pEntry );
        } else {
            pList->push_back( pEntry );
        }
    }
    else {
        if ( nInsIndex < pList->size() ) {
            ImplAccelList::iterator it = pList->begin();
            ::std::advance( it, nInsIndex );
            pList->insert( it, pEntry );
        } else {
            pList->push_back( pEntry );
        }
    }
}

// -----------------------------------------------------------------------

static USHORT ImplAccelEntryGetFirstPos( ImplAccelList* pList, USHORT nId )
{
    USHORT nIndex = ImplAccelEntryGetIndex( pList, nId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        while ( nIndex )
        {
            nIndex--;
            if ( (*pList)[ nIndex ]->mnId != nId )
                break;
        }

        if ( (*pList)[ nIndex ]->mnId != nId )
            nIndex++;
    }

    return nIndex;
}

// =======================================================================

void Accelerator::ImplInit()
{
    mnCurId             = 0;
    mnCurRepeat         = 0;
    mbIsCancel          = FALSE;
    mpDel               = NULL;
}

// -----------------------------------------------------------------------

ImplAccelEntry* Accelerator::ImplGetAccelData( const KeyCode& rKeyCode ) const
{
    return mpData->maKeyTable.Get( rKeyCode.GetFullKeyCode() );
}

// -----------------------------------------------------------------------

void Accelerator::ImplCopyData( ImplAccelData& rAccelData )
{
    // Tabellen kopieren
    for ( size_t i = 0, n = rAccelData.maIdList.size(); i < n; ++i )
    {
        ImplAccelEntry* pEntry = new ImplAccelEntry( *rAccelData.maIdList[ i ] );

        // Folge-Accelerator, dann auch kopieren
        if ( pEntry->mpAccel )
        {
            pEntry->mpAccel = new Accelerator( *(pEntry->mpAccel) );
            pEntry->mpAutoAccel = pEntry->mpAccel;
        }
        else
            pEntry->mpAutoAccel = NULL;

        mpData->maKeyTable.Insert( (ULONG)pEntry->maKeyCode.GetFullKeyCode(), pEntry );
        mpData->maIdList.push_back( pEntry );
    }
}

// -----------------------------------------------------------------------

void Accelerator::ImplDeleteData()
{
    // Accelerator-Eintraege ueber die Id-Tabelle loeschen
    for ( size_t i = 0, n = mpData->maIdList.size(); i < n; ++i ) {
        ImplAccelEntry* pEntry = mpData->maIdList[ i ];
        if ( pEntry->mpAutoAccel ) {
            delete pEntry->mpAutoAccel;
        }
        delete pEntry;
    }
    mpData->maIdList.clear();
}

// -----------------------------------------------------------------------

void Accelerator::ImplInsertAccel( USHORT nItemId, const KeyCode& rKeyCode,
                                   BOOL bEnable, Accelerator* pAutoAccel )
{
    DBG_CHKTHIS( Accelerator, NULL );
    DBG_ASSERT( nItemId, "Accelerator::InsertItem(): ItemId == 0" );

    if ( rKeyCode.IsFunction() )
    {
        USHORT nCode1;
        USHORT nCode2;
        USHORT nCode3;
                USHORT nCode4;
        ImplGetKeyCode( rKeyCode.GetFunction(), nCode1, nCode2, nCode3, nCode4 );
        if ( nCode1 )
            ImplInsertAccel( nItemId, KeyCode( nCode1, nCode1 ), bEnable, pAutoAccel );
        if ( nCode2 )
        {
            if ( pAutoAccel )
                pAutoAccel = new Accelerator( *pAutoAccel );
            ImplInsertAccel( nItemId, KeyCode( nCode2, nCode2 ), bEnable, pAutoAccel );
            if ( nCode3 )
            {
                if ( pAutoAccel )
                    pAutoAccel = new Accelerator( *pAutoAccel );
                ImplInsertAccel( nItemId, KeyCode( nCode3, nCode3 ), bEnable, pAutoAccel );
            }
        }
        return;
    }

    // Neuen Eintrag holen und fuellen
    ImplAccelEntry* pEntry  = new ImplAccelEntry;
    pEntry->mnId            = nItemId;
    pEntry->maKeyCode       = rKeyCode;
    pEntry->mpAccel         = pAutoAccel;
    pEntry->mpAutoAccel     = pAutoAccel;
    pEntry->mbEnabled       = bEnable;

    // Ab in die Tabellen
    ULONG nCode = rKeyCode.GetFullKeyCode();
    if ( !nCode )
    {
        OSL_FAIL( "Accelerator::InsertItem(): KeyCode with KeyCode 0 not allowed" );
        delete pEntry;
    }
    else if ( !mpData->maKeyTable.Insert( nCode, pEntry ) )
    {
        OSL_TRACE( "Accelerator::InsertItem(): KeyCode (Key: %lx) already exists", nCode );
        delete pEntry;
    }
    else
        ImplAccelEntryInsert( &(mpData->maIdList), pEntry );
}

// -----------------------------------------------------------------------

Accelerator::Accelerator()
{
    DBG_CTOR( Accelerator, NULL );

    ImplInit();
    mpData = new ImplAccelData;
}

// -----------------------------------------------------------------------

Accelerator::Accelerator( const Accelerator& rAccel ) :
    Resource(),
    maHelpStr( rAccel.maHelpStr ),
    maCurKeyCode( rAccel.maCurKeyCode )
{
    DBG_CTOR( Accelerator, NULL );
    DBG_CHKOBJ( &rAccel, Accelerator, NULL );

    ImplInit();
    mpData = new ImplAccelData;
    ImplCopyData( *((ImplAccelData*)(rAccel.mpData)) );
}

// -----------------------------------------------------------------------

Accelerator::Accelerator( const ResId& rResId )
{
    DBG_CTOR( Accelerator, NULL );

    ImplInit();
    mpData = new ImplAccelData;
    rResId.SetRT( RSC_ACCEL );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

void Accelerator::ImplLoadRes( const ResId& rResId )
{
    GetRes( rResId );

    maHelpStr = ReadStringRes();
    ULONG nObjFollows = ReadLongRes();

    for( ULONG i = 0; i < nObjFollows; i++ )
    {
        InsertItem( ResId( (RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
    }
}

// -----------------------------------------------------------------------

Accelerator::~Accelerator()
{
    DBG_DTOR( Accelerator, NULL );

    // AccelManager benachrichtigen, das Accelrator geloescht wurde
    if ( mpDel )
        *mpDel = TRUE;

    ImplDeleteData();
    delete mpData;
}

// -----------------------------------------------------------------------

void Accelerator::Activate()
{
    maActivateHdl.Call( this );
}

// -----------------------------------------------------------------------

void Accelerator::Deactivate()
{
    maDeactivateHdl.Call( this );
}

// -----------------------------------------------------------------------

void Accelerator::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void Accelerator::InsertItem( USHORT nItemId, const KeyCode& rKeyCode )
{
    ImplInsertAccel( nItemId, rKeyCode, TRUE, NULL );
}

// -----------------------------------------------------------------------

void Accelerator::InsertItem( const ResId& rResId )
{
    DBG_CHKTHIS( Accelerator, NULL );

    ULONG               nObjMask;
    USHORT              nAccelKeyId;
    USHORT              bDisable;
    KeyCode             aKeyCode;
    Accelerator*        pAutoAccel  = NULL;

    GetRes( rResId.SetRT( RSC_ACCELITEM ) );
    nObjMask        = ReadLongRes();
    nAccelKeyId     = sal::static_int_cast<USHORT>(ReadLongRes());
    bDisable        = ReadShortRes();

    if ( nObjMask & ACCELITEM_KEY )
    {
        // es wird ein neuer Kontext aufgespannt
        RSHEADER_TYPE * pKeyCodeRes = (RSHEADER_TYPE *)GetClassRes();
        ResId aResId( pKeyCodeRes, *rResId.GetResMgr());
        aKeyCode = KeyCode( aResId );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
    }

    if ( nObjMask & ACCELITEM_ACCEL )
    {
        pAutoAccel = new Accelerator( ResId( (RSHEADER_TYPE *)GetClassRes(), *rResId.GetResMgr() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
    }

    ImplInsertAccel( nAccelKeyId, aKeyCode, !bDisable, pAutoAccel );
}

// -----------------------------------------------------------------------

void Accelerator::RemoveItem( USHORT nItemId )
{
    DBG_CHKTHIS( Accelerator, NULL );

    // Aus der Id-Liste entfernen
    USHORT nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        USHORT nItemCount = GetItemCount();
        do
        {
            ImplAccelEntry* pEntry = ( nIndex < mpData->maIdList.size() ) ? mpData->maIdList[ nIndex ] : NULL;
            if ( pEntry && pEntry->mnId == nItemId )
            {
                mpData->maKeyTable.Remove( pEntry->maKeyCode.GetFullKeyCode() );

                ImplAccelList::iterator it = mpData->maIdList.begin();
                ::std::advance( it, nIndex );
                mpData->maIdList.erase( it );

                // AutoResAccel zerstoeren
                if ( pEntry->mpAutoAccel )
                    delete pEntry->mpAutoAccel;

                delete pEntry;
            }
            else
                break;
        }
        while ( nIndex < nItemCount );
    }
}

// -----------------------------------------------------------------------

void Accelerator::RemoveItem( const KeyCode rKeyCode )
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
    {
        // Aus der Id-Liste entfernen
        USHORT nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), pEntry->mnId );
        USHORT nItemCount = GetItemCount();
        do
        {
            if ( mpData->maIdList[ nIndex ] == pEntry )
                break;
            nIndex++;
        }
        while ( nIndex < nItemCount );

        mpData->maKeyTable.Remove( rKeyCode.GetFullKeyCode() );

        ImplAccelList::iterator it = mpData->maIdList.begin();
        ::std::advance( it, nIndex );
        mpData->maIdList.erase( it );

        // AutoResAccel zerstoeren
        if ( pEntry->mpAutoAccel )
            delete pEntry->mpAutoAccel;

        delete pEntry;
    }
}

// -----------------------------------------------------------------------

void Accelerator::Clear()
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplDeleteData();
    mpData->maKeyTable.Clear();
}

// -----------------------------------------------------------------------

USHORT Accelerator::GetItemCount() const
{
    DBG_CHKTHIS( Accelerator, NULL );

    return (USHORT)mpData->maIdList.size();
}

// -----------------------------------------------------------------------

USHORT Accelerator::GetItemId( USHORT nPos ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ( nPos < mpData->maIdList.size() ) ? mpData->maIdList[ nPos ] : NULL;
    if ( pEntry )
        return pEntry->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

KeyCode Accelerator::GetItemKeyCode( USHORT nPos ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ( nPos < mpData->maIdList.size() ) ? mpData->maIdList[ nPos ] : NULL;
    if ( pEntry )
        return pEntry->maKeyCode;
    else
        return KeyCode();
}

// -----------------------------------------------------------------------

USHORT Accelerator::GetItemId( const KeyCode& rKeyCode ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
        return pEntry->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

KeyCode Accelerator::GetKeyCode( USHORT nItemId ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    USHORT nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
        return mpData->maIdList[ nIndex ]->maKeyCode;
    else
        return KeyCode();
}

// -----------------------------------------------------------------------

BOOL Accelerator::IsIdValid( USHORT nItemId ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    USHORT nIndex = ImplAccelEntryGetIndex( &(mpData->maIdList), nItemId );
    return (nIndex != ACCELENTRY_NOTFOUND);
}

// -----------------------------------------------------------------------

BOOL Accelerator::IsKeyCodeValid( const KeyCode rKeyCode ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    return (pEntry != NULL);
}

// -----------------------------------------------------------------------

BOOL Accelerator::Call( const KeyCode& rKeyCode, USHORT nRepeat )
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
    {
        if ( pEntry->mbEnabled )
        {
            BOOL bDel = FALSE;
            mnCurId         = pEntry->mnId;
            maCurKeyCode    = rKeyCode;
            mnCurRepeat     = nRepeat;
            mpDel           = &bDel;
            Select();
            if ( !bDel )
            {
                mnCurId         = 0;
                maCurKeyCode    = KeyCode();
                mnCurRepeat     = 0;
            }

            return TRUE;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Accelerator::SetAccel( USHORT nItemId, Accelerator* pAccel )
{
    DBG_CHKTHIS( Accelerator, NULL );

    USHORT nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        USHORT nItemCount = GetItemCount();
        do
        {
            ImplAccelEntry* pEntry = mpData->maIdList[ nIndex ];
            if ( pEntry->mnId != nItemId )
                break;

            pEntry->mpAccel = pAccel;
            nIndex++;
        }
        while ( nIndex < nItemCount );
    }
}

// -----------------------------------------------------------------------

Accelerator* Accelerator::GetAccel( USHORT nItemId ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    USHORT nIndex = ImplAccelEntryGetIndex( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
        return mpData->maIdList[ nIndex ]->mpAccel;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void Accelerator::SetAccel( const KeyCode rKeyCode, Accelerator* pAccel )
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
        pEntry->mpAccel = pAccel;
}

// -----------------------------------------------------------------------

Accelerator* Accelerator::GetAccel( const KeyCode rKeyCode ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
        return pEntry->mpAccel;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void Accelerator::EnableItem( USHORT nItemId, BOOL bEnable )
{
    DBG_CHKTHIS( Accelerator, NULL );

    USHORT nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        USHORT nItemCount = GetItemCount();
        do
        {
            ImplAccelEntry* pEntry = mpData->maIdList[ nIndex ];
            if ( pEntry->mnId != nItemId )
                break;

            pEntry->mbEnabled = bEnable;
            nIndex++;
        }
        while ( nIndex < nItemCount );
    }
}

// -----------------------------------------------------------------------

BOOL Accelerator::IsItemEnabled( USHORT nItemId ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    USHORT nIndex = ImplAccelEntryGetIndex( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
        return mpData->maIdList[ nIndex ]->mbEnabled;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void Accelerator::EnableItem( const KeyCode rKeyCode, BOOL bEnable )
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
        pEntry->mbEnabled = bEnable;
}

// -----------------------------------------------------------------------

BOOL Accelerator::IsItemEnabled( const KeyCode rKeyCode ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
        return pEntry->mbEnabled;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

Accelerator& Accelerator::operator=( const Accelerator& rAccel )
{
    DBG_CHKTHIS( Accelerator, NULL );
    DBG_CHKOBJ( &rAccel, Accelerator, NULL );

    // Neue Daten zuweisen
    maHelpStr       = rAccel.maHelpStr;
    maCurKeyCode    = KeyCode();
    mnCurId         = 0;
    mnCurRepeat     = 0;
    mbIsCancel      = FALSE;

    // Tabellen loeschen und kopieren
    ImplDeleteData();
    mpData->maKeyTable.Clear();
    ImplCopyData( *((ImplAccelData*)(rAccel.mpData)) );

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
