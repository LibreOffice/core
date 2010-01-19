'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_spellcheck.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-08-28 11:43:10 $
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
'* short description :
'*
'*******************************************************************
'*
' #1 tiToolsSpellcheckCorrect
' #1 tiToolsSpellcheckError
' #1 tiToolsSpellcheckCheck
' #1 tToolsSpellcheckAutoSpellcheck
'*
'\*******************************************************************
testcase tiToolsSpellcheckCorrect
    if iSprache = 48 then
        qaerrorlog "This test is not adapted for polish, 48."
        got endsub
    endif

    Dim DieDatei as String
    dim lFiles(100) as string
    dim i as integer
    dim iFiles as integer

    lFiles(0)=0
    Printlog "- Checking Dictionary-Files" ' borrowed from w_106.inc
    select case iSprache
    case 01 : DieDatei = "01-44-hyph.dat"
    case else : DieDatei =  "" & iSprache & "-hyph.dat"
    end select
    DieDatei = Convertpath(gNetzOfficePath + "share\dict\" + DieDatei)
    if gPlatGroup <> "unx" then
        if (Dir(DieDatei) = "") then
            if bAsianLan then
                printlog "Dictionary not found : " + DieDatei + ", but is AsianLan, so OK :-)"
            else
                if gNetzInst then
                    printlog "Dictionary not found : " + DieDatei
                else
                    warnlog "Dictionary not found : " + DieDatei
                end if
            end if
        else
            Printlog "    Dictionary has been installed : " + DieDatei
        end if
    end if
    iFiles = GetFileList (Convertpath (gNetzOfficePath + "share\dict\"), "*.dat" ,lFiles())
    for i = 1 to iFiles
        printlog " " + i + ": " + DateiExtract(lFiles(i))
    next i

    Call hNewDocument
    '    sleep 2
    ToolsSpellcheck
    Kontext "Active"
    if Active.Exists(5) then
        try
            printlog "Message: Finished: Want to continue at the beginning? '" + active.gettext + "'"
            Active.No
        catch
            Warnlog "The Active-dialoge didn't have a No-button, tries with OK instead."
            Active.Ok
        endcatch
    else
        Warnlog "No 'Spellcheck finished, do you wish to continue?' message appeared"
    end if
    sleep 2

    PrintLog "- Spellcheck with correct text"
    select case iSprache
    case 01   : hTextrahmenErstellen ("This is a text without any error.<Return>",10,10,50,20)
    case 33   : hTextrahmenErstellen ("Il nous faut donc un de temps pour examiner avec soin tous les dossiers.<Return>",10,10,70,20)
    case 34   : hTextrahmenErstellen ("Este es un chico muy importante.",10,10,50,20)
    case 36   : hTextrahmenErstellen ("akit a b�r�s�g vagy a szab�lys�rt�si hat�s�g a eltiltott",10,10,70,20)
    case 39   : hTextrahmenErstellen ("La ringraziamo per l'interesse mostrato a collaborare con la firma.<Return>",10,10,70,20)
    case 46   : hTextrahmenErstellen ("Det varierar vad som behandlas och ur vilket perspektiv.<Return>",10,10,50,20)
    case 49   : hTextrahmenErstellen ("Dies ist ein Text ohne Fehler.<Return>",10,10,50,20)
    case 55   : hTextrahmenErstellen ("Esta poderia ser a resposta para suas preces?<Return>",10,10,50,20)
    case else :
        if bAsianLan then
            printlog "For the language  " + iSprache +" nothing is prepared yet, but is AsianLan, so OK :-) will use english instaed"
            ' there was smth wrong, try to find out :
            kontext
            if active.exists then
                printlog "ERROR: active: '"+active.gettext+"'"
                active.ok
            end if
            hTextrahmenErstellen ("This is a text without any error.<Return>",10,10,50,20)
        else
            Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here"
            hTextrahmenErstellen ("This is a text without any error.<Return>",10,10,50,20)
        end if
    end select
    ToolsSpellcheck
    WaitSlot (2000)
    Kontext "Active"
    if active.exists(5) then
        printlog "Message: spellchecking has finished?: '" + active.gettext + "'"
        Active.OK
    else
        errorLog "Spellcheck started :"
        Kontext "Spellcheck"
        if Spellcheck.exists then
            errorlog " - spellcheck came up and will be closed now"
            Spellcheck.Close
        else
            printlog "spellcheck didn't come up"
        end if
    end if
    WaitSlot (2000)
    Call hCloseDocument
endcase 'tiToolsSpellcheckCorrect

'------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
testcase tiToolsSpellcheckError
    if iSprache = 48 then
        qaerrorlog "This test is not adapted for polish, 48."
        got endsub
    endif
    Dim Fehler$
    Dim Sprachenname$
    Dim Dummy$
    Dim FehlerText$
    Dim i as integer

    printlog "New document"
    Call hNewDocument
    printlog "Selecting language case: " & iSprache
    select case iSprache
    case 01 : FehlerText$ = "Thatt is a failure test."                      : Fehler$ = "Thatt" : Sprachenname$ = "English (US)"
    case 34 : FehlerText$ = "Ezte es un chico muy importante."              : Fehler$ = "Ezte"  : Sprachenname$ = "Niederlaendisch"
    case 33 : FehlerText$ = "Ler nous faut donc un de temps pour examiner." : Fehler$ = "Ler"   : Sprachenname$ = "Franzoesisch"
    case 36 : FehlerText$ = "Boszniai americkai kontingens háromnegyedesek – mintegy négyezer katona – magyarországi telepítése egy éven belül megtörténhet" : Fehler$ = "tellepítésel"   : Sprachenname$ = "Ungarisch"
    case 39 : FehlerText$ = "Ringrarziamo per l'interessa mostrato a collaborare con la firma." : Fehler$ = "Ringrarziamo" : Sprachenname$ = "Italienisch"
    case 46 : FehlerText$ = "Detd varierar vad som behandlas och ur vilket perspektiv."         : Fehler$ = "Detd"         : Sprachenname$ = "Schwedisch"
    case 49 : FehlerText$ = "Diees ist ein Fehler."                         : Fehler$ = "Diees" : Sprachenname$ = "Deutsch"
    case 55 : FehlerText$ = "Eesta poderia ser a resposta para suas preces?": Fehler$ = "Eesta" : Sprachenname$ = "Portugiesisch"
    case else :
        if bAsianLan then
            printlog "For the language  " + iSprache +" nothing is prepared yet, but is AsianLan, so OK :-) using english"
            FehlerText$ = "Thatt is a failure test."
            Fehler$ = "Thatt"
            Sprachenname$ = "English (US)"
            call hSetSpellHypLanguage
        else
            Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here"
        end if
    end select
    printlog "Error Text taken for testing is: " & FehlerText$
    sleep 2

    printlog "Delete ignore word list"
    if (not wIgnorierenlisteLoeschen) then
        qaErrorLog "Can't get into Dictionary lists"
        goto endsub
    end if
    printlog "Setting doc language to english"
    ToolsOptions
    Kontext "ExtrasOptionenDlg"
    hToolsOptions("LANGUAGESETTINGS","Languages")
    Westlich.Select 34
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    printlog "Create 1 textbox with 1 spelling error (test replace always)"
    hTextrahmenErstellen (FehlerText$,30,30,90,40)

    hTypeKeys "<HOME>"
    printlog "Call 'Tools->Spellcheck->Check'"
    ToolsSpellcheck
    printlog "Spellcheck dialog has to come up, wrong word is selected."
    Kontext "Spellcheck"
    if Spellcheck.Exists then
        printlog "There has to be at least ONE suggestion."
        if (Suggestions.GetItemCount < 1) then
            printlog "If no suggestion avilable:"
            warnlog "   - no suggestion for the language: "+DictionaryLanguage.GetSelIndex+" '"+DictionaryLanguage.GetSelText+"' , press check button..."
            printlog "+ press button 'Check word'"
            Pruefen.click
            if (Suggestions.GetItemCount < 1) then
                warnlog " STILL no suggestions :-( errors will follow "
            else
                printlog "   - now suggestion for the language: "+DictionaryLanguage.GetSelIndex+" '"+DictionaryLanguage.GetSelText+"'" + Suggestions.GetItemCount
            end if
        end if
        Dummy$=Suggestions.GetItemText (1)
        printlog "Select first spellcheck suggestion (click it!)."
        Suggestions.Select (1)
        printlog "Check if textfield 'word' has changed to selected word."
        if (Suggestions.GetSelText <> Dummy$) Then
            Warnlog "Suggestion not used"
        else
            Printlog "Suggestion is used"
        end if
        sleep 1
        kontext "Spellcheck"
        printlog "click button 'Always replace'"
        ChangeAll.Click
        printlog "spellcheck dialog has to disappear and"
        printlog "There has to come up only one active: 'Spellcheck of entire document has been completed [OK]'"
        Kontext "Active"
        if Active.Exists(5) then
            Printlog "Message: Spellchecking has finished?: '" + active.gettext + "'"
            Active.OK
        else
            Printlog "'Change All' seems to have worked correctly."
            Kontext "Spellcheck"
            Spellcheck.Close
            Kontext "Active"
            if active.exists(5) then
                Printlog "Spellcheck dialog closed'" + active.gettext + "'"
                Active.OK
            else
                Printlog "Spellcheck dialog closed'"
            end if
        end if
    else
        Warnlog "  Error not recognized by the Spellchecker"
        if active.exists(5) then
            Kontext "Active"
            printlog "Message: spellchecking has finished?: '" + active.gettext + "'"
            Active.OK
        end if
    end if

    printlog "delete textbox"
    EditSelectAll
    hTypeKeys "<DELETE>"
    sleep 1

    printlog "Create same textbox again (test IGNORE function)."
    Printlog "Check function Ignore"
    hTextrahmenErstellen (FehlerText$,30,30,80,40)
    printlog "All 'Tools->Spellcheck->Check'."

    'printlog "Setting Text to english"
    'sleep 1
    'EditSelectAll
    'FormatCharacter
    'sleep 1
    'Kontext
    'Messagebox.SetPage TabFont
    'Kontext "TabFont"
    'Language.Select 41
    'TabFont.OK

    ToolsSpellcheck
    Kontext "Spellcheck"
    printlog "press button 'Ignore'"
    IgnoreOnce.Click
    printlog "spellcheck dialog has to disappear and "
    printlog " There has to come up only one active: 'Spellcheck of entire document has been completed [OK]'."
    Kontext "Active"
    if Active.Exists(5) Then
        Printlog " Spellcheck ended because of only 1 defined error. And Ignore worked.'" + active.gettext + "'"
        Active.OK
    else
        Printlog " 'Ignore Once' seems to work correctly."
        Kontext "Spellcheck"
        Spellcheck.Close
        Kontext "Active"
        if active.exists(5) then
            Printlog " Spellcheck dialog closed'" + active.gettext + "'"
            Active.OK
        else
            Printlog " Spellcheck dialog closed'"
        end if
    end if

    printlog "Call 'Tools->Spellcheck->Check."
    ToolsSpellcheck
    Kontext "Spellcheck"
    if Spellcheck.Exists Then
        Printlog "  Ignore worked"
        Spellcheck.Close
        Kontext "Active"
        if active.exists(5) then
            Printlog " " + active.gettext + "'"
            Active.OK
        else
            Printlog " Spellcheck dialog closed'"
        end if
    else
        Warnlog "  Spellcheck ended even we only ignored the error"
    end if

    printlog "delete textbox."
    EditSelectAll
    hTypeKeys "<DELETE>"
    sleep 1

    printlog "create same textbox again (test ALWAYS IGNORE function)."
    hTextrahmenErstellen (FehlerText$,30,30,60,40)

    'printlog "Setting Text to english"
    'sleep 1
    'EditSelectAll
    'FormatCharacter
    'sleep 1
    'Kontext
    'Messagebox.SetPage TabFont
    'Kontext "TabFont"
    'Language.Select 41
    'TabFont.OK

    printlog "Call 'Tools->Spellcheck->Check'."
    ToolsSpellcheck
    Kontext "Spellcheck"
    printlog "click button 'Always Ignore."
    IgnoreAll.Click
    printlog "spellcheck dialog has to disappear and"
    printlog "There has to come up only one active: 'Spellcheck of entire document has been completed [OK]'."
    Kontext "Active"
    if active.exists(5) then
        Printlog " Spellcheck ended because of only 1 defined error. And Ignore worked.'" + active.gettext + "'"
        Active.OK
    else
        Printlog " 'Ignore All' seems to work."
        Kontext "Spellcheck"
        Spellcheck.Close
        Kontext "Active"
        if active.exists(5) then
            Printlog " Spellcheck dialog closed'" + active.gettext + "'"
            Active.OK
        else
            Printlog " Spellcheck dialog closed'"
        end if
    end if

    Printlog "- Delete ignore list"
    sleep 1
    printlog "Delete ignore word list."
    if (not wIgnorierenlisteLoeschen) then
        qaErrorLog "Can't get into Dictionary lists"
        goto endsub
    end if
    Call hCloseDocument
endcase 'tiToolsSpellcheckError

'------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
testcase tiToolsSpellcheckCheck
    if iSprache = 48 then
        qaerrorlog "This test is not adapted for polish, 48."
        got endsub
    endif
    Dim Datei$
    Dim sWord(2) as string
    Dim i as integer
    Dim j as integer
    Dim s as integer
    Dim AlleBuecher as integer
    Dim sExt as string
    Dim sWordOne as string
    Dim sWordTwo as string
    Dim iBooks as integer
    Dim bWordFound(2) as boolean
    Dim iSuggestions as integer
    Dim iWord(2) as integer
    Dim bFound as boolean

    Select Case Ucase(gApplication)
    case "DRAW"         : sExt = ".odg"
    case "IMPRESS"      : sExt = ".odp"
    end select

    printlog "Load prepared document containing 4 errors : graphics\\required\\input\\recht_" & iSprache & sExt
    if (not bAsianLan) then
        printlog "Check if the document is writable."
        Call hFileOpen (gTesttoolpath + "graphics\required\input\recht_"+iSprache+sExt)
    else
        Call hFileOpen (gTesttoolpath + "graphics\required\input\recht_1"+sExt)
    end if
    if fIsDocumentWritable = false then
        printlog "Make the document writable and check if it's succesfull."
        if fMakeDocumentWritable = false then
            warnlog "The document can't be make writeable. Test stopped."
            goto endsub
        end if
    end if
    select case iSprache  ' sWord(1)=red     : sWord(2)=turquoise
    case 01 : sWord(1) = "documente"     : sWord(2) = "expriss"
    case 33 : sWord(1) = "intercu"       : sWord(2) = "Lees"
    case 34 : sWord(1) = "afekto"        : sWord(2) = "fratternal"
    case 36 : sWord(1) = "szeerint"       : sWord(2) = "tervvezi"
    case 39 : sWord(1) = "Millano"       : sWord(2) = "tarrget"
    case 46 : sWord(1) = "desa"          : sWord(2) = "occh"
    case 49 : sWord(1) = "Texxt"         : sWord(2) = "reichtt"
    case 55 : sWord(1) = "esktava"         : sWord(2) = "noitee"
    case else :
        if bAsianLan then
            sWord(1) = "documente"     : sWord(2) = "expriss"
        else
            Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here and create the file"
        end if
    end select
    sleep 2

    Printlog "Delete all added words from dictionaries."
    printlog "Call Tools->Options."
    ToolsOptions
    printlog "Select in category 'Languagesettings' entry 'Writing Aids.'"
    hToolsOptions ("LANGUAGESETTINGS","WRITINGAIDS")
    sleep 1
    Kontext "WRITINGAIDS"
    sleep 1
    printlog "Click on button 'edit' in section 'User-defined dictionaries.'"
    if (fGetIntoDictionary) then
        qaErrorLog "wTSC"
        goto endsub
    end if
    Kontext "BenutzerwoerterbuchBearbeiten"
    sleep 1
    printlog "Check every book, if it contains the words that will be added in this test."
    iBooks = Buch.getItemCount
    bWordFound(1) = false
    bWordFound(2) = false
    for i = 1 to iBooks
        Buch.select(i)
        printlog "Items in Booklist: " & WordList.getItemCount
        for j = 1 to 2
            Wort.setText sWord(j)
            sleep 1
            if ((not neu.isEnabled) and Loeschen.isEnabled) then
                printlog "If it contains the word, press button 'delete'."
                Loeschen.click
                bWordFound(j) = true
                printlog "Added word WAS in dictionary: '" + Buch.getSelText + "' - '" + sWord(j) + "'"
            end if
        next j
    next i
    if (bWordFound(1) OR bWordFound(2))then
        qaErrorLog "Word was found in dictionary - check why it was there. '" + sWord(1) + "': " + bWordFound(1) + "  '" + sWord(2) + "': " + bWordFound(2)
    end if
    printlog "Close dialog 'Edit Custom Dictionary.'"
    BenutzerwoerterbuchBearbeiten.Close
    sleep 1
    Kontext "ExtrasOptionenDlg"
    printlog "Press button 'OK' on dialog 'Writing Aids'."
    ExtrasOptionenDlg.OK
    printlog "Call dialog again and delete all remaining words from dictionary 'IgnoreAllList'."
    if (not wIgnorierenlisteLoeschen) then
        qaErrorLog "Can't get into Dictionary lists"
        goto endsub
    end if

    printlog "Test if spellcheck dialog comes up and check/set direction of spellcheck."
    printlog "Call 'Tools->Spellcheck->Check'."
    ToolsSpellcheck
    Kontext "Spellcheck"
    WaitSlot (1000)
    printlog "If no dictionary for the language is available, a messagebox comes up:"
    printlog ". . . 'Error executing the spellcheck.: Language is not supported by spellchecker funtion."
    kontext "active"
    if active.exists(5) then
        warnlog "$Language is not supported by spellchecker funtion: '" + active.gettext + "'"
        Active.OK
        printlog ". . . exiting testcase."
        goto endsub
    end if
    Kontext "Spellcheck"
    printlog "Close dialog 'Spellcheck'."
    Spellcheck.Close
    Kontext "Active"
    if active.exists(5) then
        Warnlog " Should not be any message here: " + active.gettext + "'"
        Active.OK
    else
        printlog "Spellcheck ended, dialog closed"
    end if
    printlog "Select all."
    hTypeKeys "<MOD1 A>"
    printlog "Check presupposition: 12 Words have to be complained about."
    printlog "All 'Tools->Spellcheck->Check'."
    ToolsSpellcheck
    WaitSlot (2000)
    Kontext "Spellcheck"
    printlog "Click button 'Ignore' 12 times."

    for i = 1 to 11
        Kontext "Spellcheck"
        IgnoreOnce.Click
        Kontext "Active"
        if active.exists(5) then
            warnlog "Presupposition not met: there are less than 12 errors in the document! " + i
            Active.OK
            printlog "If errors < 12 -> exiting testcase."
            goto endsub
        end if
    next i
    Kontext "Spellcheck"
    IgnoreOnce.Click
    printlog "Spellcheck dialog has to disapear and messagebox with OK has to come up."
    Kontext "Active"
    if active.exists(5) then
        printlog "Active dialog said: " + active.gettext + "'"
        Active.OK
    end if
    Kontext "Spellcheck"
    if Spellcheck.Exists(5) then
        warnlog "Presupposition not met: there are more than 12 errors in the document!"
        Spellcheck.Close
        kontext "Active"
        if active.exists(5) then
            Active.OK
        else
            printlog "bug fixed #111972# "
        end if
        printlog "If errors > 12 -> exiting testcase."
        goto endsub
    else
        Kontext "Active"
        if active.exists(5) then
            warnlog "There was a Message where none was supposed to be: '" + active.gettext + "'"
            Active.NO
        end if
        printlog "Presupposition met: there are 12 errors in the document!"
    end if

    printlog "Perform the test now:"
    printlog "Call 'Tools->Spellcheck->Check'"
    ToolsSpellcheck
    WaitSlot (2000)
    Kontext "Spellcheck"
    Printlog "----------------------------------------------------------------------------"
    Printlog "1st Test:   - Ignore now"
    printlog "1st error: ignore : 1st pink word in 1st Paragraph."
    printlog "Backwards: last green word in 3rd Paragraph."
    sWordOne = Suggestions.GetSelText
    printlog "********* Suggestion word found: '" + sWordOne + "'"
    printlog "##### suggestions: "+Suggestions.GetItemCount+"; Language: "+DictionaryLanguage.getSelText '+" ; dictionary: "+woerterbuch.getSelText
    if (Suggestions.GetItemCount > 0) then
        printlog "----- "+i+": "+Suggestions.GetSelText (1)
    end if
    printlog "Click button 'Ignore'."
    IgnoreOnce.Click
    Sleep 2

    Printlog "----------------------------------------------------------------------------"
    Printlog "2nd Test:   - Add"
    printlog "2nd error: add : 1st red word in 1st Paragraph -> hasn't to show up anymore from now on."
    printlog "Backwards: last turquoise word in 3rd Paragraph -> hasn't to show up anymore from now on."
    printlog "Check if word in textfield 'Word' changed."
    printlog "(Check if it is the expected next error - you have to look into the source code for the right word!)."
    sWordTwo = Suggestions.GetSelText
    if (sWordOne = sWordTwo) then
        warnlog "Ignore didn't work? Spellcheck didn't go on"
    end if
    if (sWord(iWord(1)) <> sWordTwo) then
        Printlog "The errornous word '" + sWord(iWord(1)) + "' would be replaced with: '" + sWordTwo + "'"
    end if
    printlog "********* word found: '" + sWordTwo + "'"
    printlog "##### suggestions: "+Suggestions.GetItemCount+"; Language: "+DictionaryLanguage.getSelText ' +" ; dictionary: "+woerterbuch.getSelText
    if (Suggestions.GetItemCount > 0) then
        printlog "----- "+i+": "+Suggestions.GetSelText (1)
    end if
    Sleep 1
    printlog "Click button 'Add' on dialog '"
    AddToDictionary.Click
    Sleep 2
    printlog "The menu has: " + MenuGetItemCount + " entries."
    hMenuSelectNr(1) 'Default
    Kontext "Active"
    if Active.Exists(5) Then
        Warnlog " - Word could not be added to dictionary: '" + active.getText + "'"
        Active.OK
        Sleep 1
    end if
    printlog "Check in options, if word exists in word list."
    printlog "Click button 'Options' on dialog 'Spellcheck'."
    Kontext "Spellcheck"
    SpellcheckOptions.Click
    Kontext "TabLinguistik"
    printlog "Click button 'Edit ...' on dialog 'Writing Aids' in section 'User-defined dictionaries'."
    if TabLinguistik.exists(5) then
        sleep 3
        if (fGetIntoDictionary) then
            qaErrorLog "wTSC"
            goto endsub
        end if
    else
        qaerrorlog "baeh"
    end if
    Kontext "BenutzerwoerterbuchBearbeiten"
    printlog "Check every book, if it contains the added word."
    if not BenutzerwoerterbuchBearbeiten.exists(5) then
        sleep 5
        qaerrorlog "baeh"
    end if
    iBooks = Buch.getItemCount
    bWordFound(1) = false
    for i = 1 to iBooks
        Buch.select(i)
        printlog "Book number selected: " & i
        Wort.setText sWord(iWord(1))
        sleep 1
        if ((not neu.isEnabled) and Loeschen.isEnabled) then
            bWordFound(1) = true
            printlog "Added word is in dictionary: '" + Buch.getSelText + "'"
        end if
    next i
    if (not bWordFound(1)) then
        warnlog "Word was not added to dictionary"
    end if
    printlog "Cancel dialog 'Edit Custom Dictionary'."
    BenutzerwoerterbuchBearbeiten.Close
    Kontext "TabLinguistik"
    printlog "Cancel dialog 'Writing Aids'."
    TabLinguistik.Close
    Kontext "Spellcheck"

    Printlog "----------------------------------------------------------------------------"
    Printlog " 3rd Test:  - Always Ignore"
    printlog "3rd error: always ignore : 1st turquoise word in 1st Paragraph -> hasn't to show up anymore from now on."
    printlog "Check if word in textfield 'Word' changed."
    printlog "(check if it is the expected next error - you have to look into the source code for the right word!)"
    sWordOne = sWordTwo
    sWordTwo = Suggestions.GetSelText 'wort.getText
    if (sWordOne = sWordTwo) then
        warnlog "Add didn't work? Spellcheck didn't go on."
    end if
    if (sWord(iWord(2)) <> sWordTwo) then
        Printlog "The erroneous word '" + sWord(iWord(2)) + "' would be replaced with: '" + sWordTwo + "'"
    else
        warnlog "ERROR: SAME WORD in the dictionary as in the text??? Must be wrong."
    end if
    printlog "********* word found: '" + sWordTwo + "'"
    printlog "##### suggestions: "+Suggestions.GetItemCount+"; Language: "+DictionaryLanguage.getSelText  ' +" ; dictionary: "+DictionaryLanguage.getSelText   'Wort.GetItemCount  'Woerterbuch.GetSelText
    if (Suggestions.GetItemCount > 0) then  'Wort.GetItemCount > 0) then
        printlog "----- "+i+": "+Suggestions.GetItemText (1)  'Wort.GetItemText (1)
    end if
    Sleep 1
    printlog "Click button 'Always ignore' on dialog."
    IgnoreAll.Click
    Sleep 2
    printlog "Check in options, if word exists in word list."
    printlog "Click button 'Options' on dialog 'Spellcheck'."
    SpellcheckOptions.Click
    Kontext "TabLinguistik"
    printlog "Click button 'Edit ...' on dialog 'Writing Aids' in section 'User-defined dictionaries."
    if TabLinguistik.exists(5) then
        sleep 3 'culprint swedish windows; wait until butrton exists?
        if (fGetIntoDictionary) then
            qaErrorLog "wTSC"
            goto endsub
        end if
    else
        qaerrorlog "baeh"
    end if
    Kontext "BenutzerwoerterbuchBearbeiten"
    if not BenutzerwoerterbuchBearbeiten.exists(5) then
        sleep 3
        qaerrorlog "baeh"
    end if
    printlog "Check every book, if it contains the added word."
    iBooks = Buch.getItemCount
    bWordFound(2) = false
    for i = 1 to iBooks
        Kontext "BenutzerwoerterbuchBearbeiten"
        Buch.select(i)
        Inhalt.setText sWord(iWord(2)) 'Wort.setText sWord(iWord(2))
        sleep 1
        if ((not neu.isEnabled) and Loeschen.isEnabled) then
            bWordFound(2) = true
            printlog " added word is in dictionary: '" + Buch.getSelText + "'"
        end if
    next i
    if (not bWordFound(2)) then
        warnlog "Word was not added to dictionary, #ixxxxxx"
    end if
    printlog "Cancel dialog 'Edit Custom Dictionary'."
    BenutzerwoerterbuchBearbeiten.Close
    Kontext "TabLinguistik"
    printlog "Cancel dialog 'Writing Aids'."
    TabLinguistik.Close
    kontext "Spellcheck"

    Printlog "----------------------------------------------------------------------------"
    Printlog "4th test:   - Replace"
    printlog "4th error: replace : 1st green word in 1st Paragraph."
    printlog "Check if word in textfield 'Word' changed."
    printlog "(check if it is the expected next error - you have to look into the source code for the right word!)."
    sWordOne = sWordTwo
    sWordTwo = Suggestions.GetSelText
    if (sWordOne = sWordTwo) then
        warnlog "Always ignore didn't work? Spellcheck didn't go on."
    end if
    printlog "********* word found: '" + sWordTwo + "'"
    iSuggestions = Suggestions.GetItemCount
    printlog "##### suggestions: " + iSuggestions + "; Language: "+DictionaryLanguage.getSelText  '+"; dictionary: "+woerterbuch.getSelText
    if (Suggestions.GetItemCount > 0) then
        printlog "----- "+i+": "+Suggestions.GetItemText (1)
    end if
    Sleep 1
    printlog "Click button 'Replace'."
    if (iSuggestions > 0) then
        Change.click
    else
        qaerrorlog "Please change the text in the file, so the spellchecker can make a suggestion for the word: '" + sWordTwo + "'"
        IgnoreOnce.Click
    end if

    Printlog "----------------------------------------------------------------------------"
    Printlog "5th Test:   - Always Replace"
    printlog "5th error: always replace : 1st pink word in 2nd Paragraph -> hasn't to show up anymore from now on."
    printlog "backwards: 1st green word in 2nd Paragraph -> hasn't to show up anymore from now on."
    printlog "check if word in textfield 'Word' changed."
    printlog "(check if it is the expected next error - you have to look into the source code for the right word!)."
    sWordOne = sWordTwo
    sWordTwo = Suggestions.GetSelText
    if (sWordOne = sWordTwo) then
        warnlog "Replace didn't work? Spellcheck didn't go on"
    end if
    printlog "********* word found: '" + sWordTwo + "'"
    iSuggestions = Suggestions.GetItemCount
    printlog "##### suggestions: " + iSuggestions + "; Language: "+DictionaryLanguage.getSelText  ' +"   ; dictionary: "+woerterbuch.getSelText
    if (Suggestions.GetItemCount > 0) then
        printlog "----- "+i+": "+Suggestions.GetItemText (1)
    end if
    Sleep 1
    printlog "click button 'Always Replace'."
    if (iSuggestions > 0) then
        ChangeAll.click
    else
        qaErrorLog "Please change the text in the file, so the spellchecker can make a suggestion for the word: '" + sWordTwo + "'"
        IgnoreOnce.Click
    end if

    printlog "2 errors are left: 4th word (green) in 2nd and 3rd paragraph."
    printlog "backwards: 1st word (pink) in 2nd and 1st paragraph."
    Kontext "Spellcheck"
    sWordOne = sWordTwo
    sWordTwo = Suggestions.GetSelText
    if (sWordOne <> sWordTwo) then
        printlog sWordTwo
    else
        warnlog "there is anopther word left, that wasn't expected!. '" + sWordTwo +"'"
    end if
    printlog "Click button 'Ignore' 2 times."
    IgnoreOnce.Click

    Kontext "Spellcheck"
    sWordOne = sWordTwo
    sWordTwo = Suggestions.GetSelText
    if (sWordOne <> sWordTwo) then
        warnlog "there is anopther word left, that wasn't expected!. '" + sWordTwo +"'"
    else
        printlog sWordTwo
    end if
    printlog "Click button 'Ignore' 2 times."
    IgnoreOnce.Click
    Kontext "Active"
    if active.exists(5) then
        printlog "Spellcheck works :-) '" + active.gettext + "'"
        Active.No
    else
        warnlog "Spellcheck didn't work :-(! there are still errors in the document."
        Kontext "Spellcheck"
        Spellcheck.Close
        Kontext "Active"
        if active.exists(5) then
            qaErrorLog "    Spellcheck dialog closed'" + active.gettext + "'"
            Active.No
        end if
    end if
    sleep 2

    Printlog "Delete all added words from dictionaries."
    printlog "Call Tools->Options."
    ToolsOptions
    printlog "Select in category 'Languagesettings' entry 'Writing Aids'."
    hToolsOptions ("LANGUAGESETTINGS","WRITINGAIDS")
    sleep 1
    Kontext "WRITINGAIDS"
    printlog "Click on button 'edit' in section 'User-defined dictionaries'."
    if (fGetIntoDictionary) then
        qaErrorLog "wTSC"
        goto endsub
    end if
    Kontext "BenutzerwoerterbuchBearbeiten"
    printlog "Check every book, if it contains the added word."
    iBooks = Buch.getItemCount
    bWordFound(1) = false
    bWordFound(2) = false
    for i = 1 to iBooks
        Buch.select(i)
        for j = 1 to 2
            Wort.setText sWord(j)
            sleep 1
            if ((not neu.isEnabled) and Loeschen.isEnabled) then
                printlog "If it contains the word, press button 'delete'."
                Loeschen.click
                bWordFound(j) = true
                printlog " added word is in dictionary: '" + Buch.getSelText + "' - '" + sWord(j) + "'"
            end if
        next j
    next i
    if ((not bWordFound(1)) AND (not bWordFound(2)))then
        warnlog "Word was not found in dictionary. '" + sWord(1) + "': " + bWordFound(1) + "  '" + sWord(2) + "': " + bWordFound(2)
    end if
    printlog "Close dialog 'Edit Custom Dictionary'."
    BenutzerwoerterbuchBearbeiten.Close
    sleep 1
    Kontext "ExtrasOptionenDlg"
    printlog "press button 'OK' on dialog 'Writing Aids'."
    ExtrasOptionenDlg.OK
    printlog "Call dialog again and delete all remaining words from dictionary 'IgnoreAllList'."
    if (not wIgnorierenlisteLoeschen) then
        qaErrorLog "Can't get into Dictionary lists"
        goto endsub
    end if

    printlog "Close document"
    Call hCloseDocument
endcase 'tiToolsSpellcheckCheck

'-------------------------------------------------------------------------------
testcase tToolsSpellcheckAutoSpellcheck

    QaErrorLog  "#i81928# - outcommented tToolsSpellcheckAutoSpellcheck due to bug."
    goto endsub
    dim i as integer
    dim x as integer
    dim y as integer
    dim q as integer
    dim z as integer
    dim iResult as long
    dim iTemp as long
    dim iTemp2 as long
    dim sTemp as string
    dim sCompare as string
    dim iCompare as long
    dim iError as long
    dim sError as string

    call hNewDocument

    call hTextrahmenErstellen ("Ein Tipp: Schiffahrt schreibt man nun mit 3f Tunfisch Amboss a", 10, 10, 90, 50)

    EditSelectAll
    setCharacterLanguage(glLocale(4))
    sleep 10
    printlog "## check ENGLISH auto spellchecking"
    iError = 0
    ' Get underlined words / wrong recognized words by spellchecker
    iResult = sAnalyseContextMenu(11, iError)
    sTemp = sLongToBinary(iResult, 11)
    sError = sLongToBinary(iError, 11)
    ' reference of words, which should be underlined
    sCompare = "11011001011"
    iCompare = sBinaryToLong(sCompare)
    ' compare result with reference -> get the difference
    iTemp = not (iResult EQV iCompare)
    ' eliminate errors from open bugs -> get the real errors
    iTemp2 = iTemp AND NOT iError
    if (iTemp2 > 0) then
        warnlog "wrong words are not underlined? Should be: " + sCompare
        warnlog "Is:                                        " + sTemp
        warnlog "Differences:                               " + sLongToBinary(iTemp, 11)
        warnlog "Wrong after merging errors from bugs       " + sLongToBinary(iTemp2, 11)
    end if

    sleep 1
    '    call hTypeKeys "<F2>"
    call hTypeKeys "<mod1 end> <Shift mod1 home>"
    setCharacterLanguage(glLocale(6))
    sleep 10
    printlog "## check GERMAN auto spellchecking"
    iError = 0
    iResult = sAnalyseContextMenu(11, iError)
    sTemp = sLongToBinary(iResult, 11)
    sError = sLongToBinary(iError, 11)
    sCompare = "00010000000"
    iCompare = sBinaryToLong(sCompare)
    ' compare result with reference -> get the difference
    iTemp = not (iResult EQV iCompare)
    ' eliminate errors from open bugs -> get the real errors
    iTemp2 = iTemp AND NOT iError
    if (iTemp2 > 0) then
        warnlog "wrong words are not underlined? Should be: " + sCompare
        warnlog "Is:                                        " + sTemp
        warnlog "Differences:                               " + sLongToBinary(iTemp, 11)
        warnlog "Wrong after merging errors from bugs       " + sLongToBinary(iTemp2, 11)
    end if
    printlog "-----------------------------------"

    hCloseDocument()
endcase 'tToolsSpellcheckAutoSpellcheck

'-------------------------------------------------------------------------------
