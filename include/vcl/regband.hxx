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

#ifndef _SV_REGBAND_HXX
#define _SV_REGBAND_HXX

#include <tools/solar.h>
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
    sal_Bool                        mbRemoved;
};

enum LineType { LINE_ASCENDING, LINE_DESCENDING, LINE_HORIZONTAL };

// element for the list with x-separations
struct ImplRegionBandPoint
{
    ImplRegionBandPoint*        mpNextBandPoint;
    long                        mnX;
    long                        mnLineId;
    sal_Bool                        mbEndPoint;
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
    sal_Bool                        mbTouched;

                                // create y-band with boundaries
                                ImplRegionBand( long nYTop, long nYBottom );
                                /** copy y-band with with all data
                                    @param theSourceBand
                                        The new ImplRegionBand object will
                                        be a copy of this band.
                                    @param bIgnorePoints
                                        When <TRUE/> (the default) the
                                        band points pointed to by
                                        mpFirstBandPoint are not copied.
                                        When <FALSE/> they are copied.
                                        You need the points when you are
                                        planning to call ProcessPoints()
                                        later on.
                                */
                                ImplRegionBand( const ImplRegionBand & theSourceBand,
                                                const bool bIgnorePoints = true);
                                ~ImplRegionBand();

    long                        GetXLeftBoundary() const;
    long                        GetXRightBoundary() const;

                                // combine overlapping bands
    sal_Bool                        OptimizeBand();

                                // generate separations from lines and process
                                // union with existing separations
    void                        ProcessPoints();
                                // insert point in the list for later processing
    sal_Bool                        InsertPoint( long nX, long nLineID,
                                             sal_Bool bEndPoint, LineType eLineType );

    void                        Union( long nXLeft, long nXRight );
    void                        Intersect( long nXLeft, long nXRight );
    void                        Exclude( long nXLeft, long nXRight );
    void                        XOr( long nXLeft, long nXRight );

    void                        MoveX( long nHorzMove );
    void                        ScaleX( double fHorzScale );

    sal_Bool                        IsInside( long nX );

    sal_Bool                        IsEmpty() const { return ((!mpFirstSep) && (!mpFirstBandPoint)); }

    sal_Bool                        operator==( const ImplRegionBand& rRegionBand ) const;

    /** Split the called band at the given vertical coordinate.  After the
        split the called band will cover the upper part not including nY.
        The new band will cover the lower part including nY.
        @param nY
            The band is split at this y coordinate.  The new, lower band
            will include this very value.
        @return
            Returns the new, lower band.
    */
    ImplRegionBand*             SplitBand (const sal_Int32 nY);
};

#endif  // _SV_REGBAND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
