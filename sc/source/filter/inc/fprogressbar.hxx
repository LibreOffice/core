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

#ifndef SC_FPROGRESSBAR_HXX
#define SC_FPROGRESSBAR_HXX

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "globstr.hrc"
#include "ftools.hxx"
#include "scdllapi.h"

class SfxObjectShell;
class ScProgress;

// ============================================================================

const sal_Int32 SCF_INV_SEGMENT = -1;

// ============================================================================

/** Progress bar for complex progress representation.

    The progress bar contains one or more segments, each with customable
    size. Each segment is represented by a unique identifier. While showing the
    progress bar, several segments can be started simultaneously. The progress
    bar displays the sum of all started segments on screen.

    It is possible to create a full featured ScfProgressBar object from
    any segment. This sub progress bar works only on that parent segment, with
    the effect, that if the sub progress bar reaches 100%, the parent segment is
    filled completely.

    After adding segments, the progress bar has to be activated. In this step the
    total size of all segments is calculated. Therefore it is not possible to add
    more segments from here.

    If a sub progress bar is created from a segment, and the main progress bar
    has been started (but not the sub progress bar), it is still possible to add
    segments to the sub progress bar. It is not allowed to get the sub progress bar
    of a started segment. And it is not allowed to modify the segment containing
    a sub progress bar directly.

    Following a few code examples, how to use the progress bar.

    Example 1: Simple progress bar (see also ScfSimpleProgressBar below).

        ScfProgressBar aProgress( ... );
        sal_Int32 nSeg = aProgress.AddSegment( 50 );        // segment with 50 steps (1 step = 2%)

        aProgress.ActivateSegment( nSeg );                  // start segment nSeg
        aProgress.Progress();                               // 0->1; display: 2%
        aProgress.ProgressAbs( 9 );                         // 1->9; display: 18%

    Example 2: Progress bar with 2 segments.

        ScfProgressBar aProgress( ... );
        sal_Int32 nSeg1 = aProgress.AddSegment( 70 );       // segment with 70 steps
        sal_Int32 nSeg2 = aProgress.AddSegment( 30 );       // segment with 30 steps
                                                            // both segments: 100 steps (1 step = 1%)

        aProgress.ActivateSegment( nSeg1 );                 // start first segment
        aProgress.Progress();                               // 0->1, display: 1%
        aProgress.Progress( 2 );                            // 1->3, display: 3%
        aProgress.ActivateSegment( nSeg2 );                 // start second segment
        aProgress.Progress( 5 );                            // 0->5, display: 8% (5+3 steps)
        aProgress.ActivateSegment( nSeg1 );                 // continue with first segment
        aProgress.Progress();                               // 3->4, display: 9% (5+4 steps)

    Example 3: Progress bar with 2 segments, one contains a sub progress bar.

        ScfProgressBar aProgress( ... );
        sal_Int32 nSeg1 = aProgress.AddSegment( 75 );       // segment with 75 steps
        sal_Int32 nSeg2 = aProgress.AddSegment( 25 );       // segment with 25 steps
                                                            // both segments: 100 steps (1 step = 1%)

        aProgress.ActivateSegment( nSeg1 );                 // start first segment
        aProgress.Progress();                               // 0->1, display: 1%

        ScfProgressBar& rSubProgress = aProgress.GetSegmentProgressBar( nSeg2 );
                                                            // sub progress bar from second segment
        sal_Int32 nSubSeg = rSubProgress.AddSegment( 5 );   // 5 steps, mapped to second segment
                                                            // => 1 step = 5 steps in parent = 5%

        rSubProgress.ActivateSegment( nSubSeg );            // start the segment (auto activate parent segment)
        rSubProgress.Progress();                            // 0->1 (0->5 in parent); display: 6% (1+5)

        // not allowed (second segment active):   aProgress.Progress();
        // not allowed (first segment not empty): aProgress.GetSegmentProgressBar( nSeg1 );
 */
class ScfProgressBar : private boost::noncopyable
{
public:
    explicit            ScfProgressBar( SfxObjectShell* pDocShell, const String& rText );
    explicit            ScfProgressBar( SfxObjectShell* pDocShell, sal_uInt16 nResId );
    virtual             ~ScfProgressBar();

    /** Adds a new segment to the progress bar.
        @return  the identifier of the segment. */
    sal_Int32           AddSegment( sal_Size nSize );
    /** Returns a complete progress bar for the specified segment.
        @descr  The progress bar can be used to create sub segments inside of the
        segment. Do not delete it (done by root progress bar)!
        @return  A reference to an ScfProgressBar connected to the segment. */
    ScfProgressBar&     GetSegmentProgressBar( sal_Int32 nSegment );

    /** Returns true, if any progress segment has been started. */
    inline bool         IsStarted() const { return mbInProgress; }
    /** Returns true, if the current progress segment is already full. */
    bool                IsFull() const;

    /** Starts the progress bar or activates another segment. */
    void                ActivateSegment( sal_Int32 nSegment );
    /** Starts the progress bar (with first segment). */
    inline void         Activate() { ActivateSegment( 0 ); }
    /** Set current segment to the specified absolute position. */
    void                ProgressAbs( sal_Size nPos );
    /** Increase current segment by the passed value. */
    void                Progress( sal_Size nDelta = 1 );

private:
    struct ScfProgressSegment;

    /** Used to create sub progress bars. */
    explicit            ScfProgressBar(
                            ScfProgressBar& rParProgress,
                            ScfProgressSegment* pParSegment );

    /** Initializes all members on construction. */
    void                Init( SfxObjectShell* pDocShell );

    /** Returns the segment specified by list index. */
    ScfProgressSegment* GetSegment( sal_Int32 nSegment );
    /** Activates progress bar and sets current segment. */
    void                SetCurrSegment( ScfProgressSegment* pSegment );
    /** Increases mnTotalPos and calls the system progress bar. */
    void                IncreaseProgressBar( sal_Size nDelta );

private:
    /** Contains all data of a segment of the progress bar. */
    struct ScfProgressSegment
    {
        typedef ::std::auto_ptr< ScfProgressBar > ScfProgressBarPtr;

        ScfProgressBarPtr   mxProgress;     /// Pointer to sub progress bar for this segment.
        sal_Size            mnSize;         /// Size of this segment.
        sal_Size            mnPos;          /// Current position of this segment.

        explicit            ScfProgressSegment( sal_Size nSize );
                            ~ScfProgressSegment();
    };

    typedef ::std::auto_ptr< ScProgress >           ScProgressPtr;
    typedef boost::ptr_vector< ScfProgressSegment > ScfSegmentList;

    ScfSegmentList      maSegments;         /// List of progress segments.
    String              maText;             /// UI string for system progress.

    ScProgressPtr       mxSysProgress;      /// System progress bar.
    SfxObjectShell*     mpDocShell;         /// The document shell for the progress bar.
    ScfProgressBar*     mpParentProgress;   /// Parent progress bar, if this is a segment progress bar.
    ScfProgressSegment* mpParentSegment;    /// Parent segment, if this is a segment progress bar.
    ScfProgressSegment* mpCurrSegment;      /// Current segment for progress.

    sal_Size            mnTotalSize;        /// Total size of all segments.
    sal_Size            mnTotalPos;         /// Sum of positions of all segments.
    sal_Size            mnUnitSize;         /// Size between two calls of system progress.
    sal_Size            mnNextUnitPos;      /// Limit for next system progress call.
    sal_Size            mnSysProgressScale; /// Additionally scaling factor for system progress.
    bool                mbInProgress;       /// true = progress bar started.
};

// ============================================================================

/** A simplified progress bar with only one segment. */
class ScfSimpleProgressBar
{
public:
    explicit            ScfSimpleProgressBar( sal_Size nSize, SfxObjectShell* pDocShell, const String& rText );
    explicit            ScfSimpleProgressBar( sal_Size nSize, SfxObjectShell* pDocShell, sal_uInt16 nResId );

    /** Set progress bar to the specified position. */
    inline void         ProgressAbs( sal_Size nPos ) { maProgress.ProgressAbs( nPos ); }
    /** Increase progress bar by 1. */
    inline void         Progress( sal_Size nDelta = 1 ) { maProgress.Progress( nDelta ); }

private:
    /** Initializes and starts the progress bar. */
    void                Init( sal_Size nSize );

private:
    ScfProgressBar      maProgress;     /// The used progress bar.
};

// ============================================================================

/** A simplified progress bar based on the stream position of an existing stream. */
class ScfStreamProgressBar
{
public:
    explicit            ScfStreamProgressBar( SvStream& rStrm, SfxObjectShell* pDocShell, sal_uInt16 nResId = STR_LOAD_DOC );

    /** Sets the progress bar to the current stream position. */
    void                Progress();

private:
    /** Initializes and starts the progress bar. */
    void                Init( SfxObjectShell* pDocShell, const String& rText );

private:
    typedef ::std::auto_ptr< ScfSimpleProgressBar > ScfSimpleProgressBarPtr;

    ScfSimpleProgressBarPtr mxProgress; /// The used progress bar.
    SvStream&           mrStrm;         /// The used stream.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
