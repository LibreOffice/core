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
'* short description : Create Text DS & further standard db tests
'*
'\***********************************************************************
sub main
    printlog "---------------------------------------------------"
    printlog "---       D B A C C E S S    Text Table         ---"
    printlog "---------------------------------------------------"

    use "dbaccess/optional/includes/db_Query.inc"
    use "dbaccess/optional/includes/db_Text.inc"
    
    call hStatusIn ("dbaccess" , "dba_db_Text.bas")
    
    Dim sFileName as string
	sFileName = gOfficePath + "user/work/TT_Text.odb"
    
    Dim sDBURL as string
	sDBURL = "user/work"
   
        'needed for: tQuery testcase - query file is copied in the work directory
        app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/text_datasource/TT_Query1.txt"),gOfficePath + ConvertPath("user/work/TT_Query1.txt")
   
   		dim dbok as boolean
   		dbok = false
        dbok = fCreateTextDatasource(sFileName, gOfficePath + ConvertPath(sDBURL))
        if dbok = true then
        
            
            call db_Query(sFileName, "text")
            
            'use "dbaccess/optional/includes/b_lvl1_Forms.inc"
            'call Forms_Test (sFileName)
            
            call db_Text(sFileName)
                        

		else
			warnlog "Data Source could not be created - beyond testcases stopped"
		endif
       
    call hStatusOut
    
end sub


sub LoadIncludeFiles   
   use "dbaccess/tools/dbinit.inc"   
   Call sDBInit
   Call GetUseFiles
   gApplication   = "WRITER"
end sub
