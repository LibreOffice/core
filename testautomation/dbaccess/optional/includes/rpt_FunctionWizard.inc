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

