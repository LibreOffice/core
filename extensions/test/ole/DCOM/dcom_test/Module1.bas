Attribute VB_Name = "Module1"
Option Explicit

Sub main()

MsgBox "hallo"

'The service manager is always the starting point
'If there is no office running then an office is started up
Dim objServiceManager As Object
Set objServiceManager = CreateObject("com.sun.star.ServiceManager")

'Create the CoreReflection service that is later used to create structs
Set objCoreReflection = objServiceManager.createInstance("com.sun.star.reflection.CoreReflection")

'Create the Desktop
Set objDesktop = objServiceManager.createInstance("com.sun.star.frame.Desktop")

'Open a new empty writer document

Set objCoreReflection = objServiceManager.createInstance("com.sun.star.reflection.CoreReflection")
'get a type description class for Size
Set propClass = objCoreReflection.forName("com.sun.star.beans.PropertyValue")

Dim prop
propClass.CreateObject prop
prop.Name = "Hidden"
prop.Value = True

'create the actual object
Dim args(0)
Set args(0) = prop

Dim args2()
'Set objDocument= objDesktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, args)
Set objDocument = objDesktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, args2)

End Sub
