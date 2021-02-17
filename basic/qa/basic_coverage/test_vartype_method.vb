'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
Type UDF ' User defined type
    l As Object
    s as String
End Type
Dim myUDF As UDF

Dim int16 As Integer,   int32 As Long,    flt32 As Single, flt64 As Double, _
    curr  As Currency,  dat   As Date,    str   As String, obj   As Object, _
    myErr As Variant,   var   As Variant, byt3  As Byte,   bool  As Boolean
    
Dim int_%, long_&, single_!, double_#, currency_@, string_$, array_

Dim intArray()  As Integer,  lngArray(5) As Long, sngArray!() As Single, dblArray#(8) As Double, _
    curArray@() As Currency, datArray()  As Date, strArray$() As String, objArray()   As Object, _
    varArray()  As Variant,  byteArray() As Byte, boolArray() As Boolean

' Constants that candidate for public exposure
Private Const V_ARRAY=8192, V_OBJECT=9, V_ERROR=10, V_BOOLEAN=11, V_VARIANT=12, V_BYTE=17

Function doUnitTest
    ' VarType()

    assert( V_EMPTY = 0   , "V_EMPTY is not 0")
    assert( V_NULL = 1    , "V_NULL is not 1")
    assert( V_INTEGER = 2 , "V_INTEGER is not 2")
    assert( V_LONG = 3    , "V_LONG is not 3")
    assert( V_SINGLE = 4  , "V_SINGLE is not 4")
    assert( V_DOUBLE = 5  , "V_DOUBLE is not 5")
    assert( V_CURRENCY = 6, "V_CURRENCY is not 6")
    assert( V_DATE = 7    , "V_DATE is not 7")
    assert( V_STRING = 8  , "V_STRING is not 8")

    assert( VarType(Empty)   = V_EMPTY , "Vartype(Empty) is not V_EMPTY")
    assert( VarType(Null)    = V_NULL  , "Vartype(Empty) is not V_NULL")
    assert( VarType(Nothing) = V_OBJECT, "Vartype(Empty) is not V_OBJECT")

    myErr = CVErr("errMsg")
    assert( VarType(int16) = V_INTEGER , "VarType(int16) is not V_INTEGER")
    assert( VarType(int32) = V_LONG    , "VarType(int32) is not V_LONG")
    assert( VarType(flt32) = V_SINGLE  , "VarType(flt32) is not V_SINGLE" )
    assert( VarType(flt64) = V_DOUBLE  , "VarType(flt64) is not V_DOUBLE" )
    assert( VarType(curr)  = V_CURRENCY, "VarType(curr) is not V_CURRENCY" )
    assert( VarType(dat)   = V_DATE    , "VarType(dat) is not V_DATE" )
    assert( VarType(str)   = V_STRING  , "VarType(str) is not V_STRING" )
    assert( VarType(obj)   = V_OBJECT  , "VarType(obj) is not V_OBJECT" )
    assert( VarType(myUDF) = V_OBJECT  , "VarType(myUDF) is not V_OBJECT" )
    assert( VarType(myErr) = V_ERROR   , "VarType(myErr) is not V_ERROR" )
    assert( VarType(bool)  = V_BOOLEAN , "VarType(bool) is not V_BOOLEAN" )
    assert( VarType(var)   = V_EMPTY   , "VarType(var) is not V_EMPTY" )
    assert( VarType(byt3)  = V_BYTE    , "VarType(byt3) is not V_BYTE" )

    assert( VarType(int_)      = V_INTEGER , "VarType(int_) is not V_INTEGER" )
    assert( VarType(long_)     = V_LONG    , "VarType(long_) is not V_LONG" )
    assert( VarType(single_)   = V_SINGLE  , "VarType(single_) is not V_SINGLE" )
    assert( VarType(double_)   = V_DOUBLE  , "VarType(double_) is not V_CURRENCY" )
    assert( VarType(currency_) = V_CURRENCY, "VarType(currency_) is not V_CURRENCY" )
    assert( VarType(string_)   = V_STRING  , "VarType(string_) is not V_STRING" )

    assert( VarType(intArray)  = V_ARRAY+V_INTEGER , "VarType(intArray) is not V_ARRAY+V_INTEGER" )
    assert( VarType(lngArray)  = V_ARRAY+V_LONG    , "VarType(lngArray) is not V_ARRAY+V_LONG" )
    assert( VarType(sngArray)  = V_ARRAY+V_SINGLE  , "VarType(sngArray) is not V_ARRAY+V_SINGLE" )
    assert( VarType(dblArray)  = V_ARRAY+V_DOUBLE  , "VarType(dblArray) is not V_ARRAY+V_DOUBLE" )
    assert( VarType(curArray)  = V_ARRAY+V_CURRENCY, "VarType(curArray) is not V_ARRAY+V_CURRENCY" )
    assert( VarType(datArray)  = V_ARRAY+V_DATE    , "VarType(datArray) is not V_ARRAY+V_DATE" )
    assert( VarType(strArray)  = V_ARRAY+V_STRING  , "VarType(strArray) is not V_ARRAY+V_STRING" )
    assert( VarType(objArray)  = V_ARRAY+V_OBJECT  , "VarType(objArray) is not V_ARRAY+V_OBJECT" )
    'assert( VarType(***Array)  = V_ARRAY+V_ERROR  , "VarType(***Array) is not V_ARRAY+V_ERROR" )
    assert( VarType(boolArray) = V_ARRAY+V_BOOLEAN , "VarType(boolArray) is not V_ARRAY+V_BOOLEAN" )
    assert( VarType(varArray)  = V_ARRAY+V_VARIANT , "VarType(varArray) is not V_ARRAY+V_VARIANT" )
    assert( VarType(byteArray) = V_ARRAY+V_BYTE    , "VarType(byteArray) is not V_ARRAY+V_BYTE" )

    If failedAssertion Then
        doUnitTest = "test_vartype_method.vb fails" + messages
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
