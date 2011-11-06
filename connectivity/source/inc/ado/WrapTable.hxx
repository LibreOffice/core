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


#ifndef CONNECTIVITY_ADO_WRAP_TABLE_HXX
#define CONNECTIVITY_ADO_WRAP_TABLE_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOCatalog;

        class WpADOTable : public WpOLEBase<_ADOTable>
        {
        public:
            WpADOTable(_ADOTable* pInt=NULL)    :   WpOLEBase<_ADOTable>(pInt){}
            WpADOTable(const WpADOTable& rhs){operator=(rhs);}

            inline WpADOTable& operator=(const WpADOTable& rhs)
                {WpOLEBase<_ADOTable>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const;
            void            put_Name(const ::rtl::OUString& _rName);
            ::rtl::OUString get_Type() const;
            WpADOColumns    get_Columns() const;
            WpADOIndexes    get_Indexes() const;
            WpADOKeys       get_Keys() const;
            WpADOCatalog    get_ParentCatalog() const;
            WpADOProperties get_Properties() const;
            void            putref_ParentCatalog(/* [in] */ _ADOCatalog __RPC_FAR *ppvObject);
        };


        typedef WpOLEAppendCollection<ADOTables,    _ADOTable,  WpADOTable>     WpADOTables;
    }
}

#endif //CONNECTIVITY_ADO_WRAP_TABLE_HXX
