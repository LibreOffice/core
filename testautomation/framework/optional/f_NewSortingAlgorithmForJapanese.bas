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
'* owner: thorsten.bosbach@sun.com
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

