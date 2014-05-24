/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_COLUMNBLOCKCONTEXT_HXX
#define SC_COLUMNBLOCKCONTEXT_HXX

#include <types.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

class ScDocument;

namespace sc {

struct ColumnBlockPosition;
class ColumnBlockPositionSet;

class ColumnBlockContext : boost::noncopyable
{
    ScDocument& mrDoc;
    boost::shared_ptr<ColumnBlockPositionSet> mpSet;

    ColumnBlockContext(); // disabled

public:
    ColumnBlockContext(ScDocument& rDoc);
    ColumnBlockContext(ScDocument& rDoc, const boost::shared_ptr<ColumnBlockPositionSet>& pSet);

    virtual ~ColumnBlockContext();

    ScDocument& getDoc();
    ColumnBlockPosition* getBlockPosition(SCTAB nTab, SCCOL nCol);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
