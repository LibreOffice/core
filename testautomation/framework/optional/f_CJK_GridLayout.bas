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
