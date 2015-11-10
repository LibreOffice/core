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

class SwCrsrShell;
struct SwPosition;
class Point;

 /** Access to the block cursor

    A block cursor contains a SwShellCrsr and additional information about
    the rectangle which has been created by pressing the mouse button and
    moving the mouse.

    It's simply an aggregation of a SwShellCrsr and a rectangle defined by
    a start and an end point.
*/
class SwBlockCursor
{
    SwShellCrsr aCursor;
    Point *pStartPt;
    Point *pEndPt;

public:
    SwBlockCursor( const SwCrsrShell& rCrsrSh, const SwPosition &rPos ) :
        aCursor( rCrsrSh, rPos ), pStartPt(nullptr), pEndPt(nullptr) {}
    /** Access to the shell cursor

        @return SwShellCrsr& which represents the start and end position of the
        current block selection
    */
    SwShellCrsr& getShellCrsr();
    /** Defines the starting vertex of the block selection

        @param rPt
        rPt should contain the document coordinates of the mouse cursor when
        the block selection starts (MouseButtonDown)
    */
    void setStartPoint( const Point &rPt );
    /** Defines the ending vertex of the block selection

        @param rPt
        rPt should contain the document coordinates of the mouse cursor when
        the block selection has started and the mouse has been moved (MouseMove)
    */
    void setEndPoint( const Point &rPt );
    /** The document coordinates where the block selection has been started

        @return 0, if no start point has been set
    */
    const Point* getStartPoint() const;
    /** The document coordinates where the block selection ends (at the moment)

        @return 0, if no end point has been set
    */
    const Point* getEndPoint() const;
    /** Deletion of the mouse created rectangle

        When start and end points exist, the block cursor depends on this. If the
        cursor is moved by cursor keys (e.g. up/down, home/end) the mouse rectangle
        is obsolete and has to be deleted.
    */
    void clearPoints();
    ~SwBlockCursor();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
