'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: g_paragraph.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:40 $
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
'* Owner : fredrik.haegg@sun.com
'*
'* short description :
'*
'***********************************************************************************
'*
' #1 tdFormatParagraph
'*
'\***********************************************************************************

testcase tdFormatParagraph
   Dim Zaehler as integer
   Dim a as integer
   dim iCount as integer '3 out of 6 have to be able to change a value!
   dim sIndent as string
   dim sIndentValue
   dim sTemp(5) as string
   dim iTemp as integer
   dim sApplication as string

   if (gApplication = "DRAW") then
      sApplication = "DRAWING"
   else
      sApplication = "IMPRESS"
   endif

   iCount = 0
   '/// open application ///'
   Call hNewDocument

   printlog "'/// - save states ///" '
   ToolsOptions
    hToolsOptions (sApplication,"General")
    iTemp = Masseinheit.GetSelIndex
    Masseinheit.TypeKeys= "<HOME>" '(first entry)
    Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

'/// call Format->Paragraph & set some values on 'indents and spacing' ///'
   FormatParagraph
      Kontext
      Active.SetPage TabEinzuegeUndAbstaende
      Kontext "TabEinzuegeUndAbstaende"
      VonLinks.SetText "2"
      sleep (1)
      VonLinks.Typekeys "<TAB>"
      ErsteZeile.SetText "2"
      sleep (1)
      ErsteZeile.Typekeys "<TAB>"
      VonRechts.SetText "2"
      sleep (1)
      VonRechts.Typekeys "<TAB>"
      Oben.SetText "2"
      sleep (1)
      Oben.Typekeys "<TAB>"
      Unten.SetText "2"
      sleep (1)
      Unten.Typekeys "<TAB>"
      Zeilenabstand.TypeKeys "<DOWN>"
      Zeilenabstand.TypeKeys "<UP>"
      printlog "linespacing count: "+Zeilenabstand.GetItemCount
      sTemp(1) = VonLinks.getText
      sTemp(2) = ErsteZeile.getText
      sTemp(3) = VonRechts.getText
      sTemp(4) = Oben.getText
      sTemp(5) = Unten.getText
      TabEinzuegeUndAbstaende.OK
   '///+ close dialog ///'
   '///+ call Format->Paragraph again and check values ///'
   FormatParagraph
      Kontext
      Active.SetPage TabEinzuegeUndAbstaende
      Kontext "TabEinzuegeUndAbstaende"
      if LiberalMeasurement(VonLinks.GetText, sTemp(1)) <> TRUE Then WarnLog "Value changed VonLinks   is: '" + VonLinks.GetText +   "' should: '" + sTemp(1) +"'"
      if LiberalMeasurement(VonRechts.GetText, sTemp(2)) <> TRUE Then WarnLog "Value changed VonRechts  is: '" + VonRechts.GetText +  "' should: '" + sTemp(2) +"'"
      if LiberalMeasurement(ErsteZeile.GetText, sTemp(3)) <> TRUE Then WarnLog "Value changed ErsteZeile is: '" + ErsteZeile.GetText + "' should: '" + sTemp(3) +"'"
      if LiberalMeasurement(Oben.GetText, sTemp(4)) <> TRUE Then WarnLog "Value changed Oben       is: '" + Oben.GetText +       "' should: '" + sTemp(4) +"'"
      if LiberalMeasurement(Unten.GetText, sTemp(5)) <> TRUE Then WarnLog "Value changed Unten      is: '" + Unten.GetText +      "' should: '" + sTemp(5) +"'"

      '///+ try to activate the metric field for line spacing, has to work for the last 3 of 6 entries !///'
      Zaehler=Zeilenabstand.GetItemCount
     for a=1 to Zaehler
        Zeilenabstand.Select a
        sIndent = Zeilenabstand.GetSelText
        sleep 1
        try
'           if um.IsEnabled then printlog  "## UM##"
'           if von.IsEnabled then printlog "##VON##"
           if (von.IsEnabled and um.IsEnabled)  then printlog "##VON#### UM## !!!!! :-("
           if von.IsEnabled then
              sIndentValue = von.GetText
              von.More
              sleep 1
              if sIndentValue = von.GetText then
                 warnlog "value didn't change :-( : '"+sIndent +"'"
              else
                 printlog "value changed :-) '"+sIndent +"'"
              endif
              iCount = iCount+1
           else
              if um.IsEnabled then
                 sIndentValue = Um.GetText
                 Um.More
                 sleep 1
                 if sIndentValue = Um.GetText then
                    warnlog "value didn't change :-( : '"+sIndent +"'"
                 else
                    printlog "value changed :-) '"+sIndent +"'"
                 endif
                 iCount = iCount+1
              else
                 printlog "value not changeable :-? '"+sIndent +"'"
              endif
           endif
        catch
           warnLog "BIGERROR :-("
        endcatch
        printlog "----------------------------------------------------------------------------------------"
        sleep 1
        TabEinzuegeUndAbstaende.OK
        sleep 1
        FormatParagraph
        Kontext
        Active.SetPage TabEinzuegeUndAbstaende
        Kontext "TabEinzuegeUndAbstaende"
     next a
     if iCount <> 3 then warnlog "Could only activate "+ iCount + " out of "+Zaehler +" vlauefields :-("
 TabeinzuegeUndAbstaende.OK

'/// call Format->Paragraph & set some values on 'alignment' ///'
'///+ check button, close dialog, open again, check if the button is stioll selected ///'
'///+ do it for all four buttons: left, right, center, justified ///'
   FormatParagraph
   Kontext
   Active.SetPage TabAusrichtungAbsatz
   Kontext "TabAusrichtungAbsatz"

   Links.Check
   TabAusrichtungAbsatz.OK
   FormatParagraph
   Kontext
   Active.SetPage TabAusrichtungAbsatz
   Kontext "TabAusrichtungAbsatz"
   if Links.IsChecked=False Then WarnLog " Button 'left' is not activated"

   Rechts.Check
   TabAusrichtungAbsatz.OK
   FormatParagraph
   Kontext
   Active.SetPage TabAusrichtungAbsatz
   Kontext "TabAusrichtungAbsatz"
   if Rechts.IsChecked=False Then WarnLog " Button 'right' is not activated"

   Zentriert.Check
   TabAusrichtungAbsatz.OK
   FormatParagraph
   Kontext
   Active.SetPage TabAusrichtungAbsatz
   Kontext "TabAusrichtungAbsatz"
   if Zentriert.IsChecked=False Then WarnLog " Button 'center' is not activated"

   Blocksatz.Check
   TabAusrichtungAbsatz.OK
   FormatParagraph
   Kontext
   Active.SetPage TabAusrichtungAbsatz
   Kontext "TabAusrichtungAbsatz"
   if Blocksatz.IsChecked=False Then WarnLog " Button 'justified' is not activated"
   TabAusrichtungAbsatz.OK

'/// call Format->Paragraph & set some values on 'tabs' ///'
   FormatParagraph
   Kontext
   Active.SetPage TabTabulator
   Kontext "TabTabulator"
   SetClipboard Position.GetItemCount
   Position.SetText "1"
   Neu.Click
   Position.SetText "2"
   Neu.Click
   Position.SetText "3"
   Neu.Click
   TabTabulator.OK
   FormatParagraph
   Kontext
   Active.SetPage TabTabulator
   Kontext "TabTabulator"
   if GetClipboardText + 3 = Position.GetItemCount Then PrintLog " positions added"
   Loeschen.Click
   if Position.GetItemCount="3" Then
      WarnLog " Delete does not work"
   else
      Printlog " TAll ran well in TabTabulator"
   end if
   TabTabulator.OK

   printlog "'/// - Reset states back to what they were before ///"'
   ToolsOptions
    hToolsOptions (sApplication,"General")
    Masseinheit.Select iTemp
    if ( Masseinheit.GetSelIndex <> iTemp ) then WarnLog "Unit of Measurement state were NOT changed back"
    Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

   '/// close application ///'
   Call hCloseDocument
endcase
