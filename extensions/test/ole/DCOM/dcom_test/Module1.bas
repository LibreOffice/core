rem *************************************************************
rem  
rem  Licensed to the Apache Software Foundation (ASF) under one
rem  or more contributor license agreements.  See the NOTICE file
rem  distributed with this work for additional information
rem  regarding copyright ownership.  The ASF licenses this file
rem  to you under the Apache License, Version 2.0 (the
rem  "License"); you may not use this file except in compliance
rem  with the License.  You may obtain a copy of the License at
rem  
rem    http://www.apache.org/licenses/LICENSE-2.0
rem  
rem  Unless required by applicable law or agreed to in writing,
rem  software distributed under the License is distributed on an
rem  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
rem  KIND, either express or implied.  See the License for the
rem  specific language governing permissions and limitations
rem  under the License.
rem  
rem *************************************************************
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
