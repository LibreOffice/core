'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: NewSortingAlgorithmForJapanese_1.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'* owner : hercule.li@sun.com
'*
'* short description : New Sorting Algorithm For Japanese Test
'*
'\***********************************************************************

testcase NewSortingForJapanese_1

'///Check if editbox "Entry Phonetic Reading " appear or not -- insert/index and tables /entry
  call  hNewDocument

  '/// turn off "Asian Language support" , check if editbox "Entry Phonetic Reading " appear is invisible
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

  '/// turn on "Asian Language support" , check if editbox "Entry Phonetic Reading " appear is visible
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

'/// Check if editbox "Entry Phonetic Reading " is enabled or not  -- insert/index and tables /entry
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

'/// Check the number of key type in sort area when choose japanese language
  call  hNewDocument

'/// Open with Insert/Index and tables/Index and tables / tabpage index/table
    InsertIndexes
    Kontext
    Active.SetPage TabVerzeichnisseVerzeichnis
    Kontext "TabVerzeichnisseVerzeichnis"

'/// choose Alphabetical index in type and choose Japanese Language
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

'/// Check the number of key type in sort area
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

'/// Check the number of key type in data/sort
  call  hNewDocument
  DocumentWriter.TypeKeys "test"
  DocumentWriter.TypeKeys "<Shift Home>"

'/// Open Tools/Sort
  ToolsSort
    Kontext "Sortieren"

'/// choose Alphabetical index in type and choose Japanese Language
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

'/// Check the number of key type in sort area
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

'/// Check the number of key type in data/sort -- Calc
  call  hNewDocument

'/// Open Data/Sort  ,tabpage option
   DataSort
   Kontext
   active.SetPage TabSortierenOptionen
   Kontext "TabSortierenOptionen"

'/// choose Alphabetical index in type and choose Japanese Language
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

'/// Check the number of key type in sort area
    if Optionen.GetItemCount <> iNumberOfOption then
       warnlog "The number of option is wrong, hope :" +iNumberOfOption + " but get:" +Optionen.GetItemCount
    end if

    TabSortierenOptionen.Cancel

  Call hCloseDocument

endcase

'-------------------------------------------------------------------------
