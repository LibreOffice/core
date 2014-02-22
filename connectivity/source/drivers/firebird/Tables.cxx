/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "Table.hxx"
#include "Tables.hxx"

#include <connectivity/dbtools.hxx>

#include <com/sun/star/sdbc/XRow.hpp>

using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;


Tables::Tables(const uno::Reference< XDatabaseMetaData >& rMetaData,
               OWeakObject& rParent,
               Mutex& rMutex,
               TStringVector& rNames) :
    OCollection(rParent,
                sal_True,
                rMutex,
                rNames),
    m_rMutex(rMutex),
    m_xMetaData(rMetaData)
{
}


void Tables::impl_refresh()
    throw(RuntimeException)
{
    
}

ObjectType Tables::createObject(const OUString& rName)
{
    
    uno::Reference< XResultSet > xTables = m_xMetaData->getTables(Any(),
                                                                  OUString(),
                                                                  rName,
                                                                  uno::Sequence< OUString >());

    if (!xTables.is())
        throw RuntimeException();

    uno::Reference< XRow > xRow(xTables,UNO_QUERY);

    if (!xRow.is() || !xTables->next())
        throw RuntimeException();

    ObjectType xRet(new Table(this,
                              m_rMutex,
                              m_xMetaData->getConnection(),
                              xRow->getString(3), 
                              xRow->getString(4), 
                              xRow->getString(5))); 

    if (xTables->next())
        throw RuntimeException(); 

    return xRet;
}

uno::Reference< XPropertySet > Tables::createDescriptor()
{
    
    
    
    return new Table(this, m_rMutex, m_xMetaData->getConnection());
}


ObjectType Tables::appendObject(const OUString& rName,
                                const uno::Reference< XPropertySet >& rDescriptor)
{
    OUString sSql(::dbtools::createSqlCreateTableStatement(rDescriptor,
                                                            m_xMetaData->getConnection()));
    m_xMetaData->getConnection()->createStatement()->execute(sSql);

    return createObject(rName);
}


void Tables::dropObject(sal_Int32 nPosition, const OUString sName)
{
    uno::Reference< XPropertySet > xTable(getObject(nPosition));

    if (!ODescriptor::isNew(xTable))
    {
        OUStringBuffer sSql("DROP ");

        OUString sType;
        xTable->getPropertyValue("Type") >>= sType;
        sSql.append(sType);

        const OUString sQuoteString = m_xMetaData->getIdentifierQuoteString();
        sSql.append(::dbtools::quoteName(sQuoteString,sName));

        m_xMetaData->getConnection()->createStatement()->execute(sSql.makeStringAndClear());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */