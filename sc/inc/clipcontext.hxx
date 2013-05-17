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

class ClipContextBase
{
    sc::ColumnBlockPositionSet maSet;

    ClipContextBase(); // disabled

public:
    ClipContextBase(ScDocument& rDoc);
    virtual ~ClipContextBase();

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);
};

class CopyFromClipContext : public ClipContextBase
{
    SCTAB mnTabStart;
    SCTAB mnTabEnd;
    ScDocument* mpRefUndoDoc;
    ScDocument* mpClipDoc;
    sal_uInt16  mnInsertFlag;
    bool        mbAsLink:1;
    bool        mbSkipAttrForEmptyCells:1;

    CopyFromClipContext(); // disabled

public:
    CopyFromClipContext(ScDocument& rDoc,
        ScDocument* pRefUndoDoc, ScDocument* pClipDoc, sal_uInt16 nInsertFlag,
        bool bAsLink, bool bSkipAttrForEmptyCells);

    virtual ~CopyFromClipContext();

    void setTabRange(SCTAB nStart, SCTAB nEnd);

    SCTAB getTabStart() const;
    SCTAB getTabEnd() const;

    ScDocument* getUndoDoc();
    ScDocument* getClipDoc();
    sal_uInt16 getInsertFlag() const;
    bool isAsLink() const;
    bool isSkipAttrForEmptyCells() const;
};

class CopyToClipContext : public ClipContextBase
{
    bool mbKeepScenarioFlags:1;
    bool mbCloneNotes:1;

    CopyToClipContext(); // disabled

public:
    CopyToClipContext(ScDocument& rDoc, bool bKeepScenarioFlags, bool bCloneNotes);
    virtual ~CopyToClipContext();

    bool isKeepScenarioFlags() const;
    bool isCloneNotes() const;
};

class CopyToDocContext : public ClipContextBase
{
public:
    CopyToDocContext(ScDocument& rDoc);
    virtual ~CopyToDocContext();
};

class MixDocContext : public ClipContextBase
{
public:
    MixDocContext(ScDocument& rDoc);
    virtual ~MixDocContext();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
