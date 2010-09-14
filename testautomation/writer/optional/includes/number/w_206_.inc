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
'* short description : Testing Number and Bullets
'*
'\***********************************************************************

sub w_206_

	Call tToolsNumbering1
    Call tToolsNumbering2
    Call tToolsNumbering3
    Call tToolsNumbering4
    Call tToolsNumbering5
    Call tToolsNumbering6
    Call tToolsNumbering7
    Call tToolsNumbering8
    Call tToolsNumbering9
    Call tToolsNumbering10
    Call tToolsNumbering11

end sub

testcase tToolsNumbering1
    printlog "Create a simple numbering without selection"
	'/// Create a simple numbering without selection
	Dim i as integer

	'/// Open a new text document
	Call hNewDocument

	'/// Activate Numbering/Bullets via Objectbar
	Call hToolbarSelect("BulletsAndNumbering", true)

	'/// Create a numbering
	Kontext "NumObjectbar"
	if Not NumObjectbar.Exists then
		Warnlog "Numbering/Bullets objectbar not accessible"
		Call hCloseDocument
		goto endsub
	end if

	NumberingOnOff.Click
	For i = 1 to 5
		call wTypeKeys ( "Numbering Level " & i )
		call wTypeKeys ( "<RETURN>" )
	next i

	'/// Check if numbering has been created
	For i = 1 to 5
		call wTypeKeys ( "<UP>" )
		if hIsNumberingBullets() = false then
			Warnlog "Seems numbering has not been completely created"
		end if
	next i

	'/// CloseDocument
	Call hCloseDocument

endcase

testcase tToolsNumbering2
	printlog "Create simple bullets without selection"
    '/// Create simple bullets without selection	
    Dim i as integer

	'/// Open a new text document
	Call hNewDocument
	
	'/// Activate Numbering/Bullets via Objectbar
	Call hToolbarSelect("BulletsAndNumbering", true)
	
	'/// Create bullets
	Kontext "NumObjectbar"
	if Not NumObjectbar.Exists then
		Warnlog "Numbering/Bullets objectbar not accessible"
		Call hCloseDocument
		goto endsub
	end if
		
	BulletsOnOff.Click
	For i = 1 to 5
		call wTypeKeys ( "Numbering Level " & i )
		call wTypeKeys ( "<RETURN>" )
	next i

	'/// Check if numbering has been created
	For i = 1 to 5
		call wTypeKeys ( "<UP>" )
		if hIsNumberingBullets() = false then
			Warnlog "Seems numbering has not been completely created"
		end if
	next i

	'/// CloseDocument
	Call hCloseDocument
	
endcase


testcase tToolsNumbering3
    printlog "Turn several selected paragraphs into numbering"	
    '/// Turn several selected paragraphs into numbering	
	Dim i as integer

	'/// Open a new text document  
	Call hNewDocument

	'/// Insert autotext 'Dummy text' 3 times
	for i = 1 to 3
		Call wBlindtextEinfuegen
	next i
	
	'///  Point cursor to beginning of document
	call wTypeKeys ( "<Mod1 Home>" )
	'/// Select all text inserted
	call wTypeKeys ( "<Mod1 A>" )
		
	'/// Activate Numbering/Bullets via Objectbar
	Call hToolbarSelect("BulletsAndNumbering", true)
	
	Kontext "NumObjectbar"
	if Not NumObjectbar.Exists then
		Warnlog "Numbering/Bullets objectbar not accessible"
		Call hCloseDocument
		goto endsub
	end if
		
	NumberingOnOff.Click
	'/// Check if numbering has been created
	call wTypeKeys ( "<Mod1 Home>" )	
	
	For i = 1 to 2
		if hIsNumberingBullets() = false then
			Warnlog "Seems numbering has not been completely created"
		end if
		call wTypeKeys ( "<Mod1 Down>" )
	next i

	'/// CloseDocument
	Call hCloseDocument
	
endcase


testcase tToolsNumbering4
	printlog "Turn several selected paragraphs into bullets"
    '/// Turn several selected paragraphs into bullets	
	Dim i as integer

	'/// Open a new text document  
	Call hNewDocument

	'/// Insert autotext 'Dummy text' 3 times
	for i = 1 to 3
		Call wBlindtextEinfuegen
	next i
	
	'///  Point cursor to beginning of document
	call wTypeKeys ( "<Mod1 Home>" )
	'/// Select all text inserted
	call wTypeKeys ( "<Mod1 A>" )
		
	'/// Activate Numbering/Bullets via Objectbar
	Call hToolbarSelect("BulletsAndNumbering", true)
	
	Kontext "NumObjectbar"
	if Not NumObjectbar.Exists then
		Warnlog "Numbering/Bullets objectbar not accessible"
		Call hCloseDocument
		goto endsub
	end if
		
	BulletsOnOff.Click
	'/// Check if numbering has been created
	call wTypeKeys ( "<Mod1 Home>" )	
	
	For i = 1 to 2
		if hIsNumberingBullets() = false then
			Warnlog "Seems bullets have not been completely created"
		end if
		call wTypeKeys ( "<Mod1 Down>" )
	next i

	'/// CloseDocument
	Call hCloseDocument
	
endcase


testcase tToolsNumbering5
    printlog "create numbering via autoformat"
    '/// create numbering via autoformat
	Dim i as integer, sClipBrd() as string

	For i = 1 to 8
		'/// Open a new text document
		Call hNewDocument
		
		'/// Insert '1.' // 'I.' // 'a.' followed by text
		Select Case i
			Case 1
				printlog "- Enter numbering starting with 1)"
				call wTypeKeys ( "1) " )
			Case 2
				call wTypeKeys ( "1. " )
				printlog "- Enter numbering starting with 1."
			Case 3
				call wTypeKeys ( "(1) " )
				printlog "- Enter numbering starting with (1)"
			Case 4
				call wTypeKeys ( "I. " )
				printlog "- Enter numbering starting with I."
			Case 5
				call wTypeKeys ( "i. " )
				printlog "- Enter numbering starting with i."
			Case 6
                call wTypeKeys ( "A) " )
				printlog "- Enter numbering starting with A)"
			Case 7
				call wTypeKeys ( "a) " )
				printlog "- Enter numbering starting with a)"
			Case 8
				call wTypeKeys ( "(a) " )
				printlog "- Enter numbering starting with (a)"
		end select
		
		'/// + 'my new numbering'
		'/// + followed by return (creats the numbering)
		'/// + followed by text 'my newer numbering'
		call wTypeKeys "my new numbering<Return>"
		call wTypeKeys "my newer numbering"
		
		'/// check if numbering has been created
		call wTypeKeys ( "<MOD1 A>" )
		EditCopy
		Select Case gPlatGroup
			Case "unx"
				sClipBrd() = Split (GetClipboardText, CHR$(10))
			Case else
				sClipBrd() = Split (GetClipboardText, CHR$(13) & CHR$(10))
		end select
		if Ubound(sClipBrd()) = 1 then
			Select Case i
				Case 1
					if Left$(sClipBrd(1), 2) <> "2)" then
						Warnlog "Seems numbering 1) has not been correctly created"
					end if
				Case 2
					if Left$(sClipBrd(1), 2) <> "2." then
						Warnlog "Seems numbering 1. has not been correctly created"
					end if
				Case 3
					if Left$(sClipBrd(1), 3) <> "(2)" then
						Warnlog "Seems numbering (1) has not been correctly created"
					end if
				Case 4
					if Left$(sClipBrd(1), 3) <> "II." then
						Warnlog "Seems numbering I. has not been correctly created"
					end if
				Case 5
					if Left$(sClipBrd(1), 3) <> "ii." then
                        QaErrorLog "#i78524#Auto-capitalisation of letter i hinders Automatic numbering"    'Warnlog "Seems numbering i. has not been correctly created"
					end if
				Case 6
                    if Left$(sClipBrd(1), 2) <> "B)" then
					    QaErrorLog "#i71136# - Autocorrect won't start numbering by 'A)'"   'warnlog "Seems numbering A) has not been correctly created"
					end if
				Case 7
                    if Left$(sClipBrd(1), 2) <> "b)" then
						QaErrorLog "#i71136# - Autocorrect won't start numbering by 'a)'"   'warnlog "Seems numbering a) has not been correctly created"
					end if
				Case 8
					if Left$(sClipBrd(1), 2) <> "(b)" then
						QaErrorLog "#i71136# - Autocorrect won't start numbering by '(a)'"   'warnlog "Seems numbering (a) has not been correctly created"
					end if
			end select
		else
			Warnlog "Error in clipboard content! " & Ubound(sClipBrd())
		end if
	
		'/// CloseDocument
		Call hCloseDocument
	next i		
	
endcase


testcase tToolsNumbering6
	printlog "Remove numbering from paragraph by pressing <DEL> key (normal numbering)"
	Dim j as integer
	Dim i as integer, sClipBrd() as string
	
	'/// Remove numbering from paragraph by pressing "DEL" key (normal numbering)
	'/// Open a new text document
	Call hNewDocument
	'/// Insert some text '1. This is the first' followed by a return
	call wTypeKeys ( "1. This is the first<Return>" )
	'/// Insert some text 'This is the second' followed by a return
	call wTypeKeys ( "This is the second<Return>" )
	'/// Insert some text 'This is the third'
	call wTypeKeys ( "This is the third" )
	
	'/// Press key <STRG HOME> to go to first
	call wTypeKeys ( "<MOD1 Home>" )
	'/// Press <HOME> to highlight numbering
	call wTypeKeys ( "<Home>" )
	'/// Press <DELETE> to remove first paragraph numbering
	call wTypeKeys ( "<DELETE>" )
	'/// Check if numbering of the following paragraphs has been adapted even after save and reload
	For j = 1 to 2 
		if j = 2 then
			'/// Save document
			Call hFileSaveAsKill ( gOfficepath & "user\work\tToolsNumbering6.odt" )
			'/// Close and reload document
			Call hCloseDocument
			hFileOpen( gOfficepath & "user\work\tToolsNumbering6.odt" )
		end if
		call wTypeKeys ( "<MOD1 A>" )	
		EditCopy
		Select Case gPlatGroup
			Case "unx"
				sClipBrd() = Split (GetClipboardText, CHR$(10))
			Case else
				sClipBrd() = Split (GetClipboardText, CHR$(13) & CHR$(10))
		end select
		
		For i = 1 to 3
			Select Case i	
				Case 1
					if sClipBrd(0) <> "This is the first" then
						Warnlog "First paragraph not 'This is the first' but: " & sClipBrd(0)
					end if
				Case 2
					if sClipBrd(1) <> "1. This is the second" then
						Warnlog "First paragraph not '1.This is the second' but: " & sClipBrd(1)
					end if		
				Case 3
					if sClipBrd(2) <> "2. This is the third" then
						Warnlog "First paragraph not '2.This is the third' but: " & sClipBrd(2)
					end if
			end select
		next i	
	next j
	'/// Close document
	Call hCloseDocument
	
endcase


testcase tToolsNumbering7
    printlog "Remove numbering from paragraph by pressing <DEL> key (outlined numbering)"
	Dim j as integer, sVorlage as string
	Dim i as integer, sClipBrd() as string		
	'/// Remove numbering from paragraph by pressing "DEL" key (outlined numbering)
	'/// Open a new text document
	Call hNewDocument
	'/// Tools / Outline Numbering
	ToolsOutlineNumbering
	Kontext
	Active.SetPage TabKapitelnumerierung
	Kontext "TabKapitelnumerierung"
	sVorlage = Absatzvorlage.GetSelText
	'/// Select as Numbering Number 1 in list
	Nummer.Select 2
	'///  Set as Seperator (After) a `.`
	Dahinter.Settext "."
	'/// Close 'Outline Numbering' dialog
	TabKapitelnumerierung.Ok

    ' Numbering of outlines will not be copied to clipboard
    ' so we use the Field 'Chapter' which does
    for i = 1 to 3
        InsertFieldsOther
        Kontext
        Active.Setpage TabDokumentFeldbefehle
        Kontext "TabDokumentFeldbefehle"
        try
            Feldtyp.Select fGetType("Chapter")
            wait 500
            Select Case iSprache
                Case 7      :Formatliste.Select 3    'Chapternumber
                Case 34     :Formatliste.Select 3    'Chapternumber
                Case else   :Formatliste.Select 2    'Chapternumber
            end select
        catch
            Warnlog "Unable to select field 'chapter'"
            TabDokumentFeldbefehle.Close
            Call hCloseDocument
            goto endsub
        endcatch    
        Einfuegen.Click
        TabDokumentFeldbefehle.Close
        
        Select Case i
            Case 1
                '/// Insert some text 'My first number' followed by a return
                call wTypeKeys ( "My first number<RETURN>" )
            Case 2
                '/// Insert some text 'My second number' followed by a return
                call wTypeKeys ( "My second number<RETURN>" )
            Case 3            
                '/// Insert some text 'My third number'
                call wTypeKeys ( "My third number" )
        end select
    next i
	'/// Select all text
    call wTypeKeys ( "<MOD1 A>" )
    '/// Set Style 'Heading 1'
	Call wStyleSet ( sVorlage )	
	
	'/// Press key <STRG HOME> to go to first
	call wTypeKeys ( "<MOD1 Home>" )
	'/// Press <HOME> to highlight numbering
	call wTypeKeys ( "<Home>" )
	'/// Press <DELETE> to remove first paragraph numbering
	call wTypeKeys ( "<DELETE>" )
	'/// Check if numbering of the following paragraphs has been adapted even after save and reload
	For j = 1 to 2 
		if j = 2 then
			'/// Save document
			Call hFileSaveAsKill ( gOfficepath & "user\work\tToolsNumbering7.odt" )
			'/// Close and reload document
			Call hCloseDocument
			hFileOpen( gOfficepath & "user\work\tToolsNumbering7.odt" )
		end if
        
		call wTypeKeys ( "<MOD1 A>" )	
		EditCopy
		Select Case gPlatGroup
			Case "unx"
				sClipBrd() = Split (GetClipboardText, CHR$(10))
			Case else
				sClipBrd() = Split (GetClipboardText, CHR$(13) & CHR$(10))
		end select
		
		For i = 1 to 3
			Select Case i	
				Case 1
					if sClipBrd(0) <> "My first number" then
						if Left$(sClipBrd(0), 2) = "1." then
                            Warnlog "Outline Numbering cannot be deleted pressing '<DELETE>' key"
                            exit for
                        else
                            Warnlog "First paragraph not 'My first number' but: " & sClipBrd(0)
                        end if
					end if
				Case 2
					if sClipBrd(1) <> "1. 1.My second number" then
						Warnlog "#128041#Second paragraph not '1. 1.My second number' but: " & sClipBrd(1)
					end if		
				Case 3
					if sClipBrd(2) <> "2. 2.My third number" then
						Warnlog "#128041#Third paragraph not '2. 2.My third number' but: " & sClipBrd(2)
					end if
			end select
		next i	
	next j
	'/// Close document
	Call hCloseDocument

endcase


testcase tToolsNumbering8
    printlog "Take care that numbering can be activated within tables"    
    Dim i as integer
    '/// Take care that numbering can be activated within tables
    '/// Open a new text document
    Call hNewDocument
    '/// Insert a table with 6 rows and 3 columns
    TableInsertTable
    Kontext "TabelleEinfuegenWriter"
    Spalten.Settext "3"
    Zeilen.Settext "6"
    TabelleEinfuegenWriter.Ok
    
    '/// Select 2nd to 6th row in first column
    call wTypeKeys ( "<Mod1 Home>" )
    call wTypeKeys ( "<Down>" )
    call wTypeKeys ( "<Shift Down>", 4 )
    
	'/// Activate Numbering/Bullets via Objectbar
	Call hToolbarSelect("BulletsAndNumbering", true)
    Kontext "NumObjectbar"
	if Not NumObjectbar.Exists then
		Warnlog "Numbering/Bullets objectbar not accessible"
		Call hCloseDocument
		goto endsub
	end if

    '/// Enable numbering
	NumberingOnOff.Click
	'/// Check if numbering has been created in table
	call wTypeKeys ( "<Mod1 Home>" )		
	For i = 1 to 5
		if i = 1 then
            if hIsNumberingBullets() = true then
			    Warnlog "Seems numbering has been created in first row"
            end if
        else
            if hIsNumberingBullets() = false then
			    Warnlog "Seems numbering has not been created correctly"
                exit for
            end if        
		end if
		call wTypeKeys ( "<Down>" )
	next i
    
    '/// Close document
    Call hCloseDocument
endcase


testcase tToolsNumbering9
    Dim i as integer
    printlog "Turn off a numbered list"
    '/// Turn off a numbered list via toolbar
    '/// Open a new text docoument
    Call hNewDocument
    '/// Create a numbering
    '/// Write in document '1. This numbering' followed by a return
    call wTypeKeys "1. This numbering<Return>"
    '/// +  write 'that will be automatically created' followed by a return
    call wTypeKeys "that will be automatically created<RETURN>"
    '/// + write 'and be turned off via toolbar'
    call wTypeKeys ( "and turned off via toolbar" )
    '/// Select all
    call wTypeKeys ( "<Mod1 A>" )
  	'/// Activate Numbering/Bullets via Objectbar
	Call hToolbarSelect("BulletsAndNumbering", true)
    Kontext "NumObjectbar"
	if Not NumObjectbar.Exists then
		Warnlog "Numbering/Bullets objectbar not accessible"
		Call hCloseDocument
		goto endsub
	end if
    '/// Turn numbering off by clicking 'Numbering' symbol
	NumberingOnOff.Click

    '/// Check if numbering has been turned off
    call wTypeKeys ( "<Mod1 Home>" )
    For i = 1 to 3
        call wTypeKeys ( "<Shift End>" )
        EditCopy
        Select Case i
            Case 1
                if GetClipboardtext <> "This numbering" then
                    Warnlog "It seems numbering isn't turned off"
                    exit for
                end if
            Case 2
                if GetClipboardtext <> "that will be automatically created" then
                    Warnlog "It seems numbering isn't turned off"
                    exit for
                end if
            Case 3
                if GetClipboardtext <> "and turned off via toolbar" then
                    Warnlog "It seems numbering isn't turned off"
                end if
        end select
        call wTypeKeys ( "<Down><Home>" )
    next i
    '/// Close document
    Call hCloseDocument
endcase


testcase tToolsNumbering10
    Dim i as integer
    printlog "Pressing <ENTER> key in an empty paragraph turns off numbering"
    '/// Press <ENTER> key in an empty paragraph turns off numbering
    '/// Open a new text docoument
    Call hNewDocument
    '/// Create a numbering
    '/// Write in document '1. This numbering' followed by a return
    call wTypeKeys "1. This numbering<Return>"
    '/// +  write 'that will be automatically created' followed by a return
    call wTypeKeys "that will be automatically created<RETURN>"
    '/// + write 'and be turned off via return' followed by a return
    call wTypeKeys ( "and turned off via return<RETURN>" )
    '/// + enter <Return> to create an empty paragraph
    call wTypeKeys ( "<Return>" )    
    '/// Hit <Return> again should leave a numbering with 3 paragraphs
    call wTypeKeys ( "No numbering anymore" )
    call wTypeKeys ( "<Shift Home>" )
    EditCopy
    if GetClipboardtext <> "No numbering anymore" then
        Warnlog "Seems numbering wasn't turned off !"
    else
        call wTypeKeys ( "<Mod1 Home>" )
        For i = 1 to 3
            if hIsNumberingBullets() = false then
			    Warnlog "Seems numbering has been completely turned off"
                exit for
            end if
            call wTypeKeys ( "<Down>" )
        next i
    end if

    '/// Close document
    Call hCloseDocument
endcase


testcase tToolsNumbering11
    Dim i as integer, sVorlage as string
    Dim sClipBrd() as string
    printlog "Create an empty heading (Ouline numbering)"
    '/// Create an empty heading (Ouline numbering)
    '/// Open a new text document
    Call hNewDocument
	' Get name of 'Heading' (language dependent)
    ToolsOutlineNumbering
	Kontext
	Active.SetPage TabKapitelnumerierung
	Kontext "TabKapitelnumerierung"
	sVorlage = Absatzvorlage.GetSelText
	TabKapitelnumerierung.Cancel
    
    '/// Create a outline numbering
    ' Numbering of outlines will not be copied to clipboard
    ' so we use the Field 'Chapter' which does
    for i = 1 to 3
        InsertFieldsOther
        Kontext
        Active.Setpage TabDokumentFeldbefehle
        Kontext "TabDokumentFeldbefehle"
        try
            Feldtyp.Select fGetType("Chapter")
            wait 500
            Select Case iSprache
                Case 7      :Formatliste.Select 3    'Chapternumber
                Case else   :Formatliste.Select 2    'Chapternumber
            end select
        catch
            Warnlog "Unable to select field 'chapter'"
            TabDokumentFeldbefehle.Close
            Call hCloseDocument
            goto endsub
        endcatch
        Einfuegen.Click
        TabDokumentFeldbefehle.Close
        
        Select Case i
            Case 1
                '/// Insert some text 'A new first paragraph' followed by a return
                call wTypeKeys ( "A new first paragraph<RETURN>" )
            Case 2
                '/// Insert an empty paragraph
                call wTypeKeys ( "<RETURN>" )
            Case 3
                '/// Insert some text 'A new third paragraph'
                call wTypeKeys ( "A new third paragraph" )
        end select
    next i
	'/// Select all text
    call wTypeKeys ( "<MOD1 A>" )
    '/// Set Style 'Heading 1'
	Call wStyleSet ( sVorlage )
	'/// Activate Numbering/Bullets via Objectbar
	Call hToolbarSelect("BulletsAndNumbering", true)
	'/// Create a numbering
	Kontext "NumObjectbar"
	if Not NumObjectbar.Exists then
		Warnlog "Numbering/Bullets objectbar not accessible"
		Call hCloseDocument
		goto endsub
	end if
	NumberingOnOff.Click
    '/// Check if second paragraph is empty but numbered
    
    EditCopy
    Select Case gPlatGroup
        Case "unx"
            sClipBrd() = Split (GetClipboardText, CHR$(10))
        Case else
            sClipBrd() = Split (GetClipboardText, CHR$(13) & CHR$(10))
    end select
    
    For i = 1 to 3
        Select Case i
            Case 1
                if sClipBrd(0) <> "1 1A new first paragraph" then
                    Warnlog "First Heading not: '1 1A new first paragraph' but: " &  sClipBrd(0)
                end if
            Case 2
                if sClipBrd(1) <> "2 2" then
                    Warnlog "First paragraph not '2 2' but: " & sClipBrd(1)
                end if
            Case 3
                if sClipBrd(2) <> "3 3A new third paragraph" then
                    Warnlog "First paragraph not '3 3A new third paragraph' but: " & sClipBrd(2)
                end if
        end select
    next i
    
    '/// Close document
    Call hCloseDocument
endcase
