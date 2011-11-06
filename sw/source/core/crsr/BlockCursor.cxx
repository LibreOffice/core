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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <IBlockCursor.hxx>
#include <viscrs.hxx>
#include "BlockCursor.hxx"

/** The implementation of the block cursor interface

    It's simply an aggregation of a SwShellCrsr and a rectangle defined by
    a start and an end point.
*/
class SwBlockCursor : public IBlockCursor
{
    SwShellCrsr aCursor;
    Point *pStartPt;
    Point *pEndPt;
public:
    SwBlockCursor( const SwCrsrShell& rCrsrSh, const SwPosition &rPos ) :
        aCursor( rCrsrSh, rPos ), pStartPt(0), pEndPt(0) {}
    virtual SwShellCrsr& getShellCrsr();
    virtual void setStartPoint( const Point &rPt );
    virtual void setEndPoint( const Point &rPt );
    virtual const Point* getStartPoint() const;
    virtual const Point* getEndPoint() const;
    virtual void clearPoints();
    virtual ~SwBlockCursor();
};

SwBlockCursor::~SwBlockCursor()
{
    delete pStartPt;
    delete pEndPt;
}

SwShellCrsr& SwBlockCursor::getShellCrsr()
{
    return aCursor;
}

void SwBlockCursor::setStartPoint( const Point &rPt )
{
    if( pStartPt )
        *pStartPt = rPt;
    else
        pStartPt = new Point( rPt );
}

void SwBlockCursor::setEndPoint( const Point &rPt )
{
    if( pEndPt )
        *pEndPt = rPt;
    else
        pEndPt = new Point( rPt );
}

const Point* SwBlockCursor::getStartPoint() const
{
    return pStartPt;
}

const Point* SwBlockCursor::getEndPoint() const
{
    return pEndPt;
}

void SwBlockCursor::clearPoints()
{
    delete pStartPt;
    delete pEndPt;
    pStartPt = 0;
    pEndPt = 0;
}

IBlockCursor *createBlockCursor( const SwCrsrShell& rCrsrSh, const SwPosition &rPos )
{
    return new SwBlockCursor( rCrsrSh, rPos );
}

