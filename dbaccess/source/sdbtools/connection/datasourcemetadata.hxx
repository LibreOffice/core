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



#ifndef DBACCESS_DATASOURCEMETADATA_HXX
#define DBACCESS_DATASOURCEMETADATA_HXX

#ifndef DBACCESS_CONNECTION_DEPENDENT_HXX
#include "connectiondependent.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDB_TOOLS_XDATASOURCEMETADATA_HPP_
#include <com/sun/star/sdb/tools/XDataSourceMetaData.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <memory>

//........................................................................
namespace sdbtools
{
//........................................................................

    //====================================================================
    //= DataSourceMetaData
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdb::tools::XDataSourceMetaData
                                    >   DataSourceMetaData_Base;
    struct DataSourceMetaData_Impl;
    /** default implementation for XDataSourceMetaData
    */
    class DataSourceMetaData    :public DataSourceMetaData_Base
                                ,public ConnectionDependentComponent
    {
    private:
        ::std::auto_ptr< DataSourceMetaData_Impl >   m_pImpl;

    public:
        /** constructs the instance
            @param _rContext
                the component's context
            @param  _rxConnection
                the connection to work with. Will be held weak. Must not be <NULL/>.
            @throws ::com::sun::star::lang::NullPointerException
                if _rxConnection is <NULL/>
        */
        DataSourceMetaData(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
        );

        // XDataSourceMetaData
        virtual ::sal_Bool SAL_CALL supportsQueriesInFrom(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~DataSourceMetaData();

    private:
        DataSourceMetaData();                                  // never implemented
        DataSourceMetaData( const DataSourceMetaData& );              // never implemented
        DataSourceMetaData& operator=( const DataSourceMetaData& );   // never implemented
    };


//........................................................................
} // namespace sdbtools
//........................................................................

#endif // DBACCESS_DATASOURCEMETADATA_HXX
