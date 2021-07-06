'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest as String
    ' CDateFromIso CDateToIso
    If ( CDateToIso( CDateFromIso("20161016") ) <> "20161016" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("2016-10-16") ) <> "20161016" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("-2016-10-16") ) <> "-20161016" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("-20161016") ) <> "-20161016" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("12016-10-16") ) <> "120161016" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("120161016") ) <> "120161016" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("-12016-10-16") ) <> "-120161016" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("-120161016") ) <> "-120161016" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("0001-01-01") ) <> "00010101" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("00010101") ) <> "00010101" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("-0001-12-31") ) <> "-00011231" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("-00011231") ) <> "-00011231" ) Then
        doUnitTest = "FAIL"
    ElseIf ( CDateToIso( CDateFromIso("991231") ) <> "19991231" ) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
    ' TODO: add some failure tests for misformed input, On Error whatever?
End Function
