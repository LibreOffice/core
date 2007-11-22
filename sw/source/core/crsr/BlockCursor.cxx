/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BlockCursor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:29:34 $
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

