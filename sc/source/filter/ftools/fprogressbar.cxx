/*************************************************************************
 *
 *  $RCSfile: fprogressbar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:50 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// ============================================================================

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

const sal_Int32 SCF_INVALID_SEG = -1;


// ============================================================================

ScfProgressBar::ScfProgressSegment::ScfProgressSegment( sal_uInt32 nSize ) :
    mpProgress( NULL ),
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
    mpParentProgress = NULL;
    mpParentSegment = mpCurrSegment = NULL;
    mnTotalSize = mnTotalPos = mnUnitSize = mnNextUnitPos = 0;
    mbInProgress = false;
}

ScfProgressBar::ScfProgressSegment* ScfProgressBar::GetSegment( sal_Int32 nSegment ) const
{
    if( nSegment < 0 )
        return NULL;
    DBG_ASSERT( maSegments.GetObject( nSegment ), "ScfProgressBar::GetSegment - invalid segment index" );
    return maSegments.GetObject( nSegment );
}

void ScfProgressBar::SetCurrSegment( ScfProgressSegment* pSegment )
{
    if( mpCurrSegment != pSegment )
    {
        mpCurrSegment = pSegment;

        if( mpParentProgress && mpParentSegment )
            mpParentProgress->SetCurrSegment( mpParentSegment );
        else if( !mpSysProgress.get() && mnTotalSize )
            mpSysProgress.reset( new ScProgress( mpDocShell, maText, mnTotalSize ) );

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
        mpParentProgress->Progress( nParentPos );
    }
    // modify system progress bar
    else if( mpSysProgress.get() )
    {
        if( nNewPos >= mnNextUnitPos )
        {
            mnNextUnitPos = nNewPos + mnUnitSize;
            mpSysProgress->SetState( nNewPos );
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
        return SCF_INVALID_SEG;

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
        if( !pSegment->mpProgress.get() )
            pSegment->mpProgress.reset( new ScfProgressBar( *this, pSegment ) );
        return *pSegment->mpProgress;
    }
    return *this;
}

void ScfProgressBar::ActivateSegment( sal_Int32 nSegment )
{
    DBG_ASSERT( mnTotalSize, "ScfProgressBar::ActivateSegment - progress range is zero" );
    if( mnTotalSize )
        SetCurrSegment( GetSegment( nSegment ) );
}

void ScfProgressBar::Progress( sal_uInt32 nPos )
{
    DBG_ASSERT( mbInProgress && mpCurrSegment, "ScfProgressBar::Progress - no segment started" );
    if( mpCurrSegment )
    {
        DBG_ASSERT( !mpCurrSegment->mpProgress.get(), "ScfProgressBar::Progress - contains sub progress" );
        DBG_ASSERT( mpCurrSegment->mnPos <= nPos, "ScfProgressBar::Progress - delta pos < 0" );
        DBG_ASSERT( nPos <= mpCurrSegment->mnSize, "ScfProgressBar::Progress - segment overflow" );
        if( (mpCurrSegment->mnPos < nPos) && (nPos <= mpCurrSegment->mnSize) )
        {
            IncreaseProgressBar( nPos - mpCurrSegment->mnPos );
            mpCurrSegment->mnPos = nPos;
        }
    }
}

void ScfProgressBar::Progress()
{
    Progress( mpCurrSegment ? (mpCurrSegment->mnPos + 1) : 0 );
}


// ============================================================================

ScfSimpleProgressBar::ScfSimpleProgressBar( sal_uInt32 nSize, SfxObjectShell* pDocShell, const String& rText ) :
    maProgress( pDocShell, rText )
{
    Init( nSize );
}

ScfSimpleProgressBar::ScfSimpleProgressBar( sal_uInt32 nSize, SfxObjectShell* pDocShell, sal_uInt16 nResId ) :
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

ScfStreamProgressBar::ScfStreamProgressBar( SvStream& rStrm, SfxObjectShell* pDocShell, sal_uInt16 nResId ) :
    mrStrm( rStrm )
{
    Init( pDocShell, ScGlobal::GetRscString( nResId ) );
}

void ScfStreamProgressBar::Progress()
{
    mpProgress->Progress( mrStrm.Tell() );
}

void ScfStreamProgressBar::Init( SfxObjectShell* pDocShell, const String& rText )
{
    sal_uInt32 nPos = mrStrm.Tell();
    mrStrm.Seek( STREAM_SEEK_TO_END );
    sal_uInt32 nSize = mrStrm.Tell();
    mrStrm.Seek( nPos );

    mpProgress.reset( new ScfSimpleProgressBar( nSize, pDocShell, rText ) );
    Progress();
}


// ============================================================================

