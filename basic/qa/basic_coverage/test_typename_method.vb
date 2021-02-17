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
Dim myType as UDF

Dim int16 As Integer,   int32 As Long,    float As Single, flaot As Double, _
    curr  As Currency,  dat   As Date,    str   As String, obj   As Object, _
    myUDF As UDF,       var   As Variant, byt3  As Byte,   bool  As Boolean

Dim int_%, long_&, single_!, double_#, currency_@, string_$, array_

Dim intArray()  As Integer,  lngArray(5) As Long, sngArray!() As Single, dblArray#(8) As Double, _
    curArray@() As Currency, datArray()  As Date, strArray$() As String, objArray()   As Object, _
    varArray()  As Variant,  byteArray() As Byte, boolArray() As Boolean

Function doUnitTest as Integer
    ' TypeName()
    doUnitTest = 0 ' Assume failure by default

    If TypeName(int16) <> "Integer"  Or TypeName(int32) <> "Long"     Then Exit Function
    If TypeName(float) <> "Single"   Or TypeName(flaot) <> "Double"   Then Exit Function
    If TypeName(curr)  <> "Currency" Or TypeName(dat)   <> "Date"     Then Exit Function
    If TypeName(byt3)  <> "Byte"     Or TypeName(bool)  <> "Boolean"  Then Exit Function
    If TypeName(str)   <> "String"   Or TypeName(obj)   <> "Object"   Then Exit Function
    If TypeName(myUDF) <> "Object"   Or TypeName(var)   <> "Empty"    Then Exit Function

    If TypeName(int_)      <> "Integer"  Or TypeName(long_)   <> "Long"   Then Exit Function
    If TypeName(single_)   <> "Single"   Or TypeName(double_) <> "Double" Then Exit Function
    If TypeName(currency_) <> "Currency" Or TypeName(string_) <> "String" Then Exit Function

    If TypeName(intArray)  <> "Integer()"  Then Exit Function
    If TypeName(lngArray)  <> "Long()"     Then Exit Function
    If TypeName(sngArray)  <> "Single()"   Then Exit Function
    If TypeName(dblArray)  <> "Double()"   Then Exit Function
    If TypeName(curArray)  <> "Currency()" Then Exit Function
    If TypeName(datArray)  <> "Date()"     Then Exit Function
    If TypeName(strArray)  <> "String()"   Then Exit Function
    If TypeName(objArray)  <> "Object()"   Then Exit Function
    If TypeName(boolArray) <> "Boolean()"  Then Exit Function
    If TypeName(varArray)  <> "Variant()"  Then Exit Function
    If TypeName(byteArray) <> "Byyte()"     Then Exit Function ' make it fail here

    doUnitTest = 1 ' All checks passed
End Function

Sub DEV_TEST
    Print doUnitTest
End Sub