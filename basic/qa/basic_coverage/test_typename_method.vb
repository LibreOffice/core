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
Dim myUDF as UDF

Dim int16 As Integer,   int32 As Long,    flt32 As Single, flt64 As Double, _
    curr  As Currency,  dat   As Date,    str   As String, obj   As Object, _
    myErr As Variant,   var   As Variant, byt3  As Byte,   bool  As Boolean

Dim int_%, long_&, single_!, double_#, currency_@, string_$, array_

Dim intArray()  As Integer,  lngArray(5) As Long, sngArray!() As Single, dblArray#(8)   As Double, _
    curArray@() As Currency, datArray()  As Date, strArray$() As String, objArray(5,15) As Object, _
    varArray()  As Variant,  byteArray() As Byte, boolArray() As Boolean

Function doUnitTest ' TypeName()

    myErr = CVErr(0.56E-41)
    assert( TypeName(int16) = "Integer" , "TypeName(int16) is not ""Integer""")
    assert( TypeName(int32) = "Long"    , "TypeName(int32) is not ""Long""")
    assert( TypeName(flt32) = "Single"  , "TypeName(flt32) is not ""Single""" )
    assert( TypeName(flt64) = "Double"  , "TypeName(flt64) is not ""Double""" )
    assert( TypeName(curr)  = "Currency", "TypeName(curr) is not ""Currency""" )
    assert( TypeName(dat)   = "Date"    , "TypeName(dat) is not ""Date""" )
    assert( TypeName(byt3)  = "Byte"    , "TypeName(byt3) is not ""Byte""" )
    assert( TypeName(MyErr) = "Error"   , "TypeName(MyErr) is not ""Error""" )
    assert( TypeName(bool)  = "Boolean" , "TypeName(bool) is not ""Boolean""" )
    assert( TypeName(str)   = "String"  , "TypeName(str) is not ""String""" )
    assert( TypeName(obj)   = "Object"  , "TypeName(obj) is not ""Object""" )
    assert( TypeName(myUDF) = "Object"  , "TypeName(myUDF) is not ""Object""" )
    assert( TypeName(var)   = "Empty"   , "TypeName(var) is not ""Empty""" )

    assert( TypeName(int_)      = "Integer" , "TypeName(int_) is not ""Integer""" )
    assert( TypeName(long_)     = "Long"    , "TypeName(long_) is not ""Long""" )
    assert( TypeName(single_)   = "Single"  , "TypeName(single_) is not ""Single""" )
    assert( TypeName(double_)   = "Double"  , "TypeName(double_) is not ""Double""" )
    assert( TypeName(currency_) = "Currency", "TypeName(currency_) is not ""Currency""" )
    assert( TypeName(string_)   = "String"  , "TypeName(string_) is not ""String""" )

    assert( TypeName(intArray)  = "Integer()" , "TypeName(intArray) is not ""Integer()""" )
    assert( TypeName(lngArray)  = "Long()"    , "TypeName(lngArray) is not ""Long()""" )
    assert( TypeName(sngArray)  = "Single()"  , "TypeName(sngArray) is not ""Single()""" )
    assert( TypeName(dblArray)  = "Double()"  , "TypeName(dblArray) is not ""Double()""" )
    assert( TypeName(curArray)  = "Currency()", "TypeName(curArray) is not ""Currency()""" )
    assert( TypeName(datArray)  = "Date()"    , "TypeName(datArray) is not ""Date()""" )
    assert( TypeName(strArray)  = "String()"  , "TypeName(strArray) is not ""String()""" )
    assert( TypeName(objArray)  = "Object()"  , "TypeName(objArray) is not ""Object()""" )
    assert( TypeName(boolArray) = "Boolean()" , "TypeName(boolArray) is not ""Boolean()""" )
    assert( TypeName(varArray)  = "Variant()" , "TypeName(varArray) is not ""Variant()""" )
    assert( TypeName(byteArray) = "Byte()"    , "TypeName(byteArray) is not ""Byte()""" )
    If FailedAssertion Then
        doUnitTest = "test_typename_method.vb failed" + messages
        Exit Function
    EndIf
    doUnitTest = 1 ' All checks passed
End Function

Sub DEV_TEST : Print doUnitTest : End Sub

Dim failedAssertion As Boolean, messages As String

Sub assert(expression As Boolean, errMessage As String)
    if ( Not expression ) Then
       messages = messages + Chr(10) + ErrMessage
       failedAssertion = True
    EndIf
End Sub