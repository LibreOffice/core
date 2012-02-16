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
'* short description: CJK Ruby DialogueProposal test
'*
'\*********************************************************************

global sAsianLanguageFlag as String

sub main
    PrintLog "---------------------------------------------------------------------"
    Printlog "-----                    CJK Ruby Dialogue Proposal test        -----"
    PrintLog "---------------------------------------------------------------------"

    use "framework\optional\includes\CJK_RubyDialogueProposal_1.inc"
    
    sAsianLanguageFlag = CheckAsianLanguageSupport("Current")
    Call CheckAsianLanguageSupport("On")
    Call hStatusIn("framework", "f_CJK_RubyDialogueProposal.bas")
    
     printLog Chr(13) + "- CJK Ruby Dialogue Proposal test -- Writer"
      Call tRubyDialog_1      'Check Apply and Close button in Ruby dialogue
      Call tRubyDialog_2      'Test scrollbar in Ruby dialog
      Call tRubyDialog_3      'The list in ruby dialogure will be updated 
                              'as soon as the uses  change the select in the document.
      Call tRubyDialog_4      'Check default setting for Character Style of Ruby Text 
      Call tRubyDialog_5      'Check Stylist
    Call hStatusOut

    if sAsianLanguageFlag = "Off" then  
        Call CheckAsianLanguageSupport("Off")
    else 
        Call CheckAsianLanguageSupport("On")
    end if      
end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"

   use "framework\tools\includes\CJK_tools.inc"

   gApplication   = "WRITER"
   Call GetUseFiles
End sub
