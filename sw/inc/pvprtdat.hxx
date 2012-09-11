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

#ifndef _PVPRTDAT_HXX
#define _PVPRTDAT_HXX


#include <tools/solar.h>

class SwPagePreViewPrtData
{
    sal_uLong nLeftSpace, nRightSpace, nTopSpace, nBottomSpace,
            nHorzSpace, nVertSpace;
    sal_uInt8 nRow, nCol;
    sal_Bool bLandscape : 1;
public:
    SwPagePreViewPrtData()
        : nLeftSpace(0), nRightSpace(0), nTopSpace(0), nBottomSpace(0),
            nHorzSpace(0), nVertSpace(0), nRow(1), nCol(1),
            bLandscape(0)
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
