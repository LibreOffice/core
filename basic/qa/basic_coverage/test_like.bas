'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testLike
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testLike()
    On Error GoTo errorHandler
    ' Negation test
    TestUtil.AssertEqual("!" Like "[.!?]",  True, "Negation1")
    TestUtil.AssertEqual("a" Like "[!abc]",  False, "Negation2")
    TestUtil.AssertEqual("!" Like "[!!?]", False, "Negation3")
    TestUtil.AssertEqual("^" Like "[.!?]", False, "Negation4")
    TestUtil.AssertEqual("^" Like "[.^?]", True, "Negation5")
    ' Like test from microsoft vba
    TestUtil.AssertEqual("aBBBa" Like "a*a", True, "Like1")
    TestUtil.AssertEqual("F" Like "[A-Z]", True, "Like2")
    TestUtil.AssertEqual("F" Like "[!A-Z]", False, "Like3")
    TestUtil.AssertEqual("a2a" Like "a#a", True, "Like4")
    TestUtil.AssertEqual("aM5b" Like "a[L-P]#[!c-e]", True, "Like5")
    TestUtil.AssertEqual("BAT123khg" Like "B?T*", True, "Like6")
    TestUtil.AssertEqual("CAT123khg" Like "B?T*", False, "Like7")
    TestUtil.AssertEqual("ab" Like "a*b", True, "Like8")
    TestUtil.AssertEqual("a*b" Like "a [*]b", False, "Like9")
    TestUtil.AssertEqual("axxxxxb" Like "a [*]b", False, "Like10")
    TestUtil.AssertEqual("a [xyz" Like "a [[]*", True, "Like11")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testLike", Err, Error$, Erl)
End Sub
