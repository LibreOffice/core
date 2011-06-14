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
'* short description : Grid Control Test
'*
'\***********************************************************************
sub ctrl_Grid

	printlog "------------------- ctrl_Grid.inc ------------------------"
           
    call tCreateGridControlColumn
    call tReplaceGridControlColumn
    call tDeleteGridControlColumn    
    call tShowHideGridControlColumn
        	
end sub
'-------------------------------------------------------------------------
testcase tCreateGridControlColumn
    
    dim i as integer 

    '/// open new document
    printlog "open new document and tear off form function toolbar"    
    call hNewDocument
    sleep(1)

    call hToolbarSelect("MoreControls",true)

    '/// insert a grid control
    printlog "insert a grid control"
    Kontext "MoreControls"
        Grid.Click
        sleep(4)
        call hDrawingWithSelection ( 40, 20, 60, 40 )
        sleep(2)
    '/// cancel the autopilot if exist
    printlog "cancel the autopilot if exist"
	Kontext "AutopilotTableElement"	
	    if AutopilotTableElement.Exists(3) then AutopilotTableElement.Cancel
        sleep(1)        
    
    Kontext "DocumentWriter"
    '/// select the gridcontrol and press return to get into the edit mode 
        printlog "select the gridcontrol and change in edit mode"      
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        '/// press SHIFT F10 to open the context menu and insert a text controls
        printlog "press SHIFT F10 to open the context menu and insert 3 controls"
        for i = 1 to 3                     
            DocumentWriter.Typekeys "<SHIFT F10>" , true            
            myMenuSelectNr(1)
            myMenuSelectNr(i) 
            sleep(1)
        next i    
        sleep(1)    
    '/// save and close the document
    printlog "save and close the document"        
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_GridControl_create.odt"))
    hCloseDocument
    sleep(1)
    '/// load the document
    printlog "load the document"
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_GridControl_create.odt"))
    sleep(1)        
    
    call hToolbarSelect("FormControls",true)

    Kontext "FormControls"
        SwitchControlDesignMode.Click
        sleep(4) 

    '/// check if the control are still there
    printlog "check if the control are still there"        
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
        sleep(1)
        DocumentWriter.Typekeys "<SHIFT F10>" , true
        'open the propety broser             
        myMenuSelectNr(5)
        sleep(1)

        Kontext "ControlPropertiesTabControl"
            ControlPropertiesTabControl.setPage TabGeneralControl          
                
        if getControlType <> "text box" then 
            warnlog "column controls is wrong"                
        endif
    ' close property browser
    Kontext "TabGeneralControl"
        TabGeneralControl.TypeKeys "<Escape>"
        sleep(1)

     call hCloseDocument

endcase
'-------------------------------------------------------------------------
testcase tDeleteGridControlColumn
    
    dim i as integer 

    '/// open new document
    printlog "open new document and tear off form function toolbar"    
    call hNewDocument
    sleep(1)

    call hToolbarSelect("MoreControls",true)

    '/// insert a grid control
    printlog "insert a grid control"
    Kontext "MoreControls"
        Grid.Click
        sleep(4)        
        call hDrawingWithSelection ( 40, 20, 60, 40 )
        sleep(2)
    '/// cancel the autopilot if exist
    printlog "cancel the autopilot if exist"
	Kontext "AutopilotTableElement"	
	    if AutopilotTableElement.Exists(3) then AutopilotTableElement.Cancel
        sleep(1)        
    
    Kontext "DocumentWriter"
    '/// select the gridcontrol and press return to get into the edit mode 
        printlog "select the gridcontrol and change in edit mode"      
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        '/// press SHIFT F10 to open the context menu and insert a text controls
        printlog "press SHIFT F10 to open the context menu and insert 3 controls"
        for i = 1 to 3                     
            DocumentWriter.Typekeys "<SHIFT F10>" , true            
            myMenuSelectNr(1)
            myMenuSelectNr(i) 
            sleep(1)
        next i    
        sleep(1)

    '/// delete inserted columns
    printlog "delete inserted columns"
    DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
    sleep(1)
    DocumentWriter.Typekeys "<RIGHT>" , true
    sleep(1)        
    DocumentWriter.Typekeys "<SHIFT F10>" , true            
    myMenuSelectNr(3)
    sleep(1)
    DocumentWriter.Typekeys "<LEFT>" , true
    sleep(1)
    DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
    sleep(1)
    DocumentWriter.Typekeys "<SHIFT F10>" , true            
    myMenuSelectNr(3)
    sleep(1)
    DocumentWriter.Typekeys "<LEFT>" , true
    sleep(1)
    DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
    sleep(1)
    DocumentWriter.Typekeys "<SHIFT F10>" , true         
    myMenuSelectNr(3)
    sleep(1)
    '/// check deletion
    printlog "check deletion"
    DocumentWriter.Typekeys "<LEFT>" , true
    sleep(1)
    DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
    sleep(1)
    DocumentWriter.Typekeys "<SHIFT F10>" , true  
    if hMenuItemGetCount <> 1 then   'context of a column has more than one item
        warnlog "Deleting column went wrong"
    else
        printlog "deleting column successfull"
    endif
    sleep(1)
    call hMenuClose()
    '/// save and close the document
        printlog "save and close the document"        
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_GridControl_delete.odt"))
        hCloseDocument
        sleep(1)
    '/// reload the document
        printlog "reload the document"
        hFileOpen ( gOfficePath + ConvertPath("user/work/TT_GridControl_delete.odt"))
        sleep(1)

    call hToolbarSelect("FormControls",true)

    Kontext "FormControls"
        SwitchControlDesignMode.Click
        sleep(4)
        
    '/// check deletion after save-&reloading
    printlog "check deletion after save-&reloading"
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<RETURN>" , true     
        sleep(1) 
        DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
        sleep(1)
        DocumentWriter.Typekeys "<SHIFT F10>" , true        
        sleep(1) 
        if hMenuItemGetCount <> 1 then   'context of a column has more than one item
            warnlog "Deleting column after save-&reloading went wrong"
        else
            printlog "deleting column after save-&reloading successfull"
        endif
        sleep(1)
        'close the open context menu
        MenuSelect(0)
    call hCloseDocument()

endcase
'-------------------------------------------------------------------------
testcase tReplaceGridControlColumn    

    '/// open new document
        printlog "open new document and tear off form function toolbar"    
        call hNewDocument
        sleep(1)
        'prepare document
    
    call hToolbarSelect("MoreControls",true)

    Kontext "MoreControls"
        Grid.Click
        sleep(4)
    '/// insert a grid control
    printlog "insert a grid control"
        printlog "insert a grid control"        
        call hDrawingWithSelection ( 40, 20, 60, 40 )
        sleep(2)
    '/// cancel the autopilot if exist
    printlog "cancel the autopilot if exist"
        printlog "cancel the autopilot if exist"
	Kontext "AutopilotTableElement"	
	    if AutopilotTableElement.Exists(3) then AutopilotTableElement.Cancel
        sleep(1)
    '/// - check replacing columns from context -
    printlog "- check replacing columns from context -"
        printlog ""
        printlog "- check replacing columns from context -"
    
    '/// press SHIFT F10 to open the context menu and insert a text controls
    printlog "press SHIFT F10 to open the context menu and insert a text controls"    
    Kontext "DocumentWriter"        
        DocumentWriter.Typekeys "<RETURN>" , true  
        sleep(1)        
        DocumentWriter.Typekeys "<SHIFT F10>" , true            
        myMenuSelectNr(1)
        myMenuSelectNr(1) 
        

    '/// replace the text control with a list box controls
    printlog "replace the text control with a list box controls"
    Kontext "DocumentWriter"
        
        DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
        sleep(1)
        DocumentWriter.Typekeys "<SHIFT F10>" , true            
        myMenuSelectNr(2)            
        myMenuSelectNr(3)            
        sleep(1)

    '/// check if the control was replaced
    printlog "check if the control was replaced"    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
        sleep(1)
        DocumentWriter.Typekeys "<SHIFT F10>" , true
        'open the propety broser             
        myMenuSelectNr(4)
        sleep(1)                    
        if getControlType <> "list box" then 
            warnlog "Replacing column controls went wrong"                
        endif
        ' close the property browser        
        Kontext "TabGeneralControl"
            TabGeneralControl.TypeKeys "<Escape>"
            sleep(1)

    '/// save and close the document
        printlog "save and close the document"        
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_GridControl_replacing.odt"))
        hCloseDocument
        sleep(1)
    '/// reload the document
        printlog "reload the document"
        hFileOpen ( gOfficePath + ConvertPath("user/work/TT_GridControl_replacing.odt"))
        sleep(1)
        'prepare document
    
    call hToolbarSelect("FormControls",true)
    
    Kontext "FormControls"
        SwitchControlDesignMode.Click
        sleep(4) 
    '/// check replacing after save-&reloading
    printlog "check replacing after save-&reloading"
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
        sleep(1)        
        DocumentWriter.Typekeys "<SHIFT F10>" , true
        ' open property browser             
        myMenuSelectNr(4)
        if getControlType <> "list box" then 
            warnlog "Replacing column controls not present after reload"            
        endif
        ' close property browser
        Kontext "TabGeneralControl"
            TabGeneralControl.TypeKeys "<Escape>"
            sleep(1)
        call hCloseDocument

endcase
'-------------------------------------------------------------------------
testcase tShowHideGridControlColumn
    
    dim i as integer

    '/// open new document
    printlog "open new document"        
    call hNewDocument
    sleep(1)
    
    call hToolbarSelect("MoreControls",true)
    
    Kontext "MoreControls"
        Grid.Click
        sleep(4)
    
    '/// insert a grid control
    printlog "insert a grid control"    
    call hDrawingWithSelection ( 40, 20, 60, 40 )
    sleep(2)

    '/// cancel the autopilot if exist
    printlog "cancel the autopilot if exist"
	Kontext "AutopilotTableElement"	
	    if AutopilotTableElement.Exists(3) then AutopilotTableElement.Cancel
        sleep(1)
            
    '/// select the gridcontrol and press return to get into the edit mode 
    printlog "select the gridcontrol and change in edit mode"
    Kontext "DocumentWriter"      
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
    
    '/// press SHIFT F10 to open the context menu and insert controls
    printlog "press SHIFT F10 to open the context menu and insert controls"       
        for i = 1 to 3                     
            DocumentWriter.Typekeys "<SHIFT F10>" , true            
            myMenuSelectNr(1)
            myMenuSelectNr(i) 
            sleep(1)
        next i    
        sleep(1)     
    '/// hide the first column
    printlog "hide the first column"
    Kontext "DocumentWriter"
        sleep(1)
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
        sleep(1)
        DocumentWriter.Typekeys "<SHIFT F10>" , true             
        myMenuSelectNr(4)
        sleep(1)
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
        sleep(1)
    
    '/// check hiding column
    printlog "check hiding column"
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        DocumentWriter.Typekeys "<SHIFT F10>" , true             
        if hMenuItemGetCount <> 6 then   'context of a column after hiding has 6 items (normal 5)
            warnlog "Hiding column went wrong"
        else
            printlog "hiding column successfull"
        endif
        call hMenuClose()
    '/// save and close the document
        printlog "save and close the document"        
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_GridControl_hide_show.odt"))
        hCloseDocument
        sleep(1)

    '/// reload the document
        printlog "reload the document"
        hFileOpen ( gOfficePath + ConvertPath("user/work/TT_GridControl_hide_show.odt"))
        sleep(1)
    
    '/// check hiding column after saving and reloading
    printlog "check hiding column after saving and reloadin"

    call hToolbarSelect("FormControls",true)
    
    Kontext "FormControls"
        SwitchControlDesignMode.Click
        sleep(4)
 
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<RETURN>" , true
        sleep(1)
        DocumentWriter.Typekeys "<MOD1 SHIFT SPACE>" , true
        sleep(1)
        DocumentWriter.Typekeys "<SHIFT F10>" , true             
        if hMenuItemGetCount <> 6 then   'context of a column after hiding has 6 items (normal 5)
            warnlog "Hiding column after saving and reloading went wrong"
        else
            printlog "hiding column after saving and reloading successfull"
        endif

    '/// show column after saving and reloading
    printlog "show column after saving and reloading"
        myMenuSelectNr(5)
        myMenuSelectNr(3) 
        sleep(1)    
        DocumentWriter.Typekeys "<SHIFT F10>" , true             
        if hMenuItemGetCount <> 5 then   'context of a column normal has 5 items (before show column 6)
            warnlog "Show column after saving and reloading went wrong"
        else
            printlog "show column after saving and reloading successfull"
        endif
        sleep(1)
        'close the open context menu
        MenuSelect(0)
        call hCloseDocument

endcase
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
function getControlType     
    'control property browser must be open
    'check what properties are present and so find out what control type
    dim sControlString as string 
	Kontext "TabGeneralControl"	
        if TabGeneralControl.exists(3) then
            TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
            if DefaultState.exists then
                sControlString = "check box"
            elseif LineCount.exists then
                if MaxTextLen.exists then
                    sControlString = "combo box"
                else
                    sControlString = "list box"
                endif
            elseif EditMask.exists then
                sControlString = "pattern field"
            elseif FormatKey.exists then
                sControlString = "formatted field"
            elseif CurrencySymbol.exists then
                sControlString = "currency field"
            elseif TimeMin.exists then
                sControlString = "time field"
            elseif DateMin.exists then
                sControlString = "date field"
            elseif StrictFormat.exists then
                sControlString = "numeric field"                                    
            elseif MaxTextLen.exists then                
                if EditMask.exists then
                    sControlString = "pattern field"
                else
                    sControlString = "text box"
                endif
            else
                sControlString = "unkown"
            endif
        else
            warnlog "TabGeneralControl not there."
        endif
	sleep(1)	
	    getControlType = sControlString
end function
'-------------------------------------------------------------------------
sub myMenuSelectNr ( EintragsNr as Integer ) as String
    Dim nID as Integer
    Sleep 1   
    nID = MenuGetItemID ( EintragsNr )   
    sleep(1)
    MenuSelect ( nID )
    sleep(1)
end sub
