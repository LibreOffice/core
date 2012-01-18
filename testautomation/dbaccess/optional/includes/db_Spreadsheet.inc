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

