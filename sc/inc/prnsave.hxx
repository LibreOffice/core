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

#ifndef SC_PRNSAVE_HXX
#define SC_PRNSAVE_HXX

#include "address.hxx"
#include <tools/solar.h>

#include <vector>

class ScRange;

class ScPrintSaverTab
{
    typedef ::std::vector< ScRange > ScRangeVec;

    ScRangeVec  maPrintRanges;      ///< Array
    ScRange*    mpRepeatCol;        ///< single
    ScRange*    mpRepeatRow;        ///< single
    sal_Bool        mbEntireSheet;

public:
            ScPrintSaverTab();
            ~ScPrintSaverTab();

    void            SetAreas( const ScRangeVec& rRanges, sal_Bool bEntireSheet );
    void            SetRepeat( const ScRange* pCol, const ScRange* pRow );

    const ScRangeVec&   GetPrintRanges() const  { return maPrintRanges; }
    sal_Bool                IsEntireSheet() const   { return mbEntireSheet; }
    const ScRange*      GetRepeatCol() const    { return mpRepeatCol; }
    const ScRange*      GetRepeatRow() const    { return mpRepeatRow; }

    sal_Bool    operator==( const ScPrintSaverTab& rCmp ) const;
};

class ScPrintRangeSaver
{
    SCTAB               nTabCount;
    ScPrintSaverTab*    pData;      ///< Array

public:
            ScPrintRangeSaver( SCTAB nCount );
            ~ScPrintRangeSaver();

    SCTAB                   GetTabCount() const     { return nTabCount; }
    ScPrintSaverTab&        GetTabData(SCTAB nTab);
    const ScPrintSaverTab&  GetTabData(SCTAB nTab) const;

    sal_Bool    operator==( const ScPrintRangeSaver& rCmp ) const;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
