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

#include <stdio.h>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include "mysqlc_types.hxx"

using namespace com::sun::star::sdbc;

TypeInfoDef mysqlc_types[] = {

    // ------------- MySQL-Type: BIT. SDBC-Type: Bit -------------
    {
        "BIT",                                       // Typename
        com::sun::star::sdbc::DataType::BIT,         // sdbc-type
        1,                                           // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "BIT",                                       // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ------------ MySQL-Type: BOOL. SDBC-Type: Bit -------------
    {
        "BOOL",                                      // Typename
        com::sun::star::sdbc::DataType::BIT,         // sdbc-type
        1,                                           // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "BOOL",                                      // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // --------- MySQL-Type: TINYINT SDBC-Type: TINYINT ----------
    {
        "TINYINT",                                   // Typename
        com::sun::star::sdbc::DataType::TINYINT,     // sdbc-type
        3,                                           // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]",               // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        true,                                    // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "TINYINT",                                   // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: BIGINT SDBC-Type: BIGINT ----------
    {
        "BIGINT",                                    // Typename
        com::sun::star::sdbc::DataType::BIGINT,      // sdbc-type
        19,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]",               // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        true,                                    // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "BIGINT",                                    // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: LONG VARBINARY SDBC-Type: LONGVARBINARY ----------
    {
        "LONG VARBINARY",                            // Typename
        com::sun::star::sdbc::DataType::LONGVARBINARY, // sdbc-type
        16777215,                                    // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "LONG VARBINARY",                            // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: MEDIUMBLOB SDBC-Type: LONGVARBINARY ----------
    {
        "MEDIUMBLOB",                                // Typename
        com::sun::star::sdbc::DataType::LONGVARBINARY, // sdbc-type
        16777215,                                    // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "MEDIUMBLOB",                                // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: LONGBLOB SDBC-Type: LONGVARBINARY ----------
    {
        "LONGBLOB",                                  // Typename
        com::sun::star::sdbc::DataType::LONGVARBINARY, // sdbc-type
        -1,                                          // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "LONGBLOB",                                  // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: BLOB SDBC-Type: LONGVARBINARY ----------
    {
        "BLOB",                                      // Typename
        com::sun::star::sdbc::DataType::LONGVARBINARY, // sdbc-type
        0xFFFF,                                      // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "BLOB",                                      // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: TINYBLOB SDBC-Type: LONGVARBINARY ----------
    {
        "TINYBLOB",                                  // Typename
        com::sun::star::sdbc::DataType::LONGVARBINARY, // sdbc-type
        0xFF,                                         // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "TINYBLOB",                                  // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: VARBINARY SDBC-Type: VARBINARY ----------
    {
        "VARBINARY",                                 // Typename
        com::sun::star::sdbc::DataType::VARBINARY,   // sdbc-type
        0xFF,                                        // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "(M)",                                       // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "VARBINARY",                                 // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: BINARY SDBC-Type: BINARY ----------
    {
        "BINARY",                                    // Typename
        com::sun::star::sdbc::DataType::BINARY,      // sdbc-type
        0xFF,                                        // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "(M)",                                       // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        true,                                    // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "VARBINARY",                                 // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: LONG VARCHAR SDBC-Type: LONG VARCHAR ----------
    {
        "LONG VARCHAR",                              // Typename
        com::sun::star::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFFFFFF,                                    // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "LONG VARCHAR",                              // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: MEDIUMTEXT SDBC-Type: LONG VARCHAR ----------
    {
        "MEDIUMTEXT",                                // Typename
        com::sun::star::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFFFFFF,                                    // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "MEDIUMTEXT",                                // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: LONGTEXT SDBC-Type: LONG VARCHAR ----------
    {
        "LONGTEXT",                                  // Typename
        com::sun::star::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFFFFFF,                                    // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "LONGTEXT",                                  // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: TEXT SDBC-Type: LONG VARCHAR ----------
    {
        "TEXT",                                      // Typename
        com::sun::star::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFFFF,                                      // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "TEXT",                                      // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: TINYTEXT SDBC-Type: LONG VARCHAR ----------
    {
        "TINYTEXT",                                  // Typename
        com::sun::star::sdbc::DataType::LONGVARCHAR, // sdbc-type
        0xFF,                                        // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "TINYTEXT",                                  // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: CHAR SDBC-Type: CHAR ----------
    {
        "CHAR",                                      // Typename
        com::sun::star::sdbc::DataType::CHAR,        // sdbc-type
        0xFF,                                        // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "(M)",                                       // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "NUMERIC",                                   // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: DECIMAL SDBC-Type: DECIMAL ----------
    {
        "DECIMAL",                                   // Typename
        com::sun::star::sdbc::DataType::DECIMAL,     // sdbc-type
        17,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M[,D])] [ZEROFILL]",                      // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "DECIMAL",                                   // local type name
        -308,                                        // minimum scale
        308,                                         // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: NUMERIC SDBC-Type: NUMERIC ----------
    {
        "NUMERIC",                                   // Typename
        com::sun::star::sdbc::DataType::NUMERIC,     // sdbc-type
        17,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M[,D])] [ZEROFILL]",                      // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "NUMERIC",                                   // local type name
        -308,                                        // minimum scale
        308,                                         // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: INTEGER SDBC-Type: INTEGER ----------
    {
        "INTEGER",                                   // Typename
        com::sun::star::sdbc::DataType::INTEGER,     // sdbc-type
        10,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]",               // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        true,                                    // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "INTEGER",                                   // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: INT SDBC-Type: INTEGER ----------
    {
        "INT",                                       // Typename
        com::sun::star::sdbc::DataType::INTEGER,     // sdbc-type
        10,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]",               // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        true,                                    // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "INT",                                       // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: MEDIUMINT SDBC-Type: INTEGER ----------
    {
        "MEDIUMINT",                                 // Typename
        com::sun::star::sdbc::DataType::INTEGER,     // sdbc-type
         7,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]",               // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        true,                                    // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "MEDIUMINT",                                 // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: SMALLINT SDBC-Type: INTEGER ----------
    {
        "SMALLINT",                                  // Typename
        com::sun::star::sdbc::DataType::SMALLINT,    // sdbc-type
         5,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M)] [UNSIGNED] [ZEROFILL]",               // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        true,                                    // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "SMALLINT",                                  // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: FLOAT SDBC-Type: REAL ----------
    {
        "FLOAT",                                     // Typename
        com::sun::star::sdbc::DataType::REAL,        // sdbc-type
        10,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M,D)] [ZEROFILL]",                        // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "FLOAT",                                     // local type name
        -38,                                         // minimum scale
        38,                                          // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: DOUBLE SDBC-Type: DOUBLE ----------
    {
        "DOUBLE",                                    // Typename
        com::sun::star::sdbc::DataType::DOUBLE,      // sdbc-type
        17,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M,D)] [ZEROFILL]",                        // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "DOUBLE",                                    // local type name
        -308,                                        // minimum scale
        308,                                         // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: DOUBLE PRECISION SDBC-Type: DOUBLE ----------
    {
        "DOUBLE PRECISION",                          // Typename
        com::sun::star::sdbc::DataType::DOUBLE,      // sdbc-type
        17,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M,D)] [ZEROFILL]",                        // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "DOUBLE PRECISION",                          // local type name
        -308,                                        // minimum scale
        308,                                         // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: REAL SDBC-Type: DOUBLE ----------
    {
        "REAL",                                      // Typename
        com::sun::star::sdbc::DataType::DOUBLE,      // sdbc-type
        17,                                          // Precision
        "",                                          // Literal prefix
        "",                                          // Literal suffix
        "[(M,D)] [ZEROFILL]",                        // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        true,                                    // auto_increment
        "REAL",                                      // local type name
        -308,                                        // minimum scale
        308,                                         // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: VARCHAR SDBC-Type: VARCHAR ----------
    {
        "VARCHAR",                                   // Typename
        com::sun::star::sdbc::DataType::VARCHAR,     // sdbc-type
        255,                                         // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "(M)",                                       // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "VARCHAR",                                   // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: ENUM SDBC-Type: VARCHAR ----------
    {
        "ENUM",                                      // Typename
        com::sun::star::sdbc::DataType::VARCHAR,     // sdbc-type
        0xFFFF,                                      // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "ENUM",                                      // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: SET SDBC-Type: VARCHAR ----------
    {
        "SET",                                       // Typename
        com::sun::star::sdbc::DataType::VARCHAR,     // sdbc-type
        64,                                          // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "SET",                                       // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: DATE SDBC-Type: DATE ----------
    {
        "DATE",                                      // Typename
        com::sun::star::sdbc::DataType::DATE,        // sdbc-type
        0,                                           // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "DATE",                                      // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: TIME SDBC-Type: TIME ----------
    {
        "TIME",                                      // Typename
        com::sun::star::sdbc::DataType::TIME,        // sdbc-type
        0,                                           // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "TIME",                                      // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: DATETIME SDBC-Type: TIMESTAMP ----------
    {
        "DATETIME",                                  // Typename
        com::sun::star::sdbc::DataType::TIMESTAMP,   // sdbc-type
        0,                                           // Precision
        "'",                                          // Literal prefix
        "'",                                          // Literal suffix
        "",                                          // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "DATETIME",                                  // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
     },

    // ----------- MySQL-Type: TIMESTAMP SDBC-Type: TIMESTAMP ----------
    {
        "TIMESTAMP",                                 // Typename
        com::sun::star::sdbc::DataType::TIMESTAMP,   // sdbc-type
        0,                                           // Precision
        "'",                                         // Literal prefix
        "'",                                         // Literal suffix
        "[(M)]",                                     // Create params
        com::sun::star::sdbc::ColumnValue::NULLABLE, // nullable
        false,                                   // case sensitive
        com::sun::star::sdbc::ColumnSearch::FULL,    // searchable
        false,                                   // unsignable
        false,                                   // fixed_prec_scale
        false,                                   // auto_increment
        "TIMESTAMP",                                 // local type name
        0,                                           // minimum scale
        0,                                           // maximum scale
        0,                                           // sql data type (unsued)
        0,                                           // sql datetime sub (unsued)
        10                                           // num prec radix
    },

    // ----------- MySQL-Type: TIMESTAMP SDBC-Type: TIMESTAMP ----------
    {
        nullptr, 0, 0, nullptr, nullptr, nullptr, 0, false, 0, false, false, false, nullptr, 0, 0, 0, 0, 0
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
