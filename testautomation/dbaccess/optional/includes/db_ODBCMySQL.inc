'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'* short description : ODBC MySQL Test
'*
'\***********************************************************************

testcase db_ODBCMySQL
    
    if gtSYSName = "MAC" or gtSYSName = "Solaris SPARC" or gtSYSName = "Solaris x86" then
        printlog "This test does work only under linux and windows yet."
        printlog "You have a: " + gtSYSName + " OS - Test Stopped !"
	else                
        
        ' **************************************************
        '   databases specific settings for ODBC MySQL
        ' **************************************************
        
        Dim sFileName as string	
        sFileName = gOfficePath + Convertpath("user/work/TT_ODBC-MYSQL.odb")
            
        Dim sTableName as string
        sTableName = "tt_test_create-table"    
        
        Dim sUser as string
        sUser = "testtool"
        
        Dim sPWD as string
        sPWD = "testtool"
        
        dim sCatalog as string
        sCatalog = " "     			' not used in this ds
        
        dim sSchema as string
        sSchema = " "     			' not used in this ds
        
        Dim aFieldTypeContent(30,2) as string 'database specific data matrix
            
        aFieldTypeContent(1,1)="tt_bool"
        aFieldTypeContent(1,2)="bool"
            
        aFieldTypeContent(2,1)="tt_tinyint"
        aFieldTypeContent(2,2)="tinyint"
             
        aFieldTypeContent(3,1)="tt_bigint"
        aFieldTypeContent(3,2)="bigint"
         
        aFieldTypeContent(4,1)="tt_long_varbinary"
        aFieldTypeContent(4,2)="long varbinary"
            
        aFieldTypeContent(5,1)="tt_mediumblob"
        aFieldTypeContent(5,2)="mediumblob"
             
        aFieldTypeContent(6,1)="tt_longblob"
        aFieldTypeContent(6,2)="longblob"
            
        aFieldTypeContent(7,1)="tt_blob"
        aFieldTypeContent(7,2)="blob"
            
        aFieldTypeContent(8,1)="tt_tinyblob"
        aFieldTypeContent(8,2)="tinyblob"
            
        aFieldTypeContent(9,1)="tt_varbinary"
        aFieldTypeContent(9,2)="varbinary"
            
        aFieldTypeContent(10,1)="tt_binary"
        aFieldTypeContent(10,2)="binary"
            
        aFieldTypeContent(11,1)="tt_longvarchar"
        aFieldTypeContent(11,2)="long varchar"
            
        aFieldTypeContent(12,1)="tt_mediumtext"
        aFieldTypeContent(12,2)="mediumtext"
            
        aFieldTypeContent(13,1)="tt_longtext"
        aFieldTypeContent(13,2)="longtext"
            
        aFieldTypeContent(14,1)="tt_text"
        aFieldTypeContent(14,2)="text"
            
        aFieldTypeContent(15,1)="tt_tinytext"
        aFieldTypeContent(15,2)="tinytext"
            
        aFieldTypeContent(16,1)="tt_char"
        aFieldTypeContent(16,2)="char"
            
        aFieldTypeContent(17,1)="tt_numeric"
        aFieldTypeContent(17,2)="numeric"
            
        aFieldTypeContent(18,1)="tt_decimal"
        aFieldTypeContent(18,2)="decimal"
            
        aFieldTypeContent(19,1)="tt_integer"
        aFieldTypeContent(19,2)="integer"
            
        aFieldTypeContent(20,1)="tt_int"
        aFieldTypeContent(20,2)="int"
            
        aFieldTypeContent(21,1)="tt_mediumint"
        aFieldTypeContent(21,2)="mediumint"
            
        aFieldTypeContent(22,1)="tt_smallint"
        aFieldTypeContent(22,2)="smallint"
           
        aFieldTypeContent(23,1)="tt_float"
        aFieldTypeContent(23,2)="float"
        
        aFieldTypeContent(24,1)="tt_varchar"	
        aFieldTypeContent(24,2)="varchar"
         
        aFieldTypeContent(25,1)="tt_date"	
        aFieldTypeContent(25,2)="date"
         
        aFieldTypeContent(26,1)="tt_time"	
        aFieldTypeContent(26,2)="time"
        
        aFieldTypeContent(27,1)="tt_datetime"	
        aFieldTypeContent(27,2)="datetime"
        
        aFieldTypeContent(28,1)="tt_timestamp"	
        aFieldTypeContent(28,2)="timestamp"
        
        aFieldTypeContent(29,1)="tt_bit"
        aFieldTypeContent(29,2)="bit"
    
        
        
        Dim aFieldContent(1,6) as string 'database specific data matrix
            
        aFieldContent(1,1)="1"
        aFieldContent(1,2)="<space>"    
        aFieldContent(1,3)="1"			
        aFieldContent(1,4)="1"		
        aFieldContent(1,5)="1"
        aFieldContent(1,6)="1"
            
        dim dbok as boolean
        dbok = false
        
        dim aDatabaseProperties(5) as string
        aDatabaseProperties() = tools_dbtools_fgetMySQLODBCDatabaseProperties()
        
        ' if and only if no properties are defined in the environment file the test is stopped
        if(aDatabaseProperties(1) = "no") then
            qaerrorlog "No database properties from Mysql defiened. The Test is stopped here."
            goto endsub
        endif
        
        dbok = fCreateMySQL_ODBC_Datasource(sFileName,aDatabaseProperties(2),aDatabaseProperties(3))        
        
        if dbok = true then
        
            call fOpendatabase(sFileName,aDatabaseProperties(4))
            call fCreateTable( aFieldTypeContent(), sTableName)
            call fInsertIntoTable( aFieldContent(), sTableName)
            call fCloseDatabase    
            
            'use "dbaccess/optional/includes/b_lvl1_Query.inc"
            'call b_lvl1_Query(sFileName,"dbase")
            
        else 
            warnlog "Data Source could not be created - beyond testcases stopped"
        endif
	endif
endcase

