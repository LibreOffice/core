'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

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

Function doUnitTest() As String
    TestUtil.TestInit
    verify_testvartype
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_testvartype()
    On Error GoTo errorHandler

    ' VarType()

    TestUtil.AssertEqual( V_EMPTY, 0   , "V_EMPTY is not 0")
    TestUtil.AssertEqual( V_NULL, 1    , "V_NULL is not 1")
    TestUtil.AssertEqual( V_INTEGER, 2 , "V_INTEGER is not 2")
    TestUtil.AssertEqual( V_LONG, 3    , "V_LONG is not 3")
    TestUtil.AssertEqual( V_SINGLE, 4  , "V_SINGLE is not 4")
    TestUtil.AssertEqual( V_DOUBLE, 5  , "V_DOUBLE is not 5")
    TestUtil.AssertEqual( V_CURRENCY, 6, "V_CURRENCY is not 6")
    TestUtil.AssertEqual( V_DATE, 7    , "V_DATE is not 7")
    TestUtil.AssertEqual( V_STRING, 8  , "V_STRING is not 8")

    TestUtil.AssertEqual( VarType(Empty)  , V_EMPTY , "Vartype(Empty) is not V_EMPTY")
    TestUtil.AssertEqual( VarType(Null)   , V_NULL  , "Vartype(Empty) is not V_NULL")
    TestUtil.AssertEqual( VarType(Nothing), V_OBJECT, "Vartype(Empty) is not V_OBJECT")

    myErr = CVErr("errMsg")
    TestUtil.AssertEqual(VarType(varName:=int16), V_INTEGER, "VarType(varName:=int16) is not V_INTEGER")

    TestUtil.AssertEqual( VarType(int16), V_INTEGER , "VarType(int16) is not V_INTEGER")
    TestUtil.AssertEqual( VarType(int32), V_LONG    , "VarType(int32) is not V_LONG")
    TestUtil.AssertEqual( VarType(flt32), V_SINGLE  , "VarType(flt32) is not V_SINGLE" )
    TestUtil.AssertEqual( VarType(flt64), V_DOUBLE  , "VarType(flt64) is not V_DOUBLE" )
    TestUtil.AssertEqual( VarType(curr) , V_CURRENCY, "VarType(curr) is not V_CURRENCY" )
    TestUtil.AssertEqual( VarType(dat)  , V_DATE    , "VarType(dat) is not V_DATE" )
    TestUtil.AssertEqual( VarType(str)  , V_STRING  , "VarType(str) is not V_STRING" )
    TestUtil.AssertEqual( VarType(obj)  , V_OBJECT  , "VarType(obj) is not V_OBJECT" )
    TestUtil.AssertEqual( VarType(myUDF), V_OBJECT  , "VarType(myUDF) is not V_OBJECT" )
    TestUtil.AssertEqual( VarType(myErr), V_ERROR   , "VarType(myErr) is not V_ERROR" )
    TestUtil.AssertEqual( VarType(bool) , V_BOOLEAN , "VarType(bool) is not V_BOOLEAN" )
    TestUtil.AssertEqual( VarType(var)  , V_EMPTY   , "VarType(var) is not V_EMPTY" )
    TestUtil.AssertEqual( VarType(byt3) , V_BYTE    , "VarType(byt3) is not V_BYTE" )

    TestUtil.AssertEqual( VarType(int_)     , V_INTEGER , "VarType(int_) is not V_INTEGER" )
    TestUtil.AssertEqual( VarType(long_)    , V_LONG    , "VarType(long_) is not V_LONG" )
    TestUtil.AssertEqual( VarType(single_)  , V_SINGLE  , "VarType(single_) is not V_SINGLE" )
    TestUtil.AssertEqual( VarType(double_)  , V_DOUBLE  , "VarType(double_) is not V_CURRENCY" )
    TestUtil.AssertEqual( VarType(currency_), V_CURRENCY, "VarType(currency_) is not V_CURRENCY" )
    TestUtil.AssertEqual( VarType(string_)  , V_STRING  , "VarType(string_) is not V_STRING" )

    TestUtil.AssertEqual( VarType(intArray) , V_ARRAY+V_INTEGER , "VarType(intArray) is not V_ARRAY+V_INTEGER" )
    TestUtil.AssertEqual( VarType(lngArray) , V_ARRAY+V_LONG    , "VarType(lngArray) is not V_ARRAY+V_LONG" )
    TestUtil.AssertEqual( VarType(sngArray) , V_ARRAY+V_SINGLE  , "VarType(sngArray) is not V_ARRAY+V_SINGLE" )
    TestUtil.AssertEqual( VarType(dblArray) , V_ARRAY+V_DOUBLE  , "VarType(dblArray) is not V_ARRAY+V_DOUBLE" )
    TestUtil.AssertEqual( VarType(curArray) , V_ARRAY+V_CURRENCY, "VarType(curArray) is not V_ARRAY+V_CURRENCY" )
    TestUtil.AssertEqual( VarType(datArray) , V_ARRAY+V_DATE    , "VarType(datArray) is not V_ARRAY+V_DATE" )
    TestUtil.AssertEqual( VarType(strArray) , V_ARRAY+V_STRING  , "VarType(strArray) is not V_ARRAY+V_STRING" )
    TestUtil.AssertEqual( VarType(objArray) , V_ARRAY+V_OBJECT  , "VarType(objArray) is not V_ARRAY+V_OBJECT" )
    'TestUtil.AssertEqual( VarType(***Array) , V_ARRAY+V_ERROR  , "VarType(***Array) is not V_ARRAY+V_ERROR" )
    TestUtil.AssertEqual( VarType(boolArray), V_ARRAY+V_BOOLEAN , "VarType(boolArray) is not V_ARRAY+V_BOOLEAN" )
    TestUtil.AssertEqual( VarType(varArray) , V_ARRAY+V_VARIANT , "VarType(varArray) is not V_ARRAY+V_VARIANT" )
    TestUtil.AssertEqual( VarType(byteArray), V_ARRAY+V_BYTE    , "VarType(byteArray) is not V_ARRAY+V_BYTE" )

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_testvartype", Err, Error$, Erl)
End Sub
