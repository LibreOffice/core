Option Explicit

Function doUnitTest() As String
	TestUtil.TestInit

	TestUtil.AssertEqual(verify_ElseIf, "OK", "verify_ElseIf", "result")
	TestUtil.AssertEqual(verify_nested_Ifs, "OK", "verify_nested_Ifs", "result")
	TestUtil.AssertEqual(verify_oneline, "OK", "verify_oneline", "result")

	doUnitTest = TestUtil.GetResult()
End Function

Sub Main : MsgBox doUnitTest : End Sub

Function verify_ElseIf As String
	On Error GoTo catch

	If False Then
		verify_ElseIf = "Fail"
	ElseIf False Then
		verify_ElseIf = "Fail"
	ElseIf False Then
		verify_ElseIf = "Fail"
	ElseIf False Then verify_ElseIf = "Fail"
	Else
		verify_ElseIf = "OK"
	End If
	Exit Function

catch:
	TestUtil.ReportErrorHandler("verify_ElseIf", Err, Error$, Erl)
End Function

Function verify_oneline As String
	If False Then verify_oneline = "Fail" Else verify_oneline = "OK"
End Function

Function verify_nested_Ifs
	'
	On Error GoTo catch

	If False Then
		verify_nested_Ifs = "Fail"
	ElseIf True Then
		If True Then
			verify_nested_Ifs = "OK"
		End If
	ElseIf False Then
		verify_nested_Ifs = "Fail"
	ElseIf False Then verify_nested_Ifs = "Fail"
	Else
		verify_nested_Ifs = "Fail"
	EndIf
	Exit Function

catch:
	TestUtil.ReportErrorHandler("verify_nested_Ifs", Err, Error$, Erl)

End Function
