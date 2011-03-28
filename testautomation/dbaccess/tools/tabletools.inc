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
'* short description : Tools for Tables
'*
'************************************************************************
'*
' #1 fCloseRelationDesign
' #1 fCloseTableDesign
' #1 fCloseTableView
' #1 fCompareMatrixValues
' #1 fCreateRefFile 
' #1 fCreateTable
' #1 fDeleteTable
' #1 fFindTable
' #1 fInsertIntoTable
' #1 fOpenRelationDesign
' #1 fOpenNew TableDesign
' #1 fOpenTable
' #1 fReadFile
'*
'\***********************************************************************
'------------------------------------------------------------------------------
function tCompareMatrixValues(sDSName as string, sRefFileUrl as string, cTable_or_Query as string, sTableName as string, iRowPos as integer, iColPos as integer, sWordSeperator as string) as integer
    'sDSName = data source name f.e. Bibliography 
    'sRefFileUrl = URL to your reference file (os indipendent - extension .ttr) f.e. base\optional\input\reference_tables\test.ttr
    'cTable_or_Query = differ between comparing reference file with a table (t) or a query-table (q)
    'sTableName = name of table or query-table to compare with f.e. Table1
    'iRowPos,iColPos = row and column amount of reference and comparing table (must have the same size ; begin with 1 ; 0 is leftout, but has to be added)

    dim i as integer 
    dim j as integer    'counter'
    dim aMatrix(iRowPos, iColPos) as string
    
    printlog ""
    if cTable_or_Query = "t" then printlog "--- Compare Table with Reference Table ---"
    if cTable_or_Query = "q" then printlog "--- Compare Query with Reference Table ---"
    
    call fReadFile(sRefFileUrl, aMatrix(), sWordSeperator)
    
	call hNewDocument
    
	'/// open Beamer and open datasource
	ViewCurrentDatabase
	sleep(1)
    printlog "open database beamer"
    
    '/// switch between table and querie search
    dim bfind_ok as boolean         'placeholder
    if cTable_or_Query = "t" then
        bfind_ok = ffindTableInBeamer(sDSName,sTableName)
        printlog "found proper Table: " & sTableName
    endif
    if cTable_or_Query = "q" then
        bfind_ok = ffindQuery(sDSName, sTableName)
        printlog "found proper Query: " & sTableName
    endif
    
    if bfind_ok then
        '/// Choosing for the right table
        sleep(1)
	Kontext "DatabaseSelection"
		DatabaseSelection.TypeKeys "<MOD1 SHIFT E>",TRUE ' # type CTRL + SHIFT + E to go from the DatabaseSelection to the TableView'
        sleep(1)
        printlog "found and opened proper table"
        '/// comparing data in table with reference table
        printlog "start comparing data in table with reference table"
	Kontext "TableView"  
        dim ivalue_ok as integer
        ivalue_ok = 0
        TableView.TypeKeys "<TAB>",TRUE
        TableView.TypeKeys "<HOME>",TRUE
        TableView.TypeKeys "<HOME>",TRUE
		for i = 1 to iRowPos -1
			for j = 1 to iColPos -1
            setclipboard("")
			'here the values are read out and put in clipboard for comparing
                TableView.TypeKeys "<MOD1 C>",TRUE
                TableView.TypeKeys "<TAB>",TRUE
                'printlog "i: " & i
                'printlog "j: " & j
                'printlog "getClipboard: " & getclipboard()
				'printlog "matrix: " & aMatrix(i,j) 
                
                'here the values are compared
                if getclipboard() <> aMatrix(i,j) then
                    ivalue_ok = ivalue_ok + 1
                    warnlog ivalue_ok & ". fault in pos(row/col): " & i & "/" & j & " - should be: " & aMatrix(i,j) & " but is: " & getclipboard() 
                endif
                'printlog ivalue_ok & ". No !!! foult but TEST - should be: " & aMatrix(i,j) & " but is: " & getclipboard()
			next j
        next i
        
        '/// Result part
        if ivalue_ok = 0 then
            printlog "** Comparing procedure without faults"
            tCompareMatrixValues = ivalue_ok
        else
            warnlog "Comparing procedure went wrong: " + ivalue_ok + " differences found!"
            tCompareMatrixValues = 1
        endif
	else
        if cTable_or_Query = "t" then warnlog "Table: " + sTableName + " not found - Test aborted"
        if cTable_or_Query = "q" then warnlog "Query: " + sTableName + " not found - Test aborted"
	endif
    printlog "*** End Testcase"
    sleep 1	
    ViewCurrentDatabase
	hCloseDocument

end function
'------------------------------------------------------------------------------
function fCreateRefFile(sDSName as string, cTable_or_Query as string, sTableName as string, sWordSeperator as string) as string
    dim bfind_ok as boolean                 'check if right table found
    dim iNumber as Integer                  'needed for open reference file
    dim iColAmount as integer
    dim iRowAmount as integer   'counter
    dim sReadContCol as string
    dim sContColMem as string 'read out content string and its memo for getting table column ammount
    dim sCellValue as string
    dim sCellValueMem as string 'read out content string and its memo for writing in reference table
    
    dim sRefTableName as string
    sRefTableName = ConvertPath("user\work\" & sTableName & ".ttr")
    dim sRefFileUrl as string
    sRefFileUrl = gOfficePath & sRefTableName
    printlog "path for reference table: " & sRefFileUrl

    
    call hNewDocument
    
    '/// open Beamer and open datasource
    ViewCurrentDatabase
    sleep(1)
    printlog "open database beamer"
    
    '/// switch between table and querie search
    if cTable_or_Query = "t" then
        printlog "search for proper Table: " & sTableName
        bfind_ok = ffindTableInBeamer(sDSName,sTableName)
    endif
    if cTable_or_Query = "q" then
        printlog "search for proper Query: " & sTableName
        bfind_ok = ffindQuery(sDSName, sTableName)
    endif
    
    if bfind_ok then
        '/// jumping to the right table
        sleep(1)
	Kontext "DatabaseSelection"
		DatabaseSelection.TypeKeys "<MOD1 SHIFT E>",TRUE ' # type CTRL + SHIFT + E to go from the DatabaseSelection to the TableView'
        sleep(1)
        
        '/// check the row and column amount
        printlog "check the row and column amount"
	Kontext "TableView"  
        sReadContCol = 1   'init
        sContColMem = 1    'init
        iColAmount = 0     'init
        iRowAmount = AllRecords.caption()
        while sReadContCol = sContColMem
            sContColMem = sReadContCol
            sReadContCol = CurrentRecord.getText()
            TableView.TypeKeys "<TAB>",TRUE
            TableView.TypeKeys "<TAB>",TRUE
            iColAmount = iColAmount +1
        wend
        iColAmount = iColAmount -1
        'printlog "sReadContCol: " & sReadContCol
        printlog "size of table (columns/rows): " & iColAmount & "/" & iRowAmount
        fCreateRefFile = iColAmount & "," & iRowAmount
        TableView.TypeKeys "<UP>",TRUE
        TableView.TypeKeys "<HOME>",TRUE
        TableView.TypeKeys "<HOME>",TRUE
        sleep(1)

        '/// reading out values from table, create a reference table and input values in
        printlog "start reading out values from table and writing in ref table"
        iNumber = Freefile
        dim i as integer    	        'counter
        dim j as integer    	        'counter
        Open sRefFileUrl for Output as iNumber
            for i = 1 to iRowAmount
                for j = 1 to iColAmount
                    Kontext "TableView"
                    TableView.TypeKeys "<MOD1 C>"
                    sCellValue = getclipboard()
                    TableView.TypeKeys "<TAB>"
                    if j <> 1 then 
                        sCellValueMem = sCellValueMem & sWordSeperator
                    endif
                    sCellValueMem = sCellValueMem & sCellValue
                    'printlog i & j & " inhalt: " & sCellValue
                    setclipboard("")
                next j
            print #iNumber, sCellValueMem
            sCellValueMem = ""
            next i
        Close #iNumber
        sleep(1)
	
        '/// warning if table not found - without proper table the test make no sense
    else
        warnlog "Table: " + sTableName + " not found."
        fCreateRefFile = ""        
    endif
    
    sleep 1	
    ViewCurrentDatabase
	hCloseDocument

end function
'------------------------------------------------------------------------------
function fReadFile(sRefFileUrl as string, aMatrix() as string, sWordSep as string)
    'connecting and opening file
     dim iNumber as Integer
     iNumber = Freefile
     'position and content variables
     dim sColumn As String
     dim iRowPos as integer
     dim iColPos as integer
     dim iPos as integer
     dim iPos1 as integer            'pointer for searching position
     
     'init position in matrix
     iRowPos = 1
     iColPos = 1
     
     '/// open reference file and store it into an array (matrix)
     printlog "search for the reference file, open it and store content in an array"
     sRefFileUrl = gOfficePath + ConvertPath(sRefFileUrl)
     Open sRefFileUrl for Input as iNumber
     while not eof(iNumber)
     	Line Input #iNumber, sColumn
        iPos1 = 1                               'startposition
        iPos = 1                                'startposition
        while iPos <> 0                         'if iPos = 0 then no more sWordSeperator found -> end of column reached
            iPos = Instr(iPos +1,sColumn,sWordSep)   'check column for sWordSeperator
            'printlog "iPos: " & iPos
            'printlog "iPos1: " & iPos1
            if iPos <> 1 then                   '<> 1 -> no content found
                aMatrix(iRowPos,iColPos) = Mid(sColumn,iPos1,iPos - iPos1)  'cut content and put into array
                'printlog "iRow: " & iRowPos
                'printlog "iCol: " & iColPos
                'printlog "aMatrix: " & aMatrix(iRowPos,iColPos)
                iPos1 = iPos +1
                iColPos = iColPos +1
            else warnlog "reference file is empty"
            endif
        wend    
            iRowPos = iRowPos +1
            iColPos = 1           
     wend
     printlog "**end of reading out reference table"
end function
'--------------------------------------------------------------------
function fCreateTable(aFieldTypeContent(),sTableName,optional sCatalog,optional sSchema)
    '/// create a table with the given FieldTypes
    '/// sSchema and sCatalog are optional.
    '/// If the optional parameter is not given then it's set to ""
    '/// <u>parameter:</u>
    '/// <b>aFieldTypeContent:</b> an arry with the table field data
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the array have to look like the following
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; array(1,1) = first_field_name
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; array(1,2) = first_field_type
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; array(2,1) = second_field_name
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; array(2,2) = second_field_type
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ...
    '/// <b>sTableName:</b> the name of the table
    '/// <b><i>optional</i> sCatalog:</b> the name of the table catalog
    '/// <b><i>optional</i> sSchema:</b> the name of the table schema
	
    Dim i as integer			        'counter
	Dim iFieldNumber as integer			'counter
	dim iNoDS as integer                'number of data source in listbox
	Dim iNumberOfFieldTypes as integer          'field type memory
	Dim iFieldTypesCounter as integer   'counter
	Dim iIndex as integer               'counter
	Dim sFieldType as string            'help var for fieldtypes
    Dim sTypeName as String             'help var for fieldtypes
    
    if ( IsMissing(sCatalog) ) then
        sCatalog = ""
    endif
    
    if ( IsMissing(sSchema) ) then
        sSchema = ""
    endif
    
    call fDeleteTable(sTableName) ' delete the table if exists
    
    call fOpenNewTableDesign    
    
    Kontext "TableDesignTable"
    
    for iFieldNumber = 1 to ubound(aFieldTypeContent) - 1

        printlog "create field : " + aFieldTypeContent(iFieldNumber,1)
        Fieldname.TypeKeys aFieldTypeContent(iFieldNumber,1) , TRUE    
        Fieldname.TypeKeys "<RETURN>" , TRUE
        sleep 1
        sTypeName = "[ " + aFieldTypeContent(iFieldNumber,2) + " ]"
        iNumberOfFieldTypes = FieldType.getitemCount()
        for iFieldTypesCounter = 1 to iNumberOfFieldTypes                 
            sFieldType = FieldType.getitemText(iFieldTypesCounter)            
            iIndex = Instr(sFieldType,sTypeName)                
            if iIndex <> 0 then
                printlog "FieldType : " + sFieldType
                iFieldTypesCounter = iNumberOfFieldTypes ' stops the for loop if proper fieldtype found'
            endif
        next
        '/// choose proper field type
        FieldType.Select(sFieldType)    'listbox entry nr'
        sleep 1
        FieldType.TypeKeys "<RETURN>" , TRUE
        Description.TypeKeys "<RETURN>" , TRUE
        if( CellDescription.isVisible() ) then
            CellDescription.TypeKeys "<RETURN>" , TRUE
        endif
        printlog "-------------------------------"
    next 
    sleep(1)    
    Kontext "TableDesignTable"         
        TableDesignTable.usemenu
        MenuSelect MenuGetItemId (1)
        sleep(1)
        menuselect MenuGetItemId (7)
        sleep(1)	
    Kontext "DatabaseTableSaveAs"
        printlog "save table as "+ sTableName
        TableName.setText sTableName
        sleep(1)
        '/// catalog handling
        if sCatalog <> "" then
            if Catalog.IsVisible then
                Catalog.SetText sCatalog
                printlog "inserting catalog name: " + sCatalog
            else 
                warnlog "The Catalog-Name could not be inserted"
            endif
        endif
        '/// schema handling
        if sSchema <> "" then
            if Schema.IsVisible then
                Schema.SetText sSchema
                printlog "inserting schema name: " + sSchema
            else 
                warnlog "The Schema-Name could not be inserted"
            endif
        endif
        DatabaseTableSaveAs.OK
        sleep 1
        Kontext "Messagebox"
        if Messagebox.Exists(1) then
            Messagebox.Yes  
            printlog "create a primary key "
        end if
        sleep 1	
        
        ' a sql exception appear. Try to get the error text
        Kontext "Messagebox"
        if Messagebox.Exists(1) then            
          	MessageBox.Click 5 ' click the more button
			Kontext "SQLException"
			if SQLException.exists() then
		        warnlog Errortext.getText()
                SQlException.OK
			end if
            Kontext "MessageBox"
            MessageBox.OK
        end if
    	
        call fCloseTableDesign
        
end function
'--------------------------------------------------------------------
function fDeleteTable(sTableName as string)  
    '/// delete the table with the given name
    '/// <u>parameter:</u>
    '/// <b>sTableName:</b> the table which shall be deleted
    if ( fFindTable(sTableName) = true ) then
        printlog "Table found press delete"    
        Kontext "ContainerView"        
            'TableTree.TypeKeys "<DELETE>" , true
            Delete ' uno-Slot .uno:DB/Delete
            sleep(1)
        Kontext "Active"
            Active.Yes
        fDeleteTable = true
    else
        fDeleteTable = false
    end if
    
end function
'--------------------------------------------------------------------
function fOpenTable(sTableName as string)    
    '/// open the table with the given name
    '/// <u>parameter:</u>
    '/// <b>sTableName:</b> the table which shall be opened    
    if ( fFindTable(sTableName) = true ) then
        printlog "Table found -> open"    
        Kontext "ContainerView"
            OpenTable ' uno-Slot .uno:DB/Open
            sleep(1)
        fOpenTable = true
    else
        fOpenTable = false
    end if
    
end function
'--------------------------------------------------------------------
function fFindTable(sTableName as string) 
	'/// select the table with the given name in the table container
    '/// <u>parameter:</u>
    '/// <b>sTableName:</b> the table which shall be selected
            
    Dim iNumbersOfTables as integer
	Dim i as integer
	
    Kontext "ContainerView"
    
        ViewTables        
    
        fFindTable = false
    
        if ( Not TableTree.exists(1) ) then
            qaerrorlog "The table tree doesn't exists"
            ' May be a messagebox appear click OK to close it
            Kontext "MessageBox"
                if MessageBox.exists(1) then
                    qaerrorlog MessageBox.getText()
                    while MessageBox.exists() ' sometimes there are more then 1 message boxe
                        MessageBox.OK
                    wend
                endif            
            exit function
        end if
        
        iNumbersOfTables = TableTree.getItemCount()

        ' this select the first entry
        TableTree.TypeKeys "<HOME>"
        TableTree.TypeKeys "<UP>"
        
        
        for i = 1 to iNumbersOfTables
            
            TableTree.TypeKeys "<ADD>"
            'printlog "i = " + i
            'printlog "TableName.getItemCount = " + TableTree.getItemCount 
            if TableTree.getItemCount >  iNumbersOfTables then
                iNumbersOfTables = TableTree.getItemCount()                    
            endif
            'printlog "TableName.getSeltext = " + TableTree.getSeltext
            if TableTree.getSeltext = sTableName then
                fFindTable = true    
                exit for
            endif
            TableTree.TypeKeys "<DOWN>"
        next
        sleep(1)        
    
end function
'--------------------------------------------------------------------
function fInsertIntoTable( aFieldContent(), sTableName)
    '/// insert data into a table
    '/// <u>parameter:</u>
    '/// <b>aFieldContent:</b> an arry with the table data
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; the array have to look like the following
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; array(1,1) = first_value_for_first_record
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; array(1,2) = second_value_for_first_record
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; array(2,1) = first_value_for_second_record
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; array(2,2) = second_value_for_second_record
    '/// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ...
    '/// <b>sTableName:</b> the name of the table
	
    Dim iCounterOfRecords as integer    
    Dim iCounterOfFields as integer
    
    Dim iNumberOfRecords as integer    
    Dim iNumberOfFields as integer
	
    iNumberOfRecords = ubound(aFieldContent)
    iNumberOfFields = ubound(aFieldContent,2)
    
	call fOpenTable(sTableName)
	
	Kontext "TableView"
		for iCounterOfRecords = 1 to iNumberOfRecords
            for iCounterOfFields = 1 to iNumberOfFields
            	TableView.TypeKeys aFieldContent(iCounterOfRecords,iCounterOfFields),TRUE
                TableView.TypeKeys "<TAB>",TRUE
            next
        next
	
    
    sleep 1	
    
	call fCloseTableView

end function
'-------------------------------------------------------------------------
function fCloseTableDesign(optional bSave) 
    '/// close an open table design
    '/// <u>parameter:</u>    
    '/// <b><i>optional</i> bSave:</b> if true then the table design is saved

    sleep(1)

    Kontext "TableDesignTable"
        TableDesignTable.UseMenu
        ' bug file / close close the whole database
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(4) ' the Close Window
        
        'hMenuSelectNr(6) ' the window menu
        'hMenuSelectNr(1) ' the Close Window        
                
    Kontext "Messagebox"
        if Messagebox.Exists(3) then
            Messagebox.No
        end if
        
    fCloseTableDesign = true
end function
'-------------------------------------------------------------------------
function fCloseTableView() 
    '/// close an open table view
    '/// <u>parameter:</u>

    sleep(1)

    Kontext "TableView"
        TableView.UseMenu
        ' bug file / close close the whole database
        'hMenuSelectNr(1) ' the file menu
        'hMenuSelectNr(4) ' the Close Window
        
        hMenuSelectNr(6) ' the window menu
        hMenuSelectNr(1) ' the Close Window        
    ' if messages box appear because of unsaved record click no in the dialog
    Kontext "Messagebox"
        if Messagebox.Exists(3) then
            Messagebox.No
        end if
        
    fCloseTableView = true
end function
'-------------------------------------------------------------------------
function fOpenRelationDesign()
    '/// open a new relation design
    '/// <u>parameter:</u>    
    
    Kontext "DATABASE"    
	    if ( Database.NotExists(3) ) then
	        fOpenRelationDesign = false
	        exit function
	    end if

        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    sleep(1)
    
    Database.UseMenu
    hMenuSelectNr(5)
    hMenuSelectNr(1)
    
    fOpenRelationDesign = true
end function
'-------------------------------------------------------------------------
function fCloseRelationDesign(optional bSave)    
    '/// close an open relation design
    '/// <u>parameter:</u>    
    '/// <b><i>optional</i> bSave:</b> if true then the relation design is saved


    Kontext "RelationDesign"
        RelationDesign.UseMenu
        ' bug file / close close the whole database
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(4) ' the Close Window
        
        'hMenuSelectNr(6) ' the window menu
        'hMenuSelectNr(1) ' the Close Window        
                
    Kontext "Messagebox"
        if Messagebox.Exists(3) then
            if ( IsMissing( bSave ) ) then
                Messagebox.No
            else
                if bSave then
                    Messagebox.Yes
                else
                    Messagebox.No
                endif
            endif
        end if
        
    fCloseRelationDesign = true
end function
'-------------------------------------------------------------------------
function fOpenNewTableDesign
    printlog "fOpenNewTableDesign called"

    printlog "check if Database exists"
    
    Kontext "DATABASE"    
        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    if ( Database.NotExists(3) ) then
        fOpenNewTableDesign = false
        warnlog "The Database windows doesn't exists"
        exit function
    end if
    
    sleep(1)
    
    printlog "open new table design"
    
    NewTableDesign

    sleep(2)    

    fOpenNewTableDesign = true    

end function
'-------------------------------------------------------------------------
function fOpenTableInDesign(sTableName as String)
    printlog "fOpenTableInDesign called"

    printlog "check if Database exists"
    
    Kontext "DATABASE"    
        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    if ( Database.NotExists(3) ) then
        fOpenTableInDesign = false
        warnlog "The Database windows doesn't exists"
        exit function
    end if
    
    sleep(1)
    
    printlog "open new table design"

    call fFindTable(sTableName)

    EditTable

    Kontext "TableDesignTable"
    if ( not TableDesignTable.exists(3) ) then
        warnlog "The Table design doesn't open"
        fOpenTableInDesign = false
        exit function
    end if
    
    fOpenTableInDesign = true    

end function
'-------------------------------------------------------------------------
function fFindTableInBeamer(sDSName1,sTableName1)
    '/// select a table with the given name in the beamer 
    '/// <u>parameter:</u>
    '/// <b>sDSName1:</b> the name of the datasource
    '/// <b>sTableName1:</b> the name of the table
    
	dim i as integer
	dim bfindTable as boolean
	dim iNoDS as integer
    dim iNoTable as integer
    
    bfindTable = false
	Kontext "DatabaseBeamer"
    Kontext "DatabaseSelection"
		
    iNoDS = DatabaseSelection.getItemCount	
	for i = 1 to iNoDS
		DatabaseSelection.Select i
		if DatabaseSelection.getText = sDSName1 then
			i = iNoDS
            bfindTable = true
		endif
	next i
	
    if (bfindTable = false) then
        warnlog "Datasource " + sDSName1 + " not found!"
        exit function
    else
        printlog "Datasource " + sDSName1 + " found!"
        ' I set the bfindTable flag again back to false
        ' for the next test of the table
        bfindTable = false
    endif
    
    wait 500
	DatabaseSelection.TypeKeys "<ADD>" , true
    wait 500
	DatabaseSelection.TypeKeys "<DOWN>" , true
    wait 500
	DatabaseSelection.TypeKeys "<DOWN>" , true
    wait 500
	DatabaseSelection.TypeKeys "<ADD>" , true
    wait 500
	DatabaseSelection.TypeKeys "<DOWN>" , true
    wait 500
	
	
    iNoTable = DatabaseSelection.getItemCount
    dim ii as integer
    ii = DatabaseSelection.GetSelIndex	
	for i = ii to iNoTable
		DatabaseSelection.Select i
		if DatabaseSelection.getText = sTableName1 then
			i = iNoTable
		    sleep 1			
		    bfindTable = true
		endif
	next i
	
    fFindTableInBeamer = bfindTable
	
end function
'-------------------------------------------------------------------------
function fStartTableWizard()
    '/// start the table wizard 
    '/// <u>parameter:</u>    

    Kontext "DATABASE"    
	    if ( Database.NotExists(3) ) then
	        fStartTableWizard = false
	        exit function
	    end if

        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    sleep(1)
    
    ViewTables
    StartTableWizard
    
    sleep(2)
    
    fStartTableWizard = true
    
end function
