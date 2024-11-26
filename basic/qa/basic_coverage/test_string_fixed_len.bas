' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option VBASupport 0
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_stringFixedLen
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_stringFixedLen()
    On Error GoTo errorHandler
    ' tdf#163680 - fixed-length strings support

    Dim s As String * 10
    TestUtil.AssertEqual(Len(s), 10, "Len(s) - default")
    ' The default value is 10 null characters
    TestUtil.AssertEqual(s, String(10, 0), "s - default")
    s = "abc"
    TestUtil.AssertEqual(Len(s), 10, "Len(s) - abc")
    TestUtil.AssertEqual("""" & s & """", """abc       """, """s"" - abc")
    s = "defghijklmno"
    TestUtil.AssertEqual(Len(s), 10, "Len(s) - defghijklmno")
    TestUtil.AssertEqual("""" & s & """", """defghijklm""", """s"" - defghijklmno")
    Let s = "xyz" ' Test also Let keyword - uses a different code path
    TestUtil.AssertEqual(Len(s), 10, "Len(s) - xyz")
    TestUtil.AssertEqual("""" & s & """", """xyz       """, """s"" - xyz")
    Let s = "opqrstuvwxyz"
    TestUtil.AssertEqual(Len(s), 10, "Len(s) - opqrstuvwxyz")
    TestUtil.AssertEqual("""" & s & """", """opqrstuvwx""", """s"" - opqrstuvwxyz")

    Dim s1 As String * 0 ' Unlike VBA, LibreOffice Basic allows this for unrestricted strings
    TestUtil.AssertEqual(Len(s1), 0, "Len(s1) - default")
    TestUtil.AssertEqual("""" & s1 & """", """""", """s1"" - default")
    s1 = "klm"
    TestUtil.AssertEqual(Len(s1), 3, "Len(s1) - klm")
    TestUtil.AssertEqual("""" & s1 & """", """klm""", """s1"" - klm")
    s = s1
    TestUtil.AssertEqual(Len(s), 10, "Len(s) - klm")
    TestUtil.AssertEqual("""" & s & """", """klm       """, """s"" - klm")
    ' Also test s1 - it must not be affected
    TestUtil.AssertEqual(Len(s1), 3, "Len(s1) - klm")
    TestUtil.AssertEqual("""" & s1 & """", """klm""", """s1"" - klm")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_stringFixedLen", Err, Error$, Erl)
End Sub
