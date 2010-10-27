/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBACCESS_CORE_API_CROWSETCOLUMN_HXX
#define DBACCESS_CORE_API_CROWSETCOLUMN_HXX

#include "RowSetRow.hxx"
#include <connectivity/CommonTools.hxx>
#include <comphelper/proparrhlp.hxx>
#include "CRowSetDataColumn.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
