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
'* owner :  helge.delfs@oracle.com
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
