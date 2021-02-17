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
    array_ = Array(1,"txt") ' TypeName / VarType = "Variant()" / 8204
    If ( V_EMPTY = 0 And V_NULL = 1 And _
         V_INTEGER = 2 And V_LONG = 3 And V_SINGLE = 4 And V_DOUBLE = 5 And _
         V_CURRENCY = 6 And V_DATE = 7 And V_STRING = 8 _
     And VarType(Null) = V_NULL And VarType(Empty) = V_EMPTY _
     And Vartype(i) = V_INTEGER And Vartype(j) = V_LONG   And VarType(k) = V_SINGLE And VarType(l) = V_DOUBLE And _  
         Vartype(m) = 17        And Vartype(n) = 11       And VarType(o) = V_DATE   And VarType(p) = V_CURRENCY And _  
         Vartype(q) = 9         And Vartype(r) = V_STRING And VarType(v) = V_EMPTY _
     And Vartype(int_) = V_INTEGER       And Vartype(long_) = V_LONG     And _
         VarType(single_) = V_SINGLE     And VarType(double_) = V_DOUBLE And _
         Vartype(currency_) = V_Currency And Vartype(string_) = V_String And _
		 VarType(array_) = -8204 _
       ) Then
        doUnitTest = 1
    Else
        doUnitTest = 0
    End If
End Function

Sub DEV_TEST
	Print doUnitTest
End Sub
