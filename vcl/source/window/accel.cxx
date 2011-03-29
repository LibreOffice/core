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
#include <tools/list.hxx>
#include <tools/table.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>

#include <vcl/svapp.hxx>
#include <accel.h>
#include <vcl/accel.hxx>



// =======================================================================

DECLARE_TABLE( ImplAccelTable, ImplAccelEntry* )
DECLARE_LIST( ImplAccelList, ImplAccelEntry* )

#define ACCELENTRY_NOTFOUND     ((sal_uInt16)0xFFFF)

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

sal_uInt16 ImplAccelEntryGetIndex( ImplAccelList* pList, sal_uInt16 nId,
                               sal_uInt16* pIndex = NULL )
{
    sal_uLong   nLow;
    sal_uLong   nHigh;
    sal_uLong   nMid;
    sal_uLong   nCount = pList->Count();
    sal_uInt16  nCompareId;

    // Abpruefen, ob der erste Key groesser als der Vergleichskey ist
    if ( !nCount || (nId < pList->GetObject( 0 )->mnId) )
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
        nCompareId = pList->GetObject( nMid )->mnId;
        if ( nId < nCompareId )
            nHigh = nMid-1;
        else
        {
            if ( nId > nCompareId )
                nLow = nMid + 1;
            else
                return (sal_uInt16)nMid;
        }
    }
    while ( nLow <= nHigh );

    if ( pIndex )
    {
        if ( nId > nCompareId )
            *pIndex = (sal_uInt16)(nMid+1);
        else
            *pIndex = (sal_uInt16)nMid;
    }

    return ACCELENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

static void ImplAccelEntryInsert( ImplAccelList* pList, ImplAccelEntry* pEntry )
{
    sal_uInt16  nInsIndex;
    sal_uInt16  nIndex = ImplAccelEntryGetIndex( pList, pEntry->mnId, &nInsIndex );

    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        do
        {
            nIndex++;
            ImplAccelEntry* pTempEntry = pList->GetObject( nIndex );
            if ( !pTempEntry || (pTempEntry->mnId != pEntry->mnId) )
                break;
        }
        while ( nIndex < pList->Count() );

        pList->Insert( pEntry, (sal_uLong)nIndex );
    }
    else
        pList->Insert( pEntry, (sal_uLong)nInsIndex );
}

// -----------------------------------------------------------------------

static sal_uInt16 ImplAccelEntryGetFirstPos( ImplAccelList* pList, sal_uInt16 nId )
{
    sal_uInt16 nIndex = ImplAccelEntryGetIndex( pList, nId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        while ( nIndex )
        {
            nIndex--;
            if ( pList->GetObject( nIndex )->mnId != nId )
                break;
        }

        if ( pList->GetObject( nIndex )->mnId != nId )
            nIndex++;
    }

    return nIndex;
}

// =======================================================================

void Accelerator::ImplInit()
{
    mnCurId             = 0;
    mnCurRepeat         = 0;
    mbIsCancel          = sal_False;
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
    ImplAccelEntry* pEntry = rAccelData.maIdList.First();
    while ( pEntry )
    {
        pEntry = new ImplAccelEntry( *pEntry );

        // Folge-Accelerator, dann auch kopieren
        if ( pEntry->mpAccel )
        {
            pEntry->mpAccel = new Accelerator( *(pEntry->mpAccel) );
            pEntry->mpAutoAccel = pEntry->mpAccel;
        }
        else
            pEntry->mpAutoAccel = NULL;

        mpData->maKeyTable.Insert( (sal_uLong)pEntry->maKeyCode.GetFullKeyCode(), pEntry );
        mpData->maIdList.Insert( pEntry, LIST_APPEND );

        pEntry = rAccelData.maIdList.Next();
    }
}

// -----------------------------------------------------------------------

void Accelerator::ImplDeleteData()
{
    // Accelerator-Eintraege ueber die Id-Tabelle loeschen
    ImplAccelEntry* pEntry = mpData->maIdList.First();
    while ( pEntry )
    {
        // AutoResAccel zerstoeren
        if ( pEntry->mpAutoAccel )
            delete pEntry->mpAutoAccel;
        delete pEntry;

        pEntry = mpData->maIdList.Next();
    }
}

// -----------------------------------------------------------------------

void Accelerator::ImplInsertAccel( sal_uInt16 nItemId, const KeyCode& rKeyCode,
                                   sal_Bool bEnable, Accelerator* pAutoAccel )
{
    DBG_CHKTHIS( Accelerator, NULL );
    DBG_ASSERT( nItemId, "Accelerator::InsertItem(): ItemId == 0" );

    if ( rKeyCode.IsFunction() )
    {
        sal_uInt16 nCode1;
        sal_uInt16 nCode2;
        sal_uInt16 nCode3;
                sal_uInt16 nCode4;
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
    sal_uLong nCode = rKeyCode.GetFullKeyCode();
    if ( !nCode )
    {
        DBG_ERROR( "Accelerator::InsertItem(): KeyCode with KeyCode 0 not allowed" );
        delete pEntry;
    }
    else if ( !mpData->maKeyTable.Insert( nCode, pEntry ) )
    {
        DBG_ERROR1( "Accelerator::InsertItem(): KeyCode (Key: %lx) already exists", nCode );
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
    sal_uLong nObjFollows = ReadLongRes();

    for( sal_uLong i = 0; i < nObjFollows; i++ )
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
        *mpDel = sal_True;

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

void Accelerator::InsertItem( sal_uInt16 nItemId, const KeyCode& rKeyCode )
{
    ImplInsertAccel( nItemId, rKeyCode, sal_True, NULL );
}

// -----------------------------------------------------------------------

void Accelerator::InsertItem( const ResId& rResId )
{
    DBG_CHKTHIS( Accelerator, NULL );

    sal_uLong               nObjMask;
    sal_uInt16              nAccelKeyId;
    sal_uInt16              bDisable;
    KeyCode             aKeyCode;
    Accelerator*        pAutoAccel  = NULL;

    GetRes( rResId.SetRT( RSC_ACCELITEM ) );
    nObjMask        = ReadLongRes();
    nAccelKeyId     = sal::static_int_cast<sal_uInt16>(ReadLongRes());
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

void Accelerator::RemoveItem( sal_uInt16 nItemId )
{
    DBG_CHKTHIS( Accelerator, NULL );

    // Aus der Id-Liste entfernen
    sal_uInt16 nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        sal_uInt16 nItemCount = GetItemCount();
        do
        {
            ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (sal_uLong)nIndex );
            if ( pEntry && pEntry->mnId == nItemId )
            {
                mpData->maKeyTable.Remove( pEntry->maKeyCode.GetFullKeyCode() );
                mpData->maIdList.Remove( (sal_uLong)nIndex );

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
        sal_uInt16 nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), pEntry->mnId );
        sal_uInt16 nItemCount = GetItemCount();
        do
        {
            if ( mpData->maIdList.GetObject( (sal_uLong)nIndex ) == pEntry )
                break;
            nIndex++;
        }
        while ( nIndex < nItemCount );

        mpData->maKeyTable.Remove( rKeyCode.GetFullKeyCode() );
        mpData->maIdList.Remove( (sal_uLong)nIndex );

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
    mpData->maIdList.Clear();
}

// -----------------------------------------------------------------------

sal_uInt16 Accelerator::GetItemCount() const
{
    DBG_CHKTHIS( Accelerator, NULL );

    return (sal_uInt16)mpData->maIdList.Count();
}

// -----------------------------------------------------------------------

sal_uInt16 Accelerator::GetItemId( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (sal_uLong)nPos );
    if ( pEntry )
        return pEntry->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

KeyCode Accelerator::GetItemKeyCode( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (sal_uLong)nPos );
    if ( pEntry )
        return pEntry->maKeyCode;
    else
        return KeyCode();
}

// -----------------------------------------------------------------------

sal_uInt16 Accelerator::GetItemId( const KeyCode& rKeyCode ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
        return pEntry->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

KeyCode Accelerator::GetKeyCode( sal_uInt16 nItemId ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    sal_uInt16 nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
        return mpData->maIdList.GetObject( (sal_uLong)nIndex )->maKeyCode;
    else
        return KeyCode();
}

// -----------------------------------------------------------------------

sal_Bool Accelerator::IsIdValid( sal_uInt16 nItemId ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    sal_uInt16 nIndex = ImplAccelEntryGetIndex( &(mpData->maIdList), nItemId );
    return (nIndex != ACCELENTRY_NOTFOUND);
}

// -----------------------------------------------------------------------

sal_Bool Accelerator::IsKeyCodeValid( const KeyCode rKeyCode ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    return (pEntry != NULL);
}

// -----------------------------------------------------------------------

sal_Bool Accelerator::Call( const KeyCode& rKeyCode, sal_uInt16 nRepeat )
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
    {
        if ( pEntry->mbEnabled )
        {
            sal_Bool bDel = sal_False;
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

            return sal_True;
        }
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void Accelerator::SetAccel( sal_uInt16 nItemId, Accelerator* pAccel )
{
    DBG_CHKTHIS( Accelerator, NULL );

    sal_uInt16 nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        sal_uInt16 nItemCount = GetItemCount();
        do
        {
            ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (sal_uLong)nIndex );
            if ( pEntry->mnId != nItemId )
                break;

            pEntry->mpAccel = pAccel;
            nIndex++;
        }
        while ( nIndex < nItemCount );
    }
}

// -----------------------------------------------------------------------

Accelerator* Accelerator::GetAccel( sal_uInt16 nItemId ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    sal_uInt16 nIndex = ImplAccelEntryGetIndex( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
        return mpData->maIdList.GetObject( (sal_uLong)nIndex )->mpAccel;
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
        return sal_False;
}

// -----------------------------------------------------------------------

void Accelerator::EnableItem( sal_uInt16 nItemId, sal_Bool bEnable )
{
    DBG_CHKTHIS( Accelerator, NULL );

    sal_uInt16 nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        sal_uInt16 nItemCount = GetItemCount();
        do
        {
            ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (sal_uLong)nIndex );
            if ( pEntry->mnId != nItemId )
                break;

            pEntry->mbEnabled = bEnable;
            nIndex++;
        }
        while ( nIndex < nItemCount );
    }
}

// -----------------------------------------------------------------------

sal_Bool Accelerator::IsItemEnabled( sal_uInt16 nItemId ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    sal_uInt16 nIndex = ImplAccelEntryGetIndex( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
        return mpData->maIdList.GetObject( (sal_uLong)nIndex )->mbEnabled;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

void Accelerator::EnableItem( const KeyCode rKeyCode, sal_Bool bEnable )
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
        pEntry->mbEnabled = bEnable;
}

// -----------------------------------------------------------------------

sal_Bool Accelerator::IsItemEnabled( const KeyCode rKeyCode ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = ImplGetAccelData( rKeyCode );
    if ( pEntry )
        return pEntry->mbEnabled;
    else
        return sal_False;
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
    mbIsCancel      = sal_False;

    // Tabellen loeschen und kopieren
    ImplDeleteData();
    mpData->maKeyTable.Clear();
    mpData->maIdList.Clear();
    ImplCopyData( *((ImplAccelData*)(rAccel.mpData)) );

    return *this;
}
