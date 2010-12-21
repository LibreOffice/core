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
'\******************************************************************************
testcase tiDatei_Fuer_Anordnen_Erstellen

    dim sTemp as string
    dim i as integer
    dim zaehler as integer
    printlog "This function creates the document, that will be used as reference and working object inthe following cases :-)"
    'Dim Datei ' somewhere global :-) defined in calling sub! :sub im_107c_
    printlog "new document "
    call hNewDocument
    if gApplication = "DRAW" then
        kontext "DocumentDraw"
    else
        kontext "DocumentImpress"
        DocumentImpress.UseMenu
        hMenuSelectNr(4)
        hMenuSelectNr(13)
        call sSelectEmptyLayout
    end if
    
    								
    if Dir(Datei)<>"" then app.Kill(Datei) ' kill old file
    printlog "create 3 overlapping objects "
    for Zaehler = 1 to 3
    Kontext
    Select Case Zaehler
        Case 1:	WL_SD_Wuerfel
            printlog "cubicel 20,5, 80,60"
            gMouseMove (20,5, 80,60) 
            Printlog " Object start ---------------------- "+ Zaehler +" cubicel "
        Case 2:	WL_SD_RechteckVoll
            printlog "rectangel 40,35, 90,90"
            gMouseMove (40,35, 90,90) 
           Printlog " Object start ---------------------- "+ Zaehler +" rectangel "
        Case 3: WL_SD_Torus
            printlog "donut 1,35, 60,95 "
            gMouseMove (1,35, 60,95) 
            Printlog " Object start ---------------------- "+ Zaehler +" donut "
    End Select
      sleep (1)
      printlog "assign different area properties for created objects (use rnd function)"
      FormatArea										
      sleep (1)
      kontext
    Select Case Zaehler
        Case 1:	active.SetPage TabFarbverlaeufe
                    kontext "TabFarbverlaeufe"
                    Tabelle.Select Int((Tabelle.GetItemCount * Rnd) + 1)
                printlog "test tbo-------------------------------------------------color of object : "+Tabelle.GetSelText
                    TabFarbverlaeufe.OK
        Case 2:	active.SetPage TabSchraffuren
                    kontext "TabSchraffuren"
                    Tabelle.Select Int((Tabelle.GetItemCount * Rnd) + 1)
                printlog "test tbo-------------------------------------------------color of object : "+Tabelle.GetSelText
                    TabSchraffuren.OK
        Case 3:	active.SetPage TabFarben
                    kontext "TabFarben"
                    Farbe.Select Int((Farbe.GetItemCount * Rnd) + 1)
                printlog "test tbo-------------------------------------------------color of object : "+Farbe.GetSelText
                    TabFarben.OK
    End Select
    sleep 1
    printlog "save position and size values into varialbes "
    ContextPositionAndSize							
    sleep (1)
    kontext
    active.SetPage TabPositionAndSize
    kontext "TabPositionAndSize"
    printlog "the variables : Ueber_Text_[1-3] get set here ! with x position of object"
        Select Case Zaehler
            Case 1:	Ueber_Text_1 = PositionX.GetText : printlog "xpos: "+zaehler+": "+PositionX.GetText
            Case 2:	Ueber_Text_2 = PositionX.GetText : printlog "xpos: "+zaehler+": "+PositionX.GetText
            Case 3:	Ueber_Text_3 = PositionX.GetText : printlog "xpos: "+zaehler+": "+PositionX.GetText
        End Select
    TabPositionAndSize.OK
      gMouseClick 96,5
   next Zaehler
   sleep 1
   printlog "save document "
   call hFileSaveAsKill (Datei)							
   printlog "OK   saved at ", Datei
   sleep 1
   printlog "select in default order and take Position X in mind ;-)"
   printlog "+ has to be from bottom to top: cubicel, rectangel, donut"
    sPrintCheckOrder (TRUE)
    printlog "close document "
    call hCloseDocument									
    
endcase 'tiDatei_Fuer_Anordnen_Erstellen
'-------------------------------------------------------------------------------
testcase tdContextSendBackward

   dim sTemp as string
   printlog "open created document 'with 3 figures'"
   if (hFileOpen (Datei)) then
      sleep 10
     sPrintCheckOrder (TRUE)
      printlog "select the middle object: rectangle, with keys: [TAB],[TAB]"
      hTypeKeys ("<TAB><TAB>")
      printlog "Send Backward / [Strg]+[-] -> one level more to the back"
      WL_TB_ANORDNUNG_WeiterNachHinten
      printlog "+ has to be from bottom to top: rectangel cubicel donut"
      sleep 2
     sPrintCheckOrder
      printlog "deselect all"
      hTypeKeys ("<escape><escape>")
      sleep 2
      printlog "select the middle object: cubical, with keys: [TAB],[TAB]"
      hTypeKeys ("<TAB><TAB>")
      sleep 2
      sTemp = fGetPositionX
      if sTemp <> Ueber_Text_1 then
          warnlog "Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_1+";"
      end if
      printlog "close document"
      Call hCloseDocument
   endif
   sleep 2
   
endcase 'tdContextSendBackward
'-------------------------------------------------------------------------------
testcase tdContextBringForward

   dim sTemp as string
   printlog "open created document 'with 3 figures'"
   if (hFileOpen (Datei)) then
      sleep 10
     sPrintCheckOrder (TRUE)
      printlog "select the middle object: rectangle, with keys: [TAB],[TAB]"
      hTypeKeys ("<TAB><TAB>")
      printlog "Bring Forward / [Strg]+[+] -> one level more to the front"
      WL_TB_ANORDNUNG_WeiterNachVorn
      printlog "has to be from bottom to top: cubicel donut rectangel"
      sleep 1
     sPrintCheckOrder
      printlog "select the middle object: donut, with keys: [TAB],[TAB]"
      hTypeKeys ("<TAB><TAB>")
      sleep 2
      sTemp = fGetPositionX
      if sTemp <> Ueber_Text_3 then
          warnlog "Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_3+";"
      end if
      sleep 1
      printlog "close document"
      Call hCloseDocument
   endif
   
endcase 'tdContextBringForward
'-------------------------------------------------------------------------------
testcase tdContextSendToBack

   dim sTemp as string
   printlog "open created document 'with 3 figures'"
   if (hFileOpen (Datei)) then
      sleep 10
     sPrintCheckOrder (TRUE)
      printlog "select the top object: donut, with keys: [TAB],[TAB],[TAB]"
      hTypeKeys ("<TAB><TAB><TAB>")
      printlog "Send to Back / [Strg]+[Shift]+[-] -> backmost object"
      OL_DRAW_GanzNachHinten
      printlog "has to be from bottom to top: rectangel cubicel donut"
      sleep 3
     sPrintCheckOrder
      printlog "select the middle object: cubical, with keys: [TAB],[TAB]"
      hTypeKeys ("<TAB><TAB>")
      sleep 1
      sTemp = fGetPositionX
      if sTemp <> Ueber_Text_1 then
          warnlog "Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_1+";"
      end if
      sleep 3
      printlog "close document"
     Call hCloseDocument
   endif
   
endcase 'tdContextSendToBack
'-------------------------------------------------------------------------------
testcase tdContextBringToFront

   dim sTemp as string
   printlog "open created document 'with 3 figures'"
   if (hFileOpen (Datei)) then
      sleep 10
     sPrintCheckOrder (TRUE)
      printlog "select the bottom object: cubical, with keys: [TAB]"
      hTypeKeys ("<TAB>")
      printlog "Bring to Front / [Strg]+[Shift]+[+] -> frontmost object"
      OL_DRAW_GanzNachVorn
      printlog "has to be from bottom to top: rectangel, donut, cubicel"
     sPrintCheckOrder
      printlog "select the middle object: donut, with keys: [TAB],[TAB]"
      hTypeKeys ("<TAB><TAB>")
      sleep 1
      sTemp = fGetPositionX
      if sTemp <> Ueber_Text_3 then
          warnlog "Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_3+";"
      end if
      sleep 3
      printlog "close document"
     Call hCloseDocument
   endif
   
endcase 'tdContextBringToFront
'-------------------------------------------------------------------------------
testcase tdContextInFrontOfObject

   dim sTemp as string
   printlog "open created document 'with 3 figures'"
   if (hFileOpen (Datei)) then
      sleep 10
     sPrintCheckOrder (TRUE)
      printlog "select the top object: donut, with keys: [TAB],[TAB],[TAB]"
      hTypeKeys ("<TAB><TAB><TAB>")
      printlog "In Front of object"
      OL_DRAW_VorDasObjekt
      printlog "left click the cubicel with the mouse"
      gMouseClick 50,5
      printlog "has to be from bottom to top: cubicel, donut, rectangel"
     sPrintCheckOrder
      sleep 1
      printlog "select the middle object: donut, with keys: [TAB],[TAB]"
      hTypeKeys ("<TAB><TAB>")
      sTemp = fGetPositionX
      if sTemp <> Ueber_Text_3 then
          printlog "DISABLED the WARNLOG, because mouseaction is not always the same :-( : Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_3+";"
      end if
      printlog "close document"
      Call hCloseDocument
   endif
   
endcase 'tdContextInFrontOfObject
'-------------------------------------------------------------------------------
testcase tdContextBehindObject

   dim sTemp as string
   printlog "open created document 'with 3 figures'"
   if (hFileOpen (Datei)) then
      sleep 10
     sPrintCheckOrder (TRUE)
      printlog "select the top object: donut, with keys: [TAB],[TAB],[TAB]"
      hTypeKeys ("<TAB><TAB><TAB>")
      printlog "Behind object"
      OL_DRAW_HinterDasObjekt
      sleep 1
      printlog "left click the cubicel with the mouse"
      gMouseClick 50,5
      printlog "has to be from bottom to top: donut, cubicel, rectangel"
     sPrintCheckOrder
      sleep 1
      printlog "select the bottom object: donut, with keys: [TAB]"
      hTypeKeys ("<TAB>")
      sTemp = fGetPositionX
      if sTemp <> Ueber_Text_3 then
          printlog "DISABLED the WARNLOG, because mouseaction is not always tthe same  Arrangement is wrong; is: "+sTemp+"; should: "+Ueber_Text_3+";"
      end if
      printlog "close document"
      Call hCloseDocument
   endif
   
endcase 'tdContextBehindObject
'-------------------------------------------------------------------------------
testcase tdContextReverse

   dim sTemp as string
   dim sTemp2 as string
   dim i as integer
   printlog "open created document 'with 3 figures'"
   if (hFileOpen (Datei)) then
      sleep 10
     sPrintCheckOrder (TRUE)
      printlog "Edit->Select All"
      EditSelectAll
      sleep 3
      printlog "Reverse"
      WL_TB_ANORDNUNG_Vertauschen
     sPrintCheckOrder
      printlog "select in default order and take Position X in mind ;-)"
      hTypeKeys ("<escape><escape>")
      for i = 1 to 3
         hTypeKeys ("<TAB>")
         sTemp = fGetPositionX
            Select Case i
                Case 1:	sTemp2 = Ueber_Text_3
                Case 2:	sTemp2 = Ueber_Text_2
                Case 3:	sTemp2 = Ueber_Text_1
            End Select
         if sTemp <> sTemp2 then
            warnlog " - " + i + " Arrangement is wrong; is: "+sTemp+"; should: "+sTemp2+";"
         end if
      next i
      printlog "close document"
      Call hCloseDocument									
   endif
   
endcase 'tdContextReverse
'-------------------------------------------------------------------------------
