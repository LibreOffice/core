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

#ifndef _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_
#define _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_

#include "connectivity/dbtoolsdllapi.hxx"
#include "connectivity/sdbcx/VColumn.hxx"

namespace connectivity
{
    namespace sdbcx
    {
        class OKeyColumn;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OKeyColumn> OKeyColumn_PROP;

        class OOO_DLLPUBLIC_DBTOOLS OKeyColumn :
            public OColumn, public OKeyColumn_PROP
        {
        protected:
            ::rtl::OUString m_ReferencedColumn;
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        public:
            OKeyColumn(sal_Bool _bCase);
            OKeyColumn( const ::rtl::OUString& _ReferencedColumn,
                            const ::rtl::OUString& _Name,
                            const ::rtl::OUString& _TypeName,
                            const ::rtl::OUString& _DefaultValue,
                            sal_Int32       _IsNullable,
                            sal_Int32       _Precision,
                            sal_Int32       _Scale,
                            sal_Int32       _Type,
                            sal_Bool        _IsAutoIncrement,
                            sal_Bool        _IsRowVersion,
                            sal_Bool        _IsCurrency,
                            sal_Bool        _bCase,
                            const ::rtl::OUString& _CatalogName,
                            const ::rtl::OUString& _SchemaName,
                            const ::rtl::OUString& _TableName);
            // just to make it not inline
            virtual ~OKeyColumn();

            virtual void construct();
            DECLARE_SERVICE_INFO();
        };
    }
}
#endif // _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
