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
'* short description : Property Browser
'*
'\***********************************************************************

sub ctrl_PropertyBrowser
	
    call tPushButton
    call tTextBox
    call tListBox
    call tComboBox        
    call tImageButton    
    call tImageControl        
    call tForms

end sub
'--------------------------------------------------------------------------------------
testcase tPushButton

    call sCloseAllToolbars	

    '/// Open a new document
    printlog "Open a new document"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true)
	
  	Kontext "FormControls"   	
   	PushButton.Click
   	sleep(1)      
	'/// insert a Push Button
    printlog "insert a Push Button"    
    call hDrawingWithSelection ( 50, 20, 60, 30 )
	sleep(2)
	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    '/// change Buttontype to URL
    printlog "change Buttontype to URL"
	Kontext "TabGeneralControl"	
    fsetControlProperty("PushButton","ButtonType", "4")
    '/// check if the URLField property is enabled
    printlog "check if the URLField property is enabled"
    if NOT URL.isEnabled then
        warnlog "The URL fields doesn't gets enabled"  
    end if
    '/// add url to URLField
    printlog "add url to URLField"
    fsetControlProperty("PushButton","URL", "http://www.sun.com")
    Kontext "TabGeneralControl"
        TabGeneralControl.typekeys("<RETURN>",true)
	'/// check if the Frame property is enabled
    printlog "check if the Frame property is enabled"
    if NOT Frame.isEnabled then
        warnlog "The Frame fields doesn't gets enabled"  
    end if
    sleep(1)
    '/// add graphicfile to URLField
    printlog "add graphicfile to URLField"
    fsetControlProperty("PushButton","Graphics", gOfficePath + ConvertPath("share/gallery/bullets/blkpearl.gif"))
    '/// check if the GraphicsAlignment property is enabled
    printlog "check if the GraphicsAlignment property is enabled"
    if NOT GraphicsAlignment.isEnabled then
        warnlog "The GraphicsAlignment doesn't gets enabled"
    end if
    '/// close the controlproperty browser    
    Kontext "ControlPropertiesDialog"
    ControlPropertiesDialog.close
    sleep(1)	
    hCloseDocument
    
endcase
'--------------------------------------------------------------------------------------
testcase tTextBox
    
    call sCloseAllToolbars
    
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
	'/// insert a text box
    printlog "insert a text box"                    
    call hDrawingWithSelection ( 50, 20, 60, 30 
	sleep(2)
	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    '/// change the propety multiline to yes
    printlog "change the propety multiline to yes"    
    fsetControlProperty("TextBox","TextType","2")
    '/// check if the scrollbars property is enabled
    printlog "check if the scrollbars property is enabled"
    if NOT Scrollbars.isEnabled then
        warnlog "The scrollbars doesn't gets enabled"  
    end if
    '/// check if the password char is enabled
    printlog "check if the password char is enabled"
    if Password.isEnabled then
        warnlog "The pasword is enabled ( issue 29477 ) "  
    end if
        
    '/// change the propety multiline to no
    printlog "change the propety multiline to no"
    fsetControlProperty("TextBox","TextType","1")
    '/// check if the scrollbars property is disabled
    printlog "check if the scrollbars property is disabled"
    if Scrollbars.isEnabled then
        warnlog "The scrollbars is enabled"  
    end if
    '/// check if the password char is disabled
    printlog "check if the password char is disabled"
    if NOT Password.isEnabled then
        warnlog "The pasword doesn't gets enabled"  
    end if
    '/// close the controlproperty browser    
    Kontext "ControlPropertiesDialog"
    ControlPropertiesDialog.close
    '/// close the document
    printlog "close the document"                
    sleep(1)    	
    hCloseDocument  

endcase
'--------------------------------------------------------------------------------------
testcase tListBox  

    call sCloseAllToolbars   
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true)
	    
  	Kontext "FormControls"
    ListBox.Click
    sleep(1)
    UseWizards.Click
   	sleep(1)
	'/// insert a list box
    printlog "insert a list box"                        
    call hDrawingWithSelection ( 50, 20, 60, 30 
	sleep(2)
	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    '/// change the property DropDown to yes
    printlog "change the propety multiline to yes"    
    fsetControlProperty("ListBox","DropDown","2")
    '/// check if the scrollbars property is enabled
    printlog "check if the LineCount property is enabled"
    if NOT LineCount.isEnabled then
        warnlog "The LineCount doesn't gets enabled"  
    end if
    
        
    '/// change the propety multiline to no
    printlog "change the propety multiline to no"
    fsetControlProperty("ListBox","DropDown","1")
    '/// check if the scrollbars property is disabled
    printlog "check if the LineCount property is disabled"
    if LineCount.isEnabled then
        warnlog "The LineCount is enabled"  
    end if
    
    Kontext "FormControls" 
        UseWizards.Click
    '/// close the controlproperty browser    
    Kontext "ControlPropertiesDialog"
    ControlPropertiesDialog.close    
    '/// close the document
    printlog "close the document"                
    sleep(1)    	
    hCloseDocument  

endcase
'--------------------------------------------------------------------------------------
testcase tComboBox     

    call sCloseAllToolbars
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true)
	    
  	Kontext "FormControls"    
   	ComboBox.Click
    sleep(1)          
    UseWizards.Click
   	sleep(1)
	'/// insert a list box
    printlog "insert a list box"                        
    call hDrawingWithSelection ( 50, 20, 60, 30 
	sleep(2)
	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    '/// change the property DropDown to yes
    printlog "change the propety multiline to yes"    
    fsetControlProperty("ComboBox","DropDown","2")
    '/// check if the scrollbars property is enabled
    printlog "check if the LineCount property is enabled"
    if NOT LineCount.isEnabled then
        warnlog "The LineCount doesn't gets enabled"  
    end if
    
        
    '/// change the propety multiline to no
    printlog "change the propety multiline to no"
    fsetControlProperty("ComboBox","DropDown","1")
    '/// check if the scrollbars property is disabled
    printlog "check if the LineCount property is disabled"
    if LineCount.isEnabled then
        warnlog "The LineCount is enabled"  
    end if
    
    Kontext "FormControls" 
        UseWizards.Click    
    '/// close the controlproperty browser    
    Kontext "ControlPropertiesDialog"
    ControlPropertiesDialog.close
    '/// close the document
    printlog "close the document"                
    sleep(1)    	
    hCloseDocument  

endcase
'--------------------------------------------------------------------------------------
testcase tImageButton

    call sCloseAllToolbars
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)
	
  	call hToolbarSelect("MoreControls",true)

    Kontext "MoreControls"    
   	Imagebutton.Click
   	sleep(1)      
	'/// insert an image control
    printlog "insert an image control"                        
    call hDrawingWithSelection ( 50, 20, 60, 30 
	sleep(2)
    call hToolbarSelect("FormControls",true)
	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    '/// change the property ButtonType to URL
    fsetControlProperty("ImagesButton","ButtonType", "4")
    '/// check if the URL property is enabled
    if NOT URL.isEnabled then
        warnlog "The URL fields doesn't gets enabled"  
    end if
    '/// add url to URLField
    printlog "add url to URLField"
    'ISSUE: see i29478
    'fsetControlProperty("ImagesButton","URL", "www.sun.com")
    fsetControlProperty("ImagesButton","URL", "http://www.sun.com")
    Kontext "TabGeneralControl"
        TabGeneralControl.typekeys("<RETURN>",true)
  	'/// check if the Frame property is enabled
    printlog "check if the Frame property is enabled"
	if NOT Frame.isEnabled then
        warnlog "The Frame fields doesn't gets enabled"  
    end if
    sleep(1)
    '/// add graphicfile to GraphicField
    fsetControlProperty("ImagesButton","Graphics", gOfficePath + ConvertPath("share/gallery/bullets/blkpearl.gif"))
    '/// check if the GraphicsAlignment property is enabled
    printlog "check if the Scale property is enabled"
    if NOT Scale.isEnabled then
        warnlog "The Scale doesn't gets enabled"
    end if
    '/// close the controlproperty browser    
    Kontext "ControlPropertiesDialog"
    ControlPropertiesDialog.close
    sleep(1)    	
    hCloseDocument  

endcase
'--------------------------------------------------------------------------------------
testcase tImageControl  

    call sCloseAllToolbars   
    
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)
	
    call hToolbarSelect("MoreControls",true)
    Kontext "MoreControls"
       	ImageControl.Click
    
   	sleep(1)      
	'/// insert an image control
    printlog "insert an image control"                        
    call hDrawingWithSelection ( 50, 20, 60, 30 
	sleep(2)
    call hToolbarSelect("FormControls",true)
	Kontext "FormControls"
	ControlProperties.Click
	sleep(2)
    '/// add graphicfile to GraphicField
    printlog "add graphicfile to GraphicField"
    fsetControlProperty("ImagesButton","Graphics", gOfficePath + ConvertPath("share/gallery/bullets/blkpearl.gif"))
    '/// check if the Scale property is enabled
    printlog "check if the Scale property is enabled"
    if NOT Scale.isEnabled then
        warnlog "The Scale doesn't gets enabled"
    end if    
    '/// close the controlproperty browser    
    Kontext "ControlPropertiesDialog"
    ControlPropertiesDialog.close
    sleep(1)    
    hCloseDocument  

endcase
'--------------------------------------------------------------------------------------
testcase tForms

    call sCloseAllToolbars
	
    '/// Open a new document,
    printlog "Open a new document,"
	call hNewDocument
    Kontext "DocumentWriter"       
	DocumentWriter.TypeKeys "tt_doc" , true
	sleep(1)

    call hToolbarSelect("FormControls",true)

  	Kontext "FormControls"  	
   	PushButton.Click
   	sleep(1)      
	'/// insert a Push Button
    printlog "insert a Push Button"    
    call hDrawingWithSelection ( 50, 20, 60, 30 
	sleep(2)
	Kontext "FormControls"
	FormProperties.Click
	sleep(2)
    
    Kontext "FormPropertiesDialog"
        FormPropertiesTabControl.setPage TabGeneralForm
	
    Kontext "TabGeneralForm"	     
        '/// set encoding to Multipart
        printlog "set encoding to Multipart"
        Encoding.select 2
        sleep(1)
        '/// check if the SubmitMethod get enabled
        printlog "check if the SubmitMethod get enabled"        
        if SubmitMethod.isEnabled then
            warnlog "The SubmitMethod fields doesn't gets enabled"  
        end if
        '/// set encoding to URL
        printlog "set encoding to URL"        
        Encoding.select 1
        sleep(1)
        '/// check if the SubmitMethod get disabled
        printlog "check if the SubmitMethod get disabled"        
        if NOT SubmitMethod.isEnabled then
            warnlog "The SubmitMethod fields doesn't gets disabled"  
        end if
        '/// set URL
        printlog "set URL"         
        'ISSUE: see i29478
        'URL.setText("www.sun.com")
        URL.setText("http://www.sun.com")
        TabGeneralForm.TypeKeys("<RETURN>")
        '/// check if the frame property gets enabled
        printlog "check if the frame property gets enabled"
        if NOT Frame.isEnabled then
            warnlog "The frame fields doesn't gets enabled"  
        end if
        
    Kontext "FormPropertiesDialog"
		FormPropertiesTabControl.setPage TabDataForm
	
    Kontext "TabDataForm"
        '/// select first datasource
        printlog "select first datasource"
        DataSource.select 1
        sleep(2)
        '/// select SQL Command as Type
        printlog "select SQL Command as Type"
        ContentType.select 3
        sleep(2)
        '/// check if the SQL Design button gets enabled
        printlog "check if the SQL Design button gets enabled"
        if OpenQueryDesign.isenabled then
            '/// open the query design with the button
            printlog "open the query design with the button"
            OpenQueryDesign.click
            sleep(5)
            '/// check if the querydesign open
            printlog "check if the querydesign open"
            Kontext "QueryDesignTable"            
            if NOT QueryDesignTable.exists(3) then
                warnlog "The Query Design doesn't open."
            end if
            Kontext "AddTables"
                AddTables.Close
            Kontext "QueryDesignTable"
            QueryDesignTable.UseMenu
            hMenuSelectNr(1)
            hMenuSelectNr(4)           
            
        else    
            warnlog "the openQueryDesign button is not enabled"
        end if
    Kontext "TabDataForm"  
        printlog "select table as Type"
        ContentType.select 1
        sleep(1)
        Content.setText("biblio")
        sleep(1)
        TabDataForm.TypeKeys "<RETURN>" , true
    
    '/// close the FormProperty Browser
    printlog "close the FormProperty Browser"    
    Kontext "ControlPropertiesDialog"
        ControlPropertiesDialog.close
    
    '/// open the form property browser
    printlog "open the form property browser"
    Kontext "FormControls"
        FormProperties.Click
        sleep(2)

    Kontext "FormPropertiesDialog"
		FormPropertiesTabControl.setPage TabDataForm

    '/// click on the filter button
    Kontext "TabDataForm"
    FilterDLGButton.Click
    '/// check if the filter dialog appear
    Kontext "FilterDialog"
        FilterDialog.OK

    '/// click on the sort button
    Kontext "TabDataForm"
    SortDLGButton.click
    '/// check if the sort dialog appear
    Kontext "SortOrder"
        SortOrder.OK
        
    '/// close the controlproperty browser    
    Kontext "ControlPropertiesDialog"
    ControlPropertiesDialog.close
    sleep(1)	
    hCloseDocument
    
endcase
