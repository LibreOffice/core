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

#ifndef _PVPRTDAT_HXX
#define _PVPRTDAT_HXX


#include <tools/solar.h>

class SwPagePreViewPrtData
{
    sal_uLong nLeftSpace, nRightSpace, nTopSpace, nBottomSpace,
            nHorzSpace, nVertSpace;
    sal_uInt8 nRow, nCol;
    sal_Bool bLandscape : 1;
    sal_Bool bStretch : 1;
public:
    SwPagePreViewPrtData()
        : nLeftSpace(0), nRightSpace(0), nTopSpace(0), nBottomSpace(0),
            nHorzSpace(0), nVertSpace(0), nRow(1), nCol(1),
            bLandscape(0),bStretch(0)
    {}

    sal_uLong GetLeftSpace() const          { return nLeftSpace; }
    void SetLeftSpace( sal_uLong n )        { nLeftSpace = n; }

    sal_uLong GetRightSpace() const         { return nRightSpace; }
    void SetRightSpace( sal_uLong n )       { nRightSpace = n; }

    sal_uLong GetTopSpace() const           { return nTopSpace; }
    void SetTopSpace( sal_uLong n )         { nTopSpace = n; }

    sal_uLong GetBottomSpace() const        { return nBottomSpace; }
    void SetBottomSpace( sal_uLong n )      { nBottomSpace = n; }

    sal_uLong GetHorzSpace() const          { return nHorzSpace; }
    void SetHorzSpace( sal_uLong n )        { nHorzSpace = n; }

    sal_uLong GetVertSpace() const          { return nVertSpace; }
    void SetVertSpace( sal_uLong n )        { nVertSpace = n; }

    sal_uInt8 GetRow() const                { return nRow; }
    void SetRow(sal_uInt8 n )               { nRow = n; }

    sal_uInt8 GetCol() const                { return nCol; }
    void SetCol( sal_uInt8 n )              { nCol = n; }

    sal_Bool GetLandscape() const           { return bLandscape; }
    void SetLandscape( sal_Bool b )         { bLandscape = b; }
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
