/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_CROWSETCOLUMN_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_CROWSETCOLUMN_HXX

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
                        const OUString& _rDescription,
                        const OUString& i_sLabel,
                        ORowSetCacheIterator& _rColumnValue);

        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue )throw (::com::sun::star::uno::Exception, std::exception ) SAL_OVERRIDE;
    };

}

#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_CROWSETCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
