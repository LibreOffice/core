/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <connectivity/sdbcx/VColumn.hxx>

namespace connectivity::firebird
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
