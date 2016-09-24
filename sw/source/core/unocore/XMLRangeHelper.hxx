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

//!!
//!! This file is an exact copy of the same file in chart2 project
//!!

#ifndef INCLUDED_SW_SOURCE_CORE_UNOCORE_XMLRANGEHELPER_HXX
#define INCLUDED_SW_SOURCE_CORE_UNOCORE_XMLRANGEHELPER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace XMLRangeHelper
{

struct Cell
{
    sal_Int32 nColumn;
    sal_Int32 nRow;
    bool bRelativeColumn;
    bool bRelativeRow;
    bool bIsEmpty;

    Cell() :
            nColumn(0),
            nRow(0),
            bRelativeColumn(false),
            bRelativeRow(false),
            bIsEmpty(true)
    {}

    inline bool empty() const { return bIsEmpty; }
};

struct CellRange
{
    Cell aUpperLeft;
    Cell aLowerRight;
    OUString aTableName;
};

CellRange getCellRangeFromXMLString( const OUString & rXMLString );

OUString getXMLStringFromCellRange( const CellRange & rRange );

} //  namespace XMLRangeHelper

// INCLUDED_SW_SOURCE_CORE_UNOCORE_XMLRANGEHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
