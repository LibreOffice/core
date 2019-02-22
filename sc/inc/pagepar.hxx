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

#ifndef INCLUDED_SC_INC_PAGEPAR_HXX
#define INCLUDED_SC_INC_PAGEPAR_HXX

#include "address.hxx"

struct ScPageTableParam
{
    bool        bCellContent;
    bool        bNotes;
    bool        bGrid;
    bool        bHeaders;
    bool        bCharts;
    bool        bObjects;
    bool        bDrawings;
    bool        bFormulas;
    bool        bNullVals;
    bool        bTopDown;
    bool        bLeftRight;
    bool        bSkipEmpty;
    bool        bScaleNone;
    bool        bScaleAll;
    bool        bScaleTo;
    bool        bScalePageNum;
    bool        bForceBreaks;
    sal_uInt16  nScaleAll;
    sal_uInt16  nScaleWidth;
    sal_uInt16  nScaleHeight;
    sal_uInt16  nScalePageNum;
    sal_uInt16  nFirstPageNo;

    ScPageTableParam();
    ~ScPageTableParam();

    void                Reset       ();
};

struct ScPageAreaParam
{
    bool    bPrintArea;
    bool    bRepeatRow;
    bool    bRepeatCol;
    ScRange aPrintArea;

    ScPageAreaParam();
    ~ScPageAreaParam();

    void                Reset       ();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
