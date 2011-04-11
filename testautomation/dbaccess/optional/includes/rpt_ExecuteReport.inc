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
'* short description : Execute Report
'*
'\***********************************************************************************
sub rpt_ExecuteReport

    printlog "------------------ rpt_ExecuteReport.inc ---------------------"
    
    call tExecute
    
end sub
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
'-------------------------------------------------------------------------
testcase tExecute     

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
    '/// align the control top left
    printlog "align the control top left"
    Kontext "ReportDesign"
        ReportAlignLeft
        ReportAlignUp
    sleep(1) ' need a sleep before using the propety browser    
    Kontext "ReportPropertiesTabControl"
        ReportPropertiesTabControl.setPage ReportDataProperties
    
    '/// bind the first field of the tabel to the control    
    printlog "bind the first field of the tabel to the control"
    Kontext "ReportDataProperties" 
        DataField.select 1
        DataField.typeKeys("<RETURN>",true) ' important to leave the listbox
    
    Kontext "ReportPropertiesTabControl"
        ReportPropertiesTabControl.setPage ReportGeneralProperties
    
    sleep(1)    
        
    '/// select the 11. color (light green) for the background color
    printlog "select the 11. color (light green) for the background color"
    Kontext "ReportGeneralProperties"
        ControlBackgroundTransparent.select 1
        sleep(1)
        BackGroundColor.select 11        
        sleep(2)
        Height.setText("5")
        Height.typeKeys("<RETURN>",true)
        
    ' select detail section with unselect the control
    Kontext "ReportDesign"
        ReportDesign.MouseDown(50, 10)
        ReportDesign.MouseUp(50, 10)
        sleep(1)
    Kontext "ReportGeneralProperties"           
        Height.setText("5")
        Height.typeKeys("<RETURN>",true)
        
    sleep(1) 
    
    '/// execute the report
    printlog "execute the report"
    call fExecuteReport
    
    sleep(10)
    
    '/// check if the report is created
    printlog "check if the report is created"
    Kontext "DocumentWriter"
        if (DocumentWriter.exists(10)) then
            hFileSaveAsKill(gOfficePath + ConvertPath("user/work/report01.odt"))
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

    '---- check the saved report with SAX parser
    dim iNumberOfChilds as integer
    dim sText as string

    UnpackStorage( gOfficePath & ConvertPath("user/work/report01.odt") , gOfficePath & ConvertPath("user/work/report01") )

    SAXReadFile(gOfficePath & ConvertPath("user/work/report01/content.xml"))
            
    SAXSeekElement("office:document-content")
    SAXSeekElement("office:body")    
    SAXSeekElement("office:text")   
    SAXSeekElement("table:table")

    iNumberOfChilds = SAXGetChildCount 
    dim iLoop as integer
    dim iCount as integer
    iCount = 0
    for iLoop = 1 to iNumberOfChilds
	    SAXSeekElement(iLoop)
		if ( SAXGetElementName() = "table:table-row" ) then
			iCount = iCount + 1
		endif
		SAXSeekElement(0)
	next
	
    if (iCount <> 59 ) then
        warnlog "there should be 59 rows in the table, but there are " + iNumberOfChilds        
        SAXRelease
        goto endsub
    endif
    SAXSeekElement("table:table-row",10)
        
  '  printlog SAXGetChildCount
    SAXSeekElement("table:table-cell")
    SAXSeekElement("text:p")
    SAXSeekElement(1)
    sText = SAXGetChars
    
    if (sText <> "SUN02") then
        warnlog "The text in the 10. row should be GRA00, but it is " + sText
    endif
    
    'check background color
    SAXSeekElement("/")
    SAXSeekElement("office:document-content")
    SAXSeekElement("office:body")    
    SAXSeekElement("office:text")   
    SAXSeekElement("table:table")
    SAXSeekElement("table:table-row",10)
    SAXSeekElement("table:table-cell")
    SAXSeekElement("text:p")
    
    'text:style-name="P1
    dim sStyle as string
    sStyle = SAXGetAttributeValue("text:style-name")
    'printlog sStyle
    
    dim iChildCount as integer
    dim iChildNumber as integer
    dim sBackgroundColor as string
    dim a as string
    SAXSeekElement("/")
    SAXSeekElement("office:document-content")
    SAXSeekElement("office:automatic-styles")
    a = SAXGetChildCount
    for iChildNumber = 1 to iChildCount
        SAXSeekElement("/")
        SAXSeekElement("office:document-content")
        SAXSeekElement("office:automatic-styles")
        SAXSeekElement(iChildNumber)
        'printlog SAXGetAttributeValue("style:name") 
        if SAXGetAttributeValue("style:name") = sStyle then
            SAXSeekElement("style:paragraph-properties")
            sBackgroundColor = SAXGetAttributeValue("fo:background-color")
            if sBackgroundColor <> "#00ff00" then
                warnlog "wrong color. It should be #00ff00 but it is " + sBackgroundColor
            else
                'printlog "right color"
            endif
        endif
    next    
    SAXRelease      
    
    '--- end of SAX check
    
endcase
'-------------------------------------------------------------------------

