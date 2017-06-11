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

#include "ado/ATables.hxx"
#include "ado/ATable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include "ado/ACatalog.hxx"
#include "ado/AConnection.hxx"
#include "ado/Awrapado.hxx"
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <connectivity/dbexception.hxx>
#include "strings.hrc"

using namespace ::cppu;
using namespace connectivity;
using namespace comphelper;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

sdbcx::ObjectType OTables::createObject(const OUString& _rName)
{
    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    return new OAdoTable(this,isCaseSensitive(),m_pCatalog,m_aCollection.GetItem(_rName));
}

void OTables::impl_refresh(  )
{
    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    m_aCollection.Refresh();
    m_pCatalog->refreshTables();
}

Reference< XPropertySet > OTables::createDescriptor()
{
    return new OAdoTable(this,isCaseSensitive(),m_pCatalog);
}

// XAppend
sdbcx::ObjectType OTables::appendObject( const OUString&, const Reference< XPropertySet >& descriptor )
{
    OAdoTable* pTable = nullptr;
    if ( !getImplementation( pTable, descriptor ) || pTable == nullptr )
        m_pCatalog->getConnection()->throwGenericSQLException( STR_INVALID_TABLE_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    if(!m_aCollection.Append(pTable->getImpl()))
        ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),static_cast<XTypeProvider*>(this));
    m_aCollection.Refresh();

    return new OAdoTable(this,isCaseSensitive(),m_pCatalog,pTable->getImpl());
}

// XDrop
void OTables::dropObject(sal_Int32 /*_nPos*/,const OUString& _sElementName)
{
    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    if ( !m_aCollection.Delete(_sElementName) )
        ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),static_cast<XTypeProvider*>(this));
}

void OTables::appendNew(const OUString& _rsNewTable)
{
    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    m_aCollection.Refresh();

    insertElement(_rsNewTable,nullptr);

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rsNewTable), Any(), Any());
    OInterfaceIteratorHelper2 aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
