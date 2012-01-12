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
'* short description : Optional test for dBase functions
'*
'\***********************************************************************
sub main
    printlog "-----------------------------------------------------------------"
    printlog "---            D B A C C E S S      dBase functions           ---"
    printlog "-----------------------------------------------------------------"

    use "dbaccess/optional/includes/db_DbaseFunction.inc"
    
	dim dbok as boolean
    
    call hStatusIn ("dbaccess" , "dba_db_dBase_functions.bas")
    
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/dbase_datasource/TT_Func1.dbf"),gOfficePath + ConvertPath("user/work/TT_Func1.dbf")       
    
    Dim sFileName as string 
    sFileName = gOfficePath + ConvertPath("user/work/tt_dbase_level2.odb")
        
    dbok = fCreateDbaseDatasource(sFileName,gOfficePath + ConvertPath("user/work"))
    if dbok = true then        
        call db_DbaseFunction(sFileName)        
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
