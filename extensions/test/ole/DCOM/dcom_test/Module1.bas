rem
rem This file is part of the LibreOffice project.
rem
rem This Source Code Form is subject to the terms of the Mozilla Public
rem License, v. 2.0. If a copy of the MPL was not distributed with this
rem file, You can obtain one at http://mozilla.org/MPL/2.0/.
rem
rem This file incorporates work covered by the following license notice:
rem
rem   Licensed to the Apache Software Foundation (ASF) under one or more
rem   contributor license agreements. See the NOTICE file distributed
rem   with this work for additional information regarding copyright
rem   ownership. The ASF licenses this file to you under the Apache
rem   License, Version 2.0 (the "License"); you may not use this file
rem   except in compliance with the License. You may obtain a copy of
rem   the License at http://www.apache.org/licenses/LICENSE-2.0 .
rem
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
