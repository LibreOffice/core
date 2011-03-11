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
'* short description : Helper Routines for Base tests.
'*
'***************************************************************************************
'*
' #1 fFindQuery
' #1 fOpenNewQueryDesign
' #1 fStartQueryWizard
' #1 fCloseQueryDesign
'*
'\***********************************************************************************
'-------------------------------------------------------------------------
function fFindQueryInBeamer(sDSName1,sQueryName1)
    '/// select a query with the given name in the beamer 
    '/// <u>parameter:</u>
    '/// <b>sDSName1:</b> the name of the datasource
    '/// <b>sQueryName1:</b> the name of the query
    
	dim i as integer
	dim bfindQuery as boolean
	dim iNoDS as integer
    dim iNoQuery as integer
    
    bfindQuery = false
	Kontext "DatabaseBeamer"
    Kontext "DatabaseSelection"
		
    iNoDS = DatabaseSelection.getItemCount	
	for i = 1 to iNoDS
		DatabaseSelection.Select i
		if DatabaseSelection.getText = sDSName1 then
			i = iNoDS
		endif
	next i
	
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
	
	
    iNoQuery = DatabaseSelection.getItemCount
    dim ii as integer
    ii = DatabaseSelection.GetSelIndex	
	for i = ii to iNoQuery
		DatabaseSelection.Select i
		if DatabaseSelection.getText = sQueryName1 then
			i = iNoQuery
		    sleep 1			
		    bfindQuery = true
		endif
	next i
	
    fFindQuery = bfindQuery
	
end function
'--------------------------------------------------------------------
function fOpenNewQueryDesign() as boolean
    '/// open a new query design 
    '/// <u>parameter:</u>    
    
    Kontext "DATABASE"    
        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    if ( Database.NotExists(3) ) then
        fOpenNewQueryDesign = false
        warnlog "The database windows doesn't exists"
        exit function
    end if
    
    sleep(1)
    
    printlog "open new query design"
    
    NewQueryDesign
    
    fOpenNewQueryDesign = true
    
end function
'--------------------------------------------------------------------
function fOpenNewSQLQueryDesign() as boolean
    '/// open a new query design in SQL mode 
    '/// <u>parameter:</u>    
    
    Kontext "DATABASE"    
        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    if ( Database.NotExists(3) ) then
        fOpenNewSQLQueryDesign = false
        warnlog "The database windows doesn't exists"
        exit function
    end if
    
    sleep(1)
    
    printlog "open new sql query design"
    
    NewSQLQueryDesign
    
    fOpenNewSQLQueryDesign = true
    
end function
'-------------------------------------------------------------------------
function fStartQueryWizard()
    '/// start the query wizard 
    '/// <u>parameter:</u>    

    Kontext "DATABASE"    
	    if ( Database.NotExists(3) ) then
	        fStartQueryWizard = false
	        exit function
	    end if

        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    sleep(1)
    
    ViewQueries
    StartQueryWizard
    
    sleep(2)
    
    fStartQueryWizard = true
    
end function
'-------------------------------------------------------------------------
function fCloseQueryDesign(optional bSave)
    '/// close the query design
    '/// <u>parameter:</u>
    '/// <b><i>optional</i>bSave:</b> if true the query is saved if false then the changes are lost

    sleep(1)

    Kontext "QueryDesignTable"
        QueryDesignTable.UseMenu
        ' bug file / close close the whole database
        'hMenuSelectNr(1) ' the file menu
        'hMenuSelectNr(4) ' the Close Window
        
        hMenuSelectNr(6) ' the window menu
        hMenuSelectNr(1) ' the Close Window
    
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
        
    sleep(1)
        
end function
'-------------------------------------------------------------------------
function fOpenQueryInSQLDesign(sQueryName as String)
    '/// open a query in sql design
    '/// <u>parameter:</u>
    '/// <b>bQueryName:</b> the name of the query which should be open

    if ( fFindQuery(sQueryName) = true ) then
        printlog "Query " + sQueryName + " found -> open in sql design"    
        
        Kontext "DATABASE"
        Database.UseMenu        
        
        hMenuSelectNr(2)
        hMenuSelectNr(9)
        
        sleep(1)
        fOpenQueryInSQLDesign= true
    else
        fOpenQueryInSQLDesign = false
    end if
    
    sleep(1)

end function
'-------------------------------------------------------------------------
function fCloseSQLQueryDesign(optional bSave)
    '/// close the query design
    '/// <u>parameter:</u>
    '/// <b><i>optional</i>bSave:</b> if true the query is saved if false then the changes are lost

    sleep(1)

    Kontext "QueryEditWindow"
        QueryEditWindow.UseMenu
        ' bug file / close close the whole database
        'hMenuSelectNr(1) ' the file menu
        'hMenuSelectNr(4) ' the Close Window
        
        hMenuSelectNr(6) ' the window menu
        hMenuSelectNr(1) ' the Close Window
    
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
        
    sleep(1)
        
end function
'-------------------------------------------------------------------------
function fSaveQueryDesign(sName as String)
    '/// save an open Query Design 
    '/// <u>parameter:</u> the name of query  
    
    sleep(1)
    Kontext "QueryDesignTable"
        DatabaseSave
                
    Kontext "DatabaseTableSaveAs"
        TableName.setText(sName)
        DatabaseTableSaveAs.OK
        
    sleep(1)
    
    fSaveQueryDesign = true

end function
'-------------------------------------------------------------------------
function fSaveSQLQueryDesign(sName as String)
    '/// save an open SQL Query Design 
    '/// <u>parameter:</u>  the name of query   
    
    sleep(1)

    Kontext "QueryEditWindow"
        DatabaseSaveAs
                
    Kontext "DatabaseTableSaveAs"
        TableName.setText(sName)
        DatabaseTableSaveAs.OK
        
    sleep(1)
    
    fSaveSQLQueryDesign = true

end function

'-------------------------------------------------------------------------
function fChooseTableInAddTableDialog(sTableName as string) as boolean
    Kontext "AddTables"
        if ( not AddTables.exists(1) ) then
            fChooseTableInQueryAddTableDialog = false    
            exit function            
        endif
        
    'TODO: add a test to click on the execute button, before the add table dialog is closed.
        dim ix as integer 
        dim i as integer
        ix = TableName.getItemCount                
        for i = 1 to ix
            TableName.Select i
            TableName.TypeKeys "<ADD>"
            'printlog "ix = " + ix
            'printlog "TableName.getItemCount = " + TableName.getItemCount 
            if TableName.getItemCount > ix then
                ix = TableName.getItemCount
                'printlog "TableName.getItemCount > ix"
            endif
            'printlog "TableName.getSeltext = " + TableName.getSeltext
            if TableName.getSeltext = sTableName then
                i = ix
            endif
        next
        if TableName.getSeltext <> sTableName then
            warnlog "Can find the table " + sTableName + "." + _
                    "Make sure that the table exists when starting this test."
            DatabaseClose                        
            sleep(5)
            Kontext "Messagebox"
                if Messagebox.Exists(3) then
                    if Messagebox.getRT = 304 then
                        Messagebox.No                        
                    end if                        
                end if                
            call hCloseDocument
            exit function
        endif
        printlog "- Add " + sTableName +" table to query design"
        '/// Add database table to Query.
        AddTable.Click
        sleep(2) 
        if AddTables.exists(2) then 
            CloseDlg.Click ' When the AddTables dialog still exists then close him
        endif
        sleep(1)
        fChooseTableInAddTableDialog = true
        
end function 
'--------------------------------------------------------------------
function fOpenQuery(sQueryName as string)    
    '/// open the query with the given name
    '/// <u>parameter:</u>
    '/// <b>squeryName:</b> the query which shall be opened    
    if ( fFindQuery(sQueryName) = true ) then
        printlog "Query " + sQueryName + " found -> open"    
        Kontext "ContainerView"
            OpenTable ' uno-Slot .uno:DB/Open
            sleep(1)
        fOpenQuery = true
    else
        fOpenQuery = false
    end if
    
end function
'--------------------------------------------------------------------
function fFindQuery(sQueryName as string) 
	'/// select the Query with the given name in the Query container
    '/// <u>parameter:</u>
    '/// <b>sQueryName:</b> the Query which shall be selected
            
    Dim iNumbersOfQuerys as integer
	Dim i as integer
	
    Kontext "ContainerView"
    
        ViewQueries        
    
        fFindQuery = false
    
        if ( Not QueriesTree.exists(1) ) then
            qaerrorlog "The Query tree doesn't exists"
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
        
        iNumbersOfQuerys = QueriesTree.getItemCount()

        ' this select the first entry
        QueriesTree.TypeKeys "<HOME>"
        QueriesTree.TypeKeys "<UP>"
        
        
        for i = 1 to iNumbersOfQuerys
            
            QueriesTree.TypeKeys "<ADD>"
            'printlog "i = " + i
            'printlog "QueryName.getItemCount = " + QueriesTree.getItemCount 
            if QueriesTree.getItemCount >  iNumbersOfQuerys then
                iNumbersOfQuerys = QueriesTree.getItemCount()                    
            endif
            'printlog "QueryName.getSeltext = " + QueriesTree.getSeltext
            if QueriesTree.getSeltext = sQueryName then
                fFindQuery = true    
                exit for
            endif
            QueriesTree.TypeKeys "<DOWN>"
        next
        sleep(1)        
    
end function
'-------------------------------------------------------------------------
function fCloseQueryView() 
    '/// close an open Query view
    '/// <u>parameter:</u>

    sleep(1)

    Kontext "TableView"
        TableView.UseMenu
        ' bug file / close close the whole database
        'hMenuSelectNr(1) ' the file menu
        'hMenuSelectNr(4) ' the Close Window
        
        hMenuSelectNr(5) ' the window menu
        hMenuSelectNr(1) ' the Close Window        
    ' if messages box appear because of unsaved record click no in the dialog
    Kontext "Messagebox"
        if Messagebox.Exists(3) then
            Messagebox.No
        end if
        
    fCloseQueryView = true
end function
