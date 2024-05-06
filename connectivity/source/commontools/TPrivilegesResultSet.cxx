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

#include <TPrivilegesResultSet.hxx>

using namespace connectivity;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OResultSetPrivileges::OResultSetPrivileges( const Reference< XDatabaseMetaData>& _rxMeta
                                           , const Any& catalog
                                           , const OUString& schemaPattern
                                           , const OUString& tableNamePattern)
                                           : ODatabaseMetaDataResultSet(eTablePrivileges)
                                           , m_bResetValues(true)
{
    osl_atomic_increment( &m_refCount );
    {
        OUString sUserWorkingFor;
        // we want all catalogues, all schemas, all tables
        Sequence< OUString > sTableTypes {u"VIEW"_ustr, u"TABLE"_ustr, u"%"_ustr}; // this last one is just to be sure to include anything else...
        try
        {
            m_xTables = _rxMeta->getTables(catalog,schemaPattern,tableNamePattern,sTableTypes);
            m_xRow.set(m_xTables,UNO_QUERY);

            sUserWorkingFor = _rxMeta->getUserName();
        }
        catch(Exception&)
        {
        }

        ODatabaseMetaDataResultSet::ORows aRows;
        ODatabaseMetaDataResultSet::ORow aRow(8);
        aRow[5] = new ORowSetValueDecorator(sUserWorkingFor);
        aRow[6] = ODatabaseMetaDataResultSet::getSelectValue();
        aRow[7] = new ORowSetValueDecorator(u"YES"_ustr);
        aRows.push_back(aRow);
        aRow[6] = ODatabaseMetaDataResultSet::getInsertValue();
        aRows.push_back(aRow);
        aRow[6] = ODatabaseMetaDataResultSet::getDeleteValue();
        aRows.push_back(aRow);
        aRow[6] = ODatabaseMetaDataResultSet::getUpdateValue();
        aRows.push_back(aRow);
        aRow[6] = ODatabaseMetaDataResultSet::getCreateValue();
        aRows.push_back(aRow);
        aRow[6] = ODatabaseMetaDataResultSet::getReadValue();
        aRows.push_back(aRow);
        aRow[6] = ODatabaseMetaDataResultSet::getAlterValue();
        aRows.push_back(aRow);
        aRow[6] = ODatabaseMetaDataResultSet::getDropValue();
        aRows.push_back(aRow);
        aRow[6] = new ORowSetValueDecorator(u"REFERENCE"_ustr);
        aRows.push_back(aRow);

        setRows(std::move(aRows));
    }
    osl_atomic_decrement( &m_refCount );
}

const ORowSetValue& OResultSetPrivileges::getValue(sal_Int32 columnIndex)
{
    switch(columnIndex)
    {
        case 1:
        case 2:
        case 3:
            if ( m_xRow.is() && m_bResetValues )
            {
                (*m_aRowsIter)[1] = new ORowSetValueDecorator(m_xRow->getString(1));
                if ( m_xRow->wasNull() )
                    (*m_aRowsIter)[1]->setNull();
                (*m_aRowsIter)[2] = new ORowSetValueDecorator(m_xRow->getString(2));
                if ( m_xRow->wasNull() )
                    (*m_aRowsIter)[2]->setNull();
                (*m_aRowsIter)[3] = new ORowSetValueDecorator(m_xRow->getString(3));
                if ( m_xRow->wasNull() )
                    (*m_aRowsIter)[3]->setNull();

                m_bResetValues = false;
            }
    }
    return ODatabaseMetaDataResultSet::getValue(columnIndex);
}

void OResultSetPrivileges::disposing(std::unique_lock<std::mutex>& rGuard)
{
    ODatabaseMetaDataResultSet::disposing(rGuard);
    m_xTables.clear();
    m_xRow.clear();
}

sal_Bool SAL_CALL OResultSetPrivileges::next(  )
{
    std::unique_lock aGuard( m_aMutex );
    throwIfDisposed(aGuard);

    bool bReturn = false;
    if ( m_xTables.is() )
    {
        if ( m_bBOF )
        {
            m_bResetValues = true;
            if ( !m_xTables->next() )
                return false;
        }

        bReturn = ODatabaseMetaDataResultSet::next(aGuard);
        if ( !bReturn )
        {
            m_bBOF = false;
            m_bResetValues = bReturn = m_xTables->next();
        }
    }
    return bReturn;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
