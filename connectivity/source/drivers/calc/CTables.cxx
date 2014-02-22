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

#include "calc/CTables.hxx"
#include "calc/CTable.hxx"
#include "file/FCatalog.hxx"
#include "file/FConnection.hxx"
#include "calc/CCatalog.hxx"
#include <comphelper/types.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::calc;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

sdbcx::ObjectType OCalcTables::createObject(const OUString& _rName)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcTables::createObject" );
    OCalcTable* pTable = new OCalcTable(this,(OCalcConnection*)static_cast<OFileCatalog&>(m_rParent).getConnection(),
                                        _rName,OUString("TABLE"));
    sdbcx::ObjectType xRet = pTable;
    pTable->construct();
    return xRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
