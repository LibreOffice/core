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
'* short description : XForms Main Test include file
'*
'************************************************************************
'*
' #1 tPropertyBrowser
' #1 tDataNavigator
' #1 tDataNavigatorSubmission
' #1 tDataNavigatorBinding
' #1 tDataNavigatorInstance
' #1 tDataNavigatorInstanceData
' #1 tDataNavigatorModel    
'*
'\***********************************************************************************

sub xforms_01

   printlog "------------------ Xforms01.inc ---------------------"
   
   call tDataNavigatorModel
   call tDataNavigatorInstance
   call tDataNavigatorInstanceData   
   call tDataNavigatorSubmission
   call tDataNavigatorBinding
   call tPropertyBrowser

end sub
'-------------------------------------------------------------------------
testcase tDataNavigatorModel    
    
    '/// open new XML Form
    printlog "open new XML Form"
    
    FileOpen "FileName", "private:factory/swriter?slot=21053" , "FrameName", "_default" , "SynchronMode" ,True
        
    sleep(5)        
	
    call hToolbarSelect("FormDesignTools",true)
        
    '/// open the datanavigator
    printlog "open the datanavigator"
    
    Kontext "XFormsDataNavigator"
        if ( not XFormsDataNavigator.exists(2) ) then
            Kontext "FormDesignTools"
            XFormsDataNavigator.Click
        endif

	sleep(1)

    Kontext "XFormsDataNavigator"
        'call DialogTest(XFormsDataNavigator)
		XFormsDataNavigator.TypeKeys "<space>" , true
        '/// open the add model dialog
        printlog "open the add model dialog"
        sleep(5)
        ModelsBtn.Click
        sleep(1)        
        hMenuSelectNr(1)
        
    sleep(1)
   
    Kontext "XFormAddModel"    
        call DialogTest(XFormAddModel)    
        '/// add Model2 as new name
        printlog "add Model2 as new name"
        ModelName.setText "Model2"    
        '/// click OK
        printlog "click OK"
        XFormAddModel.OK
    
    Kontext "XFormsDataNavigator"
        if ( Models.getSelText <> "Model2" ) then
            warnlog "The new Model is not added"    
        endif
        '/// rename the Model
        printlog "rename the Model"
        ModelsBtn.Click        
        hMenuSelectNr(2)
        sleep(1)
        
    Kontext "XFormAddModel"
        ModelName.setText "Model3"    
        XFormAddModel.OK
    
    Kontext "XFormsDataNavigator"
        if ( Models.getSelText <> "Model3" ) then
            warnlog "The Model is not renamed"
        endif

    '/// close the xform data navigator
    printlog "close the xform data navigator"
    
    Kontext "XFormsDataNavigator"
        XFormsDataNavigator.Close
        sleep(1)
        
    '/// close the document
    printlog "close the document"
    
    call hCloseDocument
        
endcase  
'-------------------------------------------------------------------------
testcase tDataNavigatorInstance

    '/// open new XML Form
    printlog "open new XML Form"
    
    FileOpen "FileName", "private:factory/swriter?slot=21053" , "FrameName", "_default" , "SynchronMode" ,True
        
    sleep(5)        
	
    call hToolbarSelect("FormDesignTools",true)
    '/// open the datanavigator
    printlog "open the datanavigator"
    Kontext "XFormsDataNavigator"
        if ( not XFormsDataNavigator.exists(2) ) then
            Kontext "FormDesignTools"
            XFormsDataNavigator.Click
        endif
    
	sleep(1)
    
    '/// add a new instance
    printlog "add a new instance"
    Kontext "XFormsDataNavigator"
        XFormsDataNavigator.TypeKeys "<space>" ,true
        InstanceBtn.Click
        hMenuSelectNr(1)
        
    Kontext "XFormAddInstance"
        call DialogTest(XFormAddInstance)
        InstanceName.setText "Instance2"
        'InstanceURL
        'LinkInstance
        '/// open the browse dialog and close it again
        printlog "open the browse dialog and close it again"
        BrowseBtn.Click
    
    Kontext "GeneralFileDialog"
        GeneralFileDialog.Cancel           
    
    Kontext "XFormAddInstance"
        XFormAddInstance.OK  
        sleep(1)
    
    '/// rename new instance
    printlog "rename new instance"
    Kontext "XFormsDataNavigator"
	    XFormsDataNavigator.TypeKeys "<space>" ,true
	    sleep(1)
        InstanceBtn.Click
        sleep(1)        
        hMenuSelectNr(2)
        sleep(1)

    Kontext "XFormAddInstance"        
        InstanceName.setText "Instance3"
        sleep(1)
        XFormAddInstance.OK
        sleep(1)
    'add a second instance due issue 
    Kontext "XFormsDataNavigator"
  	    XFormsDataNavigator.TypeKeys "<space>" ,true
        InstanceBtn.Click
        sleep(1)        
        hMenuSelectNr(1)
        sleep(1)        
        
    Kontext "XFormAddInstance"        
        InstanceName.setText "Instance4"
        sleep(1)
        XFormAddInstance.OK
        sleep(1)

    '/// delete instance     
    Kontext "XFormsDataNavigator"
        XFormsDataNavigator.TypeKeys "<space>" ,true
        InstanceBtn.Click
        sleep(1)
        hMenuSelectNr(3)
        sleep(1)
    
    Kontext "MessageBox"
        MessageBox.Yes
        sleep(1)
        
    '/// close the xform data navigator
    printlog "close the xform data navigator"
    
    Kontext "XFormsDataNavigator"
        XFormsDataNavigator.Close
        sleep(1)
        
    '/// close th document
    printlog "close th document"
    
    call hCloseDocument
    
endcase
'-------------------------------------------------------------------------
testcase tDataNavigatorInstanceData

    '/// open new XML Form
    printlog "open new XML Form"
    
    FileOpen "FileName", "private:factory/swriter?slot=21053" , "FrameName", "_default", "SynchronMode" ,True
        
    sleep(5)        
	
    call hToolbarSelect("FormDesignTools",true)
    
    sleep(1)
    
    '/// open the datanavigator
    printlog "open the datanavigator"
    Kontext "XFormsDataNavigator"
        if ( not XFormsDataNavigator.exists(2) ) then
            Kontext "FormDesignTools"
            XFormsDataNavigator.Click
        endif
    
    '/// select the submission tabpage
    printlog "select the submission tabpage"
    
    Kontext "XFormsDataNavigator"
        call DialogTest(XFormsDataNavigator)
        while ( XFormsDataNavigatorTabControl.getPageID() <> 12 )
            XFormsDataNavigatorTabControl.TypeKeys("<RIGHT>")
        wend
        ItemList.select 1
    '/// click the add element icon
    printlog "click the add element icon"
    
    Kontext "XFormToolbar"
        AddElement.Click
        
    Kontext "XFormAddItemDLG"
        call DialogTest(XFormAddItemDLG)
        ElementName.setText "element1"
        if iSprache = 82 then
            printlog "because of issue i49557 there are only 12 items in a korean version."
            if ( DataType.getItemCount <> 12) then 
                warnlog "there are not 12 data types in the listbox there are " + DataType.getItemCount + ". See issue i49557."
            endif
        else
            if ( DataType.getItemCount <> 13) then 
                warnlog "there are not 13 data types in the listbox there are " + DataType.getItemCount + ". See issue i49557."
            endif
        endif        
            Required.check
            RequiredBtn.click
    Kontext "AddConditionDLG"
        AddConditionDLG.Close
    Kontext "XFormAddItemDLG"
        Relevant.check
        RelevantBtn.click
    Kontext "AddConditionDLG"
        AddConditionDLG.Close
    Kontext "XFormAddItemDLG"
        Constraint.check
        ConstraintBtn.click
    Kontext "AddConditionDLG"
        AddConditionDLG.Close
    Kontext "XFormAddItemDLG"
        Calculate.check
        CalculateBtn.click
    Kontext "AddConditionDLG"
        AddConditionDLG.Close
    Kontext "XFormAddItemDLG"
        ReadOnly.check
        ReadOnlyBtn.click        
    Kontext "AddConditionDLG"
        AddConditionDLG.Close
    Kontext "XFormAddItemDLG"
        XFormAddItemDLG.OK
        
    '/// close the xform data navigator
    printlog "close the xform data navigator"
    
    Kontext "XFormsDataNavigator"
        XFormsDataNavigator.Close
        sleep(1)
        
    '/// close the document
    printlog "close the document"
    
    call hCloseDocument    
        
endcase
'-------------------------------------------------------------------------
testcase tDataNavigatorSubmission    
    
    '/// nopen ew XML Form
    printlog "nopen ew XML Form"
    
    FileOpen "FileName", "private:factory/swriter?slot=21053" , "FrameName", "_default", "SynchronMode" ,True
        
    sleep(5)        
	
    call hToolbarSelect("FormDesignTools",true)
    
    sleep(1)
    
    '/// open the datanavigator
    printlog "open the datanavigator"
    Kontext "XFormsDataNavigator"
        if ( not XFormsDataNavigator.exists(2) ) then
            Kontext "FormDesignTools"
            XFormsDataNavigator.Click
        endif
	sleep(1)
    
    '/// select the submission tabpage
    printlog "select the submission tabpage"
    
    Kontext "XFormsDataNavigator"
        call DialogTest(XFormsDataNavigator)
        while ( XFormsDataNavigatorTabControl.getPageID() <> 10 )
            XFormsDataNavigatorTabControl.TypeKeys("<RIGHT>")
        wend
    
    '/// click the add submission icon
    printlog "click the add submission icon"
    
    Kontext "XFormToolbar"
        AddSubmission.Click
        
    Kontext "XFormAddSubmission"
        call DialogTest(XFormAddSubmission)
        
        'i35171 crash when click
        'SubmitBindingExpressionBtn.Click
        
        '/// close the add submission dialog
        printlog "close the add submission dialog"    
        
        XFormAddSubmission.Close
        
    sleep(10)
    
    '/// close the xform data navigator
    printlog "close the xform data navigator"
    
    Kontext "XFormsDataNavigator"
        XFormsDataNavigator.Close
        sleep(1)
        
    '/// close th document
    printlog "close th document"
    
    call hCloseDocument
    
endcase
'-------------------------------------------------------------------------
testcase tDataNavigatorBinding    
    
    '/// open new XML Form
    printlog "open new XML Form"
    
    FileOpen "FileName", "private:factory/swriter?slot=21053" , "FrameName", "_default", "SynchronMode" ,True
        
    sleep(5)        
	
    call hToolbarSelect("FormDesignTools",true)

    sleep(1)
    
    '/// open the datanavigator
    printlog "open the datanavigator"
    Kontext "XFormsDataNavigator"
        if ( not XFormsDataNavigator.exists(2) ) then
            Kontext "FormDesignTools"
            XFormsDataNavigator.Click
        endif
    
	sleep(1)

    '/// select the binding tabpage
    printlog "select the binding tabpage"
    
    Kontext "XFormsDataNavigator"
        call DialogTest(XFormsDataNavigator)
        while ( XFormsDataNavigatorTabControl.getPageID() <> 11 )
            XFormsDataNavigatorTabControl.TypeKeys("<RIGHT>")
        wend
    
    '/// click the add binding icon
    printlog "click the add binding icon"
    
    Kontext "XFormToolbar"
        AddBinding.Click
        
    Kontext "XFormAddItemDLG"
        call DialogTest(XFormAddItemDLG)

        '/// close the add binding dialog
        printlog "close the add binding dialog"
        
        XFormAddItemDLG.Close
        
    sleep(10)
    
    '/// close the xform data navigator
    printlog "close the xform data navigator"
        
    Kontext "XFormsDataNavigator"
        XFormsDataNavigator.Close
        sleep(1)

    '/// close th document
    printlog "close th document"
    
    call hCloseDocument
    
endcase
'-------------------------------------------------------------------------

testcase tPropertyBrowser

    '/// new XML Form
    
    FileOpen "FileName", "private:factory/swriter?slot=21053" , "FrameName", "_default", "SynchronMode" ,True
        
    sleep(5)    

    call hToolbarSelect("FormControls",true)
            
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)      
        '/// insert a text box
        printlog "insert a text box"                    
        call hDrawingWithSelection ( 40, 40, 50, 50 )    
        sleep(2)
	Kontext "FormControls"
	    ControlProperties.Click
        sleep (10)
        
    Kontext "ControlPropertiesTabControl"        
        ControlPropertiesTabControl.setPage TabDataControl
    Kontext "TabDataControl"
        XFormDataModel.select 1
        TabDataControl.TypeKeys "<RETURN>", true
        
        XFormBindExpressionName.setText("testtool")
        TabDataControl.TypeKeys "<RETURN>", true        
    
    Kontext "ControlPropertiesDialog"        
        ControlPropertiesDialog.Close
    	sleep(1)

    call hCloseDocument
    
endcase

