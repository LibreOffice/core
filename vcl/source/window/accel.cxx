/*************************************************************************
 *
 *  $RCSfile: accel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:39 $
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

#define _SV_ACCEL_CXX

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _TABLE_HXX
#include <tools/table.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_ACCEL_H
#include <accel.h>
#endif
#ifndef _SV_ACCEL_HXX
#include <accel.hxx>
#endif
#ifndef _RC_H
#include <rc.h>
#endif

#pragma hdrstop

// =======================================================================

DECLARE_TABLE( ImplAccelTable, ImplAccelEntry* );
DECLARE_LIST( ImplAccelList, ImplAccelEntry* );

#define ACCELENTRY_NOTFOUND     ((USHORT)0xFFFF)

// =======================================================================

class ImplAccelData
{
public:
    ImplAccelTable  maKeyTable;     // Fuer KeyCodes, die mit einem Code erzeugt wurden
    ImplAccelList   maIdList;       // Id-List
};

// =======================================================================

DBG_NAME( Accelerator );

// =======================================================================

USHORT ImplAccelEntryGetIndex( ImplAccelList* pList, USHORT nId,
                               USHORT* pIndex = NULL )
{
    ULONG   nLow;
    ULONG   nHigh;
    ULONG   nMid;
    ULONG   nCount = pList->Count();
    USHORT  nCompareId;

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
            ImplAccelEntry* pTempEntry = pList->GetObject( nIndex );
            if ( !pTempEntry || (pTempEntry->mnId != pEntry->mnId) )
                break;
        }
        while ( nIndex < pList->Count() );

        pList->Insert( pEntry, (ULONG)nIndex );
    }
    else
        pList->Insert( pEntry, (ULONG)nInsIndex );
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

        mpData->maKeyTable.Insert( (ULONG)pEntry->maKeyCode.GetFullKeyCode(), pEntry );
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
        ImplGetKeyCode( rKeyCode.GetFunction(), nCode1, nCode2, nCode3 );
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
    USHORT nObjFollows = ReadShortRes();

    for( USHORT i = 0; i < nObjFollows; i++ )
    {
        InsertItem( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
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

    USHORT              nObjMask;
    USHORT              nAccelKeyId;
    USHORT              bDisable;
    KeyCode             aKeyCode;
    BOOL                bEnable     = FALSE;
    Accelerator*        pAutoAccel  = NULL;

    GetRes( rResId.SetRT( RSC_ACCELITEM ) );
    nObjMask        = ReadShortRes();
    nAccelKeyId     = ReadShortRes();
    bDisable        = ReadShortRes();

    if ( nObjMask & ACCELITEM_KEY )
    {
        // es wird ein neuer Kontext aufgespannt
        RSHEADER_TYPE * pKeyCodeRes = (RSHEADER_TYPE *)GetClassRes();
        ResId aResId( pKeyCodeRes );
        aKeyCode = KeyCode( aResId );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
    }

    if ( nObjMask & ACCELITEM_ACCEL )
    {
        pAutoAccel = new Accelerator( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
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
            ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (ULONG)nIndex );
            if ( pEntry && pEntry->mnId == nItemId )
            {
                mpData->maKeyTable.Remove( pEntry->maKeyCode.GetFullKeyCode() );
                mpData->maIdList.Remove( (ULONG)nIndex );

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
            if ( mpData->maIdList.GetObject( (ULONG)nIndex ) == pEntry )
                break;
            nIndex++;
        }
        while ( nIndex < nItemCount );

        mpData->maKeyTable.Remove( rKeyCode.GetFullKeyCode() );
        mpData->maIdList.Remove( (ULONG)nIndex );

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

USHORT Accelerator::GetItemCount() const
{
    DBG_CHKTHIS( Accelerator, NULL );

    return (USHORT)mpData->maIdList.Count();
}

// -----------------------------------------------------------------------

USHORT Accelerator::GetItemId( USHORT nPos ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (ULONG)nPos );
    if ( pEntry )
        return pEntry->mnId;
    else
        return 0;
}

// -----------------------------------------------------------------------

KeyCode Accelerator::GetItemKeyCode( USHORT nPos ) const
{
    DBG_CHKTHIS( Accelerator, NULL );

    ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (ULONG)nPos );
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
        return mpData->maIdList.GetObject( (ULONG)nIndex )->maKeyCode;
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
            ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (ULONG)nIndex );
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
        return mpData->maIdList.GetObject( (ULONG)nIndex )->mpAccel;
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
            ImplAccelEntry* pEntry = mpData->maIdList.GetObject( (ULONG)nIndex );
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
        return mpData->maIdList.GetObject( (ULONG)nIndex )->mbEnabled;
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
    mpData->maIdList.Clear();
    ImplCopyData( *((ImplAccelData*)(rAccel.mpData)) );

    return *this;
}
