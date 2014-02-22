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

#include "file/FTables.hxx"
#include "file/FTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include "file/FCatalog.hxx"
#include "file/FConnection.hxx"

using namespace connectivity;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

sdbcx::ObjectType OTables::createObject(const OUString& /*_rName*/)
{
    return sdbcx::ObjectType();
}

void OTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OFileCatalog&>(m_rParent).refreshTables();
}

void OTables::disposing(void)
{
m_xMetaData.clear();
    OCollection::disposing();
}

Any SAL_CALL OTables::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if( rType == ::getCppuType((const Reference<XColumnLocate>*)0) ||
        rType == ::getCppuType((const Reference<XDataDescriptorFactory>*)0) ||
        rType == ::getCppuType((const Reference<XAppend>*)0) ||
        rType == ::getCppuType((const Reference<XDrop>*)0))
        return Any();

    typedef sdbcx::OCollection OTables_BASE;
    return OTables_BASE::queryInterface(rType);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
