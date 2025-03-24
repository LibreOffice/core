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

#pragma once

#include <tools/long.hxx>

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
    tools::Long                        mnXLeft;
    tools::Long                        mnXRight;
    bool                        mbRemoved;
};

enum class LineType { Ascending, Descending };

// element for the list with x-separations
struct ImplRegionBandPoint
{
    ImplRegionBandPoint*        mpNextBandPoint;
    tools::Long                        mnX;
    tools::Long                        mnLineId;
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
    tools::Long                        mnYTop;             // actual boundary of the band
    tools::Long                        mnYBottom;

    bool                        mbTouched : 1;

                                // create y-band with boundaries
                                ImplRegionBand( tools::Long nYTop, tools::Long nYBottom );
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

    tools::Long                        GetXLeftBoundary() const;
    tools::Long                        GetXRightBoundary() const;

                                // combine overlapping bands
    void                        OptimizeBand();

                                // generate separations from lines and process
                                // union with existing separations
    void                        ProcessPoints();
                                // insert point in the list for later processing
    bool                        InsertPoint( tools::Long nX, tools::Long nLineID,
                                             bool bEndPoint, LineType eLineType );

    void                        Union( tools::Long nXLeft, tools::Long nXRight );
    void                        Intersect( tools::Long nXLeft, tools::Long nXRight );
    void                        Exclude( tools::Long nXLeft, tools::Long nXRight );
    void                        XOr( tools::Long nXLeft, tools::Long nXRight );

    void                        MoveX( tools::Long nHorzMove );
    void                        ScaleX( double fHorzScale );

    bool                        Contains( tools::Long nX );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
