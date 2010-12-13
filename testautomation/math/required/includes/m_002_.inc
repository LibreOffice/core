'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* Owner : thorsten.bosbach@oracle.com
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

