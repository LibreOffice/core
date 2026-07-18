/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <TConnection.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <resource/sharedresources.hxx>
#include <rtl/ref.hxx>
#include <strings.hrc>

using namespace connectivity::macab;
using namespace cppu;
using namespace css::uno;
using namespace cpo::uno;
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
      m_aMacabRecords(),
      m_bMacabRecordsNeedsDelete(false),
      m_nRowPos(-1),
      m_bWasNull(true),
      m_sTableName(MacabAddressBook::getDefaultTableName())
{
}

MacabResultSet::~MacabResultSet()
{
    if(m_aMacabRecords != nullptr && m_bMacabRecordsNeedsDelete)
        delete m_aMacabRecords;
}

void MacabResultSet::allMacabRecords()
{
    rtl::Reference<MacabConnection> pConnection = static_cast< MacabConnection *>(m_xStatement->getConnection().get());

    if(m_aMacabRecords != nullptr && m_bMacabRecordsNeedsDelete)
    {
        m_bMacabRecordsNeedsDelete = false;
        delete m_aMacabRecords;
    }
    m_aMacabRecords = pConnection->getAddressBook()->getMacabRecords(m_sTableName);
}

void MacabResultSet::someMacabRecords(const MacabCondition *pCondition)
{
    rtl::Reference<MacabConnection> pConnection = static_cast< MacabConnection *>(m_xStatement->getConnection().get());
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
    m_bMacabRecordsNeedsDelete = true;

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

Any MacabResultSet::queryInterface(const Type & rType)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if (!aRet.hasValue())
        aRet = MacabResultSet_BASE::queryInterface(rType);
    return aRet;
}

void MacabResultSet::acquire() noexcept
{
    MacabResultSet_BASE::acquire();
}

void MacabResultSet::release() noexcept
{
    MacabResultSet_BASE::release();
}

Sequence<  Type > MacabResultSet::getTypes()
{
    OTypeCollection aTypes(
        cppu::UnoType<css::beans::XMultiPropertySet>::get(),
        cppu::UnoType<css::beans::XFastPropertySet>::get(),
        cppu::UnoType<css::beans::XPropertySet>::get());

    return comphelper::concatSequences(aTypes.getTypes(), MacabResultSet_BASE::getTypes());
}

css::uno::Reference< css::beans::XPropertySetInfo > MacabResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

sal_Int32 MacabResultSet::findColumn(const OUString& columnName)
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

OUString MacabResultSet::getString(sal_Int32 columnIndex)
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

bool MacabResultSet::getBoolean(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getBoolean", nullptr);

    return false;
}

sal_Int8 MacabResultSet::getByte(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getByte", nullptr);

    return 0;
}

sal_Int16 MacabResultSet::getShort(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getShort", nullptr);

    return 0;
}

sal_Int32 MacabResultSet::getInt(sal_Int32 columnIndex)
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

sal_Int64 MacabResultSet::getLong(sal_Int32 columnIndex)
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

float MacabResultSet::getFloat(sal_Int32 columnIndex)
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

double MacabResultSet::getDouble(sal_Int32 columnIndex)
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

Sequence< sal_Int8 > MacabResultSet::getBytes(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getBytes", nullptr);

    return Sequence< sal_Int8 >();
}

Date MacabResultSet::getDate(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getDate", nullptr);

    return Date();
}

Time MacabResultSet::getTime(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getTime", nullptr);

    return css::util::Time();
}

DateTime MacabResultSet::getTimestamp(sal_Int32 columnIndex)
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

Reference< XInputStream > MacabResultSet::getBinaryStream(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getBinaryStream", nullptr);

    return nullptr;
}

Reference< XInputStream > MacabResultSet::getCharacterStream(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getCharacterStream", nullptr);

    return nullptr;
}

Any MacabResultSet::getObject(sal_Int32, const Reference< css::container::XNameAccess >&)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getObject", nullptr);

    return Any();
}

Reference< XRef > MacabResultSet::getRef(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getRef", nullptr);

    return nullptr;
}

Reference< XBlob > MacabResultSet::getBlob(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getBlob", nullptr);

    return nullptr;
}

Reference< XClob > MacabResultSet::getClob(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getClob", nullptr);

    return nullptr;
}

Reference< XArray > MacabResultSet::getArray(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    ::dbtools::throwFunctionNotSupportedSQLException("getArray", nullptr);

    return nullptr;
}

Reference< XResultSetMetaData > MacabResultSet::getMetaData()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    if (!m_xMetaData.is())
        m_xMetaData = new MacabResultSetMetaData(m_xStatement->getOwnConnection(), m_sTableName);

    Reference< XResultSetMetaData > xMetaData = m_xMetaData;
    return xMetaData;
}

bool MacabResultSet::isBeforeFirst()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    if (m_nRowPos == -1)
        return true;

    return false;
}

bool MacabResultSet::isAfterLast()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (m_nRowPos == nRecords)
        return true;

    return false;
}

bool MacabResultSet::isFirst()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    if (m_nRowPos == 0)
        return true;

    return false;
}

bool MacabResultSet::isLast()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (m_nRowPos == nRecords - 1)
        return true;

    return false;
}

void MacabResultSet::beforeFirst()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // move before the first row
    m_nRowPos = -1;
}

void MacabResultSet::afterLast()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // move after the last row
    sal_Int32 nRecords = m_aMacabRecords->size();
    m_nRowPos = nRecords;
}

void MacabResultSet::close()
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
    }
    dispose();
}

bool MacabResultSet::first()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (nRecords == 0)
        return false;

    m_nRowPos = 0;
    return true;
}

bool MacabResultSet::last()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();
    if (nRecords == 0)
        return false;

    m_nRowPos = nRecords - 1;
    return true;
}

sal_Int32 MacabResultSet::getRow()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return m_nRowPos;
}

bool MacabResultSet::absolute(sal_Int32 row)
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

bool MacabResultSet::relative(sal_Int32 row)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos + row);
}

bool MacabResultSet::next()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos + 1);
}

bool MacabResultSet::previous()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return absolute(m_nRowPos - 1);
}

Reference< XInterface > MacabResultSet::getStatement()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    Reference< XStatement > xStatement = m_xStatement;
    return xStatement;
}

bool MacabResultSet::rowDeleted()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return false;
}

bool MacabResultSet::rowInserted()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return false;
}

bool MacabResultSet::rowUpdated()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return false;
}

bool MacabResultSet::wasNull()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return m_bWasNull;
}

void MacabResultSet::cancel()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::clearWarnings()
{
}

Any MacabResultSet::getWarnings()
{
    return Any();
}

void MacabResultSet::insertRow()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // you only have to implement this if you want to insert new rows
}

void MacabResultSet::updateRow()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // only when you allow updates
}

void MacabResultSet::deleteRow()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::cancelRowUpdates()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::moveToInsertRow()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    // only when you allow inserts
}

void MacabResultSet::moveToCurrentRow()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateNull(sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateBoolean(sal_Int32, bool)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateByte(sal_Int32, sal_Int8)
{
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
}

void MacabResultSet::updateShort(sal_Int32, sal_Int16)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateInt(sal_Int32, sal_Int32)
{
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
}

void MacabResultSet::updateLong(sal_Int32, sal_Int64)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateFloat(sal_Int32, float)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateDouble(sal_Int32, double)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateString(sal_Int32, const OUString&)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateBytes(sal_Int32, const Sequence< sal_Int8 >&)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateDate(sal_Int32, const Date&)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateTime(sal_Int32, const css::util::Time&)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateTimestamp(sal_Int32, const DateTime&)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateBinaryStream(sal_Int32, const Reference< XInputStream >&, sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateCharacterStream(sal_Int32, const Reference< XInputStream >&, sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::refreshRow()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateObject(sal_Int32, const Any&)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

void MacabResultSet::updateNumericObject(sal_Int32, const Any&, sal_Int32)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);
}

// XRowLocate
Any MacabResultSet::getBookmark()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    sal_Int32 nRecords = m_aMacabRecords->size();

    if (m_nRowPos != -1 && m_nRowPos != nRecords)
    {
        macabfield *uidField = m_aMacabRecords->getField(m_nRowPos,u"UID");
        if(uidField != nullptr)
        {
            if(uidField->type == kABStringProperty)
            {
                return Any(CFStringToOUString( static_cast<CFStringRef>(uidField->value) ));
            }
        }
    }
    return Any();
}

bool MacabResultSet::moveToBookmark(const  Any& bookmark)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    OUString sBookmark = comphelper::getString(bookmark);
    sal_Int32 nRecords = m_aMacabRecords->size();

    for (sal_Int32 nRow = 0; nRow < nRecords; nRow++)
    {
        macabfield *uidField = m_aMacabRecords->getField(m_nRowPos,u"UID");
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

bool MacabResultSet::moveRelativeToBookmark(const  Any& bookmark, sal_Int32 rows)
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

sal_Int32 MacabResultSet::compareBookmarks(const  Any& firstItem, const  Any& secondItem)
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

bool MacabResultSet::hasOrderedBookmarks()
{
    return false;
}

sal_Int32 MacabResultSet::hashBookmark(const  Any& bookmark)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    OUString sBookmark = comphelper::getString(bookmark);

    return sBookmark.hashCode();
}

// XDeleteRows
Sequence< sal_Int32 > MacabResultSet::deleteRows(const  Sequence<  Any >&)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabResultSet_BASE::rBHelper.bDisposed);

    return Sequence< sal_Int32 >();
}

IPropertyArrayHelper* MacabResultSet::createArrayHelper() const
{
    return new OPropertyArrayHelper
    {
        {
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
                PROPERTY_ID_CURSORNAME,
                cppu::UnoType<OUString>::get(),
                PropertyAttribute::READONLY
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
                PROPERTY_ID_FETCHDIRECTION,
                cppu::UnoType<sal_Int32>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
                PROPERTY_ID_FETCHSIZE,
                cppu::UnoType<sal_Int32>::get(),
                0
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),
                PROPERTY_ID_ISBOOKMARKABLE,
                cppu::UnoType<bool>::get(),
                PropertyAttribute::READONLY
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
                PROPERTY_ID_RESULTSETCONCURRENCY,
                cppu::UnoType<sal_Int32>::get(),
                PropertyAttribute::READONLY
            },
            {
                ::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
                PROPERTY_ID_RESULTSETTYPE,
                cppu::UnoType<sal_Int32>::get(),
                PropertyAttribute::READONLY
            }
        }
    };
}

IPropertyArrayHelper & MacabResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

bool MacabResultSet::convertFastPropertyValue(
            Any &,
            Any &,
            sal_Int32 nHandle,
            const Any& )
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
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception("cannot set prop " + OUString::number(nHandle), nullptr);
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
