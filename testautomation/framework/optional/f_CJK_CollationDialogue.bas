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
'* short description: CJK Collation Dialogue test
'*
'\*********************************************************************

global sAsianLanguageFlag as String

sub main
    printlog "---------------------------------------------------------------------"
    Printlog "-----                    CJK Collation Dialogue test            -----"
    printlog "---------------------------------------------------------------------"
   
    use "framework\optional\includes\CJK_CollationDialogue_1.inc"
    
    sAsianLanguageFlag = CheckAsianLanguageSupport("Current")
    call CheckAsianLanguageSupport("On")
   
    call hStatusIn("framework", "f_CJK_CollationDialogue.bas")         
        printlog Chr(13) + "- Language --- Writer"
        call tLanguageTest_1       
        call tLanguageTest_2       
        call tStrokeSort          
        call tPinyinSort          
        
        gApplication = "CALC"      
        printlog Chr(13) + "- Sort option --- Calc"
        call tSortOptionTest_1 
    
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
