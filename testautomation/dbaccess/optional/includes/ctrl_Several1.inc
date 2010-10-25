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
'* short description : Control Serveral Part 1 - check each control and each property
'*
'\***********************************************************************

sub ctrl_Several1
	
    call fSwitchAutopilotOff(true) ' has to run first to switch off the autopilot
    
    call tCheckBox         ' tested properties: Tristate, Enabled, Label field
    call tTextBox          ' tested properties: Background color, Additional Info, Scrollbars, Password
    call tFormattedField   ' tested properties: Value max, Value min, Formatting
    call tPushButton       ' tested properties: Graphic, Alignment, Default button, Toggle, Take Focus
    call tOptionButton     ' tested properties: Print, Reference value, Default status
    call tListBox          ' tested properties: Line count, Multiselection, List Entries, Default selection
    call tComboBox         ' tested properties: Auto fill, Max text length, Hide selection, Border color
    call tLabelField       ' tested properties: Alignment, Help text
   
    call fSwitchAutopilotOff(false)
    
end sub

'--------------------------------------------------------------------------------------
' todo: testcase: tTextBox and tLabelFild contain 'multiline' settings, but HelpID disappeared - has to be reworked
'--------------------------------------------------------------------------------------

testcase tCheckBox
	
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
        
	Kontext "FormControls"
    addControl("Label", 40, 30, 50, 40)
    addControl("CheckBox", 55, 30, 65, 40 )
        
    call fOpenControlPropertyBrowser
    
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: Label - therefore needed: a present label field
    printlog "set properties: Label - therefore needed: a present label field"  
    Kontext "TabGeneralControl"	
        LabelFieldButton.Click
        sleep(1)
    Kontext "LabelFieldSelection"	
        LabelFieldSelection.TypeKeys "<DOWN>",TRUE
        LabelFieldSelection.TypeKeys "<DOWN>",TRUE
        sleep(1)
        LabelFieldSelection.OK
        sleep(1)
    '/// set properties: Tristate, Enabled
	    call fSetControlProperty("CheckBox","TriState","2") 
	    sleep(1)  
        call fSetControlProperty("CheckBox","Enabled","1")        'listbox property: no
   	    sleep(1)  
    '/// close control property browser    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
        sleep(1)
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_CheckBox.odt"))
	hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_CheckBox.odt"))    
    
    call fSwitchControlDesignMode()
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
        DocumentWriter.TypeKeys "<TAB>" ' select the second control
        sleep(1)
        
    call fOpenControlPropertyBrowser
	 
    '/// check if the values are saved
    printlog "check if the values are saved"
    sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
	    sleep(1)
        
    if fGetControlProperty("CheckBox","TriState") <> "2" then
        warnlog "the value 'tristate' is not saved correctly"
    endif
    
    if fGetControlProperty("CheckBox","Enabled") <> "1" then
        warnlog "the value 'enabled' is not saved correctly"
    endif
    
    printlog "property character set text: " & LabelField.getText
    if LabelField.getText = "" then
        warnlog "the value 'label' is not saved correctly"
    endif
            
    call closePropertyBrowserAndDocument()
    
endcase
'--------------------------------------------------------------------------------------

testcase tTextBox     
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
  	     
	Kontext "FormControls"
    addControl("Edit", 40, 40, 30, 30)    
        
    call fOpenControlPropertyBrowser
    
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: Background color, Additional Info, Scrollbars, Password
        printlog "set properties"  
        call fSetControlProperty("TextBox","Background","92") 
        call fSetControlProperty("TextBox","AdditionalInfo","my only friend the end.")
        call fSetControlProperty("TextBox","TextType","2")      'needed for activate scrollbar
        call fSetControlProperty("TextBox","Scrollbars","4")       
        call fSetControlProperty("TextBox","TextType","1")      'needed for activate password
        call fSetControlProperty("TextBox","Password","p")
    '/// close control property browser    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_TextBox.odt"))
	hCloseDocument
        sleep(1)
    '/// reload the document
    printlog "reload the document"        
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_TextBox.odt"))    
    
    call fSwitchControlDesignMode()   
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
        
    call fOpenControlPropertyBrowser

    '/// check if the values are saved
    printlog "check if the values are saved"
    sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
    
    if fGetControlProperty("TextBox","Background") <> "92" then
        warnlog "the value: background is not saved"
    endif  
    
    if fGetControlProperty("TextBox","AdditionalInfo") <> "my only friend the end." then
        warnlog "the AdditionalInfo value is not saved"
    endif
    call fSetControlProperty("TextBox","TextType","2")      'needed for activate scrollbar
    sleep(1)
    if fGetControlProperty("TextBox","Scrollbars") <> "4" then
        warnlog "the value: Scrollbars are not saved"
    endif       
    call fSetControlProperty("TextBox","TextType","1")      'needed for activate password
    sleep(1)
    if fGetControlProperty("TextBox","Password") <> "p" then
        warnlog "the value: Password is not saved"
    endif   
    
    call closePropertyBrowserAndDocument()        
     
endcase
'--------------------------------------------------------------------------------------

testcase tFormattedField     
    
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
        
        Kontext "FormControls"
    addControl("FormattedField", 50, 20, 60, 30)    
        
    call fOpenControlPropertyBrowser         	
   	    
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set ValueMin to 2 and ValueMax to 9 and any Formatting
        printlog "set ValueMin to 2 and ValueMax to 9 and any Formatting"    
    Kontext "TabGeneralControl"     'set any formatting
        FormatkeyButton.Click
    Kontext "TabZahlen"
        TabZahlen.TypeKeys "<DOWN>", true
        TabZahlen.TypeKeys "<RETURN>", true   
        sleep(1)    	
        fsetControlProperty("FormattedField","ValueMax","9")
        sleep(1)            
        fsetControlProperty("FormattedField","ValueMin","2")
    '/// close control property browser    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_FormattedField.odt"))
	hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_FormattedField.odt"))    
    
    call fSwitchControlDesignMode()

    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
        
    call fOpenControlPropertyBrowser
    
    '/// check if the values are saved
        printlog "check if the values are saved"
        sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
    if fgetControlProperty("FormattedField","ValueMax") = "" then
        warnlog "the value ValueMax is not saved"
    endif
    if fgetControlProperty("FormattedField","ValueMin") = "" then
        warnlog "the value ValueMin is not saved"
    endif      
    if fgetControlProperty("FormattedField","Formatkey") = "" then
        warnlog "the value ValueMin is not saved"
    endif
    
    call closePropertyBrowserAndDocument()
          
endcase
'--------------------------------------------------------------------------------------

testcase tPushButton
	
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)        
        
    Kontext "FormControls"
    addControl("PushButton", 50, 20, 60, 30)    
        
    call fOpenControlPropertyBrowser
        
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set a graphic for this button
    printlog "set a graphic for this button"	
	Kontext "TabGeneralControl"
    call fSetControlProperty("PushButton","Graphics", gOfficePath + ConvertPath("share/gallery/bullets/blkpearl.gif"))
    '/// set further properties: default button, toggle, graphics alignment
    printlog "set further properties"  
    call fSetControlProperty("PushButton","DefaultButton","2")           'listbox property: yes
    call fSetControlProperty("PushButton","ToggleProp","2")                  'listbox property: yes
    call fSetControlProperty("PushButton","GraphicsAlignment","5")
    call fSetControlProperty("PushButton","TakeFocus","1")       
    '/// close control property browser    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    '/// save the document
    printlog "save the document"    
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_PushButton.odt"))
    hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_PushButton.odt"))
    '/// check if the graphic still displayed
    printlog "check if the graphic still displayed"
    call fSwitchControlDesignMode()
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    
    call fOpenControlPropertyBrowser
        
    '/// check if everything is saved right
        printlog "check if everything is saved right"
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true
        dim sGraphics as String     'variable for graphics path
    'check if the graphic is saved
        sGraphics = fGetControlProperty("PushButton","Graphics")
    if Right(sGraphics,12) <> "blkpearl.gif" then
        warnlog "The graphics file name is not saved"
        bScrOK = false
    endif
    if fGetControlProperty("PushButton","GraphicsAlignment") <> "5" then
        warnlog "the pushbutton is not saved with property: Graphics Alignment"
        bScrOK = false
    endif
    'check if the default button is saved
    if fGetControlProperty("PushButton","DefaultButton") <> "2" then
        warnlog "the pushbutton is not saved with property: Default Button"
        bScrOK = false
    endif
    if fGetControlProperty("PushButton","ToggleProp") <> "2" then
        warnlog "the pushbutton is not saved with property: Toggle"
        bScrOK = false
    endif
    if fGetControlProperty("PushButton","TakeFocus") <> "1" then
        warnlog "the pushbutton is not saved with property:  Take Focus on click"
        bScrOK = false
    endif
        sleep(1)	
    if bScrOK = true then printlog "Pushbutton was saved correctly"
    
    call closePropertyBrowserAndDocument()
    
endcase
'--------------------------------------------------------------------------------------

testcase tOptionButton
	
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
          
    Kontext "FormControls"
    addControl("RadioButton", 50, 20, 60, 30)    
        
    call fOpenControlPropertyBrowser
        
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: Print, Reference value, Default status
        printlog "set properties"  
        call fSetControlProperty("Radiobutton","Printable","1")         'listbox property: no
        call fSetControlProperty("Radiobutton","ReferenceValue","777")
        call fSetControlProperty("Radiobutton","DefaultState","2")      'listbox property: selected
    '/// close control property browser    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_Optionbutton.odt"))
	hCloseDocument
    sleep(1)
    
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Optionbutton.odt"))
    
    call fSwitchControlDesignMode()
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
        call fOpenControlPropertyBrowser
    
    '/// check if property values are inserted
    printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true           
    if fGetControlProperty("Radiobutton","Printable") <> "1" then
        warnlog "the Optionbutton is not saved with property: Print"
        bScrOK = false
    endif
    if fGetControlProperty("Radiobutton","ReferenceValue") <> "777" then
        warnlog "the Optionbutton is not saved with property: ReferenceValue"
        bScrOK = false
    endif
    if fGetControlProperty("Radiobutton","DefaultState") <> "2" then
        warnlog "the Optionbutton is not saved with property: ReferenceValue"
        bScrOK = false
    endif
    if bScrOK = true then printlog "Optionbutton was saved correctly"
    
    call closePropertyBrowserAndDocument()
    
endcase
'--------------------------------------------------------------------------------------

testcase tListBox
	
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	    sleep(1)
    
    Kontext "FormControls"
    addControl("ListBox", 50, 20, 60, 30)    
        
    call fOpenControlPropertyBrowser
        
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: default selection, line count, list entries, multiselection
    printlog "set properties"  
    Kontext "TabGeneralControl"	
    'set property: list entries
        call fSetControlProperty("ListBox","ListEntries","My only friend the end.")
    'set property: line count
        call fSetControlProperty("ListBox","DropDown","2")    'needed for activate line count
        sleep (1)
        call fSetControlProperty("ListBox","LineCount","10")
        sleep (1)
    'set property: multiselection
        call fSetControlProperty("ListBox","Multiselection","2")    'yes
        sleep (1)
    'set property: default selection    
        call fSetControlProperty("ListBox","DefaultSelection2","88")
    '/// close control property browser    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_ListBox.odt"))
	hCloseDocument
    sleep(1)
    
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_ListBox.odt"))
    
    call fSwitchControlDesignMode()
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    
    call fOpenControlPropertyBrowser
    
    '/// check if property values are inserted
        printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true  
    '(special field - cannot be read out)
    'if fGetControlProperty("ListBox","ListEntries") = "" then
        'warnlog "the listbox is not saved with property: ListEntries"
        'bScrOK = false
    'endif
    '(special field - cannot be read out)
    'if fGetControlProperty("ListBox","DefaultSelection2") = "" then
        'warnlog "the listbox is not saved with property: DefaultSelection2"
        'bScrOK = false
    'endif 
    if fGetControlProperty("ListBox","LineCount") <> "10" then
        warnlog "the listbox is not saved with property: LineCount"
        bScrOK = false
    endif
    if fGetControlProperty("ListBox","Multiselection") <> "2" then
        warnlog "the listbox is not saved with property: Multiselection"
        bScrOK = false
    endif
    if bScrOK = true then printlog "listbox was saved correctly"
            
    call closePropertyBrowserAndDocument()
        
endcase
'--------------------------------------------------------------------------------------

testcase tComboBox
	
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
        
    Kontext "FormControls"
    addControl("ComboBox", 50, 20, 60, 30)    
        
    call fOpenControlPropertyBrowser
        
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: Auto fill, Max text length, Hide Selection, Border color
        printlog "set properties"  
    Kontext "TabGeneralControl"	
        call fSetControlProperty("ComboBox","MaxTextLen","6")
        call fSetControlProperty("ComboBox","AutoComplete","1")    'no
        call fSetControlProperty("ComboBox","HideSelection","1")   'no
        call fSetControlProperty("ComboBox","Border","3")          'needed to activate Border Color
        call fSetControlProperty("ComboBox","BorderColor","13")
    '/// close control property browser    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_ComboBox.odt"))
	hCloseDocument
    sleep(1)
    
    '/// reload the document
    printlog "reload the document"        
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_ComboBox.odt"))
    
    call fSwitchControlDesignMode()
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    
    call fOpenControlPropertyBrowser
    
    '/// check if property values are inserted
    printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true  
    if fGetControlProperty("ComboBox","MaxTextLen") <> "6" then
        warnlog "the combobox is not saved with property: MaxTextLen"
        bScrOK = false
    endif
    if fGetControlProperty("ComboBox","AutoComplete") <> "1" then
        warnlog "the combobox is not saved with property: AutoComplete"
        bScrOK = false
    endif
    if fGetControlProperty("ComboBox","HideSelection") <> "1" then
        warnlog "the combobox is not saved with property: Hide Selection"
        bScrOK = false
    endif
    if fGetControlProperty("ComboBox","BorderColor") <> "13" then
        warnlog "the combobox is not saved with property: Border Color"
        bScrOK = false
    endif
    if bScrOK = true then printlog "Combobox was saved correctly"
    
    call closePropertyBrowserAndDocument()
    
endcase
'--------------------------------------------------------------------------------------

testcase tLabelField
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
        
    Kontext "FormControls"
    addControl("Label", 50, 20, 60, 30)    
        
    call fOpenControlPropertyBrowser
        
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: alignment, multiline, help text
        printlog "set properties"  
        call fSetControlProperty("LabelField","Align","4")        'set to right
        call fSetControlProperty("LabelField","HelpText","Filled in Text")    
    '/// close control property browser    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_LabelField.odt"))
	hCloseDocument
    sleep(1)
    
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_LabelField.odt"))    
    
    call fSwitchControlDesignMode()
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    
    call fOpenControlPropertyBrowser
    
    '/// check if the values are saved
        printlog "check if the values are saved"
        sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
    if fGetControlProperty("LabelField","Align") <> "4" then
        warnlog "the value: align is not saved"
    endif  
     if fGetControlProperty("LabelField","HelpText") <> "Filled in Text" then
        warnlog "the value: HelpText is not saved"
    endif  
        
    call closePropertyBrowserAndDocument()
    
endcase
        

