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
'* owner : helge.delfs@oracle.com
'*
'* short description : Section test(Insert/Section Format/Section)
'*
'************************************************************************
'*
' #1  tInsertSection_1      'test for insert a section
' #1  tInsertSection_2      'test for the default section name and automatic numbering
' #1  tInsertSection_3      'Need to check section name
' #1  tInsertSection_4      'check if the layout in section is not destroyed
' #1  tInsertSection_5      'Linked Section test
' #1  tInsertSection_6      'Insert Section should be disabled in the follow opeation
'*
'\***********************************************************************

sub w_section_1
    printLog Chr(13) + "---------   Insert Section   ----------"
    Call tInsertSection_1
    Call tInsertSection_2
    Call tInsertSection_3
    Call tInsertSection_4
    Call tInsertSection_5
    Call tInsertSection_6
end sub

'-------------------------------------------------------------------------

testcase tInsertSection_1

    Call hNewDocument
    Call fInsertSection("Section Test")
    sleep(1)
    Call wTypeKeys "<RETURN>"
    '/// Test for insert a section with western words ///
    Call fInsertSection("Western Test")
    sleep(1)
    Call wTypeKeys "<UP>" , 1
    Call wTypeKeys "Section : western test"

    'If Asian office...
    if bAsianLan = TRUE then
        Call wTypeKeys "<DOWN>" , 1
        Call wTypeKeys "<RETURN>"
        '///+... test for insert a section with asian character -- Simplified Chinese ///
        Call fInsertSection("Asian Test")
        sleep(1)
        Call wTypeKeys "<UP>" , 1
        Call wTypeKeys "简体中文"
    end if

    Call wTypeKeys "<DOWN>" , 1
    Call wTypeKeys "<RETURN>"

    '/// Test for insert a section with a graphic object
    Call fInsertSection("Graphic Test")
    Call wTypeKeys "<UP>" , 1
    Call hGrafikEinfuegen(gTesttoolPath + "writer\optional\input\graphics\jolink.jpg")
    sleep(1)
    gMouseClick(6,1)
    Call wTypeKeys "<DOWN>" , 10
    Call wTypeKeys "<RETURN>"
    '/// Test for insert a section with a draw object ///
    Call fInsertSection("Draw Test")
    sleep(1)
    Call wTypeKeys "<UP>" , 1
    InsertObjectOLEObject
    sleep(1)
    Kontext "OLEObjektEinfuegen"
    AusDateiErstellen.Check
    oleDatei.Settext ConvertPath(gTesttoolpath + "writer\optional\input\section\section_test.sxd")
    OLEObjektEinfuegen.OK
    sleep(4)
    'Close document
    Call hCloseDocument
    sleep(1)

    Call hNewDocument

    '/// Test for insert a section with combination of different objects///
    Call fInsertSection("combination of different objects test")
    sleep(1)

    '/// -- western words  ///
    Call wTypeKeys "<UP>" , 1
    Call wTypeKeys "Section : western test<return>"
    '/// -- Graphic  ///
    Call hGrafikEinfuegen(gTesttoolPath & "writer\optional\input\graphics\jolink.jpg")
    sleep(1)
    gMouseClick(1,1)
    Call wTypeKeys "<DOWN>" , 1
    Call wTypeKeys "<RETURN>"
    '/// -- drawing object ///
    InsertObjectOLEObject
    sleep(1)
    Kontext "OLEObjektEinfuegen"
    AusDateiErstellen.Check
    oleDatei.Settext ConvertPath(gTesttoolpath & "writer\optional\input\section\section_test.sxd")
    OLEObjektEinfuegen.OK
    sleep(3)

    Call hCloseDocument

endcase

'----------------------------------------------------------

testcase tInsertSection_2

    Dim SectionName as String
    Dim j
    
    '/// Test for the default section name and automatic numbering
    '///+<b>Only Big 10 languages!</b>
    '///+ Automatic numbering test is from 1-3.
    
    Call hNewDocument
    for j=1 to 3
        SectionName = gDefaultSectionName + j
        InsertSection
        sleep(2)
        Kontext "TabBereiche"
        TabBereiche.OK
        sleep(2)
        Call wTypeKeys "<UP>"
        'Check the section name is correct or NOT (Format/Section)
        FormatSections
        sleep(1)
        Kontext "BereicheBearbeiten"
        if BereichsName.GetText <> SectionName then
            QAErrorLog "The default Section Name may be changed or wrong wrong : " & BereichsName.Gettext
        end if
        BereicheBearbeiten.Cancel
        sleep(1)
        Call wTypeKeys "<DOWN>"
    next j
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tInsertSection_3

    Dim SectionName() as Variant
    Dim i,j as Integer

    '*************************************************************
    '/// Need to check section name, include :                ///
    '///   SectionTest   --- normal name                      ///
    '///   Section Test  --- devided by space                 ///
    '///   Section-Test  --- devided by dash                  ///
    '///   Section*Test  --- devided by asterisk              ///
    '///   sèction      --- include German umlaut character  ///
    '///   säction      --- include French character         ///
    '                                                         ///
    '/// *** maybe need add more testcases here  ***          ///
    '*************************************************************

    SectionName = Array( _
                       "SectionTest"     , _
                       "Section Test"    , _
                       "Section-Test"    , _
                       "Section*Test"    , _
                       "sèction"        , _
                       "säction"        , _
                       )

    j = UBound(SectionName)

    Call hNewDocument

    For i = 0 to j
        try
            fInsertSection(SectionName(i))
            Call wTypeKeys "<Up>" , 1

            FormatSections
            Kontext "BereicheBearbeiten"
            if BereichsName.GetText <> SectionName(i) then warnlog "    The Section Name is wrong, it should be :" +SectionName(i) +", but: "+BereichsName.GetText
            BereicheBearbeiten.Cancel

            Call wTypeKeys "<Down>" , 1
        catch
            Warnlog "   Insert Section is failed , the section name should be " +SectionName(i)
        endcatch
    next i
    
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tInsertSection_4

    Dim Message1 as String
    Dim j

    '/// Check if the layout in section is not destroyed.

    '/// 1. Test for a section includes more than 1 line of text.
    printlog "     Section includes more than 1 line of text"
    Call hNewDocument
    sleep(2)
    Call wTypeKeys "This is first line<return>"
    Call wTypeKeys "This is Second line"
    EditSelectAll
    sleep(1)
    EditCopy
    sleep(1)
    Message1 = GetClipboardText
    InsertSection
    sleep(2)
    Kontext "TabBereiche"
    TabBereiche.OK
    sleep(1)
    EditSelectAll
    sleep(1)
    EditCopy
    sleep(1)
    if Message1 <> GetClipboardText then Warnlog "     Layout is destroyed"
    Call hCloseDocument

    '/// 2. test for a section includes more than 1 graphic///
    printlog "     Section includes more than 1 graphic"
    Call hNewDocument
    sleep(2)
    Call hGrafikEinfuegen ( gTesttoolPath & "writer\optional\input\graphics\jolink.jpg" )
    sleep(2)
    FormatGraphics
    sleep(2)
    Kontext
    Active.SetPage TabZusaetze
    Kontext "TabZusaetze"
    ObjektName.SetText "jolink"
    sleep(2)
    TabZusaetze.OK
    sleep(1)
    Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<RETURN>" , 2
    sleep(1)
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\optional\input\graphics\bild.jpg" )
    sleep(2)
    FormatGraphics
    sleep(2)
    Kontext
    Active.SetPage TabZusaetze
    Kontext "TabZusaetze"
    ObjektName.SetText "bild"
    TabZusaetze.OK
    sleep(1)
    Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<RETURN>" , 4
    sleep(1)
    EditSelectAll
    sleep(2)
    InsertSection
    sleep(2)
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    TabBereiche.OK
    'Check if correct graphic have been selected
    for j=1 to 2
        Call wNavigatorAuswahl(4,j)
        FormatGraphics
        sleep(2)
        Kontext
        Active.SetPage TabZusaetze
        Kontext "TabZusaetze"
        sleep(1)
        Message1 = ObjektName.GetText
        select case j
            case 1 : if Message1 <> "jolink"   then Warnlog "Wrong Graphicname: Not -> 'jolink'   But -> "+ Message1
            case 2 : if Message1 <> "bild" then Warnlog "Wrong Graphicname: Not -> 'bild'  But -> "+ Message1
        end select
        TabZusaetze.Cancel
        sleep(1)
    next j
    Call fCloseNavigator
    sleep(1)
    Call hCloseDocument

    '/// 3. test for a section across page breaks ///
    printlog "      Section across page breaks"
    Call hNewDocument

    ' Call dialog with Insert/ManualBreak and in dialog check 'page break'
    Call wTypeKeys "Page Break"
    Call wTypeKeys "<LEFT>" , 5
    Kontext
    InsertManualBreak
    sleep(1)
    Kontext "UmbruchEinfuegen"
    Seitenumbruch.Check
    UmbruchEinfuegen.OK
    sleep(1)
    EditSelectAll
    sleep(1)
    InsertSection
    sleep(2)
    Kontext "TabBereiche"
    TabBereiche.OK
    sleep(1)
    Kontext
    ViewNavigator
    sleep(1)
    Kontext "NavigatorWriter"
    Seitennummer.SetText "2"
    sleep(2)
    'set page number to 2
    if Seitennummer.GetText <> "2" then
        warnlog "Something wrong in a section across page breaks, perhaps due to bug #101873."
    end if
    Call fCloseNavigator
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tInsertSection_5

    Dim Message1
    Dim Message2
    Dim Message3
    
    if Dir (gOfficePath + "user\work\link_of_section_test.oot") <> "" then
        kill (gOfficePath + "user\work\link_of_section_test.oot")
    end if
    
    '/// Linked Section test ///
    
    Call hNewDocument
    
    '/// 1. Check option 'Link' in dialogue Insert-Section  ///
    printlog "   - Check option 'Link' in dialogue Insert-Section"
    InsertSection
    sleep 2
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    sleep 1
    
    Geschuetzt.UnCheck
    Verknuepfung.Check
    if Geschuetzt.IsChecked <>TRUE then warnlog "The checkbox 'protect' is NOT checked"
    
    '/// 2. Check Link from other sections ///
    printlog "   - Check Link from other sections "
    
    '/// - Check if the link document contains no sections"
    printlog "       - Check if the link document contains no sections"
    
    DateiVerknuepfung.Settext ConvertPath(gTesttoolpath + "writer\optional\input\section\bt.sxw")
    
    if Bereich.GetItemCount <> 0 then warnlog "Should NO entry in the section listbox"
    TabBereiche.Cancel
    
    Call wTypeKeys "Hello World!"
    EditSelectAll
    
    FormatCharacter
    sleep 2
    Kontext
    
    ' On Tabpage 'Font' adjust settings
    active.SetPage TabFont
    kontext "TabFont"
    try
        FontWest.Select 1
        Message2 = FontWest.GetSelText
        Message1 = StyleWest.GetItemCount
        StyleWest.Select Message1
        Message3 = StyleWest.GetSelText
        SizeWest.Select 24
    catch
        Font.Select 1
        Message2 = Font.GetSelText
        Message1 = Style.GetItemCount
        Style.Select Message1
        Message3 = Style.GetSelText
        Size.Select 24
    endcatch
    wait 100
    
    TabFont.OK
    
    Call fInsertSection("Section-Test-link")
    
    ' Save document
    if gApplication = "WRITER" then
        Call hFileSaveAsKill (gOfficePath + "user\work\link_of_section_test.odt")
    end if
    
    Call hCloseDocument
    
    Call hNewDocument
    
    InsertSection
    sleep 2
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Verknuepfung.Check

    Auswaehlen.Click
    Kontext "OeffnenDlg"
    Dateiname.Settext ConvertPath(gOfficePath + "user\work\link_of_section_test.odt")
    Auswaehlen.Click
    sleep 4
    
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Geschuetzt.UnCheck
    
    try
        Bereich.Select 1
    catch
        Warnlog "Something wrong in Section name . Bug i35168"
    endcatch
    
    if Bereich.GetSelText <> "Section-Test-link" then warnlog "the section name is wrong" + Bereich.GetSelText
    
    Bereichsliste.Settext "SectionLink"
    
    TabBereiche.OK
    sleep 2
    
    Call wTypeKeys "<up>" , 2
    
    FormatSections
    Kontext "BereicheBearbeiten"
    if BereichsName.GetText <> "SectionLink" then warnlog "The Section Name is wrong : " + BereichsName.Gettext
    BereicheBearbeiten.Cancel
    
    FormatCharacter
    sleep 2
    Kontext
    'On Tabpage 'Font' adjust settings
    active.SetPage TabFont
    kontext "TabFont"
    try
        if FontWest.GetSelIndex <> 1 then Warnlog "Font should "+ Message2 +" is "+ FontWest.GetSelText
        if StyleWest.GetSelIndex <> Message1 then Warnlog "FontStyle should " +Message3+" is "+ StyleWest.GetSelText
        if SizeWest.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + SizeWest.GetSelText
    catch
        if Font.GetSelIndex <> 1 then Warnlog "Font should "+ Message2 +" is "+ Font.GetSelText
        if Style.GetSelIndex <> Message1 then Warnlog "FontStyle should " +Message3+" is "+ Style.GetSelText
        if Size.GetSelIndex <> 24 then Warnlog "Wrong Font Size : " + Size.GetSelText
    endcatch
    wait 100
    TabFont.Cancel
    Call hCloseDocument
    
    '/// 3. Check simple link from file ///
    printlog "   - Check simple link from file"
    
    '/// test for entering path/name directly ///
    Call hNewDocument
    
    InsertSection
    sleep 2
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    Verknuepfung.Check
    
    DateiVerknuepfung.Settext ConvertPath(gTesttoolpath + "writer\optional\input\section\bt.sxw")
    TabBereiche.OK
    sleep 4
    
    Call hCloseDocument
    
    '/// 4. test via browse - button ///
    Call hNewDocument
    
    InsertSection
    sleep 2
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    sleep 1
    Verknuepfung.Check
    
    Auswaehlen.Click   
    Kontext "OeffnenDlg"
    Dateiname.Settext ConvertPath(gTesttoolpath + "writer\optional\input\section\bt.sxw")
    Auswaehlen.Click
    sleep 4
    
    Kontext
    Active.Setpage TabBereiche
    Kontext "TabBereiche"
    TabBereiche.OK
    sleep 4
    
    Call hCloseDocument
endcase

'----------------------------------------------------------

testcase tInsertSection_6

    '/// Insert Section should be disabled in the follow opeations///
    printlog "  - Insert Section should be disabled in the follow opeations"
    
    Call hNewDocument
    
    '/// 1. Multiselection  ///
    printlog "  - Multiselection "
    
    Call wTypeKeys "Multiselection test , Multiselection test !"
    Call fMultiSelection("Multiselection")
    
    try
        InsertSection
        sleep 2
        Warnlog "Menu Insert / Section should be disabled !"
    catch
    endcatch
    
    Call hCloseDocument
    
    Call hNewDocument
    
    '/// 2. cell selection in table ///
    printlog "  - Check cell selection in table "
    InsertTableWriter
    
    Kontext "TabelleEinfuegenWriter"
    TabelleEinfuegenWriter.OK
    sleep 1
    
    Call wTypeKeys "<Shift Right>"
    
    try
        InsertSection
        sleep 2
        Warnlog "Menu Insert / Section should be disabled !"
    catch
    endcatch
    
    Call hCloseDocument
    
    '/// 3. insert a frame  ///
    printlog "  - Check after insert a frame "
    Call hNewDocument
    InsertFrame
    Kontext
    active.SetPage TabType
    Kontext "TabType"
    TabType.OK
    
    try
        InsertSection
        sleep 2
        Warnlog "Menu Insert / Section should be disabled !"
    catch
    endcatch
    
    Call hCloseDocument
    
    '/// 4. insert a graphic  ///
    printlog "  - Check after insert a graphic "
    Call hNewDocument
    Call hGrafikEinfuegen ( gTesttoolPath + "writer\optional\input\graphics\jolink.jpg" )
    
    try
        InsertSection
        sleep 2
        Warnlog "Menu Insert / Section should be disabled !"
    catch
    endcatch
    
    Call hCloseDocument
    
    '/// 5. insert a OLE object  ///
    printlog "  - Check after insert a OLE-object (drawing) "
    Call hNewDocument
    InsertObjectOLEObject
    Kontext "OLEObjektEinfuegen"
    AusDateiErstellen.Check
    oleDatei.Settext ConvertPath(gTesttoolpath + "writer\optional\input\section\section_test.sxd")
    OLEObjektEinfuegen.OK
    sleep 3
    try
        InsertSection
        sleep 2
        Warnlog "Menu Insert / Section should be disabled !"
    catch
    endcatch
    
    Call hCloseDocument
endcase
