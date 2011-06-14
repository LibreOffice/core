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
'* short description : Hide section / Protect section / Display of dialogues
'*
'************************************************************************
'*
' #1  tHideSection_1
' #1  tHideSection_2
' #1  tHideSection_3
' #1  tProtectSection_1
' #1  tDisplaySection_1
' #1  tDisplaySection_2
' #1  tDisplaySection_3
' #1  tDisplaySection_4
' #1  tDisplaySection_5
' #1  tDisplaySection_6
' #1  tDisplaySection_7
' #1  tDisplaySection_8
' #1  tDisplaySection_9
'*
'\***********************************************************************

sub w_section_2

    printLog Chr(13) + "---------   Hide Section   ----------"
    Call tHideSection_1          'Hide Section without condition
    Call tHideSection_2          'Hide Section with condition
    Call tHideSection_3          'Headings and index entries in hidden section should disappear from an index (after actualizing the index)
    
    printLog Chr(13) + "---------   Protect Section   ----------"
    Call tProtectSection_1       'Protect Section
    
    printLog Chr(13) + "---------   Display of dialogues  (Insert - Section)   ----------"
    Call tDisplaySection_1       'Check display of dialogues -- Insert Section
    
    printLog Chr(13) + "---------   Display of dialogues  (Format - Section)   ----------"
    Call tDisplaySection_2       'Check display of dialogues -- Format Section -1
    Call tDisplaySection_3       'Check display of dialogues -- Format Section -2
    Call tDisplaySection_4       'hiding a section hierarchy hides all subsections
    
    printLog Chr(13) + "---------   Display of dialogues  (Navigator)   ----------"
    Call tDisplaySection_5       'Sections are displayed in row of their insertion in Navigator
    Call tDisplaySection_6       'Jump to a section by double-clicking it in navigator
    Call tDisplaySection_7       'Activate Navigator, insert a section , it must be displayed
    Call tDisplaySection_8       'Select a section in the Navigator, call context menu, choose "Edit"
    Call tDisplaySection_9       'Check to insert a paragraph before and after a section

end sub

'***********************************************************************

testcase tHideSection_1
    '/// <b>Hide Section without condition </b>
    printlog "Hide Section without condition "
    
    '/// 1. Test from insert/section
    '/// Open a new document.
    Call hNewDocument
    
    '/// Press Return and then input some test words
    Call wTypeKeys "<Return>"
    Call wTypeKeys "Section: hidden without condition"
    
    '/// Select the test words with SHIFT and HOME keys.
    Call wTypeKeys "<Shift Home>"
    
    '/// Insert/Section / Section, check 'Hide' in Hide area
    InsertSection
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Ausblenden.check
    TabBereiche.OK
    sleep(2)
    
    '/// Edit / SelectAll
    EditSelectAll
    
    '/// Edit / Copy
    'Edit / Copy should NOT be available because the words in the section are hidden.
    try
        EditCopy
        warnlog "Hide Section without condition doesn't work in insert/section!"
    catch
    endcatch
    
    Call hCloseDocument
    
    '/// 2.Test from format/section
    Call hNewDocument
    
    '/// Press Return and then input some test words
    Call wTypeKeys "<Return>"
    Call wTypeKeys "Section: hidden without condition"
    
    '/// Select the test words with SHIFT and HOME keys.
    Call wTypeKeys "<Shift Home>"
    
    '/// Insert a section
    Call fInsertSection("HideSection")
    
    '/// Format/Section and check 'Hide' in Hide area
    FormatSections
    Kontext "BereicheBearbeiten"
    Ausblenden.check
    BereicheBearbeiten.OK
    
    '/// Edit / SelectAll
    EditSelectAll
    
    '/// Edit / Copy
    'Edit / Copy should NOT be available , because the words in the section are hided
    try
        EditCopy
        warnlog "Hide Section without condition doesn't work in format/section!"
    catch
    endcatch
    '///Close the document.
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tHideSection_2
    Dim rightCondition as String
    Dim wrongCondition as String
    '/// <b> Hide Section with condition </b>
    printlog "Hide Section with condition "
    '/// Open a new document. 
    Call hNewDocument
    '/// Press RETURN and then input some test words.
    Call wTypeKeys "<Return>"
    Call wTypeKeys "Section: hidden with condition<return>"
    
    '/// Select test words with UP- then HOME-key. Press SHIFT- and END-keys together.
    Call wTypeKeys "<Up><Home><Shift End>"
    
    '/// Insert a section.
    Call fInsertSection("HideSection")
    
    '/// Place the cursor out of the section with DOWN key.
    Call wTypeKeys "<Down>"
    
    '/// Insert a user field
    '/// Insert/Fields/Others , Tabpage Variables ,
    '/// + select 'User field' in 'Type' area ,
    '/// + then select 'Text' in 'Format' area ,
    '/// + input 'myTest' in Name and 'a' in Value ,
    '/// + click insert button , and close the dialog
    InsertFieldsOther
    Kontext
    Active.SetPage TabVariablen
    Kontext "TabVariablen"
    Feldtyp.Select Feldtyp.GetItemCount        'User Field
    Zahlenformat.Select(1)                     'Text
    NameText.SetText "myTest"
    Formel.SetText "a"
    Einfuegen.Click                            '--insert button
    sleep(1)
    TabVariablen.Close
    
    '/// Format/Section, click Hide and enter condition
    FormatSections
    Kontext "BereicheBearbeiten"
    Ausblenden.check
    '/// -- enter wrong condition should NOT hide section
    '/// => wrong Condition is  (myTest eq "B")
    wrongCondition = "myTest eq " + String(1,34) + "B" + String(1,34)
    Bedingung.SetText wrongCondition
    BereicheBearbeiten.OK
    sleep(2)
    
    'Tools/Update/Fields to ensure to hide the section if the section can be hided
    ToolsUpdateFields
    sleep(2)
    
    '/// Use UP-, then HOME-keys, then SHIFT- and END-keys together to 
    '///+ check if the test words in section is hidden.
    Call wTypeKeys "<Up><Home><Shift End>"
    try
        EditCopy
    catch
        warnlog "Hide Section with wrong condition should NOT hide the section!"
        Goto endsub
    endcatch
    
    if GetClipboardText = "Section: hidden with condition" then
        '/// Press DOWN key.
        Call wTypeKeys "<Down>"
        Kontext
        '/// Format / Sections
        FormatSections
        Kontext "BereicheBearbeiten"
        '/// Check 'Hide'-checkbox.
        Ausblenden.check
        '/// -- enter right condition should hide section
        '/// => rightCondition is (myTest eq "A")
        rightCondition = "myTest eq " + String(1,34) + "a" + String(1,34)
        Bedingung.SetText rightCondition
        '/// Close the dialogue with OK.
        BereicheBearbeiten.OK
        sleep(2)
        '/// Tools / Update / Fields
        ToolsUpdateFields
        sleep(1)
        
        '/// Check if the test words in section are hidden.
        Call wTypeKeys "<Up><Home><Shift End>"
        try
            EditCopy
            if GetClipboardText = "Section: hidden with condition" then
                warnlog "Hide Section with wrong condition should NOT hide the section!"
            else
                warnlog "Something wrong, need to check! -- II"
            end if
        catch
        endcatch
    else
        warnlog "Something wrong, need to check! -- I"
    end if
    '///Close the document. 
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tHideSection_3

   '/// Headings and index entries in hidden section should disappear from an index (after actualizing the index)
   printlog "  -- Headings and index entries in hidden section should disappear from an index (after actualizing the index)"

    Call hNewDocument

    '/// Insert some test words
    Call wTypeKeys "Test for Heading and Index in section<Return>"
    Call wTypeKeys "<Up>"
    Call wTypeKeys "<Home><Shift End>"

    '/// Insert a section
    fInsertSection("HeadAndIndexInSection")

    '/// select part in the section.
    Call wTypeKeys "<Home><Shift Mod1 Right>"

    '/// Select Insert/Indexes and Tables/Entrys and add this part to an index
    InsertIndexesEntry
    Sleep 4
    Kontext "VerzeichniseintragEinfuegen"

    '/// generate testing index - myTest
    EintragNeu.Click
    Kontext
    Kontext "BenutzerverzeichnisDefinieren"
    NameBenutzerverzeichnis.Settext "myTest"
    BenutzerverzeichnisDefinieren.Ok

    Kontext
    Kontext "VerzeichniseintragEinfuegen"
    EintragOk.Click
    Sleep 1
    VerzeichniseintragEinfuegen.close

    '/// Move the cursor out of the section
    Call wTypeKeys "<down>"

    '/// Insert/Indexes and Tables/Indexes and Tables ...
    InsertIndexes
    Sleep 2
    Kontext
    Active.SetPage TabVerzeichnisseVerzeichnis
    Kontext "TabVerzeichnisseVerzeichnis"
    Verzeichnistyp.Select "myTest"
    '/// protected against manual changes UNcheck
    VerzeichnisGeschuetzt.UnCheck
    TabVerzeichnisseVerzeichnis.OK

    '/// Edit section with Format/Section and choose 'Hide'
    Call wTypeKeys "<Up>"
    Kontext
    FormatSections
    Kontext "BereicheBearbeiten"
    Ausblenden.check
    BereicheBearbeiten.OK

    ToolsUpdateAllIndexes

    EditSelectAll
    EditCopy

    if InStr( GetClipboardText , "Test" ) <> 0 then warnlog "Headings and index entries in hidden section don't disappear from an index:" + GetClipboardText

    Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tProtectSection_1

   printlog "   - Protect Section"
   '/// Protect Section ///

   Call hNewDocument

   '/// Open options and check 'Direct-Cursor' in options ///
   fCheckDirectCursor

   Call wTypeKeys "Should be edited<return>"
   Call wTypeKeys "Section: protected"
   Call wTypeKeys "<Shift Home>"

   '/// Insert a protected section  in the bottom ///

   InsertSection
   Sleep 2
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"
   Bereichsliste.Settext "protect section"
   Geschuetzt.Check
   TabBereiche.OK
   sleep 1

   EditCopy

   '/// 1) Warning should appear when trying to write in protected area! ///
   if GetClipBoardtext = "Section: protected" then
     Call wTypeKeys "<Home>"
     Call wTypeKeys "<Space>"
     Kontext "Active"
     If Active.Exists then
      Active.ok
     else
      warnlog "-  No warning when trying to write in protected area!"
     end if
   else
     warnlog "Testtool couldn't find the protected area!"
   end if

   Call wTypeKeys "<Up>"
   Call wTypeKeys "<Home><Shift End>"

   EditCopy

   '/// 2) Warning should NOT appear when trying to write in un-protected area
   '/// +  even the protected section at the end of document
   if GetClipBoardtext = "Should be edited" then

     Call wTypeKeys "<Home>"
     Call wTypeKeys "Can you see what I am doing?"
     Call wTypeKeys "<Home><Shift End>"
     EditCopy
     If GetClipBoardtext <> "Can you see what I am doing?Should be edited" then warnlog "Should edit in the no-protected area"

   else
      warnlog "Testtool couldn't find the edited area!"
   end if

   Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tDisplaySection_1

   '/// Check display of dialogues -- Insert Section
   printlog "   - Check display of dialogues -- Insert Section"

   Dim SectionName() as String
   Dim correctSequence() as String
   Dim selectName as String
   Dim i as Integer
   Dim j as Integer

   SectionName = Array( _
                       "a1","b2","c3","d4","e5","f6","g7", _
                       "h1","i2","j3","k4","l5","m6","n7", _
                       "H1","I2","J3","K4","L5","M6","N7", _
                       "A1","B2","C3","D4","E5","F6","G7", _
                      )

   Select case iSystemSprache
          case 81,82,86,88 : correctSequence = Array( _
                       "a1","A1","b2","B2","c3","C3","d4","D4", _
                       "e5","E5","f6","F6","g7","G7", _
                       "h1","H1","i2","I2","j3","J3","k4","K4", _
                       "l5","L5","m6","M6","n7","N7", _
                       )
          case else :        correctSequence = Array( _
                       "a1","A1","b2","B2","c3","C3","d4","D4", _
                       "e5","E5","f6","F6","g7","G7", _
                       "h1","H1","i2","I2","j3","J3","k4","K4", _
                       "l5","L5","m6","M6","n7","N7", _
                       )
          end select

'  correctSequence = Array( _
'                       "A1","a1","B2","b2","C3","c3","D4","d4", _
'                       "E5","e5","F6","f6","G7","g7", _
'                       "H1","h1","I2","i2","J3","j3","K4","k4", _
'                       "L5","l5","M6","m6","N7","n7", _
'                       )

   j = UBound(SectionName)

   Call hNewDocument

   '/// insert a lot of sections
   For i = 0 to j
     fInsertSection(SectionName(i))
   next i

   InsertSection
   Sleep 1
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"

   '/// Check if Section name display alphabetically
   For i = 1 to j+1
     Bereichsliste.Select i
     selectName = Bereichsliste.GetSelText
     if  selectName <> correctSequence(i-1) then warnlog "Section name displays alphabetically wrong! Getting section name:"+ selectName +"  The section name should be: "+ correctSequence(i-1)
   next i

   '/// check if vertical scrollbar display
   if (Bereichsliste.HasScrollbar (AlignRight)) = FALSE then warnlog "Vertical scrollbar should be displayed !"

   TabBereiche.Cancel
   sleep 1

   Call hCloseDocument

   Call hNewDocument

   '/// Insert a section with a long name
   fInsertSection("a loooooooooooooooooooooooooooooooooooooooooooooooong name section")

   InsertSection
   Sleep 1
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"

   '/// check if horizontal scrollbar display
   if (Bereichsliste.HasScrollbar (AlignBottom)) = FALSE then warnlog "horizontal scrollbar should be displayed"

   TabBereiche.Cancel
   sleep 1

   Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tDisplaySection_2

   Dim SectionName() as String
   Dim i , j         as Integer

   if Dir (gOfficePath + "user\work\testlink1.odt") <> "" then kill (gOfficePath + "user\work\testlink1.odt")
   if Dir (gOfficePath + "user\work\testlink2.odt") <> "" then kill (gOfficePath + "user\work\testlink2.odt")

   '///  Check display of dialogues -- Format Section -1
   printlog "   - Check display of dialogues -- Format Section"
   '/// -- Section names are displayed in row of their insertion
   SectionName = Array("a2","A1","a4","A5")
   j = UBound(SectionName)

   Call hNewDocument

   For i = 0 to j
     fInsertSection(SectionName(i))
   next i

   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"
   Bereich.TypeKeys "<Up>",4

   For i = 0 to j
     if  BereichsName.GetText <> SectionName(i) then warnlog "Section names are NOT displayed in row of their insertion"
     Bereich.TypeKeys "<Down>"
   next i

   BereicheBearbeiten.Cancel
   sleep 2

   Call hCloseDocument

   '/// -- An exception is the hierarchical order; this is diplayed by an indentation
   SectionName = Array("A2","testlink1","A1","testlink2")
   j = UBound(SectionName)

   Call hNewDocument
   fInsertSection("testlink1")
   Call hFileSaveAsKill (gOfficePath + "user\work\testlink1.odt")
   Call hCloseDocument

   Call hNewDocument
   fInsertSection("testlink2")
   Call hFileSaveAsKill (gOfficePath + "user\work\testlink2.odt")
   Call hCloseDocument

   '/// insert fisrt section linked a test file which include a section
   Call hNewDocument
   InsertSection
   Sleep 2
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"
   Bereichsliste.Settext "A2"
   Verknuepfung.Check

   DateiVerknuepfung.Settext ConvertPath(gOfficePath + "user\work\testlink1.odt")
   TabBereiche.OK

  '/// insert another section linked another test file which include another section
   InsertSection
   Sleep 2
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"
   Bereichsliste.Settext "A1"
   Verknuepfung.Check

   DateiVerknuepfung.Settext ConvertPath(gOfficePath + "user\work\testlink2.odt")
   TabBereiche.OK

   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"

   Bereich.TypeKeys "<Up>",4

   For i = 0 to j
     if  BereichsName.GetText <> SectionName(i) then warnlog "Section names are NOT displayed in row of their insertion"
     Bereich.TypeKeys "<Down>"
   next i

   BereicheBearbeiten.Cancel
   sleep 2

   Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tDisplaySection_3

   '/// Check display of dialogues -- Format Section -2
   '/// + double clicking a section with subsections hides the subsections,
   '/// + another double-click displays them again

   if Dir (gOfficePath + "user\work\DisplaySection_3.odt") <> "" then kill (gOfficePath + "user\work\DisplaySection_3.odt")

   printlog "   - Check display of dialogues -- Format Section -2"

   Call hNewDocument
   fInsertSection("testlink1")
   Call hFileSaveAsKill (gOfficePath + "user\work\DisplaySection_3.odt")
   Call hCloseDocument

   '/// insert a section(A2) linked a test file which include a section(testlink1)
   Call hNewDocument
   InsertSection
   Sleep 2
   Kontext
   Active.Setpage TabBereiche
   Kontext "TabBereiche"
   Bereichsliste.Settext "A2"
   Verknuepfung.Check

   DateiVerknuepfung.Settext ConvertPath(gOfficePath + "user\work\DisplaySection_3.odt")
   TabBereiche.OK

   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"

   Bereich.TypeKeys "+"
   if Bereich.GetItemCount < 2 then warnlog "Should be 2 sections in the list tree, now get " +Bereich.GetItemCount
   Bereich.TypeKeys "-"
   if Bereich.GetItemCount > 1 then warnlog "Should be 1 section in the list tree, get " +Bereich.GetItemCount
   Bereich.TypeKeys "+"
   if Bereich.GetItemCount < 2 then warnlog "Should be 2 sections in the list tree, now get " +Bereich.GetItemCount
   Bereich.TypeKeys "-"
   if Bereich.GetItemCount > 1 then warnlog "Should be 1 section in the list tree, get " +Bereich.GetItemCount

   BereicheBearbeiten.Cancel
   sleep 2

   Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tDisplaySection_4
    Dim mMessage as String
    Dim allMessage as String
    '///<u>Goal</u>: Hiding a section hierarchy hides all subsections.
    '///+(Except if the section which should be hided is the whole document - than this feature is disabled!)
    '///+<u>Note</u>: The display in the dialog should not affected by changes in the document view.
    printlog "   - Check hiding a section hierarchy hides all subsections -- Format Section -3"
    Call hNewDocument
    '/// Creating a new document.
    Call wTypeKeys "This is Section_s<Shift Home>"
    '/// Writing This is Section_s, marking it and use Insert / Section to create a <i>Section_s</i>.
    Call fInsertSection("Section_s")
    '/// Saving it as DisplaySection_4.odt in the user-work-dir.
    Call hFileSaveAsKill (gOfficePath & "user\work\DisplaySection_4.odt")
    '/// Closing the document.
    Call hCloseDocument
    '/// Creating a new document.
    Call hNewDocument
    '/// <b>Entering a RETURN at the beginning of the new document</b>!
    Call wTypeKeys "<RETURN>"
    '/// Writing <i>This is Section_m</i> and marking it.
    '/// Insert a section named <i>Section_m</i>.
    Call wTypeKeys "This is Section_m<Shift Home>"
    Call fInsertSection("Section_m")
    '/// Create a new line with END and RETURN in the actual section.
    Call wTypeKeys "<END><RETURN>"
    '/// Write <i>This Section should be Section_s</i> and mark it.
    Call wTypeKeys "This Section should be Section_s<Shift Home>"
    Kontext
    '/// Insert / Section
    InsertSection
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
        '/// Name it <i>Section_u</i>.
        Bereichsliste.Settext "Section_u"
        '/// <b>Link it</b> to the first created document and the section in it.
        '///+ There should be pop up a warning! Press OK on it!
        Verknuepfung.Check
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Active.Yes
            else
                warnlog "No messagebox warns of deleting content of the current section!"
            end if
        end if
        Kontext "TabBereiche"
        Dateiverknuepfung.Settext ConvertPath(gOfficePath & "user\work\DisplaySection_4.odt")
        '/// Unprotect it if it is protected.
        Geschuetzt.UnCheck
        '/// be sure that it is visible.
        Ausblenden.UnCheck
        '/// OK on the <i>section</i> dialog.
    TabBereiche.OK
    sleep(2)

    Call wTypeKeys "<MOD1 HOME>",2
    '/// Type the keys <CTRL HOME> together and twice.
    Call wTypeKeys "<SHIFT END>"
    '/// Type the keys <SHIFT END> together.
    EditCopy
    '/// Copy it to the clipboard (Edit / Copy).
    '/// <i>This is Section_m</i> should be now in the clipboard.
    if GetClipboardText = "This is Section_m" then
        '/// Type keys <DOWN> and then <HOME>
        Call wTypeKeys "<DOWN><HOME>"
        '/// Type keys <SHIFT END> together
        Call wTypeKeys "<SHIFT END>"
        EditCopy
        '/// Copy it to the clipboard (Edit / Copy).
        if GetClipboardText = "This is Section_s" then
            '/// <i>This is Section_s</i> should be visible/in the clipboard because the top section should not be affected if a lower section has been hided.
            Kontext
                '/// Format / Sections
                FormatSections
            Kontext "BereicheBearbeiten"
                '/// Hide the <i>Section_s</i> (Middle section).
                Bereich.TypeKeys "<Down>"
                Ausblenden.Check
                '/// OK on the dialog.
                BereicheBearbeiten.OK
                sleep(2)

                Call wTypeKeys "<HOME>"
                '/// Type key <HOME>
                Call wTypeKeys "<SHIFT END>"
                '/// Type key <SHIFT END> together
                EditCopy
                '/// Copy it to the clipboard (Edit / Copy).
                if GetClipboardText <> "This is Section_m" then warnlog "Should only display the content in section_m, but '" & GetClipboardText & "' is visible."
                '/// <i>This is Section_m</i> should be in the clipboard, nothing more.
                Kontext
                    '/// Format / Sections
                    FormatSections
                Kontext "BereicheBearbeiten"
                    '/// Verify that all sections we have created are visible in the dialog.
                    Bereich.TypeKeys "<Up>" , 2
                    if BereichsName.GetText <> "Section_m" then warnlog "The display in dialogue is affected: '" & BereichsName.GetText & "'."
                    Bereich.TypeKeys "<Down>"
                    if BereichsName.GetText <> "Section_u" then warnlog "The display in dialogue is affected: '" & BereichsName.GetText & "'."
                    Bereich.TypeKeys "<Down>"
                    if BereichsName.GetText <> "Section_s" then warnlog "The display in dialogue is affected: '" & BereichsName.GetText & "'."
                    '/// Unhide all Sections.
                    Ausblenden.UnCheck
                    '/// Go to the top section (<i>Section_m</i>
                    Bereich.TypeKeys "<UP>" , 2
                    '/// Hide the top section.
                    Ausblenden.Check
                    '/// OK on the dialog.
                    BereicheBearbeiten.OK
                    sleep(2)
                    '/// Edit / Select all
                    EditSelectAll
                    try
                        '/// Verify that Edit / Copy is NOT usable! There is nothing selected because all sections should be hided!
                        EditCopy
                        warnlog "It was possible to use Edit / Copy but it should be an empty document and the menu item should be disabled!"
                    catch
                        printlog "Edit / Copy was disabled because there should be nothing in the document which could be copied."
                    endcatch
                    Kontext
                        '/// Format / Sections
                        FormatSections
                        '///+ Verify that hidintg <i>section_m</i> in the document does not affect the dialog:
                        Kontext "BereicheBearbeiten"
                        Bereich.TypeKeys "<UP>" , 3
                        if BereichsName.GetText <> "Section_m" then warnlog "the display in dialogue is affected" +BereichsName.GetText
                        Bereich.TypeKeys "<DOWN>"
                        if BereichsName.GetText <> "Section_u" then warnlog "the display in dialogue is affected" +BereichsName.GetText
                        Bereich.TypeKeys "<DOWN>"
                        if BereichsName.GetText <> "Section_s" then warnlog "the display in dialogue is affected" +BereichsName.GetText
                        BereicheBearbeiten.Cancel
                        '/// Cancel the dialog.
        else
            warnlog "Don't get the correct content of Section_s"
        end if
    else
        warnlog "Don't get the correct content of Section_m"
    end if

   Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tDisplaySection_5

   Dim sectionName() as String
   Dim correctSequence() as String
   Dim i as Integer
   Dim j as Integer

   if Dir (gOfficePath + "user\work\NavigatorSection_1.odt") <> "" then kill (gOfficePath + "user\work\NavigatorSection_1.odt")

   '/// Sections are displayed in row of their insertion in Navigator.
   '///+Hierarchies, protected and hidden sections have no special mark.
   printlog "   - Check Section name display in navigator"

   'Generate a .odt file which include a section
   Call hNewDocument
   fInsertSection("Section_S")
   Call wTypeKeys "<up>This is section_S"
   Call hFileSaveAsKill (gOfficePath + "user\work\NavigatorSection_1.odt")
   Call hCloseDocument

   Call hNewDocument

   '/// Input some test words
   Call wTypeKeys "Test is in sectionNormal<return>"
   Call wTypeKeys "Test is in sectionHide<return>"
   Call wTypeKeys "Test is in sectionProtect<return>"

   sectionName = Array( _
                         "sectionNormal",  _
                         "sectionHide",    _
                         "sectionProtect", _
                         "sectionLink",    _
                         "Section_S",      _
                        )

   correctSequence = Array( _
                         "sectionHide",    _
                         "sectionNormal",  _
                         "sectionProtect", _
                         "sectionLink",    _
                         "Section_S",      _
                        )

   j = UBound(sectionName)

   Call wTypeKeys "<Up>",4

   For i = 0 to j-1
     Call wTypeKeys "<Home><Shift End>"
     fInsertSection(sectionName(i))
     Call wTypeKeys "<Down>"
   next i

   Kontext
   FormatSections
   Kontext "BereicheBearbeiten"

   '/// insert a normal section
   Bereich.TypeKeys "<Up>",4

   '/// insert a section with hide option
   Bereich.TypeKeys "<Down>"
   if BereichsName.GetText = sectionName(1) then
       Ausblenden.check
   else
       warnlog "The second section should be " +sectionName(1) +" bug get" +BereichsName.GetText
   end if

   '/// insert a section with protect option
   Bereich.TypeKeys "<Down>"
   if BereichsName.GetText = sectionName(2) then
       Geschuetzt.Check
   else
       warnlog "The second section should be " +sectionName(2) +" bug get" +BereichsName.GetText
   end if

   '/// insert a section with Link option
   Bereich.TypeKeys "<Down>"
   if BereichsName.GetText = sectionName(3) then
       Verknuepfung.Check
       Dateiname.Settext ConvertPath(gOfficePath + "user\work\NavigatorSection_1.odt")
   else
       warnlog "The second section should be " +sectionName(3) +" bug get" +BereichsName.GetText
   end if

   BereicheBearbeiten.OK
   Sleep 1

   'start up Navigator
   Call wNavigatorAuswahl(7,1, true)

   printlog "  -- check if the section name in navigator is right "
   for i= 0 to j
      if Auswahlliste.GetitemText(8+i) <> correctSequence(i) then QAErrorLog "The Section Name may be wrong, should be " + sectionName(i) +" but get " & Auswahlliste.GetitemText(i+2)
   next i

   'Close Navigator before close the document
   fCloseNavigator

   Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tDisplaySection_6

   Dim sectionContent() as String
   Dim j as Integer

   '/// Jump to a section by double-clicking it in navigator
   '/// +instead of 'double-click', we choose return
   printlog "   - Check Jump to a section by double-clicking it in navigator"

   Call hNewDocument

   sectionContent = Array( _
                         "Test is in Section1", _
                         "Test is in Section2", _
                         )

   '/// Input some test words
   Call wTypeKeys sectionContent(0)
   Call wTypeKeys "<return>"
   Call wTypeKeys sectionContent(1)

   Call wTypeKeys "<up><home><Shift End>"
   fInsertSection("section1")

   Call wTypeKeys "<down><home><Shift End>"
   fInsertSection("section2")

   'start up Navigator
   Call wNavigatorAuswahl(7,1, true)

   'Jump to a section by double-clicking it
   printlog "  -- check Jump to a section by double-clicking it "
   for j=1 to 2
      Auswahlliste.TypeKeys "<return>"
      Sleep 2
      Call wTypeKeys "<home><Shift End>"
      EditCopy
      if GetClipboardText <> sectionContent(j-1) then  warnlog "The focus is NOT in the correct location"
      Kontext "NavigatorWriter"
      Auswahlliste.TypeKeys "<down>"
   next j

   'Close Navigator before close the document
   fCloseNavigator

   Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tDisplaySection_7

   Dim i as Integer

   '///   Activate Navigator, insert a section , it must be displayed
   '/// + immediately in the Navigator .
   printlog "   - Check inserting a section must be displayed immediately in the Navigator "

   Call hNewDocument

   'start up Navigator first
   Call wNavigatorAuswahl(7,1)

   fInsertSection("sectionNormal")
   Sleep 1
   
   Call wNavigatorAuswahl(7,1)
   
   Kontext "NavigatorWriter"
   Auswahlliste.TypeKeys "<return>"

   printlog "  -- check if the section name in navigator is right "
   if Auswahlliste.GetitemText(8) <> "sectionNormal" then warnlog "The section can't be displayed immediately in the Navigator. Found this: '" + Auswahlliste.GetitemText(8) + "' instead."

   'Close Navigator before close the document
   fCloseNavigator

   Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tDisplaySection_8

   Dim i as Integer

   '///  Select a section in the Navigator, call context menu, choose "Edit"
   '///+ Format-Section dialogue must appear
   printlog "   - Check if the context menu should work in navigator / section "

   Call hNewDocument
   sleep(2)
   fInsertSection("sectionNormal")
   sleep(2)
   'start up Navigator
   Call wNavigatorAuswahl(7,1, true)
   sleep(2)
   Auswahlliste.OpenContextMenu
   sleep(2)
   hMenuSelectNr(4)
   sleep(2)
   'Check if the edit section dialog is up
   Kontext "BereicheBearbeiten"
   if NOT BereicheBearbeiten.Exists then
      warnlog "- Dialog to eidt section isn't up!"
   else
      BereicheBearbeiten.Cancel
   end if
   sleep(1)
   'Close Navigator before close the document
   fCloseNavigator
   sleep(1)
   Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tDisplaySection_9

   Dim i as Integer
   Dim shouldbe as string

   '/// Check to insert a paragraph before and after a section"
   printlog "   -  Check to insert a paragraph before and after a section"
   '/// Open a new writerdocument ///
   Call hNewDocument
   '/// Write some text, selct it and insert a section named 'My Section' ///
   Call wTypeKeys "Add a paragraph before and after a section!<Shift Home>"
   fInsertSection("My Section")
   Sleep 1
   '/// Point cursor before text and press Alt+Return ///
   Call wTypeKeys "<Home>"
   Call wTypeKeys "<Mod2 Return>"
   '/// A paragraph should have been inserted before section ///
   Call wTypeKeys "Paragraph before a section"
   '/// Point cursor at end of text in section and press Alt + Return ///
   Call wTypeKeys "<Down><End>"
   Call wTypeKeys "<Mod2 Return>"
   '/// A paragraph should have been inserted after section ///
   Call wTypeKeys "Paragraph after a section"
   '/// Check settings ///
   Call wTypeKeys "<Home><Up>",4
   For i = 1 to 3
      Select Case i
         Case 1
            Call wTypeKeys "<Shift End>"
            EditCopy
            if GetClipBoardtext <> "Paragraph before a section" then
               warnlog "Problem with paragraph before section"
            end if
         Case 2
            Call wTypeKeys "<Shift Down>"
            EditCopy
            if gPlatGroup = "unx" then
                shouldbe = "Add a paragraph before and after a section!" + CHR$(10)
            else
                shouldbe = "Add a paragraph before and after a section!" + CHR$(13) & CHR$(10)
            end if

            if GetClipBoardtext <> shouldbe then
               Warnlog "Copied text not correctly exported through the clipboard"   '   "Problem with section"
            end if
         Case 3
            Call wTypeKeys "<Shift End>"
            EditCopy
            if GetClipBoardtext <> "Paragraph after a section" then
               warnlog "Problem with paragraph after section"
            end if
      end select
      Call wTypeKeys "<Home><Down>"
   next i

   Call hCloseDocument

endcase
