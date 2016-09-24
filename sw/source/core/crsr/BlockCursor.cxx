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

#include <viscrs.hxx>
#include "BlockCursor.hxx"

SwBlockCursor::~SwBlockCursor()
{
    delete pStartPt;
    delete pEndPt;
}

SwShellCursor& SwBlockCursor::getShellCursor()
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
    pStartPt = nullptr;
    pEndPt = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
