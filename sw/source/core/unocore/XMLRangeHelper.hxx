/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



//!!
//!! This file is an exact copy of the same file in chart2 project
//!!

#ifndef XMLRANGEHELPER_HXX
#define XMLRANGEHELPER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

//namespace chart
//{
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
    ::rtl::OUString aTableName;
};

CellRange getCellRangeFromXMLString( const ::rtl::OUString & rXMLString );

::rtl::OUString getXMLStringFromCellRange( const CellRange & rRange );


} //  namespace XMLRangeHelper
//} //  namespace chart

// XMLRANGEHELPER_HXX
#endif
