rem
rem This file is part of the LibreOffice project.
rem
rem This Source Code Form is subject to the terms of the Mozilla Public
rem License, v. 2.0. If a copy of the MPL was not distributed with this
rem file, You can obtain one at http://mozilla.org/MPL/2.0/.
rem
rem This file incorporates work covered by the following license notice:
rem
rem   Licensed to the Apache Software Foundation (ASF) under one or more
rem   contributor license agreements. See the NOTICE file distributed
rem   with this work for additional information regarding copyright
rem   ownership. The ASF licenses this file to you under the Apache
rem   License, Version 2.0 (the "License"); you may not use this file
rem   except in compliance with the License. You may obtain a copy of
rem   the License at http://www.apache.org/licenses/LICENSE-2.0 .
rem

OPTION EXPLICIT
OPTION COMPATIBLE

Sub Main
COMPATIBILITYMODE(true)

If runtest = -1 Then
  MsgBox "Test Failed!!!"
Else
  MsgBox "Test Succeeded"
End If

End Sub

Function runtest() As Integer
Dim inBool As Boolean, inBool2 As Boolean, outBool As Boolean
Dim inByte As Integer, inByte2 As Integer
Dim inShort As Integer, inShort2 As Integer
Dim inLong As Long, inLong2 As Long, inLong3 As Long, inLong4 As Long
Dim inString As String, inString2 As String
Dim inFloat As Single, inFloat2 As Single
Dim inDouble As Double, inDouble2 As Double
Dim inVariant, inVariant2
Dim inAr, inAr2
Dim inDate As Date,inDate2 As Date, outDate As Date
Dim inCurrency As Currency, inCurrency2 As Currency, outCurrency As Currency
Dim inSCode As New com.sun.star.bridge.oleautomation.SCode
Dim inSCode2 As New com.sun.star.bridge.oleautomation.SCode
Dim inDecimal As Variant, inDecimal2 As Variant, outDecimal As Variant
Dim inrefDecimal As Variant, outrefDecimal As Variant
Dim outSCode As New com.sun.star.bridge.oleautomation.SCode
Dim outByte As Integer
Dim outShort As Integer
Dim outLong, outLong2 As Long
Dim outString As String
Dim outFloat As Single
Dim outDouble As Double
Dim outVariant
'bug #109936 causes an errOr when outObject is used As out param
Dim inObject As Object, inObject2 As Object, outObject As Object
Dim objNOTHING As Object
Dim inUnknown As Object, inUnknown2 As Object, outUnknown As Object

Dim inArray, outArray, outArray2
Dim len1, len2
Dim arString(1) As String
arString(0)= "String one"
arString(1)= "String two"

Dim factory As Object
factory= createUnoService("com.sun.star.bridge.OleObjectFactory")
Dim obj As Object
obj= factory.createInstance("AxTestComponents.Basic")

Dim objFoo As Object
objFoo = factory.createInstance("AxTestComponents.Foo")


'in parameter -------------------------------------------------------------------
inBool = true
inByte = 10
inShort = 11
inLong = 111
inString = "Hello World"
inFloat = 3.14
inDouble = 3.145
inVariant = "bla"
inDate = NOW()
inCurrency = 12345.6789
inSCode.Value = &h80020004
inDecimal = CDec("-9223372036854775808") 'lowest int64

obj.inBool(inBool)
obj.inByte(inByte)
obj.inShort(inShort)
obj.inLong(inLong)
obj.inString(inString)
obj.inFloat(inFloat)
obj.inDouble(inDouble)
obj.inVariant(inVariant)
'obj.prpString= "a string property"
obj.inObject(obj)
obj.inArray(arString())
obj.inDate(inDate)
obj.inCurrency(inCurrency)
obj.inSCode(inSCode)
obj.inUnknown(objFoo)
obj.inDecimal(inDecimal)

'out parameter -------------------------------------------------------------------------
outBool = false
obj.outBool(outBool)
outByte = 0
obj.outByte(outByte)
outShort = 0
obj.outShort(outShort)
outLong = 0
obj.outLong(outLong)
outFloat = 0
obj.outFloat(outFloat)
outDouble = 0
obj.outDouble(outDouble)
outString = ""
obj.outString(outString)
outVariant = 0
obj.outVariant(outVariant)
outObject  = NOTHING
obj.outObject(outObject)
outArray = 0
obj.outArray(outArray)
obj.outDate(outDate)
obj.outCurrency(outCurrency)
obj.outSCode(outSCode)
obj.outUnknown(outUnknown)
obj.outDecimal(outDecimal)


If inBool <> outBool Or inByte <> outByte Or inShort <> outShort Or inLong <> outLong Or _
   inFloat <> outFloat Or inDouble <> outDouble Or inString <> outString Or _
   inVariant <> outVariant Or NOT equalUnoObjects(obj, outObject) Or NOT _
   equalArrays(arString(), outArray()) Or inDate <> outDate Or inCurrency <> outCurrency Or _
   inSCode.Value <> outSCode.Value Or Not equalUnoObjects(objFoo, outUnknown) Or _
   inDecimal <> outDecimal Then
   runtest = -1
   exit Function
End If


'in-out parameter -------------------------------------------------------------
'implementation of inout methods returns the previously set value in out param
inBool = true
inBool2 = inBool
obj.inoutBool(inBool2)
outBool = false
obj.inoutBool(outBool)
inByte = 10
inByte2 = inByte
obj.inoutByte(inByte2)
outByte = 0
obj.inoutByte(outByte)
inShort =  10
inShort2 = inShort
obj.inShort(inShort2)
outShort = 0
obj.inoutShort(outShort)
inLong = 10
inLong2 = inLong
obj.inoutLong(inLong2)
outLong = 0
obj.inoutLong(outLong)
inFloat = 3.14
inFloat2 = inFloat
obj.inoutFloat(inFloat2)
outFloat = 0
obj.inoutFloat(outFloat)
inDouble= 3.14
inDouble2 = inDouble
obj.inoutDouble(inDouble2)
outDouble = 0
obj.inoutDouble(outDouble)
inString = "in"
inString2 = inString
obj.inoutString(inString2)
outString = ""
obj.inoutString(outString)
inVariant = "in"
inVariant2 = inVariant
obj.inoutVariant(inVariant2)
outVariant = 0
obj.inoutVariant(outVariant)
inObject = factory.createInstance("AxTestComponents.Basic")
inObject2 = inObject
obj.inoutObject(inObject2)
outObject = NOTHING
obj.inoutObject(outObject)
inAr = arString()
inAr2 = inAr
obj.inoutArray(inAr2)
outArray = 0
obj.outArray(outArray())
inDate = NOW()
inDate2 = inDate
obj.inoutDate(inDate2)
outDate = 0
obj.inoutDate(outDate)
inCurrency = 1234.5678
inCurrency2 = inCurrency
obj.inoutCurrency(inCurrency2)
outCurrency = 0
obj.inoutCurrency(outCurrency)
inSCode.Value = &h80020004
inSCode2 = inSCode
obj.inoutSCode(inSCode2)
outSCode.Value = 0
obj.inoutSCode(outSCode)
inUnknown = objFoo
inUnknown2 = inUnknown
obj.inoutUnknown(inUnknown2)
outUnknown = Nothing
obj.inoutUnknown(outUnknown)
inDecimal = CDec("18446744073709551615") 'highest positiv value of unsigne int64
inDecimal2 = inDecimal
obj.inoutDecimal(inDecimal2)
outDecimal = 0
obj.inoutDecimal(outDecimal)

If inBool <> outBool Or inByte <> outByte Or inShort <> outShort Or inLong <> outLong Or _
   inFloat <> outFloat Or inDouble <> outDouble Or inString <> outString Or _
   inVariant <> outVariant Or NOT equalUnoObjects(inObject, outObject) Or _
   NOT equalArrays(inAr, outArray) Or inDate <> outDate Or inCurrency <> outCurrency Or _
   inSCode.Value <> outSCode.Value  Or Not equalUnoObjects(inUnknown, outUnknown) Or _
   inDecimal <> outDecimal Then
  runtest = -1
  Exit Function
End If

'properties -------------------------------------------------------------------------
inBool = false
outBool = true
obj.prpBool = inBool
outBool = obj.prpBool
inByte = 11
outByte = 0
obj.prpByte = inByte
outByte= obj.prpByte
inShort = 127
outShort = 0
obj.prpShort= inShort
outShort= obj.prpShort
inLong = 1000
outLong = 0
obj.prpLong = inLong
outLong= obj.prpLong
inFloat = 3.14
outFloat = 0
obj.prpFloat = inFloat
outFloat= obj.prpFloat
inDouble = 3.123
outDouble = 0
obj.prpDouble = inDouble
outDouble= obj.prpDouble
inString = "bla"
outString = ""
obj.prpString = inString
outString = obj.prpString
inObject = obj
outObject = objNOTHING
obj.prpObject = inObject
outObject = obj.prpObject
inVariant = "bla"
outVariant = 0
obj.prpVariant = inVariant
outVariant= obj.prpVariant
inArray = arString()
outArray = 0
obj.prpArray = inArray()
outArray= obj.prpArray
inDate = NOW()
outDate = 0
obj.prpDate = inDate
outDate = obj.prpDate
inCurrency = 1234.5678
outCurrency = 0
obj.prpCurrency = inCurrency
outCurrency = obj.prpCurrency
inSCode.Value = &h80020004
outSCode.Value = 0
obj.prpSCode = inSCode
outSCode = obj.prpSCode
inUnknown = objFoo
outUnknown= Nothing
obj.prpUnknown = inUnknown
outUnknown = obj.prpUnknown
inDecimal = CDec("18446744073709551615")' highest unsigned int64
outDecimal = 0
obj.prpDecimal = inDecimal
outDecimal = obj.prpDecimal

If inBool <> outBool Or inByte <> outByte Or inShort <> outShort Or inLong <> outLong Or _
   inFloat <> outFloat Or inDouble <> outDouble Or inString <> outString Or _
   inVariant <> outVariant Or NOT equalUnoObjects(inObject, outObject) Or _
   NOT equalArrays(inArray, outArray) Or inDate <> outDate Or inCurrency <> outCurrency Or _
   inSCode.Value <> outSCode.Value Or Not equalUnoObjects(inUnknown, outUnknown) Or _
   inDecimal <> outDecimal Then
  runtest = -1
  Exit Function
End If

' ref parameter ------------------------------------------------------------------------
obj.inLong(0)
inLong = 123
outLong = 0
obj.inLong(0)
obj.inrefLong(inLong)
obj.outLong(outLong)
inVariant = "bla"
outVariant = 0
obj.inVariant(0)
obj.inrefVariant(inVariant)
obj.outVariant(outVariant)
If inLong <> outLong Or inVariant <> outVariant Then
  runtest = -1
  Exit Function
End If

outLong = 0
obj.prprefLong = inLong
outLong = obj.prprefLong
outVariant = 0
obj.prprefVariant = inVariant
outVariant = obj.prprefVariant
If inLong <> outLong Or  inVariant <> outVariant Then
  runtest = -1
  Exit Function
End If


'vararg --------------------------------------------------------------------------------
inLong=1
inLong2 = 2
inLong3 = 3
obj.varargfunc1(inLong)
outArray = 0
outLong = 0
obj.varargfunc2(outLong, outArray)
If inLong <> outLong Then
  runtest = -1
  Exit Function
End If
len1 = UBound(outArray)  - LBound(outArray) +1
If len1 <> 0 Then
  runtest = -1
  Exit Function
End If
outArray = 0
obj.varargfunc1(inLong, inLong2, inLong3)
obj.varargfunc2(outLong, outArray)
len1 = UBound(outArray) - LBound(outArray) +1
If len1 <> 2 Or outArray(0) <> inLong2 Or outArray(1) <> inLong3 Then
  runtest = -1
  Exit Function
End If


'defaultvalue ---------------------------------------------------------------------------
inLong = 0
inFloat = 0
inVariant = 0
inVariant2 = 0
'defaults are: 1, 2, 4
'The third parameter is a VARIANT with a default value of 4. COM gives it the type BSTR
obj.defaultvalue1()
obj.defaultvalue2(inLong, inFloat, inVariant)
If inLong <> 1 Or inFloat <> 2 Or inVariant <> "4" Then
  runtest = -1
  Exit Function
End If
inLong = 10
inFloat = 11
inLong2 = inLong
inFloat2 = inFloat
inVariant = 0
inVariant = 0
obj.defaultvalue1(inLong, inFloat)
obj.defaultvalue2(inLong, inFloat, inVariant)
If inLong <> inLong2 Or inFloat <> inFloat2 Or inVariant <> "4" Then
  runtest = -1
  Exit Function
End If

'optional parameters ----------------------------------------------------------------
inLong = 100
outLong = 0
obj.optional1(inLong)
obj.optional2(outLong)
If inLong <> outLong Then
 runtest = -1
 Exit Function
End If

inLong2 = 101
outLong2 = 0
obj.optional1(inLong, inLong2)
obj.optional2(outLong, outLong2)
If inLong <> outLong AND inLong2 <> outLong2 Then
 runtest = -1
 Exit Function
End If

inLong2 = 101
outLong2 = 0
obj.optional1(inLong, inLong2)
obj.optional1(inLong)
obj.optional2(outLong, outLong2)
If inLong <> outLong AND inLong2 <> outLong2 Then
 runtest = -1
 Exit Function
End If

inLong = 10
inLong2 = 100
outLong = 5
outLong2 = 6
obj.optional3()
obj.optional3(inLong, inLong2)
obj.optional4(outLong, outLong2) 'outLong = 10, outLong2 = 100
If inLong <> outLong AND inLong2 <> outLong2 Then
 runtest = -1
 Exit Function
End If
inLong = 10
inLong2 = 100
inLong3 = inLong
inLong4 = inLong2
obj.optional4(inLong, inLong)
outLong = 0
outLong2 = 0
obj.optional5(outLong, outLong2)
If inLong3 <> outLong AND inLong4 <> outLong2 Then
  runtest = -1
  Exit Function
End If

inLong = 10
outLong = 5
obj.optional3(inLong)
obj.optional4(outLong)
If inLong <> outLong Then
 runtest = -1
 Exit Function
End If
inLong = 10
inLong2 = inLong
outLong = 0
obj.optional4(inLong)
obj.optional5(outLong)
If inLong2 <> outLong Then
  runtest = -1
  Exit Function
End If

'named arguments-------------------------------------------------------------------------
'all args As named args, different order
obj.optional6(0, 0, 0, 0)
inLong = 1
inLong2 = 2
inLong3 = 3
inLong4 = 4
obj.optional6(val4:= inLong4, val3:=inLong3, val2:=inLong2, val1:= inLong)
Dim outLong3 As Long
Dim outLong4 As Long
outLong = 0
outLong2 = 0
outLong3 = 0
outLong4 = 0
obj.optional7(outLong, outLong2, outLong3, outLong4)
If inLong <> outLong Or inLong2 <> outLong2 _
	Or inLong3 <> outLong3 Or inLong4 <> outLong4 Then
	runtest = -1
	Exit Function
End If

'mixed positional and named args with omitted args
Dim scode_paramNotFound As New com.sun.star.bridge.oleautomation.SCode
scode_paramNotFound.Value = &h80020004

obj.optional6(0, 0, 0, 0)
'val1 and val3 will be DISP_E_PARAMNOTFOUND
obj.optional6(, inLong2, val4:=inLong4)
Dim outSCode1, outSCode2
obj.optional7(outSCode, outLong2, outSCode2, outLong4)
If outSCode.Value <> scode_paramNotFound.Value Or inLong2 <> outLong2 _
	Or outSCode2.Value <> scode_paramNotFound.Value Or inLong4 <> outLong4 Then
	runtest = -1
	Exit Function
End If

'mixed positional and named args with ommitted args as out -args
inLong = 1
inLong2 = 2
inLong3 = 3
inLong4 = 4
obj.optional6(inLong, inLong2, inLong3, inLong4)
outLong2 = 0
outLong3 = 0
obj.optional7(,outLong2, val3:= outLong3)
If inLong2 <> outLong2 Or inLong3 <> outLong3 Then
	runtest = -1
	Exit Function
End If

'test properties with additional arguments ------------------------------------
inLong = 10
inLong2 = 20
inLong3 = 30
outLong = 0
outLong2 = 0
outLong3 = 0
obj.prpMultiArg1(0,0) = 0
'obj.prpMultiArg1 = 0
obj.prpMultiArg1(inLong,inLong2) = inLong3
outLong3 = obj.prpMultiArg1(outLong, outLong2)
If outLong <> 10 Or outLong2 <> 02 Or outLong3 <> 30 Then
	runtest = -1
	Exit Function
End If

outLong = 0
outLong2 = 0
obj.prpMultiArg1(0,0) = 0
obj.prpMultiArg1(1) = 3
outLong2 = obj.prpMultiArg1(outLong)
If outLong <> 1 Or outLong2 <> 3 Then
	runtest = -1
	Exit Function
End If

outLong = 0
outLong2 = 0
obj.prpMultiArg1(0,0) = 0
obj.prpMultiArg1(val2:= 1) = 3
outLong2 = obj.prpMultiArg1(val2:=outLong)
If outLong <> 1 Or outLong2 <> 3 Then
	runtest = -1
	Exit Function
End If

outLong = -1
outLong2 = -1
obj.prpMultiArg2(0) = 0
outLong = obj.prpMultiArg2GetValues(outLong, outLong2)
If outLong <> 0 Or outLong2 <> 0 Then
	runtest = -1
	Exit Function
End If


outLong = 0
outLong2 = 0
obj.prpMultiArg2(1) = 2
obj.prpMultiArg2GetValues(outLong, outLong2)
If outLong <> 1 Or outLong2 <> 2 Then
	runtest = -1
	Exit Function
End If



' other tests ------------------------------------------------------------------
obj.inObject(NOTHING)
outObject = NOTHING
'bridge should return an XInterface any with null pointer
'A basic errOr should occur if this is not the case
obj.outObject(outObject)

If Not IsNull(outObject) Then
	runtest = -1
	Exit Function
End If
'Decimal passed by reference
inrefDecimal = CDec("9223372036854775807") 'highest positiv value of int64
obj.inrefDecimal(inrefDecimal)
outrefDecimal = 0
obj.outDecimal(outrefDecimal)
If inrefDecimal <> outrefDecimal Then
	runtest = -1
	Exit Function
End If

' Test Automation object with dual interfaces ------------------------------------
dim dispatcher as object
dim oExplorer as object
dispatcher = createUnoService("com.sun.star.bridge.OleObjectFactory")
oExplorer = dispatcher.createInstance("InternetExplorer.Application")
If Not IsNull(oExplorer) Then
	oExplorer.visible = true
	oExplorer.Navigate2("http://www.openoffice.org")
Else
	MsgBox("Could not perform test with Internet Explorer!")
End If


End Function

'One dimensional arrays with simple types.
'lower bound must be 0
Function equalArrays(ar1, ar2)
Dim len1
Dim len2
len1 = UBound(ar1) - LBound(ar1) + 1
len2 = UBound(ar2) - LBound(ar2) + 1
If len1 <> len2 Then
  equalArrays = false
  Exit Function
End If
Dim counter
FOr counter = 0 To len1 - 1
  If ar1(counter) <> ar2(counter) Then
    equalArrays = false
    Exit Function
  End If
Next
equalArrays = true
End Function
