/*************************************************************************
 *
 *  $RCSfile: TTableHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:52:38 $
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

#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include "connectivity/TTableHelper.hxx"
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
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OTableHelper::OTableHelper( sdbcx::OCollection* _pTables,
                           const Reference< XConnection >& _xConnection,
                           sal_Bool _bCase)
    :OTable_TYPEDEF(_pTables,_bCase)
    ,m_xConnection(_xConnection)
{
    try
    {
        m_xMetaData = m_xConnection->getMetaData();
    }
    catch(const Exception&)
    {
    }
}
// -------------------------------------------------------------------------
OTableHelper::OTableHelper( sdbcx::OCollection* _pTables,
                            const Reference< XConnection >& _xConnection,
                            sal_Bool _bCase,
                            const ::rtl::OUString& _Name,
                            const ::rtl::OUString& _Type,
                            const ::rtl::OUString& _Description ,
                            const ::rtl::OUString& _SchemaName,
                            const ::rtl::OUString& _CatalogName
                        ) : OTable_TYPEDEF(_pTables,
                                            _bCase,
                                            _Name,
                                          _Type,
                                          _Description,
                                          _SchemaName,
                                          _CatalogName)
                        ,m_xConnection(_xConnection)
{
    try
    {
        m_xMetaData = m_xConnection->getMetaData();
    }
    catch(const Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableHelper::disposing()
{
    OTable_TYPEDEF::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xConnection   = NULL;
    m_xMetaData     = NULL;
}
// -------------------------------------------------------------------------
void OTableHelper::refreshColumns()
{
    TStringVector aVector;
    if(!isNew())
    {
        Any aCatalog;
        if ( m_CatalogName.getLength() )
            aCatalog <<= m_CatalogName;
        Reference< XResultSet > xResult = getMetaData()->getColumns(    aCatalog,
                                                                        m_SchemaName,
                                                                        m_Name,
                                                                        ::rtl::OUString::createFromAscii("%"));

        if ( xResult.is() )
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            while( xResult->next() )
                aVector.push_back(xRow->getString(4));
            ::comphelper::disposeComponent(xResult);
        }
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = createColumns(aVector);
}
// -------------------------------------------------------------------------
void OTableHelper::refreshPrimaryKeys(std::vector< ::rtl::OUString>& _rKeys)
{
    Any aCatalog;
    if ( m_CatalogName.getLength() )
        aCatalog <<= m_CatalogName;
    Reference< XResultSet > xResult = getMetaData()->getPrimaryKeys(aCatalog,m_SchemaName,m_Name);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one primary key
        {
            ::rtl::OUString aPkName = xRow->getString(6);
            _rKeys.push_back(aPkName);
        }
        ::comphelper::disposeComponent(xResult);
    }
}
// -------------------------------------------------------------------------
void OTableHelper::refreshForgeinKeys(std::vector< ::rtl::OUString>& _rKeys)
{
    Any aCatalog;
    if ( m_CatalogName.getLength() )
        aCatalog <<= m_CatalogName;
    Reference< XResultSet > xResult = getMetaData()->getImportedKeys(aCatalog,m_SchemaName,m_Name);
    Reference< XRow > xRow(xResult,UNO_QUERY);

    if ( xRow.is() )
    {
        while( xResult->next() )
        {
            sal_Int32 nKeySeq = xRow->getInt(9);
            if ( nKeySeq == 1 )
            { // only append when the sequnce number is 1 to forbid serveral inserting the same key name
                ::rtl::OUString sFkName = xRow->getString(12);
                if ( !xRow->wasNull() && sFkName.getLength() )
                    _rKeys.push_back(sFkName);
            }
        }
        ::comphelper::disposeComponent(xResult);
    }
}
// -------------------------------------------------------------------------
void OTableHelper::refreshKeys()
{
    TStringVector aVector;

    if(!isNew())
    {
        refreshPrimaryKeys(aVector);
        refreshForgeinKeys(aVector);
    }
    if(m_pKeys)
        m_pKeys->reFill(aVector);
    else
        m_pKeys = createKeys(aVector);
}
// -------------------------------------------------------------------------
void OTableHelper::refreshIndexes()
{
    TStringVector aVector;
    if(!isNew())
    {
        // fill indexes
        Any aCatalog;
        if ( m_CatalogName.getLength() )
            aCatalog <<= m_CatalogName;
        Reference< XResultSet > xResult = getMetaData()->getIndexInfo(aCatalog,m_SchemaName,m_Name,sal_False,sal_False);

        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            ::rtl::OUString aName;
            ::rtl::OUString sCatalogSep = getMetaData()->getCatalogSeparator();
            ::rtl::OUString sPreviousRoundName;
            while( xResult->next() )
            {
                aName = xRow->getString(5);
                if(aName.getLength())
                    aName += sCatalogSep;
                aName += xRow->getString(6);
                if ( aName.getLength() )
                {
                    // don't insert the name if the last one we inserted was the same
                    if (sPreviousRoundName != aName)
                        aVector.push_back(aName);
                }
                sPreviousRoundName = aName;
            }
            ::comphelper::disposeComponent(xResult);
        }
    }

    if(m_pIndexes)
        m_pIndexes->reFill(aVector);
    else
        m_pIndexes  = createIndexes(aVector);
}
// -------------------------------------------------------------------------
// XRename
void SAL_CALL OTableHelper::rename( const ::rtl::OUString& newName ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef GCC
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    if(!isNew())
    {
        ::rtl::OUString sSql = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RENAME "));
        if ( m_Type == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW")) )
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" VIEW "));
        else
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" TABLE "));

        ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );

        ::rtl::OUString sCatalog,sSchema,sTable;
        ::dbtools::qualifiedNameComponents(getMetaData(),newName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);

        ::rtl::OUString sComposedName;
        ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,sComposedName,sal_True,::dbtools::eInDataManipulation);
        sSql += sComposedName
             + ::rtl::OUString::createFromAscii(" TO ");
        ::dbtools::composeTableName(getMetaData(),sCatalog,sSchema,sTable,sComposedName,sal_True,::dbtools::eInDataManipulation);
        sSql += sComposedName;

        Reference< XStatement > xStmt = m_xConnection->createStatement(  );
        if ( xStmt.is() )
        {
            xStmt->execute(sSql);
            ::comphelper::disposeComponent(xStmt);
        }

        OTable_TYPEDEF::rename(newName);
    }
    else
        ::dbtools::qualifiedNameComponents(getMetaData(),newName,m_CatalogName,m_SchemaName,m_Name,::dbtools::eInTableDefinitions);
}
// -----------------------------------------------------------------------------
Reference< XDatabaseMetaData> OTableHelper::getMetaData() const
{
    return m_xMetaData;
}
// -------------------------------------------------------------------------
void SAL_CALL OTableHelper::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef GCC
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    Reference< XPropertySet > xOld;
    if(::cppu::extractInterface(xOld,m_pColumns->getByIndex(index)) && xOld.is())
        alterColumnByName(getString(xOld->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),descriptor);
}

// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTableHelper::getName() throw(RuntimeException)
{
    ::rtl::OUString sComposedName;
    ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,sComposedName,sal_False,::dbtools::eInDataManipulation);
    return sComposedName;
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableHelper::acquire() throw()
{
    OTable_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableHelper::release() throw()
{
    OTable_TYPEDEF::release();
}
// -----------------------------------------------------------------------------

