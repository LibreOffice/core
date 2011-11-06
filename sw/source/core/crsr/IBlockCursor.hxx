/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

