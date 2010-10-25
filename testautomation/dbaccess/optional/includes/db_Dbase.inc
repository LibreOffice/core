'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : marc.neumann@oracle.com
'*
'* short description : Create dBase DS & Table & fill in Test
'*
'\***********************************************************************
testcase db_Dbase
    
    ' *********************************************************************
    '   databases specific settings for dBase
    ' *********************************************************************
        
    Dim i_fieldcount as integer 'amount of field types - columns
	i_fieldcount = 7 'amount of field types (no primary key supported in dBase) normally: 7
		
	Dim ifieldrow_amount as integer 'amount of inserted rows
	ifieldrow_amount = 3
	
    Dim sFileName as string
	sFileName = gOfficePath + ConvertPath("user/work/tt_dbase-01.odb")
    
    Dim sDBURL as string
	sDBURL = "user/work"
    
    Dim sTableName as string	
    sTableName = "tt_test_create-table"
    	
	Dim sPWD as string
	sPWD = "ignore"
	
	Dim sCatalog as string
	sCatalog = " "     			' not used in this ds
	
	Dim sSchema as string
	sSchema = " "     			' not used in this ds
	
	
    Dim aFieldTypeContent(i_fieldcount,2) as string 'database specific data matrix
        
    aFieldTypeContent(1,1)="tt_boolean"	        'name of fieldtype
    aFieldTypeContent(1,2)="boolean"         	'number of fieldtype (listbox entry)
    
    aFieldTypeContent(2,1)="tt_longvar"			
    aFieldTypeContent(2,2)="longvarchar"			
    
    aFieldTypeContent(3,1)="tt_char"		
    aFieldTypeContent(3,2)="char"		
        
    aFieldTypeContent(4,1)="tt_decimal"
    aFieldTypeContent(4,2)="decimal"
        
    aFieldTypeContent(5,1)="tt_varchar"
    aFieldTypeContent(5,2)="varchar"
         
    aFieldTypeContent(6,1)="tt_date"    
    aFieldTypeContent(6,2)="date"
    
    Dim aFieldContent(1,6) as string 'database specific data matrix
        
    aFieldContent(1,1)="<space>"
    aFieldContent(1,2)="this is a memo field"    
    aFieldContent(1,3)="char"			
    aFieldContent(1,4)="1"		
    aFieldContent(1,5)="this is a text field"
    aFieldContent(1,6)="13.05.2004  "
    
   		dim dbok as boolean   		
        dbok = fCreateDbaseDatasource(sFileName,gOfficePath + ConvertPath("user/work"),"TT_dBase")       
        if dbok = true then
        
            'needed for: tQuery testcase - query file is copied in the work directory
            ' the files need to be copy before the datasource is open
            app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/dbase_datasource/TT_Forms.dbf"),gOfficePath + ConvertPath("user/work/TT_Forms.dbf")
            app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/dbase_datasource/TT_Query1.dbf"),gOfficePath + ConvertPath("user/work/TT_Query1.dbf")
            app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/dbase_datasource/TT_Query1.dbt"),gOfficePath + ConvertPath("user/work/TT_Query1.dbt")
        
            call fOpendatabase(sFileName)
            call fCreateTable( aFieldTypeContent(), sTableName)
            call fInsertIntoTable( aFieldContent(), sTableName)
            call fCloseDatabase    
                       
            call db_Query(sFileName,"dbase")
            
            use "dbaccess/optional/includes/db_IndexDesign.inc"
            call tIndex(sFileName, sTableName)

		else 
            warnlog "Data Source could not be created - beyond testcases stopped"
		endif
	
endcase

