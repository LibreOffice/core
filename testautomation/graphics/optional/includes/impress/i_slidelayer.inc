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

'  Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
'  Inhalt.SetText ConvertPath (gTesttoolPath + "graphics\required\input\leer.sxd") 

'-------------------------------------------------------------------------------
testcase tiInsertExpandSlide

    dim i,x as integer
    dim sTemp as string
    dim b116350 as boolean

    printlog "open application "
    Call hNewDocument
    Call sSelectEmptyLayout
    printlog "create presupposition"
    printlog "View->Master View->Outline View "
    ViewWorkspaceOutlineView
    Sleep 1
    Kontext "DocumentImpressOutlineView"
    'create a slide with some levels...and some more slides with just one level...  "
    'it should look like this: "
    '-------------------------------------------------- "
    'Slide 1"
    ' - A "
    '- - B"
    ' - - - C"
    ' - - - - D"
    ' - - - - - - F"
    ' - B"
    ' - C"
    ' - D"
    ' Slide 2 "
    ' Slide 3 "
    ' Slide 4 "
    ' Slide 5 "
    '-------------------------------------------------- "
    DocumentImpressOutlineView.TypeKeys "Slide 1"
    for i = 1 to 6
     DocumentImpressOutlineView.TypeKeys "<return><tab>"
     DocumentImpressOutlineView.TypeKeys chr(64+i)
    next i
    DocumentImpressOutlineView.TypeKeys "<return>"
    DocumentImpressOutlineView.TypeKeys ("<shift tab>",5)
    for i = 1 to 3
     DocumentImpressOutlineView.TypeKeys chr(65+i)
     DocumentImpressOutlineView.TypeKeys "<return>"
    next i
    DocumentImpressOutlineView.TypeKeys ("<shift tab>")
    for i = 2 to 5
     DocumentImpressOutlineView.TypeKeys "Slide " + i
     DocumentImpressOutlineView.TypeKeys "<return>"
    next i
    DocumentImpressOutlineView.TypeKeys "<backspace>"
    printlog " View->Master View->Drawing View "
    ViewWorkspaceDrawingView
    Sleep 1
    printlog "there have to be 5 slides now "
    fGetSlideCount (5)

    printlog "goto the first slide "
    hTypeKeys "<home>"
    printlog "Insert->Expand Slide"
    InsertExpandSlide
      ' usually the content of the new pages is the same as the curren, there is a slide created for every part in the first outline level (gliederungsebene)
      Sleep 1
      if (8 = fGetSlideCount (8)) then
         printlog "Insert Expand Slide accomplished correctly"
      else
         warnLog "Bug with Insert Expand Slide"
      endif
      printlog " check slide content "
      hTypeKeys ("<TAB><F2>"
      EditSelectAll
      EditCopy
      sTemp = GetClipboard()
      if (sTemp <> "A") then
         Warnlog "wrong slide content, expected: '" + "Slide 1" + "'; but got: '" + sTemp + "'"
      else
         Printlog "OK :-)"
      endif
      for i = 2 to 4
         hTypeKeys "<pagedown>"
         hTypeKeys ("<TAB><F2>")
         EditSelectAll
         EditCopy
         sTemp = GetClipboard()
         if (sTemp <> chr(64+i)) then
            Warnlog "Wrong slide content, expected: '" + chr(64+i) + "'; but got: '" + sTemp + "'"
         else
            Printlog "OK :-)"
         endif
      next i
      for i = 2 to 5
         hTypeKeys "<pagedown>"   'If the first slide existed, this is where it would have been.
         hTypeKeys ("<TAB><F2>")
         EditSelectAll
         EditCopy
         sTemp = GetClipboard()
         if (sTemp <> ("Slide "+i)) then
            Warnlog "Wrong slide content, expected: '" + "Slide "+i + "'; but got: '" + sTemp + "'"
         else
            Printlog "OK :-)"
         endif
      next i

      hTypeKeys "<escape><home>"
   printlog "Insert->Expand Slide"
   InsertExpandSlide
      printlog "And since the Second slide now should be named B, we check that first "
      hTypeKeys ("<TAB><F2>")
      EditSelectAll
      try
         EditCopy
      catch
         printlog "Something wrong when trying to select Slide-name."
      endcatch
      sTemp = GetClipboard()
      if (sTemp <> "B") then
         Warnlog "Wrong slide content, expected: 'B'; but got: '" + sTemp + "'"
      else
         Printlog "OK :-)"
      endif
      hTypeKeys "<pagedown>"
      hTypeKeys ("<TAB><F2>")
      EditSelectAll
      for i = 2 to 4
         try
            EditCopy
         catch
            printlog "Something wrong when trying to select Slide-name."
         endcatch
         sTemp = GetClipboard()
         if (sTemp <> chr(64+i)) then
            Warnlog "Wrong slide content, expected: '" + chr(64+i) + "'; but got: '" + sTemp + "'"
         else
            Printlog "OK :-)"
         endif
         hTypeKeys "<pagedown>"
         hTypeKeys ("<TAB><F2>")
         EditSelectAll
	 next i
      for i = 2 to 5
         EditCopy
         sTemp = GetClipboard()
         if (sTemp <> ("Slide "+i)) then
            Warnlog "Wrong slide content, expected: '" + "Slide "+i + "'; but got: '" + sTemp + "'"
         else
            Printlog "OK :-)"
         endif
         hTypeKeys "<pagedown>"   'If the first slide existed, this is where it would have been.
         hTypeKeys ("<TAB><F2>")
         EditSelectAll
	 next i

   printlog "close application "
   Call  hCloseDocument
endcase 'tiInsertExpandSlide

'-------------------------------------------------------------------------------
testcase tiInsertSummarySlide

    dim i as integer
    dim sTemp as string
    dim saText (10) as string

    for i = 1 to 7
        saText (i) = chr(64+i)
    next i
    saText (5) = ""
    saText (7) = ""

    printlog "Open application."
    Call hNewDocument

    printlog "Presupposition : on the slide has to be something from the outline view AND i have to be on such a slide!"
    printlog "View->Master View->Outline View."
    ViewWorkspaceOutlineView
    Sleep 1
    Kontext "DocumentImpressOutlineView"
    DocumentImpressOutlineView.TypeKeys "<home>"
    printlog "+ create 7 slides like this:"
    printlog "+--------------------------------------------------"
    printlog "+ A "
    printlog "+ B"
    printlog "+ C"
    printlog "+ D"
    printlog "+  "
    printlog "+ F"
    printlog "+  "
    printlog "+--------------------------------------------------"
    for i = 1 to 7
        DocumentImpressOutlineView.TypeKeys (saText(i) + "<return>")
    next i

    printlog "View->Master View->Drawing View ."
    ViewWorkspaceDrawingView
    Sleep 1
    printlog "Goto first slide by typing [home]"
    hTypeKeys "<Home>"
    printlog "test menue entries"
    printlog "Insert->Summery Slide."
    try
        InsertSummerySlide
    catch
        warnlog "InsertSummerySlide didnt work. Check why."
    endcatch
    Sleep 1
    printlog "slide is appended after the last slide"
    printlog "the content of the slide depends on from which slide the function was called!"
    printlog "it is only from the actual slide to the end of the presentation"
    printlog "it is aloowed to have a slide without the outline stuff inbetween"
    printlog "check slide content"
    hTypeKeys "<Tab>"
    wait 50
    htypekeys "<Tab>"
    wait 50
    htypekeys "<F2>"
    wait 50
    htypekeys "<mod1 Home>"
    wait 50
    for i = 1 to 7
        hTypeKeys "<Shift End>"
        EditCopy
        sTemp = GetClipboardText
        if (saText(i) <> sTemp) then
            if ((i=5) OR (i=7)) then
                if (saText(6) <> sTemp) then
                    Warnlog "WRONG: is: '" + sTemp + "'; should: '" + saText(6) + "'"
                else
                    Printlog "is: '" + sTemp + "'; should: '" + saText(i) + "'"
                endif
             else
                Warnlog "WRONG: is: '" + sTemp + "'; should: '" + saText(i) + "'"
            endif
        else
            Printlog "is: '" + sTemp + "'; should: '" + saText(i) + "'"
        endif
        hTypeKeys "<down><home>"
    next i
    printlog "close application"
    Call  hCloseDocument
endcase 'tiInsertSummarySlide

'------------------------------------------------------------------------------
testcase tiFormatSeitenlayout

    qaerrorlog "outcommented due to bug"
    goto endsub
    
    Printlog "Format/Page Layout"
    printlog "new impress document "
     Call hNewDocument               

     printlog "format page "
     FormatPage                
     Kontext "Tasks"
     sleep 5
     printlog "get page name "
     SetClipboard LayoutsPreview.GetText
     printlog "change page name "
     SeitenName.SetText "Test"          
     sleep 1
     printlog "check background "
     if Hintergrund.IsChecked=False Then         
       Hintergrund.Check
     else
       Hintergrund.UnCheck
            if Hintergrund.IsChecked=False Then PrintLog " Background is deactivated"
     end if

     printlog "change status of objects on background "
     if ObjekteAufDemHintergrund.IsChecked = True Then       
       PrintLog " Object on background is activated"
       ObjekteAufDemHintergrund.UnCheck
       if ObjekteAufDemHintergrund.IsChecked = False Then PrintLog " Objects on background are deactivated"
     else
       ObjekteAufDemHintegrund.Check
       PrintLog " Object on background is activated"
     end if
     printlog "close dialog "
     Seitenlayout.OK              
     sleep 1
     printlog "reopen dialog "
     FormatPage                
     Kontext "LayoutsPreview"
     sleep 2
     if GetClipboardText <> SeitenName.GetText Then
       PrintLog " Page name is correct"
     else
       WarnLog " Page name is not correct"
     end if
     sleep 2
     printlog "assign different page layout "
     LayoutsPreview.TypeKeys "<TAB>"            
     LayoutsPreview.TypeKeys "<DOWN>" ,2
     LayoutsPreview.TypeKeys "<Return>"
     sleep 2
     Kontext
     if Active.Exists (5) then
        printlog "active (1): "+active.gettext
        Active.OK
     else
        kontext "Pagelayout_UndoDeleteWarning"
        Pagelayout_UndoDeleteWarning.OK
        printlog "Can't redo this action! Apply template? -> OK (1) "
        'DontShowAgain.check ' since it's a level2 test i COULD do this .-) TBO
     endif
     Kontext "DocumentImpress"
     sleep 3
     DocumentImpress.MouseDoubleClick 25,60
     sleep 3
     printlog "check in document if layout has changed "
     Kontext "GrafikEinfuegenDlg"          
     sleep 2
     if GrafikEinfuegenDlg.exists (5) then
        try
          DateiName.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\baer.tif")
          sleep 3
          Preview.Check
          sleep 3
          Oeffnen.Click
          sleep 3
          Kontext "Active"
           if Active.Exists Then
              Warnlog Active.GetText
              Active.OK
           end if
        catch
          WarnLog " LayoutsPreview did not work"
        endcatch
     else
        warnlog "Clicked beside :-( I tried to click into the dokument, to insert a graphic, but missed the right point :-("
     endif
      sleep 3
     Kontext "DocumentImpress"
     printlog "close document "
     Call hCloseDocument             
    endcase 'tiFormatSeitenlayout
    
'------------------------------------------------------------------------------
testcase t114174

    printlog "resulting from regression in #111862#  "
    dim sReference(2) as string
    dim sText(2) as string
    dim i as integer
    dim sFile as string
    
    if gApplication = "IMPRESS" then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if
    printlog "Used extension string is: " & ExtensionString
    sFile = convertPath(gOfficePath + "user/work/" + getTestcaseName & "." & ExtensionString)
    printlog "Filepath is: " & sFile
   if Dir (sFile) <> "" then kill (sFile)

    sReference(1) = "Koelle"
    sReference(2) = "Alaaf!"
    printlog "open application "
    Call hNewDocument

    printlog "Format->Modify Layout "
    FormatPage
    Kontext "Tasks"
        LayoutsPreview.TypeKeys "<TAB>"
        LayoutsPreview.TypeKeys "<Home><Right>"

      kontext "Pagelayout_UndoDeleteWarning"
      if Pagelayout_UndoDeleteWarning.exists then
         Pagelayout_UndoDeleteWarning.ok
      endif
    Kontext "Tasks"
    LayoutsPreview.TypeKeys "<Return>"
    printlog "(a) setting title and body text"
    for i = 1 to 2
        hTypeKeys("<Tab><F2>")
        hTypeKeys(sReference(i))
        EditSelectAll
        EditCopy
        sText(i) = getClipboardText()
        printlog sText(i)
        hTypeKeys("<Escape>")
        if (sText(i) <> sReference(i)) then
            warnlog "(1) selected text '" + sText(i) + "' is different from inserted text '" + sReference(i) + "'"
        endif
    next i

    printlog "deselect all by typing key 'ESCAPE' again "
    hTypeKeys("<Escape>")

    hFileSaveAsKill(sFile)
    sleep 5

    printlog "(b) checking title and body text"
    for i = 1 to 2
        hTypeKeys("<Tab><F2>")
        EditSelectAll
        try
            EditCopy
            sText(i) = getClipboardText()
        catch
            sText(i) = ""
        endcatch
        if (sText(i) <> sReference(i)) then
            warnlog "   (2) selected text '" + sText(i) + "' is different from inserted text '" + sReference(i) + "'"
        endif
        hTypeKeys("<Escape>")
    next i

    hCloseDocument()

    printlog "   (c) checking title and body text of saved document"
    hFileOpen(sFile)
    sleep 5
    for i = 1 to 2
        hTypeKeys("<Tab><F2>")
        EditSelectAll
        try
            EditCopy
            sText(i) = getClipboardText()
        catch
            sText(i) = ""
        endcatch
        hTypeKeys("<Escape>")
        if (sText(i) <> sReference(i)) then
            warnlog "   (3) selected text '" + sText(i) + "' is different from inserted text '" + sReference(i) + "'"
        endif
    next i

    hCloseDocument()
endcase 't114174

'------------------------------------------------------------------------------
testcase t111862

    qaerrorlog "outcommented due to bug"
    goto endsub
    dim sReference(2) as string
    dim sText(2) as string
    dim i as integer
    dim sFile as string

    sFile = convertPath(gOfficePath + "/user/work/" + getTestcaseName + ".sxi")

    if FileExists(sFile) then
        kill sFile
    endif

    sReference(1) = "Koelle"
    sReference(2) = "Alaaf!"
    printlog "open application "
    Call hNewDocument

    printlog "Format->Modify Layout "
    Formatpage
    Kontext "Tasks"
        LayoutsPreview.TypeKeys "<TAB>"
        LayoutsPreview.TypeKeys "<Home>"
        LayoutsPreview.TypeKeys "<Return>"
        Kontext "Pagelayout_UndoDeleteWarning"
           if Pagelayout_UndoDeleteWarning.exists(5) then
              Pagelayout_UndoDeleteWarning.OK
           endif
        Kontext "Tasks"
           LayoutsPreview.TypeKeys "<Right>"
           LayoutsPreview.TypeKeys "<Return>"
        Kontext "Pagelayout_UndoDeleteWarning"
           if Pagelayout_UndoDeleteWarning.exists(5) then
              Pagelayout_UndoDeleteWarning.OK
           endif
        Kontext "Tasks"
           LayoutsPreview.TypeKeys "<Right>"
           LayoutsPreview.TypeKeys "<Return>"
        Kontext "Pagelayout_UndoDeleteWarning"
           if Pagelayout_UndoDeleteWarning.exists(5) then
              Pagelayout_UndoDeleteWarning.OK
           endif

    EditSelectAll
    i=0
    gMouseMove2(30,1)
    while ((getMouseStyle <> 6) AND (i<100))
        inc (i)
        gMouseMove2(30,i)
        'printlog getMouseStyle
    wend

    gMouseMove(30, i, 70, i+20)

    hCloseDocument()
    
endcase 't111862
'------------------------------------------------------------------------------