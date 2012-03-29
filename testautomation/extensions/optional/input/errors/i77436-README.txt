' *************************************************************
'  
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License")' you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'  
' *************************************************************
This macro is used to verify the correct handling of the extension.
Install the extension, copy the code to the basic-ide, run it. You should get two messageboxes listing some interfaces.
This macro only works in conjunction with the sample extension in this directory.
Note that the testtool uses the same macro from framework/tools/input/macros.txt otherwise we would have to implement yet
another way of entering macros to the IDE.




' The sample macro
Sub Main
'test service
o= createUnoService("TestNamesComp")
msgbox o.dbg_supportedInterfaces

'test singleton
ctx = getDefaultContext
factory = ctx.getValueByName("org.openoffice.test.Names")
msgbox o.dbg_supportedInterfaces

End Sub
