/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BTable.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:10:52 $
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

#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#include "adabas/BTable.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLES_HXX_
#include "adabas/BTables.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_INDEXES_HXX_
#include "adabas/BIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_COLUMNS_HXX_
#include "adabas/BColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_KEYS_HXX_
#include "adabas/BKeys.hxx"
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
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif


using namespace ::comphelper;
using namespace connectivity::adabas;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OAdabasTable::OAdabasTable( sdbcx::OCollection* _pTables,
                           OAdabasConnection* _pConnection)
    :OTable_TYPEDEF(_pTables,_pConnection,sal_True)
    ,m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
OAdabasTable::OAdabasTable( sdbcx::OCollection* _pTables,
                           OAdabasConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : OTableHelper(   _pTables,
                                    _pConnection,
                                    sal_True,
                                    _Name,
                                    _Type,
                                    _Description,
                                    _SchemaName,
                                    _CatalogName)
                ,m_pConnection(_pConnection)
{
    construct();
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OAdabasTable::createColumns(const TStringVector& _rNames)
{
    return new OColumns(this,m_aMutex,_rNames);
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OAdabasTable::createKeys(const TStringVector& _rNames)
{
    return new OKeys(this,m_aMutex,_rNames);
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OAdabasTable::createIndexes(const TStringVector& _rNames)
{
    return new OIndexes(this,m_aMutex,_rNames);
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdabasTable::getUnoTunnelImplementationId()
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
sal_Int64 OAdabasTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OTable_TYPEDEF::getSomething(rId);
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OAdabasTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef GCC
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    if(m_pColumns && !m_pColumns->hasByName(colName))
        throw NoSuchElementException(colName,*this);


    if(!isNew())
    {
//      if(getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) != colName)
//          throw SQLException(::rtl::OUString::createFromAscii("Not supported by this driver!"),*this,::rtl::OUString::createFromAscii("S1000"),0,Any() );

        beginTransAction();

        try
        {
            // first we have to check what should be altered
            Reference<XPropertySet> xProp;
            m_pColumns->getByName(colName) >>= xProp;
            // first check the types
            sal_Int32 nOldType = 0,nNewType = 0,nOldPrec = 0,nNewPrec = 0,nOldScale = 0,nNewScale = 0;

            xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))         >>= nOldType;
            descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))    >>= nNewType;
            // and precsions and scale
            xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))    >>= nOldPrec;
            descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))>>= nNewPrec;
            xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))        >>= nOldScale;
            descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))   >>= nNewScale;

            if(nOldType != nNewType || nOldPrec != nNewPrec || nOldScale != nNewScale)
                alterColumnType(colName,descriptor);

            // second: check the "is nullable" value
            sal_Int32 nOldNullable = 0,nNewNullable = 0;
            xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))       >>= nOldNullable;
            descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))  >>= nNewNullable;
            if(nNewNullable != nOldNullable)
                alterNotNullValue(nNewNullable,colName);

            // third: check the default values
            ::rtl::OUString sNewDefault,sOldDefault;
            xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE))     >>= sOldDefault;
            descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE)) >>= sNewDefault;

            if(sOldDefault.getLength())
            {
                if(sNewDefault.getLength() && sOldDefault != sNewDefault)
                    alterDefaultValue(sNewDefault,colName);
                else if(!sNewDefault.getLength())
                    dropDefaultValue(colName);
            }
            else if(!sOldDefault.getLength() && sNewDefault.getLength())
                addDefaultValue(sNewDefault,colName);

            // now we should look if the name of the column changed
            ::rtl::OUString sNewColumnName;
            descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sNewColumnName;
            if(!sNewColumnName.equalsIgnoreAsciiCase(colName))
            {
                const ::rtl::OUString sQuote = m_pConnection->getMetaData()->getIdentifierQuoteString(  );
                const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

                ::rtl::OUString sSql = ::rtl::OUString::createFromAscii("RENAME COLUMN ") ;
                sSql += ::dbtools::quoteName(sQuote,m_SchemaName) + sDot + ::dbtools::quoteName(sQuote,m_Name);
                sSql += sDot + ::dbtools::quoteName(sQuote,colName);
                sSql += ::rtl::OUString::createFromAscii(" TO ");
                sSql += ::dbtools::quoteName(sQuote,sNewColumnName);

                Reference< XStatement > xStmt = m_pConnection->createStatement(  );
                if(xStmt.is())
                {
                    xStmt->execute(sSql);
                    ::comphelper::disposeComponent(xStmt);
                }
            }
            m_pColumns->refresh();
        }
        catch(const SQLException&)
        {
            rollbackTransAction();
            throw;
        }
        endTransAction();
    }
    else
    {
        if(m_pColumns)
        {
            m_pColumns->dropByName(colName);
            m_pColumns->appendByDescriptor(descriptor);
        }
    }

}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OAdabasTable::getName() throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName = m_SchemaName;
    if(m_SchemaName.getLength())
    {
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();
        sName += sDot;
    }
    sName += m_Name;
    return sName;
}
// -----------------------------------------------------------------------------
void OAdabasTable::alterColumnType(const ::rtl::OUString& _rColName, const Reference<XPropertySet>& _xDescriptor)
{
    ::rtl::OUString sSql = getAlterTableColumnPart(_rColName);
    sSql += ::rtl::OUString::createFromAscii(" ");
    sSql += OTables::getColumnSqlType(_xDescriptor);

    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    if(xStmt.is())
    {
        xStmt->execute(sSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void OAdabasTable::alterNotNullValue(sal_Int32 _nNewNullable,const ::rtl::OUString& _rColName)
{
    ::rtl::OUString sSql = getAlterTableColumnPart(_rColName);

    if(_nNewNullable == ColumnValue::NO_NULLS)
    {
        sSql += ::rtl::OUString::createFromAscii(" NOT NULL");
    }
    else
    {
        sSql += ::rtl::OUString::createFromAscii(" DEFAULT NULL");
    }

    Reference< XStatement > xStmt = m_pConnection->createStatement();
    if(xStmt.is())
    {
        xStmt->execute(sSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void OAdabasTable::alterDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName)
{
    ::rtl::OUString sSql = getAlterTableColumnPart(_rColName);
    sSql += ::rtl::OUString::createFromAscii(" ALTER ") + _sNewDefault;

    Reference< XStatement > xStmt = m_pConnection->createStatement();
    if(xStmt.is())
    {
        xStmt->execute(sSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void OAdabasTable::dropDefaultValue(const ::rtl::OUString& _rColName)
{
    ::rtl::OUString sSql = getAlterTableColumnPart(_rColName);
    sSql += ::rtl::OUString::createFromAscii(" DROP DEFAULT");

    Reference< XStatement > xStmt = m_pConnection->createStatement();
    if(xStmt.is())
    {
        xStmt->execute(sSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void OAdabasTable::addDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName)
{
    ::rtl::OUString sSql = getAlterTableColumnPart(_rColName);
    sSql += ::rtl::OUString::createFromAscii(" ADD ") + _sNewDefault;

    Reference< XStatement > xStmt = m_pConnection->createStatement();
    if(xStmt.is())
    {
        xStmt->execute(sSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void OAdabasTable::beginTransAction()
{
    try
    {
        Reference< XStatement > xStmt = m_pConnection->createStatement();
        if(xStmt.is())
        {
            xStmt->execute(::rtl::OUString::createFromAscii("SUBTRANS BEGIN") );
            ::comphelper::disposeComponent(xStmt);
        }
    }
    catch(const Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void OAdabasTable::endTransAction()
{
    try
    {
        Reference< XStatement > xStmt = m_pConnection->createStatement();
        if(xStmt.is())
        {
            xStmt->execute(::rtl::OUString::createFromAscii("SUBTRANS END") );
            ::comphelper::disposeComponent(xStmt);
        }
    }
    catch(const Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void OAdabasTable::rollbackTransAction()
{
    try
    {
        Reference< XStatement > xStmt = m_pConnection->createStatement();
        if(xStmt.is())
        {
            xStmt->execute(::rtl::OUString::createFromAscii("SUBTRANS ROLLBACK") );
            ::comphelper::disposeComponent(xStmt);
        }
    }
    catch(const Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString OAdabasTable::getAlterTableColumnPart(const ::rtl::OUString& _rsColumnName )
{
    ::rtl::OUString sSql = ::rtl::OUString::createFromAscii("ALTER TABLE ");
    const ::rtl::OUString sQuote = m_pConnection->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    sSql += ::dbtools::quoteName(sQuote,m_SchemaName) + sDot + ::dbtools::quoteName(sQuote,m_Name)
         + ::rtl::OUString::createFromAscii(" COLUMN ")
         + ::dbtools::quoteName(sQuote,_rsColumnName);
    return sSql;
}
// -----------------------------------------------------------------------------



