/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: regband.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_REGBAND_HXX
#define _SV_REGBAND_HXX

#include <vcl/sv.h>
#include <tools/poly.hxx>

/* =======================================================================

class ImplRegionBand

This class handles one y-band of the region. In this band may contain one
or more seprarations in x-direction. The y-Band do not contain any
separation after creation.

The separations are modified with basic clipping functions like Union and
Intersection - the Class will process the clipping for the actual band.

The actual separations may be checked by functions like IsInside or
IsOver.

======================================================================= */

// ------------------------
// - ImplRegionBand-Types -
// ------------------------

// element for the list with x-separations
struct ImplRegionBandSep
{
    ImplRegionBandSep*          mpNextSep;
    long                        mnXLeft;
    long                        mnXRight;
    BOOL                        mbRemoved;
};

enum LineType { LINE_ASCENDING, LINE_DESCENDING, LINE_HORIZONTAL };

// element for the list with x-separations
struct ImplRegionBandPoint
{
    ImplRegionBandPoint*        mpNextBandPoint;
    long                        mnX;
    long                        mnLineId;
    BOOL                        mbEndPoint;
    LineType                    meLineType;
};

// ------------------
// - ImplRegionBand -
// ------------------

class ImplRegionBand
{
public:
    ImplRegionBand*             mpNextBand;         // pointer to the next element of the list
    ImplRegionBand*             mpPrevBand;         // pointer to the previous element of the list (only used temporaery)
    ImplRegionBandSep*          mpFirstSep;         // root of the list with x-separations
    ImplRegionBandPoint*        mpFirstBandPoint;   // root of the list with lines
    long                        mnYTop;             // actual boundary of the band
    long                        mnYBottom;
    BOOL                        mbTouched;

                                // create y-band with boundaries
                                ImplRegionBand( long nYTop, long nYBottom );
                                // copy y-band with with all data
                                ImplRegionBand( const ImplRegionBand & theSourceBand );
                                ~ImplRegionBand();

    long                        GetXLeftBoundary() const;
    long                        GetXRightBoundary() const;

                                // combine overlapping bands
    BOOL                        OptimizeBand();

                                // generate separations from lines and process
                                // union with existing separations
    void                        ProcessPoints();
                                // insert point in the list for later processing
    BOOL                        InsertPoint( long nX, long nLineID,
                                             BOOL bEndPoint, LineType eLineType );

    void                        Union( long nXLeft, long nXRight );
    void                        Intersect( long nXLeft, long nXRight );
    void                        Exclude( long nXLeft, long nXRight );
    void                        XOr( long nXLeft, long nXRight );

    void                        MoveX( long nHorzMove );
    void                        ScaleX( double fHorzScale );

    BOOL                        IsInside( long nX );
    BOOL                        IsInside( long nLeft, long nRight );
    BOOL                        IsOver( long nLeft, long nRight );

    BOOL                        IsEmpty() const { return ((!mpFirstSep) && (!mpFirstBandPoint)); }

    BOOL                        operator==( const ImplRegionBand& rRegionBand ) const;
};

#endif  // _SV_REGBAND_HXX
