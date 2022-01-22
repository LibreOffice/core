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

#include "mysqlc_view.hxx"

#include <propertyids.hxx>

#include <com/sun/star/sdbc/XRow.hpp>

namespace connectivity::mysqlc
{
View::View(const css::uno::Reference<css::sdbc::XConnection>& _rxConnection, bool _bCaseSensitive,
           const OUString& _rSchemaName, const OUString& _rName)
    : View_Base(_bCaseSensitive, _rName, _rxConnection->getMetaData(), OUString(), _rSchemaName,
                OUString())
    , m_xConnection(_rxConnection)
{
}

View::~View() {}

void SAL_CALL View::acquire() noexcept { View_Base::acquire(); };
void SAL_CALL View::release() noexcept { View_Base::release(); };
css::uno::Any SAL_CALL View::queryInterface(const css::uno::Type& _rType)
{
    css::uno::Any aReturn = View_Base::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = View_IBASE::queryInterface(_rType);
    return aReturn;
}

css::uno::Sequence<css::uno::Type> SAL_CALL View::getTypes()
{
    return ::comphelper::concatSequences(View_Base::getTypes(), View_IBASE::getTypes());
}

css::uno::Sequence<sal_Int8> SAL_CALL View::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL View::alterCommand(const OUString& _rNewCommand)
{
    OUString aCommand = "ALTER VIEW " + m_SchemaName + "." + m_Name + " AS " + _rNewCommand;
    m_xMetaData->getConnection()->createStatement()->execute(aCommand);
}

void SAL_CALL View::getFastPropertyValue(css::uno::Any& _rValue, sal_Int32 _nHandle) const
{
    if (_nHandle == PROPERTY_ID_COMMAND)
    {
        // retrieve the very current command, don't rely on the base classes cached value
        // (which we initialized empty, anyway)
        _rValue <<= impl_getCommand();
        return;
    }

    View_Base::getFastPropertyValue(_rValue, _nHandle);
}

OUString View::impl_getCommand() const
{
    OUString aCommand("SELECT VIEW_DEFINITION FROM INFORMATION_SCHEMA.VIEWS WHERE TABLE_SCHEMA = '"
                      + m_SchemaName + "' AND TABLE_NAME = '" + m_Name + "'");
    //::utl::SharedUNOComponent< XStatement > xStatement; xStatement.set( m_xConnection->createStatement(), UNO_QUERY_THROW );
    css::uno::Reference<css::sdbc::XResultSet> xResult(
        m_xMetaData->getConnection()->createStatement()->executeQuery(aCommand),
        css::uno::UNO_SET_THROW);
    if (!xResult->next())
    {
        // hmm. There is no view the name as we know it. Can only mean some other instance
        // dropped this view meanwhile...
        std::abort();
    }

    css::uno::Reference<css::sdbc::XRow> xRow(xResult, css::uno::UNO_QUERY_THROW);
    return xRow->getString(1);
}

} // namespace connectivity::mysqlc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
