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
'* owner : hercule.li@oracle.com
'*
'* short description : New Sorting Algorithm For Japanese Test
'*
'\***********************************************************************

testcase NewSortingForJapanese_1

  call  hNewDocument

  if iSystemSprache <> 81 AND iSystemSprache <> 82 AND iSystemSprache <> 86 AND iSystemSprache <> 88 then
     Call CheckAsianLanguageSupport("Off")
     Sleep 1

      Kontext
      Sleep 3
      InsertIndexesEntry
      Sleep 2
      Kontext "VerzeichniseintragEinfuegen"
      Sleep 1
        Verzeichniseintrag.SetText "test"
        if EntryPhoneticReading.IsVisible = TRUE then warnlog "The Entry Phonetic Reading editbox should NOT be visible!"
      VerzeichniseintragEinfuegen.Close
  end if

  Call CheckAsianLanguageSupport("On")

   Kontext
   Sleep 3
   InsertIndexesEntry
   Sleep 2
   Kontext "VerzeichniseintragEinfuegen"
   Sleep 1
     Verzeichniseintrag.SetText "test"
     if EntryPhoneticReading.IsVisible <> TRUE then warnlog "The Entry Phonetic Reading editbox should be visible!"
   VerzeichniseintragEinfuegen.Close

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase NewSortingForJapanese_2

  call  hNewDocument

   InsertIndexesEntry
   Sleep 3
   Kontext "VerzeichniseintragEinfuegen"
   Sleep 2
     if EntryPhoneticReading.IsEnabled = TRUE then warnlog "The Entry Phonetic Reading editbox should NOT be enabled!"

     Verzeichniseintrag.SetText "test"

     if EntryPhoneticReading.IsEnabled <> TRUE then warnlog "The Entry Phonetic Reading editbox should be enabled!"
   VerzeichniseintragEinfuegen.Close

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase NewSortingForJapanese_3

  Dim iTypeList , iNumberOfKeyType as Integer
  Dim sJapaneseLocation            as String
  Dim i as integer

  iTypeList         = 2
  iNumberOfKeyType  = 4
  sJapaneseLocation = fGetCountryName(81)

  if Len(sJapaneseLocation) = 0 then Goto endsub

  call  hNewDocument

    InsertIndexes
    Kontext
    Active.SetPage TabVerzeichnisseVerzeichnis
    Kontext "TabVerzeichnisseVerzeichnis"

    VerzeichnisTyp.Select iTypeList
    try
        SortLanguage.Select (sJapaneseLocation)
    catch
        qaErrorLog "Entry not found: '" + sJapaneseLocation + "' trying english entry: 'Japanese'"
        try
            SortLanguage.Select ("Japanese")
        catch
            warnlog "'Japanese' isn't available either"
            for i = 1 to SortLanguage.getItemCount
                printlog "" + i + ": '" + SortLanguage.getItemText(i) + "'"
            next i
        endcatch
    endcatch
    Sleep 1

    if SortKeyType.GetItemCount <> iNumberOfKeyType then
       warnlog "The number of key type is wrong, hope :" +iNumberOfKeyType + " but get:" +SortKeyType.GetItemCount
    end if
try
    TabVerzeichnisseVerzeichnis.Cancel
catch
warnlog "#135631# crash on canceling Insert Indexes and Tables"
endcatch
  Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase NewSortingForJapanese_4

  Dim iNumberOfKeyType   as Integer
  Dim sJapaneseLocation  as String

  iNumberOfKeyType  = 4
  sJapaneseLocation = fGetCountryName(81)

  if Len(sJapaneseLocation) = 0 then Goto endsub

  call  hNewDocument
  DocumentWriter.TypeKeys "test"
  DocumentWriter.TypeKeys "<Shift Home>"

  ToolsSort
    Kontext "Sortieren"

    try
        Sprache.Select  sJapaneseLocation
    catch
        qaErrorLog "Entry not found: '" + sJapaneseLocation + "' trying english entry: 'Japanese'"
        try
            Sprache.Select ("Japanese")
        catch
            warnlog "'Japanese' isn't available either"
            for i = 1 to Sprache.getItemCount
                printlog "" + i + ": '" + Sprache.getItemText(i) + "'"
            next i
        endcatch
    endcatch
    Sleep 1

    if Schluesseltyp1.GetItemCount <> iNumberOfKeyType then
       warnlog "The number of key type is wrong, hope :" +iNumberOfKeyType + " but get:" +Schluesseltyp1.GetItemCount
    end if

    Sortieren.Cancel

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase NewSortingForJapanese_5

  Dim iNumberOfOption    as Integer
  Dim sJapaneseLocation  as String

  iNumberOfOption   = 3
  sJapaneseLocation = fGetCountryName(81)

  if Len(sJapaneseLocation) = 0 then Goto endsub

  call  hNewDocument

   DataSort
   Kontext
   active.SetPage TabSortierenOptionen
   Kontext "TabSortierenOptionen"

    try
        Sprache.Select  sJapaneseLocation
    catch
        qaErrorLog "Entry not found: '" + sJapaneseLocation + "' trying english entry: 'Japanese'"
        try
            Sprache.Select ("Japanese")
        catch
            warnlog "'Japanese' isn't available either"
            for i = 1 to Sprache.getItemCount
                printlog "" + i + ": '" + Sprache.getItemText(i) + "'"
            next i
        endcatch
    endcatch
    Sleep 1

    if Optionen.GetItemCount <> iNumberOfOption then
       warnlog "The number of option is wrong, hope :" +iNumberOfOption + " but get:" +Optionen.GetItemCount
    end if

    TabSortierenOptionen.Cancel

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------
