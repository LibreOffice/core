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
'* short description : Tools->Options: OpenOffice.org User Data
'*
'\******************************************************************************


testcase tOOoUserData
   Dim lsSave ( 20 ) as String


   hInitSingleDoc()
   ToolsOptions
   hToolsOptions ( "StarOffice", "UserData" )

 printlog " - save old data"
   lsSave ( 1  ) = Firma.GetText
   lsSave ( 2  ) = VorName.GetText
   lsSave ( 3  ) = ZuName.GetText
   lsSave ( 4  ) = Kuerzel.GetText
   lsSave ( 5  ) = Strasse.GetText
   lsSave ( 6  ) = Land.GetText
   lsSave ( 7  ) = Titel.GetText
   lsSave ( 8  ) = Position.GetText
   lsSave ( 9  ) = TelPriv.GetText
   lsSave ( 10 ) = TelGe.GetText
   lsSave ( 11 ) = Fax.GetText
   lsSave ( 12 ) = EMail.GetText

   if iSprache = 01 then
      lsSave ( 13 ) = City.GetText
      lsSave ( 14 ) = State.GetText
      lsSave ( 15 ) = Zip.GetText
   else
      lsSave ( 16 ) = PLZ.GetText
      lsSave ( 17 ) = Ort.GetText
   end if
   if Apartmentnummer.IsVisible then lsSave ( 18 ) = Apartmentnummer.GetText
   if iSprache = 07 then lsSave ( 19 ) = NameDesVaters.GetText

 printlog " - change data"
   Firma.SetText    "Company name"
   VorName.SetText  "First name"
   ZuName.SetText   "Last name"
   Kuerzel.SetText  "FnLn"
   Strasse.SetText  "Street name"
   Land.SetText     "Land name"
   Titel.SetText    "Title name"
   Position.SetText "Position name"
   TelPriv.SetText  "TelHome number"
   TelGe.SetText    "TelWork number"
   Fax.SetText      "Fax number"
   EMail.SetText    "eMail-adress"

   if iSprache = 01 then
      City.SetText  "City name"
      State.SetText "State name"
      Zip.SetText   "Zip code"
   else
      PLZ.SetText   "PLZ name"
      Ort.SetText   "Ort name"
   end if
   if Apartmentnummer.IsVisible then Apartmentnummer.SetText "Appart number"
   if iSprache = 07 then NameDesVaters.SetText "Farthers name"

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   if ( ExtrasOptionenDlg.notExists( 3 ) ) then
        printlog( "Options closed" )
    endif


   hCloseDocument

  printlog " - exit/restart StarOffice"
   ExitRestartTheOffice

 printlog " - check changes"
   ToolsOptions
   hToolsOptions ( "StarOffice", "UserData" )

   if Firma.GetText    <> "Company name"   then Warnlog "Company => changes not saved"
   if VorName.GetText  <> "First name"     then Warnlog "First name => changes not saved"
   if ZuName.GetText   <> "Last name"      then Warnlog "Last name => changes not saved"
   if Kuerzel.GetText  <> "FnLn"           then Warnlog "Initials => changes not saved"
   if Strasse.GetText  <> "Street name"    then Warnlog "Street => changes not saved"
   if Land.GetText     <> "Land name"      then Warnlog "Land => changes not saved"
   if Titel.GetText    <> "Title name"     then Warnlog "Title => changes not saved"
   if Position.GetText <> "Position name"  then Warnlog "Position => changes not saved"
   if TelPriv.GetText  <> "TelHome number" then Warnlog "Tel-home => changes not saved"
   if TelGe.GetText    <> "TelWork number" then Warnlog "tel-work => changes not saved"
   if Fax.GetText      <> "Fax number"     then Warnlog "Fax => changes not saved"
   if EMail.GetText    <> "eMail-adress"   then Warnlog "eMail => changes not saved"

   if iSprache = 01 then
      if City.GetText  <> "City name"      then Warnlog "City => changes not saved"
      if State.GetText <> "State name"     then Warnlog "State => changes not saved"
      if Zip.GetText   <> "Zip code"       then Warnlog "Zip => changes not saved"
   else
      if PLZ.GetText   <> "PLZ name"       then Warnlog "PLZ => changes not saved"
      if Ort.GetText   <> "Ort name"       then Warnlog "Ort => changes not saved"
   end if
   if Apartmentnummer.IsVisible then
      if Apartmentnummer.GetText <> "Appart number" then Warnlog "Appartment => changes not saved"
   end if
   if iSprache = 07 then
      if NameDesVaters.GetText <> "Farthers name"   then Warnlog "Farthers name => changes not saved"
   end if

 printlog " - reset to saved data"

   Firma.SetText    lsSave ( 1  )
   VorName.SetText  lsSave ( 2  )
   ZuName.SetText   lsSave ( 3  )
   Kuerzel.SetText  lsSave ( 4  )
   Strasse.SetText  lsSave ( 5  )
   Land.SetText     lsSave ( 6  )
   Titel.SetText    lsSave ( 7  )
   Position.SetText lsSave ( 8  )
   TelPriv.SetText  lsSave ( 9  )
   TelGe.SetText    lsSave ( 10 )
   Fax.SetText      lsSave ( 11 )
   EMail.SetText    lsSave ( 12 )

   if iSprache = 01 then
      City.SetText  lsSave ( 13 )
      State.SetText lsSave ( 14 )
      Zip.SetText   lsSave ( 15 )
   else
      PLZ.SetText   lsSave ( 16 )
      Ort.SetText   lsSave ( 17 )
   end if
   if Apartmentnummer.IsVisible then Apartmentnummer.SetText lsSave ( 18 )
   if iSprache = 07 then NameDesVaters.SetText lsSave ( 19 )

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   if ( ExtrasOptionenDlg.notExists( 3 ) ) then
        printlog( "Options closed" )
    endif


 printlog " - check data"

   ToolsOptions
   hToolsOptions ( "StarOffice", "UserData" )

   if Firma.GetText    <> lsSave ( 1 )  then Warnlog "Company => changes not saved"
   if VorName.GetText  <> lsSave ( 2 )  then Warnlog "First name => changes not saved"
   if ZuName.GetText   <> lsSave ( 3 )  then Warnlog "Last name => changes not saved"
   if Kuerzel.GetText  <> lsSave ( 4 )  then Warnlog "Initials => changes not saved"
   if Strasse.GetText  <> lsSave ( 5 )  then Warnlog "Street => changes not saved"
   if Land.GetText     <> lsSave ( 6 )  then Warnlog "Land => changes not saved"
   if Titel.GetText    <> lsSave ( 7 )  then Warnlog "Title => changes not saved"
   if Position.GetText <> lsSave ( 8 )  then Warnlog "Position => changes not saved"
   if TelPriv.GetText  <> lsSave ( 9 )  then Warnlog "Tel-home => changes not saved"
   if TelGe.GetText    <> lsSave ( 10 ) then Warnlog "tel-work => changes not saved"
   if Fax.GetText      <> lsSave ( 11 ) then Warnlog "Fax => changes not saved"
   if EMail.GetText    <> lsSave ( 12 ) then Warnlog "eMail => changes not saved"

   if iSprache = 01 then
      if City.GetText  <> lsSave ( 13 ) then Warnlog "City => changes not saved"
      if State.GetText <> lsSave ( 14 ) then Warnlog "State => changes not saved"
      if Zip.GetText   <> lsSave ( 15 ) then Warnlog "Zip => changes not saved"
   else
      if PLZ.GetText   <> lsSave ( 16 ) then Warnlog "PLZ => changes not saved"
      if Ort.GetText   <> lsSave ( 17 ) then Warnlog "Ort => changes not saved"
   end if
   if Apartmentnummer.IsVisible then
      if Apartmentnummer.GetText <> lsSave ( 18 ) then Warnlog "Appartment => changes not saved"
   end if
   if iSprache = 07 then
      if NameDesVaters.GetText <> lsSave ( 19 ) then Warnlog "Farthers name => changes not saved"
   end if

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
   if ( ExtrasOptionenDlg.notExists( 3 ) ) then
        printlog( "Options closed" )
    endif


endcase


