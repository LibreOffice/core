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
                            sal_Bool        _bCase);
            // just to make it not inline
            virtual ~OKeyColumn();

            virtual void construct();
            DECLARE_SERVICE_INFO();
        };
    }
}
#endif // _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
