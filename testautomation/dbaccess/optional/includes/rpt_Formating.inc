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
'* short description : Formating
'*
'\***********************************************************************************
sub rpt_Formating

    printlog "------------------ rpt_Formating.inc ---------------------"
    
    call tConditionalFormating
    call tGrowAndShrink    
    
end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tConditionalFormating

    dim iCount as Integer

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign
    
    sleep(1)
    
    '/// insert a data control
    printlog "insert a data control"    
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    
    Kontext "ReportDesign"    
        ReportDesign.MouseDown ( 30, 40 )
        ReportDesign.MouseMove ( 40, 50 )
        ReportDesign.MouseUp ( 40, 50 )
    sleep(1)  
    
    '/// open the conditional formating dialog via the menu
    printlog "open the conditional formating dialog via the menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(5)
        hMenuSelectNr(1)        
    
    '/// check if there is one condition
    printlog "check if there is one condition"
    
    '/// add 2 more conditions and check if the dialog grows
    printlog "add 2 more conditions and check if the dialog grows"
    
    '/// add one more condition and check if the scrollbar gets enabled
    printlog "add one more condition and check if the scrollbar gets enabled"
    
    '/// check if the right condition is displayed
    printlog "check if the right condition is displayed"
    
    '/// remove the second condition and check if the scrollbar gets disabled
    printlog "remove the second condition and check if the scrollbar gets disabled"
    
    '/// remove 2 conditions and check if the dialog skrink
    printlog "remove 2 conditions and check if the dialog skrink"
    
    '/// close the Conditional  Formating Dialog
    printlog "close the Conditional  Formating Dialog"
    Kontext "ConditionalFormating"    
        ConditionalFormating.OK    
    
    '/// close the report designer
    printlog "close the report designer"
    call fCloseReportDesign
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
testcase tGrowAndShrink

    dim iCount as Integer
    dim iSizeY as Integer

    '/// open Bibliography database
    printlog "open Bibliography database"    
    call fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb"))
    
    '/// open the report designer
    printlog "open the report designer"
    call fOpenNewReportDesign
    
    sleep(1)
        
    '/// insert a data control
    printlog "insert a data control"
  	Kontext "FormControls"   	
   	    Edit.Click
        sleep(1)
    
    Kontext "ReportDesign"    
        ReportDesign.MouseDown ( 30, 40 )
        ReportDesign.MouseMove ( 40, 50 )
        ReportDesign.MouseUp ( 40, 50 )
    sleep(1)                        
    
    '/// open the conditional formating dialog via the menu
    printlog "open the conditional formating dialog via the menu"
    Kontext "ReportDesign"
        ReportDesign.UseMenu
        hMenuSelectNr(5)
        hMenuSelectNr(1)        

    sleep(1)    
        
    '/// add 1 more conditions and check if the dialog grows
    printlog "add 1 more conditions and check if the dialog grows"     
        
    Kontext "ConditionalFormating"
    ' save the height
    iSizeY = ConditionalFormating.getSizeY()
    ' add one more condition
    ConditionalFormating.TypeKeys("<TAB>",true)
    ConditionalFormating.TypeKeys("<TAB>",true)
    ConditionalFormating.TypeKeys("<TAB>",true)
    ConditionalFormating.TypeKeys("<RETURN>",true)
    'compare the both height        
    if (iSizeY >= ConditionalFormating.getSizeY()) then
        warnlog "the dialog doesn't grow"
    endif    
    
    '/// remove 1 conditions and check if the dialog skrink
    printlog "remove 1 conditions and check if the dialog skrink"
    Kontext "ConditionalFormating"
    ' save the height
    iSizeY = ConditionalFormating.getSizeY()
    ' add one more condition
    ConditionalFormating.TypeKeys("<TAB>",true)
    ConditionalFormating.TypeKeys("<TAB>",true)
    ConditionalFormating.TypeKeys("<TAB>",true)
    ConditionalFormating.TypeKeys("<TAB>",true)
    ConditionalFormating.TypeKeys("<TAB>",true)
    ConditionalFormating.TypeKeys("<RETURN>",true)
    'compare the both height        
    if (iSizeY <= ConditionalFormating.getSizeY()) then
        warnlog "the dialog doesn't shrink"
    endif    
        
    ConditionalFormating.Close()    
    
    '/// add one more condition and check if the scrollbar gets enabled
    printlog "add one more condition and check if the scrollbar gets enabled"
    
    '/// check if the right condition is displayed
    printlog "check if the right condition is displayed"
    
    '/// remove the second condition and check if the scrollbar gets disabled
    printlog "remove the second condition and check if the scrollbar gets disabled"
    
    
        
    '/// close the report designer
    printlog "close the report designer"
    call fCloseReportDesign
    
    '/// close the database
    printlog "close the database"
    call fCloseDatabase
    
endcase
'-------------------------------------------------------------------------
