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

#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <vcl/outdev.hxx>
#include <vcl/MetaTextArrayAction.hxx>

#include <TypeSerializer.hxx>

MetaTextArrayAction::MetaTextArrayAction() :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{}

MetaTextArrayAction::MetaTextArrayAction( const MetaTextArrayAction& rAction ) :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    maStartPt   ( rAction.maStartPt ),
    maStr       ( rAction.maStr ),
    mnIndex     ( rAction.mnIndex ),
    mnLen       ( rAction.mnLen )
{
    if( rAction.mpDXAry )
    {
        mpDXAry.reset( new long[ mnLen ] );
        memcpy( mpDXAry.get(), rAction.mpDXAry.get(), mnLen * sizeof( long ) );
    }
}

MetaTextArrayAction::MetaTextArrayAction( const Point& rStartPt,
                                          const OUString& rStr,
                                          const long* pDXAry,
                                          sal_Int32 nIndex,
                                          sal_Int32 nLen ) :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    maStartPt   ( rStartPt ),
    maStr       ( rStr ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{
    const sal_Int32 nAryLen = pDXAry ? mnLen : 0;

    if (nAryLen > 0)
    {
        mpDXAry.reset( new long[ nAryLen ] );
        memcpy( mpDXAry.get(), pDXAry, nAryLen * sizeof(long) );
    }
}

MetaTextArrayAction::~MetaTextArrayAction()
{
}

void MetaTextArrayAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTextArray( maStartPt, maStr, mpDXAry.get(), mnIndex, mnLen );
}

rtl::Reference<MetaAction> MetaTextArrayAction::Clone()
{
    return new MetaTextArrayAction( *this );
}

void MetaTextArrayAction::Move( long nHorzMove, long nVertMove )
{
    maStartPt.Move( nHorzMove, nVertMove );
}

void MetaTextArrayAction::Scale( double fScaleX, double fScaleY )
{
    maStartPt.Scale( fScaleX, fScaleY );

    if ( mpDXAry && mnLen )
    {
        for ( sal_uInt16 i = 0, nCount = mnLen; i < nCount; i++ )
            mpDXAry[ i ] = FRound( mpDXAry[ i ] * fabs(fScaleX) );
    }
}

void MetaTextArrayAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    const sal_Int32 nAryLen = mpDXAry ? mnLen : 0;

    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);
    TypeSerializer aSerializer(rOStm);
    aSerializer.writePoint(maStartPt);
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt16(mnIndex);
    rOStm.WriteUInt16(mnLen);
    rOStm.WriteInt32(nAryLen);

    for (sal_Int32 i = 0; i < nAryLen; ++i)
        rOStm.WriteInt32( mpDXAry[ i ] );

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaTextArrayAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    mpDXAry.reset();

    VersionCompat aCompat(rIStm, StreamMode::READ);
    TypeSerializer aSerializer(rIStm);
    aSerializer.readPoint(maStartPt);
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt16 nTmpIndex(0);
    rIStm.ReadUInt16(nTmpIndex);
    mnIndex = nTmpIndex;
    sal_uInt16 nTmpLen(0);
    rIStm.ReadUInt16(nTmpLen);
    mnLen = nTmpLen;
    sal_Int32 nAryLen(0);
    rIStm.ReadInt32(nAryLen);

    if (mnLen > maStr.getLength() - mnIndex)
    {
        mnIndex = 0;
        mpDXAry = nullptr;
        return;
    }

    if( nAryLen )
    {
        // #i9762#, #106172# Ensure that DX array is at least mnLen entries long
        if ( mnLen >= nAryLen )
        {
            mpDXAry.reset( new (std::nothrow)long[ mnLen ] );
            if ( mpDXAry )
            {
                sal_Int32 i;
                sal_Int32 val;
                for( i = 0; i < nAryLen; i++ )
                {
                    rIStm.ReadInt32( val);
                    mpDXAry[ i ] = val;
                }
                // #106172# setup remainder
                for( ; i < mnLen; i++ )
                    mpDXAry[ i ] = 0;
            }
        }
        else
        {
            mpDXAry = nullptr;
            return;
        }
    }
    else
        mpDXAry = nullptr;

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
    {
        maStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(rIStm);

        if ( mnIndex + mnLen > maStr.getLength() )
        {
            mnIndex = 0;
            mpDXAry.reset();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
