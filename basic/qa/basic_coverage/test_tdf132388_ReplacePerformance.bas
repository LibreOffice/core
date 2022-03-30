'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest as String
    TestUtil.TestInit
    TestReplacePerformance()
    doUnitTest = TestUtil.GetResult()
End Function

Sub TestReplacePerformance()
    On Error GoTo errorHandler
    ' Assume the normal case to be much faster than 60 s even on slow boxes, and the test string
    ' is long enough to cause serious performance regressions make it perform much longer
    n = 10000000
    s = Space(n)
    t = Now
    s = Replace(s, " ", "*", 1, -1, 1)
    t = Now - t
    TestUtil.Assert(t <= TimeSerial(0, 2, 0), "TestReplacePerformance", Format(t, """t = ""[s]"" s"""))
    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("TestReplacePerformance", Err, Error$, Erl)
End Sub
