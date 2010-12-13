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
'* short description : general options test (test for internet-group)
'*
'\******************************************************************************

testcase tInternetProxy
   Dim iSave as Integer
   Dim lsSave ( 9 ) as String


   hNewDocument
   ToolsOptions
   hToolsOptions ( "Internet", "Proxy" )

 printlog " - save settings"
   iSave = ProxyServer.GetSelIndex
   ProxyServer.Select ( ProxyServer.GetItemCount )
   if ProxyServer.GetItemCount > 2 then Warnlog "More than 2 entries are available for ProxyServer-Setting => Bug 88387"

   lsSave (1) = HTTPProxy.GetText
   lsSave (2) = HTTPPort.GetText
   lsSave (3) = FTPProxy.GetText
   lsSave (4) = FTPPort.GetText
   lsSave (7) = NoProxy.GetText

 printlog " - change settings"
   HTTPProxy.SetText  "staroffice-news.germany.sun.com.test"
   HTTPPort.SetText   "1508"
   FTPProxy.SetText   "staroffice-news.germany.sun.com.test"
   FTPPort.SetText    "1509"
   NoProxy.SetText  "staroffice;stardivision;starstarstar"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hCloseDocument

  printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "Internet", "Proxy" )

   if ProxyServer.GetSelIndex <> ProxyServer.GetItemCount then
      Warnlog "Proxy Server => changes not saved ( should be the last entry )!"
      ProxyServer.Select ( ProxyServer.GetItemCount )
   end if

   if HTTPProxy.GetText  <> "staroffice-news.germany.sun.com.test" then warnlog "HTTP Proxy => changes not saved!"
   if FTPProxy.GetText   <> "staroffice-news.germany.sun.com.test" then warnlog "FTP Proxy => changes not saved!"
   if NoProxy.GetText    <> "staroffice;stardivision;starstarstar" then warnlog "No Proxy => changes not saved!"
   if HTTPPort.GetText   <> "1508" then warnlog "HTTP Port => changes not saved!"
   if FTPPort.GetText    <> "1509" then warnlog "FTP Port => changes not saved!"

   if gPlatgroup = "w95" then
    printlog " - change settings ( only windows )"
      ProxyServer.Select 2

      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (3)

    printlog " - check changes"
      ToolsOptions
      hToolsOptions ( "Internet", "Proxy" )

      if ProxyServer.GetSelIndex <> 2 then Warnlog "Proxy Server => changes not saved ( should be the 2nd entry )!"
   end if

 printlog " - reset settings"
   ProxyServer.Select(3) ' manual
   HTTPProxy.SetText lsSave (1)
   HTTPPort.SetText lsSave (2)
   FTPProxy.SetText lsSave (3)
   FTPPort.SetText lsSave (4)
   NoProxy.SetText lsSave (7)
   ProxyServer.Select iSave

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

 printlog " - check default"
   ToolsOptions
   hToolsOptions ( "Internet", "Proxy" )
   if ProxyServer.GetSelIndex <> iSave then Warnlog "Proxy Server => changes not saved!"
   if HTTPProxy.GetText  <> lsSave (1) then warnlog "HTTP Proxy => changes not saved!"
   if FTPProxy.GetText   <> lsSave (3) then warnlog "FTP Proxy => changes not saved!"
   if NoProxy.GetText    <> lsSave (7) then warnlog "No Proxy => changes not saved!"
   if FTPPort.GetText    <> lsSave (4) AND FTPPort.GetText   <> "0" then warnlog "FTP Port => changes not saved!"
   if HTTPPort.GetText   <> lsSave (2) AND HTTPPort.GetText  <> "0" then warnlog "HTTP Port => changes not saved!"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

endcase

testcase tInternetSearch
   Dim iSave, iInterim, i as Integer
   Dim lbSave ( 3, 10 ) as boolean
   Dim lsSave ( 6, 10 ) as String


   hNewDocument
   ToolsOptions
   hToolsOptions ( "Internet", "Search" )

 printlog " - save settings"
   iSave = SuchenIn.GetItemCount
   if iSave > 8 then
      Warnlog "Please change the index for this test, there are too many entries in the Search-List!"
      iSave = 8
   end if

   for i=1 to iSave
      SuchenIn.Select i
      lsSave (1, i) = DienstName.GetText
      lsSave (2, i) = Praefix.GetText
      lsSave (3, i) = Suffix.GetText
      lsSave (4, i) = Trenner.GetText
      lsSave (5, i) = Schreibstil.GetSelText

      lbSave (1, i) = Und.IsChecked
      lbSave (2, i) = Oder.IsChecked
      lbSave (3, i) = Exakt.IsChecked
   next i

 printlog " - change/insert settings"
   if iSave > 0 then
      SuchenIn.Select 1
      DienstName.SetText "AAAA_TT_Changes_Name"
      Und.Check
      Praefix.SetText "TT_Changes_Prefix"
      Suffix.SetText "TT_Changes_Suffix"
      Trenner.SetText ";;;;"
      Schreibstil.Select 3
      Sleep (1)
      Aendern.Click
   end if

   Neu.Click
   Sleep (1)
   DienstName.SetText "ZZZZ_TT_New_Name"
   Oder.Check
   Praefix.SetText "TT_New_Prefix"
   Suffix.SetText "TT_New_Suffix"
   Trenner.SetText ":::::"
   Schreibstil.Select 2
   Hinzufuegen.Click

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

   hCloseDocument

  printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "Internet", "Search" )

   iInterim = SuchenIn.GetItemCount
   if iInterim = 0 then
      Warnlog "No entrie in Search-List, entries are not saved! The test ends here! => BugID: 82922"
   else
      if iInterim <> ( iSave + 1 ) then
         Warnlog "Not all entries in the Search-List are saved! => BugID: 82922"
      else
         for i=1 to iInterim
            if i=1 then
               SuchenIn.Select "AAAA_TT_Changes_Name"
               if DienstName.GetText <> "AAAA_TT_Changes_Name" then Warnlog  "AAAA_TT_Changes_Name : Name => changes not saved"
               Und.Check
               if Praefix.GetText <> "TT_Changes_Prefix" then Warnlog "AAAA_TT_Changes_Name : Prefix => changes not saved!"
               if Suffix.GetText  <> "TT_Changes_Suffix" then Warnlog "AAAA_TT_Changes_Name : Suffix => changes not saved!"
               if Trenner.GetText <> ";;;;"              then Warnlog "AAAA_TT_Changes_Name : Seperator => changes not saved!"
               if Schreibstil.GetSelIndex <> 3           then Warnlog "AAAA_TT_Changes_Name : Case match => changes not saved! BugID:86660"
            else
               if i <> ( iSave + 1 ) then
                  SuchenIn.Select i
                  if DienstName.GetText <> lsSave (1, i) then Warnlog  lsSave (1, i) +  " : Name => old entry not saved correctly!"
                  if Und.IsChecked   <> lbSave (1, i) then Warnlog  lsSave (1, i) +  " : Und => old entry not saved correctly!"
                  if Oder.IsChecked  <> lbSave (2, i) then Warnlog  lsSave (1, i) +  " : Oder => old entry not saved correctly!"
                  if Exakt.IsChecked <> lbSave (3, i) then Warnlog  lsSave (1, i) +  " : Exakt => old entry not saved correctly!"
                  if Praefix.GetText <> lsSave (2, i) then Warnlog  lsSave (1, i) +  " : Prefix => old entry not saved correctly!"
                  if Suffix.GetText  <> lsSave (3, i) then Warnlog  lsSave (1, i) +  " : Suffix => old entry not saved correctly!"
                  if Trenner.GetText <> lsSave (4, i) then Warnlog  lsSave (1, i) +  " : Seperator => old entry not saved correctly!"
                  if Schreibstil.GetSelText <> lsSave (5, i) then Warnlog  lsSave (1, i) +  " : Case match => old entry not saved correctly!"
               else
                  SuchenIn.Select "ZZZZ_TT_New_Name"
                  if DienstName.GetText <> "ZZZZ_TT_New_Name" then Warnlog  "ZZZZ_TT_New_Name : Name => new entry not saved correctly!"
                  Oder.Check
                  if Praefix.GetText <> "TT_New_Prefix" then Warnlog  "ZZZZ_TT_New_Name : Prefix => new entry not saved correctly!"
                  if Suffix.GetText  <> "TT_New_Suffix" then Warnlog  "ZZZZ_TT_New_Name : Suffix => new entry not saved correctly!"
                  if Trenner.GetText <> ":::::"         then Warnlog  "ZZZZ_TT_New_Name : Seperator => new entry not saved correctly!"
                  if Schreibstil.GetSelIndex <> 2       then Warnlog  "ZZZZ_TT_New_Name : Case match => new entry not saved correctly!  BugID:86660"
               end if
            end if
         next i
      end if

 printlog " - reset settings"
      SuchenIn.Select "AAAA_TT_Changes_Name"
      DienstName.SetText lsSave (1, 1)
      if lbSave(1, 1) = TRUE then Und.Check
      if lbSave(2, 1) = TRUE then Oder.Check
      if lbSave(3, 1) = TRUE then Exakt.Check
      Praefix.SetText lsSave (2, 1)
      Suffix.SetText lsSave (3, 1)
      Trenner.SetText lsSave (4, 1)
      Schreibstil.Select lsSave (5, 1)
      Sleep (1)
      Aendern.Click
      Sleep (1)

      SuchenIn.Select "ZZZZ_TT_New_Name"
      Sleep (1)
      Loeschen.Click
      Sleep (1)

      Kontext "ExtrasOptionenDlg"
      ExtrasOptionenDlg.OK
      Sleep (3)

 printlog " - check the reset"
      ToolsOptions
      hToolsOptions ( "Internet", "Search" )

      iInterim = SuchenIn.GetItemCount
      if iInterim <> iSave then
         Warnlog "The default can't be reached!"
      else
         for i=1 to iInterim
            SuchenIn.Select lsSave (1, i)
            if DienstName.GetText <> lsSave (1, i) then Warnlog lsSave (1, i) +  " : Name => old entry not saved correctly!"
            if Und.IsChecked <> lbSave (1, i)      then Warnlog lsSave (1, i) +  " : Und => old entry not saved correctly!"
            if Oder.IsChecked <> lbSave (2, i)     then Warnlog lsSave (1, i) +  " : Oder => old entry not saved correctly!"
            if Exakt.IsChecked <> lbSave (3, i)    then Warnlog lsSave (1, i) +  " : Exakt => old entry not saved correctly!"
            if Praefix.GetText <> lsSave (2, i)    then Warnlog lsSave (1, i) +  " : Prefix => old entry not saved correctly!"
            if Suffix.GetText <> lsSave (3, i)     then Warnlog lsSave (1, i) +  " : Suffix => old entry not saved correctly!"
            if Trenner.GetText <> lsSave (4, i)    then Warnlog lsSave (1, i) +  " : Seperator => old entry not saved correctly!"
            if Schreibstil.GetSelText <> lsSave (5, i) then Warnlog lsSave (1, i) +  " : Case match => old entry not saved correctly!"
         next i
      end if
   end if

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   Sleep (3)

endcase


