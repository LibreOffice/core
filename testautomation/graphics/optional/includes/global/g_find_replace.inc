'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_find_replace.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:40 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* Owner : wolfram.garten@sun.com
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
    Dim sFile  'as string
    Dim sExpression(2) as string  ' Expression to look for: ... Replace with
    dim sExt as string

    Select Case Ucase(gApplication)
    case "DRAW"      : sExt = ".odg"
    case "IMPRESS"   : sExt = ".odp"
    end select

    'First we need some errornes words to replace. (sExpression)
    'sExpression(1) has to be 3 times in the document
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
    ' check if the document is writable
    if fIsDocumentWritable = false then
        ' make the document writable and check if it's succesfull
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

    EditSearchAndReplace
    sleep 1
    Kontext "FindAndReplace"
    More.Click
    if SimilaritySearch.IsChecked = TRUE then
        SimilaritySearch.UnCheck
        warnlog "'Similar search' checkbox was enabled! It shouldn't be the default!"
    end if

    Kontext "Messagebox"
    if Messagebox.Exists(2) then
        Printlog Messagebox.GetText
        Messagebox.OK
    end if
    sleep 1
    Kontext "FindAndReplace"

    Kontext "Messagebox"
    if Messagebox.Exists(2) then
        Printlog Messagebox.GetText
        Messagebox.OK
    end if
    sleep 1
    Kontext "FindAndReplace"

    Kontext "Messagebox"
    if Messagebox.Exists(2) then
        Printlog Messagebox.GetText
        Messagebox.OK
    end if
    sleep 1
    Kontext "FindAndReplace"

    sleep 2
    kontext "Messagebox"
    if Messagebox.Exists Then
        Printlog "- The word was found 3 times: '" + messagebox.gettext + "'"
        Messagebox.OK
    else
        kontext
        if Messagebox.Exists (5) Then
            Printlog "?: "+Messagebox.gettext
            Warnlog "- No MsgBox appeared: 'changed'"
            Messagebox.OK
        endif
        Warnlog "- No MsgBox appeared: 'changed'"
    end if

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

    gMouseClick 50,50
    sleep 1
    hTypeKeys "<MOD1 HOME>"
    EditSearchAndReplace
    sleep 1
    Kontext "FindAndReplace"

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

    Kontext "Messagebox"
    if Messagebox.Exists(2) Then
        Warnlog "  All words got replaced, but only the 1st should have been"
        Kontext "Messagebox"
        Messagebox.OK
        Kontext "FindAndReplace"
    else
        Printlog "  Word replaced"
    end if

    Kontext "FindAndReplace"
    ReplaceAll.Click
    kontext "Messagebox"
    if Messagebox.Exists(4) Then
        Messagebox.OK
    else
        Printlog "- No messagebox after replace all"
    end if

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
    Call hCloseDocument
endcase

testcase tiEditSearchAndReplaceBUGS
    dim i as integer
    dim bTemp as boolean

    Call hNewDocument
    sleep 3
    EditSearchAndReplace
    Kontext "FindAndReplace"
    if FindAndReplace.Exists (5) then
        Printlog "Dialog for Search&Replace could be opened :-)"
    else
        Warnlog "Dialog for Search&Replace could NOT be opened :-("
    endif

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
            SearchFor.SetText "a"
            SearchNow.Click
            sleep 5
            kontext
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
    Call hCloseDocument
endcase
