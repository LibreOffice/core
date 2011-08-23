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
'* short description: Grid Layout for CJK feature test
'*
'\*********************************************************************

global sAsianLanguageFlag as String

sub main
    PrintLog "---------------------------------------------------------------------"
    Printlog "-----                    Grid Layout for CJK  in Writer         -----"
    PrintLog "---------------------------------------------------------------------"
    
    use "framework\optional\includes\w_grid_layout1.inc"
   
    sAsianLanguageFlag = CheckAsianLanguageSupport("Current")
    Call CheckAsianLanguageSupport("On")
     
    Call hStatusIn("framework", "f_CJK_GridLayout.bas")

     printLog Chr(13) + "- Grid Layout for CJK test in Writer"

       Call tTextGridDialog_1         'Check if tabpage 'Text Grid' appears when "Asian Language support" is checked . (vice versa)
       Call tTextGridDialog_2         'No grid is active, no changes to existing version
       Call tTextGridDialog_3         'Grid (lines only): The page is set up with a defined number of lines
       Call tTextGridDialog_4         'Grid (lines and characters): The page is set up with a defined number of lines and a defined number of characters in each line 

       Call tGridDisplay_1            'Printgrid: The grid lines will be printed. (vice versa)

       Call tFontSizeChanges_1        'Change the font size to bigger enough will change the layout 
     
       Call tRubyText_1               'If Ruby Text is applied and longer than its Base Text it will stretch over more grid cells than the base text itself.

       Call tIndentsTest_1            'Indents should work and always indenting as close as possible to their measurements by snapping to the appropriate grid cell. Tabs therefore will be ignored.

    Call hStatusOut

    if sAsianLanguageFlag = "Off" Then  
       Call CheckAsianLanguageSupport("Off")
    else 
       Call CheckAsianLanguageSupport("On")
    end if    	
end sub

sub LoadIncludeFiles
    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
   
    use "framework\tools\includes\CJK_tools.inc"   
    gApplication   = "WRITER"
    Call GetUseFiles
end sub
