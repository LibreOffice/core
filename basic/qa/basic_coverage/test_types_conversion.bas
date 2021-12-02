'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
Option Explicit

Dim nTotalCount As Integer
Dim nPassCount As Integer
Dim nFailCount As Integer

' See LibreOffice6FloatingPointMode in basic/source/runtime/methods1.cxx
Function LibreOffice6FloatingPointMode() As Boolean
    Dim bMode As Boolean
    bMode = Environ("LIBREOFFICE6FLOATINGPOINTMODE") <> ""
    If (Not bMode) Then
	Dim oConfigProvider As Object, aNodePath(0) As New com.sun.star.beans.PropertyValue, oRegistryKey As Object
	oConfigProvider = createUnoService("com.sun.star.configuration.ConfigurationProvider")
	aNodePath(0).Name = "nodepath"
	aNodePath(0).Value = "org.openoffice.Office.Scripting/Basic/Compatibility"
	oRegistryKey = oConfigProvider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", aNodePath)
	bMode = oRegistryKey.getPropertyValue("UseLibreOffice6FloatingPointConversion")
    End If
    LibreOffice6FloatingPointMode = bMode
End Function

' For the following tests the en-US (English - United States) locale is required
Function doUnitTest() As String
    nTotalCount = 0
    nPassCount = 0
    nFailCount = 0

    ' Test implicit conversions from string to number
    Dim nVal As Double
    ' Simple integer
    StartTest()
    nVal = "123"
    AssertTest(nVal = 123)

    ' Negative integer
    StartTest()
    nVal = "-123"
    AssertTest(nVal = -123)

    ' Negative floating-point
    StartTest()
    nVal = "-123.45"
    AssertTest(nVal = -123.45)

    ' Negative floating-point with leading and trailing spaces
    StartTest()
    nVal = " -123.456 "
    AssertTest(nVal = -123.456)

    If LibreOffice6FloatingPointMode() Then
        ' Wrong decimal separator (and not even interpreted as group separator)
         StartTest()
        nVal = " -123,45 "
        AssertTest(nVal = -123)
    Else
        ' Wrong decimal separator (interpreted as group separator)
        StartTest()
        nVal = " -123,456 "
        AssertTest(nVal = -123456)
    End If

    If ((nFailCount > 0) Or (nPassCount <> nTotalCount)) Then
        doUnitTest = "FAIL"
    Else
        doUnitTest = "OK"
    End If
End Function

Sub StartTest()
    nTotalCount = nTotalCount + 1
End Sub

Sub AssertTest(testResult As Boolean)
    If (testResult) Then
        nPassCount = nPassCount + 1
    Else
        nFailCount = nFailCount + 1
    End If
End Sub
