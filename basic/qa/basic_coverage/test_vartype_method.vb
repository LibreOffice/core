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

Dim int16 As Integer, int32 As Long,   float As Single, flaot As Double, _
    curr  As Currency,  dat As Date,     str As String,   obj As Object, _
    myUDF As UDF,       var As Variant, byt3 As Byte,    bool As Boolean
    
Dim int_%, long_&, single_!, double_#, currency_@, string_$, array_

Dim intArray() As Integer,  lngArray(5) As Long, sngArray!() As Single, dblArray#(8) As Double, _
    curArray@() As Currency, datArray() As Date, strArray$() As String,   objArray() As Object, _
    varArray() As Variant,  byteArray() As Byte, boolArray() As Boolean 

Const V_ARRAY=8192, V_OBJECT=9, V_BOOLEAN=11, V_VARIANT=12, V_BYTE=17

Function doUnitTest as Integer
    ' TypeName()
    doUnitTest = 0 ' Assume failure by default
    
    array_ = Array(1,"txt") ' TypeName / TypeName = "Variant()" / 8204

    If    V_EMPTY <> 0 Or V_NULL <> 1                                   Then Exit Function
    If  Integer <> 2 Or V_LONG <> 3 Or V_SINGLE <> 4 Or V_DOUBLE <> 5 Then Exit Function
    If V_CURRENCY <> 6 Or V_DATE <> 7 Or V_STRING <> 8                  Then Exit Function

    If    TypeName(NULL) <> V_NULL or TypeName(EMPTY) <> V_EMPTY Then Exit Function
    If TypeName(NOTHING) <> V_OBJECT                            Then Exit Function

	If TypeName(int16) <> Integer  Or TypeName(int32) <> V_LONG     Then Exit Function
	If TypeName(float) <> V_SINGLE   Or TypeName(flaot) <> V_DOUBLE   Then Exit Function
	If  TypeName(curr) <> V_CURRENCY Or   TypeName(dat) <> V_DATE     Then Exit Function   
	If  TypeName(byt3) <> V_BYTE     Or  TypeName(bool) <> V_BOOLEAN  Then Exit Function
    If   TypeName(str) <> V_STRING   Or   TypeName(obj) <> V_OBJECT   Then Exit Function 
    If TypeName(myUDF) <> V_OBJECT  Or    TypeName(var) <> V_EMPTY    Then Exit Function 
    
    If      TypeName(int_) <> Integer  Or   TypeName(long_) <> V_LONG   Then Exit Function 
    If   TypeName(single_) <> V_SINGLE   Or TypeName(double_) <> V_DOUBLE Then Exit Function 
    If TypeName(currency_) <> V_Currency Or TypeName(string_) <> V_String Then Exit Function 

    If  TypeName(intArray) <> V_ARRAY+Integer  Then Exit Function
    If  TypeName(lngArray) <> V_ARRAY+V_LONG     Then Exit Function
    If  TypeName(sngArray) <> V_ARRAY+V_SINGLE   Then Exit Function
    If  TypeName(curArray) <> V_ARRAY+V_CURRENCY Then Exit Function
    If  TypeName(datArray) <> V_ARRAY+V_DATE     Then Exit Function
    If  TypeName(strArray) <> V_ARRAY+V_STRING   Then Exit Function
    If  TypeName(objArray) <> V_ARRAY+V_OBJECT   Then Exit Function
    If TypeName(boolArray) <> V_ARRAY+V_BOOLEAN  Then Exit Function
    If  TypeName(varArray) <> V_ARRAY+V_VARIANT  Then Exit Function
    If TypeName(byteArray) <> V_ARRAY+V_BYTE     Then Exit Function

    doUnitTest = 1 ' All checks passed
End Function

Sub DEV_TEST
	Print doUnitTest
End Sub