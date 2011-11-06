/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "TPrivilegesResultSet.hxx"

using namespace connectivity;
//------------------------------------------------------------------------------
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//------------------------------------------------------------------------------
OResultSetPrivileges::OResultSetPrivileges( const Reference< XDatabaseMetaData>& _rxMeta
                                           , const Any& catalog
                                           , const ::rtl::OUString& schemaPattern
                                           , const ::rtl::OUString& tableNamePattern)
                                           : ODatabaseMetaDataResultSet(eTablePrivileges)
                                           , m_bResetValues(sal_True)
{
    osl_incrementInterlockedCount( &m_refCount );
    {
        ::rtl::OUString sUserWorkingFor;
        static Sequence< ::rtl::OUString > sTableTypes;
        if ( sTableTypes.getLength() == 0 )
        {
            // we want all catalogues, all schemas, all tables
            sTableTypes.realloc(3);
            sTableTypes[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
            sTableTypes[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE"));
            sTableTypes[2] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")); // just to be sure to include anything else ....
        }
        try
        {
            m_xTables = _rxMeta->getTables(catalog,schemaPattern,tableNamePattern,sTableTypes);
            m_xRow = Reference< XRow>(m_xTables,UNO_QUERY);

            sUserWorkingFor = _rxMeta->getUserName();
        }
        catch(Exception&)
        {
        }

        ODatabaseMetaDataResultSet::ORows aRows;
        static ODatabaseMetaDataResultSet::ORow aRow(8);
        aRow[5] = new ORowSetValueDecorator(sUserWorkingFor);
        aRow[6] = ODatabaseMetaDataResultSet::getSelectValue();
        aRow[7] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("YES")));
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
        aRow[6] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REFERENCE")));
        aRows.push_back(aRow);

        setRows(aRows);
    }
    osl_decrementInterlockedCount( &m_refCount );
}
//------------------------------------------------------------------------------
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

                m_bResetValues = sal_False;
            }
    }
    return ODatabaseMetaDataResultSet::getValue(columnIndex);
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSetPrivileges::disposing(void)
{
    ODatabaseMetaDataResultSet::disposing();
m_xTables.clear();
m_xRow.clear();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetPrivileges::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );

    sal_Bool bReturn = sal_False;
    if ( m_xTables.is() )
    {
        if ( m_bBOF )
        {
            m_bResetValues = sal_True;
            if ( !m_xTables->next() )
                return sal_False;
        }

        bReturn = ODatabaseMetaDataResultSet::next();
        if ( !bReturn )
        {
            m_bBOF = sal_False;
            m_bResetValues = bReturn = m_xTables->next();
        }
    }
    return bReturn;
}
// -----------------------------------------------------------------------------
