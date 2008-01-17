/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progressbar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:59 $
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

#include "oox/helper/progressbar.hxx"
#include <com/sun/star/task/XStatusIndicator.hpp>
#include "oox/helper/helper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::task::XStatusIndicator;

namespace oox {

// ============================================================================

namespace {

const sal_Int32 PROGRESS_RANGE      = 1000000;

} // namespace

// ============================================================================

IProgressBar::~IProgressBar()
{
}

// ----------------------------------------------------------------------------

ISegmentProgressBar::~ISegmentProgressBar()
{
}

// ============================================================================
// ============================================================================

ProgressBar::ProgressBar( const Reference< XStatusIndicator >& rxIndicator, const OUString& rText ) :
    mxIndicator( rxIndicator ),
    mfPosition( 0 )
{
    if( mxIndicator.is() )
        mxIndicator->start( rText, PROGRESS_RANGE );
}

ProgressBar::~ProgressBar()
{
    if( mxIndicator.is() )
        mxIndicator->end();
}

double ProgressBar::getPosition() const
{
    return mfPosition;
}

void ProgressBar::setPosition( double fPosition )
{
    OSL_ENSURE( (mfPosition <= fPosition) && (fPosition <= 1.0), "ProgressBar::setPosition - invalid position" );
    mfPosition = getLimitedValue< double >( fPosition, mfPosition, 1.0 );
    if( mxIndicator.is() )
        mxIndicator->setValue( static_cast< sal_Int32 >( mfPosition * PROGRESS_RANGE ) );
}

// ============================================================================

namespace prv {

class SubSegment : public ISegmentProgressBar
{
public:
    explicit            SubSegment( IProgressBar& rParentProgress, double fStartPos, double fLength );

    virtual double      getPosition() const;
    virtual void        setPosition( double fPosition );

    virtual double      getFreeLength() const;
    virtual ISegmentProgressBarRef createSegment( double fLength );

private:
    IProgressBar&       mrParentProgress;
    double              mfStartPos;
    double              mfLength;
    double              mfPosition;
    double              mfFreeStart;
};

// ----------------------------------------------------------------------------

SubSegment::SubSegment( IProgressBar& rParentProgress, double fStartPos, double fLength ) :
    mrParentProgress( rParentProgress ),
    mfStartPos( fStartPos ),
    mfLength( fLength ),
    mfPosition( 0.0 ),
    mfFreeStart( 0.0 )
{
}

double SubSegment::getPosition() const
{
    return mfPosition;
}

void SubSegment::setPosition( double fPosition )
{
    OSL_ENSURE( (mfPosition <= fPosition) && (fPosition <= 1.0), "SubSegment::setPosition - invalid position" );
    mfPosition = getLimitedValue< double >( fPosition, mfPosition, 1.0 );
    mrParentProgress.setPosition( mfStartPos + mfPosition * mfLength );
}

double SubSegment::getFreeLength() const
{
    return 1.0 - mfFreeStart;
}

ISegmentProgressBarRef SubSegment::createSegment( double fLength )
{
    OSL_ENSURE( (0.0 < fLength) && (fLength <= getFreeLength()), "SubSegment::createSegment - invalid length" );
    fLength = getLimitedValue< double >( fLength, 0.0, getFreeLength() );
    ISegmentProgressBarRef xSegment( new prv::SubSegment( *this, mfFreeStart, fLength ) );
    mfFreeStart += fLength;
    return xSegment;
}

} // namespace prv

// ============================================================================

SegmentProgressBar::SegmentProgressBar( const Reference< XStatusIndicator >& rxIndicator, const OUString& rText ) :
    maProgress( rxIndicator, rText ),
    mfFreeStart( 0.0 )
{
}

double SegmentProgressBar::getPosition() const
{
    return maProgress.getPosition();
}

void SegmentProgressBar::setPosition( double fPosition )
{
    maProgress.setPosition( fPosition );
}

double SegmentProgressBar::getFreeLength() const
{
    return 1.0 - mfFreeStart;
}

ISegmentProgressBarRef SegmentProgressBar::createSegment( double fLength )
{
    OSL_ENSURE( (0.0 < fLength) && (fLength <= getFreeLength()), "SegmentProgressBar::createSegment - invalid length" );
    fLength = getLimitedValue< double >( fLength, 0.0, getFreeLength() );
    ISegmentProgressBarRef xSegment( new prv::SubSegment( maProgress, mfFreeStart, fLength ) );
    mfFreeStart += fLength;
    return xSegment;
}

// ============================================================================

} // namespace oox

