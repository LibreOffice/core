Attribute VB_Name = "Module1"
Option Explicit

Private objServiceManager
Private objCoreReflection
Private objOleTest
Private objEventListener

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
Dim bError As Boolean
ret = objOleTest.in_methodByte(10)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> 10 Then
    MsgBox "error"
End If

ret = objOleTest.in_methodFloat(3.14)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> 3.14 Then
    MsgBox "error"
End If
Dim d As Double 'try conversion
d = 3.14
ret = objOleTest.in_methodFloat(3.14)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> 3.14 Then
    MsgBox "error"
End If

ret = objOleTest.in_methodDouble(4.14)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> 4.14 Then
    MsgBox "error"
End If
Dim s As Single
s = 4.14
ret = objOleTest.in_methodDouble(s)
Debug.Print TypeName(ret) & " " & CStr(ret)
If (ret < 4.13) And (ret > 4.15) Then
    MsgBox "error"
End If

ret = objOleTest.in_methodBool(True)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> True Then
    MsgBox "error"
End If

ret = objOleTest.in_methodBool(False)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> False Then
    MsgBox "error"
End If

ret = objOleTest.in_methodShort(-10)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> -10 Then
    MsgBox "error"
End If
ret = objOleTest.in_methodUShort(10)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> 10 Then
    MsgBox "error"
End If
ret = objOleTest.in_methodLong(-1000000)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> -1000000 Then
    MsgBox "error"
End If

ret = objOleTest.in_methodULong(1000000)
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> 1000000 Then
    MsgBox "error"
End If

ret = objOleTest.in_methodString("This is a String")
Debug.Print TypeName(ret) & " " & CStr(ret)
If CStr(ret) <> "This is a String" Then
    MsgBox "error"
End If

'different character tests
ret = objOleTest.in_methodChar("A")
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> 65 Then
    MsgBox "error"
End If
'!!!Function returns char, i.e sal_Unicode which VB converts to String
Dim ret1 As String
ret1 = objOleTest.in_methodChar("A")
Debug.Print TypeName(ret1) & " " & CStr(ret1)
If ret <> 65 Then
    MsgBox "error"
End If

ret1 = objOleTest.in_methodChar(65)
Debug.Print TypeName(ret1) & " " & CStr(ret1)
If ret <> 65 Then
    MsgBox "error"
End If

ret = objOleTest.in_methodAny("input string")
Debug.Print TypeName(ret) & " " & CStr(ret)
If ret <> "input string" Then
    MsgBox "error"
End If

'Call objOleTest.in_methodAll(10, 10.1, 10.111, True, 10, 11, 12, 13, _
'            "A String", "A", "A String in an Any")
            
'Out Parameter simple types
'================================================
Dim outByte As Byte
objOleTest.testout_methodByte outByte
Debug.Print "out byte " & CStr(outByte)
If outByte <> 111 Then
    MsgBox "error"
End If

Dim outFloat As Single
objOleTest.testout_methodFloat outFloat
Debug.Print "out float " & CStr(outFloat)
If outFloat <> 3.14 Then
    MsgBox "error"
End If

Dim outDouble As Double
objOleTest.testout_methodDouble outDouble
Debug.Print "out double " & CStr(outDouble)
If outDouble <> 3.14 Then
    MsgBox "error"
End If

Dim outBool As Boolean
objOleTest.testout_methodBool outBool
Debug.Print "out bool " & CStr(outBool)
If outBool <> True Then
    MsgBox "error"
End If

Dim outInt As Integer
objOleTest.testout_methodShort outInt
Debug.Print "out short " & CStr(outInt)
If outInt <> 222 Then
    MsgBox "error"
End If

objOleTest.testout_methodUShort outInt
Debug.Print "out unsignedshort " & CStr(outInt)
If outInt <> 333 Then
    MsgBox "error"
End If

Dim outLong As Long
objOleTest.testout_methodLong outLong
Debug.Print "out long " & CStr(outLong)
If outLong <> 444 Then
    MsgBox "error"
End If

objOleTest.testout_methodULong outLong
Debug.Print "out unsigned long " & CStr(outLong)
If outLong <> 555 Then
    MsgBox "error"
End If

Dim outString As String
objOleTest.testout_methodString outString
Debug.Print "out string " & CStr(outString)
If outString <> "a little string" Then
    MsgBox "error"
End If

Dim outChar As Integer
objOleTest.testout_methodChar outChar
Debug.Print "out char " & CStr(outChar)
If outChar <> 65 Then
    MsgBox "error"
End If

Dim outCharS As String
objOleTest.testout_methodChar outCharS
Debug.Print "out char (String) " & CStr(outCharS)
If outCharS <> "A" Then
    MsgBox "error"
End If

objOleTest.testout_methodAny outString
Debug.Print "out Any " & CStr(outString)

'Out Parameter simple types (VARIANT var)
Dim outVar As Variant
objOleTest.testout_methodByte outVar
Debug.Print "out Byte (VARIANT) " & CStr(outVar)
If outVar <> 111 Then
    MsgBox "error"
End If

objOleTest.testout_methodFloat outVar
Debug.Print "out float (VARIANT) " & CStr(outVar)
If outVar <> 3.14 Then
    MsgBox "error"
End If

objOleTest.testout_methodDouble outVar
Debug.Print "out double (VARIANT) " & CStr(outVar)
If outVar <> 3.14 Then
    MsgBox "error"
End If

objOleTest.testout_methodBool outVar
Debug.Print "out bool (VARIANT) " & CStr(outVar)
If outVar <> True Then
    MsgBox "error"
End If

objOleTest.testout_methodShort outVar
Debug.Print "out short (VARIANT) " & CStr(outVar)
If outVar <> 222 Then
    MsgBox "error"
End If

objOleTest.testout_methodUShort outVar
Debug.Print "out unsigned short (VARIANT) " & CStr(outVar)
If outVar <> 333 Then
    MsgBox "error"
End If

objOleTest.testout_methodLong outVar
Debug.Print "out long (VARIANT) " & CStr(outVar)
If outVar <> 444 Then
    MsgBox "error"
End If

objOleTest.testout_methodULong outVar
Debug.Print "out unsigned long (VARIANT) " & CStr(outVar)
If outVar <> 555 Then
    MsgBox "error"
End If

objOleTest.testout_methodString outVar
Debug.Print "out string (VARIANT) " & CStr(outVar)
If outVar <> "a little string" Then
    MsgBox "error"
End If

objOleTest.testout_methodChar outVar
Debug.Print "out char (VARIANT) " & CStr(outVar)
If outVar <> 65 Then
    MsgBox "error"
End If

objOleTest.testout_methodAny outVar
Debug.Print "out any (VARIANT) " & CStr(outVar)
If outVar <> "I am a string in an any" Then
    MsgBox "error"
End If

'In/Out simple types
'============================================
outByte = 10
objOleTest.testinout_methodByte outByte
Debug.Print "inout byte " & CStr(outByte)
If outByte <> 11 Then
    MsgBox "error"
End If

outFloat = 3.14
objOleTest.testinout_methodFloat outFloat
Debug.Print "inout float " & CStr(outFloat)
If (outFloat > 4.15) And (outFloat < 4.13) Then
    MsgBox "error"
End If

outDouble = 4.14
objOleTest.testinout_methodDouble outDouble
Debug.Print "inout double " & CStr(outDouble)
If outDouble <> 5.14 Then
    MsgBox "error"
End If

outBool = True
objOleTest.testinout_methodBool outBool
Debug.Print "inout bool " & CStr(outBool)
If outBool <> False Then
    MsgBox "error"
End If

outInt = 10
objOleTest.testinout_methodShort outInt
Debug.Print "inout short " & CStr(outInt)
If outInt <> 11 Then
    MsgBox "error"
End If

outInt = 20
objOleTest.testinout_methodUShort outInt
Debug.Print "inout unsignedshort " & CStr(outInt)
If outInt <> 21 Then
    MsgBox "error"
End If

outLong = 30
objOleTest.testinout_methodLong outLong
Debug.Print "inout long " & CStr(outLong)
If outLong <> 31 Then
    MsgBox "error"
End If

outLong = 40
objOleTest.testinout_methodULong outLong
Debug.Print "inout unsigned long " & CStr(outLong)
If outLong <> 41 Then
    MsgBox "error"
End If

outString = "this is an in string"
objOleTest.testinout_methodString outString
Debug.Print "inout string " & CStr(outString)
If outString <> "this is an in string out string" Then
    MsgBox "error"
End If

'different Char conversions
objOleTest.testout_methodChar outString
Debug.Print "out char (in: String)" & CStr(outString)
If outString <> "A" Then
    MsgBox "error"
End If

objOleTest.testout_methodChar outInt
Debug.Print "out char (in: Int)" & CStr(outInt)
If outInt <> 65 Then
    MsgBox "error"
End If

'--
outString = "this is another in out string"
objOleTest.testout_methodAny outString
Debug.Print "out Any " & CStr(outString)
If outString <> "I am a string in an any" Then
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
outVar = Null
retObj.testout_methodAny outVar
Debug.Print "test out Interface " & CStr(outVar)
If outVar <> "I am a string in an any" Then
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

'inout later

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
Dim i As Long, j As Long
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


objVal.InitInOutParam "byte", 10
objOleTest.testinout_methodByte objVal

ret = 0
ret = objVal.Get()
Debug.Print ret
If ret <> 11 Then
    MsgBox "error"
End If

objVal.InitOutParam
objOleTest.testout_methodChar objVal 'Returns 'A' (65)
ret = 0
ret = objVal.Get()
Debug.Print ret
If ret <> 65 Then
    MsgBox "error"
End If

End Sub
