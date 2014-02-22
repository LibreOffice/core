/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <vcl/animate.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <rtl/crc.h>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <impanmvw.hxx>
#include <vcl/dibtools.hxx>


#define MIN_TIMEOUT 2L
#define INC_TIMEOUT 0L

sal_uLong Animation::mnAnimCount = 0UL;

sal_uLong AnimationBitmap::GetChecksum() const
{
    sal_uInt32  nCrc = aBmpEx.GetChecksum();
    SVBT32      aBT32;

    UInt32ToSVBT32( aPosPix.X(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( aPosPix.Y(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( aSizePix.Width(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( aSizePix.Height(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( (long) nWait, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( (long) eDisposal, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( (long) bUserInput, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    return nCrc;
}

Animation::Animation() :
    mnLoopCount         ( 0 ),
    mnLoops             ( 0 ),
    mnPos               ( 0 ),
    meCycleMode         ( CYCLE_NORMAL ),
    mbIsInAnimation     ( false ),
    mbLoopTerminated    ( false ),
    mbIsWaiting         ( false )
{
    maTimer.SetTimeoutHdl( LINK( this, Animation, ImplTimeoutHdl ) );
}

Animation::Animation( const Animation& rAnimation ) :
    maBitmapEx          ( rAnimation.maBitmapEx ),
    maGlobalSize        ( rAnimation.maGlobalSize ),
    mnLoopCount         ( rAnimation.mnLoopCount ),
    mnPos               ( rAnimation.mnPos ),
    meCycleMode         ( rAnimation.meCycleMode ),
    mbIsInAnimation     ( false ),
    mbLoopTerminated    ( rAnimation.mbLoopTerminated ),
    mbIsWaiting         ( rAnimation.mbIsWaiting )
{

    for( size_t i = 0, nCount = rAnimation.maList.size(); i < nCount; i++ )
        maList.push_back( new AnimationBitmap( *rAnimation.maList[ i ] ) );

    maTimer.SetTimeoutHdl( LINK( this, Animation, ImplTimeoutHdl ) );
    mnLoops = mbLoopTerminated ? 0 : mnLoopCount;
}

Animation::~Animation()
{

    if( mbIsInAnimation )
        Stop();

    for( size_t i = 0, n = maList.size(); i < n; ++i )
        delete maList[ i ];

    for( size_t i = 0, n = maViewList.size(); i < n; ++i )
        delete maViewList[ i ];
}

Animation& Animation::operator=( const Animation& rAnimation )
{
    Clear();

    for( size_t i = 0, nCount = rAnimation.maList.size(); i < nCount; i++ )
        maList.push_back( new AnimationBitmap( *rAnimation.maList[ i ] ) );

    maGlobalSize = rAnimation.maGlobalSize;
    maBitmapEx = rAnimation.maBitmapEx;
    meCycleMode = rAnimation.meCycleMode;
    mnLoopCount = rAnimation.mnLoopCount;
    mnPos = rAnimation.mnPos;
    mbLoopTerminated = rAnimation.mbLoopTerminated;
    mbIsWaiting = rAnimation.mbIsWaiting;
    mnLoops = mbLoopTerminated ? 0 : mnLoopCount;

    return *this;
}

bool Animation::operator==( const Animation& rAnimation ) const
{
    const size_t nCount = maList.size();
    bool bRet = false;

    if(  rAnimation.maList.size() == nCount
      && rAnimation.maBitmapEx    == maBitmapEx
      && rAnimation.maGlobalSize  == maGlobalSize
      && rAnimation.meCycleMode   == meCycleMode
      )
    {
        bRet = true;

        for( size_t n = 0; n < nCount; n++ )
        {
            if( ( *maList[ n ] ) != ( *rAnimation.maList[ n ] ) )
            {
                bRet = false;
                break;
            }
        }
    }

    return bRet;
}

void Animation::Clear()
{
    maTimer.Stop();
    mbIsInAnimation = false;
    maGlobalSize = Size();
    maBitmapEx.SetEmpty();

    for( size_t i = 0, n = maList.size(); i < n; ++i )
        delete maList[ i ];
    maList.clear();

    for( size_t i = 0, n = maViewList.size(); i < n; ++i )
        delete maViewList[ i ];
    maViewList.clear();
}

bool Animation::IsTransparent() const
{
    Point       aPoint;
    Rectangle   aRect( aPoint, maGlobalSize );
    bool        bRet = false;

    
    
    
    
    for( size_t i = 0, nCount = maList.size(); i < nCount; i++ )
    {
        const AnimationBitmap* pAnimBmp = maList[ i ];

        if(  DISPOSE_BACK == pAnimBmp->eDisposal
          && Rectangle( pAnimBmp->aPosPix, pAnimBmp->aSizePix ) != aRect
          )
        {
            bRet = true;
            break;
        }
    }

    if( !bRet )
        bRet = maBitmapEx.IsTransparent();

    return bRet;
}

sal_uLong Animation::GetSizeBytes() const
{
    sal_uLong nSizeBytes = GetBitmapEx().GetSizeBytes();

    for( size_t i = 0, nCount = maList.size(); i < nCount; i++ )
    {
        const AnimationBitmap* pAnimBmp = maList[ i ];
        nSizeBytes += pAnimBmp->aBmpEx.GetSizeBytes();
    }

    return nSizeBytes;
}

sal_uLong Animation::GetChecksum() const
{
    SVBT32      aBT32;
    sal_uInt32  nCrc = GetBitmapEx().GetChecksum();

    UInt32ToSVBT32( maList.size(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( maGlobalSize.Width(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( maGlobalSize.Height(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    UInt32ToSVBT32( (long) meCycleMode, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    for( size_t i = 0, nCount = maList.size(); i < nCount; i++ )
    {
        UInt32ToSVBT32( maList[ i ]->GetChecksum(), aBT32 );
        nCrc = rtl_crc32( nCrc, aBT32, 4 );
    }

    return nCrc;
}

bool Animation::Start( OutputDevice* pOut, const Point& rDestPt, const Size& rDestSz, long nExtraData,
                       OutputDevice* pFirstFrameOutDev )
{
    bool bRet = false;

    if( !maList.empty() )
    {
        if(  ( pOut->GetOutDevType() == OUTDEV_WINDOW )
          && !mbLoopTerminated
          && ( ANIMATION_TIMEOUT_ON_CLICK != maList[ mnPos ]->nWait )
          )
        {
            ImplAnimView*   pView;
            ImplAnimView*   pMatch = NULL;

            for( size_t i = 0; i < maViewList.size(); ++i )
            {
                pView = maViewList[ i ];
                if( pView->ImplMatches( pOut, nExtraData ) )
                {
                    if( pView->ImplGetOutPos() == rDestPt &&
                        pView->ImplGetOutSizePix() == pOut->LogicToPixel( rDestSz ) )
                    {
                        pView->ImplRepaint();
                        pMatch = pView;
                    }
                    else
                    {
                        delete maViewList[ i ];
                        maViewList.erase( maViewList.begin() + i );
                        pView = NULL;
                    }

                    break;
                }
            }

            if( maViewList.empty() )
            {
                maTimer.Stop();
                mbIsInAnimation = false;
                mnPos = 0UL;
            }

            if( !pMatch )
                maViewList.push_back( new ImplAnimView( this, pOut, rDestPt, rDestSz, nExtraData, pFirstFrameOutDev ) );

            if( !mbIsInAnimation )
            {
                ImplRestartTimer( maList[ mnPos ]->nWait );
                mbIsInAnimation = true;
            }
        }
        else
            Draw( pOut, rDestPt, rDestSz );

        bRet = true;
    }

    return bRet;
}

void Animation::Stop( OutputDevice* pOut, long nExtraData )
{
    for( size_t i = 0; i < maViewList.size(); )
    {


        ImplAnimView* pView = maViewList[ i ];
        if( pView->ImplMatches( pOut, nExtraData ) )
        {
            delete pView;
            maViewList.erase( maViewList.begin() + i );
        }
        else
            i++;
    }

    if( maViewList.empty() )
    {
        maTimer.Stop();
        mbIsInAnimation = false;
    }
}

void Animation::Draw( OutputDevice* pOut, const Point& rDestPt ) const
{
    Draw( pOut, rDestPt, pOut->PixelToLogic( maGlobalSize ) );
}

void Animation::Draw( OutputDevice* pOut, const Point& rDestPt, const Size& rDestSz ) const
{
    const size_t nCount = maList.size();

    if( nCount )
    {
        AnimationBitmap* pObj = maList[ std::min( mnPos, nCount - 1 ) ];

        if(  pOut->GetConnectMetaFile()
          || ( pOut->GetOutDevType() == OUTDEV_PRINTER )
          )
            maList[ 0 ]->aBmpEx.Draw( pOut, rDestPt, rDestSz );
        else if( ANIMATION_TIMEOUT_ON_CLICK == pObj->nWait )
            pObj->aBmpEx.Draw( pOut, rDestPt, rDestSz );
        else
        {
            const size_t nOldPos = mnPos;
            ( (Animation*) this )->mnPos = mbLoopTerminated ? ( nCount - 1UL ) : mnPos;
            delete new ImplAnimView( (Animation*) this, pOut, rDestPt, rDestSz, 0 );
            ( (Animation*) this )->mnPos = nOldPos;
        }
    }
}

void Animation::ImplRestartTimer( sal_uLong nTimeout )
{
    maTimer.SetTimeout( std::max( nTimeout, (sal_uLong)(MIN_TIMEOUT + ( mnAnimCount - 1 ) * INC_TIMEOUT) ) * 10L );
    maTimer.Start();
}

typedef ::std::vector< AInfo* > AInfoList_impl;

IMPL_LINK_NOARG(Animation, ImplTimeoutHdl)
{
    const size_t nAnimCount = maList.size();
    AInfoList_impl aAInfoList;

    if( nAnimCount )
    {
        ImplAnimView*   pView;
        bool        bGlobalPause = true;

        if( maNotifyLink.IsSet() )
        {
            AInfo* pAInfo;

            
            for( size_t i = 0, n = maViewList.size(); i < n; ++i )
                aAInfoList.push_back( maViewList[ i ]->ImplCreateAInfo() );

            maNotifyLink.Call( this );

            
            for( size_t i = 0, n = aAInfoList.size(); i < n; ++i )
            {
                pAInfo = aAInfoList[ i ];
                if( !pAInfo->pViewData )
                {
                    pView = new ImplAnimView( this, pAInfo->pOutDev,
                                              pAInfo->aStartOrg, pAInfo->aStartSize, pAInfo->nExtraData );

                    maViewList.push_back( pView );
                }
                else
                    pView = (ImplAnimView*) pAInfo->pViewData;

                pView->ImplPause( pAInfo->bPause );
                pView->ImplSetMarked( true );
            }

            
            for( size_t i = 0, n = aAInfoList.size(); i < n; ++i )
                delete aAInfoList[ i ];
            aAInfoList.clear();

            
            for( size_t i = 0; i < maViewList.size(); )
            {
                pView = maViewList[ i ];
                if( !pView->ImplIsMarked() )
                {
                    delete pView;
                    maViewList.erase( maViewList.begin() + i );
                }
                else
                {
                    if( !pView->ImplIsPause() )
                        bGlobalPause = false;

                    pView->ImplSetMarked( false );
                    i++;
                }
            }
        }
        else
            bGlobalPause = false;

        if( maViewList.empty() )
            Stop();
        else if( bGlobalPause )
            ImplRestartTimer( 10 );
        else
        {
            AnimationBitmap* pStepBmp = (++mnPos < maList.size()) ? maList[ mnPos ] : NULL;

            if( !pStepBmp )
            {
                if( mnLoops == 1 )
                {
                    Stop();
                    mbLoopTerminated = true;
                    mnPos = nAnimCount - 1UL;
                    maBitmapEx = maList[ mnPos ]->aBmpEx;
                    return 0L;
                }
                else
                {
                    if( mnLoops )
                        mnLoops--;

                    mnPos = 0;
                    pStepBmp = maList[ mnPos ];
                }
            }

            
            
            
            
            for( size_t i = 0; i < maViewList.size(); )
            {
                pView = maViewList[ i ];
                pView->ImplDraw( mnPos );

                if( pView->ImplIsMarked() )
                {
                    delete pView;
                    maViewList.erase( maViewList.begin() + i );
                }
                else
                    i++;
            }

            
            if( maViewList.empty() )
                Stop();
            else
                ImplRestartTimer( pStepBmp->nWait );
        }
    }
    else
        Stop();

    return 0L;
}

bool Animation::Insert( const AnimationBitmap& rStepBmp )
{
    bool bRet = false;

    if( !IsInAnimation() )
    {
        Point       aPoint;
        Rectangle   aGlobalRect( aPoint, maGlobalSize );

        maGlobalSize = aGlobalRect.Union( Rectangle( rStepBmp.aPosPix, rStepBmp.aSizePix ) ).GetSize();
        maList.push_back( new AnimationBitmap( rStepBmp ) );

        
        if( maList.size() == 1 )
            maBitmapEx = rStepBmp.aBmpEx;

        bRet = true;
    }

    return bRet;
}

const AnimationBitmap& Animation::Get( sal_uInt16 nAnimation ) const
{
    DBG_ASSERT( ( nAnimation < maList.size() ), "No object at this position" );
    return *maList[ nAnimation ];
}

void Animation::Replace( const AnimationBitmap& rNewAnimationBitmap, sal_uInt16 nAnimation )
{
    DBG_ASSERT( ( nAnimation < maList.size() ), "No object at this position" );

    delete maList[ nAnimation ];
    maList[ nAnimation ] = new AnimationBitmap( rNewAnimationBitmap );

    
    
    if ( (  !nAnimation
         && (  !mbLoopTerminated
            || ( maList.size() == 1 )
            )
         )
         ||
         (  ( nAnimation == maList.size() - 1 )
         && mbLoopTerminated
         )
       )
    {
        maBitmapEx = rNewAnimationBitmap.aBmpEx;
    }
}

void Animation::SetLoopCount( const sal_uLong nLoopCount )
{
    mnLoopCount = nLoopCount;
    ResetLoopCount();
}

void Animation::ResetLoopCount()
{
    mnLoops = mnLoopCount;
    mbLoopTerminated = false;
}

bool Animation::Convert( BmpConversion eConversion )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    bool bRet;

    if( !IsInAnimation() && !maList.empty() )
    {
        bRet = true;

        for( size_t i = 0, n = maList.size(); ( i < n ) && bRet; ++i )
            bRet = maList[ i ]->aBmpEx.Convert( eConversion );

        maBitmapEx.Convert( eConversion );
    }
    else
        bRet = false;

    return bRet;
}

bool Animation::ReduceColors( sal_uInt16 nNewColorCount, BmpReduce eReduce )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    bool bRet;

    if( !IsInAnimation() && !maList.empty() )
    {
        bRet = true;

        for( size_t i = 0, n = maList.size(); ( i < n ) && bRet; ++i )
            bRet = maList[ i ]->aBmpEx.ReduceColors( nNewColorCount, eReduce );

        maBitmapEx.ReduceColors( nNewColorCount, eReduce );
    }
    else
        bRet = false;

    return bRet;
}

bool Animation::Invert()
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    bool bRet;

    if( !IsInAnimation() && !maList.empty() )
    {
        bRet = true;

        for( size_t i = 0, n = maList.size(); ( i < n ) && bRet; ++i )
            bRet = maList[ i ]->aBmpEx.Invert();

        maBitmapEx.Invert();
    }
    else
        bRet = false;

    return bRet;
}

bool Animation::Mirror( sal_uLong nMirrorFlags )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    bool    bRet;

    if( !IsInAnimation() && !maList.empty() )
    {
        bRet = true;

        if( nMirrorFlags )
        {
            for( size_t i = 0, n = maList.size(); ( i < n ) && bRet; ++i )
            {
                AnimationBitmap* pStepBmp = maList[ i ];
                if( ( bRet = pStepBmp->aBmpEx.Mirror( nMirrorFlags ) ) )
                {
                    if( nMirrorFlags & BMP_MIRROR_HORZ )
                        pStepBmp->aPosPix.X() = maGlobalSize.Width() - pStepBmp->aPosPix.X() - pStepBmp->aSizePix.Width();

                    if( nMirrorFlags & BMP_MIRROR_VERT )
                        pStepBmp->aPosPix.Y() = maGlobalSize.Height() - pStepBmp->aPosPix.Y() - pStepBmp->aSizePix.Height();
                }
            }

            maBitmapEx.Mirror( nMirrorFlags );
        }
    }
    else
        bRet = false;

    return bRet;
}

bool Animation::Adjust( short nLuminancePercent, short nContrastPercent,
             short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
             double fGamma, bool bInvert )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    bool bRet;

    if( !IsInAnimation() && !maList.empty() )
    {
        bRet = true;

        for( size_t i = 0, n = maList.size(); ( i < n ) && bRet; ++i )
        {
            bRet = maList[ i ]->aBmpEx.Adjust( nLuminancePercent,
                                               nContrastPercent,
                                               nChannelRPercent,
                                               nChannelGPercent,
                                               nChannelBPercent,
                                               fGamma, bInvert
                                             );
        }

        maBitmapEx.Adjust( nLuminancePercent, nContrastPercent,
                           nChannelRPercent, nChannelGPercent, nChannelBPercent,
                           fGamma, bInvert );
    }
    else
        bRet = false;

    return bRet;
}

bool Animation::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    bool bRet;

    if( !IsInAnimation() && !maList.empty() )
    {
        bRet = true;

        for( size_t i = 0, n = maList.size(); ( i < n ) && bRet; ++i )
            bRet = maList[ i ]->aBmpEx.Filter( eFilter, pFilterParam, pProgress );

        maBitmapEx.Filter( eFilter, pFilterParam, pProgress );
    }
    else
        bRet = false;

    return bRet;
}

SvStream& WriteAnimation( SvStream& rOStm, const Animation& rAnimation )
{
    const sal_uInt16 nCount = rAnimation.Count();

    if( nCount )
    {
        const sal_uInt32    nDummy32 = 0UL;

        
        
        if( !rAnimation.GetBitmapEx().GetBitmap() )
            WriteDIBBitmapEx(rAnimation.Get( 0 ).aBmpEx, rOStm);
        else
            WriteDIBBitmapEx(rAnimation.GetBitmapEx(), rOStm);

        
        rOStm.WriteUInt32( (sal_uInt32) 0x5344414e ).WriteUInt32( (sal_uInt32) 0x494d4931 );

        for( sal_uInt16 i = 0; i < nCount; i++ )
        {
            const AnimationBitmap&  rAnimBmp = rAnimation.Get( i );
            const sal_uInt16            nRest = nCount - i - 1;

            
            WriteDIBBitmapEx(rAnimBmp.aBmpEx, rOStm);
            WritePair( rOStm, rAnimBmp.aPosPix );
            WritePair( rOStm, rAnimBmp.aSizePix );
            WritePair( rOStm, rAnimation.maGlobalSize );
            rOStm.WriteUInt16( (sal_uInt16) ( ( ANIMATION_TIMEOUT_ON_CLICK == rAnimBmp.nWait ) ? 65535 : rAnimBmp.nWait ) );
            rOStm.WriteUInt16( (sal_uInt16) rAnimBmp.eDisposal );
            rOStm.WriteUChar( (sal_uInt8) rAnimBmp.bUserInput );
            rOStm.WriteUInt32( (sal_uInt32) rAnimation.mnLoopCount );
            rOStm.WriteUInt32( nDummy32 ); 
            rOStm.WriteUInt32( nDummy32 ); 
            rOStm.WriteUInt32( nDummy32 ); 
            write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, OString()); 
            rOStm.WriteUInt16( nRest ); 
        }
    }

    return rOStm;
}

SvStream& ReadAnimation( SvStream& rIStm, Animation& rAnimation )
{
    Bitmap  aBmp;
    sal_uLong   nStmPos = rIStm.Tell();
    sal_uInt32  nAnimMagic1, nAnimMagic2;
    sal_uInt16  nOldFormat = rIStm.GetNumberFormatInt();
    bool    bReadAnimations = false;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    nStmPos = rIStm.Tell();
    rIStm.ReadUInt32( nAnimMagic1 ).ReadUInt32( nAnimMagic2 );

    rAnimation.Clear();

    
    
    if( ( nAnimMagic1 == 0x5344414e ) && ( nAnimMagic2 == 0x494d4931 ) && !rIStm.GetError() )
        bReadAnimations = true;
    
    else
    {
        rIStm.Seek( nStmPos );
        ReadDIBBitmapEx(rAnimation.maBitmapEx, rIStm);
        nStmPos = rIStm.Tell();
        rIStm.ReadUInt32( nAnimMagic1 ).ReadUInt32( nAnimMagic2 );

        if( ( nAnimMagic1 == 0x5344414e ) && ( nAnimMagic2 == 0x494d4931 ) && !rIStm.GetError() )
            bReadAnimations = true;
        else
            rIStm.Seek( nStmPos );
    }

    
    if( bReadAnimations )
    {
        AnimationBitmap aAnimBmp;
        BitmapEx        aBmpEx;
        sal_uInt32          nTmp32;
        sal_uInt16          nTmp16;
        bool           cTmp;

        do
        {
            ReadDIBBitmapEx(aAnimBmp.aBmpEx, rIStm);
            ReadPair( rIStm, aAnimBmp.aPosPix );
            ReadPair( rIStm, aAnimBmp.aSizePix );
            ReadPair( rIStm, rAnimation.maGlobalSize );
            rIStm.ReadUInt16( nTmp16 ); aAnimBmp.nWait = ( ( 65535 == nTmp16 ) ? ANIMATION_TIMEOUT_ON_CLICK : nTmp16 );
            rIStm.ReadUInt16( nTmp16 ); aAnimBmp.eDisposal = ( Disposal) nTmp16;
            rIStm.ReadCharAsBool( cTmp ); aAnimBmp.bUserInput = cTmp;
            rIStm.ReadUInt32( nTmp32 ); rAnimation.mnLoopCount = (sal_uInt16) nTmp32;
            rIStm.ReadUInt32( nTmp32 ); 
            rIStm.ReadUInt32( nTmp32 ); 
            rIStm.ReadUInt32( nTmp32 ); 
            read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm); 
            rIStm.ReadUInt16( nTmp16 ); 

            rAnimation.Insert( aAnimBmp );
        }
        while( nTmp16 && !rIStm.GetError() );

        rAnimation.ResetLoopCount();
    }

    rIStm.SetNumberFormatInt( nOldFormat );

    return rIStm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
