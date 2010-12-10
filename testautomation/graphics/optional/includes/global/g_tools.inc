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
'* short description : will be moved to autocorrection.bas/inc
'*
'\*************************************************************************************
'   sFile = gTesttoolpath + "graphics\required\input\silben_"+iSprache+sExt

'-------------------------------------------------------------------------------
testcase tiToolsThesaurus

    Dim sWord as string
    Dim sChangedWord as string
    Dim sReplaceWord as string
    Dim sAlternativeText as string
    Dim sExt as string
    Dim ZaehlerBedeutung
    dim sWorkFile as string

    'for normal text, the thesaurus is enabled,
    'if the format->character->language of the word has a thesaurus
    'unfortunately in textboxes this doesn't work, you can give the word a langauge,
    'but this is been ignored by our thesaurus
    'so especially for asian languages you have to set a default language in the options,
    'to be able to use the thesaurus there - fallback to englisch,
    'because there is no thesaurus for cjk languages

    'As long as OOo has no modules for that, disabled
    if gOOO = True then
        printlog "No spellchecking in OOo"
        goto endsub
    endif

    'First we make sure we test a language where Thesaurus
    select case iSprache
    case 01 : sWord = "Hello"
    case 03 : sWord = "Alo"
    case 07 : printlog "- No Thesaur for Russian available"
        goto endsub
    case 30 : Printlog "- No Thesaurus available !"
        goto endsub
    case 31 : sWord = "Guiten"
    case 33 : sWord = "Bonjour"
    case 34 : sWord = "Hola"
    case 35 : Printlog "- No Thesaurus available !"
        goto endsub
    case 36 : Printlog "- No Thesaurus available !"
        goto endsub
    case 37 : Printlog "- No Thesaurus available !"
        goto endsub
    case 39 : sWord = "Ciao"
    case 42 : Printlog "- No Thesaurus available !"
        goto endsub
    case 45 : sWord = "Hej"
    case 46 : sWord = "Välkommen"
    case 47 : Printlog "- No Thesaurus available !"
        goto endsub
    case 48 : Printlog "- No Thesaurus in Polish!"
        goto endsub
    case 49 : sWord = "Hallo"
    case 50 : sWord = "Prosojnica"
    case 51 : Printlog "- No Thesaurus available !"
        goto endsub
    case 55 : Printlog "- No Thesaurus in Brazilian!"
        goto endsub
    case 90 : Printlog "- No Thesaurus available !"
        goto endsub

    case else : if bAsianLan then
            hSetSpellHypLanguage
            sWord = "Hello"
        else
            Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here"
            sWord = "Hello"
        endif
    end select

    PrintLog "Thesaurus with 1 word: " + sWord

    if( gApplication = "DRAW" ) then
        sExt = ".odg"
    elseif( gApplication = "IMPRESS" ) then
        sExt = ".odp"
    endif

    sWorkFile = gTesttoolPath & "graphics\required\input\recht_" & iSprache & sExt

    Call hFileOpenLocally( sWorkFile )
    Call hTextrahmenErstellen (sWord,20,20,50,30)
    sleep 1

    printlog "select the word"
    hTypeKeys "<Home><SHIFT END>"

    try
        ExtrasThesaurusDraw
    catch
        warnlog "No Thesaurus available."
        hCloseDocument()
        goto endsub
    endcatch

    Kontext "Thesaurus"
    if Thesaurus.Exists(3) then

        if CurrentWord.GetSelText <> sWord Then
            WarnLog "marked word should be:" + sWord + " but it is:" + CurrentWord.GetSelText
        else
            PrintLog "Marked word is correctly changed"
        endif

        if Alternatives.GetItemCount = 0 Then
            warnlog "There are no alternative for this word"
            sleep 1
        else
            printlog "select the first alternative"
            Alternatives.Select 1

            printlog "check if the selected alternative apprear in the ReplaceWith field."

            sAlternativeText = Alternatives.getSeltext(1,3)
            'remove the part in the () if there is any
            if( Instr(sAlternativeText, "(") <> 0 ) then
                sAlternativeText = left(sAlternativeText, Instr(sAlternativeText, "(") - 2 )
            endif

            if ( ReplaceWith.getText() <> sAlternativeText ) then
                warnlog "the text is no transfered to the ReplaceWith field. " + "ReplaceWith = "  + ReplaceWith.getText() + ", Alternative = " + sAlternativeText
            else
                printlog "the text is transfered to the ReplaceWith field."
            endif

            sReplaceWord = ReplaceWith.getText()
            sleep 1
        endif
        Thesaurus.OK

        if(gApplication = "DRAW") then
            Kontext "Draw"
        elseif(gApplication = "IMPRESS") then
            Kontext "Impress"
        else ' fallback
            warnlog "gApplication is wrong"
            goto endsub
        endif

        hTypeKeys "<Home><SHIFT END>"
        EditCopy
        sChangedWord = getClipboard()
        printlog "sChangedWord = " + sChangedWord
        if(sChangedWord <> sReplaceWord) then
            warnlog "the text was not replaced"
        else
            printlog "the text was successfull replaced"
        endif
    else
        warnlog "thesaurus didn't come up."
    end if

    Call hCloseDocument

endcase 'tiToolsThesaurus
'-------------------------------------------------------------------------------
testcase tiToolsThesaurusWithoutText

    printlog "Thesaurus without any word in the document"
    Call hNewDocument
    try
        ExtrasThesaurusDraw
    catch
        printlog "the Thesaurus is correctly not available"
        call hCloseDocument
        goto endsub
    endcatch

    WarnLog "If no word is in the document, Thesaurus shouldn't be available"
    Kontext "Thesaurus"
    if Thesaurus.Exists then
        Thesaurus.Cancel
    endif

    Call hCloseDocument


endcase 'tiToolsThesaurusWithoutText
'-------------------------------------------------------------------------------
testcase tiToolsThesaurusContextMenu

    Dim sWord as string
    Dim sExt as string
    Dim iItemCount as Integer

    'for normal text, the thesaurus is enabled,
    'if the format->character->language of the word has a thesaurus
    'unfortunately in textboxes this doesn't work, you can give the word a langauge,
    'but this is been ignored by our thesaurus
    'so especially for asian languages you have to set a default language in the options,
    'to be able to use the thesaurus there - fallback to englisch,
    'because there is no thesaurus for cjk languages

    'As long as OOo has no modules for that, disabled
    if gOOO = True then
        printlog "No spellchecking in OOo"
        goto endsub
    endif

    'First we make sure we test a language where Thesaurus
    select case iSprache
    case 01 : sWord = "Hello"
    case 03 : sWord = "Alo"
    case 07 : printlog "- No Thesaur for Russian available"
        goto endsub
    case 30 : Printlog "- No Thesaurus available !"
        goto endsub
    case 31 : sWord = "Guiten"
    case 33 : sWord = "Bonjour"
    case 34 : sWord = "Hola"
    case 35 : Printlog "- No Thesaurus available !"
        goto endsub
    case 36 : Printlog "- No Thesaurus available !"
        goto endsub
    case 37 : Printlog "- No Thesaurus available !"
        goto endsub
    case 39 : sWord = "Ciao"
    case 42 : Printlog "- No Thesaurus available !"
        goto endsub
    case 45 : sWord = "Hej"
    case 46 : sWord = "Välkommen"
    case 47 : Printlog "- No Thesaurus available !"
        goto endsub
    case 48 : Printlog "- No Thesaurus in Polish!"
        goto endsub
    case 49 : sWord = "Hallo"
    case 50 : sWord = "Prosojnica"
    case 51 : Printlog "- No Thesaurus available !"
        goto endsub
    case 55 : Printlog "- No Thesaurus in Brazilian!"
        goto endsub
    case 90 : Printlog "- No Thesaurus available !"
        goto endsub

    case else : if bAsianLan then
            hSetSpellHypLanguage
            sWord = "Hello"
        else
            Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here"
            sWord = "Hello"
        endif
    end select

    PrintLog "Thesaurus with 1 word: " + sWord

    Call hNewDocument
    Call hTextrahmenErstellen (sWord,20,20,50,30)
    sleep 1

    printlog "select the word"
    hTypeKeys "<Home><SHIFT END>"

    hOpenContextMenu

    hMenuSelectNr(14) ' the synonyms
    iItemCount = hMenuItemGetCount
    printlog iItemCount
    if(iItemCount <= 2 ) then ' none and Thesaurus item
        warnlog "There should be a synonymus display for the word '" + sWord + "' but it isn't."
    endif

    hMenuSelectNr(iItemCount) ' the last item

    Kontext "Thesaurus"
    if Thesaurus.Exists(5) then
        printlog "Thesaurus dialog appear from the context menu"
        Thesaurus.Cancel
    else
        warnlog "the thesaurus dialog does not appear fro mthe context menu of a selected word"
    endif

    call hCloseDocument

endcase 'tiToolsThesaurusContextMenu
'-------------------------------------------------------------------------------
testcase tiToolsHyphenation1

   printlog "   - Hyphenation without any text"
   printlog "New impress document "
   Call hNewDocument									
   sleep 3
   ToolsLanguageHyphenationDraw								
   Kontext "Hyphenation"
    printlog "Try with empty document"
    if Hyphenation.Exists(1) = True Then
        Warnlog "  - Dialog ist da"
        Hyphenation.Cancel
    else
        Printlog "  - Hyphenation activated"
    end if
'   Kontext "DocumentImpress"
   printlog "   - Hyphenation with text, but without any possibility to break words"
   select case iSprache
       case 01 : hTextrahmenErstellen ("This ist a text with no split words",10,10,40,40)	
       case 03 : hTextrahmenErstellen ("Compare a tradu" + Chr(231) + Chr(227) + "o com o original",10,10,40,40)
       case 07 : qaerrorlog "- No Hypenation for Russian available"
                printlog "Close document"
                 Call hCloseDocument                    
                 goto endsub
       case 31 : hTextrahmenErstellen ("Het is mijn bedoeling om voor en hulpmiddel.",10,10,40,40)
       case 33 : hTextrahmenErstellen ("Merci beaucoup pout votre candidature du 13/11/97.",10,10,40,40)
       case 34 : hTextrahmenErstellen ("This ist a text with no spliting words",10,10,40,40)
       case 36 : hTextrahmenErstellen ("Semmi karhozattyoc nincz most azoknac, a kic az Iesus Christusban be oltattac",10,10,40,40)
       case 39 : hTextrahmenErstellen ("Il documento 'Senza nome 1' stato modififcato",10,10,40,40)
       case 45 : hTextrahmenErstellen ("at du har nok i dit hvis du",10,10,40,40 )
       case 46 : hTextrahmenErstellen ("Detta är en text utan delade ord",10,10,40,40)
       case 48 : hTextrahmenErstellen (" Jeoli jednak zechcesz",10,10,40,40)
       case 49 : hTextrahmenErstellen ("Dies ist ein Text ohne eine Moeglichkeit Woerter zu trennen",10,10,40,40)
       case 50 : hTextrahmenErstellen ("Zato ves �?as iš�?emo nove �?lane: sodelujte",10,10,40,40)
       case 55 : hTextrahmenErstellen ("a Sun tentou adquirir a Apple",10,10,40,40)
       case else : if bAsianLan then
                     printlog "For the language  " + iSprache +" nothing is prepared yet, but is AsianLan, so OK :-)"
                     printlog " :-) will use English instead"
                     hTextrahmenErstellen ("This ist a text with no split words",10,10,40,40)
                  else
                     Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here"
                  endif
   end select
   hTypeKeys "<Home>"
   ToolsLanguageHyphenationDraw							
   Kontext "Hyphenation"
   if Hyphenation.Exists =True Then
      Warnlog "Dialog exists"
      Hyphenation.Cancel
   else
      Kontext "Active"
      if Active.Exists then Active.OK
   end if
   printlog "Close document "
   Call hCloseDocument 										
   
endcase 'tiToolsHyphenation1

'-------------------------------------------------------------------------------
testcase tiToolsHyphenation2

    dim iWaitIndex as integer
    dim sExt as string
    Dim i : Dim DasWort2 : Dim DasWort3 : Dim DasWort4 : Dim DasWort5 : Dim DasWort6 : Dim DasWort7
    Dim DasWort as string
    dim sDraw as string
    dim sFile as string
    PrintLog "- Tools/Hyphenation2"
    Select Case gApplication
    case "DRAW"         : sExt = ".odg"
    case "IMPRESS"      : sExt = ".odp"
    end select

    if ((not bAsianLan) AND (iSprache <> 07)) then
        sFile = gTesttoolpath + "graphics\required\input\silben_"+iSprache+sExt
        Call hFileOpenLocally( sFile )
    endif

   select case iSprache					
       case 01 :DasWort = "ation" : DasWort2 = "tion" : DasWort3 = "ion" : DasWort4 = "on"  '+Chr(34)
       case 07 : qaerrorlog "- No Hypenation for Russian available"
                 goto endsub
       case 31 :DasWort = "siteit" : DasWort2 = "iteit" : DasWort3 = "teit" : DasWort4 = "eit"
       case 33 :DasWort = "ture" : DasWort2 = "ure" : DasWort3 = "re" : DasWort4 = "e"
       case 34 :DasWort = "chvo" : DasWort2 = "hvo" : DasWort3 = "vo" : DasWort4 = "o"
       case 36 :DasWort = "zatnal" : DasWort2 = "atnal" : DasWort3 = "tnal" : DasWort4 = "nal"
       case 39 :DasWort = "sentu" : DasWort2 = "entu" : DasWort3 = "ntu" : DasWort4 = "tu"
       case 46 :DasWort = "niska" : DasWort2 = "iska" : DasWort3 = "ska" : DasWort4 = "ka"
       case 48 :DasWort = "nie" : DasWort2 = "ie" : DasWort3 = "e"
       case 49 :DasWort = "nung" : DasWort2 = "ung" : DasWort3 = "ng" : DasWort4 = "g"
       case 50 :DasWort = "nica" : DasWort2 = "ica" : DasWort3 = "ca" : DasWort4 = "a"
       case 55 :DasWort = "gia" : DasWort2 = "ia" : DasWort3 = "a" : DasWort4 = "ogia"
       case else : if bAsianLan then
             printlog "For the language  " + iSprache +" nothing is prepared yet, but is AsianLan, so OK :-)"
             printlog " :-) will use English instead"
             printlog "Open prepared document"
             Call hFileOpen (gTesttoolpath + "graphics\required\input\silben_"+"1"+sExt)
             DasWort = "ation" : DasWort2 = "tion" : DasWort3 = "ion" : DasWort4 = "on"  '+Chr(34)
          else
             Warnlog "For the language  " + iSprache +" nothing is prepared yet: insert text here and create file: "+sFile
             hNewDocument
          endif
   end select

  printlog "Test hyphenation"
printlog "Press CTRL + A to select all"
   EditSelectAll 
   printlog "press return to go into the edit mode"
   hTypeKeys "<Return>"

   sleep 1
   ' WA for draw, or smth else: get text part before hyphenation:
   if iSprache = "1" OR bAsianLan = TRUE then
    printlog "Select part of text and compare it with original text at this place"
      hTypeKeys "<MOD1 END><HOME><MOD1 SHIFT RIGHT RIGHT>"      
   else
      hTypeKeys "<MOD1 END><HOME><MOD1 SHIFT RIGHT>"
   endif
   EditCopy
   sDraw = GetClipboardText()
   printlog "Select whole text"
   EditSelectAll
   printlog "Run Hyphenation"
   ToolsLanguageHyphenationDraw					
   gMouseClick 50,50
   printlog "Select part of text and compare it with original text at this place"
   hTypeKeys "<MOD1 END><HOME><MOD1 SHIFT RIGHT>"		
   sleep 3
   EditCopy

    printlog GetClipboardText
    select case GetClipboardText
    case DasWort  : printlog "  - Hyphenation worked"
    case DasWort2 : printlog "  - Hyphenation worked"
    case DasWort3 : printlog "  - Hyphenation worked"
    case DasWort4 : printlog "  - Hyphenation worked"
    case else : warnlog "  - Hyphenation didnt work"
    end select
    if GetClipboardText <> sDraw then
        Printlog " - WA draw hyphenation did work :-)"
    else
        Warnlog "  - Hyphenation did not work"
        Printlog "Expected: '"+DasWort+"'; or something different than: '"+sDraw+"'; But what we got was: '"+GetClipboardText+"'"
    end if
   sleep 1
   printlog "Close document"
 Call hCloseDocument						
 
endcase 'tiToolsHyphenation2

'-------------------------------------------------------------------------------
testcase tToolsMacro

qaerrorlog( "#i116017# - [Automation]: TypeKeys executes wrong commands" )
    goto endsub


    Dim i as integer
    Dim j as integer
    Dim Zaehler as integer
    Dim ZaehlerListe as integer

    printlog "New document"
    Call hNewDocument

    ToolsMacro
    Kontext "Makro"

    printlog "to get the count of all macros, the list needs to get elapsed-all"
    for j = 1 to 2
        Zaehler=MakroAus.GetItemCount
        MakroAus.typeKeys "<end>"
        for i = 1 to Zaehler
            MakroAus.typeKeys "<+>"
            sleep 1
            MakroAus.typeKeys "<up>"
        next i
    next j
    Zaehler=MakroAus.GetItemCount
    for i= 1 to Zaehler
        printlog "Starting Macro: (" + i + "/"+ Zaehler + ")"
        MakroAus.Select i
        if Makroname.GetText = "" Then
            try
                Ausfuehren.Click
                Aufzeichnen.Click
                Zuordnen.Click
                Bearbeiten.Click
                Warnlog "  - Controls should not be visible"
            catch
                Printlog "  Contrls invisible because no macro choosen"
            endcatch
        else
            SetClipboard = Makroname.GetText
            ZaehlerListe=MakroListe.GetItemCount
            for j=2 to ZaehlerListe
                Makroliste.Select j
                if Makroname.GetText<>GetClipboardText Then
                    Printlog "  Selection got used"
                else
                    Warnlog "  - Selection not right, it should be: " + Makroliste.GetSelText + " but it is: " + GetClipboardText
                end if
            next j
        end if
    next i

    Printlog "  run macros"
    MakroAus.Select 6  '3
    Ausfuehren.Click
    sleep 10
    if Makro.Exists Then
        Warnlog "  - Macro not executed"
    else
        Printlog "  Macro executed"
        ToolsMacro
        Kontext "Makro"
    end if

    Zuordnen.Click
    Kontext
    Active.SetPage TabCustomizeMenu
    Kontext "TabCustomizeMenu"
    If TabCustomizeMenu.Exists Then
        Printlog "  Assign macro works"
        TabCustomizeMenu.Cancel
        sleep 3
        Kontext "Makro"
    else
        Warnlog "  - Assigning macro does not work"
    end if

    Bearbeiten.Click
    try
        sleep 1 ' this sleep is necessary. it takes some time, until focuse switched to basic window!
        ViewNavigator
        kontext "NavigatorDraw"
        if NavigatorDraw.exists (5) then
            Warnlog "  - Edit macro does not work"
            ViewNavigator
        endif
    catch
        hCloseDocument
        Kontext "DocumentDraw"
        ToolsMacro
        Kontext "Makro"
        Printlog "  Edit macro works"
    endcatch

    MakroAus.typeKeys "<end>"
    Zaehler = Makroliste.GetItemCount
    Neu.Click ' same as delete
    kontext "NeuesModul"
    Modulname.SetText "NewMarco"
    NeuesModul.OK
    ToolsMacro
    Kontext "Makro"
    if (Makroliste.GetItemCount = (Zaehler+1)) Then
        Printlog "  New macro works"
    else
        Warnlog "  - Macro wasn't created"
    end if

    if (Makroliste.GetItemCount <> Makroliste.getSelIndex) Then
        Warnlog "  created Macro not selected"
        Makroaus.Select(Makroaus.GetItemCount)
    end if
    sleep 1
    Loeschen.Click
    sleep 1
    Kontext "Active"
    printlog Active.getText
    Active.Yes
    sleep 2
    Kontext "Makro"
    if (Makroliste.GetItemCount = 0) Then
        Printlog "  Delete macro works"
    else
        Warnlog "  - Macro wasn't deleted"
    end if
    sleep 2
    makro.close
    'TODO: organizer testing!
    Call hCloseDocument
    Call hCloseDocument

endcase 'tToolsMacro
