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
'* short description : Function Wizard
'*
'\***********************************************************************************
sub rpt_FunctionWizard

    printlog "------------------ rpt_FunctionWizard.inc ---------------------"
    
    call tFunctionWizard
    
end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tFunctionWizard
    
    printlog "open Bibliography database"    
    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
        
    printlog "open the report designer"
    call fOpenNewReportDesign

    printlog "select the first table in the content list box"
    Kontext "ReportDataProperties"   
        Content.select 1
        Content.typeKeys("<RETURN>",true) ' important to leave the listbox
    
    Kontext "ReportDesign"
        ReportDesign.TypeKeys("<MOD1 TAB>",true)
        ReportDesign.TypeKeys("<MOD1 TAB>",true)
        
    Kontext "ReportAddField"
        ReportAddFieldList.select 1    
        ReportAddField.TypeKeys("<RETURN>",true)        
        
    'close the Add Field dialog to get the focus back to the design
    call fCloseAddFieldDialog()
    
    'press 2 time tab to select the edit field
    Kontext "ReportDesign"
        ReportDesign.TypeKeys("<TAB>",true)
        ReportDesign.TypeKeys("<TAB>",true)
    
    sleep(1)
    
    printlog "click on the ... button behind the datafield property in the property browser"    
    Kontext "ReportDataProperties"
        OpenFormularWizard.Click
    
    printlog "check if the function wizard appear"    
    Kontext "FunctionWizard"
        if(FunctionWizard.exists(5)) then
            printlog "The function wizard appear -> OK"
            
            CategoryLB.select(5)
            sleep(1)            
            FunctionLB.select(6)
            sleep(1)
            NextBt.Click
            sleep(1)
            Editfield1.setText("""12/18/2008""")
            sleep(1)
            
            FunctionWizard.OK
        else
            warnlog "The function wizard does not appear -> FAILED"
        endif
    
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
        
    printlog "close the report designer"
    call fCloseReportDesign
    printlog "close the database"
    call fCloseDatabase

endcase
'-------------------------------------------------------------------------

