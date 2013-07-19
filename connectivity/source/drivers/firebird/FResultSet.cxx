/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "FResultSet.hxx"
#include "FResultSetMetaData.hxx"
#include <propertyids.hxx>
#include <TConnection.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>

using namespace ::comphelper;
using namespace ::connectivity;
using namespace ::connectivity::firebird;
using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;

OResultSet::OResultSet(OConnection* pConnection,
                       const uno::Reference< XStatement >& xStatement,
                       isc_stmt_handle& aStatementHandle,
                       XSQLDA* pSqlda)
    : OResultSet_BASE(pConnection->getMutex())
    , OPropertySetHelper(OResultSet_BASE::rBHelper)
    , m_pConnection(pConnection)
    , m_xStatement(xStatement)
    , m_xMetaData(0)
    , m_pSqlda(pSqlda)
    , m_statementHandle(aStatementHandle)
    , m_bWasNull(false)
    , m_currentRow(0)
    , m_bIsAfterLastRow(false)
    , m_fieldCount(pSqlda? pSqlda->sqld : 0)
{
    SAL_INFO("connectivity.firebird", "OResultSet().");

    if (!pSqlda)
        return; // TODO: what?

}

OResultSet::~OResultSet()
{
}

// void OResultSet::ensureDataAvailable() throw (SQLException)
// {
//     MutexGuard aGuard(m_pConnection->getMutex());
//     checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
//
//     if (!m_bIsPopulated)
//     {
//
//         ISC_STATUS aErr = isc_dsql_free_statement(m_statusVector,
//                                                   &m_statementHandle,
//                                                   DSQL_drop);
//         // TODO: cleanup the XSQLDA, probably in the destructor?
//
//         // fetchstat == 100L if fetching of data completed successfully.
//         if ((fetchStat != 100L) || aErr)
//         {
//             SAL_WARN("connectivity.firebird", "Error when populating data");
//             OConnection::evaluateStatusVector(m_statusVector,
//                                               "isc_dsql_free_statement",
//                                               *this);
//         }
//
//         SAL_INFO("connectivity.firebird", "Populated dataset with " << m_rowCount << " rows.");
//         m_bIsPopulated = true;
//     }
// }


// ---- XResultSet -- Row retrieval methods ------------------------------------
sal_Int32 SAL_CALL OResultSet::getRow() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow;
}

sal_Bool SAL_CALL OResultSet::next() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    SAL_INFO("connectivity.firebird", "Fetching row from cursor");

    m_currentRow++;

    ISC_STATUS fetchStat = isc_dsql_fetch(m_statusVector,
                               &m_statementHandle,
                               1,
                               m_pSqlda);
    if (fetchStat == 0)         // SUCCESSFUL
    {
        return sal_True;
    }
    else if (fetchStat == 100L) // END OF DATASET
    {
        // TODO: shut the statement
        return sal_False;
    }
    else
    {
        SAL_WARN("connectivity.firebird", "Error when populating data");
        // Throws sql exception as appropriate
        OConnection::evaluateStatusVector(m_statusVector,
                                          "isc_dsql_fetch",
                                          *this);
        return sal_False;
    }

            //         {
//             m_rowCount++;
//
//             TRow aRow(m_fieldCount);
//             m_sqlData.push_back(aRow);
//             TRow& rRow = m_sqlData.back();
//
//             XSQLVAR* pVar = m_pSqlda->sqlvar;
//             for (int i = 0; i < m_fieldCount; pVar++, i++)
//             {
//                 if ((pVar->sqltype & 1) == 0) // Means: Cannot contain NULL
//                 {
//                     // TODO: test for null here and set as appropriate
//                 }
//                 else // Means: Can contain NULL
//                 {
//                     // otherwise we need to test for SQL_TYPE and SQL_TYPE+1 below
//                     pVar->sqltype--;
//                 }
//                 switch (pVar->sqltype)
//                 {
//                     case SQL_SHORT:
//                         rRow[i] = (sal_Int16) *pVar->sqldata;
//                         break;
//                     case SQL_LONG:
//                         rRow[i] = (sal_Int32) *pVar->sqldata;
//                         break;
//                     case SQL_INT64:
//                         rRow[i] = (sal_Int64) *pVar->sqldata;
//                         break;
//                     // TODO: remember sqlscale for decimal types
//                     default:
//                         rRow[i] = OUString(pVar->sqldata, pVar->sqllen, RTL_TEXTENCODING_UTF8);
//                         break;
//                 }
//             }
//         }
}

sal_Bool SAL_CALL OResultSet::previous() throw(SQLException, RuntimeException)
{
    throw SQLException("Firebird doesn't support previous()", *this, OUString(), 0, Any());
}

sal_Bool SAL_CALL OResultSet::isLast() throw(SQLException, RuntimeException)
{
    throw SQLException("Firebird doesn't support isLast()", *this, OUString(), 0, Any());
}

sal_Bool SAL_CALL OResultSet::isBeforeFirst() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow == 0;
}

sal_Bool SAL_CALL OResultSet::isAfterLast() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_bIsAfterLastRow;
}

sal_Bool SAL_CALL OResultSet::isFirst() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_currentRow == 1 && !m_bIsAfterLastRow;
}

// Move to front
void SAL_CALL OResultSet::beforeFirst() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_currentRow != 0)
        throw SQLException("Firebird doesn't support beforeFirst()", *this, OUString(), 0, Any());
}
// Move to back
void SAL_CALL OResultSet::afterLast() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (!m_bIsAfterLastRow)
        throw SQLException("Firebird doesn't support afterLast()", *this, OUString(), 0, Any());
}

sal_Bool SAL_CALL OResultSet::first() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (m_currentRow == 0)
    {
        return next();
    }
    else if (m_currentRow == 1 && !m_bIsAfterLastRow)
    {
        return true;
    }
    else
    {
           throw SQLException("Firebird doesn't support first()", *this, OUString(), 0, Any());
    }
}

sal_Bool SAL_CALL OResultSet::last() throw(SQLException, RuntimeException)
{
    // We need to iterate past the last row to know when we've passed the last
    // row, so we can't actually move to last.
        throw SQLException("Firebird doesn't support last()", *this, OUString(), 0, Any());
}

sal_Bool SAL_CALL OResultSet::absolute(sal_Int32 aRow) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (aRow > m_currentRow)
    {
        sal_Int32 aIterations = aRow - m_currentRow;
        return relative(aIterations);
    }
    else
    {
        throw SQLException("Firebird doesn't support retrieval of rows before the current row",
                            *this, OUString(), 0, Any());
    }
}

sal_Bool SAL_CALL OResultSet::relative(sal_Int32 row) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if (row > 0)
    {
        while (row--)
        {
            if (!next())
                return sal_False;
        }
        return sal_True;
    }
    else
    {
        throw SQLException("Firebird doesn't support relative() for a negative offset",
                           *this, OUString(), 0, Any());
    }
}

void SAL_CALL OResultSet::checkColumnIndex(sal_Int32 index)
    throw (SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if( index < 1 || index > m_fieldCount )
    {
        throw SQLException( "Column Index is outwith valid range", *this, OUString(), 1, Any() );
    }
}

void SAL_CALL OResultSet::checkRowIndex()
    throw (SQLException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if((m_currentRow < 1) || m_bIsAfterLastRow)
    {
        throw SQLException( "Row index is out of valid range.", *this, OUString(),1, Any() );
    }
}
// -------------------------------------------------------------------------
void OResultSet::disposing(void)
{
    SAL_INFO("connectivity.firebird", "disposing().");

    OPropertySetHelper::disposing();

    MutexGuard aGuard(m_pConnection->getMutex());

    m_xMetaData     = NULL;
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
 Sequence<  Type > SAL_CALL OResultSet::getTypes(  ) throw( RuntimeException)
{
    OTypeCollection aTypes(
        ::cppu::UnoType< uno::Reference< ::com::sun::star::beans::XMultiPropertySet > >::get(),
        ::cppu::UnoType< uno::Reference< ::com::sun::star::beans::XFastPropertySet > >::get(),
        ::cppu::UnoType< uno::Reference< ::com::sun::star::beans::XPropertySet > >::get());

    return concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::findColumn( const OUString& columnName ) throw(SQLException, RuntimeException)
{

    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // find the first column with the name columnName

    MutexGuard aGuard(m_pConnection->getMutex());

    uno::Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            break;
    return i;
}
// -------------------------------------------------------------------------
uno::Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}

// ---- Internal Utilities ---------------------------------------------------
bool OResultSet::isNull(sal_Int32 columnIndex)
{
    assert(columnIndex <= m_fieldCount);
    XSQLVAR* pVar = m_pSqlda->sqlvar;

    if (pVar[columnIndex-1].sqltype & 1) // Indicates column may contain null
    {
        if (*pVar[columnIndex-1].sqlind == -1)
            return true;
    }
    return false;
}

template <typename T>
T OResultSet::retrieveValue(sal_Int32 columnIndex)
{
    if ((m_bWasNull = isNull(columnIndex)))
        return 0;

    return *m_pSqlda->sqlvar[columnIndex-1].sqldata;
}

template <>
OUString OResultSet::retrieveValue<OUString>(sal_Int32 columnIndex)
{
    // TODO: check we don't have SQL_VARYING (first 2 bytes = short with length
    // of string.
    if ((m_bWasNull = isNull(columnIndex)))
        return OUString();

    return OUString(m_pSqlda->sqlvar[columnIndex-1].sqldata,
                    m_pSqlda->sqlvar[columnIndex-1].sqllen,
                    RTL_TEXTENCODING_UTF8);
}

template <typename T>
T OResultSet::safelyRetrieveValue(sal_Int32 columnIndex)
    throw (SQLException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    checkColumnIndex(columnIndex);
    checkRowIndex();

    return retrieveValue< T >(columnIndex);
}
// ---- Simple Numerical types -----------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    // TODO: maybe retrieve as string and test for "true", "t", "1" etc. instead?
    return safelyRetrieveValue< bool >(columnIndex);
}

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex )
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< sal_Int8 >(columnIndex);
}

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return Sequence< sal_Int8 >(); // TODO: implement
    //return safelyRetrieveValue(columnIndex);
}

sal_Int16 SAL_CALL OResultSet::getShort(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< sal_Int16 >(columnIndex);
}

sal_Int32 SAL_CALL OResultSet::getInt(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< sal_Int32 >(columnIndex);
}

sal_Int64 SAL_CALL OResultSet::getLong(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< sal_Int64 >(columnIndex);
}

float SAL_CALL OResultSet::getFloat(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return 0.0f; // TODO: implement
//     return safelyRetrieveValue(columnIndex);
}

double SAL_CALL OResultSet::getDouble(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return 0.0; // TODO: implement
//     return safelyRetrieveValue(columnIndex);
}

// ---- More complex types ---------------------------------------------------
OUString SAL_CALL OResultSet::getString(sal_Int32 columnIndex)
    throw(SQLException, RuntimeException)
{
    return safelyRetrieveValue< OUString >(columnIndex);
}

Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return Time();
//     return safelyRetrieveValue(columnIndex);
}

DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return DateTime(); // TODO: implement
//     return safelyRetrieveValue(columnIndex);
}

Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    return Date(); // TODO: implement
//     return safelyRetrieveValue(columnIndex);
}
// -------------------------------------------------------------------------
uno::Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pConnection, m_pSqlda);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
uno::Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}

// -------------------------------------------------------------------------

uno::Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------
uno::Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------

uno::Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) typeMap;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return Any();
}



// -------------------------------------------------------------------------

void SAL_CALL OResultSet::close(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "close().");

    {
        MutexGuard aGuard(m_pConnection->getMutex());
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    }
    dispose();
}


uno::Reference< XInterface > SAL_CALL OResultSet::getStatement()
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return m_xStatement;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return sal_False;
}

// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bWasNull;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::insertRow(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // you only have to implement this if you want to insert new rows
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    // only when you allow updates
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    // only when you allow insert's
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_pConnection->getMutex());

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    MutexGuard aGuard(m_pConnection->getMutex());

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -----------------------------------------------------------------------
void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const OUString& x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const Date& x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const Time& x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const DateTime& x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const uno::Reference< XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    (void) length;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const uno::Reference< XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    (void) length;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    (void) columnIndex;
    (void) x;
    (void) scale;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL OResultSet::getBookmark(  ) throw( SQLException,  RuntimeException)
{
     MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    // if you don't want to support bookmark you must remove the XRowLocate interface

     return Any();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveToBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    (void) bookmark;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows ) throw( SQLException,  RuntimeException)
{
    (void) bookmark;
    (void) rows;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::compareBookmarks( const  Any& aFirst, const  Any& aSecond ) throw( SQLException,  RuntimeException)
{
    (void) aFirst;
    (void) aSecond;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return CompareBookmark::NOT_EQUAL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks(  ) throw( SQLException,  RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::hashBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    (void) bookmark;
    throw SQLException();
}
// -------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows( const  Sequence<  Any >& rows ) throw( SQLException,  RuntimeException)
{
    (void) rows;
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(6);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP1IMPL(CURSORNAME,          OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
    DECL_BOOL_PROP1IMPL(ISBOOKMARKABLE) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *const_cast<OResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    (void) rConvertedValue;
    (void) rOldValue;
    (void) rValue;
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void OResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    (void) rValue;
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_FETCHSIZE:
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    (void) rValue;
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            ;
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::release() throw()
{
    OResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------

// ---- XServiceInfo -----------------------------------------------------------
OUString SAL_CALL OResultSet::getImplementationName() throw ( RuntimeException)
{
    return OUString("com.sun.star.sdbcx.firebird.ResultSet");
}

Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames()
    throw( RuntimeException)
{
     Sequence< OUString > aSupported(2);
    aSupported[0] = OUString("com.sun.star.sdbc.ResultSet");
    aSupported[1] = OUString("com.sun.star.sdbcx.ResultSet");
    return aSupported;
}

sal_Bool SAL_CALL OResultSet::supportsService(const OUString& _rServiceName)
    throw( RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
