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
#ifndef CONNECTIVITY_MYSQL_COLUMN_HXX
#define CONNECTIVITY_MYSQL_COLUMN_HXX
#include "connectivity/TColumnsHelper.hxx"
#include "connectivity/sdbcx/VColumn.hxx"

namespace connectivity
{
    namespace mysql
    {
        class OMySQLColumns : public OColumnsHelper
        {
        protected:
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
        public:
            OMySQLColumns(  ::cppu::OWeakObject& _rParent
                            ,sal_Bool _bCase
                            ,::osl::Mutex& _rMutex
                            ,const TStringVector &_rVector
                            ,sal_Bool _bUseHardRef = sal_True
                        );
        };

        class OMySQLColumn;
        typedef sdbcx::OColumn OMySQLColumn_BASE;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OMySQLColumn> OMySQLColumn_PROP;

        class OMySQLColumn :    public OMySQLColumn_BASE,
                                public OMySQLColumn_PROP
        {
            ::rtl::OUString m_sAutoIncrement;
        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        public:
            OMySQLColumn(sal_Bool _bCase);
            virtual void construct();

            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // CONNECTIVITY_MYSQL_COLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
