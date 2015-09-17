/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "listenercontext.hxx"
#include "document.hxx"
#include "mtvelements.hxx"

namespace sc {

StartListeningContext::StartListeningContext(ScDocument& rDoc) :
    mrDoc(rDoc), mpSet(new ColumnBlockPositionSet(rDoc)) {}

StartListeningContext::StartListeningContext(
    ScDocument& rDoc, const boost::shared_ptr<ColumnBlockPositionSet>& pSet) :
    mrDoc(rDoc), mpSet(pSet) {}

ColumnBlockPosition* StartListeningContext::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    return mpSet->getBlockPosition(nTab, nCol);
}

EndListeningContext::EndListeningContext(ScDocument& rDoc, ScTokenArray* pOldCode) :
    mrDoc(rDoc), maSet(false), mpPosSet(new ColumnBlockPositionSet(rDoc)),
    mpOldCode(pOldCode), maPosDelta(0,0,0) {}

EndListeningContext::EndListeningContext(
    ScDocument& rDoc, const boost::shared_ptr<ColumnBlockPositionSet>& pSet, ScTokenArray* pOldCode) :
    mrDoc(rDoc), maSet(false), mpPosSet(pSet),
    mpOldCode(pOldCode), maPosDelta(0,0,0) {}

void EndListeningContext::setPositionDelta( const ScAddress& rDelta )
{
    maPosDelta = rDelta;
}

ScAddress EndListeningContext::getOldPosition( const ScAddress& rPos ) const
{
    ScAddress aOldPos = rPos;
    aOldPos.IncCol(maPosDelta.Col());
    aOldPos.IncRow(maPosDelta.Row());
    aOldPos.IncTab(maPosDelta.Tab());
    return aOldPos;
}

ColumnBlockPosition* EndListeningContext::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    return mpPosSet->getBlockPosition(nTab, nCol);
}

void EndListeningContext::addEmptyBroadcasterPosition(SCTAB nTab, SCCOL nCol, SCROW nRow)
{
    maSet.set(nTab, nCol, nRow, true);
}

void EndListeningContext::purgeEmptyBroadcasters()
{
    PurgeListenerAction aAction(mrDoc);
    maSet.executeAction(aAction);
}

PurgeListenerAction::PurgeListenerAction(ScDocument& rDoc) :
    mrDoc(rDoc), mpBlockPos(new ColumnBlockPosition) {}

void PurgeListenerAction::startColumn( SCTAB nTab, SCCOL nCol )
{
    mrDoc.InitColumnBlockPosition(*mpBlockPos, nTab, nCol);
}

void PurgeListenerAction::execute( const ScAddress& rPos, SCROW nLength, bool bVal )
{
    if (bVal)
    {
        mrDoc.DeleteBroadcasters(*mpBlockPos, rPos, nLength);
    }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
