/*************************************************************************
 *
 *  $RCSfile: KeySet.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-02 11:14:53 $
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
#ifndef _LIST_
#include <list>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
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

// -------------------------------------------------------------------------
OKeySet::OKeySet(const Reference< XResultSet>& _xDriverSet,
                 const connectivity::OSQLTable& _xTable,
                 const ::rtl::OUString& _rUpdateTableName,    // this can be the alias or the full qualified name
                 const Reference< XSQLQueryComposer >& _xComposer)
            : OCacheSet(_xDriverSet)
            ,m_xTable(_xTable)
            ,m_bRowCountFinal(sal_False)
            ,m_xComposer(_xComposer)
            ,m_sUpdateTableName(_rUpdateTableName)
            ,m_aKeyColumnNames(m_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers() ? true : false)
            ,m_aColumnNames(m_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers() ? true : false)
{
    try
    {
        Reference<XNameAccess> xKeyColumns  = getKeyColumns();
        Reference<XColumnsSupplier> xSup(m_xComposer,UNO_QUERY);
        ::dbaccess::getColumnPositions(xSup->getColumns(),xKeyColumns,m_sUpdateTableName,m_aKeyColumnNames);
        ::dbaccess::getColumnPositions(xSup->getColumns(),_xTable->getColumns(),m_sUpdateTableName,m_aColumnNames);

        // the first row is empty because it's now easier for us to distinguish when we are beforefirst or first
        // without extra varaible to be set
        m_aKeyMap.insert(OKeySetMatrix::value_type(0,OKeySetValue(NULL,0)));

        m_aKeyIter = m_aKeyMap.begin();

        static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
        static ::rtl::OUString aQuote   = m_xConnection->getMetaData()->getIdentifierQuoteString();

        ::rtl::OUString aFilter,aCatalog,aSchema,aTable,aComposedName;

        Reference<XPropertySet> xTableProp(_xTable,UNO_QUERY);
        xTableProp->getPropertyValue(PROPERTY_CATALOGNAME)  >>= aCatalog;
        xTableProp->getPropertyValue(PROPERTY_SCHEMANAME)   >>= aSchema;
        xTableProp->getPropertyValue(PROPERTY_NAME)         >>= aTable;

        ::dbtools::composeTableName(m_xConnection->getMetaData(),aCatalog,aSchema,aTable,aComposedName,sal_True);

        // create the where clause
        OColumnNamePos::const_iterator aIter;
        for(aIter = m_aKeyColumnNames.begin();aIter != m_aKeyColumnNames.end();)
        {
            aFilter += aComposedName;
            aFilter += ::rtl::OUString::createFromAscii(".");
            aFilter += ::dbtools::quoteName( aQuote,aIter->first);
            aFilter += ::rtl::OUString::createFromAscii(" = ?");
            ++aIter;
            if(aIter != m_aKeyColumnNames.end())
                aFilter += aAnd;
        }
        _xComposer->setFilter(aFilter);
        m_xStatement = m_xConnection->prepareStatement(_xComposer->getComposedQuery());
    }
    catch(SQLException&)
    {
    }
}
// -----------------------------------------------------------------------------
OKeySet::~OKeySet()
{
    ::comphelper::disposeComponent(m_xStatement);
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
    static ::rtl::OUString aQuote   = m_xConnection->getMetaData()->getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");
    static ::rtl::OUString aOr      = ::rtl::OUString::createFromAscii(" OR ");
    static ::rtl::OUString aEqual   = ::rtl::OUString::createFromAscii(" = ?");


    // use keys and indexes for excat postioning
    // first the keys
    Reference<XNameAccess> xKeyColumns = getKeyColumns();

    ::rtl::OUString aCondition = ::rtl::OUString::createFromAscii("( ");
    OColumnNamePos::const_iterator aIter = m_aKeyColumnNames.begin();
    for(;aIter != m_aKeyColumnNames.end();++aIter)
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
            for(;aKeyIter != m_aKeyIter->second.first->end();++aKeyIter)
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
            m_aKeyMap.erase(nPos);
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
    static ::rtl::OUString aQuote   = m_xConnection->getMetaData()->getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");

    // use keys and indexes for excat postioning
    // first the keys
    Reference<XNameAccess> xKeyColumns = getKeyColumns();

    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if(xIndexSup.is())
        xIndexes = Reference<XIndexAccess>(xIndexSup->getIndexes(),UNO_QUERY);

    Reference<XPropertySet> xIndexColsSup;
    Reference<XNameAccess> xIndexColumns;
    ::std::vector< Reference<XNameAccess> > aAllIndexColumns;
    OSL_ENSURE(xIndexes.is(),"No indexes!");
    if(xIndexes.is())
    {
        for(sal_Int32 j=0;j<xIndexes->getCount();++j)
        {
            xIndexes->getByIndex(j) >>= xIndexColsSup;
            if( xIndexColsSup.is()
                && connectivity::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISUNIQUE))
                && !connectivity::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISPRIMARYKEYINDEX))
              )
                aAllIndexColumns.push_back(Reference<XColumnsSupplier>(xIndexColsSup,UNO_QUERY)->getColumns());
        }
    }

    ::rtl::OUString aColumnName;
    ::rtl::OUString aCondition,sSetValues;

    // here we build the condition part for the update statement
    OColumnNamePos::const_iterator aIter = m_aColumnNames.begin();
    for(;aIter != m_aColumnNames.end();++aIter)
    {
        if(xKeyColumns.is() && xKeyColumns->hasByName(aIter->first))
        {
            aCondition += ::dbtools::quoteName( aQuote,aIter->first);
            if((*_rOrginalRow)[aIter->second].isNull())
                aCondition += ::rtl::OUString::createFromAscii(" IS NULL");
            else
                aCondition += ::rtl::OUString::createFromAscii(" = ?");
            aCondition += aAnd;
        }
        for( ::std::vector< Reference<XNameAccess> >::const_iterator aIndexIter = aAllIndexColumns.begin();
                aIndexIter != aAllIndexColumns.end();++aIndexIter)
        {
            if((*aIndexIter)->hasByName(aIter->first))
            {
                aCondition += ::dbtools::quoteName( aQuote,aIter->first);
                if((*_rOrginalRow)[aIter->second].isNull())
                    aCondition += ::rtl::OUString::createFromAscii(" IS NULL");
                else
                    aCondition += ::rtl::OUString::createFromAscii(" = ?");
                aCondition += aAnd;
                break;
            }
        }
        if((*_rInsertRow)[aIter->second].isModified())
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
        throw SQLException(::rtl::OUString::createFromAscii("No modified Values!"),m_xConnection,::rtl::OUString::createFromAscii("HY0000"),1000,Any());

    if(aCondition.getLength())
    {
        aCondition = aCondition.replaceAt(aCondition.getLength()-5,5,::rtl::OUString::createFromAscii(" "));

        aSql += ::rtl::OUString::createFromAscii(" WHERE ");
        aSql += aCondition;
    }
    else
        throw SQLException(::rtl::OUString::createFromAscii("No where condition!"),m_xConnection,::rtl::OUString::createFromAscii("HY0000"),1000,Any());


    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    sal_Int32 i = 1;
    // first the set values
    aIter = m_aColumnNames.begin();
    for(;aIter != m_aColumnNames.end();++aIter)
    {
        if((*_rInsertRow)[aIter->second].isModified())
            setParameter(i++,xParameter,(*_rInsertRow)[aIter->second]);
    }
    // and then the values of the where condition
    aIter = m_aKeyColumnNames.begin();
    for(;aIter != m_aKeyColumnNames.end();++aIter,++i)
    {
        setParameter(i,xParameter,(*_rOrginalRow)[aIter->second]);
    }

     m_bUpdated = xPrep->executeUpdate() > 0;


    if(m_bUpdated)
    {
        m_aKeyIter = m_aKeyMap.find(::comphelper::getINT32((*_rInsertRow)[0].getAny()));
        OSL_ENSURE(m_aKeyIter != m_aKeyMap.end(),"New inserted row not found!");
        m_aKeyIter->second.second = 2;
        connectivity::ORowVector< ORowSetValue >::iterator aIter = m_aKeyIter->second.first->begin();
        OColumnNamePos::const_iterator aPosIter = m_aKeyColumnNames.begin();
        for(;aPosIter != m_aKeyColumnNames.end();++aPosIter,++aIter)
            *aIter = (*_rInsertRow)[aPosIter->second];
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
    static ::rtl::OUString aQuote = m_xConnection->getMetaData()->getIdentifierQuoteString();
    static ::rtl::OUString aComma = ::rtl::OUString::createFromAscii(",");

    OColumnNamePos::const_iterator aIter = m_aColumnNames.begin();
    for(;aIter != m_aColumnNames.end();++aIter)
    {
        aSql += ::dbtools::quoteName( aQuote,aIter->first);
        aSql += aComma;
        aValues += aPara;
    }

    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
    aValues = aValues.replaceAt(aValues.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

    aSql += aValues;
    // now create,fill and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    OColumnNamePos::const_iterator aPosIter = m_aColumnNames.begin();
    //  for(aIter = _rInsertRow->begin()+1; aIter != _rInsertRow->end();++aIter,++i)
    for(sal_Int32 i = 1;aPosIter != m_aColumnNames.end();++aPosIter,++i)
    {
        if((*_rInsertRow)[aPosIter->second].isNull())
            xParameter->setNull(i,(*_rInsertRow)[aPosIter->second].getTypeKind());
        else
            setParameter(i,xParameter,(*_rInsertRow)[aPosIter->second]);

//          if(aIter->isNull())
//              xParameter->setNull(i,aIter->getTypeKind());
//          else
//              setParameter(i,xParameter,*aIter);
    }

    m_bInserted = xPrep->executeUpdate() > 0;

    //  OCacheSet::insertRow( _rInsertRow,_xTable);
    if(m_bInserted)
    {
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >(m_aKeyColumnNames.size());
        connectivity::ORowVector< ORowSetValue >::iterator aIter = aKeyRow->begin();
        OColumnNamePos::const_iterator aPosIter = m_aKeyColumnNames.begin();
        for(;aPosIter != m_aKeyColumnNames.end();++aPosIter,++aIter)
            *aIter = (*_rInsertRow)[aPosIter->second];

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
    static ::rtl::OUString aQuote   = m_xConnection->getMetaData()->getIdentifierQuoteString();
    static ::rtl::OUString aAnd     = ::rtl::OUString::createFromAscii(" AND ");

    // use keys and indexes for excat postioning
    Reference<XNameAccess> xKeyColumns = getKeyColumns();
    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if(xIndexSup.is())
        xIndexes = Reference<XIndexAccess>(xIndexSup->getIndexes(),UNO_QUERY);

    //  Reference<XColumnsSupplier>
    Reference<XPropertySet> xIndexColsSup;
    Reference<XNameAccess> xIndexColumns;
    ::std::vector< Reference<XNameAccess> > aAllIndexColumns;
    if(xIndexes.is())
    {
        for(sal_Int32 j=0;j<xIndexes->getCount();++j)
        {
            xIndexes->getByIndex(j) >>= xIndexColsSup;
            if( xIndexColsSup.is()
                && connectivity::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISUNIQUE))
                && !connectivity::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISPRIMARYKEYINDEX))
              )
                aAllIndexColumns.push_back(Reference<XColumnsSupplier>(xIndexColsSup,UNO_QUERY)->getColumns());
        }
    }

    ::rtl::OUString aColumnName;

    OColumnNamePos::const_iterator aIter = m_aColumnNames.begin();
    for(;aIter != m_aColumnNames.end();++aIter)
    {
        if(xKeyColumns.is() && xKeyColumns->hasByName(aIter->first))
        {
            aSql += ::dbtools::quoteName( aQuote,aIter->first);
            if((*_rDeleteRow)[aIter->second].isNull())
            {
                OSL_ENSURE(0,"can a primary key be null");
                aSql += ::rtl::OUString::createFromAscii(" IS NULL");
            }
            else
                aSql += ::rtl::OUString::createFromAscii(" = ?");
            aSql += aAnd;
        }
//      for( ::std::vector< Reference<XNameAccess> >::const_iterator aIndexIter = aAllIndexColumns.begin();
//              aIndexIter != aAllIndexColumns.end();++aIndexIter)
//      {
//          if((*aIndexIter)->hasByName(aIter->first))
//          {
//              aSql += ::dbtools::quoteName( aQuote,aIter->first);
//              if((*_rDeleteRow)[aIter->second].isNull())
//                  aSql += ::rtl::OUString::createFromAscii(" IS NULL");
//              else
//                  aSql += ::rtl::OUString::createFromAscii(" = ?");
//              aSql += aAnd;
//              break;
//          }
//      }
    }
    aSql = aSql.replaceAt(aSql.getLength()-5,5,::rtl::OUString::createFromAscii(" "));

    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);

    aIter = m_aKeyColumnNames.begin();
    for(sal_Int32 i = 1;aIter != m_aKeyColumnNames.end();++aIter,++i)
    {
        setParameter(i,xParameter,(*_rDeleteRow)[aIter->second]);
    }

    m_bDeleted = xPrep->executeUpdate() > 0;

    if(m_bDeleted)
    {
        sal_Int32 nPos = ::comphelper::getINT32((*_rDeleteRow)[0].getAny());
        m_aKeyMap.erase(nPos);
        m_aKeyIter = m_aKeyMap.end();
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
        for(sal_Int32 i=0;i< xKeys->getCount();++i)
        {
            xKeys->getByIndex(i) >>= xProp;
            sal_Int32 nKeyType = 0;
            xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
            if(KeyType::PRIMARY == nKeyType)
            {
                xKeyColsSup = Reference<XColumnsSupplier>(xProp,UNO_QUERY);
                OSL_ENSURE(xKeyColsSup.is(),"Columnsupplier is null!");
                xKeyColumns = xKeyColsSup->getColumns();
                break;
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

    OKeySetMatrix::iterator aTemp = m_aKeyIter;
    sal_Int32 i=0;
    while(aTemp-- != m_aKeyMap.begin())
        ++i;

    return i;
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

    connectivity::ORowVector< ORowSetValue >::iterator aIter = m_aKeyIter->second.first->begin();
    OColumnNamePos::const_iterator aPosIter = m_aKeyColumnNames.begin();
    for(sal_Int32 nPos=1;aPosIter != m_aKeyColumnNames.end();++aPosIter,++aIter,++nPos)
    {
        switch(aIter->getTypeKind())
        {
        case DataType::CHAR:
        case DataType::VARCHAR:
            xParameter->setString(nPos,*aIter);
            break;
        case DataType::DOUBLE:
        case DataType::FLOAT:
        case DataType::REAL:
        case DataType::DECIMAL:
        case DataType::NUMERIC:
            xParameter->setDouble(nPos,*aIter);
            break;
        case DataType::DATE:
            xParameter->setDate(nPos,*aIter);
            break;
        case DataType::TIME:
            xParameter->setTime(nPos,*aIter);
            break;
        case DataType::TIMESTAMP:
            xParameter->setTimestamp(nPos,*aIter);
            break;
        case DataType::BINARY:
        case DataType::VARBINARY:
        case DataType::LONGVARBINARY:
        case DataType::LONGVARCHAR:
            xParameter->setBytes(nPos,*aIter);
            break;
        case DataType::BIT:
            xParameter->setBoolean(nPos,*aIter);
            break;
        case DataType::TINYINT:
            xParameter->setByte(nPos,*aIter);
            break;
        case DataType::SMALLINT:
            xParameter->setShort(nPos,*aIter);
            break;
        case DataType::INTEGER:
            xParameter->setInt(nPos,*aIter);
            break;
        case DataType::CLOB:
            {
                Reference<XInputStream> xStream;
                aIter->getAny() >>= xStream;
                xParameter->setCharacterStream(nPos,xStream,xStream.is() ? xStream->available() : sal_Int32(0));
            }
            break;
        case DataType::BLOB:
            {
                Reference<XInputStream> xStream;
                aIter->getAny() >>= xStream;
                xParameter->setBinaryStream(nPos,xStream,xStream.is() ? xStream->available() : sal_Int32(0));
            }
            break;
        }
    }

    m_xSet = m_xStatement->executeQuery();
    OSL_ENSURE(m_xSet.is(),"No resultset form statement!");
    sal_Bool bOK = m_xSet->next();
    OSL_ENSURE(bOK,"No rows!");
    m_xRow = Reference< XRow>(m_xSet,UNO_QUERY);
    OSL_ENSURE(m_xRow.is(),"No row form statement!");
}
// -----------------------------------------------------------------------------
sal_Bool OKeySet::fetchRow()
{
    sal_Bool bRet;
    if(!m_bRowCountFinal && (bRet = m_xDriverSet->next()))
    {
        ORowSetRow aKeyRow = new connectivity::ORowVector< ORowSetValue >(m_aKeyColumnNames.size());
        connectivity::ORowVector< ORowSetValue >::iterator aIter = aKeyRow->begin();
        OColumnNamePos::const_iterator aPosIter = m_aKeyColumnNames.begin();
        for(;aPosIter != m_aKeyColumnNames.end();++aPosIter,++aIter)
        {
            sal_Int32 nType = m_xSetMetaData->getColumnType(aPosIter->second);

            switch(nType)
            {
            case DataType::CHAR:
            case DataType::VARCHAR:
                (*aIter) = m_xDriverRow->getString(aPosIter->second);
                break;
            case DataType::DOUBLE:
            case DataType::FLOAT:
            case DataType::REAL:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
                (*aIter) = m_xDriverRow->getDouble(aPosIter->second);
                break;
            case DataType::DATE:
                (*aIter) = m_xDriverRow->getDate(aPosIter->second);
                break;
            case DataType::TIME:
                (*aIter) = m_xDriverRow->getTime(aPosIter->second);
                break;
            case DataType::TIMESTAMP:
                (*aIter) = m_xDriverRow->getTimestamp(aPosIter->second);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::LONGVARCHAR:
                (*aIter) = m_xDriverRow->getBytes(aPosIter->second);
                break;
            case DataType::BIT:
                (*aIter) = m_xDriverRow->getBoolean(aPosIter->second);
                break;
            case DataType::TINYINT:
                (*aIter) = (sal_Int32)m_xDriverRow->getByte(aPosIter->second);
                break;
            case DataType::SMALLINT:
                (*aIter) = (sal_Int32)m_xDriverRow->getShort(aPosIter->second);
                break;
            case DataType::INTEGER:
                (*aIter) = m_xDriverRow->getInt(aPosIter->second);
                break;
            case DataType::CLOB:
                *aIter = makeAny(m_xDriverRow->getCharacterStream(aPosIter->second));
                aIter->setTypeKind(DataType::CLOB);
                break;
            case DataType::BLOB:
                *aIter = makeAny(m_xDriverRow->getBinaryStream(aPosIter->second));
                aIter->setTypeKind(DataType::BLOB);
                break;
            }
            if(m_xDriverRow->wasNull())
                aIter->setNull();
            aIter->setTypeKind(nType);
        }
        m_aKeyIter = m_aKeyMap.insert(OKeySetMatrix::value_type(m_aKeyMap.size(),OKeySetValue(aKeyRow,0))).first;
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

        ::comphelper::UStringMixEqual bCase(static_cast< ::comphelper::UStringMixLess*>(&_rColumnNames.key_comp())->isCaseSensitive());

        for(sal_Int32 nPos = 1;pSelBegin != pSelEnd;++pSelBegin,++nPos)
        {
            Reference<XPropertySet> xColumnProp;
            _rxQueryColumns->getByName(*pSelBegin) >>= xColumnProp;
            ::rtl::OUString sRealName,sTableName;
            OSL_ENSURE(xColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
            OSL_ENSURE(xColumnProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
            xColumnProp->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
            xColumnProp->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;

            for(;pTableBegin != pTableEnd;++pTableBegin)
            {
                if(bCase(sRealName,*pTableBegin) && bCase(_rsUpdateTableName,sTableName) && _rColumnNames.find(*pTableBegin) == _rColumnNames.end())
                {
                    _rColumnNames[sRealName] = nPos;
                    if((sal_Int32)_rColumnNames.size() == aTableNames.getLength())
                        return; // early break we found all columns
                    break;
                }
            }
            pTableBegin = aTableNames.getConstArray();
        }
    }
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.12  2001/02/06 12:51:13  rt
    #65293# syntax

    Revision 1.11  2001/02/01 14:23:57  oj
    change for insert , delete and update rows

    Revision 1.10  2001/01/31 12:35:35  oj
    use of qouteName

    Revision 1.9  2001/01/30 14:27:46  oj
    new member which holds the column names

    Revision 1.8  2001/01/24 09:50:49  oj
    #82628# rowset modifications

    Revision 1.7  2001/01/22 07:38:23  oj
    #82632# change member

    Revision 1.6  2000/11/15 15:57:40  oj
    change for rowset

    Revision 1.5  2000/10/30 09:24:02  oj
    use tablecontainer if no tablesupplier is supported

    Revision 1.4  2000/10/25 07:30:24  oj
    make strings unique for lib's

    Revision 1.3  2000/10/17 10:18:12  oj
    some changes for the rowset

    Revision 1.2  2000/10/11 11:18:10  fs
    replace unotools with comphelper

    Revision 1.1.1.1  2000/09/19 00:15:38  hr
    initial import

    Revision 1.2  2000/09/18 14:52:46  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:19:46  oj
    rowset addons

    Revision 1.0 01.08.2000 09:09:34  oj
------------------------------------------------------------------------*/


