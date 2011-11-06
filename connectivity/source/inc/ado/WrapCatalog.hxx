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


#ifndef CONNECTIVITY_ADO_WRAP_CATALOG_HXX
#define CONNECTIVITY_ADO_WRAP_CATALOG_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOCatalog : public WpOLEBase<_ADOCatalog>
        {
        public:
            WpADOCatalog(_ADOCatalog* pInt = NULL)  :   WpOLEBase<_ADOCatalog>(pInt){}
            WpADOCatalog(const WpADOCatalog& rhs){operator=(rhs);}

            inline WpADOCatalog& operator=(const WpADOCatalog& rhs)
                {WpOLEBase<_ADOCatalog>::operator=(rhs); return *this;}

            ::rtl::OUString GetObjectOwner(const ::rtl::OUString& _rName, ObjectTypeEnum _eNum);

            void putref_ActiveConnection(IDispatch* pCon);
            WpADOTables     get_Tables();
            WpADOViews      get_Views();
            WpADOGroups     get_Groups();
            WpADOUsers      get_Users();
            ADOProcedures*  get_Procedures();
            void Create();
        };
    }
}

#endif //CONNECTIVITY_ADO_WRAP_CATALOG_HXX
