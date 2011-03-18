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
#include "precompiled_sc.hxx"
#include "fprogressbar.hxx"
#include "global.hxx"
#include "progress.hxx"

// ============================================================================

ScfProgressBar::ScfProgressSegment::ScfProgressSegment( sal_Size nSize ) :
    mxProgress( 0 ),
    mnSize( nSize ),
    mnPos( 0 )
{
}

ScfProgressBar::ScfProgressSegment::~ScfProgressSegment()
{
}

// ============================================================================

ScfProgressBar::ScfProgressBar( SfxObjectShell* pDocShell, const String& rText ) :
    maText( rText )
{
    Init( pDocShell );
}

ScfProgressBar::ScfProgressBar( SfxObjectShell* pDocShell, sal_uInt16 nResId ) :
    maText( ScGlobal::GetRscString( nResId ) )
{
    Init( pDocShell );
}

ScfProgressBar::ScfProgressBar( ScfProgressBar& rParProgress, ScfProgressSegment* pParSegment )
{
    Init( rParProgress.mpDocShell );
    mpParentProgress = &rParProgress;
    mpParentSegment = pParSegment;
}

ScfProgressBar::~ScfProgressBar()
{
}

void ScfProgressBar::Init( SfxObjectShell* pDocShell )
{
    mpDocShell = pDocShell;
    mpParentProgress = 0;
    mpParentSegment = mpCurrSegment = 0;
    mnTotalSize = mnTotalPos = mnUnitSize = mnNextUnitPos = 0;
    mnSysProgressScale = 1;     // used to workaround the ULONG_MAX/100 limit
    mbInProgress = false;
}

ScfProgressBar::ScfProgressSegment* ScfProgressBar::GetSegment( sal_Int32 nSegment )
{
    if( nSegment < 0 )
        return 0;
    return &(maSegments.at( nSegment ));
}

void ScfProgressBar::SetCurrSegment( ScfProgressSegment* pSegment )
{
    if( mpCurrSegment != pSegment )
    {
        mpCurrSegment = pSegment;

        if( mpParentProgress && mpParentSegment )
        {
            mpParentProgress->SetCurrSegment( mpParentSegment );
        }
        else if( !mxSysProgress.get() && (mnTotalSize > 0) )
        {
            // System progress has an internal limit of ULONG_MAX/100.
            mnSysProgressScale = 1;
            sal_uLong nSysTotalSize = static_cast< sal_uLong >( mnTotalSize );
            while( nSysTotalSize >= ULONG_MAX / 100 )
            {
                nSysTotalSize /= 2;
                mnSysProgressScale *= 2;
            }
            mxSysProgress.reset( new ScProgress( mpDocShell, maText, nSysTotalSize ) );
        }

        if( !mbInProgress && mpCurrSegment && (mnTotalSize > 0) )
        {
            mnUnitSize = mnTotalSize / 256 + 1;   // at most 256 calls of system progress
            mnNextUnitPos = 0;
            mbInProgress = true;
        }
    }
}

void ScfProgressBar::IncreaseProgressBar( sal_Size nDelta )
{
    sal_Size nNewPos = mnTotalPos + nDelta;

    // call back to parent progress bar
    if( mpParentProgress && mpParentSegment )
    {
        // calculate new position of parent progress bar
        sal_Size nParentPos = static_cast< sal_Size >(
            static_cast< double >( nNewPos ) * mpParentSegment->mnSize / mnTotalSize );
        mpParentProgress->ProgressAbs( nParentPos );
    }
    // modify system progress bar
    else if( mxSysProgress.get() )
    {
        if( nNewPos >= mnNextUnitPos )
        {
            mnNextUnitPos = nNewPos + mnUnitSize;
            mxSysProgress->SetState( static_cast< sal_uLong >( nNewPos / mnSysProgressScale ) );
        }
    }
    else
    {
        DBG_ERRORFILE( "ScfProgressBar::IncreaseProgressBar - no progress bar found" );
    }

    mnTotalPos = nNewPos;
}

sal_Int32 ScfProgressBar::AddSegment( sal_Size nSize )
{
    DBG_ASSERT( !mbInProgress, "ScfProgressBar::AddSegment - already in progress mode" );
    if( nSize == 0 )
        return SCF_INV_SEGMENT;

    maSegments.push_back( new ScfProgressSegment( nSize ) );
    mnTotalSize += nSize;
    return static_cast< sal_Int32 >( maSegments.size() - 1 );
}

ScfProgressBar& ScfProgressBar::GetSegmentProgressBar( sal_Int32 nSegment )
{
    ScfProgressSegment* pSegment = GetSegment( nSegment );
    DBG_ASSERT( !pSegment || (pSegment->mnPos == 0), "ScfProgressBar::GetSegmentProgressBar - segment already started" );
    if( pSegment && (pSegment->mnPos == 0) )
    {
        if( !pSegment->mxProgress.get() )
            pSegment->mxProgress.reset( new ScfProgressBar( *this, pSegment ) );
        return *pSegment->mxProgress;
    }
    return *this;
}

bool ScfProgressBar::IsFull() const
{
    DBG_ASSERT( mbInProgress && mpCurrSegment, "ScfProgressBar::IsFull - no segment started" );
    return mpCurrSegment && (mpCurrSegment->mnPos >= mpCurrSegment->mnSize);
}

void ScfProgressBar::ActivateSegment( sal_Int32 nSegment )
{
    DBG_ASSERT( mnTotalSize > 0, "ScfProgressBar::ActivateSegment - progress range is zero" );
    if( mnTotalSize > 0 )
        SetCurrSegment( GetSegment( nSegment ) );
}

void ScfProgressBar::ProgressAbs( sal_Size nPos )
{
    DBG_ASSERT( mbInProgress && mpCurrSegment, "ScfProgressBar::ProgressAbs - no segment started" );
    if( mpCurrSegment )
    {
        DBG_ASSERT( mpCurrSegment->mnPos <= nPos, "ScfProgressBar::ProgressAbs - delta pos < 0" );
        DBG_ASSERT( nPos <= mpCurrSegment->mnSize, "ScfProgressBar::ProgressAbs - segment overflow" );
        if( (mpCurrSegment->mnPos < nPos) && (nPos <= mpCurrSegment->mnSize) )
        {
            IncreaseProgressBar( nPos - mpCurrSegment->mnPos );
            mpCurrSegment->mnPos = nPos;
        }
    }
}

void ScfProgressBar::Progress( sal_Size nDelta )
{
    ProgressAbs( mpCurrSegment ? (mpCurrSegment->mnPos + nDelta) : 0 );
}

// ============================================================================

ScfSimpleProgressBar::ScfSimpleProgressBar( sal_Size nSize, SfxObjectShell* pDocShell, const String& rText ) :
    maProgress( pDocShell, rText )
{
    Init( nSize );
}

ScfSimpleProgressBar::ScfSimpleProgressBar( sal_Size nSize, SfxObjectShell* pDocShell, sal_uInt16 nResId ) :
    maProgress( pDocShell, nResId )
{
    Init( nSize );
}

void ScfSimpleProgressBar::Init( sal_Size nSize )
{
    sal_Int32 nSegment = maProgress.AddSegment( nSize );
    if( nSegment >= 0 )
        maProgress.ActivateSegment( nSegment );
}

ScfStreamProgressBar::ScfStreamProgressBar( SvStream& rStrm, SfxObjectShell* pDocShell, sal_uInt16 nResId ) :
    mrStrm( rStrm )
{
    Init( pDocShell, ScGlobal::GetRscString( nResId ) );
}

void ScfStreamProgressBar::Progress()
{
    mxProgress->ProgressAbs( mrStrm.Tell() );
}

void ScfStreamProgressBar::Init( SfxObjectShell* pDocShell, const String& rText )
{
    sal_Size nPos = mrStrm.Tell();
    mrStrm.Seek( STREAM_SEEK_TO_END );
    sal_Size nSize = mrStrm.Tell();
    mrStrm.Seek( nPos );

    mxProgress.reset( new ScfSimpleProgressBar( nSize, pDocShell, rText ) );
    Progress();
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
