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

#ifndef OOX_HELPER_PROGRESSBAR_HXX
#define OOX_HELPER_PROGRESSBAR_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/Reference.hxx>

namespace rtl { class OUString; }

namespace com { namespace sun { namespace star {
    namespace task { class XStatusIndicator; }
} } }

namespace oox {

// Interfaces =================================================================

/** Interface for progress bar classes.
 */
class IProgressBar
{
public:
    virtual             ~IProgressBar();

    /** Returns the current position of the progress bar.

        @return  Position of the progress bar, in the range from 0.0 (beginning
        of the progress bar) to 1.0 (end of the progress bar) inclusive.
     */
    virtual double      getPosition() const = 0;

    /** Sets the current position of the progress bar.

        @param fPosition  New position of the progress bar, in the range from
        0.0 (beginning of the progress bar) to 1.0 (end of the progress bar)
        inclusive.
     */
    virtual void        setPosition( double fPosition ) = 0;
};

typedef ::boost::shared_ptr< IProgressBar > IProgressBarRef;

// ----------------------------------------------------------------------------

class ISegmentProgressBar;
typedef ::boost::shared_ptr< ISegmentProgressBar > ISegmentProgressBarRef;

/** Interface for a segment in a progress bar, that is able to create sub
    segments from itself.
 */
class ISegmentProgressBar : public IProgressBar
{
public:
    virtual             ~ISegmentProgressBar();

    /** Returns the length that is still free for creating sub segments. */
    virtual double      getFreeLength() const = 0;

    /** Adds a new segment with the specified length. */
    virtual ISegmentProgressBarRef createSegment( double fLength ) = 0;
};

// ============================================================================
// ============================================================================

/** A simple progress bar.
 */
class ProgressBar : public IProgressBar
{
public:
    explicit            ProgressBar(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >& rxIndicator,
                            const ::rtl::OUString& rText );

    virtual             ~ProgressBar();

    /** Returns the current position of the progress bar. */
    virtual double      getPosition() const;
    /** Sets the current position of the progress bar. */
    virtual void        setPosition( double fPosition );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >
                        mxIndicator;
    double              mfPosition;
};

// ============================================================================

/** A progress bar containing several independent segments.
 */
class SegmentProgressBar : public ISegmentProgressBar
{
public:
    explicit            SegmentProgressBar(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >& rxIndicator,
                            const ::rtl::OUString& rText );

    /** Returns the current position of the progress bar segment. */
    virtual double      getPosition() const;
    /** Sets the current position of the progress bar segment. */
    virtual void        setPosition( double fPosition );

    /** Returns the length that is still free for creating sub segments. */
    virtual double      getFreeLength() const;
    /** Adds a new segment with the specified length. */
    virtual ISegmentProgressBarRef createSegment( double fLength );

private:
    ProgressBar         maProgress;
    double              mfFreeStart;
};

// ============================================================================

} // namespace oox

#endif

