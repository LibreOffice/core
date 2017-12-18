/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_HCOLUMN_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_HCOLUMN_HXX
#include <connectivity/TColumnsHelper.hxx>
#include <connectivity/sdbcx/VColumn.hxx>

namespace connectivity
{
    namespace firebird
    {
        class Column;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<Column> Column_PROP;
        class Column : public sdbcx::OColumn,
                           public Column_PROP
        {
            OUString m_sAutoIncrement;
        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
        public:
            Column();
            virtual void construct() override;
            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_HSQLDB_HCOLUMNS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
