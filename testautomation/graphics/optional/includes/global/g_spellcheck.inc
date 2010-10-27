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
'* short description :
'*
'\*******************************************************************
testcase tiToolsSpellcheckCheck_AlwaysIgnore
    
    Dim sExt as string
    
    Dim sWrongWord as String
    Dim sRightWord as string

    dim aWords(2) as string
    aWords() = getWordsForLanguage()

    sWrongWord = aWords(0)
    sRightWord = aWords(1)
    
    if(sWrongWord = "" OR sRightWord = "") then ' if any word is empty then the test should stop
        qaerrorlog "no spellcheck test for the langauges " + iSprache + " available."            
        goto endsub
    endif

    delete_word_from_dictionary(sWrongWord,"IgnoreAllList")

    if Ucase(gApplication) = "DRAW" then
        sExt = ".odg"
    else
        sExt = ".odp"
    endif

    Call hFileOpen (gTesttoolpath + "graphics\required\input\recht_"+iSprache+sExt)
    call fMakeDocumentWritable()

    sleep(1)

    ' Perform the test now:
    printlog "Call Tools->Spellcheck"
    ToolsSpellcheck    
    sleep(2)
    Kontext "Spellcheck"
    printlog "Click button 'Always ignore' on dialog."
    IgnoreAll.Click
    Sleep 2    

    ToolsOptions
    printlog "Select in category 'Languagesettings' entry 'Writing Aids.'"
    hToolsOptions ("LANGUAGESETTINGS","WRITINGAIDS")

    fSelectWithString(Benutzerwoerterbuch,"IgnoreAllList")

    Bearbeiten.Click

    Kontext "BenutzerwoerterbuchBearbeiten"
    if(Inhalt.getText <> sWrongWord) then
        warnlog " added word is not in dictionary: '" + Buch.getSelText + "'"
    else
        printlog " added word is in dictionary: '" + Buch.getSelText + "'"
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
    case 50   : hTextrahmenErstellen ("To je besedilo brez napake.<Return>",10,10,50,20)
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
    case 50 : FehlerText$ = "Toje napaka, ker manjka presledek."            : Fehler$ = "Toje" : Sprachenname$ = "Slovenš�?ina"
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

    printlog "Close dialog 'Edit Custom Dictionary'."
    BenutzerwoerterbuchBearbeiten.Close
    Kontext "ExtrasOptionenDlg"
    printlog "Close the Option dialog."
    ExtrasOptionenDlg.OK

    printlog "Close document"
    Call hCloseDocument

    ' remove the word again from the dictionary
    delete_word_from_dictionary(sWrongWord,"IgnoreAllList")

endcase 'tiToolsSpellcheckCheck
'-------------------------------------------------------------------------------
testcase tiToolsSpellcheckCheck_Change

    Dim sExt as string
    Dim sWrongWord as String
    Dim sRightWord as string

    dim aWords(2) as string
    aWords() = getWordsForLanguage()

    sWrongWord = aWords(0)
    sRightWord = aWords(1)
    
    if(sWrongWord = "" OR sRightWord = "") then ' if any word is empty then the test should stop
        qaerrorlog "no spellcheck test for the langauges " + iSprache + " available."            
        goto endsub
    endif

    if Ucase(gApplication) = "DRAW" then
        sExt = ".odg"
    else
        sExt = ".odp"
    endif

    Call hFileOpen (gTesttoolpath + "graphics\required\input\recht_"+iSprache+sExt)

    call fMakeDocumentWritable

    ' Perform the test now:
    printlog "Call Tools->Spellcheck"
    ToolsSpellcheck
    sleep(2)
    Kontext "Spellcheck"
    printlog "Click button 'Change' on dialog."
    Change.Click
    sleep(10)
    SpellcheckClose.click

    if Ucase(gApplication) = "DRAW" then
        Kontext "DocumentDraw"
        DocumentDraw.TypeKeys("<TAB>",true)
        DocumentDraw.TypeKeys("<SPACE>",true)        
    else
        Kontext "DocumentImpress"
        DocumentImpress.TypeKeys("<TAB>",true)
        DocumentImpress.TypeKeys("<SPACE>",true)
    endif

    EditSelectAll
    EditCopy
    
    dim s as string
    s = getClipBoard()

    printlog s

    if(Instr(s,sWrongWord) <> 0) then    	
        warnlog "word not changed. '" + s + "' contains still wrong word '" + sWrongWord + "'"
    else
        printlog "word changed."
    endif

    if(Instr(s,sRightWord) <> 0) then
        printlog "word changed."
    else
        warnlog "word not changed."
    endif
    
    printlog "Close document"
    Call hCloseDocument

endcase
'-------------------------------------------------------------------------------
testcase tiToolsSpellcheckCheck_ChangeAll

    Dim sExt as string
    Dim sRightWord as String
    Dim sWrongWord as string

    dim aWords(2) as string
    aWords() = getWordsForLanguage()

    sWrongWord = aWords(0)
    sRightWord = aWords(1)
    
    if(sWrongWord = "" OR sRightWord = "") then ' if any word is empty then the test should stop
        qaerrorlog "no spellcheck test for the langauges " + iSprache + " available."
        goto endsub
    endif

    'delete_word_from_dictionary(sWord,"IgnoreListAll")

    if Ucase(gApplication) = "DRAW" then
        sExt = ".odg"
    else
        sExt = ".odp"
    endif

    Call hFileOpen (gTesttoolpath + "graphics\required\input\recht_"+iSprache+sExt)

    call fMakeDocumentWritable()

    ' Perform the test now:
    printlog "Call Tools->Spellcheck"
    ToolsSpellcheck
    sleep(2)
    Kontext "Spellcheck"
    printlog "Click button 'Change' on dialog."
    ChangeAll.Click
    'TODO: check result beware of issue 110688
    Sleep 2

    SpellCheckClose.click

    printlog "Close document"
    Call hCloseDocument

endcase
'-------------------------------------------------------------------------------
function delete_all_added_words(aWords)

    '/// this function delete the words in sWords in all user dictionaries

    Dim iBookCounter as integer
    Dim iWordCounter as integer
    Dim iBooks as integer

    Printlog "Delete the given words from dictionaries."
    printlog "Call Tools->Options."
    ToolsOptions
    printlog "Select in category 'Languagesettings' entry 'Writing Aids.'"
    hToolsOptions ("LANGUAGESETTINGS","WRITINGAIDS")
    sleep 1
    Kontext "WRITINGAIDS"
    sleep 1
    printlog "select the first User-defined dictionaries and click EDIT"
    Benutzerwoerterbuch.select 1
    Bearbeiten.Click

    Kontext "BenutzerwoerterbuchBearbeiten"
    sleep 1
    printlog "Check every book, if it contains the words."
    iBooks = Buch.getItemCount
    for iBookCounter = 1 to iBooks
        Buch.select(iBookCounter)
        printlog "Items in Booklist: " & WordList.getItemCount
        for iWordCounter = 1 to uBound(aWords)
            Wort.setText aWords(iWordCounter)
            sleep 1
            if ((not neu.isEnabled) and Loeschen.isEnabled) then
                printlog "If it contains the word, press button 'delete'."
                Loeschen.click
                printlog "Added word WAS in dictionary: '" + Buch.getSelText + "' - '" + aWords(iWordCounter) + "'"
            end if
        next iWordCounter
    next iBookCounter
    printlog "Close dialog 'Edit Custom Dictionary.'"
    BenutzerwoerterbuchBearbeiten.Close
    sleep 1
    Kontext "ExtrasOptionenDlg"
    printlog "Press button 'OK' on dialog 'Writing Aids'."
    ExtrasOptionenDlg.OK

end function 'delete_all_added_words
   
'-------------------------------------------------------------------------------
function delete_word_from_dictionary(sWord as String, sDictionary as String)

    '/// this function delete the words in sWords in all user dictionaries

    Dim iBookCounter as integer
    Dim iWordCounter as integer
    Dim iBooks as integer

    Printlog "Delete the given word from dictionaries."
    printlog "Call Tools->Options."
    ToolsOptions
    printlog "Select in category 'Languagesettings' entry 'Writing Aids.'"
    hToolsOptions ("LANGUAGESETTINGS","WRITINGAIDS")
    sleep 1
    Kontext "WRITINGAIDS"
    sleep 1
    printlog "select the dictionary and click EDIT"

    fSelectWithString(Benutzerwoerterbuch,sDictionary)

    Bearbeiten.Click

    Kontext "BenutzerwoerterbuchBearbeiten"

    Wort.setText sWord
    sleep 1
    if ((not neu.isEnabled) and Loeschen.isEnabled) then
        printlog "If it contains the word, press button 'delete'."
        Loeschen.click
        printlog "Added word WAS in dictionary: '" + Buch.getSelText + "' - '" + sWord + "'"
    end if
        
    printlog "Close dialog 'Edit Custom Dictionary.'"
    BenutzerwoerterbuchBearbeiten.Close
    sleep 1
    Kontext "ExtrasOptionenDlg"
    printlog "Press button 'OK' on dialog 'Writing Aids'."
    ExtrasOptionenDlg.OK

end function

function fSelectWithString(oControl as Object,sText as String)

    dim iCounter as Integer
    dim iNumberOfItems as Integer
    iNumberOfItems = oControl.getItemCount()

    for iCounter = 1 to iNumberOfItems
        oControl.select iCounter
        if( Instr(oControl.getSelText,sText) <> 0 ) then ' the correct entry is selected
            iCounter = iNumberOfItems ' stop the loop
        endif
    next

end function


function getWordsForLanguage()

    dim sWrongWord as String
    dim sRightWord as String
    dim aWords(2) as String

    select case iSprache
        case 1 :
                sWrongWord = "documente"
                sRightWord = "document"
        case 31 :
                sWrongWord = "miejn"
                sRightWord = "mijn"
        case 33 :
                sWrongWord = "projjet"
                sRightWord = "projet"
        case 34 :
                sWrongWord = "lazsos"
                sRightWord = "lazos"
        case 36 :
                sWrongWord = "Bozniai"
                sRightWord = "Boszniai"
        case 39 :
                sWrongWord = "borrdo"
                sRightWord = "borro"
        case 46 :
                sWrongWord = "desa"
                sRightWord = "edas"
        case 48 :
                sWrongWord = "werrsji"
                sRightWord = "wersji"
        case 49 :
                sWrongWord = "Feehlern"
                sRightWord = "Fehlern"
        case 55 :
                sWrongWord = "Elle"
                sRightWord = "Elze"
        case else :
                sWrongWord = ""
                sRightWord = ""
    end select

    aWords(0) = sWrongWord
    aWords(1) = sRightWord

    getWordsForLanguage = aWords

end function
