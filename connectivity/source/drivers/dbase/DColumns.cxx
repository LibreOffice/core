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

#include <dbase/DColumns.hxx>
#include <dbase/DTable.hxx>
#include <connectivity/sdbcx/VColumn.hxx>

using namespace connectivity::dbase;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

sdbcx::ObjectType ODbaseColumns::createObject(const OUString& _rName)
{
    ODbaseTable* pTable = static_cast<ODbaseTable*>(m_pTable);

    const ::rtl::Reference<OSQLColumns>& aCols = pTable->getTableColumns();
    OSQLColumns::const_iterator aIter = find(aCols->begin(),aCols->end(),_rName,::comphelper::UStringMixEqual(isCaseSensitive()));

    sdbcx::ObjectType xRet;
    if(aIter != aCols->end())
        xRet = sdbcx::ObjectType(*aIter,UNO_QUERY);
    return xRet;
}


void ODbaseColumns::impl_refresh()
{
    m_pTable->refreshColumns();
}

Reference< XPropertySet > ODbaseColumns::createDescriptor()
{
    return new sdbcx::OColumn(isCaseSensitive());
}


// XAppend
sdbcx::ObjectType ODbaseColumns::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    if ( m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    m_pTable->addColumn( descriptor );
    return createObject( _rForName );
}


// XDrop
void ODbaseColumns::dropObject(sal_Int32 _nPos, const OUString& /*_sElementName*/)
{
    if(!m_pTable->isNew())
        m_pTable->dropColumn(_nPos);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
