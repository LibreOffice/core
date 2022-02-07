Option Explicit
Option Compatible

Function doUnitTest() As String
    TestUtil.TestInit
    verify_CallByName
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_CallByName()
    Const vbGet = 2, vbLet = 4, vbMethod = 1, vbSet = 8

    On Error GoTo errorHandler

    ' Basic modules are Objects
    TestUtil.AssertEqual(CallByName(TestUtil, "failCount", vbGet), 0, "CallByName(TestUtil, ""failCount"", vbGet")
    TestUtil.AssertEqual(CallByName(TestUtil, "passCount", vbGet), 1, "CallByName(TestUtil, ""passCount"", vbGet")
    TestUtil.AssertEqual(CallByName(TestUtil, "GetResult", vbMethod), "OK", "CallByName(TestUtil, ""GetResult"", vbMethod")

    ' Collection instances
    Dim c As New Collection
    c.Add(100, "1st") : c.Add(101)
    TestUtil.AssertEqual(CallByName(c,"Count", vbGet), 2, "CallByName(c,""Count"", vbGet)")
    c.Remove(2)
    TestUtil.AssertEqual(CallByName(callType:=vbGet, object:=c, procName:="Count"), 1, _
        "CallByName(callType:=vbGet, object:=c, procedureName:=""Count"")")

    ' ClassModule instances or UNO services are 'CallByNamable' objects too!

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_CallByName", Err, Error$, Erl)
    Resume Next
End Sub