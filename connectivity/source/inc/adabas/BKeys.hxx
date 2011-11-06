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



#ifndef _CONNECTIVITY_ADABAS_KEYS_HXX_
#define _CONNECTIVITY_ADABAS_KEYS_HXX_

#include "connectivity/TKeys.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "adabas/BTable.hxx"
namespace connectivity
{
    namespace adabas
    {
        class OKeys : public OKeysHelper
        {
        protected:
            virtual sdbcx::ObjectType appendObject( const ::rtl::OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );

            virtual ::rtl::OUString getDropForeignKey() const;
        public:
            OKeys(OAdabasTable* _pTable,
                     ::osl::Mutex& _rMutex,
                     const TStringVector &_rVector
                     ) : OKeysHelper(_pTable,_rMutex,_rVector)
            {}
        };
    }
}

#endif // _CONNECTIVITY_ADABAS_KEYS_HXX_

