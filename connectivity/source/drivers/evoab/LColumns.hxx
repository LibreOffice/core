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



#ifndef _CONNECTIVITY_EVOAB_LCOLUMNS_HXX_
#define _CONNECTIVITY_EVOAB_LCOLUMNS_HXX_

#include "file/FColumns.hxx"

namespace connectivity
{
    namespace evoab
    {
        class OEvoabColumns : public file::OColumns
        {
        protected:
            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        public:
            OEvoabColumns(file::OFileTable* _pTable,
                            ::osl::Mutex& _rMutex,
                            const TStringVector &_rVector
                         ) : file::OColumns(_pTable,_rMutex,_rVector)
            {}

        };
    }
}
#endif // _CONNECTIVITY_EVOAB_LCOLUMNS_HXX_

