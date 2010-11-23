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
'* Owner : wolfram.garten@oracle.com
'*
'* short description : Edit -> Find & Replace
'*
'***********************************************************************************
'*
' #1 tiEditSearchAndReplace
' #1 tiEditSearchAndReplaceBUGS
'*
'\**********************************************************************************
testcase tiEditSearchAndReplace
    printlog "This test tests the 'Search and Replace' -dialogue."
    qaerrorlog "#i115121# Find and replace: wrong replacement"
    goto endsub
   Dim sFile  'as string
   Dim sExpression(2) as string  ' Expression to look for: ... Replace with
   dim sExt as string

    Select Case Ucase(gApplication)
        case "DRAW"      : sExt = ".odg"
        case "IMPRESS"   : sExt = ".odp"
    end select

    printlog "First we need some errornes words to replace. (sExpression)"
    printlog "sExpression(1) has to be 3 times in the document"
    select case iSprache
         case 01 : sExpression(1) = "Alle" : sExpression(2) = "Allz"
         case 03 : sExpression(1) = "empreza" : sExpression(2) = "Emperor"       ' Portuguese
         case 07 : qaerrorlog "No files for Russian yet, will use english instead."
                           sExpression(1) = "Alle" :  sExpression(2) = "Allz"
         case 30 :        ' Greek
         case 31 : sExpression(1) = "bedoeling" : sExpression(2) = "Bedoling"    ' Netherlands
         case 33 : sExpression(1) = "projjet" : sExpression(2) = "projete"       ' French
         case 34 : sExpression(1) = "lazsos" : sExpression(2) = "Lazsi"           ' Spanish
         case 35 :        ' Finnish
         case 36 : sExpression(1) = "Bozniai" : sExpression(2) = "Boszniai"            ' Hungaria
         case 37 :        ' Catalan
         case 39 : sExpression(1) = "Presentazione" : sExpression(2) = "Raggazi" ' Italian
         case 42 :        ' Czech
         case 43 :        ' Slowak
         case 44 :        ' English (GB)
         case 45 : sExpression(1) = "installlere" : sExpression(2) = "installer" ' Danish
         case 46 : sExpression(1) = "desa" : sExpression(2) = "ased"             ' Swedish
         case 47 :        ' Norwegian
         case 48 : sExpression(1) = "programmu" : sExpression(2) = "programu"    ' Polish
         case 49 : sExpression(1) = "Texxt" : sExpression(2) = "Schrieb"         ' German
         case 50 : sExpression(1) = "primer" : sExpression(2) = "napakka"        ' Slovenian
         case 55 : sExpression(1) = "esktava" : sExpression(2) = "norte"         ' Portuguese (Brazil)
         case 66 :        ' Thai
'         case 81 : sExpression(1) = "���" : sExpression(2) = "���"       ' Japanese
         '      case 82 :        ' Korean
         '      case 86 :        ' Chinese (simplified)
         '      case 88 :        ' Chinese (traditional)
         case 90 :        ' Turkish
         case 91 :        ' Hindi
         case 96 :        ' Arab
         case 97 :        ' Hebrew
         case else :
'                Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here and create the file"
                printlog " :-) Will use English instead"
                sExpression(1) = "Alle" :  sExpression(2) = "Allz"
    end select

    if ((bAsianLan) OR (iSprache = 07)) then
        Call hFileOpen (gTesttoolpath + "graphics\required\input\recht_"+"1"+sExt)
    else
        Call hFileOpen (gTesttoolpath + "graphics\required\input\recht_"+iSprache+sExt)
    endif
    printlog "check if the document is writable"
    if fIsDocumentWritable = false then
        printlog "make the document writable and check if it's succesfull"
        if fMakeDocumentWritable = false then
            warnlog "The document can't be make writeable. Test stopped."
            goto endsub
        endif
    endif
    sleep 3
    gMouseClick 50,40
    sleep 3
    Kontext "Messagebox"
    if Messagebox.Exists then
        Warnlog Messagebox.GetText
        Messagebox.OK
    end if
    sleep 2
    printlog "Select the text"
    hTypeKeys "<MOD1 HOME>"     

    printlog "Edit-Find & Replace"
    EditSearchAndReplace
    sleep 1
    Kontext "FindAndReplace"
    printlog "Clap the 'more options'-part open"
    More.Click
    printlog "verify, that search is done without 'similarity search' !!!"
    if SimilaritySearch.IsChecked = TRUE then
        SimilaritySearch.UnCheck
        warnlog "'Similar search' checkbox was enabled! It shouldn't be the default!"
    end if
    printlog "set Search text"
    SearchFor.SetText sExpression(1)
    printlog "set Replace text"
    ReplaceWith.SetText sExpression(2)                            

    printlog "Word has to be there ONLY three times"
    printlog "1st search for text"
    SearchNow.Click                                        
    Kontext "Messagebox"
        if Messagebox.Exists(2) then
            Printlog Messagebox.GetText
            Messagebox.OK
        end if
    sleep 1
    Kontext "FindAndReplace"

    printlog "2nd search for text"
    SearchNow.Click                   
    Kontext "Messagebox"
        if Messagebox.Exists(2) then
            Printlog Messagebox.GetText
            Messagebox.OK
        end if
    sleep 1
    Kontext "FindAndReplace"

    printlog "3th search for text"
    SearchNow.Click                   
    Kontext "Messagebox"
        if Messagebox.Exists(2) then
            Printlog Messagebox.GetText
            Messagebox.OK
        end if
    sleep 1
    Kontext "FindAndReplace"
    printlog "4th search for text; but it is no more available!"
    SearchNow.Click                   
    sleep 2
    kontext "Messagebox"
    if Messagebox.Exists Then
        Printlog "- The word was found 3 times: '" + messagebox.gettext + "'"
        Messagebox.OK
    else
        printlog "5th search for word 1"
        SearchNow.Click               
        kontext
        if Messagebox.Exists (5) Then
            Printlog "?: "+Messagebox.gettext
        Warnlog "- No MsgBox appeared: 'changed'"
            Messagebox.OK
        endif
        Warnlog "- No MsgBox appeared: 'changed'"
    end if

 printlog "Replace all"
    Kontext "FindAndReplace"
    sleep 1
    ReplaceAll.Click
    Kontext "Messagebox"
    if Not Messagebox.Exists (2) then
    '      Warnlog "  Not all words are replaced" ' there never was a mesagebox ! (compared to 5.2)
    else
        printlog messagebox.gettext
        qaerrorlog "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
        Messagebox.OK
    end if
    Kontext "FindAndReplace"
    sleep 2
    printlog "Search for word 1 to check if all words are replaced"
    SearchNow.Click                                        
    Kontext "Messagebox"
    if Not Messagebox.Exists then
        Warnlog "  Not all words are replaced (Replace All)"
    else
        printlog "Search key not found!: '" + messagebox.gettext + "'"
        Messagebox.OK
    end if
    Kontext "FindAndReplace"
    FindAndReplace.Close
    sleep 3

 printlog "Edit-Find & Replace II"
    gMouseClick 50,50
    sleep 1
    hTypeKeys "<MOD1 HOME>"
    EditSearchAndReplace
    sleep 1
    Kontext "FindAndReplace"
    printlog "Search for word 2"
    SearchFor.SetText sExpression(2)
    printlog "Replace with word 1"
    ReplaceWith.SetText sExpression(1)       

    printlog "Replace 1"
    ReplaceBtn.Click                      
    Kontext "Messagebox"
    if Messagebox.Exists(4) Then
        Kontext "Messagebox"
        Messagebox.OK
        Printlog "  Clicked 'OK' in the messagebox after searching and replacing the word(s)"
    else
        Printlog "  Word replaced, no messagebox appeared afterwards"
    end if
    Kontext "FindAndReplace"
    sleep 1

    printlog "2nd search for word 2"
    SearchNow.Click                       
    Kontext "Messagebox"
    if Messagebox.Exists(2) Then
        Warnlog "  All words got replaced, but only the 1st should have been"
        Kontext "Messagebox"
        Messagebox.OK
        Kontext "FindAndReplace"
    else
        Printlog "  Word replaced"
    end if

 printlog "Replace all"
    Kontext "FindAndReplace"
    ReplaceAll.Click
    kontext "Messagebox"
    if Messagebox.Exists(4) Then
        Messagebox.OK
    else
        Printlog "- No messagebox after replace all"
    end if

 printlog "Search for word 2 to check if all are replaced"
    Kontext "FindAndReplace"
    SearchNow.Click
    kontext "Messagebox"
    if Messagebox.Exists(2) Then
        printlog "Search key not found!: " + messagebox.gettext +"'"
        Messagebox.OK
    else
        Warnlog "- Replace all does not work properly"
    end if

    Kontext "FindAndReplace"
    More.Click
    FindAndReplace.Close
    sleep 2
  printlog "Close document"
    Call hCloseDocument
    
endcase 'tiEditSearchAndReplace

'--------------------------------------------------------

testcase tiEditSearchAndReplaceBUGS

printlog "inspired by bugs: 101974, 101975, 101976"
   dim i as integer
   dim bTemp as boolean

   printlog "open application"
   Call hNewDocument
   sleep 3
   printlog "Edit->Search and Replace"
   EditSearchAndReplace
   Kontext "FindAndReplace"
   if FindAndReplace.Exists (5) then
      Printlog "Dialog for Search&Replace could be opened :-)"
   else
      Warnlog "Dialog for Search&Replace could NOT be opened :-("
   endif

   printlog "for al 5 views in 'View->Workspace->...' do: "
   for i = 1 to 6
      Printlog "- " + i + " View->Workspace->... ------------------------------------------------------"
      try
         select case i
            case 1:Printlog "                                        Drawing"
                     ViewWorkspaceDrawingView
            case 2:Printlog "                                        Outline"
                     ViewWorkspaceOutlineView
            case 3:Printlog "                                         Slides"
                     ViewWorkspaceSlidesView
            case 4:Printlog "                                         Notes"
                     ViewWorkspaceNotesView
            case 5:Printlog "                                         Handout"
                     ' this is done this way, to regcognize BUGID:101975
                     ViewWorkspaceHandoutView
            case 6:Printlog "                                         Drawing"
                     ViewWorkspaceDrawingView
         end select
      catch
         Warnlog "Couldn't excecute the last view ^^^^^^^^^ :-("
         Printlog "Will close the search&replace dialog as a workaround and then try to switch to the view again"
         Kontext "FindAndReplace"
         FindAndReplace.close
         sleep 5
         try
            select case i
               case 1:ViewWorkspaceDrawingView
               case 2:ViewWorkspaceOutlineView
               case 3:ViewWorkspaceSlidesView
               case 4:ViewWorkspaceNotesView
               case 5:ViewWorkspaceHandoutView
            end select
         catch
            Warnlog "Couldn't really switch the view :-("
         endcatch
      endcatch
      sleep 5
      Kontext "FindAndReplace"
      printlog "if not already open, open. edit->search and replace"
      if (FindAndReplace.exists <> TRUE) then
         try
            EditSearchAndReplace
         catch
         endcatch
      endif
      if (FindAndReplace.exists <> TRUE) then
         if (i=3) then
           ' SlideView
         else
            Warnlog "Search and replace was not open, and could not be opened :-("
         endif
      else
         Kontext "FindAndReplace"
            printlog "set text to look for to a"
         SearchFor.SetText "a"
         Printlog " click button 'find'"
         SearchNow.Click
         sleep 5
         kontext
         printlog "if active exists 'wrap/not found...' say OK"
         if active.exists(5) then
            Printlog "Active: '" + active.GetText + "'"
            try
               active.ok
            catch
               active.yes
               sleep 2
               if active.exists(5) then
                  Printlog "Active: '" + active.GetText + "'"
                  active.ok
               endif
            endcatch
         else
            ' this is done this way, to regcognize BUGID:101974
            Warnlog "No message about 'search key not found' :-("
         endif
      endif
   next i
   Kontext "FindAndReplace"
   FindAndReplace.cancel
   printlog "close application"
   Call hCloseDocument
   
endcase 'tiEditSearchAndReplaceBUGS
