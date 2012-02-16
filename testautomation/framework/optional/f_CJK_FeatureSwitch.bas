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
'* short description: CJK feature switch test
'*
'\*********************************************************************

global sAsianLanguageFlag as String

sub main
    printlog "---------------------------------------------------------------------"
    printlog "-----                    CJK feature switch test                -----"
    printlog "---------------------------------------------------------------------"
   
    use "framework\optional\includes\CJK_FeatureSwitchTest_1.inc"
    
    sAsianLanguageFlag = CheckAsianLanguageSupport("Current")

    call CheckAsianLanguageSupport("On")
    
    call hStatusIn("framework", "f_CJK_FeatureSwitch.bas") 
      printlog Chr(13) + "- CJK Feature Switch test -- Writer"    
          call tSwitchLocation      
          '--Turn on/off Asian language support --"
          call tWriterUIChangesTest1    
          call tWriterUIChangesTest2    
          call tWriterUIChangesTest3    
          call tWriterUIChangesTest4    
          call tWriterUIChangesTest5    
          call tWriterUIChangesTest6    
          call tWriterUIChangesTest7    
          call tWriterUIChangesTest8    
          call tWriterUIChangesTest9    
          call tWriterUIChangesTest10   
          call tWriterUIChangesTest11   
          call tProlongedVowelsTest1    
      printlog Chr(13) + "- CJK Feature Switch test -- Calc"
          gApplication = "CALC"
          '--Turn on/off Asian language support --"
          call tCalcUIChangesTest_10       
      printlog Chr(13) + "- CJK Feature Switch test -- Impress"
          gApplication = "IMPRESS"
          '--Turn on/off Asian language support --"
          call tImpressUIChangesTest_10          
      printlog Chr(13) + "- CJK Feature Switch test -- Draw"
          gApplication = "DRAW"    
          '--Turn on/off Asian language support --"
          call tDrawUIChangesTest_10

    call hStatusOut

    if sAsianLanguageFlag = "Off" then  
        call CheckAsianLanguageSupport("Off")
     else 
       call CheckAsianLanguageSupport("On")
    end if      
end sub

'-------------------------------------------------------------------------

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
   
    use "framework\tools\includes\CJK_tools.inc"
   
    gApplication   = "WRITER"
    call GetUseFiles
    
end sub
