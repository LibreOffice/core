/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fprogressbar.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 11:54:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_FPROGRESSBAR_HXX
#include "fprogressbar.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_PROGRESS_HXX
#include "progress.hxx"
#endif

// ============================================================================

ScfProgressBar::ScfProgressSegment::ScfProgressSegment( sal_uInt32 nSize ) :
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

ScfProgressBar::ScfProgressBar( SfxObjectShell* pDocShell, USHORT nResId ) :
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

ScfProgressBar::ScfProgressSegment* ScfProgressBar::GetSegment( sal_Int32 nSegment ) const
{
    if( nSegment < 0 )
        return 0;
    DBG_ASSERT( maSegments.GetObject( nSegment ), "ScfProgressBar::GetSegment - invalid segment index" );
    return maSegments.GetObject( nSegment );
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
        else if( !mxSysProgress.get() && mnTotalSize )
        {
            // System progress has an internal limit of ULONG_MAX/100.
            mnSysProgressScale = 1;
            ULONG nSysTotalSize = mnTotalSize;
            while( nSysTotalSize >= ULONG_MAX / 100 )
            {
                nSysTotalSize /= 2;
                mnSysProgressScale *= 2;
            }
            mxSysProgress.reset( new ScProgress( mpDocShell, maText, nSysTotalSize ) );
        }

        if( !mbInProgress && mpCurrSegment && mnTotalSize )
        {
            mnUnitSize = mnTotalSize / 256 + 1;   // at most 256 calls of system progress
            mnNextUnitPos = 0;
            mbInProgress = true;
        }
    }
}

void ScfProgressBar::IncreaseProgressBar( sal_uInt32 nDelta )
{
    sal_uInt32 nNewPos = mnTotalPos + nDelta;

    // call back to parent progress bar
    if( mpParentProgress && mpParentSegment )
    {
        // calculate new position of parent progress bar
        sal_uInt32 nParentPos = static_cast< sal_uInt32 >(
            static_cast< double >( nNewPos ) * mpParentSegment->mnSize / mnTotalSize );
        mpParentProgress->ProgressAbs( nParentPos );
    }
    // modify system progress bar
    else if( mxSysProgress.get() )
    {
        if( nNewPos >= mnNextUnitPos )
        {
            mnNextUnitPos = nNewPos + mnUnitSize;
            mxSysProgress->SetState( nNewPos / mnSysProgressScale );
        }
    }
    else
        DBG_ERRORFILE( "ScfProgressBar::IncreaseProgressBar - no progress bar found" );

    mnTotalPos = nNewPos;
}

sal_Int32 ScfProgressBar::AddSegment( sal_uInt32 nSize )
{
    DBG_ASSERT( !mbInProgress, "ScfProgressBar::AddSegment - already in progress mode" );
    if( !nSize )
        return SCF_INV_SEGMENT;

    maSegments.Append( new ScfProgressSegment( nSize ) );
    mnTotalSize += nSize;
    return maSegments.Count() - 1;
}

ScfProgressBar& ScfProgressBar::GetSegmentProgressBar( sal_Int32 nSegment )
{
    ScfProgressSegment* pSegment = GetSegment( nSegment );
    DBG_ASSERT( !pSegment || !pSegment->mnPos, "ScfProgressBar::GetSegmentProgressBar - segment already started" );
    if( pSegment && !pSegment->mnPos )
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
    DBG_ASSERT( mnTotalSize, "ScfProgressBar::ActivateSegment - progress range is zero" );
    if( mnTotalSize )
        SetCurrSegment( GetSegment( nSegment ) );
}

void ScfProgressBar::ProgressAbs( sal_uInt32 nPos )
{
    DBG_ASSERT( mbInProgress && mpCurrSegment, "ScfProgressBar::Progress - no segment started" );
    if( mpCurrSegment )
    {
        DBG_ASSERT( mpCurrSegment->mnPos <= nPos, "ScfProgressBar::Progress - delta pos < 0" );
        DBG_ASSERT( nPos <= mpCurrSegment->mnSize, "ScfProgressBar::Progress - segment overflow" );
        if( (mpCurrSegment->mnPos < nPos) && (nPos <= mpCurrSegment->mnSize) )
        {
            IncreaseProgressBar( nPos - mpCurrSegment->mnPos );
            mpCurrSegment->mnPos = nPos;
        }
    }
}

void ScfProgressBar::Progress( sal_uInt32 nDelta )
{
    ProgressAbs( mpCurrSegment ? (mpCurrSegment->mnPos + nDelta) : 0 );
}

// ============================================================================

ScfSimpleProgressBar::ScfSimpleProgressBar( sal_uInt32 nSize, SfxObjectShell* pDocShell, const String& rText ) :
    maProgress( pDocShell, rText )
{
    Init( nSize );
}

ScfSimpleProgressBar::ScfSimpleProgressBar( sal_uInt32 nSize, SfxObjectShell* pDocShell, USHORT nResId ) :
    maProgress( pDocShell, nResId )
{
    Init( nSize );
}

void ScfSimpleProgressBar::Init( sal_uInt32 nSize )
{
    sal_Int32 nSegment = maProgress.AddSegment( nSize );
    if( nSegment >= 0 )
        maProgress.ActivateSegment( nSegment );
}

// ============================================================================

ScfStreamProgressBar::ScfStreamProgressBar( SvStream& rStrm, SfxObjectShell* pDocShell, const String& rText ) :
    mrStrm( rStrm )
{
    Init( pDocShell, rText );
}

ScfStreamProgressBar::ScfStreamProgressBar( SvStream& rStrm, SfxObjectShell* pDocShell, USHORT nResId ) :
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
    sal_uInt32 nPos = mrStrm.Tell();
    mrStrm.Seek( STREAM_SEEK_TO_END );
    sal_uInt32 nSize = mrStrm.Tell();
    mrStrm.Seek( nPos );

    mxProgress.reset( new ScfSimpleProgressBar( nSize, pDocShell, rText ) );
    Progress();
}

// ============================================================================

