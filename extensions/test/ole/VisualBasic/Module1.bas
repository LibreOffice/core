Attribute VB_Name = "Module1"
Option Explicit

Private objServiceManager
Private objCoreReflection
Private objOleTest
Private objEventListener
'General counter
Dim i As Long
Dim j As Long
Dim sError As String

Sub Main()
    Set objServiceManager = CreateObject("com.sun.star.ServiceManager")
    Set objCoreReflection = objServiceManager.createInstance("com.sun.star.reflection.CoreReflection")
    ' extensions/test/ole/cpnt
    Set objOleTest = objServiceManager.createInstance("oletest.OleTest")
    ' extensions/test/ole/EventListenerSample/VBEventListener
    Set objEventListener = CreateObject("VBasicEventListener.VBEventListener")
    Debug.Print TypeName(objOleTest)

' In Parameter, simple types
'============================================
Dim tmpVar As Variant
Dim ret As Variant
Dim inByte As Byte, outByte As Byte, retByte As Byte
Dim inBool As Boolean, outBool As Boolean, retBool As Boolean
Dim inShort As Integer, outShort As Integer, retShort As Integer
Dim inUShort As Integer, outUShort As Integer, retUShort As Integer
Dim inLong As Long, outLong As Long, retLong As Long
Dim inULong As Long, outULong As Long, retULong As Long
Dim inHyper As Variant, outHyper As Variant, retHyper As Variant
Dim inUHyper As Variant, outUHyper As Variant, retUHyper As Variant
Dim inFloat As Single, outFloat As Single, retFloat As Single
Dim inDouble As Double, outDouble As Double, retDouble As Double
Dim inString As String, outString As String, retString As String
Dim inChar As Integer, outChar As Integer, retChar As Integer, retChar2 As Integer
Dim inCharAsString As String, outCharAsString As String, retCharAsString As String
Dim inAny As Variant, outAny As Variant, retAny As Variant
Dim inType As Object, outType As Object, retType As Object
Dim inXInterface As Object, outXInterface As Object, retXInterface As Object
Dim inXInterface2 As Object, outXInterface2 As Object, retXInterface2 As Object


Dim outVarByte As Variant
Dim outVarBool As Variant
Dim outVarShort As Variant
Dim outVarUShort As Variant
Dim outVarLong As Variant
Dim outVarULong As Variant
Dim outVarFloat As Variant
Dim outVarDouble As Variant
Dim outVarString As Variant
Dim outVarChar As Variant
Dim outVarAny As Variant
Dim outVarType As Variant

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
Set inType = objServiceManager.Bridge_CreateType("[]long")
Set inXInterface = objCoreReflection
Set inXInterface2 = objEventListener

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
Set retType = objOleTest.in_methodType(inType)
Set retXInterface = objOleTest.in_methodXInterface(inXInterface) ' UNO object
Set retXInterface2 = objOleTest.in_methodXInterface(inXInterface2)

If retByte <> inByte Or retBool <> inBool _
    Or retShort <> inShort Or retUShort <> inUShort _
    Or retLong <> inLong Or retULong <> inULong _
    Or retHyper <> inHyper Or retUHyper <> inUHyper _
    Or retFloat <> inFloat Or retDouble <> inDouble _
    Or retString <> inString Or retChar <> inChar _
    Or retChar2 <> Asc(inCharAsString) Or retAny <> inAny _
    Or retType <> inType _
    Or Not (inXInterface Is retXInterface) _
    Or Not (inXInterface2 Is retXInterface2) Then
    sError = "in - parameter and return value test failed"
    GoTo onerror
End If

'Out Parameter simple types
'================================================

objOleTest.testout_methodByte outByte
objOleTest.testout_methodFloat outFloat
objOleTest.testout_methodDouble outDouble
objOleTest.testout_methodBool outBool
objOleTest.testout_methodShort outShort
objOleTest.testout_methodUShort outUShort
objOleTest.testout_methodLong outLong
objOleTest.testout_methodULong outULong
objOleTest.testout_methodHyper outHyper
objOleTest.testout_methodUHyper outUHyper
objOleTest.testout_methodString outString
objOleTest.testout_methodChar outChar
objOleTest.testout_methodChar outCharAsString
objOleTest.testout_methodAny outAny
objOleTest.testout_methodType outType
'objOleTest.in_methodXInterface (inXInterface) ' UNO object
Call objOleTest.in_methodXInterface(inXInterface)  ' UNO object
objOleTest.testout_methodXInterface outXInterface
Call objOleTest.in_methodXInterface(inXInterface2)  ' COM object
objOleTest.testout_methodXInterface outXInterface2


If outByte <> inByte Or outFloat <> inFloat _
    Or outDouble <> inDouble Or outBool <> inBool _
    Or outShort <> inShort Or outUShort <> inUShort _
    Or outLong <> inLong Or outULong <> inULong _
    Or outHyper <> inHyper Or outUHyper <> inUHyper _
    Or outString <> inString Or outChar <> inChar _
    Or outCharAsString <> inCharAsString _
    Or outAny <> inAny Or outType <> inType _
    Or Not (inXInterface Is outXInterface) _
    Or Not (inXInterface2 Is outXInterface2) Then

    sError = "out - parameter test failed!"
    GoTo onerror
End If

'Out Parameter simple types (VARIANT var)
'====================================================
objOleTest.testout_methodByte outVarByte
objOleTest.testout_methodBool outVarBool
objOleTest.testout_methodChar outVarChar
objOleTest.testout_methodShort outVarShort
objOleTest.testout_methodUShort outVarUShort
objOleTest.testout_methodLong outVarLong
objOleTest.testout_methodULong outVarULong
objOleTest.testout_methodString outVarString
objOleTest.testout_methodFloat outVarFloat
objOleTest.testout_methodDouble outVarDouble
objOleTest.testout_methodAny outVarAny
objOleTest.testout_methodType outVarType

If outVarByte <> inByte Or outVarBool <> inBool _
    Or outVarChar <> inChar _
    Or outVarShort <> inShort Or outVarUShort <> inUShort _
    Or outVarLong <> inLong Or outVarULong <> inULong _
    Or outVarString <> inString _
    Or outVarFloat <> inFloat Or outVarDouble <> inDouble _
    Or outVarAny <> inAny _
    Or outVarType <> inType _
    Then
    sError = "out - parameter (VARIANT) test failed!"
    GoTo onerror
End If

'In/Out simple types
'============================================
objOleTest.in_methodByte (0)
objOleTest.in_methodBool (False)
objOleTest.in_methodShort (0)
objOleTest.in_methodUShort (0)
objOleTest.in_methodLong (0)
objOleTest.in_methodULong (0)
objOleTest.in_methodHyper (0)
objOleTest.in_methodUHyper (0)
objOleTest.in_methodFloat (0)
objOleTest.in_methodDouble (0)
objOleTest.in_methodString (0)
objOleTest.in_methodChar (0)
objOleTest.in_methodAny (0)
objOleTest.in_methodType (objServiceManager.Bridge_CreateType("boolean"))
Set outXInterface = Nothing
Call objOleTest.in_methodXInterface(outXInterface)


outByte = 10
retByte = outByte
objOleTest.testinout_methodByte retByte
objOleTest.testinout_methodByte retByte
outBool = True
retBool = outBool
objOleTest.testinout_methodBool retBool
objOleTest.testinout_methodBool retBool
outShort = 10
retShort = outShort
objOleTest.testinout_methodShort retShort
objOleTest.testinout_methodShort retShort
outUShort = 20
retUShort = outUShort
objOleTest.testinout_methodUShort retUShort
objOleTest.testinout_methodUShort retUShort
outLong = 30
retLong = outLong
objOleTest.testinout_methodLong retLong
objOleTest.testinout_methodLong retLong
outULong = 40
retULong = outULong
objOleTest.testinout_methodULong retLong
objOleTest.testinout_methodULong retLong
outHyper = CDec("9223372036854775807") 'highest positiv value of int64
retHyper = outHyper
objOleTest.testinout_methodHyper retHyper
objOleTest.testinout_methodHyper retHyper
outUHyper = CDec("18446744073709551615") 'highest value of unsigned int64
retUHyper = outUHyper
objOleTest.testinout_methodUHyper retUHyper
objOleTest.testinout_methodUHyper retUHyper
outFloat = 3.14
retFloat = outFloat
objOleTest.testinout_methodFloat retFloat
objOleTest.testinout_methodFloat retFloat
outDouble = 4.14
retDouble = outDouble
objOleTest.testinout_methodDouble retDouble
objOleTest.testinout_methodDouble retDouble
outString = "Hello World!"
retString = outString
objOleTest.testinout_methodString retString
objOleTest.testinout_methodString retString
outChar = 66
retChar = outChar
objOleTest.testinout_methodChar retChar
objOleTest.testinout_methodChar retChar
outCharAsString = "H"
retCharAsString = outCharAsString
objOleTest.testinout_methodChar retCharAsString
objOleTest.testinout_methodChar retCharAsString
outAny = "Hello World 2!"
retAny = outAny
objOleTest.testinout_methodAny retAny
objOleTest.testinout_methodAny retAny
Set outType = objServiceManager.Bridge_CreateType("long")
Set retType = outType
objOleTest.testinout_methodType retType
objOleTest.testinout_methodType retType

Set outXInterface = objCoreReflection
Set retXInterface = outXInterface
objOleTest.testinout_methodXInterface2 retXInterface

If outByte <> retByte Or outBool <> retBool _
    Or outShort <> retShort Or outUShort <> retUShort _
    Or outLong <> retLong Or outULong <> retULong _
    Or outHyper <> retHyper Or outUHyper <> outUHyper _
    Or outFloat <> retFloat _
    Or outDouble <> retDouble _
    Or outString <> retString Or outChar <> retChar _
    Or outCharAsString <> retCharAsString Or outAny <> retAny _
    Or outType <> retType _
    Or Not (outXInterface Is retXInterface) Then
    sError = "in/out - parameter test failed!"
    GoTo onerror
End If


'======================================================================
'======================================================================
'Attributes
objOleTest.AByte = inByte
retByte = 0
retByte = objOleTest.AByte
objOleTest.AFloat = inFloat
retFloat = 0
retFloat = objOleTest.AFloat
objOleTest.AType = inType
Set retType = Nothing
Set retType = objOleTest.AType

If inByte <> retByte _
    Or inFloat <> retFloat _
    Or inType <> retType _
    Then
    sError = "Attributes - test failed!"
    GoTo onerror
End If

'======================================================================
' Other Hyper tests
Dim emptyVar As Variant
retAny = emptyVar
inHyper = CDec("9223372036854775807") 'highest positiv value of int64
retAny = objOleTest.in_methodAny(inHyper)
sError = "hyper test failed"
If inHyper <> retAny Then
    GoTo onerror
End If
inHyper = CDec("-9223372036854775808") 'lowest negativ value of int64
retAny = objOleTest.in_methodAny(inHyper)
If inHyper <> retAny Then
    GoTo onerror
End If
inHyper = CDec("18446744073709551615") 'highest positiv value of unsigne int64
retAny = objOleTest.in_methodAny(inHyper)
If inHyper <> retAny Then
    GoTo onerror
End If
inHyper = CDec(-1)
retAny = objOleTest.in_methodAny(inHyper)
If inHyper <> retAny Then
    GoTo onerror
End If
inHyper = CDec(0)
retAny = objOleTest.in_methodAny(inHyper)
If inHyper <> retAny Then
    GoTo onerror
End If

'==============================================================================

Dim outVAr

'Any test. We pass in an any as value object. If it is not correct converted
'then the target component throws a RuntimeException
Dim lengthInAny As Long

lengthInAny = 10
Dim seqLongInAny(10) As Long
For i = 0 To lengthInAny - 1
    seqLongInAny(i) = i + 10
Next
Dim anySeqLong As Object
Set anySeqLong = objOleTest.Bridge_GetValueObject()
anySeqLong.Set "[]long", seqLongInAny
Dim anySeqRet As Variant
Err.Clear
On Error Resume Next
anySeqRet = objOleTest.other_methodAny(anySeqLong, "[]long")

If Err.Number <> 0 Then
    MsgBox "error"
End If


'Objects
'
'==========================================================
' COM obj
Dim retObj As Object
'OleTest receives a COM object that implements XEventListener
'OleTest then calls a disposing on the object. The object then will be
'asked if it has been called
objEventListener.setQuiet True
objEventListener.resetDisposing
Set retObj = objOleTest.in_methodInvocation(objEventListener)
ret = objEventListener.disposingCalled
If ret = False Then
    MsgBox "Error"
End If

'The returned object should be objEventListener, test it by calling disposing
' takes an IDispatch as Param ( EventObject).To provide a TypeMismatch
'we put in another IDispatch
retObj.resetDisposing
retObj.disposing objEventListener
If retObj.disposingCalled = False Then
    MsgBox "Error"
End If


' out param gives out the OleTestComponent
objOleTest.testout_methodXInterface retObj
outVAr = Null
retObj.testout_methodAny outVAr
Debug.Print "test out Interface " & CStr(outVAr)
If outVAr <> "I am a string in an any" Then
    MsgBox "error"
End If

'in out
' in: UNO object, the same is expected as out param
' the function expects OleTest as parameter and sets a value
Dim objOleTest2 As Object
Set objOleTest2 = objServiceManager.createInstance("oletest.OleTest")
'Set a value
objOleTest2.AttrAny2 = "VBString "
objOleTest.testinout_methodXInterface2 objOleTest2
tmpVar = Null
tmpVar = objOleTest2.AttrAny2
Debug.Print "in: Uno out: the same object // " & CStr(tmpVar)
If tmpVar <> "VBString  this string was written in the UNO component to the inout pararmeter" Then
    MsgBox "error"
End If


'create a struct
Dim structClass As Object
Set structClass = objCoreReflection.forName("oletest.SimpleStruct")
Dim structInstance As Object
structClass.CreateObject structInstance
structInstance.message = "Now we are in VB"
Debug.Print "struct out " & structInstance.message
If structInstance.message <> "Now we are in VB" Then
    MsgBox "error"
End If

'put the struct into OleTest. The same struct will be returned with an added String
Dim structRet As Object
Set structRet = objOleTest.in_methodStruct(structInstance)
Debug.Print "struct in - return " & structRet.message
If structRet.message <> "Now we are in VBThis string was set in OleTest" Then
    MsgBox "error"
End If


'Arrays
'========================================
Dim arrLong(2) As Long
Dim arrObj(2) As Object
Dim countvar As Long
For countvar = 0 To 2
    arrLong(countvar) = countvar + 10
    Debug.Print countvar
    Set arrObj(countvar) = CreateObject("VBasicEventListener.VBEventListener")
    arrObj(countvar).setQuiet True
Next

'Arrays always contain VARIANTS
Dim seq() As Variant
seq = objOleTest.methodLong(arrLong)

For countvar = 0 To 2
    Debug.Print CStr(seq(countvar))
    If arrLong(countvar) <> seq(countvar) Then
        MsgBox "error"
    End If
Next
seq = objOleTest.methodXInterface(arrObj)
For countvar = 0 To 2
    Dim tmp As Object
    seq(countvar).resetDisposing
    seq(countvar).disposing tmp
    If seq(countvar).disposingCalled = False Then
       MsgBox "Error"
    End If
Next

'Array containing interfaces (element type is VT_DISPATCH)
Dim arEventListener(2) As Object
For countvar = 0 To 2
    Set arEventListener(countvar) = CreateObject("VBasicEventListener.VBEventListener")
    arEventListener(countvar).setQuiet True
Next

'The function calls disposing on the listeners
seq = objOleTest.methodXEventListeners(arEventListener)
Dim count
For countvar = 0 To 2
    If arEventListener(countvar).disposingCalled = False Then
        MsgBox "Error"
    End If
Next
'Array containing interfaces (element type is VT_VARIANT which contains VT_DISPATCH
Dim arEventListener2(2) As Variant
For countvar = 0 To 2
     Set arEventListener2(countvar) = CreateObject("VBasicEventListener.VBEventListener")
     arEventListener2(countvar).setQuiet True
Next
seq = objOleTest.methodXEventListeners(arEventListener2)
For countvar = 0 To 2
    If arEventListener2(countvar).disposingCalled = False Then
        MsgBox "Error"
    End If
Next

'Variant containing Array containing interfaces (element type is VT_VARIANT which contains VT_DISPATCH
Dim arEventListener3(2) As Variant
For countvar = 0 To 2
     Dim var As Variant
     Set arEventListener3(countvar) = CreateObject("VBasicEventListener.VBEventListener")
     arEventListener3(countvar).setQuiet True
Next
Dim varContAr As Variant
varContAr = arEventListener3
seq = objOleTest.methodXEventListeners(varContAr)
For countvar = 0 To 2
    If arEventListener3(countvar).disposingCalled = False Then
        MsgBox "Error"
    End If
Next

'Get a sequence created in UNO, out param is Variant ( VT_BYREF|VT_VARIANT)
Dim seqX As Variant

objOleTest.testout_methodSequence seqX
Dim key
For Each key In seqX
    Debug.Print CStr(seqX(key))
    If seqX(key) <> key Then
        MsgBox "error"
    End If
Next
'Get a sequence created in UNO, out param is array Variant ( VT_BYREF|VT_VARIANT|VT_ARRAY)
Dim seqX2() As Variant
objOleTest.testout_methodSequence seqX2

For Each key In seqX2
    Debug.Print CStr(seqX2(key))
Next

'pass it to UNO and get it back
Dim seq7() As Variant
seq7 = objOleTest.methodLong(seqX)
Dim key2
For Each key2 In seq7
    Debug.Print CStr(seq7(key2))
    If seqX2(key) <> key Then
        MsgBox "error"
    End If
Next

'array with starting index != 0
Dim seqIndex(1 To 2) As Long
Dim seq8() As Variant
Dim longVal1 As Long, longVal2 As Long
longVal1 = 1
longVal2 = 2
seqIndex(1) = longVal1
seqIndex(2) = longVal2
seq8 = objOleTest.methodLong(seqIndex)
If longVal1 <> seq8(0) And longVal2 <> seq8(1) Then
    MsgBox "error"
End If


'in out Array
' arrLong is Long Array
Dim inoutVar(2) As Variant

For countvar = 0 To 2
    inoutVar(countvar) = countvar + 10
Next

objOleTest.testinout_methodSequence inoutVar

countvar = 0
For countvar = 0 To 2
    Debug.Print CStr(inoutVar(countvar))
    If inoutVar(countvar) <> countvar + 11 Then
        MsgBox "error"
    End If
Next

'Multidimensional array
'============================================================
' Sequence< Sequence<long> > methodSequence( Sequence< Sequence long> >)
' Real multidimensional array Array
' 9 is Dim 1 (least significant) with C API
Dim mulAr(9, 1) As Long
For i = 0 To 1
    For j = 0 To 9
        mulAr(j, i) = i * 10 + j
    Next j
Next i

Dim resMul As Variant
resMul = objOleTest.methodSequence(mulAr)

Dim countDim1 As Long
Dim countDim2 As Long
For countDim2 = 0 To 1
    Dim arr
    arr = resMul(countDim2)
    For countDim1 = 0 To 9
        Debug.Print arr(countDim1)
        If arr(countDim1) <> mulAr(countDim1, countDim2) Then
            MsgBox "Error Multidimensional Array"
        End If
    Next countDim1
Next countDim2
IsArray (resMul)

'Array of VARIANTs containing arrays
Dim mulAr2(1) As Variant
Dim arr2(9) As Long
For i = 0 To 1
   ' Dim arr(9) As Long
    For j = 0 To 9
        arr2(j) = i * 10 + j
    Next j
    mulAr2(i) = arr2
Next i

resMul = 0
resMul = objOleTest.methodSequence(mulAr2)
arr = 0
For countDim2 = 0 To 1
    arr = resMul(countDim2)
    tmpVar = mulAr2(countDim2)
    For countDim1 = 0 To 9
        Debug.Print arr(countDim1)
        If arr(countDim1) <> tmpVar(countDim1) Then
            MsgBox "Error Multidimensional Array"
        End If
    Next countDim1
Next countDim2

'Array containing interfaces (element type is VT_DISPATCH)
Dim arArEventListener(1, 2) As Object
For i = 0 To 1
    For j = 0 To 2
        Set arArEventListener(i, j) = CreateObject("VBasicEventListener.VBEventListener")
        arArEventListener(i, j).setQuiet True
    Next
Next
'The function calls disposing on the listeners
seq = objOleTest.methodXEventListenersMul(arArEventListener)
For i = 0 To 1
    For j = 0 To 2
        If arArEventListener(i, j).disposingCalled = False Then
            MsgBox "Error"
        End If
    Next
Next

'Array containing interfaces (element type is VT_VARIANT containing VT_DISPATCH)
Dim arArEventListener2(1, 2) As Variant
For i = 0 To 1
    For j = 0 To 2
        Set arArEventListener2(i, j) = CreateObject("VBasicEventListener.VBEventListener")
        arArEventListener2(i, j).setQuiet True
    Next
Next
'The function calls disposing on the listeners
seq = objOleTest.methodXEventListenersMul(arArEventListener2)
For i = 0 To 1
    For j = 0 To 2
        If arArEventListener2(i, j).disposingCalled = False Then
            MsgBox "Error"
        End If
    Next
Next

' SAFEARRAY of VARIANTS containing SAFEARRAYs
'The ultimate element type is VT_DISPATCH ( XEventListener)
Dim arEventListener4(1) As Variant
Dim seq1(2) As Object
Dim seq2(2) As Object
For i = 0 To 2
    Set seq1(i) = CreateObject("VBasicEventListener.VBEventListener")
    Set seq2(i) = CreateObject("VBasicEventListener.VBEventListener")
    seq1(i).setQuiet True
    seq2(i).setQuiet True
Next
arEventListener4(0) = seq1
arEventListener4(1) = seq2
'The function calls disposing on the listeners
seq = objOleTest.methodXEventListenersMul(arEventListener4)
For i = 0 To 2
    If seq1(i).disposingCalled = False Or seq2(i).disposingCalled = False Then
            MsgBox "Error"
    End If
Next

'Bridge_GetStruct
'========================================================
'Try to create a hidden document
Dim objPropValue
Set objPropValue = objOleTest.Bridge_GetStruct("com.sun.star.beans.PropertyValue")
'Set the members. If this fails then there is an Error
objPropValue.Name = "hidden"
objPropValue.Handle = -1
objPropValue.Value = True

'create a hidden document
'Create the Desktop
Dim objDesktop As Object
Set objDesktop = objServiceManager.createInstance("com.sun.star.frame.Desktop")
'Open a new empty writer document
Dim args(0) As Object
Set args(0) = objPropValue
Dim objDocument As Object
Set objDocument = objDesktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, args())
'Create a text object
Dim objText As Object
Set objText = objDocument.GetText

'Bridge_ImplementedInterfaces
'=================================================
' call an UNO function that takes an XEventListener interface
'We provide a COM implementation (IDispatch) as EventListener
objEventListener.resetDisposing
objDocument.addEventListener objEventListener
objDocument.dispose
If objEventListener.disposingCalled = False Then
    MsgBox "Error"
End If


'Bridge_GetValueObject
'==================================================
Dim objVal As Object
Set objVal = objOleTest.Bridge_GetValueObject()
Dim arrByte(9) As Byte
For countvar = 0 To 9
    arrByte(countvar) = countvar
Next countvar

objVal.Set "[]byte", arrByte
ret = 0
ret = objOleTest.methodByte(objVal)
'Test if ret is the same array

key = 0
For Each key In ret
    If ret(key) <> arrByte(key) Then
        MsgBox "Error"
    End If
    Debug.Print ret(key)
Next key


outByte = 77
retByte = outByte
objVal.InitInOutParam "byte", retByte
objOleTest.testinout_methodByte objVal
objVal.InitInOutParam "byte", retByte
objOleTest.testinout_methodByte objVal

ret = 0
ret = objVal.Get()
Debug.Print ret
If ret <> outByte Then
    MsgBox "error"
End If

objVal.InitOutParam
inChar = 65
objOleTest.in_methodChar (inChar)
objOleTest.testout_methodChar objVal 'Returns 'A' (65)
ret = 0
ret = objVal.Get()
Debug.Print ret
If ret <> inChar Then
    MsgBox "error"
End If

Exit Sub
onerror:
MsgBox "Error: " + sError
End Sub
