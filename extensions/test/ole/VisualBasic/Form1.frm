VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   4572
   ClientLeft      =   48
   ClientTop       =   336
   ClientWidth     =   8460
   LinkTopic       =   "Form1"
   ScaleHeight     =   4572
   ScaleWidth      =   8460
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton ctrInParamStart 
      Caption         =   "Start"
      Height          =   252
      Left            =   240
      TabIndex        =   20
      Top             =   480
      Width           =   732
   End
   Begin VB.TextBox tareaResult 
      Height          =   1932
      Left            =   4920
      TabIndex        =   19
      Top             =   480
      Width           =   3372
   End
   Begin VB.CommandButton Command20 
      Caption         =   "Command20"
      Height          =   252
      Left            =   3000
      TabIndex        =   18
      Top             =   2640
      Width           =   492
   End
   Begin VB.CommandButton Command19 
      Caption         =   "string"
      Height          =   252
      Left            =   2280
      TabIndex        =   17
      Top             =   2640
      Width           =   492
   End
   Begin VB.CommandButton Command18 
      Caption         =   "long"
      Height          =   252
      Left            =   1560
      TabIndex        =   16
      Top             =   2640
      Width           =   492
   End
   Begin VB.CommandButton Command17 
      Caption         =   "short"
      Height          =   252
      Left            =   840
      TabIndex        =   15
      Top             =   2640
      Width           =   492
   End
   Begin VB.CommandButton Command16 
      Caption         =   "byte"
      Height          =   252
      Left            =   120
      TabIndex        =   14
      Top             =   2640
      Width           =   492
   End
   Begin VB.CommandButton Command15 
      Caption         =   "array"
      Height          =   252
      Left            =   3000
      TabIndex        =   12
      Top             =   1920
      Width           =   612
   End
   Begin VB.CommandButton Command14 
      Caption         =   "string"
      Height          =   252
      Left            =   2280
      TabIndex        =   11
      Top             =   1920
      Width           =   492
   End
   Begin VB.CommandButton Command13 
      Caption         =   "long"
      Height          =   252
      Left            =   1440
      TabIndex        =   10
      Top             =   1920
      Width           =   612
   End
   Begin VB.CommandButton Command12 
      Caption         =   "short"
      Height          =   252
      Left            =   720
      TabIndex        =   9
      Top             =   1920
      Width           =   612
   End
   Begin VB.CommandButton Command11 
      Caption         =   "byte"
      Height          =   252
      Left            =   120
      TabIndex        =   8
      Top             =   1920
      Width           =   492
   End
   Begin VB.CommandButton Command10 
      Caption         =   "array"
      Height          =   252
      Left            =   3000
      TabIndex        =   6
      Top             =   1200
      Width           =   612
   End
   Begin VB.CommandButton Command9 
      Caption         =   "string"
      Height          =   252
      Left            =   2280
      TabIndex        =   5
      Top             =   1200
      Width           =   612
   End
   Begin VB.CommandButton Command8 
      Caption         =   "long"
      Height          =   252
      Left            =   1560
      TabIndex        =   4
      Top             =   1200
      Width           =   612
   End
   Begin VB.CommandButton Command7 
      Caption         =   "short"
      Height          =   252
      Left            =   840
      TabIndex        =   3
      Top             =   1200
      Width           =   612
   End
   Begin VB.CommandButton Command6 
      Caption         =   "byte"
      Height          =   252
      Left            =   120
      TabIndex        =   2
      Top             =   1200
      Width           =   612
   End
   Begin VB.Label Label4 
      Caption         =   "Properties"
      Height          =   252
      Left            =   120
      TabIndex        =   13
      Top             =   2280
      Width           =   1212
   End
   Begin VB.Label Label3 
      Caption         =   "In Out Parameter"
      Height          =   252
      Left            =   120
      TabIndex        =   7
      Top             =   1560
      Width           =   1332
   End
   Begin VB.Label Label2 
      Caption         =   "Out Parameter"
      Height          =   252
      Left            =   120
      TabIndex        =   1
      Top             =   840
      Width           =   1332
   End
   Begin VB.Label Label1 
      Caption         =   "In Parameter"
      Height          =   252
      Left            =   240
      TabIndex        =   0
      Top             =   120
      Width           =   1212
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private objServiceManager
Private objCoreReflection
Private objOleTest

Private Sub ctrInParamStart_Click()
Dim ret As Variant

' In Parameter, simple types
'============================================
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
ret = objOleTest.in_methodChar("A")
Debug.Print TypeName(ret) & " " & CStr(ret)
ret = objOleTest.in_methodAny("This is a String in an any")
Debug.Print TypeName(ret) & " " & CStr(ret)

Call objOleTest.in_methodAll(10, 10.1, 10.111, True, 10, 11, 12, 13, _
            "A String", "A", "A String in an Any")
            
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

outString = "this is an in String"
objOleTest.testout_methodChar outString
Debug.Print "out char " & CStr(outString)
outString = "this is another in out string"
objOleTest.testout_methodAny outString
Debug.Print "out Any " & CStr(outString)





Dim var As Boolean
var = True
Debug.Print CStr(var)


End Sub

Private Sub Form_Load()
    Set objServiceManager = CreateObject("com.sun.star.ServiceManager")
    Set objCoreReflection = objServiceManager.createInstance("com.sun.star.reflection.CoreReflection")

    Set objOleTest = objServiceManager.createInstance("oletest.OleTest")
    Debug.Print TypeName(objOleTest)
End Sub

Private Sub Text1_Change()

End Sub
