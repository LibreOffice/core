/*************************************************************************
 *
 *  $RCSfile: animate.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
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

#define _SV_ANIMATE_CXX
#define ENABLE_BYTESTRING_STREAM_OPERATORS

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_IMPANMVW_HXX
#include <impanmvw.hxx>
#endif
#ifndef _SV_ANIMATE_HXX
#include <animate.hxx>
#endif

DBG_NAME( Animation );

// -----------
// - Defines -
// -----------

#define MIN_TIMEOUT 2L
#define INC_TIMEOUT 0L

// -----------
// - statics -
// -----------

ULONG Animation::mnAnimCount = 0UL;

// -------------------
// - AnimationBitmap -
// -------------------

ULONG AnimationBitmap::GetChecksum() const
{
    sal_uInt32  nCrc = aBmpEx.GetChecksum();
    SVBT32      aBT32;

    LongToSVBT32( aPosPix.X(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( aPosPix.Y(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( aSizePix.Width(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( aSizePix.Height(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( (long) nWait, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( (long) eDisposal, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( (long) bUserInput, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    return nCrc;
}

// -------------
// - Animation -
// -------------

Animation::Animation() :
    mbIsInAnimation     ( FALSE ),
    meCycleMode         ( CYCLE_NORMAL ),
    mnLoopCount         ( 0 ),
    mnLoops             ( 0 ),
    mnPos               ( 0 ),
    mbLoopTerminated    ( FALSE ),
    mbIsWaiting         ( FALSE )
{
    DBG_CTOR( Animation, NULL );
    maTimer.SetTimeoutHdl( LINK( this, Animation, ImplTimeoutHdl ) );
    mpViewList = new List;
}

// -----------------------------------------------------------------------

Animation::Animation( const Animation& rAnimation ) :
    maGlobalSize        ( rAnimation.maGlobalSize ),
    maBitmapEx          ( rAnimation.maBitmapEx ),
    meCycleMode         ( rAnimation.meCycleMode ),
    mbIsInAnimation     ( FALSE ),
    mnLoopCount         ( rAnimation.mnLoopCount ),
    mnPos               ( rAnimation.mnPos ),
    mbLoopTerminated    ( rAnimation.mbLoopTerminated ),
    mbIsWaiting         ( rAnimation.mbIsWaiting )
{
    DBG_CTOR( Animation, NULL );

    for( long i = 0, nCount = rAnimation.maList.Count(); i < nCount; i++ )
        maList.Insert( new AnimationBitmap( *(AnimationBitmap*) rAnimation.maList.GetObject( i ) ), LIST_APPEND );

    maTimer.SetTimeoutHdl( LINK( this, Animation, ImplTimeoutHdl ) );
    mpViewList = new List;
    mnLoops = mbLoopTerminated ? 0 : mnLoopCount;
}

// -----------------------------------------------------------------------

Animation::~Animation()
{
    DBG_DTOR( Animation, NULL );

    if( mbIsInAnimation )
        Stop();

    for( void* pStepBmp = maList.First(); pStepBmp; pStepBmp = maList.Next() )
        delete (AnimationBitmap*) pStepBmp;

    for( void* pView = mpViewList->First(); pView; pView = mpViewList->Next() )
        delete (ImplAnimView*) pView;

    delete mpViewList;
}

// -----------------------------------------------------------------------

Animation& Animation::operator=( const Animation& rAnimation )
{
    Clear();

    for( long i = 0, nCount = rAnimation.maList.Count(); i < nCount; i++ )
        maList.Insert( new AnimationBitmap( *(AnimationBitmap*) rAnimation.maList.GetObject( i ) ), LIST_APPEND );

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

// -----------------------------------------------------------------------

BOOL Animation::operator==( const Animation& rAnimation ) const
{
    const ULONG nCount = maList.Count();
    BOOL        bRet = FALSE;

    if( rAnimation.maList.Count() == nCount &&
        rAnimation.maBitmapEx == maBitmapEx &&
        rAnimation.maGlobalSize == maGlobalSize &&
        rAnimation.meCycleMode == meCycleMode )
    {
        bRet = TRUE;

        for( ULONG n = 0; n < nCount; n++ )
        {
            if( ( *(AnimationBitmap*) maList.GetObject( n ) ) !=
                ( *(AnimationBitmap*) rAnimation.maList.GetObject( n ) ) )
            {
                bRet = FALSE;
                break;
            }
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL Animation::IsEqual( const Animation& rAnimation ) const
{
    const ULONG nCount = maList.Count();
    BOOL        bRet = FALSE;

    if( rAnimation.maList.Count() == nCount &&
        rAnimation.maBitmapEx.IsEqual( maBitmapEx ) &&
        rAnimation.maGlobalSize == maGlobalSize &&
        rAnimation.meCycleMode == meCycleMode )
    {
        for( ULONG n = 0; ( n < nCount ) && !bRet; n++ )
            if( ( (AnimationBitmap*) maList.GetObject( n ) )->IsEqual( *(AnimationBitmap*) rAnimation.maList.GetObject( n ) ) )
                bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL Animation::IsEmpty() const
{
    return( maBitmapEx.IsEmpty() && !maList.Count() );
}

// ------------------------------------------------------------------

void Animation::SetEmpty()
{
    maTimer.Stop();
    mbIsInAnimation = FALSE;
    maGlobalSize = Size();
    maBitmapEx.SetEmpty();

    for( void* pStepBmp = maList.First(); pStepBmp; pStepBmp = maList.Next() )
        delete (AnimationBitmap*) pStepBmp;
    maList.Clear();

    for( void* pView = mpViewList->First(); pView; pView = mpViewList->Next() )
        delete (ImplAnimView*) pView;
    mpViewList->Clear();
}

// -----------------------------------------------------------------------

void Animation::Clear()
{
    SetEmpty();
}

// -----------------------------------------------------------------------

BOOL Animation::IsTransparent() const
{
    Point       aPoint;
    Rectangle   aRect( aPoint, maGlobalSize );
    BOOL        bRet = FALSE;

    // Falls irgendein 'kleines' Bildchen durch den Hintergrund
    // ersetzt werden soll, muessen wir 'transparent' sein, um
    // richtig dargestellt zu werden, da die Appl. aus Optimierungsgruenden
    // kein Invalidate auf nicht-transp. Grafiken ausfuehren
    for( long i = 0, nCount = maList.Count(); i < nCount; i++ )
    {
        const AnimationBitmap* pAnimBmp = (AnimationBitmap*) maList.GetObject( i );

        if( DISPOSE_BACK == pAnimBmp->eDisposal && Rectangle( pAnimBmp->aPosPix, pAnimBmp->aSizePix ) != aRect )
        {
            bRet = TRUE;
            break;
        }
    }

    if( !bRet )
        bRet = maBitmapEx.IsTransparent();

    return bRet;
}

// -----------------------------------------------------------------------

ULONG Animation::GetSizeBytes() const
{
    ULONG nSizeBytes = GetBitmapEx().GetSizeBytes();

    for( long i = 0, nCount = maList.Count(); i < nCount; i++ )
    {
        const AnimationBitmap* pAnimBmp = (AnimationBitmap*) maList.GetObject( i );
        nSizeBytes += pAnimBmp->aBmpEx.GetSizeBytes();
    }

    return nSizeBytes;
}

// -----------------------------------------------------------------------

ULONG Animation::GetChecksum() const
{
    SVBT32      aBT32;
    sal_uInt32  nCrc = GetBitmapEx().GetChecksum();

    LongToSVBT32( maList.Count(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( maGlobalSize.Width(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( maGlobalSize.Height(), aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( (long) meCycleMode, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    for( long i = 0, nCount = maList.Count(); i < nCount; i++ )
    {
        LongToSVBT32( ( (AnimationBitmap*) maList.GetObject( i ) )->GetChecksum(), aBT32 );
        nCrc = rtl_crc32( nCrc, aBT32, 4 );
    }

    return nCrc;
}

// -----------------------------------------------------------------------

BOOL Animation::Start( OutputDevice* pOut, const Point& rDestPt, long nExtraData,
                       OutputDevice* pFirstFrameOutDev )
{
    return Start( pOut, rDestPt, pOut->PixelToLogic( maGlobalSize ), nExtraData, pFirstFrameOutDev );
}

// -----------------------------------------------------------------------

BOOL Animation::Start( OutputDevice* pOut, const Point& rDestPt, const Size& rDestSz, long nExtraData,
                       OutputDevice* pFirstFrameOutDev )
{
    BOOL bRet = FALSE;

    if( maList.Count() )
    {
        if( ( pOut->GetOutDevType() == OUTDEV_WINDOW ) && !mbLoopTerminated &&
            ( ANIMATION_TIMEOUT_ON_CLICK != ( (AnimationBitmap*) maList.GetObject( mnPos ) )->nWait ) )
        {
            ImplAnimView*   pView;
            ImplAnimView*   pMatch = NULL;

            for( pView = (ImplAnimView*) mpViewList->First(); pView; pView = (ImplAnimView*) mpViewList->Next() )
            {
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
                        delete (ImplAnimView*) mpViewList->Remove( pView );
                        pView = NULL;
                    }

                    break;
                }
            }

            if( !mpViewList->Count() )
            {
                maTimer.Stop();
                mbIsInAnimation = FALSE;
                mnPos = 0UL;
            }

            if( !pMatch )
                mpViewList->Insert( new ImplAnimView( this, pOut, rDestPt, rDestSz, nExtraData, pFirstFrameOutDev ), LIST_APPEND );

            if( !mbIsInAnimation )
            {
                ImplRestartTimer( ( (AnimationBitmap*) maList.GetObject( mnPos ) )->nWait );
                mbIsInAnimation = TRUE;
            }
        }
        else
            Draw( pOut, rDestPt, rDestSz );

        bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------

void Animation::Stop( OutputDevice* pOut, long nExtraData )
{
    ImplAnimView* pView = (ImplAnimView*) mpViewList->First();

    while( pView )
    {
        if( pView->ImplMatches( pOut, nExtraData ) )
        {
            delete (ImplAnimView*) mpViewList->Remove( pView );
            pView = (ImplAnimView*) mpViewList->GetCurObject();
        }
        else
            pView = (ImplAnimView*) mpViewList->Next();
    }

    if( !mpViewList->Count() )
    {
        maTimer.Stop();
        mbIsInAnimation = FALSE;
    }
}

// -----------------------------------------------------------------------

void Animation::Draw( OutputDevice* pOut, const Point& rDestPt ) const
{
    Draw( pOut, rDestPt, pOut->PixelToLogic( maGlobalSize ) );
}

// -----------------------------------------------------------------------

void Animation::Draw( OutputDevice* pOut, const Point& rDestPt, const Size& rDestSz ) const
{
    const ULONG nCount = maList.Count();

    if( nCount )
    {
        AnimationBitmap* pObj = (AnimationBitmap*) maList.GetObject( Min( mnPos, (long) nCount - 1L ) );

        if( pOut->GetConnectMetaFile() || ( pOut->GetOutDevType() == OUTDEV_PRINTER ) )
            ( (AnimationBitmap*) maList.GetObject( 0 ) )->aBmpEx.Draw( pOut, rDestPt, rDestSz );
        else if( ANIMATION_TIMEOUT_ON_CLICK == pObj->nWait )
            pObj->aBmpEx.Draw( pOut, rDestPt, rDestSz );
        else
        {
            const ULONG nOldPos = mnPos;
            ( (Animation*) this )->mnPos = mbLoopTerminated ? ( nCount - 1UL ) : mnPos;
            delete new ImplAnimView( (Animation*) this, pOut, rDestPt, rDestSz, 0 );
            ( (Animation*) this )->mnPos = nOldPos;
        }
    }
}

// -----------------------------------------------------------------------

void Animation::ImplRestartTimer( ULONG nTimeout )
{
#ifdef REMOTE_APPSERVER
    nTimeout = nTimeout > 40 ? nTimeout : 40;
#endif
    maTimer.SetTimeout( Max( nTimeout, MIN_TIMEOUT + ( mnAnimCount - 1 ) * INC_TIMEOUT ) * 10L );
    maTimer.Start();
}

// -----------------------------------------------------------------------

IMPL_LINK( Animation, ImplTimeoutHdl, Timer*, pTimer )
{
    const ULONG nAnimCount = maList.Count();

    if( nAnimCount )
    {
        ImplAnimView*   pView;
        BOOL            bGlobalPause = TRUE;

        if( maNotifyLink.IsSet() )
        {
            AInfo* pAInfo;

            // create AInfo-List
            for( pView = (ImplAnimView*) mpViewList->First(); pView; pView = (ImplAnimView*) mpViewList->Next() )
                maAInfoList.Insert( pView->ImplCreateAInfo() );

            maNotifyLink.Call( this );

            // set view state from AInfo structure
            for( pAInfo = (AInfo*) maAInfoList.First(); pAInfo; pAInfo = (AInfo*) maAInfoList.Next() )
            {
                if( !pAInfo->pViewData )
                {
                    pView = new ImplAnimView( this, pAInfo->pOutDev,
                                              pAInfo->aStartOrg, pAInfo->aStartSize, pAInfo->nExtraData );

                    mpViewList->Insert( pView, LIST_APPEND );
                }
                else
                    pView = (ImplAnimView*) pAInfo->pViewData;

                pView->ImplPause( pAInfo->bPause );
                pView->ImplSetMarked( TRUE );
            }

            // delete AInfo structures
            for( pAInfo = (AInfo*) maAInfoList.First(); pAInfo; pAInfo = (AInfo*) maAInfoList.Next() )
                delete (AInfo*) pAInfo;
            maAInfoList.Clear();

            // delete all unmarked views and reset marked state
            pView = (ImplAnimView*) mpViewList->First();
            while( pView )
            {
                if( !pView->ImplIsMarked() )
                {
                    delete (ImplAnimView*) mpViewList->Remove( pView );
                    pView = (ImplAnimView*) mpViewList->GetCurObject();
                }
                else
                {
                    if( !pView->ImplIsPause() )
                        bGlobalPause = FALSE;

                    pView->ImplSetMarked( FALSE );
                    pView = (ImplAnimView*) mpViewList->Next();
                }
            }
        }
        else
            bGlobalPause = FALSE;

        if( !mpViewList->Count() )
            Stop();
        else if( bGlobalPause )
            ImplRestartTimer( 10 );
        else
        {
            AnimationBitmap* pStepBmp = (AnimationBitmap*) maList.GetObject( ++mnPos );

            if( !pStepBmp )
            {
                if( mnLoops == 1 )
                {
                    Stop();
                    mbLoopTerminated = TRUE;
                    mnPos = nAnimCount - 1UL;
                    maBitmapEx = ( (AnimationBitmap*) maList.GetObject( mnPos ) )->aBmpEx;
                    return 0L;
                }
                else
                {
                    if( mnLoops )
                        mnLoops--;

                    mnPos = 0;
                    pStepBmp = (AnimationBitmap*) maList.GetObject( mnPos );
                }
            }

            // Paint all views; after painting check, if view is
            // marked; in this case remove view, because area of output
            // lies out of display area of window; mark state is
            // set from view itself
            pView = (ImplAnimView*) mpViewList->First();
            while( pView )
            {
                pView->ImplDraw( mnPos );

                if( pView->ImplIsMarked() )
                {
                    delete (ImplAnimView*) mpViewList->Remove( pView );
                    pView = (ImplAnimView*) mpViewList->GetCurObject();
                }
                else
                    pView = (ImplAnimView*) mpViewList->Next();
            }

            // stop or restart timer
            if( !mpViewList->Count() )
                Stop();
            else
                ImplRestartTimer( pStepBmp->nWait );
        }
    }
    else
        Stop();

    return 0L;
}

// -----------------------------------------------------------------------

BOOL Animation::Insert( const AnimationBitmap& rStepBmp )
{
    BOOL bRet = FALSE;

    if( !IsInAnimation() )
    {
        Point       aPoint;
        Rectangle   aGlobalRect( aPoint, maGlobalSize );

        maGlobalSize = aGlobalRect.Union( Rectangle( rStepBmp.aPosPix, rStepBmp.aSizePix ) ).GetSize();
        maList.Insert( new AnimationBitmap( rStepBmp ), LIST_APPEND );

        // zunaechst nehmen wir die erste BitmapEx als Ersatz-BitmapEx
        if( maList.Count() == 1 )
            maBitmapEx = rStepBmp.aBmpEx;

        bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------

const AnimationBitmap& Animation::Get( USHORT nAnimation ) const
{
    DBG_ASSERT( ( nAnimation < maList.Count() ), "No object at this position" );
    return *(AnimationBitmap*) maList.GetObject( nAnimation );
}

// -----------------------------------------------------------------------

void Animation::Replace( const AnimationBitmap& rNewAnimationBitmap, USHORT nAnimation )
{
    DBG_ASSERT( ( nAnimation < maList.Count() ), "No object at this position" );

    delete (AnimationBitmap*) maList.Replace( new AnimationBitmap( rNewAnimationBitmap ), nAnimation );

    // Falls wir an erster Stelle einfuegen,
    // muessen wir natuerlich auch,
    // auch die Ersatzdarstellungs-BitmapEx
    // aktualisieren;
    if ( ( !nAnimation && ( !mbLoopTerminated || ( maList.Count() == 1 ) ) ) ||
         ( ( nAnimation == maList.Count() - 1 ) && mbLoopTerminated ) )
    {
        maBitmapEx = rNewAnimationBitmap.aBmpEx;
    }
}

// -----------------------------------------------------------------------

void Animation::SetLoopCount( const ULONG nLoopCount )
{
    mnLoopCount = nLoopCount;
    ResetLoopCount();
}

// -----------------------------------------------------------------------

void Animation::ResetLoopCount()
{
    mnLoops = mnLoopCount;
    mbLoopTerminated = FALSE;
}

// -----------------------------------------------------------------------

BOOL Animation::Convert( BmpConversion eConversion )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    BOOL bRet;

    if( !IsInAnimation() && maList.Count() )
    {
        bRet = TRUE;

        for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
            bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Convert( eConversion );

        maBitmapEx.Convert( eConversion );
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------

BOOL Animation::ReduceColors( USHORT nNewColorCount, BmpReduce eReduce )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    BOOL bRet;

    if( !IsInAnimation() && maList.Count() )
    {
        bRet = TRUE;

        for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
            bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.ReduceColors( nNewColorCount, eReduce );

        maBitmapEx.ReduceColors( nNewColorCount, eReduce );
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------

BOOL Animation::Invert()
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    BOOL bRet;

    if( !IsInAnimation() && maList.Count() )
    {
        bRet = TRUE;

        for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
            bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Invert();

        maBitmapEx.Invert();
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------

BOOL Animation::Mirror( ULONG nMirrorFlags )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    BOOL    bRet;

    if( !IsInAnimation() && maList.Count() )
    {
        bRet = TRUE;

        if( nMirrorFlags )
        {
            for( AnimationBitmap* pStepBmp = (AnimationBitmap*) maList.First();
                 pStepBmp && bRet;
                 pStepBmp = (AnimationBitmap*) maList.Next() )
            {
                if( ( bRet = pStepBmp->aBmpEx.Mirror( nMirrorFlags ) ) == TRUE )
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
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------

BOOL Animation::Dither( ULONG nDitherFlags, const BitmapPalette* pDitherPal )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    BOOL bRet;

    if( !IsInAnimation() && maList.Count() )
    {
        bRet = TRUE;

        for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
            bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Dither( nDitherFlags, pDitherPal );

        maBitmapEx.Dither( nDitherFlags, pDitherPal );
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------

BOOL Animation::Adjust( short nLuminancePercent, short nContrastPercent,
             short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
             double fGamma, BOOL bInvert )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    BOOL bRet;

    if( !IsInAnimation() && maList.Count() )
    {
        bRet = TRUE;

        for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
        {
            bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Adjust( nLuminancePercent, nContrastPercent,
                                                                    nChannelRPercent, nChannelGPercent, nChannelBPercent,
                                                                    fGamma, bInvert );
        }

        maBitmapEx.Adjust( nLuminancePercent, nContrastPercent,
                           nChannelRPercent, nChannelGPercent, nChannelBPercent,
                           fGamma, bInvert );
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------

BOOL Animation::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

    BOOL bRet;

    if( !IsInAnimation() && maList.Count() )
    {
        bRet = TRUE;

        for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
            bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Filter( eFilter, pFilterParam, pProgress );

        maBitmapEx.Filter( eFilter, pFilterParam, pProgress );
    }
    else
        bRet = FALSE;

    return bRet;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Animation& rAnimation )
{
    const USHORT nCount = rAnimation.Count();

    if( nCount )
    {
        const ByteString    aDummyStr;
        const UINT32        nDummy32 = 0UL;

        // Falls keine BitmapEx gesetzt wurde, schreiben wir
        // einfach die erste Bitmap der Animation
        if( !rAnimation.GetBitmapEx().GetBitmap() )
            rOStm << rAnimation.Get( 0 ).aBmpEx;
        else
            rOStm << rAnimation.GetBitmapEx();

        // Kennung schreiben ( SDANIMA1 )
        rOStm << (UINT32) 0x5344414e << (UINT32) 0x494d4931;

        for( USHORT i = 0; i < nCount; i++ )
        {
            const AnimationBitmap&  rAnimBmp = rAnimation.Get( i );
            const UINT16            nRest = nCount - i - 1;

            // AnimationBitmap schreiben
            rOStm << rAnimBmp.aBmpEx;
            rOStm << rAnimBmp.aPosPix;
            rOStm << rAnimBmp.aSizePix;
            rOStm << rAnimation.maGlobalSize;
            rOStm << (UINT16) ( ( ANIMATION_TIMEOUT_ON_CLICK == rAnimBmp.nWait ) ? 65535 : rAnimBmp.nWait );
            rOStm << (UINT16) rAnimBmp.eDisposal;
            rOStm << (BYTE) rAnimBmp.bUserInput;
            rOStm << (UINT32) rAnimation.mnLoopCount;
            rOStm << nDummy32;  // unbenutzt
            rOStm << nDummy32;  // unbenutzt
            rOStm << nDummy32;  // unbenutzt
            rOStm << aDummyStr; // unbenutzt
            rOStm << nRest;     // Anzahl der Strukturen, die noch _folgen_
        }
    }

    return rOStm;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Animation& rAnimation )
{
    Bitmap  aBmp;
    ULONG   nStmPos = rIStm.Tell();
    UINT32  nAnimMagic1, nAnimMagic2;
    USHORT  nOldFormat = rIStm.GetNumberFormatInt();
    BOOL    bReadAnimations = FALSE;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    nStmPos = rIStm.Tell();
    rIStm >> nAnimMagic1 >> nAnimMagic2;

    rAnimation.Clear();

    // Wenn die BitmapEx am Anfang schon gelesen
    // wurde ( von Graphic ), koennen wir direkt die Animationsbitmaps einlesen
    if( ( nAnimMagic1 == 0x5344414e ) && ( nAnimMagic2 == 0x494d4931 ) && !rIStm.GetError() )
        bReadAnimations = TRUE;
    // ansonsten versuchen wir erstmal die Bitmap(-Ex) zu lesen
    else
    {
        rIStm.Seek( nStmPos );
        rIStm >> rAnimation.maBitmapEx;
        nStmPos = rIStm.Tell();
        rIStm >> nAnimMagic1 >> nAnimMagic2;

        if( ( nAnimMagic1 == 0x5344414e ) && ( nAnimMagic2 == 0x494d4931 ) && !rIStm.GetError() )
            bReadAnimations = TRUE;
        else
            rIStm.Seek( nStmPos );
    }

    // ggf. Animationsbitmaps lesen
    if( bReadAnimations )
    {
        AnimationBitmap aAnimBmp;
        BitmapEx        aBmpEx;
        ByteString      aDummyStr;
        Point           aPoint;
        Size            aSize;
        UINT32          nTmp32;
        UINT16          nTmp16;
        BYTE            cTmp;

        do
        {
            rIStm >> aAnimBmp.aBmpEx;
            rIStm >> aAnimBmp.aPosPix;
            rIStm >> aAnimBmp.aSizePix;
            rIStm >> rAnimation.maGlobalSize;
            rIStm >> nTmp16; aAnimBmp.nWait = ( ( 65535 == nTmp16 ) ? ANIMATION_TIMEOUT_ON_CLICK : nTmp16 );
            rIStm >> nTmp16; aAnimBmp.eDisposal = ( Disposal) nTmp16;
            rIStm >> cTmp; aAnimBmp.bUserInput = (BOOL) cTmp;
            rIStm >> nTmp32; rAnimation.mnLoopCount = (USHORT) nTmp32;
            rIStm >> nTmp32;    // unbenutzt
            rIStm >> nTmp32;    // unbenutzt
            rIStm >> nTmp32;    // unbenutzt
            rIStm >> aDummyStr; // unbenutzt
            rIStm >> nTmp16;    // Rest zu lesen

            rAnimation.Insert( aAnimBmp );
        }
        while( nTmp16 && !rIStm.GetError() );

        rAnimation.ResetLoopCount();
    }

    rIStm.SetNumberFormatInt( nOldFormat );

    return rIStm;
}
