'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'
' This file incorporates work covered by the following license notice:
'
'   Licensed to the Apache Software Foundation (ASF) under one or more
'   contributor license agreements. See the NOTICE file distributed
'   with this work for additional information regarding copyright
'   ownership. The ASF licenses this file to you under the Apache
'   License, Version 2.0 (the "License"); you may not use this file
'   except in compliance with the License. You may obtain a copy of
'   the License at http://www.apache.org/licenses/LICENSE-2.0 .
'

Option Strict Off
Option Explicit On
Module Module1

Private objServiceManager As Object
Private objCoreReflection As Object
Private objOleTest As Object
Private objEventListener As Object
'General counter
Dim i As Integer
Dim j As Integer
Dim sError As String
Dim outHyper, inHyper, retHyper As Object

Public Sub Main()
        objServiceManager = CreateObject("com.sun.star.ServiceManager")
        objCoreReflection = objServiceManager.createInstance("com.sun.star.reflection.CoreReflection")
        ' extensions/test/ole/cpnt
        objOleTest = objServiceManager.createInstance("oletest.OleTest")
        ' extensions/test/ole/EventListenerSample/VBEventListener
        objEventListener = CreateObject("VBasicEventListener.VBEventListener")
        Debug.Print(TypeName(objOleTest))


        testBasics()
        testHyper()
        testAny()
        testObjects()
        testGetStruct()
        ''dispose not working i103353
        'testImplementedInterfaces()
        testGetValueObject()
        testArrays()
        testProps()

    End Sub
	Function testProps() As Object

		Dim aToolbarItemProp1 As Object
        aToolbarItemProp1 = objOleTest.Bridge_GetStruct("com.sun.star.beans.PropertyValue")
		Dim aToolbarItemProp2 As Object
        aToolbarItemProp2 = objOleTest.Bridge_GetStruct("com.sun.star.beans.PropertyValue")
		Dim aToolbarItemProp3 As Object
        aToolbarItemProp3 = objOleTest.Bridge_GetStruct("com.sun.star.beans.PropertyValue")
		Dim properties(2) As Object

        aToolbarItemProp1.Name = "CommandURL"
        aToolbarItemProp1.Value = "macro:///standard.module1.TestIt"
        aToolbarItemProp2.Name = "Label"
        aToolbarItemProp2.Value = "Test"
        aToolbarItemProp3.Name = "Type"
        aToolbarItemProp3.Value = 0

		properties(0) = aToolbarItemProp1
		properties(1) = aToolbarItemProp2
		properties(2) = aToolbarItemProp3


        Dim dummy(-1) As Object
        
        Dim Desktop As Object
        Desktop = objServiceManager.createInstance("com.sun.star.frame.Desktop")
		Dim Doc As Object
        Doc = Desktop.loadComponentFromURL("private:factory/swriter", "_blank", 2, dummy)
		Dim LayoutManager As Object
        LayoutManager = Doc.currentController.Frame.LayoutManager

        LayoutManager.createElement("private:resource/toolbar/user_toolbar1")
        LayoutManager.showElement("private:resource/toolbar/user_toolbar1")
		Dim ToolBar As Object
        ToolBar = LayoutManager.getElement("private:resource/toolbar/user_toolbar1")
		Dim settings As Object
        settings = ToolBar.getSettings(True)

		'the changes are here:
		Dim aany As Object
        aany = objServiceManager.Bridge_GetValueObject()
        Call aany.Set("[]com.sun.star.beans.PropertyValue", properties)
        Call settings.insertByIndex(0, aany)
        Call ToolBar.setSettings(settings)


	End Function


	Function testBasics() As Object
		' In Parameter, simple types
		'============================================
		Dim tmpVar As Object
		Dim ret As Object
		Dim outByte, inByte, retByte As Byte
		Dim outBool, inBool, retBool As Boolean
		Dim outShort, inShort, retShort As Short
		Dim outUShort, inUShort, retUShort As Short
		Dim outLong, inLong, retLong As Integer
		Dim outULong, inULong, retULong As Integer
		Dim outHyper, inHyper, retHyper As Object
		Dim outUHyper, inUHyper, retUHyper As Object
		Dim outFloat, inFloat, retFloat As Single
		Dim outDouble, inDouble, retDouble As Double
		Dim outString, inString, retString As String
		Dim retChar, inChar, outChar, retChar2 As Short
		Dim outCharAsString, inCharAsString, retCharAsString As String
		Dim outAny, inAny, retAny As Object
		Dim outType, inType, retType As Object
		Dim outXInterface, inXInterface, retXInterface As Object
		Dim outXInterface2, inXInterface2, retXInterface2 As Object


		Dim outVarByte As Object
		Dim outVarBool As Object
		Dim outVarShort As Object
		Dim outVarUShort As Object
		Dim outVarLong As Object
		Dim outVarULong As Object
		Dim outVarFloat As Object
		Dim outVarDouble As Object
		Dim outVarString As Object
		Dim outVarChar As Object
		Dim outVarAny As Object
		Dim outVarType As Object

		inByte = 10
		inBool = True
		inShort = -10
		inUShort = -100
		inLong = -1000
        inHyper = CDec("-9223372036854775808") 'lowest int64
        inUHyper = CDec("18446744073709551615") ' highest unsigned int64
		inULong = 10000
		inFloat = 3.14
		inDouble = 3.14
		inString = "Hello World!"
		inChar = 65
		inCharAsString = "A"
        inAny = "Hello World"
        inType = objServiceManager.Bridge_CreateType("[]long")
		inXInterface = objCoreReflection
		inXInterface2 = objEventListener

        retByte = objOleTest.in_methodByte(inByte)
        retBool = objOleTest.in_methodBool(inBool)
        retShort = objOleTest.in_methodShort(inShort)
        retUShort = objOleTest.in_methodUShort(inUShort)
        retLong = objOleTest.in_methodLong(inLong)
        retULong = objOleTest.in_methodULong(inULong)
        retHyper = objOleTest.in_methodHyper(inHyper)
        retUHyper = objOleTest.in_methodUHyper(inUHyper)
        retFloat = objOleTest.in_methodFloat(inFloat)
        retDouble = objOleTest.in_methodDouble(inDouble)
        retString = objOleTest.in_methodString(inString)
        retChar = objOleTest.in_methodChar(inChar)
        retChar2 = objOleTest.in_methodChar(inCharAsString)
        retAny = objOleTest.in_methodAny(inAny)
        retType = objOleTest.in_methodType(inType)
        retXInterface = objOleTest.in_methodXInterface(inXInterface) ' UNO object
        retXInterface2 = objOleTest.in_methodXInterface(inXInterface2)

        If retByte <> inByte Or retBool <> inBool Or retShort <> inShort Or retUShort <> inUShort _
            Or retLong <> inLong Or retULong <> inULong Or retHyper <> inHyper _
            Or retUHyper <> inUHyper Or retFloat <> inFloat Or retDouble <> inDouble _
            Or retString <> inString Or retChar <> inChar Or retChar2 <> Asc(inCharAsString) _
            Or retAny <> inAny Or Not (retType.Name = inType.Name) _
            Or inXInterface IsNot retXInterface Or inXInterface2 IsNot retXInterface2 Then
            sError = "in - parameter and return value test failed"
            MsgBox(sError)

        End If
 
        'Out Parameter simple types
        '================================================


        objOleTest.testout_methodByte(outByte)
        objOleTest.testout_methodFloat(outFloat)
        objOleTest.testout_methodDouble(outDouble)
        objOleTest.testout_methodBool(outBool)
        objOleTest.testout_methodShort(outShort)
        objOleTest.testout_methodUShort(outUShort)
        objOleTest.testout_methodLong(outLong)
        objOleTest.testout_methodULong(outULong)
        objOleTest.testout_methodHyper(outHyper)
        objOleTest.testout_methodUHyper(outUHyper)
        objOleTest.testout_methodString(outString)
        objOleTest.testout_methodChar(outChar)
        'outCharAsString is a string. Therefore the returned sal_Unicode value of 65 will be converted
        'to a string "65"
        objOleTest.testout_methodChar(outCharAsString)
        objOleTest.testout_methodAny(outAny)
        objOleTest.testout_methodType(outType)
        'objOleTest.in_methodXInterface (inXInterface) ' UNO object
        Call objOleTest.in_methodXInterface(inXInterface) ' UNO object
        objOleTest.testout_methodXInterface(outXInterface)
        Call objOleTest.in_methodXInterface(inXInterface2) ' COM object
        objOleTest.testout_methodXInterface(outXInterface2)

        If outByte <> inByte Or outFloat <> inFloat Or outDouble <> inDouble _
            Or outBool <> inBool Or outShort <> inShort Or outUShort <> inUShort _
            Or outLong <> inLong Or outULong <> inULong Or outHyper <> inHyper _
            Or outUHyper <> inUHyper Or outString <> inString Or outChar <> inChar _
            Or Not (outCharAsString = "65") Or outAny <> inAny _
            Or Not (outType.Name = inType.Name) Or inXInterface IsNot outXInterface _
            Or inXInterface2 IsNot outXInterface2 Then

            sError = "out - parameter test failed!"
            MsgBox(sError)
        End If

        'Out Parameter simple types (VARIANT var)
        '====================================================
        objOleTest.testout_methodByte(outVarByte)
        objOleTest.testout_methodBool(outVarBool)
        objOleTest.testout_methodChar(outVarChar)
        objOleTest.testout_methodShort(outVarShort)
        objOleTest.testout_methodUShort(outVarUShort)
        objOleTest.testout_methodLong(outVarLong)
        objOleTest.testout_methodULong(outVarULong)
        objOleTest.testout_methodString(outVarString)
        objOleTest.testout_methodFloat(outVarFloat)
        objOleTest.testout_methodDouble(outVarDouble)
        objOleTest.testout_methodAny(outVarAny)
        objOleTest.testout_methodType(outVarType)

        If outVarByte <> inByte Or outVarBool <> inBool Or outVarChar <> inChar _
            Or outVarShort <> inShort Or outVarUShort <> inUShort _
            Or outVarLong <> inLong Or outVarULong <> inULong Or outVarString <> inString _
            Or outVarFloat <> inFloat Or outVarDouble <> inDouble Or outVarAny <> inAny _
            Or Not (outVarType.Name = inType.Name) Then
            sError = "out - parameter (VARIANT) test failed!"
            MsgBox(sError)
        End If

        'In/Out simple types
        '============================================
        objOleTest.in_methodByte(0)
        objOleTest.in_methodBool(False)
        objOleTest.in_methodShort(0)
        objOleTest.in_methodUShort(0)
        objOleTest.in_methodLong(0)
        objOleTest.in_methodULong(0)
        objOleTest.in_methodHyper(0)
        objOleTest.in_methodUHyper(0)
        objOleTest.in_methodFloat(0)
        objOleTest.in_methodDouble(0)
        objOleTest.in_methodString(0)
        objOleTest.in_methodChar(0)
        objOleTest.in_methodAny(0)
        objOleTest.in_methodType(objServiceManager.Bridge_CreateType("boolean"))
        outXInterface = Nothing
        Call objOleTest.in_methodXInterface(outXInterface)

        outByte = 10
        retByte = outByte
        objOleTest.testinout_methodByte(retByte)
        objOleTest.testinout_methodByte(retByte)
        outBool = True
        retBool = outBool
        objOleTest.testinout_methodBool(retBool)
        objOleTest.testinout_methodBool(retBool)
        outShort = 10
        retShort = outShort
        objOleTest.testinout_methodShort(retShort)
        objOleTest.testinout_methodShort(retShort)
        outUShort = 20
        retUShort = outUShort
        objOleTest.testinout_methodUShort(retUShort)
        objOleTest.testinout_methodUShort(retUShort)
        outLong = 30
        retLong = outLong
        objOleTest.testinout_methodLong(retLong)
        objOleTest.testinout_methodLong(retLong)
        outULong = 40
        retULong = outULong
        objOleTest.testinout_methodULong(retLong)
        objOleTest.testinout_methodULong(retLong)
        outHyper = CDec("9223372036854775807") 'highest positiv value of int64
        retHyper = outHyper
        objOleTest.testinout_methodHyper(retHyper)
        objOleTest.testinout_methodHyper(retHyper)
        outUHyper = CDec("18446744073709551615") 'highest value of unsigned int64
        retUHyper = outUHyper
        objOleTest.testinout_methodUHyper(retUHyper)
        objOleTest.testinout_methodUHyper(retUHyper)
        outFloat = 3.14
        retFloat = outFloat
        objOleTest.testinout_methodFloat(retFloat)
        objOleTest.testinout_methodFloat(retFloat)
        outDouble = 4.14
        retDouble = outDouble
        objOleTest.testinout_methodDouble(retDouble)
        objOleTest.testinout_methodDouble(retDouble)
        outString = "Hello World!"
        retString = outString
        objOleTest.testinout_methodString(retString)
        objOleTest.testinout_methodString(retString)
        outChar = 66
        retChar = outChar
        objOleTest.testinout_methodChar(retChar)
        objOleTest.testinout_methodChar(retChar)
        outCharAsString = "H"
        retCharAsString = outCharAsString
        objOleTest.testinout_methodChar(retCharAsString)
        objOleTest.testinout_methodChar(retCharAsString)
        outAny = "Hello World 2!"
        retAny = outAny
        objOleTest.testinout_methodAny(retAny)
        objOleTest.testinout_methodAny(retAny)
        outType = objServiceManager.Bridge_CreateType("long")
        retType = outType
        objOleTest.testinout_methodType(retType)
        objOleTest.testinout_methodType(retType)

        outXInterface = objCoreReflection
        retXInterface = outXInterface
        objOleTest.testinout_methodXInterface2(retXInterface)

        If outByte <> retByte Or outBool <> retBool Or outShort <> retShort _
            Or outUShort <> retUShort Or outLong <> retLong Or outULong <> retULong _
            Or outHyper <> retHyper Or outUHyper <> outUHyper _
            Or outFloat <> retFloat Or outDouble <> retDouble _
            Or outString <> retString Or outChar <> retChar _
            Or outCharAsString <> retCharAsString _
            Or outAny <> retAny Or Not (outType.Name = retType.Name) _
            Or outXInterface IsNot retXInterface Then
            sError = "in/out - parameter test failed!"
            MsgBox(sError)
        End If

        'Attributes
        objOleTest.AByte = inByte
        retByte = 0
        retByte = objOleTest.AByte
        objOleTest.AFloat = inFloat
        retFloat = 0
        retFloat = objOleTest.AFloat
        objOleTest.AType = inType
        retType = Nothing

        retType = objOleTest.AType

        If inByte <> retByte Or inFloat <> retFloat Or Not (inType.Name = retType.Name) Then
            sError = "Attributes - test failed!"
            MsgBox(sError)
        End If

    End Function
	Function testHyper() As Object

		'======================================================================
		' Other Hyper tests
		Dim emptyVar As Object
		Dim retAny As Object

		retAny = emptyVar
		inHyper = CDec("9223372036854775807") 'highest positive value of int64
		retAny = objOleTest.in_methodAny(inHyper)
		sError = "hyper test failed"
		If inHyper <> retAny Then
			MsgBox(sError)
		End If
		inHyper = CDec("-9223372036854775808") 'lowest negative value of int64
		retAny = objOleTest.in_methodAny(inHyper)

		If inHyper <> retAny Then
			MsgBox(sError)
		End If
		inHyper = CDec("18446744073709551615") 'highest positive value of unsigned int64
		retAny = objOleTest.in_methodAny(inHyper)

		If inHyper <> retAny Then
			MsgBox(sError)
		End If
		inHyper = CDec(-1)
		retAny = objOleTest.in_methodAny(inHyper)
		If inHyper <> retAny Then
			MsgBox(sError)
		End If
		inHyper = CDec(0)
		retAny = objOleTest.in_methodAny(inHyper)
		If inHyper <> retAny Then
			MsgBox(sError)
		End If

		'==============================================================================


	End Function
	Function testAny() As Object
		Dim outVAr As Object

		'Any test. We pass in an any as value object. If it is not correct converted
		'then the target component throws a RuntimeException
		Dim lengthInAny As Integer

		lengthInAny = 10
		Dim seqLongInAny(10) As Integer
		For i = 0 To lengthInAny - 1
			seqLongInAny(i) = i + 10
		Next 
		Dim anySeqLong As Object
		anySeqLong = objOleTest.Bridge_GetValueObject()
		anySeqLong.Set("[]long", seqLongInAny)
		Dim anySeqRet As Object
		Err.Clear()
		On Error Resume Next
		anySeqRet = objOleTest.other_methodAny(anySeqLong, "[]long")

		If Err.Number <> 0 Then
			MsgBox("error")
		End If
	End Function

	Function testObjects() As Object
		' COM obj
		Dim outVAr As Object
		Dim retObj As Object
		'OleTest receives a COM object that implements XEventListener
		'OleTest then calls a disposing on the object. The object then will be
		'asked if it has been called
		objEventListener.setQuiet(True)
		objEventListener.resetDisposing()
		retObj = objOleTest.in_methodInvocation(objEventListener)
		Dim ret As Object
		ret = objEventListener.disposingCalled
		If ret = False Then
			MsgBox("Error")
		End If

		'The returned object should be objEventListener, test it by calling disposing
		' takes an IDispatch as Param ( EventObject).To provide a TypeMismatch
		'we put in another IDispatch
		retObj.resetDisposing()
		retObj.disposing(objEventListener)
		If retObj.disposingCalled = False Then
			MsgBox("Error")
		End If

		' out param gives out the OleTestComponent
		'objOleTest.testout_methodXInterface retObj
		'outVAr = Null
		'retObj.testout_methodAny outVAr
		'Debug.Print "test out Interface " & CStr(outVAr)
		'If outVAr <> "I am a string in an any" Then
		'    MsgBox "error"
		'End If


		'in out
		' in: UNO object, the same is expected as out param
		' the function expects OleTest as parameter and sets a value

		Dim myAny As Object



		Dim objOleTest2 As Object
		objOleTest2 = objServiceManager.createInstance("oletest.OleTest")
		'Set a value
		objOleTest2.AttrAny2 = "VBString "

		'testinout_methodXInterfaces substitutes the argument with the object set in in_methodXInterface
		objOleTest.AttrAny2 = "VBString  this string was written in the UNO component to the inout pararmeter"
		objOleTest.in_methodXInterface(objOleTest)
		objOleTest.testinout_methodXInterface2(objOleTest2)
		Dim tmpVar As Object
		tmpVar = System.DBNull.Value
		tmpVar = objOleTest2.AttrAny2
		Debug.Print("in: Uno out: the same object // " & CStr(tmpVar))
		If tmpVar <> "VBString  this string was written in the UNO component to the inout pararmeter" Then
			MsgBox("error")
		End If


		'create a struct
		Dim structClass As Object
		structClass = objCoreReflection.forName("oletest.SimpleStruct")
		Dim structInstance As Object
		structClass.CreateObject(structInstance)
		structInstance.message = "Now we are in VB"
		Debug.Print("struct out " & structInstance.message)
		If structInstance.message <> "Now we are in VB" Then
			MsgBox("error")
		End If

		'put the struct into OleTest. The same struct will be returned with an added String
		Dim structRet As Object
		structRet = objOleTest.in_methodStruct(structInstance)
		Debug.Print("struct in - return " & structRet.message)
		If structRet.message <> "Now we are in VBThis string was set in OleTest" Then
			MsgBox("error")
		End If


	End Function
	Function testGetStruct() As Object
		'Bridge_GetStruct
		'========================================================
		Dim objDocument As Object
        objDocument = createHiddenDocument()
        'dispose not working i103353
        'objDocument.dispose()
        objDocument.close(True)
	End Function

	Function testImplementedInterfaces() As Object
		'Bridge_ImplementedInterfaces
		'=================================================
		' call an UNO function that takes an XEventListener interface
		'We provide a COM implementation (IDispatch) as EventListener
		'Open a new empty writer document

		Dim objDocument As Object
		objDocument = createHiddenDocument()
		objEventListener.resetDisposing()
		objDocument.addEventListener(objEventListener)
		objDocument.dispose()
		If objEventListener.disposingCalled = False Then
			MsgBox("Error")
		End If
	End Function

	Function testGetValueObject() As Object
		'Bridge_GetValueObject
		'==================================================
		Dim objVal As Object
		objVal = objOleTest.Bridge_GetValueObject()
		Dim arrByte(9) As Byte
		Dim countvar As Integer
		For countvar = 0 To 9
			arrByte(countvar) = countvar
		Next countvar

		objVal.Set("[]byte", arrByte)
		Dim ret As Object
		ret = 0
		ret = objOleTest.methodByte(objVal)
		'Test if ret is the same array

		Dim key As Object
		key = 0
		For	Each key In ret
			If ret(key) <> arrByte(key) Then
				MsgBox("Error")
			End If
			Debug.Print(ret(key))
		Next key

		Dim outByte As Byte
		outByte = 77
		Dim retByte As Byte
		retByte = outByte
		objVal.InitInOutParam("byte", retByte)
		objOleTest.testinout_methodByte(objVal)
		objVal.InitInOutParam("byte", retByte)
		objOleTest.testinout_methodByte(objVal)

		ret = 0
		ret = objVal.Get()
		Debug.Print(ret)
		If ret <> outByte Then
			MsgBox("error")
		End If

		objVal.InitOutParam()
		Dim inChar As Short
		inChar = 65
		objOleTest.in_methodChar(inChar)
		objOleTest.testout_methodChar(objVal) 'Returns 'A' (65)
		ret = 0
		ret = objVal.Get()
		Debug.Print(ret)
		If ret <> inChar Then
			MsgBox("error")
		End If

	End Function

	Function testArrays() As Object
		'Arrays
		'========================================
		Dim arrLong(2) As Integer
		Dim arrObj(2) As Object
		Dim countvar As Integer
		For countvar = 0 To 2
			arrLong(countvar) = countvar + 10
			Debug.Print(countvar)
			arrObj(countvar) = CreateObject("VBasicEventListener.VBEventListener")
			arrObj(countvar).setQuiet(True)
		Next 

		'Arrays always contain VARIANTS
		Dim seq() As Object
		seq = objOleTest.methodLong(arrLong)

		For countvar = 0 To 2
			Debug.Print(CStr(seq(countvar)))
			If arrLong(countvar) <> seq(countvar) Then
				MsgBox("error")
			End If
		Next 
		seq = objOleTest.methodXInterface(arrObj)
		Dim tmp As Object
		For countvar = 0 To 2
			seq(countvar).resetDisposing()
            seq(countvar).disposing(CObj(tmp))
			If seq(countvar).disposingCalled = False Then
				MsgBox("Error")
			End If
		Next 

		'Array containing interfaces (element type is VT_DISPATCH)
		Dim arEventListener(2) As Object
		For countvar = 0 To 2
			arEventListener(countvar) = CreateObject("VBasicEventListener.VBEventListener")
			arEventListener(countvar).setQuiet(True)
		Next 

		'The function calls disposing on the listeners
		seq = objOleTest.methodXEventListeners(arEventListener)
		Dim count As Object
		For countvar = 0 To 2
			If arEventListener(countvar).disposingCalled = False Then
				MsgBox("Error")
			End If
		Next 
		'Array containing interfaces (element type is VT_VARIANT which contains VT_DISPATCH
		Dim arEventListener2(2) As Object
		For countvar = 0 To 2
			arEventListener2(countvar) = CreateObject("VBasicEventListener.VBEventListener")
			arEventListener2(countvar).setQuiet(True)
		Next 
		seq = objOleTest.methodXEventListeners(arEventListener2)
		For countvar = 0 To 2
			If arEventListener2(countvar).disposingCalled = False Then
				MsgBox("Error")
			End If
		Next 

		'Variant containing Array containing interfaces (element type is VT_VARIANT which contains VT_DISPATCH
		Dim arEventListener3(2) As Object
		Dim var As Object
		For countvar = 0 To 2
			arEventListener3(countvar) = CreateObject("VBasicEventListener.VBEventListener")
			arEventListener3(countvar).setQuiet(True)
		Next 
		Dim varContAr As Object
		varContAr = VB6.CopyArray(arEventListener3)
		seq = objOleTest.methodXEventListeners(varContAr)
		For countvar = 0 To 2
			If arEventListener3(countvar).disposingCalled = False Then
				MsgBox("Error")
			End If
		Next 

		'Get a sequence created in UNO, out param is Variant ( VT_BYREF|VT_VARIANT)
		Dim seqX As Object

		objOleTest.testout_methodSequence(seqX)
		Dim key As Object
		For	Each key In seqX
			Debug.Print(CStr(seqX(key)))
			If seqX(key) <> key Then
				MsgBox("error")
			End If
		Next key
		'Get a sequence created in UNO, out param is array Variant ( VT_BYREF|VT_VARIANT|VT_ARRAY)
		Dim seqX2() As Object
		objOleTest.testout_methodSequence(seqX2)

		For	Each key In seqX2
			Debug.Print(CStr(seqX2(key)))
		Next key

		'pass it to UNO and get it back
		Dim seq7() As Object
		seq7 = objOleTest.methodLong(seqX)
		Dim key2 As Object
		For	Each key2 In seq7
			Debug.Print(CStr(seq7(key2)))
			If seqX2(key) <> key Then
				MsgBox("error")
			End If
		Next key2

		'array with starting index != 0
		Dim seqIndex(2) As Integer
        Dim seq8() As Object
		Dim longVal1, longVal2 As Integer
		longVal1 = 1
		longVal2 = 2
		seqIndex(1) = longVal1
        seqIndex(2) = longVal2
        'The bridge returns a Safearray of Variants. It does not yet convert to an _
        'array of a particular type!
        'Comparing of elements from seq8 (Object) with long values worked without _
        'explicit cast as is necessary in VS 2008. Also arrays in VS 2008 start at _
        'index 0
        seq8 = objOleTest.methodLong(seqIndex)
        If longVal1 <> CInt(seq8(1)) And longVal2 <> CInt(seq8(2)) Then
            MsgBox("error")
        End If

		'in out Array
		' arrLong is Long Array
		Dim inoutVar(2) As Object

		For countvar = 0 To 2
			inoutVar(countvar) = countvar + 10
		Next 

		objOleTest.testinout_methodSequence(inoutVar)

		countvar = 0
		For countvar = 0 To 2
			Debug.Print(CStr(inoutVar(countvar)))
			If inoutVar(countvar) <> countvar + 11 Then
				MsgBox("error")
			End If
		Next 

		'Multidimensional array
		'============================================================
		' Sequence< Sequence<long> > methodSequence( Sequence< Sequence long> >)
		' Real multidimensional array Array
		' 9 is Dim 1 (least significant) with C API
		Dim mulAr(9, 1) As Integer
		For i = 0 To 1
			For j = 0 To 9
				mulAr(j, i) = i * 10 + j
			Next j
		Next i

		Dim resMul As Object
		resMul = objOleTest.methodSequence(mulAr)

		Dim countDim1 As Integer
		Dim countDim2 As Integer
		Dim arr As Object
		For countDim2 = 0 To 1
			arr = resMul(countDim2)
			For countDim1 = 0 To 9
				Debug.Print(arr(countDim1))
				If arr(countDim1) <> mulAr(countDim1, countDim2) Then
					MsgBox("Error Multidimensional Array")
				End If
			Next countDim1
		Next countDim2
		IsArray(resMul)

		'Array of VARIANTs containing arrays
		Dim mulAr2(1) As Object
		Dim arr2(9) As Integer
		For i = 0 To 1
			' Dim arr(9) As Long
			For j = 0 To 9
				arr2(j) = i * 10 + j
			Next j
			mulAr2(i) = VB6.CopyArray(arr2)
		Next i

		resMul = 0
		resMul = objOleTest.methodSequence(mulAr2)
		arr = 0
		Dim tmpVar As Object
		For countDim2 = 0 To 1
			arr = resMul(countDim2)
			tmpVar = mulAr2(countDim2)
			For countDim1 = 0 To 9
				Debug.Print(arr(countDim1))
				If arr(countDim1) <> tmpVar(countDim1) Then
					MsgBox("Error Multidimensional Array")
				End If
			Next countDim1
		Next countDim2

		'Array containing interfaces (element type is VT_DISPATCH)
		Dim arArEventListener(1, 2) As Object
		For i = 0 To 1
			For j = 0 To 2
				arArEventListener(i, j) = CreateObject("VBasicEventListener.VBEventListener")
				arArEventListener(i, j).setQuiet(True)
			Next 
		Next 
		'The function calls disposing on the listeners
		seq = objOleTest.methodXEventListenersMul(arArEventListener)
		For i = 0 To 1
			For j = 0 To 2
				If arArEventListener(i, j).disposingCalled = False Then
					MsgBox("Error")
				End If
			Next 
		Next 

		'Array containing interfaces (element type is VT_VARIANT containing VT_DISPATCH)
		Dim arArEventListener2(1, 2) As Object
		For i = 0 To 1
			For j = 0 To 2
				arArEventListener2(i, j) = CreateObject("VBasicEventListener.VBEventListener")
				arArEventListener2(i, j).setQuiet(True)
			Next 
		Next 
		'The function calls disposing on the listeners
		seq = objOleTest.methodXEventListenersMul(arArEventListener2)
		For i = 0 To 1
			For j = 0 To 2
				If arArEventListener2(i, j).disposingCalled = False Then
					MsgBox("Error")
				End If
			Next 
		Next 

		' SAFEARRAY of VARIANTS containing SAFEARRAYs
		'The ultimate element type is VT_DISPATCH ( XEventListener)
		Dim arEventListener4(1) As Object
		Dim seq1(2) As Object
		Dim seq2(2) As Object
		For i = 0 To 2
			seq1(i) = CreateObject("VBasicEventListener.VBEventListener")
			seq2(i) = CreateObject("VBasicEventListener.VBEventListener")
			seq1(i).setQuiet(True)
			seq2(i).setQuiet(True)
		Next 
		arEventListener4(0) = VB6.CopyArray(seq1)
		arEventListener4(1) = VB6.CopyArray(seq2)
		'The function calls disposing on the listeners
		seq = objOleTest.methodXEventListenersMul(arEventListener4)
		For i = 0 To 2
			If seq1(i).disposingCalled = False Or seq2(i).disposingCalled = False Then
				MsgBox("Error")
			End If
		Next 

	End Function

	Function createHiddenDocument() As Object
		'Try to create a hidden document
		Dim objPropValue As Object
		objPropValue = objOleTest.Bridge_GetStruct("com.sun.star.beans.PropertyValue")
		'Set the members. If this fails then there is an Error
        objPropValue.Name = "Hidden"
		objPropValue.Handle = -1
		objPropValue.Value = True

		'create a hidden document
		'Create the Desktop
		Dim objDesktop As Object
		objDesktop = objServiceManager.createInstance("com.sun.star.frame.Desktop")
		'Open a new empty writer document
		Dim args(0) As Object
		args(0) = objPropValue
		createHiddenDocument = objDesktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, args)
	End Function
End Module
