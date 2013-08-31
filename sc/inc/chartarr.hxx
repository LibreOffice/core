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

#ifndef SC_CHARTARR_HXX
#define SC_CHARTARR_HXX

#include "rangelst.hxx"
#include "chartpos.hxx"

#include <boost/ptr_container/ptr_vector.hpp>

class ScDocument;

// ScMemChart is a stripped-down SchMemChart from old chart,
// used only to transport a rectangular data array for the UNO API,
// contains only column/row header text and data values.

class ScMemChart
{
    short           nRowCnt;
    short           nColCnt;
    double*         pData;
    OUString* pColText;
    OUString* pRowText;

    ScMemChart(const ScMemChart& rMemChart);      // not implemented

public:
    ScMemChart(short nCols, short nRows);
    ~ScMemChart();

    short GetColCount() const { return nColCnt; }
    short GetRowCount() const { return nRowCnt; }
    const OUString& GetColText(short nCol) const { return pColText[nCol]; }
    const OUString& GetRowText(short nRow) const { return pRowText[nRow]; }
    double GetData(short nCol, short nRow) const { return pData[nCol * nRowCnt + nRow]; }
    void SetData(short nCol, short nRow, const double& rVal) { pData[nCol * nRowCnt + nRow] = rVal; }
    void SetColText(short nCol, const OUString& rText) { pColText[nCol] = rText; }
    void SetRowText(short nRow, const OUString& rText) { pRowText[nRow] = rText; }
};

class SC_DLLPUBLIC ScChartArray             // only parameter-struct
{
    OUString aName;
    ScDocument* pDocument;
    ScChartPositioner aPositioner;
    bool        bValid;             // for creation out of SchMemChart

private:
    ScMemChart* CreateMemChartSingle();
    ScMemChart* CreateMemChartMulti();
public:
    ScChartArray( ScDocument* pDoc, SCTAB nTab,
                  SCCOL nStartColP, SCROW nStartRowP,
                  SCCOL nEndColP, SCROW nEndRowP,
                  const OUString& rChartName );
    ScChartArray( ScDocument* pDoc, const ScRangeListRef& rRangeList,
                  const OUString& rChartName );
    ScChartArray( const ScChartArray& rArr );
    ~ScChartArray();

    const ScRangeListRef&   GetRangeList() const { return aPositioner.GetRangeList(); }
    void    SetRangeList( const ScRangeListRef& rNew ) { aPositioner.SetRangeList(rNew); }
    void    SetRangeList( const ScRange& rNew ) { aPositioner.SetRangeList(rNew); }
    const   ScChartPositionMap* GetPositionMap() { return aPositioner.GetPositionMap(); }

    void    SetHeaders(bool bCol, bool bRow) { aPositioner.SetHeaders(bCol, bRow); }
    bool    HasColHeaders() const { return aPositioner.HasColHeaders(); }
    bool    HasRowHeaders() const { return aPositioner.HasRowHeaders(); }
    bool IsValid() const { return bValid; }
    void SetName(const OUString& rNew) { aName = rNew; }
    const OUString& GetName() const { return aName; }

    bool operator==(const ScChartArray& rCmp) const;

    ScMemChart* CreateMemChart();
};

class ScChartCollection
{
    typedef ::boost::ptr_vector<ScChartArray> DataType;
    DataType maData;
public:
    ScChartCollection();
    ScChartCollection(const ScChartCollection& rColl);

    SC_DLLPUBLIC void push_back(ScChartArray* p);
    void clear();
    size_t size() const;
    bool empty() const;
    ScChartArray* operator[](size_t nIndex);
    const ScChartArray* operator[](size_t nIndex) const;

    bool operator==(const ScChartCollection& rCmp) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
