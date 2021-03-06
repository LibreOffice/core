/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "address.hxx"
#include "columnspanset.hxx"
#include "mtvelements.hxx"

#include <memory>

class ScDocument;
class ScTokenArray;

namespace sc
{
class ColumnSet;

class StartListeningContext
{
    ScDocument& mrDoc;
    std::shared_ptr<ColumnBlockPositionSet> mpSet;
    std::shared_ptr<const ColumnSet> mpColSet;

public:
    StartListeningContext(const StartListeningContext&) = delete;
    const StartListeningContext& operator=(const StartListeningContext&) = delete;
    StartListeningContext(ScDocument& rDoc);
    StartListeningContext(ScDocument& rDoc, const std::shared_ptr<ColumnBlockPositionSet>& pSet);
    void setColumnSet(const std::shared_ptr<const ColumnSet>& pColSet);
    const std::shared_ptr<const ColumnSet>& getColumnSet() const;
    ScDocument& getDoc() { return mrDoc; }

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);
};

class EndListeningContext
{
    ScDocument& mrDoc;
    ColumnSpanSet maSet;
    std::shared_ptr<ColumnBlockPositionSet> mpPosSet;
    ScTokenArray* mpOldCode;
    ScAddress maPosDelta; // Add this to get the old position prior to the move.

public:
    EndListeningContext(const EndListeningContext&) = delete;
    const EndListeningContext& operator=(const EndListeningContext&) = delete;
    EndListeningContext(ScDocument& rDoc, ScTokenArray* pOldCode = nullptr);
    EndListeningContext(ScDocument& rDoc, const std::shared_ptr<ColumnBlockPositionSet>& pSet,
                        ScTokenArray* pOldCode = nullptr);

    void setPositionDelta(const ScAddress& rDelta);

    ScDocument& getDoc() { return mrDoc; }
    ScTokenArray* getOldCode() { return mpOldCode; }
    ScAddress getOldPosition(const ScAddress& rPos) const;

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);

    void addEmptyBroadcasterPosition(SCTAB nTab, SCCOL nCol, SCROW nRow);
    void purgeEmptyBroadcasters();
};

class PurgeListenerAction final : public ColumnSpanSet::Action
{
    ScDocument& mrDoc;
    std::unique_ptr<ColumnBlockPosition> mpBlockPos;

public:
    PurgeListenerAction(const PurgeListenerAction&) = delete;
    const PurgeListenerAction& operator=(const PurgeListenerAction&) = delete;
    PurgeListenerAction(ScDocument& rDoc);

    virtual void startColumn(SCTAB nTab, SCCOL nCol) override;
    virtual void execute(const ScAddress& rPos, SCROW nLength, bool bVal) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
