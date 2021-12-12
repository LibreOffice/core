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

#include "NResultSetMetaData.hxx"
#include "NDatabaseMetaData.hxx"
#include <connectivity/dbexception.hxx>
#include <strings.hrc>

using namespace connectivity::evoab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;

OEvoabResultSetMetaData::OEvoabResultSetMetaData(const OUString& _aTableName)
    : m_aTableName(_aTableName)
{

}

OEvoabResultSetMetaData::~OEvoabResultSetMetaData()
{
}

void OEvoabResultSetMetaData::setEvoabFields(const ::rtl::Reference<connectivity::OSQLColumns> &xColumns)
{
        static constexpr OUStringLiteral aName = u"Name";

        for (const auto& rxColumn : *xColumns)
        {
                OUString aFieldName;

                rxColumn->getPropertyValue(aName) >>= aFieldName;
                guint nFieldNumber = findEvoabField(aFieldName);
                if (nFieldNumber == guint(-1))
                {
                    connectivity::SharedResources aResource;
                    const OUString sError( aResource.getResourceStringWithSubstitution(
                            STR_INVALID_COLUMNNAME,
                            "$columnname$", aFieldName
                         ) );
                    ::dbtools::throwGenericSQLException( sError, *this );
                }
                m_aEvoabFields.push_back(nFieldNumber);
        }
}


sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnDisplaySize( sal_Int32 /*nColumnNum*/ )
{
    return 50;
}

sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnType( sal_Int32 nColumnNum )
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldType (nField);
}

sal_Int32 SAL_CALL OEvoabResultSetMetaData::getColumnCount(  )
{
    return m_aEvoabFields.size();
}

sal_Bool SAL_CALL OEvoabResultSetMetaData::isCaseSensitive( sal_Int32 /*nColumnNum*/ )
{
    return true;
}

OUString SAL_CALL OEvoabResultSetMetaData::getSchemaName( sal_Int32 /*nColumnNum*/ )
{
    return OUString();
}

OUString SAL_CALL OEvoabResultSetMetaData::getColumnName( sal_Int32 nColumnNum )
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldName( nField );
}

OUString SAL_CALL OEvoabResultSetMetaData::getColumnTypeName( sal_Int32 nColumnNum )
{
    sal_uInt32 nField = m_aEvoabFields[nColumnNum - 1];
    return evoab::getFieldTypeName( nField );
}

OUString SAL_CALL OEvoabResultSetMetaData::getColumnLabel( sal_Int32 nColumnNum )
{
    return getColumnName(nColumnNum);
}

OUString SAL_CALL OEvoabResultSetMetaData::getColumnServiceName( sal_Int32 /*nColumnNum*/ )
{
    return OUString();
}

OUString SAL_CALL OEvoabResultSetMetaData::getTableName( sal_Int32 /*nColumnNum*/ )
{
    return m_aTableName;//OUString("TABLE");
}

OUString SAL_CALL OEvoabResultSetMetaData::getCatalogName( sal_Int32 /*nColumnNum*/ )
{
    return OUString();
}


sal_Bool SAL_CALL OEvoabResultSetMetaData::isCurrency( sal_Int32 /*nColumnNum*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabResultSetMetaData::isAutoIncrement( sal_Int32 /*nColumnNum*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabResultSetMetaData::isSigned( sal_Int32 /*nColumnNum*/ )
{
    return false;
}

sal_Int32 SAL_CALL OEvoabResultSetMetaData::getPrecision( sal_Int32 /*nColumnNum*/ )
{
    return 0;
}

sal_Int32 SAL_CALL OEvoabResultSetMetaData::getScale( sal_Int32 /*nColumnNum*/ )
{
    return 0;
}

sal_Int32 SAL_CALL OEvoabResultSetMetaData::isNullable( sal_Int32 /*nColumnNum*/ )
{
    return 0;
}

sal_Bool SAL_CALL OEvoabResultSetMetaData::isSearchable( sal_Int32 /*nColumnNum*/ )
{
    return true;
}

sal_Bool SAL_CALL OEvoabResultSetMetaData::isReadOnly( sal_Int32 /*nColumnNum*/ )
{
    return true;
}

sal_Bool SAL_CALL OEvoabResultSetMetaData::isDefinitelyWritable( sal_Int32 /*nColumnNum*/ )
{
    return false;
}

sal_Bool SAL_CALL OEvoabResultSetMetaData::isWritable( sal_Int32 /*nColumnNum*/ )
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
