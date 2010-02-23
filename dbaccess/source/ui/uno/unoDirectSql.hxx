/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

