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


#include "MacabResultSet.hxx"
#include "MacabAddressBook.hxx"
#include "MacabRecords.hxx"
#include "macabutilities.hxx"
#include "MacabResultSetMetaData.hxx"
#include "MacabConnection.hxx"
#include "macabcondition.hxx"
#include "macaborder.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include "TConnection.hxx"
#include <connectivity/dbexception.hxx>
#include "resource/sharedresources.hxx"
#include "resource/macab_res.hrc"

using namespace connectivity::macab;
using namespace cppu;
using namespace css::uno;
using namespace css::lang;
using namespace css::beans;
using namespace css::sdbc;
using namespace css::sdbcx;
using namespace css::io;
using namespace css::util;

IMPLEMENT_SERVICE_INFO(MacabResultSet, "com.sun.star.sdbc.drivers.MacabResultSet", "com.sun.star.sdbc.ResultSet");

MacabResultSet::MacabResultSet(MacabCommonStatement* pStmt)
    : MacabResultSet_BASE(m_aMutex),
      OPropertySetHelper(MacabResultSet_BASE::rBHelper),
      m_xStatement(pStmt),
      m_xMetaData(nullptr),
      m_aMacabRecords(),
      m_nRowPos(-1),
      m_bWasNull(true)
{
    m_sTableName = MacabAddressBook::getDefaultTableName();
}

MacabResultSet::~MacabResultSet()
{
}

void MacabResultSet::allMacabRecords()
{
    MacabConnection* pConnection = static_cast< MacabConnection *>(m_xStatement->getConnection().get());

    m_aMacabRecords = pConnection->getAddressBook()->getMacabRecords(m_sTableName);
}

void MacabResultSet::someMacabRecords(const MacabCondition *pCondition)
{
    MacabConnection* pConnection = static_cast< MacabConnection *>(m_xStatement->getConnection().get());
    MacabRecords* allRecords;

    allRecords = pConnection->getAddressBook()->getMacabRecords(m_sTableName);

    // Bad table!! Throw exception?
    if(allRecords == nullptr)
        return;

    if(m_aMacabRecords != nullptr && m_aMacabRecords != allRecords)
        delete m_aMacabRecords;

    // The copy constructor copies everything but records (including the
    // maximum allocated size, which means that we'll never have to resize)
    m_aMacabRecords = new MacabRecords(allRecords);

    if(pCondition->isAlwaysFalse())
    {
        return;
    }

    MacabRecords::iterator iterator;

    for (iterator = allRecords->begin();
         iterator != allRecords->end();
         ++iterator)
    {
        if (pCondition->eval(*iterator))
            m_aMacabRecords->insertRecord(*iterator);
    }
}

void MacabResultSet::sortMacabRecords(const MacabOrder *pOrder)
{
    // I do this with ints rather than an iterator because the ids will
    // be changing when we change the order and ints are easier to deal
    // with (for me).
    sal_Int32 i, j, size, smallest;
    size = m_aMacabRecords->size();

    for(i = 0; i < size; i++)
    {
        smallest = i;
        for( j = i + 1; j < size; j++)
        {
            // if smallest > j
            if(pOrder->compare(m_aMacabRecords->getRecord(smallest),
                        m_aMacabRecords->getRecord(j) ) > 0)
            {
                smallest = j;
            }

        }

        if(smallest != i)
        {
            m_aMacabRecords->swap(i,smallest);
        }
    }

}

void MacabResultSet::setTableName(OUString const & _sTableName)
{
    m_sTableName = _sTableName;
}

void MacabResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

m_xStatement.clear();
m_xMetaData.clear();
}

Any SAL_CALL MacabResultSet::queryInterface(const Type & rType) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if (!aRet.hasValue())
        aRet = MacabResultSet_BASE::queryInterface(rType);
    return aRet;
}

void SAL_CALL MacabResultSet::acquire() throw()
{
    MacabResultSet_BASE::acquire();
}

void SAL_CALL MacabResultSet::release() throw()
{
    MacabResultSet_BASE::release();
}

Sequence<  Type > SAL_CALL MacabResultSet::getTypes() throw(RuntimeException)
{
    OTypeCollection aTypes(
        cppu::UnoType<css::beans::XMultiPropertySet>::get(),
        cppu::UnoType<css::beans::XFastPropertySet>::get(),
        cppu::UnoType<css::beans::XPropertySet>::get());

    return comphelper::concatSequences(aTypes.getTypes(), MacabResultSet_BASE::getTypes());
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL MacabResultSet::getPropertySetInfo(  ) throw(css::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

sal_Int32 SAL_CALL MacabResultSet::findColumn(const OUString& columnName) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // find the first column with the name columnName
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();

    for (sal_Int32 i = 1; i <= nLen; ++i)
    {
        if (xMeta->isCaseSensitive(i) ?
            columnName == xMeta->getColumnName(i) :
            columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
                return i;
    }

    ::dbtools::throwInvalidColumnException( columnName, *this );
    assert(false);
    return 0; // Never reached
}

OUString SAL_CALL MacabResultSet::getString(sal_Int32 columnIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    OUString aRet;
    sal_Int32 nRecords = m_aMacabRecords->size();
    m_bWasNull = true;

    if (m_nRowPos != -1 && m_nRowPos != nRecords && m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(columnIndex);
        macabfield *aField = m_aMacabRecords->getField(m_nRowPos,nFieldNumber);
        if(aField != nullptr)
        {
            if(aField->type == kABStringProperty)
            {
                aRet = CFStringToOUString(static_cast<CFStringRef>(aField->value));
                m_bWasNull = false;
            }
        }
    }

// Trigger an exception if m_bWasNull is true?
    return aRet;
}

sal_Bool SAL_CALL MacabResultSet::getBoolean(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedSQLException("getBoolean", nullptr);

    return false;
}

sal_Int8 SAL_CALL MacabResultSet::getByte(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedSQLException("getByte", nullptr);

    sal_Int8 nRet = 0;
    return nRet;
}

sal_Int16 SAL_CALL MacabResultSet::getShort(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedSQLException("getShort", nullptr);

    sal_Int16 nRet = 0;
    return nRet;
}

sal_Int32 SAL_CALL MacabResultSet::getInt(sal_Int32 columnIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRet = 0;
    sal_Int32 nRecords = m_aMacabRecords->size();
    m_bWasNull = true;

    if (m_nRowPos != -1 && m_nRowPos != nRecords && m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(columnIndex);
        macabfield *aField = m_aMacabRecords->getField(m_nRowPos,nFieldNumber);
        if(aField != nullptr)
        {
            if(aField->type == kABIntegerProperty)
            {
                CFNumberType numberType = CFNumberGetType( static_cast<CFNumberRef>(aField->value) );
                // m_bWasNull now becomes whether getting the value was successful
                // Should we check for the wrong type here, e.g., a float or a 64 bit int?
                m_bWasNull = !CFNumberGetValue(static_cast<CFNumberRef>(aField->value), numberType, &nRet);
            }
        }
    }

// Trigger an exception if m_bWasNull is true?
    return nRet;
}

sal_Int64 SAL_CALL MacabResultSet::getLong(sal_Int32 columnIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int64 nRet = 0;
    sal_Int32 nRecords = m_aMacabRecords->size();
    m_bWasNull = true;

    if (m_nRowPos != -1 && m_nRowPos != nRecords && m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(columnIndex);
        macabfield *aField = m_aMacabRecords->getField(m_nRowPos,nFieldNumber);
        if(aField != nullptr)
        {
            if(aField->type == kABIntegerProperty)
            {
                CFNumberType numberType = CFNumberGetType( static_cast<CFNumberRef>(aField->value) );
                // m_bWasNull now becomes whether getting the value was successful
                // Should we check for the wrong type here, e.g., a float or a 32 bit int?
                m_bWasNull = !CFNumberGetValue(static_cast<CFNumberRef>(aField->value), numberType, &nRet);
            }
        }
    }

// Trigger an exception if m_bWasNull is true?
    return nRet;
}

float SAL_CALL MacabResultSet::getFloat(sal_Int32 columnIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    float nVal = 0;
    sal_Int32 nRecords = m_aMacabRecords->size();
    m_bWasNull = true;

    if (m_nRowPos != -1 && m_nRowPos != nRecords && m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(columnIndex);
        macabfield *aField = m_aMacabRecords->getField(m_nRowPos,nFieldNumber);
        if(aField != nullptr)
        {
            if(aField->type == kABRealProperty)
            {
                CFNumberType numberType = CFNumberGetType( static_cast<CFNumberRef>(aField->value) );
                // m_bWasNull now becomes whether getting the value was successful
                // Should we check for the wrong type here, e.g., an int or a double?
                m_bWasNull = !CFNumberGetValue(static_cast<CFNumberRef>(aField->value), numberType, &nVal);
            }
        }
    }

// Trigger an exception if m_bWasNull is true?
    return nVal;
}

double SAL_CALL MacabResultSet::getDouble(sal_Int32 columnIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    double nVal = 0;
    sal_Int32 nRecords = m_aMacabRecords->size();
    m_bWasNull = true;

    if (m_nRowPos != -1 && m_nRowPos != nRecords && m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(columnIndex);
        macabfield *aField = m_aMacabRecords->getField(m_nRowPos,nFieldNumber);
        if(aField != nullptr)
        {
            if(aField->type == kABRealProperty)
            {
                CFNumberType numberType = CFNumberGetType( static_cast<CFNumberRef>(aField->value) );
                // m_bWasNull now becomes whether getting the value was successful
                // Should we check for the wrong type here, e.g., an int or a float?
                m_bWasNull = !CFNumberGetValue(static_cast<CFNumberRef>(aField->value), numberType, &nVal);
            }
        }
    }

// Trigger an exception if m_bWasNull is true?
    return nVal;
}

Sequence< sal_Int8 > SAL_CALL MacabResultSet::getBytes(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getBytes", nullptr);

    return Sequence< sal_Int8 >();
}

Date SAL_CALL MacabResultSet::getDate(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getDate", nullptr);

    Date aRet;
    return aRet;
}

Time SAL_CALL MacabResultSet::getTime(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getTime", nullptr);

    css::util::Time nRet;
    return nRet;
}

DateTime SAL_CALL MacabResultSet::getTimestamp(sal_Int32 columnIndex) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    DateTime nRet;
    sal_Int32 nRecords = m_aMacabRecords->size();
    m_bWasNull = true;

    if (m_nRowPos != -1 && m_nRowPos != nRecords && m_xMetaData.is())
    {
        sal_Int32 nFieldNumber = m_xMetaData->fieldAtColumn(columnIndex);
        macabfield *aField = m_aMacabRecords->getField(m_nRowPos,nFieldNumber);
        if(aField != nullptr)
        {
            if(aField->type == kABDateProperty)
            {
                nRet = CFDateToDateTime(static_cast<CFDateRef>(aField->value));
                m_bWasNull = false;
            }
        }
    }

// Trigger an exception if m_bWasNull is true?
    return nRet;
}

Reference< XInputStream > SAL_CALL MacabResultSet::getBinaryStream(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getBinaryStream", nullptr);

    return nullptr;
}

Reference< XInputStream > SAL_CALL MacabResultSet::getCharacterStream(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getCharacterStream", nullptr);

    return nullptr;
}

Any SAL_CALL MacabResultSet::getObject(sal_Int32, const Reference< css::container::XNameAccess >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getObject", nullptr);

    return Any();
}

Reference< XRef > SAL_CALL MacabResultSet::getRef(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getRef", nullptr);

    return nullptr;
}

Reference< XBlob > SAL_CALL MacabResultSet::getBlob(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getBlob", nullptr);

    return nullptr;
}

Reference< XClob > SAL_CALL MacabResultSet::getClob(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedSQLException("getClob", nullptr);

    return nullptr;
}

Reference< XArray > SAL_CALL MacabResultSet::getArray(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

::dbtools::throwFunctionNotSupportedSQLException("getArray", nullptr);

    return nullptr;
}

Reference< XResultSetMetaData > SAL_CALL MacabResultSet::getMetaData() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    if (!m_xMetaData.is())
        m_xMetaData = new MacabResultSetMetaData(m_xStatement->getOwnConnection(), m_sTableName);

    Reference< XResultSetMetaData > xMetaData = m_xMetaData.get();
    return xMetaData;
}

sal_Bool SAL_CALL MacabResultSet::isBeforeFirst() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    if (m_nRowPos == -1)
        return true;

    return false;
}

sal_Bool SAL_CALL MacabResultSet::isAfterLast() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (m_nRowPos == nRecords)
        return true;

    return false;
}

sal_Bool SAL_CALL MacabResultSet::isFirst() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    if (m_nRowPos == 0)
        return true;

    return false;
}

sal_Bool SAL_CALL MacabResultSet::isLast() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (m_nRowPos == nRecords - 1)
        return true;

    return false;
}

void SAL_CALL MacabResultSet::beforeFirst() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // move before the first row
    m_nRowPos = -1;
}

void SAL_CALL MacabResultSet::afterLast() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // move after the last row
    sal_Int32 nRecords = m_aMacabRecords->size();
    m_nRowPos = nRecords;
}

void SAL_CALL MacabResultSet::close() throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
    }
    dispose();
}

sal_Bool SAL_CALL MacabResultSet::first() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (nRecords == 0)
        return false;

    m_nRowPos = 0;
    return true;
}

sal_Bool SAL_CALL MacabResultSet::last() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (nRecords == 0)
        return false;

    m_nRowPos = nRecords - 1;
    return true;
}

sal_Int32 SAL_CALL MacabResultSet::getRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return m_nRowPos;
}

sal_Bool SAL_CALL MacabResultSet::absolute(sal_Int32 row) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (row <= -1 ||
        row >= nRecords)
        return false;

    m_nRowPos = row;
    return true;
}

sal_Bool SAL_CALL MacabResultSet::relative(sal_Int32 row) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos + row);
}

sal_Bool SAL_CALL MacabResultSet::next() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos + 1);
}

sal_Bool SAL_CALL MacabResultSet::previous() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos - 1);
}

Reference< XInterface > SAL_CALL MacabResultSet::getStatement() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    Reference< XStatement > xStatement = m_xStatement.get();
    return xStatement;
}

sal_Bool SAL_CALL MacabResultSet::rowDeleted() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL MacabResultSet::rowInserted() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL MacabResultSet::rowUpdated() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return false;
}

sal_Bool SAL_CALL MacabResultSet::wasNull() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}

void SAL_CALL MacabResultSet::cancel() throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::clearWarnings() throw(SQLException, RuntimeException)
{
}

Any SAL_CALL MacabResultSet::getWarnings() throw(SQLException, RuntimeException)
{
    return Any();
}

void SAL_CALL MacabResultSet::insertRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // you only have to implement this if you want to insert new rows
}

void SAL_CALL MacabResultSet::updateRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // only when you allow updates
}

void SAL_CALL MacabResultSet::deleteRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::cancelRowUpdates() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::moveToInsertRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // only when you allow inserts
}

void SAL_CALL MacabResultSet::moveToCurrentRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateNull(sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateBoolean(sal_Int32, sal_Bool) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateByte(sal_Int32, sal_Int8) throw(SQLException, RuntimeException)
{
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
}

void SAL_CALL MacabResultSet::updateShort(sal_Int32, sal_Int16) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateInt(sal_Int32, sal_Int32) throw(SQLException, RuntimeException)
{
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
}

void SAL_CALL MacabResultSet::updateLong(sal_Int32, sal_Int64) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateFloat(sal_Int32, float) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateDouble(sal_Int32, double) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateString(sal_Int32, const OUString&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateBytes(sal_Int32, const Sequence< sal_Int8 >&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateDate(sal_Int32, const Date&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateTime(sal_Int32, const css::util::Time&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateTimestamp(sal_Int32, const DateTime&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateBinaryStream(sal_Int32, const Reference< XInputStream >&, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateCharacterStream(sal_Int32, const Reference< XInputStream >&, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::refreshRow() throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateObject(sal_Int32, const Any&) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void SAL_CALL MacabResultSet::updateNumericObject(sal_Int32, const Any&, sal_Int32) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

// XRowLocate
Any SAL_CALL MacabResultSet::getBookmark() throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();

    if (m_nRowPos != -1 && m_nRowPos != nRecords)
    {
        macabfield *uidField = m_aMacabRecords->getField(m_nRowPos,OUString("UID"));
        if(uidField != nullptr)
        {
            if(uidField->type == kABStringProperty)
            {
                return makeAny(CFStringToOUString( static_cast<CFStringRef>(uidField->value) ));
            }
        }
    }
    return Any();
}

sal_Bool SAL_CALL MacabResultSet::moveToBookmark(const  Any& bookmark) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    OUString sBookmark = comphelper::getString(bookmark);
        sal_Int32 nRecords = m_aMacabRecords->size();

    for (sal_Int32 nRow = 0; nRow < nRecords; nRow++)
    {
        macabfield *uidField = m_aMacabRecords->getField(m_nRowPos,OUString("UID"));
        if(uidField != nullptr)
        {
            if(uidField->type == kABStringProperty)
            {
                OUString sUniqueIdentifier = CFStringToOUString( static_cast<CFStringRef>(uidField->value) );
                if (sUniqueIdentifier == sBookmark)
                {
                    m_nRowPos = nRow;
                    return true;
                }
            }
        }
    }
    return false;
}

sal_Bool SAL_CALL MacabResultSet::moveRelativeToBookmark(const  Any& bookmark, sal_Int32 rows) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRowSave = m_nRowPos;

    if (moveToBookmark(bookmark))
    {
        sal_Int32 nRecords = m_aMacabRecords->size();

        m_nRowPos += rows;

        if (-1 < m_nRowPos && m_nRowPos < nRecords)
            return true;
    }

    m_nRowPos = nRowSave;
    return false;
}

sal_Int32 SAL_CALL MacabResultSet::compareBookmarks(const  Any& firstItem, const  Any& secondItem) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    OUString sFirst = comphelper::getString(firstItem);
    OUString sSecond = comphelper::getString(secondItem);

    if (sFirst < sSecond)
        return CompareBookmark::LESS;
    if (sFirst > sSecond)
        return CompareBookmark::GREATER;
    return CompareBookmark::EQUAL;
}

sal_Bool SAL_CALL MacabResultSet::hasOrderedBookmarks() throw( SQLException,  RuntimeException)
{
    return false;
}

sal_Int32 SAL_CALL MacabResultSet::hashBookmark(const  Any& bookmark) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    OUString sBookmark = comphelper::getString(bookmark);

    return sBookmark.hashCode();
}

// XDeleteRows
Sequence< sal_Int32 > SAL_CALL MacabResultSet::deleteRows(const  Sequence<  Any >&) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return Sequence< sal_Int32 >();
}

IPropertyArrayHelper* MacabResultSet::createArrayHelper() const
{
    Sequence< Property > aProps(6);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
        PROPERTY_ID_CURSORNAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
        PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
        PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),
        PROPERTY_ID_ISBOOKMARKABLE, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}

IPropertyArrayHelper & MacabResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool MacabResultSet::convertFastPropertyValue(
            Any &,
            Any &,
            sal_Int32 nHandle,
            const Any& )
                throw (css::lang::IllegalArgumentException)
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:
            ;
    }
    return false;
}

void MacabResultSet::setFastPropertyValue_NoBroadcast(
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

void MacabResultSet::getFastPropertyValue(
            Any& _rValue,
            sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
            _rValue <<= false;
            break;
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            ;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
