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

#ifndef INCLUDED_CONNECTIVITY_SDBCX_VINDEXCOLUMN_HXX
#define INCLUDED_CONNECTIVITY_SDBCX_VINDEXCOLUMN_HXX

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/sdbcx/VColumn.hxx>

namespace connectivity
{
    namespace sdbcx
    {
        class OIndexColumn;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OIndexColumn> OIndexColumn_PROP;

        class OOO_DLLPUBLIC_DBTOOLS OIndexColumn :
            public OColumn, public OIndexColumn_PROP
        {
            bool    m_IsAscending;
        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        public:
            OIndexColumn(   bool _bCase);
            OIndexColumn(   bool IsAscending,
                            const OUString& Name,
                            const OUString& TypeName,
                            const OUString& DefaultValue,
                            sal_Int32       IsNullable,
                            sal_Int32       Precision,
                            sal_Int32       Scale,
                            sal_Int32       Type,
                            bool            _bCase,
                            const OUString& CatalogName,
                            const OUString& SchemaName,
                            const OUString& TableName);

            virtual void construct() override;
            DECLARE_SERVICE_INFO();
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SDBCX_VINDEXCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
