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

#include "sdbdatacolumn.hxx"

//..............................................................................
namespace svxform
{
//..............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::container;

    //==========================================================================
    //= DataColumn - a class wrapping an object implementing a sdb::DataColumn service
    //==========================================================================
    DataColumn::DataColumn(const Reference< ::com::sun::star::beans::XPropertySet>& _rxIFace)
    {
        m_xPropertySet = _rxIFace;
        m_xColumn = Reference< ::com::sun::star::sdb::XColumn>(_rxIFace, UNO_QUERY);
        m_xColumnUpdate = Reference< ::com::sun::star::sdb::XColumnUpdate>(_rxIFace, UNO_QUERY);

        if (!m_xPropertySet.is() || !m_xColumn.is())
        {
            m_xPropertySet = NULL;
            m_xColumn = NULL;
            m_xColumnUpdate = NULL;
        }
    }

    // Reference< XPropertySet>
    Reference< XPropertySetInfo> DataColumn::getPropertySetInfo() const throw( RuntimeException )
    {
        return m_xPropertySet->getPropertySetInfo();
    }

    void DataColumn::setPropertyValue(const OUString& aPropertyName, const Any& aValue) throw( UnknownPropertyException,  PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
    {
        m_xPropertySet->setPropertyValue(aPropertyName, aValue);
    }

    Any DataColumn::getPropertyValue(const OUString& PropertyName) const throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
    {
        return m_xPropertySet->getPropertyValue(PropertyName);
    }

    void DataColumn::addPropertyChangeListener(const OUString& aPropertyName, const Reference< XPropertyChangeListener>& xListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
    {
        m_xPropertySet->addPropertyChangeListener(aPropertyName, xListener);
    }

    void DataColumn::removePropertyChangeListener(const OUString& aPropertyName, const Reference< XPropertyChangeListener>& aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
    {
        m_xPropertySet->removePropertyChangeListener(aPropertyName, aListener);
    }

    void DataColumn::addVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener>& aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
    {
        m_xPropertySet->addVetoableChangeListener(PropertyName, aListener);
    }

    void DataColumn::removeVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener>& aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
    {
        m_xPropertySet->removeVetoableChangeListener(PropertyName, aListener);
    }

    // XColumn
    sal_Bool DataColumn::wasNull() throw( SQLException, RuntimeException )
    {
        return m_xColumn->wasNull();
    }

    OUString DataColumn::getString() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getString();
    }

    sal_Bool DataColumn::getBoolean() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getBoolean();
    }

    sal_Int8 DataColumn::getByte() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getByte();
    }

    sal_Int16 DataColumn::getShort() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getShort();
    }

    sal_Int32 DataColumn::getInt() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getInt();
    }

    sal_Int64 DataColumn::getLong() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getLong();
    }

    float DataColumn::getFloat() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getFloat();
    }

    double DataColumn::getDouble() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getDouble();
    }

    Sequence< sal_Int8 > DataColumn::getBytes() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getBytes();
    }

    com::sun::star::util::Date DataColumn::getDate() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getDate();
    }

    com::sun::star::util::Time DataColumn::getTime() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getTime();
    }

    com::sun::star::util::DateTime DataColumn::getTimestamp() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getTimestamp();
    }

    Reference< XInputStream> DataColumn::getBinaryStream() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getBinaryStream();
    }

    Reference< XInputStream> DataColumn::getCharacterStream() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getCharacterStream();
    }

    Any DataColumn::getObject(const Reference< XNameAccess>& typeMap) throw( SQLException, RuntimeException )
    {
        return m_xColumn->getObject(typeMap);
    }

    Reference< XRef> DataColumn::getRef() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getRef();
    }

    Reference< XBlob> DataColumn::getBlob() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getBlob();
    }

    Reference< XClob> DataColumn::getClob() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getClob();
    }

    Reference< XArray> DataColumn::getArray() throw( SQLException, RuntimeException )
    {
        return m_xColumn->getArray();
    }

    // XColumnUpdate
    void DataColumn::updateNull() throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateNull();
    }

    void DataColumn::updateBoolean(sal_Bool x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateBoolean(x);
    }

    void DataColumn::updateByte(sal_Int8 x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateByte(x);
    }

    void DataColumn::updateShort(sal_Int16 x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateShort(x);
    }

    void DataColumn::updateInt(sal_Int32 x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateInt(x);
    }

    void DataColumn::updateLong(sal_Int64 x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateLong(x);
    }

    void DataColumn::updateFloat(float x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateFloat(x);
    }

    void DataColumn::updateDouble(double x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateDouble(x);
    }

    void DataColumn::updateString(const OUString& x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateString(x);
    }

    void DataColumn::updateBytes(const Sequence< sal_Int8 >& x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateBytes(x);
    }

    void DataColumn::updateDate(const com::sun::star::util::Date& x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateDate(x);
    }

    void DataColumn::updateTime(const com::sun::star::util::Time& x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateTime(x);
    }

    void DataColumn::updateTimestamp(const com::sun::star::util::DateTime& x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateTimestamp(x);
    }

    void DataColumn::updateBinaryStream(const Reference< XInputStream>& x, sal_Int32 length) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateBinaryStream(x, length);
    }

    void DataColumn::updateCharacterStream(const Reference< XInputStream>& x, sal_Int32 length) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateCharacterStream(x, length);
    }

    void DataColumn::updateObject(const Any& x) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateObject(x);
    }

    void DataColumn::updateNumericObject(const Any& x, sal_Int32 scale) throw( SQLException, RuntimeException )
    {
        m_xColumnUpdate->updateNumericObject(x, scale);
    }

    //..............................................................................
}   // namespace svxform
//..............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
