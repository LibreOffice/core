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
'* short description : This testcase is for thai issues only
'*
'\***********************************************************************

sub wr_o_5

    Call tLanguageSettingsComplexTextLayout

end sub

testcase tLanguageSettingsComplexTextLayout

    Dim i as integer
    Dim j as integer
    Dim iNumeralsCounter as integer
    Dim iNumeralsCount as integer
    Dim bAsianLanguage as boolean
    Dim bCTLLanguage as boolean
    Dim sCTLLan() as string
    Dim sCurrentDocumentLanguage as string
    Dim sCurrentCTLLanguage as string
    Dim bSequenceChecking as boolean

    '/// Open new writerdocument
    Call hNewDocument
    '/// Activate Asian support
    bAsianLanguage = ActiveDeactivateAsianSupport(True)
    '/// Activate CTL support 
    bCTLLanguage = ActiveDeactivateCTLSupport(True)

	ToolsOptions
        '/// Goto 'Tools->Options->Language settings->Languages
	Call hToolsOptions ("LanguageSettings","Languages")
        sCurrentCTLLanguage = LanguageComplexScript.GetSelText
	Kontext "TabSprachen"
    For i = 1 to LanguageComplexScript.GetItemCount
        '/// Select CTL-Language
        LanguageComplexScript.Select i
        sCurrentDocumentLanguage = LanguageComplexScript.GetSelText
        printlog "- Current selected CTL Language: " & sCurrentDocumentLanguage
        '/// close options dialog with 'ok'
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.Ok
        '/// Reopen options and go to Tabpage 'Complex Text Layout'
        ToolsOptions
        Call hToolsOptions ("LanguageSettings","ComplexTextLayout")

        Kontext "TabComplexTextLayout"
            bSequenceChecking = FALSE
            Select Case iSprache
                '/// Checkboxes under 'Sequence checking' should all be checked for CTL languages
                '/// + like "Thai", "Lao", "Khmer", "Vietnamese"
                Case 01
                    sCTLLan = Array("Thai", "Lao", "Khmer", "Burmese", "Vietnamese")
                Case 49
                    sCTLLan = Array("Thai", "Lao", "Khmer", "Birmanisch", "Vietnamesisch")
                Case else
                    QAErrorlog "Please adapt testcase for this language"
                    Kontext "ExtrasOptionenDlg"
                        ExtrasOptionenDlg.Ok
                    goto IfNoSupport
		end select

        for j = 0 to 4
            if sCTLLan(j) = sCurrentDocumentLanguage then
                bSequenceChecking = TRUE
                exit for
            end if
        next j

        Select Case bSequenceChecking
            Case TRUE
                if UseSequenceChecking.IsChecked = False then
                    Warnlog "'Sequence Checking' should be checked for language: " & sCurrentDocumentLanguage
                end if
                if Restricted.IsEnabled = False or Restricted.IsChecked = False then
                    Warnlog "'Restricted' should be enabled and checked for language: " & sCurrentDocumentLanguage
                end if
                if TypeAndReplace.IsEnabled = False or TypeAndReplace.IsChecked = False then
                    Warnlog "'Type And Replace' should be enabled and checked for language: " & sCurrentDocumentLanguage
                end if

            Case FALSE
                if UseSequenceChecking.IsChecked = True then
                    QAErrorlog "'Sequence Checking' should not be checked for language: " & sCurrentDocumentLanguage
                end if
                if Restricted.IsEnabled = True AND Restricted.IsChecked = True then
                    QAErrorlog "'Restricted' should not be checked for language: " & sCurrentDocumentLanguage
                end if
                if TypeAndReplace.IsEnabled = True AND TypeAndReplace.IsChecked = True then
                    QAErrorlog "'Type And Replace' should not checked for language: " & sCurrentDocumentLanguage
                end if
        end select

        'Get the number of Numerals-entries.
        iNumeralsCount = Numerals.GetItemCount

        Kontext "ExtrasOptionenDlg"
            ExtrasOptionenDlg.Ok
        ToolsOptions
        Call hToolsOptions ("LanguageSettings","Languages")
        Kontext "TabSprachen"
    next i

    Kontext "ExtrasOptionenDlg"
        if ExtrasOptionenDlg.Exists then ExtrasOptionenDlg.Ok

    printlog "Check the Numerals-menu."
    ToolsOptions
    Call hToolsOptions ("LanguageSettings","ComplexTextLayout")

    Printlog " Select every entry in the list, then press OK to close the dialogue, then open it again and check if the value got saved."
    for iNumeralsCounter = 1 to iNumeralsCount
        Numerals.Select iNumeralsCounter
        Printlog " Numerals nr: " + iNumeralsCounter + " = " + Numerals.GetSelText

        'Press ok, and open the dialogue again.
        Kontext "ExtrasOptionenDlg"
	    if ExtrasOptionenDlg.Exists then ExtrasOptionenDlg.Ok
        ToolsOptions
        Call hToolsOptions ("LanguageSettings","ComplexTextLayout")

        if Numerals.GetSelIndex <> iNumeralsCounter then Warnlog "The Numeral-Entry number: " + iNumeralsCounter + " didn't get saved when pressing OK in the FormatCharacter-dialogue."
    next iNumeralsCounter

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    '/// Choose Thai as CTL language
    ToolsOptions
    Call hToolsOptions ("LanguageSettings","Languages")
    Kontext "TabSprachen"
    LanguageComplexScript.Select sCTLLan(0)
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    '/// Restart office
    Call wOfficeRestart

    '/// Reopen options and go to Tabpage 'Complex Text Layout'
    ToolsOptions
    Call hToolsOptions ("LanguageSettings","ComplexTextLayout")
    '/// Check if all checkboxes under 'Sequence checking' are checked 
    if UseSequenceChecking.IsChecked = False then
        Warnlog "'Sequence Checking' should be checked for language: " & sCTLLan(0)
    end if
    if Restricted.IsEnabled = False or Restricted.IsChecked = False then
        Warnlog "'Restricted' should be enabled and checked for language: " & sCTLLan(0)
    end if
    if TypeAndReplace.IsEnabled = False or TypeAndReplace.IsChecked = False then
        Warnlog "'Type And Replace' should be enabled and checked for language: " & sCTLLan(0)
    end if

    '/// Reset CTL Language
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok
    ToolsOptions
    Call hToolsOptions ("LanguageSettings","Languages")
    Kontext "TabSprachen"
    LanguageComplexScript.Select sCurrentCTLLanguage
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

IfNoSupport:
    Call ActiveDeactivateAsianSupport(bAsianLanguage)
    Call ActiveDeactivateCTLSupport(bCTLLanguage)

    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
