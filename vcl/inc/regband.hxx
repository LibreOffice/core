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

#ifndef INCLUDED_VCL_INC_REGBAND_HXX
#define INCLUDED_VCL_INC_REGBAND_HXX

/*

class ImplRegionBand

This class handles one y-band of the region. In this band may contain one
or more separations in x-direction. The y-Band do not contain any
separation after creation.

The separations are modified with basic clipping functions like Union and
Intersection - the Class will process the clipping for the actual band.

*/

// element for the list with x-separations
struct ImplRegionBandSep
{
    ImplRegionBandSep*          mpNextSep;
    sal_Int32                   mnXLeft;
    sal_Int32                   mnXRight;
    bool                        mbRemoved;
};

enum LineType { LINE_ASCENDING, LINE_DESCENDING, LINE_HORIZONTAL };

// element for the list with x-separations
struct ImplRegionBandPoint
{
    ImplRegionBandPoint*        mpNextBandPoint;
    sal_Int32                   mnX;
    sal_Int32                   mnLineId;
    bool                        mbEndPoint;
    LineType                    meLineType;
};

class ImplRegionBand
{
public:
    ImplRegionBand*             mpNextBand;         // pointer to the next element of the list
    ImplRegionBand*             mpPrevBand;         // pointer to the previous element of the list (only used temporarily)
    ImplRegionBandSep*          mpFirstSep;         // root of the list with x-separations
    ImplRegionBandPoint*        mpFirstBandPoint;   // root of the list with lines
    sal_Int32                   mnYTop;             // actual boundary of the band
    sal_Int32                   mnYBottom;

    bool                        mbTouched : 1;

                                // create y-band with boundaries
                                ImplRegionBand( sal_Int32 nYTop, sal_Int32 nYBottom );
                                /** copy y-band with all data
                                    @param theSourceBand
                                        The new ImplRegionBand object will
                                        be a copy of this band.
                                    @param bIgnorePoints
                                        When true (the default) the
                                        band points pointed to by
                                        mpFirstBandPoint are not copied.
                                        When false they are copied.
                                        You need the points when you are
                                        planning to call ProcessPoints()
                                        later on.
                                */
                                ImplRegionBand( const ImplRegionBand & theSourceBand,
                                                const bool bIgnorePoints = true);
                                ~ImplRegionBand();

    sal_Int32                   GetXLeftBoundary() const;
    sal_Int32                   GetXRightBoundary() const;

                                // combine overlapping bands
    void                        OptimizeBand();

                                // generate separations from lines and process
                                // union with existing separations
    void                        ProcessPoints();
                                // insert point in the list for later processing
    bool                        InsertPoint( sal_Int32 nX, sal_Int32 nLineID,
                                             bool bEndPoint, LineType eLineType );

    void                        Union( sal_Int32 nXLeft, sal_Int32 nXRight );
    void                        Intersect( sal_Int32 nXLeft, sal_Int32 nXRight );
    void                        Exclude( sal_Int32 nXLeft, sal_Int32 nXRight );
    void                        XOr( sal_Int32 nXLeft, sal_Int32 nXRight );

    void                        MoveX( sal_Int32 nHorzMove );
    void                        ScaleX( double fHorzScale );

    bool                        IsInside( sal_Int32 nX );

    bool                        IsEmpty() const { return ((!mpFirstSep) && (!mpFirstBandPoint)); }

    bool                        operator==( const ImplRegionBand& rRegionBand ) const;

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

#endif // INCLUDED_VCL_INC_REGBAND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
