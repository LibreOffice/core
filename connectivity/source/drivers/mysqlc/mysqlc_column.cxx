/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mysqlc_column.hxx"

#include <TConnection.hxx>

connectivity::mysqlc::Column::Column()
    : OColumn(true) // case sensitive
{
    construct();
}

void connectivity::mysqlc::Column::construct()
{
    m_sAutoIncrement = "auto_increment";
    registerProperty(
        OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION),
        PROPERTY_ID_AUTOINCREMENTCREATION, 0, &m_sAutoIncrement,
        cppu::UnoType<decltype(m_sAutoIncrement)>::get());
}

::cppu::IPropertyArrayHelper*
    connectivity::mysqlc::Column::createArrayHelper(sal_Int32 /*_nId*/) const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper& SAL_CALL connectivity::mysqlc::Column::getInfoHelper()
{
    return *Column_PROP::getArrayHelper(isNew() ? 1 : 0);
}

css::uno::Sequence<OUString> SAL_CALL connectivity::mysqlc::Column::getSupportedServiceNames()
{
    return { u"com.sun.star.sdbcx.Column"_ustr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
