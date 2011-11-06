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



#ifndef _CONNECTIVITY_ADABAS_KEYCOLUMN_HXX_
#define _CONNECTIVITY_ADABAS_KEYCOLUMN_HXX_

#include "ado/AColumn.hxx"

namespace connectivity
{
    namespace ado
    {
        class OKeyColumn : public OAdoColumn
        {
        protected:
            ::rtl::OUString m_ReferencedColumn;
        public:
            OKeyColumn();
            OKeyColumn(     const ::rtl::OUString& _ReferencedColumn,
                            const ::rtl::OUString& _Name,
                            const ::rtl::OUString& _TypeName,
                            const ::rtl::OUString& _DefaultValue,
                            sal_Int32       _IsNullable,
                            sal_Int32       _Precision,
                            sal_Int32       _Scale,
                            sal_Int32       _Type,
                            sal_Bool        _IsAutoIncrement);

            virtual void construct();
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_KEYCOLUMN_HXX_

