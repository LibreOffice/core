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

namespace sc {

namespace {

class PurgeAction : public ColumnSpanSet::Action
{
    ScDocument& mrDoc;
public:
    PurgeAction(ScDocument& rDoc) : mrDoc(rDoc) {}
    virtual void execute(const ScAddress& rPos, SCROW nLength, bool bVal)
    {
        if (bVal)
            mrDoc.DeleteBroadcasters(rPos, nLength);
    };
};

}

StartListeningContext::StartListeningContext(ScDocument& rDoc) : mrDoc(rDoc), maSet(rDoc) {}

ScDocument& StartListeningContext::getDoc()
{
    return mrDoc;
}

ColumnBlockPosition* StartListeningContext::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    return maSet.getBlockPosition(nTab, nCol);
}

EndListeningContext::EndListeningContext(ScDocument& rDoc) : mrDoc(rDoc), maPosSet(rDoc) {}

ScDocument& EndListeningContext::getDoc()
{
    return mrDoc;
}

ColumnBlockPosition* EndListeningContext::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    return maPosSet.getBlockPosition(nTab, nCol);
}

void EndListeningContext::addEmptyBroadcasterPosition(SCTAB nTab, SCCOL nCol, SCROW nRow)
{
    maSet.set(nTab, nCol, nRow, true);
}

void EndListeningContext::purgeEmptyBroadcasters()
{
    PurgeAction aAction(mrDoc);
    maSet.executeFromBottom(aAction);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
