'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: id_002.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:41 $
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
'* Owner : wolfram.garten@sun.com
'*
'* short description :
'*
'***********************************************************************************
' #1 tiEditUndoRedo
' #1 tiEditRepeat
' #1 tiEditCutPasteCopySelectall
' #1 tiEditPasteSpecial
' #1 tiEditSearchAndReplace
' #1 tiEditDuplicate
' #1 tiEditFields
' #1 tdEditDeleteSlide
' #1 tiEditLinks
' #1 tiEditImageMap
' #1 tiEditObjectProperties
' #1 tiEditObjectEdit
' #1 tiEditPlugIn
' #1 tiEditHyperlink
' #1 tEditPoints
'\**********************************************************************************

testcase tiEditUndoRedo

    hNewDocument
    call hTBOtypeInDoc

    EditUndo
    WaitSlot (2000)
    EditRedo
    WaitSlot (2000)
    Call hCloseDocument
endcase

testcase tiEditRepeat
    goto endsub 'Quaste, ask FHA
    Call hNewDocument

    gMouseClick 50,50
    Call hRechteckErstellen ( 30, 10, 70, 30 )
    WaitSlot (1000)
    Call hRechteckErstellen ( 20, 20, 60, 40 )
    WaitSlot (1000)
    Call hRechteckErstellen ( 80, 50, 40, 20 )
    WaitSlot (1000)
    ContextArrangeBringBackward
    WaitSlot (2000)
    try
        EditRepeat
    catch
        Warnlog " Menu entry is disabled #i26129#"
    endcatch

    Call hCloseDocument
endcase

testcase tiEditCutPasteCopySelectall
    Call hNewDocument

    call hTBOtypeInDoc

    EditCut
    WaitSlot (2000)
    EditPaste
    WaitSlot (2000)
    EditCopy
    WaitSlot (2000)
    EditPaste
    WaitSlot (2000)
    EditSelectAll
    WaitSlot (2000)
    EditCut
    WaitSlot (2000)
    EditPaste
    WaitSlot (2000)
    EditDeleteContents
    WaitSlot (2000)
    Call hCloseDocument
endcase

testcase tiEditPasteSpecial
    Call  hNewDocument

    SetClipboard "This is a Text in the Clipboard"

    EditPasteSpecial
    WaitSlot (1000)
    Kontext "InhaltEinfuegen"
    DialogTest ( InhaltEinfuegen )

    InhaltEinfuegen.Cancel
    WaitSlot (1000)
    Call hCloseDocument
endcase

testcase tiEditSearchAndReplace
    Call  hNewDocument

    try
        EditSearchAndReplace
        WaitSlot (1000)
        Kontext "FindAndReplace"
        DialogTest ( FindAndReplace )

        More.Click
        SimilaritySearch.Check ' culprint for errors if not resetted !
        WaitSlot (1000)
        SimilaritySearchFor.Click
        Kontext "Aehnlichkeitssuche"
        DialogTest (Aehnlichkeitssuche )
        Aehnlichkeitssuche.Cancel
        Kontext "FindAndReplace"
        SimilaritySearch.UnCheck
        More.Click
        FindAndReplace.Close
    catch
        Warnlog "EditSearchAndReplace caused an error"
    endcatch
    Call hCloseDocument
endcase

testcase tiEditDuplicate
    Call hNewDocument
    call hTBOtypeInDoc
    EditSelectAll
    EditDuplicate

    Kontext "Duplizieren"
    Call DialogTest ( Duplizieren )
    Duplizieren.Cancel

    Call hCloseDocument
endcase

testcase tEditPoints
    Call hNewDocument
    call hTBOtypeInDoc
    FormatEditPoints
    EditGluePoints
    Call hCloseDocument
endcase

testcase tiEditFields
    Call hNewDocument
    WaitSlot (2000)
    InsertFieldsDateFix
    WaitSlot (1000)
    gMouseDoubleClick 10,10

    hTypeKeys "<ESCAPE>"
    hTypeKeys "<Tab>"                  ' With a Tab catches we always the Object
    hTypeKeys "<F2>"                   ' Here we enter Edit-Mode and therefore also the right place
    hTypeKeys "<Home>"                 ' Here we enter Edit-Mode and therefore also the right place

    try
        EditFieldsDraw
        Kontext "FeldbefehlBearbeitenDraw"
        Call DialogTest ( FeldbefehlBearbeitenDraw )
        FeldbefehlBearbeitenDraw.Close
    catch
        Warnlog "- Slot could not be accessed"
    endcatch

    Call hCloseDocument
endcase

testcase tdEditDeleteSlide
    Call hNewDocument
    InsertSlide
    WaitSlot (2000)
    hTypekeys "<Pagedown>"
    WaitSlot (2000)
    Kontext "Navigator"
    sleep (2)
    if Navigator.exists then
        printlog "Navigator: open :-)"
    else
        printlog "Navigator: NOT available :-( Will be opened now!"
        ViewNavigator
    end if
    WaitSlot (2000)
    Kontext "NavigatorDraw"
    if Liste.GetItemCount<>2 Then
        Warnlog "-  No slide inserted"
        Kontext "Navigator"
        Navigator.Close
        Call hCloseDocument
        goto endsub
    else
        Liste.Select 2
        Kontext "Navigator"
        Navigator.Close
    end if
    WaitSlot (2000)
    EditDeleteSlide
    WaitSlot (2000)
    Call hCloseDocument
endcase

testcase tiEditLinks
    Call  hNewDocument

    InsertGraphicsFromFile
    Kontext "GrafikEinfuegenDlg"
    try
        if Link.Exists then
            Link.Check
        else
            Warnlog "- Link in Insert graphic is not working"
        end if
        Dateiname.settext Convertpath (gTesttoolPath + "global\input\graf_inp\stabler.tif")
        Oeffnen.Click
        Kontext "Messagebox"
        if Messagebox.Exists=True Then
            Warnlog Messagebox.GetText
            Messagebox.Ok
        end if
        InsertGraphicsFromFile
        Kontext "GrafikEinfuegenDlg"
        Link.Check
        Dateiname.SetText ConvertPath (gTesttoolPath + "global\input\graf_inp\desp.bmp")
        Oeffnen.Click
        sleep 2
        kontext "Messagebox"
        if Messagebox.Exists then
            Warnlog Messagebox.GetText
            Messagebox.OK
            sleep 1
        end if
    catch
        Warnlog "Insert graphic caused errors"
    endcatch

    WaitSlot (2000)
    try
        EditLinksDraw
        WaitSlot (2000)
        Kontext "VerknuepfungenBearbeiten"
        Call DialogTest ( VerknuepfungenBearbeiten )
        VerknuepfungenBearbeiten.Close
        WaitSlot (1000)
    catch
        Warnlog "- EditLinks could not be executed, could be the graphic was not imported"
    endcatch

    Call hCloseDocument
endcase

testcase tiEditImageMap
    Call  hNewDocument

    EditImageMap
    Kontext "ImageMapEditor"
    sleep (1)
    if ImageMapEditor.Exists (2) then
        printlog "- ImageMap exists"
        DialogTest ( ImageMapEditor )
        try
            ImageMapEditor.Close
            Printlog "ImageMap closed using the close button"
        catch
            EditImageMap
            Printlog "ImageMap closed using menue 'edit-imagemap'"
        endcatch
    else
        warnlog "ImageMap didn't come up!"
    end if
    Call  hCloseDocument
endcase

testcase tiEditObjectProperties
    dim i as integer

    Call hNewDocument

    InsertFloatingFrame
    WaitSlot (2000)

    Kontext "TabEigenschaften"
    FrameName.SetText "Hello"
    Inhalt.SetText ConvertPath ( gTesttoolpath + "global\input\graf_inp\desp.bmp" )
    WaitSlot (2000)
    TabEigenschaften.OK
    WaitSlot (2000)
    gMouseDoubleClick 1,1

    hTypekeys "<tab>"

    kontext
    WaitSlot (2000)
    EditObjectProperties
    WaitSlot (1000)
    Kontext "TabEigenschaften"
    DialogTest ( TabEigenschaften )
    sleep(1)
    Oeffnen.Click
    Kontext "OeffnenDlg"
    Call DialogTest ( OeffnenDlg )
    OeffnenDlg.Cancel
    Kontext "TabEigenschaften"
    TabEigenschaften.Cancel

    Call hCloseDocument
endcase

testcase tiEditObjectEdit
    dim i as integer
    Call hNewDocument

    InsertObjectOLEObject
    WaitSlot (1000)
    Kontext "OLEObjektEinfuegen"
    ObjektTyp.Select 1
    OLEObjektEinfuegen.OK
    WaitSlot (1000)

    gMouseClick 20,1

    hTypekeys "<tab>"

    EditObjectEdit
    ' try EditObjectEdit again, to see, if it is in edit mode !
    WaitSlot (2000)
    try
        ContextNameObject
        warnlog " Couldn't get into edit mode!"
    catch
        printlog "Reached edit mode - ok :-)"
        gMouseClick 20,1
    endcatch

    EditSelectAll

    EditObjectSaveCopyAs
    Kontext "SpeichernDlg"
    Call DialogTest ( SpeichernDlg )
    SpeichernDlg.Cancel
    WaitSlot (2000)
    Kontext "Active"
    if Active.Exists(2) then Active.No
        Call hCloseDocument
endcase

testcase tiEditPlugIn
    Call hNewDocument

    InsertObjectPlugin
    Kontext "PlugInEinfuegen"
    '    DialogTest ( PlugInEinfuegen)
    Durchsuchen.click
    Kontext "OeffnenDlg"
    '      Call DialogTest ( OeffnenDlg )
    if OeffnenDlg.exists (5) then
        OeffnenDlg.Cancel
    else
        warnlog "Open file dialog didn't come up"
    end if
    WaitSlot (5000)
    Kontext "PlugInEinfuegen"
    if PlugInEinfuegen.exists then
        DateiUrl.SetText (ConvertPath ( gTesttoolpath + "graphics\required\input\sample.mov" ))

        Optionen.SetText "Fiddler's Green"
        Optionen.TypeKeys "<HOME>"
        Optionen.TypeKeys "<SHIFT><END>"
        Optionen.TypeKeys "<delete>"
        PlugInEinfuegen.Ok
    else
        warnlog "Insert plugin isn't visible"
    end if
    WaitSlot (5000)
    kontext "Messagebox"
    if Messagebox.exists (5) then
        warnlog "Messagebox: " + Messagebox.gettext
        Messagebox.ok
    end if
    EditPlugIn
    printlog "Editplugin works!"

    Call hCloseDocument
endcase

testcase tiEditHyperlink
    hNewDocument
    InsertHyperlink
    WaitSlot (5000)
    Kontext "Hyperlink"
    Auswahl.MouseDown 50, 5
    Auswahl.MouseUp 50, 5
    Auswahl.typekeys "<PAGEDOWN><PAGEUP>"
    Auswahl.typekeys "<TAB>"
    sleep 3
    Kontext "TabHyperlinkInternet"

    'Workaround to get rid of a Focusing-problem...
    NameText.Typekeys "alal <RETURN>"
    NameText.Typekeys "<MOD1 A><DELETE>"
    TabHyperlinkInternet.Typekeys "<TAB>", 6
    TabHyperlinkInternet.Typekeys "<LEFT>", 3
    'End of workaround...

    Internet.Check           'Just to make sure the radio-button is addressable.
    ZielUrl.Settext "http://www.liegerad-fahrer.de"
    Kontext "Hyperlink"
    Uebernehmen.Click
    Hyperlink.Close
    hTypeKeys "<TAB><F2>"
    EditSelectAll
    try
        EditHyperlinkDraw
        Kontext "Hyperlink"
        if Hyperlink.Exists then
            Hyperlink.Close
        else
            Warnlog "- Hyperlinkdialog not up"
        end if
    catch
        Warnlog "- Not able to edit Hyperlink!"
    endcatch

    Call hCloseDocument
endcase
