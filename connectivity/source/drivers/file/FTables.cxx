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

#include <file/FTables.hxx>
#include <file/FCatalog.hxx>

using namespace connectivity;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;

sdbcx::ObjectType OTables::createObject(const OUString& /*_rName*/)
{
    return sdbcx::ObjectType();
}

void OTables::impl_refresh(  )
{
    static_cast<OFileCatalog&>(m_rParent).refreshTables();
}

Any SAL_CALL OTables::queryInterface( const Type & rType )
{
    if( rType == cppu::UnoType<XColumnLocate>::get()||
        rType == cppu::UnoType<XDataDescriptorFactory>::get()||
        rType == cppu::UnoType<XAppend>::get()||
        rType == cppu::UnoType<XDrop>::get())
        return Any();

    typedef sdbcx::OCollection OTables_BASE;
    return OTables_BASE::queryInterface(rType);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
