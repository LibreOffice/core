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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Test VBA settings-page
'*
'\******************************************************************************

testcase tLoadSaveVBA

   Dim lbSave ( 7 ) as Boolean


   hNewDocument
   ToolsOptions
   hToolsOptions ( "LoadSave", "VBAProperties" )

 printlog " - save old settings"
   lbSave ( 1 ) = WinwordBasicLaden.IsChecked
   lbSave ( 2 ) = WinwordBasicSpeichern.IsChecked
   lbSave ( 3 ) = ExcelBasicLaden.IsChecked
   lbSave ( 4 ) = ExcelBasicSpeichern.IsChecked
   lbSave ( 5 ) = PowerpointBasicLaden.IsChecked
   lbSave ( 6 ) = PowerpointBasicSpeichern.IsChecked

 printlog " - invert settings"
   if lbSave ( 1 ) = TRUE then WinwordBasicLaden.UnCheck else WinwordBasicLaden.Check
   if lbSave ( 2 ) = TRUE then WinwordBasicSpeichern.UnCheck else WinwordBasicSpeichern.Check
   if lbSave ( 3 ) = TRUE then ExcelBasicLaden.UnCheck else ExcelBasicLaden.Check
   if lbSave ( 4 ) = TRUE then ExcelBasicSpeichern.UnCheck else ExcelBasicSpeichern.Check
   if lbSave ( 5 ) = TRUE then PowerpointBasicLaden.UnCheck else PowerpointBasicLaden.Check
   if lbSave ( 6 ) = TRUE then PowerpointBasicSpeichern.UnCheck else PowerpointBasicSpeichern.Check

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hCloseDocument

 printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check inverting"
   ToolsOptions
   hToolsOptions ( "LoadSave", "VBAProperties" )

   if WinwordBasicLaden.IsChecked        = lbSave ( 1 ) then Warnlog "'Load Winword basic' => changes not saved!"
   if WinwordBasicSpeichern.IsChecked    = lbSave ( 2 ) then Warnlog "'Save Winword basic' => changes not saved!"
   if ExcelBasicLaden.IsChecked          = lbSave ( 3 ) then Warnlog "'Load Excel basic' => changes not saved!"
   if ExcelBasicSpeichern.IsChecked      = lbSave ( 4 ) then Warnlog "'Save Excel basic' => changes not saved!"
   if PowerpointBasicLaden.IsChecked     = lbSave ( 5 ) then Warnlog "'Load Powerpoint basic' => changes not saved!"
   if PowerpointBasicSpeichern.IsChecked = lbSave ( 6 ) then Warnlog "'Save Powerpoint basic' => changes not saved!"

 printlog " - make other changes"
   WinwordBasicLaden.Check
   WinwordBasicSpeichern.Uncheck
   ExcelBasicLaden.Uncheck
   ExcelBasicSpeichern.Check
   PowerpointBasicLaden.Uncheck
   PowerpointBasicSpeichern.Check

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "LoadSave", "VBAProperties" )

   if WinwordBasicLaden.IsChecked        <> TRUE  then Warnlog "'Load Winword basic' => changes not saved!"
   if WinwordBasicSpeichern.IsChecked    <> FALSE then Warnlog "'Save Winword basic' => changes not saved!"
   if ExcelBasicLaden.IsChecked          <> FALSE then Warnlog "'Load Excel basic' => changes not saved!"
   if ExcelBasicSpeichern.IsChecked      <> TRUE  then Warnlog "'Save Excel basic' => changes not saved!"
   if PowerpointBasicLaden.IsChecked     <> FALSE then Warnlog "'Load Powerpoint basic' => changes not saved!"
   if PowerpointBasicSpeichern.IsChecked <> TRUE  then Warnlog "'Save Powerpoint basic' => changes not saved!"

 printlog " - reset to saved settings"

   if lbSave ( 1 ) = TRUE then WinwordBasicLaden.Check else WinwordBasicLaden.UnCheck
   if lbSave ( 2 ) = TRUE then WinwordBasicSpeichern.Check else WinwordBasicSpeichern.UnCheck
   if lbSave ( 3 ) = TRUE then ExcelBasicLaden.Check else ExcelBasicLaden.UnCheck
   if lbSave ( 4 ) = TRUE then ExcelBasicSpeichern.Check else ExcelBasicSpeichern.UnCheck
   if lbSave ( 5 ) = TRUE then PowerpointBasicLaden.Check else PowerpointBasicLaden.UnCheck
   if lbSave ( 6 ) = TRUE then PowerpointBasicSpeichern.Check else PowerpointBasicSpeichern.UnCheck

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check settings"
   ToolsOptions
   hToolsOptions ( "LoadSave", "VBAProperties" )

   if WinwordBasicLaden.IsChecked        <> lbSave ( 1 ) then Warnlog "'Load Winword basic' => changes not saved!"
   if WinwordBasicSpeichern.IsChecked    <> lbSave ( 2 ) then Warnlog "'Save Winword basic' => changes not saved!"
   if ExcelBasicLaden.IsChecked          <> lbSave ( 3 ) then Warnlog "'Load Excel basic' => changes not saved!"
   if ExcelBasicSpeichern.IsChecked      <> lbSave ( 4 ) then Warnlog "'Save Excel basic' => changes not saved!"
   if PowerpointBasicLaden.IsChecked     <> lbSave ( 5 ) then Warnlog "'Load Powerpoint basic' => changes not saved!"
   if PowerpointBasicSpeichern.IsChecked <> lbSave ( 6 ) then Warnlog "'Save Powerpoint basic' => changes not saved!"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK

endcase
