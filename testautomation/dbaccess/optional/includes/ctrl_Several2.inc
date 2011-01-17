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
'* short description : Control Serveral Part 2 (more controls) - check each control and each property
'*
'\***********************************************************************

sub ctrl_Several2
	
    call fSwitchAutopilotOff(true)  ' has to run first to switch off the autopilot
    
    call tSpinButton       ' tested properties: Repeat, Name, Symbol color (disabled bug:i39944), Background color
    call tScrollBar        ' tested properties: Scroll value min, Scroll value max, Default Scroll value, Orientation, Line increment (small change), Block increment (large change), Delay, Visible Size
    call tImageButton      ' tested properties: Scale, Frame (disabled bug:i39970)
    call tImageControl     ' tested properties: Help URL
    call tFileControl      ' tested properties: Border, Tabstop
    call tDateField        ' tested properties: Date max, Date min, Date format, Dropdown
    call tTimeField        ' tested properties: Time min, Time max, Time format
    call tNumericField     ' tested properties: Spin Button, Strict format, Thousands Seperator, Value step
    call tCurrencyField    ' tested properties: Inc/dec value, Prefix symbol, Currency symbol, Accuracy
    call tPatternField     ' tested properties: Read only, Edit mask, Literal mask, Order
    call tGroupBox         ' tested properties: Character set, Label
    call tTableControl     ' tested properties: Navigation bar, Record marker, Row high
    call tNavigationBar    ' tested properties: IconSize, Positioning, Navigation, RecordActions, FilteringSorting
    
    call fSwitchAutopilotOff(false)
    
end sub


'--------------------------------------------------------------------------------------

testcase tSpinButton
	
    '/// Open a new document,
        printlog "Open a new document,"
        call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)    
        call addControl ( "SpinButton", 50, 20, 60, 30 )
        sleep(2)
    '/// open the control property browser
	call fOpenControlPropertyBrowser
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: Repeat, Name
        printlog "set properties"  
        call fsetControlProperty("ScrollBar","Repeat","1")           'listbox property: no
        call fsetControlProperty("ScrollBar","NameText","Changed")
        call fsetControlProperty("ScrollBar","Background","55")
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_Spinbutton.odt"))
	hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Spinbutton.odt"))
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
        call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    
    call fOpenControlPropertyBrowser     
        
    '/// check if inserted property values are saved right
    printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true           
    if fgetControlProperty("Spinbutton","Repeat") <> "1" then
        warnlog "the spinbutton is not saved with property: Repeat"
        bScrOK = false
    endif
    if fgetControlProperty("Spinbutton","NameText") <> "Changed" then
        warnlog "the spinbutton is not saved with property: NameText"
        bScrOK = false
    endif
    if fgetControlProperty("Spinbutton","Background") <> "55" then
        warnlog "the spinbutton is not saved with property: Background"
        bScrOK = false
    endif
    if bScrOK = true then printlog "Spinbutton was saved correctly"
        sleep(1)  
    
    call closePropertyBrowserAndDocument()
       
endcase
'--------------------------------------------------------------------------------------

testcase tScrollBar
	
    dim sMem as string      'comparing variable
    
    '/// Open a new document,
        printlog "Open a new document,"
        call hNewDocument
    Kontext "DocumentWriter"       
        DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)       	
   	    call addControl("ScrollBar", 50, 20, 60, 30 )        
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser	    
        
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: Scroll value min, Scroll value max, Default Scroll value, Orientation, Line increment, Block increment, Delay, symbol color, Visible Size
        printlog "set properties"  
        call fsetControlProperty("ScrollBar","ScrollValueMin","10")
        call fsetControlProperty("ScrollBar","ScrollValueMax","90")
        call fsetControlProperty("ScrollBar","ScrollValueDefault","5")
        call fsetControlProperty("ScrollBar","Orientation","2")           'listbox property: vertical
        call fsetControlProperty("ScrollBar","LineIncrement","10")
        call fsetControlProperty("ScrollBar","BlockIncrement","25")
        call fsetControlProperty("ScrollBar","Delay","100")
        TabGeneralControl.TypeKeys "<RETURN>" , true            'an ms is added to 100 -> 100ms
        sMem = fgetControlProperty("ScrollBar","Delay")    
        call fsetControlProperty("ScrollBar","VisibleSize","10")    
    
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_Scrollbar.odt"))
    hCloseDocument    
    
    '/// reload the document
    printlog "reload the document"
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Scrollbar.odt"))
    
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
        call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
        
    call fOpenControlPropertyBrowser    
    
    '/// check if inserted property values are saved right
        printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true           
    if fgetControlProperty("ScrollBar","ScrollValueMin") <> "10" then
        warnlog "the scrollbar is not saved with property: ScrollValueMin"
        bScrOK = false
    endif
    if fgetControlProperty("ScrollBar","ScrollValueMax") <> "90" then
        warnlog "the scrollbar is not saved with property: ScrollValueMax"
        bScrOK = false
    endif
    if fgetControlProperty("ScrollBar","ScrollValueDefault") <> "5" then
        warnlog "the scrollbar is not saved with property: ScrollValueDefault"
        bScrOK = false
    endif
    if fgetControlProperty("ScrollBar","Orientation") <> "2" then
        warnlog "the scrollbar is not saved with property: Orientation"
        bScrOK = false
    endif
    if fgetControlProperty("ScrollBar","LineIncrement") <> "10" then
        warnlog "the scrollbar is not saved with property: LineIncrement"
        bScrOK = false
    endif
    if fgetControlProperty("ScrollBar","BlockIncrement") <> "25" then
        warnlog "the scrollbar is not saved with property: BlockIncrement"
        bScrOK = false
    endif
    if fgetControlProperty("ScrollBar","Delay") <> sMem then
        warnlog "the scrollbar is not saved with property: Delay"
        bScrOK = false
    endif
    if fgetControlProperty("ScrollBar","VisibleSize") <> "10" then
        warnlog "the scrollbar is not saved with property: VisibleSize"
        bScrOK = false
    endif
    if bScrOK = true then printlog "Scrollbar was saved correctly"
        sleep(1)   
    
    call closePropertyBrowserAndDocument()
        
endcase
'--------------------------------------------------------------------------------------

testcase tImageButton
    
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        
	call addControl("ImageButton", 50, 20, 60, 30 )
        
	printlog "open the control property browser"
    call fOpenControlPropertyBrowser
    
    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties
         printlog "set properties" 
    'to activate property: scale, following properties must be active: Graphics (with choosen path)
    Kontext "TabGeneralControl"	
    fsetControlProperty("ImageButton","Graphics", gOfficePath + ConvertPath("share/gallery/bullets/blkpearl.gif"))
    
    sleep(1)
        TabGeneralControl.TypeKeys "<RETURN>" , true
        fsetControlProperty("ImageControl","Scale","1")          'listbox content: no   
        'fsetControlProperty("ImageControl","Frame","4")
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_ImageButton.odt"))
	hCloseDocument
        sleep(1)
    '/// reload and prepare the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_ImageButton.odt"))
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
        call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if property values are inserted
        printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true   
    if fgetControlProperty("ImageButton","Scale") <> "1" then
        warnlog "the imagebutton is not saved with property: Frame"
        bScrOK = false
    endif        
    'if fgetControlProperty("ImageButton","Frame") <> "4" then
        'warnlog "the imagebutton is not saved with property: Frame"
        'bScrOK = false
    'endif
    if bScrOK = true then printlog "Imagebutton was saved correctly"
        
    call closePropertyBrowserAndDocument()
           
endcase
'--------------------------------------------------------------------------------------
testcase tImageControl     
    
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
    
    call addControl("ImageControl", 50, 20, 60, 30 )
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set HelpURL 
        printlog "set HelpURL"
        fsetControlProperty("ImageControl","HelpURL","customcontrol2")
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_ImageControl.odt"))
	hCloseDocument
        sleep(1)
    '/// reload and prepare the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_ImageControl.odt"))
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
        call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if property values are inserted
        printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true           
    if fgetControlProperty("ImageControl","HelpURL") <> "customcontrol2" then
        warnlog "the imagecontrol is not saved with property: HelpURL"
        bScrOK = false
    endif
    if bScrOK = true then printlog "Imagecontrol was saved correctly"
        sleep(1)   

     call closePropertyBrowserAndDocument()
   
endcase
'--------------------------------------------------------------------------------------

testcase tFileControl
	
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        
    call addControl("FileControl", 50, 20, 60, 30 )
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: border to flat, tabstop to no    
        printlog "set border to flat"        	
        fsetControlProperty("FileControl","Border","3")	
        fsetControlProperty("FileControl","TabStop","1")
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_FileControl.odt"))
	hCloseDocument
        sleep(1)
    '/// reload the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_FileControl.odt"))    
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if the values are saved
        printlog "check if the values are saved"
        sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't know why but it is.
    if fgetControlProperty("FileControl","Border") <> "3" then
        warnlog "the value border is not saved"
    endif  
    if fgetControlProperty("FileControl","TabStop") <> "1" then
        warnlog "the value border is not saved"
    endif  
    
    call closePropertyBrowserAndDocument()
    
endcase
'--------------------------------------------------------------------------------------

testcase tDateField
	
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        
    call addControl("DateField", 50, 20, 60, 30 )
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set DateMin to 09/09/2009 and DateMax to 10/10/2010
        printlog "set DateMin to 09/09/2009 and DateMax to 10/10/2010"  
    if iSystemSprache = 01 then      	
	    fsetControlProperty("DateField","DateMin","09/09/2009")
        fsetControlProperty("DateField","DateMax","10/10/2010")
    elseif iSystemSprache = 49 then
        fsetControlProperty("DateField","DateMin","09.09.2009")
        fsetControlProperty("DateField","DateMax","10.10.2010")
    endif  
    '/// set date format, dropdown
        printlog "set date format, date format"
        fsetControlProperty("DateField","DateFormat","7")        'YY/MM/DD
        printlog "set date format, dropdown"
        fsetControlProperty("DateField","DropDown","2")          'yes
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_DateField.odt"))
	hCloseDocument
        sleep(1)
    '/// reload the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_DateField.odt"))    
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode 
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if the values are saved
        printlog "check if the values are saved"
        sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
    if iSystemSprache = 01 then
        if fgetControlProperty("DateField","DateMin") <> "09/09/2009" then
            warnlog "the value date min is not saved"
        endif
        if fgetControlProperty("DateField","DateMax") <> "10/10/2010" then
            warnlog "the value date max is not saved"
        endif
    elseif iSystemSprache = 49 then
        if fgetControlProperty("DateField","DateMin") <> "09.09.2009" then
            warnlog "the value date min is not saved"
        endif
        if fgetControlProperty("DateField","DateMax") <> "10.10.2010" then
            warnlog "the value date max is not saved"
        endif
    endif
    if fgetControlProperty("DateField","DateFormat") <> "7" then
        warnlog "the value date format is not saved"
    endif  
    if fgetControlProperty("DateField","DropDown") <> "2" then
        warnlog "the value dropdown is not saved"
    endif  
        sleep(1)  
        printlog " - testcase end"
    
    call closePropertyBrowserAndDocument()
        
endcase
'--------------------------------------------------------------------------------------

testcase tTimeField
	
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        
    call addControl("TimeField", 50, 20, 60, 30 )
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set TimeMin to 09:09:09 and TimeMax to 10:10:10 and time format to 01:45 PM
        printlog "set TimeMin to 09:09:09 and TimeMax to 10:10:10 and time format to 01:45 PM"        	
        fsetControlProperty("TimeField","TimeMin","09:09:09")
        fsetControlProperty("TimeField","TimeMax","10:10:10") 
        fsetControlProperty("TimeField","TimeFormat","3")
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_TimeField.odt"))
	hCloseDocument
        sleep(1)
    '/// reload the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_TimeField.odt"))    
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if the values are saved
        printlog "check if the values are saved"
        sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
    if fgetControlProperty("TimeField","TimeMin") <> "09:09:09" then
        warnlog "the value time min is not saved"
    endif
    if fgetControlProperty("TimeField","TimeMax") <> "10:10:10" then
        warnlog "the value time max is not saved"
    endif
    if fgetControlProperty("TimeField","TimeFormat") <> "3" then
        warnlog "the value time format is not saved"
    endif
        sleep(1)  
        printlog " - testcase end"
    
    call closePropertyBrowserAndDocument()
        
endcase
'--------------------------------------------------------------------------------------

testcase tNumericField
	
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        
    call addControl("NumericField", 50, 20, 60, 30 )
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: spinbutton, Strict format, Thousands seperator
        printlog "set properties: spinbutton, Default value, Strict format, Tabstop, Thousands seperator"   
        fsetControlProperty("NumericField","StrictFormat","2")           'yes
        fsetControlProperty("NumericField","ThousandSeperator","2")      'yes
        fsetControlProperty("NumericField","Spin","2")                   'yes
        fsetControlProperty("NumericField","ValueStep","6") 
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_NumericField.odt"))
	hCloseDocument
        sleep(1)
    '/// reload and prepare the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_NumericField.odt"))
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode 
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if property values are inserted
        printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true           
    if fgetControlProperty("NumericField","StrictFormat") <> "2" then
        warnlog "the NumericField is not saved with property: StrictFormat"
        bScrOK = false
    endif
    if fgetControlProperty("NumericField","ThousandSeperator") <> "2" then
        warnlog "the NumericField is not saved with property: ThousandSeperator"
        bScrOK = false
    endif
    if fgetControlProperty("NumericField","Spin") <> "2" then
        warnlog "the NumericField is not saved with property: Spin"
        bScrOK = false
    endif
    '(spin field - cannot be read out)
    'if fgetControlProperty("NumericField","ValueStep") <> "2" then
        'warnlog "the NumericField is not saved with property: Value Step"
        'bScrOK = false
    'endif
    if bScrOK = true then printlog "Numeric field was saved correctly"
        sleep(1)   
        
    call closePropertyBrowserAndDocument()
          
endcase
'--------------------------------------------------------------------------------------

testcase tCurrencyField
	
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
    
    call addControl("CurrencyField", 50, 20, 60, 30 )
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set valueStep to 5 and prefix symbol to yes and current symbol to zoq
        printlog "set valueStep to 5 and prefix symbol to yes and current symbol to zoq"       
        fsetControlProperty("CurrencyField","CurrSymPosition","2")    
        fsetControlProperty("CurrencyField","ValueStep","5")
        fsetControlProperty("CurrencyField","CurrencySymbol","nin")
        fsetControlProperty("CurrencyField","Accuracy","3")
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_CurrencyField.odt"))
	hCloseDocument
        sleep(1)
    '/// reload the document
        printlog "reload the document"
        hFileOpen ( gOfficePath + ConvertPath("user/work/TT_CurrencyField.odt"))    
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if the values are saved
        printlog "check if the values are saved"
        sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
    if fgetControlProperty("CurrencyField","CurrSymPosition") <> "2" then    
        warnlog "the prefix symbol is not saved"
    endif    
    if fgetControlProperty("CurrencyField","ValueStep") <> "5" then
        warnlog "the value step is not saved"
    endif
    if fgetControlProperty("CurrencyField","CurrencySymbol") <> "nin" then
        warnlog "the CurrencySymbol is not saved"
    endif
    '(spin field - cannot be read out)
    'if fgetControlProperty("CurrencyField","Accuracy") <> "3" then
        'warnlog "the CurrencySymbol is not saved"
    'endif
        sleep(1)  
        printlog " - testcase end"
        
    call closePropertyBrowserAndDocument()
        
endcase
'--------------------------------------------------------------------------------------

testcase tPatternField     
    
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
    
    call addControl("PatternField", 50, 20, 60, 30 )
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: Readonly, Edit mask, Literal mask, Order
        printlog "set properties: Readonly, Edit mask, Literal mask, Order"            	
        fsetControlProperty("PatternField","Readonly","2")   
        fsetControlProperty("PatternField","EditMask","oxox")    
        fsetControlProperty("PatternField","LiteralMask","ooxoo")    
        fsetControlProperty("PatternField","Order","7")  
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_PatternField.odt"))
	hCloseDocument
        sleep(1)
    '/// reload the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_PatternField.odt"))    
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if the values are saved
        printlog "check if the values are saved"
        sleep(2)
    Kontext "TabGeneralControl"    
	    TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.
    if fgetControlProperty("PatternField","Readonly") <> "2" then
        warnlog "the value readonly is not saved"
    endif        
    if fgetControlProperty("PatternField","EditMask") <> "oxox" then
        warnlog "the value EditMask is not saved"
    endif  
    if fgetControlProperty("PatternField","LiteralMask") <> "ooxoo" then
        warnlog "the value LiteralMask is not saved"
    endif  
    if fgetControlProperty("PatternField","Order") <> "7" then
        warnlog "the value Order is not saved"
    endif  
        sleep(1)  
        printlog " - testcase end"
        
    call closePropertyBrowserAndDocument()
         
endcase
'--------------------------------------------------------------------------------------

testcase tGroupBox
	
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        
    addControl("GroupBox", 50, 20, 60, 30 )
     
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
    '/// set properties: Label, Character set
        printlog "set properties"  
    Kontext "TabGeneralControl"
        TabGeneralControl.TypeKeys "<TAB>",TRUE                 'label
        TabGeneralControl.TypeKeys "Changed label name",TRUE
        CharacterSetButton.Click                                'character set
        sleep(1)
    Kontext "TabFont"
        TabFont.TypeKeys "<DOWN>",TRUE
        TabFont.OK
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_GroupBox.odt"))
	hCloseDocument
        sleep(1)
    '/// reload and prepare the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_GroupBox.odt"))
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if property values are inserted
        printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        TabGeneralControl.TypeKeys "<TAB>" , true    
        TabGeneralControl.TypeKeys "<MOD1 C>" , true
    if getclipboard() <> "Changed label name" then
        warnlog "Property:  Label is not saved correctly"
    endif    
    'printlog "property character set text: " & CharacterSet.getText
    if CharacterSet.getText = "" then
        warnlog "Property:  Character set is not saved correctly"
    endif
        sleep(1)
        printlog " - testcase end"
       
    call closePropertyBrowserAndDocument()
        
endcase
'--------------------------------------------------------------------------------------

testcase tTableControl
    
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        
    call addControl("Grid", 50, 20, 60, 30 )
     
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: Navigation bar, Record marker, Row high
        printlog "set properties" 
        fsetControlProperty("TableControl","NavigationBar","1")      'listbox content: no
        fsetControlProperty("TableControl","RecordMarker","1")       'listbox content: no
        fsetControlProperty("TableControl","RowHeight","5")          
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_TableControl.odt"))
	hCloseDocument
        sleep(1)
    '/// reload and prepare the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_TableControl.odt"))
    Kontext "Toolbar"
	    call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode  
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if property values are inserted
        printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true           
    if fgetControlProperty("TableControl","NavigationBar") <> "1" then
        warnlog "the imagebutton is not saved with property: NavigationBar"
        bScrOK = false
    endif
    if fgetControlProperty("TableControl","RecordMarker") <> "1" then
        warnlog "the imagebutton is not saved with property: RecordMarker"
        bScrOK = false
    endif
    if fgetControlProperty("TableControl","RowHeight") = "" then
        warnlog "the imagebutton is not saved with property: RowHeight"
        bScrOK = false
    endif
    if bScrOK = true then printlog "TableControl was saved correctly"
        sleep(1)  
        
    call closePropertyBrowserAndDocument()
          
endcase
'--------------------------------------------------------------------------------------

testcase tNavigationBar
    
    '/// Open a new document,
        printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
    
    call addControl("NavigationBar", 50, 20, 60, 30 )
    
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"   
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)
    '/// set properties: IconSize, Positioning, Navigation, RecordActions, FilteringSorting
        printlog "set properties" 
        fsetControlProperty("NavigationBar","IconSize","2")          'listbox content: large
        fsetControlProperty("NavigationBar","Positioning","1")       'listbox content: no
        fsetControlProperty("NavigationBar","Navigation","1")        'listbox content: no
        fsetControlProperty("NavigationBar","RecordActions","1")     'listbox content: no
        fsetControlProperty("NavigationBar","FilteringSorting","1")  'listbox content: no
    '/// save the document
        printlog "save the document"
        hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_NavigationBar.odt"))
	hCloseDocument
        sleep(1)
    '/// reload and prepare the document
        printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_NavigationBar.odt"))
        Kontext "Toolbar"
        call hToolbarSelect("MoreControls",true)
        call hToolbarSelect("FormControls",true)
  	call fSwitchControlDesignMode 
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<SHIFT F4>"
        sleep(1)
    call fOpenControlPropertyBrowser
    '/// check if property values are inserted
        printlog "check if property values are inserted"
    Kontext "TabGeneralControl"	
        dim bScrOK as boolean       'control variable if all properties are correct
        bScrOK = true           
    if fgetControlProperty("NavigationBar","IconSize") <> "2" then
        warnlog "the NavigationBar is not saved with property: IconSize"
        bScrOK = false
    endif
    if fgetControlProperty("NavigationBar","Positioning") <> "1" then
        warnlog "the NavigationBar is not saved with property: Positioning"
        bScrOK = false
    endif
    if fgetControlProperty("NavigationBar","Navigation") <> "1" then
        warnlog "the NavigationBar is not saved with property: Navigation"
        bScrOK = false
    endif
    if fgetControlProperty("NavigationBar","RecordActions") <> "1" then
        warnlog "the NavigationBar is not saved with property: RecordActions"
        bScrOK = false
    endif
    if fgetControlProperty("NavigationBar","FilteringSorting") <> "1" then
        warnlog "the NavigationBar is not saved with property: FilteringSorting"
        bScrOK = false
    endif
    if bScrOK = true then printlog "NavigationBar was saved correctly"
        sleep(1)   
        
    call closePropertyBrowserAndDocument()
           
endcase
'--------------------------------------------------------------------------------------
'--------------------------------------------------------------------------------------

   

'make shure that design mode is not off, else switch on
function fSwitchOn_DesignMode
Kontext "FormControls"
        try
            PushButton.Click    
        catch 
            SwitchControlDesignMode.Click
            sleep(2)
        endcatch
end function 
