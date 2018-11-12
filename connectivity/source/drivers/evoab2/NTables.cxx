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

#include "NTables.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <connectivity/sdbcx/VTable.hxx>
#include "NCatalog.hxx"
#include "NConnection.hxx"
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/types.hxx>
#include "NTable.hxx"
using namespace ::comphelper;

using namespace ::cppu;
using namespace connectivity::evoab;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;

ObjectType OEvoabTables::createObject(const OUString& aName)
{
    Sequence< OUString > aTypes { "TABLE" };

    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),"%",aName,aTypes);

    ObjectType xRet;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one table with this name
        {
            OEvoabTable* pRet = new OEvoabTable(
                    this,
                    static_cast<OEvoabCatalog&>(m_rParent).getConnection(),
                    aName,
                    xRow->getString(4),
                    xRow->getString(5),
                    "",
                    "");
            xRet = pRet;
        }
    }

    ::comphelper::disposeComponent(xResult);

    return xRet;
}

void OEvoabTables::impl_refresh(  )
{
    static_cast<OEvoabCatalog&>(m_rParent).refreshTables();
}

void OEvoabTables::disposing()
{
    m_xMetaData.clear();
    OCollection::disposing();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
