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

#include <mysql/YCatalog.hxx>
#include <mysql/YUsers.hxx>
#include <mysql/YTables.hxx>
#include <mysql/YViews.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <comphelper/types.hxx>

using namespace connectivity;
using namespace connectivity::mysql;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OMySQLCatalog::OMySQLCatalog(const Reference<XConnection>& _xConnection)
    : OCatalog(_xConnection)
    , m_xConnection(_xConnection)
{
}

void OMySQLCatalog::refreshObjects(const Sequence<OUString>& _sKindOfObject,
                                   ::std::vector<OUString>& _rNames)
{
    Reference<XResultSet> xResult = m_xMetaData->getTables(Any(), "%", "%", _sKindOfObject);
    fillNames(xResult, _rNames);
}

void OMySQLCatalog::refreshTables()
{
    ::std::vector<OUString> aVector;

    Sequence<OUString> sTableTypes(3);
    sTableTypes[0] = "VIEW";
    sTableTypes[1] = "TABLE";
    sTableTypes[2] = "%"; // just to be sure to include anything else ....

    refreshObjects(sTableTypes, aVector);

    if (m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables.reset(new OTables(m_xMetaData, *this, m_aMutex, aVector));
}

void OMySQLCatalog::refreshViews()
{
    Sequence<OUString> aTypes{ "VIEW" };

    // let's simply assume the server is new enough to support views. Current drivers
    // as of this writing might not return the proper information in getTableTypes, so
    // don't rely on it.

    ::std::vector<OUString> aVector;
    refreshObjects(aTypes, aVector);

    if (m_pViews)
        m_pViews->reFill(aVector);
    else
        m_pViews.reset(new OViews(m_xMetaData, *this, m_aMutex, aVector));
}

void OMySQLCatalog::refreshGroups() {}

void OMySQLCatalog::refreshUsers()
{
    ::std::vector<OUString> aVector;
    Reference<XStatement> xStmt = m_xConnection->createStatement();
    Reference<XResultSet> xResult = xStmt->executeQuery(
        "SELECT grantee FROM information_schema.user_privileges GROUP BY grantee");
    if (xResult.is())
    {
        Reference<XRow> xRow(xResult, UNO_QUERY);
        while (xResult->next())
            aVector.push_back(xRow->getString(1));
        ::comphelper::disposeComponent(xResult);
    }
    ::comphelper::disposeComponent(xStmt);

    if (m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers.reset(new OUsers(*this, m_aMutex, aVector, m_xConnection, this));
}

Any SAL_CALL OMySQLCatalog::queryInterface(const Type& rType)
{
    if (rType == cppu::UnoType<XGroupsSupplier>::get())
        return Any();

    return OCatalog::queryInterface(rType);
}

Sequence<Type> SAL_CALL OMySQLCatalog::getTypes()
{
    Sequence<Type> aTypes = OCatalog::getTypes();
    std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for (; pBegin != pEnd; ++pBegin)
    {
        if (*pBegin != cppu::UnoType<XGroupsSupplier>::get())
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    return Sequence<Type>(aOwnTypes.data(), aOwnTypes.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
