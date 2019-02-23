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


#include "MacabTables.hxx"
#include "MacabTable.hxx"
#include "MacabCatalog.hxx"
#include "MacabConnection.hxx"
#include <comphelper/types.hxx>
#include <com/sun/star/sdbc/XRow.hpp>

using namespace connectivity::macab;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

sdbcx::ObjectType MacabTables::createObject(const OUString& _rName)
{
    OUString aName,aSchema;
    aSchema = "%";
    aName = _rName;

    Sequence< OUString > aTypes { "%" };

    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(), aSchema, aName, aTypes);

    sdbcx::ObjectType xRet;
    if (xResult.is())
    {
        Reference< XRow > xRow(xResult, UNO_QUERY);
        if (xResult->next()) // there can be only one table with this name
        {
            MacabTable* pRet = new MacabTable(
                    this,
                    static_cast<MacabCatalog&>(m_rParent).getConnection(),
                    aName,
                    xRow->getString(4),
                    xRow->getString(5),
                    "");
            xRet = pRet;
        }
    }
    ::comphelper::disposeComponent(xResult);

    return xRet;
}

void MacabTables::impl_refresh(  )
{
    static_cast<MacabCatalog&>(m_rParent).refreshTables();
}

void MacabTables::disposing()
{
    m_xMetaData.clear();
    OCollection::disposing();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
