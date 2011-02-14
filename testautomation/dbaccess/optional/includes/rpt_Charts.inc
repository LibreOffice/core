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
'* owner :
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