Option VBASupport 1

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function GetResult()
    If passCount <> 0 and failCount = 0 Then
        GetResult = "OK"
    Else
        GetResult = result
    End If
End Function

Sub TestInit()
    passCount = 0
    failCount = 0
    result = result & "Test Results" & Chr$(10) & "============" & Chr$(10)
End Sub

Sub TestEnd()
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
End Sub

Sub AssertTrue(assertion As Boolean, Optional testId As String, Optional testComment As String)

    If assertion = True Then
        passCount = passCount + 1
    Else
        Dim testMsg As String
        If Not IsMissing(testId) Then
            testMsg = testMsg + " : " + testId
        End If
        If Not IsMissing(testComment) And Not (testComment = "") Then
            testMsg = testMsg + " (" + testComment + ")"
        End If

        result = result & Chr$(10) & " Failed: " & testMsg
        failCount = failCount + 1
    End If

End Sub

Sub AssertEqual(actual As Variant, expected As Variant, testName As String)
    If expected = actual Then
        passCount = passCount + 1
    Else
        result = result & Chr$(10) & " Failed: " & testName & " returned " & actual & ", expected " & expected
        failCount = failCount + 1
    End If
End Sub
