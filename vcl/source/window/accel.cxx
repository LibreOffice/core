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

#include <tools/debug.hxx>
#include <tools/rc.h>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <accel.h>
#include <vcl/accel.hxx>
#include <map>
#include <vector>

typedef ::std::map< sal_uLong, ImplAccelEntry* > ImplAccelMap;
typedef ::std::vector< ImplAccelEntry* > ImplAccelList;

#define ACCELENTRY_NOTFOUND     ((sal_uInt16)0xFFFF)

class ImplAccelData
{
public:
    ImplAccelMap  maKeyMap; // for keycodes, generated with a code
    ImplAccelList maIdList; // Id-List
};

sal_uInt16 ImplAccelEntryGetIndex( ImplAccelList* pList, sal_uInt16 nId,
                               sal_uInt16* pIndex = NULL )
{
    size_t  nLow;
    size_t  nHigh;
    size_t  nMid;
    size_t  nCount = pList->size();
    sal_uInt16  nCompareId;

    // check if first key is larger then the key to compare
    if ( !nCount || (nId < (*pList)[ 0 ]->mnId) )
    {
        if ( pIndex )
            *pIndex = 0;
        return ACCELENTRY_NOTFOUND;
    }

    // Binary search
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

static void ImplAccelEntryInsert( ImplAccelList* pList, ImplAccelEntry* pEntry )
{
    sal_uInt16  nInsIndex(0);
    sal_uInt16  nIndex = ImplAccelEntryGetIndex( pList, pEntry->mnId, &nInsIndex );

    if ( nIndex != ACCELENTRY_NOTFOUND )
    {
        do
        {
            nIndex++;
            ImplAccelEntry* pTempEntry = NULL;
            if ( nIndex < pList->size() )
                pTempEntry = (*pList)[ nIndex ];
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

static sal_uInt16 ImplAccelEntryGetFirstPos( ImplAccelList* pList, sal_uInt16 nId )
{
    sal_uInt16 nIndex = ImplAccelEntryGetIndex( pList, nId );
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

void Accelerator::ImplInit()
{
    mnCurId             = 0;
    mnCurRepeat         = 0;
    mbIsCancel          = false;
    mpDel               = NULL;
}

ImplAccelEntry* Accelerator::ImplGetAccelData( const vcl::KeyCode& rKeyCode ) const
{
    ImplAccelMap::iterator it = mpData->maKeyMap.find( rKeyCode.GetFullCode() );
    if( it != mpData->maKeyMap.end() )
        return it->second;
    else
        return NULL;
}

void Accelerator::ImplCopyData( ImplAccelData& rAccelData )
{
    // copy table
    for ( size_t i = 0, n = rAccelData.maIdList.size(); i < n; ++i )
    {
        ImplAccelEntry* pEntry = new ImplAccelEntry( *rAccelData.maIdList[ i ] );

        // sequence accelerator, then copy also
        if ( pEntry->mpAccel )
        {
            pEntry->mpAccel = new Accelerator( *(pEntry->mpAccel) );
            pEntry->mpAutoAccel = pEntry->mpAccel;
        }
        else
            pEntry->mpAutoAccel = NULL;

        mpData->maKeyMap.insert( std::make_pair( pEntry->maKeyCode.GetFullCode(), pEntry ) );
        mpData->maIdList.push_back( pEntry );
    }
}

void Accelerator::ImplDeleteData()
{
    // delete accelerator-entries using the id-table
    for ( size_t i = 0, n = mpData->maIdList.size(); i < n; ++i ) {
        ImplAccelEntry* pEntry = mpData->maIdList[ i ];
        if ( pEntry->mpAutoAccel ) {
            delete pEntry->mpAutoAccel;
        }
        delete pEntry;
    }
    mpData->maIdList.clear();
}

void Accelerator::ImplInsertAccel( sal_uInt16 nItemId, const vcl::KeyCode& rKeyCode,
                                   bool bEnable, Accelerator* pAutoAccel )
{
    DBG_ASSERT( nItemId, "Accelerator::InsertItem(): ItemId == 0" );

    if ( rKeyCode.IsFunction() )
    {
        sal_uInt16 nCode1;
        sal_uInt16 nCode2;
        sal_uInt16 nCode3;
                sal_uInt16 nCode4;
        ImplGetKeyCode( rKeyCode.GetFunction(), nCode1, nCode2, nCode3, nCode4 );
        if ( nCode1 )
            ImplInsertAccel( nItemId, vcl::KeyCode( nCode1, nCode1 ), bEnable, pAutoAccel );
        if ( nCode2 )
        {
            if ( pAutoAccel )
                pAutoAccel = new Accelerator( *pAutoAccel );
            ImplInsertAccel( nItemId, vcl::KeyCode( nCode2, nCode2 ), bEnable, pAutoAccel );
            if ( nCode3 )
            {
                if ( pAutoAccel )
                    pAutoAccel = new Accelerator( *pAutoAccel );
                ImplInsertAccel( nItemId, vcl::KeyCode( nCode3, nCode3 ), bEnable, pAutoAccel );
            }
        }
        return;
    }

    // fetch and fill new entries
    ImplAccelEntry* pEntry  = new ImplAccelEntry;
    pEntry->mnId            = nItemId;
    pEntry->maKeyCode       = rKeyCode;
    pEntry->mpAccel         = pAutoAccel;
    pEntry->mpAutoAccel     = pAutoAccel;
    pEntry->mbEnabled       = bEnable;

    // now into the tables
    sal_uLong nCode = rKeyCode.GetFullCode();
    if ( !nCode )
    {
        OSL_FAIL( "Accelerator::InsertItem(): KeyCode with KeyCode 0 not allowed" );
        delete pEntry;
    }
    else if ( !mpData->maKeyMap.insert( std::make_pair( nCode, pEntry ) ).second )
    {
        SAL_WARN( "vcl.layout", "Accelerator::InsertItem(): KeyCode (Key: " << nCode << ") already exists" );
        delete pEntry;
    }
    else
        ImplAccelEntryInsert( &(mpData->maIdList), pEntry );
}

Accelerator::Accelerator()
{

    ImplInit();
    mpData = new ImplAccelData;
}

Accelerator::Accelerator( const Accelerator& rAccel ) :
    Resource(),
    maHelpStr( rAccel.maHelpStr ),
    maCurKeyCode( rAccel.maCurKeyCode )
{

    ImplInit();
    mpData = new ImplAccelData;
    ImplCopyData(*rAccel.mpData);
}

Accelerator::Accelerator( const ResId& rResId )
{

    ImplInit();
    mpData = new ImplAccelData;
    rResId.SetRT( RSC_ACCEL );
    ImplLoadRes( rResId );
}

void Accelerator::ImplLoadRes( const ResId& rResId )
{
    GetRes( rResId );

    maHelpStr = ReadStringRes();
    sal_uLong nObjFollows = ReadLongRes();

    for( sal_uLong i = 0; i < nObjFollows; i++ )
    {
        InsertItem( ResId( static_cast<RSHEADER_TYPE *>(GetClassRes()), *rResId.GetResMgr() ) );
        IncrementRes( GetObjSizeRes( static_cast<RSHEADER_TYPE *>(GetClassRes()) ) );
    }
}

Accelerator::~Accelerator()
{

    // inform AccelManager about deleting the Accelerator
    if ( mpDel )
        *mpDel = true;

    ImplDeleteData();
    delete mpData;
}

void Accelerator::Activate()
{
    maActivateHdl.Call( *this );
}

void Accelerator::Select()
{
    maSelectHdl.Call( *this );
}

void Accelerator::InsertItem( sal_uInt16 nItemId, const vcl::KeyCode& rKeyCode )
{
    ImplInsertAccel( nItemId, rKeyCode, true, NULL );
}

void Accelerator::InsertItem( const ResId& rResId )
{

    sal_uLong               nObjMask;
    sal_uInt16              nAccelKeyId;
    sal_uInt16              bDisable;
    vcl::KeyCode            aKeyCode;
    Accelerator*        pAutoAccel  = NULL;

    GetRes( rResId.SetRT( RSC_ACCELITEM ) );
    nObjMask        = ReadLongRes();
    nAccelKeyId     = sal::static_int_cast<sal_uInt16>(ReadLongRes());
    bDisable        = ReadShortRes();

    if ( nObjMask & ACCELITEM_KEY )
    {
        // new context was created
        RSHEADER_TYPE * pKeyCodeRes = static_cast<RSHEADER_TYPE *>(GetClassRes());
        ResId aResId( pKeyCodeRes, *rResId.GetResMgr());
        aKeyCode = vcl::KeyCode( aResId );
        IncrementRes( GetObjSizeRes( static_cast<RSHEADER_TYPE *>(GetClassRes()) ) );
    }

    if ( nObjMask & ACCELITEM_ACCEL )
    {
        pAutoAccel = new Accelerator( ResId( static_cast<RSHEADER_TYPE *>(GetClassRes()), *rResId.GetResMgr() ) );
        IncrementRes( GetObjSizeRes( static_cast<RSHEADER_TYPE *>(GetClassRes()) ) );
    }

    ImplInsertAccel( nAccelKeyId, aKeyCode, !bDisable, pAutoAccel );
}

sal_uInt16 Accelerator::GetItemCount() const
{

    return (sal_uInt16)mpData->maIdList.size();
}

vcl::KeyCode Accelerator::GetKeyCode( sal_uInt16 nItemId ) const
{

    sal_uInt16 nIndex = ImplAccelEntryGetFirstPos( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
        return mpData->maIdList[ nIndex ]->maKeyCode;
    else
        return vcl::KeyCode();
}

sal_uInt16 Accelerator::GetItemId( sal_uInt16 nPos ) const
{

    ImplAccelEntry* pEntry = ( nPos < mpData->maIdList.size() ) ? mpData->maIdList[ nPos ] : NULL;
    if ( pEntry )
        return pEntry->mnId;
    else
        return 0;
}

Accelerator* Accelerator::GetAccel( sal_uInt16 nItemId ) const
{

    sal_uInt16 nIndex = ImplAccelEntryGetIndex( &(mpData->maIdList), nItemId );
    if ( nIndex != ACCELENTRY_NOTFOUND )
        return mpData->maIdList[ nIndex ]->mpAccel;
    else
        return NULL;
}

Accelerator& Accelerator::operator=( const Accelerator& rAccel )
{

    // assign new data
    maHelpStr       = rAccel.maHelpStr;
    maCurKeyCode    = vcl::KeyCode();
    mnCurId         = 0;
    mnCurRepeat     = 0;
    mbIsCancel      = false;

    // delete and copy tables
    ImplDeleteData();
    mpData->maKeyMap.clear();
    ImplCopyData(*rAccel.mpData);

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
