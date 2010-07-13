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
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'**************************************************************************************
' #1 tiEditUndo
' #1 tiEditRedo
' #1 tiEditClipboard
' #1 tiEditPasteSpecial_HTML
' #1 tdEditPasteSpecial_Text
' #1 tdEditPasteSpecial_Graphic
' #1 tdEditPasteSpecial_Circle
' #1 tiEditPasteSpecial
'\*************************************************************************************

testcase tiEditUndo

    Dim sDatei as string
    Dim Ueber_Text_1 as string
    
    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if
    
    sDatei = gTesttoolPath + "graphics\required\input\leer."+ExtensionString
    
    hFileOpen ConvertPath (sDatei)
    sleep 5
    printlog "Checking for readonly state. If so, make it editable."
    sMakeReadOnlyDocumentEditable

    hTypeKeys "<TAB><TAB>"
    sleep 3
    ContextPositionAndSize
    Kontext
    if Messagebox.exists (5) then
      Messagebox.SetPage TabPositionAndSize
    else
      print "what"
    endif
    Kontext "TabPositionAndSize"
    Ueber_Text_1=PositionX.GetText				'/// defining variable for original position of object ///'
    TabPositionAndSize.Cancel
    sleep 2
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    ContextArrangeBringBackward					'/// Bring Object backward ///'
    sleep 1
    gMouseClick 60,60
    sleep 1
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    hTypeKeys "<TAB><TAB>"
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    ContextPositionAndSize					'/// Open Position and Size dialog, checking position ///'
    Kontext
    Messagebox.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    if PositionX=0 Then Warnlog "  Command was not completely executed, and it could not be undone."
    TabPositionAndSize.Cancel
    sleep 2
    gMouseClick 60,60
    sleep 1
    
    EditUndo							'/// Edit Undo for BringBackward ///'
    Sleep 2
    gMouseClick 60,60
    sleep 1
    Kontext "Toolbar"
    Auswahl.Click
    hTypeKeys "<TAB><TAB>"
    sleep 1
    Kontext "Toolbar"
    Auswahl.Click
    ContextPositionAndSize					'/// Again checking position using Position and Size dialog ///'
    kontext
    Messagebox.SetPage TabPositionAndSize
    kontext "TabPositionAndSize"
    if PositionX.GetText = Ueber_Text_1 then
      printlog "OK, last order could be redone"
    else
      warnlog "Last order could not be redone"
    end if
    TabPositionAndSize.Cancel
    
    Call hCloseDocument						'/// Close document ///'

endcase

'-------------------------------------------------------------------------------
testcase tiEditRedo
    
    Dim Datei$
    Dim Ueber_Text_1
    
    if (gApplication = "IMPRESS") then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if
    
    Datei$ = gTesttoolPath + "graphics\required\input\leer."+ExtensionString
    
    hFileOpen ConvertPath (Datei$)			'/// Open test document (leer.od?) ///'
    sleep 3
    ' check if the document is writable
    if fIsDocumentWritable = false then
        ' make the document writable and check if it's succesfull
        if fMakeDocumentWritable = false then
            warnlog "The document can't be make writeable. Test stopped."
            goto endsub
        endif
    endif
    sleep 1
    gMouseClick 30,10
    sleep 1
    hTypeKeys "<TAB>"
    sleep 2
    ContextPositionAndSize
    sleep 1
    Kontext
    Messagebox.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    Ueber_Text_1=PositionX.GetText
    TabPositionAndSize.Cancel
    sleep 1
    hTypeKeys "<TAB>"
    sleep 1
    ContextArrangeBringBackward					'/// move object behind other object ///'
    sleep 1
    gMouseClick 60,60
    sleep 1
    hTypeKeys "<TAB>"
    sleep 1
    ContextPositionAndSize
    sleep 1
    Kontext
    Messagebox.SetPage TabPositionAndSize					'/// checking new position values ///'
    Kontext "TabPositionAndSize"
     if PositionX.GetText = Ueber_Text_1 Then Warnlog "  Nothing changed, so nothing can be undone"
    TabPositionAndSize.Cancel
    sleep 1
    gMouseClick 60,60
    EditUndo							'/// Edit Undo///'
    sleep 1
    hTypeKeys "<TAB>"
    sleep 1
    ContextPositionAndSize
    sleep 1
    kontext
    Messagebox.SetPage TabPositionAndSize					'/// checking again position ///'
    kontext "TabPositionAndSize"
    if PositionX.GetText <> Ueber_Text_1 Then Warnlog "  Undo didn't worked, so repeat last action wont work either"
    TabPositionAndSize.Cancel
    sleep 2
    EditRedo							'/// Edit Redo ///'
    sleep 2
    gMouseClick 60,60
    sleep 2
    hTypeKeys "<TAB>"
    sleep 2
    ContextPositionAndSize
    sleep 1
    kontext
    Messagebox.SetPage TabPositionAndSize
    kontext "TabPositionAndSize"
    if PositionX.GetText <> Ueber_Text_1 then			'/// checking position ///'
      printlog "Action Undo repeated"
    else
      warnlog "Action Undo not repeated"
    end if
    TabPositionAndSize.OK
    sleep 1
    call hCloseDocument						'/// close document ///'
    sleep 1
endcase

'-------------------------------------------------------------------------------
testcase tiEditClipboard

    Dim Ueber_Text_4 as string
    dim sTemp as string
    
    '/// open application ///'
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 1
    SetClipboard ""
    '/// Create rectangle ///'
    Call hRechteckErstellen ( 65, 65, 80, 80 )
    sleep 1
    '///+ RightClick onto rectangle///'
    gMouseClick 70,70
    sleep 1
    Ueber_Text_4 = fGetPositionX()				'/// Get values for position variables ///'
    Sleep 1
    '/// <b> Edit->Copy </b> ///'
    EditCopy
    '///+ Type Key [Deletee] ///'
    hTypeKeys "<DELETE>"
    '///+ <i> Edit->Paste </i> ///'
    EditPaste
    sleep 1
    '///+   Edit->Select All ///'
    EditSelectAll
    sTemp = fGetPositionX()
    if LiberalMeasurement(sTemp, Ueber_Text_4) then			'/// TabPositionAndSize: comparing position with original position ///'
      printlog "OK   EditCopy and EditPaste"
    else
      warnlog "- Edit paste did not work; is: '" + sTemp + "' ; should: '" + Ueber_Text_4 + "'"
    end if
    sleep 1
    '/// Type Keys [escape], + EditSelectAll (to deselect object and to select again) ///'
    hTypeKeys "<escape>"
    EditSelectAll
    sleep 2
    '/// <b> Edit->Cut </b>///'
    EditCut
    sleep 1
    hTypeKeys "<TAB>"
    try
      '///+ - Format->Position And Size -> HAS TO FAIL / IS NOT AVAILABLE !!! '///'
      ContextPositionAndSize
         kontext
         '///+ - select TabPage 'Position And Size' ///'
         Messagebox.SetPage TabPositionAndSize
         Kontext "TabPositionAndSize"
         if TabPositionAndSize.NOTexists then
            warnlog "TabPositionAndSize isn't up ?!!"
         endif
      '///+ - Ok dialog 'Position And Size' ///'
      TabPositionAndSize.Ok
      warnlog "TabPositionAndSize IS up :-( Doesn't have to be, because object should be deleted !!!"
    catch
      Printlog " - There is no object in the document anymore :-)"
    endcatch
    sleep 1
    '///+ <i> Edit->Paste </i> ///'
    EditPaste
    sleep 2
    sTemp = fGetPositionX()
    if LiberalMeasurement(sTemp, Ueber_Text_4) then
        printlog "OK   Edit paste"
    else
      warnlog "- Edit paste did not work; is: '" + sTemp + "' ; should: '" + Ueber_Text_4 + "'"
    end if
    sleep 3
    '/// close application ///'
    Call hCloseDocument
endcase

'-------------------------------------------------------------------------------
testcase tiEditPasteSpecial_HTML
    
    dim Zaehler as integer
    Kontext "Standardbar"
    sleep 1
    Call hFileOpen ConvertPath (gTesttoolPath + "graphics\required\input\frame1.htm")	'/// Load html document ///'
    sleep 5
    autoexecute=false
    Kontext "DocumentWriter"
    DocumentWriter.MouseDown 10,10
    DocumentWriter.MouseUp 10,10
    autoexecute=true
    EditSelectAll
    sleep 3
    EditCopy									'/// Copy selection ///'
    Sleep 2
    FileClose									'/// Close document ///'
    sleep 3
    Call hNewDocument								'/// Create new Impress document ///'
    sleep 1
    EditPasteSpecial								'/// Edit paste special ///'
    sleep 1
    Kontext "InhaltEinfuegen"
    For Zaehler = 1 to Auswahl.GetItemCount
       Auswahl.select Zaehler
       printlog Zaehler, ". Selection = ", Auswahl.GetSelText
    next Zaehler
    
    if Auswahl.GetItemCount <> 4 then						'/// Controlling number of possible formats ///'
      warnlog "formats for inserting Html <> 4"
    else
      printlog "OK   4 formats for HTML-Document"
    end if
    
    sleep 3
    InhaltEinfuegen.OK								'/// Paste clipboard content ///'
    sleep 3
    try
     EditCopy									'/// Copy object back into clipboard ///'
    catch
     Warnlog "-  Nothing inserted into document"
    endcatch
    sleep 3
    Call hCloseDocument  								'/// Close document ///'
    sleep 3
endcase

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Text
    
    printlog "Insert Text"
    call hNewDocument                 '/// new document  ///'
    SetClipboard "I am a cool test-text in the Clipboard"  '/// put text into clipboard ///'
    EditPasteSpecial                  '/// edit paste special ///'
    sleep 1
    Kontext "InhaltEinfuegen"
    Auswahl.Select 1                  '/// select list entree 1 ///'
    printlog "Selection = ", Auswahl.GetSelText
    sleep 1
    if Auswahl.GetItemCount <> 1 then
      warnlog "Selectionformat <> 1"
    else
      printlog "Format Text OK"
    end if
    InhaltEinfuegen.OK                '/// close dialog ///'
    sleep 1
    
    Call hCloseDocument                   '/// close document ///'
endcase

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Graphic
    
    dim Zaehler as integer
    
    call hNewDocument                  '/// ^new document  ///'
    printlog "Insert graphic file"
    InsertGraphicsFromFile               '/// insert graphic (sample.bmp)///'
    
    Kontext "GrafikEinfuegenDlg"
      Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
      Oeffnen.Click
    sleep 1
    EditSelectAll                     '/// select graphic ///'
    EditCut                     '/// cut graphic ///'
    EditPasteSpecial                  '/// edit paste special ///'
    sleep 1
    Kontext "InhaltEinfuegen"
    For Zaehler = 1 to Auswahl.GetItemCount         '/// get item count for Auswahl ///'
    Auswahl.select Zaehler
    printlog Zaehler, ". Selection = ", Auswahl.GetSelText
    next Zaehler
    
    if Auswahl.GetItemCount <> 4 then            '/// check if item count = 4 ///'
      warnlog "Format for insert graphic files should be 4, but is: " + Auswahl.GetItemCount
    else
      printlog "There are " + Auswahl.GetItemCount + " formats for graphic files"
    end if
    InhaltEinfuegen.OK                '/// insert graphic from clipboard ///'
    gMouseClick 90,90
    sleep 1
    Call hCloseDocument                   '/// close document  ///'
endcase

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Circle

   dim Zaehler as integer
 call hNewDocument                  '/// new document  ///'
  sleep 5
  printlog "Insert circle"                '/// create circle ///'
  Kontext "Toolbar"
  Ellipsen.Click
  sleep 3
  gMouseMove (20,20,80,80)
  sleep 1
  hTypeKeys "<escape>"
  sleep 2
  EditSelectAll               '/// select circle ///'
  sleep 5
  EditCut                     '/// cut circle ///'
  sleep 1
  EditPasteSpecial                  '/// edit paste special ///'
  sleep 1
  Kontext "InhaltEinfuegen"
  if EinfuegenAls.IsVisible Then EinfuegenAls.Check      '/// check "EinfuegenAls" ///'
   For Zaehler = 1 to Auswahl.GetItemCount         '/// get item count for Auswahl ///'
       Auswahl.select Zaehler
       printlog Zaehler, ". Selection = ", Auswahl.GetSelText
   next Zaehler
   if Auswahl.GetItemCount <> 4 then            '/// check if item count = 4 ///'
     warnlog "Number of possible formats <> 4"
   else
     printlog "There are 4 formats for circle"
   end if
  sleep 1
  InhaltEinfuegen.OK                '/// paste circle ///'
  sleep 1
 Call hCloseDocument                   '/// close document  ///'
endcase

'-------------------------------------------------------------------------------
testcase tiEditPasteSpecial

  dim i as integer
  dim x as integer
'   warnlog "TODOTBO: from draw, impress, math..."
   '/// go into writer, type a text, select all, Edit->Cut, close writer ///'
   '/// go into application to test, insert every format you can get from Edit->Paste->Special :-) ///'
   gApplication   = "WRITER"
   call hNewDocument
   kontext "DocumentWriter"
   DocumentWriter.TypeKeys "abc<shift home>"
   editcut
   call hCloseDocument
   gApplication   = "IMPRESS"
   call hNewDocument
   sleep 1

 Printlog " get count of possibilities: "
   EditPasteSpecial
   sleep 1
   Kontext "InhaltEinfuegen"
   x = Auswahl.GetItemCount
   for i = 1 to x
      printlog " - " + i + " - " + Auswahl.GetItemText (i)
   next i
   InhaltEinfuegen.Cancel

 Printlog " insert every possibility"
   for i = 1 to x
   Printlog " - processing number: "+i
      EditPasteSpecial
      sleep 1
      Kontext "InhaltEinfuegen"
      Auswahl.Select i
      sleep 1
      InhaltEinfuegen.OK
      sleep 1
   next i
   Call hCloseDocument  '/// close document ///'
endcase

'-------------------------------------------------------------------------------
