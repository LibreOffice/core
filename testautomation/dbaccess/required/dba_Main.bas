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
'* short description : DBAccess Main Test
'*
'\***********************************************************************
sub main
    
    printlog "---------------------------------------------------------------------"
    printlog "-----            D B A C C E S S  -  M A I N T E S T            -----"
    printlog "---------------------------------------------------------------------"
   
    use "dbaccess/required/includes/Query.inc"
    use "dbaccess/required/includes/MainApp.inc"
    use "dbaccess/required/includes/TableDesign.inc"
    use "dbaccess/required/includes/Table.inc"
    use "dbaccess/required/includes/Forms.inc"
    use "dbaccess/required/includes/DatabaseTypes.inc"
    use "dbaccess/required/includes/Wizards.inc"
    
    Dim startes
    startes = Now
    
    call hStatusIn ("dbaccess","dba_Main.bas" , "DBAccess Main Test")
   
    call MainApp
    call Query
    call TableDesign
    call Table
    call Forms
    
    call ExitRestartTheOffice
    call DatabaseTypes
    call ExitRestartTheOffice    
    call Wizards
    
    call hStatusOut
    
    printlog "---------------------------------------------------------------------"
    printlog "End: " & Date & ", " & Time & ". (" & wielange(Startes) & ")"
    printlog "---------------------------------------------------------------------"
end sub


sub LoadIncludeFiles
    use "global/tools/includes/optional/t_extension_manager_tools.inc"
    use "dbaccess/tools/dbinit.inc"
    Call sDBInit
    Call GetUseFiles
    gApplication   = "WRITER"
end sub
