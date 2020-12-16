Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testPartition()
If failCount <> 0 Or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_testPartition() as String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)


    Dim testName As String
    Dim retStr As String
    testName = "Test Partition function"
    On Error GoTo errorHandler

    retStr = Partition(20, 0, 98, 5)
    'MsgBox retStr
    TestLog_ASSERT retStr = "20:24", "the number 20 occurs in the range:" & retStr

    retStr = Partition(20, 0, 99, 1)
    'MsgBox retStr
    TestLog_ASSERT retStr = " 20: 20", "the number 20 occurs in the range:" & retStr

    retStr = Partition(120, 0, 99, 5)
    'MsgBox retStr
    TestLog_ASSERT retStr = "100:   ", "the number 120 occurs in the range:" & retStr

     retStr = Partition(-5, 0, 99, 5)
    'MsgBox retStr
    TestLog_ASSERT retStr = "   : -1", "the number -5 occurs in the range:" & retStr

     retStr = Partition(2, 0, 5, 2)
    'MsgBox retStr
    TestLog_ASSERT retStr = " 2: 3", "the number 2 occurs in the range:" & retStr
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testPartition = result
    Exit Function
errorHandler:
    TestLog_ASSERT (false), "verify_testPartition failed, hit error handler"
End Function

'%%include%% _test_asserts.vb
