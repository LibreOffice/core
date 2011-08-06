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
#ifndef _SHDWCRSR_HXX
#define _SHDWCRSR_HXX


#include <tools/gen.hxx>
#include <tools/color.hxx>
#include  <limits.h>

class Window;

class SwShadowCursor
{
    Window* pWin;
    Color aCol;
    Point aOldPt;
    long nOldHeight;
    sal_uInt16 nOldMode;

    void DrawTri( const Point& rPt, long nHeight, sal_Bool bLeft );
    void DrawCrsr( const Point& rPt, long nHeight, sal_uInt16 nMode );

public:
    SwShadowCursor( Window& rWin, const Color& rCol )
        : pWin( &rWin ), aCol( rCol ), nOldHeight(0), nOldMode( USHRT_MAX ) {}
    ~SwShadowCursor();

    void SetPos( const Point& rPt, long nHeight, sal_uInt16 nMode );

    void Paint();

    const Point& GetPoint() const   { return aOldPt; }
    long GetHeight() const          { return nOldHeight; }
    sal_uInt16 GetMode() const          { return nOldMode; }

    Rectangle GetRect() const;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
