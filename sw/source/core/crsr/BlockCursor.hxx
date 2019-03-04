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
#ifndef INCLUDED_SW_SOURCE_CORE_CRSR_BLOCKCURSOR_HXX
#define INCLUDED_SW_SOURCE_CORE_CRSR_BLOCKCURSOR_HXX

#include <crsrsh.hxx>
#include <boost/optional.hpp>

class SwCursorShell;
struct SwPosition;
class Point;

 /** Access to the block cursor

    A block cursor contains a SwShellCursor and additional information about
    the rectangle which has been created by pressing the mouse button and
    moving the mouse.

    It's simply an aggregation of a SwShellCursor and a rectangle defined by
    a start and an end point.
*/
class SwBlockCursor
{
    SwShellCursor maCursor;
    boost::optional<Point> maStartPt;
    boost::optional<Point> maEndPt;

public:
    SwBlockCursor( const SwCursorShell& rCursorSh, const SwPosition &rPos ) :
        maCursor( rCursorSh, rPos ) {}
    /** Access to the shell cursor

        @return SwShellCursor& which represents the start and end position of the
        current block selection
    */
    SwShellCursor& getShellCursor();
    /** Defines the starting vertex of the block selection

        @param rPt
        rPt should contain the document coordinates of the mouse cursor when
        the block selection starts (MouseButtonDown)
    */
    void setStartPoint( const Point &rPt ) { maStartPt = rPt; }
    /** Defines the ending vertex of the block selection

        @param rPt
        rPt should contain the document coordinates of the mouse cursor when
        the block selection has started and the mouse has been moved (MouseMove)
    */
    void setEndPoint( const Point &rPt ) { maEndPt = rPt; }
    /** The document coordinates where the block selection has been started

        @return 0, if no start point has been set
    */
    boost::optional<Point> const & getStartPoint() const { return maStartPt; }
    /** The document coordinates where the block selection ends (at the moment)

        @return 0, if no end point has been set
    */
    boost::optional<Point> const & getEndPoint() const { return maEndPt; }
    /** Deletion of the mouse created rectangle

        When start and end points exist, the block cursor depends on this. If the
        cursor is moved by cursor keys (e.g. up/down, home/end) the mouse rectangle
        is obsolete and has to be deleted.
    */
    void clearPoints() { maStartPt.reset(); maEndPt.reset(); }
    ~SwBlockCursor();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
