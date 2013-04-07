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

#include "flat/EColumns.hxx"
#include "flat/ETable.hxx"
#include "connectivity/sdbcx/VColumn.hxx"

using namespace connectivity::flat;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;


sdbcx::ObjectType OFlatColumns::createObject(const OUString& _rName)
{

    OFlatTable* pTable = (OFlatTable*)m_pTable;
    ::rtl::Reference<OSQLColumns> aCols = pTable->getTableColumns();
    OSQLColumns::Vector::const_iterator aIter = find(aCols->get().begin(),aCols->get().end(),_rName,::comphelper::UStringMixEqual(isCaseSensitive()));
    sdbcx::ObjectType xRet;
    if(aIter != aCols->get().end())
        xRet = sdbcx::ObjectType(*aIter,UNO_QUERY);
    return xRet;
}
// -------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
