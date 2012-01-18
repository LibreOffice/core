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
'* short description :
'*
'\***********************************************************************

sub rpt_Charts

    tInsertChart

end sub
'--------------------------------------------------------------------------------------
testcase tInsertChart

    printlog "open Bibliography database"
    call fOpenDataBase(gOfficePath + ConvertPath("user/database/biblio.odb"))

    printlog "open the report designer"
    call fOpenNewReportDesign

    sleep(1)

    printlog "select the first table in the content list box"
    Kontext "ReportDataProperties"
        Content.select 1
        Content.typeKeys("<RETURN>",true) ' important to leave the listbox

    'close the Add Field dialog to get the focus back to the design
    call fCloseAddFieldDialog

    printlog "insert a chart"
  	Kontext "StandardBar"
   	    CreateChart.Click
        sleep(1)

    Kontext "ReportDesign"
        ReportDesign.MouseDown ( 30, 10 )
        ReportDesign.MouseMove ( 40, 20 )
        ReportDesign.MouseUp ( 40, 20 )

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

endcase
