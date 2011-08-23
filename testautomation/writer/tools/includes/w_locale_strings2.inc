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
'/***********************************************************************
'*
'* owner : helge.delfs@oracle.com
'*
'* short description : defining locale dependant strings for language guessing
'*
'\***********************************************************************
function wGetLanguageGuessString(sValue as string, iLanguageCount as integer) as string 'function for getting strings out of this file

    'Variables:
    DIM WordPreSet as string 'string as it shoud be for words
    DIM ParagraphPreSet as string 'string as it shoud be for paragraphs

    '///already covered Office languages: ///'
    '///english     01 ///'
    '///russian     07 ///'
    '///dutch       31 ///'
    '///french      33///'
    '///spanish     34 ///'
    '///hungarian   36 ///'
    '///italian     39 ///'
    '///swedish     46 ///'
    '///polish      48 ///'
    '///german      49 ///'
    '///slovenian      50 ///'
    '///german      55 ///'

    '///already covered languages in the test document: ///'
    '///Case 1: english ///'
    '///Case 2: portugese///'
    '///Case 3: russian///'
    '///Case 4: dutch///'
    '///Case 5: french///'
    '///Case 6: spanish///'
    '///Case 7: hungarian///'
    '///Case 8: italian///'
    '///Case 9: swedish///'
    '///Case 10: polish///'
    '///Case 11: german///'

    Select Case iSprache        'Detecting Office language
    Case 01         'Case 01:  english
        '-----------------------------------------------------------------------
        Select Case iLanguageCount
        Case 1: WordPreSet      = "Word is English (USA)"
            ParagraphPreSet = "Paragraph is English (USA)"
        Case 2: WordPreSet      = "Word is Portuguese (Portugal)"
            ParagraphPreSet = "Paragraph is Portuguese (Portugal)"
        Case 3: WordPreSet      = "Word is Russian"
            ParagraphPreSet = "Paragraph is Russian"
        Case 4: WordPreSet      = "Word is Dutch (Netherlands)"
            ParagraphPreSet = "Paragraph is Dutch (Netherlands)"
        Case 5: WordPreSet      = "Word is French (France)"
            ParagraphPreSet = "Paragraph is French (France)"
        Case 6: WordPreSet      = "Word is Spanish (Spain)"
            ParagraphPreSet = "Paragraph is Spanish (Spain)"
        Case 7: WordPreSet      = "Word is Hungarian"
            ParagraphPreSet = "Paragraph is Hungarian"
        Case 8: WordPreSet      = "Word is Italian (Italy)"
            ParagraphPreSet = "Paragraph is Italian (Italy)"
        Case 9: WordPreSet      = "Word is Swedish (Sweden)"
            ParagraphPreSet = "Paragraph is Swedish (Sweden)"
        Case 10:WordPreSet      = "Word is Polish"
            ParagraphPreSet = "Paragraph is Polish"
        Case 11:WordPreSet      = "Word is German (Germany)"
            ParagraphPreSet = "Paragraph is German (Germany)"
        end select
        '-----------------------------------------------------------------------
    Case 07        'Case 07: russian
        Select Case iLanguageCount
        Case 1: WordPreSet      = "Язык слова: Английский (США)"
            ParagraphPreSet = "Язык абзаца: Английский (США)"
        Case 2: WordPreSet      = "Язык слова: Португальский (Португалия)"
            ParagraphPreSet = "Язык абзаца: Португальский (Португалия)"
        Case 3: WordPreSet      = "Язык слова: Русский"
            ParagraphPreSet = "Язык абзаца: Русский"
        Case 4: WordPreSet      = "Язык слова: Голландский (Нидерланды)"
            ParagraphPreSet = "Язык абзаца: Голландский (Нидерланды)"
        Case 5: WordPreSet      = "Язык слова: Французский (Франция)"
            ParagraphPreSet = "Язык абзаца: Французский (Франция)"
        Case 6: WordPreSet      = "Язык слова: Испанский (Испания)"
            ParagraphPreSet = "Язык абзаца: Испанский (Испания)"
        Case 7: WordPreSet      = "Язык слова: Венгерский"
            ParagraphPreSet = "Язык абзаца: Венгерский"
        Case 8: WordPreSet      = "Язык слова: Итальянский (Италия) "
            ParagraphPreSet = "Язык абзаца: Итальянский (Италия) "
        Case 9: WordPreSet      = "Язык слова: Шведский (Швеция)"
            ParagraphPreSet = "Язык абзаца: Шведский (Швеция)"
        Case 10:WordPreSet      = "Язык слова: Польский"
            ParagraphPreSet = "Язык абзаца: Польский"
        Case 11:WordPreSet      = "Язык слова: Немецкий (Германия)"
            ParagraphPreSet = "Язык абзаца: Немецкий (Германия)"
        end select
        '-----------------------------------------------------------------------
    Case 31        'Case 31: dutch
        Select Case iLanguageCount
        Case 1: WordPreSet      = "Woord is Engels (VS)"
            ParagraphPreSet = "Alinea is Engels (VS)"
        Case 2: WordPreSet      = "Woord is Portugees (Portugal)"
            ParagraphPreSet = "Alinea is Portugees (Portugal)"
        Case 3: WordPreSet      = "Woord is Russisch"
            ParagraphPreSet = "Alinea is Russisch"
        Case 4: WordPreSet      = "Woord is Nederlands (NL)"
            ParagraphPreSet = "Alinea is Nederlands (NL)"
        Case 5: WordPreSet      = "Woord is Frans (Frankrijk)"
            ParagraphPreSet = "Alinea is Frans (Frankrijk)"
        Case 6: WordPreSet      = "Woord is Spaans (Spanje)"
            ParagraphPreSet = "Alinea is Spaans (Spanje)"
        Case 7: WordPreSet      = "Woord is Hongaars"
            ParagraphPreSet = "Alinea is Hongaars"
        Case 8: WordPreSet      = "Woord is Italiaans (Italië)"
            ParagraphPreSet = "Alinea is Italiaans (Italië)"
        Case 9: WordPreSet      = "Woord is Zweeds (Zweden)"
            ParagraphPreSet = "Alinea is Zweeds (Zweden)"
        Case 10:WordPreSet      = "Woord is Pools"
            ParagraphPreSet = "Alinea is Pools"
        Case 11:WordPreSet      = "Woord is Duits (Duitsland)"
            ParagraphPreSet = "Alinea is Duits (Duitsland)"
        end select
        '-----------------------------------------------------------------------
    Case 33        'Case 33: french
        Select Case iLanguageCount
        Case 1: WordPreSet      = "Le mot est en Anglais (U.S.A.)"
            ParagraphPreSet = "Le paragraphe est en Anglais (U.S.A.)"
        Case 2: WordPreSet      = "Le mot est en Portugais (Portugal)"
            ParagraphPreSet = "Le paragraphe est en Portugais (Portugal)"
        Case 3: WordPreSet      = "Le mot est en Russe"
            ParagraphPreSet = "Le paragraphe est en Russe"
        Case 4: WordPreSet      = "Le mot est en Néerlandais (Pays-Bas)"
            ParagraphPreSet = "Le paragraphe est en Néerlandais (Pays-Bas)"
        Case 5: WordPreSet      = "Le mot est en Français (France)"
            ParagraphPreSet = "Le paragraphe est en Français (France)"
        Case 6: WordPreSet      = "Le mot est en Espagnol (Espagne)"
            ParagraphPreSet = "Le paragraphe est en Espagnol (Espagne)"
        Case 7: WordPreSet      = "Le mot est en Hongrois"
            ParagraphPreSet = "Le paragraphe est en Hongrois"
        Case 8: WordPreSet      = "Le mot est en Italien (Italie)"
            ParagraphPreSet = "Le paragraphe est en Italien (Italie)"
        Case 9: WordPreSet      = "Le mot est en Suédois (Suède)"
            ParagraphPreSet = "Le paragraphe est en Suédois (Suède)"
        Case 10: WordPreSet     = "Le mot est en Polonais"
            ParagraphPreSet = "Le paragraphe est en Polonais"
        Case 11: WordPreSet     = "Le mot est en Allemand (Allemagne)"
            ParagraphPreSet = "Le paragraphe est en Allemand (Allemagne)"
        end select
        '-----------------------------------------------------------------------
    Case 34        'Case 34: spanish
        Select Case iLanguageCount
        Case 1: WordPreSet      = "La palabra está Inglés (Estados Unidos)"
            ParagraphPreSet = "El párrafo es  Inglés (Estados Unidos)"
        Case 2: WordPreSet      = "La palabra está Español (España)"
            ParagraphPreSet = "El párrafo es  Portugués (Portugal)"
        Case 3: WordPreSet      = "La palabra está Ruso"
            ParagraphPreSet = "El párrafo es  Ruso"
        Case 4: WordPreSet      = "La palabra está Holandés (Países Bajos)"
            ParagraphPreSet = "El párrafo es  Holandés (Países Bajos)"
        Case 5: WordPreSet      = "La palabra está Francés (Francia)"
            ParagraphPreSet = "El párrafo es  Francés (Francia)"
        Case 6: WordPreSet      = "La palabra está Español (España)"
            ParagraphPreSet = "El párrafo es  Español (España)"
        Case 7: WordPreSet      = "La palabra está Húngaro"
            ParagraphPreSet = "El párrafo es  Húngaro"
        Case 8: WordPreSet      = "La palabra está Italiano (Italia)"
            ParagraphPreSet = "El párrafo es  Italiano (Italia)"
        Case 9: WordPreSet      = "La palabra está Sueco (Suecia)"
            ParagraphPreSet = "El párrafo es  Sueco (Suecia)"
        Case 10:WordPreSet     = "La palabra está Polaco"
            ParagraphPreSet = "El párrafo es  Polaco"
        Case 11:WordPreSet     = "La palabra está Alemán (Alemania)"
            ParagraphPreSet = "El párrafo es  Alemán (Alemania)"
        end select
        '-----------------------------------------------------------------------
    Case 36        'Case 36: hungarian
        Select Case iLanguageCount
        Case 1: WordPreSet      = "A szó: Angol (USA)"
            ParagraphPreSet = "Bekezdés: Angol (USA)"
        Case 2: WordPreSet      = "A szó: Portugál (Portugália)"
            ParagraphPreSet = "Bekezdés: Portugál (Portugália)"
        Case 3: WordPreSet      = "A szó: Orosz"
            ParagraphPreSet = "Bekezdés: Orosz"
        Case 4: WordPreSet      = "A szó: Holland (Hollandia)"
            ParagraphPreSet = "Bekezdés: Holland (Hollandia)"
        Case 5: WordPreSet      = "A szó: Francia (Franciaország)"
            ParagraphPreSet = "Bekezdés: Francia (Franciaország)"
        Case 6: WordPreSet      = "A szó: Spanyol (Spanyolország)"
            ParagraphPreSet = "Bekezdés: Spanyol (Spanyolország)"
        Case 7: WordPreSet      = "A szó: Magyar"
            ParagraphPreSet = "Bekezdés: Magyar"
        Case 8: WordPreSet      = "A szó: Olasz (Olaszország)"
            ParagraphPreSet = "Bekezdés: Olasz (Olaszország)"
        Case 9: WordPreSet      = "A szó: Svéd (Svédország)"
            ParagraphPreSet = "Bekezdés: Svéd (Svédország)"
        Case 10:WordPreSet      = "A szó: Lengyel"
            ParagraphPreSet = "Bekezdés: Lengyel"
        Case 11:WordPreSet      = "A szó: Német (Németország)"
            ParagraphPreSet = "Bekezdés: Német (Németország)"
        end select
        '-----------------------------------------------------------------------
    Case 39        'Case 39: italian
        Select Case iLanguageCount
        Case 1: WordPreSet      = "La parola è Inglese (USA)"
            ParagraphPreSet = "Il paragrafo è Inglese (USA)"
        Case 2: WordPreSet      = "La parola è Portoghese (Portogallo)"
            ParagraphPreSet = "Il paragrafo è Portoghese (Portogallo)"
        Case 3: WordPreSet      = "La parola è Russo"
            ParagraphPreSet = "Il paragrafo è Russo"
        Case 4: WordPreSet      = "La parola è Olandese (Paesi Bassi)"
            ParagraphPreSet = "Il paragrafo è Olandese (Paesi Bassi)"
        Case 5: WordPreSet      = "La parola è Francese (Francia)"
            ParagraphPreSet = "Il paragrafo è Francese (Francia)"
        Case 6: WordPreSet      = "La parola è Spagnolo (Spagna)"
            ParagraphPreSet = "Il paragrafo è Spagnolo (Spagna)"
        Case 7: WordPreSet      = "La parola è Ungherese"
            ParagraphPreSet = "Il paragrafo è Ungherese"
        Case 8: WordPreSet      = "La parola è Italiano (Italia)"
            ParagraphPreSet = "Il paragrafo è Italiano (Italia)"
        Case 9: WordPreSet      = "La parola è Svedese (Svezia)"
            ParagraphPreSet = "Il paragrafo è Svedese (Svezia)"
        Case 10: WordPreSet     = "La parola è Polacco"
            ParagraphPreSet = "Il paragrafo è Polacco"
        Case 11: WordPreSet     = "La parola è Tedesco (Germania)"
            ParagraphPreSet = "Il paragrafo è Tedesco (Germania)"
        end select
        '-----------------------------------------------------------------------
    Case 46        'Case 46: swedish
        Select Case iLanguageCount
        Case 1: WordPreSet      = "Ordet är  Engelska (USA)"
            ParagraphPreSet = "Stycket är  Engelska (USA)"
        Case 2: WordPreSet      = "Ordet är  Portugisiska (Portugal)"
            ParagraphPreSet = "Stycket är  Portugisiska (Portugal)"
        Case 3: WordPreSet      = "Ordet är  Ryska"
            ParagraphPreSet = "Stycket är  Ryska"
        Case 4: WordPreSet      = "Ordet är  Svenska (Sverige)"
            ParagraphPreSet = "Stycket är  Holländska (Nederländerna)"
        Case 5: WordPreSet      = "Ordet är  Franska (Frankrike)"
            ParagraphPreSet = "Stycket är  Franska (Frankrike)"
        Case 6: WordPreSet      = "Ordet är  Spanska (Spanien)"
            ParagraphPreSet = "Stycket är  Spanska (Spanien)"
        Case 7: WordPreSet      = "Ordet är  Ungerska"
            ParagraphPreSet = "Stycket är  Ungerska"
        Case 8: WordPreSet      = "Ordet är  Italienska (Italien)"
            ParagraphPreSet = "Stycket är  Italienska (Italien)"
        Case 9: WordPreSet      = "Ordet är  Svenska (Sverige)"
            ParagraphPreSet = "Stycket är  Svenska (Sverige)"
        Case 10:WordPreSet      = "Ordet är  Polska"
            ParagraphPreSet = "Stycket är  Polska"
        Case 11:WordPreSet      = "Ordet är  Tyska (Tyskland)"
            ParagraphPreSet = "Stycket är  Tyska (Tyskland)"
        end select
        '-----------------------------------------------------------------------
    Case 48        'Case 48: polish
        Select Case iLanguageCount
        Case 1: WordPreSet      = "Wyraz jest Angielski (USA)"
            ParagraphPreSet = "Akapit jest Angielski (USA)"
        Case 2: WordPreSet      = "Wyraz jest Portugalski (Portugalia)"
            ParagraphPreSet = "Akapit jest Portugalski (Portugalia)"
        Case 3: WordPreSet      = "Wyraz jest Rosyjski"
            ParagraphPreSet = "Akapit jest Rosyjski"
        Case 4: WordPreSet      = "Wyraz jest Holenderski (Holandia)"
            ParagraphPreSet = "Akapit jest Holenderski (Holandia)"
        Case 5: WordPreSet      = "Wyraz jest Francuski (Francja)"
            ParagraphPreSet = "Akapit jest Francuski (Francja)"
        Case 6: WordPreSet      = "Wyraz jest Hiszpański (Hiszpania)"
            ParagraphPreSet = "Akapit jest Hiszpański (Hiszpania)"
        Case 7: WordPreSet      = "Wyraz jest Węgierski"
            ParagraphPreSet = "Akapit jest Węgierski"
        Case 8: WordPreSet      = "Wyraz jest Włoski (Włochy)"
            ParagraphPreSet = "Akapit jest Włoski (Włochy)"
        Case 9: WordPreSet      = "Wyraz jest Szwedzki (Szwecja)"
            ParagraphPreSet = "Akapit jest Szwedzki (Szwecja)"
        Case 10:WordPreSet      = "Wyraz jest Polski"
            ParagraphPreSet = "Akapit jest Polski"
        Case 11:WordPreSet      = "Wyraz jest Niemiecki (Niemcy)"
            ParagraphPreSet = "Akapit jest Niemiecki (Niemcy)"
        end select
        '-----------------------------------------------------------------------
    Case 49        'Case 49: german
        Select Case iLanguageCount
        Case 1: WordPreSet      = "Wort ist Englisch (USA)"
            ParagraphPreSet = "Absatz ist Englisch (USA)"
        Case 2: WordPreSet      = "Wort ist Portugiesisch (Portugal)"
            ParagraphPreSet = "Absatz ist Portugiesisch (Portugal)"
        Case 3: WordPreSet      = "Wort ist Russisch"
            ParagraphPreSet = "Absatz ist Russisch"
        Case 4: WordPreSet      = "Wort ist Niederländisch (Niederlande)"
            ParagraphPreSet = "Absatz ist Niederländisch (Niederlande)"
        Case 5: WordPreSet      = "Wort ist Französisch (Frankreich)"
            ParagraphPreSet = "Absatz ist Französisch (Frankreich)"
        Case 6: WordPreSet      = "Wort ist Spanisch (Spanien)"
            ParagraphPreSet = "Absatz ist Spanisch (Spanien)"
        Case 7: WordPreSet      = "Wort ist Ungarisch"
            ParagraphPreSet = "Absatz ist Ungarisch"
        Case 8: WordPreSet      = "Wort ist Italienisch (Italien)"
            ParagraphPreSet = "Absatz ist Italienisch (Italien)"
        Case 9: WordPreSet      = "Wort ist Schwedisch (Schweden)"
            ParagraphPreSet = "Absatz ist Schwedisch (Schweden)"
        Case 10:WordPreSet      = "Wort ist Polnisch"
            ParagraphPreSet = "Absatz ist Polnisch"
        Case 11:WordPreSet      = "Wort ist Deutsch (Deutschland)"
            ParagraphPreSet = "Absatz ist Deutsch (Deutschland)"
        end select
        '-----------------------------------------------------------------------
    Case 50        'Case 50: slovenian
       Select Case iLanguageCount
       Case 1: WordPreSet      = "Jezik besede je angleški (ZDA)"
           ParagraphPreSet = "Jezik odstavka je angleški (ZDA)"
       Case 2: WordPreSet      = "Jezik besede je portugalski (Portugalska)"
           ParagraphPreSet = "Jezik odstavka je portugalski (Portugalska)"
       Case 3: WordPreSet      = "Jezik besede je ruski"
           ParagraphPreSet = "Jezik odstavka je ruski"
       Case 4: WordPreSet      = "Jezik besede je nizozemski (Nizozemska)"
           ParagraphPreSet = "Jezik odstavka je nizozemski (Nizozemska)"
       Case 5: WordPreSet      = "Jezik besede je francoski (Francija)"
           ParagraphPreSet = "Jezik odstavka je francoski (Francija)"
       Case 6: WordPreSet      = "Jezik besede je španski (Španija)"
           ParagraphPreSet = "Jezik odstavka je španski (Španija)"
       Case 7: WordPreSet      = "Jezik besede je madžarski"
           ParagraphPreSet = "Jezik odstavka je madžarski"
       Case 8: WordPreSet      = "Jezik besede je italijanski (Italija)"
           ParagraphPreSet = "Jezik odstavka je italijanski (Italija)"
       Case 9: WordPreSet      = "Jezik besede je švedski (Švedska)"
           ParagraphPreSet = "Jezik odstavka je švedski (Švedska)"
       Case 10:WordPreSet      = "Jezik besede je poljski"
           ParagraphPreSet = "Jezik odstavka je poljski"
       Case 11:WordPreSet      = "Jezik besede je nemški (Nemčija)"
           ParagraphPreSet = "Jezik odstavka je nemški (Nemčija)"
       end select
       '-----------------------------------------------------------------------
    Case 55        'Case 55: Portuguese (Brazil)
        Select Case iLanguageCount
        Case 1: WordPreSet      = "a palavra é Inglês (EUA)"
            ParagraphPreSet = "O parágrafo é Inglês (EUA)"
        Case 2: WordPreSet      = "a palavra é Português (Brasil)"
            ParagraphPreSet = "O parágrafo é Português (Portugal)"
        Case 3: WordPreSet  	= "a palavra é Russo"
            ParagraphPreSet = "O parágrafo é Russo"
        Case 4: WordPreSet      = "a palavra é Holandês (Holanda)"
            ParagraphPreSet = "O parágrafo é Holandês (Holanda)"
        Case 5: WordPreSet      = "a palavra é Francês (França)"
            ParagraphPreSet = "O parágrafo é Francês (França)"
        Case 6: WordPreSet      = "a palavra é Português (Brasil)"
            ParagraphPreSet = "O parágrafo é Espanhol (Espanha)"
        Case 7: WordPreSet      = "a palavra é Húngaro"
            ParagraphPreSet = "O parágrafo é Húngaro"
        Case 8: WordPreSet      = "a palavra é Italiano (Itália)"
            ParagraphPreSet = "O parágrafo é Italiano (Itália)"
        Case 9: WordPreSet      = "a palavra é Sueco (Suécia)"
            ParagraphPreSet = "O parágrafo é Sueco (Suécia)"
        Case 10:WordPreSet      = "a palavra é Polonês"
            ParagraphPreSet = "O parágrafo é Polonês"
        Case 11:WordPreSet      = "a palavra é Alemão (Alemanha)"
            ParagraphPreSet = "O parágrafo é Alemão (Alemanha)"
        end select
        '-----------------------------------------------------------------------
    case else
        'if Office language is not already covered: warning appears.
        Warnlog "Language has to be added to testcase. Please see w_locale_strings2.inc for further details"
        QAErrorlog "Please add " & iSprache & " to file!"
        wGetLanguageGuessString = ""
    end select
    '-----------------------------------------------------------------------
    'distinguishing if string for word or string for paragraph is handed over
    if sValue = "WordPreSet" then
        wGetLanguageGuessString = WordPreSet
    elseif sValue = "ParagraphPreSet" then
        wGetLanguageGuessString = ParagraphPreSet
    end if

end function
