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
'* short description :
'*
'\*****************************************************************

sub M_002_
   printlog Chr(13) + "--  Edit Menu  m_002_ --"

   Call tmEditUndoRedo
   Call tmEditAllClipboard
   Call tmEditMarker
   Call tmEditFailure
end sub

testcase tmEditUndoRedo
   '/// Edit / Undo and Edit / Redo ///
   '/// open application ///'
   hNewDocument
   kontext "DocumentMath"
   '/// type something into the document ///'
   call SchreibenInMathdok "a over b"
   try
      '/// Edit->Undo ///'
      EditUndo
   catch
      warnlog "edit->undo doesn't work :-("
   endcatch
   Sleep 2
   '/// Edit->Redo ///'
   EditRedo
   Sleep 2
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmEditAllClipboard
   '/// open application ///'
   hNewDocument
   kontext "DocumentMath"
'   DocumentMath.TypeKeys Eingabe not working
'   kontext "Commands"     also not working -- only way to type smth is clippboard
'   Commands.TypeKeys Eingabe

   '/// type something into the document ///'
   call SchreibenInMathdok "a over b"
   '/// Edit->Select All ///'
   EditSelectAllMath
   Sleep 2
   '/// Edit->Cut ///'
   EditCut
   Sleep 2
   '/// Edit->Paste ///'
   EditPaste
   Sleep 2
   '/// Edit->Paste ///'
   EditPaste
   Sleep 2
   '/// Edit->Select All ///'
   EditSelectAllMath
   Sleep 2
   '/// Edit->Copy ///'
   EditCopy
   Sleep 2
   '/// Edit->Paste ///'
   EditPaste
   Sleep 2
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmEditMarker
   '/// open application ///'
   hNewDocument
   '/// type something into the document ///'
   SchreibenInMathdok "Nonsense"
   '/// Edit->Previous Marker ///'
   EditPreviousMarker
   '/// Edit->Previous Marker ///'
   EditPreviousMarker
   Sleep 2
   '/// Edit->Next Marker ///'
   EditNextMarker
   Sleep 2
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tmEditFailure
   '/// open application ///'
   hNewDocument
   '/// type something into the document ///'
   SchreibenInMathdok "Nonsense"
   '/// Edit->Next Error ///'
   EditNextError
   '/// Edit->Next Error ///'
   EditNextError
   Sleep 2
   '/// Edit->Previous Error ///'
   EditPreviousError
   Sleep 2
   '/// close application ///'
   Call hCloseDocument
endcase

