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

#ifndef INCLUDED_SW_INC_PVPRTDAT_HXX
#define INCLUDED_SW_INC_PVPRTDAT_HXX

#include <tools/solar.h>

class SwPagePreviewPrtData
{

    sal_Int32 nLeftSpace, nRightSpace, nTopSpace, nBottomSpace,
            nHorzSpace, nVertSpace;
    sal_uInt8 nRow, nCol;
    bool bLandscape : 1;
public:
    SwPagePreviewPrtData()
        : nLeftSpace(0), nRightSpace(0), nTopSpace(0), nBottomSpace(0),
            nHorzSpace(0), nVertSpace(0), nRow(1), nCol(1),
            bLandscape(false)
    {}

    sal_Int32 GetLeftSpace() const          { return nLeftSpace; }
    void SetLeftSpace( sal_Int32 n )        { nLeftSpace = n; }

    sal_Int32 GetRightSpace() const         { return nRightSpace; }
    void SetRightSpace( sal_Int32 n )       { nRightSpace = n; }

    sal_Int32 GetTopSpace() const           { return nTopSpace; }
    void SetTopSpace( sal_Int32 n )         { nTopSpace = n; }

    sal_Int32 GetBottomSpace() const        { return nBottomSpace; }
    void SetBottomSpace(sal_Int32 n )       { nBottomSpace = n; }

    sal_Int32 GetHorzSpace() const          { return nHorzSpace; }
    void SetHorzSpace( sal_Int32 n )        { nHorzSpace = n; }

    sal_Int32 GetVertSpace() const          { return nVertSpace; }
    void SetVertSpace( sal_Int32 n )        { nVertSpace = n; }

    sal_uInt8 GetRow() const                { return nRow; }
    void SetRow(sal_uInt8 n )               { nRow = n; }

    sal_uInt8 GetCol() const                { return nCol; }
    void SetCol( sal_uInt8 n )              { nCol = n; }

    bool GetLandscape() const               { return bLandscape; }
    void SetLandscape( bool b )         { bLandscape = b; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
