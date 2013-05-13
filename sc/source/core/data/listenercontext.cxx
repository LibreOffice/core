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

EndListeningContext::EndListeningContext(ScDocument& rDoc) : mrDoc(rDoc) {}

ScDocument& EndListeningContext::getDoc()
{
    return mrDoc;
}

void EndListeningContext::addEmptyBroadcasterPosition(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
}

void EndListeningContext::purgeEmptyBroadcasters()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
