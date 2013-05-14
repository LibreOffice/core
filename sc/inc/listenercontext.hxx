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

class ScDocument;

namespace sc {

class EndListeningContext : boost::noncopyable
{
    ScDocument& mrDoc;
    ColumnSpanSet maSet;
public:
    EndListeningContext(ScDocument& rDoc);
    ScDocument& getDoc();

    void addEmptyBroadcasterPosition(SCCOL nCol, SCROW nRow, SCTAB nTab);
    void purgeEmptyBroadcasters();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
