Option VBASupport 1
Option Explicit
'%%include%% _test_header.vb

Function doUnitTest() As String
result = verify_testSwitch()
If failCount <> 0 or passCount = 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function



Function verify_testSwitch() As String

    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

    Dim testName As String
    Dim date1, date2
    testName = "Test Switch function"
    On Error GoTo errorHandler

    date2 = "French"
    date1 = MatchUp("Paris")
    TestLog_ASSERT date1 = date2, "the return Switch is: " & date1

    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_testSwitch = result

    Exit Function
errorHandler:
        TestLog_ASSERT (False), testName & ": hit error handler"
End Function

Function MatchUp(CityName As String)
    MatchUp = Switch(CityName = "London", "English", CityName _
                    = "Rome", "Italian", CityName = "Paris", "French")
End Function



'%%include%% _test_asserts.vb

