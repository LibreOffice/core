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
'* short description : Search & Replace and testing of regular expressions
'*
'\***********************************************************************

global Such_Text as string
global Vergleichstext as string
global Meldungs_Text as string
global dummy_Text as string

sub main
    use "writer\optional\includes\regexp\regular.inc"
    use "writer\optional\includes\regexp\search.inc"

    Dim Start : Start = Now()

    Call hStatusIn("writer","w_search.bas","Search & Replace")
    printlog "** Search and Replace **"
    Call search
    printlog "** Regular expressions **"
    Call regular
    Call hStatusOut

    printlog "Duration : " +  Wielange ( Start )
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    gApplication = "WRITER"
    Call GetUseFiles
end sub
