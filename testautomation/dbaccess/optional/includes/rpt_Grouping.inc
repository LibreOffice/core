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

