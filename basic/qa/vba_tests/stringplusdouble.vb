Option VBASupport 1
Option Explicit
Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_stringplusdouble()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_stringplusdouble() as String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    DSD
    SSD
    DSS
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_stringplusdouble = result
End Function

Sub DSD()
    Dim testName As String
    testName = "double = string + double"
    Dim testCompute As String

    Dim s As String
    Dim d As Double
    Dim r As Double

    On Error GoTo ErrorHandler

    testCompute = "s = null, d = null, r = s + d"
    r = s + d
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = null, d = null, r = s & d"
    r = s & d
    TestLog_ASSERT r = 0, testCompute & " .The result is: " & r

    testCompute = "s = null, d = 20, r = s + d"
    d = 20
    r = s + d
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = null, d = 20, r = s & d"
    d = 20
    r = s & d
    TestLog_ASSERT r = 20, testCompute & " .The result is: " & r


    ''''''''''''''
    s = "10"
    Dim d2 As Double
    testCompute = "s = '10', d = null, r = s + d"
    r = s + d2
    TestLog_ASSERT r = 10, testCompute & " .The result is: " & r

    testCompute = "s = '10', d = null, r = s & d"
    r = s & d2
    TestLog_ASSERT r = 100, testCompute & " .The result is: " & r

    testCompute = "s = '10', d = 20, r = s + d"
    d2 = 20
    r = s + d2
    TestLog_ASSERT r = 30, testCompute & " .The result is: " & r

    testCompute = "s = '10', d = 20, r = s & d"
    d2 = 20
    r = s & d2
    TestLog_ASSERT r = 1020, testCompute & " .The result is: " & r

     ''''''''''''''
    s = "abc"
    Dim d3 As Double
    testCompute = "s = 'abc', d = null, r = s + d"
    r = s + d3
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = null, r = s & d"
    r = s & d3
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = 20, r = s + d"
    d3 = 20
    r = s + d3
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = 20, r = s & d"
    d3 = 20
    r = s & d3
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    Exit Sub

ErrorHandler:
    r = -1
'    TestLog_Comment "The next compute raises error: " & testCompute
    Resume Next
End Sub


Sub SSD()
    Dim testName As String
    testName = "string = string + double"
    Dim testCompute As String

    Dim s As String
    Dim d As Double
    Dim r As String

    On Error GoTo ErrorHandler

    testCompute = "s = null, d = null, r = s + d"
    r = s + d
    TestLog_ASSERT r = "-1", testCompute & " .The result is: " & r

    testCompute = "s = null, d = null, r = s & d"
    r = s & d
    TestLog_ASSERT r = "0", testCompute & " .The result is: " & r

    testCompute = "s = null, d = 20, r = s + d"
    d = 20
    r = s + d
    TestLog_ASSERT r = "-1", testCompute & " .The result is: " & r

    testCompute = "s = null, d = 20, r = s & d"
    d = 20
    r = s & d
    TestLog_ASSERT r = "20", testCompute & " .The result is: " & r


    ''''''''''''''
    s = "10"
    Dim d2 As Double
    testCompute = "s = '10', d = null, r = s + d"
    r = s + d2
    TestLog_ASSERT r = "10", testCompute & " .The result is: " & r

    testCompute = "s = '10', d = null, r = s & d"
    r = s & d2
    TestLog_ASSERT r = "100", testCompute & " .The result is: " & r

    testCompute = "s = '10', d = 20, r = s + d"
    d2 = 20
    r = s + d2
    TestLog_ASSERT r = "30", testCompute & " .The result is: " & r

    testCompute = "s = '10', d = 20, r = s & d"
    d2 = 20
    r = s & d2
    TestLog_ASSERT r = "1020", testCompute & " .The result is: " & r

     ''''''''''''''
    s = "abc"
    Dim d3 As Double
    testCompute = "s = 'abc', d = null, r = s + d"
    r = s + d3
    TestLog_ASSERT r = "-1", testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = null, r = s & d"
    r = s & d3
    TestLog_ASSERT r = "abc0", testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = 20, r = s + d"
    d3 = 20
    r = s + d3
    TestLog_ASSERT r = "-1", testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = 20, r = s & d"
    d3 = 20
    r = s & d3
    TestLog_ASSERT r = "abc20", testCompute & " .The result is: " & r
    Exit Sub

ErrorHandler:
    r = "-1"
'    TestLog_Comment "The next compute raises error: " & testCompute
    Resume Next
End Sub

Sub DSS()
    Dim testName As String
    testName = "double = string + string"
    Dim testCompute As String

    Dim s As String
    Dim d As String
    Dim r As Double

    On Error GoTo ErrorHandler

    testCompute = "s = null, d = null, r = s + d"
    r = s + d
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = null, d = null, r = s & d"
    r = s & d
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = null, d = 20, r = s + d"
    d = "20"
    r = s + d
    TestLog_ASSERT r = 20, testCompute & " .The result is: " & r

    testCompute = "s = null, d = 20, r = s & d"
    d = "20"
    r = s & d
    TestLog_ASSERT r = 20, testCompute & " .The result is: " & r


    ''''''''''''''
    s = "10"
    Dim d2 As String
    testCompute = "s = '10', d = null, r = s + d"
    r = s + d2
    TestLog_ASSERT r = 10, testCompute & " .The result is: " & r

    testCompute = "s = '10', d = null, r = s & d"
    r = s & d2
    TestLog_ASSERT r = 10, testCompute & " .The result is: " & r

    testCompute = "s = '10', d = 20, r = s + d"
    d2 = "20"
    r = s + d2
    TestLog_ASSERT r = 1020, testCompute & " .The result is: " & r

    testCompute = "s = '10', d = 20, r = s & d"
    d2 = "20"
    r = s & d2
    TestLog_ASSERT r = 1020, testCompute & " .The result is: " & r

     ''''''''''''''
    s = "abc"
    Dim d3 As String
    testCompute = "s = 'abc', d = null, r = s + d"
    r = s + d3
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = null, r = s & d"
    r = s & d3
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = 20, r = s + d"
    d3 = "20"
    r = s + d3
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r

    testCompute = "s = 'abc', d = 20, r = s & d"
    d3 = "20"
    r = s & d3
    TestLog_ASSERT r = -1, testCompute & " .The result is: " & r
    Exit Sub

ErrorHandler:
    r = -1
'    TestLog_Comment "The next compute raises error: " & testCompute
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
