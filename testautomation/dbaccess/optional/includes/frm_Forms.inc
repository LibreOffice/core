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
'* short description : Form Test
'*
'\***********************************************************************
global gDSName as String

sub frm_Forms(sdsname)
    
    gDSName = sdsname    
    call tCreateForm
    call tNavigate
    call tSearch
    call tSort     
    call tViewAsGrid
    call tNewSaveDelete
    call tCreateSubForms
    call tCheckSubForm
    call tCreateSubFormsNewMethod
    call tCheckSubForm
    
    'test forms inside a database
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/hsql_datasource/TT_hsqldb.odb"),gOfficePath + ConvertPath("user/work/TT_hsqldb.odb")    

    call tDatabaseFormsMain    
    call tNewFolder
    call tRenameFolder
    
    'activate after cws fwkdbdesign01 is integrated
    call tKeyboardNavigate    
 
    ' test for issue 78286
    call tViewAsGridEmbeddedForm

end sub
'--------------------------------------------------------------------------------------
testcase tDatabaseFormsMain

    call fOpenDatabase(gOfficePath + "user/work/TT_hsqldb.odb")
    
    call fOpenNewFormDesign
    
    call hToolbarSelect("FormControls",true)	
    
    '/// insert a textfield
    printlog "insert a textfield"
    Kontext "FormControls"   	        
        Edit.Click
        call hDrawingWithSelection ( 50, 20, 60, 30 )
        sleep(1)

    Kontext "FormControls"
        ControlProperties.Click
    
    Kontext "ControlPropertiesTabControl"
   	    ControlPropertiesTabControl.SetPage TabDataControl
        sleep(1)
        
    Kontext "TabDataControl"
        DataField.setText "id"        
        sleep(1)
        
    Kontext "ControlPropertiesDialog"        
        ControlPropertiesDialog.Close
        sleep(1)
                
    '/// save the form under the name TT_form1
    printlog "save the form under the name TT_form1"
    call fSaveForm( "TT_Form1", true )

    '/// close the database
    printlog "close the database"            
    call fCloseDatabase
    
    '/// open the database again
    printlog "open the database again"
    call fOpenDatabase(gOfficePath + "user/work/TT_hsqldb.odb")

    '/// open the form
    printlog "open the form"
    call fOpenForm("TT_Form1")
    
    '/// close the form
    printlog "close the form"
    call fCloseForm()

    '/// close the database
    printlog "close the database"
    call fCloseDatabase()
    
endcase
'--------------------------------------------------------------------------------------
testcase tCreateForm
    
    '/// open a new text document
    printlog "open a new text document"
	call hNewDocument
	sleep(1)
	'/// create a new form via the form navigator
    printlog "create a new form via the form navigator"
    call hToolbarSelect("FORMDESIGNTOOLS",true)	

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
        Liste.TypeKeys "<RETURN>"
        sleep(3)
        Liste.OpenContextmenu
        sleep(2)
        hMenuSelectNr (6)
        sleep(4)      
        
  	Kontext "FormPropertiesDialog"
   	    FormPropertiesTabControl.SetPage TabDataForm
        sleep(1)
  	Kontext "TabDataForm"   	
        '/// set the datasource gDatasourceName
        printlog "set the datasource " + gDSName          
        DataSource.setText(gDSName)
        TabDataForm.TypeKeys("<RETURN>")
        sleep(1)
        '/// set the table TT_Forms
        printlog "set the table TT_Forms"              
        Content.setText("TT_Forms")
        TabDataForm.TypeKeys("<RETURN>")
        sleep(1)
  	Kontext "FormPropertiesDialog"
        FormPropertiesDialog.Close
        sleep(3)
  	Kontext "FormularNavigator"
        FormularNavigator.Close
        sleep(4)

    call hToolbarSelect("FormControls",true)	

  	Kontext "FormControls"
   	'/// insert a textfield
    printlog "insert a textfield"        
    Edit.Click
   	call hDrawingWithSelection ( 50, 30, 60, 40 )
   	sleep(1)
    
    Kontext "FormControls"
        ControlProperties.Click
    
    Kontext "ControlPropertiesTabControl"
   	    ControlPropertiesTabControl.SetPage TabDataControl
        sleep(1)
    Kontext "TabDataControl"
        DataField.setText "id"
        TabDataControl.TypeKeys("<RETURN>")        
        sleep(1)            
    Kontext "ControlPropertiesDialog"        
        ControlPropertiesDialog.Close
        sleep(1)
    
    Kontext "FormControls"
	 SwitchControlDesignMode.Click	
   	 sleep(1)
     
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_Forms.odt"))
    
    call hCloseDocument
    
endcase
'-------------------------------------------------------------------------
testcase tNavigate
    
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Forms.odt"))
    
    FM_Next_Record
    sleep(1)
    FM_Last_Record
    sleep(1)
    FM_Previous_Record
    sleep(1)
    FM_First_Record
    sleep(1)
    
    hCloseDocument 
    
endcase
'-------------------------------------------------------------------------
testcase tKeyboardNavigate
    
    '/// open the database <OfficePath>/user/work/TT_hsqldb.odb
    printlog "open the database " + gOfficePath + "user/work/TT_hsqldb.odb"
    call fOpenDatabase(gOfficePath + "user/work/TT_hsqldb.odb")
    
    call fOpenForm("TT_Form1")
    
    Kontext "DocumentWriter"    
        DocumentWriter.TypeKeys("<MOD1 MOD2 END>")
    
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard = "3" then
            printlog "right record is displayed."
        else        
            warnlog "wrong record is displayed. It should be 3 but it is " + getClipboard
        endif    
    
        DocumentWriter.TypeKeys("<MOD1 MOD2 HOME>")
    
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard = "1" then
            printlog "right record is displayed."
        else        
            warnlog "wrong record is displayed. It should be 1 but it is " + getClipboard
        endif    
    
        DocumentWriter.TypeKeys("<MOD1 MOD2 RIGHT>")
    
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard = "2" then
            printlog "right record is displayed."
        else        
            warnlog "wrong record is displayed. It should be 2 but it is " + getClipboard
        endif    
    
        DocumentWriter.TypeKeys("<MOD1 MOD2 LEFT>")
    
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard = "1" then
            printlog "right record is displayed."
        else        
            warnlog "wrong record is displayed. It should be 1 but it is " + getClipboard
        endif
        
        sleep(10)
      
    call fCloseDatabase 
    
endcase
'-------------------------------------------------------------------------
testcase tSearch
    
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Forms.odt"))
    sleep(1)
    FM_Search
    sleep(1)
    Kontext "RecordSearch"
        SearchText.setText "3"
        Search.Click
        sleep(1)
        CloseBtn.Click
        
        sleep(1)
                
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)
		DocumentWriter.TypeKeys "<HOME>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard <> "3" then
            warnlog "wrong record is displayed"
        endif
        sleep(1)
    hCloseDocument 
    
endcase
'-------------------------------------------------------------------------
testcase tSort
    
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Forms.odt"))
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
    
    FM_Sort_Des
    sleep(1)
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)
		DocumentWriter.TypeKeys "<HOME>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard <> "3" then
            warnlog "wrong record is displayed. It is " + getClipboard + " but it have to 3"
        endif
    sleep(1)
    
    FM_Sort_Asc
    sleep(1)
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)
		DocumentWriter.TypeKeys "<HOME>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard <> "1" then
            warnlog "wrong record is displayed. It is " + getClipboard + " but it have to 1" 
        endif
    sleep(1)
    
    FM_Sort_Des
    sleep(1)    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)
		DocumentWriter.TypeKeys "<HOME>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard <> "3" then
            warnlog "wrong record is displayed. It is " + getClipboard + " but it have to 3" 
        endif
    sleep(1)
    
    FM_Remove_Filter
    sleep(1)
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)
		DocumentWriter.TypeKeys "<HOME>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard <> "1" then
            warnlog "wrong record is displayed. It is " + getClipboard + " but it have to 1" 
        endif
    sleep(1)
      
    FM_Sort_Order
    sleep(1)
    Kontext "SortOrder"
        FieldName1.select 2
        Order1.select 2
        SortOrder.OK
        
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)
		DocumentWriter.TypeKeys "<HOME>" , true
        sleep(1)        
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard <> "3" then
            warnlog "wrong record is displayed. It is " + getClipboard + " but it have to 3" 
        endif
    sleep(1)
    hCloseDocument 
    
endcase
'-------------------------------------------------------------------------
testcase tViewAsGrid
    
    '///open the document TT_Forms.odt
    printlog "pen the document TT_Forms.odt"
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Forms.odt"))
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true

    '/// click on the icon view as grid
    printlog "click on the icon view as grid"        
    FM_View_As_Grid
    
    sleep(1)

    '/// check if the grid appear
    printlog "check if the grid appear"    
    Kontext "TableView"    
        if CurrentRecord.getText <> "1" then
            warnlog "the first record is wrong"
        endif
        sleep(1)
        NextRecord.Click
        if CurrentRecord.getText <> "2" then
            warnlog "the next record button in the table grid doesn't work"
        endif
        sleep(1)        
        
    '/// close the view
    printlog "close the view"
    FM_View_As_Grid    

    sleep(1)

    '/// close the document
    printlog "close the document"
    hCloseDocument 
    
endcase
'-------------------------------------------------------------------------
testcase tNewSaveDelete
    
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_Forms.odt"))
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        
    FM_New_Record
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "7" , true
    
    FM_Save_Record
    
    FM_First_Record
    FM_Last_Record    
    
    FM_Delete_Record
      
    Kontext "MessageBox"
        if MessageBox.exists(3) then
            MessageBox.Yes
        else
            warnlog "No confirm dialog when press delete record"
        endif
        
    sleep(1)
    hCloseDocument 
    
endcase
'-------------------------------------------------------------------------
testcase tCreateSubForms
    
    '/// open a new text document
    printlog "open a new text document"
	call hNewDocument
	sleep(1)
	'/// create a new form via the form navigator
    printlog "create a new form via the form navigator"
    
    call hToolbarSelect("FORMDESIGNTOOLS",true)	

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
   	 Liste.TypeKeys "<RETURN>"
  	 sleep(3)
   	 Liste.OpenContextmenu
  	 sleep(2)
   	 hMenuSelectNr (6)
   	 sleep(4)      
        
  	Kontext "FormPropertiesDialog"
   	 FormPropertiesTabControl.SetPage TabDataForm
  	Kontext "TabDataForm"   	
   	 '/// set the datasource gDatasourceName
     printlog "set the datasource gDatasourceName"          
     DataSource.setText(gDSName)	
     TabDataForm.TypeKeys("<RETURN>")
     '/// set the table TT_Forms
     printlog "set the table TT_Forms"
     Content.setText("TT_Forms")
     sleep(1)
     Content.TypeKeys("<RETURN>",true)
     sleep(1)
  	Kontext "FormPropertiesDialog"
     sleep(1)
   	 FormPropertiesDialog.Close
   	sleep(3)
  	Kontext "FormularNavigator"
   	 FormularNavigator.Close
   	sleep(4)

    call hToolbarSelect("FormControls",true)	

  	Kontext "FormControls"
   	'/// insert a textfield
    printlog "insert a textfield"        
    Edit.Click
   	call hDrawingWithSelection ( 50, 30, 60, 40 )
   	sleep(1)
    
    Kontext "FormControls"
        ControlProperties.Click
    
    Kontext "ControlPropertiesTabControl"
   	    ControlPropertiesTabControl.SetPage TabDataControl
    Kontext "TabDataControl"
        DataField.setText "id"
        TabDataControl.TypeKeys("<RETURN>",true)
        sleep(1)            
    Kontext "ControlPropertiesDialog"        
        ControlPropertiesDialog.Close
    
    '/// now create the sub form
    
    call hToolbarSelect("FORMDESIGNTOOLS",true)	

    Kontext "FormDesignTools"
	    ShowFmExplorer.Click	
        sleep(2)

	Kontext "FormularNavigator" 
        Liste.TypeKeys "<HOME>" ' move to the first entry in the formnavigator tree
        sleep(1)
        Liste.TypeKeys "<LEFT>" ' collaps the tree
        sleep(1)
        Liste.TypeKeys "<RIGHT>" ' expand the tree
        sleep(1)
        Liste.TypeKeys "<DOWN>" ' go one down
        sleep(1)     
        Liste.OpenContextmenu
        sleep(2)
        hMenuSelectNr (1)
        sleep(2)
        hMenuSelectNr (1)
        sleep(3)
        Liste.TypeKeys "<RETURN>"
        sleep(3)
        Liste.OpenContextmenu
        sleep(2)
        hMenuSelectNr (6)
        sleep(4)      

    Kontext "FormPropertiesDialog"
       FormPropertiesTabControl.SetPage TabDataForm
  	 Kontext "TabDataForm"   	
       '/// set the datasource gDatasourceName
       printlog "set the datasource gDatasourceName"          
       DataSource.setText(gDSName)
       TabDataForm.TypeKeys("<RETURN>",true)	
       '/// set the contenttype SQL
       printlog "set the contenttype SQL"
       ContentType.select 3       
       sleep(1)       
       '/// set the sql string : select * from TT_Forms where id=:var1"
       printlog "set the sql string : select * from TT_Forms where id=:var1"
       TabDataForm.TypeKeys "<RETURN>" , true
       sleep(1)
       Content.TypeKeys "select * from TT_Forms where id=:var1" , true
       sleep(1)
       TabDataForm.TypeKeys("<RETURN>")
       sleep(1)
       '/// set link master field
       printlog "set link master field"
       TabDataForm.TypeKeys "<TAB>" , 11 , true
       sleep(1)
       TabDataForm.TypeKeys "id" , true
       sleep(1)
       TabDataForm.TypeKeys "<RETURN>" , true
       sleep(1)       
       '/// set link slave field
       printlog "set link slave field"
       TabDataForm.TypeKeys "<RETURN>" , true
       sleep(1)
       TabDataForm.TypeKeys "var1" , true
       sleep(1)  
       TabDataForm.TypeKeys("<RETURN>",true)
       sleep(1)
       
    Kontext "FormPropertiesDialog"
   	 FormPropertiesDialog.Close
   	 sleep(3)
    Kontext "FormularNavigator"
   	 FormularNavigator.Close
   	 sleep(4)

    call hToolbarSelect("FormControls",true)	

  	Kontext "FormControls"

       '/// insert a textfield
       printlog "insert a textfield"
       'Edit.Click
   	 call hDrawingWithSelection ( 50, 50, 60, 60 )
   	 sleep(1)
    
    Kontext "FormControls"
        ControlProperties.Click
    
    Kontext "ControlPropertiesTabControl"
   	    ControlPropertiesTabControl.SetPage TabDataControl
    Kontext "TabDataControl"
        DataField.setText "id"
        TabDataControl.TypeKeys("<RETURN>")
        sleep(1)            
    Kontext "ControlPropertiesDialog"        
        ControlPropertiesDialog.Close
    	     
    Kontext "FormControls"
	    SwitchControlDesignMode.Click	
   	 sleep(1)
     
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_SubForm.odt"))
        
    call hCloseDocument     
    
endcase
'-------------------------------------------------------------------------
testcase tCheckSubForm
   
    '/// open the file TTSubForm.sxw
    printlog "open the file TTSubForm.sxw"
    hFileOpen ( gOfficePath + ConvertPath("user/work/TT_SubForm.odt"))
   
    '/// set cursor into the first control
    printlog "set cursor into the first control"   
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F6>" , true
        wait(1000)
        DocumentWriter.TypeKeys "<MOD1 F5>" , true
        sleep(1)
           
    FM_Next_Record
    sleep(1)
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<TAB>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<SHIFT END>" , true
        sleep(1)
        DocumentWriter.TypeKeys "<MOD1 C>" , true
        sleep(1)        
        if getClipboard <> "2" then
            warnlog "wrong record is displayed in subform. It is " + getClipboard + " but it have to 2" 
        endif
    sleep(1)
    call hCloseDocument
   
endcase
'-------------------------------------------------------------------------
testcase tCreateSubFormsNewMethod
    
    '/// open a new text document
    printlog "open a new text document"
	call hNewDocument
	sleep(1)
	'/// create a new form via the form navigator
    printlog "create a new form via the form navigator"

    call hToolbarSelect("FORMDESIGNTOOLS",true)	

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
        Liste.TypeKeys "<RETURN>"
        sleep(3)
        Liste.OpenContextmenu
        sleep(2)
        hMenuSelectNr (6)
        sleep(4)      
    
  	Kontext "FormPropertiesDialog"
        FormPropertiesTabControl.SetPage TabDataForm
  	Kontext "TabDataForm"   	
        '/// set the datasource gDatasourceName
        printlog "set the datasource gDatasourceName"          
        DataSource.setText(gDSName)	
        TabDataForm.TypeKeys "<RETURN>" , true
        '/// set the table TT_Forms
        printlog "set the table TT_Forms"              
        Content.setText("TT_Forms")     
        TabDataForm.TypeKeys "<RETURN>" , true
        sleep 1
  	Kontext "FormPropertiesDialog"
        FormPropertiesDialog.Close
        sleep(3)
  	Kontext "FormularNavigator"
   	    FormularNavigator.Close
        sleep(4)

    call hToolbarSelect("FormControls",true)	

  	Kontext "FormControls"  	
        '/// insert a textfield
        printlog "insert a textfield"        
        Edit.Click
        call hDrawingWithSelection ( 50, 30, 60, 40 )
        sleep(1)
    
    Kontext "FormControls"
        ControlProperties.Click        
        sleep 1
    
    Kontext "ControlPropertiesTabControl"
   	    ControlPropertiesTabControl.SetPage TabDataControl
   	    sleep 1
    Kontext "TabDataControl"
        DataField.setText "id"
        TabDataControl.TypeKeys "<RETURN>" , true
        sleep(1)            
    Kontext "ControlPropertiesDialog"        
        ControlPropertiesDialog.Close
        sleep 1
    
    Kontext "DocumentWriter"
    	DocumentWriter.TypeKeys("subform",true)
    
    '/// now create the sub form
    call hToolbarSelect("FORMDESIGNTOOLS",true)	

    Kontext "FormDesignTools"
	    ShowFmExplorer.Click	
        sleep(2)
       
	Kontext "FormularNavigator" 
        Liste.TypeKeys "<HOME>" ' move to the first entry in the formnavigator tree
        sleep(1)
        Liste.TypeKeys "<LEFT>" ' collaps the tree
        sleep(1)
        Liste.TypeKeys "<RIGHT>" ' expand the tree
        sleep(1)
        Liste.TypeKeys "<DOWN>" ' go one down
        sleep(1)     
        Liste.OpenContextmenu
        sleep(2)
        hMenuSelectNr (1)
        sleep(2)
        hMenuSelectNr (1)
        sleep(3)
        Liste.TypeKeys "<RETURN>"
        sleep(3)
        Liste.OpenContextmenu
        sleep(2)
        hMenuSelectNr (6)
        sleep(4)      

    Kontext "FormPropertiesDialog"
        FormPropertiesTabControl.SetPage TabDataForm
  	Kontext "TabDataForm"   	
        '/// set the datasource gDatasourceName
        printlog "set the datasource gDatasourceName"          
        DataSource.setText(gDSName)	
        TabDataForm.TypeKeys("<RETURN>", true)
        sleep 1
        '/// set the contenttype SQL
        printlog "set the contenttype to table"
        ContentType.select 1
        TabDataForm.TypeKeys("<RETURN>", true)
        sleep(1)       
        '/// set the table TT_Forms
        printlog "set the table TT_Forms"
        Content.select 1
        sleep(1)
        TabDataForm.TypeKeys("<RETURN>", true)
        sleep 1
        '/// open the Form Link dialog set link master field
        printlog "open the Form Link dialog set link master field"       
        FormLinkDLGButton.click
        sleep(1)
    Kontext "FormLinkDialog"       
        '/// select the field id for the master and the link field
        printlog "select the field id for the master and the link field"
        LinkDetail1.select 1
        LinkMaster1.select 1
        '/// close the dialog with OK
        printlog "close the dialog with OK"
        FormLinkDialog.OK         
       
    Kontext "FormPropertiesDialog"
        FormPropertiesDialog.Close
        sleep(3)
    
    Kontext "FormularNavigator"
        FormularNavigator.Close
        sleep(4)

    call hToolbarSelect("FormControls",true)	

  	Kontext "FormControls"
    
       '/// insert a textfield
       printlog "insert a textfield"
       'Edit.Click
       call hDrawingWithSelection ( 50, 40, 60, 50 )
       sleep(1)
    
    Kontext "FormControls"
        ControlProperties.Click
    
    Kontext "ControlPropertiesTabControl"
   	    ControlPropertiesTabControl.SetPage TabDataControl
    Kontext "TabDataControl"
        DataField.setText "id"
        TabDataControl.TypeKeys("<RETURN>" , true)
        sleep(1)            
    Kontext "ControlPropertiesDialog"        
        ControlPropertiesDialog.Close
    sleep(1)
    
    Kontext "FormControls"
	    SwitchControlDesignMode.Click	
   	 sleep(1)
    
    hFileSaveAsKill ( gOfficePath + ConvertPath("user/work/TT_SubForm.odt"))
        
    call hCloseDocument     
    
endcase
'--------------------------------------------------------------------------------------
testcase tNewFolder

    '/// test the folder feature inside the form container
    
    '/// open the database <OfficePath>/user/work/TT_hsqldb.odb
    printlog "open the database " + gOfficePath + "user/work/TT_hsqldb.odb"
    call fOpenDatabase(gOfficePath + "user/work/TT_hsqldb.odb")
    
    '/// display the form container
    printlog "display the form container"
    Kontext "ContainerView"    
        ViewForms
        if ( Not FormTree.exists(1) ) then
            warnlog "The form tree doesn't exists"            
            goto endsub
        end if

    '/// insert a new folder via INSERT/FOLDER menu
    printlog "insert a new folder via INSERT/FOLDER menu"
    Kontext "DATABASE"
        Database.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(8)

    '/// insert a new folder name
    printlog "insert a new folder name"
    Kontext "FormFolderNameDialog"
        Foldername.setText("Folder1")
        FormFolderNameDialog.OK

    '/// close the database and save changes
    printlog "close the database and save changes"
    call fCloseDatabase(true)

    '/// open the database again
    printlog "open the database again"
    call fOpenDatabase(gOfficePath + "user/work/TT_hsqldb.odb")    

    '/// check if he folder still exists
    printlog "check if he folder still exists"
    if (fFindForm("Folder1")) then
        printlog "Folder available after save and reload"
    else
        warnlog "Folder _not_ available after save and reload"
    endif

    '/// close the database again
    printlog "close the database again"
    call fCloseDatabase()
    
endcase
'--------------------------------------------------------------------------------------
testcase tRenameFolder

    '/// test the folder feature inside the form container
    '/// test for issue 75723
    
    '/// open the database <OfficePath>/user/work/TT_hsqldb.odb
    printlog "open the database " + gOfficePath + "user/work/TT_hsqldb.odb"
    call fOpenDatabase(gOfficePath + "user/work/TT_hsqldb.odb")
    
    '/// display the form container
    printlog "display the form container"
    Kontext "ContainerView"    
        ViewForms
        if ( Not FormTree.exists(1) ) then
            warnlog "The form tree doesn't exists"            
            goto endsub
        end if

    '/// insert a new folder via INSERT/FOLDER menu
    printlog "insert a new folder via INSERT/FOLDER menu"
    Kontext "DATABASE"
        Database.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(8)

    '/// insert a new folder name
    printlog "insert a new folder name"
    Kontext "FormFolderNameDialog"
        Foldername.setText("Folder2")
        FormFolderNameDialog.OK

    '/// check if he folder still exists
    printlog "check if he folder still exists"
    if (fFindForm("Folder2")) then
        printlog "new folder available"
    else
        warnlog "new folder not available"
        goto endsub
    endif

    '/// insert a new folder under the first folder
    printlog "insert a new folder under the first folder"   
    
    Kontext "DATABASE"
        Database.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(8)

    '/// insert a new folder name
    printlog "insert a new folder name"
    Kontext "FormFolderNameDialog"
        Foldername.setText("Folder3")
        FormFolderNameDialog.OK
    
    '/// select the first folder
    printlog "select the first folder"    
    if (fFindForm("Folder2")) then
        printlog "first folder selected"
    else
        warnlog "first folder not found"
        goto endsub
    endif        

    '/// rename the firt folder
    printlog "rename the firt folder"    
    Kontext "DATABASE"
        Database.UseMenu
        hMenuSelectNr(2)
        hMenuSelectNr(7)

    Kontext "FormFolderNameDialog"
        FolderReName.setText("Folder4")
        FormFolderNameDialog.OK        

    '/// check if the folder still exists
    printlog "check if the folder still exists"
    if (fFindForm("Folder3")) then
        printlog "folder found"
    else
        warnlog "#i75723# folder not found."        
    endif
        
    '/// close the database and discade changes
    printlog "close the database and discade changes"
    call fCloseDatabase(false)
    
endcase
'-------------------------------------------------------------------------
testcase tViewAsGridEmbeddedForm
    
    '/// open the database <OfficePath>/user/work/TT_hsqldb.odb
    printlog "open the database " + gOfficePath + "user/work/TT_hsqldb.odb"
    call fOpenDatabase(gOfficePath + "user/work/TT_hsqldb.odb")
    
    
    '/// open form Form1
    printlog "open form Form1"    
        call fOpenForm("Form1")
    
    Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 F5>" , true

    '/// click on the icon view as grid
    printlog "click on the icon view as grid"        
    FM_View_As_Grid
    
    sleep(1)

    '/// check if the grid appear
    printlog "check if the grid appear"    
    Kontext "TableView"    
        if CurrentRecord.getText <> "1" then
            warnlog "the first record is wrong"
        endif
        sleep(1)
        NextRecord.Click
        if CurrentRecord.getText <> "2" then
            warnlog "the next record button in the table grid doesn't work"
        endif
        sleep(1)        
        
    '/// close the view
    printlog "close the view"
    FM_View_As_Grid    

    sleep(1)

    '/// close the form
    printlog "close the form"
    fCloseForm
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase
    
endcase

