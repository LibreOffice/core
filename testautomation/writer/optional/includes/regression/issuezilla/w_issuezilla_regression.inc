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
'* short description : Avoid regression (issuezilla tasks)
'*
'\***********************************************************************

sub w_issuezilla_regression

    Call i64991
    Call i65094
    Call i69045
    Call i69521
    Call i75027
    Call i76637
    Call i88656
	Call i103265

end sub

'--------------------------------------------------------------------------

testcase i64991
    Dim PrevMacroSetting as integer
    Dim sDocName as string, i as integer, j as integer

    sDocName = ConvertPath(gtesttoolpath & "writer\optional\input\regression\issuezilla\i64991.odt")
    '/// New writer document
    Call hNewDocument
    '/// Set macro security to 'Medium' to activate Macros
    PrevMacroSetting = hSetMacroSecurity(1)
    '/// Open file \\writer\\optional\\input\\regression\\issuezilla\\i64991.odt
    printlog "- Open file: " & sDocName 
    call hFileOpen (sDocName,false)
    Kontext "SecurityWarning"
    if SecurityWarning.Exists(2) then
        SecurityWarning.OK
    else
        Warnlog "Security Warning not up. Test interrupted!"
        Call hCloseDocument
        goto endsub
    end if
    
	printlog "Save document to be able to find macro module later"    
    Call hFileSaveAsKill (gOfficepath + "user\work\i64991.odt")
	
	printlog "- execute macro"
    ToolsMacrosRunMacro
    ' Sometimes it lasts long until dialog is shown
    For i = 1 to 15
        Sleep(i)
        Kontext "ScriptSelector"
        if ScriptSelector.Exists then exit for
    next i
    printlog " - select 'i64991' in Library list"
    Kontext "ScriptSelector"
    For i = 1 to LibraryTreeList.GetItemCount
        LibraryTreeList.Select i
        if InStr(LibraryTreeList.GetSelText,"i64991") then
            printlog " - open node 'i64991' in Library list"
            i = LibraryTreeList.GetItemCount + 1
            LibraryTreeList.TypeKeys "+"
            wait 500
            printlog " - select 'Standard' in Library list"
            LibraryTreeList.TypeKeys "<Down>"            
            wait 500
            printlog " - open node 'Standard' in Library list"
            LibraryTreeList.TypeKeys "+"
            wait 500
            LibraryTreeList.TypeKeys "<Down>"
            printlog " - select 'check_page_style' in Library list"
            if LibraryTreeList.GetSelText <> "check_page_style" then
                Warnlog "Unable to find macro. Test interrupted!"
                ScriptSelector.Cancel
                Call hCloseDocument
                goto endsub
            end if
            printlog "- Now select script 'Main'"
            for j = 1 to ScriptList.GetItemCount
                ScriptList.Select j
                if ScriptList.GetSelText = "Main" then
                    printlog " - execute Script 'Main'"
                    ScriptSelector.Ok
                    exit for
                end if
            next j
        end if
    next i
    Kontext "ScriptSelector"
    if ScriptSelector.Exists then
        Warnlog "Something failed executing macro. Test interrupted"
        ScriptSelector.Cancel
    else
        '/// check if page layout on first page is landscape
        printlog "Check layout on first page"
        FormatPageWriter
        Kontext
        Active.Setpage TabSeite
        Kontext "TabSeite"
        if Not Quer.IsChecked then Warnlog "Setting a printer make the page styles to be lost"
        TabSeite.Cancel

        Call wTypeKeys ("<Down>")

        '/// check if page layout on second page is still portrait
        printlog "Check layout on second page"
        FormatPageWriter
        Kontext
        Active.Setpage TabSeite
        Kontext "TabSeite"
        if Not Hoch.IsChecked then Warnlog "Setting a printer make the page styles to be lost"
        TabSeite.Cancel
    end if

    '/// Set macro setting to previous value
    Call hSetMacroSecurity(PrevMacroSetting)

    '/// Close document
    Call hCloseDocument

endcase
'--------------------------------------------------------------------------

testcase i65094

    '/// New writer document
    Call hNewDocument
    '/// Open file \\writer\\optional\\input\\regression\\issuezilla\\i65094a.odt
    printlog "- Open file: i65094a.odt" 
    hFileOpenLocally(  gTesttoolPath + "writer\optional\input\regression\issuezilla\i65094a.odt" )
    
    '/// Compare document with other version \\writer\\optional\\input\\regression\\issuezilla\\i65094b.odt
'    UseBindings
    printlog "- Compare document with other version"
    EditCompareDocument
    Kontext "OeffnenDlg"
    '/// Open document ( i65094b.odt )
    Dateiname.SetText ConvertPath( gTesttoolPath + "writer\optional\input\regression\issuezilla\i65094b.odt" )
'    Sleep 2
    Oeffnen.Click
    Sleep 5
    
    printlog "- Accept all"
    '/// In Redlining dialog select 'Accept all'
    Kontext "Redlining"
    if Redlining.exists then
        try
            AlleAkzeptieren.Click
            Sleep 1
        catch
            warnlog "accept button is disabled"
        endcatch
        '/// Close Redlining dialog
        Redlining.Close
        '/// Save document as "..\user\work\i65094compare.odt
        Call hFileSaveAsKill (gOfficepath + "user\work\i65094compare.odt")
        '/// Save document as "..\user\work\i65094compare.odt" 
        Wait 500
        printlog "- Compare done and document saved"
    else
        warnlog "Redlining dialog does not exist"
    end if
    Call hCloseDocument
    
endcase
'--------------------------------------------------------------------------

testcase i69045

    '/// New writer document
    Call hNewDocument
    printlog "- Open new dokument, enable redlining and make some changes"
    '/// Enable redlining
    EditChangesRecord
    '/// Change document
    Call wTypeKeys "Here are some changes"
    '/// Save document as "..\user\work\i65094compare.odt and close it
    Call hFileSaveAsKill (gOfficepath & "user\work\i69045.odt")
    Call hCloseDocument
    printlog "- Save, close and load document"
    '/// Open saved document
    hFileOpen( gOfficepath & "user\work\i69045.odt" )
    '/// Check 
    try
        EditChangesAcceptOrReject
    catch
        warnlog "Redlining has not been saved or does not work"
    endcatch
    Kontext "Redlining"
    if Redlining.exists then
        try
            AlleAkzeptieren.Click
            Sleep 1
            printlog "- Edit changes has been saved and accepted"
        catch
            warnlog "AcceptAll is not enabled"
        endcatch    
        '/// Close Redlining dialog
        Redlining.Close
    end if
    Call hCloseDocument

endcase
'--------------------------------------------------------------------------

testcase i69521

    Dim iItems as integer

    '/// New writer document
    Call hNewDocument
    '/// Load document: \\writer\\optional\\input\\regression\\issuezilla\\i69521.odt
    hFileOpenLocally( gTesttoolpath & "writer\optional\input\regression\issuezilla\i69521.odt")
    
    '/// Loaded document should have 5 EPS Graphics
    iItems = CountItemsInNavigatorGroup(4)
    if iItems <> 5 then
        Warnlog "Initially there are not 5 Grahics in document but: " & iItems
        Call hCloseDocument
        goto endsub
    end if

    '/// save document as OfficePath\\user\work\\i96521-1.odt"
    Call hFileSaveAsKill (gOfficePath & "user\work\i96521-1.odt")
    '/// close document
    Call hCloseDocument
    '/// reload previous saved document
    hFileOpen( gOfficePath & "user\work\i96521-1.odt")
    '/// Check if all 5 Graphics are still in document
    iItems = CountItemsInNavigatorGroup(4)
    if iItems <> 5 then
        Warnlog "After save and reload there are not 5 Grahics in document but: " & iItems
    end if

    Call hCloseDocument

endcase
'--------------------------------------------------------------------------

testcase i75027

    Dim i as integer
    Dim j as integer
    Dim sClipBrd() as string
    Dim bNumberingError as boolean

    gApplication   = "HTML"
    '/// New HTML Document
    Call hNewDocument
    '/// type 'This<BR>is<BR>a numbering<BR>test'
    Call wTypeKeys ("This<Return>is<Return>a numbering<Return>test")
    '/// Select all paragraphs
    Call wTypeKeys ("<Mod1 A>")     
    '/// Format with simple numbering
    Kontext "TextObjectbar"
    if Not TextObjectbar.Exists then Call hToolbarSelect ( "FORMATTING", true )
    '/// Select 'Numbering' in object bar ///
    Numerierung.Click        
    '/// move "is" to numbering level 2
    '/// move "a numbering" to numbering level 3
    '/// move "test" to numbering level 4  
    printlog "- create numbering"
    Call wTypeKeys ("<Mod1 Home>"    
    for i = 1 to 3    
        Call wTypeKeys ("<Down>")
        Kontext "NumObjectbar"
        if Not NumObjectbar.Exists then Call hToolbarSelect("BULLETSANDNUMBERING", true)
        for j = 1 to 1 + i
            Hinunterstufen.Click
        next j
    next i
    '/// should look like:
    '/// 1. this
    '///    1. is 
    '///        1. a numbering
    '///            1. test
    printlog "- check numbering"
    Call wTypeKeys ( "<MOD1 A>" )
    EditCopy
    Select Case gPlatGroup
        Case "unx"
            sClipBrd() = Split (GetClipboardText, CHR$(10))
        Case else
            sClipBrd() = Split (GetClipboardText, CHR$(13) & CHR$(10))
    end select
    if Ubound(sClipBrd()) = 3 then
        if Left$(sClipBrd(0), 2) <> "1." then bNumberingError = true
        if Left$(sClipBrd(1), 2) <> "1." then bNumberingError = true
        if Left$(sClipBrd(2), 2) <> "1." then bNumberingError = true
        if Left$(sClipBrd(3), 2) <> "1." then bNumberingError = true
    else
        Warnlog "Error in clipboard content! " & Ubound(sClipBrd())
    end if
    if bNumberingError = true then
        Warnlog "Seems numbering I. has not been correctly created"
    end if
    bNumberingError = false
    '/// save as HTML-file i75027.html
    Call hFileSaveAsKill (gOfficepath & "user\work\i75027.html")    
    '/// Reload previous saved document
    Call hCloseDocument
    hFileOpen( gOfficepath & "user\work\i75027.html")
    '/// Check that the numbering is still the same.
    printlog "- check numbering (previous saved)"
    Call wTypeKeys ( "<MOD1 A>" )
    EditCopy
    Select Case gPlatGroup
        Case "unx"
            sClipBrd() = Split (GetClipboardText, CHR$(10))
        Case else
            sClipBrd() = Split (GetClipboardText, CHR$(13) & CHR$(10))
    end select
    if Ubound(sClipBrd()) = 3 then
        if Left$(sClipBrd(0), 2) <> "1." then bNumberingError = true
        if Left$(sClipBrd(1), 2) <> "1." then bNumberingError = true
        if Left$(sClipBrd(2), 2) <> "1." then bNumberingError = true
        if Left$(sClipBrd(3), 2) <> "1." then bNumberingError = true
    else
        Warnlog "Error in clipboard content! " & Ubound(sClipBrd())
    end if
    if bNumberingError = true then
        Warnlog "Seems numbering I. has not been correctly created"
    end if    
    '/// Close Document
    Call hCloseDocument
    gApplication   = "WRITER"

endcase
'--------------------------------------------------------------------------

testcase i76637

    'Variables:
    DIM sDocument           AS String 'variable for saving the doc

    gApplication   = "WRITER"
    
    '/// open a swriter
    Call hNewDocument 'NewDocument
    Kontext "DocumentWriter"
    printlog "New writer doc"
    
    '/// [Insert]-[Envelope]
    InsertEnvelope
    printlog "inserting envelope.."
    
    '/// click "New Doc" button in Envelope_Dlg
    Kontext
    active.SetPage TabEnvelope 'setting correct Tabpage
    Kontext "TabEnvelope"
    TabEnvelope.OK 'clicking on NewDoc button, which is OK button with another name
    
    '/// Save document
    sDocument = ConvertPath ( gOfficePath + "user\work\test1.odt")
    'deleting old Docs in the directory
    if Dir (sDocument) <> "" then kill sDocument 'deleting old docs
    Call hFileSaveAsKill (sDocument)
    printlog "Document saved"
    
    '/// click [File]-[Reload]
    Kontext "DocumentWriter"
    FileReload
    printlog "reloading ..here the crash happened in i76637."
    '///crash occurred here

    Call hCloseDocument
    Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase i88656
    printlog "Test Format - AutoFormat - Apply And Edit Changes."

    printlog " Open a new writerdocument."
    Call hNewDocument
    printlog "Write the forrlowing text into the document: This is a test-text to be autoformatted."
    wTypeKeys ("This is a test-text to be autoformatted.")

    'Without text selected.
    printlog "Use the menu: Format - Autoformat - Apply and Edit Changes."
    FormatAutoformatApplyAndEditChanges

    kontext "active"
    if active.exists(2) then
       printlog " In the dialog: choose 'Accept."
       active.Ok
    end if

    printlog "Press SHIFT + HOME. To select the text we just wrote."
    wTypeKeys ("<SHIFT HOME>")

    'with text selected
    printlog "Again, but with the text selected; use the menu: Format - Autoformat - Apply and Edit Changes."
    FormatAutoformatApplyAndEditChanges

    kontext "active"
    if active.exists(3) then
       printlog " In the dialog: choose 'no'."
       active.Cancel
    end if

    kontext "active"
    if active.exists(3) then
       printlog " In the dialog: choose 'no'."
       active.No
    end if

    Call hCloseDocument
endcase

'--------------------------------------------------------------------------

testcase i103265

	Dim i as integer

	printlog "ODF Hyperlinks in TOCs"
	Call hNewDocument
    printlog "- Open test document"
	hFileOpenLocally(  gTesttoolPath + "writer\optional\input\regression\issuezilla\i103265.odt" )
    
    printlog "- Update TOC"
	ToolsUpdateAllIndexes
	
	printlog "Check that every hyperlink jumps correctly to its target"
	For i = 1 to 3
		' point cursor to beginning of document
		Call wTypeKeys "<MOD1 HOME>"
		' point cursor to index entry
		Call wTypeKeys "<DOWN>", 4 + i
		'execute Hyperlink
		ContextOpenHyperlink
		' check if it jumps to correct target
		Call wTypeKeys "<SHIFT END>"
		EditCopy
		Select Case i
			Case 1:
				if GetClipboardText <> "# 1 Test" then
					Warnlog "First hyperlink not correctly jumped to. Not '# 1 Test' but " & GetClipboardtext
				end if
			Case 2:
				if GetClipboardText <> "1. Test" then
					Warnlog "Second hyperlink not correctly jumped to. Not '# 1. Test' but " & GetClipboardtext
				end if
			Case 3:
				if GetClipboardText <> "1 Test " then
					Warnlog "Third hyperlink not correctly jumped to. Not '1 Test' but " & GetClipboardtext
				end if
		end select
	next i

	printlog "- Save as HTML"
	gApplication = "HTML"	
	Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\ODFHyperlinksInTOCs.html", "HTML (StarWriter)" )
    Call hCloseDocument
	
    printlog "- Reload"
	hFileOpen( gOfficePath & "user\work\ODFHyperlinksInTOCs.html" )
	printlog "Check that every hyperlink jumps correctly to its target"
	For i = 1 to 3
		' point cursor to beginning of document
		Call wTypeKeys "<MOD1 HOME>"
		' point cursor to index entry
		Call wTypeKeys "<DOWN>", 5 + i
		'execute Hyperlink
		ContextOpenHyperlink
		' check if it jumps to correct target
		Call wTypeKeys "<SHIFT END>"
		EditCopy
		Select Case i
			Case 1:
				if GetClipboardText <> "# 1 Test" then
					Warnlog "First hyperlink not correctly jumped to. Not '# 1 Test' but " & GetClipboardtext
				end if
			Case 2:
				if GetClipboardText <> "1. Test" then
					Warnlog "First hyperlink not correctly jumped to. Not '# 1. Test' but " & GetClipboardtext
				end if
			Case 3:
				if GetClipboardText <> "1Test " then
					Warnlog "First hyperlink not correctly jumped to. Not 'Test ' but " & GetClipboardtext
				end if
		end select
	next i
	
	Call hCloseDocument

endcase