/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_COLUMNSPANSET_HXX
#define SC_COLUMNSPANSET_HXX

#include "address.hxx"

#include <vector>
#include <mdds/flat_segment_tree.hpp>
#include <boost/noncopyable.hpp>

class ScDocument;
class ScColumn;
class ScMarkData;
class ScRange;

namespace sc {

struct ColumnBlockConstPosition;

struct RowSpan
{
    SCROW mnRow1;
    SCROW mnRow2;

    RowSpan(SCROW nRow1, SCROW nRow2);
};

/**
 * Structure that stores segments of boolean flags per column, and perform
 * custom action on those segments.
 */
class ColumnSpanSet : boost::noncopyable
{
    typedef mdds::flat_segment_tree<SCROW, bool> ColumnSpansType;

    struct ColumnType
    {
        ColumnSpansType maSpans;
        ColumnSpansType::const_iterator miPos;

        ColumnType(SCROW nStart, SCROW nEnd, bool bInit);
    };

    typedef std::vector<ColumnType*> TableType;
    typedef std::vector<TableType*> DocType;

    DocType maDoc;
    bool mbInit;

    ColumnType& getColumn(SCTAB nTab, SCCOL nCol);

public:
    class Action
    {
    public:
        virtual ~Action() = 0;
        virtual void startColumn(SCTAB nTab, SCCOL nCol);
        virtual void execute(const ScAddress& rPos, SCROW nLength, bool bVal) = 0;
    };

    class ColumnAction
    {
    public:
        virtual ~ColumnAction() = 0;
        virtual void startColumn(ScColumn* pCol) = 0;
        virtual void execute(SCROW nRow1, SCROW nRow2, bool bVal) = 0;
    };

    ColumnSpanSet(bool bInit);
    ~ColumnSpanSet();

    void set(SCTAB nTab, SCCOL nCol, SCROW nRow, bool bVal);
    void set(SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2, bool bVal);
    void set(const ScRange& rRange, bool bVal);

    void executeAction(Action& ac) const;
    void executeColumnAction(ScDocument& rDoc, ColumnAction& ac) const;
};

/**
 * Keep track of spans in a single column only.
 */
class SingleColumnSpanSet
{
public:
    typedef mdds::flat_segment_tree<SCROW, bool> ColumnSpansType;

    typedef std::vector<RowSpan> SpansType;

    SingleColumnSpanSet();

    /**
     * Scan an entire column and tag all non-empty cell positions.
     */
    void scan(const ScColumn& rColumn);

    /**
     * Scan a column between specified range, and tag all non-empty cell
     * positions.
     */
    void scan(const ScColumn& rColumn, SCROW nStart, SCROW nEnd);

    void scan(
        ColumnBlockConstPosition& rBlockPos, const ScColumn& rColumn, SCROW nStart, SCROW nEnd);

    /**
     * Scan all marked data and tag all marked segments in specified column.
     */
    void scan(const ScMarkData& rMark, SCTAB nTab, SCCOL nCol);

    void set(SCROW nRow1, SCROW nRow2, bool bVal);

    void getRows(std::vector<SCROW> &rRows) const;

    void getSpans(SpansType& rSpans) const;

private:
    ColumnSpansType maSpans;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
