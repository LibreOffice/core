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
#ifndef CHART2_XMLRANGEHELPER_HXX
#define CHART2_XMLRANGEHELPER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include "charttoolsdllapi.hxx"

namespace chart
{
namespace XMLRangeHelper
{

struct OOO_DLLPUBLIC_CHARTTOOLS Cell
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

struct OOO_DLLPUBLIC_CHARTTOOLS CellRange
{
    Cell aUpperLeft;
    Cell aLowerRight;
    OUString aTableName;
};

CellRange getCellRangeFromXMLString( const OUString & rXMLString );

OUString getXMLStringFromCellRange( const CellRange & rRange );

} //  namespace XMLRangeHelper
} //  namespace chart

// CHART2_XMLRANGEHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
