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

Dim int16 As Integer,  int32 As Long,    float As Single, flaot As Double, _
    curr  As Currency, dat   As Date,    str   As String, obj   As Object, _
    myUDF As UDF,      var   As Variant, byt3  As Byte,   bool  As Boolean

Dim int_%, long_&, single_!, double_#, currency_@, string_$, array_

Dim intArray()  As Integer,  lngArray(5) As Long, sngArray!() As Single, dblArray#(8) As Double, _
    curArray@() As Currency, datArray()  As Date, strArray$() As String, objArray()   As Object, _
    varArray()  As Variant,  byteArray() As Byte, boolArray() As Boolean

Const V_ARRAY=8192, V_OBJECT=9, V_BOOLEAN=11, V_VARIANT=12, V_BYTE=17

Function doUnitTest as Integer
    ' VarType()
    doUnitTest = 0 ' Assume failure by default

    If V_EMPTY    <> 0 Or V_NULL <> 1                                   Then Exit Function
    If V_INTEGER  <> 2 Or V_LONG <> 3 Or V_SINGLE <> 4 Or V_DOUBLE <> 5 Then Exit Function
    If V_CURRENCY <> 6 Or V_DATE <> 7 Or V_STRING <> 8                  Then Exit Function

    If VarType(NULL)    <> V_NULL Or VarType(EMPTY) <> V_EMPTY Then Exit Function
    If VarType(NOTHING) <> V_OBJECT                            Then Exit Function

    If VarType(int16) <> V_INTEGER  Or VarType(int32) <> V_LONG     Then Exit Function
    If VarType(float) <> V_SINGLE   Or VarType(flaot) <> V_DOUBLE   Then Exit Function
    If VarType(curr)  <> V_CURRENCY Or VarType(dat)   <> V_DATE     Then Exit Function
    If VarType(byt3)  <> V_BYTE     Or VarType(bool)  <> V_BOOLEAN  Then Exit Function
    If VarType(str)   <> V_STRING   Or VarType(obj)   <> V_OBJECT   Then Exit Function
    If VarType(myUDF) <> V_OBJECT   Or VarType(var)   <> V_EMPTY    Then Exit Function

    If VarType(int_)      <> V_INTEGER  Or VarType(long_)   <> V_LONG   Then Exit Function
    If VarType(single_)   <> V_SINGLE   Or VarType(double_) <> V_DOUBLE Then Exit Function
    If VarType(currency_) <> V_Currency Or VarType(string_) <> V_String Then Exit Function

    If VarType(intArray)  <> V_ARRAY+V_INTEGER  Then Exit Function
    If VarType(lngArray)  <> V_ARRAY+V_LONG     Then Exit Function
    If VarType(sngArray)  <> V_ARRAY+V_SINGLE   Then Exit Function
    If VarType(dblArray)  <> V_ARRAY+V_DOUBLE   Then Exit Function
    If VarType(curArray)  <> V_ARRAY+V_CURRENCY Then Exit Function
    If VarType(datArray)  <> V_ARRAY+V_DATE     Then Exit Function
    If VarType(strArray)  <> V_ARRAY+V_STRING   Then Exit Function
    If VarType(objArray)  <> V_ARRAY+V_OBJECT   Then Exit Function
    If VarType(boolArray) <> V_ARRAY+V_BOOLEAN  Then Exit Function
    If VarType(varArray)  <> V_ARRAY+V_VARIANT  Then Exit Function
    If VarType(byteArray) <> V_ARRAY+V_BYTE     Then Exit Function

    doUnitTest = 1 ' All checks passed
End Function

Sub DEV_TEST
    Print doUnitTest
End Sub
