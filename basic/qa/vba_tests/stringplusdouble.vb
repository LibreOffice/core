'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 1
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    DSD ' double = string + double
    SSD ' string = string + double
    DSS ' double = string + string
    doUnitTest = TestUtil.GetResult()
End Function

Sub DSD()
    Dim s As String
    Dim d As Double
    Dim r As Double

    On Error GoTo ErrorHandler

    r = s + d
    TestUtil.AssertEqual(r, -1, "s = null, d = null, r = s + d")

    r = s & d
    TestUtil.AssertEqual(r, 0, "s = null, d = null, r = s & d")

    d = 20
    r = s + d
    TestUtil.AssertEqual(r, -1, "s = null, d = 20, r = s + d")

    d = 20
    r = s & d
    TestUtil.AssertEqual(r, 20, "s = null, d = 20, r = s & d")

    ''''''''''''''
    s = "10"
    Dim d2 As Double
    r = s + d2
    TestUtil.AssertEqual(r, 10, "s = '10', d = null, r = s + d")

    r = s & d2
    TestUtil.AssertEqual(r, 100, "s = '10', d = null, r = s & d")

    d2 = 20
    r = s + d2
    TestUtil.AssertEqual(r, 30, "s = '10', d = 20, r = s + d")

    d2 = 20
    r = s & d2
    TestUtil.AssertEqual(r, 1020, "s = '10', d = 20, r = s & d")

     ''''''''''''''
    s = "abc"
    Dim d3 As Double
    r = s + d3
    TestUtil.AssertEqual(r, -1, "s = 'abc', d = null, r = s + d")

    r = s & d3
    TestUtil.AssertEqual(r, -1, "s = 'abc', d = null, r = s & d")

    d3 = 20
    r = s + d3
    TestUtil.AssertEqual(r, -1, "s = 'abc', d = 20, r = s + d")

    d3 = 20
    r = s & d3
    TestUtil.AssertEqual(r, -1, "s = 'abc', d = 20, r = s & d")

    Exit Sub

ErrorHandler:
    r = -1
    Resume Next
End Sub

Sub SSD()
    Dim s As String
    Dim d As Double
    Dim r As String

    On Error GoTo ErrorHandler

    r = s + d
    TestUtil.AssertEqual(r, "-1", "s = null, d = null, r = s + d")

    r = s & d
    TestUtil.AssertEqual(r, "0", "s = null, d = null, r = s & d")

    d = 20
    r = s + d
    TestUtil.AssertEqual(r, "-1", "s = null, d = 20, r = s + d")

    d = 20
    r = s & d
    TestUtil.AssertEqual(r, "20", "s = null, d = 20, r = s & d")

    ''''''''''''''
    s = "10"
    Dim d2 As Double
    r = s + d2
    TestUtil.AssertEqual(r, "10", "s = '10', d = null, r = s + d")

    r = s & d2
    TestUtil.AssertEqual(r, "100", "s = '10', d = null, r = s & d")

    d2 = 20
    r = s + d2
    TestUtil.AssertEqual(r, "30", "s = '10', d = 20, r = s + d")

    d2 = 20
    r = s & d2
    TestUtil.AssertEqual(r, "1020", "s = '10', d = 20, r = s & d")

     ''''''''''''''
    s = "abc"
    Dim d3 As Double
    r = s + d3
    TestUtil.AssertEqual(r, "-1", "s = 'abc', d = null, r = s + d")

    r = s & d3
    TestUtil.AssertEqual(r, "abc0", "s = 'abc', d = null, r = s & d")

    d3 = 20
    r = s + d3
    TestUtil.AssertEqual(r, "-1", "s = 'abc', d = 20, r = s + d")

    d3 = 20
    r = s & d3
    TestUtil.AssertEqual(r, "abc20", "s = 'abc', d = 20, r = s & d")
    Exit Sub

ErrorHandler:
    r = "-1"
    Resume Next
End Sub

Sub DSS()
    Dim s As String
    Dim d As String
    Dim r As Double

    On Error GoTo ErrorHandler

    r = s + d
    TestUtil.AssertEqual(r, -1, "s = null, d = null, r = s + d")

    r = s & d
    TestUtil.AssertEqual(r, -1, "s = null, d = null, r = s & d")

    d = "20"
    r = s + d
    TestUtil.AssertEqual(r, 20, "s = null, d = 20, r = s + d")

    d = "20"
    r = s & d
    TestUtil.AssertEqual(r, 20, "s = null, d = 20, r = s & d")

    ''''''''''''''
    s = "10"
    Dim d2 As String
    r = s + d2
    TestUtil.AssertEqual(r, 10, "s = '10', d = null, r = s + d")

    r = s & d2
    TestUtil.AssertEqual(r, 10, "s = '10', d = null, r = s & d")

    d2 = "20"
    r = s + d2
    TestUtil.AssertEqual(r, 1020, "s = '10', d = 20, r = s + d")

    d2 = "20"
    r = s & d2
    TestUtil.AssertEqual(r, 1020, "s = '10', d = 20, r = s & d")

     ''''''''''''''
    s = "abc"
    Dim d3 As String
    r = s + d3
    TestUtil.AssertEqual(r, -1, "s = 'abc', d = null, r = s + d")

    r = s & d3
    TestUtil.AssertEqual(r, -1, "s = 'abc', d = null, r = s & d")

    d3 = "20"
    r = s + d3
    TestUtil.AssertEqual(r, -1, "s = 'abc', d = 20, r = s + d")

    d3 = "20"
    r = s & d3
    TestUtil.AssertEqual(r, -1, "s = 'abc', d = 20, r = s & d")
    Exit Sub

ErrorHandler:
    r = -1
    Resume Next
End Sub

Sub test2()
    Dim s As String
    Dim d As Double
    s = ""
    d = s ' fail in MSO
    MsgBox d
End Sub

Sub testBoolean()
    Dim a As String
    Dim b As Boolean
    Dim c As Boolean
    Dim d As String

    b = True

    a = "1"
    c = a + b ' c = false
    MsgBox c

    d = a + b 'd = 0
    MsgBox d
End Sub

Sub testCurrency()
    Dim a As String
    Dim b As Currency
    Dim c As Currency
    Dim d As String

    a = "10"
    b = 30.3

    c = a + b ' c = 40.3
    MsgBox c

    d = a + b ' c =40.3
    MsgBox d

End Sub
