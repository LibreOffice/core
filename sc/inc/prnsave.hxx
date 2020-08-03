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

#pragma once

#include "address.hxx"
#include <vector>
#include <memory>

class ScPrintSaverTab
{
    typedef ::std::vector< ScRange > ScRangeVec;

    ScRangeVec  maPrintRanges;      ///< Array
    std::unique_ptr<ScRange> mpRepeatCol;        ///< single
    std::unique_ptr<ScRange> mpRepeatRow;        ///< single
    bool        mbEntireSheet;

public:
            ScPrintSaverTab();
            ~ScPrintSaverTab();

    void            SetAreas( const ScRangeVec& rRanges, bool bEntireSheet );
    void            SetRepeat( const ScRange* pCol, const ScRange* pRow );

    const ScRangeVec&   GetPrintRanges() const  { return maPrintRanges; }
    bool                IsEntireSheet() const   { return mbEntireSheet; }
    const ScRange*      GetRepeatCol() const    { return mpRepeatCol.get(); }
    const ScRange*      GetRepeatRow() const    { return mpRepeatRow.get(); }

    bool    operator==( const ScPrintSaverTab& rCmp ) const;
};

class ScPrintRangeSaver
{
    SCTAB                               nTabCount;
    std::unique_ptr<ScPrintSaverTab[]>  pData;

public:
            ScPrintRangeSaver( SCTAB nCount );
            ~ScPrintRangeSaver();

    SCTAB                   GetTabCount() const     { return nTabCount; }
    ScPrintSaverTab&        GetTabData(SCTAB nTab);
    const ScPrintSaverTab&  GetTabData(SCTAB nTab) const;

    bool    operator==( const ScPrintRangeSaver& rCmp ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
