'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Dim i As Integer, j As Long,     k As Single, l As Double, _
    m As Byte,    n As Boolean,  o As Date,   p As Currency, _
    q As Object,  r As String,   v As Variant
    
Dim int_%, long_&, single_!, double_#, currency_@, string_$, array_

Function doUnitTest as Integer
    ' VarType()
    doUnitTest = 0 ' Assume failure by default
    
    array_ = Array(1,"txt") ' TypeName / VarType = "Variant()" / 8204

    If    V_EMPTY <> 0 Or V_NULL <> 1                                   Then Exit Function
    If  V_INTEGER <> 2 Or V_LONG <> 3 Or V_SINGLE <> 4 Or V_DOUBLE <> 5 Then Exit Function
    If V_CURRENCY <> 6 Or V_DATE <> 7 Or V_STRING <> 8                  Then Exit Function

    If VarType(Null) <> V_NULL or VarType(Empty) <> V_EMPTY Then Exit Function
    If VarType(NOthing) <> 9                                Then Exit Function

    If Vartype(i) <> V_INTEGER Or Vartype(j) <> V_LONG     Then Exit Function
    If VarType(k) <> V_SINGLE  Or VarType(l) <> V_DOUBLE   Then Exit Function
    If Vartype(m) <> 17        Or Vartype(n) <> 11         Then Exit Function
    If VarType(o) <> V_DATE    Or VarType(p) <> V_CURRENCY Then Exit Function   
    If Vartype(q) <> 9         Or Vartype(r) <> V_STRING   Then Exit Function 
    If VarType(v) <> V_EMPTY                               Then Exit Function 
    
    If      Vartype(int_) <> V_INTEGER  Or   Vartype(long_) <> V_LONG   Then Exit Function 
    If   VarType(single_) <> V_SINGLE   Or VarType(double_) <> V_DOUBLE Then Exit Function 
    If Vartype(currency_) <> V_Currency Or Vartype(string_) <> V_String Then Exit Function 

    If VarType(array_) <> 8204 Then Exit Function 

    doUnitTest = 1 ' All checks passed
End Function

Sub DEV_TEST
    Print doUnitTest
End Sub
