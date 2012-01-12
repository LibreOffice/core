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
'* short description : Test for the property browser
'*
'\***********************************************************************
sub main
    
    printlog "---------------------------------------------------------------"
    printlog "-----         D B A C C E S S  -  Property Browser        -----"
    printlog "---------------------------------------------------------------"

    use "dbaccess/optional/includes/rpt_PropertyBrowser.inc"

    call hStatusIn ("dbaccess" , "dba_rpt_PropertyBrowser.bas")

    if ( tools_reporttools_InstallExtension() = 0 ) then        
        call rpt_PropertyBrowser
    else
        warnlog "report extension could not installed"    
    endif
    
    call hStatusOut
    
end sub


sub LoadIncludeFiles      
   use "dbaccess/tools/dbinit.inc"   
   Call sDBInit
   Call GetUseFiles
   gApplication   = "WRITER"
end sub
