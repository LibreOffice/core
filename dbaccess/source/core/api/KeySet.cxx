/*************************************************************************
 *
 *  $RCSfile: KeySet.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:00:09 $
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

#ifndef DBACCESS_CORE_API_KEYSET_HXX
#include "KeySet.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCxParameterS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
#endif
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XINDEXESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#include <list>
#include <algorithm>
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef DBACCESS_CORE_API_QUERYCOMPOSER_HXX
#include "querycomposer.hxx"
#endif

using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::cppu;
using namespace ::osl;

namespace
{
    void lcl_fillIndexColumns(const Reference<XIndexAccess>& _xIndexes, ::std::vector< Reference<XNameAccess> >& _rAllIndexColumns)
    {
        if ( _xIndexes.is() )
        {
            Reference<XPropertySet> xIndexColsSup;
            sal_Int32 nCount = _xIndexes->getCount();
            for(sal_Int32 j = 0 ; j < nCount ; ++j)
            {
                xIndexColsSup.set(_xIndexes->getByIndex(j),UNO_QUERY);
                if( xIndexColsSup.is()
                    && comphelper::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISUNIQUE))
                    && !comphelper::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISPRIMARYKEYINDEX))
                )
                    _rAllIndexColumns.push_back(Reference<XColumnsSupplier>(xIndexColsSup,UNO_QUERY)->getColumns());
            }
        }
    }
}
// -------------------------------------------------------------------------
OKeySet::OKeySet(const connectivity::OSQLTable& _xTable,
                 const ::rtl::OUString& _rUpdateTableName,    // this can be the alias or the full qualified name
                 const Reference< XSQLQueryComposer >& _xComposer)
            :m_xTable(_xTable)
            ,m_bRowCountFinal(sal_False)
            ,m_xComposer(_xComposer)
            ,m_sUpdateTableName(_rUpdateTableName)
            ,m_pKeyColumnNames(NULL)
            ,m_pColumnNames(NULL)
{
}
// -----------------------------------------------------------------------------
OKeySet::~OKeySet()
{
    try
    {
        ::comphelper::disposeComponent(m_xStatement);
    }
    catch(Exception&)
    {
        m_xStatement = NULL;
    }
    catch(...)
    {
        OSL_ENSURE(0,"Unknown Exception occured");
    }
    m_xComposer = NULL;
    delete m_pKeyColumnNames;
    delete m_pColumnNames;
}
// -----------------------------------------------------------------------------
void OKeySet::construct(const Reference< XResultSet>& _xDriverSet)
{
    OCacheSet::construct(_xDriverSet);

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    bool bCase = (xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers()) ? true : false;
    m_pKeyColumnNames = new OColumnNamePos(bCase);
    m_pColumnNames = new OColumnNamePos(bCase);

    Reference<XNameAccess> xKeyColumns  = getKeyColumns();
    Reference<XColumnsSupplier> xSup(m_xComposer,UNO_QUERY);
    Reference<XNameAccess> xSourceColumns = m_xTable->getColumns();

    ::dbaccess::getColumnPositions(xSup->getColumns(),xKeyColumns,m_sUpdateTableName,(*m_pKeyColumnNames));
    ::dbaccess::getColumnPositions(xSup->getColumns(),xSourceColumns,m_sUpdateTableName,(*m_pColumnNames));

    OColumnNamePos::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
    for(;aPosIter != (*m_pKeyColumnNames).end();++aPosIter)
    {
        if(xSourceColumns->hasByName(aPosIter->first))
        {
            Reference<XPropertySet> xProp(xSourceColumns->getByName(aPosIter->first),UNO_QUERY);
            sal_Bool bAuto;
            if( (xProp->getPropertyValue(PROPERTY_ISAUTOINCREMENT) >>= bAuto) && bAuto)
                m_aAutoColumns.push_back(aPosIter->first);
        }
    }

    // the first row is empty because it's now easier for us to distinguish when we are beforefirst or first
    // without extra varaible to be set
    m_aKeyMap.insert(OKeySetMatrix::value_type(0,OKeySetValue(NULL,0)));

    m_aKeyIter = m_aKeyMap.begin();

    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    ::rtl::OUString aQuote  = getIdentifierQuoteString();

    ::rtl::OUString aFilter;
    ::rtl::OUString sCatalog,sSchema,sTable;

    Reference<XPropertySet> xTableProp(m_xTable,UNO_QUERY);
    xTableProp->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
    xTableProp->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
    xTableProp->getPropertyValue(PROPERTY_NAME)         >>= sTable;

    m_aSelectComposedTableName = getComposedTableName(sCatalog,sSchema,sTable);

    ::rtl::OUString sComposedName;
    sCatalog = sSchema = sTable = ::rtl::OUString();
    ::dbtools::qualifiedNameComponents(xMetaData,m_sUpdateTableName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
    ::dbtools::composeTableName(xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_True,::dbtools::eInDataManipulation);
    // create the where clause
    OColumnNamePos::const_iterator aIter;
    for(aIter = (*m_pKeyColumnNames).begin();aIter != (*m_pKeyColumnNames).end();)
    {
        aFilter += sComposedName;
        aFilter += ::rtl::OUString::createFromAscii(".");
        aFilter += ::dbtools::quoteName( aQuote,aIter->first);
        aFilter += ::rtl::OUString::createFromAscii(" = ?");
        ++aIter;
        if(aIter != (*m_pKeyColumnNames).end())
            aFilter += aAnd;
    }
    // don't simply overwrite an existent filter, this would lead to problems if this existent
    // filter contains paramters (since we're going to add parameters ourself)
    // 2003-12-12 - #23418# - fs@openoffice.org
    ::rtl::OUString sOldFilter = m_xComposer->getFilter();
    if ( sOldFilter.getLength() )
    {
        FilterCreator aCreator;
        aCreator.append(sOldFilter);
        aCreator.append(aFilter);
        aFilter = aCreator.m_sFilter;
    }
    m_xComposer->setFilter(aFilter);
    m_xStatement = m_xConnection->prepareStatement(m_xComposer->getComposedQuery());
}
// -------------------------------------------------------------------------
Any SAL_CALL OKeySet::getBookmark( const ORowSetRow& _rRow ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin(),
        "getBookmark is only possible when we stand on a valid row!");
    return makeAny(m_aKeyIter->first);
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    return m_aKeyIter != m_aKeyMap.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(bookmark));
    if(m_aKeyIter != m_aKeyMap.end())
    {
        relative(rows);
    }

    return !isBeforeFirst() && !isAfterLast();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OKeySet::compareBookmarks( const Any& first, const Any& second ) throw(SQLException, RuntimeException)
{
    sal_Int32 nFirst,nSecond;
    first >>= nFirst;
    second >>= nSecond;

    return (nFirst != nSecond) ? CompareBookmark::NOT_EQUAL : CompareBookmark::EQUAL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OKeySet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    return ::comphelper::getINT32(bookmark);
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL OKeySet::deleteRows( const Sequence< Any >& rows ,const connectivity::OSQLTable& _xTable) throw(SQLException, RuntimeException)
{
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DELETE FROM ");
    aSql += m_aComposedTableName;
    aSql += ::rtl::OUString::createFromAscii(" WHERE ");

    // list all cloumns that should be set
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    static ::rtl::OUString aOr      = ::rtl::OUString::createFromAscii(" OR ");
    static ::rtl::OUString aEqual   = ::rtl::OUString::createFromAscii(" = ?");


    // use keys and indexes for excat postioning
    // first the keys
    Reference<XNameAccess> xKeyColumns = getKeyColumns();

    ::rtl::OUString aCondition = ::rtl::OUString::createFromAscii("( ");

    OColumnNamePos::const_iterator aIter = (*m_pKeyColumnNames).begin();
    for(;aIter != (*m_pKeyColumnNames).end();++aIter)
    {
        aCondition += ::dbtools::quoteName( aQuote,aIter->first);
        aCondition += aEqual;
        aCondition += aAnd;
    }
    aCondition = aCondition.replaceAt(aCondition.getLength()-5,5,::rtl::OUString::createFromAscii(" )"));

    const Any* pBegin   = rows.getConstArray();
    const Any* pEnd     = pBegin + rows.getLength();

    Sequence< Any > aKeys;
    for(;pBegin != pEnd;++pBegin)
    {
        aSql += aCondition;
        aSql += aOr;
    }
    aSql = aSql.replaceAt(aSql.getLength()-3,3,::rtl::OUString::createFromAscii(" "));

    // now create end execute the prepared statement

    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    pBegin  = rows.getConstArray();
    sal_Int32 i=1;
    for(;pBegin != pEnd;++pBegin)
    {
        m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32(*pBegin));
        if(m_aKeyIter != m_aKeyMap.end())
        {
            connectivity::ORowVector< ORowSetValue >::iterator aKeyIter = m_aKeyIter->second.first->begin();
            for(sal_uInt16 j = 0;aKeyIter != m_aKeyIter->second.first->end();++aKeyIter,++j)
            {
                setParameter(i++,xParameter,*aKeyIter);
            }
        }
    }

    sal_Bool bOk = xPrep->executeUpdate() > 0;
    Sequence< sal_Int32 > aRet(rows.getLength());
    memset(aRet.getArray(),bOk,sizeof(sal_Int32)*aRet.getLength());
    if(bOk)
    {
        const Any* pBegin   = rows.getConstArray();
        const Any* pEnd     = pBegin + rows.getLength();

        for(;pBegin != pEnd;++pBegin)
        {
            sal_Int32 nPos;
            *pBegin >>= nPos;
            if(m_aKeyIter == m_aKeyMap.find(nPos) && m_aKeyIter != m_aKeyMap.end())
                ++m_aKeyIter;
            m_aKeyMap.erase(nPos);
            m_bDeleted = sal_True;
        }
    }
    return aRet;
}
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("UPDATE ");
    aSql += m_aComposedTableName;
    aSql += ::rtl::OUString::createFromAscii(" SET ");
    // list all cloumns that should be set
    static ::rtl::OUString aPara    = ::rtl::OUString::createFromAscii(" = ?,");
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");

    // use keys and indexes for excat postioning
    // first the keys
    Reference<XNameAccess> xKeyColumns = getKeyColumns();

    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if ( xIndexSup.is() )
        xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);


    ::std::vector< Reference<XNameAccess> > aAllIndexColumns;
    lcl_fillIndexColumns(xIndexes,aAllIndexColumns);

    ::rtl::OUString aColumnName;
    ::rtl::OUString aCondition,sKeyCondition,sIndexCondition,sSetValues;
    ::std::vector<sal_Int32> aIndexColumnPositions;

    sal_Int32 i = 1;
    // here we build the condition part for the update statement
    OColumnNamePos::const_iterator aIter = m_pColumnNames->begin();
    for(;aIter != m_pColumnNames->end();++aIter,++i)
    {
        sal_Bool bSigned = m_xSetMetaData->isSigned(i);
        if(xKeyColumns.is() && xKeyColumns->hasByName(aIter->first))
        {
            sKeyCondition += ::dbtools::quoteName( aQuote,aIter->first);
            if((*_rOrginalRow)[aIter->second.first].isNull())
                sKeyCondition += ::rtl::OUString::createFromAscii(" IS NULL");
            else
                sKeyCondition += ::rtl::OUString::createFromAscii(" = ?");
            sKeyCondition += aAnd;
        }
        else
        {
            for( ::std::vector< Reference<XNameAccess> >::const_iterator aIndexIter = aAllIndexColumns.begin();
                    aIndexIter != aAllIndexColumns.end();++aIndexIter)
            {
                if((*aIndexIter)->hasByName(aIter->first))
                {
                    sIndexCondition += ::dbtools::quoteName( aQuote,aIter->first);
                    if((*_rOrginalRow)[aIter->second.first].isNull())
                        sIndexCondition += ::rtl::OUString::createFromAscii(" IS NULL");
                    else
                    {
                        sIndexCondition += ::rtl::OUString::createFromAscii(" = ?");
                        aIndexColumnPositions.push_back(aIter->second.first);
                    }
                    sIndexCondition += aAnd;
                    break;
                }
            }
        }
        if((*_rInsertRow)[aIter->second.first].isModified())
        {
            sSetValues += ::dbtools::quoteName( aQuote,aIter->first);
            sSetValues += aPara;
        }
    }

    if(sSetValues.getLength())
    {
        sSetValues = sSetValues.replaceAt(sSetValues.getLength()-1,1,::rtl::OUString::createFromAscii(" "));
        aSql += sSetValues;
    }
    else
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_VALUE_CHANGED),m_xConnection,::rtl::OUString::createFromAscii("HY0000"),1000,Any());

    if(sKeyCondition.getLength() || sIndexCondition.getLength())
    {
        if(sKeyCondition.getLength() && sIndexCondition.getLength())
        {
            aCondition =  sKeyCondition;
            aCondition += sIndexCondition;
        }
        else if(sKeyCondition.getLength())
        {
            aCondition = sKeyCondition;
        }
        else if(sIndexCondition.getLength())
        {
            aCondition = sIndexCondition;
        }

        aCondition = aCondition.replaceAt(aCondition.getLength()-5,5,::rtl::OUString::createFromAscii(" "));

        aSql += ::rtl::OUString::createFromAscii(" WHERE ");
        aSql += aCondition;
    }
    else
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_CONDITION_FOR_PK),m_xConnection,::rtl::OUString::createFromAscii("HY0000"),1000,Any());


    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    i = 1;
    // first the set values
    aIter = m_pColumnNames->begin();
    sal_uInt16 j = 0;
    for(;aIter != m_pColumnNames->end();++aIter,++j)
    {

        sal_Int32 nPos = aIter->second.first;
        if((*_rInsertRow)[nPos].isModified())
        {
            (*_rInsertRow)[nPos].setSigned((*_rOrginalRow)[nPos].isSigned());
            setParameter(i++,xParameter,(*_rInsertRow)[nPos],aIter->second.second);
        }
    }
    // and then the values of the where condition
    aIter = (*m_pKeyColumnNames).begin();
    j = 0;
    for(;aIter != (*m_pKeyColumnNames).end();++aIter,++i,++j)
    {
        setParameter(i,xParameter,(*_rOrginalRow)[aIter->second.first],aIter->second.second);
    }

    // now we have to set the index values
    ::std::vector<sal_Int32>::iterator aIdxColIter = aIndexColumnPositions.begin();
    j = 0;
    for(;aIdxColIter != aIndexColumnPositions.end();++aIdxColIter,++i,++j)
    {
        setParameter(i,xParameter,(*_rOrginalRow)[*aIdxColIter],(*_rOrginalRow)[*aIdxColIter].getTypeKind());
    }

     m_bUpdated = xPrep->executeUpdate() > 0;


    if(m_bUpdated)
    {
        m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32((*_rInsertRow)[0].getAny()));
        OSL_ENSURE(m_aKeyIter != m_aKeyMap.end(),"New inserted row not found!");
        m_aKeyIter->second.second = 2;
        connectivity::ORowVector< ORowSetValue >::iterator aIter = m_aKeyIter->second.first->begin();
        OColumnNamePos::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
        for(;aPosIter != (*m_pKeyColumnNames).end();++aPosIter,++aIter)
            *aIter = (*_rInsertRow)[aPosIter->second.first];
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aSql(::rtl::OUString::createFromAscii("INSERT INTO "));
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    aSql += m_aComposedTableName;
    aSql += ::rtl::OUString::createFromAscii(" ( ");
    // set values and column names
    ::rtl::OUString aValues = ::rtl::OUString::createFromAscii(" VALUES ( ");
    static ::rtl::OUString aPara = ::rtl::OUString::createFromAscii("?,");
    ::rtl::OUString aQuote = getIdentifierQuoteString();
    static ::rtl::OUString aComma = ::rtl::OUString::createFromAscii(",");

    OColumnNamePos::const_iterator aIter = m_pColumnNames->begin();
    sal_Int32 j = 1;
    for(;aIter != m_pColumnNames->end();++aIter,++j)
    {
        if((*_rInsertRow)[aIter->second.first].isModified())
        {
            aSql += ::dbtools::quoteName( aQuote,aIter->first);
            aSql += aComma;
            aValues += aPara;
        }
    }

    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
    aValues = aValues.replaceAt(aValues.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

    aSql += aValues;
    // now create,fill and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    OColumnNamePos::const_iterator aPosIter = m_pColumnNames->begin();
    sal_uInt16 k = 0;
    for(sal_Int32 i = 1;aPosIter != m_pColumnNames->end();++aPosIter)
    {
        sal_Int32 nPos = aPosIter->second.first;
        if((*_rInsertRow)[nPos].isModified())
        {
            if((*_rInsertRow)[nPos].isNull())
                xParameter->setNull(i++,(*_rInsertRow)[nPos].getTypeKind());
            else
            {
                (*_rInsertRow)[nPos].setSigned(m_aSignedFlags[nPos-1]);
                setParameter(i++,xParameter,(*_rInsertRow)[nPos],aPosIter->second.second);
            }
        }
    }

    m_bInserted = xPrep->executeUpdate() > 0;
    sal_Bool bAutoValuesFetched = sal_False;
    if ( m_bInserted )
    {
        try
        {
            Reference< XGeneratedResultSet > xGRes(xPrep, UNO_QUERY);
            if ( xGRes.is() )
            {
                Reference< XResultSet > xRes = xGRes->getGeneratedValues();
                Reference< XRow > xRow(xRes,UNO_QUERY);
                if ( xRow.is() && xRes->next() )
                {
                    Reference< XResultSetMetaDataSupplier > xMdSup(xRes,UNO_QUERY);
                    Reference< XResultSetMetaData > xMd = xMdSup->getMetaData();
                    sal_Int32 nColumnCount = xMd->getColumnCount();
                    ::std::vector< ::rtl::OUString >::iterator aAutoIter = m_aAutoColumns.begin();
                    ::std::vector< ::rtl::OUString >::iterator aAutoEnd = m_aAutoColumns.end();
                    for (sal_Int32 i = 1;aAutoIter !=  aAutoEnd && i <= nColumnCount; ++aAutoIter,++i)
                    {
#if OSL_DEBUG_LEVEL > 0
                        ::rtl::OUString sColumnName( xMd->getColumnName(i) );
#endif
                        OColumnNamePos::iterator aFind = m_pKeyColumnNames->find(*aAutoIter);
                        if(aFind != m_pKeyColumnNames->end())
                            fetchValue(i,(*_rInsertRow)[aFind->second.first].getTypeKind(),xRow,(*_rInsertRow)[aFind->second.first]);
                    }
                    bAutoValuesFetched = sal_True;
                }
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"Could not execute GeneratedKeys() stmt");
        }
    }

    ::comphelper::disposeComponent(xPrep);

    if ( !bAutoValuesFetched )
    {
        // first check if all key column values were set
        ::rtl::OUString sQuote = getIdentifierQuoteString();
        ::rtl::OUString sMaxStmt;
        ::std::vector< ::rtl::OUString >::iterator aAutoIter = m_aAutoColumns.begin();
        for (;aAutoIter !=  m_aAutoColumns.end(); ++aAutoIter)
        {
            // we will only fetch values which are keycolumns
            if ( m_pKeyColumnNames->find(*aAutoIter) != m_pKeyColumnNames->end() )
            {
                sMaxStmt += ::rtl::OUString::createFromAscii(" MAX(");
                sMaxStmt += ::dbtools::quoteName( sQuote,*aAutoIter);
                sMaxStmt += ::rtl::OUString::createFromAscii("),");
            }
        }

        if(sMaxStmt.getLength())
        {
            sMaxStmt = sMaxStmt.replaceAt(sMaxStmt.getLength()-1,1,::rtl::OUString::createFromAscii(" "));
            ::rtl::OUString sStmt = ::rtl::OUString::createFromAscii("SELECT ");
            sStmt += sMaxStmt;
            sStmt += ::rtl::OUString::createFromAscii("FROM ");
            sStmt += m_aSelectComposedTableName;
            try
            {
                // now fetch the autoincrement values
                Reference<XStatement> xStatement = m_xConnection->createStatement();
                Reference<XResultSet> xRes = xStatement->executeQuery(sStmt);
                Reference<XRow> xRow(xRes,UNO_QUERY);
                if(xRow.is() && xRes->next())
                {
                    aAutoIter = m_aAutoColumns.begin();
                    for (sal_Int32 i=1;aAutoIter !=  m_aAutoColumns.end(); ++aAutoIter,++i)
                    {
                        // we will only fetch values which are keycolumns
                        OColumnNamePos::iterator aFind = m_pKeyColumnNames->find(*aAutoIter);
                        if(aFind != m_pKeyColumnNames->end())
                            fetchValue(i,(*_rInsertRow)[aFind->second.first].getTypeKind(),xRow,(*_rInsertRow)[aFind->second.first]);
                    }
                }
                ::comphelper::disposeComponent(xStatement);
            }
            catch(SQLException&)
            {
                OSL_ENSURE(0,"Could not fetch with MAX() ");
            }
        }
    }
    if ( m_bInserted )
    {
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >((*m_pKeyColumnNames).size());
        connectivity::ORowVector< ORowSetValue >::iterator aIter = aKeyRow->begin();
        OColumnNamePos::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
        for(;aPosIter != (*m_pKeyColumnNames).end();++aPosIter,++aIter)
            *aIter = (*_rInsertRow)[aPosIter->second.first];


        OKeySetMatrix::iterator aKeyIter = m_aKeyMap.end();
        --aKeyIter;
        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(aKeyIter->first + 1,OKeySetValue(aKeyRow,1))).first;
        // now we set the bookmark for this row
        (*_rInsertRow)[0] = makeAny((sal_Int32)m_aKeyIter->first);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::deleteRow(const ORowSetRow& _rDeleteRow,const connectivity::OSQLTable& _xTable   ) throw(SQLException, RuntimeException)
{
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DELETE FROM ");
    aSql += m_aComposedTableName;
    aSql += ::rtl::OUString::createFromAscii(" WHERE ");

    // list all cloumns that should be set
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");

    // use keys and indexes for excat postioning
    Reference<XNameAccess> xKeyColumns = getKeyColumns();
    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if ( xIndexSup.is() )
        xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);

    //  Reference<XColumnsSupplier>
    ::std::vector< Reference<XNameAccess> > aAllIndexColumns;
    lcl_fillIndexColumns(xIndexes,aAllIndexColumns);

    ::rtl::OUString aColumnName,sIndexCondition;
    ::std::vector<sal_Int32> aIndexColumnPositions;
    OColumnNamePos::const_iterator aIter = m_pColumnNames->begin();

    sal_Int32 i = 1;
    for(i = 1;aIter != m_pColumnNames->end();++aIter,++i)
    {
        if(xKeyColumns.is() && xKeyColumns->hasByName(aIter->first))
        {
            aSql += ::dbtools::quoteName( aQuote,aIter->first);
            if((*_rDeleteRow)[aIter->second.first].isNull())
            {
                OSL_ENSURE(0,"can a primary key be null");
                aSql += ::rtl::OUString::createFromAscii(" IS NULL");
            }
            else
                aSql += ::rtl::OUString::createFromAscii(" = ?");
            aSql += aAnd;
        }
        else
        {
            for( ::std::vector< Reference<XNameAccess> >::const_iterator aIndexIter = aAllIndexColumns.begin();
                    aIndexIter != aAllIndexColumns.end();++aIndexIter)
            {
                if((*aIndexIter)->hasByName(aIter->first))
                {
                    sIndexCondition += ::dbtools::quoteName( aQuote,aIter->first);
                    if((*_rDeleteRow)[aIter->second.first].isNull())
                        sIndexCondition += ::rtl::OUString::createFromAscii(" IS NULL");
                    else
                    {
                        sIndexCondition += ::rtl::OUString::createFromAscii(" = ?");
                        aIndexColumnPositions.push_back(aIter->second.first);
                    }
                    sIndexCondition += aAnd;

                    break;
                }
            }
        }
    }
    aSql += sIndexCondition;
    aSql = aSql.replaceAt(aSql.getLength()-5,5,::rtl::OUString::createFromAscii(" "));

    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    aIter = (*m_pKeyColumnNames).begin();
    i = 1;
    for(;aIter != (*m_pKeyColumnNames).end();++aIter,++i)
    {
        setParameter(i,xParameter,(*_rDeleteRow)[aIter->second.first],aIter->second.second);
    }

    // now we have to set the index values
    ::std::vector<sal_Int32>::iterator aIdxColIter = aIndexColumnPositions.begin();
    for(;aIdxColIter != aIndexColumnPositions.end();++aIdxColIter,++i)
    {
        setParameter(i,xParameter,(*_rDeleteRow)[*aIdxColIter],(*_rDeleteRow)[*aIdxColIter].getTypeKind());
    }

    m_bDeleted = xPrep->executeUpdate() > 0;

    if(m_bDeleted)
    {
        sal_Int32 nBookmark = ::comphelper::getINT32((*_rDeleteRow)[0].getAny());
        if(m_aKeyIter == m_aKeyMap.find(nBookmark) && m_aKeyIter != m_aKeyMap.end())
            ++m_aKeyIter;
        m_aKeyMap.erase(nBookmark);
        m_bDeleted = sal_True;
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
}
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OKeySet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Reference<XNameAccess> OKeySet::getKeyColumns() const
{
    // use keys and indexes for excat postioning
    // first the keys
    Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
    Reference<XIndexAccess> xKeys;
    if(xKeySup.is())
        xKeys = xKeySup->getKeys();

    Reference<XColumnsSupplier> xKeyColsSup;
    Reference<XNameAccess> xKeyColumns;
    if(xKeys.is())
    {
        Reference<XPropertySet> xProp;
        sal_Int32 nCount = xKeys->getCount();
        for(sal_Int32 i = 0;i< nCount;++i)
        {
            xProp.set(xKeys->getByIndex(i),UNO_QUERY);
            if ( xProp.is() )
            {
                sal_Int32 nKeyType = 0;
                xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                if(KeyType::PRIMARY == nKeyType)
                {
                    xKeyColsSup.set(xProp,UNO_QUERY);
                    OSL_ENSURE(xKeyColsSup.is(),"Columnsupplier is null!");
                    xKeyColumns = xKeyColsSup->getColumns();
                    break;
                }
            }
        }
    }

    return xKeyColumns;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::next(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;

    if(isAfterLast())
        return sal_False;
    if(!m_bRowCountFinal) // not yet all records fetched
    {
        ++m_aKeyIter; // this is possible because we stand on begin() and this is the "beforefirst" row
        if(m_aKeyIter == m_aKeyMap.end() && !fetchRow())
            m_aKeyIter = m_aKeyMap.end();
    }
    else if(!isAfterLast())
        ++m_aKeyIter;

    refreshRow();
    return !isAfterLast();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    return m_aKeyIter == m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    return  m_bRowCountFinal && m_aKeyIter == m_aKeyMap.end();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::isFirst(  ) throw(SQLException, RuntimeException)
{
    OKeySetMatrix::iterator aTemp = m_aKeyMap.begin();
    ++aTemp;
    return m_aKeyIter == aTemp && m_aKeyIter != m_aKeyMap.end();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::isLast(  ) throw(SQLException, RuntimeException)
{
    if(!m_bRowCountFinal)
        return sal_False;

    OKeySetMatrix::iterator aTemp = m_aKeyMap.end();
    --aTemp;
    return m_aKeyIter == aTemp;
}
// -----------------------------------------------------------------------------
void SAL_CALL OKeySet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.begin();
    m_xSet = NULL;
    ::comphelper::disposeComponent(m_xRow);
}
// -----------------------------------------------------------------------------
void SAL_CALL OKeySet::afterLast(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aKeyIter = m_aKeyMap.end();
    m_xSet = NULL;
    ::comphelper::disposeComponent(m_xRow);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::first(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aKeyIter = m_aKeyMap.begin();
    ++m_aKeyIter;
    if(m_aKeyIter == m_aKeyMap.end() && !fetchRow())
        m_aKeyIter = m_aKeyMap.end();

    refreshRow();
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::last(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();

    m_aKeyIter = m_aKeyMap.end();
    --m_aKeyIter;
    refreshRow();
    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OKeySet::getRow(  ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(!isAfterLast(),"getRow is not allowed when afterlast record!");
    OSL_ENSURE(!isBeforeFirst(),"getRow is not allowed when beforefirst record!");

    return ::std::distance(m_aKeyMap.begin(),m_aKeyIter);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    OSL_ENSURE(row,"absolute(0) isn't allowed!");
    if(row < 0)
    {
        if(!m_bRowCountFinal)
            fillAllRows();

        for(;row < 0 && m_aKeyIter != m_aKeyMap.begin();++row)
            m_aKeyIter--;
    }
    else
    {
        if(row >= (sal_Int32)m_aKeyMap.size())
        {
            if(!m_bRowCountFinal)
            {
                sal_Bool bNext = sal_True;
                for(sal_Int32 i=m_aKeyMap.size()-1;i < row && bNext;++i)
                    bNext = fetchRow();
            }
            else
                m_aKeyIter = m_aKeyMap.end();
        }
        else
        {
            m_aKeyIter = m_aKeyMap.begin();
            for(;row > 0 && m_aKeyIter != m_aKeyMap.end();--row)
                ++m_aKeyIter;
        }
    }
    refreshRow();

    return m_aKeyIter != m_aKeyMap.end() && m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    if(!rows)
    {
        refreshRow();
        return sal_True;
    }
    return absolute(getRow()+rows);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::previous(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    if(m_aKeyIter != m_aKeyMap.begin())
    {
        --m_aKeyIter;
        refreshRow();
    }
    return m_aKeyIter != m_aKeyMap.begin();
}
// -----------------------------------------------------------------------------
void SAL_CALL OKeySet::refreshRow() throw(SQLException, RuntimeException)
{
    if(isBeforeFirst() || isAfterLast() || !m_xStatement.is())
        return;

    m_xSet = NULL;
    ::comphelper::disposeComponent(m_xRow);
    // we just areassign the base members
    Reference< XParameters > xParameter(m_xStatement,UNO_QUERY);
    OSL_ENSURE(xParameter.is(),"No Parameter interface!");
    xParameter->clearParameters();
    connectivity::ORowVector< ORowSetValue >::const_iterator aExternParamIter = m_aParameterRow.begin();
    sal_Int32 nPos=1;
    for(;aExternParamIter != m_aParameterRow.end();++aExternParamIter,++nPos)
    {

        switch(aExternParamIter->getTypeKind())
        {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::DECIMAL:
        case DataType::NUMERIC:
        case DataType::LONGVARCHAR:
            xParameter->setString(nPos,*aExternParamIter);
            break;
        case DataType::BIGINT:
            if ( aExternParamIter->isSigned() )
                xParameter->setLong(nPos,*aExternParamIter);
            else
                xParameter->setString(nPos,*aExternParamIter);
            break;
        case DataType::FLOAT:
            xParameter->setFloat(nPos,*aExternParamIter);
            break;
        case DataType::DOUBLE:
        case DataType::REAL:
            xParameter->setDouble(nPos,*aExternParamIter);
            break;
        case DataType::DATE:
            xParameter->setDate(nPos,*aExternParamIter);
            break;
        case DataType::TIME:
            xParameter->setTime(nPos,*aExternParamIter);
            break;
        case DataType::TIMESTAMP:
            xParameter->setTimestamp(nPos,*aExternParamIter);
            break;
        case DataType::BINARY:
        case DataType::VARBINARY:
        case DataType::LONGVARBINARY:
            xParameter->setBytes(nPos,*aExternParamIter);
            break;
        case DataType::BIT:
            xParameter->setBoolean(nPos,*aExternParamIter);
            break;
        case DataType::TINYINT:
            if ( aExternParamIter->isSigned() )
                xParameter->setByte(nPos,*aExternParamIter);
            else
                xParameter->setShort(nPos,*aExternParamIter);
            break;
        case DataType::SMALLINT:
            if ( aExternParamIter->isSigned() )
                xParameter->setShort(nPos,*aExternParamIter);
            else
                xParameter->setInt(nPos,*aExternParamIter);
            break;
        case DataType::INTEGER:
            if ( aExternParamIter->isSigned() )
                xParameter->setInt(nPos,*aExternParamIter);
            else
                xParameter->setLong(nPos,*aExternParamIter);
            break;
        case DataType::CLOB:
            {
                Reference<XInputStream> xStream(aExternParamIter->getAny(),UNO_QUERY);
                xParameter->setCharacterStream(nPos,xStream,xStream.is() ? xStream->available() : sal_Int32(0));
            }
            break;
        case DataType::BLOB:
            {
                Reference<XInputStream> xStream(aExternParamIter->getAny(),UNO_QUERY);
                xParameter->setBinaryStream(nPos,xStream,xStream.is() ? xStream->available() : sal_Int32(0));
            }
            break;
        }
    }

    connectivity::ORowVector< ORowSetValue >::const_iterator aIter = m_aKeyIter->second.first->begin();
    OColumnNamePos::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
    for(;aPosIter != (*m_pKeyColumnNames).end();++aPosIter,++aIter,++nPos)
        setParameter(nPos,xParameter,*aIter);

    m_xSet = m_xStatement->executeQuery();
    OSL_ENSURE(m_xSet.is(),"No resultset form statement!");
    sal_Bool bOK = m_xSet->next();
    OSL_ENSURE(bOK,"No rows!");
    m_xRow.set(m_xSet,UNO_QUERY);
    OSL_ENSURE(m_xRow.is(),"No row form statement!");
}
// -----------------------------------------------------------------------------
sal_Bool OKeySet::fetchRow()
{
    // fetch the next row and append on the keyset
    sal_Bool bRet = sal_False;
    if(!m_bRowCountFinal && (bRet = m_xDriverSet->next()))
    {
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >((*m_pKeyColumnNames).size());
        connectivity::ORowVector< ORowSetValue >::iterator aIter = aKeyRow->begin();
        OColumnNamePos::const_iterator aPosIter = (*m_pKeyColumnNames).begin();
        for(;aPosIter != (*m_pKeyColumnNames).end();++aPosIter,++aIter)
        {
            const TPositionTypePair& rPair = aPosIter->second;
            fetchValue(rPair.first,rPair.second,m_xDriverRow,*aIter);
        }
        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(m_aKeyMap.rbegin()->first+1,OKeySetValue(aKeyRow,0))).first;
    }
    else
        m_bRowCountFinal = sal_True;
    return bRet;
}
// -------------------------------------------------------------------------
void OKeySet::fillAllRows()
{
    if(!m_bRowCountFinal)
    {
        while(fetchRow())
            ;
    }
}
// XRow
sal_Bool SAL_CALL OKeySet::wasNull(  ) throw(SQLException, RuntimeException)
{
    return m_xRow->wasNull();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OKeySet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getString(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBoolean(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL OKeySet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getByte(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL OKeySet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getShort(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OKeySet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getInt(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL OKeySet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getLong(columnIndex);
}
// -------------------------------------------------------------------------
float SAL_CALL OKeySet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getFloat(columnIndex);
}
// -------------------------------------------------------------------------
double SAL_CALL OKeySet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getDouble(columnIndex);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OKeySet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBytes(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL OKeySet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getDate(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL OKeySet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getTime(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL OKeySet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getTimestamp(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OKeySet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBinaryStream(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OKeySet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getCharacterStream(columnIndex);
}
// -------------------------------------------------------------------------
Any SAL_CALL OKeySet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getObject(columnIndex,typeMap);
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL OKeySet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getRef(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OKeySet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getBlob(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL OKeySet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getClob(columnIndex);
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OKeySet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(m_xRow.is(),"m_xRow is null!");
    return m_xRow->getArray(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second == 2;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    return m_aKeyIter != m_aKeyMap.begin() && m_aKeyIter != m_aKeyMap.end() && m_aKeyIter->second.second == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OKeySet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    sal_Bool bDeleted = m_bDeleted;
    m_bDeleted = sal_False;
    return bDeleted;
}
// -----------------------------------------------------------------------------
void OKeySet::setExternParameters(const connectivity::ORowVector< ORowSetValue >& _rParameterRow)
{
    m_aParameterRow = _rParameterRow;
}
// -----------------------------------------------------------------------------
::rtl::OUString OKeySet::getComposedTableName(const ::rtl::OUString& _sCatalog,
                                              const ::rtl::OUString& _sSchema,
                                              const ::rtl::OUString& _sTable)
{
    ::rtl::OUString aComposedName;
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    sal_Bool bUseCatalogInSelect = ::dbtools::isDataSourcePropertyEnabled(m_xConnection,PROPERTY_USECATALOGINSELECT,sal_True);
    sal_Bool bUseSchemaInSelect = ::dbtools::isDataSourcePropertyEnabled(m_xConnection,PROPERTY_USESCHEMAINSELECT,sal_True);

    if( xMetaData.is() && xMetaData->supportsTableCorrelationNames() )
    {
        ::dbtools::composeTableName(xMetaData,_sCatalog,_sSchema,_sTable,aComposedName,sal_False,::dbtools::eInDataManipulation);
        // first we have to check if the composed tablename is in the select clause or if an alias is used
        Reference<XTablesSupplier> xTabSup(m_xComposer,UNO_QUERY);
        Reference<XNameAccess> xSelectTables = xTabSup->getTables();
        OSL_ENSURE(xSelectTables.is(),"No Select tables!");
        if(xSelectTables.is())
        {
            if(!xSelectTables->hasByName(aComposedName))
            { // the composed name isn't used in the select clause so we have to find out which name is used instead
                ::rtl::OUString sCatalog,sSchema,sTable;
                ::dbtools::qualifiedNameComponents(xMetaData,m_sUpdateTableName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);
                ::dbtools::composeTableName(xMetaData,sCatalog,sSchema,sTable,aComposedName,sal_True,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
            }
            else
                ::dbtools::composeTableName(xMetaData,_sCatalog,_sSchema,_sTable,aComposedName,sal_True,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
        }
    }
    else
        ::dbtools::composeTableName(xMetaData,_sCatalog,_sSchema,_sTable,aComposedName,sal_True,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);

    return aComposedName;
}
// -----------------------------------------------------------------------------
namespace dbaccess
{
    void getColumnPositions(const Reference<XNameAccess>& _rxQueryColumns,
                            const Reference<XNameAccess>& _rxColumns,
                            const ::rtl::OUString& _rsUpdateTableName,
                            OColumnNamePos& _rColumnNames)
    {
        // get the real name of the columns
        Sequence< ::rtl::OUString> aSelNames(_rxQueryColumns->getElementNames());
        const ::rtl::OUString* pSelBegin    = aSelNames.getConstArray();
        const ::rtl::OUString* pSelEnd      = pSelBegin + aSelNames.getLength();

        Sequence< ::rtl::OUString> aTableNames(_rxColumns->getElementNames());
        const ::rtl::OUString* pTableBegin  = aTableNames.getConstArray();
        const ::rtl::OUString* pTableEnd    = pTableBegin + aTableNames.getLength();

        ::comphelper::UStringMixLess aTmp(_rColumnNames.key_comp());
        ::comphelper::UStringMixEqual bCase(static_cast< ::comphelper::UStringMixLess*>(&aTmp)->isCaseSensitive());

        for(sal_Int32 nPos = 1;pSelBegin != pSelEnd;++pSelBegin,++nPos)
        {
            Reference<XPropertySet> xColumnProp(_rxQueryColumns->getByName(*pSelBegin),UNO_QUERY);
            ::rtl::OUString sRealName,sTableName;
            OSL_ENSURE(xColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
            OSL_ENSURE(xColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
            xColumnProp->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
            xColumnProp->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;

            for(;pTableBegin != pTableEnd;++pTableBegin)
            {
                if(bCase(sRealName,*pTableBegin) && bCase(_rsUpdateTableName,sTableName) && _rColumnNames.find(*pTableBegin) == _rColumnNames.end())
                {
                    sal_Int32 nType;
                    xColumnProp->getPropertyValue(PROPERTY_TYPE)    >>= nType;
                    _rColumnNames[sRealName] = TPositionTypePair(nPos,nType);
                    break;
                }
            }
            pTableBegin = aTableNames.getConstArray();
        }
    }
}


