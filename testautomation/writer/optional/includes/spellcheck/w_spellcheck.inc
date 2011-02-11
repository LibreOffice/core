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
'* short description : Spellcheck-Testing. Functional-Test.
'*
'\***********************************************************************

sub w_spellcheck

    Call tToolsSpellcheckWithShortcut
    Call tToolsSpellcheckCorrect
    Call tToolsSpellcheck1Mistake
    Call tToolsSpellcheckForward
    Call tToolsSpellcheckOnlyOneWord
    Call tToolsSpellcheckAutomatic
    Call tToolsSpellcheckChangeLanguageAttribute
    Call tToolsThesaurus
    Call tToolsSpellcheckButtonChange
    Call tToolsSpellcheckButtonChangeAll
    Call tToolsThesaurusSynonyms

end sub


testcase tToolsSpellcheckWithShortcut
    Dim TheFile as String
    '/// Check if Spellcheck dialog comes up with shortcut 'F7'
    printlog " Open document where the formatting is set to the language we wish to test."
    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if FileExists ( TheFile ) <> TRUE then
       Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
       exit sub
    end if

    call hFileOpenLocally (TheFile,false)
    

    printlog " Insert some faulty text"
    select case iSprache
        case 01 : Call wTypeKeys ("This is a spellchecktestttttt<Home>")
        case 33 : Call wTypeKeys ("Je Suiss la Penneeee<Home>")
        case 34 : Call wTypeKeys ("Habla Espanollll<Home>")
        case 39 : Call wTypeKeys ("Don Cara Mio, capitoliooo?<Home>")
        case 49 : Call wTypeKeys ("Dies ist ein Rechtschreibtexxxxt<Home>")
        case 46 : Call wTypeKeys ("Det här är en rättstavningstexxxxt<Home>")
        case 50 : Call wTypeKeys ("To je test �?rkvana<Home>")
        case 07 : Printlog "- No Spellcheck in Russian!"
                     goto endsub
        case 48 : Printlog "- No Spellcheck in Polish!"
                     goto endsub
        case 55 : Printlog "- No Spellcheck in Brazilian!"
                     goto endsub
        case 81 : Printlog "- No Spellcheck in Japanese!"
                     goto endsub
        case 82 : Printlog "- No Spellcheck in Korean!"
                     goto endsub
        case 86 : Printlog "- No Spellcheck in Chinese (simplified)!"
                     goto endsub
        case 88 : Printlog "- No Spellcheck in Chinese (traditional)!"
                     goto endsub
        case 03, 30, 35, 36, 37, 42, 47, 51, 90
                  Printlog "- No Spellcheck available !"
                  goto endsub
        case else
    end select

    Call wTypeKeys "<HOME>"         'Setting cursor to beginning
    Call wTypeKeys "<F7>"           'Calling spellchecker with shortcut
    sleep (1)

    Kontext "Active"
    if Active.Exists(1) then        'checking if messagebox comes up
        if Active.getRT = 304 then      'checking ressource type
            Active.OK
        else
            printlog "No message Box!"
        end if
        printlog "Message Box closed, Spellchecker came up"
    else
        Kontext "Spellcheck"            'checking if spellcheck dialog came up
        if Spellcheck.Exists then
            printlog "OK, Spellchecker comes up"
            Spellcheck.Close            'closing spellchecker
        else
            Warnlog "Spellchecker does not come up with shortcut 'F7'"
        end if
    end if
    Call hCloseDocument
endcase

'----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tToolsSpellcheckCorrect
    Dim sHyphenationFile as String
    Dim TheFile as String
    Dim FFText as String

    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if FileExists ( TheFile ) <> TRUE then
       Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
       exit sub
    end if

    Printlog "- Checking Dictionary-Files"
    select case iSprache
        case 1 : sHyphenationFile = "01-44-hyph.dat"
        case 81 : Printlog "- No Spellcheck in Japanese!"
                  goto endsub
        case 82 : Printlog "- No Spellcheck in Korean!"
                  goto endsub
        case 86 : Printlog "- No Spellcheck in Chinese (simplified)!"
                  goto endsub
        case 88 : Printlog "- No Spellcheck in Chinese (traditional)!"
                  goto endsub
        case else : sHyphenationFile =  "" & iSprache & "-hyph.dat"
    end select
    if gNetzInst = TRUE then
        sHyphenationFile = Convertpath (gNetzOfficePath + "share\dict\" + trim$(sHyphenationFile))
    else
        sHyphenationFile = Convertpath (gOfficePath + "share\dict\" + trim$(sHyphenationFile))
    end if

    PrintLog "- Tools / Spellcheck without text"

    printlog " Open document where the formatting is set to the language we wish to test."
    call hFileOpenLocally (TheFile,false)
    

    printlog " Insert some faulty text"
    Sleep 1
    '/// Call Spellcheck in empty document ///
    ToolsSpellcheckWriter
    Sleep 2
    Kontext "Active"
    if NOT Active.Exists(1) then
        Warnlog "No Finished message"
    else
        Active.OK
    end if

    PrintLog "- Tools / Spellcheck with correct text"
    '/// Call Spellcheck for correct text ///
    select case iSprache
        case 01   : FFText = "This is a text without any error.<Return>"
        case 03   : FFText = "Escreva os seus dados de utilizador correctamente e tome nota deles (guardar / imprimir).<Return>"
        case 07   : FFText = "прин�?тый Го�?удар�?твенной Думой в первом чтении 16 но�?бр�? 1999 года.<Return>"
        case 31   : FFText = "Dit werk is beschermd door de Auteurswet.<Return>"
        case 33   : FFText = "Il nous faut donc un de temps pour examiner avec soin tous les dossiers.<Return>"
        case 34   : FFText = "Este archivo contiene información importante sobre el programa.<Return>"
        case 36   : FFText = "Boszniai amerikai kontingens háromnegyedesek – mintegy négyezer katona – magyarországi telepítése egy éven belül megtörténhet.<Return>"
        case 39   : FFText = "La ringraziamo per l'interesse mostrato a collaborare con la firma.<Return>"
        case 45   : FFText = "Danmark ikke er sikret maksimal indflydelse på den fremtidige europæiske udlændingepolitik.<Return>"
        case 46   : FFText = "Den här filen innehåller viktig programinformation.<Return>"
        case 48   : FFText = "Nie wolno w zadnim wypadku przerywa uruchomionego serwisu !<Return>"
        case 49   : FFText = "Dies ist ein Text ohne Fehler.<Return>"
        case 50   : FFText = "To je besedilo brez napake.<Return>"
        case 55	  : FFText = "Ele ouviu passos silenciosos atrás dele. Isso não estava certo.<Return>"
        case else : QAErrorLog " - The language " + iSprache +" has no defined language text. No Test !"
                    exit Sub
    end select

    Call wTypeKeys FFText
    wait 500
    ToolsSpellcheckWriter
    Sleep 2
    Kontext "Active"
    try
        Active.Yes
        Sleep 1
    catch
        Warnlog "No message to jump to start"
    endcatch

    Kontext "Active"
    try
        Active.OK
    catch
        Kontext "Spellcheck"
        if Spellcheck.Exists then
            Warnlog "With correct text the Spellcheck dialog appears ( maybe wrong language selected )"
            Spellcheck.Close
        end if
    endcatch

    '/// Close document ///
    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSpellcheck1Mistake
    Dim sWordToBeChecked as string
    Dim sLanguageName as string
    Dim sDummy as string
    Dim sErrorSampleText as string
    Dim TheFile as String

    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if FileExists ( TheFile ) <> TRUE then
       Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
       exit sub
    end if

    PrintLog "- Tools / Spellcheck with a defined mistake"
    Select Case iSprache
        case 01   : sErrorSampleText = "Thatts not a trick, it"  ' 's a Sony."    (removing this last part because of different apostrophes in StarSuite)
                    sWordToBeChecked = "Thatts"
                    sLanguageName = "English (USA)"
        case 03   : sErrorSampleText = "Escrive os seus dados de utilizador correctamente e tome nota deles."
                    sWordToBeChecked = "Escrive"
                    sLanguageName = "Português"
        case 07   : sErrorSampleText = "Военна�? техниче�?кого �?aтрудниче�?тва, проблему задолженно�?ти и вопро�?ы региональных �?в�?зей."
                    sWordToBeChecked = "�?aтрудниче�?тва"
                    sLanguageName = "Ру�?�?кий"
        case 31   : sErrorSampleText = "Wijek danken u voor uw interesse in het samenwerken met Sun Microsystems."
                    sWordToBeChecked = "Wijek"
                    sLanguageName = "Nederlands (NL)"
        case 33   : sErrorSampleText = "Ilk nous faut donc un de temps pour examiner avec soin"
                    sWordToBeChecked = "Ilk"
                    sLanguageName = "Français (France)"
        case 34   : sErrorSampleText = "Este archivo contiene información importnte sobre el programa."
                    sWordToBeChecked = "importnte"
                    sLanguageName = "Español (España)"
        case 36   : sErrorSampleText = "Boszniai amerikai kontingens háromnegyedének – mintegy négyezer katona – magyarországi telepítése egy éven belül megtörténhet."
                    sWordToBeChecked = "háromnegyedének"
                    sLanguageName = "Magyar"
        case 39   : sErrorSampleText = "Lab ringraziamo per l'interesse"
                    sWordToBeChecked = "Lab"
                    sLanguageName = "Italiano (Italia)"
        case 45   : sErrorSampleText = "Igke er sikret maksimal indflydelse på den fremtidige europæiske udlændingepolitik."
                    sWordToBeChecked = "Igke"
                    sLanguageName = "Dansk"
        case 46   : sErrorSampleText = "Dsän här filen innehåller viktig programinformation."
                    sWordToBeChecked = "Dsän"
                    sLanguageName = "svenska (Sverige)"
        case 48   : sErrorSampleText  = "walno w zadnim wypadku przerywa uruchomionego serwisu !"
                    sWordToBeChecked = "Walno"
                    sLanguageName = "Polski"
        case 49   : sErrorSampleText = "Diees ist ein Fehler."
                    sWordToBeChecked = "Diees"
                    sLanguageName = "Deutsch (Deutschland)"
        case 50   : sErrorSampleText = "Tuki je nekaj narobe."
                    sWordToBeChecked = "Tuki"
                    sLanguageName = "Slovenš�?ina"
        case 55	  : sErrorSampleText = "Esta poderia ser a respista para suas preces?"
        			sWordToBeChecked = "respista"
        			sLanguageName = "Português (Brasil)"
        case 81 : Printlog "- No Spellcheck in Japanese!"
                  goto endsub
        case 82 : Printlog "- No Spellcheck in Korean!"
                  goto endsub
        case 86 : Printlog "- No Spellcheck in Chinese (simplified)!"
                  goto endsub
       case 88 :  Printlog "- No Spellcheck in Chinese (traditional)!"
                  goto endsub
        case else : QAErrorLog "The language " + wSysLanguage +" has no defined Errortext. No test"
                  goto endsub
    end select

    printlog " Open document where the formatting is set to the language we wish to test."
    call hFileOpenLocally (TheFile,false)
    

    '/// Insert a sentence with one definded mistake and execute spellcheck ///
    Call wTypeKeys sErrorSampleText
    Call wTypeKeys "<Return><Up>"
	wait 500
    ToolsSpellcheckWriter
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
            catch
                Warnlog Active.Gettext
                Active.Ok
            endcatch
        end if
    end if
    Sleep 2
    Kontext "Spellcheck"
    if Not Spellcheck.Exists then
        Warnlog "Spellchecker is not up !"
        Call hCloseDocument
        goto endsub
    end if
    printlog "Current selected dictionary language is: " & DictionaryLanguage.GetSelText
    '  Dictionary Language is language dependend. Function to be created
    printlog "   - check language"
    'if DictionaryLanguage.GetSelText <> gLanguage then Warnlog "Dictionary language " + DictionaryLanguage.GetSelText +" selected"
    printlog "   - replace wrong word with a suggestion"
    '/// replace wrong word with a suggestion ///

    if trim$(NotInDictionary.GetText) <> trim(sErrorSampleText) then
        warnlog " " & sErrorSampleText & " has not been detected as mistake. Detected mistake is:" & NotInDictionary.GetText
    end if

    if Suggestions.GetItemCount > 0 then
        if Suggestions.IsEnabled then
	    try
		sDummy = Suggestions.GetItemText (2)
		Suggestions.Select 2
	    catch
		sDummy = Suggestions.GetItemText (1)
		Suggestions.Select 1
	    endcatch
            ' Next check does not work because if word is selected it will only be highlited!!"
            ' if Not NotInDictionary.GetText <> sErrorSampleText then Warnlog "Text in preview has not been replaced with the suggestion"
            Change.Click
	else
	    QAErrorLog "No suggestions made from spellchecker"
	end if
    else
        QAErrorLog "Please choose another word to check suggestions, because no suggestion found!"
    end if
    Kontext "Active"
    if Active.Exists then
        try
            Active.OK
        catch
            Warnlog Active.Gettext
            Active.No
        endcatch
    end if

    Kontext "Spellcheck"
    if Spellcheck.Exists then SpellcheckClose.Click

	Call wTypeKeys "<Up><Home>"
    Call wTypeKeys "<Shift End>"
    Sleep 1
	EditCopy
    if GetClipboardText = sErrorSampleText then Warnlog "Replaced text in document is wrong. Not "+ sErrorSampleText + " but " + GetClipboardText
    Call hCloseDocument

endcase

' *******************************************************************

testcase tToolsSpellcheckForward
    Dim sDummy as string
    Dim sWordToBeAdded as string
    Dim InsFile as string
    Dim Add_Word as string
    Dim sDictionary as string

    PrintLog "- Tools / Spellcheck / check forward"
    '/// Open specified document depending on language to check spellcheck ///
    '/// document has 4 defined wrong words ///
    InsFile = wGetSpellcheckDocumentName(1)
    Select Case InsFile
        Case "None"
            Printlog "- No Spellcheck for this language available!"
            goto endsub
        case "Empty"
            QAErrorLog "The language " + iSprache +" has no defined Errortext. No test!"  'iSystemSprache
            goto endsub
    end select

    if gApplication = "MASTERDOCUMENT" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
            EditPaste
    else
        hFileOpenLocally( InsFile)
        
    end if
    '/// point cursor to beginning of document and execute spellcheck ///
    Call wTypeKeys "<Mod1 Home>"

    select case iSprache
        case 01   : sDummy = "exprass"      : sWordToBeAdded = "pablischer"
        case 07   : sDummy = "решаaт"       : sWordToBeAdded = "кaнкретных"
        case 03   : sDummy = "quante"       : sWordToBeAdded = "cempas"
        case 31   : sDummy = "haaietanden"  : sWordToBeAdded = "widerzijdse"
        case 33   : sDummy = "votrex"       : sWordToBeAdded = "recontacterons"
        case 34   : sDummy = "Asegúree"     : sWordToBeAdded = "diretorio"
        case 36   : sDummy = "hétozres"     : sWordToBeAdded = "jelentos"
        case 39   : sDummy = "meczanici"    : sWordToBeAdded = "anniche"
        case 45   : sDummy = "grend"        : sWordToBeAdded = "lando"
        case 46   : sDummy = "handbroken"   : sWordToBeAdded = "viktikt"
        case 48   : sDummy = "znajdujacej"  : sWordToBeAdded = "usuniete"
        case 49   : sDummy = "reichtt"      : sWordToBeAdded = "viierter"
        case 50   : sDummy = "Paglejmo"     : sWordToBeAdded = "�?trto"
        case 55   : sDummy = "horat"        : sWordToBeAdded = "dpa"
    end select

    wait 500
    ToolsSpellcheckWriter
    Sleep 2
    Kontext "Spellcheck"
    Printlog "   - Replace all (" + NotInDictionary.Gettext + ") ...."
    '/// Replace first found word with 1 suggestion in list ///
    if Suggestions.IsEnabled then
        Suggestions.Select 1
	Printlog "   - .... with (" + NotInDictionary.Gettext + ")"
	wait 500
	try
	    ChangeAll.Click
	catch
	    Warnlog "Button 'change all' is not accessible!"
	endcatch
	'/// Select 'Always replace' ///
	Sleep 1
    else
	QAErrorLog "No suggestions made from spellchecker!=>Choose 'Ignore All'"
        if IgnoreAll.IsEnabled then IgnoreAll.Click
    end if

    Printlog "   - Ignore All (" + NotInDictionary.Gettext + ")"
    '/// For second wrong word select 'Ignore all' ///
    wait 500
    try
        IgnoreAll.Click
    catch
        Warnlog "Button 'Ignore all' is not accessible"
    endcatch
    Sleep 1

    Printlog "   - Ignore (" + NotInDictionary.Gettext + ")"
    '/// For 3rd wrong word select 'Ignore' ///
    wait 500
'    try
'        IgnoreOnce.Click
'    catch
'        Warnlog "Button 'Ignore once' is not accessible"
'    endcatch
'    Sleep 1

    Kontext "Active"
    if Active.Exists(3) then
        if Active.GetRT = 304 then
            Warnlog "Spellcheck has ended! Not reproducable by hand !"
        end if
    else
        Kontext "Spellcheck"
        Add_Word = trim$(NotInDictionary.Gettext)
        Printlog "   - Add (" + sDummy + ")"
        '/// Add 4th word to dictionary ///
        if AddToDictionary.IsEnabled then
            AddToDictionary.Click
	    wait 500
	    '/// remember selected dictionary
	    sDictionary = MenuGetitemText(1)
	    Call hMenuSelectNr(1)
	    Sleep 1
	else
	    Warnlog "Unable to add word to dictionary->button is disabled!"
	end if
    end if

	Kontext "Active"
	if Active.Exists then
	    If Active.GetRT = 304 then
		Active.Ok
	    end if
	end if

    Kontext "Spellcheck"
    if Spellcheck.Exists then Spellcheck.Close
    Call wTypeKeys "<Home>"

    Printlog "   - recheck: One mistake has to be found 3 times"
    wait 500
    '/// Recheck: One mistake has to be found 3 times ///
    ToolsSpellcheckWriter
    Sleep 2
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Ok
            catch
                Active.Yes
                Warnlog Active.Gettext
            endcatch
        end if
    end if
    Kontext "Spellcheck"
    if Spellcheck.Exists then
        if Instr(NotInDictionary.GetText, sDummy) = 0 then
            Warnlog "Jumped to wrong mistake! Not " & sDummy & " but " & Suggestions.GetSelText
        else
            try
                IgnoreAll.Click
            catch
                Warnlog "Button 'Ignore all' is not accessible"
            endcatch
        end if
    end if
    Sleep 2
    Kontext
    if Active.Exists(2) then
        Active.OK
    else
        Kontext "Spellcheck"
        if Spellcheck.Exists then Spellcheck.Close
    end if
    Sleep 1

    GoOn:
    Printlog "- Deleting the Ignore list"
    '/// Delete entrys in Ignorelist ///
    Call wIgnorierenListeLoeschen

    Printlog "   - Delete added word in dictionary"
    '/// Delete added word in dictionary ///
    Call WortAusWoerterbuchLoeschen ( sDummy )

    '/// Close document ///
    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSpellcheckOnlyOneWord
    Dim sWordToBeChecked as string
    Dim InsFile as string
    Dim ReplacedWith as String
    Dim i as integer

    '/// Open specified document depending on language to check spellcheck ///
    '/// document has 1 defined wrong word ///
    PrintLog "- Tools / Spellchecking / Check only one word"
    InsFile = wGetSpellcheckDocumentName(2)
    Select Case InsFile
        Case "None"
            Printlog "- No Spellcheck for this language available!"
            goto endsub
        case "Empty"
            QAErrorLog "The language " + iSprache +" has no defined Errortext. No test!"
            goto endsub
    end select

    if gApplication = "MASTERDOCUMENT" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
            EditPaste
    else
        hFileOpenLocally( InsFile)
        
    end if
    Call wTypeKeys "<Mod1 Home>"

    select case iSprache
        case 01   : sWordToBeChecked = "excetionally"      ' this word is wrong
        case 07   : sWordToBeChecked = "Кaн�?титуции"
        case 03   : sWordToBeChecked = "semstre"           ' this word is wrong
        case 31   : sWordToBeChecked = "lidstartesn"        ' this word is wrong
        case 33   : sWordToBeChecked = "heues"            ' this word is wrong
        case 34   : sWordToBeChecked = "vacunaión"         ' this word is wrong
        case 36   : sWordToBeChecked = "fekvo"         ' this word is wrong
        case 39   : sWordToBeChecked = "brasilianir"       ' this word is wrong
        case 45   : sWordToBeChecked = "Amserdam"         ' this word is wrong
        case 46   : sWordToBeChecked = "avrttades"         ' this word is wrong
        case 48   : sWordToBeChecked = "stosowanei"         ' this word is wrong
        case 49   : sWordToBeChecked = "georaphischen"     ' this word is wrong
        case 50   : sWordToBeChecked = "ssmo"     ' this word is wrong
        case 55   : sWordToBeChecked = "Veigo"     ' this word is wrong
    end select

    EditSearchAndReplace
    Kontext "FindAndReplace"
    try
        SearchFor.Settext ""
    catch
    endcatch
    if SimilaritySearch.IsVisible = False then More.Click
    if SimilaritySearch.IsEnabled then SimilaritySearch.Uncheck
    if SearchForStyles.IsEnabled then SearchForStyles.Uncheck
    if RegularExpressions.IsEnabled then RegularExpressions.Uncheck
    if NoFormat.IsEnabled then NoFormat.Click
    if CurrentSelectionOnly.IsEnabled then CurrentSelectionOnly.Uncheck
    WholeWordsOnly.Uncheck
    Backwards.Uncheck
    MatchCase.Uncheck
    RegularExpressions.Check

    SearchFor.Settext sWordToBeChecked
    SearchNow.Click
    Kontext "Active"
    if Active.Exists then
        if Active.getRT = 304 then
            Warnlog Active.Gettext
            try
                Active.Ok
            catch
                Active.No
            endcatch
        end if
    end if

    Kontext "FindAndReplace"
    if FindAndReplace.Exists then FindAndReplace.Close

    ToolsSpellcheckWriter
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog "Spellcheck doesn't work correctly:" & Active.Gettext
            try
                Active.Ok
            catch
                Active.No
            endcatch
            Call hCloseDocument
            goto endsub
        end if
    end if

    Kontext "Spellcheck"
    Printlog "   - Replace all"
    if Suggestions.GetItemCount > 0 then
        if Suggestions.IsEnabled then
			Suggestions.Select 1
			wait 500
			Suggestions.TypeKeys "<Down><Up>"
			ReplacedWith = Suggestions.GetSelText
			wait 500
			if Change.IsEnabled then Change.Click
			Sleep 1
		else
			QAErrorLog "No suggestions made from spellchecker!"
            Spellcheck.Close
            Call hCloseDocument
            goto endsub
		end if
    else
        Warnlog "No suggestion for word: " & sWordToBeChecked & " found. Test stops"
        Spellcheck.Close
        Call hCloseDocument
        goto endsub
    end if

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Ok
            catch
                Active.No
                Warnlog Active.Gettext
            endcatch
        end if
    end if

    Kontext "Spellcheck"
    if Spellcheck.Exists then SpellcheckClose.Click

    Call wTypeKeys "<Mod1 Shift Left>"
    try
        EditCopy
        if GetClipboardtext = sWordToBeChecked then
            Warnlog "Spellchecker does not commit changes correctly (" & GetClipboardtext & " found )"
            Call hCloseDocument
            goto endsub
        end if
    catch
        Warnlog "- Didn't found the word to check! No Spellchecking for one word!"
        Call hCloseDocument
        goto endsub
    endcatch

    '/// Execute spellcheck again ///
    wait 500
    ToolsSpellcheckWriter
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
                Kontext "Active"
                if Active.Exists then
                    if Active.GetRT = 304 then
                        Active.Ok
                        Warnlog "Spellchecker finishes though wrong words in documents"
                    end if
                end if
            catch
                Active.Ok
            endcatch
        end if
    else
        Sleep 2
        Kontext "Spellcheck"
        if Spellcheck.Exists then
            Warnlog "Probably the replaced word found wrong again!"
            Spellcheck.Close
        end if
    end if

    Kontext "Spellcheck"
    if Spellcheck.Exists then Spellcheck.Close

    '/// Close document ///
    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSpellcheckAutomatic
    Dim iError1 as integer
    Dim iError2 as integer
    Dim sError1 as string
    Dim sError2 as string
    Dim j as integer
    Dim sNumberOfSuggestions as string
    Dim sReplaceWithWord as string
    Dim sDefaultFormat as string
    Dim TheFile as String

    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if FileExists ( TheFile ) <> TRUE then
       Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
       exit sub
    end if

    PrintLog "- Tools / Spellcheck / AutoSpellcheck"
    printlog " Open document where the formatting is set to the language we wish to test."
    call hFileOpenLocally (TheFile,false)
    

    '/// Enable Auto-Spellcheck in options
    Call wEnableAutoSpellcheck ( true )

    '/// Check Auto-Spellcheck and insert a sentence with 2 mistakes ///
    printlog "   - write text"
    select case iSprache
        case 01   : wTypeKeys "Hello thiis here are too errorrs "
                  iError1 = 1 : iError2 = 4
                  sError1 = "thiis"  : sError1 = "errorrs"
                  sDefaultFormat = "Standard"
        case 03   : wTypeKeys "Aqui poderá elimitnar componentes já instalados ou instalar novos compotentes numa "
                  iError1 = 2 : iError2 = 7
                  sError1 = "elimitnar"  : sError1 = "compotentes"
                  sDefaultFormat = "Padrão"
        case 31   : wTypeKeys "Betaling binnern 14 dagen rekeningsdatum met 2% kortng, binnen 30 dagen nettor "
                  iError1 = 1 : iError2 = 7
                  sError1 = "binnern"  : sError1 = "rekeningsdatum"
                  sDefaultFormat = "Standaard"
        case 33   : wTypeKeys "Je vious remercie d'accuser réception dex ce courrier "
                  iError1 = 1 : iError2 = 4
                  sError1 = "vious"  : sError1 = "dex"
                  sDefaultFormat = "Standard"
        case 34   : wTypeKeys "Este archvo contiene información importnte sobre el programa."
                  iError1 = 1 : iError2 = 3
                  sError1 = "archvo"  : sError1 = "importnte"
                  sDefaultFormat = "Estándar"
        case 36   : wTypeKeys "A hétezres koszovói amerikai kontingens jelentos részét is a magyarországi bázison helyezi majd el."
                  iError1 = 1 : iError2 = 4
                  sError1 = "hétezres"  : sError1 = "jelentos"
                  sDefaultFormat = "Alapértelmezett"
        case 39   : wTypeKeys "La sfumaturar è statsi modificata senza salvare "
                  iError1 = 1 : iError2 = 2
                  sError1 = "sfumaturar"  : sError1 = "statsi"
                  sDefaultFormat = "Standard"
        case 45   : wTypeKeys "Han advarade både mod at kalde traktüten et fredens projekt "
                  iError1 = 1 : iError2 = 5
                  sError1 = "advarade"  : sError1 = "traktüten"
                  sDefaultFormat = "Standard"
        case 46   : wTypeKeys "Den här fiten innehåller viktit programinformation "
                  iError1 = 2 : iError2 = 2
                  sError1 = "fiten"  : sError1 = "viktit"
                  sDefaultFormat = "Standard"
        case 48   : wTypeKeys "Nie wolno w zadim wypadku przerywa uruchomiongo serwisu "
                  iError1 = 3 : iError2 = 3
                  sError1 = "zadim"  : sError1 = "uruchomiongo"
                  sDefaultFormat = "Standard"
        case 49   : wTypeKeys "Hallo hiaer sind swei Fehler "
                  iError1 = 1 : iError2 = 2
                  sError1 = "hiaer"  : sError1 = "swei"
                  sDefaultFormat = "Standard"
        case 50   : wTypeKeys "Tuki sta dve napki"
                  iError1 = 1 : iError2 = 2
                  sError1 = "Tuki"  : sError1 = "napki"
                  sDefaultFormat = "Standard"
        case 55   : wTypeKeys "Aqui poderá elimitnar componentes já instalados ou instalar novos compotentes numa "
                  iError1 = 2 : iError2 = 7
                  sError1 = "elimitnar"  : sError1 = "compotentes"
                  sDefaultFormat = "Padrão"
        case 81 : Printlog "- No Spellcheck in Japanese!"
                 Call hCloseDocument
                 goto endsub
        case 82 : Printlog "- No Spellcheck in Korean!"
                 Call hCloseDocument
                 goto endsub
        case 86 : Printlog "- No Spellcheck in Chinese (simplified)!"
                 Call hCloseDocument
                 goto endsub
        case 88 : Printlog "- No Spellcheck in Chinese (traditional)!"
                 Call hCloseDocument
                 goto endsub
        case else : QAErrorlog "The language " + iSprache +" has no defined errortext. No Test!"
                  Call hCloseDocument
                  goto endsub
    end select

    Call wTypeKeys "<Space><Return>"

    printlog "   -  relieve mistake"
    Call wTypeKeys "<Up><Home>"
    Call wTypeKeys "<Mod1 Right>", iError1
    Call wTypeKeys "<Right>"          ' Cursor in fehlerhaftes NotInDictionary stellen
    Wait (500)
    '/// Open contextmenu in first wrong word ///
    Call wTypeKeys "<Shift F10>"
    sReplaceWithWord=hMenuItemGetText (2)
    if sReplaceWithWord<>sDefaultFormat then
        sNumberOfSuggestions=MenuGetItemCount - 6
        '/// Check if there is a suggestion and replace word ///
        if sNumberOfSuggestions <=0 then
            Warnlog "- No suggestions in Context menu for the first wrong word!"
        else
            printlog "- Context menu has " + sNumberOfSuggestions + " suggestions to the 1. wrong word!"
            Call hMenuSelectNr (2)
            Call wTypeKeys "<Shift Mod1 Left>"
            try
                EditCopy
                if GetClipboardtext<> sReplaceWithWord then
                    Warnlog "- 1. wrong Word has not been replaced with " + sReplaceWithWord + " but "+ GetClipBoardtext
                else
                    printlog "- 1. wrong word has been replaced with the 1. entry in context menu!"
                end if
            catch
            endcatch
        end if
    else
        Warnlog "- AutoSpellcheck doesn't work , wrong Context menu! (1.Word)"
    end if
    Wait (5000)
    Call wTypeKeys "<Mod1 Right>", iError2
    Wait (5000)
    Call wTypeKeys "<Right>"          ' Cursor in fehlerhaftes NotInDictionary stellen
    Wait (5000)
    Call wTypeKeys "<Shift F10>"

    sReplaceWithWord=hMenuItemGetText (2)
    if sReplaceWithWord<>sDefaultFormat then
        sNumberOfSuggestions=MenuGetItemCount - 6
        if sNumberOfSuggestions <=0 then
            Warnlog "- No suggestions in Contextmenu for 2. wrong Word !"
        else
            printlog "- Context menu has " + sNumberOfSuggestions + " suggestions for the 2. wrong word!"
            Call hMenuSelectNr (2)
            Call wTypeKeys "<Shift Mod1 Left>"
            try
                EditCopy
                if GetClipboardtext<> sReplaceWithWord then
                    Warnlog "- 2. wrong word has not been replaced with " + sReplaceWithWord + " but "+ GetClipBoardtext
                else
                    printlog "- 2. mistake has been replaced with the 1. entry in context menu!"
                end if
            catch
            endcatch
        end if
    else
        Warnlog "- AutoSpellcheck doesn't work, wrong Context menu (2.Word)!"
    end if

    Call wTypeKeys "<Home>"
    Call wTypeKeys "<F7>"
    Sleep 2
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.ok
            catch
                Active.Gettext
                Active.No
            endcatch
        end if
    else
        Kontext "Spellcheck"
        if Spellcheck.Exists then
            QAErrorlog "- Spellchecking dialog is still open. Not all words have been replaced!"
            Spellcheck.Close
        end if
    end if

    Kontext "Active"
    if Active.Exists then Active.ok

    '/// Close document ///
    Call hCloseDocument
endcase

'------------------------------------------------------------------------------------------------------------------------------------------------------------------

testcase tToolsSpellcheckChangeLanguageAttribute

    Dim sWordToBeChecked as string
    Dim InsFile as string
    Dim i as integer
    Dim ReplacedWith as string
    Dim PriorLanguage as string
    Dim CurrentLanguage as string

    '/// Open specified document depending on language to check spellcheck ///
    '/// document has 1 defined wrong word ///
    PrintLog "- Tools / Spellchecking / Check only one word"
    InsFile = wGetSpellcheckDocumentName(2)
    Select Case InsFile
        Case "None"
            Printlog "- No Spellcheck for this language available!"
            goto endsub
        case "Empty"
            QAErrorLog "The language " + iSprache +" has no defined Errortext. No test!" 'iSystemSprache
            goto endsub
    end select

    if gApplication = "MASTERDOCUMENT" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
        EditPaste
    else
        hFileOpenLocally( InsFile)
        
    end if
    Call wTypeKeys "<Mod1 Home>"

    select case iSprache
        case 01   : sWordToBeChecked = "excetionally"      ' this word is wrong
        case 03   : sWordToBeChecked = "semstre"           ' this word is wrong
        case 07   : sWordToBeChecked = "военно"
        case 31   : sWordToBeChecked = "lidstartesn"        ' this word is wrong
        case 33   : sWordToBeChecked = "heues"            ' this word is wrong
        case 34   : sWordToBeChecked = "vacunaión"         ' this word is wrong
        case 36   : sWordToBeChecked = "jelentos"
        case 39   : sWordToBeChecked = "brasilianir"       ' this word is wrong
        case 45   : sWordToBeChecked = "Amserdam"         ' this word is wrong
        case 46   : sWordToBeChecked = "avrttades"         ' this word is wrong
        case 48   : sWordToBeChecked = "stosowanei"         ' this word is wrong
        case 49   : sWordToBeChecked = "georaphischen"     ' this word is wrong
        case 50   : sWordToBeChecked = "ssmo"     ' this word is wrong
        case 55   : sWordToBeChecked = "Veigo"     ' this word is wrong
    end select

    EditSearchAndReplace
    Kontext "FindAndReplace"
    try
        SearchFor.Settext ""
    catch
    endcatch
    if SimilaritySearch.IsVisible = False then More.Click
    if SimilaritySearch.IsEnabled then SimilaritySearch.Uncheck
    if SearchForStyles.IsEnabled then SearchForStyles.Uncheck
    if RegularExpressions.IsEnabled then RegularExpressions.Uncheck
    if NoFormat.IsEnabled then NoFormat.Click
    if CurrentSelectionOnly.IsEnabled then CurrentSelectionOnly.Uncheck
    WholeWordsOnly.Uncheck
    Backwards.Uncheck
    MatchCase.Uncheck
    RegularExpressions.Check

    SearchFor.Settext sWordToBeChecked
    SearchNow.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog "- Didn't found the word to check! No Spellchecking for one word!"
            Active.Ok
            Kontext "FindAndReplace"
            FindAndReplace.Cancel
            Call hCloseDocument
            goto endsub
        end if
    end if
    Kontext "FindAndReplace"
    if FindAndReplace.Exists then FindAndReplace.Cancel
    EditCopy

    '/// Execute spellcheck: change language Attributes
    wait 500
    ToolsSpellcheckWriter
    Sleep 2
    Kontext "Spellcheck"
    if Not Spellcheck.Exists then
        Kontext "Active"
        If Active.Exists then
            If Active.GetRT = 304 then
                Warnlog Active.Gettext
                Warnlog "- Probably didn't found the word to check! No Spellchecking for one word!"
                Active.ok
            end if
        end if
        ToolsSpellcheckWriter
        Kontext "Spellcheck"
    end if
	'/// Replace found word with suggestion from list
    if Spellcheck.Exists then
        PriorLanguage = DictionaryLanguage.GetSelText
        printlog "- remember selected language: " & PriorLanguage
        for i = 1 to DictionaryLanguage.GetItemCount
            DictionaryLanguage.Select i
            if Suggestions.GetItemCount > 0 then
                CurrentLanguage = DictionaryLanguage.GetSelText
                if Suggestions.IsEnabled and CurrentLanguage <> PriorLanguage then
                    printlog "-  replace with language: " & CurrentLanguage
                    Suggestions.Select 1
                    ReplacedWith = Suggestions.GetSelText
                    printlog "-  replace with word: " & ReplacedWith
                    i = DictionaryLanguage.GetItemCount + 1
                end if
            end if
        next i
        '/// Close spellcheck dialog and check if properties have been set
        if ReplacedWith > "" then
            Change.Click
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    try
                        Active.Yes
                    catch
                    endcatch
                end if
            end if
            ' Finished message ?
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    try
                        Active.Ok
                    catch
                    endcatch
                end if
            end if
            wait 500
            Kontext "Spellcheck"
            if Spellcheck.Exists then
                SpellcheckClose.Click
            end if
            wait 500
            Call wTypeKeys "<Mod1 Home>"
            printlog "- find replaced word"

            EditSearchAndReplace
            Kontext "FindAndReplace"
            try
                SearchFor.Settext ""
            catch
            endcatch

            SearchFor.Settext ReplacedWith
            SearchNow.Click
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    Warnlog "- Didn't found the replaced word !"
                    Active.Ok
                    Kontext "FindAndReplace"
                    FindAndReplace.Cancel
                    Call hCloseDocument
                    goto endsub
                end if
            end if
            Kontext "FindAndReplace"
            if FindAndReplace.Exists then FindAndReplace.Cancel
            wait 500
            FormatCharacter
            Kontext
            Active.Setpage TabFont
            Kontext "TabFont"
            try
                if Language.GetSelText <> CurrentLanguage then
                    if Language.GetSelText = PriorLanguage then
                        Warnlog "Language Attributes has not been set to" & CurrentLanguage
                    else
                        Warnlog "Language Attributes has not been set to " & CurrentLanguage & " but " & Language.GetSelText
                    end if
                end if
            catch
                if LanguageWest.GetSelText <> CurrentLanguage then
                    if LanguageWest.GetSelText = PriorLanguage then
                        Warnlog "Language Attributes has not been set to" & CurrentLanguage
                    else
                        Warnlog "Language Attributes has not been set to " & CurrentLanguage & " but " & LanguageWest.GetSelText
                    end if
                end if
            endcatch
            TabFont.Cancel
        else
            QAErrorlog "No suggestions found! Maybe wrong word should be changed to run more deeper testing"
        end if
    else
        Warnlog "Spellcheck dialog is not up !"
    end if
    '/// Close document ///
    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSpellcheckButtonChange
    Dim InsFile as string
    Dim i as integer
    '/// Open specified document depending on language to check spellcheck ///
    '/// document has 1 defined wrong word ///
    PrintLog "- Tools / Spellchecking / Check button 'change'"
    InsFile = wGetSpellcheckDocumentName(2)
    Select Case InsFile
        Case "None"
            Printlog "- No Spellcheck for this language available!"
            goto endsub
        case "Empty"
            QAErrorLog "The language " + iSprache +" has no defined Errortext. No test!"  'iSystemSprache
            goto endsub
    end select

    if gApplication = "MASTERDOCUMENT" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
        EditPaste
    else
        hFileOpenLocally( InsFile)
        
    end if
    Call wTypeKeys "<Mod1 Home>"

    '/// invoke the spellchecker
    ToolsSpellcheckWriter
    '/// Check if button change if disabled if
    '/// 1. No suggestion has been given from spellchecker
    '/// 2. The button 'change' should be enabled if found mistake has been changed!"
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Ok
                Warnlog "No mistake found in sample document -> Bug!"
            catch
            endcatch
        else
            Kontext "Spellcheck"
            if Spellcheck.Exists then
                for i = 1 to DictionaryLanguage.GetItemCount
                    DictionaryLanguage.Select i
                    if Suggestions.GetItemCount = 0 then
                        if Change.IsEnabled = true then
                            Warnlog "Button 'Change' is enabled though no sugggestion is given"
                        else
                            NotInDictionary.TypeKeys "Enable me"
                            wait 500
                            if Not Change.IsEnabled = true then
                                Warnlog "Button 'Change' is disabled though text has been changed"
                            end if
                         end if
                     end if
                next i
            end if
        end if
    end if
    Kontext "Spellcheck"
    if Spellcheck.Exists then SpellcheckClose.Click
    '/// Close document
    Call hCloseDocument

endcase

' *******************************************************************

testcase tToolsSpellcheckButtonChangeAll
    Dim InsFile as string
    Dim i as integer
    '/// Open specified document depending on language to check spellcheck ///
    '/// document has 1 defined wrong word ///
    PrintLog "- Tools / Spellchecking / Check button 'change all'"
    InsFile = wGetSpellcheckDocumentName(2)
    Select Case InsFile
        Case "None"
            Printlog "- No Spellcheck for this language available!"
            goto endsub
        case "Empty"
            QAErrorLog "The language " + iSprache +" has no defined Errortext. No test!"  'iSystemSprache
            goto endsub
    end select

    if gApplication = "MASTERDOCUMENT" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
        EditPaste
    else
        hFileOpenLocally( InsFile)
        
    end if
    Call wTypeKeys "<Mod1 Home>"

    '/// invoke the spellchecker
    ToolsSpellcheckWriter
    '/// Check if button 'change all' disabled if found mistake has been changed!"
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Ok
                Warnlog "No mistake found in sample document -> Bug!"
            catch
            endcatch
        else
            Kontext "Spellcheck"
            if ChangeAll.IsEnabled then
                NotInDictionary.TypeKeys "Disable me"
                wait 500
                if ChangeAll.IsEnabled = true then
                    Warnlog "Button 'Change all' is disabled though text has been changed"
                end if
            else
                Warnlog "Button 'Change all' should be enabled"
            end if
            SpellcheckClose.Click
        end if
    end if
    '/// Close document
    Call hCloseDocument

endcase

' *******************************************************************

testcase tToolsThesaurus

    Dim sWordInitial as string
    Dim sWordToBeFound as string
    Dim TheFile as String

    PrintLog "- Tools / Thesaurus"

    printlog " Open document where the formatting is set to the language we wish to test."
    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if FileExists ( TheFile ) <> TRUE then
       Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
       exit sub
    end if

    select case iSprache
        case 01 : sWordInitial = "Morning"
        case 03 : sWordInitial = "Confirmam"
        case 07 : Printlog "- No Thesaurus in Russian!"
                  goto endsub
        case 31 : sWordInitial = "Hulp"
        case 33 : sWordInitial = "Payer"
        case 34 : sWordInitial = "Suficiente"
        case 36 : Printlog "- No Thesaurus in Hungarian!"
                  goto endsub
        case 39 : sWordInitial = "Lingua"
        case 45 : sWordInitial = "Først"
        case 46 : sWordInitial = "Däng"
        case 48 : sWordInitial = "wypowiedź"
        case 49 : sWordInitial = "Mann"
        case 50 : sWordInitial = "smisel"
        case 55 : Printlog "- No Thesaurus in Portuguese (Brazil) !"
        		  goto endsub
        case 81 : Printlog "- No Thesaurus in Japanese!"
                  goto endsub
        case 82 : Printlog "- No Thesaurus in Korean!"
                  goto endsub
        case 86 : Printlog "- No Thesaurus in Chinese (simplified)!"
                  goto endsub
        case 88 : Printlog "- No Thesaurus in Chinese (traditional)!"
                  goto endsub
        case 30, 35, 36, 37, 42, 47, 48, 51, 90
                  Printlog "- No Thesaurus available !"
                  goto endsub
        case else : QAErrorlog "The language " + iSprache + " has no defined errortext. No Test!"
                    goto endsub
    end select

    printlog "Open new preformatted-writerdocument"
    call hFileOpenLocally (TheFile,false)
    

    printlog "-  Thesaurus without a word"
    '/// Call Thesaurus without a written word ///
	ToolsLanguageThesaurus
    Kontext "Thesaurus"
    if CurrentWord.GetSelText <> "" then Warnlog "- Though no word has been selected Current Word textfield is filled !"
    if ReplaceWith.GetText <> "" then Warnlog "- Though no word has been selected there is a replacetext in the textfield!"
    printlog "Set language dependent word in textfield Current Word"
    CurrentWord.Settext sWordInitial

    Sleep 2
    if Alternatives.GetItemCount = 0 then
        Warnlog "No alternatives text"
    else
        if ReplaceWith.Gettext <> Alternatives.GetText(Alternatives.GetSelIndex) then
            Warnlog "Replace with textfield was not filled with word from alternative"
        endif
    endif
    sWordToBeFound = ReplaceWith.GetText
    Thesaurus.OK
    Call wTypeKeys "<Home><Shift End>"
    wait 200
    EditCopy
    if iSprache = 1 then
        if GetClipboardText <> sWordToBeFound then Warnlog "Wrong word has been replaced, not => " + sWordInitial + " but => " + GetClipboardText
    else
        if LCase (GetClipboardText) <> LCase (sWordToBeFound) then Warnlog "Wrong word has been replaced, not => " + sWordToBeFound + " but => " + GetClipboardText
    end if
    Call hCloseDocument
    wait 500

    printlog " Open new preformatted-document where the formatting is set to the language we wish to test."
    call hFileOpenLocally (TheFile,false)
    

    Call wTypeKeys sWordInitial
    Call wTypeKeys "<Home><Right><Right>"

    printlog "-  Replace ( select alternative with cursor within word )"
    ToolsLanguageThesaurus
    Kontext "Thesaurus"
    wait 500
    if CurrentWord.GetSeltext = sWordInitial then
    	sWordToBeFound = ReplaceWith.Gettext
		if Alternatives.GetItemCount > 0 then
			printlog "Select last entry in alternatives list"
			Alternatives.Select Alternatives.GetItemCount
			Alternatives.TypeKeys "<Space>"
			if ReplaceWith.Gettext = sWordToBeFound then
				Warnlog "Though alternative was selected replace with was not changed"
			else
				sWordToBeFound = ReplaceWith.Gettext
			end if
			Thesaurus.OK
			Call wTypeKeys "<Home><Shift End>"
			wait 200
			EditCopy
			if GetClipboardText <> sWordToBeFound then Warnlog "Text has not been correctly replaced, not => " + sWordToBeFound + " but => " + GetClipboardText
		else
			Warnlog "- No Alternative for current word"
			Thesaurus.Ok
		end if
	else
		Warnlog "Current word is not selected word !"
	end if

    Call wTypeKeys "<Home><Shift End><Delete>"
    Call wTypeKeys sWordInitial

    printlog "-  Replace unchecked ( Cursor at words end )"
    ToolsLanguageThesaurus
    Kontext "Thesaurus"
    wait 500
    if CurrentWord.GetSeltext = sWordInitial then
    	sWordToBeFound = ReplaceWith.Gettext
		if Alternatives.GetItemCount > 0 then
			printlog "Select last entry in alternatives list"
			Alternatives.Select Alternatives.GetItemCount

			if ReplaceWith.Gettext = sWordToBeFound then
				Warnlog "Though alternative was selected replace with was not changed"
			else
			    Alternatives.TypeKeys "<Space>"
				sWordToBeFound = ReplaceWith.Gettext
			end if
			Thesaurus.OK
			Call wTypeKeys "<Home><Shift End>"
			wait 200
			EditCopy
			if GetClipboardText <> sWordToBeFound then Warnlog "Text has not been correctly replaced, not => " + sWordToBeFound + " but => " + GetClipboardText
		else
			Warnlog "- No Alternative for current word"
			Thesaurus.Ok
		end if
	else
		Warnlog "Current word is not selected word !"
	end if

    '/// Close thesaurus and writerdocument ///
    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsThesaurusSynonyms

    Dim sWordInitial as string
    Dim sNumberOfSuggestions as string
    Dim sReplaceWithWord as string
    Dim TheFile as String

    PrintLog "- Tools / Thesaurus"

    printlog " Open document where the formatting is set to the language we wish to test."
    TheFile = Convertpath (gTesttoolpath + "writer\optional\input\spellcheck\spellcheck_" + iSprache + ".odt")
    if FileExists ( TheFile ) <> TRUE then
       Warnlog "Found no Document for the language you're testing in, Please inform the Test-Developer."
       exit sub
    end if

    select case iSprache
        case 01 : sWordInitial = "Morning"
        case 03 : sWordInitial = "Confirmam"
        case 07 : Printlog "- No Thesaurus in Russian!"
                  goto endsub
        case 31 : sWordInitial = "Hulp"
        case 33 : sWordInitial = "Payer"
        case 34 : sWordInitial = "Suficiente"
        case 36 : Printlog "- No Thesaurus in Hungarian!"
                  goto endsub
        case 39 : sWordInitial = "Lingua"
        case 45 : sWordInitial = "Først"
        case 46 : sWordInitial = "Däng"
        case 48 : sWordInitial = "wypowiedź"
        case 49 : sWordInitial = "Mann"
        case 50 : sWordInitial = "smisel"
        case 55 : Printlog "- No Thesaurus in Portuguese (Brazil) !"
        		  goto endsub
        case 81 : Printlog "- No Thesaurus in Japanese!"
                  goto endsub
        case 82 : Printlog "- No Thesaurus in Korean!"
                  goto endsub
        case 86 : Printlog "- No Thesaurus in Chinese (simplified)!"
                  goto endsub
        case 88 : Printlog "- No Thesaurus in Chinese (traditional)!"
                  goto endsub
        case 30, 35, 36, 37, 42, 47, 48, 51, 90
                  Printlog "- No Thesaurus available !"
                  goto endsub
        case else : QAErrorlog "The language " + iSprache + " has no defined errortext. No Test!"
                    goto endsub
    end select

    printlog "Open new preformatted-writerdocument"
    call hFileOpenLocally (TheFile,false)
    
    Call wTypeKeys sWordInitial
    Call wTypeKeys "<Shift Home>"

    printlog "Open contextmenu in wrong word"
    Call wTypeKeys "<Shift F10>"
    printlog "Select 'Synonyms' in context menu"
    Call hMenuSelectNr (13)
    sNumberOfSuggestions=MenuGetItemCount - 2
    '/// Check if there is a suggestion and replace word ///
    if sNumberOfSuggestions <=0 then
        Warnlog "- No synonyms in Context menu for the first wrong word!"
    else
        printlog "- Context menu has " + sNumberOfSuggestions + " synonyms for the 1. wrong word!"
        sReplaceWithWord=hMenuItemGetText (1)
        Call hMenuSelectNr (1)
        Call wTypeKeys "<Shift Mod1 Left>"
        try
            EditCopy
            if GetClipboardtext<> sReplaceWithWord then
                Warnlog "- 1. wrong Word has not been replaced with " + sReplaceWithWord + " but "+ GetClipBoardtext
            else
                printlog "- 1. wrong word has been replaced with the 1. entry in context menu!"
            end if
        catch
        endcatch
    end if
    Call hCloseDocument

endcase
