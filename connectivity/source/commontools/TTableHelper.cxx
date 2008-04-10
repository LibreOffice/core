/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TTableHelper.cxx,v $
 * $Revision: 1.9 $
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
#include "connectivity/TTableHelper.hxx"
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
#include "connectivity/sdbcx/VCollection.hxx"
#include <unotools/sharedunocomponent.hxx>
#include "TConnection.hxx"

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
namespace
{
    typedef sal_Int32   OrdinalPosition;
    struct ColumnDesc
    {
        ::rtl::OUString sName;
        OrdinalPosition nOrdinalPosition;

        ColumnDesc() {}
        ColumnDesc( const ::rtl::OUString& _rName, OrdinalPosition _nPosition )
            :sName( _rName )
            ,nOrdinalPosition( _nPosition )
        {
        }
    };

    /** collects ColumnDesc's from a resultset produced by XDatabaseMetaData::getColumns
    */
    void lcl_collectColumnDescs_throw( const Reference< XResultSet >& _rxResult, ::std::vector< ColumnDesc >& _out_rColumns )
    {
        Reference< XRow > xRow( _rxResult, UNO_QUERY_THROW );
        ::rtl::OUString sName;
        OrdinalPosition nOrdinalPosition( 0 );
        while ( _rxResult->next() )
        {
            sName = xRow->getString( 4 );           // COLUMN_NAME
            nOrdinalPosition = xRow->getInt( 17 );  // ORDINAL_POSITION
            _out_rColumns.push_back( ColumnDesc( sName, nOrdinalPosition ) );
        }
    }

    /** checks a given array of ColumnDesc's whether it has reasonable ordinal positions. If not,
        they will be normalized to be the array index.
    */
    void lcl_sanitizeColumnDescs( ::std::vector< ColumnDesc >& _rColumns )
    {
        if ( _rColumns.empty() )
            return;

        // collect all used ordinals
        ::std::set< OrdinalPosition > aUsedOrdinals;
        for (   ::std::vector< ColumnDesc >::iterator collect = _rColumns.begin();
                collect != _rColumns.end();
                ++collect
            )
            aUsedOrdinals.insert( collect->nOrdinalPosition );

        // we need to have as much different ordinals as we have different columns
        bool bDuplicates = aUsedOrdinals.size() != _rColumns.size();
        // and it needs to be a continuous range
        size_t nOrdinalsRange = *aUsedOrdinals.rbegin() - *aUsedOrdinals.begin() + 1;
        bool bGaps = nOrdinalsRange != _rColumns.size();

        // if that's not the case, normalize it
        if ( bGaps || bDuplicates )
        {
            OSL_ENSURE( false, "lcl_sanitizeColumnDescs: database did provide invalid ORDINAL_POSITION values!" );

            OrdinalPosition nNormalizedPosition = 1;
            for (   ::std::vector< ColumnDesc >::iterator normalize = _rColumns.begin();
                    normalize != _rColumns.end();
                    ++normalize
                )
                normalize->nOrdinalPosition = nNormalizedPosition++;
            return;
        }

        // what's left is that the range might not be from 1 to <column count>, but for instance
        // 0 to <column count>-1.
        size_t nOffset = *aUsedOrdinals.begin() - 1;
        for (   ::std::vector< ColumnDesc >::iterator offset = _rColumns.begin();
                offset != _rColumns.end();
                ++offset
            )
            offset->nOrdinalPosition -= nOffset;
    }
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

        ::utl::SharedUNOComponent< XResultSet > xResult( getMetaData()->getColumns(
            aCatalog,
            m_SchemaName,
            m_Name,
            ::rtl::OUString::createFromAscii("%")
        ) );

        // collect the column names, together with their ordinal position
        ::std::vector< ColumnDesc > aColumns;
        lcl_collectColumnDescs_throw( xResult, aColumns );

        // ensure that the ordinal positions as obtained from the meta data do make sense
        lcl_sanitizeColumnDescs( aColumns );

        // sort by ordinal position
        ::std::map< OrdinalPosition, ::rtl::OUString > aSortedColumns;
        for (   ::std::vector< ColumnDesc >::const_iterator copy = aColumns.begin();
                copy != aColumns.end();
                ++copy
            )
            aSortedColumns[ copy->nOrdinalPosition ] = copy->sName;

        // copy them to aVector, now that we have the proper ordering
        ::std::transform(
            aSortedColumns.begin(),
            aSortedColumns.end(),
            ::std::insert_iterator< TStringVector >( aVector, aVector.begin() ),
            ::std::select2nd< ::std::map< OrdinalPosition, ::rtl::OUString >::value_type >()
        );
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
// -----------------------------------------------------------------------------
::rtl::OUString OTableHelper::getRenameStart() const
{
    ::rtl::OUString sSql(RTL_CONSTASCII_USTRINGPARAM("RENAME "));
    if ( m_Type == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW")) )
        sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" VIEW "));
    else
        sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" TABLE "));

    return sSql;
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
        ::rtl::OUString sSql = getRenameStart();
        ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );

        ::rtl::OUString sCatalog,sSchema,sTable;
        ::dbtools::qualifiedNameComponents(getMetaData(),newName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);

        ::rtl::OUString sComposedName;
        sComposedName = ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,sal_True,::dbtools::eInDataManipulation);
        sSql += sComposedName
             + ::rtl::OUString::createFromAscii(" TO ");
        sComposedName = ::dbtools::composeTableName(getMetaData(),sCatalog,sSchema,sTable,sal_True,::dbtools::eInDataManipulation);
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
    sComposedName = ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,sal_False,::dbtools::eInDataManipulation);
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

