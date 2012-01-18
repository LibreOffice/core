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
'* short description : Date Time Dialog
'*
'\***********************************************************************************
sub rpt_DateTime

    printlog "------------------ rpt_DateTime.inc ---------------------"
    
    call tDefaultSetting
    call tDateTime
    
end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tDefaultSetting

    '/// FILE / OPEN / biblio.odb
    printlog "FILE / OPEN / biblio.odb"        
    call fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// INSERT / REPORT
    printlog "INSERT / REPORT"    
    call fOpenNewReportDesign
    
    sleep(1)
    
    '/// select any section to get the insert page number menu item activated
    printlog "select any section to get the insert page number menu item activated"
    Kontext "ReportDesign"
        ReportDesign.MouseDown(50, 10)
        ReportDesign.MouseUp(50, 10)
    
    '/// INSERT / DATE TIME 
    Kontext "ReportDesign"
    ReportDesign.UseMenu 
        hMenuSelectNr(4)
        hMenuSelectNr(2)        
    
	sleep(1)
    '/// check if the "Date Time" dialog appear
    printlog "check if the ""Date Time"" dialog appear"    
    Kontext "ReportDateTime"
        if (ReportDateTime.exists(3)) then
        
            '/// check if ""IncludeDate"" is checked
            printlog "check if ""IncludeDate"" is checked"            
            if ( IncludeDate.isChecked() ) then
                printlog "Default ""IncludeDate"" is checked"
            else
                warnlog "Default ""IncludeDate"" is not checked"
            endif    
            
            '/// check if the date format list box is enabled
            printlog "check if the date format list box is enabled"            
            if DateFormat.isEnabled() then
                printlog "Date fomat List box is enabled"
            else
                printlog "Date fomat List box is disabled"
            endif
            
            '/// check if ""IncludeTime"" is checked
            printlog "check if ""IncludeTime"" is checked"                
            if ( IncludeTime.isChecked() ) then
                printlog "Default ""IncludeTime"" is checked"
            else
                warnlog """IncludeTime"" is not checked"
            endif
            
            '/// check if the time format list box is enabled
            printlog "check if the time format list box is enabled"            
            if TimeFormat.isEnabled() then
                printlog "Time fomat list box is enabled"
            else
                printlog "Time fomat Llst box is disabled"
            endif
    
	        '/// close dialog with OK
            printlog "close dialog with OK"
	        ReportDateTime.OK
		else
            warnlog "ReportDateTime doesn't exists"	        
	    endif    
    
    '/// close the report designer with WINDOW / CLOSE
    printlog "close the report designer with WINDOW / CLOSE"    
    call fCloseReportDesign
    
    '/// close the database with FILE / CLOSE
    printlog "close the database with FILE / CLOSE"    
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tDateTime

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
    
    '/// turn off the page header    
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
        ReportAlignLeft
        ReportAlignUp        
        
    sleep(1)    
        
    '/// select the first field in the property browser for this control
    printlog "select the first field in the property browser for this control"
    Kontext "ReportPropertiesTabControl"    
        ReportPropertiesTabControl.setPage ReportDataProperties
    
    Kontext "ReportDataProperties" 
        DataField.select 1
        DataField.typeKeys("<RETURN>",true) ' important to leave the listbox
    
    ' select detail section with unselect the control
    Kontext "ReportDesign"
        ReportDesign.MouseDown(50, 10)
        ReportDesign.MouseUp(50, 10)
    
    '/// set the Detail sectio to the minial height
    printlog "set the Detail sectio to the minial height"    
    Kontext "ReportGeneralProperties"           
        Height.setText("0")
        Height.typeKeys("<RETURN>",true)
        
    '/// tunr on pageHeader again    
    call fSwitchPageHeader
        
    '/// select the page header
    Kontext "ReportDesign"
        ReportDesign.MouseDown(50, 10)
        ReportDesign.MouseUp(50, 10)
    
    '/// INSERT / DATE TIME    
    Kontext "ReportDesign"
    ReportDesign.UseMenu 
        hMenuSelectNr(4)
        hMenuSelectNr(2)        
    
	sleep(1)
    
    '/// check the 3. date and time option
    printlog "check the 3. date and time option"
    Kontext "ReportDateTime"    
        DateFormat.select 3
        TimeFormat.select 3
                
        '/// close dialog with OK
        printlog "close dialog with OK"
        ReportDateTime.OK
	
    sleep(2)
    
    '/// select the date field and check if the date field contain the function TODAY()
    printlog "select the date field and check if the date field contain the function TODAY()"
    Kontext "ReportDesign"
        ReportDesign.typeKeys("<TAB>",true)        
        sleep(1)
    Kontext "ReportPropertiesTabControl"
        ReportPropertiesTabControl.setPage ReportDataProperties 
        sleep(1)
    Kontext "ReportDataProperties"
        dim s as string
        s = DataField.getSelText()        
        if (instr(s,"TODAY()") = 0) then
            warnlog "The date function is not correct.The function is " + s
        else
            printlog "The date function is correct."
        endif
    
    '/// select the time field and check if the data field contains the function now()
    printlog "select the time field and check if the data field contains the function now()"    
    Kontext "ReportDesign"
        ReportDesign.typeKeys("<MOD1 F6>",true) ' go to the document window
        sleep(1)
        ReportDesign.typeKeys("<MOD1 F6>",true) ' really go to the document window
        sleep(1)
        ReportDesign.typeKeys("<TAB>",true)
        sleep(1)
    
    Kontext "ReportPropertiesTabControl"
        ReportPropertiesTabControl.setPage ReportDataProperties 
        sleep(1)
    Kontext "ReportDataProperties"        
        s = DataField.getSelText()        
        if (instr(s,"NOW()") = 0) then
            warnlog "The time function is not correct.The function is " + s
        else
            printlog "The time function is correct."
        endif
        
    '/// execute the report            
    call fExecuteReport
    
    sleep(10)
                     
    '/// check if the report is created
    printlog "check if the report is created"
    Kontext "DocumentWriter"
        if (DocumentWriter.exists(10)) then
            hFileSaveAsKill(gOfficePath + ConvertPath("user/work/report_date_time.odt"))
            call fCloseReportView
        else
            warnlog "No report is created."
        endif
        
    '/// close the report designer with WINDOW / CLOSE
    printlog "close the report designer with WINDOW / CLOSE"    
    call fCloseReportDesign
    
    '/// close the database with FILE / CLOSE
    printlog "close the database with FILE / CLOSE"    
    call fCloseDatabase
    
    
    
    '---- check the saved report with SAX parser
    dim iNumberOfChilds as integer
    dim sText as string

    UnpackStorage( gOfficePath & ConvertPath("user/work/report_date_time.odt") , gOfficePath & ConvertPath("user/work/report_date_time") )

    SAXReadFile(gOfficePath & ConvertPath("user/work/report_date_time/content.xml"))
            
    SAXSeekElement("office:document-content")
    SAXSeekElement("office:body")    
    SAXSeekElement("office:text")   
    SAXSeekElement("table:table")

    iNumberOfChilds = SAXGetChildCount 
    'print iNumberOfChilds
    
    
    SAXRelease      
    
    '--- end of SAX check
    
endcase
'-------------------------------------------------------------------------

