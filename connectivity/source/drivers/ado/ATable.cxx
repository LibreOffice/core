/*************************************************************************
 *
 *  $RCSfile: ATable.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:58:09 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _CONNECTIVITY_ADO_TABLE_HXX_
#include "ado/ATable.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_INDEXES_HXX_
#include "ado/AIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#include "ado/AColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_COLUMN_HXX_
#include "ado/AColumn.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_KEYS_HXX_
#include "ado/AKeys.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;

// -------------------------------------------------------------------------
OAdoTable::OAdoTable(sdbcx::OCollection* _pTables,sal_Bool _bCase,OCatalog* _pCatalog,_ADOTable* _pTable)
    : OTable_TYPEDEF(_pTables,_bCase,::rtl::OUString(),::rtl::OUString())
    ,m_pCatalog(_pCatalog)
{
    construct();
    m_aTable = WpADOTable(_pTable);
    //  m_aTable.putref_ParentCatalog(_pCatalog->getCatalog());
    fillPropertyValues();

}
// -----------------------------------------------------------------------------
OAdoTable::OAdoTable(sdbcx::OCollection* _pTables,sal_Bool _bCase,OCatalog* _pCatalog)
    : OTable_TYPEDEF(_pTables,_bCase)
    ,m_pCatalog(_pCatalog)
{
    construct();
    m_aTable.Create();
    m_aTable.putref_ParentCatalog(_pCatalog->getCatalog());

}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoTable::disposing(void)
{
    OTable_TYPEDEF::disposing();
    m_aTable.clear();
}
// -------------------------------------------------------------------------
void OAdoTable::refreshColumns()
{
    TStringVector aVector;

    WpADOColumns aColumns;
    if ( m_aTable.IsValid() )
    {
        aColumns = m_aTable.get_Columns();
        aColumns.fillElementNames(aVector);
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns = new OColumns(*this,m_aMutex,aVector,aColumns,isCaseSensitive(),m_pCatalog->getConnection());
}
// -------------------------------------------------------------------------
void OAdoTable::refreshKeys()
{
    TStringVector aVector;

    WpADOKeys aKeys;
    if(m_aTable.IsValid())
    {
        aKeys = m_aTable.get_Keys();
        aKeys.fillElementNames(aVector);
    }

    if(m_pKeys)
        m_pKeys->reFill(aVector);
    else
        m_pKeys = new OKeys(*this,m_aMutex,aVector,aKeys,isCaseSensitive(),m_pCatalog->getConnection());
}
// -------------------------------------------------------------------------
void OAdoTable::refreshIndexes()
{
    TStringVector aVector;

    WpADOIndexes aIndexes;
    if(m_aTable.IsValid())
    {
        aIndexes = m_aTable.get_Indexes();
        aIndexes.fillElementNames(aVector);
    }

    if(m_pIndexes)
        m_pIndexes->reFill(aVector);
    else
        m_pIndexes = new OIndexes(*this,m_aMutex,aVector,aIndexes,isCaseSensitive(),m_pCatalog->getConnection());
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OAdoTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            (sal_Int64)this
                :
            OTable_TYPEDEF::getSomething(rId);
}
// -------------------------------------------------------------------------
// XRename
void SAL_CALL OAdoTable::rename( const ::rtl::OUString& newName ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    m_aTable.put_Name(newName);
    ADOS::ThrowException(*(m_pCatalog->getConnection()->getConnection()),*this);

    OTable_TYPEDEF::rename(newName);
}
// -----------------------------------------------------------------------------
Reference< XDatabaseMetaData> OAdoTable::getMetaData() const
{
    return m_pCatalog->getConnection()->getMetaData();
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OAdoTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    sal_Bool bError = sal_True;
    OAdoColumn* pColumn = NULL;
    if(getImplementation(pColumn,descriptor) && pColumn != NULL)
    {
        WpADOColumns aColumns = m_aTable.get_Columns();
        bError = !aColumns.Delete(colName);
        bError = bError || !aColumns.Append(pColumn->getColumnImpl());
    }
    if(bError)
        ADOS::ThrowException(*(m_pCatalog->getConnection()->getConnection()),*this);

    m_pColumns->refresh();
    refreshColumns();
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoTable::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(rBHelper.bDisposed);

    Reference< XPropertySet > xOld;
    m_pColumns->getByIndex(index) >>= xOld;
    if(xOld.is())
        alterColumnByName(getString(xOld->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),descriptor);
}
// -------------------------------------------------------------------------
void OAdoTable::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aTable.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                m_aTable.put_Name(getString(rValue));
                break;

            case PROPERTY_ID_TYPE:
                OTools::putValue(   m_aTable.get_Properties(),
                                OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),
                                getString(rValue));
                break;

            case PROPERTY_ID_DESCRIPTION:
                OTools::putValue(   m_aTable.get_Properties(),
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Description")),
                                getString(rValue));
                break;

            case PROPERTY_ID_SCHEMANAME:
                break;

            default:
                                throw Exception();
        }
    }
    OTable_TYPEDEF::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoTable::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OTable_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoTable::release() throw(::com::sun::star::uno::RuntimeException)
{
    OTable_TYPEDEF::release();
}
// -----------------------------------------------------------------------------

