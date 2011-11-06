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



#ifndef _DBAUI_DATASOURCECONNECTOR_HXX_
#define _DBAUI_DATASOURCECONNECTOR_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif

namespace dbtools
{
    class SQLExceptionInfo;
}

class Window;
//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= ODatasourceConnector
    //=====================================================================
    class ODatasourceConnector
    {
    protected:
        Window*         m_pErrorMessageParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;
        ::rtl::OUString m_sContextInformation;

    public:
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pMessageParent
        );
        ODatasourceConnector(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            Window* _pMessageParent,
            const ::rtl::OUString& _rContextInformation
        );

        /// returns <TRUE/> if the object is able to create data source connections
        sal_Bool    isValid() const { return m_xORB.is(); }

        /** creates a connection to the data source, displays the possible error to the user, or returns it
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(
                        const ::rtl::OUString& _rDataSourceName,
                        ::dbtools::SQLExceptionInfo* _pErrorInfo
                    ) const;

        /** creates a connection to the data source, displays the possible error to the user, or returns it
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    connect(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& _xDataSource,
                        ::dbtools::SQLExceptionInfo* _pErrorInfo
                    ) const;
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DATASOURCECONNECTOR_HXX_

