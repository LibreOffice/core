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

sub w_spellcheck1
    Call tLanguage_Guess
end sub

'-------------------------------------------------------------------------

testcase tLanguage_Guess

'Variables:
DIM iStringPosition     AS integer 'Position of string in context menu for word, paragraph
DIM iLanguageCount      AS integer 'variable for counting through languages in test doc
DIM WordPreSet          AS string 'given string for comparison from w_locale_strings2.inc
DIM WordLanguage        AS string '"word is..."entry in context menu
DIM ParagraphLanguage   AS string '"paragraph is..."entry in context menu
DIM ParagraphPreSet     AS string 'given string for comparison of paragraphs from w_locale_strings2.inc
DIM WordLanguageTemp    AS string 'variable for checking if Office language is already covered in w_locale_strings2.inc

    '///Load test document///
    call hFileOpen (ConvertPath ( gTesttoolPath + "writer\optional\input\spellcheck\Language_Guessing_Sample_Texts.odt"))      'loading Test-Doc
    
    sleep(5)
    '----------------------------------------------------------------
    '///Enable auto spellcheck///
    Kontext "Standardbar"
    if AutoSpellcheck.GetState(2) <> 1 then     'cheking if auto-spellcheck is checked
        printlog "Autospellcheck not checked, checking Autospellcheck now"
        AutoSpellcheck.Click
        printlog "Autospellcheck checked"
    else
        printlog "Autospellcheck is already checked"
    end if
    '----------------------------------------------------------------
    ViewNavigator       'opening navigator and selecting table
    sleep(1)
    Call wNavigatorAuswahl(2,1)
    DocumentWriter.TypeKeys "<MOD1 UP>"
    ViewNavigator       'closing navigator
    printlog "-----------------"
    '----------------------------------------------------------------
    '///Put the cursor in the first wrong word///
        
    for iLanguageCount = 1 to 11         'running through the languages
    'checking the word proposal
        printlog "CHECKING WORD:"
        DocumentWriter.TypeKeys "<MOD1 DOWN>"
        DocumentWriter.TypeKeys "<MOD1 DOWN>"
        '///Call context menu///
        DocumentWriter.TypeKeys "<SHIFT F10>"       'calling context menu
        'selecting 'word is... position
        iStringPosition = hMenuItemGetCount - 1     'calculating last but one entry
        
        'getting string from position
        WordLanguage = hMenuItemGetText(iStringPosition)        'gets text from entry
        'printing out string for having a look at the values.
        printlog "WordLanguage is " + WordLanguage
        '///Verify that the language proposal "word is..." is correct///
        WordLanguageTemp = wGetLanguageGuessString("WordPreSet", iLanguageCount) 'comparing reality against the string in w_locale_strings2.inc
        if WordLanguageTemp > "" then 'checking if the language has really an entry in w_locale_strings2.inc
            if WordLanguage = WordLanguageTemp then
                printlog "Word language proposal is correct" 'all ok
            else
                warnlog "Word language proposal is not correct!" 'problem!
            end if
        else
            Call MenuSelect(0) 'closing menu when language is not yet covered
            Call hCloseDocument ''closing doc when language is not yet covered
            exit sub
        end if
        
        '///Apply the language for this word ///
        call hMenuItemCheck (iStringPosition)        'clicking on menu entry, closing context. Word looses redline.
        sleep (1)
        '///Verifiy that the language is set correctly for this word ///
        DocumentWriter.TypeKeys "<SHIFT F10>"       'calling context menu
        iStringPosition = hMenuItemGetCount - 1     'calculating last but one entry
        WordLanguage = hMenuItemGetText(iStringPosition)        'gets text from entry
        if WordLanguage = WordPreSet then ' checking if context menu ahs changed
            warnlog "Word language not applied correctly!" 'Problem!
        else
            printlog "Word language applied correctly" ''all ok
        end if
        Call MenuSelect(0) 'closing menu
        
        '----------------------------------------------------------------
        printlog "CHECKING PARAGRAPH:"
        EditUndo        'undoing hMenuItemCheck from Check-word-part
        DocumentWriter.TypeKeys "<MOD1 LEFT>"
        sleep (1)
        '///Call context menu///
        DocumentWriter.TypeKeys "<SHIFT F10>"       'calling context menu
        'selecting 'paragraph is... position
        iStringPosition = hMenuItemGetCount     'calculating last entry
        'getting string from position
        ParagraphLanguage = hMenuItemGetText(iStringPosition)        'gets text from entry
        'printing out string
        printlog ParagraphLanguage
        '///Verify that the language proposal "paragraph is..." is correct///
        if ParagraphLanguage = wGetLanguageGuessString("ParagraphPreSet", iLanguageCount) then 'comparing reality against the string in w_locale_strings2.inc
            printlog "Paragraph language proposal is correct" 'all ok
        else
            warnlog "Paragraph language proposal is not correct!" 'Problem!
        end if 
        
        
        '///Apply the language for this paragraph ///
        call hMenuItemCheck (iStringPosition)        'clicking on last menu entry, closing context. Paragraph looses redline.
        sleep (1)
        '///Verify that the language is set correctly for this paragraph ///
        DocumentWriter.TypeKeys "<SHIFT F10>"       'calling context menu
        iStringPosition = hMenuItemGetCount     'calculating last entry
        ParagraphLanguage = hMenuItemGetText(iStringPosition)        'gets text from entry
        '///Verify that the language is set correctly for this paragraph ///
        if ParagraphLanguage = ParagraphPreSet then
           warnlog "Paragraph language not applied correctly!" 'Problem
        else
            printlog "Paragraph language applied correctly" 'all ok
        end if
        Call MenuSelect(0) 'closing menu
        printlog "-----------------"
        '///Go to the next text sample ///
        '///Repeat for all western languages ///
    next iLanguageCount
    
    call hCloseDocument      'engl. Test-Doc closed
    '----------------------------------------------------------------
endcase

