/*************************************************************************
 *
 *  $RCSfile: TPrivilegesResultSet.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 14:38:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CONNECTIVITY_PRIVILEGESRESULTSET_HXX
#include "TPrivilegesResultSet.hxx"
#endif

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
                                           : m_bResetValues(sal_True)
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

        setTablePrivilegesMap();
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
    m_xTables = NULL;
    m_xRow = NULL;
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

        if ( !(bReturn = ODatabaseMetaDataResultSet::next()) )
        {
            m_bBOF = sal_False;
            ODatabaseMetaDataResultSet::next();
            m_bResetValues = bReturn = m_xTables->next();
        }
    }
    return bReturn;
}
// -----------------------------------------------------------------------------
