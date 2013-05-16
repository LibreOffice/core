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

namespace sc {

/**
 * Structure that stores segments of boolean flags per column, and perform
 * custom action on those segments.
 */
class ColumnSpanSet : boost::noncopyable
{
    typedef mdds::flat_segment_tree<SCROW, bool> ColumnSpansType;
    typedef std::vector<ColumnSpansType*> TableType;
    typedef std::vector<TableType*> DocType;

    DocType maDoc;

    ColumnSpansType& getColumnSpans(SCTAB nTab, SCCOL nCol);

public:
    class Action
    {
    public:
        virtual ~Action() = 0;
        virtual void startColumn(SCTAB nTab, SCCOL nCol);
        virtual void execute(const ScAddress& rPos, SCROW nLength, bool bVal) = 0;
    };

    ~ColumnSpanSet();

    void set(SCTAB nTab, SCCOL nCol, SCROW nRow, bool bVal);
    void set(SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2, bool bVal);

    void executeFromTop(Action& ac) const;
    void executeFromBottom(Action& ac) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
