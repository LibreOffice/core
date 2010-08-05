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
'* short description : Controls linked to spreadsheet cells
'*
'\***********************************************************************

'* preconditions : no FormDesignToolbar should be open
'* testpurpose : test if controls linked to cells work correct

sub ctrl_Spreadsheet

    call preCon_CloseFormDesignToolbar
    call tTextBox
    call tListBox
    call tCheckBox
    
end sub
'--------------------------------------------------------------------------------------
testcase tListBox
    
    Dim j as integer
    Dim k as integer
    dim i as integer
    printlog "- Formular toolbox: List box element (" & gApplication & ")"
    '/// open a spreadsheet, insert a listbox (the wizard appears)
    gApplication = "CALC"
    call hNewDocument
    Kontext "DocumentCalc"
    DocumentCalc.TypeKeys("Value1")
    DocumentCalc.TypeKeys("<RETURN>")
    DocumentCalc.TypeKeys("Value2")
    DocumentCalc.TypeKeys("<RETURN>")
    DocumentCalc.TypeKeys("Value3")
    DocumentCalc.TypeKeys("<RETURN>")
    DocumentCalc.TypeKeys("Value4")
    DocumentCalc.TypeKeys("<RETURN>")
    DocumentCalc.TypeKeys("Value5")
    DocumentCalc.TypeKeys("<RETURN>")
    sleep(1)

    call hToolbarSelect("FormControls",true)
    
    Kontext "FormControls"
    Listbox.Click
    sleep(3)
    call hDrawingWithSelection ( 60, 40, 70, 55 )
    sleep(3)
    Kontext "AutoPilotListComboBox"
    AutoPilotListComboBox.MouseDown(10,10)
    AutoPilotListComboBox.MouseUp(10,10)
    '/// close the autopilot
    if AutoPilotListComboBox.Exists(1) then        
        AutopilotListComboBox.Cancel                
    end if
	sleep(1)

    call hToolbarSelect("FormControls",true)   
	
    Kontext "FormControls"    
	ControlProperties.Click
	sleep(1)
    call fsetControlProperty("ListBox","CellRange","A1:A5")
	call fsetControlProperty("ListBox","CellBound","B1")
	sleep(1)    
    '/// close control property browser
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.Close
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_ListBox.ods"))
	hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_ListBox.ods"))	
    sleep 1
    KOntext "DocumentCalc"
    DocumentCalc.TypeKeys "<MOD1 F5>" , true
    sleep(1)
    DocumentCalc.TypeKeys "<DOWN>" , true
    sleep(1)
    DocumentCalc.TypeKeys "<DOWN>" , true
    sleep(1)
    DocumentCalc.TypeKeys "<MOD1 F6>" , true
    sleep(1)
    DocumentCalc.TypeKeys "<MOD1 HOME>" , true
    sleep(1)
    DocumentCalc.TypeKeys "<RIGHT>" , true
    sleep(1)
    DocumentCalc.TypeKeys "<MOD1 C>" , true
    sleep(1)
    Dim str1 as String
    str1 = getClipboardText()    
    if str1 <> "Value2" then
        warnlog "the value is not right. Is should be Value2 but it is " + str1
    endif
	hCloseDocument

endcase
'--------------------------------------------------------------------------------------
testcase tTextBox     
    
    '/// Open a new document,
    gApplication = "CALC"
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentCalc"       
	'DocumentWriter.TypeKeys "tt_doc" , true

	sleep(1)	

    call hToolbarSelect("FormControls",true)
	
    Kontext "FormControls"
  	   	
   	Edit.Click
   	sleep(1)      

    '/// insert a text box
    printlog "insert a text box"
    sleep(3)
    call hDrawingWithSelection ( 30, 30, 40, 40 )
	sleep(2)
	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    '/// set multiline and alignment to right            	
	call fsetControlProperty("TextBox","CellBound","A1")    
    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.Close
    
    '/// save the document
    printlog "save the document"
	hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_TextBox.ods"))
	hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_TextBox.ods"))    

    call hToolbarSelect("FormControls",true)    

    sleep(1)
  	Kontext "FormControls"
    SwitchControlDesignMode.Click
    sleep(1)
    Kontext "DocumentCalc"
     DocumentCalc.typeKeys "Hello world" , true ' this is important to get the focus into the document
     sleep(1)
     DocumentCalc.typeKeys "<F6>" , 6 , true ' it's important that you don't do this in 6 lines
     sleep(1)

    call hToolbarSelect("FormDesignTools",true)     

    Kontext "DocumentCalc"
        DocumentCalc.MouseDown(50,50)
        DocumentCalc.MouseUp(50,50)
    
    Kontext "FormDesignTools"
        ShowFmExplorer.Click

    Kontext "FormularNavigator" 
   	 Liste.TypeKeys "<HOME>"
     sleep(1)
     Liste.TypeKeys "<DOWN>"
     sleep(1)
     Liste.TypeKeys "<DOWN>"
     sleep(1)
     
     Liste.OpenContextmenu
     sleep(1)
     call hMenuSelectNr(5)
     sleep(1)
    
    'Kontext "FormControls"    
    ' try
    '    ControlProperties.Click
    '    sleep(1)
    ' catch 
    '    warnlog "error while open the control properties browser"
    '    call hCloseDocument
    '    goto endsub
    ' endcatch
    '/// check if the values are saved
    printlog "check if the values are saved"
	sleep(2)
    Kontext "ControlPropertiesTabControl"
        ControlPropertiesTabControl.SetPage TabDataControl
        sleep(1)
    if fgetControlProperty("TextBox","CellBound") <> "A1" then
        warnlog "the bound cell is not saved"
        call hCloseDocument
        goto endsub
    endif            
    sleep(1)    	
    
    call hToolbarSelect("FormControls",true)
    
    Kontext "FormControls"
        SwitchControlDesignMode.Click
        sleep(1)

    Kontext "DocumentCalc"
        Documentcalc.TypeKeys "<MOD1 F5>", true
        sleep(1)
        Documentcalc.TypeKeys "<MOD1 HOME>", true
        sleep(1)
        Documentcalc.TypeKeys "<MOD1 SHIFT END>", true
        sleep(1)
        Documentcalc.TypeKeys "<MOD1 C>", true
        sleep(1)
        printlog getClipboard
        if getClipboard <> "Hello world" then
            warnlog "the cell bound doesn't work"
        endif
    hCloseDocument
    gApplication = "WRITER"

endcase
'--------------------------------------------------------------------------------------
testcase tCheckBox     
    
    '/// Open a new document,
    gApplication = "CALC"
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentCalc"	
	
    call hToolbarSelect("FormControls",true)

  	Kontext "FormControls"   	
   	CheckBox.Click
   	sleep(3)      
    '/// insert a checkbox box
    printlog "insert a checkbox box"                        
    call hDrawingWithSelection ( 50, 50, 70, 70 )
	sleep(2)

    call hToolbarSelect("FormControls",true)
	
  	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    '/// set multiline and alignment to right            	
	call fsetControlProperty("CheckBox","CellBound","A1")
    call fsetControlProperty("CheckBox","ReferenceValueOff","unchecked")
    call fsetControlProperty("CheckBox","ReferenceValue","checked")
    '/// close control property browser
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.Close
    '/// save the document
    printlog "save the document"
	hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_CheckBox.ods"))
	hCloseDocument    
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_CheckBox.ods"))    
    
    call hToolbarSelect("FormControls",true)    

  	Kontext "FormControls"
    SwitchControlDesignMode.Click
    sleep(1)
    'Kontext "DocumentCalc"
    ' DocumentCalc.typeKeys "hello world" , true ' this is important to get the focus into the document

    call hToolbarSelect("FormDesignTools",true)
    
    Kontext "FormDesignTools"
        ShowFmExplorer.Click    
    
    Kontext "FormularNavigator" 
   	 Liste.TypeKeys "<HOME>"
     sleep(1)
     Liste.TypeKeys "<DOWN>"
     sleep(1)
     Liste.TypeKeys "<DOWN>"
     sleep(1)
     Liste.OpenContextmenu
     sleep(1)
     call hMenuSelectNr(5)
     sleep(1)
     
    'Kontext "FormControls"    
    ' try
    '    ControlProperties.Click
    ' catch 
    '    warnlog "error while open the control properties browser"
    '    call hCloseDocument
    '    goto endsub
    ' endcatch
    
    '/// check if the values are saved
    printlog "check if the values are saved"
	sleep(2)
    Kontext "ControlPropertiesTabControl"
        ControlPropertiesTabControl.SetPage TabGeneralControl
    Kontext "TabGeneralControl"    
	TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
    if fgetControlProperty("CheckBox","CellBound") <> "A1" then
        warnlog "the bound cell is not saved"
        call hCloseDocument
        goto endsub
    endif            
    sleep(1)    	
    
    call hToolbarSelect("FormControls",true)
    Kontext "FormControls"
        SwitchControlDesignMode.Click        
        sleep(1)
    
    Kontext "DocumentCalc"
        DocumentCalc.MouseDown(20,20)
        DocumentCalc.MouseUp(20,20)
        
        sleep(1)
        
    Kontext "DocumentCalc"
        Documentcalc.TypeKeys "<MOD1 F5>", true
        sleep(1)
        Documentcalc.TypeKeys "<SPACE>", true
        sleep(2)
        
        ' need to activate the document again        
        Documentcalc.MouseDown(10,10)
        Documentcalc.MouseUp(10,10)
        
        sleep(2)
        
        DocumentCalc.TypeKeys "<MOD1 HOME>" , true

        sleep(1)
        
        Documentcalc.TypeKeys "<MOD1 C>", true

        sleep(1)
        
        Dim str2 as String
        str2 = getClipboardText()
        printlog str2
        ' use STRCOMP to compare the strings thats works.
        if StrComp(str2,"checked") <> 0 then
            printlog StrComp(str2,"checked")
            warnlog "the cell bound doesn't work"
        endif
    hCloseDocument
    gApplication = "WRITER"

endcase
'-------------------------------------------------------------------------
function preCon_CloseFormDesignToolbar
    
    printlog "preCondition close FormDesignToolbar"
    call hNewDocument
    call hToolbarSelect("FormDesignTools",false)
    call hCloseDocument
    call ExitRestartTheOffice

end function
