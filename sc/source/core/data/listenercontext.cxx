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
    ColumnBlockContext(rDoc) {}

StartListeningContext::StartListeningContext(
    ScDocument& rDoc, const boost::shared_ptr<ColumnBlockPositionSet>& pSet) :
    ColumnBlockContext(rDoc, pSet) {}

EndListeningContext::EndListeningContext(ScDocument& rDoc, ScTokenArray* pOldCode) :
    ColumnBlockContext(rDoc),
    maSet(false), mpOldCode(pOldCode), maPosDelta(0,0,0) {}

EndListeningContext::EndListeningContext(
    ScDocument& rDoc, const boost::shared_ptr<ColumnBlockPositionSet>& pSet, ScTokenArray* pOldCode) :
    ColumnBlockContext(rDoc, pSet),
    maSet(false), mpOldCode(pOldCode), maPosDelta(0,0,0) {}

void EndListeningContext::setPositionDelta( const ScAddress& rDelta )
{
    maPosDelta = rDelta;
}

ScTokenArray* EndListeningContext::getOldCode()
{
    return mpOldCode;
}

ScAddress EndListeningContext::getOldPosition( const ScAddress& rPos ) const
{
    ScAddress aOldPos = rPos;
    aOldPos.IncCol(maPosDelta.Col());
    aOldPos.IncRow(maPosDelta.Row());
    aOldPos.IncTab(maPosDelta.Tab());
    return aOldPos;
}

void EndListeningContext::addEmptyBroadcasterPosition(SCTAB nTab, SCCOL nCol, SCROW nRow)
{
    maSet.set(nTab, nCol, nRow, true);
}

void EndListeningContext::purgeEmptyBroadcasters()
{
    PurgeListenerAction aAction(getDoc());
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
