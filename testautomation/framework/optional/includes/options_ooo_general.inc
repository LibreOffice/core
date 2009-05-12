'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: options_ooo_general.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : 1. test for general group userdata ... view)
'*
'\******************************************************************************


testcase tOOoGeneral

   dim lsSave (4) as String
   dim lbSave (3) as Boolean

  '///short test if 'StarOffice / General' is saved in configuration
   '///<ul>
   '///<li>open a new document</li>
   hNewDocument

   '///<li>open options 'StarOffice' / 'General'</li>
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )

   '///<li>save old data</li>
   printlog " - save old data"
   lsSave ( 1 ) = Zweistellig.GetText
   lbSave ( 1 ) = Aktivieren.IsChecked
   Aktivieren.Check
   if gPlatgroup = "w95" then
      lbSave (2) = StarOfficeDialogeBenutzen.IsChecked
   end if
   lbSave ( 3 ) = DruckenStatus.IsChecked
   lsSave ( 3 ) = StyleSheet.GetSelText

   '///<li>invert/change data</li>
   printlog " - invert/change data"
   Zweistellig.SetText "1950"
   Zuruecksetzen.Click
   if gPlatgroup = "w95" then
      if lbSave (2) = TRUE then StarOfficeDialogeBenutzen.Uncheck else StarOfficeDialogeBenutzen.Check
   end if
   If lbSave ( 3 ) = TRUE then DruckenStatus.Uncheck else DruckenStatus.Check
   StyleSheet.Select 3

   '///<li>close options-dialog with OK</li>
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   '///<li>close document</li>
   hCloseDocument

   '///<li>exit/restart StarOffice</li>
   printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

   '///<li>open options 'StarOffice' / 'General'</li>
   printlog " - check data"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )

   '///<li>check data</li>
   if Zweistellig.GetText <> "1950" then Warnlog "Year ( 2 digits ) => changes not saved: '" + Zweistellig.GetText +"' #i29810"
   if Aktivieren.IsChecked <> TRUE then Warnlog "Enable HelpAgent => changes not saved"
   Zuruecksetzen.Click
   if gPlatgroup = "w95" then
      if StarOfficeDialogeBenutzen.IsChecked = lbSave (2) then Warnlog "Use StarOffice dialogs => changes not saved!"
   end if
   If DruckenStatus.IsChecked = lbSave ( 3 ) then Warnlog "Printing sets 'Document modified' status => changes not saved!"

   '///<li>make 2. changes</li>
   printlog " - 2. change data"
   Zweistellig.SetText "1800"
   DruckenStatus.UnCheck

   '///<li>close options-dialog with OK</li>
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   '/// Check Issue i52248 Modify flag is set after printing though option is not set ///'
   'It's a little tricky to reproduce.
   if gPlatgroup = "unx" then
       hNewDocument()
       FilePrint
       Kontext "DruckenDLG"
       if DruckenDLG.exists(5) then
           if gPlatform <> "osx" then
              Eigenschaften.click
              kontext
              active.setPage TabSPAPaper
              Kontext "TabSPAPaper"
              if TabSPAPaper.exists (5) then
                  TabSPAPaper.OK
                  sleep 5
              else
                  qaErrorlog "Properties of Printer didn't come up."
              endif
            end if
            Kontext "DruckenDLG"
            DruckenDLG.OK
            ' check state
            kontext
            if active.exists(5) then
                printlog "active: printing failed? '" + active.gettext + "'"
                active.ok
            endif
            try
                FileSave
                qaErrorlog "#i52248# Modify flag is set after printing though option is not set."
                kontext "SpeichernDlg"
                if SpeichernDlg.exists(5) then
                    SpeichernDlg.cancel
                endif
            catch
                printlog "#i52248# doesn't come up."
            endcatch
       else
            qaErrorLog "No printer available - resuming."
            kontext
            if active.exists(5) then
                printlog active.getText
                active.ok
            endif
       endif
       hCloseDocument
   endif
   
   '///<li>open options 'StarOffice' / 'General'</li>
   printlog " - check data"
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )

   '///<li>check data</li>
   if Zweistellig.GetText <> "1800" then Warnlog "Year ( 2 digits ) => changes not saved : '" + Zweistellig.GetText +"'"
   If DruckenStatus.IsChecked <> FALSE then Warnlog "Printing sets 'Document modified' status => changes not saved!"
   if StyleSheet.GetSelIndex <> 3 then Warnlog "Style sheet for StarOffice Help => changes not saved!"

   '///<li>reset to saved data</li>
   printlog " - reset to saved data"
   Zweistellig.SetText lsSave ( 1 )
   Aktivieren.Check
   Zuruecksetzen.Click
   if lbSave (1) = TRUE then Aktivieren.Check else Aktivieren.Uncheck
   if gPlatgroup = "w95" then
      if lbSave (2) = TRUE then StarOfficeDialogeBenutzen.Check else StarOfficeDialogeBenutzen.UnCheck
   end if
   If lbSave(3)= TRUE then DruckenStatus.Check else DruckenStatus.UnCheck
   StyleSheet.Select lsSave(3)

   '///<li>close options-dialog with OK</li>
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   '///<li>open options 'StarOffice' / 'General'</li>
   ToolsOptions
   hToolsOptions ( "StarOffice", "General" )

   '///<li>check data</li>
   printlog " - check the reset data"
   if Zweistellig.GetText <> lsSave ( 1 ) then Warnlog "Year ( 2 digits ) => changes not saved"
   if Aktivieren.IsChecked <> lbSave (1) then Warnlog "Enable HelpAgent => changes not saved"
   Aktivieren.Check
   Zuruecksetzen.Click
   if lbSave (1) = TRUE then Aktivieren.Check else Aktivieren.Uncheck
   if gPlatgroup = "w95" then
      if StarOfficeDialogeBenutzen.IsChecked <> lbSave (2) then Warnlog "Use StarOffice dialogs => changes not saved!"
   end if
   If DruckenStatus.IsChecked <> lbSave(3) then Warnlog "Printing sets 'Document modified' status => changes not saved!"
   if StyleSheet.GetSelText <> lsSave (3) then Warnlog "Style sheet for StarOffice Help => changes not saved!"

   '///<li>close options-dialog with OK</li>
   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   '///</ul>

endcase

