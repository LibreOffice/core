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


#ifndef CONNECTIVITY_PRIVILEGESRESULTSET_HXX
#define CONNECTIVITY_PRIVILEGESRESULTSET_HXX

#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OResultSetPrivileges :
        public ODatabaseMetaDataResultSet
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>   m_xTables;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>         m_xRow;
        sal_Bool                                                                m_bResetValues;
    protected:
        virtual const ORowSetValue& getValue(sal_Int32 columnIndex);
    public:
        OResultSetPrivileges(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta
            ,const ::com::sun::star::uno::Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern);

        // ::cppu::OComponentHelper
        virtual void SAL_CALL disposing(void);
        // XResultSet
        virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif // CONNECTIVITY_PRIVILEGESRESULTSET_HXX
