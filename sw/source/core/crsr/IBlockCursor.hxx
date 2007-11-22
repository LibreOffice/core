/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IBlockCursor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:30:00 $
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

