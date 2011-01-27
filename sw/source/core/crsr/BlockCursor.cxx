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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
