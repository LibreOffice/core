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
'* short description : Forms Test
'*
'\***********************************************************************
sub main
    printlog "-----------------------------------------------------------------------"
    printlog "--------- B A S E   L E V E L - 1   F O R M S                 ---------"
    printlog "-----------------------------------------------------------------------"
    
    use "dbaccess/optional/includes/frm_Forms.inc"
    
    call hStatusIn ("dbaccess" , "dba_frm_Forms.bas")      
    
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/dbase_datasource/TT_Forms.dbf"),gOfficePath + ConvertPath("user/work/TT_Forms.dbf")
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/dbase_datasource/TT_Query1.dbf"),gOfficePath + ConvertPath("user/work/TT_Query1.dbf")
    app.FileCopy gTesttoolPath + ConvertPath("dbaccess/optional/input/dbase_datasource/TT_Query1.dbt"),gOfficePath + ConvertPath("user/work/TT_Query1.dbt")
    
    call fCreateDbaseDatasource(gOfficePath + ConvertPath("user/work/tt_dbase-01.odb"),gOfficePath + ConvertPath("user/work"),"TT_Forms")
    call frm_Forms("TT_Forms")

    call hStatusOut

end sub

sub LoadIncludeFiles      
   use "dbaccess/tools/dbinit.inc"   
   Call sDBInit
   Call GetUseFiles
   gApplication   = "WRITER"
end sub
