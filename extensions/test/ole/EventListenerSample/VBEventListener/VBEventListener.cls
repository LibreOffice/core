VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "VBEventListener"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit
Private interfaces(0) As String
Private bDisposingCalled As Boolean
Private bQuiet As Boolean

Public Property Get Bridge_ImplementedInterfaces() As Variant
    Bridge_ImplementedInterfaces = interfaces
End Property

Private Sub Class_Initialize()
interfaces(0) = "com.sun.star.lang.XEventListener"
bDisposingCalled = False
bQuiet = False
End Sub

Private Sub Class_Terminate()
   On Error Resume Next
   Debug.Print "Terminate VBEventListener"
End Sub

Public Sub disposing(ByVal source As Object)
    If bQuiet = False Then
    MsgBox "disposing called"
    End If
    bDisposingCalled = True
End Sub

Public Sub setQuiet(quiet As Boolean)
    bQuiet = quiet
End Sub

Public Sub resetDisposing()
    bDisposingCalled = False
End Sub

Public Function disposingCalled()
    disposingCalled = bDisposingCalled
End Function
