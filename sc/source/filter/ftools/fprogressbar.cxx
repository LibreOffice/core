/*************************************************************************
 *
 *  $RCSfile: fprogressbar.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2002-11-21 12:09:09 $
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

ScfProgressBar::ScfProgressBar( const String& rText ) :
    maText( rText ),
    mpSysProgress( NULL ),
    mpParentProgress( NULL ),
    mpParentSegment( NULL ),
    mpCurrSegment( NULL ),
    mnTotalSize( 0 ),
    mnTotalPos( 0 ),
    mnUnitSize( 0 ),
    mnNextUnitPos( 0 ),
    mbInProgress( false )
{
}

ScfProgressBar::ScfProgressBar( ScfProgressBar* pParProgress, ScfProgressSegment* pParSegment ) :
    mpSysProgress( NULL ),
    mpParentProgress( pParProgress ),
    mpParentSegment( pParSegment ),
    mpCurrSegment( NULL ),
    mnTotalSize( 0 ),
    mnTotalPos( 0 ),
    mnUnitSize( 0 ),
    mnNextUnitPos( 0 ),
    mbInProgress( false )
{
}

ScfProgressBar::~ScfProgressBar()
{
}

ScfProgressBar::ScfProgressSegment* ScfProgressBar::GetSegment( sal_uInt32 nSegment ) const
{
    if( nSegment == ~0UL )
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
            mpSysProgress.reset( new ScProgress( NULL, maText, mnTotalSize ) );

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

sal_uInt32 ScfProgressBar::AddSegment( sal_uInt32 nSize )
{
    DBG_ASSERT( !mbInProgress, "ScfProgressBar::AddSegment - already in progress mode" );
    if( !nSize )
        return ~0UL;

    maSegments.Append( new ScfProgressSegment( nSize ) );
    mnTotalSize += nSize;
    return maSegments.Count() - 1;
}

ScfProgressBar& ScfProgressBar::GetSegmentProgressBar( sal_uInt32 nSegment )
{
    ScfProgressSegment* pSegment = GetSegment( nSegment );
    if( pSegment && !pSegment->mpProgress.get() )
        pSegment->mpProgress.reset( new ScfProgressBar( this, pSegment ) );
    return pSegment ? *pSegment->mpProgress : *this;
}

void ScfProgressBar::ActivateSegment( sal_uInt32 nSegment )
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

