Rem Attribute VBA_ModuleType=VBAModule
Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String
Type MyType
    ax(3) As Integer
    bx As Double
End Type

Function doUnitTest() As String
result = verify_testARRAY()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testARRAY() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    Dim date1, date2 As Date   'variables for test
    Dim a, b, C As Variant
    a = Array(10, 20, 30)
    testName = "Test ARRAY function"
    On Error GoTo errorHandler

    b = 10
    C = a(0)
    TestLog_ASSERT b = C, "the return ARRAY is: " & C

    b = 20
    C = a(1)
    TestLog_ASSERT b = C, "the return ARRAY is: " & C

    b = 30
    C = a(2)
    TestLog_ASSERT b = C, "the return ARRAY is: " & C

    Dim MyWeek, MyDay
    MyWeek = Array("Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun")

    b = "Tue"
    MyDay = MyWeek(1)   ' MyDay contains "Tue".
    TestLog_ASSERT b = MyDay, "the return ARRAY is: " & MyDay

    b = "Thu"
        MyDay = MyWeek(3)   ' MyDay contains "Thu".
    TestLog_ASSERT b = MyDay, "the return ARRAY is: " & MyDay

Dim mt As MyType
    mt.ax(0) = 42
    mt.ax(1) = 43
    mt.bx = 3.14
     b = 43
        C = mt.ax(1)
    TestLog_ASSERT b = C, "the return ARRAY is: " & C

    b = 3.14
    C = mt.bx
    TestLog_ASSERT b = C, "the return ARRAY is: " & C

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testARRAY = result

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

