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
'* short description : Create Spreadsheet DS & standard ds tests
'*
'\***********************************************************************
testcase db_Spreadsheet
    
    Dim sFileName as string
	sFileName = gOfficePath + ConvertPath("user/work/TT_Spreadsheet.odb")
    
    Dim sDBURL as string
	sDBURL = "user/work/TT_Query1.ods"
   
    'needed for: tQuery testcase - query file is copied in the work directory
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/spreadsheet_datasource/TT_Query1.ods"),gOfficePath + ConvertPath("user/work/TT_Query1.ods")

    dim dbok as boolean
    dbok = false
    dbok = fCreateSpreadsheetDatasource(sFileName, gOfficePath + ConvertPath(sDBURL),"tt_spreadsheet")
    if dbok = true then
            
        call db_Query(sFileName, "CALC")
        
        'outcomment because of several bugs inside forms
        'use "dbaccess/optional/includes/b_lvl1_Forms.inc"
        'call b_lvl1_Forms("tt_spreadsheet")
        
    else
        warnlog "Data Source could not be created - beyond testcases stopped"
    endif
    
    
endcase

