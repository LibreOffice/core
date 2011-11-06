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



#ifndef DBAUI_UNODIRECTSQL_HXX
#define DBAUI_UNODIRECTSQL_HXX

#ifndef _SVT_GENERICUNODIALOG_HXX_
#include <svtools/genericunodialog.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    //=====================================================================
    //= ODirectSQLDialog
    //=====================================================================
    class ODirectSQLDialog;
    typedef ::svt::OGenericUnoDialog                                        ODirectSQLDialog_BASE;
    typedef ::comphelper::OPropertyArrayUsageHelper< ODirectSQLDialog >     ODirectSQLDialog_PBASE;

    class ODirectSQLDialog
            :public ODirectSQLDialog_BASE
            ,public ODirectSQLDialog_PBASE
    {
        OModuleClient m_aModuleClient;
        ::rtl::OUString m_sInitialSelection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xActiveConnection;
    protected:
        ODirectSQLDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        virtual ~ODirectSQLDialog();

    public:
        DECLARE_IMPLEMENTATION_ID( );

        DECLARE_SERVICE_INFO_STATIC( );

        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

    protected:
        // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
        virtual void implInitialize(const com::sun::star::uno::Any& _rValue);
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_UNODIRECTSQL_HXX

