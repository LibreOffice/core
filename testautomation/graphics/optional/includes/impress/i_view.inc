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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'*********************************************************************
' #1 tAnsichtHintergrundNotizen
' #1 tViewWorkspaceHandoutView
' #1 tViewWorkspaceOutlineView
' #1 tViewWorkspaceDrawingView
' #1 tAnsichtEbene
'\********************************************************************

'Datei$ = ConvertPath (gOfficePath + "user\work\test.sxi")

testcase tAnsichtHintergrundNotizen
    Printlog "- View background notes"
    printlog "new document"
    Call hNewDocument
    try
        printlog "view background notes"
        ViewNotes					
        sleep 3
    catch
        Warnlog  "View-Notes could not be accessed"
    endcatch
    ViewWorkspaceDrawingView
    sleep 1
    printlog "close document"
    Call hCloseDocument						
    sleep 1
endcase 'tAnsichtHintergrundNotizen

'-------------------------------------------------------------------------------

testcase tViewWorkspaceHandoutView
    Printlog "- View background handout"
    printlog "new document"
    Call hNewDocument						
    sleep 2
    Kontext "DocumentImpress"
    DocumentImpress.UseMenu
    hMenuSelectNr (5)
    hMenuSelectNr (13)
    Printlog "  ViewWorkspaceHandoutView"
    ViewWorkspaceHandoutView						
    sleep 2
    Kontext "Tasks"
    sleep 2
    if LayoutsPreview.GetItemCount <> 6 Then
        Warnlog "  this doesn't seem to be a handout ?: should be: 6 layouts in the Preview window, but are: " + LayoutsPreview.GetItemCount
    else
        Printlog "  ViewWorkspaceHandoutView works"
    end if
    Printlog "ViewWorkspaceHandoutView closed"
    printlog "back to view background drawing"
    ViewWorkspaceDrawingView
    printlog "close document"
    Call hCloseDocument						
endcase 'tViewWorkspaceHandoutView

'-------------------------------------------------------------------------------

testcase tViewWorkspaceOutlineView
    qaerrorlog "Outcommenting tViewWorkspaceOutlineView due to unreability. - FHA"
    goto endsub
   dim sFile as string
   dim i as integer

   printlog "load a document with heading styles in writer and use the function 'writer:'File->Send->Outline to presentation' to get it into the presentation"
   ' all 'heading 1' are new slides

   printlog "document used : graphics\\required\\input\\outlinefromwriter.sxw -> you get into writer"
   sFile = ConvertPath (gTesttoolPath + "graphics\required\input\outlinefromwriter.sxw")
   hDateiOeffnen (sFile)
   sleep 10
   printlog "with 'File->Send->Outline to presentation' you get into impress"
   FileSendOutlineToPresentation
   sleep 10
   Printlog "printlog View->Workspace->Outline View"
   ViewWorkspaceOutlineView
   sleep 1
   printlog "go to the line with the 'I' (via 'search & replace' function)"
   hTypeKeys "<STRG HOME>"
   EditSelectAll
   hFindeImDokument ("I")

   printlog "expected level/style: 'Outline 8'"
   printlog "get last character from selected style in stylist (expect a number of the outline(heading level)"
   fGetPresentationStyle ("8")

   printlog "type keys [home] [tab] to go one level down"
   Kontext "DocumentImpressOutlineView"
   DocumentImpressOutlineView.TypeKeys "<Home><Tab>"

   printlog "expected level/style: 'Outline 9'"
   fGetPresentationStyle ("9")

   Kontext "DocumentImpressOutlineView"
   printlog "Edit - Select All to go select the whole text"
   EditSelectAll
   printlog "type keys [shift]+[tab] eight times, to get all lines to the highest, except the one with the 'I'"
   DocumentImpressOutlineView.TypeKeys ("<Shift Tab>", 8)
    
hPrepareSearchBUG
    hTypeKeys "<STRG HOME>"
    EditSelectAll
    hFindeImDokument ("I")
    printlog "expected level/style: 'Outline 1"
    fGetPresentationStyle ("1")
    
    printlog "type keys [shift]+[tab] to get the last line to the highest level, too"
    Kontext "DocumentImpressOutlineView"
    DocumentImpressOutlineView.TypeKeys ("<Shift Tab>", 1)
    printlog "expected level/style: 'Title'"
    fGetPresentationStyle (0)
    
    Kontext "DocumentImpressOutlineView"
    DocumentImpressOutlineView.TypeKeys ("<escape>", 1)
    Printlog "call 'Edit->Undo' nine times"
    for i = 1 to 9
      editundo
    next i

hPrepareSearchBUG
    hFindeImDokument ("I")
    printlog "expected level/style: 'Outline 9'"
    fGetPresentationStyle (9)
    
    Printlog "call 'Edit->Redo' nine times "
    for i = 1 to 9
      editredo
    next i
    try
      editredo
      Warnlog "NOT OK :-("
    catch
      Printlog "OK"
    endcatch

hPrepareSearchBUG
    hTypeKeys "<STRG HOME>"
    EditSelectAll
    hFindeImDokument ("I")
    printlog "expected level/style: 'Title'"
    fGetPresentationStyle (0)

    Printlog "call 'Edit->Undo' nine times"
    for i = 1 to 9
        editundo
    next i

hPrepareSearchBUG
    hTypeKeys "<STRG HOME>"
    EditSelectAll
    hFindeImDokument ("I")
    printlog "expected level/style: 'Outline 9"
    fGetPresentationStyle (9)

    printlog "type keys [strg]+[A]"
    Kontext "DocumentImpressOutlineView"
    EditSelectAll
    printlog "type keys [strg]+[X]"
    DocumentImpressOutlineView.TypeKeys "<mod1 X>"

    kontext
    if messagebox.exists (5) then
        Printlog "active about deleting pages with graphic objects: '" + messagebox.GetText + "'"
        Messagebox.ok
    else
        Warnlog "messagebox missing ! (about deleting pages with graphic objects:)"
    endif

   printlog "'Edit->Undo'"
    editundo
   printlog "'Edit->Redo'"
    editredo
   printlog "'Edit->Undo'"
    editundo

   printlog "type keys [strg]+[A]"
   Kontext "DocumentImpressOutlineView"
   EditSelectAll
   printlog "type keys [strg]+[X]"
   DocumentImpressOutlineView.TypeKeys "<mod1 X>"

    kontext
    if messagebox.exists (5) then
        Printlog "active about deleting pages with graphic objects: '" + messagebox.GetText + "'"
        Messagebox.ok
    else
        Warnlog "messagebox missing ! (about deleting pages with graphic objects:)"
    endif

    printlog "type keys [strg]+[V]"
    Kontext "DocumentImpressOutlineView"
    DocumentImpressOutlineView.TypeKeys "<mod1 V>"

    printlog "'Edit->Undo'"
    editundo
    printlog "'Edit->Redo'"
    editredo

    printlog "type keys [strg]+[End]"
    Kontext "DocumentImpressOutlineView"
    DocumentImpressOutlineView.TypeKeys "<mod1 End>"
    printlog "type keys [strg]+[V]"
    DocumentImpressOutlineView.TypeKeys "<mod1 V>"
    printlog "type keys [strg]+[A]"
    EditSelectAll
    printlog "type keys [delete]"
    DocumentImpressOutlineView.TypeKeys "<delete>"

    kontext
    if messagebox.exists (5) then
        Printlog "active-box about deleting pages with graphic objects: '" + messagebox.GetText + "'"
        Printlog "SAY CANCEL TO DIALOG!"
        Messagebox.Cancel
    else
        Warnlog "messagebox missing ! (about deleting pages with graphic objects:)"
        endif

    printlog "type keys [strg]+[A]"
    Kontext "DocumentImpressOutlineView"
    EditSelectAll
    try
        editcopy
        Printlog "OK"
    catch
        Warnlog "error 4711 TBO :-("
    endcatch
    
    printlog "type keys [delete]"
    Kontext "DocumentImpressOutlineView"
    DocumentImpressOutlineView.TypeKeys "<delete>"

    kontext
    if messagebox.exists (5) then
      Printlog "Active-box about deleting pages with graphic objects: '" + messagebox.GetText + "'"
      Messagebox.ok
    else
      Warnlog "Messagebox missing ! (about deleting pages with graphic objects:)"
    endif
    
    Kontext "DocumentImpressOutlineView"
    EditSelectAll
    try
      editcopy
      Warnlog "Didn't work!"
      Printlog "'" + getclipboard() + "'"
    catch
      Printlog "OK"
    endcatch
    
    ViewWorkspaceDrawingView
    printlog "close document"
    Call  hCloseDocument						
endcase 'tViewWorkspaceOutlineView

'-------------------------------------------------------------------------------

testcase tViewWorkspaceDrawingView
    Printlog "- View/Background-drawing"
    printlog "new document"
    Call hNewDocument						
    sleep 2
    printlog "view background drawing"
    ViewDrawing						
    try
        printlog "create rectangle"
        hRechteckErstellen ( 20,20,40,40 )			
        sleep 1
        Printlog "  - rectangle created"
    catch
        Warnlog "  Rectangle could not be created"
    endcatch
    printlog "close document"
    Call hCloseDocument		
endcase 'tViewWorkspaceDrawingView

'-------------------------------------------------------------------------------

testcase tAnsichtEbene
    printLog "- View/Layer"							

    printlog "new document-autopilot"
    FileAutopilotPresentation					
    Kontext "AutopilotPraesentation1"
    sleep 5
    AutopilotPraesentation1.Ok
    sleep 2
    sleep 2
    printlog "back to view slide"
    ViewSlide							
    Sleep 2

    printlog "close document"
    Call  hCloseDocument						

endcase 'tAnsichtEbene

'-------------------------------------------------------------------------------
