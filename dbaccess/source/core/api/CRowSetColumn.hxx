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


#ifndef DBACCESS_CORE_API_CROWSETCOLUMN_HXX
#define DBACCESS_CORE_API_CROWSETCOLUMN_HXX

#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#include "RowSetRow.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef DBACCESS_CORE_API_CROWSETDATACOLUMN_HXX
#include "CRowSetDataColumn.hxx"
#endif

namespace dbaccess
{
    class ORowSetColumn;
    class ORowSetColumn :public ORowSetDataColumn
                        ,public ::comphelper::OPropertyArrayUsageHelper< ORowSetColumn >

    {
    public:
        ORowSetColumn(  const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >& _xMetaData,
                        const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRow >& _xRow,
                        sal_Int32 _nPos,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMeta,
                        const ::rtl::OUString& _rDescription,
                        const ::rtl::OUString& i_sLabel,
                        ORowSetCacheIterator& _rColumnValue);

        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue )throw (::com::sun::star::uno::Exception );
    };

}

#endif // DBACCESS_CORE_API_CROWSETCOLUMN_HXX
