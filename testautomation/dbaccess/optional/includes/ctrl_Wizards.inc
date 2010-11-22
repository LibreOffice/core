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
'* short description : Control Wizards Test
'*
'\***********************************************************************
global sDSName as String
global sFileName as String

sub ctrl_Wizards

    printlog "---------------- Control Wizards -------------------"
    
    sDSName = "TT"

    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")
    call fRegisterDatabaseFile(gOfficePath + ConvertPath("user/work/TT_hsqldb.odb"),"TT")
 
    call fSwitchAutopilotOff(false)  ' has to run first to switch on the autopilot
    call tGroupBoxWithDatasource
    call tGroupBoxWithoutDatasource
    call tListBox                                          
    call tComboBox
    call tTableControl
    call fDeRegisterDatabaseFile(sDSName)
    
end sub

'--------------------------------------------------------------------

testcase tGroupBoxWithDatasource

    '/// open a new text document
    printlog "open a new text document"
	call hNewDocument
	sleep(1)
	'/// create a new form via the form navigator
    printlog "create a new form via the form navigator"
        
    call hToolbarSelect("FORMDESIGN",true)
    Kontext "FormDesignTools"
	 ShowFmExplorer.Click	
   	 sleep(2)
     
	Kontext "FormularNavigator" 
   	 Liste.TypeKeys "<SPACE>"
   	 sleep(3)
   	 Liste.OpenContextmenu
   	 sleep(2)
   	 hMenuSelectNr (1)
   	 sleep(2)
   	 hMenuSelectNr (1)
   	 sleep(3)
   	 Liste.TypeKeys "<RETURN>",true
  	 sleep(3)
   	 Liste.OpenContextmenu
  	 sleep(2)
   	 hMenuSelectNr (6)
   	 sleep(4)      
        
  	Kontext "ControlPropertiesTabControl"
   	 ControlPropertiesTabControl.SetPage TabDataControl
  	Kontext "TabDataForm"   	
   	 '/// set the datasource TT
     printlog "set the datasource TT"
     DataSource.setText(sDSName)
     DataSource.TypeKeys "<RETURN>" , true
     sleep(1)
     '/// insert password 'testtool' into login dialog
     printlog "insert password 'testtool' into login dialog"
     Kontext "LoginDialog"
        if LoginDialog.Exists then
         	LoginPassword.setText "testtool"
         	LoginDialog.OK
            sleep(1)
        else
            printlog "Login dialog doesn't exists. Why?"
        endif
     Kontext "LoginDialog"
        if LoginDialog.Exists then
         	LoginPassword.setText "testtool"
         	LoginDialog.OK
            sleep(1)
        else
            printlog "Login dialog doesn't exists. Why?"
        endif
    Kontext "TabDataForm"         
    '/// set the table TT_Control
    printlog "set the table TT_Control"    
    Content.setText("TT_Control")
    Content.TypeKeys "<RETURN>" , true
    sleep(1)
  	Kontext "ControlPropertiesDialog"
   	 ControlPropertiesDialog.Close
   	sleep(3)
  	Kontext "FormularNavigator"
   	 FormularNavigator.Close
   	sleep(4)
    
    call hToolbarSelect("MoreControls",true)
  	Kontext "MoreControls"
   	 '/// insert a groupbox (the wizard appears)
     printlog "insert a groupbox (the wizard appears)"    
   	 Groupbox.Click
    
   	call hDrawingWithSelection ( 50, 20, 60, 30 )
   	sleep(10)
    
    Dim i as integer
    Kontext "AutopilotGroupElement"
     AutopilotGroupElement.MouseDown (10,10)
     AutopilotGroupElement.MouseUp (10,10)
     '/// insert three fields with the names: red, green, blue & next
     printlog "insert three fields with the names: red, green, blue & next"
    Kontext "TabGroupBoxData"
     Names.setText "red"
     AddTo.Click
     sleep(1)
     Names.setText "green"
     AddTo.Click
     sleep(1)
     Names.setText "blue"
     AddTo.Click
     sleep(1)
    Kontext "AutopilotGroupElement"
     if NextButton.IsEnabled then
       NextButton.Click
     else
       warnlog "In first window the 'Next'-button is disabled!"
       AutopilotGroupElement.Cancel
       goto endsub
     end if
    printlog "  - Autopilot group box: Page 2/5"
   Kontext "TabDefaultFieldSelection"
    '/// choose no default and click two times next
    printlog "choose no default and click two times next"    
    NoSelected.check    
   Kontext "AutopilotGroupElement"
    if NextButton.IsEnabled then
       NextButton.Click
     else
      warnlog "In second window the 'Next'-button is disabled!!"
      AutopilotGroupElement.Cancel
      goto endsub
    end if
    sleep(1)
    printlog "  - Autopilot group box: Page 3/5"
   Kontext "TabDatabaseValues"
    ' nothing
   Kontext "AutopilotGroupElement"
    if NextButton.IsEnabled then
       NextButton.Click
     else
       warnlog "In third window the 'Next'-button is disabled!"
      AutopilotGroupElement.Cancel
    end if
    sleep(3)
    printlog "  - Autopilot group box: Page 4/5"
   '/// choose the database field Colors 
   printlog "choose the database field Colors"   
   'Kontext "TabDatabaseValues" 
   Kontext "TabDatabaseField" '      dbp:TabPage:RID_PAGE_OPTION_DBFIELD   
    YesSaveIt.check
    Fieldbox.select 2    
   Kontext "AutopilotGroupElement"
    if NextButton.IsEnabled then
       NextButton.Click
     else
       warnlog "In fourth window the 'Next'-button is disabled!"
       AutopilotGroupElement.Cancel
       goto endsub
    end if
    sleep(1)
    printlog "  - Autopilot group box: Page 5/5"
   Kontext "TabCreateOptionGroup"
    ' nothing
   Kontext "AutopilotGroupElement"
    if CreateButton.IsEnabled then
       CreateButton.Click
     else
       warnlog "In fifth window the 'Create'-button is disabled!"
       AutopilotGroupElement.Cancel
       goto endsub
    end if  
   
    sleep(1)
    
    call hToolbarSelect("FORMDESIGN",true)
    Kontext "FormDesignTools"
	 ShowFmExplorer.Click	
   	 sleep(3)
	
    Kontext "FormularNavigator"
     sleep(1)     
     'Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<HOME>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true     
     sleep(1)
     Liste.OpenContextmenu
  	 sleep(2)
   	 hMenuSelectNr (5)
   	 sleep(5)

    Kontext "ControlPropertiesTabControl"
        ControlPropertiesTabControl.setPage TabGeneralControl
     
    Kontext "TabGeneralControl"
     TabGeneralControl.TypeKeys "<TAB>" , true 'this tab is importent, I don't not why but it is.
     sleep(1)
     HelpURL.setText "customcontrol1"
     HelpURL.TypeKeys "<RETURN>",true
     sleep(1)
    Kontext "ControlPropertiesDialog" 
     ControlPropertiesDialog.close
         
    'open and close the formnavigator to get the focus 
    call hToolbarSelect("FORMDESIGN",true)
    Kontext "FormDesignTools"
	 ShowFmExplorer.Click	
   	 sleep(2)     
     
    Kontext "FormDesignTools"
	 ShowFmExplorer.Click	
   	 sleep(2)        
    
    Kontext "FormularNavigator"
     'Liste.TypeKeys "<DOWN>" , true ' this is needed , I don't know why
     sleep(1)
     Liste.TypeKeys "<HOME>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.OpenContextmenu
  	 sleep(2)
   	 hMenuSelectNr (5)
   	 sleep(5)
     
    Kontext "TabGeneralControl"
	 TabGeneralControl.TypeKeys "<TAB>" , true 'this tab is importent, I don't not why but it is.
     sleep(1)
     HelpURL.setText "customcontrol2"
     HelpURL.TypeKeys "<RETURN>", true
     sleep(1)
    Kontext "ControlPropertiesDialog" 
     ControlPropertiesDialog.close
    
    'open and close the frmnavigator to get the focus 
    call hToolbarSelect("FORMDESIGN",true)
    Kontext "FormDesignTools"
	 ShowFmExplorer.Click	
   	 sleep(2)     
     
    Kontext "FormDesignTools"
	 ShowFmExplorer.Click	
   	 sleep(2)
    
    Kontext "FormularNavigator"
     'Liste.TypeKeys "<DOWN>" , true ' this is needed , I don't know why
     sleep(1)
     Liste.TypeKeys "<HOME>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true
     sleep(1)
     Liste.TypeKeys "<DOWN>" , true     
     sleep(1)
     Liste.OpenContextmenu
  	 sleep(2)
   	 hMenuSelectNr (5)
   	 sleep(5)
     
    Kontext "TabGeneralControl"
     TabGeneralControl.TypeKeys "<TAB>" , true ' this tab is importent, I don't not why but it is.     
     HelpURL.setText "customcontrol3"
     HelpURL.TypeKeys "<RETURN>", true
     sleep(1)
    Kontext "ControlPropertiesDialog" 
     ControlPropertiesDialog.close     
     
    '/// save the document
    printlog "save the document"    
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_AP_GroupBox.odt"))
	hCloseDocument
    sleep(1)

    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_AP_GroupBox.odt"))
    
    '/// check if the options buttons work right
    printlog "check if the options buttons work right"
    
    dim a as boolean
    dim b as boolean 
    dim c as boolean
    
    Kontext "MSC_Special_Dialog"
        a = MSC_Special_Dialog.ischecked
    Kontext "MSC_Special_Dialog2"                
        b = MSC_Special_Dialog2.ischecked
    Kontext "MSC_Special_Dialog3"
        c = MSC_Special_Dialog3.ischecked
    
    if not a AND not b AND c then
        printlog "ok"
    else
        warnlog "the third option isn't checked"
    endif
        
    printlog "next record"
    FM_Next_Record
    sleep(1)
    Kontext "MSC_Special_Dialog"
        a = MSC_Special_Dialog.ischecked
    Kontext "MSC_Special_Dialog2"                
        b = MSC_Special_Dialog2.ischecked
    Kontext "MSC_Special_Dialog3"
        c = MSC_Special_Dialog3.ischecked
    
    if not a AND b AND not c then
        'nothing
    else
        warnlog "the second option isn't checked"
    endif
    
    printlog "next record"
    FM_Next_Record
    sleep(1)
    Kontext "MSC_Special_Dialog"
        a = MSC_Special_Dialog.ischecked
    Kontext "MSC_Special_Dialog2"                
        b = MSC_Special_Dialog2.ischecked
    Kontext "MSC_Special_Dialog3"
        c = MSC_Special_Dialog3.ischecked
    
    if a AND not b AND not c then
        'nothing
    else
        warnlog "the first option isn't checked"
    endif
    sleep(1)
    call hCloseDocument
endcase

'--------------------------------------------------------------------

testcase tGroupBoxWithoutDatasource

    '/// open a textdocument
    printlog "open a textdocument"

	call hNewDocument
	sleep(1)

	call hToolbarSelect("MoreControls",true)

  	Kontext "MoreControls"
   	 '/// insert a groupbox (the wizard appears)
     printlog "insert a groupbox (the wizard appears)"    
   	 Groupbox.Click
   	 call hDrawingWithSelection ( 50, 20, 60, 30 )
   	 sleep(2)
	 Dim i as integer
	Kontext "AutopilotGroupElement"
	 AutopilotGroupElement.MouseDown (10,10)
	 AutopilotGroupElement.MouseUp (10,10)
	 sleep (1)
	 if AutopilotGroupElement.Exists(3) then  	
	    printlog "  - Autopilot group element: Page 1/4"
        '/// insert 6 diffent options
        printlog "insert 6 diffent options"
   		Kontext "TabGroupBoxData"
    	for i = 1 to 6
	    	Names.Settext "Option field " + i
		  	if AddTo.IsEnabled then
       			AddTo.Click
			else
	       		warnlog "Entry couldn't be overtaken in first window of the group box autopilot!"
				AutopilotGroupElement.Cancel
	    	end if			
		next
		Kontext "AutopilotGroupElement"
    	if NextButton.IsEnabled then
    		NextButton.Click
	    else
    		warnlog "In first window the 'Next'-button is disabled!"
	    	AutopilotGroupElement.Cancel
	    end if
	    sleep(3)
	    printlog "  - Autopilot group box: Page 2/4"
		Kontext "TabDefaultFieldSelection"
		 sleep (1)
         '/// set Option Field 3 as default selection
         printlog "set Option Field 3 as default selection"
         YesFollowing.Check   		
	     Selection.Select "Option field 3"		
		Kontext "AutopilotGroupElement"
	    if NextButton.IsEnabled then
	    	NextButton.Click
	    else
	    	warnlog "In second window the 'Next'-button is disabled!!"
	    	AutopilotGroupElement.Cancel
	    end if
	    sleep(3)
	    printlog "  - Autopilot group box: Page 3/4"
		Kontext "TabDatabaseValues"
		dim iCountOptionFields
		iCountOptionFields = OptionFields.getitemCount
		if iCountOptionFields <> 6 then 
			warnlog "there are not 6 options."
	    	AutopilotGroupElement.Cancel
			goto endsub
		end if
		'/// set some new reference for the options fields
        printlog "set some new reference for the options fields"
		OptionFields.select 1
		WhichValue.setText "a"
		OptionFields.select 2
		WhichValue.setText "b"
		OptionFields.select 3
		WhichValue.setText "c"
		OptionFields.select 4
		WhichValue.setText "d"
		OptionFields.select 5
		WhichValue.setText "e"
		OptionFields.select 6
		WhichValue.setText "f"

		Kontext "AutopilotGroupElement"
	    if NextButton.IsEnabled then
    		NextButton.Click
    	else
	    	warnlog "In third window the 'Next'-button is disabled!"
	    	AutopilotGroupElement.Cancel
	    end if
	    sleep(3)
	    
	    printlog "  - Autopilot group box: Page 4/4"
		'/// change the goup name to Group 123
        printlog "change the goup name to Group 123"
        Kontext "TabCreateOptionGroup" 
		 GroupCaption.setText "Group 123"   
		Kontext "AutopilotGroupElement"
	     if CreateButton.IsEnabled then
	    	 CreateButton.Click
	     else
	    	 warnlog "In fourth window the 'Create'-button is disabled!"
	    	 AutopilotGroupElement.Cancel
	     end if	   
	else	   	
	    warnlog "- Autopilot for Group Box didn't appear!"
	end if
	sleep(2)
    
    '/// save the document
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_AP_GroupBox2.odt"))
	hCloseDocument
    sleep(1)

    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_AP_GroupBox2.odt"))
	
	call hToolbarSelect("FormControls",true)

	sleep(1)
	Kontext "FormControls"
	 SwitchControlDesignMode.Click
     sleep(1)
    
    call hToolbarSelect("FormDesignTools",true)
    Kontext "FormDesignTools"
        ShowFmExplorer.Click	
	sleep(1)
   	Kontext "FormularNavigator" 
   	 Liste.TypeKeys "<HOME>"
   	 Liste.TypeKeys "<DOWN>"
   	 Liste.TypeKeys "<DOWN>"
	 sleep (2)
   	 Liste.OpenContextmenu
   	 sleep(1)
   	 try
   	    hMenuSelectNr (5)
   	 catch
		printlog "Control property browser already open."
	 endcatch
	sleep (2)
  	Kontext "ControlPropertiesTabControl"
   	ControlPropertiesTabControl.SetPage TabGeneralControl
   	sleep (2)
  	Kontext "TabGeneralControl"          
     if Label.getText() <> "Group 123" then
		 warnlog "The label of the groupbox is not right." + getClipboard
	 end if
	 sleep (2)
   	Kontext "FormularNavigator" 
   	 Liste.TypeKeys "<DOWN>"
   	 sleep (2)
	
     if fgetControlproperty("RadioButton","DefaultState") <> "1" then
	 	 warnlog "The default state is set to not selected"
	 end if
	 
	 sleep 1			
	Kontext "FormularNavigator" 
   	 Liste.TypeKeys "<DOWN>"
   	 Liste.TypeKeys "<DOWN>"
   	 sleep (2)	
    
	 if fgetControlproperty("RadioButton","DefaultState") <> "2" then
	 	 warnlog "The default state is set to not selected"
	 end if
     
     if fgetControlproperty("RadioButton","ReferenceValue") <> "c" then
		 warnlog "The reference value is not right, it have to be c but it is " + ReferenceValue.getText
	 end if	
	sleep(2)
   	call hCloseDocument
endcase

'--------------------------------------------------------------------

testcase tListBox
    
    Dim j as integer
    Dim k as integer
    dim i as integer
    printlog "- Formular toolbox: List box element (" & gApplication & ")"
    '/// open a text, insert a listbox (the wizard appears)
    call hNewDocument
    sleep(1)

    call hToolbarSelect("FormControls",true)

    Kontext "FormControls"
    Listbox.Click
    call hDrawingWithSelection ( 50, 20, 60, 25 )
    sleep(3)
    Kontext "AutoPilotListComboBox"
    AutoPilotListComboBox.MouseDown(10,10)
    AutoPilotListComboBox.MouseUp(10,10)

    if AutoPilotListComboBox.Exists(1) then        
        printlog "  - Autopilot List/Combo Box: Page 1/4"
        Kontext "TabListBoxData"                
        '/// choose database TT & click next
        DataSource.Select(sDSName)
        Kontext "LoginDialog"
        if LoginDialog.Exists(3) then
            printlog "login dialog"
            LoginPassword.SetText "testtool"
            LoginDialog.OK
            sleep 1
        end if	    
        '/// choose table TPCoA-1 & click next
        Kontext "TabListBoxData"                
        Table.Select("TT_Control")
        sleep(1)
        Kontext "AutopilotListComboBox"
        if NextButton.IsEnabled then
            NextButton.Click
        else
            warnlog "In first window the 'Next'-button is disabled!"
            AutopilotListComboBox.Cancel
        end if
        sleep(1)
        printlog "  - Autopilot List/Combo Box: Page 2/4"
        Kontext "TabTableSelection"    
        TableSelection.Select("TT_Control")
        sleep(1)
        Kontext "AutopilotListComboBox"
        if NextButton.IsEnabled then
            NextButton.Click
        else
            warnlog "In second window the 'Next'-button is disabled!!"
            AutopilotListComboBox.Cancel
            goto endsub
        end if
        sleep(1)
        printlog "  - Autopilot List/Combo Box: Page 3/4"
        Kontext "TabFieldSelection"        
        '/// choose InfoColor as displayed field & click next
        ExistingFields.Select("InfoColor")
        sleep(3)
        Kontext "AutopilotListComboBox"
        if NextButton.IsEnabled then
            NextButton.Click
        else
            warnlog "In third window the 'Next'-button is disabled!"
            AutopilotListComboBox.Cancel
        end if
        sleep(1)
        printlog "  - Autopilot List/Combo Box: Page 4/4"
        Kontext "TabFieldLink"
        '/// take Colors as value field, ID as list field & click create
        ValueTable.Select("Colors")
        sleep(1)
        ListTable.Select("ID")
        sleep(1)
        Kontext "AutopilotListComboBox"
        if CreateButton.IsEnabled then
            CreateButton.Click
        else
            warnlog "In fourth window the 'Create'-button is disabled!"
            AutopilotListComboBox.Cancel
        end if
    else
        warnlog "The Autopilot doesn't start!"        
    end if
	sleep(1)
   
	Kontext "FormControls"    
	ControlProperties.Click
	sleep(1)	
    '/// check if the list box is in dropdown style		
	if fGetControlProperty("ListBox","DropDown") <> "2" then
		warnlog "The dropdown style is not active."
	endif			
	sleep(1)
	fSetControlProperty("ListBox","HelpURL","customcontrol1")
	sleep(1)
    '/// save the document
    printlog "save the document"
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_AP_ListBox.odt"))
	hCloseDocument
    sleep(1)
    '/// reload the document
    printlog "reload the document"
	hFileOpen ( gOfficePath + ConvertPath("user/work/TT_AP_ListBox.odt"))
    
    '/// open the listbox, and ccheck the list entries
	Kontext "MSC_Special_Dialog"	
	dim ii as integer
	ii = MSC_Special_Dialog.GetItemCount
	dim stext as string
	for i = 1 to ii
		MSC_Special_Dialog.Select (i)
		wait 500
		sText = MSC_Special_Dialog.GetSelText
		select case i
			case 1
				if sText <> "" then
					warnlog "The 1. entry is not empty"
				endif
			case 2
				if sText <> "red = 1" then
					warnlog "The 2. entry is not red = 1"
				endif				
			case 3
				if sText <> "green = 2" then
					warnlog "The 3. entry is not green = 2"
				endif								
			case 4
				if sText <> "blue = 3" then
					warnlog "The 4. entry is not blue = 3"
				endif
		end select					
	next
	sleep 1
	hCloseDocument
	
endcase

'--------------------------------------------------------------------

testcase tComboBox
	
    Dim k as integer
	printlog "- Formular toolbox: List box element (" & gApplication & ")"
	'/// open new text document
    call hNewDocument
	sleep(1)
    
    call hToolbarSelect("FormControls",true)
	
	'/// insert a new combo box
    Kontext "FormControls"
	 Combobox.Click
	call hDrawingWithSelection ( 50, 20, 60, 30 )
   	sleep(10)

    Kontext "AutoPilotListComboBox"
     AutoPilotListComboBox.MouseDown (10,10)
     AutoPilotListComboBox.MouseUp (10,10)
    if AutoPilotListComboBox.Exists(3) then
        printlog "  - Autopilot List/Combo Box: Page 1/4"
        Kontext "TabListBoxData"
         DataSource.Select(sDSName)
         Table.Select("TT_Control")
        Kontext "AutopilotListComboBox"
        if NextButton.IsEnabled then
            NextButton.Click
        else
            warnlog "In first window the 'Next'-button is disabled!"
            AutopilotListComboBox.Cancel
            goto endsub
        end if
        sleep(3)
        printlog "  - Autopilot List/Combo Box: Page 2/4"
        Kontext "TabTableSelection"
            TableSelection.Select("TT_Control")
            sleep(3)
        Kontext "AutopilotListComboBox"
        if NextButton.IsEnabled then
            NextButton.Click
        else
            warnlog "In second window the 'Next'-button is disabled!!"
            AutopilotListComboBox.Cancel
            goto endsub
        end if
        sleep(3)
        printlog "  - Autopilot List/Combo Box: Page 3/4"
        Kontext "TabFieldSelection"
            ExistingFields.Select(1)
            sleep(3)
        Kontext "AutopilotListComboBox"
        if NextButton.IsEnabled then
            NextButton.Click
        else
            warnlog "In third window the 'Next'-button is disabled!"
            AutopilotListComboBox.Cancel
            goto endsub
        end if
        sleep(3)
        printlog "  - Autopilot List/Combo Box: Page 4/4"
        Kontext "TabDatabaseField"        
        sleep(3)
        Kontext "AutopilotListComboBox"
        if CreateButton.IsEnabled then
            CreateButton.Click
        else
            warnlog "In fourth window the 'Create'-button is disabled!"
            AutopilotListComboBox.Cancel
            goto endsub
        end if
    else
        warnlog "- Autopilot for Combo Box didn't appear!"
    end if
    sleep(2)
	call hCloseDocument

endcase

'--------------------------------------------------------------------

testcase tTableControl

  printlog "- Formular toolbox: Tablecontrol element (" & gApplication & ")"
  '/// open a new textdocument & insert a table control
  call hNewDocument
  sleep(1)
    
  call hToolbarSelect("MoreControls",true)

  Kontext "MoreControls"
   Grid.Click
   call hDrawingWithSelection ( 40, 20, 60, 40 )
   sleep(10)
   '/// take as datasource table biblio, mark it & next
   '/// select some fields (in both directions) & create
   '/// switch off the design view & open the database beamer (TODO)
   '/// compare the header & the contents with the table control (TODO)
	Dim k as integer
	Kontext "AutopilotTableElement"
	AutopilotTableElement.MouseDown (10,10)
	AutopilotTableElement.MouseUp (10,10)
	if AutopilotTableElement.Exists(3) then
    	printlog "  - Autopilot Table Element: Page 1/2"
		Kontext "TabListBoxData"
		DataSource.Select(sDSName)
		sleep (1)
		Kontext "LoginDialog"
        if LoginDialog.Exists(3) then
        	printlog "login dialog"
                LoginPassword.SetText "testtool"
	        LoginDialog.OK
	        sleep (1)
        end if	    
        sleep (1)
   		Kontext "TabListBoxData"
		Table.Select("TT_Control")
		sleep(3)
		Kontext "AutopilotTableElement"
    	if NextButton.IsEnabled then
			NextButton.Click
		else
			warnlog "In first window the 'Next'-button is disabled!"
       		AutopilotTableElement.Cancel
		end if
		sleep(1)
		printlog "  - Autopilot Table Element: Page 2/2"
		Kontext "TabTableFieldSelection"
		ExistingFields.Select("ID")
		AddTo.Click
		ExistingFields.Select("Colors")
		AddTo.Click
		ExistingFields.Select("InfoColor")
		AddTo.Click
		sleep(1)
		Kontext "AutopilotTableElement"
		if CreateButton.IsEnabled then
			CreateButton.Click
		else
			warnlog "In fourth window the 'Create'-button is disabled!"
			AutopilotTableElement.Cancel
		end if
	else
		warnlog "- Autopilot for Table Element didn't appear!"
	end if
	sleep(2)
	
	hCloseDocument
	
endcase

'-------------------------------------------------------------------------

