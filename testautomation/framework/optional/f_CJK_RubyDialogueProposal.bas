'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
' 
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner: thorsten.bosbach@oracle.com
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
