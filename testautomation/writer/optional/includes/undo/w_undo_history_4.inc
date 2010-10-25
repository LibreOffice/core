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
'* short description : Writer Undo-History - Test
'*
'\***********************************************************************

sub w_undo_history_4

    Call tUndoOLEObject			' Objects (Insert/Delete)
    Call tUndoFloatingFrame			' Floating Frame (Insert/Delete)
    Call tUndoInsertFile			' Insert file
    Call tUndoAsianPhoneticGuides			' Asian Phonetic Guides
    Call tUndoAutocorrect			' Autocorrect / Autoformat
    Call tUndoDrawObject			' Draw Object (Insert)
    Call tUndoDrawObjectGroup			' Draw Object (Group/ungroup/Name/Delete)
    Call tUndoSpellcheck			' Spellcheck (Autocheck/Spellcheck)
    Call tUndoFormControl			' Form Control (Insert/resize/move/replace/change property/delete)
    Call tUndoHeaderFooter			' Header/Footer insert and delete

end sub

testcase tUndoOLEObject

    '/// <B>OLE-Objects (Insert/Delete)</B>
    printlog "- Insert OLE-Object"
    '/// new Document
    Call hNewDocument
    '/// Insert / Object / OLE-Object ///
    InsertObjectOLEObject
    Kontext "OLEObjektEinfuegen"
    Objekttyp.Select 1
    OLEObjektEinfuegen.Ok
    wait 500
    Call wTypeKeys "<Esc>"
    wait 500
    Call gMouseClick(15,5)
    wait 500
    '///  string in Undo-list has to be: "Insert object")
    Call CheckUndoStringInUndoList ( 10, "InsertObject" )

    printlog "- delete object"
    '/// Select object with <Shift F4>
    Call wTypeKeys "<Shift F4>"
    wait 500
    '/// <B>Delete object</B>
    Call wTypeKeys "<Delete>"
    wait 500
    '/// press 'delete' to remove object
    '///  string in Undo-list has to be: "Delete object")
    Call CheckUndoStringInUndoList ( 10, "DeleteObject" )

    '/// close Document
    Call hCloseDocument
endcase


testcase tUndoFloatingFrame

    '/// <B>Floating frames</B>
    printlog "- Insert floating frame"
    '/// new document
    Call hNewDocument
    '/// Insert / Floating frame
    InsertFloatingFrame
    Kontext "TabEigenschaften"
    '/// Choose a file as floating frame
    Inhalt.Settext Convertpath (gTesttoolPath & "writer\optional\input\graphics\jolink.jpg" )
    TabEigenschaften.Ok
    Sleep 2
    '///  string in Undo-list has to be: "Insert floating frame")
    Call CheckUndoStringInUndoList ( 10, "InsertFloatingFrame" )

    '/// Delete floating frame
    printlog "- delete floating frame"
    Call wTypeKeys "<Delete>"
    '///  string in Undo-list has to be: "Delete floating frame")
    Call CheckUndoStringInUndoList ( 10, "DeleteFloatingFrame" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoInsertFile

    '/// <B>Insert File</B>
    printlog "- Insert file"
    '/// new document
    Call hNewDocument
    '/// Insert / File
    InsertFileWriter
    Sleep 3
    Kontext "OeffnenDlg"
    Sleep 2
    '/// Choose "..\\writer\\input\\htmltest.sxw"
    if OeffnenDlg.Exists then
        Dateiname.Settext Convertpath (gTesttoolPath & "writer\optional\input\undo\htmltest.sxw" )
        Oeffnen.Click
    else
        Warnlog "File open dialog missing"
        Call hCloseDocument
        goto endsub
    end if

    '///  string in Undo-list has to be: "Insert document")
    Call CheckUndoStringInUndoList ( 10, "InsertDocument" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoAsianPhoneticGuides

    '/// <B>Asian Phonetic Guides</B>
    printlog "- Asian Phonetic Guides"
    '/// new document
    Call hNewDocument
    '/// type some text and select it
    Call wTypeKeys ( "JuppHeidiUndJuppHeida" )
    Call wTypeKeys ( "<Home>" )
    '/// Check if Asian support is enabled, if not enable it
    if gAsianSup = true then
        FormatRuby
        Kontext "RubyDialog"
        RubyText1.Settext "SeppHeidiUndSeppHeida"
        RubyDialog.Ok
        RubyDialog.Close
    else
        Call ActiveDeactivateAsianSupport(True)
        FormatRuby
        Kontext "RubyDialog"
        RubyText1.Settext "SeppHeidiUndSeppHeida"
        RubyDialog.Ok
        RubyDialog.Close
        Call ActiveDeactivateAsianSupport(False)
    end if

    '///  string in Undo-list has to be: "Asian Phonectic Guides Setting")
    Call CheckUndoStringInUndoList ( 10, "APGS" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoAutocorrect

    '/// <B>Autocorrect / Autoformat</B>
    printlog "- Autocorrect / Autoformat"
    '/// new document
    Call hNewDocument
    '/// <B>Autocorrect</B>
    printlog "- Autocorrect"
    '/// type with quote and blank at the end 'www.sun.com '
    Call wTypeKeys ( "www.sun.com " )
    '///  string in Undo-list has to be: "AutoCorrect")
    Call CheckUndoStringInUndoList ( 10, "AutoCorrect" )

    '/// insert a line break
    Call wTypeKeys ( "<Return>" )
    '/// <B>Autoformat</B>
    printlog "- Autoformat"
    '/// type - AutoFormat test<Return>
    Call wTypeKeys ( "- AutoFormat test<Return>" )
    '///  string in Undo-list has to be: "AutoCorrect")
    Call CheckUndoStringInUndoList ( 10, "AutoFormat" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoDrawObject
    '/// <B>Draw object</B>
    printlog "- Draw object"
    '/// new document
    Call hNewDocument
    '/// Insert a freeform line object

    ViewZoom
    Kontext "Massstab"
    Optimal.Check
    Massstab.OK

    Call hToolbarSelect ("Drawing", true)
    Kontext "Drawbar"
    printlog "  - Draw Freeform"
    Freihand.Click
    Call gMouseMove ( 33, 40, 55, 51 )
    Call gMouseMove ( 35, 31, 40, 90 )
    Call gMouseMove ( 41, 91, 55 , 70 )
    Call gMouseClick( 55, 70 )
    Call wTypeKeys ("<Escape>")

    '///  string in Undo-list has to be: "Insert draw object: freeform line")
    Call CheckUndoStringInUndoList ( 10, "FreeForm" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoDrawObjectGroup

    '/// <B>Group of Drawing objects</B>
    printlog "- Group of Drawing objects"
    '/// new document ///
    Call hNewDocument
    ViewZoom
    Kontext "Massstab"
    Optimal.Check
    Massstab.OK
    Sleep 1
    '/// Insert 2 drawing objects (rectangle) ///
    Call wZeichenobjektEinfuegen ( "Rechteck", 40, 50, 60, 60 )
    Call wTypeKeys "<ESCAPE>"
    Call wTypeKeys "<ESCAPE>"
    Call wZeichenobjektEinfuegen ( "Rechteck", 40, 55, 50, 65 )
    '/// Select both objects ///
    Call wObjektSelektieren ( 35, 35, 70, 70 )
    '/// Select Format / Group / Group ///
    try
        printlog "- group objects"
        FormatGroupGroup
        Sleep 1
        '///  string in Undo-list has to be: "Group draw objects")
        Call CheckUndoStringInUndoList ( 10, "DrawGroup" )

        '/// Format / Name object
        printlog "- name objects"
        FormatNameObject
        Kontext "NameDlgObject"
        NameField.Settext "MyGroup"
        NameDlgObject.OK
        '///  string in Undo-list has to be: "Rename 2 Rectangles")
        Call CheckUndoStringInUndoList ( 10, "RenameGroup" )

        '/// Select Format / Group / Ungroup ///
        printlog "- ungroup objects"
        FormatUngroupDraw
        '///  string in Undo-list has to be: "Ungroup group object")
        Call CheckUndoStringInUndoList ( 10, "DrawUnGroup" )

        '/// Select 1 object and delete it
        Call gMouseClick ( 40, 52 )
        printlog "- delete object"
        Call wTypeKeys "<Delete>"
        '///  string in Undo-list has to be: "Delete 2 Rectangles")
        Call CheckUndoStringInUndoList ( 10, "DeleteGroup" )
    catch
        Warnlog "Unable to execute 'Format / Group / Group' -> disabled!"
    endcatch

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoSpellcheck
    Dim sReplace as string

    '/// <B>Spellcheck(Autocheck/Spellcheck)</B>
    printlog "- Spellcheck: Autocheck"
    '/// new document ///
    Call hfileOpenLocally (gTesttoolpath + "writer\optional\input\undo\spellcheck_undo.odt")
    
    '/// Type "This is a testp<End>"
    Call wTypeKeys ( "<End>" )  'This is a testp
    '/// Select word' testp'
    Call wTypeKeys ( "<Mod1 Shift Left>" )
    '/// Tools / Spellcheck
    ToolsSpellcheckWriter
    Kontext "Spellcheck"
    Suggestions.Select 1
    sReplace = Suggestions.GetSelText
    '///Replace with 1st word in suggestion list
    printlog "-> Replace with word: " & sReplace
    Change.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Ok
        end if
    end if
    Kontext "Spellcheck"
    if Spellcheck.Exists then Spellcheck.Close
    '///  string in Undo-list has to be: "Replace: 'Testp' -> 'Test'")
    Call CheckUndoStringInUndoList ( 10, "Autocheck/Spellcheck" )

    '/// Uncheck 'Autospellcheck' in Toolbar ///
    ToolsSpellcheckAutoSpellcheck

    printlog "- Spellcheck"
    Call wTypeKeys ( "<End><Return>", 2 )
    '/// retype 'testp'
    Call wTypeKeys ( "testp<Shift Home>" )
    ToolsSpellcheckWriter
    Kontext "Spellcheck"
    ' Workaround: For some reasons sometimes dialog is disabled here
    ' Only reproducable by testtool
    try
        Suggestions.Select 1
    catch
        Spellcheck.Close
        ToolsSpellcheckWriter
        Kontext "Spellcheck"
        Suggestions.Select 1
    endcatch
    Change.Click
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            try
                Active.No
            catch
                Active.Ok
            endcatch
        end if
    end if
    Kontext "Spellcheck"
    if Spellcheck.Exists then Spellcheck.Close

    '///  string in Undo-list has to be: "Replace: 'Testp' -> 'Test'")
    Call CheckUndoStringInUndoList ( 10, "Autocheck/Spellcheck" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoFormControl

    '/// <B>Form Control (Insert/resize/move/replace/change property/delete)</B>
    printlog "- Form control (insert)"
    '/// new document
    Call hNewDocument
    '/// Open 'Form' Toolbox ///
    Call hToolbarSelect ("FormControls", true)
    'Formsbar.Move 20, 30
    '/// Insert a pushbutton from toolbox ///
    Pushbutton.Click
    Kontext "DocumentWriter"
    DocumentWriter.MouseDown ( 40, 50 )
    DocumentWriter.MouseMove ( 50, 55 )
    DocumentWriter.MouseUp   ( 50, 55 )
    Sleep 1

    '///  string in Undo-list has to be: "Insert control")
    Call CheckUndoStringInUndoList ( 10, "ControlInsert" )

    printlog "- Form control (move)"
    '/// Press '<Up>' to move control
    Call wTypeKeys ( "<Up>" )
    '///  string in Undo-list has to be: "Move control")
    Call CheckUndoStringInUndoList ( 10, "ControlMove" )

    printlog "- Form control (resize)"
    '/// Press '<Ctrl+Tab>' to enter handle selection mode
    Call wTypeKeys ( "<Mod1 Tab>" )
    '/// Press '<Up>' to resize control
    Call wTypeKeys ( "<Up>" )
    '///  string in Undo-list has to be: "Resize control")
    Call CheckUndoStringInUndoList ( 10, "ControlResize" )

    printlog "- Form control (replace)"
    '/// Select 'Format->Replace with-> Text box'
    FormatReplaceWithTextBox
    wait 500
    '///  string in Undo-list has to be: "Replace control")
    Call CheckUndoStringInUndoList ( 10, "ControlReplace" )

    printlog "- Form control (change property)"
    '/// Select 'Format->Control' and change name of control
    FormatControl
    Kontext "ControlPropertiesTabControl"
    ControlPropertiesTabControl.setPage TabGeneralControl
    Kontext "TabGeneralControl"
    NameText.Settext "MyControl"
    FormatControl
    '///  string in Undo-list has to be: "Set property 'Name'")
    Call CheckUndoStringInUndoList ( 10, "ControlChange" )

    printlog "- Form control (delete)"
    '/// Press '<Delete>' to delete control
    Call wTypeKeys ( "<Delete>" )
    '///  string in Undo-list has to be: "Delete control")
    Call CheckUndoStringInUndoList ( 10, "ControlDelete" )

    '/// close document
    Call hCloseDocument
endcase


testcase tUndoHeaderFooter

    QAErrorlog "#i56552#footer deletion cannot be undone"
    goto endsub
    '/// Insert header
    printlog "- Insert header"
    '/// new Document
    Call hNewDocument
    '/// Insert / Header / Default
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(9))
    Sleep 2
    MenuSelect(Menugetitemid(1))
    Sleep 2
    '///  string in Undo-list has to be: "Header/footer changed")
    Call CheckUndoStringInUndoList ( 10, "Header/Footer" )

    '///  Delete header
    printlog "- Delete header"
    Kontext "DocumentWriter"
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(9))
    Sleep 2
    MenuSelect(Menugetitemid(1))
    Sleep 2
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Yes
        end if
    end if
    '///  string in Undo-list has to be: "Header/footer changed")
    Call CheckUndoStringInUndoList ( 10, "Header/Footer" )

    '/// Insert footer
    printlog "- Insert footer"
    '/// Insert / Header / Default
    Kontext "DocumentWriter"
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(10))
    Sleep 2
    MenuSelect(Menugetitemid(1))
    Sleep 2
    '///  string in Undo-list has to be: "Header/footer changed")
    Call CheckUndoStringInUndoList ( 10, "Header/Footer" )

    '///  Delete footer
    printlog "- Delete footer"
    Kontext "DocumentWriter"
    DocumentWriter.UseMenu
    MenuSelect(Menugetitemid(4))
    Sleep 2
    MenuSelect(Menugetitemid(10))
    Sleep 2
    MenuSelect(Menugetitemid(1))
    Sleep 2
    Kontext "Active"
    if Active.Exists then
        if Active.GetRT = 304 then
            Active.Yes
        end if
    end if
    '///  string in Undo-list has to be: "Header/footer changed")
    Call CheckUndoStringInUndoList ( 10, "Header/Footer" )

    '/// close document
    Call hCloseDocument
endcase
