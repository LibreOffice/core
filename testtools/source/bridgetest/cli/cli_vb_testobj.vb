'/*************************************************************************
 '
 '  $RCSfile: cli_vb_testobj.vb,v $
 '
 '  $Revision: 1.2 $
 '
 '  last change: $Author: obo $ $Date: 2003-09-04 09:17:28 $
 '
 '  The Contents of this file are made available subject to the terms of
 '  either of the following licenses
 '
 '         - GNU Lesser General Public License Version 2.1
 '         - Sun Industry Standards Source License Version 1.1
 '
 '  Sun Microsystems Inc., October, 2000
 '
 '  GNU Lesser General Public License Version 2.1
 '  =============================================
 '  Copyright 2000 by Sun Microsystems, Inc.
 '  901 San Antonio Road, Palo Alto, CA 94303, USA
 '
 '  This library is free software; you can redistribute it and/or
 '  modify it under the terms of the GNU Lesser General Public
 '  License version 2.1, as published by the Free Software Foundation.
 '
 '  This library is distributed in the hope that it will be useful,
 '  but WITHOUT ANY WARRANTY; without even the implied warranty of
 '  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 '  Lesser General Public License for more details.
 '
 '  You should have received a copy of the GNU Lesser General Public
 '  License along with this library; if not, write to the Free Software
 '  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 '  MA  02111-1307  USA
 '
 '
 '  Sun Industry Standards Source License Version 1.1
 '  =================================================
 '  The contents of this file are subject to the Sun Industry Standards
 '  Source License Version 1.1 (the "License"); You may not use this file
 '  except in compliance with the License. You may obtain a copy of the
 '  License at http://www.openoffice.org/license.html.
 '
 '  Software provided under this License is provided on an "ASIS" basis,
 '  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 '  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 '  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 '  See the License for the specific provisions governing your rights and
 '  obligations concerning the Software.
 '
 '  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 '
 '  Copyright: 2000 by Sun Microsystems, Inc.
 '
 '  All Rights Reserved.
 '
 '  Contributor(s): _______________________________________
 '
 '
 '******************************************************************/

Option Explicit On
Option Strict On

imports System
imports uno
imports uno.util
imports unoidl.com.sun.star.lang
imports unoidl.com.sun.star.uno
imports unoidl.com.sun.star.test.bridge
imports System.Windows.Forms
imports System.Diagnostics
imports System.Reflection


Namespace vb_testobj
Public Class VBBridgeTestObject 
    Inherits WeakBase
    Implements XRecursiveCall, XBridgeTest2

    Private m_xContext As XComponentContext 
    
    Public Sub New (xContext As XComponentContext)
        MyBase.New
        m_xContext = xContext
    End Sub

    Private m_bool As Boolean
    Private m_char As Char
    Private m_byte As Byte
    Private m_short  As Short
    Private m_ushort As UInt16
    Private m_long As Integer
    Private m_ulong As UInt32
    Private m_hyper As Long
    Private m_uhyper As UInt64 
    Private m_float As Single
    Private m_double As Double
    Private m_string As String
    Private m_xInterface As Object
    Private m_any As Any
    Private m_testEnum As TestEnum = TestEnum.TEST
    Private m_testElements() As TestElement = New TestElement(){}
    Private m_testDataElements As TestDataElements = New TestDataElements
    Private m_nLastCallId As Integer = 0
    Private m_bFirstCall As Boolean = True
    Private m_bSequenceOfCallTestPassed As Boolean = True

    Private m_arBool() As Boolean
    Private m_arChar() As Char
    Private m_arByte() As Byte
    Private m_arShort() As Short
    Private m_arLong() As Integer
    Private m_arHyper() As Long
    Private m_arUShort() As UInt16
    Private m_arULong() As UInt32
    Private m_arUHyper() As UInt64
    Private m_arString() As String
    Private m_arFloat() As Single
    Private m_arDouble() As Double
    Private m_arEnum() As TestEnum
    Private m_arObject() As Object
    Private m_arLong2()() As Integer
    Private m_arLong3()()() As Integer
    Private m_arAny() As Any
    
    Public Overridable Sub setValues( _
        bBool As Boolean, aChar As Char, nByte As Byte, nShort As Short, _
        nUShort As UInt16, nLong As Integer, nULong As UInt32, _
        nHyper As Long, nUHyper As UInt64, fFloat As Single, _
        fDouble As Double, testEnum As TestEnum, str As String, _
        xInterface As Object, any As Any, testElements() As TestElement, _
        testDataElements As TestDataElements) _
        Implements XBridgeTest2.setValues
#if DEBUG
      '  Console.WriteLine( "##### " + GetType().FullName + ".setValues:" + any )
#endif
        m_bool             = bBool
        m_char             = aChar
        m_byte             = nByte
        m_short            = nShort
        m_ushort           = nUShort
        m_long             = nLong
        m_ulong            = nULong
        m_hyper            = nHyper
        m_uhyper           = nUHyper
        m_float            = fFloat
        m_double           = fDouble
        m_testEnum         = testEnum
        m_string           = str
        m_xInterface       = xInterface
        m_any              = any
        m_testElements     = testElements
        m_testDataElements = testDataElements
    End Sub
    
    Public Overridable Function setValues2( _
        ByRef io_bool As Boolean, ByRef io_char As Char, _
        ByRef io_byte As Byte, ByRef io_short As Short, _
        ByRef io_ushort As UInt16, ByRef io_long As Integer, _
        ByRef io_ulong As UInt32, ByRef io_hyper As Long, _
        ByRef io_uhyper As UInt64, ByRef io_float As Single, _
        ByRef io_double As Double, ByRef io_testEnum As TestEnum, _
        ByRef io_string As String, ByRef io_xInterface As Object, _
        ByRef io_any As Any, ByRef io_testElements() As TestElement, _
        ByRef io_testDataElements As TestDataElements) As TestDataElements _
        Implements XBridgeTest2.setValues2
    
#if DEBUG
        'Console.WriteLine( "##### " + GetType().FullName + ".setValues2:" + io_any )
#endif
        
        m_bool             = io_bool
        m_char             = io_char
        m_byte             = io_byte
        m_short            = io_short
        m_ushort           = io_ushort
        m_long             = io_long
        m_ulong            = io_ulong
        m_hyper            = io_hyper
        m_uhyper           = io_uhyper
        m_float            = io_float
        m_double           = io_double
        m_testEnum         = io_testEnum
        m_string           = io_string
        m_xInterface       = io_xInterface
        m_any              = io_any
        m_testElements     = DirectCast(io_testElements.Clone(), TestElement()) 
        m_testDataElements = io_testDataElements
        
        Dim temp As TestElement = io_testElements(0)
        io_testElements(0) = io_testElements(1)
        io_testElements(1) = temp
        
        Return m_testDataElements
    End Function
    
    Public Overridable Function  getValues( _
        ByRef o_bool As Boolean, ByRef o_char As Char, _
        ByRef o_byte As Byte, ByRef o_short As Short, _
        ByRef o_ushort As UInt16, ByRef o_long As Integer, _
        ByRef o_ulong As UInt32, ByRef o_hyper As Long, _
        ByRef o_uhyper As UInt64, ByRef o_float As Single, _
        ByRef o_double As Double, ByRef o_testEnum As TestEnum, _
        ByRef o_string As String, ByRef o_xInterface As Object, _
        ByRef o_any As Any, ByRef o_testElements() As TestElement, _
        ByRef o_testDataElements As TestDataElements) As TestDataElements _
        Implements XBridgeTest2.getValues
#if DEBUG
        'Console.WriteLine( "##### " + GetType().FullName + ".getValues" )
#endif
        
        o_bool             = m_bool
        o_char             = m_char
        o_byte             = m_byte
        o_short            = m_short
        o_ushort           = m_ushort
        o_long             = m_long
        o_ulong            = m_ulong
        o_hyper            = m_hyper
        o_uhyper           = m_uhyper
        o_float            = m_float
        o_double           = m_double
        o_testEnum         = m_testEnum
        o_string           = m_string
        o_xInterface       = m_xInterface
        o_any              = m_any
        o_testElements     = m_testElements
        o_testDataElements = m_testDataElements
        
        Return m_testDataElements
    End Function
    
    ' Attributes ---------------------------------------------------------
    Public Overridable Property Bool As Boolean _
        Implements XBridgeTest2.Bool
        Get 
            Return m_bool
        End Get
        Set (Value As Boolean)
            m_bool = value
        End Set
    End Property

    Public Overridable Property [Byte] As Byte _
        Implements XBridgeTest2.Byte
        Get  
            Return m_byte 
        End Get
        Set (Value As Byte)
            m_byte = value
        End Set
    End Property

    Public Overridable Property [Char] As Char _
        Implements XBridgeTest2.Char
        Get
            Return m_char
        End Get
        Set (Value As Char)
            m_char = value 
        End Set
    End Property

    Public Overridable Property [Short] As Short _
        Implements XBridgeTest2.Short
        Get 
            Return m_short 
        End Get
        Set (Value As Short)
            m_short = value 
        End Set
    End Property

    Public Overridable Property UShort As UInt16 _
        Implements XBridgeTest2.UShort
        Get
            Return m_ushort
        End Get
        Set (Value As UInt16)
            m_ushort = value
        End Set
    End Property

    Public Overridable Property [Long] As Integer _
        Implements XBridgeTest2.Long
        Get
            Return m_long 
        End Get
        Set (Value As Integer)
            m_long = value 
        End Set
    End Property

    Public Overridable Property ULong() As UInt32 _
        Implements XBridgeTest2.ULong
        Get
            Return m_ulong 
        End Get
        Set (Value As UInt32)
            m_ulong = value 
        End Set
    End Property

    Public Overridable Property Hyper As Long _
        Implements XBridgeTest2.Hyper
        Get
            Return m_hyper
        End Get
        Set (Value As Long)
            m_hyper = value
        End Set
    End Property

    Public Overridable Property UHyper As UInt64 _
        Implements XBridgeTest2.UHyper
        Get 
            Return m_uhyper
        End Get
        Set (Value As UInt64)
            m_uhyper = value 
        End Set
    End Property

    Public Overridable Property Float As Single _
        Implements XBridgeTest2.Float
        Get
            Return m_float 
        End Get
        Set (Value As Single)
            m_float = value 
        End Set
    End Property

    Public Overridable Property [Double] As Double _
        Implements XBridgeTest2.Double
        Get
            Return m_double 
        End Get
        Set (Value As Double) 
            m_double = value
        End Set
    End Property

    Public Overridable Property [Enum] As TestEnum _
        Implements XBridgeTest2.Enum
        Get 
            Return m_testEnum
        End Get
        Set (Value As TestEnum)
            m_testEnum = value 
        End Set
    End Property

    Public Overridable Property [String] As String _
        Implements XBridgeTest2.String
        Get
            Return m_string
        End Get
        Set (Value As String)
            m_string = value
        End Set
    End Property

    Public Overridable Property [Interface] As Object _
        Implements XBridgeTest2.Interface
        Get 
            Return m_xInterface
        End Get
        Set (Value As Object)
            m_xInterface = value
        End Set
    End Property

    Public Overridable Property  Any As uno.Any _
        Implements XBridgeTest2.Any
        Get
#if DEBUG
'            Console.WriteLine( "##### " + GetType().FullName + ".Any" )
#endif
            Return m_any
        End Get
        Set (Value As Any)
#if DEBUG
            'Console.WriteLine( "##### " + GetType().FullName + ".Any:" + value )
#endif
            m_any = value
        End Set
    End Property

    Public Overridable Property Sequence As TestElement() _
        Implements XBridgeTest2.Sequence
        Get
            Return m_testElements 
        End Get
        Set (Value() As TestElement)
            m_testElements = value
        End Set
    End Property

    Public Overridable Property Struct As TestDataElements _
        Implements XBridgeTest2.Struct
        Get
            Return m_testDataElements 
        End Get
        Set (Value As TestDataElements)
            m_testDataElements = value
        End Set
    End Property
    
    Public Overridable Function transportAny(value As Any) As Any _
        Implements XBridgeTest2.transportAny
        Return value
    End Function

    Public Overridable Sub [call](nCallId As Integer, nWaitMUSEC As Integer) _
        Implements XBridgeTest2.call

        Threading.Thread.Sleep(CType(nWaitMUSEC / 10000, Integer))     
        If m_bFirstCall = True
            m_bFirstCall = False
        Else 
            m_bSequenceOfCallTestPassed = m_bSequenceOfCallTestPassed And (nCallId > m_nLastCallId)
        End If
        m_nLastCallId = nCallId
    End Sub

    Public Overridable Sub callOneway(nCallId As Integer, nWaitMUSEC As Integer) _
        Implements XBridgeTest2.callOneway

        Threading.Thread.Sleep(CType(nWaitMUSEC / 10000, Integer)) 
        m_bSequenceOfCallTestPassed = m_bSequenceOfCallTestPassed And (nCallId > m_nLastCallId)
        m_nLastCallId = nCallId
    End Sub

    Public Overridable Function sequenceOfCallTestPassed() As Boolean _
        Implements XBridgeTest2.sequenceOfCallTestPassed
        Return m_bSequenceOfCallTestPassed
    End Function

    Public Overridable Sub callRecursivly(xCall As  XRecursiveCall, nToCall As Integer) _
        Implements XRecursiveCall.callRecursivly
        SyncLock (Me)
            If nToCall <> 0
                nToCall = nToCall - 1
                xCall.callRecursivly(Me , nToCall)
            End If
        End SyncLock
    End Sub

    Public Overridable Sub startRecursiveCall(xCall As  XRecursiveCall, nToCall As Integer) _
        Implements XBridgeTest2.startRecursiveCall
        SyncLock (Me)
            If nToCall <> 0
                nToCall = nToCall - 1
                xCall.callRecursivly(Me , nToCall )
            End If
        End SyncLock
    End Sub
    
    ' XBridgeTest
    Public Overridable Function raiseException( _
        nArgumentPos As Short, rMsg As String, xContext As Object) As TestDataElements _
        Implements XBridgeTest2.raiseException
        Throw New IllegalArgumentException(rMsg, xContext, nArgumentPos)
    End Function

    Public Overridable Sub raiseRuntimeExceptionOneway(rMsg As  String , xContext As Object) _
        Implements XBridgeTest2.raiseRuntimeExceptionOneway
        Throw New RuntimeException(rMsg, xContext)
    End Sub

    Public Overridable Property RuntimeException As Integer _
        Implements XBridgeTest2.RuntimeException
        Get
            Throw New RuntimeException(m_string, m_xInterface) 
        End Get
        Set (Value As Integer)
            Throw New RuntimeException(m_string, m_xInterface) 
        End Set
    End Property
    
    ' XBridgeTest2
    Public Overridable Function setDim2(val()() As Integer) As Integer()() _
        Implements XBridgeTest2.setDim2
        m_arLong2 = val
        Return val
    End Function

    Public Overridable Function setDim3(val()()() As Integer) As Integer()()() _
        Implements XBridgeTest2.setDim3
        m_arLong3 = val
        Return val
    End Function

    Public Overridable Function setSequenceAny(val() As Any) As Any() _
        Implements XBridgeTest2.setSequenceAny
        m_arAny = val
        Return val
    End Function

    Public Overridable Function setSequenceBool(val() As Boolean) As Boolean() _
        Implements XBridgeTest2.setSequenceBool
        m_arBool = val
        Return val
    End Function

    Public Overridable Function setSequenceByte(val() As Byte) As Byte() _
        Implements XBridgeTest2.setSequenceByte
        m_arByte = val
        Return val
    End Function

    Public Overridable Function setSequenceChar(val() As Char) As Char() _
        Implements XBridgeTest2.setSequenceChar
        m_arChar = val
        Return val
    End Function

    Public Overridable Function setSequenceShort(val() As Short) As Short() _
        Implements XBridgeTest2.setSequenceShort
        m_arShort = val
        Return val
    End Function

    Public Overridable Function setSequenceLong(val() As Integer) As Integer() _
        Implements XBridgeTest2.setSequenceLong

        m_arLong = val
        Return val
    End Function

    Public Overridable Function setSequenceHyper(val() As Long) As Long() _
        Implements XBridgeTest2.setSequenceHyper
        m_arHyper = val
        Return val
    End Function

    Public Overridable Function setSequenceFloat(val() As Single) As Single() _
        Implements XBridgeTest2.setSequenceFloat
        m_arFloat = val
        Return val
    End Function

    Public Overridable Function setSequenceDouble(val() As Double) As Double() _
        Implements XBridgeTest2.setSequenceDouble
        m_arDouble = val
        Return val
    End Function

    Public Overridable Function setSequenceEnum(val() As TestEnum) As TestEnum() _
        Implements XBridgeTest2.setSequenceEnum
        m_arEnum = val
        Return val
    End Function

    Public Overridable Function setSequenceUShort(val() As UInt16) As UInt16() _
        Implements XBridgeTest2.setSequenceUShort
        m_arUShort = val
        Return val
    End Function

    Public Overridable Function setSequenceULong(val() As  UInt32) As  UInt32() _
        Implements XBridgeTest2.setSequenceULong
        m_arULong = val
        Return  val
    End Function

    Public Overridable Function setSequenceUHyper(val() As UInt64) As UInt64() _
        Implements XBridgeTest2.setSequenceUHyper
        m_arUHyper = val
        Return val
    End Function

    Public Overridable Function setSequenceXInterface(val() As Object ) As Object() _
        Implements XBridgeTest2.setSequenceXInterface
        m_arObject = val
        Return val
    End Function

    Public Overridable Function setSequenceString(val() As String) As String() _
        Implements XBridgeTest2.setSequenceString
        m_arString = val
        Return val
    End Function

    Public Overridable Function setSequenceStruct(val() As TestElement) As TestElement() _
        Implements XBridgeTest2.setSequenceStruct
        m_testElements = val
        Return val
    End Function
    
    Public Overridable Sub setSequencesInOut( _
        ByRef aSeqBoolean() As Boolean, ByRef aSeqChar() As Char, _
        ByRef aSeqByte() As Byte, ByRef aSeqShort() As Short, _
        ByRef aSeqUShort() As UInt16, ByRef aSeqLong() As Integer, _
        ByRef aSeqULong() As  UInt32, ByRef aSeqHyper() As Long, _
        ByRef aSeqUHyper() As UInt64, ByRef aSeqFloat() As Single, _
        ByRef aSeqDouble() As Double, ByRef aSeqTestEnum() As TestEnum, _
        ByRef aSeqString() As String, ByRef aSeqXInterface() As Object, _
        ByRef aSeqAny() As Any, ByRef aSeqDim2()() As Integer, _
        ByRef aSeqDim3()()() As Integer) _
        Implements XBridgeTest2.setSequencesInOut

        m_arBool = aSeqBoolean
        m_arChar = aSeqChar
        m_arByte = aSeqByte
        m_arShort = aSeqShort
        m_arUShort = aSeqUShort
        m_arLong = aSeqLong
        m_arULong = aSeqULong
        m_arHyper  = aSeqHyper
        m_arUHyper = aSeqUHyper
        m_arFloat = aSeqFloat
        m_arDouble = aSeqDouble
        m_arEnum = aSeqTestEnum
        m_arString = aSeqString
        m_arObject = aSeqXInterface
        m_arAny = aSeqAny
        m_arLong2 = aSeqDim2
        m_arLong3 = aSeqDim3
    End Sub

    Public Overridable Sub setSequencesOut( _
        ByRef aSeqBoolean() As Boolean, ByRef aSeqChar() As Char, _
        ByRef aSeqByte() As Byte, ByRef aSeqShort() As Short, _
        ByRef aSeqUShort() As UInt16, ByRef aSeqLong() As Integer, _
        ByRef aSeqULong() As  UInt32, ByRef aSeqHyper() As Long, _
        ByRef aSeqUHyper() As UInt64, ByRef  aSeqFloat() As Single, _
        ByRef aSeqDouble() As Double, ByRef aSeqTestEnum() As TestEnum, _
        ByRef aSeqString() As String, ByRef aSeqXInterface() As Object, _
        ByRef aSeqAny() As Any, ByRef aSeqDim2()() As Integer, _
        ByRef aSeqDim3()()() As Integer) _
        Implements XBridgeTest2.setSequencesOut

        aSeqBoolean = m_arBool
        aSeqChar = m_arChar
        aSeqByte = m_arByte
        aSeqShort = m_arShort
        aSeqUShort = m_arUShort
        aSeqLong = m_arLong
        aSeqULong = m_arULong
        aSeqHyper = m_arHyper
        aSeqUHyper = m_arUHyper
        aSeqFloat = m_arFloat
        aSeqDouble = m_arDouble
        aSeqTestEnum = m_arEnum
        aSeqString = m_arString
        aSeqXInterface = m_arObject
        aSeqAny = m_arAny
        aSeqDim2 = m_arLong2
        aSeqDim3 = m_arLong3
            
    End Sub

End Class

End Namespace
