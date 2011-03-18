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
