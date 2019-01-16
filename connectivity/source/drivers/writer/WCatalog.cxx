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

#include <writer/WCatalog.hxx>
#include <writer/WTables.hxx>

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>

#include <connectivity/sdbcx/VCollection.hxx>

#include <writer/WConnection.hxx>

using namespace ::com::sun::star;

namespace connectivity
{
namespace writer
{
OWriterCatalog::OWriterCatalog(OWriterConnection* pConnection)
    : file::OFileCatalog(pConnection)
{
}

void OWriterCatalog::refreshTables()
{
    ::std::vector<OUString> aVector;
    uno::Sequence<OUString> aTypes;
    OWriterConnection::ODocHolder aDocHolder(static_cast<OWriterConnection*>(m_pConnection));
    uno::Reference<sdbc::XResultSet> xResult = m_xMetaData->getTables(uno::Any(), "%", "%", aTypes);

    if (xResult.is())
    {
        uno::Reference<sdbc::XRow> xRow(xResult, uno::UNO_QUERY);
        while (xResult->next())
            aVector.push_back(xRow->getString(3));
    }
    if (m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = std::make_unique<OWriterTables>(m_xMetaData, *this, m_aMutex, aVector);
}

} // namespace writer
} // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
