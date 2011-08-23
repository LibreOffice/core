/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rangelst.hxx,v $
 * $Revision: 1.9.32.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __SC_SIMPLERANGELIST_HXX__
#define __SC_SIMPLERANGELIST_HXX__

#include <boost/shared_ptr.hpp>

#include "address.hxx"

#include <map>
#include <list>

class ScAddress;
class ScRange;
class ScRangeList;

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
        SCCOL mnCol1;
        SCROW mnRow1;
        SCCOL mnCol2;
        SCROW mnRow2;
        explicit Range(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);

        bool contains(const Range& r) const;
    };
    typedef ::boost::shared_ptr< ::std::list<Range> > RangeListRef;
    typedef ::std::map<SCTAB, RangeListRef> TabType;

    ScSimpleRangeList();

    void addRange(const ScRange& rRange);
    void insertCol(SCCOL nCol, SCTAB nTab);
    void insertRow(SCROW nRow, SCTAB nTab);
    void getRangeList(::std::list<ScRange>& rList) const;
    void clear();

private:
    RangeListRef findTab(SCTAB nTab);

private:
    TabType maTabs;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
