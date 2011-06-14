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

#ifndef IBLOCKCURSOR_HXX_INCLUDED
#define IBLOCKCURSOR_HXX_INCLUDED

class SwShellCrsr;
class Point;

 /** Access to the block cursor

    A block cursor contains a SwShellCrsr and additional information about
    the rectangle which has been created by pressing the mouse button and
    moving the mouse.
    This interface provides access to the SwShellCrsr and to start and end
    point of the mouse movement.
 */
 class IBlockCursor
 {
 public:
/** Access to the shell cursor

    @return SwShellCrsr& which represents the start and end position of the
    current block selection
*/
    virtual SwShellCrsr& getShellCrsr() = 0;

/** Defines the starting vertex of the block selection

    @param rPt
    rPt should contain the document coordinates of the mouse cursor when
    the block selection starts (MouseButtonDown)
*/
    virtual void setStartPoint( const Point &rPt ) = 0;

/** Defines the ending vertex of the block selection

    @param rPt
    rPt should contain the document coordinates of the mouse cursor when
    the block selection has started and the mouse has been moved (MouseMove)
*/
    virtual void setEndPoint( const Point &rPt ) = 0;

/** The document coordinates where the block selection has been started

    @return 0, if no start point has been set
*/
    virtual const Point* getStartPoint() const = 0;


/** The document coordinates where the block selection ends (at the moment)

    @return 0, if no end point has been set
*/
    virtual const Point* getEndPoint() const = 0;

/** Deletion of the mouse created rectangle

    When start and end points exist, the block cursor depends on this. If the
    cursor is moved by cursor keys (e.g. up/down, home/end) the mouse rectangle
    is obsolet and has to be deleted.
*/
    virtual void clearPoints() = 0;

/** Destructor of the block curosr interface
*/
    virtual ~IBlockCursor() {};
 };

#endif // IBLOCKCURSOR_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
