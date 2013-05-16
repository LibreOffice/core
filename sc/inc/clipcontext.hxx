/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_CLIPCONTEXT_HXX
#define SC_CLIPCONTEXT_HXX

#include "address.hxx"
#include "mtvelements.hxx"

#include <vector>
#include <boost/unordered_map.hpp>

class ScDocument;

namespace sc {

class CopyFromClipContext
{
    typedef boost::unordered_map<SCCOL, ColumnBlockPosition> ColumnsType;
    typedef std::vector<ColumnsType> TablesType;

    TablesType maTables;

    ScDocument& mrDoc;

    ScDocument* mpRefUndoDoc;
    ScDocument* mpClipDoc;
    sal_uInt16  mnInsertFlag;
    SCTAB       mnTabStart;
    SCTAB       mnTabEnd;
    bool        mbAsLink:1;
    bool        mbSkipAttrForEmptyCells:1;

    CopyFromClipContext(); // disabled

public:
    CopyFromClipContext(ScDocument& rDoc,
        ScDocument* pRefUndoDoc, ScDocument* pClipDoc, sal_uInt16 nInsertFlag,
        bool bAsLink, bool bSkipAttrForEmptyCells);

    ~CopyFromClipContext();

    void setTabRange(SCTAB nStart, SCTAB nEnd);

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);

    ScDocument* getUndoDoc();
    ScDocument* getClipDoc();
    sal_uInt16 getInsertFlag() const;
    SCTAB getTabStart() const;
    SCTAB getTabEnd() const;
    bool isAsLink() const;
    bool isSkipAttrForEmptyCells() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
