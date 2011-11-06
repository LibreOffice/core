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


#ifndef _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_
#define _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_

#include "connectivity/CommonTools.hxx"
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    namespace sdbcx
    {
        class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE IRefreshableGroups
        {
        public:
            virtual void refreshGroups() = 0;
        };

        class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE IRefreshableUsers
        {
        public:
            virtual void refreshUsers() = 0;
        };

        class OOO_DLLPUBLIC_DBTOOLS SAL_NO_VTABLE IRefreshableColumns
        {
        public:
            virtual void refreshColumns()   = 0;
        };
    }
}
#endif //_CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_

