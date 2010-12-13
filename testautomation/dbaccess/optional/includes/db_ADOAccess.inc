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
'* short description : Create ADO Access DS & Table & fill in Test
'*
'\***********************************************************************
testcase db_ADOAccess

    if gtSYSName = "Mac OS X" or gtSYSName = "Solaris SPARC" or gtSYSName = "Solaris x86" or gtSYSName = "Linux" then
        printlog "This is an ADO test and works only on Windosen"
        printlog "You have a: " + gtSYSName + " OS - Test Stopped !"
	else 
    
        ' *********************************************************************
        '   databases specific settings for ADO Access OLEDB 4.0 on Windosen
        ' *********************************************************************
            
        Dim i_fieldcount as integer 'amount of field types - columns
        i_fieldcount = 16
            
        Dim ifieldrow_amount as integer 'amount of inserted rows
        ifieldrow_amount = 3
        
        Dim sFileName as string
        sFileName = gOfficePath + ConvertPath("user/work/TT_Ado_Access.odb")
        
        Dim sDBURL as string
        sDBURL = "PROVIDER=Microsoft.Jet.OLEDB.4.0;DATA SOURCE="
        
        Dim sTableName as string
        sTableName = "tt_test_create-table"        
        
        Dim sPWD as string
        sPWD = "ignore"
        
        dim sCatalog as string
        sCatalog = " "     			' not used in this ds
        
        dim sSchema as string
        sSchema = " "     			' not used in this ds
        
        dim sRelTable1 as string    'for relation test (tRelation1&2)
        sRelTable1 = "TT_Rel1"
        
        dim sRelTable2 as string    'for relation test (tRelation1&2)
        sRelTable2 = "TT_Rel2"
        
        dim sRelTable3 as string    'for relation test (tRelation2)
        sRelTable3 = "TT_Rel3"
        
        
        Dim aFields(i_fieldcount,2) as string
               
        aFields(1,1)="tt_bit"			'name of fieldtype
        aFields(1,2)="bit"				'number of fieldtype (listbox entry)
                
        aFields(2,1)="tt_byte"		
        aFields(2,2)="byte"
        
        aFields(3,1)="tt_guid"		
        aFields(3,2)="guid"
        
        aFields(4,1)="tt_bigbinary"		
        aFields(4,2)="bigbinary"
        
        aFields(5,1)="tt_longbinary"		
        aFields(5,2)="longbinary"
        
        aFields(6,1)="tt_varbinary"		
        aFields(6,2)="varbinary"
        
        'SQL Null [ ] is a foult in metadata of driver bug: #108535 in OLEDB 4.0 (number without field -> field does not exist)
        
        aFields(7,1)="tt_decimal"	' bug #108512 - should work after that
        aFields(7,2)="decimal"
        
        aFields(8,1)="tt_long"		
        aFields(8,2)="long"
           
        aFields(9,1)="tt_short"		
        aFields(9,2)="short"
        
        aFields(10,1)="tt_single"		
        aFields(10,2)="single"		
        
        aFields(11,1)="tt_double"		
        aFields(11,2)="double"
        
        aFields(12,1)="tt_currency"		
        aFields(12,2)="currency"
        
        aFields(13,1)="tt_longtext"		
        aFields(13,2)="longtext"		
        
        aFields(14,1)="tt_varchar"		
        aFields(14,2)="varchar"
        
        aFields(15,1)="tt_datetime"		
        aFields(15,2)="datetime"
        
        
        Dim aFieldsContent(1,16) as string
               
        aFieldsContent(1,1)="1"
        aFieldsContent(1,2)="<space>"
        aFieldsContent(1,3)="1"
        aFieldsContent(1,4)="1"
        aFieldsContent(1,5)=""
        aFieldsContent(1,6)=""
        aFieldsContent(1,7)=""
        aFieldsContent(1,8)=""
        aFieldsContent(1,9)="1"
        aFieldsContent(1,10)="1"
        aFieldsContent(1,11)="1"
        aFieldsContent(1,12)="1"
        aFieldsContent(1,13)="1"
        aFieldsContent(1,14)="1"
        aFieldsContent(1,15)="1"
        aFieldsContent(1,16)="1"       
          
        dim dbok as boolean
   		dbok = false
        
        ' restart the office to unlock the mdb file. Remember the Quickstarter
        printlog "tart the office to unlock the mdb file. Remember the Quickstarter"
        ExitRestartTheOffice
        
    	app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/ado_datasource/testdb.mdb"),gOfficePath + ConvertPath("user/work/testdb.mdb")
        dbok =  fCreateADODatasource( sFileName, sDBURL + gOfficePath + ConvertPath("user/work/testdb.mdb"))
        if dbok = true then

        	call fOpendatabase(sFileName)
            call fCreateTable(aFields(),sTableName)
            call fInsertIntoTable( aFieldsContent(), sTableName)
            call fDeleteTable(sTableName)	
	        call fCloseDatabase
                        
            call tRelation( sFileName, sPWD, sRelTable1, sRelTable2 )
            call tDoubleRelation( sFileName, sPWD, sRelTable1, sRelTable2, sRelTable3 )
                        
            call db_Query(sFileName)

		else 
            warnlog "Data Source could not be created - beyond testcases stopped"
		endif
	endif
        
endcase

