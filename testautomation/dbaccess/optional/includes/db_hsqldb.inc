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
'* short description : hsqldb test
'*
'\***********************************************************************
sub db_hsqldb

    call tGeneral
    call ExitRestartTheOffice
    call tRenameTable
    call tOpenDatabaseWithViews
    call tRenameTableWhichUsedInViews
    
    call tQueryOpenInSQLMode
    call ExitRestartTheOffice
    
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb") , gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")            
    call db_Query(gOfficePath + "user/work/TT_hsqldb.odb" ,"hsqldb")
   
end sub

testcase tGeneral
    
    ' *********************************************************************
    '   databases specific settings for hsqldb
    ' *********************************************************************
        
    Dim i_fieldcount as integer 'amount of field types - columns
    i_fieldcount = 8 'amount of field types (no primary key supported in dBase) normally: 7
		
    Dim ifieldrow_amount as integer 'amount of inserted rows
    ifieldrow_amount = 3
	
    Dim sFileName as string
    sFileName = gOfficePath + ConvertPath("user/work/tt_hsqldb.odb")
    
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
    
    aFieldTypeContent(2,1)="tt_longvarchar"			
    aFieldTypeContent(2,2)="longvarchar"			
    
    aFieldTypeContent(3,1)="tt_char"		
    aFieldTypeContent(3,2)="char"		
        
    aFieldTypeContent(4,1)="tt_decimal"
    aFieldTypeContent(4,2)="decimal"
        
    aFieldTypeContent(5,1)="tt_varchar"
    aFieldTypeContent(5,2)="varchar"
         
    aFieldTypeContent(6,1)="tt_date"    
    aFieldTypeContent(6,2)="date"
    
    aFieldTypeContent(7,1)="tt_time_stamp"    
    aFieldTypeContent(7,2)="timestamp"
    
    Dim aFieldContent(1,7) as string 'database specific data matrix
        
    aFieldContent(1,1)="<space>"
    aFieldContent(1,2)="this is a memo field"    
    aFieldContent(1,3)="char"			
    aFieldContent(1,4)="1"		
    aFieldContent(1,5)="this is a text field"
    aFieldContent(1,6)="13.05.2004  "
    aFieldContent(1,7)="0"
    
   		dim dbok as boolean   		
        dbok = fCreateHSQLDatasource(sFileName)
        'dbok = true
        if dbok = true then
        
            call fOpendatabase(sFileName)
            dim bCreateTable as boolean 
            bCreateTable = fCreateTable( aFieldTypeContent(), sTableName)
            if bCreateTable = true then 'the table was created            
                call fInsertIntoTable( aFieldContent(), sTableName)
            end if    
            call fCloseDatabase    
            
            if bCreateTable = true then
                'test the index design                    
                call tIndex(sFileName, sTableName)
            end if

            'outcomment because of some bugs in forms
            'use "dbaccess/optional/includes/b_lvl1_Forms.inc"
            'call b_lvl1_Forms ("TT_dBase")
            
                      
		else 
            warnlog "Data Source could not be created - beyond testcases stopped"
		endif
	
endcase
'-------------------------------------------------------------------------
testcase tRenameTable

    app.FileCopy ConvertPath(gTesttoolPath + "dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb")
    call fOpenDatabase(ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb"))
    
    fFindTable("TT_1")
    
    printlog "rename the selected table"
    Kontext "DATABASE"
        Database.UseMenu
        call hMenuSelectNr(2)
        call hMenuSelectNr(7)

    printlog "insert any new name"
    Kontext "FormFolderNameDialog"    
        FolderReName.setText("newname")        
        printlog "and close the rename dialog with OK"
        FormFolderNameDialog.OK
       
    printlog "check if a message box appear"
    Kontext "MessageBox"
        if (MessageBox.exists()) then                     
            warnlog MessageBox.GetFixedText(1)
        endif
        
    printlog "Save and Close the database"
    fCloseDatabase(true)
    
    call fOpenDatabase(ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb"))
    
    fOpenTable("newname")
    
    fCloseTableView()
    
    fCloseDatabase()
    
endcase
'-------------------------------------------------------------------------
testcase tOpenDatabaseWithViews
    
    app.FileCopy ConvertPath(gTesttoolPath + "dbaccess/optional/input/hsql_datasource/TT_hsqldb_2.0.0.odb"),ConvertPath(gOfficePath + "user/work/TT_hsqldb_2.0.0.odb")
 
    call fOpenDatabase(ConvertPath(gOfficePath + "user/work/TT_hsqldb_2.0.0.odb"))
    
    if fOpenTable("View 1") then
        printlog "View 1 successful open"
        call fCloseTableView()
    else
        warnlog "View 1 not successful open"
        
    endif

    call fCloseDatabase()

endcase
'-------------------------------------------------------------------------
testcase tRenameTableWhichUsedInViews

    '/// check is issue 64885 occurr

    Dim s as String    
    printlog "copy the file $TesttoolPath/dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb to a locale place"
    app.FileCopy ConvertPath(gTesttoolPath + "dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb")
    
    printlog "open the data base TT_hsqldb.odb"
    call fOpenDatabase(ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb"))

    '/// select the table TT_TableForView
    printlog "select the table TT_TableForView"     
    fFindTable("TT_TableForView")

    '/// rename the selcted table
    printlog "rename the selcted table"
    Kontext "DATABASE"
        Database.UseMenu
        call hMenuSelectNr(2)
        call hMenuSelectNr(7)

    '/// insert any new name
    printlog "insert any new name"
    Kontext "FormFolderNameDialog"    
        FolderReName.setText("newname")
        '///+ and close the rename dialog with OK
        printlog "and close the rename dialog with OK"
        FormFolderNameDialog.OK
   
    '/// check if a message box appear that the table is used inside a view
    printlog "check if a message box appear that the table is used inside a view"
    Kontext "MessageBox"
        if (MessageBox.exists()) then                     
            s = MessageBox.GetFixedText(1)
            if ( InStr(s,"Table is referenced by view") = 0 ) then
                warnlog "There is a messagebox but the message $chr(13) does not tell something about the view $chr(13)"
                printlog "The content of the messages box is: " + s
            else
                printlog "OK. The right messge box appear"
            endif 
            '/// close the message box with OK
            printlog "close the message box with OK"
            MessageBox.OK
            '/// cancel the rename dialog
            printlog "cancel the rename dialog"
            Kontext "FormFolderNameDialog"    
                FormFolderNameDialog.Cancel
        else
            warnlog "There is no messages box, but there should one."
        endif 

    '/// close the database
    printlog "close the database"
    call fCloseDatabase()

endcase
'------------------------------------------------------------------------- 
testcase tQueryOpeninSQLMode

    '/// check feature issue 64651

    Dim s as String    
    printlog "copy the file $TesttoolPath/dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb to a locale place"
    app.FileCopy ConvertPath(gTesttoolPath + "dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb")
    
    printlog "open the data base TT_hsqldb.odb"
    call fOpenDatabase(ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb"))
    
    '/// open a ew Query SQL Design
    call fOpenNewSQLQueryDesign
    
    sleep(2)
    
    Kontext "QueryEditWindow"    
    QueryEditWindow.typeKeys ("Select * from TT_Query1" ,true)
    
    '/// save the sql query design
    call fSaveSQLQueryDesign("q1")
    
    '/// close the sql query design
    call fCloseSQLQueryDesign()
    
    '/// open the query in sql mode
    call fOpenQueryInSQLDesign("q1")
    
    '/// close the sql query design
    call fCloseSQLQueryDesign()
    '/// close the database
    printlog "close the database"
    call fCloseDatabase()


endcase
