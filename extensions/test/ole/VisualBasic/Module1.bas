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
ret = objOleTest.in_methodFloat(3.14)
Debug.Print TypeName(ret) & " " & CStr(ret)

Dim d As Double 'try conversion
d = 3.14
ret = objOleTest.in_methodFloat(3.14)
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodDouble(4.14)
Debug.Print TypeName(ret) & " " & CStr(ret)
Dim s As Single
s = 4.14
ret = objOleTest.in_methodDouble(s)
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodBool(True)
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodBool(False)
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodShort(-10)
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodUShort(10)
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodLong(-1000000)
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodULong(1000000)
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodString("This is a String")
Debug.Print TypeName(ret) & " " & CStr(ret)
'different character tests
ret = objOleTest.in_methodChar("A")
Debug.Print TypeName(ret) & " " & CStr(ret)
Dim ret1 As String
ret1 = objOleTest.in_methodChar("A")
Debug.Print TypeName(ret1) & " " & CStr(ret1)
ret1 = objOleTest.in_methodChar(65)
Debug.Print TypeName(ret1) & " " & CStr(ret1)
ret = objOleTest.in_methodAny("This is a String in an any")
Debug.Print TypeName(ret) & " " & CStr(ret)
'Call objOleTest.in_methodAll(10, 10.1, 10.111, True, 10, 11, 12, 13, _
'            "A String", "A", "A String in an Any")
            
'Out Parameter simple types
'================================================
Dim outByte As Byte
objOleTest.testout_methodByte outByte
Debug.Print "out byte " & CStr(outByte)
Dim outFloat As Single
objOleTest.testout_methodFloat outFloat
Debug.Print "out float " & CStr(outFloat)
Dim outDouble As Double
objOleTest.testout_methodDouble outDouble
Debug.Print "out double " & CStr(outDouble)
Dim outBool As Boolean
objOleTest.testout_methodBool outBool
Debug.Print "out bool " & CStr(outBool)
Dim outInt As Integer
objOleTest.testout_methodShort outInt
Debug.Print "out short " & CStr(outInt)
objOleTest.testout_methodUShort outInt
Debug.Print "out unsignedshort " & CStr(outInt)
Dim outLong As Long
objOleTest.testout_methodLong outLong
Debug.Print "out long " & CStr(outInt)
objOleTest.testout_methodULong outLong
Debug.Print "out unsigned long " & CStr(outInt)
Dim outString As String
objOleTest.testout_methodString outString
Debug.Print "out string " & CStr(outString)
Dim outChar As Integer
objOleTest.testout_methodChar outChar
Debug.Print "out char " & CStr(outChar)
Dim outCharS As String
objOleTest.testout_methodChar outCharS
Debug.Print "out char (String) " & CStr(outCharS)
objOleTest.testout_methodAny outString
Debug.Print "out Any " & CStr(outString)

'Out Parameter simple types (VARIANT var)
Dim outVar As Variant
objOleTest.testout_methodByte outVar
Debug.Print "out Byte (VARIANT) " & CStr(outVar)
objOleTest.testout_methodFloat outVar
Debug.Print "out float (VARIANT) " & CStr(outVar)
objOleTest.testout_methodDouble outVar
Debug.Print "out double (VARIANT) " & CStr(outVar)
objOleTest.testout_methodBool outVar
Debug.Print "out bool (VARIANT) " & CStr(outVar)
objOleTest.testout_methodShort outVar
Debug.Print "out short (VARIANT) " & CStr(outVar)
objOleTest.testout_methodUShort outVar
Debug.Print "out unsigned short (VARIANT) " & CStr(outVar)
objOleTest.testout_methodLong outVar
Debug.Print "out long (VARIANT) " & CStr(outVar)
objOleTest.testout_methodULong outVar
Debug.Print "out unsigned long (VARIANT) " & CStr(outVar)
objOleTest.testout_methodString outVar
Debug.Print "out string (VARIANT) " & CStr(outVar)
objOleTest.testout_methodChar outVar
Debug.Print "out char (VARIANT) " & CStr(outVar)
objOleTest.testout_methodAny outVar
Debug.Print "out any (VARIANT) " & CStr(outVar)

'In/Out simple types
'============================================
outByte = 10
objOleTest.testinout_methodByte outByte
Debug.Print "inout byte " & CStr(outByte)
outFloat = 3.14
objOleTest.testinout_methodFloat outFloat
Debug.Print "inout float " & CStr(outFloat)
outDouble = 4.14
objOleTest.testinout_methodDouble outDouble
Debug.Print "inout double " & CStr(outDouble)
outBool = True
objOleTest.testinout_methodBool outBool
Debug.Print "inout bool " & CStr(outBool)
outInt = 10
objOleTest.testinout_methodShort outInt
Debug.Print "inout short " & CStr(outInt)
outInt = 20
objOleTest.testinout_methodUShort outInt
Debug.Print "inout unsignedshort " & CStr(outInt)
outLong = 30
objOleTest.testinout_methodLong outLong
Debug.Print "inout long " & CStr(outLong)
outLong = 40
objOleTest.testinout_methodULong outLong
Debug.Print "inout unsigned long " & CStr(outLong)
outString = "this is an in string"
objOleTest.testinout_methodString outString
Debug.Print "inout string " & CStr(outString)
'different Char conversions
objOleTest.testout_methodChar outString
Debug.Print "out char (in: String)" & CStr(outString)
objOleTest.testout_methodChar outInt
Debug.Print "out char (in: Int)" & CStr(outInt)
'--
outString = "this is another in out string"
objOleTest.testout_methodAny outString
Debug.Print "out Any " & CStr(outString)

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


'create a struct
Dim structClass As Object
Set structClass = objCoreReflection.forName("oletest.SimpleStruct")
Dim structInstance As Object
structClass.CreateObject structInstance
structInstance.message = "Now we are in VB"
Debug.Print "struct out " & structInstance.message
'put the struct into OleTest. The same struct will be returned with an added String
Dim structRet As Object
Set structRet = objOleTest.in_methodStruct(structInstance)
Debug.Print "struct in - return " & structRet.message
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

'Get a sequence created in UNO, out param is Variant ( VT_BYREF|VT_VARIANT)
Dim seqX As Variant

objOleTest.testout_methodSequence seqX
Dim key
For Each key In seqX
    Debug.Print CStr(seqX(key))
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
Next

'in out Array
' arrLong is Long Array
Dim inoutVar(2) As Variant

For countvar = 0 To 2
    Debug.Print countvar
    inoutVar(countvar) = countvar + 10
Next

objOleTest.testinout_methodSequence inoutVar
Dim key5
For Each key5 In inoutVar
    Debug.Print CStr(key5)
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


objVal.InitInOutParam "[]byte", 10
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





Dim var As Boolean
var = True
Debug.Print CStr(var)


End Sub
