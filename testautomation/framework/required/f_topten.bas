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
'* short description : smoke test for each application
'*
'\******************************************************************************

sub main

    use "framework\required\includes\topten.inc"
    use "framework\required\includes\smoketest.inc"
    use "framework\required\includes\first.inc"

    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )

    call hStatusIn ("framework", "f_topten.bas")
    call tAllNew
    call tInFilterCountCompare
    call tJava
    call tDatabaseCheck
    call topten
    call tSmokeTest
    call hStatusOut

    hSetMacroSecurity( GC_MACRO_SECURITY_LEVEL_DEFAULT )

end sub

'*******************************************************************************

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_ui_filters.inc"
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_listfuncs.inc"
   
    gApplication = "BACKGROUND"
    Call GetUseFiles
    
end sub
