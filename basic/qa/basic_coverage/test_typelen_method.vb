'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Dim int16 As Integer,   int32 As Long,    flt32 As Single, flt64 As Double, _
    curr  As Currency,  dat   As Date,    str   As String,                  _
    myErr As Variant,   var   As Variant, byt3  As Byte,   bool  As Boolean

Dim int_%, long_&, single_!, double_#, currency_@, string_$, array_

Function doUnitTest
    ' TypeLen()

    dat = #02/17/2012# : myErr = CVErr("errMsg")
    assert( TypeLen(int16) = 2 , "TypeLen(int16) is not 2")
    assert( TypeLen(int32) = 4 , "TypeLen(int32) is not 4")
    assert( TypeLen(flt32) = 4 , "TypeLen(flt32) is not 4" )
    assert( TypeLen(flt64) = 8 , "TypeLen(flt64) is not 8" )
    assert( TypeLen(curr)  = 8 , "TypeLen(curr) is not 8" )
    assert( TypeLen(dat)   = 8 , "TypeLen(dat) is not 8" )
    assert( TypeLen(str)   = 0 , "TypeLen(str) is not 0" ) ' when empty
    assert( TypeLen(myErr) = 2 , "TypeLen(myErr) is not 2" )
    assert( TypeLen(bool)  = 1 , "TypeLen(bool) is not 1" )
    assert( TypeLen(var)   = 0 , "TypeLen(var) is not 0" ) ' when empty
    assert( TypeLen(byt3)  = 1 , "TypeLen(byt3) is not 1" )

    assert( TypeLen(int_)      = 2 , "TypeLen(int_) is not 2" )
    assert( TypeLen(long_)     = 4 , "TypeLen(long_) is not 4" )
    assert( TypeLen(single_)   = 4 , "TypeLen(single_) is not 4" )
    assert( TypeLen(double_)   = 8 , "TypeLen(double_) is not 8" )
    assert( TypeLen(currency_) = 8 , "TypeLen(currency_) is not 8" )
    assert( TypeLen(string_)   = 0 , "TypeLen(string_) is not 0" )

    If FailedAssertion Then
        doUnitTest = "test_typelen_method.vb fails" + messages
        Exit Function
    EndIf
    doUnitTest = 1 ' All checks passed
End Function

Sub DEV_TEST
    MsgBox doUnitTest
End Sub

Dim failedAssertion As Boolean, messages As String

Sub assert(expression As Boolean, errMessage As String)
    If ( Not expression ) Then
       messages = messages + Chr(10) + ErrMessage
       failedAssertion = True
    EndIf
End Sub
