'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_spellcheck.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-18 15:03:58 $
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
'* owner : fredrik.haegg@sun.com
'*
'* short description :
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

end sub


testcase tToolsSpellcheckWithShortcut
    '/// Check if Spellcheck dialog comes up with shortcut 'F7'
    Call hNewDocument
    Call wDokSchreiben "Check if spellchecker comes up!" 
    wait 500    
    Call wDokSchreiben "<HOME>"         'Setting cursor to beginning
    Call wDokSchreiben "<F7>"           'Calling spellchecker with shortcut
    sleep (1)
    
    Kontext "Active"
    if Active.Exists(1) then        'checking if messagebox comes up
        if Active.getRT = 304 then      'checking ressource type 
            Active.OK
        else
            printlog "No message Box!"
        endif
        printlog "Message Box closed, Spellchecker came up"
    else
        Kontext "Spellcheck"            'checking if spellcheck dialog came up
        if Spellcheck.Exists then
            printlog "OK, Spellchecker comes up"
            Spellcheck.Close            'closing spellchecker
        else
            Warnlog "#i35000#Spellchecker does not come up with shortcut 'F7'"
        endif
    endif
    Call hCloseDocument
endcase


testcase tToolsSpellcheckCorrect
    Dim sHyphenationFile as String
    Dim sLocale(3) as string
    '/// Checking if dictionary-files exists ///
    sLocale(3) = fgetDocumentLanguages(sLocale(), true)
    
    Printlog "- Checking Dictionary-Files"
    select case sLocale(1)
        case 01 : sHyphenationFile = "01-44-hyph.dat"
        case 81 : Printlog "- No Spellcheck in Japanese!"
                  goto endsub
        case 82 : Printlog "- No Spellcheck in Korean!"
                  goto endsub
        case 86 : Printlog "- No Spellcheck in Chinese (simplified)!"
                  goto endsub
        case 88 : Printlog "- No Spellcheck in Chinese (traditional)!"
                  goto endsub
        case else : sHyphenationFile =  "" & sLocale(1) & "-hyph.dat"
    end select
    if gNetzInst = TRUE then
        sHyphenationFile = Convertpath (gNetzOfficePath + "share\dict\" + trim$(sHyphenationFile))
    else
        sHyphenationFile = Convertpath (gOfficePath + "share\dict\" + trim$(sHyphenationFile))
    endif

    PrintLog "- Tools / Spellcheck without text"
    '/// New writerdocument ///
    Call hNewDocument
    Sleep 1
    '/// Call Spellcheck in empty document ///
    ToolsSpellcheck
    Sleep 2
    Kontext "Active"
    if NOT Active.Exists(1) then
        Warnlog "No Finished message"
    else
        Active.OK
    end if

    Dim FFText as String
    PrintLog "- Tools / Spellcheck with correct text"
    '/// Call Spellcheck for correct text ///
    select case sLocale(1)
        case 01   : FFText = "This is a text without any error.<Return>"
        case 03   : FFText = "Escreva os seus dados de utilizador correctamente e tome nota deles (guardar / imprimir).<Return>"
        case 07   : FFText = "принятый Государственной Думой в первом чтении 16 ноября 1999 года.<Return>"
        case 31   : FFText = "Dit werk is beschermd door de Auteurswet.<Return>"
        case 33   : FFText = "Il nous faut donc un de temps pour examiner avec soin tous les dossiers.<Return>"
        case 34   : FFText = "Este archivo contiene información importante sobre el programa.<Return>"
        case 36   : FFText = "Boszniai amerikai kontingens háromnegyedesek – mintegy négyezer katona – magyarországi telepítése egy éven belül megtörténhet.<Return>"
        case 39   : FFText = "La ringraziamo per l'interesse mostrato a collaborare con la firma.<Return>"
        case 45   : FFText = "Danmark ikke er sikret maksimal indflydelse på den fremtidige europæiske udlændingepolitik.<Return>"
        case 46   : FFText = "Den här filen innehåller viktig programinformation.<Return>"
        case 48   : FFText = "Nie wolno w zadnim wypadku przerywa uruchomionego serwisu !<Return>"
        case 49   : FFText = "Dies ist ein Text ohne Fehler.<Return>"
        case 55	  : FFText = "Ele ouviu passos silenciosos atrás dele. Isso não estava certo.<Return>"
        case else : QAErrorLog " - The language " + sLocale(1) +" has no defined language text. No Test !"
                    exit Sub
    end select

    Call wDokSchreiben FFText
    wait 500
    ToolsSpellcheck
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
    Dim sLocale(3) as string
    '/// Checking if dictionary-files exists ///
    sLocale(3) = fgetDocumentLanguages(sLocale(), true)
    
    PrintLog "- Tools / Spellcheck with a defined mistake"
    Select Case sLocale(1)
        case 01   : sErrorSampleText = "Thats not a trick, it's a Sony."
                    sWordToBeChecked = "Thats"
                    sLanguageName = "English (USA)"
        case 03   : sErrorSampleText = "Escrive os seus dados de utilizador correctamente e tome nota deles."
                    sWordToBeChecked = "Escrive"
                    sLanguageName = "Português"
        case 07   : sErrorSampleText = "Военная технического сaтрудничества, проблему задолженности и вопросы региональных связей."
                    sWordToBeChecked = "сaтрудничества"
                    sLanguageName = "Русский"        
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
    '/// New writerdocument ///
    Call hNewDocument
    '/// Insert a sentence with one definded mistake and execute spellcheck ///
    Call wDokSchreiben sErrorSampleText
    Call wDokSchreiben "<Return><Up>"
	wait 500
    ToolsSpellcheck
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
            catch
                Warnlog Active.Gettext
                Active.Ok
            endcatch
        endif
    endif
    Sleep 2
    Kontext "Spellcheck"
    if Not Spellcheck.Exists then
        Warnlog "Spellchecker is not up !"
        Call hCloseDocument
        goto endsub
    endif
    printlog "Current selected dictionary language is: " & DictionaryLanguage.GetSelText 
    '  Dictionary Language is language dependend. Function to be created
    printlog "   - check language"
    'if DictionaryLanguage.GetSelText <> gLanguage then Warnlog "Dictionary language " + DictionaryLanguage.GetSelText +" selected"
    printlog "   - replace wrong word with a suggestion"
    '/// replace wrong word with a suggestion ///
    if trim$(NotInDictionary.GetText) <> trim(sErrorSampleText) then
        QAErrorlog "#i79988#" & sErrorSampleText & " has not been detected as mistake. Detected mistake is:" & NotInDictionary.GetText
    endif
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
		endif
    else
        QAErrorLog "Please choose another word to check suggestions, because no suggestion found!"
    endif
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

	Call wDokSchreiben "<Up><Home>"
    Call wDokSchreiben "<Shift End>"
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
    Dim sLocale(3) as string
    '/// Checking if dictionary-files exists ///
    sLocale(3) = fgetDocumentLanguages(sLocale(), true)
    
    PrintLog "- Tools / Spellcheck / check forward"
    '/// Open specified document depending on language to check spellcheck ///
    '/// document has 4 defined wrong words ///
    InsFile = wGetSpellcheckDocumentName(1)
    Select Case InsFile
        Case "None"
            Printlog "- No Spellcheck for this language available!"
            goto endsub
        case "Empty"
            QAErrorLog "The language " + iSystemSprache +" has no defined Errortext. No test!"
            goto endsub
    end select
    
    if gApplication = "MASTERDOC" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
            EditPaste
    else
        Call hFileOpen(InsFile)
    endif
    '/// point cursor to beginning of document and execute spellcheck ///
    Call wDokSchreiben "<Mod1 Home>"
    
    select case sLocale(1)
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
        case 55   : sDummy = "horat"        : sWordToBeAdded = "dpa"
    end select

    wait 500
    ToolsSpellcheck
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
	endif

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
    try
        IgnoreOnce.Click
    catch
        Warnlog "Button 'Ignore once' is not accessible"
    endcatch
    Sleep 1

    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Warnlog "Spellcheck has ended! Not reproducable by hand !"
        endif
    else
        Kontext "Spellcheck"
        Add_Word = trim$(NotInDictionary.Gettext)
        Printlog "   - Add (" + sWordToBeAdded + ")"
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
		endif
    endif

	Kontext "Active"
	if Active.Exists then
		If Active.GetRT = 304 then
			Active.Ok
		endif
	endif
	
    Kontext "Spellcheck"
    if Spellcheck.Exists then Spellcheck.Close
    Call wDokSchreiben "<Home>"

    Printlog "   - recheck: One mistake has to be found 3 times"
    wait 500
    '/// Recheck: One mistake has to be found 3 times ///
    ToolsSpellcheck
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
        endif
    endif
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
        endif
    endif
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
    Call WortAusWoerterbuchLoeschen ( sWordToBeAdded )

    '/// Close document ///
    Call hCloseDocument
endcase

' *******************************************************************

testcase tToolsSpellcheckOnlyOneWord
    Dim sWordToBeChecked as string
    Dim InsFile as string
    Dim ReplacedWith as String
    Dim i as integer
    Dim sLocale(3) as string
    '/// Checking if dictionary-files exists ///
    sLocale(3) = fgetDocumentLanguages(sLocale(), true)
        
    '/// Open specified document depending on language to check spellcheck ///
    '/// document has 1 defined wrong word ///
    PrintLog "- Tools / Spellchecking / Check only one word"
    InsFile = wGetSpellcheckDocumentName(2)
    Select Case InsFile
        Case "None"
            Printlog "- No Spellcheck for this language available!"
            goto endsub
        case "Empty"
            QAErrorLog "The language " + iSystemSprache +" has no defined Errortext. No test!"
            goto endsub
    end select
    
    if gApplication = "MASTERDOC" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
            EditPaste
    else
        Call hFileOpen(InsFile)
    endif
    Call wDokSchreiben "<Mod1 Home>"
    
    select case sLocale(1)
        case 01   : sWordToBeChecked = "excetionally"      ' this word is wrong
        case 07   : sWordToBeChecked = "Кaнституции"
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
        endif
    endif

    Kontext "FindAndReplace"
    if FindAndReplace.Exists then FindAndReplace.Close

    ToolsSpellcheck
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
        endif
    endif

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
		endif
    else
        Warnlog "No suggestion for word: " & sWordToBeChecked & " found. Test stops"
        Spellcheck.Close
        Call hCloseDocument
        goto endsub
    endif

    
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Ok
            catch
                Active.No
                Warnlog Active.Gettext
            endcatch
        endif
    endif       
    
    Kontext "Spellcheck"
    if Spellcheck.Exists then SpellcheckClose.Click
    
    Call wDokSchreiben "<Mod1 Shift Left>"
    try
        EditCopy
        if GetClipboardtext = sWordToBeChecked then
            Warnlog "#i40498#Spellchecker does not commit changes correctly (" & GetClipboardtext & " found )"
            Call hCloseDocument
            goto endsub
        endif
    catch
        Warnlog "- Didn't found the word to check! No Spellchecking for one word!"
        Call hCloseDocument
        goto endsub
    endcatch

    '/// Execute spellcheck again ///
    wait 500
    ToolsSpellcheck
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.Yes
                Kontext "Active"
                if Active.Exists then
                    if Active.GetRT = 304 then
                        Active.Ok
                        Warnlog "#i40480#Spellchecker finishes though wrong words in documents"
                    endif
                endif
            catch
                Active.Ok
            endcatch
        endif
    else
        Sleep 2
        Kontext "Spellcheck"
        if Spellcheck.Exists then
            Warnlog "Probably the replaced word found wrong again!"
            Spellcheck.Close
        endif
    endif
        
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
    Dim sLocale(3) as string
    '/// Checking if dictionary-files exists ///
    sLocale(3) = fgetDocumentLanguages(sLocale(), true)
        
    PrintLog "- Tools / Spellcheck / AutoSpellcheck"
    '/// New writerdocument ///
    Call hNewDocument
    '/// Enable Auto-Spellcheck in options
    Call wEnableAutoSpellcheck ( true ) 
    
    '/// Check Auto-Spellcheck and insert a sentence with 2 mistakes ///
    printlog "   - write text"
    select case iSprache
        case 01   : wDokSchreiben "Hello thiis here are too errorrs "
                  iError1 = 1 : iError2 = 4
                  sError1 = "thiis"  : sError1 = "errorrs"
                  sDefaultFormat = "Standard"
        case 03   : wDokSchreiben "Aqui poderá elimitnar componentes já instalados ou instalar novos compotentes numa "
                  iError1 = 2 : iError2 = 7
                  sError1 = "elimitnar"  : sError1 = "compotentes"
                  sDefaultFormat = "Padrão"
        case 31   : wDokSchreiben "Betaling binnern 14 dagen rekeningsdatum met 2% kortng, binnen 30 dagen nettor "
                  iError1 = 1 : iError2 = 7
                  sError1 = "binnern"  : sError1 = "rekeningsdatum"
                  sDefaultFormat = "Standaard"
        case 33   : wDokSchreiben "Je vious remercie d'accuser réception dex ce courrier "
                  iError1 = 1 : iError2 = 4
                  sError1 = "vious"  : sError1 = "dex"
                  sDefaultFormat = "Standard"
        case 34   : wDokschreiben "Este archvo contiene información importnte sobre el programa."
                  iError1 = 1 : iError2 = 3
                  sError1 = "archvo"  : sError1 = "importnte"
                  sDefaultFormat = "Estándar"
        case 36   : wDokSchreiben "A hétezres koszovói amerikai kontingens jelentos részét is a magyarországi bázison helyezi majd el."
                  iError1 = 1 : iError2 = 4
                  sError1 = "hétezres"  : sError1 = "jelentos"
                  sDefaultFormat = "Alapértelmezett"                
        case 39   : wDokSchreiben "La sfumaturar è statsi modificata senza salvare "
                  iError1 = 1 : iError2 = 2
                  sError1 = "sfumaturar"  : sError1 = "statsi"
                  sDefaultFormat = "Standard"
        case 45   : wDokSchreiben "Han advarade både mod at kalde traktüten et fredens projekt "
                  iError1 = 1 : iError2 = 5
                  sError1 = "advarade"  : sError1 = "traktüten"
                  sDefaultFormat = "Standard"
        case 46   : wDokSchreiben "Den här fiten innehåller viktit programinformation "
                  iError1 = 2 : iError2 = 2
                  sError1 = "fiten"  : sError1 = "viktit"
                  sDefaultFormat = "Standard"
        case 48   : wDokSchreiben "Nie wolno w zadim wypadku przerywa uruchomiongo serwisu "
                  iError1 = 3 : iError2 = 3
                  sError1 = "zadim"  : sError1 = "uruchomiongo"
                  sDefaultFormat = "Standard"
        case 49   : wDokSchreiben "Hallo hiaer sind swei Fehler "
                  iError1 = 1 : iError2 = 2
                  sError1 = "hiaer"  : sError1 = "swei"
                  sDefaultFormat = "Standard"
        case 55   : wDokSchreiben "Aqui poderá elimitnar componentes já instalados ou instalar novos compotentes numa "
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
        case else : QAErrorlog "The language " + iSystemSprache +" has no defined errortext. No Test!"
                  Call hCloseDocument
                  goto endsub
    end select

    Call wDokSchreiben "<Space><Return>"
    
    printlog "   -  relieve mistake"
    Call wDokSchreiben "<Up><Home>"
    Sleep 5
    Call wDokSchreiben "<Mod1 Right>", iError1
    Call wDokSchreiben "<Right>"          ' Cursor in fehlerhaftes NotInDictionary stellen
    '/// Open contextmenu in first wrong word ///
    Call wOpenContextMenu
    sReplaceWithWord=hMenuItemGetText (2)
    if sReplaceWithWord<>sDefaultFormat then
        sNumberOfSuggestions=MenuGetItemCount - 6
        '/// Check if there is a suggestion and replace word ///
        if sNumberOfSuggestions <=0 then
            Warnlog "- No suggestions in Context menu for the first wrong word!"
        else
            printlog "- Context menu has " + sNumberOfSuggestions + " suggestions to the 1. wrong word!"
            Call hMenuSelectNr (2)
            Call wDokSchreiben "<Shift Mod1 Left>"
            try
                EditCopy
                if GetClipboardtext<> sReplaceWithWord then
                    Warnlog "- 1. wrong Word has not been replaced with " + sReplaceWithWord + " but "+ GetClipBoardtext
                else
                    printlog "- 1. wrong word has been replaced with the 1. entry in context menu!"
                endif
            catch
            endcatch
        endif
    else
        Warnlog "- AutoSpellcheck doesn't work , wrong Context menu! (1.Word)"
    endif

    Call wDokSchreiben "<Mod1 Right>", iError2
    Call wDokSchreiben "<Right>"          ' Cursor in fehlerhaftes NotInDictionary stellen
    Call wOpenContextMenu
    sReplaceWithWord=hMenuItemGetText (2)
    if sReplaceWithWord<>sDefaultFormat then
        sNumberOfSuggestions=MenuGetItemCount - 6
        if sNumberOfSuggestions <=0 then
            Warnlog "- No suggestions in Contextmenu for 2. wrong Word !"
        else
            printlog "- Context menu has " + sNumberOfSuggestions + " suggestions for the 2. wrong word!"
            Call hMenuSelectNr (2)
            Call wDokSchreiben "<Shift Mod1 Left>"
            try
                EditCopy
                if GetClipboardtext<> sReplaceWithWord then
                    Warnlog "- 2. wrong word has not been replaced with " + sReplaceWithWord + " but "+ GetClipBoardtext
                else
                    printlog "- 2. mistake has been replaced with the 1. entry in context menu!"
                endif
            catch
            endcatch
        endif
    else
        Warnlog "- AutoSpellcheck doesn't work, wrong Context menu (2.Word)!"
    endif

    Call wDokSchreiben "<Home>"
    Call wDokSchreiben "<F7>"
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
        endif
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



testcase tToolsSpellcheckChangeLanguageAttribute
      
    Dim sWordToBeChecked as string
    Dim InsFile as string
    Dim i as integer
    Dim ReplacedWith as string
    Dim PriorLanguage as string
    Dim CurrentLanguage as string
    Dim sLocale(3) as string
    '/// Checking if dictionary-files exists ///
    sLocale(3) = fgetDocumentLanguages(sLocale(), true)
        
    '/// Open specified document depending on language to check spellcheck ///
    '/// document has 1 defined wrong word ///
    PrintLog "- Tools / Spellchecking / Check only one word"
    InsFile = wGetSpellcheckDocumentName(2)
    Select Case InsFile
        Case "None"
            Printlog "- No Spellcheck for this language available!"
            goto endsub
        case "Empty"
            QAErrorLog "The language " + iSystemSprache +" has no defined Errortext. No test!"
            goto endsub
    end select
    
    if gApplication = "MASTERDOC" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
            EditPaste
    else
        Call hFileOpen(InsFile)
    endif
    Call wDokSchreiben "<Mod1 Home>"
    
    select case sLocale(1)
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
        endif
    endif
    Kontext "FindAndReplace"
    if FindAndReplace.Exists then FindAndReplace.Cancel
    EditCopy

    '/// Execute spellcheck: change language Attributes
    wait 500
    ToolsSpellcheck
    Sleep 2
    Kontext "Spellcheck"
    if Not Spellcheck.Exists then
        Kontext "Active"
        If Active.Exists then
            If Active.GetRT = 304 then
                Warnlog Active.Gettext
                Warnlog "- Probably didn't found the word to check! No Spellchecking for one word!"
                Active.ok
            endif
        endif
        ToolsSpellcheck
        Kontext "Spellcheck"
    endif
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
                endif
            endif        
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
                endif
            endif
            ' Finished message ?
            Kontext "Active"
            if Active.Exists then
                if Active.GetRT = 304 then
                    try
                        Active.Ok
                    catch
                    endcatch
                endif
            endif
            wait 500
            Kontext "Spellcheck"
            if Spellcheck.Exists then
                SpellcheckClose.Click
            endif
            wait 500
            Call wDokSchreiben "<Mod1 Home>"
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
                endif
            endif
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
                    endif
                endif
            catch
                if LanguageWest.GetSelText <> CurrentLanguage then
                    if LanguageWest.GetSelText = PriorLanguage then
                        Warnlog "Language Attributes has not been set to" & CurrentLanguage
                    else
                        Warnlog "Language Attributes has not been set to " & CurrentLanguage & " but " & LanguageWest.GetSelText                    
                    endif
                endif
            endcatch
            TabFont.Cancel
        else
            QAErrorlog "No suggestions found! Maybe wrong word should be changed to run more deeper testing"
        endif
    else
        Warnlog "Spellcheck dialog is not up !"
    endif    
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
            QAErrorLog "The language " + iSystemSprache +" has no defined Errortext. No test!"
            goto endsub
    end select
    
    if gApplication = "MASTERDOC" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
            EditPaste
    else
        Call hFileOpen(InsFile)
    endif
    Call wDokSchreiben "<Mod1 Home>"

    '/// invoke the spellchecker
    ToolsSpellcheck
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
                            endif
                         endif
                     endif
                next i    
            endif
        endif
    endif
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
            QAErrorLog "The language " + iSystemSprache +" has no defined Errortext. No test!"
            goto endsub
    end select
    
    if gApplication = "MASTERDOC" then
        if wInsertDocumentinMasterDoc(InsFile) = False then goto endsub
            EditPaste
    else
        Call hFileOpen(InsFile)
    endif
    Call wDokSchreiben "<Mod1 Home>"

    '/// invoke the spellchecker
    ToolsSpellcheck
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
                endif
            else
                Warnlog "Button 'Change all' should be enabled"
            endif
            SpellcheckClose.Click
        endif
    endif
    '/// Close document
    Call hCloseDocument
    
endcase

' *******************************************************************

testcase tToolsThesaurus
    Dim sWordInitial as string
    Dim sLanguageToBeSelected as string
    Dim sWordToBeFound as string
    Dim sWordReplacedWith as string
    Dim i as integer
    Dim sLocale(3) as string
    '/// Checking if dictionary-files exists ///
    sLocale(3) = fgetDocumentLanguages(sLocale(), true)
    
    PrintLog "- Tools / Thesaurus"

    select case sLocale(1)    
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
        case else : QAErrorlog "The language " +  iSystemSprache + " has no defined errortext. No Test!"
                    goto endsub
    end select

    Call hNewDocument
    '/// New writerdocument ///
    printlog "-  Thesaurus without a word"
    '/// Call Thesaurus without a written word ///
	ToolsLanguageThesaurus
    Kontext "Thesaurus"
    if Ersetzen.GetText <> "" then Warnlog "- Though no word has been selected there is a replacetext in the textfield!"
    printlog "-  enter one word and search"
    Ersetzen.SetText sWordInitial
    Nachschlagen.Click
    Kontext "Active"
    if Active.Exists then
        Warnlog Active.Gettext
        Active.ok
    endif
    Kontext "Thesaurus"
    if Bedeutung.GetSelText(1) = "" then Warnlog "No meaning text"
    if Ersetzen.Gettext = "" then Ersetzen.SetText = sWordInitial
    sWordToBeFound = Ersetzen.GetText
    Thesaurus.OK
    Call wDokSchreiben "<Home><Shift End>"
    wait 200
    EditCopy
    if iSystemSprache = 1 then
        if GetClipboardText <> sWordToBeFound then Warnlog "Wrong word has been replaced, not => " + sWordInitial + " but => " + GetClipboardText
    else
        if LCase (GetClipboardText) <> LCase (sWordToBeFound) then Warnlog "Wrong word has been replaced, not => " + sWordToBeFound + " but => " + GetClipboardText
    end if
    Call hCloseDocument
    wait 500
    Call hNewDocument

    printlog "-  check the language"
    Call wDokSchreiben sWordInitial + "<Home>"
    '/// Check language in Thesaurus ///
    ToolsLanguageThesaurus
    Kontext "Thesaurus"

    Sprache.Click
    Kontext "SpracheAuswaehlen"
    select case iSystemSprache    
        case 01 : sLanguageToBeSelected = "English (USA)"
        case 07 : sLanguageToBeSelected = "Русский"
        case 03 : sLanguageToBeSelected = "Português"
        case 31 : sLanguageToBeSelected = "Nederlands (NL)"
        case 33 : sLanguageToBeSelected = "Français (France)"
        case 34 : sLanguageToBeSelected = "Español (España)"
        case 36 : sLanguageToBeSelected = "Madyar"
        case 39 : sLanguageToBeSelected = "Italiano (Italia)"
        case 45 : sLanguageToBeSelected = "Dansk"
        case 46 : sLanguageToBeSelected = "svenska (Sverige)"
        case 48 : sLanguageToBeSelected = "Polski"
        case 49 : sLanguageToBeSelected = "Deutsch (Deutschland)"
    end select
    'if Auswahl.GetSelText <> gLanguage then Warnlog "Selected language is " + Auswahl.GetSelText + " and not "+ sLanguageToBeSelected
    SpracheAuswaehlen.OK
    Sleep 5

    Kontext "Thesaurus"
    printlog "-  Search for a Synonym"
    '/// Search for a synonym ///
    Synonym.Select 1
    wait 500
    sWordToBeFound = Synonym.GetSelText
    if Ersetzen.GetText <> sWordToBeFound then Warnlog "Synonym has not been taken over"
    Nachschlagen.Click
	wait 500
	if Wort.GetSelText <> sWordToBeFound then Warnlog "Synonym has not been added to the word listand activated"
	
    printlog "-  Search for a meaning"
    '/// Search for a meaning ///
    wait 500
	Bedeutung.Select 1
    wait 500
	Nachschlagen.Click
    Sleep 1
    Kontext "Active"
    if Active.Exists then
        printlog "-   " + Active.Gettext
        Active.Ok
        Kontext "Thesaurus"
    else                                               ' Nur wenn Wort auch enthalten ist
        Kontext "Thesaurus"
        for i = 1 to 100
            if Bedeutung.GetSeltext <> "" then i = 101
            Sleep 1
        next i
        sWordToBeFound = Bedeutung.GetSelText
        if Ersetzen.GetText <> sWordToBeFound then Warnlog "Meaning has not been overtaken"
    endif

    printlog "-  Replace unchecked ( Cursor in front of words 1. character )"
    '/// replace unchecked ///
    sWordReplacedWith = Ersetzen.GetText
    Thesaurus.OK
    Call wDokSchreiben "<Shift Home>"
    wait 200
    EditCopy
    if GetClipboardText <> sWordReplacedWith then Warnlog "Text has not been correctly replaced, not => " + sWordReplacedWith + " but => " + GetClipboardText

    Call wDokSchreiben "<End><Shift Home><Delete>" + sWordInitial
    Call wDokSchreiben "<Home><Right><Right>"

    printlog "-  Replace unchecked ( Cursor in word )"
    ToolsLanguageThesaurus
    Kontext "Thesaurus"
    wait 500
    if Synonym.GetItemCount >0 then
        Synonym.Select 1
        sWordReplacedWith = Ersetzen.GetText
        Thesaurus.OK
        Call wDokSchreiben "<Home><Shift End>"
        wait 200
        EditCopy
        if GetClipboardText <> sWordReplacedWith then Warnlog "Text has not been correctly replaced, not => " + sWordReplacedWith + " but => " + GetClipboardText
    else
        Warnlog "- No Synonym when cursor is in word!"
        Thesaurus.Ok
    endif

    Call wDokSchreiben "<End><Shift Home><Delete>"
    Call wDokSchreiben sWordInitial + "<End><Left>"

    printlog "-  Replace unchecked ( Cursor at words end )"
    ToolsLanguageThesaurus
    Kontext "Thesaurus"
    if Synonym.GetItemCount >0 then
        Synonym.Select 1
        sWordReplacedWith = Ersetzen.GetText
        Thesaurus.OK
        Call wDokSchreiben "<Home><Shift End>"
        wait 200
        EditCopy
        if GetClipboardText <> sWordReplacedWith then Warnlog "Text has not been correctly replaced, not => " + sWordReplacedWith + " but => " + Trim$(GetClipboardText)
    else
        Warnlog "- No sysnonym when cursor pointed to words end!"
        Thesaurus.Ok
    endif

    Call wDokSchreiben "<End><Shift Home><Delete>"
    Call wDokSchreiben sWordInitial + "<End><Left><Shift Mod1 Left>"

    printlog "-  Replace checked ( back to front )"
    '/// replace checked ///
    ToolsLanguageThesaurus
    Kontext "Thesaurus"
    if Synonym.GetItemCount >0 then
        Synonym.Select 1
        sWordReplacedWith = Ersetzen.GetText
        Thesaurus.Ok
        Call wDokSchreiben "<Home><Shift End><Shift Left>"
        wait 200
        EditCopy
        if GetClipboardText <> sWordReplacedWith then Warnlog "Text has not been correctly replaced, not => " + sWordReplacedWith + " but => " + GetClipboardText
    else
        Nachschlagen.Click
        Kontext "Active"
        if Active.Exists then
            Active.Ok
            Kontext "Thesaurus"
            Thesaurus.OK
        endif
    endif

    Call wDokSchreiben "<End><Shift Home><Delete>"
    Call wDokSchreiben sWordInitial + "<Home><Shift End>"
    printlog "-  Replace checked ( front to back )"
    ToolsLanguageThesaurus
    Kontext "Thesaurus"
    if Synonym.GetItemCount >0 then
        Synonym.Select 1
        sWordReplacedWith = Ersetzen.GetText
        Thesaurus.Ok
        Call wDokSchreiben "<Home><Shift End>"
        wait 200
        EditCopy
        if GetClipboardText <> sWordReplacedWith then Warnlog "Text has not been correctly replaced, not => " + sWordReplacedWith + " but => " + GetClipboardText
    else
        Warnlog "- No sysnonym when word is marked!"
        Thesaurus.Ok
    endif

    Call wDokSchreiben "<End><Shift Home><Delete>"
    Call wDokSchreiben sWordInitial + "<Home><Right><Shift Right><Shift Right>"

    printlog "-  Part of Replace checked ( back to front )  - no checking anymore"
    ToolsLanguageThesaurus
    Kontext "Thesaurus"
    Thesaurus.OK
    '/// Close thesaurus and writerdocument ///
    Call hCloseDocument
endcase
