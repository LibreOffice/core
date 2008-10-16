'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: wh_o_4.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: fredrikh $ $Date: 2008-06-18 15:03:56 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : fredrik.haegg@sun.com
'*
'* short description :
'*
'\***********************************************************************

sub wh_o_4
   'Call tToolsOptionsHTMLSourceText : removed > srx642y
   'Call tToolsOptionsHTMLBackground : removed > srx642y
end sub

'------------------------------------------------------------------------------
testcase tToolsOptionsHTMLSourceText

'/ SourceText                     ///'
'/ ----------                     ///'
'/ tToolsOptionsHTMLSourceText    ///'
'/ normal                         ///'
'/ Function: only manual possible! ///'
'/                                ///'

   dim iListBox(4) as integer         ' ListBox
   dim iTempListBox(4) as integer
   Dim IsViewHTMLSource as Boolean


'iListBox(i) / iTempListBox(i)
'(1) = SGML
'(2) = Kommentar
'(3) = Schluesselwort
'(4) = Unbekannt

   hNewDocument
  printlog "'///- Tools/Options/HTML: SourceText ///"

  printlog ("'/// - save states ///")
    ToolsOptions
    hToolsOptions ("HtmlDocument","SOURCE")

      iListBox(1) = SGML.GetSelIndex
      iListBox(2) = Kommentar.GetSelIndex
      iListBox(3) = Schluesselwort.GetSelIndex
      iListBox(4) = Unbekannt.GetSelIndex

      printlog ("'/// - all states inverting ///")

      If ( SGML.GetItemCount           <> iListBox(1) ) Then SGML.Select           (iListBox(1) + 1) Else SGML.Select           (iListBox(1) - 1)
      If ( 1                           <> iListBox(2) ) Then Kommentar.Select      (iListBox(2) - 1) Else Kommentar.Select      (iListBox(2) + 1)
      If ( Schluesselwort.GetItemCount <> iListBox(3) ) Then Schluesselwort.Select (iListBox(3) + 1) Else Schluesselwort.Select (iListBox(3) - 1)
      If ( 1                           <> iListBox(4) ) Then Unbekannt.Select      (iListBox(4) - 1) Else Unbekannt.Select      (iListBox(4) + 1)
      iTempListBox(1) = SGML.GetSelIndex
      iTempListBox(2) = Kommentar.GetSelIndex
      iTempListBox(3) = Schluesselwort.GetSelIndex
      iTempListBox(4) = Unbekannt.GetSelIndex

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

  printlog ("'/// - SO quit - start ///")

  wOfficeRestart

'--------------------------------------------------------------------------
'/// Function test: Syntax Highlighting ///'
   '///+ uses "input\\writer\\options\\htmltag.html" ///
   hFileOpen ( gTesttoolPath + "writer\optional\input\options\htmltag.html" )
   Call sMakeReadOnlyDocumentEditable

'///+ Check if we are in HTML-Source-View ///'
   try
      FormatObject
   catch
      kontext "Funktionsleiste"
      Bearbeiten.click                   ' maybee theres something missing :-) TBO
   endcatch
   sleep (3)

   try
      ViewZoom
      kontext "massstab"
      massstab.cancel
      IsViewHTMLSource = False
   catch
      IsViewHTMLSource = True
   endcatch

   if True <> IsViewHTMLSource Then
      AnsichtHTMLQuelltext ' THE ONLY ONE THAT WORKS
   end if
' Now we are in HTML-Source-View

'  printlog "Screenshot taken: " + ScreenShot ("h_st_hig")
      AnsichtHTMLQuelltext
   hCloseDocument
'--------------------------------------------------------------------------

  printlog ("'/// - checking states ///")
    ToolsOptions
    hToolsOptions ("HtmlDocument","SOURCE")

      If ( SGML.GetSelIndex           <> iTempListBox(1) ) Then WarnLog "SGML           state changed"
      If ( Kommentar.GetSelIndex      <> iTempListBox(2) ) Then WarnLog "Kommentar      state changed"
      If ( Schluesselwort.GetSelIndex <> iTempListBox(3) ) Then WarnLog "Schluesselwort state changed"
      If ( Unbekannt.GetSelIndex      <> iTempListBox(4) ) Then WarnLog "Unbekannt      state changed"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

  printlog ("'/// - all UnCheck -> o ///")
    ToolsOptions
    hToolsOptions ( "HtmlDocument" , "SOURCE" )

      SGML.Select           (1)
      Kommentar.Select      (1)
      Schluesselwort.Select (1)
      Unbekannt.Select      (1)

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

  printlog ("'/// - check if all UnChecked ///")
    ToolsOptions
    hToolsOptions ( "HtmlDocument" , "SOURCE" )

      If ( SGML.GetSelIndex           <> 1 ) Then WarnLog "SGML           is not item 1"
      If ( Kommentar.GetSelIndex      <> 1 ) Then WarnLog "Kommentar      is not item 1"
      If ( Schluesselwort.GetSelIndex <> 1 ) Then WarnLog "Schluesselwort is not item 1"
      If ( Unbekannt.GetSelIndex      <> 1 ) Then WarnLog "Unbekannt      is not item 1"

      Printlog ("'/// - all Check -> x ///")

      SGML.Select          (SGML.GetItemCount          )
      Kommentar.Select     (Kommentar.GetItemCount     )
      Schluesselwort.Select(Schluesselwort.GetItemCount)
      Unbekannt.Select     (Unbekannt.GetItemCount     )

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

  printlog ("'/// - check if all Checked ///")
    ToolsOptions
    hToolsOptions ( "HtmlDocument" , "SOURCE" )

      If ( SGML.GetSelIndex           <> SGML.GetItemCount           ) Then WarnLog "SGML           is not item (GetItemCount)"
      If ( Kommentar.GetSelIndex      <> Kommentar.GetItemCount      ) Then WarnLog "Kommentar      is not item (GetItemCount)"
      If ( Schluesselwort.GetSelIndex <> Schluesselwort.GetItemCount ) Then WarnLog "Schluesselwort is not item (GetItemCount)"
      If ( Unbekannt.GetSelIndex      <> Unbekannt.GetItemCount      ) Then WarnLog "Unbekannt      is not item (GetItemCount)"

  printlog ("'/// - restore states ///")

      SGML.Select           (iListBox(1))
      Kommentar.Select      (iListBox(2))
      Schluesselwort.Select (iListBox(3))
      Unbekannt.Select      (iListBox(4))

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

   hCloseDocument
endcase


testcase tToolsOptionsHTMLBackground

'/ Background                       ///'
'/ ----------                       ///'
'/ tToolsOptionsHTMLBackground      ///'
'/ NO TEST AVAIL                    ///'
' Function: not yet!               ///'
'/                                  ///'
'/ No test available (can't be controled :-() ///'
'/     Hintergrundfarbe       HID_BACKGROUND_CTL_BGDCOLORSET    ///'
'/                                  ///'
'/ just calling the TabPage :->     ///'
'/                                  ///'

'----in Format->Page->Backround:
'Durchsuchen            svx:PushButton:RID_SVXPAGE_BACKGROUND:BTN_BROWSE
'Verknuepfen            svx:CheckBox:RID_SVXPAGE_BACKGROUND:BTN_LINK
'Vorschau               svx:CheckBox:RID_SVXPAGE_BACKGROUND:BTN_PREVIEW
'Position               svx:RadioButton:RID_SVXPAGE_BACKGROUND:BTN_POSITION
'Flaeche                svx:RadioButton:RID_SVXPAGE_BACKGROUND:BTN_AREA
'Kachel                 svx:RadioButton:RID_SVXPAGE_BACKGROUND:BTN_TILE
'ArtPosition            HID_BACKGROUND_CTL_POSITION
'Als                    svx:ListBox:RID_SVXPAGE_BACKGROUND:LB_SELECTOR
'Fuer                   svx:ListBox:RID_SVXPAGE_BACKGROUND:LB_TBL_BOX
'FuerAbsatz             svx:ListBox:RID_SVXPAGE_BACKGROUND:LB_PARA_BOX
'------------------------------------------------------------------------------
   hNewDocument
  printlog "'///- Tools/Options/HTML: Content ///"

'/// TBO: not realy a test ;-) ///'

  printlog ("'/// - save states ///")
    ToolsOptions
    hToolsOptions ("HtmlDocument","BACKGROUND")

'   setcontroltype ctbrowsebox
'   try
'      printlog "3"+Hintergrundfarbe.GetItemText
'print "1"
'   catch
'      printlog "2"+Hintergrundfarbe.GetSelText
'print "1"
'      printlog "1"+Hintergrundfarbe.GetText
'print "1"
'   endcatch
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
   hCloseDocument
endcase

