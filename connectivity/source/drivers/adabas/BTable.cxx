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
#include "adabas/BTable.hxx"
#include "adabas/BTables.hxx"
#include "adabas/BIndexes.hxx"
#include "adabas/BColumns.hxx"
#include "adabas/BKeys.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include "connectivity/dbtools.hxx"
#include "adabas/BCatalog.hxx"


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

                ::rtl::OUString sSql( RTL_CONSTASCII_USTRINGPARAM( "RENAME COLUMN " )) ;
                sSql += ::dbtools::quoteName(sQuote,m_SchemaName) + sDot + ::dbtools::quoteName(sQuote,m_Name);
                sSql += sDot + ::dbtools::quoteName(sQuote,colName);
                sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" TO "));
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
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
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
        sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" NOT NULL"));
    }
    else
    {
        sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DEFAULT NULL"));
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
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ALTER ")) + _sNewDefault;

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
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DROP DEFAULT"));

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
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ADD ")) + _sNewDefault;

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
            xStmt->execute(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SUBTRANS BEGIN")) );
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
            xStmt->execute(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SUBTRANS END")) );
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
            xStmt->execute(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SUBTRANS ROLLBACK")) );
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
    ::rtl::OUString sSql( RTL_CONSTASCII_USTRINGPARAM( "ALTER TABLE " ));
    const ::rtl::OUString sQuote = m_pConnection->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    sSql += ::dbtools::quoteName(sQuote,m_SchemaName) + sDot + ::dbtools::quoteName(sQuote,m_Name)
         + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" COLUMN "))
         + ::dbtools::quoteName(sQuote,_rsColumnName);
    return sSql;
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
