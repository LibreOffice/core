' -*- tab-width: 4; indent-tabs-mode: nil -*-

If WScript.Arguments.Count <> 1 Then
   WScript.Echo "Pass $(SRCDIR) as parameter"
   WScript.Quit(1)
End If

srcdir = WScript.Arguments.Item(0)

exitStatus = 0

testCounter = 0
okCounter = 0

Sub ExitWithReport
    If okCounter <> testCounter Then
        exitStatus = 1
    End If
    WScript.Echo "OK (" + CStr(okCounter) + ")"
    WScript.Quit(exitstatus)
End Sub

Sub CheckFatal(expr)
    testCounter = testCounter + 1
    If Not Eval(expr) Then
        WScript.Echo "FAIL: " & expr
        ExitWithReport
    Else
        WScript.Echo "PASS: " & expr
        okCounter = okCounter + 1
    End If
End Sub

Sub Check(expr)
    testCounter = testCounter + 1
    If Not Eval(expr) Then
        WScript.Echo "FAIL: " & expr
    Else
        WScript.Echo "PASS: " & expr
        okCounter = okCounter + 1
    End If
End Sub

Sub CheckIfExpected(expr, expected)
    testCounter = testCounter + 1
    actual = Eval(expr)
    If actual <> expected Then
        WScript.Echo "FAIL: Value of '" & expr & "' was expected to be '" & CStr(expected) & "', but was " & CStr(actual)
    Else
        WScript.Echo "PASS: " & expr & " == " & CStr(expected)
        okCounter = okCounter + 1
    End If
End Sub

Sub CheckErrorFatal(test)
    testCounter = testCounter + 1
    Execute(test)
    If Err.Number <> 0 Then
        WScript.Echo "FAIL: " & test
        WScript.Echo "ERROR: " & Err.Description
        ExitWithReport
    Else
        WScript.Echo "PASS: " & test
        okCounter = okCounter + 1
    End If
End Sub

Sub CheckError(test)
    testCounter = testCounter + 1
    Execute(test)
    If Err.Number <> 0 Then
        WScript.Echo "FAIL: " & test
        WScript.Echo "ERROR: " & Err.Description
    Else
        WScript.Echo "PASS: " & test
        okCounter = okCounter + 1
    End If
End Sub

WScript.Echo "Running Automation client tests"

On Error Resume Next

' FIXME: How can we ever make this work specifically with the
' LibreOffice in instdir, when WScript.CreateObject() wants the
' symbolic name that it then looks up from the Registry to find the
' CLSID of the class?

CheckErrorFatal "Set writer = WScript.CreateObject(""Writer.Application"")"
CheckErrorFatal "writer.Visible = True"
CheckErrorFatal "writer.Caption = ""=== This is Writer ==="""
CheckErrorFatal "writer.ShowMe"

CheckErrorFatal "Set documents = writer.Documents"

' Open two randomly chosen documents
CheckErrorFatal "Set d1 = documents.Open(""" & srcdir & "/sw/qa/extras/ww8export/data/n325936.doc"")"
CheckErrorFatal "Set d2 = documents.Open(""" & srcdir & "/sw/qa/extras/ww8export/data/bnc636128.doc"")"

CheckErrorFatal "n1 = d1.FullName"
CheckErrorFatal "n2 = d2.FullName"

CheckIfExpected "n1", "n325936.doc"
CheckIfExpected "n2", "bnc636128.doc"

CheckErrorFatal "Set c1 = d1.Content"
CheckErrorFatal "Set c2 = d2.Content"

' The range of characters in these documents
CheckIfExpected "c1.Start", 0
CheckIfExpected "c1.End", 4
CheckIfExpected "c2.Start", 0
CheckIfExpected "c2.End", 42

' Check number of paragraphs in each document
CheckErrorFatal "Set p1 = d1.Paragraphs"
nparas = 0
For Each i in p1
    nparas = nparas + 1
Next
CheckIfExpected "nparas", 1

CheckErrorFatal "Set p2 = d2.Paragraphs"
nparas = 0
For Each i in p2
    nparas = nparas + 1
Next
CheckIfExpected "nparas", 1

CheckErrorFatal "writer.Quit"

ExitWithReport
