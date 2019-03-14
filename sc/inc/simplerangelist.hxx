/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_INC_SIMPLERANGELIST_HXX
#define INCLUDED_SC_INC_SIMPLERANGELIST_HXX

#include "address.hxx"

#include <list>
#include <map>
#include <memory>

/**
 * This container is optimized for use in the ods import filter, to store
 * ranges for cell styles.  We may change the name of this class once we
 * have a better name for what it does.  Using this is way more efficient
 * than ScRangeList.
 */
class ScSimpleRangeList
{
public:
    struct Range
    {
        SCROW mnRow1;
        SCROW mnRow2;
        SCCOL mnCol1;
        SCCOL mnCol2;
        explicit Range(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    };
    typedef std::shared_ptr< ::std::list<Range> > RangeListRef;
    typedef ::std::map<SCTAB, RangeListRef> TabType;

    ScSimpleRangeList();

    void addRange(const ScRange& rRange);
    void insertCol(SCCOL nCol, SCTAB nTab);
    void getRangeList(::std::list<ScRange>& rList) const;
    void clear();

private:
    RangeListRef findTab(SCTAB nTab);

private:
    TabType maTabs;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
