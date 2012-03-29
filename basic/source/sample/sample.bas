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
' Sample-Programm fuer Sample-Objekte

Sub Main
	Dim Sample As SampleObject
	Dim Element1 As Object, Element2 As Object
	Set Element1 = Sample!Create "Objekt"
	Set Element2 = Sample.Create "Objekt"
	Element1 = "Element 1"
	Element2 = "Element 2"
	For i = 0 to 1
		Print Sample.Objekt( i )
	Next
	'Test der Event-Methode im Sample-Objekt
	Sample.Event "Bang"
End Sub

Sub Bang
	print "Sample-Callback: BANG!"
End Sub




















