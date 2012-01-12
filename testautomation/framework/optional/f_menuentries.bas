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
'* short description : Read menubar and first level drop down lists and compare to reference
'*
'\*****************************************************************

sub main
' Raffaella Braconi and her l10n team defined these reference files;
' If there is a difference, it is a bug, that has to get defined by her.
    use "framework\optional\includes\menuentries.inc"
qaerrorlog("Test is disabled, until I get the reference files...")
'    Call hStatusIn ( "Framework","f_lvl1_menuentries.bas" )
'    PrintLog Chr(13) + " menubar Test "
'    t_getMenuText
'    Call hStatusOut
end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   gApplication = "writer"
   Call GetUseFiles
end sub

