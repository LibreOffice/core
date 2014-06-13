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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8TABLEINFO_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8TABLEINFO_HXX
#include <boost/unordered_map.hpp>
#include <string>
#include <map>
#include <set>
#include <functional>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <sal/types.h>
#include <swrect.hxx>

class SwTable;
class SwTableLine;
class SwTableBox;
class SwNode;
class AttributeOutputBase;

namespace ww8
{
const unsigned int MAXTABLECELLS = 63;

class WW8TableNodeInfo;
typedef boost::shared_ptr<SwRect> SwRectPtr;
typedef ::std::vector<const SwTableBox *> TableBoxVector;
typedef boost::shared_ptr<TableBoxVector> TableBoxVectorPtr;
typedef ::std::vector<sal_uInt32> GridCols;
typedef boost::shared_ptr<GridCols> GridColsPtr;
typedef ::std::vector<sal_uInt32> RowSpans;
typedef boost::shared_ptr<RowSpans> RowSpansPtr;
typedef ::std::vector<sal_uInt32> Widths;
typedef boost::shared_ptr<Widths> WidthsPtr;

class WW8TableNodeInfoInner
{
    WW8TableNodeInfo * mpParent;
    sal_uInt32 mnDepth;
    sal_uInt32 mnCell;
    sal_uInt32 mnRow;
    sal_uInt32 mnShadowsBefore;
    sal_uInt32 mnShadowsAfter;
    bool mbEndOfLine;
    bool mbEndOfCell;
    bool mbFirstInTable;
    bool mbVertMerge;
    const SwTableBox * mpTableBox;
    const SwTable * mpTable;
    SwRect maRect;

public:
    typedef boost::shared_ptr<WW8TableNodeInfoInner> Pointer_t;

    WW8TableNodeInfoInner(WW8TableNodeInfo * pParent);
    ~WW8TableNodeInfoInner();

    void setDepth(sal_uInt32 nDepth);
    void setCell(sal_uInt32 nCell);
    void setRow(sal_uInt32 nRow);
    void setShadowsBefore(sal_uInt32 nShadowsBefore);
    void setShadowsAfter(sal_uInt32 nShadowsAfter);
    void setEndOfLine(bool bEndOfLine);
    void setEndOfCell(bool bEndOfCell);
    void setFirstInTable(bool bFirstInTable);
    void setVertMerge(bool bVertMErge);
    void setTableBox(const SwTableBox * pTableBox);
    void setTable(const SwTable * pTable);
    void setRect(const SwRect & rRect);

    sal_uInt32 getDepth() const { return mnDepth;}
    sal_uInt32 getCell() const { return mnCell;}
    sal_uInt32 getRow() const { return mnRow;}
    sal_uInt32 getShadowsBefore() const { return mnShadowsBefore;}
    sal_uInt32 getShadowsAfter() const { return mnShadowsAfter;}
    bool isEndOfCell() const { return mbEndOfCell;}
    bool isEndOfLine() const { return mbEndOfLine;}
    bool isFirstInTable() const { return mbFirstInTable;}
    bool isVertMerge() const;
    const SwTableBox * getTableBox() const { return mpTableBox;}
    const SwTable * getTable() const { return mpTable;}
    const SwRect & getRect() const { return maRect;}

    const SwNode * getNode() const;

    TableBoxVectorPtr getTableBoxesOfRow();
    WidthsPtr getWidthsOfRow();
    WidthsPtr getColumnWidthsBasedOnAllRows();
    GridColsPtr getGridColsOfRow(AttributeOutputBase & rBase, bool calculateColumnsFromAllRows = false);
    RowSpansPtr getRowSpansOfRow();

#ifdef DBG_UTIL
    ::std::string toString() const;
#endif
};

class CellInfo
{
    SwRect m_aRect;
    WW8TableNodeInfo * m_pNodeInfo;
    unsigned long m_nFmtFrmWidth;

public:
    CellInfo(const SwRect & aRect, WW8TableNodeInfo * pNodeInfo);

    CellInfo(const CellInfo & aRectAndTableInfo)
        : m_aRect(aRectAndTableInfo.m_aRect),
          m_pNodeInfo(aRectAndTableInfo.m_pNodeInfo),
          m_nFmtFrmWidth(aRectAndTableInfo.m_nFmtFrmWidth)
    {
    }

    ~CellInfo() {}

    bool operator < (const CellInfo & aCellInfo) const;

    long top() const { return m_aRect.Top(); }
    long bottom() const { return m_aRect.Bottom(); }
    long left() const { return m_aRect.Left(); }
    long right() const { return m_aRect.Right(); }
    long width() const { return m_aRect.Width(); }
    long height() const { return m_aRect.Height(); }
    SwRect getRect() const { return m_aRect; }
    WW8TableNodeInfo * getTableNodeInfo() const
    { return m_pNodeInfo; }
    unsigned long getFmtFrmWidth() const
    {
        return m_nFmtFrmWidth;
    }

    void setFmtFrmWidth(unsigned long nFmtFrmWidth)
    {
        m_nFmtFrmWidth = nFmtFrmWidth;
    }

#ifdef DBG_UTIL
    ::std::string toString() const;
#endif
};

typedef ::std::multiset<CellInfo, ::std::less<CellInfo> > CellInfoMultiSet;
typedef boost::shared_ptr<CellInfoMultiSet> CellInfoMultiSetPtr;

class WW8TableInfo;
class WW8TableNodeInfo
{
public:
    typedef ::std::map<sal_uInt32, WW8TableNodeInfoInner::Pointer_t,
                ::std::greater<sal_uInt32> > Inners_t;

private:
    WW8TableInfo * mpParent;
    sal_uInt32 mnDepth;
    const SwNode * mpNode;
    Inners_t mInners;
    WW8TableNodeInfo * mpNext;
    const SwNode * mpNextNode;

public:
    typedef boost::shared_ptr<WW8TableNodeInfo> Pointer_t;

    WW8TableNodeInfo(WW8TableInfo * pParent, const SwNode * pTxtNode);
    virtual ~WW8TableNodeInfo();

    void setDepth(sal_uInt32 nDepth);
    void setEndOfLine(bool bEndOfLine);
    void setEndOfCell(bool bEndOfCell);
    void setFirstInTable(bool bFirstInTable);
    void setVertMerge(bool bVertMerge);
    void setTableBox(const SwTableBox *pTableBox);
    void setTable(const SwTable * pTable);
    void setCell(sal_uInt32 nCell);
    void setRow(sal_uInt32 nRow);
    void setShadowsBefore(sal_uInt32 nShadowsBefore);
    void setShadowsAfter(sal_uInt32 nShadowsAfter);
    void setNext(WW8TableNodeInfo * pNext);
    void setNextNode(const SwNode * pNode);
    void setRect(const SwRect & rRect);

    WW8TableInfo * getParent() const { return mpParent;}
    sal_uInt32 getDepth() const;
    bool isEndOfLine() const;
    bool isEndOfCell() const;
    bool isFirstInTable() const;
    const SwNode * getNode() const { return mpNode;}
    const SwTableBox * getTableBox() const;
    WW8TableNodeInfo * getNext() const { return mpNext;}
    const SwNode * getNextNode() const { return mpNextNode;}
    const SwRect & getRect() const;

    const Inners_t & getInners() const { return mInners;}
    const WW8TableNodeInfoInner::Pointer_t getFirstInner() const;
    const WW8TableNodeInfoInner::Pointer_t getInnerForDepth(sal_uInt32 nDepth) const;

    sal_uInt32 getCell() const;
    sal_uInt32 getRow() const;

#ifdef DBG_UTIL
    ::std::string toString() const;
#endif

    bool operator < (const WW8TableNodeInfo & rInfo) const;
};

struct hashNode
{
    size_t operator()(const SwNode * pNode) const
    { return reinterpret_cast<size_t>(pNode); }
};

struct hashTable
{
    size_t operator()(const SwTable * pTable) const
    { return reinterpret_cast<size_t>(pTable); }
};

class WW8TableCellGridRow
{
    CellInfoMultiSetPtr m_pCellInfos;
    TableBoxVectorPtr m_pTableBoxVector;
    WidthsPtr m_pWidths;
    RowSpansPtr m_pRowSpans;

public:
    typedef boost::shared_ptr<WW8TableCellGridRow> Pointer_t;
    WW8TableCellGridRow();
    ~WW8TableCellGridRow();

    void insert(const CellInfo & rCellInfo);
    CellInfoMultiSet::const_iterator begin() const;
    CellInfoMultiSet::const_iterator end() const;

    void setTableBoxVector(TableBoxVectorPtr pTableBoxVector);
    void setWidths(WidthsPtr pGridCols);
    void setRowSpans(RowSpansPtr pRowSpans);

    TableBoxVectorPtr getTableBoxVector() const { return m_pTableBoxVector;}
    WidthsPtr getWidths() const { return m_pWidths;}
    RowSpansPtr getRowSpans() const { return m_pRowSpans;}
};

class WW8TableCellGrid
{
    typedef ::std::set<long> RowTops_t;
    typedef ::std::map<long, WW8TableCellGridRow::Pointer_t> Rows_t;

    RowTops_t m_aRowTops;
    Rows_t m_aRows;

    WW8TableCellGridRow::Pointer_t getRow(long nTop, bool bCreate = true);
    RowTops_t::const_iterator getRowTopsBegin() const;
    RowTops_t::const_iterator getRowTopsEnd() const;
    CellInfoMultiSet::const_iterator getCellsBegin(long nTop);
    CellInfoMultiSet::const_iterator getCellsEnd(long nTop);

public:
    typedef ::boost::shared_ptr<WW8TableCellGrid> Pointer_t;

    WW8TableCellGrid();
    ~WW8TableCellGrid();

    void insert(const SwRect & rRect, WW8TableNodeInfo * pNodeInfo,
                unsigned long * pFmtFrmWidth = NULL);
    void addShadowCells();
    WW8TableNodeInfo * connectCells();

#ifdef DBG_UTIL
    ::std::string toString();
#endif

    TableBoxVectorPtr getTableBoxesOfRow(WW8TableNodeInfoInner * pNodeInfo);
    WidthsPtr getWidthsOfRow(WW8TableNodeInfoInner * pNodeInfo);
    RowSpansPtr getRowSpansOfRow(WW8TableNodeInfoInner * pNodeInfo);
};

class WW8TableInfo
{
    friend class WW8TableNodeInfoInner;
    typedef boost::unordered_map<const SwNode *, WW8TableNodeInfo::Pointer_t, hashNode > Map_t;
    Map_t mMap;

    typedef boost::unordered_map<const SwTable *, WW8TableCellGrid::Pointer_t, hashTable > CellGridMap_t;
    CellGridMap_t mCellGridMap;

    typedef boost::unordered_map<const SwTable *, const SwNode *, hashTable > FirstInTableMap_t;
    FirstInTableMap_t mFirstInTableMap;

    WW8TableNodeInfo *
    processTableLine(const SwTable * pTable,
                     const SwTableLine * pTableLine,
                     sal_uInt32 nRow,
                     sal_uInt32 nDepth, WW8TableNodeInfo * pPrev);

    WW8TableNodeInfo *
    processTableBox(const SwTable * pTable,
                    const SwTableBox * pTableBox,
                    sal_uInt32 nRow,
                    sal_uInt32 nCell,
                    sal_uInt32 nDepth, bool bEndOfLine, WW8TableNodeInfo * pPrev);

    WW8TableNodeInfo::Pointer_t
    processTableBoxLines(const SwTableBox * pBox,
                         const SwTable * pTable,
                         const SwTableBox * pBoxToSet,
                         sal_uInt32 nRow,
                         sal_uInt32 nCell,
                         sal_uInt32 nDepth);

    WW8TableNodeInfo::Pointer_t
    insertTableNodeInfo(const SwNode * pNode,
                        const SwTable * pTable,
                        const SwTableBox * pTableBox,
                        sal_uInt32 nRow,
                        sal_uInt32 nCell,
                        sal_uInt32 nDepth,
                        SwRect * pRect = NULL);

    WW8TableCellGrid::Pointer_t getCellGridForTable(const SwTable * pTable,
                                                    bool bCreate = true);

public:
    typedef boost::shared_ptr<WW8TableInfo> Pointer_t;

    WW8TableInfo();
    virtual ~WW8TableInfo();

    void processSwTable(const SwTable * pTable);
    WW8TableNodeInfo * processSwTableByLayout(const SwTable * pTable);
    WW8TableNodeInfo::Pointer_t getTableNodeInfo(const SwNode * pNode);
    const SwNode * getNextNode(const SwNode * pNode);
    const WW8TableNodeInfo * getFirstTableNodeInfo() const;

    WW8TableNodeInfo * reorderByLayout(const SwTable * pTable);
};

}
#endif // INCLUDED_SW_SOURCE_FILTER_WW8_WW8TABLEINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
