'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
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
'*************************************************************************
'*
'* short description : Chart resource test - Menu WINDOW
'*
'************************************************************************
'*
' #1 tWindowNewWindowAndClose
'*
'\************************************************************************

testcase tWindowNewWindowAndClose
'///<u><b>Test Window::New Window and Window::Close Window</b></u>

'///<ul>
   '///+<li>Load simple test document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   sleep(2)
   '///+<li>Invoke Window::New Window</li>
   WindowNewWindow
   '///+<li>Invoke Window::Close Window</li>
   WindowCloseWindow

   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   '///+<li>Close document</li>///
   Call hCloseDocument
'///</ul>
endcase

