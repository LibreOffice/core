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

#include <sal/config.h>

#include <dbase/DConnection.hxx>
#include <dbase/DTables.hxx>
#include <dbase/DTable.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <file/FCatalog.hxx>
#include <file/FConnection.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <dbase/DCatalog.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <strings.hrc>
#include <connectivity/dbexception.hxx>

using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

sdbcx::ObjectType ODbaseTables::createObject(const OUString& _rName)
{
    rtl::Reference<ODbaseTable> pRet = new ODbaseTable(this, static_cast<ODbaseConnection*>(static_cast<OFileCatalog&>(m_rParent).getConnection()),
                                        _rName,"TABLE");

    pRet->construct();
    return pRet;
}

void ODbaseTables::impl_refresh(  )
{
    static_cast<ODbaseCatalog*>(&m_rParent)->refreshTables();
}

Reference< XPropertySet > ODbaseTables::createDescriptor()
{
    return new ODbaseTable(this, static_cast<ODbaseConnection*>(static_cast<OFileCatalog&>(m_rParent).getConnection()));
}

// XAppend
sdbcx::ObjectType ODbaseTables::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    auto pTable = dynamic_cast<ODbaseTable*>(descriptor.get());
    if(pTable)
    {
        pTable->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),Any(_rForName));
        try
        {
            if(!pTable->CreateImpl())
                throw SQLException();
        }
        catch(SQLException&)
        {
            throw;
        }
        catch(Exception& ex)
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw SQLException( ex.Message, nullptr, "", 0, anyEx );
        }
    }
    return createObject( _rForName );
}

// XDrop
void ODbaseTables::dropObject(sal_Int32 _nPos, const OUString& _sElementName)
{
    Reference< XInterface > xTunnel;
    try
    {
        xTunnel.set(getObject(_nPos),UNO_QUERY);
    }
    catch(const Exception&)
    {
        if(ODbaseTable::Drop_Static(ODbaseTable::getEntry(static_cast<OFileCatalog&>(m_rParent).getConnection(),_sElementName),false,nullptr))
            return;
    }

    if ( xTunnel.is() )
    {
        ODbaseTable* pTable = dynamic_cast<ODbaseTable*>(xTunnel.get());
        if(pTable)
            pTable->DropImpl();
    }
    else
    {
        const OUString sError( static_cast<OFileCatalog&>(m_rParent).getConnection()->getResources().getResourceStringWithSubstitution(
                    STR_TABLE_NOT_DROP,
                    "$tablename$", _sElementName
                 ) );
        ::dbtools::throwGenericSQLException( sError, nullptr );
    }
}

Any SAL_CALL ODbaseTables::queryInterface( const Type & rType )
{
    typedef sdbcx::OCollection OTables_BASE;
    return OTables_BASE::queryInterface(rType);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
