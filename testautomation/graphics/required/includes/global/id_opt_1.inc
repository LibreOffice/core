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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'\******************************************************************************

testcase tToolsOptionsDrawGeneral (sApplication as string)
    dim irgendwas(9) as boolean
    dim iMasseinheit as integer
    dim sTabAbstand as string
    dim iZeichnungsmasstab as integer
    dim sTemp as string
    dim iTemp as integer
    dim sTemp2 as string
    dim iTemp2 as integer
    dim sMeasUnit as string

    hNewDocument

    printlog "save states"
    ToolsOptions
    hToolsOptions (sApplication,"General")

    irgendwas(1) = Schnellbearbeitung.IsChecked
    irgendwas(2) = TextbereichSelektierbar.IsChecked

    irgendwas(3) = CacheFuerHintergrund.IsChecked
    irgendwas(4) = KopieBeimVerschieben.IsChecked
    irgendwas(5) = ObjekteImmerVerschiebbar.IsChecked

    if (gApplication = "DRAW") then
        irgendwas(6) = SetzenAufKreis.IsChecked   ' DRAW
        iZeichnungsmasstab = Zeichnungsmasstab.GetSelIndex ' DRAW
    else ' IMPRESS
        irgendwas(7) = MitAutopilotStarten.IsChecked
        irgendwas(8) = MitAktuellerSeite.IsChecked
        irgendwas(9) = AlleAbsatzabstaendeAddieren.IsChecked
    endif

    iMasseinheit = Masseinheit.GetSelIndex
    sTabAbstand = Tabulatorabstand.GetText

    printlog "all states inverting"

    if irgendwas(1) Then Schnellbearbeitung.UnCheck Else       Schnellbearbeitung.Check
    if irgendwas(2) Then TextbereichSelektierbar.UnCheck Else  TextbereichSelektierbar.Check

    if irgendwas(3) Then CacheFuerHintergrund.UnCheck     Else     CacheFuerHintergrund.Check
    if irgendwas(4) Then KopieBeimVerschieben.UnCheck     Else     KopieBeimVerschieben.Check
    if irgendwas(5) Then ObjekteImmerVerschiebbar.UnCheck Else ObjekteImmerVerschiebbar.Check

    Tabulatorabstand.ToMax
    sTemp = Tabulatorabstand.GetText
    Tabulatorabstand.More

    if (LiberalMeasurement (sTemp,Tabulatorabstand.GetText) <> TRUE) Then Warnlog "Something wrong, please check!"
        Tabulatorabstand.SetText (sTabAbstand)

        if (gApplication = "DRAW") then
            printlog "Index Zeichnungsmasstab= " & iZeichnungsmasstab
            printlog "Zeichnungsmasstab Item Count= " & (Zeichnungsmasstab.GetItemCount)
            If ( Zeichnungsmasstab.GetItemCount <> iZeichnungsmasstab ) Then
                Zeichnungsmasstab.Select (Zeichnungsmasstab.GetItemCount)
            Else 
                Zeichnungsmasstab.Select (1)
            endif
            iTemp2 = Zeichnungsmasstab.GetSelIndex
            printlog "iTemp2= " & iTemp2
            if irgendwas(6) Then SetzenAufKreis.UnCheck Else           SetzenAufKreis.Check
        else ' IMPRESS
            if irgendwas(7) Then MitAutopilotStarten.UnCheck         Else  MitAutopilotStarten.Check
            if irgendwas(8) Then MitAktuellerSeite.UnCheck           Else  MitAktuellerSeite.Check
            if irgendwas(9) Then AlleAbsatzabstaendeAddieren.UnCheck Else  AlleAbsatzabstaendeAddieren.Check
        endif

        printlog "<Meas.Unit> interacts with <Tap stops> and vice versa"

        If ( Masseinheit.GetItemCount <> iMasseinheit ) Then Masseinheit.Select (Masseinheit.GetItemCount) _
        Else Masseinheit.Select (1)
        iTemp = Masseinheit.GetSelIndex

        sTemp = Tabulatorabstand.GetText
        Tabulatorabstand.ToMax
        If ( Tabulatorabstand.GetText = sTemp ) Then Tabulatorabstand.ToMin
        sTemp = Tabulatorabstand.GetText

        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK

        printlog "-------> Office quit - start"

        ExitRestartTheOffice
        printlog "opening new doc to get the Options"
        Call hNewDocument
        printlog "checking states"
        ToolsOptions
        hToolsOptions (sApplication,"General")

        If ( irgendwas(1) = Schnellbearbeitung.IsChecked         ) Then WarnLog "Allow Quick Editing state changed"
        If ( irgendwas(2) = TextbereichSelektierbar.IsChecked    ) Then WarnLog "Only Text Area Selectable state changed"

        If ( irgendwas(3) = CacheFuerHintergrund.IsChecked       ) Then WarnLog "Use Background Cache state changed"
        If ( irgendwas(4) = KopieBeimVerschieben.IsChecked       ) Then WarnLog "Copy When Moving state changed"
        If ( irgendwas(5) = ObjekteImmerVerschiebbar.IsChecked   ) Then WarnLog "Objects Always Movable state changed"

        if (gApplication = "DRAW") then
        printlog "Index Zeichnungsmasstab= " & (Zeichnungsmasstab.GetSelIndex)
        printlog "iTemp2= " & iTemp2
         If ( Zeichnungsmasstab.GetSelIndex <> iTemp2 ) Then WarnLog "Drawing Scale state changed BUG #97391 is:"+Zeichnungsmasstab.GetSelIndex+"; should: "+iTemp2
         If ( irgendwas(6) = SetzenAufKreis.IsChecked             ) Then WarnLog "'Do not disort objects in curve' state changed"
        else ' IMPRESS
         if ( irgendwas(7) = MitAutopilotStarten.IsChecked          ) Then WarnLog "'Start with AutoPilot' state changed."
        '
         if ( irgendwas(8) = MitAktuellerSeite.IsChecked            ) Then WarnLog "'Always with current page' changed"
         if ( irgendwas(9) = AlleAbsatzabstaendeAddieren.IsChecked  ) Then WarnLog "'Add spacing between paragraphs in the current document' changed"
        endif

        If ( Masseinheit.GetSelIndex <> iTemp ) Then WarnLog "Unit of Measurement state changed"
        If (LiberalMeasurement (sTemp,Tabulatorabstand.GetText) <> TRUE ) Then WarnLog "Tab stops state changed - watch BugID: 82744 for Status"

        printlog "all UnCheck -> o"

        Schnellbearbeitung.UnCheck
        TextbereichSelektierbar.UnCheck

        CacheFuerHintergrund.UnCheck
        KopieBeimVerschieben.UnCheck
        ObjekteImmerVerschiebbar.UnCheck

        if (gApplication = "DRAW") then
            Zeichnungsmasstab.Select (1)
            SetzenAufKreis.UnCheck
        else ' IMPRESS
            MitAutopilotStarten.UnCheck
            MitAktuellerSeite.UnCheck
            AlleAbsatzabstaendeAddieren.UnCheck
        endif

        Masseinheit.Select (1)
        Tabulatorabstand.ToMin
        sTemp = Tabulatorabstand.GetText
        printlog " More test "
        Tabulatorabstand.More
        if ( sTemp = Tabulatorabstand.GetText ) Then WarnLog "Tab Stops More-Button not working; stemp1: "+sTemp + " =? (2) " +Tabulatorabstand.GetText
            sTemp = Tabulatorabstand.GetText

        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK

        printlog "check if all UnChecked"
        ToolsOptions
        hToolsOptions (sApplication,"General")

        If Schnellbearbeitung.IsChecked        Then WarnLog "Allow Quick Editing x"
        If TextbereichSelektierbar.IsChecked   Then WarnLog "Only Text Area Selectable x"

        If CacheFuerHintergrund.IsChecked      Then WarnLog "Use Background Cache x"
        If KopieBeimVerschieben.IsChecked      Then WarnLog "Copy When Moving x"
        If ObjekteImmerVerschiebbar.IsChecked  Then WarnLog "Objects Always Movable x"

        if (gApplication = "DRAW") then
            If ( Zeichnungsmasstab.GetSelIndex <> 1 ) Then WarnLog "'Drawing Scale' is not item 1"
            If SetzenAufKreis.IsChecked            Then WarnLog "'Do not disort objects in curve' x"
        else ' IMPRESS
            if MitAutopilotStarten.IsChecked         Then WarnLog "Start with AutoPilot x"
            if MitAktuellerSeite.IsChecked           Then WarnLog "'Always with current page' x"
            if AlleAbsatzabstaendeAddieren.IsChecked Then WarnLog "'Add spacing between paragraphs in the current document' x"
        endif

        If ( Masseinheit.GetSelIndex <> 1 ) Then WarnLog "Unit of Measurement is not item 1"
        If (LiberalMeasurement (sTemp,Tabulatorabstand.GetText) <> TRUE ) Then WarnLog "Tab stops is not min. is: " +Tabulatorabstand.GetText + " should: " +sTemp

   Kontext "ExtrasOptionenDlg"
   ExtrasOptionenDlg.OK
'---------------------------------------------------------------------------------
    printlog "Tabstops have to be saved in dokument!"

    printlog "check if tab stops get saved in document"
    hNewDocument

    printlog "set Default Tab Stop to 2,5cm"
    ToolsOptions
    hToolsOptions (sApplication,"General")
    Masseinheit.Select (2) ' cm
    sMeasUnit = GetMeasUnit(Tabulatorabstand.GetText)
    '         printlog "before " + Tabulatorabstand.GetText
    Tabulatorabstand.SetText ("2" + GetDecimalSeperator(Tabulatorabstand.GetText) + "50"+ sMeasUnit)
    '         printlog "after " + Tabulatorabstand.GetText
    '         printlog "seperator: " + GetDecimalSeperator(Tabulatorabstand.GetText)
    '         printlog "generated string: " + "2" + GetDecimalSeperator(Tabulatorabstand.GetText) + "50cm"
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "creating document with tabstops and save it"
    if (gApplication = "DRAW") then
        kontext "Toolbar"
        Textfield.Click
        kontext "DocumentDraw"
        hMouseClick DocumentDraw,50,50
    else
        kontext "Toolbar"
        Textfield.Click
        kontext "DocumentImpress"
        hMouseClick DocumentImpress,20,50
    endif
    hTypeKeys "a<Tab>b<Tab>c<Tab>d<Tab>e<Tab>f<Tab>g<Tab>h<Tab>i<Tab>j"
    if ( gApplication = "IMPRESS" ) then 
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if
    hFileSaveAsKill (gOfficePath + "user\work\tab25cm." & ExtensionString )
    hCloseDocument
    sleep (2)
    printlog "change tab stops"
    hNewDocument
    printlog "Changed the Default Tab Stop to 2,9cm"
    ToolsOptions
    hToolsOptions (sApplication,"General")
    Masseinheit.Select (2) ' cm
    '         printlog "before " + Tabulatorabstand.GetText
    Tabulatorabstand.SetText ("2" + GetDecimalSeperator(Tabulatorabstand.GetText) + "90"+sMeasUnit )
    '         printlog "after " + Tabulatorabstand.GetText
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    sleep (2)



    printlog "load the saved file into this environment"
    printlog "use output\\writer\\w95\\tab25cm.odt"
    hFileOpen (gOfficePath + "user\work\tab25cm." + ExtensionString)

    sleep (10)
    printlog "check TabStops, have to be the ones this file was saved with"
    ToolsOptions
    hToolsOptions (sApplication,"General")
    Masseinheit.Select (2) ' cm
'         printlog "before " + Tabulatorabstand.GetText
    if Tabulatorabstand.GetText <> ("2" + GetDecimalSeperator(Tabulatorabstand.GetText) + "50"+sMeasUnit ) Then
        warnlog "The tab stops didn't get saved in the document !!! - i33211"
        printlog "should be 2,50"+sMeasUnit +", is " + Tabulatorabstand.GetText
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    hCloseDocument
    hCloseDocument

'---------------------------------------------------------------------------------
    Printlog ("all Check -> x")

    printlog "Click in the open impress document to get the focus back to the document"
    printlog "Otherwise it sometimes happend that the Tools/Option dialog doesn't display the correct applications"
    hNewDocument
    if ( sApplication = "IMPRESS") then
        Kontext "DocumentImpress"
        DocumentImpress.MouseDown(50,50)
        DocumentImpress.MouseUp(50,50)
    else
        Kontext "DocumentDraw"
        DocumentDraw.MouseDown(50,50)
        DocumentDraw.MouseUp(50,50)
    endif


    ToolsOptions
    hToolsOptions (sApplication,"General")
    kontext "TabSonstigesDraw"
    Schnellbearbeitung.Check
    TextbereichSelektierbar.Check

    CacheFuerHintergrund.Check
    KopieBeimVerschieben.Check
    ObjekteImmerVerschiebbar.Check

    if (gApplication = "DRAW") then
         Zeichnungsmasstab.Select (2)
         itemp2 = Zeichnungsmasstab.GetSelIndex
         SetzenAufKreis.Check
    else ' IMPRESS
        '         MitAutopilotStarten.Check
         MitAktuellerSeite.Check
         AlleAbsatzabstaendeAddieren.Check
    endif

    Masseinheit.Select (2)
    itemp = Masseinheit.GetSelIndex
    sTemp = "3" + GetDecimalSeperator(Tabulatorabstand.GetText) + "21"+sMeasUnit
    Tabulatorabstand.SetText (sTemp)

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog ("check if all Checked")
    ToolsOptions
    hToolsOptions (sApplication,"General")

    If True <> Schnellbearbeitung.IsChecked        Then WarnLog "Allow Quick Editing o"
    If True <> TextbereichSelektierbar.IsChecked   Then WarnLog "Only Text Area Selectable o"

    If True <> CacheFuerHintergrund.IsChecked      Then WarnLog "Use Background Cache o i26631"
    If True <> KopieBeimVerschieben.IsChecked      Then WarnLog "Copy When Moving o"
    If True <> ObjekteImmerVerschiebbar.IsChecked  Then WarnLog "Objects Always Movable o"

    if (gApplication = "DRAW") then
    If ( Zeichnungsmasstab.GetSelIndex <> iTemp2 ) Then WarnLog "Drawing Scale is not item " + iTemp2 + " it's: " + Zeichnungsmasstab.GetSelIndex
    If True <> SetzenAufKreis.IsChecked            Then WarnLog "'Do not disort objects in curve' o"
    else ' IMPRESS
    '         If True <> MitAutopilotStarten.IsChecked         Then WarnLog "Start with AutoPilot o"
    If True <> MitAktuellerSeite.IsChecked           Then WarnLog "'Always with current page' o"
    If True <> AlleAbsatzabstaendeAddieren.IsChecked Then WarnLog "'Add spacing between paragraphs in the current document' o"
    endif

    If ( Masseinheit.GetSelIndex <> iTemp ) Then WarnLog "Masseinheit is not item " + iTemp + " it's: " + Masseinheit.GetSelIndex
    If (LiberalMeasurement (sTemp,Tabulatorabstand.GetText) <> TRUE ) Then WarnLog "Tab stops is not " + sTemp + " it's: " + Tabulatorabstand.GetText

    printlog "restore states"

    If ( irgendwas(1) = TRUE ) Then  Schnellbearbeitung.Check Else       Schnellbearbeitung.UnCheck
    If ( irgendwas(2) = TRUE ) Then  TextbereichSelektierbar.Check Else  TextbereichSelektierbar.UnCheck

    If ( irgendwas(3) = TRUE ) Then  CacheFuerHintergrund.Check Else     CacheFuerHintergrund.UnCheck
    If ( irgendwas(4) = TRUE ) Then  KopieBeimVerschieben.Check Else     KopieBeimVerschieben.UnCheck
    If ( irgendwas(5) = TRUE ) Then  ObjekteImmerVerschiebbar.Check Else ObjekteImmerVerschiebbar.UnCheck

    if (gApplication = "DRAW") then
        Zeichnungsmasstab.Select (iZeichnungsmasstab)
        If ( irgendwas(6) = TRUE ) Then  SetzenAufKreis.Check Else           SetzenAufKreis.UnCheck
    else ' IMPRESS
        if ( irgendwas(7) = TRUE ) Then MitAutopilotStarten.Check         Else  MitAutopilotStarten.UnCheck
        if ( irgendwas(8) = TRUE ) Then MitAktuellerSeite.Check           Else  MitAktuellerSeite.UnCheck
        if ( irgendwas(9) = TRUE ) Then AlleAbsatzabstaendeAddieren.Check Else  AlleAbsatzabstaendeAddieren.UnCheck
    endif

    Masseinheit.Select (iMasseinheit) 'sw:ListBox:TP_LAYOUT_O
    Tabulatorabstand.SetText (sTabAbstand) '     sw:MetricField:TP_LAYOUT_OPT:MF_TAB

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    hCloseDocument
endcase 'tToolsOptionsDrawGeneral

'------------------------------------------------------------------------------
testcase tToolsOptionsDrawView (sApplication as string)
    dim irgendwas(8) as boolean

    hNewDocument

    printlog "save states"
    ToolsOptions
    hToolsOptions (sApplication,"View")

    irgendwas(1) = LinealeSichtbar.IsChecked
    irgendwas(2) = HilfslinienBeimBewegen.IsChecked
    irgendwas(3) = KontrolpunkteImBezierEditor.IsChecked
    irgendwas(4) = KonturFuerIndividuelleObjekte.IsChecked

    printlog "all states inverting"

    if irgendwas(1) Then LinealeSichtbar.UnCheck               Else LinealeSichtbar.Check
    if irgendwas(2) Then HilfslinienBeimBewegen.UnCheck        Else HilfslinienBeimBewegen.Check
    if irgendwas(3) Then KontrolpunkteImBezierEditor.UnCheck   Else KontrolpunkteImBezierEditor.Check
    if irgendwas(4) Then KonturFuerIndividuelleObjekte.UnCheck Else KonturFuerIndividuelleObjekte.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "Office quit - start"

    ExitRestartTheOffice
    printlog "opening new doc to get the Options"
    Call hNewDocument
    printlog "checking states"
    ToolsOptions
    hToolsOptions (sApplication,"View")

    If ( irgendwas(1) = LinealeSichtbar.IsChecked               ) Then WarnLog "LinealeSichtbar state changed"
    If ( irgendwas(2) = HilfslinienBeimBewegen.IsChecked        ) Then WarnLog "HilfslinienBeimBewegen state changed"
    If ( irgendwas(3) = KontrolpunkteImBezierEditor.IsChecked   ) Then WarnLog "KontrolpunkteImBezierEditor state changed"
    If ( irgendwas(4) = KonturFuerIndividuelleObjekte.IsChecked ) Then WarnLog "KonturFuerIndividuelleObjekte state changed"

    printlog "all UnCheck -> o"

    LinealeSichtbar.UnCheck
    HilfslinienBeimBewegen.UnCheck
    KontrolpunkteImBezierEditor.UnCheck
    KonturFuerIndividuelleObjekte.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "check if all UnChecked"
    ToolsOptions
    hToolsOptions (sApplication,"View")

    If LinealeSichtbar.IsChecked                Then WarnLog "Rulers Visible x"
    If HilfslinienBeimBewegen.IsChecked         Then WarnLog "Guides When Moving x"
    If KontrolpunkteImBezierEditor.IsChecked    Then WarnLog "'All Control Points In B�zier Editor' x"
    If KonturFuerIndividuelleObjekte.IsChecked  Then WarnLog "'Contour of Each Individual Object' x"

    Printlog "all Check -> x"

    LinealeSichtbar.Check
    HilfslinienBeimBewegen.Check
    KontrolpunkteImBezierEditor.Check
    KonturFuerIndividuelleObjekte.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "check if all Checked"
    ToolsOptions
    hToolsOptions (sApplication,"View")

    If True <> LinealeSichtbar.IsChecked               Then WarnLog "Rulers Visible o"
    If True <> HilfslinienBeimBewegen.IsChecked        Then WarnLog "Guides When Moving o"
    If True <> KontrolpunkteImBezierEditor.IsChecked   Then WarnLog "'All Control Points In B�zier Editor' o"
    If True <> KonturFuerIndividuelleObjekte.IsChecked Then WarnLog "'Contour of Each Individual Object' o"

    printlog "restore states"

    If ( irgendwas(1) = TRUE ) Then  LinealeSichtbar.Check               Else LinealeSichtbar.UnCheck
    If ( irgendwas(2) = TRUE ) Then  HilfslinienBeimBewegen.Check        Else HilfslinienBeimBewegen.UnCheck
    If ( irgendwas(3) = TRUE ) Then  KontrolpunkteImBezierEditor.Check   Else KontrolpunkteImBezierEditor.UnCheck
    If ( irgendwas(4) = TRUE ) Then  KonturFuerIndividuelleObjekte.Check Else KonturFuerIndividuelleObjekte.UnCheck

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    hCloseDocument
endcase 'tToolsOptionsDrawView

'--------------------------------------------------------------------------------

testcase tToolsOptionsDrawGrid (sApplication as string)
    dim irgendwas(10) as boolean
    dim sMetricField(7) as string      ' MetricField
    dim sTempMetricField(7) as string
    dim iTry as integer ' some increment variable

    if ( gApplication = "IMPRESS" ) then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    hNewDocument

    printlog "save states"
    ToolsOptions
    hToolsOptions (sApplication,"Grid")

    irgendwas(1) = FangrasterBenutzen.IsChecked
    irgendwas(2) = RasterSichtbar.IsChecked

    irgendwas(3) = AchsenSynchronisieren.IsChecked

    irgendwas(4) = AnFanglinien.IsChecked
    irgendwas(5) = AnSeitenraendern.IsChecked
    irgendwas(6) = AmObjektrahmen.IsChecked
    irgendwas(7) = AnObjektpunkten.IsChecked

    irgendwas(8) = BeimErzeugenUndBewegen.IsChecked
    irgendwas(9) = LaengereKantenlaenge.IsChecked
    irgendwas(10)= BeimDrehen.IsChecked
    if BeimDrehen.IsChecked then sMetricField(7) = Winkel.getText

    sMetricField(1) = RasterAufloesungXAchse.GetText
    sMetricField(2) = RasterAufloesungYAchse.GetText
    sMetricField(3) = RasterUnterteilungXAchse.GetText
    sMetricField(4) = RasterUnterteilungYAchse.GetText
    sMetricField(5) = FangenFangbereich.GetText
    sMetricField(6) = Punktreduktion.GetText

    printlog "all states inverting"

    if irgendwas(1) Then FangrasterBenutzen.UnCheck      Else FangrasterBenutzen.Check
    if irgendwas(2) Then RasterSichtbar.UnCheck          Else RasterSichtbar.Check

    '      if irgendwas(3) Then AchsenSynchronisieren.UnCheck   Else AchsenSynchronisieren.Check
    '   this CheckBox interacts with the MetricBoxes:
    AchsenSynchronisieren.UnCheck

    if irgendwas(4) Then AnFanglinien.UnCheck            Else AnFanglinien.Check
    if irgendwas(5) Then AnSeitenraendern.UnCheck        Else AnSeitenraendern.Check
    if irgendwas(6) Then AmObjektrahmen.UnCheck          Else AmObjektrahmen.Check
    if irgendwas(7) Then AnObjektpunkten.UnCheck         Else AnObjektpunkten.Check

    if irgendwas(8) Then BeimErzeugenUndBewegen.UnCheck  Else BeimErzeugenUndBewegen.Check
    if irgendwas(9) Then LaengereKantenlaenge.UnCheck    Else LaengereKantenlaenge.Check
    if irgendwas(10)Then BeimDrehen.UnCheck              Else BeimDrehen.Check

    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    sTempMetricField(5) = FangenFangbereich.GetText
    sTempMetricField(6) = Punktreduktion.GetText
    RasterAufloesungXAchse.ToMax
    RasterAufloesungYAchse.ToMin
    RasterUnterteilungXAchse.ToMin
    RasterUnterteilungYAchse.ToMax
    FangenFangbereich.ToMin
    Punktreduktion.ToMax
    If ( RasterAufloesungXAchse.GetText =   sTempMetricField(1) ) Then RasterAufloesungXAchse.ToMin
    If ( RasterAufloesungYAchse.GetText =   sTempMetricField(2) ) Then RasterAufloesungYAchse.ToMax
    If ( RasterUnterteilungXAchse.GetText = sTempMetricField(3) ) Then RasterUnterteilungXAchse.ToMax
    If ( RasterUnterteilungYAchse.GetText = sTempMetricField(4) ) Then RasterUnterteilungYAchse.ToMin
    If ( FangenFangbereich.GetText = sTempMetricField(5) ) Then FangenFangbereich.ToMax
    If ( Punktreduktion.GetText = sTempMetricField(6) ) Then Punktreduktion.ToMin
    RasterAufloesungXAchse.More  ' because it must greater than the.min value for the subdivisions if they are .tomax!
    RasterAufloesungXAchse.More
    RasterAufloesungYAchse.More
    RasterAufloesungYAchse.More
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    sTempMetricField(5) = FangenFangbereich.GetText
    sTempMetricField(6) = Punktreduktion.GetText

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "Office quit - start"

    ExitRestartTheOffice
    printlog "opening new doc to get the Options"
    Call hNewDocument
    
    printlog "checking states"
    ToolsOptions
    hToolsOptions (sApplication,"Grid")

    If ( irgendwas(1) = FangrasterBenutzen.IsChecked      ) Then WarnLog "Snap to Grid state changed"
    If ( irgendwas(2) = RasterSichtbar.IsChecked          ) Then WarnLog "Visible Grid state changed"

    '      If ( irgendwas(3) = AchsenSynchronisieren.IsChecked   ) Then WarnLog "Synchronize Axes state changed"
    if ( AchsenSynchronisieren.IsChecked = TRUE ) Then WarnLog "Synchronize Axes state changed"

    If ( irgendwas(4) = AnFanglinien.IsChecked            ) Then WarnLog "To snap lines state changed"
    If ( irgendwas(5) = AnSeitenraendern.IsChecked        ) Then WarnLog "To the page margins state changed"
    If ( irgendwas(6) = AmObjektrahmen.IsChecked          ) Then WarnLog "To object frame state changed"
    If ( irgendwas(7) = AnObjektpunkten.IsChecked         ) Then WarnLog "To object points state changed"

    If ( irgendwas(8) = BeimErzeugenUndBewegen.IsChecked  ) Then WarnLog "When creating or moving objects state changed"
    If ( irgendwas(9) = LaengereKantenlaenge.IsChecked    ) Then WarnLog "Extend Edges state changed"
    If ( irgendwas(10)= BeimDrehen.IsChecked              ) Then WarnLog "When Rotating state changed"

    If ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) Then WarnLog "GridResolutionXAxel state changed"
    If ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) Then WarnLog "GridResolutionYAxel state changed"
    If ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) Then WarnLog "GridSubdivisionXAxel state changed is: "+RasterUnterteilungXAchse.GetText+"; should: "+sTempMetricField(3)
    If ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) Then WarnLog "GridSubdivisionYAxel state changed is: "+RasterUnterteilungYAchse.GetText+"; should: "+sTempMetricField(4)
    If ( FangenFangbereich.GetText <> sTempMetricField(5) ) Then WarnLog "SnapSnap range state changed"
    If ( Punktreduktion.GetText <> sTempMetricField(6) ) Then WarnLog "Pointreduction state changed"

    printlog "all UnCheck -> o"

    FangrasterBenutzen.UnCheck
    RasterSichtbar.UnCheck

    AchsenSynchronisieren.UnCheck

    AnFanglinien.UnCheck
    AnSeitenraendern.UnCheck
    AmObjektrahmen.UnCheck
    AnObjektpunkten.UnCheck

    BeimErzeugenUndBewegen.UnCheck
    LaengereKantenlaenge.UnCheck
    BeimDrehen.UnCheck

    RasterAufloesungXAchse.ToMin
    RasterAufloesungYAchse.ToMin
    RasterUnterteilungXAchse.ToMin
    RasterUnterteilungYAchse.ToMin
    FangenFangbereich.ToMin
    Punktreduktion.ToMin
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    sTempMetricField(5) = FangenFangbereich.GetText
    sTempMetricField(6) = Punktreduktion.GetText
    printlog "More test"
    RasterAufloesungXAchse.More
    RasterAufloesungYAchse.More
    RasterUnterteilungXAchse.More
    RasterUnterteilungYAchse.More
    FangenFangbereich.More
    Punktreduktion.More
    if ( sTempMetricField(1) = RasterAufloesungXAchse.GetText   ) Then WarnLog "GridResolutionXAxel More-Button not working"
    if ( sTempMetricField(2) = RasterAufloesungYAchse.GetText   ) Then WarnLog "GridResolutionYAxel More-Button not working"
    if ( sTempMetricField(3) = RasterUnterteilungXAchse.GetText ) Then WarnLog "GridSubdivisionXAxel  More-Button not working"
    if ( sTempMetricField(4) = RasterUnterteilungYAchse.GetText ) Then WarnLog "GridSubdivisionYAxel  More-Button not working"
    if ( sTempMetricField(5) = FangenFangbereich.GetText ) Then WarnLog "Snap Snap Range More-Button not working"
    if ( sTempMetricField(6) = Punktreduktion.GetText ) Then WarnLog "Pointreduction    More-Button not working is: "+Punktreduktion.GetText
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText
    sTempMetricField(5) = FangenFangbereich.GetText
    sTempMetricField(6) = Punktreduktion.GetText

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "check if all UnChecked"
    ToolsOptions
    hToolsOptions (sApplication,"Grid")

    If FangrasterBenutzen.IsChecked     Then WarnLog "Snap to Grid x"
    If RasterSichtbar.IsChecked         Then WarnLog "Visible Grid x"

    If AchsenSynchronisieren.IsChecked  Then WarnLog "Synchronize Axes x"

    If AnFanglinien.IsChecked           Then WarnLog "To Snap Lines x"
    If AnSeitenraendern.IsChecked       Then WarnLog "To The Page Margins x"
    If AmObjektrahmen.IsChecked         Then WarnLog "To Object Frame x"
    If AnObjektpunkten.IsChecked        Then WarnLog "To Object Points x"

    If BeimErzeugenUndBewegen.IsChecked Then WarnLog "When Creating or Moving Objects x"
    If LaengereKantenlaenge.IsChecked   Then WarnLog "Extend Edges x"
    If BeimDrehen.IsChecked             Then WarnLog "When Rotating x"

    If ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) Then WarnLog "GridResolutionXAxel  is not min."
    If ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) Then WarnLog "GridResolutionYAxel  is not min."
    If ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) Then WarnLog "GridSubdivisionXAxel  is not min."
    If ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) Then WarnLog "GridSubdivisionYAxel  is not min."
    If ( FangenFangbereich.GetText <> sTempMetricField(5) ) Then WarnLog "Snap Snap Range  is not min."
    If ( Punktreduktion.GetText <> sTempMetricField(6) ) Then WarnLog "Point Reduction  is not min."

    Printlog "all Check -> x"

    FangrasterBenutzen.Check
    RasterSichtbar.Check

    AchsenSynchronisieren.Check

    printlog "Synchronize axes check"
    RasterAufloesungXAchse.ToMax
    sleep 1
    sTempMetricField(1) = RasterAufloesungXAchse.GetText
    if RasterAufloesungYAchse.GetText <> sTempMetricField(1) then Warnlog "Y-axis res. differs from X-axis"
    RasterUnterteilungXAchse.ToMax
    sleep 1
    sTempMetricField(3) = RasterUnterteilungXAchse.GetText
    if RasterUnterteilungYAchse.GetText <> sTempMetricField(3) then Warnlog "Y-axis sub. differs from X-axis"
    '--------------------------------------------
    sTempMetricField(2) = RasterAufloesungYAchse.GetText
    sTempMetricField(4) = RasterUnterteilungYAchse.GetText

    FangenFangbereich.Less
    Punktreduktion.Less
    sTempMetricField(5) = FangenFangbereich.GetText
    sTempMetricField(6) = Punktreduktion.GetText

    AnFanglinien.Check
    AnSeitenraendern.Check
    AmObjektrahmen.Check
    AnObjektpunkten.Check

    BeimErzeugenUndBewegen.Check
    LaengereKantenlaenge.Check
    BeimDrehen.Check
    Winkel.ToMax
    sTempMetricField(7) = Winkel.GetText

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "check if all Checked"
    ToolsOptions
    hToolsOptions (sApplication,"Grid")

    If True <> FangrasterBenutzen.IsChecked     Then WarnLog "Snap to Grid o"
    If True <> RasterSichtbar.IsChecked         Then WarnLog "Visible Grid o"

    If True <> AchsenSynchronisieren.IsChecked  Then WarnLog "Synchronize Axes o"

    If True <> AnFanglinien.IsChecked           Then WarnLog "to snap lines o"
    If True <> AnSeitenraendern.IsChecked       Then WarnLog "to the page margins o"
    If True <> AmObjektrahmen.IsChecked         Then WarnLog "to object frame o"
    If True <> AnObjektpunkten.IsChecked        Then WarnLog "To object points o"

    If True <> BeimErzeugenUndBewegen.IsChecked Then WarnLog "When creating or moving objects o"
    If True <> LaengereKantenlaenge.IsChecked   Then WarnLog "Extend Edges o"
    If True <> BeimDrehen.IsChecked             Then WarnLog "When Rotating o"
     If ( Winkel.GetText <> sTempMetricField(7) ) Then WarnLog "Angle    is not max."

    If ( RasterAufloesungXAchse.GetText <>   sTempMetricField(1) ) Then WarnLog "GridResolutionXAxel  is not max.Bug 54934 in 01:"
    If ( RasterAufloesungYAchse.GetText <>   sTempMetricField(2) ) Then WarnLog "GridResolutionYAxel  is not max."
    If ( RasterUnterteilungXAchse.GetText <> sTempMetricField(3) ) Then WarnLog "GridSubdivisionXAxel  is not max."
    If ( RasterUnterteilungYAchse.GetText <> sTempMetricField(4) ) Then WarnLog "GridSubdivisionYAxel  is not max."
    If ( FangenFangbereich.GetText <> sTempMetricField(5) ) Then WarnLog "Snap Snap Range  is not max."
    If ( Punktreduktion.GetText <> sTempMetricField(6) ) Then WarnLog "Pointreduction  is not max."

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

'-------------------------------------------------------------------------------
    printlog "Function Test: Grid Resolution"
    printlog "gTesttoolPath + graphics\required\input\gridtst.od?"
    if ( gApplication = "IMPRESS" ) then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    Call hFileOpen ( gTesttoolPath + "graphics\required\input\gridtst."+ExtensionString, False )
    sleep 5
    printlog "If write-protected - open as Temp-file"
    Kontext "Standardbar"
    if Bearbeiten.GetState(2) <> 1 then
      Bearbeiten.Click '0 = not pressed. 1 = pressed.
      Kontext
      if Active.Exists() then
         Active.Yes
      else
         warnlog "No messagebox after making document editable? - Test canceled here"
         goto endsub
      endif
    endif
    sleep (8)
    if uCase(gApplication) = "IMPRESS" then
      Kontext "DocumentImpress"
    else
      Kontext "DocumentDraw"
    endif
    printlog "select grafik element"
    for iTry = 1 to 20
        try
            EditSelectAll
            printlog "EditSelectAll ready after " & iTry/10 & " seconds"
            exit for
        catch
            wait( 100 )
        endcatch
    next iTry

   
    ContextPositionAndSize
    Kontext
    active.setpage TabPositionAndSize
    Kontext "TabPositionAndSize"
    sleep 1
    if ( StrToDouble (PositionX.GetText) <> 0 ) Then WarnLog "HorizontalBy wrong presupposition != 0"
    if ( StrToDouble (Positiony.GetText) <> 0 ) Then WarnLog   "VerticalBy   wrong presupposition != 0"
    TabPositionAndSize.Cancel
    sleep (3)

    printlog "move grafik with <Alt> + <up/down>"
    printlog "resulting step = defined for accessability <> one pixel with cursor traveling"
    hTypeKeys "<Mod2 Down>"
    sleep (3)
    hTypeKeys "<Mod2 Right>"

    printlog "check if @ (1,1) Current-Measure-Unit :-)"
    for iTry = 1 to 20
        try
            EditSelectAll
            printlog "EditSelectAll ready after " & iTry/10 & " seconds"
            exit for
        catch
            wait( 100 )
        endcatch
    next iTry
    try
      ContextPositionAndSize
    catch
      printlog "Why is format position and size not enabled ? Wrong focus?"
    endcatch
    Kontext
    active.setpage TabPositionAndSize
    Kontext "TabPositionAndSize"
    Printlog "X difference is: '"+StrToDouble (PositionX.GetText)+"'; "+PositionX.GetText
    Printlog "Y difference is: '"+StrToDouble (PositionY.GetText)+"'; "+PositionY.GetText
    TabPositionAndSize.Cancel
    '   sleep (3)

   hCloseDocument

'--------------------------------------------------
    printlog "restore states"
    hNewDocument
    ToolsOptions
    hToolsOptions (sApplication,"Grid")

    If ( irgendwas(2) = TRUE ) Then  FangrasterBenutzen.Check      Else FangrasterBenutzen.UnCheck
    If ( irgendwas(3) = TRUE ) Then  RasterSichtbar.Check          Else RasterSichtbar.UnCheck

    If ( irgendwas(4) = TRUE ) Then  AchsenSynchronisieren.Check   Else AchsenSynchronisieren.UnCheck

    If ( irgendwas(5) = TRUE ) Then  AnFanglinien.Check            Else AnFanglinien.UnCheck
    If ( irgendwas(6) = TRUE ) Then  AnSeitenraendern.Check        Else AnSeitenraendern.UnCheck
    If ( irgendwas(7) = TRUE ) Then  AmObjektrahmen.Check          Else AmObjektrahmen.UnCheck
    If ( irgendwas(8) = TRUE ) Then  AnObjektpunkten.Check         Else AnObjektpunkten.UnCheck

    If ( irgendwas(6) = TRUE ) Then  BeimErzeugenUndBewegen.Check  Else BeimErzeugenUndBewegen.UnCheck
    If ( irgendwas(7) = TRUE ) Then  LaengereKantenlaenge.Check    Else LaengereKantenlaenge.UnCheck
    If ( irgendwas(8) = TRUE ) Then  BeimDrehen.Check              Else BeimDrehen.UnCheck
    if BeimDrehen.IsChecked then Winkel.SetText (sMetricField(7))

    RasterAufloesungXAchse.SetText   (sMetricField(1))
    RasterAufloesungYAchse.SetText   (sMetricField(2))
    RasterUnterteilungXAchse.SetText (sMetricField(3))
    RasterUnterteilungYAchse.SetText (sMetricField(4))
    FangenFangbereich.SetText (sMetricField(5))
    Punktreduktion.SetText (sMetricField(6))

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    hCloseDocument
endcase 'tToolsOptionsDrawGrid

'-------------------------------------------------------------------------------

testcase tToolsOptionsDrawPrint (sApplication as string)

    dim irgendwas(11) as boolean
    dim bRadioBut(7) as boolean         ' RadioButtons
    dim bTempRadioBut(7) as boolean
    dim iShouldHaveBeenAMultiListBox as integer

    hNewDocument

    printlog "save states"
    ToolsOptions
    hToolsOptions (sApplication,"Print")

    Standard.Check
    irgendwas(1) = Seitenname.IsChecked
    irgendwas(2) = Datum.IsChecked
    irgendwas(3) = Zeit.IsChecked
    irgendwas(4) = AusgeblendeteSeitenDrucken.IsChecked

    Prospekt.Check
    irgendwas(5) = Vorderseite.IsChecked
    irgendwas(6) = Rueckseite.IsChecked

    irgendwas(7) = AusDruckereinstellung.IsChecked

    bRadioBut(1) = StandardAusgabe.IsChecked
    bRadioBut(2) = Graustufen.IsChecked
    bRadioBut(3) = SchwarzWeiss.IsChecked
    bRadioBut(4) = Standard.IsChecked
    bRadioBut(5) = AnSeitengroesseAnpassen.IsChecked
    bRadioBut(6) = SeitenKacheln.IsChecked

    printlog "all states inverting"

    Standard.Check
    if irgendwas(1) Then Seitenname.UnCheck            Else Seitenname.Check
    if irgendwas(2) Then Datum.UnCheck                 Else Datum.Check
    if irgendwas(3) Then Zeit.UnCheck                  Else Zeit.Check
    if irgendwas(4) Then AusgeblendeteSeitenDrucken.UnCheck   Else AusgeblendeteSeitenDrucken.Check

    Prospekt.Check
    if irgendwas(5) Then Vorderseite.UnCheck           Else Vorderseite.Check
    if irgendwas(6) Then Rueckseite.UnCheck            Else Rueckseite.Check

    if irgendwas(7) Then AusDruckereinstellung.UnCheck Else AusDruckereinstellung.Check

    if (StandardAusgabe.IsChecked = True) Then Graustufen.Check Else StandardAusgabe.Check
    bTempRadioBut(1) = StandardAusgabe.IsChecked
    bTempRadioBut(2) = Graustufen.IsChecked
    bTempRadioBut(3) = SchwarzWeiss.IsChecked
    if (Standard.IsChecked = True) Then AnSeitengroesseAnpassen.Check Else Standard.Check
    bRadioBut(4) = Standard.IsChecked
    bRadioBut(5) = AnSeitengroesseAnpassen.IsChecked
    bRadioBut(6) = SeitenKacheln.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "SO quit - start"

    ExitRestartTheOffice
    printlog "opening new doc to get the Options"
    Call hNewDocument
    printlog "checking states"
    ToolsOptions
    hToolsOptions (sApplication,"Print")

    Standard.Check
    If ( irgendwas(1) = Seitenname.IsChecked            ) Then WarnLog "Pagename  state changed"
    If ( irgendwas(2) = Datum.IsChecked                 ) Then WarnLog "Date  state changed"
    If ( irgendwas(3) = Zeit.IsChecked                  ) Then WarnLog "Time  state changed"
    If ( irgendwas(4) = AusgeblendeteSeitenDrucken.IsChecked   ) Then WarnLog "Hidden Pages  state changed"

    Prospekt.Check
    If ( irgendwas(5) = Vorderseite.IsChecked           ) Then WarnLog "Brochure Front  state changed"
    If ( irgendwas(6) = Rueckseite.IsChecked            ) Then WarnLog "Brochure Back  state changed"

    If ( irgendwas(7) = AusDruckereinstellung.IsChecked ) Then WarnLog "Paper Tray From Printer Settings  state changed"

    If ( bTempRadioBut(1) <> StandardAusgabe.IsChecked ) Then WarnLog "Quality Default  state changed"
    If ( bTempRadioBut(2) <> Graustufen.IsChecked      ) Then WarnLog "Quality Grayscale  state changed"
    If ( bTempRadioBut(3) <> SchwarzWeiss.IsChecked    ) Then WarnLog "Quality Black & White  state changed"
    If ( bTempRadioBut(4) <> Standard.IsChecked                ) Then WarnLog "Page Options Default  state changed"
    If ( bTempRadioBut(5) <> AnSeitengroesseAnpassen.IsChecked ) Then WarnLog "Page Options Fit to Page  state changed"
    If ( bTempRadioBut(6) <> SeitenKacheln.IsChecked           ) Then WarnLog "Page Options Tile Pages state changed"

    printlog "all UnCheck -> o"

    Standard.Check
    Seitenname.UnCheck
    Datum.UnCheck
    Zeit.UnCheck
    AusgeblendeteSeitenDrucken.UnCheck

    Prospekt.Check
    Vorderseite.UnCheck
    Rueckseite.UnCheck

    AusDruckereinstellung.UnCheck

    if (Graustufen.IsChecked = True) Then SchwarzWeiss.Check Else Graustufen.Check
    bTempRadioBut(1) = StandardAusgabe.IsChecked
    bTempRadioBut(2) = Graustufen.IsChecked
    bTempRadioBut(3) = SchwarzWeiss.IsChecked
    if (AnSeitengroesseAnpassen.IsChecked = True) Then SeitenKacheln.Check Else AnSeitengroesseAnpassen.Check
    bRadioBut(4) = Standard.IsChecked
    bRadioBut(5) = AnSeitengroesseAnpassen.IsChecked
    bRadioBut(6) = SeitenKacheln.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "check if all UnChecked"
    ToolsOptions
    hToolsOptions (sApplication,"Print")

    Standard.Check
    If Seitenname.IsChecked            Then WarnLog "Print: Page Name  x"
    If Datum.IsChecked                 Then WarnLog "Print: Date  x"
    If Zeit.IsChecked                  Then WarnLog "Print: Time  x"
    If AusgeblendeteSeitenDrucken.IsChecked   Then WarnLog "Print: Hidden Pages  x"
    Prospekt.Check
    If Vorderseite.IsChecked           Then WarnLog "Page options: Brochure: Front  x"
    If Rueckseite.IsChecked            Then WarnLog "Page options: Brochure: Back  x"
    If AusDruckereinstellung.IsChecked Then WarnLog "Page options: Paper tray from printer settings  x"
    If ( bTempRadioBut(1) <> StandardAusgabe.IsChecked) Then WarnLog "Quality: Default  state changed"
    If ( bTempRadioBut(2) <> Graustufen.IsChecked     ) Then WarnLog "Quality: Grayscale  state changed"
    If ( bTempRadioBut(3) <> SchwarzWeiss.IsChecked   ) Then WarnLog "Quality: Black & White  state changed"
    If ( bTempRadioBut(4) <> Standard.IsChecked                ) Then WarnLog "Page options: Default  state changed"
    If ( bTempRadioBut(5) <> AnSeitengroesseAnpassen.IsChecked ) Then WarnLog "Page options: Fit to page  state changed"
    If ( bTempRadioBut(6) <> SeitenKacheln.IsChecked           ) Then WarnLog "Page options: Tile Pages  state changed"

    Printlog "all Check -> x"
    Standard.Check
    Seitenname.Check
    Datum.Check
    Zeit.Check
    AusgeblendeteSeitenDrucken.Check
    Prospekt.Check
    Vorderseite.Check
    Rueckseite.Check
    AusDruckereinstellung.Check
    if (SchwarzWeiss.IsChecked = True) Then StandardAusgabe.Check Else SchwarzWeiss.Check
    bTempRadioBut(1) = StandardAusgabe.IsChecked
    bTempRadioBut(2) = Graustufen.IsChecked
    bTempRadioBut(3) = SchwarzWeiss.IsChecked
    if (SeitenKacheln.IsChecked = True) Then Standard.Check Else SeitenKacheln.Check
    bRadioBut(4) = Standard.IsChecked
    bRadioBut(5) = AnSeitengroesseAnpassen.IsChecked
    bRadioBut(6) = SeitenKacheln.IsChecked

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    printlog "check if all Checked"
    ToolsOptions
    hToolsOptions (sApplication,"Print")

    Standard.Check
    If True <> Seitenname.IsChecked            Then WarnLog "Print: Page name  o"
    If True <> Datum.IsChecked                 Then WarnLog "Print: Date  o"
    If True <> Zeit.IsChecked                  Then WarnLog "Print: Time  o"
    If True <> AusgeblendeteSeitenDrucken.IsChecked   Then WarnLog "Print: Hidden pages  o"

    Prospekt.Check
    If True <> Vorderseite.IsChecked           Then WarnLog "Page options: Brochure: Front  o"
    If True <> Rueckseite.IsChecked            Then WarnLog "Page options: Brochure: Back  o"

    If True <> AusDruckereinstellung.IsChecked Then WarnLog "Page options: Paper tray from printer settings  o"

    If ( bTempRadioBut(1) <> StandardAusgabe.IsChecked) Then WarnLog "Quality: Default  state changed"
    If ( bTempRadioBut(2) <> Graustufen.IsChecked     ) Then WarnLog "Quality: Grayscale  state changed"
    If ( bTempRadioBut(3) <> SchwarzWeiss.IsChecked   ) Then WarnLog "Quality: Black & White  state changed"
    If ( bTempRadioBut(4) <> Standard.IsChecked                ) Then WarnLog "Page options: Default  state changed"
    If ( bTempRadioBut(5) <> AnSeitengroesseAnpassen.IsChecked ) Then WarnLog "Page options: Fit to page  state changed"
    If ( bTempRadioBut(6) <> SeitenKacheln.IsChecked           ) Then WarnLog "Page options: Tile pages  state changed"

    printlog "restore states"

    Standard.Check
    If ( irgendwas(1) = TRUE ) Then Seitenname.Check            Else Seitenname.UnCheck
    If ( irgendwas(2) = TRUE ) Then Datum.Check                 Else Datum.UnCheck
    If ( irgendwas(3) = TRUE ) Then Zeit.Check                  Else Zeit.UnCheck
    If ( irgendwas(4) = TRUE ) Then AusgeblendeteSeitenDrucken.Check   Else AusgeblendeteSeitenDrucken.UnCheck

    Prospekt.Check
    If ( irgendwas(5) = TRUE ) Then Vorderseite.Check           Else Vorderseite.UnCheck
    If ( irgendwas(6) = TRUE ) Then Rueckseite.Check            Else Rueckseite.UnCheck

    If ( irgendwas(7) = TRUE ) Then AusDruckereinstellung.Check Else AusDruckereinstellung.UnCheck

    If ( bRadioBut(1) = TRUE ) Then StandardAusgabe.Check
    If ( bRadioBut(2) = TRUE ) Then Graustufen.Check
    If ( bRadioBut(3) = TRUE ) Then SchwarzWeiss.Check
    Standard.Check

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK

    hCloseDocument
endcase 'tToolsOptionsDrawPrint