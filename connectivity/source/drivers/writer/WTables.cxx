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

#include <writer/WTables.hxx>

#include <sal/config.h>

#include <comphelper/types.hxx>

#include <writer/WConnection.hxx>
#include <file/FCatalog.hxx>
#include <file/FConnection.hxx>
#include <writer/WCatalog.hxx>
#include <writer/WTable.hxx>

using namespace ::com::sun::star;

namespace connectivity
{
namespace writer
{

sdbcx::ObjectType OWriterTables::createObject(const OUString& rName)
{
    OWriterTable* pTable = new OWriterTable(this, static_cast<OWriterConnection*>(static_cast<file::OFileCatalog&>(m_rParent).getConnection()), rName, "TABLE");
    sdbcx::ObjectType xRet = pTable;
    pTable->construct();
    return xRet;
}

} // namespace writer
} // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
