'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
'*
'* short description : Grouping Report
'*
'\***********************************************************************************
sub rpt_Grouping

    printlog "------------------ rpt_Grouping.inc ---------------------"
    
    call tGrouping
    
end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tGrouping

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign
    
    sleep(1)
    
    '/// select the first table in the content list box
    printlog "select the first table in the content list box"
    Kontext "ReportDataProperties"   
        Content.select 1
        Content.typeKeys("<RETURN>",true) ' important to leave the listbox
    
    'close the Add Field dialog to get the focus back to the design
    call fCloseAddFieldDialog
    
    '/// turn of the page header    
    call fSwitchPageHeader
    
    '/// insert a data control
    printlog "insert a data control"    
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    
    Kontext "ReportDesign"    
        ReportDesign.MouseDown ( 30, 10 )            
        ReportDesign.MouseMove ( 40, 20 )            
        ReportDesign.MouseUp ( 40, 20 )

    Kontext "ReportDesign"

        '/// align the control to the left    
        ReportDesign.UseMenu 
        hMenuSelectNr(5)
        hMenuSelectNr(5)
        hMenuSelectNr(1)
        
        '/// align the control to the top
        ReportDesign.UseMenu 
        hMenuSelectNr(5)
        hMenuSelectNr(5)
        hMenuSelectNr(4)
        
    sleep(1)    
        
    Kontext "ReportPropertiesTabControl"
        ReportPropertiesTabControl.setPage ReportDataProperties
    
    Kontext "ReportDataProperties" 
        DataField.select 1
        DataField.typeKeys("<RETURN>",true) ' important to leave the listbox
    
    ' select detail section with unselect the control
    Kontext "ReportDesign"
        ReportDesign.MouseDown(50, 10)
        ReportDesign.MouseUp(50, 10)
        
    Kontext "ReportGeneralProperties"           
        Height.setText("0")
        Height.typeKeys("<RETURN>",true)
        

    
    '/// open the sorting and grouping
    Kontext "ReportDesign"
    ReportDesign.UseMenu 
        hMenuSelectNr(3)
        hMenuSelectNr(4)        
    
	sleep(1)
    
    Kontext "ReportSortingGrouping"
        if (ReportSortingGrouping.exists(3)) then
	        Groups.typeKeys("Type",true)
    	    Groups.typeKeys("<return>",true)
	        sleep(1)
	        ReportSortingGrouping.Close
		else
            warnlog "ReportSortingGrouping doesn't exists"	        
	    endif    
    
    sleep(1)
    
    '/// execute the report
    call fExecuteReport
    
    sleep(10)
    
    '/// check if the report is created
    printlog "check if the report is created"
    Kontext "DocumentWriter"
        if (DocumentWriter.exists(10)) then
            call fCloseReportView
        else
            warnlog "No report is created."
        endif
    
    '/// close the report designer
    printlog "close the report designer"
    call fCloseReportDesign
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------

