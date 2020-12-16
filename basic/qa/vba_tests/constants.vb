Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testConstants
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testConstants()

    TestUtilModule.TestInit

    Dim testName As String
    testName = "Test Constants"
    On Error GoTo errorHandler

    If GetGuiType() = 1 Then
        TestUtilModule.AssertTrue(vbNewline = vbCrLf, "vbNewLine is the same as vbCrLf on Windows")
    Else
        TestUtilModule.AssertTrue(vbNewLine = vbLf, "vbNewLine is the same as vbLf on others than Windows")
    End If

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

