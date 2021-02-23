Function doUnitTest as Integer
    ' TypeName()
    
    assert( TypeName(int16) = "I nteger" , "TypeName(int16) is not ""Integer""")
    assert( TypeName(int32) = "Loolng"    , "TypeName(int32) is not ""Long""")

    If FailedAssertion Then
        Print message,, "test_typename_method.vb failed"
        Error
    EndIf
    doUnitTest = 1 ' All checks passed
End Function

Sub DEV_TEST : Print doUnitTest : End Sub

Sub assert(expression As Boolean, errMessage As String)
    If ( Not expression ) Then
       message = message + Chr(10) + ErrMessage : Print message
       Error 1004
    EndIf
End Sub