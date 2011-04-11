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
'* short description : Shortcut-test in writer
'*
'************************************************************************
'*
'*    tShortcutGlobalNew
'*    tShortcutGlobalOpen
'*    tShortcutGlobalSave
'*    tShortcutGlobalSaveAs
'*    tShortcutSelectAll
'*    tShortcutFindAndReplace
'*    tShortcutBold
'*    tShortcutItalic
'*    tShortcutUnderline
'*    tShortcutDoubleUnderline
'*    tShortcutAlign
'*    tShortcutSuperscript
'*    tShortcutSubscript
'*
'\***********************************************************************

sub w_shortcuts

    Call tShortcutGlobalNew
    Call tShortcutGlobalOpen
    Call tShortcutGlobalSave
    Call tShortcutGlobalSaveAs
    Call tShortcutSelectAll
    Call tShortcutFindAndReplace
    Call tShortcutBold
    Call tShortcutItalic
    Call tShortcutUnderline
    Call tShortcutDoubleUnderline
    Call tShortcutAlign
    Call tShortcutSuperscript
    Call tShortcutSubscript

end sub

testcase tShortcutGlobalNew
    Dim vShortcut as string
    Dim vNoOfDocs as integer
    '/// This testcases checks the global shortcut -> New

    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 49, 31, 33, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'New' in en/sv/pt-BR is <Strg+N>
            '/// Shortcut for 'New' in de is <Strg+N>
            '/// Shortcut for 'New' in fr is <Strg+N>
            '/// Shortcut for 'New' in it is <Strg+N>
            vShortcut = "<Mod1 N>"
        Case 34
            '/// Shortcut for 'New' in es is <Strg+U>
            vShortcut = "<Mod1 U>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select
    
    vNoOfDocs = GetDocumentCount
    '/// After executing previous named shortcut a new document should be opened
    Call wTypeKeys ( vShortcut )
    wait 500
    if GetDocumentCount = (vNoOfDocs + 1) then
        printlog "Shortcut " & vShortcut & " opens a new document!"
        '/// close document
        Call hCloseDocument
    else
        Warnlog "Shortcut " & vShortcut & " didn't open a new document!"
    end if
    '/// Close document opened on start
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutGlobalOpen
    Dim vShortcut as string
    '/// This testcases checks the global shortcut -> Open

    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 49, 31, 33, 39, 36, 46, 55, 86, 82, 81
            '/// Shortcut for 'Open' in en/sv/pt-BR is <Strg+O>
            '/// Shortcut for 'Open' in de is <Strg+O>
            '/// Shortcut for 'Open' in fr is <Strg+O>
            '/// Shortcut for 'Open' in it is <Strg+O>
            vShortcut = "<Mod1 O>"
        Case 34
            '/// Shortcut for 'Open' in es is <Strg+A>
            vShortcut = "<Mod1 A>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select
    
    '/// After executing previous named shortcut FileOpen dialog should be opened
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "OeffnenDlg"
    if OeffnenDlg.Exists then
        printlog "Shortcut " & vShortcut & " opens FileOpen dialog"
        '/// close FileOpenDialog
        OeffnenDlg.Cancel
    else
        printlog "Shortcut " & vShortcut & " didn't open FileOpen dialog"
    end if
    '/// Close document opened on start
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutGlobalSave
    Dim vShortcut as string
    '/// This testcases checks the global shortcut -> Save
    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 49, 31, 33, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Save' in en/sv/pt-BR is <Strg+S>
            '/// Shortcut for 'Save' in de is <Strg+S>
            '/// Shortcut for 'Save' in fr is <Strg+S>
            '/// Shortcut for 'Save' in it is <Strg+S>
            vShortcut = "<Mod1 S>"
        Case 34
            '/// Shortcut for 'Save' in es is <Strg+G>
            vShortcut = "<Mod1 G>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select
    
    '/// After executing previous named shortcut FileSave dialog should be opened
    Call wTypeKeys ( vShortcut )
    Kontext "SpeichernDlg"
    if SpeichernDlg.Exists then
        printlog "Shortcut " & vShortcut & " opens FileSave dialog"
        '/// close FileOpenDialog
        SpeichernDlg.Cancel
    else
        printlog "Shortcut " & vShortcut & " didn't open FileSave dialog"
    end if
    '/// Close document opened on start
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutGlobalSaveAs
    Dim vShortcut as string
    '/// This testcases checks the global shortcut -> Save

    '/// Open a new text document
    Call hNewDocument
    vShortcut = "<Mod1 Shift S>"
    
    '/// After executing previous named shortcut FileSave dialog should be opened
    Call wTypeKeys ( vShortcut )
    Kontext "SpeichernDlg"
    if SpeichernDlg.Exists then
        printlog "Shortcut " & vShortcut & " opens FileSaveAs dialog"
        '/// close FileOpenDialog
        SpeichernDlg.Cancel
    else
        printlog "Shortcut " & vShortcut & " didn't open FileSaveAs dialog"
    end if
    '/// Close document opened on start
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutSelectAll
    Dim vShortcut as string
    '/// This testcases checks the shortcut -> Select All

    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 49, 31, 33, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Select All' in en/sv/pt-BR is <Strg+A>
            '/// Shortcut for 'Select All' in de is <Strg+A>
            '/// Shortcut for 'Select All' in fr is <Strg+A>
            '/// Shortcut for 'Select All' in it is <Strg+A>
            vShortcut = "<Mod1 A>"
        Case 34
            '/// Shortcut for 'Select All' in es is <Strg+E>
            vShortcut = "<Mod1 E>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select
 
    '/// write down 'The complete text should be selected'
    Call wTypeKeys ( "The complete text should be selected" )
    '/// execute shortcut->the complete text should be selected
    Call wTypeKeys ( vShortcut )
    wait 500
    try
        EditCopy
    catch
        Warnlog "Text seems not to be selected! Edit / Copy is disabled"
        Call hCloseDocument
        goto endsub
    endcatch
    
    if GetClipboardText <> "The complete text should be selected" then
        Warnlog "Not all of the text has been selected with shortcut " & vShortcut
    else
        printlog "All of the text has been selected!"
    end if
    
    '/// close document previously opened
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutFindAndReplace
    Dim vShortcut as string
    '/// This testcases checks the shortcut -> Find & Replace and Repeat Search
    '/// Open a new text document
    Call hNewDocument
    '///  Check shortcut to open 'Find & Replace' dialog
    Select Case iSprache
        Case 01, 03, 07, 49, 31, 33, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Find & Replace' in en/sv/pt-BR is <Strg+F>
            '/// Shortcut for 'Find & Replace' in de is <Strg+F>
            '/// Shortcut for 'Find & Replace' in fr is <Strg+F>
            '/// Shortcut for 'Find & Replace' in it is <Strg+F>
            vShortcut = "<Mod1 F>"
        Case 34
            '/// Shortcut for 'Find & Replace' in es is <Strg+B>
            vShortcut = "<Mod1 B>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select
    
    printlog "Shortcut for 'Find & Replace'"
    '/// write down 'Check Find & Replace'
    Call wTypeKeys ( "Check Find & Replace" )
    '/// execute shortcut->the Find & Replace dialog should come up
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "FindAndReplace"
    if FindAndReplace.Exists then
        printlog "Shortcut " & vShortcut & " opens Find & Replace dialog"
        FindAndReplace.Cancel
    else
        printlog "Shortcut " & vShortcut & " didn't open Find & Replace dialog"
    end if
    
    '///  Check shortcut to Repeat search
    Select Case iSprache
        Case 01, 03, 07, 49, 31, 33, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Repeat Search' in en/sv/pt-BR is <Strg+Shift+F>
            '/// Shortcut for 'Repeat Search' in de is <Strg+Shift+F>
            '/// Shortcut for 'Repeat Search' in fr is <Strg+Shift+F>
            '/// Shortcut for 'Repeat Search' in it is <Strg+Shift+F>
            vShortcut = "<Mod1 Shift F>"
        Case 34
            '/// Shortcut for 'Find & Replace' in es is <Strg+N>
            vShortcut = "<Mod1 B>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select
 
    printlog "Shortcut for 'Repeat Search'"
    '/// execute shortcut->a messagebox should appear asking to start from the beginning
    Call wTypeKeys ( vShortcut )
    wait 500    
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
                printlog "Shortcut " & vShortcut & " seems to work"
            catch
                Active.Ok
                printlog "Shortcut " & vShortcut & " seems not to work correctly"
            endcatch
        end if
    end if
    '/// Quitting messagebox with 'Yes' should bring up another Messagebox
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Ok
        end if
    end if  
    
    '/// close document previously opened
    Call hCloseDocument
    
endcase

' --------------------------------------------------------------------------------

testcase tShortcutBold
    Dim vShortcut as string
    '/// This testcases checks the shortcut -> Bold
    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 31, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Bold' in en/sv/pt-BR is <Strg+B>
            '/// Shortcut for 'Bold' in it is <Strg+B>
            vShortcut = "<Mod1 B>"
        Case 49
            '/// Shortcut for 'Bold' in de is <Strg+Shift+F>
            vShortcut = "<Mod1 Shift F>"
        Case 33
            '/// Shortcut for 'Bold' in fr is <Strg+G>
            vShortcut = "<Mod1 G>"
        Case 34
            '/// Shortcut for 'Bold' in es is <Strg+N>
            vShortcut = "<Mod1 N>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    '/// write down word 'Bold' and select it
    Call wTypeKeys ( "Bold" )
    Call wTypeKeys ( "<Shift Home>" )
    '/// execute shortcut and check if word is now formatted as 'Bold'
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "TextObjectbar"
    if Fett.Getstate(2) = 1 then
        printlog "Shortcut " & vShortcut & " to format 'Bold' works"
    else
        Warnlog "Shortcut " & vShortcut & " to format 'Bold' doesn't work"
    end if
    
    '/// close previous opened document
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutItalic
    Dim vShortcut as string
    '/// This testcases checks the shortcut -> Italic
    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Italic' in en/sv/pt-BR is <Strg+I>
            '/// Shortcut for 'Italic' in fr is <Strg+I>
            '/// Shortcut for 'Italic' in it is <Strg+I>
            vShortcut = "<Mod1 I>"
        Case 49
            '/// Shortcut for 'Italic' in de is <Strg+Shift+K>
            vShortcut = "<Mod1 Shift K>"
        Case 34
            '/// Shortcut for 'Italic' in es is <Strg+K>
            vShortcut = "<Mod1 K>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    '/// write down word 'Italic' and select it
    Call wTypeKeys ( "Italic" )
    Call wTypeKeys ( "<Shift Home>" )
    '/// execute shortcut and check if word is now formatted as 'Italic'
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "TextObjectbar"
    if Kursiv.Getstate(2) = 1 then
        printlog "Shortcut " & vShortcut & " to format 'Italic' works"
    else
        Warnlog "Shortcut " & vShortcut & " to format 'Italic' doesn't work"
    end if
    
    '/// close previous opened document
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutUnderline
    Dim vShortcut as string
    '/// This testcases checks the shortcut -> Underline
    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Underline' in en/sv/pt-BR is <Strg+U>
            '/// Shortcut for 'Underline' in fr is <Strg+U>
            '/// Shortcut for 'Underline' in it is <Strg+U>
            vShortcut = "<Mod1 U>"
        Case 49
            '/// Shortcut for 'Underline' in de is <Strg+Shift+U>
            vShortcut = "<Mod1 Shift U>"
        Case 34
            '/// Shortcut for 'Underline' in es is <Strg+S>
            vShortcut = "<Mod1 S>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    '/// write down word 'Underline' and select it
    Call wTypeKeys ( "Underline" )
    Call wTypeKeys ( "<Shift Home>" )
    '/// execute shortcut and check if word is now formatted as 'Underline'
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "TextObjectbar"
    if Unterstrichen.Getstate(2) = 1 then
        printlog "Shortcut " & vShortcut & " to format 'Underline' works"
    else
        Warnlog "Shortcut " & vShortcut & " to format 'Underline' doesn't work"
    end if
    
    '/// close previous opened document
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutDoubleUnderline
    Dim vShortcut as string
    '/// This testcases checks the shortcut -> Double Underline
    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 34, 36, 39, 49, 46, 55, 86, 82, 81
            '/// Shortcut for 'Underline' in en/sv/pt-BR is <Strg+D>
            '/// Shortcut for 'Underline' in fr is <Strg+D>
            '/// Shortcut for 'Underline' in es is <Strg+D>
            '/// Shortcut for 'Underline' in it is <Strg+D>
            '/// Shortcut for 'Underline' in de is <Strg+D>
            vShortcut = "<Mod1 D>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    '/// write down word 'Underline' and select it
    Call wTypeKeys ( "Double Underline" )
    Call wTypeKeys ( "<Shift Home>" )
    '/// execute shortcut and check if word is now formatted as 'Double Underline'
    Call wTypeKeys ( vShortcut )
    wait 500
    FormatCharacter
    Kontext
    Active.Setpage TabFontEffects
    Kontext "TabFontEffects"
    if Underline.GetSelIndex <> 3 then
        Warnlog "Shortcut " & vShortcut & " to format 'Double Underline' doesn't work"
    else
        printlog "Shortcut " & vShortcut & " to format 'Double Underline' works"
    end if

    TabFontEffects.Cancel
    
    '/// close previous opened document
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutAlign
    Dim vShortcut as string
    '/// This testcases checks the shortcuts -> Align Left, Right, Centered, Justified

    '/// Open a new text document
    printlog "- Shortcut 'Align Left'"
    Call hNewDocument
    '/// "- Shortcut 'Align Left'"
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 34, 36, 39, 49, 46, 55, 86, 82, 81
            '/// Shortcut for 'Align Left' in en/sv/pt-BR is <Strg+L>
            '/// Shortcut for 'Align Left' in fr is <Strg+L>
            '/// Shortcut for 'Align Left' in es is <Strg+L>
            '/// Shortcut for 'Align Left' in it is <Strg+L>
            '/// Shortcut for 'Align Left' in de is <Strg+L>
            vShortcut = "<Mod1 L>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    '/// write down word 'Align Left'
    Call wTypeKeys ( "Align Left" )
    '/// execute shortcut and check if word is now aligned left
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "TextObjectbar"
    if Linksbuendig.Getstate(2) = 1 then
        printlog "-  Shortcut " & vShortcut & " to 'Align Left' works"
    else
        Warnlog "-  Shortcut " & vShortcut & " to 'Align Left' doesn't work"
    end if

    '/// "- Shortcut 'Align Right'"
    printlog "- Shortcut 'Align Right'"
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 34, 36, 39, 49, 46, 55, 86, 82, 81
            '/// Shortcut for 'Align Right' in en/sv/pt-BR is <Strg+R>
            '/// Shortcut for 'Align Right' in fr is <Strg+R>
            '/// Shortcut for 'Align Right' in es is <Strg+R>
            '/// Shortcut for 'Align Right' in it is <Strg+R>
            '/// Shortcut for 'Align Right' in de is <Strg+R>
            vShortcut = "<Mod1 R>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    '/// Insert a line break
    Call wTypeKeys ( "<Return>" )
    '/// write down word 'Align Right'
    Call wTypeKeys ( "Align Right" )
    '/// execute shortcut and check if word is now aligned right
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "TextObjectbar"
    if Rechtsbuendig.Getstate(2) = 1 then
        printlog "-  Shortcut " & vShortcut & " to 'Align Right' works"
    else
        Warnlog "-  Shortcut " & vShortcut & " to 'Align Right' doesn't work"
    end if

    '/// "- Shortcut 'Align Centered'"
    printlog "- Shortcut 'Align Centered'"
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 36, 39, 49, 46, 55, 86, 82, 81
            '/// Shortcut for 'Align Centered' in en/sv/pt-BR is <Strg+E>
            '/// Shortcut for 'Align Centered' in fr is <Strg+E>
            '/// Shortcut for 'Align Centered' in it is <Strg+E>
            '/// Shortcut for 'Align Centered' in de is <Strg+E>
            vShortcut = "<Mod1 E>"
        case 34
            '/// Shortcut for 'Align Centered' in es is <Strg+T>
            vShortcut = "<Mod1 T>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    '/// Insert a line break
    Call wTypeKeys ( "<Return>" )
    '/// write down word 'Align Right'
    Call wTypeKeys ( "Align Centered" )
    '/// execute shortcut and check if word is now aligned centered
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "TextObjectbar"
    if Zentriert.Getstate(2) = 1 then
        printlog "-  Shortcut " & vShortcut & " to 'Align Centered' works"
    else
        Warnlog "-  Shortcut " & vShortcut & " to 'Align Centered' doesn't work"
    end if

    '/// "- Shortcut 'Align Justified'"
    printlog "- Shortcut 'Align Justified'"
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 34, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Align Justified' in en/sv/pt-BR is <Strg+J>
            '/// Shortcut for 'Align Justified' in fr is <Strg+J>
            '/// Shortcut for 'Align Justified' in it is <Strg+J>
            '/// Shortcut for 'Align Justified' in de is <Strg+J>
            vShortcut = "<Mod1 J>"
        case 49
            '/// Shortcut for 'Align Justified' in de is <Strg+B>
            vShortcut = "<Mod1 B>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    '/// Insert a line break
    Call wTypeKeys ( "<Return>" )
    '/// write down word 'Align Right'
    Call wTypeKeys ( "Align Justified" )
    '/// execute shortcut and check if word is now aligned centered
    Call wTypeKeys ( vShortcut )
    wait 500
    Kontext "TextObjectbar"
    if Blocksatz.Getstate(2) = 1 then
        printlog "-  Shortcut " & vShortcut & " to 'Align Justified' works"
    else
        Warnlog "-  Shortcut " & vShortcut & " to 'Align Justified' doesn't work"
    end if

    '/// close previous opened document
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutSuperscript
    Dim vShortcut as string
    '/// This testcases checks the shortcut -> Superscript
    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 34, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Superscript' in en/sv/pt-BR is <Strg+D>
            '/// Shortcut for 'Superscript' in fr is <Strg+D>
            '/// Shortcut for 'Superscript' in es is <Strg+D>
            '/// Shortcut for 'Superscript' in it is <Strg+D>
            vShortcut = "<Mod1 Shift P>"
        Case 49
            '/// Shortcut for 'Superscript' in de is <Strg+H>
            vShortcut = "<Mod1 H>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select

    Kontext "DrawBar"
    if Not DrawBar.Exists then Call hToolbarSelect("Drawing", true)
    '/// Open 'Draw functions' toolbar ///
    Textobjekt.Click
    '/// Draw a Textobject ///
    Call gMouseMove ( 30, 30, 40, 40 )
    '/// write down word 'Superscript' and select it
    call wTypeKeys ("Superscript")
    call wTypeKeys ("<Shift Home>")

    '/// execute shortcut and check if word is now formatted as 'Superscript'
    Call wTypeKeys ( vShortcut )
    wait 500
    FormatCharacter
    Kontext
    Active.Setpage TabFontPosition
    Kontext "TabFontPosition"
    if Superscript.IsChecked = true then
        printlog "Shortcut " & vShortcut & " to format 'Superscript' works"
    else
        Warnlog "Shortcut " & vShortcut & " to format 'Superscript' doesn't work"
    end if
    TabFontPosition.Cancel

    '/// close previous opened document
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutSubscript
    Dim vShortcut as string
    '/// This testcases checks the shortcut -> Subscript
    '/// Open a new text document
    Call hNewDocument
    Select Case iSprache
        Case 01, 03, 07, 31, 33, 34, 36, 39, 46, 55, 86, 82, 81
            '/// Shortcut for 'Subscript' in en/sv/pt-BR is <Strg+Shift+B>
            '/// Shortcut for 'Subscript' in fr is <Strg+Shift+B>
            '/// Shortcut for 'Subscript' in es is <Strg+Shift+B>
            '/// Shortcut for 'Subscript' in it is <Strg+Shift+B>
            vShortcut = "<Mod1 Shift B>"
        Case 49    
            '/// Shortcut for 'Subscript' in de is <Strg+T>
            vShortcut = "<Mod1 T>"
        Case else
            Warnlog "Shortcuts for language: " & iSprache & " are not defined in this test"
            Call hCloseDocument
            goto endsub
    end select
    
    Kontext "DrawBar"
    if Not DrawBar.Exists then Call hToolbarSelect("Drawing", true)
    '/// Open 'Draw functions' toolbar ///
    Textobjekt.Click
    '/// Draw a Textobject ///
    Call gMouseMove ( 30, 30, 40, 40 )
    '/// write down word 'Subscript' and select it

    call wTypeKeys ("Subscript")
    call wTypeKeys ("<Shift Home>")

    '/// execute shortcut and check if word is now formatted as 'Superscript'
    Call wTypeKeys ( vShortcut )
    wait 500
    FormatCharacter
    Kontext
    Active.Setpage TabFontPosition
    Kontext "TabFontPosition"
    if Subscript.IsChecked = true then
        printlog "Shortcut " & vShortcut & " to format 'Subscript' works"
    else
        Warnlog "Shortcut " & vShortcut & " to format 'Subscript' doesn't work"
    end if
    TabFontPosition.Cancel

    '/// close previous opened document
    Call hCloseDocument

endcase

' --------------------------------------------------------------------------------

testcase tShortcutPasteUnformattedText

	Dim vShortcut as string
	Dim DefaultSize as integer
	Dim DefaultStyle as integer
	
	printlog "This testcases checks the shortcut -> Paste Unformatted Text"
	printlog "Open a new text document"
	Call hNewDocument
	
	vShortcut = "<Mod1 Mod2 Shift V>"
	printlog "Type 'This is a formatted sentence'"
	Call wTypeKeys ("This is a formatted sentence")
	
	printlog "Format the word 'formatted' in bold, italic, font size 20"
	Call wTypeKeys ("<Home>")
	Call wTypeKeys ("<Mod1 Right>", 3)
	Call wTypeKeys ("<Mod1 Shift Right>")	
	
	FormatCharacter
	WaitSlot (5)
	Kontext	
	Active.Setpage TabFont
	Kontext "TabFont"
    wait 500
	DefaultSize = Size.GetSelIndex
	Size.Select 14 ' Size 20
	DefaultStyle = Style.GetSelIndex
	Style.Select 4 ' Style Bold, Italic
	TabFont.Ok
	
	printlog "Copy word 'formatted'"
	EditCopy		
	
	printlog "Go to the end of line"
	Call wTypeKeys ("<End>")
	printlog "insert a space"
	Call wTypeKeys ("<Space>")
	printlog "Press Ctrl+Alt+Shift+V (cmd+option+shift+V on Mac)"
    Call wTypeKeys ( vShortcut )
    wait 500
	
	Call wTypeKeys ("<Mod1 Home>")
	printlog "check if sentence is now: This is a formatted sentence formatted"
	Call wTypeKeys ("<Shift End>")
	EditCopy
	if Trim(GetClipboardText) = "This is a formatted sentence formatted" then
		printlog "- shortcut did work"
	else
		if gPlatgroup = "osx" then
			Warnlog "It is not possible to customize the keyboard combination cmd+option on MAC OSX."
			Call hCloseDocument
			goto endsub
		else
			Warnlog "Shortcut did not work"
			Call hCloseDocument
			goto endsub			
		end if
	end if	
	
	printlog "Check if formatted word held its formatting"
	Call wTypeKeys ("<Home>")
	Call wTypeKeys ("<Mod1 Right>", 3)
	Call wTypeKeys ("<Mod1 Shift Right>")
	
	FormatCharacter
	WaitSlot (5)
	Kontext	
	Active.Setpage TabFont
	Kontext "TabFont"
    wait 500
	if Size.GetSelIndex <> 14 then
		Warnlog "formatted word has lost its size"
	endif
	if Style.GetSelIndex <> 4 then
		Warnlog "formatted word has lost its style"
	endif
	TabFont.Cancel
	
	printlog "Check if pasted word has default formatting"
	Call wTypeKeys ("<End>")
	Call wTypeKeys ("<Mod1 Right>", 3)
	Call wTypeKeys ("<Mod1 Shift Left>")
	
	FormatCharacter
	WaitSlot (5)
	Kontext	
	Active.Setpage TabFont
	Kontext "TabFont"
    wait 500
	if Size.GetSelIndex <>  DefaultSize then
		Warnlog "formatted word has no default size"
	endif
	if Style.GetSelIndex <> DefaultStyle then
		Warnlog "formatted word has no default style"
	endif
	TabFont.Cancel
		
    printlog "close previous opened document"
    Call hCloseDocument

endcase
