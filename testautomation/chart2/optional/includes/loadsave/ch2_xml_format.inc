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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Verify the Chart2 XML file format
'*
'************************************************************************
'*
' #1 tCompareXMLFormatCreate   ' Verify Chart2 XML file format by using three characteristcs on new chart
' #1 tCompareXMLFormatLoad     ' Verify Chart2 XML file format by using three characteristcs on loaded chart
'*
'\***********************************************************************

testcase tCompareXMLFormatCreate
    '///+ Test purpose: Verify within three characteristics that there is really a chart

    dim sOutputFile as string
    dim sUnpackedStorageDir as string
    dim sChartContentXML as string

    gApplication   = "WRITER"
    sOutputFile = convertpath(gOfficepath & "user\work\Chart2_XML.odt")
    sUnpackedStorageDir = (gOfficePath & ConvertPath("user\work\chart2_XML2"))
    sChartContentXML = (gOfficePath & ConvertPath("user\work\chart2_XML2") & ConvertPath("\Object 1\content.xml"))

    '/// File / New / Text
    printlog " Open new Writer document"
    call hNewDocument
    '/// Insert chart with Insert / Object / Chart
    printlog " Insert chart with Insert / Object / Chart"
    InsertChart
    '/// Save document to <i>gOfficepath</i>/user/work/Chart2_XML.odt
    printlog " Save document to <i>gOfficepath</i>/user/work/Chart2_XML.odt"
    if NOT hFileSaveAsKill (sOutputFile) then
        warnlog "Saving test document failed -> Aborting"
        call hCloseDocument
        goto endsub
    else
        printlog "File saved successfully."
    endif
    '/// File / Close
    call hCloseDocument
    sleep(3)
    printlog "File closed"
    '/// Uncompress with <i>unzip</i> or <i>jar</i> the
    '///+ <i>gOfficepath</i>/user/work/verySimpleChart.ods document
    '///+ into <i>gOfficePath</i>/user/work/chart2_XML-directory
    printlog "Unpack storage of test document."
    UnpackStorage(sOutputFile, sUnpackedStorageDir)
    printlog "Read the test documents DOM."
    '/// Load the XML file <i>gOfficePath</i>/user/work/chart2_XML/Object 1/content.xml
    '///+ in an XML editor
    SAXReadFile(sChartContentXML)
    printlog "Set the DOM pointer to document root"
    printlog "|-/"
    SAXSeekElement("/")
    printlog "  |-office:document-content"
    '/// Open the element 'office:document-content'
    SAXSeekElement("office:document-content")
    printlog "    |-office:body"
    '/// Open the element 'office:body'
    SAXSeekElement("office:body")
    printlog "      |-office:chart"
    '/// Open the element 'office:chart'
    SAXSeekElement("office:chart")
    printlog "         |-chart:chart"
    '/// Open the element 'chart:chart'
    SAXSeekElement("chart:chart")
    '/// Get the attribute value for <i>chart:class</i>.
    '///+ If it is 'chart:bar' the first characteristics validation has been
    '///+ passed.
    if SAXGetAttributeValue("chart:class") = "chart:bar" then
        printlog "This is a bar chart as expected"
    else
        warnlog "This is no bar chart, it is a " & SaxGetAttributeValue("chart:class")
    endif
    printlog "Seek for chart:plot-area"
    printlog "            |-chart:plot-area"
    '/// Open the element 'chart:plot-area'
    SAXSeekElement("chart:plot-area")
    '/// If the attribute value of 'chart:data-source-has-labels' is 'both'
    '///+ the second characteristics validation has been passed.
    if SAXGetAttributeValue("chart:data-source-has-labels") = "both" then
        printlog "The label attribute for chart:plot-area is correct"
    else
        warnlog "The lable attribute is not both, it is " & SaxGetAttributeValue("chart:data-source-has-labels")
    endif
    printlog "Seek for chart:series"
    printlog "               |-chart:series"
    '/// Open the element 'chart:series'
    SAXSeekElement("chart:series")
    '/// If the attribute valie of 'chart:values-cell-range-address' is local-table.$B$2:.$B$5''
    '///+ the hird characteristics validation has been passed.
    if SAXGetAttributeValue("chart:values-cell-range-address") = "local-table.$B$2:.$B$5" then
        printlog "The cell range for chart:series is correct"
    else
        warnlog "The cell range is not local-table.$B$2:.$B$5, it is " & SaxGetAttributeValue("chart:values-cell-range-address")
    endif
    ' DEBUG: printlog "I'm here: " & SAXGetElementName
    '/// Close the XML editor.
    SAXRelease
endcase
'
'--------------------------------------------------------------------
'
testcase tCompareXMLFormatLoad
    '///+ Test purpose: Verify within three characteristics that there is really a chart

    dim sOutputFile as string
    dim sUnpackedStorageDir as string
    dim sChartContentXML as string

    gApplication   = "CALC"
    sOutputFile = convertpath(gOfficepath & "user\work\verySimpleChart.ods")
    sUnpackedStorageDir = (gOfficePath & ConvertPath("user\work\chart2_XML"))
    sChartContentXML = (gOfficePath & ConvertPath("user\work\chart2_XML") & ConvertPath("\Object 1\content.xml"))

    '/// File / Open / <i>gTestToolPath</i>/chart2/update/input/verySimpleChart.ods
    '/// File / Save As / <i>gOfficepath</i>/user/work/verySimpleChart.ods
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document failed!"
        goto endsub
    else
        printlog "Test document has successfully been loaded."
    endif

    '/// Select the chart object using the navigator
    printlog "Select chart using navigator"
    if fSelectFirstOLE = 0 then
        printlog "Chart is selected"
    else
        warnlog "Chart could not be selected"
        call hCloseDocument
        goto endsub
    endif

    '/// Activate chart with Edit / Object / Edit
    printlog "Activate chart by Edit / Object / Edit"
    EditObjectEdit

    'Save document localy
    Kontext "DocumentCalc"
    '/// Press twice <ESCAPE> to leave the chart object
    DocumentCalc.TypeKeys "<ESCAPE>" , 2   'Leave chart and unselect it
    '/// Press <CTRL+HOME> to get to cell A1
    DocumentCalc.TypeKeys "<MOD1 HOME>"
    '/// Press <F9> to re-calculate the spreadsheet and the objects
    DocumentCalc.TypeKeys "<F9>"
    '/// Save the document again
    if NOT hFileSave (sOutputFile) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    else
        printlog "File with very simple chart saved successfully."
    endif
    '/// File / Close
    call hCloseDocument
    sleep(3)
    printlog "File with very simple chart closed"
    '/// Uncompress with <i>unzip</i> or <i>jar</i> the
    '///+ <i>gOfficepath</i>/user/work/verySimpleChart.ods document
    '///+ into <i>gOfficePath</i>/user/work/chart2_XML-directory
    printlog "Unpack storage of test document."
    UnpackStorage(sOutputFile, sUnpackedStorageDir)
    printlog "Read the test documents DOM."
    '/// Load the XML file <i>gOfficePath</i>/user/work/chart2_XML/Object 1/content.xml
    '///+ in an XML editor
    SAXReadFile(sChartContentXML)
    printlog "Set the DOM pointer to document root"
    printlog "|-/"
    SAXSeekElement("/")
    printlog "  |-office:document-content"
    '/// Open the element 'office:document-content'
    SAXSeekElement("office:document-content")
    printlog "    |-office:body"
    '/// Open the element 'office:body'
    SAXSeekElement("office:body")
    printlog "      |-office:chart"
    '/// Open the element 'office:chart'
    SAXSeekElement("office:chart")
    printlog "         |-chart:chart"
    '/// Open the element 'chart:chart'
    SAXSeekElement("chart:chart")
    '/// Get the attribute value for <i>chart:class</i>.
    '///+ If it is 'chart:bar' the first characteristics validation has been
    '///+ passed.
    if SAXGetAttributeValue("chart:class") = "chart:bar" then
        printlog "This is a bar chart as expected"
    else
        warnlog "This is no bar chart, it is a " & SaxGetAttributeValue("chart:class")
    endif
    printlog "Seek for chart:plot-area"
    printlog "            |-chart:plot-area"
    '/// Open the element 'chart:plot-area'
    SAXSeekElement("chart:plot-area")
    '/// If the attribute value of 'table:cell-range-address' is 'Sheet1.A1:Sheet1.B2'
    '///+ the second characteristics validation has been passed.
    if SAXGetAttributeValue("table:cell-range-address") = "Sheet1.A1:Sheet1.B2" then
        printlog "The cell range for chart:plot-area is correct"
    else
        warnlog "The cell range is not Sheet1.A1:Sheet1.B2, it is " & SaxGetAttributeValue("table:cell-range-address")
    endif
    printlog "Seek for chart:series"
    printlog "               |-chart:series"
    '/// Open the element 'chart:series'
    SAXSeekElement("chart:series")
    '/// If the attribute valie of 'chart:values-cell-range-address' is Sheet1.A1:Sheet1.A2''
    '///+ the hird characteristics validation has been passed.
    if SAXGetAttributeValue("chart:values-cell-range-address") = "Sheet1.A1:Sheet1.A2" then
        printlog "The cell range for chart:series is correct"
    else
        warnlog "The cell range is not Sheet1.A1:Sheet1.A2, it is " & SaxGetAttributeValue("chart:values-cell-range-address")
    endif
    ' DEBUG: printlog "I'm here: " & SAXGetElementName
    '/// Close the XML editor.
    SAXRelease
endcase
