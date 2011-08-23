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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Tools->Options: OpenOffice.org Fonts
'*
'\******************************************************************************

testcase tOOoFontReplacement
   Dim i, il as Integer

   ToolsOptions
   hToolsOptions ( "StarOffice", "Fontreplacement" )

 printlog " - change settings"
   if Anwenden.IsChecked then
      Warnlog "The default for this page is wrong. 'Apply replacement table' has top be unchecked!"
   else
      Anwenden.Check
   end if

   Schriftart.Select 4
   ErsetzenDurch.Select 6
   Uebernehmen.Click
   sleep 1 'gh13
   Schriftart.Select 10
   ErsetzenDurch.Select 5
   Uebernehmen.Click

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Fontreplacement" )

   if Anwenden.IsChecked = FALSE then
      Warnlog "Apply replacement table' => changes not saved!"
   else
      Anwenden.Check
   end if

   il = Liste.GetItemCount
   if il <> 2 then Warnlog "Not all entries are saved!"

   for i=1 to il
      if i=1 then
         Liste.TypeKeys "<Down><Up>"
      else
         Liste.TypeKeys "<Down>"
      end if
      if i=1 then
         if Schriftart.GetSelIndex <> 4 then Warnlog "Font : the first entrie is not correctly saved!"
         if ErsetzenDurch.GetSelIndex <> 6  then Warnlog "Replace with : the first entrie is not correctly saved!"
      end if
      if i=2 then
         if Schriftart.GetSelIndex <> 10 then Warnlog "Font : the first entrie is not correctly saved!"
         if ErsetzenDurch.GetSelIndex <> 5  then Warnlog "Replace with : the first entrie is not correctly saved!"
      end if
   next i

 printlog " - reset to default"
   for i=1 to il
      Liste.TypeKeys "<Down><Up>"
      Loeschen.Click
      sleep 1 'gh13
   next i
   Anwenden.Uncheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check the default"
   ToolsOptions
   hToolsOptions ( "StarOffice", "Fontreplacement" )

   if Anwenden.IsChecked = TRUE then
      Warnlog "Apply replacement table' => changes not saved!"
   else
      Anwenden.Check
   end if
   if Liste.GetItemCount <> 0 then Warnlog "Not all deleted entries are realy deleted!"
   Anwenden.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)
endcase
