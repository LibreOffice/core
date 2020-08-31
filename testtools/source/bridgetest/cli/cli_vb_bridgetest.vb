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

Option Explicit On
Option Strict On

imports System
imports uno
imports uno.util
imports unoidl.com.sun.star.lang
imports unoidl.com.sun.star.uno
'imports unoidl.com.sun.star.test.bridge
imports unoidl.test.testtools.bridgetest
imports System.Windows.Forms
imports System.Diagnostics
imports System.Reflection

Class CONSTANTS
Friend Shared STRING_TEST_CONSTANT As String  = """ paco\' chorizo\\\' ""\'"
End Class

Namespace foo

    Public Interface MyInterface
    End Interface
End Namespace

Namespace vb_bridetest
Class ORecursiveCall
    Inherits WeakBase
    Implements XRecursiveCall
    
    Overridable Sub callRecursivly(xCall As XRecursiveCall, nToCall As Integer) _
    Implements XRecursiveCall.callRecursivly
		SyncLock Me
            If nToCall > 0
                nToCall = nToCall - 1
                xCall.callRecursivly(Me, nToCall)
            End If
       End SyncLock    
    End Sub
End Class
        



Public Class BridgeTest
       Inherits uno.util.WeakBase
       Implements XMain

    Private m_xContext As XComponentContext

    Public Sub New( xContext As unoidl.com.sun.star.uno.XComponentContext )
        mybase.New()
        m_xContext = xContext
    End Sub

    Private Shared Function check( b As Boolean , message As String  ) As Boolean
        If Not b
            Console.WriteLine("{0} failed\n" , message)
        End If
        Return b
    End Function

    Private Shared Sub assign( rData As TestElement, bBool As Boolean, _
			aChar As Char, nByte As Byte, nShort As Short, nUShort As UInt16, _
					nLong As Integer, nULong As UInt32, nHyper As Long, _
                    nUHyper As UInt64, fFloat As Single, fDouble As Double, _
					eEnum As TestEnum, rStr As String, xTest As Object, _
					rAny As Any)

    	rData.Bool = bBool
	    rData.Char = aChar
	    rData.Byte = nByte
	    rData.Short = nShort
	    rData.UShort = nUShort
	    rData.Long = nLong
	    rData.ULong = nULong
	    rData.Hyper = nHyper
	    rData.UHyper = nUHyper
	    rData.Float = fFloat
	    rData.Double = fDouble
	    rData.Enum = eEnum
	    rData.String = rStr
	    rData.Interface = xTest
	    rData.Any = rAny
    End Sub

    Private Shared Sub assign( rData As TestDataElements, bBool As Boolean, _
            aChar As Char, nByte As Byte, nShort As Short, nUShort As UInt16, _
			nLong As Integer, nULong As UInt32, nHyper As Long, _
            nUHyper As UInt64, fFloat As Single, fDouble As Double, _
    		eEnum As TestEnum, rStr As String, xTest As Object, _
			rAny As Any, rSequence() As TestElement)

    	assign( DirectCast( rData,TestElement), _
			bBool, aChar, nByte, nShort, nUShort, nLong, nULong, nHyper, _
            nUHyper, fFloat, fDouble, eEnum, rStr, xTest, rAny )
	    rData.Sequence = rSequence
    End Sub

	Private Shared Function compareData(val1 As Object, val2 As Object) As Boolean	
		If val1 Is Nothing And val2 Is Nothing OrElse _
            val1 Is val2
			Return True
        End If
		If  val1 Is Nothing And Not(val2 Is Nothing)  OrElse _
		    Not (val1 Is Nothing) And val2 Is Nothing OrElse _
            Not val1.GetType().Equals( val2.GetType())
			Return False
		End If			

		Dim ret As Boolean = False
		Dim t1 As Type = val1.GetType()
        'Sequence
		If t1.IsArray()
			ret = compareSequence(DirectCast( val1, Array), _
                  DirectCast( val2, Array))
		'String
		ElseIf TypeOf val1 Is String
			ret = DirectCast( val1, string) = DirectCast( val2, string)
		' Interface implementation
		ElseIf t1.GetInterfaces().Length > 0 And Not t1.IsValueType
			ret = val1 Is val2
		' Struct	
		ElseIf  Not t1.IsValueType
			ret = compareStruct(val1, val2)
		ElseIf TypeOf val1 Is Any
			Dim a1 As Any = DirectCast( val1, Any)
			Dim a2 As Any = DirectCast( val2, Any)
			ret = a1.Type.Equals( a2.Type ) And compareData( a1.Value, a2.Value )
		ElseIf t1.IsValueType
			'Any, enum, int, bool char, float, double etc.
			ret = val1.Equals(val2)
		Else
			Debug.Assert(False)
		End If
		Return ret
	End Function

	' Arrays have only one dimension
	Private Shared Function compareSequence( ar1 As Array, ar2 As Array) As Boolean
		Debug.Assert( Not (ar1 Is Nothing) And Not (ar2 Is Nothing) )
		Dim t1 As Type  = ar1.GetType()
		Dim t2 As Type  = ar2.GetType()
		
		if ( Not(ar1.Rank = 1 And ar2.Rank = 1 _
			And ar1.Length = ar2.Length And t1.GetElementType().Equals(t2.GetElementType())))
			return False
        End If
		'arrays have same rank and size and element type.
		Dim len As Integer  = ar1.Length
		Dim elemType As Type = t1.GetElementType()
		Dim ret As Boolean = True
        Dim i As Integer
		For i = 0 To len - 1
			If (compareData(ar1.GetValue(i), ar2.GetValue(i)) = False)
				ret = False
				Exit For
			End If
		Next i
		
        Return ret
	End Function

	Private Shared Function compareStruct( val1 As Object, val2 As Object) As Boolean
		Debug.Assert( Not(val1 Is Nothing) And Not(val2 Is Nothing))
		Dim t1 As Type = val1.GetType()
		Dim t2 As Type = val2.GetType()
		If Not t1.Equals(t2)
			Return False
        End If
		Dim fields() As FieldInfo = t1.GetFields()
		Dim cFields As Integer = fields.Length
		Dim ret As Boolean = True
        Dim i As Integer
		For i = 0 To cFields - 1
			Dim fieldVal1 As Object = fields(i).GetValue(val1)
			Dim fieldVal2 As Object = fields(i).GetValue(val2)
			If Not compareData(fieldVal1, fieldVal2)
				ret = False
				Exit For
			End If
		Next i
		Return ret
	End Function


    Private Shared Function performSequenceTest(xBT As XBridgeTest) As Boolean
        Dim bRet As Boolean = True
        'Automati cast ?? like with COM objects
        Dim xBT2 As XBridgeTest2 
        Try
            xBT2 = DirectCast(xBT,XBridgeTest2)
        Catch e As InvalidCastException
            Return False
        End Try

        ' perform sequence tests (XBridgeTest2)
        'create the sequence which are compared with the results
        Dim arBool() As Boolean = {True, False, True}
        Dim arChar() As Char = {"A"C,"B"C,"C"C}
        Dim arByte() As Byte = { 1,  2,  &Hff}
        Dim arShort() As Short = {Int16.MinValue, 1,  Int16.MaxValue}
        Dim arUShort() As UInt16 = {Convert.ToUInt16(0), Convert.ToUInt16(1), _
                                    Convert.ToUInt16(&Hffff)}
        Dim arLong() As Integer = {Int32.MinValue, 1, Int32.MaxValue}
        Dim arULong() As UInt32 = {Convert.ToUInt32(0), Convert.ToUInt32(1), _
                                   Convert.ToUInt32(&HffffffffL)}
        Dim arHyper() As Long = {Int64.MinValue, 1, Int64.MaxValue}
        Dim arUHyper() As UInt64 = {Convert.ToUInt64(0), Convert.ToUInt64(1), _
                                    Convert.ToUInt64(&Hffffffff5L)}
        Dim arFloat() As Single = {1.1f, 2.2f, 3.3f}
        Dim arDouble() As Double = {1.11, 2.22, 3.33}
        Dim arString() As String = {"String 1", "String 2", "String 3"}

        Dim arAny() As Any = {New Any(True), New Any(11111), New Any(3.14)}
        Dim arObject() As Object = {New WeakBase(), New WeakBase(), New WeakBase()}
        Dim arEnum() As TestEnum = {TestEnum.ONE, TestEnum.TWO, TestEnum.CHECK}

        Dim arStruct() As TestElement = {New TestElement(), New TestElement(), _
                               New TestElement()}
        assign( arStruct(0), True, "@"C, 17, &H1234, Convert.ToUInt16(&Hfedc), _
            &H12345678, Convert.ToUInt32(&H123456), &H123456789abcdef0, _
            Convert.ToUInt64(123456788), 17.0815F, 3.1415926359, _
            TestEnum.LOLA, CONSTANTS.STRING_TEST_CONSTANT, arObject(0), _
            New Any(GetType(System.Object), arObject(0)))
        assign( arStruct(1), True, "A"C, 17, &H1234, Convert.ToUInt16(&Hfedc), _
            &H12345678, Convert.ToUInt32(&H123456), &H123456789abcdef0, _
            Convert.ToUInt64(12345678), 17.0815F, 3.1415926359, _
            TestEnum.TWO, CONSTANTS.STRING_TEST_CONSTANT, arObject(1), _
            New Any(GetType(System.Object), arObject(1)) )
        assign( arStruct(2), True, "B"C, 17, &H1234, Convert.ToUInt16(&Hfedc), _ 
            &H12345678, Convert.ToUInt32(654321), &H123456789abcdef0, _
            Convert.ToUInt64(87654321), 17.0815F, 3.1415926359, _ 
            TestEnum.CHECK, Constants.STRING_TEST_CONSTANT, arObject(2), _
            New Any(GetType(System.Object), arObject(2)))

    
        Dim arLong3()()() As Integer = New Integer()()() { _
        New Integer()(){New Integer(){1,2,3},New Integer(){4,5,6}, New Integer(){7,8,9} }, _
        New Integer ()(){New Integer(){1,2,3},New Integer(){4,5,6}, New Integer(){7,8,9}}, _
        New Integer()(){New Integer(){1,2,3},New Integer(){4,5,6}, New Integer(){7,8,9}}}

        Dim seqSeqRet()() As Integer = xBT2.setDim2(arLong3(0))
        bRet = check( compareData(seqSeqRet, arLong3(0)), "sequence test") _
               And bRet
        Dim seqSeqRet2()()() As Integer = xBT2.setDim3(arLong3)
        bRet = check( compareData(seqSeqRet2, arLong3), "sequence test") _
               And bRet
        Dim seqAnyRet() As Any = xBT2.setSequenceAny(arAny)
        bRet = check( compareData(seqAnyRet, arAny), "sequence test") And bRet
        Dim seqBoolRet() As Boolean = xBT2.setSequenceBool(arBool)
        bRet = check( compareData(seqBoolRet, arBool), "sequence test") _
               And bRet
        Dim seqByteRet() As Byte = xBT2.setSequenceByte(arByte)
        bRet = check( compareData(seqByteRet, arByte), "sequence test") _
               And bRet
        Dim seqCharRet() As Char = xBT2.setSequenceChar(arChar)
        bRet = check( compareData(seqCharRet, arChar), "sequence test") _
                   And bRet
        Dim seqShortRet() As Short = xBT2.setSequenceShort(arShort)
        bRet = check( compareData(seqShortRet, arShort), "sequence test") _
               And bRet
        Dim seqLongRet() As Integer = xBT2.setSequenceLong(arLong)
        bRet = check( compareData(seqLongRet, arLong), "sequence test") _
                   And bRet
        Dim seqHyperRet() As Long = xBT2.setSequenceHyper(arHyper)
        bRet = check( compareData(seqHyperRet,arHyper), "sequence test") _
               And bRet
        Dim seqFloatRet() As Single = xBT2.setSequenceFloat(arFloat)
        bRet = check( compareData(seqFloatRet, arFloat), "sequence test") _ 
               And bRet
        Dim seqDoubleRet() As Double= xBT2.setSequenceDouble(arDouble)
        bRet = check( compareData(seqDoubleRet, arDouble), "sequence test") _
               And bRet
        Dim seqEnumRet() As TestEnum = xBT2.setSequenceEnum(arEnum)
        bRet = check( compareData(seqEnumRet, arEnum), "sequence test") _
               And bRet
        Dim seqUShortRet() As UInt16 = xBT2.setSequenceUShort(arUShort)
        bRet = check( compareData(seqUShortRet, arUShort), "sequence test") _ 
               And bRet
        Dim seqULongRet() As UInt32 = xBT2.setSequenceULong(arULong)
        bRet = check( compareData(seqULongRet, arULong), "sequence test") _
               And bRet
        Dim seqUHyperRet() As UInt64 = xBT2.setSequenceUHyper(arUHyper)
        bRet = check( compareData(seqUHyperRet, arUHyper), "sequence test") _
               And bRet
        Dim seqObjectRet() As Object = xBT2.setSequenceXInterface(arObject)
        bRet = check( compareData(seqObjectRet, arObject), "sequence test") _
               And bRet
        Dim seqStringRet() As String = xBT2.setSequenceString(arString)
        bRet = check( compareData(seqStringRet, arString), "sequence test") _
               And bRet
        Dim seqStructRet() As TestElement = xBT2.setSequenceStruct(arStruct)
        bRet = check( compareData(seqStructRet, arStruct), "sequence test") _
               And bRet
    
    
        Dim arBoolTemp() As Boolean = DirectCast(arBool.Clone(), Boolean())
        Dim arCharTemp() As Char = DirectCast(arChar.Clone(), Char())
        Dim arByteTemp() As Byte = DirectCast(arByte.Clone(), Byte())
        Dim arShortTemp() As Short = DirectCast(arShort.Clone(), Short())
        Dim arUShortTemp() As UInt16 = DirectCast(arUShort.Clone(), UInt16())
        Dim arLongTemp() As Integer= DirectCast(arLong.Clone(), Integer())
        Dim arULongTemp() As UInt32 =  DirectCast(arULong.Clone(), UInt32())
        Dim arHyperTemp() As Long = DirectCast(arHyper.Clone(), Long())
        Dim arUHyperTemp() As UInt64 = DirectCast(arUHyper.Clone(), UInt64())
        Dim arFloatTemp() As Single = DirectCast(arFloat.Clone(), Single())
        Dim arDoubleTemp() As Double = DirectCast(arDouble.Clone(), Double())
        Dim arEnumTemp() As TestEnum = DirectCast(arEnum.Clone(), TestEnum())
        Dim arStringTemp() As String = DirectCast(arString.Clone(), String())
        Dim arObjectTemp() As Object = DirectCast(arObject.Clone(), Object())
        Dim arAnyTemp() As Any = DirectCast(arAny.Clone(), Any())
        ' make sure this are has the same contents as arLong3(0)
        Dim arLong2Temp()() As Integer = New Integer()(){New Integer(){1,2,3}, _
                                         New Integer(){4,5,6}, New Integer(){7,8,9} }
        ' make sure this are has the same contents as arLong3
        Dim arLong3Temp()()() As Integer = New Integer()()(){ _
            New Integer()(){New Integer(){1,2,3},New Integer(){4,5,6}, New Integer(){7,8,9} }, _ 
            New Integer ()(){New Integer(){1,2,3},New Integer(){4,5,6}, New Integer(){7,8,9}}, _
            New Integer()(){New Integer(){1,2,3},New Integer(){4,5,6}, New Integer(){7,8,9}}}
        
        xBT2.setSequencesInOut( arBoolTemp, arCharTemp,  arByteTemp, _
                            arShortTemp,  arUShortTemp,  arLongTemp, _
                            arULongTemp, arHyperTemp,  arUHyperTemp, _
                            arFloatTemp, arDoubleTemp,  arEnumTemp, _
                            arStringTemp,   arObjectTemp, _
                             arAnyTemp,  arLong2Temp,  arLong3Temp)
        bRet = check( _
            compareData(arBoolTemp, arBool) And _
            compareData(arCharTemp , arChar) And _
            compareData(arByteTemp , arByte) And _
            compareData(arShortTemp , arShort) And _
            compareData(arUShortTemp , arUShort) And _
            compareData(arLongTemp , arLong) And _
            compareData(arULongTemp , arULong) And _
            compareData(arHyperTemp , arHyper) And _ 
            compareData(arUHyperTemp , arUHyper) And _
            compareData(arFloatTemp , arFloat) And _
            compareData(arDoubleTemp , arDouble) And _
            compareData(arEnumTemp , arEnum) And _
            compareData(arStringTemp , arString) And _
            compareData(arObjectTemp , arObject) And _
            compareData(arAnyTemp , arAny) And _ 
            compareData(arLong2Temp , arLong3(0)) And _
            compareData(arLong3Temp , arLong3), "sequence test") And bRet
        
        Dim arBoolOut() As Boolean
        Dim arCharOut() As Char
        Dim arByteOut() As Byte
        Dim arShortOut() As Short
        Dim arUShortOut() As UInt16
        Dim arLongOut() As Integer
        Dim arULongOut() As UInt32
        Dim arHyperOut() As Long
        Dim arUHyperOut() As UInt64
        Dim arFloatOut() As Single
        Dim arDoubleOut() As Double
        Dim arEnumOut() As TestEnum
        Dim arStringOut() As String
        Dim arObjectOut() As Object
        Dim arAnyOut() As Any
        Dim arLong2Out()() As Integer
        Dim arLong3Out()()() As Integer

        xBT2.setSequencesOut( arBoolOut,  arCharOut,  arByteOut, _ 
                             arShortOut,  arUShortOut,  arLongOut, _
                             arULongOut,  arHyperOut,  arUHyperOut, _
                             arFloatOut,  arDoubleOut,  arEnumOut, _
                             arStringOut,  arObjectOut,  arAnyOut, _
                             arLong2Out,  arLong3Out)
        bRet = check( _
            compareData(arBoolOut, arBool) And _
            compareData(arCharOut, arChar) And _
            compareData(arByteOut, arByte) And _
            compareData(arShortOut, arShort) And _
            compareData(arUShortOut, arUShort) And _
            compareData(arLongOut, arLong) And _
            compareData(arULongOut, arULong) And _
            compareData(arHyperOut, arHyper) And _
            compareData(arUHyperOut, arUHyper) And _
            compareData(arFloatOut, arFloat) And _
            compareData(arDoubleOut, arDouble) And _
            compareData(arEnumOut, arEnum) And _
            compareData(arStringOut, arString) And _
            compareData(arObjectOut, arObject) And _
            compareData(arAnyOut, arAny) And _
            compareData(arLong2Out, arLong3(0)) And _
            compareData(arLong3Out, arLong3), "sequence test") And bRet
        
        
        'test with empty sequences
        Dim _arLong2()() As Integer = New Integer()(){}
        seqSeqRet = xBT2.setDim2(_arLong2)
        bRet = check( compareData(seqSeqRet, _arLong2), "sequence test") And bRet
        Dim _arLong3()()() As Integer = New Integer()()(){}
        seqSeqRet2 = xBT2.setDim3(_arLong3)
        bRet = check( compareData(seqSeqRet2, _arLong3), "sequence test") And bRet
        Dim _arAny() As Any = New Any(){}
        seqAnyRet = xBT2.setSequenceAny(_arAny)
        bRet = check( compareData(seqAnyRet, _arAny), "sequence test") And bRet
        Dim _arBool() As Boolean = New Boolean() {}
        seqBoolRet = xBT2.setSequenceBool(_arBool)
        bRet = check( compareData(seqBoolRet, _arBool), "sequence test") And bRet
        Dim _arByte() As Byte = New Byte() {}
        seqByteRet = xBT2.setSequenceByte(_arByte)
        bRet = check( compareData(seqByteRet, _arByte), "sequence test") And bRet
        Dim _arChar() As Char = New Char() {}
        seqCharRet  = xBT2.setSequenceChar(_arChar)
        bRet = check( compareData(seqCharRet, _arChar), "sequence test") And bRet
        Dim _arShort() As Short = New Short() {}
        seqShortRet = xBT2.setSequenceShort(_arShort)
        bRet = check( compareData(seqShortRet, _arShort), "sequence test") And bRet
        Dim _arLong() As Integer = New Integer() {}
        seqLongRet = xBT2.setSequenceLong(_arLong)
        bRet = check( compareData(seqLongRet, _arLong), "sequence test") And bRet
        Dim _arHyper() As Long = New Long(){}
        seqHyperRet = xBT2.setSequenceHyper(_arHyper)
        bRet = check( compareData(seqHyperRet, _arHyper), "sequence test") And bRet
        Dim _arFloat() As Single = New Single(){}
        seqFloatRet = xBT2.setSequenceFloat(_arFloat)
        bRet = check( compareData(seqFloatRet, _arFloat), "sequence test") And bRet
        Dim _arDouble() As Double = New Double(){}
        seqDoubleRet = xBT2.setSequenceDouble(_arDouble)
        bRet = check( compareData(seqDoubleRet, _arDouble), "sequence test") And bRet
        Dim _arEnum() As TestEnum = New TestEnum(){}
        seqEnumRet = xBT2.setSequenceEnum(_arEnum)
        bRet = check( compareData(seqEnumRet, _arEnum), "sequence test") And bRet
        Dim  _arUShort() As UInt16 = New UInt16(){}
        seqUShortRet = xBT2.setSequenceUShort(_arUShort)
        bRet = check( compareData(seqUShortRet, _arUShort), "sequence test") And bRet
        Dim _arULong() As UInt32 = New UInt32(){}
        seqULongRet = xBT2.setSequenceULong(_arULong)
        bRet = check( compareData(seqULongRet, _arULong), "sequence test") And bRet
        Dim  _arUHyper() As UInt64 = New UInt64(){}
        seqUHyperRet = xBT2.setSequenceUHyper(_arUHyper)
        bRet = check( compareData(seqUHyperRet, _arUHyper), "sequence test") And bRet
        Dim _arObject() As Object = New Object(){}
        seqObjectRet = xBT2.setSequenceXInterface(_arObject)
        bRet = check( compareData(seqObjectRet, _arObject), "sequence test") And bRet
        Dim _arString() As String = New String(){}
        seqStringRet = xBT2.setSequenceString(_arString)
        bRet = check( compareData(seqStringRet, _arString), "sequence test") And bRet
        Dim _arStruct() As TestElement = New TestElement(){}
        seqStructRet = xBT2.setSequenceStruct(_arStruct)
        bRet = check( compareData(seqStructRet, _arStruct), "sequence test") And bRet
        Return bRet
    End Function

    Private Shared Function testAny(typ As Type, value As  Object, _
                                    xLBT As  XBridgeTest ) As Boolean
   
	    Dim any As Any
	    If (typ Is Nothing)
		    any = New Any(value.GetType(), value)
	    Else
		    any = New Any(typ, value)
        End If

	    Dim any2 As Any = xLBT.transportAny(any)
	    Dim ret As Boolean = compareData(any, any2)
	    If ret = False
            Console.WriteLine("any is different after roundtrip: in {0}, " _
                              & "out {1}\n", _
                            any.Type.FullName, any2.Type.FullName)
        End If
	    Return ret
    End Function

    Private Shared Function performAnyTest(xLBT As XBridgeTest, _
                                           data As TestDataElements) As Boolean
        Dim bReturn As Boolean = True
	    bReturn = testAny( Nothing, data.Byte ,xLBT ) And bReturn
	    bReturn = testAny( Nothing, data.Short,xLBT ) And bReturn
	    bReturn = testAny(  Nothing, data.UShort,xLBT ) And bReturn
	    bReturn = testAny(  Nothing, data.Long,xLBT ) And bReturn
	    bReturn = testAny(  Nothing, data.ULong,xLBT ) And bReturn
	    bReturn = testAny(  Nothing, data.Hyper,xLBT ) And bReturn
	    bReturn = testAny(  Nothing,data.UHyper,xLBT ) And bReturn
	    bReturn = testAny( Nothing, data.Float,xLBT ) And bReturn
	    bReturn = testAny( Nothing, data.Double,xLBT ) And bReturn
	    bReturn = testAny( Nothing, data.Enum,xLBT ) And bReturn
	    bReturn = testAny( Nothing, data.String,xLBT ) And bReturn
	    bReturn = testAny(GetType(unoidl.com.sun.star.uno.XWeak), _
                     data.Interface,xLBT ) And bReturn
	    bReturn = testAny(Nothing, data, xLBT ) And bReturn

        Dim a1 As Any = New Any(True)
        Dim a2 As Any = xLBT.transportAny( a1 )
	    bReturn = compareData(a2, a1) And bReturn
	
        Dim a3 As Any = New Any("A"C)
	    Dim a4 As Any = xLBT.transportAny(a3)
	    bReturn = compareData(a4, a3) And bReturn
	
	    Return bReturn 
    End Function

    Private Shared Function performSequenceOfCallTest(xLBT As XBridgeTest) As Boolean
    
	    Dim i, nRounds As Integer
	    Dim nGlobalIndex As Integer = 0
	    const nWaitTimeSpanMUSec As Integer = 10000
	    For nRounds = 0 To 9   
		    For i = 0 To  nRounds - 1 
			    ' fire oneways
			    xLBT.callOneway(nGlobalIndex, nWaitTimeSpanMUSec)
			    nGlobalIndex = nGlobalIndex + 1
		    Next

		    ' call synchron
		    xLBT.call(nGlobalIndex, nWaitTimeSpanMUSec)
		    nGlobalIndex = nGlobalIndex + 1
	    Next 
 	    Return xLBT.sequenceOfCallTestPassed()
    End Function

    Private Shared Function performRecursiveCallTest(xLBT As XBridgeTest) As Boolean  
	    xLBT.startRecursiveCall(new ORecursiveCall(), 50)
	    ' on failure, the test would lock up or crash
	    Return True
    End Function


    Private Shared Function performTest(xLBT As XBridgeTest) As Boolean
	    check( Not xLBT Is Nothing, "### no test interface!" )
        Dim bRet As Boolean = True
	    If xLBT Is Nothing
            Return False
        End If
		'this data is never ever granted access to by calls other than equals(), assign()!
		Dim aData As New TestDataElements' test against this data
		Dim xI As New WeakBase

        Dim aAny As New Any(GetType(System.Object), xI)
		assign( DirectCast(aData, TestElement), _
			True, "@"C, 17, &H1234, Convert.ToUInt16(&HdcS), &H12345678, _
            Convert.ToUInt32(4294967294), _
			&H123456789abcdef0, Convert.ToUInt64(14294967294), _
			17.0815f, 3.1415926359, TestEnum.LOLA, _
			CONSTANTS.STRING_TEST_CONSTANT, xI, _
			aAny)

		bRet = check( aData.Any.Value Is xI, "### unexpected any!" ) And bRet
		
		aData.Sequence = New TestElement(1){}
        aData.Sequence(0) = New TestElement( _
			aData.Bool, aData.Char, aData.Byte, aData.Short, _
			aData.UShort, aData.Long, aData.ULong, _
			aData.Hyper, aData.UHyper, aData.Float, _
			aData.Double, aData.Enum, aData.String, _
			aData.Interface, aData.Any)
        aData.Sequence(1) = New TestElement 'is empty
        
		' aData complete
		'
		' this is a manually copy of aData for first setting...
		Dim aSetData As New TestDataElements
		Dim aAnySet As New Any(GetType(System.Object), xI)
		assign( DirectCast(aSetData, TestElement), _
				aData.Bool, aData.Char, aData.Byte, aData.Short, aData.UShort, _
				aData.Long, aData.ULong, aData.Hyper, aData.UHyper, aData.Float, _
                aData.Double, aData.Enum, aData.String, xI, aAnySet)
		
		aSetData.Sequence = New TestElement(1){}
        aSetData.Sequence(0) = New TestElement( _
			aSetData.Bool, aSetData.Char, aSetData.Byte, aSetData.Short, _
			aSetData.UShort, aSetData.Long, aSetData.ULong, _
			aSetData.Hyper, aSetData.UHyper, aSetData.Float, _
			aSetData.Double, aSetData.Enum, aSetData.String, _
			aSetData.Interface, aSetData.Any)
        aSetData.Sequence(1) = New TestElement ' empty struct

		xLBT.setValues( _
				aSetData.Bool, aSetData.Char, aSetData.Byte, aSetData.Short, _
                aSetData.UShort, aSetData.Long, aSetData.ULong, _
                aSetData.Hyper, aSetData.UHyper, aSetData.Float, _
                aSetData.Double, aSetData.Enum, aSetData.String, _
                aSetData.Interface, aSetData.Any, aSetData.Sequence, _
                aSetData )
		
		
		Dim aRet As New TestDataElements
        Dim aRet2 As New TestDataElements
		xLBT.getValues( _
			aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, _
            aRet.UShort, aRet.Long, aRet.ULong, _
            aRet.Hyper, aRet.UHyper, aRet.Float, _
            aRet.Double, aRet.Enum, aRet.String, _
            aRet.Interface, aRet.Any, aRet.Sequence, _
            aRet2 )
		
		bRet = check( compareData( aData, aRet ) And _
                      compareData( aData, aRet2 ) , "getValues test") And bRet
        
		' set last retrieved values
		Dim  aSV2ret As TestDataElements= xLBT.setValues2( _
			aRet.Bool, aRet.Char, aRet.Byte, _
            aRet.Short, aRet.UShort, aRet.Long, _
            aRet.ULong, aRet.Hyper, aRet.UHyper, _
            aRet.Float, aRet.Double, aRet.Enum, _
            aRet.String, aRet.Interface, aRet.Any, _
            aRet.Sequence, aRet2 )
		
        ' check inout sequence order
        ' => inout sequence parameter was switched by test objects
		Dim temp As TestElement = aRet.Sequence( 0 )
        aRet.Sequence( 0 ) = aRet.Sequence( 1 )
        aRet.Sequence( 1 ) = temp
        
		bRet = check( _
            compareData( aData, aSV2ret ) And compareData( aData, aRet2 ), _
            "getValues2 test") And bRet
		
		
		aRet = New TestDataElements
        aRet2 = New TestDataElements
		Dim  aGVret As TestDataElements= xLBT.getValues( _
			aRet.Bool, aRet.Char, aRet.Byte, _ 
            aRet.Short, aRet.UShort, aRet.Long, _
            aRet.ULong, aRet.Hyper, aRet.UHyper, _
            aRet.Float, aRet.Double, aRet.Enum, _
            aRet.String, aRet.Interface, aRet.Any, _
            aRet.Sequence, aRet2 )
		
		bRet = check( compareData( aData, aRet ) And _
                      compareData( aData, aRet2 ) And _ 
                      compareData( aData, aGVret ), "getValues test" ) And bRet
		
		' set last retrieved values
		xLBT.Bool = aRet.Bool
		xLBT.Char = aRet.Char
		xLBT.Byte = aRet.Byte
		xLBT.Short = aRet.Short
		xLBT.UShort = aRet.UShort
        xLBT.Long = aRet.Long
		xLBT.ULong = aRet.ULong
		xLBT.Hyper = aRet.Hyper
		xLBT.UHyper = aRet.UHyper
		xLBT.Float = aRet.Float
		xLBT.Double = aRet.Double
		xLBT.Enum = aRet.Enum
		xLBT.String = aRet.String
		xLBT.Interface = aRet.Interface
		xLBT.Any = aRet.Any
		xLBT.Sequence = aRet.Sequence
		xLBT.Struct = aRet2
		
		
		aRet = New TestDataElements
        aRet2 = New TestDataElements
		aRet.Hyper = xLBT.Hyper
		aRet.UHyper = xLBT.UHyper
		aRet.Float = xLBT.Float
		aRet.Double = xLBT.Double
		aRet.Byte = xLBT.Byte
		aRet.Char = xLBT.Char
		aRet.Bool = xLBT.Bool
		aRet.Short = xLBT.Short
		aRet.UShort = xLBT.UShort
		aRet.Long = xLBT.Long
		aRet.ULong = xLBT.ULong
		aRet.Enum = xLBT.Enum
		aRet.String = xLBT.String
		aRet.Interface = xLBT.Interface
		aRet.Any = xLBT.Any
		aRet.Sequence = xLBT.Sequence
		aRet2 = xLBT.Struct
		
		bRet = check( compareData( aData, aRet ) And _
                      compareData( aData, aRet2 ) , "struct comparison test") _
                     And bRet

		bRet = check(performSequenceTest(xLBT), "sequence test") And bRet

		' any test
		bRet = check( performAnyTest( xLBT , aData ) , "any test" ) And bRet

		'sequence of call test
		bRet = check( performSequenceOfCallTest( xLBT ) , _
                      "sequence of call test" ) And bRet

		' recursive call test
		bRet = check( performRecursiveCallTest( xLBT ) , "recursive test" ) _
                And bRet
		
		bRet = (compareData( aData, aRet ) And compareData( aData, aRet2 )) _
                And bRet 

        ' check setting of null reference
        xLBT.Interface = Nothing
        aRet.Interface = xLBT.Interface
        bRet = (aRet.Interface Is Nothing) And bRet
        
        Return bRet
    End Function

    Private Shared Function raiseException(xLBT As XBridgeTest) As Boolean
	    Dim nCount As Integer = 0
	    Try
		    Try
			    Try
				    Dim aRet As TestDataElements = New TestDataElements
                    Dim aRet2 As TestDataElements = New TestDataElements
				    xLBT.raiseException( _
					    5, CONSTANTS.STRING_TEST_CONSTANT, xLBT.Interface )
			    Catch  rExc As unoidl.com.sun.star.lang.IllegalArgumentException
				    If rExc.ArgumentPosition = 5 And _
                        rExc.Context Is xLBT.Interface
					    nCount = nCount + 1
				    Else
					    check( False, "### unexpected exception content!" )
				    End If
    				
				    'it is certain, that the RuntimeException testing will fail,
                    '    if no 
				    xLBT.RuntimeException = 0
			    End Try
		    Catch rExc As unoidl.com.sun.star.uno.RuntimeException
			    If rExc.Context Is xLBT.Interface
			       nCount = nCount + 1
			    Else
				    check( False, "### unexpected exception content!" )
			    End If
                xLBT.RuntimeException = CType(&Hcafebabe, Integer)
    	    End Try
	    Catch rExc As unoidl.com.sun.star.uno.Exception
		    If rExc.Context Is xLBT.Interface
		        nCount = nCount + 1
		    Else
			    check( False, "### unexpected exception content!" )
            End If
		    Return nCount = 3
	    End Try
        Return False
    End Function

    Private Shared Function raiseOnewayException(xLBT As XBridgeTest) As Boolean
        Dim bReturn As Boolean= True
	    Dim sCompare As String = CONSTANTS.STRING_TEST_CONSTANT
	    Try
		    ' Note : the exception may fly or not (e.g. remote scenario).
		    '        When it flies, it must contain the correct elements.
		    xLBT.raiseRuntimeExceptionOneway(sCompare, xLBT.Interface )
	    Catch e As RuntimeException
		    bReturn =  xLBT.Interface Is e.Context
	    End Try
        Return bReturn
    End Function

    'Test the System::Object method on the proxy object
    '
    Private Shared Function testObjectMethodsImplementation(xLBT As XBridgeTest) As Boolean
        Dim ret As Boolean = False
        Dim obj As Object = New Object
	    Dim xInt As Object = DirectCast(xLBT, Object) 
	    Dim xBase As XBridgeTestBase = DirectCast(xLBT, XBridgeTestBase)
	    ' Object.Equals
	    ret = DirectCast(xLBT, Object).Equals(obj) = False
	    ret = DirectCast(xLBT, Object).Equals(xLBT) And ret
	    ret = Object.Equals(obj, obj) And ret
	    ret = Object.Equals(xLBT, xBase) And ret
	    'Object.GetHashCode
	    ' Don't know how to verify this. Currently it is not possible to get the object id from a proxy
	    Dim nHash As Integer = DirectCast(xLBT, Object).GetHashCode()
	    ret = nHash = DirectCast(xBase, Object).GetHashCode() And ret

	    'Object.ToString
        ' Don't know how to verify this automatically.
	    Dim s As String = DirectCast(xLBT, Object).ToString()
        ret = (s.Length > 0) And ret
        Return ret
    End Function

    Private Shared Function performQueryForUnknownType(xLBT As XBridgeTest) As Boolean
        Dim bRet As Boolean = False
        ' test queryInterface for an unknown type 
        Try
            Dim a As foo.MyInterface = DirectCast(xLBT, foo.MyInterface)
        Catch e As System.InvalidCastException
            bRet = True
        End Try

        Return bRet
    End Function


    Private Shared Sub perform_test( xLBT As XBridgeTest)
        Dim bRet As Boolean = True
        bRet = check( performTest( xLBT ), "standard test" ) And bRet
        bRet = check( raiseException( xLBT ) , "exception test" ) And bRet
        bRet = check( raiseOnewayException( xLBT ), "oneway exception test" ) _
               And bRet
        bRet = check( testObjectMethodsImplementation(xLBT), _ 
               "object methods test") And bRet
        bRet = performQueryForUnknownType( xLBT ) And bRet
        If  Not bRet
            Throw New unoidl.com.sun.star.uno.RuntimeException( "error: test failed!", Nothing)
        End If
    End Sub       


    
    Public Overridable Function run(args() As String) As Integer _
	   Implements XMain.run
        Try
            If (args.Length < 1)
                Throw New RuntimeException( _
                    "missing argument for bridgetest!", Me )
            End If
           
            Dim test_obj As Object = _
                m_xContext.getServiceManager().createInstanceWithContext( _
                    args( 0 ), m_xContext )

            Debug.WriteLine( _
                "cli target bridgetest obj: {0}", test_obj.ToString() )
            Dim xTest As XBridgeTest = DirectCast(test_obj, XBridgeTest) 
            perform_test( xTest )
            Console.WriteLine("### cli_uno VB bridgetest succeeded.")
            return 0
	Catch e as unoidl.com.sun.star.uno.RuntimeException
	     Throw 
        Catch e as System.Exception 
	      Throw New unoidl.com.sun.star.uno.RuntimeException( _
		    "cli_vb_bridgetest.vb: unexpected exception occurred in XMain::run. " _
		    & "Original exception: " + e.GetType().Name + "\n Message: " _
		    & e.Message , Nothing)

        End Try
    End Function

End Class

End Namespace
