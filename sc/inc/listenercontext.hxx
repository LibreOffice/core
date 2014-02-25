/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_LISTENERCONTEXT_HXX
#define SC_LISTENERCONTEXT_HXX

#include "address.hxx"
#include "columnspanset.hxx"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

class ScDocument;
class ScTokenArray;

namespace sc {

struct ColumnBlockPosition;
class ColumnBlockPositionSet;

class StartListeningContext : boost::noncopyable
{
    ScDocument& mrDoc;
    boost::scoped_ptr<ColumnBlockPositionSet> mpSet;
public:
    StartListeningContext(ScDocument& rDoc);
    ScDocument& getDoc();

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);
};

class EndListeningContext : boost::noncopyable
{
    ScDocument& mrDoc;
    ColumnSpanSet maSet;
    boost::scoped_ptr<ColumnBlockPositionSet> mpPosSet;
    ScTokenArray* mpOldCode;
    ScAddress maPosDelta; // Add this to get the old position prior to the move.

public:
    EndListeningContext(ScDocument& rDoc, ScTokenArray* pOldCode = NULL);

    void setPositionDelta( const ScAddress& rDelta );

    ScDocument& getDoc();
    ScTokenArray* getOldCode();
    ScAddress getOldPosition( const ScAddress& rPos ) const;

    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);

    void addEmptyBroadcasterPosition(SCTAB nTab, SCCOL nCol, SCROW nRow);
    void purgeEmptyBroadcasters();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
