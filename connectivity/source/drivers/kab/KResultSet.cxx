/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "KResultSet.hxx"
#include "KResultSetMetaData.hxx"
#include "KConnection.hxx"
#include "kcondition.hxx"
#include "korder.hxx"
#include "kfields.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include "TConnection.hxx"
#include <connectivity/dbexception.hxx>
#include "resource/kab_res.hrc"
#include "resource/sharedresources.hxx"

using namespace connectivity::kab;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(KabResultSet, "com.sun.star.sdbc.drivers.KabResultSet", "com.sun.star.sdbc.ResultSet");
// -------------------------------------------------------------------------
KabResultSet::KabResultSet(KabCommonStatement* pStmt)
    : KabResultSet_BASE(m_aMutex),
      OPropertySetHelper(KabResultSet_BASE::rBHelper),
      m_xStatement(pStmt),
      m_xMetaData(NULL),
      m_aKabAddressees(),
      m_nRowPos(-1),
      m_bWasNull(sal_True)
{
}
// -------------------------------------------------------------------------
KabResultSet::~KabResultSet()
{
}
// -------------------------------------------------------------------------
void KabResultSet::allKabAddressees()
{
    KabConnection* pConnection = static_cast< KabConnection *>(m_xStatement->getConnection().get());
    KABC::AddressBook* pAddressBook = pConnection->getAddressBook();

    m_aKabAddressees = pAddressBook->allAddressees();
}
// -------------------------------------------------------------------------
void KabResultSet::someKabAddressees(const KabCondition *pCondition)
{
    KabConnection* pConnection = static_cast< KabConnection *>(m_xStatement->getConnection().get());
    KABC::AddressBook* pAddressBook = pConnection->getAddressBook();

    KABC::AddressBook::Iterator iterator;

    for (iterator = pAddressBook->begin();
         iterator != pAddressBook->end();
         ++iterator)
    {
        if (pCondition->eval(*iterator))
            m_aKabAddressees.push_back(*iterator);
    }
}
// -------------------------------------------------------------------------
void KabResultSet::sortKabAddressees(const KabOrder *pOrder)
{
    // We do not use class KAddresseeList, which has a sorting algorithm in it, because
    // it uses templates. It would expand to more or less the same code as the one
    // which follows, but it would need not be called in a much less convenient way.

    KABC::Addressee::List::Iterator
        begin = m_aKabAddressees.begin(),
        end = m_aKabAddressees.end(),
        iterator;

    // Bubble sort. Feel free to implement a better algorithm.
    while (begin != end)
    {
        end--;
        for (iterator = begin; iterator != end; ++iterator)
        {
            if (pOrder->compare(*iterator, *end) > 0)
                qSwap(*iterator, *end);
        }
    }
}
// -------------------------------------------------------------------------
void KabResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

m_xStatement.clear();
m_xMetaData.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL KabResultSet::queryInterface(const Type & rType) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if (!aRet.hasValue())
        aRet = KabResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::acquire() throw()
{
    KabResultSet_BASE::acquire();
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::release() throw()
{
    KabResultSet_BASE::release();
}
// -------------------------------------------------------------------------
Sequence<  Type > SAL_CALL KabResultSet::getTypes() throw(RuntimeException)
{
    OTypeCollection aTypes(
        ::getCppuType( (const Reference< ::com::sun::star::beans::XMultiPropertySet >*) 0),
        ::getCppuType( (const Reference< ::com::sun::star::beans::XFastPropertySet >*) 0),
        ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet >*) 0));

    return comphelper::concatSequences(aTypes.getTypes(), KabResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL KabResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSet::findColumn(const ::rtl::OUString& columnName) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    // find the first column with the name columnName
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();

    for (sal_Int32 i = 1; i <= nLen; ++i)
        if (xMeta->isCaseSensitive(i) ?
            columnName == xMeta->getColumnName(i) :
            columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
                return i;

    ::connectivity::SharedResources aResources;
    const ::rtl::OUString sError( aResources.getResourceStringWithSubstitution(
            STR_INVALID_COLUMNNAME,
            "$columnname$",columnName
         ) );
    ::dbtools::throwGenericSQLException(sError,NULL);

    // Unreachable:
    OSL_ASSERT(false);
    return 0;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL KabResultSet::getString(sal_Int32 columnIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    ::rtl::OUString aRet;
    sal_Int32 nAddressees = m_aKabAddressees.size();
    ::KABC::Field::List aFields = ::KABC::Field::allFields();

    if (m_nRowPos != -1 && m_nRowPos != nAddressees && m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(columnIndex);
        QString aQtName;

        switch (nFieldNumber)
        {
            case KAB_FIELD_REVISION:
// trigger an exception here
m_bWasNull = true;
return aRet;
            default:
                aQtName = aFields[nFieldNumber - KAB_DATA_FIELDS]->value(m_aKabAddressees[m_nRowPos]);
        }
// KDE address book currently does not use NULL values.
// But it might do it someday
        if (!aQtName.isNull())
        {
            m_bWasNull = false;
            aRet = ::rtl::OUString((const sal_Unicode *) aQtName.ucs2());
            return aRet;
        }
    }
// Trigger an exception ?
    m_bWasNull = true;
    return aRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::getBoolean(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getBoolean", NULL);

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL KabResultSet::getByte(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getByte", NULL);

    sal_Int8 nRet = 0;
    return nRet;
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL KabResultSet::getShort(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getShort", NULL);

    sal_Int16 nRet = 0;
    return nRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSet::getInt(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getInt", NULL);

    sal_Int32 nRet = 0;
    return nRet;
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL KabResultSet::getLong(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getLong", NULL);

    return sal_Int64();
}
// -------------------------------------------------------------------------
float SAL_CALL KabResultSet::getFloat(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getFloat", NULL);

    float nVal(0);
    return nVal;
}
// -------------------------------------------------------------------------
double SAL_CALL KabResultSet::getDouble(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getDouble", NULL);

    double nRet = 0;
    return nRet;
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL KabResultSet::getBytes(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("", NULL);

    return Sequence< sal_Int8 >();
}
// -------------------------------------------------------------------------
Date SAL_CALL KabResultSet::getDate(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getDate", NULL);

    Date aRet;
    return aRet;
}
// -------------------------------------------------------------------------
Time SAL_CALL KabResultSet::getTime(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getTime", NULL);

    Time nRet;
    return nRet;
}
// -------------------------------------------------------------------------
DateTime SAL_CALL KabResultSet::getTimestamp(sal_Int32 columnIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    DateTime nRet;
    sal_Int32 nAddressees = m_aKabAddressees.size();

    if (m_nRowPos != -1 && m_nRowPos != nAddressees && m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(columnIndex);

        if (nFieldNumber == KAB_FIELD_REVISION)
        {
            QDateTime nRevision(m_aKabAddressees[m_nRowPos].revision());

            if (!nRevision.isNull())
            {
                m_bWasNull = false;
                nRet.Year = nRevision.date().year();
                nRet.Month = nRevision.date().month();
                nRet.Day = nRevision.date().day();
                nRet.Hours = nRevision.time().hour();
                nRet.Minutes = nRevision.time().minute();
                nRet.Seconds = nRevision.time().second();
                nRet.HundredthSeconds = nRevision.time().msec() / 10;
                return nRet;
            }
        }
        else {
            ;
        }
// trigger an exception here
    }
// Trigger an exception ?
    m_bWasNull = true;
    return nRet;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL KabResultSet::getBinaryStream(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getBinaryStream", NULL);

    return NULL;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL KabResultSet::getCharacterStream(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getCharacterStream", NULL);

    return NULL;
}
// -------------------------------------------------------------------------
Any SAL_CALL KabResultSet::getObject(sal_Int32, const Reference< ::com::sun::star::container::XNameAccess >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getObject", NULL);

    return Any();
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL KabResultSet::getRef(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getRef", NULL);

    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL KabResultSet::getBlob(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getBlob", NULL);

    return NULL;
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL KabResultSet::getClob(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getClob", NULL);

    return NULL;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL KabResultSet::getArray(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedException("getArray", NULL);

    return NULL;
}
// -------------------------------------------------------------------------
Reference< XResultSetMetaData > SAL_CALL KabResultSet::getMetaData() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    if (!m_xMetaData.is())
        m_xMetaData = new KabResultSetMetaData;

    Reference< XResultSetMetaData > xMetaData = m_xMetaData.get();
    return xMetaData;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::isBeforeFirst() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    if (m_nRowPos == -1)
        return sal_True;

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::isAfterLast() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nAddressees = m_aKabAddressees.size();
    if (m_nRowPos == nAddressees)
        return sal_True;

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::isFirst() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    if (m_nRowPos == 0)
        return sal_True;

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::isLast() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nAddressees = m_aKabAddressees.size();
    if (m_nRowPos == nAddressees - 1)
        return sal_True;

    return sal_False;
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::beforeFirst() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    // move before the first row
    m_nRowPos = -1;
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::afterLast() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    // move after the last row
    sal_Int32 nAddressees = m_aKabAddressees.size();
    m_nRowPos = nAddressees;
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::close() throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
    }
    dispose();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::first() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nAddressees = m_aKabAddressees.size();
    if (nAddressees == 0)
        return sal_False;

    m_nRowPos = 0;
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::last() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nAddressees = m_aKabAddressees.size();
    if (nAddressees == 0)
        return sal_False;

    m_nRowPos = nAddressees - 1;
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSet::getRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return m_nRowPos;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::absolute(sal_Int32 row) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nAddressees = m_aKabAddressees.size();
    if (row <= -1 ||
        row >= nAddressees)
        return sal_False;

    m_nRowPos = row;
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::relative(sal_Int32 row) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos + row);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::next() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos + 1);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::previous() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos - 1);
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL KabResultSet::getStatement() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    Reference< XStatement > xStatement = m_xStatement.get();
    return xStatement;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::rowDeleted() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::rowInserted() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::rowUpdated() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::wasNull() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::cancel() throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::clearWarnings() throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL KabResultSet::getWarnings() throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::insertRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    // you only have to implement this if you want to insert new rows
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    // only when you allow updates
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::deleteRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::cancelRowUpdates() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::moveToInsertRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    // only when you allow inserts
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::moveToCurrentRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateNull(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateBoolean(sal_Int32, sal_Bool) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateByte(sal_Int32, sal_Int8) throw(SQLException, RuntimeException)
{
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateShort(sal_Int32, sal_Int16) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateInt(sal_Int32, sal_Int32) throw(SQLException, RuntimeException)
{
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateLong(sal_Int32, sal_Int64) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -----------------------------------------------------------------------
void SAL_CALL KabResultSet::updateFloat(sal_Int32, float) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateDouble(sal_Int32, double) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateString(sal_Int32, const ::rtl::OUString&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateBytes(sal_Int32, const Sequence< sal_Int8 >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateDate(sal_Int32, const Date&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateTime(sal_Int32, const Time&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateTimestamp(sal_Int32, const DateTime&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateBinaryStream(sal_Int32, const Reference< XInputStream >&, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateCharacterStream(sal_Int32, const Reference< XInputStream >&, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::refreshRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateObject(sal_Int32, const Any&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
void SAL_CALL KabResultSet::updateNumericObject(sal_Int32, const Any&, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);
}
// -------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL KabResultSet::getBookmark() throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

        sal_Int32 nAddressees = m_aKabAddressees.size();

        if (m_nRowPos != -1 && m_nRowPos != nAddressees)
        {
                QString aQtName = m_aKabAddressees[m_nRowPos].uid();
               ::rtl::OUString sUniqueIdentifier = ::rtl::OUString((const sal_Unicode *) aQtName.ucs2());
        return makeAny(sUniqueIdentifier);
    }
    return Any();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::moveToBookmark(const  Any& bookmark) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    ::rtl::OUString sBookmark = comphelper::getString(bookmark);
        sal_Int32 nAddressees = m_aKabAddressees.size();

    for (sal_Int32 nRow = 0; nRow < nAddressees; nRow++)
    {
                QString aQtName = m_aKabAddressees[nRow].uid();
               ::rtl::OUString sUniqueIdentifier = ::rtl::OUString((const sal_Unicode *) aQtName.ucs2());

        if (sUniqueIdentifier == sBookmark)
        {
            m_nRowPos = nRow;
            return sal_True;
        }
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::moveRelativeToBookmark(const  Any& bookmark, sal_Int32 rows) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRowSave = m_nRowPos;

    if (moveToBookmark(bookmark))
    {
        sal_Int32 nAddressees = m_aKabAddressees.size();

        m_nRowPos += rows;

        if (-1 < m_nRowPos && m_nRowPos < nAddressees)
            return sal_True;
    }

    m_nRowPos = nRowSave;
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSet::compareBookmarks(const  Any& firstItem, const  Any& secondItem) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    ::rtl::OUString sFirst = comphelper::getString(firstItem);
    ::rtl::OUString sSecond = comphelper::getString(secondItem);

    if (sFirst < sSecond)
        return CompareBookmark::LESS;
    if (sFirst > sSecond)
        return CompareBookmark::GREATER;
    return CompareBookmark::EQUAL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL KabResultSet::hasOrderedBookmarks() throw( SQLException,  RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL KabResultSet::hashBookmark(const  Any& bookmark) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    ::rtl::OUString sBookmark = comphelper::getString(bookmark);

    return sBookmark.hashCode();
}
// -------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL KabResultSet::deleteRows(const  Sequence<  Any >&) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabResultSet_BASE::rBHelper.bDisposed);

    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* KabResultSet::createArrayHelper() const
{
    Sequence< Property > aProps(6);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP1IMPL(CURSORNAME,          ::rtl::OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
    DECL_BOOL_PROP1IMPL(ISBOOKMARKABLE) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & KabResultSet::getInfoHelper()
{
    return *static_cast<KabResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool KabResultSet::convertFastPropertyValue(
            Any &,
            Any &,
            sal_Int32 nHandle,
            const Any& )
                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch (nHandle)
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
void KabResultSet::setFastPropertyValue_NoBroadcast(
            sal_Int32 nHandle,
            const Any& )
                 throw (Exception)
{
    switch (nHandle)
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
void KabResultSet::getFastPropertyValue(
            Any& _rValue,
            sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
            _rValue <<= (sal_Bool)sal_False;
            break;
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            ;
    }
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
