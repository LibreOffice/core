/*************************************************************************
 *
 *  $RCSfile: MTable.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mmaher $ $Date: 2001-10-11 10:07:54 $
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

#ifndef _CONNECTIVITY_MOZAB_TABLE_HXX_
#include "MTable.hxx"
#endif
#ifndef _CONNECTIVITY_MOZAB_TABLES_HXX_
#include "MTables.hxx"
#endif
#ifndef _CONNECTIVITY_MOZAB_COLUMNS_HXX_
#include "MColumns.hxx"
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
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _CONNECTIVITY_MOZAB_CATALOG_HXX_
#include "MCatalog.hxx"
#endif


using namespace ::comphelper;
using namespace connectivity::mozab;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OTable::OTable( sdbcx::OCollection* _pTables, OConnection* _pConnection)
    : OTable_TYPEDEF(_pTables, sal_True),m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
OTable::OTable( sdbcx::OCollection* _pTables,
                OConnection* _pConnection,
                const ::rtl::OUString& _Name,
                const ::rtl::OUString& _Type,
                const ::rtl::OUString& _Description ,
                const ::rtl::OUString& _SchemaName,
                const ::rtl::OUString& _CatalogName
                ) : OTable_TYPEDEF(_pTables,sal_True,
                                  _Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
                ,m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
void OTable::refreshColumns()
{
    TStringVector aVector;
    if(!isNew())
    {
        Reference< XResultSet > xResult = m_pConnection->getMetaData()->getColumns(Any(),
                                                        m_SchemaName,m_Name,::rtl::OUString::createFromAscii("%"));

        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            while(xResult->next())
                aVector.push_back(xRow->getString(4));
        }
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OColumns(this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OTable::refreshPrimaryKeys(std::vector< ::rtl::OUString>& _rKeys)
{
    Reference< XResultSet > xResult = m_pConnection->getMetaData()->getPrimaryKeys(Any(),m_SchemaName,m_Name);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one primary key
        {
            ::rtl::OUString aPkName = xRow->getString(6);
            _rKeys.push_back(aPkName);
        }
    }
}
// -------------------------------------------------------------------------
void OTable::refreshForgeinKeys(std::vector< ::rtl::OUString>& _rKeys)
{
    Reference< XResultSet > xResult = m_pConnection->getMetaData()->getImportedKeys(Any(),m_SchemaName,m_Name);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            _rKeys.push_back(xRow->getString(12));
    }
}
// -------------------------------------------------------------------------
void OTable::refreshKeys()
{
}
// -------------------------------------------------------------------------
void OTable::refreshIndexes()
{
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OTable::getUnoTunnelImplementationId()
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
sal_Int64 OTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            (sal_Int64)this
                :
            OTable_TYPEDEF::getSomething(rId);
}
// -------------------------------------------------------------------------
// XRename
void SAL_CALL OTable::rename( const ::rtl::OUString& newName ) throw(SQLException, ElementExistException, RuntimeException)
{
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OTable::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
}

// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTable::getName() throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
#ifdef DARREN_WORK
// -----------------------------------------------------------------------------
void SAL_CALL OTable::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OTable_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OTable::release() throw(::com::sun::star::uno::RuntimeException)
{
    OTable_TYPEDEF::release();
}
#endif /* DARREN_WORK */
// -----------------------------------------------------------------------------
void OTable::alterColumnType(sal_Int32 nNewType,const ::rtl::OUString& _rColName, const Reference<XPropertySet>& _xDescriptor)
{
}
// -----------------------------------------------------------------------------
void OTable::alterNotNullValue(sal_Int32 _nNewNullable,const ::rtl::OUString& _rColName)
{
}
// -----------------------------------------------------------------------------
void OTable::alterDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName)
{
}
// -----------------------------------------------------------------------------
void OTable::dropDefaultValue(const ::rtl::OUString& _rColName)
{
}
// -----------------------------------------------------------------------------
void OTable::addDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName)
{
}
// -----------------------------------------------------------------------------
void OTable::beginTransAction()
{
}
// -----------------------------------------------------------------------------
void OTable::endTransAction()
{
}
// -----------------------------------------------------------------------------
void OTable::rollbackTransAction()
{
}
// -----------------------------------------------------------------------------
::rtl::OUString OTable::getAlterTableColumnPart(const ::rtl::OUString& _rsColumnName )
{
    ::rtl::OUString sSql = ::rtl::OUString::createFromAscii("");
    return sSql;
}
// -----------------------------------------------------------------------------



