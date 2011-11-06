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



#ifndef CONNECTIVITY_TRESULTSETHELPER_HXX
#define CONNECTIVITY_TRESULTSETHELPER_HXX

#include <sal/types.h>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE IResultSetHelper
    {
    public:
        enum Movement
        {
            NEXT        = 0,
            PRIOR,
            FIRST,
            LAST,
            RELATIVE,
            ABSOLUTE,
            BOOKMARK
        };
    public:
        virtual sal_Bool move(Movement _eCursorPosition, sal_Int32 _nOffset, sal_Bool _bRetrieveData) = 0;
        virtual sal_Int32 getDriverPos() const = 0;
        virtual sal_Bool deletedVisible() const = 0;
        virtual sal_Bool isRowDeleted() const = 0;
    };
}

#endif // CONNECTIVITY_TRESULTSETHELPER_HXX

