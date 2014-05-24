/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <columnblockcontext.hxx>
#include <mtvelements.hxx>

namespace sc {

ColumnBlockContext::ColumnBlockContext(ScDocument& rDoc) :
    mrDoc(rDoc), mpSet(new ColumnBlockPositionSet(rDoc)) {}

ColumnBlockContext::ColumnBlockContext(
    ScDocument& rDoc, const boost::shared_ptr<ColumnBlockPositionSet>& pSet) :
    mrDoc(rDoc), mpSet(pSet) {}

ColumnBlockContext::~ColumnBlockContext() {}

ScDocument& ColumnBlockContext::getDoc()
{
    return mrDoc;
}

ColumnBlockPosition* ColumnBlockContext::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    return mpSet->getBlockPosition(nTab, nCol);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
