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
'* owner : helge.delfs@oracle.com
'*
'* short description :
'*
'\***********************************************************************

sub wr_o_4

    Call tToolsOptionsWriterOther
    Call tToolsOptionsWriterAutoCaption
    Call tToolsOptionsWriterGeneral
    Call tToolsOptionsWriterMailMerge

end sub

'------------------------------------------------------------------------------
testcase tToolsOptionsWriterOther
    dim irgendwas(7) as boolean    ' Checkbox states
    dim bRadioBut(3) as boolean    ' RadioButton states
    dim bTempRadioBut(3) as boolean
    dim iMasseinheit as integer
    dim sTabAbstand as string
    dim sTemp as string
    dim iTemp as integer
    dim sTemp2 as string


    ' irgendwas(i)
    '-(1) = NurFuerAktuellesDokument

    '(2) = Feldbefehle
    '(3) = Diagramme
    '-(4) = FelderUndDiagrammeAktuellesDokument

    '(5) = Abstaende
    '(6) = AbstaendeSeitenanfang
    '(7) = TabulatorpositionAusrichten

    ' bRadioBut(i) and bTempRadioBut(i)
    '(1) = Immer
    '(2) = AufNachfrage
    '(3) = Nie

    ' !!! -Feldbefehle- has to be checked, to get -Diagramme- enabeld !!!

    '- !!! -Current Document only- ist bei office beenden zu beachten!!!

    Call hNewDocument
    printlog ("- Tools/Options/Writer: General ")

    printlog (" - save states ")
    ToolsOptions
    Call hToolsOptions ("WRITER","GENERAL")

    irgendwas(2) = Feldbefehle.IsChecked
    irgendwas(3) = Diagramme.IsChecked

    bRadioBut(1) = Immer.IsChecked
    bRadioBut(2) = AufNachfrage.IsChecked
    bRadioBut(3) = Nie.IsChecked

    iMasseinheit = Masseinheit.GetSelIndex
    sTabAbstand = Tabulatorenabstand.GetText

    printlog ("- all states inverting ")

    if irgendwas(2) Then Feldbefehle.UnCheck Else Feldbefehle.Check
    if Feldbefehle.IsChecked  Then if irgendwas(3) Then Diagramme.UnCheck Else Diagramme.Check

    if (Immer.IsChecked = True) Then AufNachfrage.Check Else Immer.Check

    bTempRadioBut(1) = Immer.IsChecked
    bTempRadioBut(2) = AufNachfrage.IsChecked
    bTempRadioBut(3) = Nie.IsChecked

    Tabulatorenabstand.ToMax
    sTemp = Tabulatorenabstand.GetText
    Tabulatorenabstand.More

    if (LiberalMeasurement (sTemp,Tabulatorenabstand.GetText) <> TRUE) Then Warnlog ("BugID: 82740: Tab stops max. isn't max.")
    Tabulatorenabstand.SetText (sTabAbstand)

    if ( Masseinheit.GetItemCount <> iMasseinheit ) Then Masseinheit.Select (Masseinheit.GetItemCount) _
    Else Masseinheit.Select (1)
    iTemp = Masseinheit.GetSelIndex

    sTemp = Tabulatorenabstand.GetText
    Tabulatorenabstand.ToMax
    if ( Tabulatorenabstand.GetText = sTemp ) Then Tabulatorenabstand.ToMin
    sTemp = Tabulatorenabstand.GetText

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("- SO quit - start ")

    Call wOfficeRestart

    printlog ("- checking states ")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GENERAL" )

    if ( irgendwas(2) = Feldbefehle.IsChecked                         ) Then WarnLog "Feldbefehle state changed"
    if Feldbefehle.IsChecked Then if ( irgendwas(3) = Diagramme.IsChecked ) Then WarnLog "Diagramme state changed"
    if ( bTempRadioBut(1) <> Immer.IsChecked       ) Then WarnLog "Immer state changed"
    if ( bTempRadioBut(2) <> AufNachfrage.IsChecked) Then WarnLog "AufNachfrage state changed"
    if ( bTempRadioBut(3) <> Nie.IsChecked         ) Then WarnLog "Nie state changed"

    if ( Masseinheit.GetSelIndex <> iTemp ) Then WarnLog "Masseinheit state changed"
    if (LiberalMeasurement (sTemp,Tabulatorenabstand.GetText) <> TRUE ) Then WarnLog "Tabulatorenabstand state changed - watch BugID: 82744 for Status"

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("- all UnCheck -> o ")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GENERAL" )

    Feldbefehle.UnCheck

    if (AufNachfrage.IsChecked = True) Then Nie.Check Else AufNachfrage.Check

    bTempRadioBut(1) = Immer.IsChecked
    bTempRadioBut(2) = AufNachfrage.IsChecked
    bTempRadioBut(3) = Nie.IsChecked

    Masseinheit.Select (1)
    Tabulatorenabstand.ToMin
    sTemp = Tabulatorenabstand.GetText
    printlog ("More test ///'")
    Tabulatorenabstand.More
    if ( sTemp = Tabulatorenabstand.GetText ) Then WarnLog "Tabulatorenabstand More-Button not working; stemp1: "+sTemp + " =? (2) " +Tabulatorenabstand.GetText
    sTemp = Tabulatorenabstand.GetText

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("- check if all UnChecked ")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GENERAL" )

    if Feldbefehle.IsChecked                         Then WarnLog "Feldbefehle x"

    if ( bTempRadioBut(1) <> Immer.IsChecked       ) Then WarnLog "Immer state changed"
    if ( bTempRadioBut(2) <> AufNachfrage.IsChecked) Then WarnLog "AufNachfrage state changed"
    if ( bTempRadioBut(3) <> Nie.IsChecked         ) Then WarnLog "Nie state changed"

    if ( Masseinheit.GetSelIndex <> 1 ) Then WarnLog "Masseinheit is not item 1"
    sTemp2 = Tabulatorenabstand.GetText
    if (LiberalMeasurement (sTemp, sTemp2) <> TRUE ) Then WarnLog "Tabulatorenabstand is not min. is: " +Tabulatorenabstand.GetText + " should: " +sTemp

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

'---------------------------------------------------------------------------------

    printlog ("- check if tab stops get saved in document ")
    Call hNewDocument

    ' set Default Tab Stop to 2,5cm
    ToolsOptions
    Call hToolsOptions ("WRITER","GENERAL")
    Masseinheit.Select (2) ' cm
'   printlog "before " + Tabulatorenabstand.GetText
    Tabulatorenabstand.SetText ("2" + GetDecimalSeperator(Tabulatorenabstand.GetText) + "50" + mUnit)
'   printlog "after " + Tabulatorenabstand.GetText
'   printlog "seperator: " + GetDecimalSeperator(Tabulatorenabstand.GetText)
'   printlog "generated string: " + "2" + GetDecimalSeperator(Tabulatorenabstand.GetText) + "50cm"
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

'   creating document with tabstops and save it
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "a<Tab>b<Tab>c<Tab>d<Tab>e<Tab>f<Tab>g<Tab>h<Tab>i<Tab>j"
    Call hFileSaveAsWithFilterKill ( gOfficePath + "user\work\tab25cm.odt", "Writer8")
    Call hCloseDocument
    sleep (2)

'   change tab stops
    Call hNewDocument
'   set Default Tab Stop to 2,9cm
    ToolsOptions
    Call hToolsOptions ("WRITER","GENERAL")
    Masseinheit.Select (2) ' cm
'   printlog "before " + Tabulatorenabstand.GetText
    Tabulatorenabstand.SetText ( "2" + GetDecimalSeperator(Tabulatorenabstand.GetText) + "90" + mUnit )
'   printlog "after " + Tabulatorenabstand.GetText
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    sleep (2)

'   load file in this environment
    '///use "output\\writer\\w95\\tab25cm.odt" ///
    hFileOpen( gOfficePath & "user\work\tab25cm.odt")
    sleep (2)
'   check TabStops, have to be the ones this file was saved with
    ToolsOptions
    Call hToolsOptions ("WRITER","GENERAL")
    Masseinheit.Select (2) ' cm
'   printlog "before " + Tabulatorenabstand.GetText
    if Tabulatorenabstand.GetText <> ("2" + GetDecimalSeperator(Tabulatorenabstand.GetText) + "50" + mUnit) Then
        warnlog "the tab stops didn't get saved in the document !!!"
        printlog "should be 2,.50" + mUnit + ", is" + Tabulatorenabstand.GetText
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Call hCloseDocument
    Call hCloseDocument

'---------------------------------------------------------------------------------
    Printlog ("- all Check -> x ")

    ToolsOptions
    Call hToolsOptions ("WRITER","GENERAL")

'   NurFuerAktuellesDokument.Check
    Feldbefehle.Check
    Diagramme.Check
'   FelderUndDiagrammeAktuellesDokument.Check

    if (Nie.IsChecked = True) Then Immer.Check Else Nie.Check

    bTempRadioBut(1) = Immer.IsChecked
    bTempRadioBut(2) = AufNachfrage.IsChecked
    bTempRadioBut(3) = Nie.IsChecked

    Masseinheit.Select (2)
    itemp = Masseinheit.GetSelIndex
    sTemp = "3" + GetDecimalSeperator(Tabulatorenabstand.GetText) + "21" + mUnit
    Tabulatorenabstand.SetText (sTemp)

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("- check if all Checked ")
    ToolsOptions
    Call hToolsOptions ( "WRITER" , "GENERAL" )

'   if not NurFuerAktuellesDokument.IsChecked            Then WarnLog "NurFuerAktuellesDokument o"
    if not Feldbefehle.IsChecked                         Then WarnLog "Feldbefehle o"
    if not Diagramme.IsChecked                           Then WarnLog "Diagramme o"
'   if not FelderUndDiagrammeAktuellesDokument.IsChecked Then WarnLog "FelderUndDiagrammeAktuellesDokument o"

    if ( bTempRadioBut(1) <> Immer.IsChecked       ) Then WarnLog "Immer state changed"
    if ( bTempRadioBut(2) <> AufNachfrage.IsChecked) Then WarnLog "AufNachfrage state changed"
    if ( bTempRadioBut(3) <> Nie.IsChecked         ) Then WarnLog "Nie state changed"

    if ( Masseinheit.GetSelIndex <> iTemp ) Then WarnLog "Masseinheit is not item " + iTemp + " it's: " + Masseinheit.GetSelIndex
    if (LiberalMeasurement (sTemp,Tabulatorenabstand.GetText) <> TRUE ) Then WarnLog "Tabulatorenabstand is not " + sTemp + " it's: " + Tabulatorenabstand.GetText

    printlog ("- restore states ")

'   if ( irgendwas(1) =  TRUE ) Then NurFuerAktuellesDokument.Check            Else NurFuerAktuellesDokument.UnCheck
    if ( irgendwas(2) =  TRUE ) Then Feldbefehle.Check                         Else Feldbefehle.UnCheck
    if ( irgendwas(3) =  TRUE ) Then Diagramme.Check                           Else _
    if Feldbefehle.IsChecked Then Diagramme.UnCheck
'   if Diagramme.IsVisible Then Diagramme.UnCheck
'   if ( irgendwas(4) =  TRUE ) Then FelderUndDiagrammeAktuellesDokument.Check Else FelderUndDiagrammeAktuellesDokument.UnCheck

    if ( bRadioBut(1) = TRUE ) Then Immer.Check
    if ( bRadioBut(2) = TRUE ) Then AufNachfrage.Check
    if ( bRadioBut(3) = TRUE ) Then Nie.Check

    Masseinheit.Select (iMasseinheit) 'sw:ListBox:TP_LAYOUT_O
    Tabulatorenabstand.SetText (sTabAbstand) '     sw:MetricField:TP_LAYOUT_OPT:MF_TAB

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    hFileCloseAll()
    
endcase

'------------------------------------------------------------------------------
sub hGetAllOptionsWriterGeneralCaption
    Dim fHoleTabPage as String
    Dim fHoleTabPage2 as String
    Dim lsHoleTabPage(100) as String
    Dim lsHoleTabPage2(100) as String
    dim i as integer
    dim x as integer
    dim y as integer ' for schleifen stuff

    '/// usually there are 4 entries in the list for Options->Writer->General->Caption->Categorie ///'
    '///+ if not, this routine is called ///'
    '///+ The error only appears, if the wrong listItem is not called yet! ///'

    lsHoleTabPage(0)=0
    lsHoleTabPage2(0)=0

    '/// on error, save before selecting entries to "output\\writer\\opt__" + iSprache + "." + gPlatGroup) ///
    '/// on error, save entries after selecting all to "output\\writer\\opt2__" + iSprache + "." + gPlatGroup) ///
    fHoleTabPage = ConvertPath(gOfficePath + "user\work\opt__" + iSprache + "." + gPlatGroup)
    fHoleTabPage2 = ConvertPath(gOfficePath + "user\work\opt2_" + iSprache + "." + gPlatGroup)

    '------------------------
    Printlog "(1/3) Get all items before selecting smth. "
    ' put them in lsHoleTabPage and write them to 'user\work\opt__...'

    Call hNewDocument

    ToolsOptions
    Call hToolsOptions ("WRITER","AutoCaption")
    Kontext "TabAutoCaption"

    for x=1 to ObjectList.GetItemCount
'       Printlog x
        ObjectList.Select (x)
        ListAppend ( lsHoleTabPage(), ObjectList.GetText + " " + Category.GetItemCount)
        for y=1 to Category.GetItemCount
            Category.Select (y)
            sleep (1)
            ListAppend ( lsHoleTabPage(), "   " + Category.GetSelText )
        next y
    next x

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    Call ListWrite(lsHoleTabPage(),fHoleTabPage, "utf8")
    Printlog "list with all entries written to: "+fHoleTabPage

'   ------------------------

    Printlog "(2/3) Select every ListItem & exit with OK "
    ToolsOptions
    Call hToolsOptions ("WRITER","AutoCaption")
    Kontext "TabAutoCaption"

    for x=1 to ObjectList.GetItemCount
'       Printlog "-" + x
        ObjectList.Select (x)
        i=0
        while ( i < Category.GetItemCount )
            try
                Category.Select (i+1)
            catch
                Category.Select (i)
            endcatch
            i = Category.GetSelIndex
'           Printlog i
            sleep (2)
            Kontext "TabAutoCaption"
        Wend
    next x

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    '------------------------
    Printlog "(3/3) Get all items after selecting all "
    ' put them in lsHoleTabPage2 and write them to 'user\work\opt2_...'

    ToolsOptions
    Call hToolsOptions ("WRITER","AutoCaption")
    Kontext "TabAutoCaption"
    for x=1 to ObjectList.GetItemCount
'       Printlog x
        ObjectList.Select (x)
        ListAppend ( lsHoleTabPage2(), ObjectList.GetText + " " + Category.GetItemCount)
        for y=1 to Category.GetItemCount
            Category.Select (y)
            sleep (1)
            ListAppend ( lsHoleTabPage2(), "   " + Category.GetSelText )
        next y
    next x

    Call ListWrite(lsHoleTabPage2(),fHoleTabPage2, "utf8")
    Printlog "list with all entries after selecting every entry written to: "+fHoleTabPage2

end sub

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterAutoCaption

    '/// look if properties get saved for each list item ///
    '///+ take "Numbering" ListBox to verify this ///

    ' checking Listentry '(7) = StarOffice 6.0 Impress' completely

    dim vObjectList(10) as integer
    dim vObjectListDescription(10) as string
    Dim vCategory(10) as integer
    Dim vCategoryTmp(10) as string
    Dim vNumbering(10) as integer
    Dim vCaptionSeparator(10) as string
    Dim vPosition(10) as integer
    Dim vLevel(10) as integer
    Dim vChapterSeparator(10) as string
    Dim vCharacterStyle(10) as integer
    Dim vApplyBorderAndShadow(10) as boolean 
    Dim NumberOfObjects as integer
    dim i as integer
    dim y as integer ' for schleifen stuff

    '/// !no more Listentries than 9, you have to update the allocation for the var's! ///'
    Call hNewDocument
    printlog "- Tools/Options/Writer: AutoCaption "

    printlog ("- save states ")
    ToolsOptions
    Call hToolsOptions ("WRITER","AutoCaption")
    Kontext "TabAutoCaption"

    NumberOfObjects = ObjectList.GetItemCount
    Select Case gPlatGroup
        Case "w95"
            if (NumberOfObjects <> 9) then
                warnlog "Number of Object List is not correct"
            end if
        Case else
            if (NumberOfObjects <> 8) then
                warnlog "Number of Object List is not correct"
            end if
    end select

    for i=1 to NumberOfObjects
        ObjectList.Select (i)
        vObjectList(i) = ObjectList.IsChecked
        vObjectListDescription(i) = ObjectList.GetText
        vCategory(i) = Category.GetSelIndex
        vNumbering(i) = Numbering.GetSelIndex
        vCaptionSeparator(i) = CaptionSeparator.GetText
        vPosition(i) = Position.GetSelIndex
        vLevel(i) = Level.GetSelIndex
        vChapterSeparator(i) = ChapterSeparator.GetText
        vCharacterStyle(i) = CharacterStyle.GetSelIndex
        vApplyBorderAndShadow(i) = ApplyBorderAndShadow.IsChecked
    next i

    '-- just checking the checkboxes of 'ObjectList'
    printlog ("- uncheck all object - checkboxes ")
    for i=1 to NumberOfObjects
        ObjectList.Select (i)
        ObjectList.UnCheck
    next i
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    
    printlog ("- verify if all unchecked and check all ")
    ToolsOptions
    Call hToolsOptions ("WRITER","AutoCaption")
    Kontext "TabAutoCaption"
    for i=1 to NumberOfObjects
        ObjectList.Select (i)
        if ObjectList.IsChecked Then
            WarnLog ObjectList.GetText & " is checked though it had been unchecked before"
        else
            ObjectList.Check
        end if
    next i
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("- verify all checkboxes are checked ")
    ToolsOptions
    Call hToolsOptions ("WRITER","AutoCaption")
    Kontext "TabAutoCaption"
    for i=1 to NumberOfObjects
        ObjectList.Select (i)
        if ( ObjectList.IsChecked = FALSE ) Then
            WarnLog ObjectList.GetText & " is unchecked though it had been checked before"
            ObjectList.Check
        end if
    next i

    printlog ("- change states ")
    '/// look if properties get saved for each list item
    for i=1 to NumberOfObjects
        ObjectList.Select (i)
        printlog " - " & ObjectList.GetText
        if ObjectList.IsChecked Then
            if i <= 7 then Numbering.Select i
            CaptionSeparator.Settext "Caption Separator " & cstr(i)
            ChapterSeparator.Settext cstr(i)
            Level.Select i
            CharacterStyle.Select i
            Select Case i
                case 1:
                    Category.Select 2 
                    Position.Select 1
                case 2:
                    Category.Select 3
                    Position.Select 2
                case 3:
                    Category.Select 4
                    ApplyBorderAndShadow.Check
                case 4:
                    Category.Select 5
                case 5:
                    Category.Select 2
                case 6:
                    Category.Select 3
                case 7:
                    Category.Select 4
                case 8:
                    Category.Select 5
                    if Numbering.GetItemCount > 7 then
                        Numbering.Select Numbering.GetItemCount
                    else
                        Numbering.Select 4
                    end if
                case 9:
                    Category.Select 2
                    if Numbering.GetItemCount > 7 then
                        Numbering.Select Numbering.GetItemCount
                    else
                        Numbering.Select 5
                    end if

                end select
                vCategoryTmp(i) = Category.GetSelText
        else
            Warnlog ObjectList.GetText & " is not checked: Verify test script"
        end if
    next i
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("- SO quit - start")

    Call wOfficeRestart

    printlog ("- checking states ")
    ToolsOptions
    Call hToolsOptions ("WRITER","AutoCaption")
    Kontext "TabAutoCaption"
    for i=1 to NumberOfObjects
        ObjectList.Select (i)
        printlog " - " & ObjectList.GetText
		wait 500
        if ObjectList.IsChecked Then
            if i <= 7 then
                if Numbering.GetSelIndex <> i then
                    Warnlog "-    'Numbering' changed after restart"
                else
                    printlog "-   'Numbering' => ok"
                end if
            end if
            if CaptionSeparator.Gettext <> "Caption Separator " & cstr(i) then
                Warnlog "-    'Caption Separator' changed after restart"
            else
                printlog "-    'Caption Separator' => ok"
            end if
            if ChapterSeparator.Gettext <>  cstr(i) then
                Warnlog "-    'Chapter Separator' changed after restart"
            else
                printlog "-    'Chapter Separator' => ok"
            end if
            if Level.GetSelIndex <> i then 
                Warnlog "-    'Level' changed after restart"
            else
                Printlog "-    'Level' => ok"
            end if
            if CharacterStyle.GetSelIndex <> i then
                Warnlog "-    'Character Style' changed after restart"
            else
                printlog "-    'Character Style' => ok"
            end if

            if Category.GetSelText <> vCategoryTmp(i) then
                Warnlog "-    'Category' changed after restart. Not " & vCategoryTmp(i) & " but " & Category.GetSelText
            else
                printlog "-    'Category' => ok"
            end if

            Select Case i
                case 1:
                    if Position.GetSelIndex <> 1 then
                        Warnlog "-    'Position' changed after restart"
                    else
                        printlog "-    'Position' => ok"
                    end if

                case 2:
                    if Position.GetSelIndex <> 2 then
                        Warnlog "-    'Position' changed after restart"
                    else
                        printlog "-    'Position' => ok"
                    end if

                case 3:
                    if Position.IsEnabled = true then
                        Warnlog "-    'Position' should be disabled"
                    else
                        printlog "-    'Position' disabled => ok"
                    end if
                    
                    if ApplyBorderAndShadow.IsChecked = false then
                        Warnlog "-    'Apply Border and Shadow' changed after restart"
                    else
                        printlog "-    'Apply Border and Shadow => ok"
                    end if

                case 4:
                    if Position.IsEnabled = true then
                        Warnlog "-    'Position' should be disabled"
                    else
                        printlog "-    'Position' disabled => ok"
                    end if

                case 5:
                    if Position.IsEnabled = true then
                        Warnlog "-    'Position' should be disabled"
                    else
                        printlog "-    'Position' disabled => ok"
                    end if

                case 6:
                    if Position.IsEnabled = true then
                        Warnlog "-    'Position' should be disabled"
                    else
                        printlog "-    'Position' disabled => ok"
                    end if

                case 7:
                    if Position.IsEnabled = true then
                        Warnlog "-    'Position' should be disabled"
                    else
                        printlog "-    'Position' disabled => ok"
                    end if

                case 8:
                    if Numbering.GetItemCount > 7 then
                        if Numbering.GetSelIndex <> Numbering.GetItemCount then
                            Warnlog "-    'Numbering' changed after restart"
                        else
                            printlog "-    'Numbering' => ok"
                        end if
                    else
                        if Numbering.GetSelIndex <> 4 then
                            Warnlog "-    'Numbering' changed after restart"
                        else
                            printlog "-    'Numbering' => ok"
                        end if
                    end if

                    if Position.IsEnabled = true then
                        Warnlog "-    'Position' should be disabled"
                    else
                        printlog "-    'Position' disabled => ok"
                    end if

                case 9:
                    if Numbering.GetItemCount > 7 then
                        if Numbering.GetSelIndex <> Numbering.GetItemCount then
                            Warnlog "-    Numbering changed after restart"
                        else
                            printlog "-    Numbering => ok"
                        end if
                    else
                        if Numbering.GetSelIndex <> 5 then
                            Warnlog "-    'Numbering changed after restart"
                        else
                            printlog "-    'Numbering' => ok"
                        end if
                    end if

                    if Position.IsEnabled = true then
                        Warnlog "-    'Position' should be disabled"
                    else
                        printlog "-    'Position' disabled => ok"
                    end if

            end select
        else
            Warnlog "-   " & ObjectList.GetText & " is not checked"
        end if
    next i

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("- restore states ")

    ToolsOptions
    Call hToolsOptions ("WRITER","AutoCaption")
    Kontext "TabAutoCaption"

    for i=1 to NumberOfObjects
        ObjectList.Select (i)
        if ObjectList.IsChecked then
            if vCategory(i) > Category.GetItemCount then
                Category.Select Category.GetItemCount
            else
                Category.Select vCategory(i)
            end if
            Numbering.Select vNumbering(i)
            CaptionSeparator.SetText vCaptionSeparator(i)
            if Position.IsEnabled then Position.Select vPosition(i)
            Level.Select vLevel(i)
            ChapterSeparator.Settext vChapterSeparator(i)
            CharacterStyle.Select vCharacterStyle(i)
            if ApplyBorderAndShadow.IsEnabled then
                Select Case vApplyBorderAndShadow(i)
                    Case true
                        ApplyBorderAndShadow.Check
                    case false
                        ApplyBorderAndShadow.Uncheck
                end select
            end if
        end if
        Select case vObjectList(i)
            Case true
                ObjectList.Check
            case false
                ObjectList.Uncheck
        end select
    next i


    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    hFileCloseAll()
    
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterGeneral
    Dim i as integer
    Dim ist_aktualisiert as boolean
    Dim CompareThis as string

    ' State is saved with document. Thats why I use 3 docs
    Call hNewDocument
    printlog "- Extras / Optionen / Sonstiges / Verkn?pfungen aktualisieren "
    ToolsOptions
    Call hToolsOptions("WRITER","GENERAL")
    printlog "- Update links when loading -> always "
    Immer.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    FileOpen
    Kontext "OeffnenDlg"
    Dateiname.SetText ConvertPath(gtesttoolpath + "writer\optional\input\masterdoc\update1.sxg")
    Oeffnen.Click
    Kontext "Active"
    if active.exists then
        if Active.GetRT = 304 then
            Warnlog Active.Gettext
            Warnlog "Update all links should not appear!"
            wait 500
            try
                Active.yes
            catch
                Active.Ok
            endcatch
        end if
    end if
    Sleep 2
    Call hCloseDocument
    Sleep 2

    ToolsOptions
    Call hToolsOptions("WRITER","GENERAL")
    if not Immer.IsChecked then Warnlog "Option 'always' has not been changed"
    printlog "- Update links when loading -> On request "
    AufNachfrage.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    FileOpen
    Kontext "OeffnenDlg"
    Dateiname.SetText ConvertPath(gtesttoolpath + "writer\optional\input\masterdoc\update2.sxg")
    Oeffnen.Click
    Sleep 2
    Kontext "Active"
    if active.exists then
        if Active.GetRT = 304 then
            Active.yes
        else
            Warnlog "Update all links should appear!"
        end if
    end if
    Sleep 2
    Call hCloseDocument
    Sleep 2

    ToolsOptions
    Call hToolsOptions("WRITER","GENERAL")
    if not AufNachfrage.IsChecked then Warnlog "Option 'On request' has been changed"
    printlog "- Update links when loading -> Never "
    Nie.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    FileOpen
    Kontext "OeffnenDlg"
    Dateiname.SetText ConvertPath(gtesttoolpath + "writer\optional\input\masterdoc\update3.sxg")
    Oeffnen.Click
    Kontext "Active"
    if active.exists then
        if Active.GetRT = 304 then
            Warnlog "Update all links should not appear!"
            Active.yes
         end if
    end if
    Sleep 3
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Up>",10
    DocumentWriter.TypeKeys "<Mod1 Home>", 4
    for i=1 to 4
        DocumentWriter.TypeKeys "<Shift End>"
        try
            EditCopy
            if i <> 4 and GetClipboardtext <> "Dies ist der Text" then ist_aktualisiert = true
        catch
            ist_aktualisiert=false
        endcatch
        DocumentWriter.TypeKeys "<Mod1 End>"
        wait 500
        DocumentWriter.TypeKeys "<Down><Home>"
    next i
    if ist_aktualisiert=true then Warnlog " Document has been updated !"
    Sleep 2
    Call hCloseDocument
    Sleep 2

    ToolsOptions
    Call hToolsOptions("WRITER","GENERAL")
    if Nie.IsChecked = false then Warnlog "Option 'Never' is not checked"
    AufNachfrage.Check
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    hFileCloseAll()
    
endcase

'------------------------------------------------------------------------------

testcase tToolsOptionsWriterMailMerge

    Call hNewDocument
    printlog ("- Tools/Options/Writer: Mail Merge ")
    Kontext "Navigator"
    if Navigator.Exists then ViewNavigator
    ToolsOptions
    Call hToolsOptions ("WRITER","MailMergeEMail")

    '/// Fill out all fields
    printlog " - fill out Mail Settings"
    Kontext "TabMailMergeEmail"
    YourName.Settext "Fredrik Haegg"
    EmailAddress.Settext "helge.delfs@oracle.com"
    SendReplies.Check
    ReplyAddress.Settext "fredrikh@openoffice.org"
    ServerName.Settext "MySMTP-Server"
    UseSecure.Check
    
    '/// Change details for Server Authentication
    printlog " - fill out Authentication Settings"
    ServerAuthentication.Click
    wait 500
    Kontext "ServerAuthentication"
    SMTPauthentication.Check
    
    IncomingMailServer.Check
    Server.Settext "MySMTP-Server"
    Port.Settext "111"
    IMAP.Check
    User.Settext "MyAuthenticationUserName"
    Password.Settext "MyAuthenticationPassword"
    ServerAuthentication.Ok
    
    '/// Close 'Server Authentication dialog, reopen and check values set
    printlog " - check settings"
    Kontext "TabMailMergeEmail"
    ServerAuthentication.Click
    wait 500
    Kontext "ServerAuthentication"
    if Not SMTPauthentication.IsChecked then Warnlog "SMTP Authentication is not checked anymore"
    if Not IncomingMailServer.IsChecked then Warnlog "Incoming Mail Server has been changed"
    if Server.Gettext <> "MySMTP-Server" then Warnlog "Servername has been changed"
    if Port.Gettext <> "111" then Warnlog "Server Port has been changed"
    if Not IMAP.IsChecked then Warnlog "IMAP-state has been changed"
    if User.Gettext <> "MyAuthenticationUserName" then Warnlog "Authentication username has been changed"
    if Password.Gettext <> "MyAuthenticationPassword" then Warnlog "Authentication password has been changed"
    
    '/// Change settings for SMTP outgoing
    SMTPOutgoing.Check
    OutgoingUsername.Settext "MyOutgoingUsername"
    OutgoingPassword.Settext "MyOutgoingPassword"
    ServerAuthentication.Ok
    
    '/// Close options dialog and restart the office
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok
    printlog " - restart office"
    Call wOfficeRestart

    '/// Reopen Tools/Options and check settings
    printlog " - check settings"
    ToolsOptions
    Call hToolsOptions ("WRITER","MailMergeEMail")

    Kontext "TabMailMergeEmail"
    if YourName.Gettext <> "Fredrik Haegg" then Warnlog "'Your Name' has been changed after a restart"
    if EmailAddress.Gettext <> "helge.delfs@oracle.com" then Warnlog "'Email Address' has been changed after a restart"
    if Not SendReplies.IsChecked then Warnlog "'Send replies' has been changed after a restart"
    if ReplyAddress.Gettext <> "fredrikh@openoffice.org" then Warnlog "'Reply adresses' has been changed after a restart"
    if ServerName.Gettext <> "MySMTP-Server" then Warnlog "'Server name' has been changed after a restart"
    if UseSecure.IsChecked = false then Warnlog "'Secure' has been changed after a restart"

    ServerAuthentication.Click
    wait 500
    Kontext "ServerAuthentication"
    if Not SMTPOutgoing.IsChecked then Warnlog "'SMTP Server' has been changed after a restart"
    if OutgoingUsername.Gettext <> "MyOutgoingUsername" then Warnlog "'Username' has been changed after a restart"
    if OutgoingPassword.Gettext <> "MyOutgoingPassword" then Warnlog "'Password' has been changed after a restart"
    
    ServerAuthentication.Ok

    '/// Check 'Test Settings' button
    Kontext "TabMailMergeEmail"
    TestSettings.Click
    
    Kontext "TestAccountSettings"
    Do
        if TaskStatusListBox.GetItemCount > 0 then
            TestAccountSettings.Cancel
            exit do
        end if
        wait 500
    Loop
    if TestAccountSettings.Exists then TestAccountSettings.Cancel
    
    '/// Reset all states
    Kontext "TabMailMergeEmail"
    ServerAuthentication.Click
    
    Kontext "ServerAuthentication"
    IncomingMailServer.Check
    Server.Settext ""
    Port.Settext "110"
    User.Settext ""
    Password.Settext ""

    SMTPOutgoing.Check
    OutgoingUsername.Settext ""
    OutgoingPassword.Settext ""

    SMTPauthentication.UnCheck
    ServerAuthentication.Ok

    Kontext "TabMailMergeEmail"
    YourName.Settext ""
    EmailAddress.Settext = ""
    ReplyAddress.Settext ""
    ServerName.Settext ""
    SendReplies.UnCheck
    UseSecure.Uncheck

    '/// Close options dialog and restart the office
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.Ok

    hFileCloseAll()
    
endcase
'------------------------------------------------------------------------------
