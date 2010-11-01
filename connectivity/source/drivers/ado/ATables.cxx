/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
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
#include "resource/ado_res.hrc"

using namespace ::cppu;
using namespace connectivity;
using namespace comphelper;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;

typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OTables::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    return new OAdoTable(this,isCaseSensitive(),m_pCatalog,m_aCollection.GetItem(_rName));
}
// -------------------------------------------------------------------------
void OTables::impl_refresh(  ) throw(RuntimeException)
{
    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    m_aCollection.Refresh();
    m_pCatalog->refreshTables();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OTables::createDescriptor()
{
    return new OAdoTable(this,isCaseSensitive(),m_pCatalog);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OTables::appendObject( const ::rtl::OUString&, const Reference< XPropertySet >& descriptor )
{
    OAdoTable* pTable = NULL;
    if ( !getImplementation( pTable, descriptor ) || pTable == NULL )
        m_pCatalog->getConnection()->throwGenericSQLException( STR_INVALID_TABLE_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    if(!m_aCollection.Append(pTable->getImpl()))
        ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),static_cast<XTypeProvider*>(this));
    m_aCollection.Refresh();

    return new OAdoTable(this,isCaseSensitive(),m_pCatalog,pTable->getImpl());
}
// -------------------------------------------------------------------------
// XDrop
void OTables::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    if ( !m_aCollection.Delete(_sElementName) )
        ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),static_cast<XTypeProvider*>(this));
}
// -----------------------------------------------------------------------------
void OTables::appendNew(const ::rtl::OUString& _rsNewTable)
{
    OSL_ENSURE(m_aCollection.IsValid(),"Collection isn't valid");
    m_aCollection.Refresh();

    insertElement(_rsNewTable,NULL);

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rsNewTable), Any(), Any());
    OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
