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



#ifndef SVTOOLS_INC_TABLE_TABLETYPES_HXX
#define SVTOOLS_INC_TABLE_TABLETYPES_HXX

#include <sal/types.h>

//........................................................................
namespace svt { namespace table
{
//........................................................................
    /// a value denoting the size of a table
    typedef sal_Int32   TableSize;

    /// a value denoting a column position within a table
    typedef sal_Int32   ColPos;
    /// a value denoting a row position within a table
    typedef sal_Int32   RowPos;

    /** a value denoting an arbitrary coordinate value of a position within
        a table

        Values of this type are guaranteed to be large enough to hold column
        positions as well as row positions.
    */
    typedef sal_Int32   AnyPos;

    typedef sal_Int32   TableMetrics;

/// denotes the column containing the row headers
#define COL_ROW_HEADERS         ((::svt::table::ColPos)-1)
/// denotes the row containing the column headers
#define ROW_COL_HEADERS         ((::svt::table::RowPos)-1)

/// denotes an invalid column index
#define COL_INVALID             ((::svt::table::ColPos)-2)
/// denotes an invalid row index
#define ROW_INVALID             ((::svt::table::RowPos)-2)


//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLETYPES_HXX
