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
'* short description : Clipboard export Test
'*
'\******************************************************************

sub main
    Call hStatusIn ( "Graphics","g_clipexport.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\g_clipexport.inc"
    use "graphics\optional\includes\global\g_clipexport2.inc"
    use "graphics\optional\includes\global\g_clipexport3.inc"

    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tClipboardFromDrawTest

    gApplication = "IMPRESS"
    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tClipboardFromDrawTest

    'TODO FHA - Find and write bugs for exporting to writer and calc.
    '    gApplication = "WRITER"
    '    PrintLog "-------------------------" + gApplication + "-------------------"
    '       Call tClipboardFromDrawTest

    '    gApplication = "CALC"
    '    PrintLog "-------------------------" + gApplication + "-------------------"
    '       Call tClipboardFromDrawTest

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\required\t_menu.inc"   'Window-control
    gApplication = "DRAW"
    Call GetUseFiles
end sub
