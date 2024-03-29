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

#include "PrivateRow.hxx"

using namespace dbaccess;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star;

sal_Bool SAL_CALL OPrivateRow::wasNull(  )
    {
        return m_aRow[m_nPos].isNull();
    }
    OUString SAL_CALL OPrivateRow::getString( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getString();
    }
    sal_Bool SAL_CALL OPrivateRow::getBoolean( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getBool();
    }
    ::sal_Int8 SAL_CALL OPrivateRow::getByte( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getInt8();
    }
    ::sal_Int16 SAL_CALL OPrivateRow::getShort( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getInt16();
    }
    ::sal_Int32 SAL_CALL OPrivateRow::getInt( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getInt32();
    }
    ::sal_Int64 SAL_CALL OPrivateRow::getLong( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getLong();
    }
    float SAL_CALL OPrivateRow::getFloat( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getFloat();
    }
    double SAL_CALL OPrivateRow::getDouble( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getDouble();
    }
    Sequence< ::sal_Int8 > SAL_CALL OPrivateRow::getBytes( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getSequence();
    }
    css::util::Date SAL_CALL OPrivateRow::getDate( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getDate();
    }
    css::util::Time SAL_CALL OPrivateRow::getTime( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getTime();
    }
    css::util::DateTime SAL_CALL OPrivateRow::getTimestamp( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].getDateTime();
    }
    Reference< css::io::XInputStream > SAL_CALL OPrivateRow::getBinaryStream( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return Reference< css::io::XInputStream >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Reference< css::io::XInputStream > SAL_CALL OPrivateRow::getCharacterStream( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return Reference< css::io::XInputStream >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Any SAL_CALL OPrivateRow::getObject( ::sal_Int32 columnIndex, const Reference< css::container::XNameAccess >&  )
    {
        m_nPos = columnIndex;
        return m_aRow[m_nPos].makeAny();
    }
    Reference< XRef > SAL_CALL OPrivateRow::getRef( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return Reference< XRef >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Reference< XBlob > SAL_CALL OPrivateRow::getBlob( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return Reference< XBlob >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Reference< XClob > SAL_CALL OPrivateRow::getClob( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return Reference< XClob >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }
    Reference< XArray > SAL_CALL OPrivateRow::getArray( ::sal_Int32 columnIndex )
    {
        m_nPos = columnIndex;
        return Reference< XArray >(m_aRow[m_nPos].makeAny(),UNO_QUERY);
    }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
