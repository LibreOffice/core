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
'* short description : test the Main Table Design Application Window
'*
'************************************************************************
'* 
' #1 tEdit                
' #1 tSave
' #1 tSaveAs
' #1 tIndexDesign
' #1 tCut
' #1 tCopy    
' #1 tPaste
' #1 tUndo
' #1 tRedo          
'*
'\***********************************************************************************
sub TableDesign

    printlog "------------------ TableDesign.inc ---------------------"
    
    printlog "-------------------- main menu -------------------------"
     call tMainMenuBar
    
    printlog "--------------------- toolbar --------------------------"
     call tSave     
     call tIndexDesign
     call tCut
     call tCopy    
     call tPaste
     call tUndo
     call tRedo
     
end sub
'-------------------------------------------------------------------------
testcase tSave

    '/// open a new table design
    printlog "open a new table design"
    initTableDesign    
    sleep(1)
    '/// insert a fieldname
    printlog "insert a fieldname"
    Kontext "TableDesignTable"
        Fieldname.setText "hello"        
    sleep(1)
    '/// click on the save button
    printlog "click on the save button"
    Kontext "Toolbar"
        Save.Click
        sleep(1)
    '/// check if the save dialog appear
    printlog "check if the save dialog appear"
    Kontext "DatabaseTableSaveAs"
        if ( not DatabaseTableSaveAs.exists(3) ) then
            warnlog "the save dialog doesn't appear."
        else
            '/// cancel the save dialog
            printlog "cancel the save dialog"    
            DatabaseTableSaveAs.Cancel
        end if
    '/// close tables design and the database
    printlog "close tables design and the database"    
    call fCloseTableDesign
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tIndexDesign

    '/// open a new table design
    printlog "open a new table design"
    initTableDesign    
    sleep(1)
    '/// insert a fieldname
    printlog "insert a fieldname"
    Kontext "TableDesignTable"
        Fieldname.setText "hello"        
    sleep(1)
    '/// click on the save button
    printlog "click on the save button"
    Kontext "Toolbar"
        Save.Click
        sleep(1)

    Kontext "DatabaseTableSaveAs"
        if ( not DatabaseTableSaveAs.exists(3) ) then
            warnlog "the save dialog doesn't appear."
            goto endsub
        else
            '/// save with the default name
            printlog "save with the default name"
            Kontext "DatabaseTableSaveAs"
            TableName.setText("testtable1")
            DatabaseTableSaveAs.OK
        end if
    '/// click on the index design icon
    printlog "click on the index design icon"
    Kontext "Toolbar"
        DBIndexDesign.Click
        sleep(1)            

    '/// close the index design
    printlog "close the index design"
    Kontext "DatabaseIndexes"
        DatabaseIndexesClose.click
        
    '/// close the table design
    printlog "close the table design"
    call fCloseTableDesign
    
    '/// delete the created table
    printlog "delete the created table"
    call fDeleteTable("testtable1")
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tCut

    '/// open a new tabledesign
    printlog "open a new tabledesign"
    initTableDesign
    sleep(1)
    '/// insert a field name
    printlog "insert a field name"
    Kontext "TableDesignTable"        
        TableDesignTable.TypeKeys "hello" , true
        sleep(1)
        '/// select the field name with SHIFT HOME
        printlog "select the field name with SHIFT HOME"
        TableDesignTable.TypeKeys "<SHIFT HOME>" , true
        sleep(1)
    
    '/// click on the cut button
    printlog "click on the cut button"
    Kontext "Toolbar"
        Cut.Click
        sleep(1)
       
    '/// check if the text is in the clipboard
    printlog "check if the text is in the clipboard"                
    if getClipboard <> "hello" then
        warnlog "The Cut button doesn't work"
    end if        

    '/// check if the fieldname is cutted
    printlog "check if the fieldname is cutted"
    Kontext "TableDesignTable"
        if ( Fieldname.getText <> "" ) then
            warnlog "the text was not cutted"
        end if
        
    '/// close the table design and the database
    printlog "close the table design and the database"
    call fCloseTableDesign
    call fCloseDatabase   
        
endcase
'-------------------------------------------------------------------------
testcase tCopy
    '/// open a new tabledesign
    printlog "open a new tabledesign"
    initTableDesign    
    '/// insert a field name
    printlog "insert a field name"
    Kontext "TableDesignTable"
        TableDesignTable.TypeKeys "hello" , true
        sleep(1)
        '/// select the field name with SHIFT HOME
        printlog "select the field name with SHIFT HOME"
        TableDesignTable.TypeKeys "<SHIFT HOME>" , true
        sleep(1)

    '/// click on the copy icon
    printlog "click on the copy icon"    
    Kontext "Toolbar"
        Copy.Click
        sleep(1)
       
    '/// check if the text is in the clipboard
    printlog "check if the text is in the clipboard"                
    if getClipboard <> "hello" then
        warnlog "The Cut button doesn't work"
    end if        
    '/// close the table design and the database
    printlog "close the table design and the database"
    call fCloseTableDesign
    call fCloseDatabase   
        
endcase
'-------------------------------------------------------------------------
testcase tPaste

    '/// opene new table design
    printlog "opene new table design"    
    initTableDesign

    '/// set a string to the clipboard
    printlog "set a string to the clipboard"
    setClipboard "word"
    
    '/// press the cursor left key
    printlog "press the cursor left key"
    Kontext "TableDesignTable"
        TableDesignTable.TypeKeys "<LEFT>" , true

    '/// paste the text from the clipbaord
    printlog "paste the text from the clipbaord"
    Kontext "Toolbar"
        Paste.Click
        sleep(1)
    '/// check if the text is pasted to the fieldname
    printlog "check if the text is pasted to the fieldname"    
    Kontext "TableDesignTable"                
        if Fieldname.getText <> "word" then
            warnlog "the paste button doesn't work"
        end if
    '/// close the table design and the database
    printlog "close the table design and the database"    
    call fCloseTableDesign
    call fCloseDatabase     
    
endcase
'-------------------------------------------------------------------------
testcase tUndo

    '/// opene new table design
    printlog "opene new table design"
    initTableDesign
    
    '/// set a string to the clipboard
    printlog "set a string to the clipboard"
    setClipboard "word"
    
    Kontext "TableDesignTable"
        TableDesignTable.TypeKeys "" , true

    '/// paste the text from the clipbaord
    printlog "paste the text from the clipbaord"
    Kontext "Toolbar"
        Paste.Click
        sleep(1)
    '/// check if the text is pasted to the fieldname
    printlog "check if the text is pasted to the fieldname"    
    Kontext "TableDesignTable"        
        if Fieldname.getText <> "word" then
            warnlog "the paste button doesn't work"
        end if

    '/// click the undo icon
    printlog "click the undo icon"
    Kontext "Toolbar"
        Undo.Click
        sleep(1)
    
    '/// check if the fieldname is empty again(undo work)
    printlog "check if the fieldname is empty again(undo work)"        
    Kontext "TableDesignTable"
        TableDesignTable.TypeKeys "<LEFT>" , true
        if Fieldname.getText <> "" then
            warnlog "the undo button doesn't work"
        end if
    '/// close the table design and the database
    printlog "close the table design and the database"    
    call fCloseTableDesign
    call fCloseDatabase   
     
    
endcase
'-------------------------------------------------------------------------
testcase tRedo

    '/// opene new table design
    printlog "opene new table design"
    initTableDesign
    
    '/// set a string to the clipboard
    printlog "set a string to the clipboard"
    setClipboard "word"
    
    '/// press the cursor left key
    printlog "press the cursor left key" 
    Kontext "TableDesignTable"
        TableDesignTable.TypeKeys "<LEFT>" , true

    '/// paste the text from the clipbaord
    printlog "paste the text from the clipbaord"
    Kontext "Toolbar"
        Paste.Click
        sleep(1)
        
    '/// check if the text is pasted to the fieldname
    printlog "check if the text is pasted to the fieldname"    
    Kontext "TableDesignTable"
        if Fieldname.getText <> "word" then
            warnlog "the paste button doesn't work"
        end if
        
    '/// click the undo icon
    printlog "click the undo icon"
    Kontext "Toolbar"
        Undo.Click
        sleep(1)

    '/// press the cursor left key
    printlog "press the cursor left key"        
    Kontext "TableDesignTable"
        TableDesignTable.TypeKeys "<LEFT>" , true
        if Fieldname.getText <> "" then
            warnlog "the undo button doesn't work"
        end if

    '/// click the redo icon
    printlog "click the redo icon"        
    Kontext "Toolbar"        
        Redo.Click
        sleep(1)

    '/// press the cursor left key
    printlog "press the cursor left key"        
    Kontext "TableDesignTable"
        TableDesignTable.TypeKeys "<LEFT>" , true
        if Fieldname.getText <> "word" then
            warnlog "the redo button doesn't work"
        end if

    '/// close the tabledesign and the database
    printlog "close the tabledesign and the database"    
    call fCloseTableDesign
    call fCloseDatabase   
     
    
endcase
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tMainMenuBar

    dim iCount as integer ' the numbers of the items in the menu.    
    
    call initTableDesign
    
    Kontext "TableDesignTable"
        TableDesignTable.UseMenu
           
        '/// check if there are between 8 and 12 items in the file menu.
        printlog "check if there are between 8 and 12 items in the file menu."
        '/// + there different count related to the file pick list.                
        printlog "+ there different count related to the file pick list."        
        hMenuSelectNr(1) ' the file menu        
        iCount = getMenuItemCount()        
        if ( lcase( gPlatform ) = "osx" ) then
            if ( iCount <> 7 ) then
                warnlog( "File menu: 7 items expected, found " & iCount )
            endif
        else
            if (iCount <> 8 ) then
                warnlog "There should between 8 and 12 items in the file menu but there are " + iCount + " items."
            end if
        endif

        '/// check if there are 12 items in the FILE/NEW menu.
        printlog "check if there are 12 items in the FILE/NEW menu."
        printlog "check the FILE/NEW"
        hMenuSelectNr(1) ' FILE/NEW menu        
        iCount = hMenuItemGetCount()        
        if (iCount <> 12) then
            warnlog "There should 12 items in the FILE/NEW menu but there are " + iCount + " items."
        end if        
        call hMenuClose() ' close the FILE menu
        
        '/// check the FILE/AUTOPILOTS
        printlog "check the FILE/AUTOPILOTS"
        TableDesignTable.UseMenu        
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(3) ' the autopilots menu        
        iCount = hMenuItemGetCount()
        if (iCount <> 8) then
            warnlog "There should 8 items in the FILE/AUTOPILOTS menu but there are " + iCount + " items."
        endif
        call hMenuClose()
        
        '/// check if there are 5 items in the edit menu.
        printlog "check if there are 5 items in the edit menu."
        TableDesignTable.UseMenu 
        hMenuSelectNr(2)        
        iCount = hMenuItemGetCount()
        if (iCount <> 5) then
            warnlog "There should 5 items in the edit menu but there are " + iCount + " items."
        end if
        call hMenuClose()

        '/// check if there are 2 items in the view menu.
        printlog "check if there are 2 items in the view menu."
        TableDesignTable.UseMenu 
        hMenuSelectNr(3)        
        iCount = hMenuItemGetCount()
        if (iCount <> 2) then
            warnlog "There should 2 items in the edit menu but there are " + iCount + " items."
        end if
        call hMenuClose()

        '/// check if there are 5 items in the tools menu.
        printlog "check if there are 5 items in the tools menu."        
        TableDesignTable.UseMenu 
        hMenuSelectNr(4)        
        iCount = hMenuItemGetCount()
        if ( lcase( gPlatform ) = "osx" ) then
            if ( iCount <> 4 ) then
                warnlog( "Tools menu: 4 items expected, found " & iCount )
            endif
        else
            if (iCount <> 5) then
                warnlog "There should 5 items in the tools menu but there are " + iCount + " items."
            end if
        endif
        call hMenuClose()
        
        '/// check if there are 4 items in the TOOLS/MACRO menu.
        printlog "check if there are 4 items in the TOOLS/MACRO menu."        
        TableDesignTable.UseMenu 
        hMenuSelectNr(4)
        hMenuSelectNr(2)        
        iCount = hMenuItemGetCount()
        if (iCount <> 4) then
            warnlog "There should 4 items in the TOOLS/MACRO menu but there are " + iCount + " items."
        end if
        call hMenuClose()
        
        call fCloseTableDesign
        
        call fCloseDatabase
        
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
'-------------------------------------------------------------------------
sub initTableDesign

    hFileOpen( gOfficePath & "user/database/biblio.odb" )
    
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    
    sleep(1)
    
    ViewTables
    
    sleep(1)
    
    NewTableDesign
    
    sleep(2)
    
end sub    
