Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_testTypeName()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testTypeName() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test TypeName function"
    On Error GoTo errorHandler
    Dim b1 As Boolean
    Dim c1 As Byte
    Dim d1 As Date
    Dim d2 As Double
    Dim i1 As Integer
    Dim l1 As Long

    date2 = "String"
    date1 = TypeName(testName)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Boolean"
    date1 = TypeName(b1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Byte"
    date1 = TypeName(c1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Date"
    date1 = TypeName(d1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Double"
    date1 = TypeName(d2)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Integer"
    date1 = TypeName(i1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    date2 = "Long"
    date1 = TypeName(l1)
    TestLog_ASSERT date1 = date2, "the return TypeName is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testTypeName = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

Sub TestLog_ASSERT(assertion As Boolean, Optional testId As String, Optional testComment As String)

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

