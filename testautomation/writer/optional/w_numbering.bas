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
'* short description : Writer numbering test
'*
'\***********************************************************************

global TBOstringLocale(30) as string
global WriterFilterExtension(10) as string

sub main
    Dim StartZeit
    StartZeit = Now()

    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool5.inc"
    use "writer\tools\includes\w_tool6.inc"
	use "writer\tools\includes\w_tool7.inc"
    use "writer\optional\includes\number\w_205_.inc"
    use "writer\optional\includes\number\w_205a_.inc"
	use "writer\optional\includes\number\w_206_.inc"
	use "writer\optional\includes\number\w_207_.inc"

    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )

    WriterFilterExtension() = hGetFilternameExtension ( "writer8" )

    Call hStatusIn("writer","w_numbering.bas","Writer Numbering test")
    Printlog ""
    Printlog "----------------------------------------------------------"
    Printlog "|                  Writer Numbering Test                 |"
    Printlog "----------------------------------------------------------"
    Printlog ""
    Call w_205_
    Call w_205a_
	Call w_206_
	Call w_207_
    Call hStatusOut

    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_ui_filters.inc"
    use "global/tools/includes/optional/t_listfuncs.inc"
    use "global/tools/includes/optional/t_security_tools.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end sub
