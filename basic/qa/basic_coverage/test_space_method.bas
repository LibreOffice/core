' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest As String
    TestUtil.Testinit
    verify_testSpace
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testSpace()
    On Error Goto errorHandler

    TestUtil.AssertEqual(Space(3), "   ", "Space(3)")
    TestUtil.AssertEqual(Space(2), "  ",  "Space(2)")
    TestUtil.AssertEqual(Space(0), "",    "Space(0)")

    ' Negative values generate an empty string
    TestUtil.AssertEqual(Space(-1), "", "Space(-1)")
    TestUtil.AssertEqual(Space(-4), "", "Space(-4)")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testSpace", Err, Error$, Erl)
End Sub
