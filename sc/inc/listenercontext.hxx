/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_LISTENERCONTEXT_HXX
#define INCLUDED_SC_INC_LISTENERCONTEXT_HXX

#include "address.hxx"
#include "columnspanset.hxx"

#include <boost/noncopyable.hpp>
#include <memory>

class ScDocument;
class ScTokenArray;

namespace sc {

struct ColumnBlockPosition;
class ColumnBlockPositionSet;

class StartListeningContext : boost::noncopyable
{
    ScDocument& mrDoc;
    std::shared_ptr<ColumnBlockPositionSet> mpSet;
public:
    StartListeningContext(ScDocument& rDoc);
    StartListeningContext(ScDocument& rDoc, const std::shared_ptr<ColumnBlockPositionSet>& pSet);
    ScDocument& getDoc() { return mrDoc;}

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);
};

class EndListeningContext : boost::noncopyable
{
    ScDocument& mrDoc;
    ColumnSpanSet maSet;
    std::shared_ptr<ColumnBlockPositionSet> mpPosSet;
    ScTokenArray* mpOldCode;
    ScAddress maPosDelta; // Add this to get the old position prior to the move.

public:
    EndListeningContext(ScDocument& rDoc, ScTokenArray* pOldCode = nullptr);
    EndListeningContext(ScDocument& rDoc, const std::shared_ptr<ColumnBlockPositionSet>& pSet, ScTokenArray* pOldCode = nullptr);

    void setPositionDelta( const ScAddress& rDelta );

    ScDocument& getDoc() { return mrDoc;}
    ScTokenArray* getOldCode() { return mpOldCode;}
    ScAddress getOldPosition( const ScAddress& rPos ) const;

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);

    void addEmptyBroadcasterPosition(SCTAB nTab, SCCOL nCol, SCROW nRow);
    void purgeEmptyBroadcasters();
};

class PurgeListenerAction : public ColumnSpanSet::Action, boost::noncopyable
{
    ScDocument& mrDoc;
    std::unique_ptr<ColumnBlockPosition> mpBlockPos;

public:
    PurgeListenerAction( ScDocument& rDoc );

    virtual void startColumn( SCTAB nTab, SCCOL nCol ) override;
    virtual void execute( const ScAddress& rPos, SCROW nLength, bool bVal ) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
