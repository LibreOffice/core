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

#include <fprogressbar.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <progress.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>

#include <climits>

ScfProgressBar::ScfProgressSegment::ScfProgressSegment( std::size_t nSize ) :
    mnSize( nSize ),
    mnPos( 0 )
{
}

ScfProgressBar::ScfProgressSegment::~ScfProgressSegment()
{
}

ScfProgressBar::ScfProgressBar( SfxObjectShell* pDocShell, const OUString& rText ) :
    maText( rText )
{
    Init( pDocShell );
}

ScfProgressBar::ScfProgressBar(SfxObjectShell* pDocShell, const char* pResId)
    : maText(ScResId(pResId))
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
    mpParentProgress = nullptr;
    mpParentSegment = mpCurrSegment = nullptr;
    mnTotalSize = mnTotalPos = mnUnitSize = mnNextUnitPos = 0;
    mnSysProgressScale = 1;     // used to workaround the ULONG_MAX/100 limit
    mbInProgress = false;
}

ScfProgressBar::ScfProgressSegment* ScfProgressBar::GetSegment( sal_Int32 nSegment )
{
    if( nSegment < 0 )
        return nullptr;
    return maSegments.at( nSegment ).get();
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
            mxSysProgress.reset( new ScProgress( mpDocShell, maText, nSysTotalSize, true ) );
        }

        if( !mbInProgress && mpCurrSegment && (mnTotalSize > 0) )
        {
            mnUnitSize = mnTotalSize / 256 + 1;   // at most 256 calls of system progress
            mnNextUnitPos = 0;
            mbInProgress = true;
        }
    }
}

void ScfProgressBar::IncreaseProgressBar( std::size_t nDelta )
{
    std::size_t nNewPos = mnTotalPos + nDelta;

    // call back to parent progress bar
    if( mpParentProgress && mpParentSegment )
    {
        // calculate new position of parent progress bar
        std::size_t nParentPos = static_cast< std::size_t >(
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
        OSL_FAIL( "ScfProgressBar::IncreaseProgressBar - no progress bar found" );
    }

    mnTotalPos = nNewPos;
}

sal_Int32 ScfProgressBar::AddSegment( std::size_t nSize )
{
    OSL_ENSURE( !mbInProgress, "ScfProgressBar::AddSegment - already in progress mode" );
    if( nSize == 0 )
        return SCF_INV_SEGMENT;

    maSegments.push_back( std::make_unique<ScfProgressSegment>( nSize ) );
    mnTotalSize += nSize;
    return static_cast< sal_Int32 >( maSegments.size() - 1 );
}

ScfProgressBar& ScfProgressBar::GetSegmentProgressBar( sal_Int32 nSegment )
{
    ScfProgressSegment* pSegment = GetSegment( nSegment );
    OSL_ENSURE( !pSegment || (pSegment->mnPos == 0), "ScfProgressBar::GetSegmentProgressBar - segment already started" );
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
    OSL_ENSURE( mbInProgress && mpCurrSegment, "ScfProgressBar::IsFull - no segment started" );
    return mpCurrSegment && (mpCurrSegment->mnPos >= mpCurrSegment->mnSize);
}

void ScfProgressBar::ActivateSegment( sal_Int32 nSegment )
{
    OSL_ENSURE( mnTotalSize > 0, "ScfProgressBar::ActivateSegment - progress range is zero" );
    if( mnTotalSize > 0 )
        SetCurrSegment( GetSegment( nSegment ) );
}

void ScfProgressBar::ProgressAbs( std::size_t nPos )
{
    OSL_ENSURE( mbInProgress && mpCurrSegment, "ScfProgressBar::ProgressAbs - no segment started" );
    if( mpCurrSegment )
    {
        OSL_ENSURE( mpCurrSegment->mnPos <= nPos, "ScfProgressBar::ProgressAbs - delta pos < 0" );
        OSL_ENSURE( nPos <= mpCurrSegment->mnSize, "ScfProgressBar::ProgressAbs - segment overflow" );
        if( (mpCurrSegment->mnPos < nPos) && (nPos <= mpCurrSegment->mnSize) )
        {
            IncreaseProgressBar( nPos - mpCurrSegment->mnPos );
            mpCurrSegment->mnPos = nPos;
        }
    }
}

void ScfProgressBar::Progress( std::size_t nDelta )
{
    ProgressAbs( mpCurrSegment ? (mpCurrSegment->mnPos + nDelta) : 0 );
}

ScfSimpleProgressBar::ScfSimpleProgressBar( std::size_t nSize, SfxObjectShell* pDocShell, const OUString& rText ) :
    maProgress( pDocShell, rText )
{
    Init( nSize );
}

ScfSimpleProgressBar::ScfSimpleProgressBar(std::size_t nSize, SfxObjectShell* pDocShell, const char* pResId)
    : maProgress(pDocShell, pResId)
{
    Init( nSize );
}

void ScfSimpleProgressBar::Init( std::size_t nSize )
{
    sal_Int32 nSegment = maProgress.AddSegment( nSize );
    if( nSegment >= 0 )
        maProgress.ActivateSegment( nSegment );
}

ScfStreamProgressBar::ScfStreamProgressBar( SvStream& rStrm, SfxObjectShell* pDocShell ) :
    mrStrm( rStrm )
{
    Init( pDocShell, ScResId( STR_LOAD_DOC ) );
}

void ScfStreamProgressBar::Progress()
{
    mxProgress->ProgressAbs( mrStrm.Tell() );
}

void ScfStreamProgressBar::Init( SfxObjectShell* pDocShell, const OUString& rText )
{
    sal_uInt64 const nSize = mrStrm.TellEnd();
    mxProgress.reset( new ScfSimpleProgressBar( nSize, pDocShell, rText ) );
    Progress();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
