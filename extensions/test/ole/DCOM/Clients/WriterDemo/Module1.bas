Attribute VB_Name = "Module1"
Option Explicit

Sub main()
    Dim obj As Object
    Set obj = CreateObject("dcomtest.writerdemo.wsc")
    obj.run
End Sub
