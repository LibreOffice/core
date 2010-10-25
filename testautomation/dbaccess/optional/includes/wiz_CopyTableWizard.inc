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
'* short description : Copy Table Autopilot
'*
'\**************************************************************************************
Dim sDSName as string
Dim sTabName as string
dim i as integer

sub wiz_CopyTableWizard

	printlog "------------------- wiz_CopyTableWizard.inc ------------------------"
	
	call tCopy_Table_Dialog_Definition_and_data        
    call tCopy_Table_Dialog_Definition        
    call tCopy_Table_Dialog_Attach_data        
    call tCopy_Table_from_Text_Document        
    call tCopy_Table_from_Calc_Document
    ' because of issue 68334 I need to restart the office
    call ExitRestartTheOffice
    call tCopy_Table_from_hsql_to_dBase
    ' test issue 81299
    call tCopy_Table_Paste_Special
    
end sub
'--------------------------------------------------------------------
testcase tCopy_Table_Dialog_Definition_and_data

    Dim sTableDimension as string
    Dim iRowCount as integer
    Dim iColumnCount as integer
	
    '/// open the bibliography database
    printlog "open the bibliography database"	
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))

    '///Make sure that there isn't a biblio2 table
    printlog "Make sure that there isn't a biblio2 table"
    If fFindtable("biblio2") = true then
       call fDeleteTable("biblio2")
    EndIf
    
    '/// select the biblio table
    printlog "select the biblio table"
    call fFindtable("biblio")

    '/// press CTRL + C to copy the table and CTRL + V to paste the table again
    printlog "press CTRL + C to copy the table and CTRL + V to paste the table again"
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 C>",true)
        sleep(1)
        Database.typeKeys("<MOD1 V>",true)
        sleep(1)

    '/// the copy table dialog appear
    printlog "the copy table dialog appear"	
	Kontext "CopyDatabaseTable"
        if not CopyDatabaseTable.Exists(3) then
            warnlog "the Copy Table Autopilot doesn't appear"
            goto endsub
        endif
    
    '/// select the Definition and Data option
    printlog "select the Definition and Data option"
    Kontext "TabCopyDatabaseTable"
        DefinitionAndData.Check
        
    '/// change the table name to biblio
    printlog "change the table name to biblio" 
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio"        
        
    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"
	    NextBt.Click
    '/// there should be a message that the table name already exists.
    printlog "there should be a message that the table name already exists."	
	Kontext "MessageBox"
	    If not MessageBox.Exists(1) then
            warnlog "There have to be an error messages that the table name already exists"
        else
            MessageBox.OK
        endif
	
    '/// change the table name to biblio2
    printlog "change the table name to biblio2" 
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio2"
        DefinitionAndData.Check
    
    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"        	
        NextBt.Click

	sleep(1)

    '/// add all columns
    printlog "add all columns"	
	Kontext "TabColumnSelect"
	    AddAll.Click
        sleep(1)

    '/// click next
    printlog "click next"	
	Kontext "CopyDatabaseTable"
	    NextBt.Click
        sleep(1)

    '/// click OK
    printlog "click OK"	
	Kontext "CopyDatabaseTable"
        CopyDatabaseTable.OK
        sleep(1)
        
    qaerrorlog "because of issue 76051 this copied table is not checked"
    '///Check if the table is copied correct
    'printlog "Check, if the table is copied correct"
    'sTableDimension = fCreateRefFile("Bibliography", "t" , "biblio2" , "#")
    'if (sTableDimension = "") then
    '    printlog "Problem while creating Reference File"
    'else
    '    'printlog sTableDimension       'Debugcode
    '    iColumnCount = CInt(Left(sTableDimension, instr(sTableDimension,",")))+1
    '    'printlog "Number of Columns: " & iColumnCount  'Debugcode
    '    iRowCount = CInt(Right(sTableDimension, (Len(sTableDimension) - instr(sTableDimension,","))))+1
    '    'printlog "Number of Rows: " & iRowCount    'Debugcode
    '    call tCompareMatrixValues("Bibliography", "user/work/biblio2.ttr", "t", "biblio", iRowCount, iColumnCount, "#")
    'endif        
       

    '/// delete the copy table
    printlog "delete the copy table"
    call fDeleteTable("biblio2")

	'/// close the database
    printlog "close the database"
	call fCloseDatabase()
    
endcase
'--------------------------------------------------------------------
testcase tCopy_Table_Dialog_Definition
    
    '/// open the bibliography database
    printlog "open the bibliography database"
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))

    '/// select the biblio table
    printlog "select the biblio table"
    call fFindtable("biblio")
    
    '/// press CTRL + C to copy the table and CTRL + V to paste the table again
    printlog "press CTRL + C to copy the table and CTRL + V to paste the table again"
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 C>",true)
        sleep(1)
        Database.typeKeys("<MOD1 V>",true)
        sleep(1)


    '/// the copy table dialog appear
    printlog "the copy table dialog appear"	
	Kontext "CopyDatabaseTable"
        if not CopyDatabaseTable.Exists(3) then
            warnlog "the Copy Table Autopilot doesn't appear"
            goto endsub
        endif
    '/// check definition
    printlog "check definition"
    Kontext "TabCopyDatabaseTable"
        Definition.Check

    '/// change the table name to biblio2
    printlog "change the table name to biblio2" 
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio2"
        DefinitionAndData.Check
    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"	
        NextBt.Click
	sleep(1)

    '/// add all columns
    printlog "add all columns"	
	Kontext "TabColumnSelect"
	    AddAll.Click
        sleep(1)

    '/// click next
    printlog "click next"	
	Kontext "CopyDatabaseTable"
	    NextBt.Click
        sleep(1)

    '/// click OK
    printlog "click OK"
    Kontext "CopyDatabaseTable"	
        CopyDatabaseTable.OK	
        sleep(1)

    'TODO: check if the table is copied correct

    '/// delete the copy table
    printlog "delete the copy table"    
    call fDeleteTable("biblio2")

    '/// close the database
    printlog "close the database"
	call fCloseDatabase()
    
endcase
'--------------------------------------------------------------------
testcase tCopy_Table_Dialog_Attach_data

    '/// open the bibliography database
    printlog "open the bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))

    '/// select the biblio table
    printlog "select the biblio table"    
    call fFindtable("biblio")
    
    '/// press CTRL + C to copy the table and CTRL + V to paste the table again
    printlog "press CTRL + C to copy the table and CTRL + V to paste the table again"
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 C>",true)
        sleep(1)
        Database.typeKeys("<MOD1 V>",true)
        sleep(1)

    '/// the copy table dialog appear
    printlog "the copy table dialog appear"	
	Kontext "CopyDatabaseTable"
        if not CopyDatabaseTable.Exists(3) then
            warnlog "the Copy Table Autopilot doesn't appear"
            goto endsub
        endif

    '/// check definition
    Kontext "TabCopyDatabaseTable"
        Definition.Check

    '/// change the table name to biblio3
    printlog "change the table name to biblio3" 
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio3"
        DefinitionAndData.Check
    
    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"	
        NextBt.Click
        sleep(1)

    '/// add all columns
    printlog "add all columns"	
	Kontext "TabColumnSelect"
	    AddAll.Click
        sleep(1)

    '/// click next
    printlog "click next"	
	Kontext "CopyDatabaseTable"
	    NextBt.Click
        sleep(1)

    '/// click OK
    printlog "click OK"	
	Kontext "CopyDatabaseTable"
        CopyDatabaseTable.OK	
        sleep(1)

    '/// select the biblio table
    printlog "select the biblio table"
    call fFindtable("biblio")

    '/// press CTRL + C to copy the table and CTRL + V to paste the table again
    printlog "press CTRL + C to copy the table and CTRL + V to paste the table again"
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 C>",true)
        sleep(1)
        Database.typeKeys("<MOD1 V>",true)
        sleep(1)

    '/// the copy table dialog appear
    printlog "the copy table dialog appear"	
	Kontext "CopyDatabaseTable"
        if not CopyDatabaseTable.Exists(3) then
            warnlog "the Copy Table Autopilot doesn't appear"
            goto endsub
        endif    	
    '/// change the table name to biblio3 and choose attach data
    printlog "change the table name to biblio3" 
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio3"
        AttachData.Check

    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"	
        NextBt.Click
        sleep(1)
    
    '/// click OK
    printlog "click OK"
    Kontext "CopyDatabaseTable"	
        CopyDatabaseTable.OK	
        sleep(1)

    'TODO: check if the table is copied correct

    '/// delete the copy table
    call fDeleteTable("biblio3")

    '/// close the database
    call fCloseDatabase()
    
endcase
'--------------------------------------------------------------------
testcase tCopy_Table_from_Text_Document

    '/// open a new document
    printlog "open a new document"
	call hNewDocument

    '/// insert a table and insert some data
    printlog "insert a table and insert some data"	
    InsertTableWriter
    Sleep 2
    Kontext "TabelleEinfuegenWriter"
        wait 500
        TabelleEinfuegenWriter.OK
        Sleep 1
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "field1" , true
        DocumentWriter.TypeKeys "<TAB>" , true
        DocumentWriter.TypeKeys "field2" , true
        DocumentWriter.TypeKeys "<TAB>" , true
        DocumentWriter.TypeKeys "field3" , true
        DocumentWriter.TypeKeys "<TAB>" , true
        DocumentWriter.TypeKeys "field4" , true
        DocumentWriter.TypeKeys "<TAB>" , true
        DocumentWriter.TypeKeys "field5" , true
        DocumentWriter.TypeKeys "<TAB>" , true
        DocumentWriter.TypeKeys "1" , true
        DocumentWriter.TypeKeys "<TAB>" , true
        DocumentWriter.TypeKeys "value1" , true
        DocumentWriter.TypeKeys "<TAB>" , true
        DocumentWriter.TypeKeys "value2" , true
        DocumentWriter.TypeKeys "<TAB>" , true
        DocumentWriter.TypeKeys "31.7.2003" , true
        DocumentWriter.TypeKeys "<TAB>" , true
		EditSelectAll
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true    
    sleep(2)
    
    '/// open database for past to
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))

    '/// select the biblio table    
    call fFindtable("biblio")

    '/// press CTRL + V to paste the table
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 V>",true)
        sleep(1)

    '/// the copy table dialog appear
    printlog "the copy table dialog appear"	
	Kontext "CopyDatabaseTable"
	if not CopyDatabaseTable.Exists(3) then
		warnlog "the Copy Table Autopilot doesn't appear"
		goto endsub
	endif
    '/// change the table name to biblio2
    printlog "change the table name to biblio2"
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio2"

    '/// select Definition and Data
    printlog "select Definition and Data"	
	DefinitionAndData.Check

    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"
        NextBt.Click
        sleep(1)

    '/// add all columns
    printlog "add all columns"	
	Kontext "TabColumnSelect"
        AddAll.Click
        sleep(1)

    '/// click next
    printlog "click next"	
	Kontext "CopyDatabaseTable"
        NextBt.Click
        sleep(1)

    '/// click OK
    printlog "click OK"
	CopyDatabaseTable.OK	
	sleep(1)	

    'TODO: check if the table is copied correct

    '/// delete the copy table
    printlog "delete the copy table"
    call fDeleteTable("biblio2")
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase

    '/// close the document
    printlog "close the document"
    call hCloseDocument

endcase
'--------------------------------------------------------------------
testcase tCopy_Table_from_Calc_Document

    '/// open a new calc document
    printlog "open a calc new document"
	gApplication = "CALC"
    call hNewDocument    

    '/// insert some data
    printlog "insert some data"
    Kontext "DocumentCalc"
        DocumentCalc.TypeKeys "Field1" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        DocumentCalc.TypeKeys "Field2" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        DocumentCalc.TypeKeys "Field3" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        DocumentCalc.TypeKeys "Field4" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        DocumentCalc.TypeKeys "Field5" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        DocumentCalc.TypeKeys "<RETURN>" , true
        DocumentCalc.TypeKeys "<HOME>" , true
        DocumentCalc.TypeKeys "1" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        DocumentCalc.TypeKeys "value1" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        DocumentCalc.TypeKeys "value2" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        DocumentCalc.TypeKeys "31.7.2003" , true
        DocumentCalc.TypeKeys "<TAB>" , true
        '/// select all and press CTRL + C to copy the data to the clipboard
        printlog "select all and press CTRL + C to copy the data to the clipboard"
		EditSelectALL
        sleep(1)        
        DocumentCalc.TypeKeys "<MOD1 C>" , true    
    sleep(2)

    '/// open database for paste to
    printlog "open database for paste to"
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")    
    call fOpendatabase(ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb"))    

    '/// open the tables container
    printlog "open the tables container"
    call fFindtable("biblio2")

    '/// press CTRL + V to paste the table again
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 V>",true)
        sleep(1)

    '/// the copy table dialog appear
    printlog "the copy table dialog appear"	
	Kontext "CopyDatabaseTable"
	if not CopyDatabaseTable.Exists(3) then
		warnlog "the Copy Table Autopilot doesn't appear"
		goto endsub
	endif
    '/// change the table name to biblio2
    printlog "change the table name to biblio2"	
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio2"

    '/// select Definition and Data
    printlog "select Definition and Data"	
	DefinitionAndData.Check
    
    '/// let the wizard create a primary key
    printlog "let the wizard create a primary key"
    CreatePrimaryKey.Check
    '/// check if the default text for the primary key is available
    printlog "check if the default text for the primary key is available"
    if ( PrimaryKeyName.getText = "" ) then
        warnlog "the automatic default text for the primary key doesn't work"        
    endif    

    '/// change the default text to ID111
    printlog "change the default text to ID111"
    PrimaryKeyName.setText("ID1111")

    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"
    	NextBt.Click
        sleep(1)

    '/// add all columns
    printlog "add all columns"	
	Kontext "TabColumnSelect"
        AddAll.Click
        sleep(1)

    '/// click next
    printlog "click next"	
	Kontext "CopyDatabaseTable"
	NextBt.Click
	sleep(1)

    '/// click OK
    printlog "click OK"
	CopyDatabaseTable.OK	
	sleep(1)

    '/// ceck if the table is copied correct
    printlog "ceck if the table is copied correct"
    call fOpenTableInDesign("biblio2")
    
    Kontext "TableDesignTable"
        dim s as String
        s = Fieldname.getText()
        printlog s
        if s <> "ID1111" then
            warnlog "the first field is wrong. It should ID1111 but it is " + s
        endif
        TableDesignTable.TypeKeys("<DOWN>")
        s = Fieldname.getText()
        printlog s
        if s <> "Field1" then
            warnlog "the second field is wrong. It should Field1 but it is " + s
        endif
        TableDesignTable.TypeKeys("<DOWN>")
        s = Fieldname.getText()
        printlog s
        if s <> "Field2" then
            warnlog "the third field is wrong. It should Field2 but it is " + s
        endif
        TableDesignTable.TypeKeys("<DOWN>")
        s = Fieldname.getText()
        printlog s
        if s <> "Field3" then
            warnlog "the 4. field is wrong. It should Field3 but it is " + s
        endif
        TableDesignTable.TypeKeys("<DOWN>")
        s = Fieldname.getText()
        printlog s
        if s <> "Field4" then
            warnlog "the 5. field is wrong. It should Field4 but it is " + s
        endif
        TableDesignTable.TypeKeys("<DOWN>")
        s = Fieldname.getText()
        printlog s
        if s <> "Field5" then
            warnlog "the 6. field is wrong. It should Field5 but it is " + s
        endif
        TableDesignTable.TypeKeys("<DOWN>")

    '/// close the table design
    printlog "close the table design"
    call fCloseTableDesign

    '/// delete the copy table
    printlog "delete the copy table"
    call fDeleteTable("biblio2")
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase

    '/// close the spreadsheet document
    printlog "close the spreadsheet document"    
    call hCloseDocument

    gApplication = "WRITER"

endcase
'--------------------------------------------------------------------
testcase tCopy_Table_from_hsql_to_dBase

    '/// copy the database file to a local path
    printlog "copy the database file to a local path"
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")

    '/// open a hsql database
    printlog "open a hsql database"
    call fOpendatabase(ConvertPath(gOfficePath + "user/work/TT_hsqldb.odb"))
    sleep(5)

    '/// select the tabel TT_CopyTest
    printlog "select the tabel TT_CopyTest"
    call fFindTable("TT_1")

    '/// press CTRL + C to copy the table
    printlog "press CTRL + C to copy the table"
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 C>",true)

    '/// open the bibliography database
    printlog "open the bibliography database"
    call fOpenDatabase(gOfficePath + "user/database/biblio.odb")

    call fDeleteTable("TT_1") ' delete the table TT_1 if the table exists.

    '/// select the biblio table
    printlog "select the biblio table"
    call fFindTable("biblio")
    
    '/// press CTRL + V to paste the table
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 V>",true)

    '/// the copy table dialog appear
    printlog "the copy table dialog appear"	
	Kontext "CopyDatabaseTable"
	if not CopyDatabaseTable.Exists(3) then
		warnlog "the Copy Table Autopilot doesn't appear"
		goto endsub
	endif

    '/// select Definition and Data
    printlog "select Definition and Data"
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText("TT_1")
        DefinitionAndData.Check

    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"
        NextBt.Click

    '/// add all columns
    printlog "add all columns"	
	Kontext "TabColumnSelect"
        AddAll.Click

    ' there is a messages box about missing compatible field type.    
    Kontext "MessageBox"
    if MessageBox.exists(3) then
        printlog "There is a messageBox. " + MessageBox.getText
        MessageBox.OK        
    endif
    
    '/// click next
    printlog "click next"	
	Kontext "CopyDatabaseTable"
	    NextBt.Click
	
    '/// click OK
    printlog "click OK"
	CopyDatabaseTable.OK	
	sleep(1)

    Kontext "MessageBox"
    if MessageBox.exists(3) then
        printlog "There is a messageBox. " + MessageBox.getText
        MessageBox.Yes        
    endif

    if MessageBox.exists(3) then
        printlog "There is a messageBox. " + MessageBox.getText
        MessageBox.OK        
    endif
    
    '/// delete the copied table
    printlog "delete the copied table"
    call fDeleteTable("TT_1")

    '/// close the database
    printlog "close the database"
    call fCloseDatabase()

    '/// close the other database
    printlog "close the other database"
    call fCloseDatabase()
    
endcase
'--------------------------------------------------------------------
testcase tCopy_Table_Paste_Special
    
    Dim sTableDimension as string
    Dim iRowCount as integer
    Dim iColumnCount as integer
	
    '/// open the bibliography database
    printlog "open the bibliography database"	
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))

    '///Make sure that there isn't a biblio2 table
    printlog "Make sure that there isn't a biblio2 table"
    If fFindtable("biblio2") = true then
       call fDeleteTable("biblio2")
    EndIf
    
    '/// select the biblio table
    printlog "select the biblio table"
    call fFindtable("biblio")

    '/// press CTRL + C to copy the table and CTRL + V to paste the table again
    printlog "press CTRL + C to copy the table and CTRL + V to paste the table again"
    Kontext "DATABASE"
        Database.typeKeys("<MOD1 C>",true)
        sleep(1) 
        Database.UseMenu
        hMenuSelectNr(2)
        hMenuSelectNr(5)                
        sleep(1)
        Kontext "InhaltEinfuegen"
            InhaltEinfuegen.OK
                
    '/// the copy table dialog appear
    printlog "the copy table dialog appear"	
	Kontext "CopyDatabaseTable"
        if not CopyDatabaseTable.Exists(3) then
            warnlog "the Copy Table Autopilot doesn't appear"
            goto endsub
        endif
    
    '/// change the table name to biblio
    printlog "change the table name to biblio" 
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio"        
        
    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"
	    NextBt.Click
    '/// there should be a message that the table name already exists.
    printlog "there should be a message that the table name already exists."	
	Kontext "MessageBox"
	    If not MessageBox.Exists(1) then
            warnlog "There have to be an error messages that the table name already exists"
        else
            MessageBox.OK
        endif
	
    '/// change the table name to biblio2
    printlog "change the table name to biblio2" 
	Kontext "TabCopyDatabaseTable"	
	    TableName.setText "biblio2"
        DefinitionAndData.Check
    
    '/// click next
    printlog "click next"
	Kontext "CopyDatabaseTable"        	
        NextBt.Click

	sleep(1)

    '/// add all columns
    printlog "add all columns"	
	Kontext "TabColumnSelect"
	    AddAll.Click
        sleep(1)

    '/// click next
    printlog "click next"	
	Kontext "CopyDatabaseTable"
	    NextBt.Click
        sleep(1)

    '/// click OK
    printlog "click OK"	
	Kontext "CopyDatabaseTable"
        CopyDatabaseTable.OK
        sleep(1)
        
    qaerrorlog "because of issue 76051 this copied table is not checked"
    '///Check if the table is copied correct
    'printlog "Check, if the table is copied correct"
    'sTableDimension = fCreateRefFile("Bibliography", "t" , "biblio2" , "#")
    'if (sTableDimension = "") then
    '    printlog "Problem while creating Reference File"
    'else
    '    'printlog sTableDimension       'Debugcode
    '    iColumnCount = CInt(Left(sTableDimension, instr(sTableDimension,",")))+1
    '    'printlog "Number of Columns: " & iColumnCount  'Debugcode
    '    iRowCount = CInt(Right(sTableDimension, (Len(sTableDimension) - instr(sTableDimension,","))))+1
    '    'printlog "Number of Rows: " & iRowCount    'Debugcode
    '    call tCompareMatrixValues("Bibliography", "user/work/biblio2.ttr", "t", "biblio", iRowCount, iColumnCount, "#")
    'endif        
       

    '/// delete the copy table
    printlog "delete the copy table"
    call fDeleteTable("biblio2")

	'/// close the database
    printlog "close the database"
	call fCloseDatabase()
    
endcase 

