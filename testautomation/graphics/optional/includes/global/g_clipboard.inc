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
'**************************************************************************************

testcase tiEditUndo

    Dim Ueber_Text_1 as string
    dim sTestfile as string
    dim sLocaltestfile as string
    
    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if
    
    sTestfile = gTesttoolPath + "graphics\required\input\leer."+ExtensionString
    printlog "Loading test file.."
    hFileOpenLocally( sTestfile )
    sleep 3
    printlog "Selecting grey rectangle.."
    hTypeKeys "<TAB><TAB>"
    sleep 3
    printlog "Opening Position and size dialog."
    ContextPositionAndSize
    Kontext
    if Messagebox.exists (5) then
        Messagebox.SetPage TabPositionAndSize
    else
        printlog "Not the expected Dialog came up. Please check."
    endif
    Kontext "TabPositionAndSize"
    printlog "defining variable for original position of object"
    Ueber_Text_1=PositionX.GetText
    printlog "Position X is: " & Ueber_Text_1
    printlog "Canceling dialog."
    TabPositionAndSize.Cancel
    sleep 2
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    printlog "Sending Rectangle backwards."
    ContextArrangeBringBackward	
    sleep 1
    printlog "Deselect."
    gMouseClick 60,60
    sleep 1
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    printlog "trying to select blue rectangle in front"
    hTypeKeys "<TAB><TAB>"
    Kontext "Toolbar"
    Auswahl.Click
    sleep 1
    printlog "Open Position and Size dialog, checking position"
    ContextPositionAndSize
    Kontext
    Messagebox.SetPage TabPositionAndSize
    Kontext "TabPositionAndSize"
    printlog "Position X is: " & PositionX.GetText
    if (PositionX.GetText)=0 Then Warnlog "Command was not completely executed, and it could not be undone."
    TabPositionAndSize.Cancel
    sleep 2
    gMouseClick 60,60
    sleep 1

    printlog "Edit Undo for BringBackward"
    EditUndo							
    Sleep 2
    gMouseClick 60,60
    sleep 1
    Kontext "Toolbar"
    Auswahl.Click
    printlog "Trying to select grey rectangle again."
    hTypeKeys "<TAB><TAB>"
    sleep 1
    Kontext "Toolbar"
    Auswahl.Click
    printlog "Again checking position using Position and Size dialog"
    ContextPositionAndSize					
    kontext
    Messagebox.SetPage TabPositionAndSize
    kontext "TabPositionAndSize"
    printlog "Checking if x Position is like ata the start again."
    if PositionX.GetText = Ueber_Text_1 then
      printlog "OK, last order could be redone"
    else
      warnlog "Last order could not be redone"
    end if
    TabPositionAndSize.Cancel
    printlog "Close document"
    Call hCloseDocument
    sLocaltestfile = hFileGetLocalPath( gTesttoolPath + "leer."+ExtensionString )
    printlog sLocaltestfile
    if (FileExists (sLocaltestfile)) then
        app.kill (sLocaltestfile)
        printlog "Deleting used file."
    else
        warnlog "File is missing."
    endif
    					
endcase 'tiEditUndo

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

    printlog "Open test document (leer.od?)"
    hFileOpen ConvertPath (Datei$)			
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
    printlog "move object behind other object"
    ContextArrangeBringBackward					
    sleep 1
    gMouseClick 60,60
    sleep 1
    hTypeKeys "<TAB>"
    sleep 1
    ContextPositionAndSize
    sleep 1
    Kontext
    printlog "checking new position values"
    Messagebox.SetPage TabPositionAndSize					
    Kontext "TabPositionAndSize"
     if PositionX.GetText = Ueber_Text_1 Then Warnlog "  Nothing changed, so nothing can be undone"
    TabPositionAndSize.Cancel
    sleep 1
    gMouseClick 60,60
    printlog "Edit Undo"
    EditUndo							
    sleep 1
    hTypeKeys "<TAB>"
    sleep 1
    ContextPositionAndSize
    sleep 1
    kontext
    printlog "checking again position"
    Messagebox.SetPage TabPositionAndSize					
    kontext "TabPositionAndSize"
    if PositionX.GetText <> Ueber_Text_1 Then Warnlog "  Undo didn't worked, so repeat last action wont work either"
    TabPositionAndSize.Cancel
    sleep 2
    printlog "Edit Redo"
    EditRedo							
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
    printlog "checking position"
    if PositionX.GetText <> Ueber_Text_1 then			
      printlog "Action Undo repeated"
    else
      warnlog "Action Undo not repeated"
    end if
    TabPositionAndSize.OK
    sleep 1
    printlog "close document"
    call hCloseDocument						
    sleep 1
endcase 'tiEditRedo

'-------------------------------------------------------------------------------
testcase tiEditClipboard

    Dim Ueber_Text_4 as string
    dim sTemp as string
    
    printlog "open application"
    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 1
    SetClipboard ""
    printlog "Create rectangle"
    Call hRechteckErstellen ( 65, 65, 80, 80 )
    sleep 1
    printlog "RightClick onto rectangle"
    gMouseClick 70,70
    sleep 1
    printlog "Get values for position variables"
    Ueber_Text_4 = fGetPositionX()				
    Sleep 1
    printlog " Edit->Copy"
    EditCopy
    printlog " Type Key [Delete]"
    hTypeKeys "<DELETE>"
    printlog "Edit->Paste"
    EditPaste
    sleep 1
    printlog "Edit->Select All"
    EditSelectAll
    sTemp = fGetPositionX()
    printlog "TabPositionAndSize: comparing position with original position"
    if LiberalMeasurement(sTemp, Ueber_Text_4) then			
      printlog "OK   EditCopy and EditPaste"
    else
      warnlog "- Edit paste did not work; is: '" + sTemp + "' ; should: '" + Ueber_Text_4 + "'"
    end if
    sleep 1
    printlog "Type Keys [escape], + EditSelectAll (to deselect object and to select again)"
    hTypeKeys "<escape>"
    EditSelectAll
    sleep 2
    printlog "<b> Edit->Cut"
    EditCut
    sleep 1
    hTypeKeys "<TAB>"
    try
      printlog "Format->Position And Size -> HAS TO FAIL / IS NOT AVAILABLE !!! "
      ContextPositionAndSize
         kontext
         printlog "select TabPage 'Position And Size'"
         Messagebox.SetPage TabPositionAndSize
         Kontext "TabPositionAndSize"
         if TabPositionAndSize.NOTexists then
            warnlog "TabPositionAndSize isn't up ?!!"
         endif
        printlog "Ok dialog 'Position And Size'"
      TabPositionAndSize.Ok
      warnlog "TabPositionAndSize IS up :-( Doesn't have to be, because object should be deleted !!!"
    catch
      Printlog " - There is no object in the document anymore :-)"
    endcatch
    sleep 1
    printlog "Edit->Paste"
    EditPaste
    sleep 2
    sTemp = fGetPositionX()
    if LiberalMeasurement(sTemp, Ueber_Text_4) then
        printlog "OK   Edit paste"
    else
      warnlog "- Edit paste did not work; is: '" + sTemp + "' ; should: '" + Ueber_Text_4 + "'"
    end if
    sleep 3
    printlog "close application"
    Call hCloseDocument
endcase 'tiEditClipboard

'-------------------------------------------------------------------------------
testcase tiEditPasteSpecial_HTML
    
    dim Zaehler as integer
    Kontext "Standardbar"
    sleep 1
    printlog "Load html document"
    Call hFileOpen ConvertPath (gTesttoolPath + "graphics\required\input\frame1.htm")	
    sleep 5
    autoexecute=false
    Kontext "DocumentWriter"
    DocumentWriter.MouseDown 10,10
    DocumentWriter.MouseUp 10,10
    autoexecute=true
    EditSelectAll
    sleep 3
    printlog "Copy selection"
    EditCopy									
    Sleep 2
    printlog "Close document"
    FileClose									
    sleep 3
    printlog "Create new Impress document"
    Call hNewDocument								
    sleep 1
    printlog "Edit paste special"
    EditPasteSpecial								
    sleep 1
    Kontext "InhaltEinfuegen"
    For Zaehler = 1 to Auswahl.GetItemCount
       Auswahl.select Zaehler
       printlog Zaehler, ". Selection = ", Auswahl.GetSelText
    next Zaehler

    printlog "Controlling number of possible formats"
    if Auswahl.GetItemCount <> 4 then						
      warnlog "formats for inserting Html <> 4"
    else
      printlog "OK   4 formats for HTML-Document"
    end if
    
    sleep 3
    printlog "Paste clipboard content"
    InhaltEinfuegen.OK								
    sleep 3
    try
    printlog "Copy object back into clipboard"
     EditCopy									
    catch
     Warnlog "-  Nothing inserted into document"
    endcatch
    sleep 3
    printlog "Close document"
    Call hCloseDocument  								
    sleep 3
endcase 'tiEditPasteSpecial_HTML

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Text
    
    printlog "Insert Text"
    printlog "new document "
    call hNewDocument
    printlog "put text into clipboard"
    SetClipboard "I am a cool test-text in the Clipboard"  
    printlog "edit paste special"
    EditPasteSpecial                  
    sleep 1
    Kontext "InhaltEinfuegen"
    printlog "select list entree 1"
    Auswahl.Select 1                  
    printlog "Selection = ", Auswahl.GetSelText
    sleep 1
    if Auswahl.GetItemCount <> 1 then
      warnlog "Selectionformat <> 1"
    else
      printlog "Format Text OK"
    end if
    printlog "close dialog"
    InhaltEinfuegen.OK                
    sleep 1

    printlog "close document"
    Call hCloseDocument                   
endcase 'tdEditPasteSpecial_Text

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Graphic
    
    dim Zaehler as integer

    printlog "new document "
    call hNewDocument                  
    printlog "Insert graphic file"
    InsertGraphicsFromFile               
    
    Kontext "GrafikEinfuegenDlg"
    Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
    Oeffnen.Click
    sleep 1
    printlog "select graphic"
    EditSelectAll
    printlog "cut graphic"
    EditCut
    printlog "edit paste special"
    EditPasteSpecial                  
    sleep 1
    Kontext "InhaltEinfuegen"
    printlog "get item count for Auswahl"
    For Zaehler = 1 to Auswahl.GetItemCount         
    Auswahl.select Zaehler
    printlog Zaehler, ". Selection = ", Auswahl.GetSelText
    next Zaehler

    printlog "check if item count = 4"
    if Auswahl.GetItemCount <> 4 then            
      warnlog "Format for insert graphic files should be 4, but is: " + Auswahl.GetItemCount
    else
      printlog "There are " + Auswahl.GetItemCount + " formats for graphic files"
    end if
    printlog "insert graphic from clipboard"
    InhaltEinfuegen.OK                
    gMouseClick 90,90
    sleep 1
    printlog "close document "
    Call hCloseDocument                   
endcase 'tdEditPasteSpecial_Graphic

'-------------------------------------------------------------------------------
testcase tdEditPasteSpecial_Circle

    dim Zaehler as integer
    printlog "new document "
    call hNewDocument
  sleep 5
  printlog "Insert circle"                
  Kontext "Toolbar"
  Ellipsen.Click
  sleep 3
  gMouseMove (20,20,80,80)
  sleep 1
  hTypeKeys "<escape>"
  sleep 2
  printlog "select circle"
  EditSelectAll               
  sleep 5
  printlog "cut circle"
  EditCut                     
  sleep 1
  printlog "edit paste special"
  EditPasteSpecial                  
  sleep 1
  Kontext "InhaltEinfuegen"
  printlog "check Insert as.."
  if EinfuegenAls.IsVisible Then EinfuegenAls.Check
  printlog "get item count for Auswahl"
   For Zaehler = 1 to Auswahl.GetItemCount         
       Auswahl.select Zaehler
       printlog Zaehler, ". Selection = ", Auswahl.GetSelText
   next Zaehler
   printlog "check if item count = 4"
   if Auswahl.GetItemCount <> 4 then            
     warnlog "Number of possible formats <> 4"
   else
     printlog "There are 4 formats for circle"
   end if
  sleep 1
  printlog "paste circle"
  InhaltEinfuegen.OK                
  sleep 1
  printlog "close document "
 Call hCloseDocument                   
endcase 'tdEditPasteSpecial_Circle

'-------------------------------------------------------------------------------
testcase tiEditPasteSpecial

  dim i as integer
  dim x as integer
'   warnlog "TODOTBO: from draw, impress, math..."
   printlog "go into writer, type a text, select all, Edit->Cut, close writer"
   printlog "go into application to test, insert every format you can get from Edit->Paste->Special :-)"
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
    printlog "close document"
    Call hCloseDocument
endcase 'tiEditPasteSpecial

'-------------------------------------------------------------------------------
