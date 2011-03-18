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
'* owner : helge.delfs@oracle.com
'*
'* short description : Test CJK and CTL support dependency
'*
'************************************************************************
'*
' #1 tCJKDependency_1        'Test Tabpage 'Text Grid'
' #1 tCJKDependency_2        'Test Tabpage 'Asian Typography'
' #1 tCJKDependency_3        'Test CheckBox 'Snap to text grid (if active)'
' #1 tCJKDependency_4        'Test listbox 'Text direction' in page
' #1 tCJKDependency_5        'Test listbox 'Text direction' in Frame
' #1 tCTLDependency_1        'Test listbox 'Text direction' -- insert/section
' #1 tCTLDependency_2        'Test listbox 'Text direction' -- format/section
' #1 tCTLDependency_3        'Test listbox 'Text direction' -- table
' #1 tCTLDependency_4        'Test 'left to right' and 'right to left' button(Writer)
' #1 tCTLDependency_5        'Test 'left to right' and 'right to left' button - HTML(HTML 3.2)
' #1 tCTLDependency_6        'Test 'left to right' and 'right to left' button - HTML(Microsoft Internet Bxplorer)
' #1 tCTLDependency_7        'Test 'left to right' and 'right to left' button - HTML(Netscape Navigator)
' #1 tCTLDependency_8        'Test 'left to right' and 'right to left' button - HTML(StarOffice Writer)
' #1 tCTLKashidaCheck        'Check for additional numerals if CTL support is enabled
'*
'\***********************************************************************

testcase tCJKDependency_1

  Dim bAsianLanguage   as Boolean

  PrintLog "- Tabpage 'Text Grid"

  printlog " Open a new document"
  Call hNewDocument

  printlog " Set 'Asian Language support' ON"
   bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

  printlog " Format/Page , the tabpage 'Text Grid' should exist"
   FormatPageWriter
   try
       Kontext
       Active.SetPage TabGrid
       Kontext "TabGrid"
       TabGrid.Cancel
   catch
       Kontext
       Active.SetPage TabSeite
       Kontext "TabSeite"
       TabSeite.Cancel
       warnlog "Should get Tabpage: Grid Text"
   endcatch

   'if system language is Asian language, the checkbox
   ''Asian Language support' is checked by default and
   'can't do any operation for it
   if iSystemSprache = 81 Or iSystemSprache = 82 Or iSystemSprache = 86 or iSystemSprache = 88 then
       Goto NoTest
   end if

  printlog " Set 'Asian Language support' Off"
   ActiveDeactivateAsianSupport(FALSE)

  printlog " Format/Page , the tabpage 'Text Grid' shouldn't exist"
   FormatPageWriter
   try
       Kontext
       Active.SetPage TabGrid
       Kontext "TabGrid"
       TabGrid.Cancel
       warnlog "Shouldn't get Tabpage: Grid Text"
   catch
       Kontext
       Active.SetPage TabSeite
       Kontext "TabSeite"
       TabSeite.Cancel
   endcatch

  printlog " Set the 'Asian Language support' to default"
   if bAsianLanguage = FALSE then
       Call ActiveDeactivateAsianSupport(FALSE)
   end if

NoTest:
  printlog " Close active document"
   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCJKDependency_2

  Dim bAsianLanguage   as Boolean

  PrintLog "- Tabpage'Asian Typography'"

  printlog " Open a new document"
  Call hNewDocument

  printlog " Set 'Asian Language support' ON"
   bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

  printlog " Format/Paragraph , the tabpage 'Asian Typography' should exist"
   FormatParagraph
   try
       Kontext
       Active.SetPage TabAsianTypography
       Kontext "TabAsianTypography"
       TabAsianTypography.Cancel
   catch
       Kontext
       Active.SetPage TabTextfluss
       Kontext "TabTextfluss"
       TabTextfluss.Cancel
       warnlog "Should get Tabpage: Asian Typography"
   endcatch

   'if system language is Asian language, the checkbox
   ''Asian Language support' is checked by default and
   'can't do any operation for it
   if iSystemSprache = 81 Or iSystemSprache = 82 Or iSystemSprache = 86 or iSystemSprache = 88 then
       Goto NoTest
   end if

  printlog " Set 'Asian Language support' Off"
   ActiveDeactivateAsianSupport(FALSE)

  printlog " Format/Page , the tabpage 'Asian Typography' shouldn't exist"
   FormatParagraph
   try
       Kontext
       Active.SetPage TabAsianTypography
       Kontext "TabAsianTypography"
       TabAsianTypography.Cancel
       warnlog "Shouldn't get Tabpage: Asian Typography"
   catch
       Kontext
       Active.SetPage TabTextfluss
       Kontext "TabTextfluss"
       TabTextfluss.Cancel
   endcatch

  printlog " Set the 'Asian Language support' to default"
   if bAsianLanguage = FALSE then
       Call ActiveDeactivateAsianSupport(FALSE)
   end if

NoTest:
  printlog " Close active document"
   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCJKDependency_3

  Dim bAsianLanguage as Boolean

  PrintLog "- CheckBox 'Snap to text grid (if active)'"

  printlog " Open a new document"
  Call hNewDocument

  printlog " Set 'Asian Language support' ON"
   bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

  printlog " Format/Paragraph / Alignment ,"
  printlog " the CheckBox 'Snap to text grid (if active)' should exist"
   Call fFormatParagraph("TabAusrichtungAbsatz")
       try
           TextrasterFangen.UnCheck
       catch
           warnlog "Should get checkbox 'Snap to text grid (if active)'"
       endcatch
   TabAusrichtungAbsatz.Cancel

   'if system language is Asian language, the checkbox
   ''Asian Language support' is checked by default and
   'can't do any operation for it
   if iSystemSprache = 81 Or iSystemSprache = 82 Or iSystemSprache = 86 or iSystemSprache = 88 then
       Goto NoTest
   end if

  printlog " Set 'Asian Language support' Off"
   ActiveDeactivateAsianSupport(FALSE)

  printlog " Format/Paragraph / Alignment ,"
  printlog " the CheckBox 'Snap to text grid (if active)' shouldn't exist"
   Call fFormatParagraph("TabAusrichtungAbsatz")
       try
           TextrasterFangen.UnCheck
           warnlog "Shouldn't get checkbox 'Snap to text grid (if active)'"
       catch
       endcatch
   TabAusrichtungAbsatz.Cancel

  printlog " Set the 'Asian Language support' to default"
   if bAsianLanguage = FALSE then
       Call ActiveDeactivateAsianSupport(FALSE)
   end if

NoTest:
  printlog " Close active document"
   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCJKDependency_4

    Dim bAsianLanguage as Boolean
    Dim bCTLLanguage as Boolean
    'if system language is Asian language, the checkbox
    ''Asian Language support' is checked by default and
    'can't do any operation for it
    if iSystemSprache = 81 Or iSystemSprache = 82 Or iSystemSprache = 86 or iSystemSprache = 88 then
        goto endsub
    end if

    PrintLog "- listbox 'Text direction' - Page"

    printlog " Open a new document"
    Call hNewDocument

    printlog " Set 'CTL language support' OFF"
    bCTLLanguage = ActiveDeactivateCTLSupport(FALSE)

    printlog " Set 'Asian Language support' ON"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)

    printlog " Format/Page / Page, the listbox 'Text direction' should exist"
    Call fFormatPageWriter("TabSeite")
    if Textfluss.IsVisible = TRUE then
        Textfluss.Select 2
    else
        warnlog "Should get listbox 'Text direction'"
    end if
    TabSeite.Cancel

    printlog " Set 'Asian Language support' Off"
    ActiveDeactivateAsianSupport(FALSE)    
    printlog " Format/Page / Page, the listbox 'Text direction' shouldn't exist"
    Call fFormatPageWriter("TabSeite")
    if Textfluss.IsVisible = TRUE then
        warnlog "Shouldn't get listbox 'Text direction'"
    end if
    TabSeite.Cancel

    printlog " Set the 'Asian Language support' to default"
    if bAsianLanguage = FALSE then
       Call ActiveDeactivateAsianSupport(FALSE)
    end if

    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCJKDependency_5

    Dim bAsianLanguage as Boolean
    Dim bCTLLanguage as Boolean

    PrintLog "- listbox 'Text direction' - Frame"

    printlog " Open a new document"
    Call hNewDocument

    printlog " Set 'CTL language support' OFF"
    bCTLLanguage = ActiveDeactivateCTLSupport(FALSE)

    printlog " Set 'Asian Language support' ON"
    bAsianLanguage = ActiveDeactivateAsianSupport(TRUE)
    
    printlog " Insert a frame"
    Call wInsertFrame
    
    printlog " Format/Frame / Options, the listbox 'Text direction' should exist"
    Call fFormatFrame("TabZusaetze")
    if Not TextDirection.IsVisible then
        warnlog "Should get listbox 'Text direction'"
    end if
    TabZusaetze.Cancel
    
    'if system language is Asian language, the checkbox
    ''Asian Language support' is checked by default and
    'can't do any operation for it
    if iSystemSprache = 81 Or iSystemSprache = 82 Or iSystemSprache = 86 or iSystemSprache = 88 then
        Goto NoTest
    end if
    
    printlog " Set 'Asian Language support' Off"
    ActiveDeactivateAsianSupport(FALSE)
    
    printlog " Format/Frame / Options, the listbox 'Text direction' shouldn't exist"
    Call fFormatFrame("TabZusaetze")
    if TextDirection.IsVisible = TRUE then
        QAErrorLog "#135837# Shouldn't get listbox 'Text direction'."
    end if
    TabZusaetze.Cancel
    
    printlog " Set the 'Asian Language support' to default"
    if bAsianLanguage = FALSE then
        Call ActiveDeactivateAsianSupport(FALSE)
    end if
    
NoTest:
    printlog " Set the 'CTL language support' to default"
    if bCTLLanguage = TRUE then
        Call ActiveDeactivateAsianSupport(TRUE)
    end if
    
    printlog " Close active document"
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCTLDependency_1

    Dim bCTLLanguage as Boolean

    PrintLog "- listbox 'Text direction' -- insert/section"

    printlog " Open a new document"
    Call hNewDocument

    printlog " Set 'CTL Language support' ON"
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    
    printlog " Insert/Section/Columns,"
    printlog " the listbox 'Text direction' should exist"
    Call fInsertSection("TabSpalten")
    if Not TextDirection.IsVisible then
        Warnlog "Should get listbox 'Text direction'"
    end if
    TabSpalten.Cancel
    
    printlog " Set 'CTL Language support' Off"
    Call ActiveDeactivateCTLSupport(FALSE)
    
    printlog " Insert/Section/Columns,"
    printlog " the listbox 'Text direction' shouldn't exist"
    Call fInsertSection("TabSpalten")
    if TextDirection.IsVisible then
        Warnlog "Shouldn't get listbox 'Text direction'"
    end if
    TabSpalten.Cancel
    
    printlog " Set the 'CTL Language support' to default"
    if bCTLLanguage = TRUE then
        Call ActiveDeactivateCTLSupport(TRUE)
    end if

    printlog " Close active document"
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCTLDependency_2

    Dim bCTLLanguage as Boolean

    PrintLog "- listbox 'Text direction' -- format/section"

    printlog " Open a new document"
    Call hNewDocument
    
    printlog " Insert a section"
    Call fInsertSection("TabBereiche")
    TabBereiche.OK
    
    printlog " Set 'CTL Language support' ON"
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    
    printlog " Format/Section , then click option button,"
    printlog " the listbox 'Text direction' should exist"
    FormatSections
    Kontext "BereicheBearbeiten"
    Optionen.Click
    Sleep 1
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    if Not TextDirection.IsVisible then
        Warnlog "Should get listbox 'Text direction'"
    end if
    TabSpalten.Cancel
    Sleep 1
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel
    
    printlog " Set 'CTL Language support' Off"
    Call ActiveDeactivateCTLSupport(FALSE)
    
    printlog " Format/Section , then click option button,"
    printlog " the listbox 'Text direction' shouldn't exist"
    FormatSections
    Kontext "BereicheBearbeiten"
    Optionen.Click
    Kontext
    Active.Setpage TabSpalten
    Kontext "TabSpalten"
    if TextDirection.IsVisible then
        Warnlog "Shouldn't get listbox 'Text direction'"
    end if
    TabSpalten.Cancel
    Sleep 1
    Kontext "BereicheBearbeiten"
    BereicheBearbeiten.Cancel
    
    printlog " Set the 'CTL Language support' to default"
    if bCTLLanguage = TRUE then
        Call ActiveDeactivateCTLSupport(TRUE)
    end if
    
    printlog " Close active document"
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCTLDependency_3

    Dim bCTLLanguage as Boolean
    PrintLog "- listbox 'Text direction' -- Table"
    
    printlog " Open a new document"
    Call hNewDocument
    
    printlog " Insert a table"
    Call hTabelleEinfuegen
    
    printlog " Set 'CTL Language support' ON"
    bCTLLanguage = ActiveDeactivateCTLSupport(TRUE)
    
    printlog " Format/Table / Table ,"
    printlog " the listbox 'Text direction' should exist"
    Call fFormatTable("TabTabelle")
    if Not TextDirection.IsVisible then
        Warnlog "Should get listbox 'Text direction'"
    end if
    TabTabelle.Cancel
    
    printlog " Set 'CTL Language support' Off"
    Call ActiveDeactivateCTLSupport(FALSE)
    
    printlog " Format/Table / Table ,"
    printlog " the listbox 'Text direction' shouldn't exist"
    Call fFormatTable("TabTabelle")
    if TextDirection.IsVisible then
        Warnlog "Shouldn't get listbox 'Text direction'"
    end if
    TabTabelle.Cancel
    
    printlog " Set the 'CTL Language support' to default"
    if bCTLLanguage = TRUE then
        Call ActiveDeactivateCTLSupport(TRUE)
    end if
    
    printlog " Close active document"
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCTLDependency_4

  Dim bCTLLanguage as Boolean
  Dim bLToR as Boolean
  Dim bRToL as Boolean

  PrintLog "- 'left to right' and 'right to left' button "

  printlog " Open a new document"
  Call hNewDocument

   bLToR = fActiveObjectInToolbar("Text Object" , "Left-To-Right" , TRUE)
   bRToL = fActiveObjectInToolbar("Text Object" , "Right-To-Left" , TRUE)

  printlog " Set 'CTL Language support' Off"
   bCTLLanguage = ActiveDeactivateCTLSupport(FALSE)
   Sleep 2

  printlog " Check if 'left to right' button and 'right to left' button"
  printlog " can be clicked in text tool bar -- Shouldn't be"
   Kontext "TextObjectbar"
   try
       LeftToRight.Click
       warnlog "Left To Right button shouldn't be clicked!"
   catch
   endcatch
   try
       RightToLeft.Click
       warnlog "Right To Left button shouldn't be clicked!"
   catch
   endcatch

  printlog " Set 'CTL Language support' ON"
   ActiveDeactivateCTLSupport(TRUE)

  printlog " Check if 'left to right' button and 'right to left' button"
  printlog " can be clicked in text tool bar -- Should be"
   Kontext "TextObjectbar"
   Sleep 2
   try
       LeftToRight.Click
       Sleep 2
   catch
       warnlog "#i38788# Left To Right button can't be clicked!"
   endcatch
   try
       RightToLeft.Click
       Sleep 2
   catch
       warnlog "#i38788# Right To Left button can't be clicked!"
   endcatch

  'Set R to L and L to R button disappear
   if bLToR = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Left-To-Right" , FALSE)
   end if
   if bRToL = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Right-To-Left" , FALSE)
   end if

  printlog " Set the 'CTL Language support' to default"
   if bCTLLanguage = FALSE then
       Call ActiveDeactivateCTLSupport(FALSE)
   end if

  printlog " Close active document"
   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCTLDependency_5

    Dim bCTLLanguage as Boolean
    Dim bLToR as Boolean
    Dim bRToL as Boolean
    Dim iHtmlExportMode as Integer
    Dim iCurrentHtmlExportMode as Integer
    
    iHtmlExportMode = 1
    
    PrintLog "- 'left to right' and 'right to left' button - HTML (HTML 3.2)"
    
    printlog " Open a new document"
    Call hNewDocument
    
    bLToR = fActiveObjectInToolbar("Text Object" , "Left-To-Right" , TRUE)
    bRToL = fActiveObjectInToolbar("Text Object" , "Right-To-Left" , TRUE)
    
    printlog " Set Export mode to HTML 3.2"
    printlog " Tools/load&save/HTML compatibility/Exprt"
    ToolsOptions
    Call hToolsOptions ( "LoadSave", "HTMLCompatibility" )
    iCurrentHtmlExportMode = Export.GetSelIndex
    Sleep 1
    Export.Select iHtmlExportMode
    Sleep 1
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    PrintLog "Set 'CTL Language support' Off"
    bCTLLanguage = ActiveDeactivateCTLSupport(FALSE)
    Sleep 2
    
    printlog " Check if 'left to right' button and 'right to left' button"
    printlog " can be clicked in text tool bar -- Shouldn't be"
    Kontext "TextObjectbar"
    try
        LeftToRight.Click
        warnlog "Left To Right button shouldn't be clicked!"
    catch
    endcatch
    try
        RightToLeft.Click
        warnlog "Right To Left button shouldn't be clicked!"
    catch
    endcatch
    
    PrintLog "Set 'CTL Language support' ON"
    ActiveDeactivateCTLSupport(TRUE)
    
    printlog " Check if 'left to right' button and 'right to left' button"
    printlog " can be clicked in text tool bar -- Should NOT be"
    Kontext "TextObjectbar"
    Sleep 2
    try
       LeftToRight.Click
       QAErrorLog "#115836# Left To Right button shouldn't be clicked!"
       Sleep 2
    catch
    endcatch
    try
       RightToLeft.Click
       QAErrorLog "#115836# Right To Left button shouldn't be clicked!"
       Sleep 2
    catch
    endcatch
    
    'Set R to L and L to R button disappear
    if bLToR = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Left-To-Right" , FALSE)
    end if
    if bRToL = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Right-To-Left" , FALSE)
    end if

    printlog " Set the 'CTL Language support' to default"
    if bCTLLanguage = FALSE then
       Call ActiveDeactivateCTLSupport(FALSE)
    end if

    printlog " Set Export mode to default"
    ToolsOptions
    Call hToolsOptions ( "LoadSave", "HTMLCompatibility" )
     Export.Select iCurrentHtmlExportMode
     Sleep 1
     Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog " Close active document"
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCTLDependency_6

  Dim bCTLLanguage as Boolean
  Dim bLToR as Boolean
  Dim bRToL as Boolean
  Dim iHtmlExportMode as Integer
  Dim iCurrentHtmlExportMode as Integer

  iHtmlExportMode = 2

  PrintLog "- 'left to right' and 'right to left' button - HTML(Microsoft Internet Bxplorer)"

  printlog " Open a new document"
  Call hNewDocument

   bLToR = fActiveObjectInToolbar("Text Object" , "Left-To-Right" , TRUE)
   bRToL = fActiveObjectInToolbar("Text Object" , "Right-To-Left" , TRUE)

  printlog " Set Export mode to Microsoft Internet Bxplorer"
  printlog " Tools/load&save/HTML compatibility/Exprt"
   ToolsOptions
   Call hToolsOptions ( "LoadSave", "HTMLCompatibility" )
     iCurrentHtmlExportMode = Export.GetSelIndex
     Export.Select iHtmlExportMode
     Sleep 1
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  printlog " Set 'CTL Language support' Off"
   bCTLLanguage = ActiveDeactivateCTLSupport(FALSE)
   Sleep 2

  printlog " Check if 'left to right' button and 'right to left' button"
  printlog " can be clicked in text tool bar -- Shouldn't be"
   Kontext "TextObjectbar"
   try
       LeftToRight.Click
       warnlog "Left To Right button shouldn't be clicked!"
   catch
   endcatch
   try
       RightToLeft.Click
       warnlog "Right To Left button shouldn't be clicked!"
   catch
   endcatch

  printlog " Set 'CTL Language support' ON"
   ActiveDeactivateCTLSupport(TRUE)

  printlog " Check if 'left to right' button and 'right to left' button"
  printlog " can be clicked in text tool bar -- Should be"
   Kontext "TextObjectbar"
   Sleep 2
   try
       LeftToRight.Click
       Sleep 2
   catch
       warnlog "#i38788# Left To Right button can't be clicked!"
   endcatch
   try
       RightToLeft.Click
       Sleep 2
   catch
       warnlog "#i38788# Right To Left button can't be clicked!"
   endcatch

  'Set R to L and L to R button disappear
   if bLToR = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Left-To-Right" , FALSE)
   end if
   if bRToL = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Right-To-Left" , FALSE)
   end if

  printlog " Set the 'CTL Language support' to default"
   if bCTLLanguage = FALSE then
       Call ActiveDeactivateCTLSupport(FALSE)
   end if

  printlog " Set Export mode to default"
   ToolsOptions
   Call hToolsOptions ( "LoadSave", "HTMLCompatibility" )
     Export.Select iCurrentHtmlExportMode
     Sleep 1
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  printlog " Close active document"
   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCTLDependency_7

  Dim bCTLLanguage as Boolean
  Dim bLToR as Boolean
  Dim bRToL as Boolean
  Dim iHtmlExportMode as Integer
  Dim iCurrentHtmlExportMode as Integer

  iHtmlExportMode = 3

  PrintLog "- 'left to right' and 'right to left' button - HTML(Netscape Navigator)"

  printlog " Open a new document"
  Call hNewDocument

   bLToR = fActiveObjectInToolbar("Text Object" , "Left-To-Right" , TRUE)
   bRToL = fActiveObjectInToolbar("Text Object" , "Right-To-Left" , TRUE)

  printlog " Set Export mode to Netscape Navigator"
  printlog " Tools/load&save/HTML compatibility/Exprt"
   ToolsOptions
   Call hToolsOptions ( "LoadSave", "HTMLCompatibility" )
     iCurrentHtmlExportMode = Export.GetSelIndex
     Export.Select iHtmlExportMode
     Sleep 1
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  printlog " Set 'CTL Language support' Off"
   bCTLLanguage = ActiveDeactivateCTLSupport(FALSE)
   Sleep 2

  printlog " Check if 'left to right' button and 'right to left' button"
  printlog " can be clicked in text tool bar -- Shouldn't be"
   Kontext "TextObjectbar"
   try
       LeftToRight.Click
       warnlog "Left To Right button shouldn't be clicked!"
   catch
   endcatch
   try
       RightToLeft.Click
       warnlog "Right To Left button shouldn't be clicked!"
   catch
   endcatch

  printlog " Set 'CTL Language support' ON"
   ActiveDeactivateCTLSupport(TRUE)

  printlog " Check if 'left to right' button and 'right to left' button"
  printlog " can be clicked in text tool bar -- Should be"
   Kontext "TextObjectbar"
   Sleep 2
   try
       LeftToRight.Click
       Sleep 2
   catch
       warnlog "#i38788# Left To Right button can't be clicked!"
   endcatch
   try
       RightToLeft.Click
       Sleep 2
   catch
       warnlog "#i38788# Right To Left button can't be clicked!"
   endcatch

  'Set R to L and L to R button disappear
   if bLToR = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Left-To-Right" , FALSE)
   end if
   if bRToL = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Right-To-Left" , FALSE)
   end if

  printlog " Set the 'CTL Language support' to default"
   if bCTLLanguage = FALSE then
       Call ActiveDeactivateCTLSupport(FALSE)
   end if

  printlog " Set Export mode to default"
   ToolsOptions
   Call hToolsOptions ( "LoadSave", "HTMLCompatibility" )
     Export.Select iCurrentHtmlExportMode
     Sleep 1
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  printlog " Close active document"
   Call hCloseDocument
endcase

'-------------------------------------------------------------------------------------------

testcase tCTLDependency_8

  Dim bCTLLanguage as Boolean
  Dim bLToR as Boolean
  Dim bRToL as Boolean
  Dim iHtmlExportMode as Integer
  Dim iCurrentHtmlExportMode as Integer

  iHtmlExportMode = 4

  PrintLog "- 'left to right' and 'right to left' button - HTML(StarOffice Writer)"

  printlog " Open a new document"
  Call hNewDocument

   bLToR = fActiveObjectInToolbar("Text Object" , "Left-To-Right" , TRUE)
   bRToL = fActiveObjectInToolbar("Text Object" , "Right-To-Left" , TRUE)

  printlog " Set Export mode to StarOffice Writer"
  printlog " Tools/load&save/HTML compatibility/Exprt"
   ToolsOptions
   Call hToolsOptions ( "LoadSave", "HTMLCompatibility" )
     iCurrentHtmlExportMode = Export.GetSelIndex
     Export.Select iHtmlExportMode
     Sleep 1
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  printlog " Set 'CTL Language support' Off"
   bCTLLanguage = ActiveDeactivateCTLSupport(FALSE)
   Sleep 2

  printlog " Check if 'left to right' button and 'right to left' button"
  printlog " can be clicked in text tool bar -- Shouldn't be"
   Kontext "TextObjectbar"
   try
       LeftToRight.Click
       warnlog "Left To Right button shouldn't be clicked!"
   catch
   endcatch
   try
       RightToLeft.Click
       warnlog "Right To Left button shouldn't be clicked!"
   catch
   endcatch

  printlog " Set 'CTL Language support' ON"
   ActiveDeactivateCTLSupport(TRUE)

  printlog " Check if 'left to right' button and 'right to left' button"
  printlog " can be clicked in text tool bar -- Should be"
   Kontext "TextObjectbar"
   Sleep 2
   try
       LeftToRight.Click
       Sleep 2
   catch
       warnlog "#i38788# Left To Right button can't be clicked!"
   endcatch
   try
       RightToLeft.Click
       Sleep 2
   catch
       warnlog "#i38788# Right To Left button can't be clicked!"
   endcatch

  'Set R to L and L to R button disappear
   if bLToR = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Left-To-Right" , FALSE)
   end if
   if bRToL = FALSE then
       Call fActiveObjectInToolbar("Text Object" , "Right-To-Left" , FALSE)
   end if

  printlog " Set the 'CTL Language support' to default"
   if bCTLLanguage = FALSE then
       Call ActiveDeactivateCTLSupport(FALSE)
   end if

  printlog " Set Export mode to default"
   ToolsOptions
   Call hToolsOptions ( "LoadSave", "HTMLCompatibility" )
     Export.Select iCurrentHtmlExportMode
     Sleep 1
     Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

  printlog " Close active document"
   Call hCloseDocument
endcase

'----------------------------------------------------------------------------------------------------

testcase tCTLKashidaCheck

    Dim bCTLLanguage as Boolean
    Dim bAsianLanguage as boolean
    Dim iListCounter as Integer

    printlog " Open a new document"
    Call hNewDocument

    printlog " Set 'Asian support' to ON"
    bAsianLanguage = ActiveDeactivateAsianSupport(True)
    printlog " Set 'CTL Language support' to ON" 
    bCTLLanguage = ActiveDeactivateCTLSupport(True)         
    Sleep 2

    ToolsOptions
    Call hToolsOptions ("LanguageSettings","ComplexTextLayout")        
    Kontext "TabComplexTextLayout"     

    iListCounter = Numerals.GetItemCount
    
    if iListCounter <> 4 then
        Warnlog "There are not 4 items in list 'Numerlas' but " + iListCounter
    endif  

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    if bAsianLanguage = FALSE then
        printlog " Set the 'Asian Language support' to default"    
        Call ActiveDeactivateAsianSupport(bAsianLanguage)
    end if
    
    if bCTLLanguage = FALSE then
        printlog " Set the 'CTL Language support' to default"    
        Call ActiveDeactivateCTLSupport(FALSE)
    end if

   printlog " Close active document"
   Call hCloseDocument
endcase

'----------------------------------------------------------------------------------------------------


