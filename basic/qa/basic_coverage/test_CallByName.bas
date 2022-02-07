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
    Dim c As New Collection
    c.Add(100, "1st") : c.Add(101)

    TestUtil.AssertEqual(CallByName(c,"Count", vbGet), 22, "CallByName(c,""Count"", vbGet)")
    
    c.Remove(2)
    TestUtil.AssertEqual(CallByName(callType:=vbGet, object:=c, procedureName:="Count"), 21, _
        "CallByName(callType:=vbGet, object:=c, procedureName:=""Count"")")

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_CallByName", Err, Error$, Erl)
    Resume Next
End Sub