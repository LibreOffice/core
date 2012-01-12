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
'* short description : test availablity of gridcontrol
'*
'\******************************************************************************

sub main

    use "framework\optional\includes\basic_gridcontrol.inc"

    call hStatusIn ("framework", "f_basic_gridcontrol.bas")

    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_LOW )
    call tGridcontrolLoad
    call tTabcontrolLoad
    hSetMacroSecurityAPI( GC_MACRO_SECURITY_LEVEL_DEFAULT )

    call hStatusOut()

end sub

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"

    use "global\tools\includes\optional\t_treelist_tools.inc"
    use "global\tools\includes\optional\t_macro_tools.inc"
    
    gApplication = "WRITER"
    call GetUseFiles()
end sub

