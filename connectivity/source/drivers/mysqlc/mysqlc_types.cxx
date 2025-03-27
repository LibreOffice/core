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

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include "mysqlc_types.hxx"

using namespace com::sun::star::sdbc;

TypeInfoDef const mysqlc_types[] = {

    // ------------- MySQL-Type: BIT. SDBC-Type: Bit -------------
    {
        "BIT", // Typename
        css::sdbc::DataType::BIT, // sdbc-type
        1, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "BIT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ------------ MySQL-Type: BOOL. SDBC-Type: Bit -------------
    {
        "BOOL", // Typename
        css::sdbc::DataType::BIT, // sdbc-type
        1, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "BOOL", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // --------- MySQL-Type: TINYINT SDBC-Type: TINYINT ----------
    {
        "TINYINT", // Typename
        css::sdbc::DataType::TINYINT, // sdbc-type
        3, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        true, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "TINYINT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: BIGINT SDBC-Type: BIGINT ----------
    {
        "BIGINT", // Typename
        css::sdbc::DataType::BIGINT, // sdbc-type
        19, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        true, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "BIGINT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: LONG VARBINARY SDBC-Type: LONGVARBINARY ----------
    {
        "LONG VARBINARY", // Typename
        css::sdbc::DataType::LONGVARBINARY, // sdbc-type
        16777215, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "LONG VARBINARY", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: MEDIUMBLOB SDBC-Type: LONGVARBINARY ----------
    {
        "MEDIUMBLOB", // Typename
        css::sdbc::DataType::LONGVARBINARY, // sdbc-type
        16777215, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "MEDIUMBLOB", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: LONGBLOB SDBC-Type: LONGVARBINARY ----------
    {
        "LONGBLOB", // Typename
        css::sdbc::DataType::LONGVARBINARY, // sdbc-type
        -1, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "LONGBLOB", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: BLOB SDBC-Type: LONGVARBINARY ----------
    {
        "BLOB", // Typename
        css::sdbc::DataType::LONGVARBINARY, // sdbc-type
        0xFFFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "BLOB", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: TINYBLOB SDBC-Type: LONGVARBINARY ----------
    {
        "TINYBLOB", // Typename
        css::sdbc::DataType::LONGVARBINARY, // sdbc-type
        0xFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "TINYBLOB", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: VARBINARY SDBC-Type: VARBINARY ----------
    {
        "VARBINARY", // Typename
        css::sdbc::DataType::VARBINARY, // sdbc-type
        0xFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "(M)", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "VARBINARY", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: BINARY SDBC-Type: BINARY ----------
    {
        "BINARY", // Typename
        css::sdbc::DataType::BINARY, // sdbc-type
        0xFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "(M)", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        true, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "VARBINARY", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: LONG VARCHAR SDBC-Type: LONG VARCHAR ----------
    {
        "LONG VARCHAR", // Typename
        css::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFFFFFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "LONG VARCHAR", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: MEDIUMTEXT SDBC-Type: LONG VARCHAR ----------
    {
        "MEDIUMTEXT", // Typename
        css::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFFFFFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "MEDIUMTEXT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: LONGTEXT SDBC-Type: LONG VARCHAR ----------
    {
        "LONGTEXT", // Typename
        css::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFFFFFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "LONGTEXT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: TEXT SDBC-Type: LONG VARCHAR ----------
    {
        "TEXT", // Typename
        css::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFFFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "TEXT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: TINYTEXT SDBC-Type: LONG VARCHAR ----------
    {
        "TINYTEXT", // Typename
        css::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "TINYTEXT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: CHAR SDBC-Type: CHAR ----------
    {
        "CHAR", // Typename
        css::sdbc::DataType::CHAR, // sdbc-type
        0xFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "(M)", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "NUMERIC", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: DECIMAL SDBC-Type: DECIMAL ----------
    {
        "DECIMAL", // Typename
        css::sdbc::DataType::DECIMAL, // sdbc-type
        17, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M[,D])] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "DECIMAL", // local type name
        -308, // minimum scale
        308 // maximum scale
    },

    // ----------- MySQL-Type: NUMERIC SDBC-Type: NUMERIC ----------
    {
        "NUMERIC", // Typename
        css::sdbc::DataType::NUMERIC, // sdbc-type
        17, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M[,D])] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "NUMERIC", // local type name
        -308, // minimum scale
        308 // maximum scale
    },

    // ----------- MySQL-Type: INTEGER SDBC-Type: INTEGER ----------
    {
        "INTEGER", // Typename
        css::sdbc::DataType::INTEGER, // sdbc-type
        10, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        true, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "INTEGER", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: INT SDBC-Type: INTEGER ----------
    {
        "INT", // Typename
        css::sdbc::DataType::INTEGER, // sdbc-type
        10, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        true, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "INT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: MEDIUMINT SDBC-Type: INTEGER ----------
    {
        "MEDIUMINT", // Typename
        css::sdbc::DataType::INTEGER, // sdbc-type
        7, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        true, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "MEDIUMINT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: SMALLINT SDBC-Type: INTEGER ----------
    {
        "SMALLINT", // Typename
        css::sdbc::DataType::SMALLINT, // sdbc-type
        5, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        true, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "SMALLINT", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: FLOAT SDBC-Type: REAL ----------
    {
        "FLOAT", // Typename
        css::sdbc::DataType::REAL, // sdbc-type
        10, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M,D)] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "FLOAT", // local type name
        -38, // minimum scale
        38 // maximum scale
    },

    // ----------- MySQL-Type: DOUBLE SDBC-Type: DOUBLE ----------
    {
        "DOUBLE", // Typename
        css::sdbc::DataType::DOUBLE, // sdbc-type
        17, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M,D)] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "DOUBLE", // local type name
        -308, // minimum scale
        308 // maximum scale
    },

    // ----------- MySQL-Type: DOUBLE PRECISION SDBC-Type: DOUBLE ----------
    {
        "DOUBLE PRECISION", // Typename
        css::sdbc::DataType::DOUBLE, // sdbc-type
        17, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M,D)] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "DOUBLE PRECISION", // local type name
        -308, // minimum scale
        308 // maximum scale
    },

    // ----------- MySQL-Type: REAL SDBC-Type: DOUBLE ----------
    {
        "REAL", // Typename
        css::sdbc::DataType::DOUBLE, // sdbc-type
        17, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M,D)] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "REAL", // local type name
        -308, // minimum scale
        308 // maximum scale
    },

    // ----------- MySQL-Type: VARCHAR SDBC-Type: VARCHAR ----------
    {
        "VARCHAR", // Typename
        css::sdbc::DataType::VARCHAR, // sdbc-type
        // tdf#165928: a VARCHAR should be more than 255 characters
        // Mysql/MariaDB accepts 65535 bytes
        // see https://mariadb.com/docs/skysql-dbaas/ref/xpand/data-types/VARCHAR/
        // but precision corresponds to the number of characters and not the number of bytes:
        // see https://docs.oracle.com/javase/8/docs/api/java/sql/DatabaseMetaData.html
        // Knowing that Unicode can use until 4 bytes, it means a varchar field can contain:
        // 65535 / 4, so 16383 characters
        // TODO if there's a way to retrieve the encoding, we could be more precise
        16383, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "(M)", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "VARCHAR", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: ENUM SDBC-Type: VARCHAR ----------
    {
        "ENUM", // Typename
        css::sdbc::DataType::VARCHAR, // sdbc-type
        0xFFFF, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "ENUM", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: SET SDBC-Type: VARCHAR ----------
    {
        "SET", // Typename
        css::sdbc::DataType::VARCHAR, // sdbc-type
        64, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "SET", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: DATE SDBC-Type: DATE ----------
    {
        "DATE", // Typename
        css::sdbc::DataType::DATE, // sdbc-type
        0, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "DATE", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: TIME SDBC-Type: TIME ----------
    {
        "TIME", // Typename
        css::sdbc::DataType::TIME, // sdbc-type
        0, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "TIME", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: DATETIME SDBC-Type: TIMESTAMP ----------
    {
        "DATETIME", // Typename
        css::sdbc::DataType::TIMESTAMP, // sdbc-type
        0, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "DATETIME", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: TIMESTAMP SDBC-Type: TIMESTAMP ----------
    {
        "TIMESTAMP", // Typename
        css::sdbc::DataType::TIMESTAMP, // sdbc-type
        0, // Precision
        "'", // Literal prefix
        "'", // Literal suffix
        "[(M)]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        false, // unsignable
        false, // fixed_prec_scale
        false, // auto_increment
        "TIMESTAMP", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: TIMESTAMP SDBC-Type: TIMESTAMP ----------

    // ----------- MySQL-Type: YEAR SDBC-Type: INTEGER  ----------
    {
        "YEAR", // Typename
        css::sdbc::DataType::SMALLINT, // sdbc-type
        10, // Precision
        "", // Literal prefix
        "", // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]", // Create params
        css::sdbc::ColumnValue::NULLABLE, // nullable
        false, // case sensitive
        css::sdbc::ColumnSearch::FULL, // searchable
        true, // unsignable
        false, // fixed_prec_scale
        true, // auto_increment
        "YEAR", // local type name
        0, // minimum scale
        0 // maximum scale
    },

    // ----------- MySQL-Type: YEAR SDBC-Type: INTEGER  ----------
    { nullptr, 0, 0, nullptr, nullptr, nullptr, 0, false, 0, false, false, false, nullptr, 0, 0 }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
