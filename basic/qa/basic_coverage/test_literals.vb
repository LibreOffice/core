' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
    result = verify_testOptionalsBasic()
    If failCount <> 0 Or passCount = 0 Then
        doUnitTest = 0
    Else
        doUnitTest = 1
    End If
End Function

Function verify_testLiterals() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    testName = "Test Literals (Basic)"
    On Error GoTo errorHandler

    Dim Variable1&
    Dim Variable2%
    Dim Variable3!
    Dim Variable4#
    Dim Variable5@
    Dim Variable6$
    Dim Variable7 As Boolean
    Dim Variable8 As Date

    TestLog_ASSERT TypeName(Variable1), "Long", "TypeName(""Long"")"
    TestLog_ASSERT TypeName(Variable2), "Integer", "TypeName(""Integer"")"
    TestLog_ASSERT TypeName(Variable3), "Single", "TypeName(""Single"")"
    TestLog_ASSERT TypeName(Variable4), "Double", "TypeName(""Double"")"
    TestLog_ASSERT TypeName(Variable5), "Currency", "TypeName(""Currency"")"
    TestLog_ASSERT TypeName(Variable6), "String", "TypeName(""String"")"
    TestLog_ASSERT TypeName(Variable7), "Boolean", "TypeName(""Boolean"")"
    TestLog_ASSERT TypeName(Variable8), "Date", "TypeName(""Date"")"

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testOptionalsBasic = result

    Exit Function
errorHandler:
    TestLog_ASSERT False, True, Err.Description
End Function
