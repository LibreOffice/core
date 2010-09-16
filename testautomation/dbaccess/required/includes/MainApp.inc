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
'* short description : test the Main Application Window
'*
'\******************************************************************************

sub MainApp

    printlog "------------------ MainApp.inc ---------------------"
    
    call tMainApp
    call tMainMenuBar
    call tDatabaseProperties    
    call tTableView
    call tPreview    
    call tTableFilters
    
    call tRegisterDatabaseDialog
    call tDatabaseBeamer
    
end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tTableFilters
    
    hFileOpen( gOfficePath & "user/database/biblio.odb" )
        
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)
    TableFilters
    sleep(1)
    Kontext "TablesFilter"
    
    TablesFilter.Cancel
    sleep 1
    
    call hCloseDocument
    
    
endcase
'-------------------------------------------------------------------------
testcase tTableView
   
    hFileOpen( gOfficePath & "user/database/biblio.odb" )
        
    Kontext "DATABASE"

    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
        
    if not DatabaseSwapIconView.exists(1) then        
        warnlog "InsightSwapIconView doesn't exists"
        goto endsub
    end if
        
    '/// display Tables View
    printlog "display Tables View"
    ViewTables
    Kontext "ContainerView"
    if not TableTree.exists(1) then        
        warnlog "TableTree doesn't appear"
        goto endsub
    end if
    
    TableTree.TypeKeys("<HOME>")
    TableTree.TypeKeys("<UP>")
    
    sleep(5)
    
    call hCloseDocument
    
endcase
'-------------------------------------------------------------------------
testcase tMainApp
    
    dim iCount as integer ' the numbers of the items in the creation list.
    
    hFileOpen( gOfficePath & "user/database/biblio.odb" )

    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    
    Kontext "DATABASE"
    if Database.exists() then
        printlog "Insight appear"
    else
        warnlog "Insight doesn't appear"
        goto endsub
    end if
    
    if DatabaseSwapView.exists() then
        printlog "InsightSwapView appear"        
    else
        warnlog "InsightSwapView doesn't appear"
        goto endsub
    end if
        
    '/// display Tables View
    printlog "display Tables View"
    ViewTables    
    if DatabaseTablesView.exists(1) then
        printlog "InsightTablesView appear"        
    else
        warnlog "InsightTablesView doesn't appear"
        goto endsub
    end if
    
    '/// count the items in the CreationList
    printlog "count the items in the CreationList"
    iCount = CreationList.getItemCount() 
    if ( iCount <> 2) then
        warnlog "There are not 2 items in the CreationList. There are " + iCount + " items."
    end if
        
    '/// display Queries View
    printlog "display Queries View"
    ViewQueries    
    if DatabaseQueriesView.exists(1) then
        printlog "InsightQueriesView appear"        
    else
        warnlog "InsightQueriesView doesn't appear"
        goto endsub
    end if
    
    '/// count the items in the CreationList
    printlog "count the items in the CreationList"
    iCount = CreationList.getItemCount() 
    if ( iCount <> 3) then
        warnlog "There are not 3 items in the CreationList. There are " + iCount + " items."
    end if
    
    '/// display Forms View
    printlog "display Forms View"
	ViewForms
    Kontext "ContainerView"    
    if FormTree.exists(1) then
        printlog "InsightFormsView appear"        
    else
        warnlog "InsightFormsView doesn't appear"
        goto endsub
    end if
    Kontext "DATABASE"
    '/// count the items in the CreationList
    printlog "count the items in the CreationList"
    iCount = CreationList.getItemCount() 
    if ( iCount <> 2) then
        warnlog "There are not 2 items in the CreationList. There are " + iCount + " items."
    end if
    
    '/// display Reports View
    printlog "display Reports View"
    ViewReports    
    if DatabaseReportsView.exists(1) then
        printlog "InsightReportsView appear"        
    else
        warnlog "InsightReportsView doesn't appear"
        goto endsub
    end if
    
    '/// count the items in the CreationList
    printlog "count the items in the CreationList"
    Dim iCheckCount as Integer
    iCount = CreationList.getItemCount()
    if ( gOOO ) then
        iCheckCount = 1
    else
        iCheckCount = 2
    endif
    if ( iCount <> iCheckCount ) then
        warnlog "There are not " + iCheckCount + " items in the CreationList. There are " + iCount + " items."
    end if
    
    call hCloseDocument
    
endcase
'-------------------------------------------------------------------------
testcase tPreview

    hFileOpen( gOfficePath & "user/database/biblio.odb" )

    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)    
    
    ViewTables
    
    sleep(1)
    
    Kontext "ContainerView"        
        TableTree.select 1
    
    Kontext "DATABASE"
        Database.UseMenu
        hMenuSelectNr(3)
        hMenuSelectNr(4)
        hMenuSelectNr(3)
        
    sleep(5)
    
    
    Kontext "DatabaseBeamer"
        if ( not DatabaseBeamer.exists(1) ) then
            warnlog "the preview doesn't display the table"
        else
            printlog "preview displayed"
        endif    
        
    Kontext "DATABASE"
        Database.UseMenu
        hMenuSelectNr(3)
        hMenuSelectNr(4)
        hMenuSelectNr(1)
    
    sleep(5)    
        
    Kontext "DatabaseBeamer"
        if ( DatabaseBeamer.exists(1) ) then
            warnlog "the preview is still there"
        else
            printlog "preview turn off"
        endif        
        
    sleep(1)    
        
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tMainMenuBar

    dim iCount as integer ' the numbers of the items in the menu.    
    
    hFileOpen( gOfficePath & "user/database/biblio.odb" )
    
    Kontext "DATABASE"
        Database.UseMenu        
        '/// check if there are 11 items in the file menu.
        printlog "check if there are 11 items in the file menu."
        '/// + there different count related to the file pick list.                
        printlog "+ there different count related to the file pick list."        
        hMenuSelectNr(1) ' the file menu        
        iCount = getMenuItemCount()        
        if ( lcase( gPlatform ) = "osx" ) then
            if ( iCount <> 10 ) then
                warnlog( "File menu: 10 items expected, found " & iCount )
            endif
        else
            if (iCount <> 11 ) then
               warnlog "There should be 11 items in the file menu but there are " + iCount + " items."
            end if
        endif

        '/// check if there are 12 items in the FILE/NEW menu.
        printlog "check if there are 12 items in the FILE/NEW menu."
        hMenuSelectNr(1) ' FILE/NEW menu        
        iCount = hMenuItemGetCount()        
        if (iCount <> 12) then
            warnlog "#i112496# - There should 12 items in the FILE/NEW menu but there are " + iCount + " items."
        end if        
        call hMenuClose() ' close the FILE menu
        
        '/// check the FILE/AUTOPILOTS
        printlog "check the FILE/AUTOPILOTS"
        Database.UseMenu        
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(4) ' the autopilots menu        
        iCount = hMenuItemGetCount()
        if (iCount <> 8) then
            warnlog "There should 8 items in the FILE/AUTOPILOTS menu but there are " + iCount + " items."
        endif
        call hMenuClose()
        
        '/// check if there are 15 items in the edit menu.
        printlog "check if there are 15 items in the edit menu."
        Database.UseMenu 
        hMenuSelectNr(2)        
        iCount = hMenuItemGetCount()
        if (iCount <> 15) then
            warnlog "There should 15 items in the edit menu but there are " + iCount + " items."
        end if
        call hMenuClose()

        '/// check if there are 6 items in the view menu.
        printlog "check if there are 6 items in the view menu."
        Database.UseMenu 
        hMenuSelectNr(3)        
        iCount = hMenuItemGetCount()
        if (iCount <> 6) then
            warnlog "There should 6 items in the view menu but there are " + iCount + " items."
        end if
        call hMenuClose()

        '/// check the VIEW/DATABASE OBJECTS
        printlog "check the VIEW/DATABASE OBJECTS"
        Database.UseMenu
        hMenuSelectNr(3)
        hMenuSelectNr(1)                
        iCount = hMenuItemGetCount()        
        if (iCount <> 4) then
            warnlog "There should 4 items in the VIEW/DATABASE menu but there are " + iCount + " items."
        end if        
        call hMenuClose()

        '/// check the VIEW/SORT OBJECTS
        printlog "check the VIEW/SORT OBJECTS"
        Database.UseMenu
        hMenuSelectNr(3)
        hMenuSelectNr(5)                
        iCount = hMenuItemGetCount()        
        if (iCount <> 2) then
            warnlog "There should 2 items in the VIEW/SORT menu but there are " + iCount + " items."
        end if        
        call hMenuClose()

        '/// check the VIEW/PREVIEW
        printlog "check the VIEW/PREVIEW"
        Database.UseMenu
        hMenuSelectNr(3)
        hMenuSelectNr(4 )                
        iCount = hMenuItemGetCount()        
        if (iCount <> 3) then
            warnlog "There should 3 items in the VIEW/PREVIEW menu but there are " + iCount + " items."
        end if        
        call hMenuClose()
        
        '/// check if there are 8 items in the insert menu.
        printlog "check if there are 8 items in the insert menu."
        Database.UseMenu 
        hMenuSelectNr(4)       
        iCount = hMenuItemGetCount()
        if (iCount <> 8) then
            warnlog "There should 8 items in the insert menu but there are " + iCount + " items."
        end if
        call hMenuClose()

        '/// check if there are 10 items in the tools menu.
        printlog "check if there are 10 items in the tools menu."        
        Database.UseMenu 
        hMenuSelectNr(5)        
        iCount = hMenuItemGetCount()
        if ( lcase( gPlatform ) = "osx" ) then
            if ( iCount <> 9 ) then
                warnlog( "Tools menu: 9 items expected, found " & iCount ) 
            endif
        else
            if (iCount <> 10) then
                warnlog "There should 10 items in the tools menu but there are " + iCount + " items."
            end if
        endif
        call hMenuClose()
        
        '/// check if there are 4 items in the TOOLS/MACRO menu.
        printlog "check if there are 4 items in the TOOLS/MACRO menu."        
        Database.UseMenu 
        hMenuSelectNr(5)
        hMenuSelectNr(6)        
        iCount = hMenuItemGetCount()
        if (iCount <> 4) then
            warnlog "There should 4 items in the TOOLS/MACRO menu but there are " + iCount + " items."
        end if
        call hMenuClose()
        
    call hCloseDocument
        
endcase
'-------------------------------------------------------------------------
testcase tDatabaseProperties 
    
    dim sValue as string
    
    hFileOpen( gOfficePath & "user/database/biblio.odb" )
    
    Kontext "DATABASE"        
        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    Kontext "DATABASE"    
        Database.UseMenu    
        hMenuSelectNr(2)
        hMenuSelectNr(15)
        hMenuSelectNr(1)
    sleep(1)
    Kontext "DatabaseProperties"
        call DialogTest(DatabaseProperties)
        ' TabControl.SetPage TabGeneral
    ' 
    ' '/// check the general tabpage
    ' printlog "check the general tabpage"     
    ' Kontext "TabGeneral"
    ' 'TODO: check real names. 
        ' sValue = SourceName.getText()
        ' if ( strComp(sValue,"TT_Biblio") <> 0 ) then
            ' warnlog "The datasource name is not right. it should be 'TT_Biblio' but it is " + sValue
        ' end if
        ' sValue = DataBaseType.getSelText()
        ' if ( strComp(sValue,"dBASE") <> 0 ) then
            ' warnlog "The datasource type is not right. it should be 'dBase' but it is " + sValue
        ' end if
        ' 
        ' Advanced.Click
    ' Kontext "Advanced"
        ' Advanced.Close
        
' tabconnection in this script == tabgeneral in the office ( historic reasons )        
    '/// check the connection tabpage
    printlog "check the connection tabpage"
    Kontext "DatabaseProperties"             
        TabControl.SetPage TabConnection
    
    Kontext "TabConnection"
        if NOT TabConnection.exists(1) then
            warnlog "Tabpage doesn't exists."
        end if
        '/// check if the text in the path is correct
        'printlog "check if the text in the path is correct"
        sValue = dBasePath.getText()
        'if ( strComp(sValue,"sdbc:dbase:$(userurl)/database/biblio") <> 0 ) then
        '    warnlog "the path is not correct. It is " + sValue
        'end if
        '/// click the select path button
        printlog "click the select path button"        
        SelectPath.Click        
    
    Kontext "GeneralFileDialog"
        '/// cancel the open dialog
        printlog "cancel the open dialog"
        GeneralFileDialog.Cancel
    
    Kontext "TabConnection"
        '/// click the test connection button
        printlog "click the test connection button"
        TestConnection.Click
    
    Kontext "Active"
        '/// close the messagesdialog
        printlog "close the messagesdialog"
        Active.OK
    
    Kontext "DatabaseProperties"             
        '/// switch to the additional Settings
        printlog "switch to the additional Settings"
        TabControl.SetPage TabdBase
    
    Kontext "TabdBase"        
        '/// open the index dialog
        printlog "open the index dialog"
        Indexes.Click
    
    Kontext "Indexes"
        '/// close the index dialog with cancel
        Indexes.Cancel
    
    Kontext "DatabaseProperties"
        '/// Cancel the datasource admin dialog
        printlog "Cancel the datasource admin dialog"
        DatabaseProperties.Cancel
        
    call hCloseDocument()
    
endcase
'--------------------------------------------------------------------
testcase tRegisterDatabaseDialog
    
    dim i as integer
    '/// open new document
    printlog "open new document"
    call hNewDocument
    '/// open TOOLS / OPTIONS
    printlog "open TOOLS / OPTIONS"
    ToolsOptions
    '/// open Data Source / DATABASES
    printlog "open Data Source / DATABASES"    
    call hToolsOptions ( "Datasources", "Databases" )
       
    Kontext "TabRegisteredDatabase"
        call DialogTest( TabRegisteredDatabase )    
        if (RegisteredDatabases.getItemCount() = 0 ) then
            warnlog "there are no registered databases. There should be at least a  registered 'Bibliography' Database."
            Kontext "OptionenDlg"
                OptionenDlg.Cancel
            call hCloseDocument
            goto endsub
        endif            
        RegisteredDatabases.select 1
        '/// check if the Bibliography is registered
        printlog "check if the Bibliography is registered"
        if RegisteredDatabases.getSeltext() <> "Bibliography" then
            warnlog "the bibliography database is not registered"
        else
            printlog "database bibliography is registered"
        endif
        '/// click in the delete button but don't delete the database
        printlog "click in the delete button but don't delete the database"
        DeleteBtn.Click	
        Kontext "Active"
            Active.no
    '/// click on the new button
    printlog "click on the new button"            
    Kontext "TabRegisteredDatabase"
        NewBtn.Click
    sleep(1)
    '/// cancel the new dialog
    printlog "cancel the new dialog"
    Kontext "CreateDatabaseLink"
        call DialogTest( CreateDatabaseLink )
        CreateDatabaseLink.Cancel

    '/// click on the edit button
    printlog "click on the edit button"            
    Kontext "TabRegisteredDatabase"
        EditBtn.Click
    sleep(1)
    '/// cancel the edit dialog
    Kontext "CreateDatabaseLink"
        call DialogTest( CreateDatabaseLink )
        CreateDatabaseLink.Cancel        
        
    sleep(1)
    '/// close TOOLS / OPTIONS with cancel
    printlog "close TOOLS / OPTIONS with cancel"
    Kontext "OptionenDlg"
        OptionenDlg.Cancel
        
    '/// close document
    printlog "close document"
        
    call hCloseDocument        
        
endcase
'--------------------------------------------------------------------
testcase tDatabaseBeamer
    
    printlog "open new document"    
    call hNewDocument
    
    printlog "open the database beamer"    
    ViewCurrentDatabase
    WaitSlot(1)
    
    printlog "select database Bibliography"
    Kontext "DatabaseSelection"
    DatabaseSelection.select 1
    
    printlog "open the selected database"
    DatabaseSelection.OpenContextmenu    
    hMenuSelectNr(1)
    WaitSlot(1)
    
    Kontext "DATABASE"
    If Database.exists(10) then
        printlog "database open -> OK"
        printlog "close the open database"
        call fCloseDatabase
    else
        warnlog "database mot open"
    endif
    
    printlog "close the open document"        
    call hCloseDocument        
        
endcase
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
function getMenuItemCount as Integer
'use this function becasue this doesn't count the pick list
    Dim i as Integer
    Dim j as Integer
    Dim Ende as Integer
    Dim nID as Integer
    Sleep 2
    j=0
    for i=1 to MenuGetItemCount
        if NOT MenuIsSeperator ( i ) then            
            dim s as String 
            s = Mid(MenuGetItemText( MenuGetItemID(i)),3,1)
            'printlog "s = " + s
            'printlog "MenuGetItemText( MenuGetItemID(i)) = " + MenuGetItemText( MenuGetItemID(i))
            if s <> ":" then             
                j=j+1
            endif
        endif
    next i
    getMenuItemCount = j
end function

