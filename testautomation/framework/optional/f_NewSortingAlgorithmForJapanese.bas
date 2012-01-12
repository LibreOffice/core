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
'* short description: New Sorting Algorithm For Japanese
'*
'\*********************************************************************

global sAsianLanguageFlag  as String

sub main
    printlog "---------------------------------------------------------------------"
    printlog "-----            New Sorting Algorithm For Japanese test        -----"
    printlog "---------------------------------------------------------------------"

    use "framework\optional\includes\NewSortingAlgorithmForJapanese_1.inc"
    

    sAsianLanguageFlag = CheckAsianLanguageSupport("Current")    
    Call CheckAsianLanguageSupport("On")

    Call hStatusIn("framework", "f_NewSortingAlgorithmForJapanese.bas")

    printLog Chr(13) + "-New Sorting Algorithm For Japanese test -- Writer"
    Call NewSortingForJapanese_1      'Check if editbox "Entry Phonetic Reading " appear or not 
    Call NewSortingForJapanese_2      'Check if editbox "Entry Phonetic Reading " is enabled or not
    Call NewSortingForJapanese_3      'Check the number of key type in sort area when choose japanese language 
    Call NewSortingForJapanese_4      'Check the number of key type in data/sort

    printLog Chr(13) + "-New Sorting Algorithm For Japanese test -- Calc"
    gApplication   = "CALC"       
    Call NewSortingForJapanese_5      'Check the number of key type in data/sort -- Calc

    Call hStatusOut
    
    if sAsianLanguageFlag = "Off" then  
        Call CheckAsianLanguageSupport("Off")
    else 
        Call CheckAsianLanguageSupport("On")
    end if      
end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   
   use "framework\tools\includes\CJK_tools.inc"
   
   gApplication   = "WRITER"
   Call GetUseFiles
end sub

