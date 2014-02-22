/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <stdio.h>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include "mysqlc_types.hxx"

using namespace com::sun::star::sdbc;

TypeInfoDef mysqlc_types[] = {

    
    {
        "BIT",                                       
        com::sun::star::sdbc::DataType::BIT,         
        1,                                           
        "",                                          
        "",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "BIT",                                       
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "BOOL",                                      
        com::sun::star::sdbc::DataType::BIT,         
        1,                                           
        "",                                          
        "",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "BOOL",                                      
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "TINYINT",                                   
        com::sun::star::sdbc::DataType::TINYINT,     
        3,                                           
        "",                                          
        "",                                          
        "[(M)] [UNSIGNED] [ZEROFILL]",               
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_True,                                    
        sal_False,                                   
        sal_True,                                    
        "TINYINT",                                   
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "BIGINT",                                    
        com::sun::star::sdbc::DataType::BIGINT,      
        19,                                          
        "",                                          
        "",                                          
        "[(M)] [UNSIGNED] [ZEROFILL]",               
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_True,                                    
        sal_False,                                   
        sal_True,                                    
        "BIGINT",                                    
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "LONG VARBINARY",                            
        com::sun::star::sdbc::DataType::LONGVARBINARY, 
        16777215,                                    
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "LONG VARBINARY",                            
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "MEDIUMBLOB",                                
        com::sun::star::sdbc::DataType::LONGVARBINARY, 
        16777215,                                    
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "MEDIUMBLOB",                                
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "LONGBLOB",                                  
        com::sun::star::sdbc::DataType::LONGVARBINARY, 
        -1,                                          
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "LONGBLOB",                                  
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "BLOB",                                      
        com::sun::star::sdbc::DataType::LONGVARBINARY, 
        0xFFFF,                                      
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "BLOB",                                      
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "TINYBLOB",                                  
        com::sun::star::sdbc::DataType::LONGVARBINARY, 
        0xFF,                                         
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "TINYBLOB",                                  
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "VARBINARY",                                 
        com::sun::star::sdbc::DataType::VARBINARY,   
        0xFF,                                        
        "'",                                          
        "'",                                          
        "(M)",                                       
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "VARBINARY",                                 
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "BINARY",                                    
        com::sun::star::sdbc::DataType::BINARY,      
        0xFF,                                        
        "'",                                          
        "'",                                          
        "(M)",                                       
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_True,                                    
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "VARBINARY",                                 
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "LONG VARCHAR",                              
        com::sun::star::sdbc::DataType::LONGVARCHAR, 
        0xFFFFFF,                                    
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "LONG VARCHAR",                              
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "MEDIUMTEXT",                                
        com::sun::star::sdbc::DataType::LONGVARCHAR, 
        0xFFFFFF,                                    
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "MEDIUMTEXT",                                
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "LONGTEXT",                                  
        com::sun::star::sdbc::DataType::LONGVARCHAR, 
        0xFFFFFF,                                    
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "LONGTEXT",                                  
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "TEXT",                                      
        com::sun::star::sdbc::DataType::LONGVARCHAR, 
        0xFFFF,                                      
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "TEXT",                                      
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "TINYTEXT",                                  
        com::sun::star::sdbc::DataType::LONGVARCHAR, 
        0xFF,                                        
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "TINYTEXT",                                  
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "CHAR",                                      
        com::sun::star::sdbc::DataType::CHAR,        
        0xFF,                                        
        "'",                                          
        "'",                                          
        "(M)",                                       
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "NUMERIC",                                   
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "DECIMAL",                                   
        com::sun::star::sdbc::DataType::DECIMAL,     
        17,                                          
        "",                                          
        "",                                          
        "[(M[,D])] [ZEROFILL]",                      
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_True,                                    
        "DECIMAL",                                   
        -308,                                        
        308,                                         
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "NUMERIC",                                   
        com::sun::star::sdbc::DataType::NUMERIC,     
        17,                                          
        "",                                          
        "",                                          
        "[(M[,D])] [ZEROFILL]",                      
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_True,                                    
        "NUMERIC",                                   
        -308,                                        
        308,                                         
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "INTEGER",                                   
        com::sun::star::sdbc::DataType::INTEGER,     
        10,                                          
        "",                                          
        "",                                          
        "[(M)] [UNSIGNED] [ZEROFILL]",               
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_True,                                    
        sal_False,                                   
        sal_True,                                    
        "INTEGER",                                   
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "INT",                                       
        com::sun::star::sdbc::DataType::INTEGER,     
        10,                                          
        "",                                          
        "",                                          
        "[(M)] [UNSIGNED] [ZEROFILL]",               
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_True,                                    
        sal_False,                                   
        sal_True,                                    
        "INT",                                       
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "MEDIUMINT",                                 
        com::sun::star::sdbc::DataType::INTEGER,     
         7,                                          
        "",                                          
        "",                                          
        "[(M)] [UNSIGNED] [ZEROFILL]",               
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_True,                                    
        sal_False,                                   
        sal_True,                                    
        "MEDIUMINT",                                 
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "SMALLINT",                                  
        com::sun::star::sdbc::DataType::SMALLINT,    
         5,                                          
        "",                                          
        "",                                          
        "[(M)] [UNSIGNED] [ZEROFILL]",               
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_True,                                    
        sal_False,                                   
        sal_True,                                    
        "SMALLINT",                                  
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "FLOAT",                                     
        com::sun::star::sdbc::DataType::REAL,        
        10,                                          
        "",                                          
        "",                                          
        "[(M,D)] [ZEROFILL]",                        
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_True,                                    
        "FLOAT",                                     
        -38,                                         
        38,                                          
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "DOUBLE",                                    
        com::sun::star::sdbc::DataType::DOUBLE,      
        17,                                          
        "",                                          
        "",                                          
        "[(M,D)] [ZEROFILL]",                        
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_True,                                    
        "DOUBLE",                                    
        -308,                                        
        308,                                         
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "DOUBLE PRECISION",                          
        com::sun::star::sdbc::DataType::DOUBLE,      
        17,                                          
        "",                                          
        "",                                          
        "[(M,D)] [ZEROFILL]",                        
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_True,                                    
        "DOUBLE PRECISION",                          
        -308,                                        
        308,                                         
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "REAL",                                      
        com::sun::star::sdbc::DataType::DOUBLE,      
        17,                                          
        "",                                          
        "",                                          
        "[(M,D)] [ZEROFILL]",                        
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_True,                                    
        "REAL",                                      
        -308,                                        
        308,                                         
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "VARCHAR",                                   
        com::sun::star::sdbc::DataType::VARCHAR,     
        255,                                         
        "'",                                          
        "'",                                          
        "(M)",                                       
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "VARCHAR",                                   
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "ENUM",                                      
        com::sun::star::sdbc::DataType::VARCHAR,     
        0xFFFF,                                      
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "ENUM",                                      
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "SET",                                       
        com::sun::star::sdbc::DataType::VARCHAR,     
        64,                                          
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "SET",                                       
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "DATE",                                      
        com::sun::star::sdbc::DataType::DATE,        
        0,                                           
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "DATE",                                      
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "TIME",                                      
        com::sun::star::sdbc::DataType::TIME,        
        0,                                           
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "TIME",                                      
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "DATETIME",                                  
        com::sun::star::sdbc::DataType::TIMESTAMP,   
        0,                                           
        "'",                                          
        "'",                                          
        "",                                          
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "DATETIME",                                  
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
     },

    
    {
        "TIMESTAMP",                                 
        com::sun::star::sdbc::DataType::TIMESTAMP,   
        0,                                           
        "'",                                         
        "'",                                         
        "[(M)]",                                     
        com::sun::star::sdbc::ColumnValue::NULLABLE, 
        sal_False,                                   
        com::sun::star::sdbc::ColumnSearch::FULL,    
        sal_False,                                   
        sal_False,                                   
        sal_False,                                   
        "TIMESTAMP",                                 
        0,                                           
        0,                                           
        0,                                           
        0,                                           
        10                                           
    },

    
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
