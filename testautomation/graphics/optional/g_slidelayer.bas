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
'* short description : Graphics Function: Slide and Layer
'*
'\******************************************************************

'Variables:
global ExtensionString as String

sub main
    PrintLog "------------------------- g_slidelayer test -------------------------"
    Call hStatusIn ( "Graphics","g_slidelayer.bas")

    use "graphics\tools\id_tools.inc"
    use "graphics\tools\id_tools_2.inc"
    use "graphics\optional\includes\global\g_slidelayer.inc"
    use "graphics\optional\includes\impress\i_slidelayer.inc"

    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tInsertSlide
    Call tInsertDuplicateSlide
    Call tiInsertExpandSlide  ' only in impress
    Call tiInsertSummarySlide  ' only in impress
    Call tiFormatSeitenlayout  ' only in impress
    Call t114174  ' only in impress
    Call t111862

    gApplication = "DRAW"
    PrintLog "-------------------------" + gApplication + "-------------------"
    Call tInsertSlide
    Call tInsertDuplicateSlide
    Call tInsertLayer ' in impress not here, instead Edit->Layer->Insert

    Call hStatusOut
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "IMPRESS"
    Call GetUseFiles
end sub
