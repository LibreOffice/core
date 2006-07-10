/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BColumns.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:21:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ADABAS_COLUMNS_HXX_
#include "adabas/BColumns.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#include "adabas/BTable.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLES_HXX_
#include "adabas/BTables.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

using namespace ::comphelper;


using namespace connectivity::adabas;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OColumns::createObject(const ::rtl::OUString& _rName)
{
    Reference< XResultSet > xResult = m_pTable->getMetaData()->getColumns(Any(),
                                                            m_pTable->getSchema(),m_pTable->getTableName(),_rName);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                sal_Int32 nType             = xRow->getInt(5);
                ::rtl::OUString sTypeName   = xRow->getString(6);
                sal_Int32 nPrec             = xRow->getInt(7);
                OAdabasCatalog::correctColumnProperties(nPrec,nType,sTypeName);

                xRet = new OColumn(_rName,
                                            sTypeName,
                                            xRow->getString(13),
                                            xRow->getInt(11),
                                            nPrec,
                                            xRow->getInt(9),
                                            nType,
                                            sal_False,sal_False,sal_False,sal_True);
                break;
            }
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createDescriptor()
{
    return new OColumn(sal_True);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OColumns::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if ( m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    ::rtl::OUString aSql(RTL_CONSTASCII_USTRINGPARAM("ALTER TABLE "));
    ::rtl::OUString sQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    m_pTable->beginTransAction();
    try
    {
        aSql += ::dbtools::quoteName(sQuote,m_pTable->getSchema()) + sDot + ::dbtools::quoteName(sQuote,m_pTable->getTableName());
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ADD ("));
        aSql += ::dbtools::quoteName(sQuote,_rForName);
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
        aSql += OTables::getColumnSqlType(descriptor);
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" )"));

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement();
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);

        m_pTable->alterNotNullValue(getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))),_rForName);
    }
    catch(const Exception&)
    {
        m_pTable->rollbackTransAction();
        throw;
    }
    m_pTable->endTransAction();

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OColumns::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    OSL_ENSURE(m_pTable,"OColumns::dropByName: Table is null!");
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aSql(RTL_CONSTASCII_USTRINGPARAM("ALTER TABLE "));
        ::rtl::OUString sQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        aSql += ::dbtools::quoteName(sQuote,m_pTable->getSchema()) + sDot + ::dbtools::quoteName(sQuote,m_pTable->getTableName());
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DROP "));
        aSql += ::dbtools::quoteName(sQuote,_sElementName);

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------



