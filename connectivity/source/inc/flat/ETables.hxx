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



#ifndef _CONNECTIVITY_FLAT_TABLES_HXX_
#define _CONNECTIVITY_FLAT_TABLES_HXX_

#include "file/FTables.hxx"

namespace connectivity
{
    namespace flat
    {
                //      namespace ::com::sun::star::sdbcx             = ::com::sun::star::sdbcx;
        typedef file::OTables OFlatTables_BASE;

        class OFlatTables : public OFlatTables_BASE
        {
        protected:
            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
        public:
            OFlatTables(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rMetaData,::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
                const TStringVector &_rVector) : OFlatTables_BASE(_rMetaData,_rParent,_rMutex,_rVector)
            {}
        };
    }
}
#endif // _CONNECTIVITY_FLAT_TABLES_HXX_

