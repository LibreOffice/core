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
'* short description : Test Chinese Translation
'*
'\*******************************************************************

sub main

    Dim StartZeit
    Dim bAsianLanguage   as Boolean
    Dim DefaultLanguage  as String
    
    StartZeit = Now()
    
    use "writer\tools\includes\w_tools.inc"
    use "writer\tools\includes\w_tool3.inc"
    use "writer\optional\includes\tools\tools1.inc"
    use "writer\optional\includes\tools\tools2.inc"
    use "writer\optional\includes\chinesetranslate\w_chinesetranslate1.inc"
    use "writer\optional\includes\chinesetranslate\w_chinesetranslate2.inc"
    
    printlog Chr(13) + "Loading of Include - Files takes: " + Wielange ( StartZeit )
    printlog Chr(13) + "******* Writer - Level 1 - Test *******"
    
    Call hStatusIn ( "writer", "w_chinesetranslate.bas" , "Chinese Translation" )
    
    printlog Chr(13) + "      - Test Chinese Translation"
    
    'Enable 'Asian Language support' ON
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    ' Set default language(asian) for document is "Simplified Chinese"
    DefaultLanguage = fSetAsian(fGetLanguageName(86))
    
    printlog Chr(13) + " -- Default setting -- "
    
    'Don't need to run this case - tDefaultSetting_1
    Call tDefaultSetting_1
    Call tDefaultSetting_2
    Call tDefaultSetting_3
    
    printlog Chr(13) + " -- Standard Function -- "
    Call tChineseTranslate_1
    Call tChineseTranslate_2
    Call tChineseTranslate_3
    Call tChineseTranslate_4
    Call tChineseTranslate_5
    
    printlog Chr(13) + " -- Chinese Dictionary -- "
    Call tChineseTranslate_6
    Call tChineseTranslate_7
    Call tChineseTranslate_8
    Call tChineseTranslate_9
    Call tChineseTranslate_10
    Call tChineseTranslate_11
    
    printlog Chr(13) + " -- Undo/Redo -- "
    Call tUndoRedo
    
    'Set default language(asian) to default
    Call fSetAsian(DefaultLanguage)   
    'Set the 'Asian Language support' to default
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    
    Call hStatusOut
    
    Printlog Chr(13) + "End of Level 1 Test - Chinese Conversion"
    Printlog "Duration: "+ WieLange ( StartZeit )
    Printlog "Date: " +  Date + "    Time: " + Time

end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    Call GetUseFiles
    gApplication = "WRITER"
end Sub
