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
#include "dbase/DTables.hxx"
#include "dbase/DTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include "file/FCatalog.hxx"
#include "file/FConnection.hxx"
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include "dbase/DCatalog.hxx"
#include <comphelper/types.hxx>
#include "resource/dbase_res.hrc"
#include "connectivity/dbexception.hxx"

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
namespace starutil      = ::com::sun::star::util;

sdbcx::ObjectType ODbaseTables::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString aName,aSchema;
    ODbaseTable* pRet = new ODbaseTable(this,(ODbaseConnection*)static_cast<OFileCatalog&>(m_rParent).getConnection(),
                                        _rName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE")));

    sdbcx::ObjectType xRet = pRet;
    pRet->construct();
    return xRet;
}
// -------------------------------------------------------------------------
void ODbaseTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<ODbaseCatalog*>(&m_rParent)->refreshTables();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > ODbaseTables::createDescriptor()
{
    return new ODbaseTable(this,(ODbaseConnection*)static_cast<OFileCatalog&>(m_rParent).getConnection());
}
typedef connectivity::sdbcx::OCollection ODbaseTables_BASE_BASE;
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType ODbaseTables::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    Reference<XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        ODbaseTable* pTable = reinterpret_cast< ODbaseTable* >( xTunnel->getSomething(ODbaseTable::getUnoTunnelImplementationId()) );
        if(pTable)
        {
            pTable->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),makeAny(_rForName));
            try
            {
                if(!pTable->CreateImpl())
                    throw SQLException();
            }
            catch(SQLException&)
            {
                throw;
            }
            catch(Exception&)
            {
                throw SQLException();
            }
        }
    }
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void ODbaseTables::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference< XUnoTunnel> xTunnel;
    try
    {
        xTunnel.set(getObject(_nPos),UNO_QUERY);
    }
    catch(const Exception&)
    {
        if(ODbaseTable::Drop_Static(ODbaseTable::getEntry(static_cast<OFileCatalog&>(m_rParent).getConnection(),_sElementName),sal_False,NULL))
            return;
    }

    if ( xTunnel.is() )
    {
        ODbaseTable* pTable = reinterpret_cast< ODbaseTable* >( xTunnel->getSomething(ODbaseTable::getUnoTunnelImplementationId()) );
        if(pTable)
            pTable->DropImpl();
    }
    else
    {
        const ::rtl::OUString sError( static_cast<OFileCatalog&>(m_rParent).getConnection()->getResources().getResourceStringWithSubstitution(
                    STR_TABLE_NOT_DROP,
                    "$tablename$", _sElementName
                 ) );
        ::dbtools::throwGenericSQLException( sError, NULL );
    }
}
// -------------------------------------------------------------------------
Any SAL_CALL ODbaseTables::queryInterface( const Type & rType ) throw(RuntimeException)
{
    typedef sdbcx::OCollection OTables_BASE;
    return OTables_BASE::queryInterface(rType);
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
