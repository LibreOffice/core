Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
	result = verify_stringReplace()
	If failCount <> 0 Or passCount = 0 Then
		doUnitTest = 0
	Else
        doUnitTest = 1
    End If
End Function

Function verify_stringReplace() As String
	passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)

	' tdf#132389 - case-insensitive operation for non-ASCII characters
	retStr = Replace("ABCabc", "b", "*")
	TestLog_ASSERT retStr, "A*Ca*c", "case-insensitive ASCII: " & retStr
	retStr = Replace("АБВабв", "б", "*")
	TestLog_ASSERT retStr, "А*Ва*в", "case-insensitive non-ASCII: " & retStr

	result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_stringReplace = result
End Function

Sub TestLog_ASSERT(actual As Variant, expected As Variant, testName As String)
	If expected = actual Then
		passCount = passCount + 1
	Else
		result = result & Chr$(10) & "Failed: " & testName & " returned " & actual & ", expected " & expected
		failCount = failCount + 1
	End If
End Sub
