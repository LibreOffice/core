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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description :
'*
'********************************************************************
'*
' #1 tToolsOptions
' #1 tToolsConfigure
' #1 tToolsCatalog
' #1 tToolsCatalogNew
'*
'\******************************************************************

sub m_105
   printlog Chr(13) +  "--  m_105  --"
   Call tToolsOptions
   Call tToolsConfigure
'   tToolsCatalog get's called in m_101_.inc to get it also printed there!
   Call tToolsCatalogNew
end sub

testcase tToolsOptions
   dim irgendwas(7) as boolean
   dim sProzent as string
   dim sTemp as string
   dim i as integer
   dim iItemCount as integer

   '/// open application ///'
   Call hNewDocument
   '/// Tools->Options ///'
  printlog ("'/// - save states ///")
   ToolsOptions
   sleep 1
   hToolsOptions ("MATH", "SETTINGS")

      irgendwas(1) = DruckeTitelzeile.IsChecked
      irgendwas(2) = DruckeFormeltext.IsChecked
      irgendwas(3) = DruckeRahmen.IsChecked
      irgendwas(4) = Originalgroesse.IsChecked
      irgendwas(5) = DerSeiteAnpassen.IsChecked
      irgendwas(6) = Skalierung.IsChecked
      irgendwas(7) = Ignorieren.IsChecked
      if Skalierung.IsChecked then sProzent = Prozent.GetText

     printlog ("'/// - all states inverting ///")

      if irgendwas(1) Then DruckeTitelzeile.UnCheck Else DruckeTitelzeile.Check
      if irgendwas(2) Then DruckeFormeltext.UnCheck Else DruckeFormeltext.Check
      if irgendwas(3) Then DruckeRahmen.UnCheck Else     DruckeRahmen.Check
      if irgendwas(7) Then Ignorieren.UnCheck Else       Ignorieren.Check

      Skalierung.Check
      Prozent.ToMax
      sTemp = Prozent.GetText

   Kontext "OptionenDlg"
   OptionenDlg.OK

  printlog ("'/// - SO quit - start ///")

TBOrestart

  printlog ("'/// - checking states ///")
   ToolsOptions
   hToolsOptions ("MATH", "SETTINGS")

      If ( irgendwas(1) = DruckeTitelzeile.IsChecked ) Then WarnLog "DruckeTitelzeile state changed"
      If ( irgendwas(2) = DruckeFormeltext.IsChecked ) Then WarnLog "DruckeFormeltext state changed"
      If ( irgendwas(3) = DruckeRahmen.IsChecked     ) Then WarnLog "DruckeRahmen     state changed"
      If ( irgendwas(7) = Ignorieren.IsChecked       ) Then WarnLog "Ignorieren       state changed"

      If True <> Skalierung.IsChecked     Then WarnLog "Skalierung       state changed"
      If sTemp <> Prozent.GetText Then         WarnLog "Prozent          state changed"

  printlog ("'/// - all UnCheck -> o ///")

      DruckeTitelzeile.UnCheck
      DruckeFormeltext.UnCheck
      DruckeRahmen.UnCheck
      Ignorieren.UnCheck

   Kontext "OptionenDlg"
   OptionenDlg.OK

  printlog ("'/// - check if all UnChecked ///")
   ToolsOptions
   hToolsOptions ("MATH", "SETTINGS")

      If DruckeTitelzeile.IsChecked Then WarnLog "DruckeTitelzeil  x"
      If DruckeFormeltext.IsChecked Then WarnLog "DruckeFormeltext x"
      If DruckeRahmen.IsChecked     Then WarnLog "DruckeRahmen     x"
      If Ignorieren.IsChecked       Then WarnLog "Ignorieren       x"

      If sTemp <> Prozent.GetText Then         WarnLog "Prozent          state changed"

     Printlog ("'/// - all Check -> x ///")

      DruckeTitelzeile.Check
      DruckeFormeltext.Check
      DruckeRahmen.Check
      Ignorieren.Check

      DerSeiteAnpassen.Check

   Kontext "OptionenDlg"
   OptionenDlg.OK

  printlog ("'/// - check if all Checked ///")
      '----- WorkAround i48383 Tools->Options run on wrong window; Target OOo later ---------
      kontext "DocumentMath"
      DocumentMath.mouseDown 10,10
      DocumentMath.mouseUp 10,10
      '-----------------------------------------------
   ToolsOptions
   hToolsOptions ("MATH", "SETTINGS")

      try
          If True <> DruckeTitelzeile.IsChecked Then WarnLog "DruckeTitelzeil  o"
      catch
          Kontext "ExtrasOptionenDlg"
          warnlog "Getting source of error... TBO: " 
          iItemCount = Optionsliste.getItemCount
          for i = 1 to iItemCount
              printlog "("+i+"/"+iItemCount+"): '"+ Optionsliste.getItemText(i)+ "'"
          next i
          Kontext "TabDruckenMath"
      endcatch
      If True <> DruckeFormeltext.IsChecked Then WarnLog "DruckeFormeltext o"
      If True <> DruckeRahmen.IsChecked     Then WarnLog "DruckeRahmen     o"
      If True <> Ignorieren.IsChecked       Then WarnLog "Ignorieren       o"

      If True <> DerSeiteAnpassen.IsChecked       Then WarnLog "DerSeiteAnpassen    o"

  printlog ("'/// - restore states ///")

      If ( irgendwas(1) = TRUE ) Then  DruckeTitelzeile.Check Else DruckeTitelzeile.UnCheck
      If ( irgendwas(2) = TRUE ) Then  DruckeFormeltext.Check Else DruckeFormeltext.UnCheck
      If ( irgendwas(3) = TRUE ) Then  DruckeRahmen.Check Else     DruckeRahmen.UnCheck
      If ( irgendwas(4) = TRUE ) Then  Originalgroesse.Check
      If ( irgendwas(5) = TRUE ) Then  DerSeiteAnpassen.Check
      If ( irgendwas(6) = TRUE ) Then  Skalierung.Check
      If ( irgendwas(7) = TRUE ) Then  Ignorieren.Check Else       Ignorieren.UnCheck

      if Skalierung.IsChecked then Prozent.SetText (sProzent)

   '/// close dialog 'Options' with OK ///'
   Kontext "OptionenDlg"
   OptionenDlg.OK
   '/// close application ///'
   Call hCloseDocument
endcase

sub TBOrestart
    hCloseDocument
    sleep (5)
    ExitRestartTheOffice
    sleep (10)
    hNewDocument
    sleep (5)
end sub

testcase tToolsConfigure
    '/// idea from #108930 find double entries of keyboard-keys ///'
    dim sKeys(600,2) as string 
    dim i,a,b as integer
    dim sTemp, sTemp2 as string
    dim iBugCount(2) as integer
    dim c as integer
    
    '/// open application ///'
    Call hNewDocument
    sleep 2
    '/// Tools->Configure ///'
    ToolsCustomize
    sleep 3
    Kontext
    '/// switch to tabpage 'Keyboard' ///'
    Messagebox.SetPage TabTastatur         ' 2 ------------------
    Kontext "TabTastatur"
    '/// Check Checkbox 'StarOffice' ///'
    StarOffice.Check
    Printlog ("-------------------- Keylist for StarOpenOfficeSuite.org --------------------")
    i = Tastatur.GetItemCount
    sKeys(0,1) = i
    sKeys(0,2) = i
    for a = 1 to i
	try
            sKeys(a,1) = Tastatur.GetItemText(a,1)
	    c = 2
	catch
	    sKeys(a,1) = Tastatur.GetItemText(a,2)
	    c = 3
	endcatch
        try
	    sKeys(a,2) = Tastatur.GetItemText(a,c)
        catch
            sKeys(a,2) = ""
            iBugCount(1) = iBugCount(1) + 1
        endcatch
        printlog " " + a + " - '" + sKeys(a,1) + "' -- '" + sKeys(a,2) + "'"
    next a
    sortList(sKeys())
    sTemp2 = sKeys(1,1)
    for a = 1 to i
        sTemp = Stemp2
        sTemp2 = sKeys(a+1,1)
        if (sTemp = sTemp2) then
            qaErrorLog " " + a + ". This Key is double: '" + sTemp + "' - These Functions are assigned: '" + sKeys(a,2) + "' + '" + sKeys(a+1,2) + "'"
            iBugCount(2) = iBugCount(2) + 1
        endif
    next a

    '/// Check Checkbox 'Math' ///'
    Application.Check
    Printlog ("-------------------- Keylist for Formula ------------------------")
    i = Tastatur.GetItemCount
    sKeys(0,1) = i
    sKeys(0,2) = i
    for a = 1 to i
        sKeys(a,1) = Tastatur.GetItemText(a,c-1)
        try
            sKeys(a,2) = Tastatur.GetItemText(a,c)
        catch
            sKeys(a,2) = ""
            iBugCount(1) = iBugCount(1) + 1
        endcatch
        printlog " " + a + " - '" + sKeys(a,1) + "' -- '" + sKeys(a,2) + "'"
    next a
    sortList(sKeys())
    sTemp2 = sKeys(1,1)
    for a = 1 to i
        sTemp = Stemp2
        sTemp2 = sKeys(a+1,1)
        if (sTemp = sTemp2) then
            qaErrorLog " " + a + ". This Key is double: '" + sTemp + "' - These Functions are assigned: '" + sKeys(a,2) + "' + '" + sKeys(a+1,2) + "'"
            iBugCount(2) = iBugCount(2) + 1
        endif
    next a
    if (iBugCount(1) > 0) then warnlog "column-entry is empty -> testtool issue"
    if (iBugCount(2) > 0) then qaErrorLog "key-names are double"
    '/// cancel dialog 'Customize' ///'
    TabTastatur.cancel
    '/// close application ///'
    Call hCloseDocument
endcase

sub sortList(lsList() as String)
    '/// special variation of global routine, because this array is 2-dimensional ///'
    Dim Listenanzahl as Integer, i as Integer, j as Integer
    Dim Zwischenspeicher as String
    Dim Zwischenspeicher2 as String

    ListenAnzahl = Val(lsList(0,1))
    for i=ListenAnzahl-1 to 1 step -1
        for j=1 to i
            if uCase ( lsList(j,1) ) > uCase ( lsList(j+1,1) ) then            ' downward sorting
                Zwischenspeicher = lsList (j,1)                               ' invert value (i) with value (i+1)
                Zwischenspeicher2 = lsList (j,2)                              ' invert value (i) with value (i+1)
                lsList (j,1) = lsList(j+1,1)
                lsList (j,2) = lsList(j+1,2)
                lsList (j+1,1) = Zwischenspeicher
                lsList (j+1,2) = Zwischenspeicher2
            end if
        next j
    next i
end sub

testcase tToolsCatalog
'   tToolsCatalog get's called in m_101_.inc to get it also printed there!
    '/// idea from #111622 Crash on inserting 21. math ole-object ... ///'
    dim i, j, x(10), y, z as integer
    dim sTemp as string
    dim iSymbolSets as integer
    dim bChecked as boolean
    dim sFileName as string
    dim sAllSymbols as string
    dim lAllSymbols(200) as string
    dim lAllSymbolsSort(200) as string
    dim iTimeOut as integer
    dim sFilterName as string
    dim sFilter as string
    dim sFilterExt() as string
    dim sNotAllowed() as string
    
   '/// open Writer application ///'
    gApplication = "WRITER"
    Call hNewDocument
    '/// Insert->Object->Formula ///'
    InsertObjectFormulaWriter
    Kontext "MATH"
    '/// Tools->Catalog ///'
    try
        ToolsSymbolsCatalog
    catch
        warnlog "some slots in chart/math are not working."
        call hCloseDocument
        goto endsub
    endcatch
        Kontext "SymboleMath"
        symbols.typekeys "<right><right>"
        symbols.typekeys "<right><right>"
        '/// click button "Edit..." ///'
        Bearbeiten.Click
            Kontext "EditSymbols"
        symbols.typekeys "<right><right>"
        symbols.typekeys "<right><right>"
            '/// There are 2 Symbol sets: 1. 'Greek' with 54 entries and 2. 'Special' with 12 entries ///'
            '///+ These symbol set names are availble in 2 listboxes 1. 'Old Symbol Set' and 2. 'Symbol Set' ///'
            iSymbolSets = OldSymbolSet.GetItemCount
            if (iSymbolSets <> 3) then
                WarnLog "- OldSymbolSet: '" + iSymbolSets + "' is not 3; expected is 3. ('Special' and 'Greek' and 'iGreek')"
                for i = 1 to iSymbolSets
                    qaErrorLog "- " + i + ": '" + OldSymbolSet.GetItemText(i) + "'"
                next i
            else
                printlog "- OldSymbolSet: " + iSymbolSets
            endif
            OldSymbolSet.Select 1
            SymbolSet.Select 1
            x(1) = OldSymbol.GetItemCount
            printlog "- 1: OldSymbol: " + x(1)
            if (iSymbolSets <> SymbolSet.GetItemCount) then
                WarnLog "Count is different; OldSymbolSet: '" + iSymbolSets + "'; SymbolSet: '" + SymbolSet.GetItemCount + "'"
            endif
            printlog "- 1:    Symbol: " + Symbol.GetItemCount
            printlog "-    SymbolSet: " + SymbolSet.GetItemCount
            OldSymbolSet.Select 2
            SymbolSet.Select 2
            x(2) = OldSymbol.GetItemCount
            printlog "- 2: OldSymbol: " + x(2)
            printlog "- 2:    Symbol: " + Symbol.GetItemCount
            printlog "-    SymbolSet: " + SymbolSet.GetItemCount
            '/// cancel dialog 'Edit Symbols' ///'
            EditSymbols.Cancel
        Kontext "SymboleMath"
        '/// close dialog 'Symbols' ///'
        SymboleMath.close
    Kontext "DocumentWriter"
    '///+ click into writer document to leave math  ///'
    DocumentWriter.mouseDown 1,1
    DocumentWriter.mouseUp   1,1
    '/// type key [delete] in writer document to delete (emtpy) math object ///'
    DocumentWriter.typeKeys "<Delete>"
    sleep 1
        sAllSymbols = ""
        lAllSymbols(0) = "0"
        '/// for all symbol sets do... ///'
        for i = 1 to iSymbolSets
            '///+ insert for the count of symbols for each symbol set the symbol into the document  ///'
            for j = 1 to x(i)
                printlog "(" + i + "/" + iSymbolSets + ")(" + j + "/" + x(i) + ")"
                try
                    '///+ Insert->Object->Formula ///'
                    InsertObjectFormulaWriter ' culprint after 20 calls :-( in #111622
                catch
                    qaErrorLog "Can not leave object selection"
                    ' somehow not triggerable on macos
                    call hCloseDocument
                    goto endsub
                endcatch
                Kontext "DocumentMath"
                '///+ Tools->Catalog ///'
                ToolsSymbolsCatalog
                Kontext "SymboleMath"
                '///+ select the symbolset ///'
                Symbolset.Select i
                Kontext "SymboleMath"
                '///+ type key [home] on dialog 'Symbols'///'
                Symbols.TypeKeys "<Home>", true
                Symbols.TypeKeys "<right><left>", true ' because <home> doesn't select the first item
                '///+ type key [right] i++ times, to jump to next symbol///'
                for y = 2 to j
                    Symbols.TypeKeys "<right>", true
                next y
                '///+ click button 'Insert' ///'
                Uebernehmen.Click
                sleep 1
                '///+ close dialog 'Symbols' ///'
                try
                    SymboleMath.close
                catch
                    if ((2=i)AND(11=j))then
                        '#i42011# 
                        warnlog "Expected: Timeout on closing Tools->Catalog with this character. (" + i + "/" + iSymbolSets + ")(" + j + "/" + x(i) + ")"
                    else
                        warnlog "Not Expected! Timeout on closing Tools->Catalog with this character. (" + i + "/" + iSymbolSets + ")(" + j + "/" + x(i) + ")"
                    endif
                    iTimeOut = 1
                    while ((iTimeOut > 0) AND (iTimeOut < 30))
                        try
                            Kontext "CommandsMath"
                            Commands.typeKeys "<mod1 a>"
                            iTimeOut = iTimeOut * (-1)
                        catch
                            sleep (10)
                            inc(iTimeOut)
                        endcatch
                    wend
                    printlog "Duration: " + (iTimeOut * 10)
                endcatch
                '///+ select text in 'Commands' window ///'
                Kontext "CommandsMath"
                Commands.typeKeys "<mod1 a>"
                '/// Edit->Copy ///'
                sleep 1
                try
                    EditCopy
                catch
                    commands.typekeys "<mod1 c>"
                    qaerrorlog "Workaround failed slot EditCopy with <Strg>+c:"+getClipboardText()
                endcatch
                sTemp = getClipboardText()
                Kontext "DocumentWriter"
                '///+ click into writer document to leave math  ///'
                DocumentWriter.mouseDown 1,1
                DocumentWriter.mouseUp   1,1
                '///+ click into writer document to diable selection of math object  ///'
                DocumentWriter.mouseDown 1,1
                DocumentWriter.mouseUp   1,1
                sleep 1
                '///+ insert the copied text into the document ///'
                DocumentWriter.typeKeys "<Mod1 End> - " + sTemp + ": " + j + "<Tab>"
                sAllSymbols = sAllSymbols + sTemp
                listAppend(lAllSymbols(), rtrim(sTemp))
            next j
        next i

    if (listCount(lAllSymbols()) <> 108) then
        warnlog "There have to be 108 Symbols, but there are: " + listCount(lAllSymbols())
    endif
    ' TODO: check sAllSymbols for not allowed characters!
    listCopy(lAllSymbols(),lAllSymbolsSort())
    listSort(lAllSymbolsSort())
    sTemp = lAllSymbolsSort(1)
    for i = 2 to listCount(lAllSymbolsSort())
        if (sTemp = lAllSymbolsSort(i)) then
            warnlog "There are Symbol names double: '" + sTemp + "'"
            for j = 1 to listCount(lAllSymbols())
                 If (sTemp = lAllSymbols(j)) then
                    printlog "" + j + ": after: " + lAllSymbols(j-1)
                 endif
            next j
        endif
        sTemp = lAllSymbolsSort(i)
    next i
    ' Check for not allowed characters in strings
    sNotAllowed = array(" ","_","..")
    for i = 1 to listCount(lAllSymbols())
        for j = 0 to uBound(sNotAllowed())
            if inStr(lAllSymbols(i), sNotAllowed(j)) > 0 then
                warnlog "("+i+") Character '" + sNotAllowed(j) + "' is not allowed in string: '" + lAllSymbols(i) + "'"
            endif
        next j
    next i
    sFileName = convertpath( gOfficePath + "user/work/math/level1/ftc_a.odt")
    
    hFileSaveAsWithFilterKill (sFileName, "writer8")
    '/// close application ///'
    Call hCloseDocument
endcase

testcase tToolsCatalogNew
    dim i, j as integer
    dim sTemp as string
    dim iStartingSymbolSetCount as integer
    
    Call hNewDocument
'1
    Kontext "MATH"
    '/// Tools->Catalog ///'
    try
        ToolsSymbolsCatalog
    catch
        warnlog "some slots in chart/math are not working."
        call hCloseDocument
        goto endsub
    endcatch
        Kontext "SymboleMath"
        if SymboleMath.exists then
            '/// click button "Edit..." ///'
            Bearbeiten.Click
                Kontext "EditSymbols"
                if EditSymbols.exists then
                    iStartingSymbolSetCount = SymbolSet.getItemCount
                    if iStartingSymbolSetCount <> 3 then
                        qaErrorLog "There are already other symbolsets, instead of 3: " + iStartingSymbolSetCount
                        for i = 1 to iStartingSymbolSetCount
                            printlog "("+i+"/"+iStartingSymbolSetCount+"): '" + SymbolSet.getItemText (i) + "'"
                        next i
                    else
                        printlog "Entry condition met; 2 symbolsets only available"
                    endif
                    '/// type 'TBOnewSymbolSet' into the combobox 'Symbol set' ///'
                    SymbolSet.setText "TBOnewSymbolSet"
                    '/// press buton 'Modify' to add the new Symbolset ///'
                    modify.click
                    EditSymbols.ok
                else
                    warnlog "not there"
                endif
            Kontext "SymboleMath"
            if SymboleMath.exists then
            sleep 1
            '/// select the new symbol set ///'
            try
                Symbolset.select "TBOnewSymbolSet"
            catch
                j=Symbolset.getItemCount
                warnLog "New symbolset not created: " + "TBOnewSymbolSet"
                for i = 1 to j
                    printlog "("+i+"/"+j+"): '" + SymbolSet.getItemText (i) + "'"
                next i
            endcatch
            '/// close dialog 'Symbols' ///'
            SymboleMath.close
            else
                warnlog "no dialog"
            endif
        endif
    '/// exit and restart OOo ///'
    exitRestartTheOffice
'2    
    Call hNewDocument
    Kontext "MATH"
    '/// Tools->Catalog ///'
    try
        ToolsSymbolsCatalog
    catch
        warnlog "some slots in chart/math are not working."
        call hCloseDocument
        goto endsub
    endcatch
        Kontext "SymboleMath"
        if SymboleMath.exists then
            '/// click button "Edit..." ///'
            Bearbeiten.Click
                Kontext "EditSymbols"
                if EditSymbols.exists then
                    i = SymbolSet.getItemCount
                    if (iStartingSymbolSetCount +1) <> i then
                        WarnLog "the new symbolset is not available after restart: " + i + "; should be: " + (iStartingSymbolSetCount +1)
                    endif
                    try
                        SymbolSet.select "TBOnewSymbolSet"
                    catch
                        'i64504
                        qaerrorlog "Symbol set name is not saved on exiting OOo"
                        SymbolSet.select ""
                    endcatch
                    SymbolSet.setText "TBOnewSymbolSet"
                    Symbol.setText "ALPHANEWTBO"
                    try
                        add.click
                    catch
                        qaerrorlog "no add"
                    endcatch
                    EditSymbols.ok
                endif
            Kontext "SymboleMath"
            if SymboleMath.exists then
                sleep 1
                '/// select the new symbol set ///'
                Symbolset.select "TBOnewSymbolSet"
                '/// close dialog 'Symbols' ///'
                SymboleMath.close
            else
                warnlog "no dialog"
            endif
        endif
    '/// exit and restart OOo ///'
    exitRestartTheOffice
'3
    Call hNewDocument
    Kontext "MATH"
    '/// Tools->Catalog ///'
    try
        ToolsSymbolsCatalog
    catch
        warnlog "some slots in chart/math are not working."
        call hCloseDocument
        goto endsub
    endcatch
        Kontext "SymboleMath"
        if SymboleMath.exists then
            symbolset.select "TBOnewSymbolSet"
            '/// click button "Edit..." ///'
            Bearbeiten.Click
                Kontext "EditSymbols"
                if EditSymbols.exists then
                    i = SymbolSet.getItemCount
                    'if (iStartingSymbolSetCount +1) <> i then
                        'WarnLog "the new symbolset is not available after restart: " + i + "; should be: " + (iStartingSymbolSetCount +1)
                    'endif
                    try
                        SymbolSet.select "TBOnewSymbolSet"
                    catch
                        warnlog "Symbol set name is not saved on exiting OOo"
                        SymbolSet.select ""
                    endcatch
                    i = Symbol.getItemCount
                    if i < 2 then
                        warnlog "Symbol not available in new Symbolset: intsead of 1: " + i
                    endif
                    SymbolSet.setText "TBOnewSymbolSetdelete"
                    delete.click
                    EditSymbols.ok
                endif
            Kontext "SymboleMath"
        if SymboleMath.exists then
            'cleanup
            for i = 1 to symbolset.getItemCount
                sTemp = symbolset.getItemText(i)
                'if sTemp = "" then
                    'warnlog "Empty name"
                'endif
                printlog ""+i+": '"+sTemp+"'"
            next i
            try
                symbolset.select "TBOnewSymbolSet"
                printlog "found: TBOnewSymbolSet"
            catch
                try
                    symbolset.select ""
                    printlog "found: "
                catch
                    qaerrorlog "not expected"
                endcatch
            endcatch
            '/// click button "Edit..." ///'
            Bearbeiten.Click
                Kontext "EditSymbols"
                if EditSymbols.exists then
                    try
                        SymbolSet.select "TBOnewSymbolSet"
                    catch
                        try
                            symbolset.select ""
                        printlog "found: "
                        catch
                            qaerrorlog "not expected"
                        endcatch
                    endcatch
                    SymbolSet.setText "TBOnewSymbolSetdelete"
                    delete.click
                    EditSymbols.ok
                endif
        endif
            Kontext "SymboleMath"
            j = symbolset.getItemCount
            if j <> 3 then
                warnlog "There is a number != 3 of symbollists listed"
            else
                printlog "Everything is cleaned up."
            endif
            for i = 1 to j
                printlog ""+i+": '"+symbolset.getitemtext(i)+"'"
            next i
            '/// close dialog 'Symbols' ///'
            SymboleMath.close
        endif
    '/// close application ///'
    Call hCloseDocument
endcase

