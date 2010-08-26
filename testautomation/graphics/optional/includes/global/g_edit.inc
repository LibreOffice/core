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
' #1 tEditDuplicate
' #1 tEditFields
' #1 tEditDeleteSlide
' #1 tEditLinks
' #1 tdEditDeleteLayer
' #1 tEditObjectEdit
'\*************************************************************************************

'   sDatei = gTesttoolPath + "graphics\required\input\leer.sx"+left(gApplication,1)
'   sDatei = gTesttoolPath + "graphics\required\input\leer.sx"+left(gApplication,1)
'      Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")

testcase tEditDuplicate
 dim Ueber_Text_1
 dim Zaehler

 Call hNewDocument						'/// New Impress document ///'
  Call hRechteckErstellen ( 5, 5, 20, 30 )			'/// Create rectangle ///'
  EditSelectAll							'/// Select rectangle ///'
  EditDuplicate							'/// Edit-duplicate ///'
  Kontext "Duplizieren"
  Ueber_Text_1 = AnzahlAnKopien.GetText
  AnzahlAnKopien.SetText "50"					'/// Set number of copies to 50 ///'
  Duplizieren.Cancel						'/// Cancel Edit Duplicate ///'
  sleep 1
   For Zaehler = 1 to 8
       EditDuplicate						'/// Edit Duplicate ///'
       sleep 1
       Kontext "Duplizieren"
       AnzahlAnKopien.SetText "30"				'/// Number of copies = 30 ///'

       Select Case Zaehler					'/// Setting values for axis and angles ///'
          Case 1:	XAchse.SetText "0,0"
            YAchse.SetText "0,45"
            Hoehe.SetText  "-0,1"
            Breite.SetText "-0,1"
            Drehwinkel.SetText "3"
            Anfang.Select Int((Anfang.GetItemCount * Rnd)+1)
          Case 2,6:	XAchse.SetText "0,6"
            YAchse.SetText "0,0"
            Hoehe.SetText  "0,15"
            Breite.SetText "0,01"
            Drehwinkel.SetText "353"
              Case 3,7:	XAchse.SetText "0,0"
            YAchse.SetText "-0,4"
            Hoehe.SetText  "-0,1"
            Breite.SetText "0,06"
            Drehwinkel.SetText "11"
          Case 4,8:	XAchse.SetText "-0,6"
            YAchse.SetText "0,0"
            Hoehe.SetText  "0,05"
            Breite.SetText "-0,08"
            Drehwinkel.SetText "347"
          Case 5:	XAchse.SetText "0,1"
            YAchse.SetText "0,48"
            Hoehe.SetText  "-0,05"
            Breite.SetText "-0,05"
            Drehwinkel.SetText "355"
    End Select
    Ende.Select Int((Ende.GetItemCount * Rnd)+1)
    Duplizieren.OK						'/// Execute Duplicate ///'
    sleep 1
   Next Zaehler
  sleep 2
 Call hCloseDocument						'/// Close document ///'
 
endcase 'tEditDuplicate
'---------------------------------------------------------
testcase tEditFields

    Call hNewDocument
    Call sSelectEmptyLayout
    sleep 3
    printlog "insert date-fix"
    InsertFieldsDateFix
    printlog "select inserted field"
    EditSelectAll
    sleep 1
    printlog "go into edit mode"
    hTypeKeys "<F2>"
    sleep 2
    printlog "select date"
    EditSelectAll
    sleep 2
    printlog "edit fields"
    EditfieldsDraw
    Kontext "FeldbefehlBearbeitenDraw"
    sleep 2
    printlog "check field type fix"
    FeldtypFix.Check
    If	FeldtypFix.Ischecked then
        Printlog "OK   FieldtypeFix checkable"
    else
        warnlog "FieldtypeFix not checkable"
    end if
    printlog "check field type variable 1"
    FeldtypVariabel.Check
    sleep 1
    If	FeldtypVariabel.Ischecked then
        Printlog "OK   FieldtypeVariabel enabled"
    else
        warnlog "FieldtypeVariabel not enabled"
    end if
    '   For Zaehler = 1 to FeldtypFormat.GetItemCount
    '    FeldTypFormat.Select Zaehler
    '    printlog "OK   Format ", Zaehler, " = ", FeldtypFormat.GetSelText
    '   Next Zaehler
    printlog "close edit fields dialog"
    FeldbefehlBearbeitenDraw.OK
    sleep 2
    printlog "Close Document"
    Call hCloseDocument
    sleep 2
  
endcase 'tEditFields
'--------------------------------------------------------
testcase tEditDeleteSlide

 dim Ueber_Text_1 as string
 dim Ueber_Text_2 as string
 dim Ueber_Text_3 as string
  Call hNewDocument						'/// New impress document ///'
  sleep 3
  Call hRechteckErstellen ( 30, 30, 70, 70 )			'/// create rectangle ///'
  sleep 1
  ContextPositionAndSize					'/// open Position and Size dialog ///'
  kontext
  Messagebox.SetPage TabPositionAndSize
  kontext "TabPositionAndSize"
  Ueber_Text_1 = PositionX.GetText				'/// get position values for rectangle ///'
  TabPositionAndSize.OK
  InsertSlide							'/// insert slide ///'
  sleep 2
  hTypekeys "<Pagedown>"
  sleep 2
  Call hRechteckErstellen ( 40, 40, 60, 60 )			'/// create rectangle ///'
  sleep 1
  EditDeleteSlide						'/// Delete slide ///'
  sleep 1
  Ueber_Text_2 = "OK   Page was closed"
  Ueber_Text_3 = "Page was not closed"
  call Position_Vergleichen (Ueber_Text_1,Ueber_Text_2,Ueber_Text_3)					'/// compare position of rectange ///'
  sleep 1
 Call hCloseDocument						'/// close document ///'
 
endcase 'tEditDeleteSlide
'--------------------------------------------------------
testcase tEditLinks

 Call  hNewDocument						'/// New document ///'
  InsertGraphicsFromFile					'/// insert graphic (sample.bmp) ///'
  Kontext "GrafikEinfuegenDlg"
  Link.Check							'/// check link ///'
      Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\enter.bmp")
      Oeffnen.Click
  sleep 3
  EditLinksDraw					'/// edit links ///'
  Kontext "VerknuepfungenBearbeiten"
  Aktualisieren.Click						'/// refresh ///'
  sleep 1
  'Aendern							'kann man noch den Dialog aufrufen...
  Loesen.Click
  sleep 1
  Kontext
  Messagebox.Yes
  VerknuepfungenBearbeiten.Close				'/// close dialog ///'
  sleep 1
  try
    EditLinksDraw				'/// check if last changes remain in the dialog ///'
    Kontext "VerknuepfungenBearbeiten"
    Loesen.Click
    sleep 1
    Kontext
    Messagebox.Yes
    VerknuepfungenBearbeiten.Close
    warnlog "Break link does not work"
  catch
    printlog "OK   Link broken"
  endcatch
 Call hCloseDocument						'/// close document ///'
 
endcase 'tEditLinks
'---------------------------------------------------------
testcase tdEditDeleteLayer

 dim Ueber_Text_1 as string
 dim Ueber_Text_2 as string
 dim Ueber_Text_3 as string
 Call hNewDocument                     '/// new document ///'
  sleep 2
  Call hRechteckErstellen ( 30, 30, 70, 70 )          '/// create rectangle ///'
  sleep 1
'/// View->Layer ///'
   ViewLayer
  ContextPositionAndSize                  '/// get position values for rectangle ///'
  kontext
  active.SetPage TabPositionAndSize
  kontext "TabPositionAndSize"
  Ueber_Text_1 = PositionX.GetText
  TabPositionAndSize.OK
  InsertLayer                             '/// insert layer ///'
  Kontext "EbeneEinfuegenDlg"
  EbeneEinfuegenDlg.OK
  Call hRechteckErstellen ( 10, 40, 90, 60 )          '/// create rectangle on created layer ///'
  sleep 1
  EditDeleteLayer                   '/// delete layer ///'
  sleep 1
  Kontext
  Active.Yes
  sleep 1
  Ueber_Text_2 = "OK   Layer was deleted"            '/// compare position of selected rectangle with position of created rectangle in deteted layer ///'
  Ueber_Text_3 = "Layer was not deleted"
  Call Position_Vergleichen (Ueber_Text_1,Ueber_Text_2,Ueber_Text_3)
  sleep 1
 Call hCloseDocument                   '/// close document ///'
 
endcase 'tdEditDeleteLayer
'--------------------------------------------------------
testcase tEditObjectEdit

 Dim Schrieb  as string

  Schrieb = gOLEWriter
  if Schrieb = "" then
      warnlog " the OLE is not defined :-((( gOLEWriter: " + iSprache
  else

      Call hNewDocument
      call sSelectEmptyLayout
      sleep 1               '/// new document ///'
      InsertObjectOLEObject             '/// insert writer ole object ///'
      Kontext "OLEObjektEinfuegen"
      sleep 3
      if ObjektTyp.GetItemCount = 0 Then
         Warnlog "- No available Ole-entry in the list, maybe an error in the Install.ini?"
         sleep 1
         OleObjektEinfuegen.Cancel
         sleep 1
       else

          ObjektTyp.Select Schrieb
          sleep 1
          OLEObjektEinfuegen.OK
          sleep 3                                 ' Changed from Sleep 1 to Sleep 3
          gMouseMove  10,10,90,90
          sleep 1
          EditSelectAll

          try
             EditObjectEdit      'keine ID      '/// try edit-objects ///'
             sleep 2
          catch
             Warnlog "-  The EditObjectEdit did not work"
          endcatch


          gMouseClick 90,90
        end if
      sleep 1
     Call hCloseDocument                     '/// close document ///'
  endif
endcase 'tEditObjectEdit
