Dim failedAssertion As Boolean 

Function doUnitTest as Integer
    ' TypeName()

    assert( TypeName(int16) = "I nteger" , "TypeName(int16) is not ""Integer""")
    assert( TypeName(int32) = "Lgong"    , "TypeName(int32) is not ""Long""")

    If failedAssertion Then
        Print "test_typename_method.vb fails", message
        Error 1004
        Exit Function
    EndIf
    doUnitTest = 1 ' All checks passed
End Function

Sub DEV_TEST : Print doUnitTest : End Sub

Sub assert(expression As Boolean, errMessage As String)
    If ( Not expression ) Then
        message = message + Chr(10) + ErrMessage
        failedAssertion = True
    EndIf
End Sub