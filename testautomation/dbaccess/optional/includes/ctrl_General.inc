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
'* short description : Data Source Administrator
'*
'\***********************************************************************
sub ctrl_General
	
    call tLoad_Save_Print
    call tAutomaticControlFocus1
    call tAutomaticControlFocus2'
    call tPosAndSize_Protect_Position
    call tPosAndSize_Protect_Size
    call tPosAndSize_Keyboard
    call tPosAndSize_Dialog
    call tPosAndSize_KeepRatio
    'TODO Control the Setting on tabpage Rotation & Slant (should be inactive).
    
    call tTabStop
    call tActivationOrder
    call tAddFields
    call tLabelFieldSelection

end sub

'--------------------------------------------------------------------------------------

testcase tLoad_Save_Print
    
    dim iXMLElementCount as integer
    dim i as integer
	'/// Load the document TT_All_Controls.odt		
    hFileOpen ( gTesttoolPath + ConvertPath("dbaccess/optional/input/TT_All_Controls.odt"))
    'TODO: Are all controlls shown right (color, fonts etc.)?
    '/// save the doc under a diffrent name.
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_All_Controls.odt"))
    '/// Reload & compare it with the origin doc.
    hCloseDocument
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_All_Controls.odt"))
    hCloseDocument
    'TODO: Print the doc & compare it with the origin.
    '(Attention: Background can't be printed colored (Task: <A HREF="http://webserver1.stardiv.de/Bugtracker/Source/Body_ReportDetail.asp?ID=57223">57223</A>)).

endcase    
'--------------------------------------------------------------------------------------
testcase tAutomaticControlFocus1
	
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true)   
	
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(2)      
	'/// insert a Text Boxes
    printlog "insert three Text Boxes"                                                                    
    call hDrawingWithSelection ( 40, 40, 55, 55 ) 
        
    sleep(2)
    
	Kontext "FormControls"
	    ControlProperties.Click            
	sleep(2)	
	
    fSetControlProperty("Edit","DefaultValue","tt_control")
	sleep(1)
    '/// activate the Automatic control focus.
    printlog "activate the Automatic control focus."
	
    call hToolbarSelect("FormDesignTools",true)   
	    
    Kontext "FormDesignTools"
        AutoControlFocus.Click
        
	sleep(2)	
    '/// save the document
    printlog "save the document"
	hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_Control_Test1.odt"))
    sleep(1)
	hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Control_Test1.odt"))
    ' /// check if the control has the focus after reload.
    printlog " check if the second control has the focus after reload."
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<MOD1 HOME>" , true
    sleep(1)       
	DocumentWriter.TypeKeys "<MOD1 SHIFT END>" , true
    sleep(1)
    DocumentWriter.TypeKeys "<MOD1 C>" , true
    sleep(1)
    if GetClipboard <> "tt_control" then
        warnlog "The control has not the focus. The document has the focus." 
    endif
    
    hCloseDocument
    
endcase
'--------------------------------------------------------------------------------------
testcase tAutomaticControlFocus2
	
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true)   
	
  	Kontext "FormControls"   	
   	Edit.Click
   	sleep(1)
	'/// insert three Text Boxes
    printlog "insert three Text Boxes"
    call hDrawingWithSelection ( 50, 20, 60, 30 )
	sleep(2)
	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    fSetControlProperty("Edit","DefaultValue","tt_control1")
	
	call hDrawingWithSelection  ( 50, 40, 60, 50 )
	sleep(2)
    ' open and close the form property browser to get the focus
	Kontext "FormControls"
	ControlProperties.Click
    sleep(2)
    ControlProperties.Click
	sleep(2)	
    
    fSetControlProperty("Edit","TabOrder","1")
    
    ' open and close the property browser
    Kontext "FormControls"
	ControlProperties.Click
    sleep(2)
    ControlProperties.Click
	sleep(2)	
	
    fSetControlProperty("Edit","DefaultValue","tt_control2")

    sleep(1)
    
    call hDrawingWithSelection  ( 50, 60, 60, 70 )
	sleep(2)
    ' open and close the form property browser to get the focus
    Kontext "FormControls"
	ControlProperties.Click
    sleep(2)
    ControlProperties.Click	
	sleep(2)	
    fSetControlProperty("Edit","DefaultValue","tt_control3")
	    
    '/// activate the Automatic control focus.
    printlog "activate the Automatic control focus."

    call hToolbarSelect("FormDesignTools",true)

    Kontext "FormDesignTools"
        AutoControlFocus.Click
        
	sleep(2)	
    '/// save the document
    printlog "save the document"
	hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_Control_Test2.odt"))
	hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Control_Test2.odt"))
    ' /// check if the second control has the focus after reload.
    printlog " check if the second control has the focus after reload."
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<MOD1 HOME>" , true
    sleep(1)       
	DocumentWriter.TypeKeys "<MOD1 SHIFT END>" , true
    sleep(1)
    DocumentWriter.TypeKeys "<MOD1 C>" , true
    sleep(1)
    if GetClipboard <> "tt_control2" then
        warnlog "The wrong control has the focus." 
    endif
    
    hCloseDocument
    
endcase
'--------------------------------------------------------------------------------------
testcase tPosAndSize_Keyboard
    
    dim sPosX as String
    dim sPosY as String
    dim sWidth as String
    dim sHeight as String
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    
	sleep(1)
    call hToolbarSelect("FormControls",true)
  	Kontext "FormControls"   	
   	    Edit.Click
   	
	'/// insert a Text Box
    printlog "insert a Text Box"    
    Kontext "DocumentWriter"
    DocumentWriter.MouseDown ( 45, 45 )
    DocumentWriter.MouseMove ( 55, 55 )
    DocumentWriter.MouseUp   ( 55, 55 )
	sleep(2)
	'/// open the position and size dialog.
    printlog "open the position and size dialog."
    
    call fOpenPositionAndSizeDialog    
    
    '/// remember the settings.
    printlog "remember the settings."
    Kontext "TabPositionAndSizeWriter"
    sPosX = HorizontalBy.getText()
    sPosY = Verticalby.getText()
    sWidth = Width.getText()
    sHeight = Height.getText()    
    TabPositionAndSizeWriter.OK
    sleep(2)
    
    Kontext "DocumentWriter"    
    gMouseClick(50,50)    
    sleep(2)
    
    '/// move the control via keyboard
    printlog "move the control via keyboard"     
    
    DocumentWriter.TypeKeys "<DOWN>" , true
    DocumentWriter.TypeKeys "<RIGHT>" , true
    DocumentWriter.TypeKeys "<MOD1 TAB>" , true
    DocumentWriter.TypeKeys "<DOWN>" , true
    DocumentWriter.TypeKeys "<RIGHT>" , true
        
    sleep(2)
    '/// open the position and size dialog
    printlog "open the position and size dialog"
    
    call fOpenPositionAndSizeDialog
    
    Kontext "TabPositionAndSizeWriter"
    '/// check the settings.
    printlog "check the settings."
    if sPosX = HorizontalBy.getText() then
        warnlog "The X position has not change."
    endif
    if sPosY = VerticalBy.getText() then
        warnlog "The Y position has not change."
    endif
    if sWidth = Width.getText() then
        warnlog "The width position has not change."
    endif
    if sHeight = Height.getText() then
        warnlog "The height position has not change."
    endif
    
    TabPositionAndSizeWriter.OK
    sleep(1)
    
    hCloseDocument
    
endcase
'--------------------------------------------------------------------------------------
testcase tPosAndSize_Dialog
    
    dim sPosX as String
    dim sPosY as String
    dim sWidth as String
    dim sHeight as String
    dim sText as String
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
    
    call hToolbarSelect("FormControls",true)

  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    '/// insert a Text Box
    printlog "insert a Text Box"
    call hDrawingWithSelection ( 40, 40, 60, 60 )
	sleep(2)
	'/// open the position and size dialog
    printlog "open the position and size dialog"
    
    call fOpenPositionAndSizeDialog         
        
    '/// set values for X , Y , width and height
    printlog "set values for X , Y , width and height"
    Kontext "TabPositionAndSizeWriter"
        ' uncheck KeepRatio because maybe it have set from a test before.
        KeepRatio.UnCheck
        HorizontalBy.setText("3")
        VerticalBy.setText("3")
        Width.setText("3")
        Height.setText("3")    
        sleep(1)
        TabPositionAndSizeWriter.OK
    sleep(2)
    '/// open the position and size dialog
    printlog "open the position and size dialog"
    
    call fOpenPositionAndSizeDialog
    
    Kontext "TabPositionAndSizeWriter"
    '/// check the settings.
    printlog "check the settings."
    sText = HorizontalBy.getText()
    if Instr(sText,"3") = 0 then
        warnlog "The X position has not changed."
    endif
    sText = VerticalBy.getText()
    if Instr(sText,"3") = 0 then
        warnlog "The Y position has not changed."
    endif
    sText = Width.getText()
    if Instr(sText,"3") = 0 then
        warnlog "The width has not changed."
    endif
    sText = Height.getText()    
    if Instr(sText,"3") = 0 then
        warnlog "The height has not changed."
    endif
    
    TabPositionAndSizeWriter.OK
    sleep(1)
    
    hCloseDocument
    
endcase
'--------------------------------------------------------------------------------------
testcase tPosAndSize_Protect_Position
    
    dim sPosX as String
    dim sPosY as String    
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true)
	
  	Kontext "FormControls"   	
   	    Edit.Click
   	sleep(1)
	'/// insert a Text Box
    printlog "insert a Text Box"
    call hDrawingWithSelection ( 40, 40, 60, 60 )
	sleep(2)
	'/// open the position and size dialog
    printlog "open the position and size dialog"

    call fOpenPositionAndSizeDialog
    
    Kontext "TabPositionAndSizeWriter"
    sPosX = HorizontalBy.getText()
    sPosY = VerticalBy.getText()
    '/// check protect position
    printlog "check protect position"
    ProtectPosition.Check
    TabPositionAndSizeWriter.OK
    sleep(2)
    
    Kontext "DocumentWriter"    
    gMouseClick(50,50)        
    sleep(2)
    'Kontext "DocumentWriter"    
    '/// test if is possible to move the control via keyboard
    printlog "test if is possible to move the control via keyboard"
    DocumentWriter.TypeKeys "<DOWN>" , true
    DocumentWriter.TypeKeys "<RIGHT>" , true
          
    sleep(2)
    '/// open the position and size dialog
    printlog "open the position and size dialog"
    
    call fOpenPositionAndSizeDialog
    
    Kontext "TabPositionAndSizeWriter"
    '/// check the settings, if the position is changed.
    printlog "check the settings, if the position is changed."
    if sPosX <> HorizontalBy.getText() then
        warnlog "The X position has change although the protect position is checked."
    endif
    if sPosY <> VerticalBy.getText() then
        warnlog "The Y position has change although the protect position is checked."
    endif   
    TabPositionAndSizeWriter.OK
    sleep(1)
    
    hCloseDocument
    
endcase
'--------------------------------------------------------------------------------------
testcase tPosAndSize_KeepRatio
    
    dim sWidth as String
    dim sHeight as String
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true) 	

  	Kontext "FormControls"   	
   	    Edit.Click
   	sleep(1)
	'/// insert a Text Box
    printlog "insert a Text Box"
    call hDrawingWithSelection ( 40, 40, 60, 60 )
	sleep(2)
	'/// open the position and size dialog
    printlog "open the position and size dialog"
    
    call fOpenPositionAndSizeDialog

    sleep(1)
    '/// set a width and height.
    printlog "set a width and height."
    Kontext "TabPositionAndSizeWriter"
        KeepRatio.UnCheck
        Width.setText("4")
        sleep(1)
        Height.setText("2")
        sleep(1)
        '/// check keep ratio
        printlog "check keep ratio"
        KeepRatio.Check
        sleep(1)
        Width.setText("6")
        sleep(1)
        '/// check if keep ratio is working
        printlog "check if keep ratio is working"
        if Instr(Height.getText(),"3") = -1  then
            warnlog "The height doesn't keep the ratio"
        endif
        TabPositionAndSizeWriter.OK
    sleep(2)
    '/// open the position and size dialog
    printlog "open the position and size dialog"
    call fOpenPositionAndSizeDialog
    
    Kontext "TabPositionAndSizeWriter"
        '/// check the settings.
        printlog "check the settings."
        if Instr(Width.getText(),"6") = -1 then
            warnlog "The width isn't correct. " + Width.getText()+" instead of 6."
        endif
        if Instr(Height.getText(),"3") = -1 then
           warnlog "The height isn't correct. " + height.getText()+" instead of 3."
        endif
        
        TabPositionAndSizeWriter.OK
    sleep(1)
    
    hCloseDocument
    
endcase
'--------------------------------------------------------------------------------------
testcase tPosAndSize_Protect_Size
    
    dim sWidth as String
    dim sHeight as String
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true) 
	
  	Kontext "FormControls"   	
   	    Edit.Click
   	sleep(1)
	'/// insert a Text Box
    printlog "insert a Text Box"
    call hDrawingWithSelection ( 40, 40, 60, 60 )
	sleep(2)
	'/// open the position and size dialog
    printlog "open the position and size dialog"
    
    call fOpenPositionAndSizeDialog
    
    Kontext "TabPositionAndSizeWriter"
        sWidth = Width.getText()
        sHeight = Height.getText()
        '/// check protect size
        printlog "check protect size"
        ProtectSize.Check
        TabPositionAndSizeWriter.OK
    sleep(2)
    
    Kontext "DocumentWriter"    
    gMouseClick(50,50)    
    sleep(2)
    'Kontext "DocumentWriter"    
    '/// test is is possible to change the size of the control via keyboard
    printlog "test is is possible to change the size of the control via keyboard"    
    DocumentWriter.TypeKeys "<MOD1 TAB>" , true
    DocumentWriter.TypeKeys "<DOWN>" , true
    DocumentWriter.TypeKeys "<RIGHT>" , true
    sleep(2)
    '/// open the position and size dialog
    printlog "open the position and size dialog"
    
    call fOpenPositionAndSizeDialog
    
    Kontext "TabPositionAndSizeWriter"
    '/// check the settings
    printlog "check the settings"
    if sWidth <> Width.getText() then
        warnlog "The width has change although the protect size is checked."
    endif
    if sHeight <> Height.getText() then
        warnlog "The height has change although the protect size is checked."
    endif   
    TabPositionAndSizeWriter.OK
    sleep(1)
    
    hCloseDocument
    
endcase
'-------------------------------------------------------------------------
testcase tTabStop
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true) 
	
  	Kontext "FormControls"   	
   	    Edit.Click
   	sleep(1)
	'/// insert one Text Box
    printlog "insert one Text Box"    
    call hDrawingWithSelection ( 50, 20, 60, 30 )
	sleep(2)
	
    '/// open the control propery browser
    printlog "open the control propery browser"
    call fOpenControlPropertyBrowser
    
    '/// the the default text to tt_control1
    printlog "the the default text to tt_control1"
    call fSetControlProperty("TextBox","DefaultValue","tt_control1")	
		
	' close the property browser
    Kontext "FormControls"
	    ControlProperties.Click

	sleep(1)
    
	'/// insert again one Text Box
    printlog "insert again one Text Box"
	call hDrawingWithSelection  ( 50, 40, 60, 50 )
	
    sleep(2)    
	
    '/// open the property browser
    printlog "open the property browser"
    call fOpenControlPropertyBrowser
    
    '/// set the Order property to 2
    printlog "set the Order property to 2"
    call fSetControlProperty("TextBox","Order","2")
    
    'close and reopen the control property browser to get the default focus
    call fOpenControlPropertyBrowser
    
    '/// set the DefaultValue property to tt_control2
    call fSetControlProperty("TextBox","DefaultValue","tt_control2")

    'close the control propery browser
    Kontext "FormControls"
	    ControlProperties.Click

    '/// insert again one Text Box
    printlog "insert again one Text Box"    
    call hDrawingWithSelection  ( 50, 60, 60, 70 )
	
    sleep(2)
    
    ' open and close the form property browser to get the focus    
    call fOpenControlPropertyBrowser
    
    '/// set the Order property to 1
    printlog "set the Order property to 1"
    call fSetControlProperty("TextBox","Order","1")
    
    'close and reopen the control property browser to get the default focus
    call fOpenControlPropertyBrowser
    
    '/// set the DeaultValue property to tt_control3
    printlog "set the DeaultValue property to tt_control3"
    call fSetControlProperty("TextBox","DefaultValue","tt_control3")
    
    call hToolbarSelect("FormDesignTools",true) 
    Kontext "FormDesignTools"
        '/// activate the AutomaticControlFocus
        printlog "activate the AutomaticControlFocus"
        AutoControlFocus.Click
        sleep(1)		
        '/// turn of the design view
        printlog "turn of the design view"
    Kontext "FormControls"
        SwitchControlDesignMode.Click    
        printlog "after designview click"
        sleep(4)
    '/// check if the right control has the focus
    printlog "check if the right control has the focus"
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 HOME>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if GetClipboard = "tt_doc" then
            warnlog "The control has not the focus. The document has the focus." 
        elseif GetClipboard <> "tt_control3" then
            warnlog "The wrong control has the focus."
        endif  
        '/// check that tyou travel to the right controls via TAB
        printlog "check that tyou travel to the right controls via TAB"
        DocumentWriter.TypeKeys "<TAB>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 HOME>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if GetClipboard <> "tt_control2" then
            warnlog "The wrong control has the focus." 
        endif        
    
    hCloseDocument
    
endcase
'--------------------------------------------------------------------------------------
testcase tActivationOrder

    Dim sSelText as string

    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
    
    call hToolbarSelect("FormControls",true)    
	
    Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
        
    '/// insert one Text Box
    printlog "insert one Text Box"
    call hDrawingWithSelection ( 50, 20, 60, 30 )
	sleep(2)
    
    '/// open the control property browser
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser
    
    '/// set the Name Property to tt_control1
    printlog "set the Name Property to tt_control1"
    call fSetControlProperty("TextBox","NameText","tt_control1")
    
    'close and reopen the control property browser to get the default focus
    call fOpenControlPropertyBrowser
    
    '/// set the DefaultValue Property to tt_control1
    printlog "set the DefaultValue Property to tt_control1"
    call fSetControlProperty("TextBox","DefaultValue","tt_control1")

    sleep(1)    

    ' close the property browser
    Kontext "FormControls"
	    ControlProperties.Click
    
    '/// insert one Text Box
    printlog "insert one Text Box"
	call hDrawingWithSelection  ( 50, 40, 60, 50 )
	sleep(2)
    
    '/// open the control property browser
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser
    
    '/// set the Name Property to tt_control2
    printlog "set the Name Property to tt_control2"
    call fSetControlProperty("TextBox","NameText","tt_control2")
    
    'close and reopen the control property browser to get the default focus
    call fOpenControlPropertyBrowser
    
    '/// set the DefaultValue Property to tt_control2
    call fSetControlProperty("TextBox","DefaultValue","tt_control2")
    
    ' close the property browser
    Kontext "FormControls"
	    ControlProperties.Click
    
    '/// insert one Text Box
    printlog "insert one Text Box"
    call hDrawingWithSelection  ( 50, 60, 60, 70 )
	sleep(2)
    
    '/// open the control property browser
    printlog "open the control property browser"
    call fOpenControlPropertyBrowser
    
    '/// set the Name Property to tt_control3
    call fSetControlProperty("TextBox","NameText","tt_control3")
    
    'close and reopen the control property browser to get the default focus
    call fOpenControlPropertyBrowser
    
    '/// set the Defaultvalue property to tt_control3
    call fSetControlProperty("TextBox","DefaultValue","tt_control3")
    
    '/// open the tab order dialog
    printlog "open the tab order dialog"
    call fOpenTabOrder
    
    ' IndexKLiener
    printlog "IndexKLiener"     
    Kontext "ActivationOrder"        
        sleep(1)
        ControlList.TypeKeys "<END>" , true ' to select the first entry        
        sleep(1)        
        ControlList.TypeKeys "<HOME>" , true        
        sleep(1)
        sSelText = ControlList.getSelText()
        if sSelText <> "tt_control1" then
            warnlog "Wrong control on first position in activation order dialog.(" + sSelText + ")"
        endif        
        sleep(1)
        ControlList.TypeKeys "<DOWN>" , true
        sleep(1)
        sSelText = ControlList.getSelText()
        if sSelText <> "tt_control2" then
            warnlog "Wrong control on second position in activation order dialog.(" + sSelText + ")"
        endif
        sleep(1)        
        ControlList.TypeKeys "<DOWN>" , true
        sleep(1)
        sSelText = ControlList.getSelText()
        if sSelText <> "tt_control3" then
            warnlog "Wrong control on third position in activation order dialog.(" + sSelText + ")"
        endif
        sleep(1)
        MoveUp.Click
        sleep(1)
        ControlList.TypeKeys "<HOME>" , true
        sleep(1)
        ControlList.TypeKeys "<DOWN>" , true
        sSelText = ControlList.getSelText()
        if sSelText <> "tt_control3" then
            warnlog "The move up button in activation order dialog doesn't work.(" + sSelText + ")"
        endif
        
        
        ActivationOrder.OK        
    sleep(1)        
    printlog "open TabOrder dialog"
    call fOpenTabOrder
        
    Kontext "ActivationOrder"        
        sleep(1)
        ControlList.TypeKeys "<END>" , true ' to select the first entry        
        sleep(1)        
        ControlList.TypeKeys "<HOME>" , true        
        sleep(1)
        sSelText = ControlList.getSelText()
        if sSelText <> "tt_control1" then
            warnlog "Wrong control on first position in activation order dialog.(" + sSelText + ")"
        endif        
        sleep(1)
        MoveDown.Click
        sleep(1)        
        ControlList.TypeKeys "<HOME>" , true
        sleep(1)
        ControlList.TypeKeys "<DOWN>" , true
        sSelText = ControlList.getSelText()
        if sSelText <> "tt_control1" then
            warnlog "The move down button in activation order dialog doesn't work.(" + sSelText + ")"
        endif        
        
        ActivationOrder.OK
    
    sleep(1)

    call hToolbarSelect("FormDesignTools",true)

    Kontext "FormDesignTools"
        
            '/// activate the AutomaticControlFocus
            printlog "activate the AutomaticControlFocus"
            AutoControlFocus.Click
            sleep(1)		
            '/// turn of the design view
            printlog "turn of the design view"
        Kontext "FormControls"
            SwitchControlDesignMode.Click    
            printlog "after designview click"
            sleep(4)
       
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 HOME>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if GetClipboard = "tt_doc" then
            warnlog "The control has not the focus. The document has the focus." 
        elseif GetClipboard <> "tt_control3" then
            warnlog "The wrong control has the focus. (" + GetClipboard +  ")"
        endif      
        DocumentWriter.TypeKeys "<TAB>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 HOME>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if GetClipboard = "tt_doc" then
            warnlog "The control has not the focus. The document has the focus." 
        elseif GetClipboard <> "tt_control1" then
            warnlog "The wrong control has the focus. (" + GetClipboard +  ")"
        endif
        
        DocumentWriter.TypeKeys "<TAB>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 HOME>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if GetClipboard = "tt_doc" then
            warnlog "The control has not the focus. The document has the focus." 
        elseif GetClipboard <> "tt_control2" then
            warnlog "The wrong control has the focus. (" + GetClipboard +  ")"
        endif

        
        printlog "turn of the design view"
        Kontext "FormControls"
            SwitchControlDesignMode.Click    
            printlog "after designview click"
            sleep(4)
        
    printlog "open TabOrder dialog"
    call fOpenTabOrder
        
    Kontext "ActivationOrder"
        AutoOrder.Click
        sleep(1)
        ActivationOrder.OK
    sleep(1)
    
    Kontext "FormControls"	
        '/// turn off the design view
        printlog "turn of the design view"    
        SwitchControlDesignMode.Click
    sleep(2)
        
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 HOME>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if GetClipboard = "tt_doc" then
            warnlog "The control has not the focus. The document has the focus." 
        elseif GetClipboard <> "tt_control1" then
            warnlog "The wrong control has the focus.(" + GetClipboard +  ")"
        endif      
        DocumentWriter.TypeKeys "<TAB>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if GetClipboard = "tt_doc" then
            warnlog "The control has not the focus. The document has the focus." 
        elseif GetClipboard <> "tt_control2" then
            warnlog "The wrong control has the focus.(" + GetClipboard +  ")"
        endif
        
        DocumentWriter.TypeKeys "<TAB>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if GetClipboard = "tt_doc" then
            warnlog "The control has not the focus. The document has the focus." 
        elseif GetClipboard <> "tt_control3" then
            warnlog "The wrong control has the focus.(" + GetClipboard +  ")"
        endif
    
    sleep(1)
    
    call hCloseDocument
    
endcase
'-------------------------------------------------------------------------
testcase tAddFields
    
    Dim sControlName1 as String
    Dim sControlName2 as String
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	    DocumentWriter.TypeKeys "tt_doc" , true
        sleep(1)
    
    call hToolbarSelect("FormControls",true)
    	
    Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    '/// insert three Text Boxes
    printlog "insert three Text Boxes"
    call hDrawingWithSelection ( 50, 20, 60, 30 )
	sleep(2)
	Kontext "FormControls"
	    FormProperties.Click
        sleep(1)
   
    Kontext "FormPropertiesDialog"
   	    FormPropertiesTabControl.SetPage TabDataForm
  	
    Kontext "TabDataForm"                
		sleep(1)		
        DataSource.Select "Bibliography"
        TabDataForm.typeKeys("<RETURN>")
        sleep(1)
        Content.Select "biblio"
        TabDataForm.typeKeys("<RETURN>")
        sleep(1)
        
    Kontext "FormControls"   
	    FormProperties.Click
        sleep(1)

    call hToolbarSelect("FormDesignTools",true)
       
    Kontext "FormDesignTools"    
        AddField.Click
        sleep(1)
    
    Kontext "FeldAuswahl"
        Liste.Select 1
        sControlName1 = Liste.getSelText
        Liste.TypeKeys "<RETURN>"
        sleep(3)
        
    Kontext "FormDesignTools"
        ShowFmExplorer.Click
        sleep(1)

    Kontext "FormularNavigator" 
       	Liste.TypeKeys "<HOME>" , true 
        Liste.TypeKeys "<DOWN>" , 4, true   	    
        sControlName2 = Liste.getSelText
        if sControlName1 <> sControlName2 then
            warnlog "The control name is not the same as the fieldname. controlname = " + sControlName2 + " fieldname = " + sControlName1
        endif
        
    sleep (2)    
    
    call hCloseDocument
    
endcase
'-------------------------------------------------------------------------
testcase tLabelFieldSelection
    ' check if the assignment of lables to text fields work
    dim sLabelText as String

    printlog "Open a new document,"
    call hNewDocument
    Kontext "DocumentWriter"       
    DocumentWriter.TypeKeys "tt_doc" , true
    sleep(1)

    printlog "open the form control toolbar"
    call hToolbarSelect("FormControls",true)
    
    Kontext "FormControls"   	
    Label.Click
    sleep(1)
    
    printlog "insert a label field"
    call hDrawingWithSelection ( 20, 20, 30, 30 )
    sleep(2)

    Kontext "FormControls"   	
    Edit.Click
    sleep(1)
    
    printlog "insert a text field"
    call hDrawingWithSelection ( 40, 40, 50, 50 )
    sleep(2)

    printlog "open the control property browser"
    call fOpenControlPropertyBrowser

    Kontext "ControlPropertiesTabControl"
        ControlPropertiesTabControl.setPage TabGeneralControl       'make shure that tabpage general in property is choosen
        sleep(2)

    printlog "click on button behind the lable field to open the field selection dialog"
    Kontext "TabGeneralControl"
        LabelFieldButton.Click
        sleep(1)

    printlog "select the lable fiiel in the tree list box"
    Kontext "LabelFieldSelection"
        LabelFieldSelection.TypeKeys "<DOWN>",TRUE
        LabelFieldSelection.TypeKeys "<DOWN>",TRUE
        sleep(1)
        printlog "close the field selection dialog"
        LabelFieldSelection.OK
        sleep(1)

    printlog "check if the lable field property contains some text"
    'TODO: check for the correct text this has a language dependencie
    Kontext "TabGeneralControl"        
        sLabelText = LabelField.getText()
        if(sLabelText = "") then
            warnlog "the lable for a text field is not set"
        else
            printlog "the label for the text field is set"
        endif

    printlog "click on button behind the lable field to open the field selection dialog"
    Kontext "TabGeneralControl"
        LabelFieldButton.Click
        sleep(1)

    Kontext "LabelFieldSelection"    
        LabelFieldSelection.TypeKeys "<DOWN>",TRUE
        LabelFieldSelection.TypeKeys "<DOWN>",TRUE
        sleep(1)
        printlog "select the lable field and check the checkbox 'no assignment'"
        CBNoAssignment.check
        printlog "close the field selection dialog"
        LabelFieldSelection.OK
        sleep(1)

    printlog "check if the lable field property contains no text"
    Kontext "TabGeneralControl"        
        sLabelText = LabelField.getText()
        if(sLabelText <> "") then
            warnlog "the lable for a text field is not deleted"
        else
            printlog "the label for the text field is deleted"
        endif    

    printlog "close the document"
    call hCloseDocument

endcase
'-------------------------------------------------------------------------
function fOpenPositionAndSizeDialog

    call hToolbarSelect("FormDesignTools",true)  	
            
    Kontext "FormDesignTools"        
        PositionAndSizeDialog.Click
    
    fOpenPositionAndSizeDialog=true
    
end function
'-------------------------------------------------------------------------
function fOpenTabOrder

    printlog "open FormControls Toolbar"

    call hToolbarSelect("FormDesignTools",true)  	

    Kontext "FormDesignTools"    
        TabDialog.Click    
    
    fOpenTabOrder=true
    
end function

'-------------------------------------------------------------------------


