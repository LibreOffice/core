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
'* short description : Functions for HTML-Test
'*
'************************************************************************
'*
' #0 wPasteAvailableClipboardFormats(TheNumber as integer
' #0 wSetClipboardtestDefaults(ForWhat as string) as boolean
' #0 wInsertNewCalcSheet(SheetName as string) as booloean
' #0 wRenameCalcSheet(NewName as string) as boolean
' #0 wFilterSpecialCharacters(ToFilter as string) as string
' #0 ReplaceCharacter(stringToChange$
' #0 wChangeHTMLCompatibility ( optional RecentCompatibility as integer ) as integer
'*
'\***********************************************************************

function wPasteAvailableClipboardFormats(TheNumber as integer, CheckWhat as String )
    Dim i as integer, ClipboardFormat as string
    For i = 1 to TheNumber
        try
            Auswahl.Select i
        catch
            if i <= TheNumber then
                QAErrorlog "Number of clipboard formats seems to be changed!"
            end if
            exit for
        endcatch
        ClipboardFormat = Auswahl.GetSeltext
        printlog "- Paste as: " + ClipboardFormat
        Select Case CheckWhat
            Case "ctext", "DRAW"
                if lcase(gPlatform) = "sol" and lcase(ClipboardFormat) = "bitmap" then
                    QAErrorlog "#i49505#Paste drawing object as bitmap crashes office"
                    goto s_next_item
                else
                    InhaltEinfuegen.Ok
                    Sleep 3
                end if
            Case else
                InhaltEinfuegen.Ok
                Sleep 3
        end select

        Select Case gApplication
            Case "CALC"
                Kontext "TextImport"
                if TextImport.Exists then TextImport.Ok
        end select
        
        Kontext "Active"
        if Active.Exists then
            QAErrorlog " - " + Active.Gettext + "->Bug#110181"
            Active.Ok
        end if
        hSetDocumentContext()
        Call wTypeKeys "<Escape>",2
        Select Case gApplication
            Case "WRITER","MASTERDOCUMENT","HTML"
                Select Case CheckWhat
                    Case "text","field","table"
                    Call wTypeKeys "<Down><End><Return>"
                    Call wTypeKeys "(" + Clipboardformat + ")"
                    Call wTypeKeys "<End><Return>",2
            Case "frame", "DRAW", "graphicL", "graphicE", "ole", "control", "ctext"
                'Call gMouseClick (50,100)
                Call wTypeKeys ("<Escape>")
                Call wTypeKeys "(" + Clipboardformat + ")"
                if i < TheNumber Then
                    if gApplication <> "HTML" then
                        InsertManualBreak
                        Kontext "UmbruchEinfuegen"
                        Seitenumbruch.Check
                        UmbruchEinfuegen.OK
                    else
                        Call wTypeKeys ("<Return>" , 2)
                    end if
                end if
        end select
        EditPasteSpecialWriter

        Case "IMPRESS","DRAW"
            Call gMouseClick(7,7)
            Call wRenameImpressSlide(ClipboardFormat)
            if i < TheNumber Then
                Call wInsertNewImpressSlide
                EditPasteSpecial
            end if
        Case "CALC"
            printlog " Rename first sheet"
            if wRenameCalcSheet(Clipboardformat) = False then
                Warnlog "Unable to rename Sheet Name !"
            end if
            if i < TheNumber Then
                if wInsertNewCalcSheet(Clipboardformat) = False then
                    Warnlog "Unable to set Sheetname : " + Clipboardformat
                end if
                EditPasteSpecialCalc

            end if
        end select
        s_next_item:
        Kontext "InhaltEinfuegen"
    next i
    kontext "NavigatorDraw"
    if NavigatorDraw.Exists then NavigatorDraw.Close
    kontext "Navigator"
    if Navigator.Exists then Navigator.Close

    kontext "InhaltEinfuegen"
    f_exit:
    if InhaltEinfuegen.Exists then InhaltEinfuegen.Cancel
end function

' ---------------------------------------------------------------------------------

function wSetClipboardtestDefaults(ForWhat as string) as boolean
    printlog " Points cursor to beginning of document "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\writer.sxw")
    
    Kontext "DocumentWriter"
    printlog " Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    printlog " Check if beginning of document reached "
    Call wTypeKeys "<Mod1 Shift Right>"
    EditCopy

    Select Case ForWhat
        Case "text"
        '"+ Select first paragraph "
        Call wTypeKeys "<Shift End>"
        Call wTypeKeys "<Shift Down>"
        '"+ Copy selected text "
        
        Case "field"
        '"+ Select paragraph with 'Date Field' "
        Call wTypeKeys "<Down>",3
        Call wTypeKeys "<Home><Shift End>"
        '"+ Copy selected text "
        
        Case "table"
        '"+ Select paragraph with 'Table' "
        Call wNavigatorAuswahl(2,1)
        'Call wTypeKeys "<Down>",6
        Call wTypeKeys "<Mod1 A>",2
        '"+ Copy selected table "
        
        Case "frame"
        '"+ Select 'Frame' "
        Call wTypeKeys ( "<Shift F4>" )
        '"+ Copy selected frame "
        
        Case "DRAW"
        '"+ Select 'Drawing Object' "
        Call wTypeKeys ( "<Shift F4>" )
        Call wTypeKeys "<Tab>"
        '"+ Copy selected Drawing Object "
        
        Case "graphicL"
        '"+ Select 'Linked Graphic' "
        Call wTypeKeys ( "<Shift F4>" )
        Call wTypeKeys "<Tab>",2
        '"+ Copy selected Linked Graphic "
        
        Case "graphicE"
        '"+ Select 'Embedded Graphic' "
        Call wTypeKeys ( "<Shift F4>" )
        Call wTypeKeys "<Tab>",3
        '"+ Copy selected Embedded Graphic "
        
        Case "ole"
        '"+ Select 'OLE Object' "
        Call wTypeKeys ( "<Shift F4>" )
        Call wTypeKeys "<Tab>",4
        '"+ Copy selected OLE Object "
        
        Case "control"
        '"+ Select 'Control' "
        Call wTypeKeys ( "<Shift F4>" )
        Call wTypeKeys "<Tab>",5
        '"+ Copy selected Control "
        
        Case else
            Warnlog "Unknown object!"
    end select

    try
        EditCopy
        EditCopy 'and a second time to make sure..
        wSetClipboardtestDefaults = True
    catch
        QAErrorlog "Error jump to beginning of document!"
        wSetClipboardtestDefaults = False
    endcatch
    
    ' Because of Clipboard bug set
    wSetClipboardtestDefaults = True
end function

' ---------------------------------------------------------------------------------

function wInsertNewCalcSheet(SheetName as string) as boolean
    SheetName= wFilterSpecialCharacters(SheetName)
    printlog " Inserts a new shett and sets the name for it "
    InsertSheetCalc
    Kontext "TabelleEinfuegenCalc"
    if TabelleEinfuegenCalc.Exists then
        Nach.Check
        printlog " Check 'After current sheet' "
        NeuErstellen.Check
        printlog " Check 'New Sheet' "
        'Tabellenname.Settext SheetName
        printlog " Set Name of sheet "
        TabelleEinfuegenCalc.Ok
        printlog " Unable to set name of Sheet ? "
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Warnlog Active.Gettext
                Active.Ok
                Kontext "TabelleEinfuegenCalc"
                if TabelleEinfuegenCalc.Exists then TabelleEinfuegenCalc.Cancel
                wInsertNewCalcSheet = False
            else
                wInsertNewCalcSheet = True
            end if
        else
            wInsertNewCalcSheet = True
        end if
    else
        Warnlog "Dialog 'Insert Sheet' not up!"
        wInsertNewCalcSheet = False
    end if
end function

' ---------------------------------------------------------------------------------

function wRenameCalcSheet(NewName as string) as boolean
    printlog " Renames an existing sheet in calc "
    FormatSheetRename
    Kontext "TabelleUmbenennen"
    if TabelleUmbenennen.Exists then
        TabellenName.Settext wFilterSpecialCharacters(NewName)
        TabelleUmbenennen.Ok
        Kontext "Active"
        if Active.Exists then
            if Active.GetRT = 304 then
                Active.Ok
                Kontext "TabelleUmbenennen"
                if TabelleUmbenennen.Exists then TabelleUmbenennen.Cancel
                wRenameCalcSheet = False
            else
                wRenameCalcSheet = True
            end if
        else
            wRenameCalcSheet = True
        end if
    else
        wRenameCalcSheet = False
    end if
end function

' ---------------------------------------------------------------------------------

sub wInsertNewImpressSlide()
    InsertSlide
end sub

' ---------------------------------------------------------------------------------

sub wRenameImpressSlide(NewName as string)
    printlog " Edit->Layer->Rename "'
    try
        EditRenameSlide
        Kontext "NameDlgPage"
        if NameDlgPage.Exists then
            NameField.Settext NewName
            NameDlgPage.Ok
        else
            try
                Kontext "DocumentDrawImpress"
                TabBar.TypeKeys NewName + "<Return>" , true
            catch
                Warnlog "Unable to rename Slide (No access to to Tab-Bar!)"
            endcatch
        end if
    catch
        Warnlog "Unable to rename Slide!"
    endcatch
    
end sub

' ---------------------------------------------------------------------------------

function wFilterSpecialCharacters(ToFilter as string) as string
     Dim i as integer, SpecialCharacters as string
     SpecialCharacters = "!$%&/()=?\}][{*+~'#;,:.-"
     printlog " Replace SpecialCharacters in SheetName with an underscore (_) "
     For i = 1 to len(SpecialCharacters)
        ToFilter = ReplaceCharacter(ToFilter,Mid$(SpecialCharacters,i,1),"_")
     next i
     wFilterSpecialCharacters = ToFilter
end function

' ---------------------------------------------------------------------------------

function ReplaceCharacter(stringToChange$, charToReplace$, replaceWith$) As String

    'Replaces a specified character in a string with another character that you specify
    Dim ln As Long
    Dim n As Long
    Dim NextLetter As String
    Dim FinalString As String
    Dim txt As String
    Dim char As String
    Dim rep As String
    txt = stringToChange$ 'store all arguments in
    char = charToReplace$ 'new variables
    rep = replaceWith$

    ln = Len(txt)

    For n = 1 To ln Step 1
        NextLetter = Mid(txt, n, 1)
        If NextLetter = char Then
            NextLetter = rep
        End If
        FinalString = FinalString & NextLetter
    Next n
    ReplaceCharacter = FinalString

end function

' ---------------------------------------------------------------------------------

Sub wDisableImpressAutopilot()

    gApplication = "IMPRESS"
    Call hNewDocument
    ToolsOptions
    Call hToolsOptions ("IMPRESS","General")
    MitAutopilotStarten.UnCheck
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Call hCloseDocument

end sub

function wChangeHTMLCompatibility ( optional RecentCompatibility as integer ) as integer
    Dim i as integer
    Dim CurrentCharSet as String
    Dim RecentCharSet as integer 
    Dim CharsetFound as boolean
    printlog " This function sets the charset in options to UTF-8 "
    printlog " Giving a parameter a special charset will be chosen "
    
    CharsetFound = False
    ToolsOptions
    Call hToolsOptions("LOADSAVE", "HTMLCOMPATIBILITY")
    if IsMissing(RecentCompatibility) = True then
        RecentCharSet = Zeichensatz.GetSelIndex
        For i = 1 to Zeichensatz.GetItemCount
            Zeichensatz.Select i
            CurrentCharset = Zeichensatz.GetSelText
            if Instr(Ucase(CurrentCharset), "UTF-8") then
                i = Zeichensatz.GetItemCount + 1
                CharsetFound = True
            end if
        next i
    else
        CharsetFound = True
        RecentCharSet = RecentCompatibility
        Zeichensatz.Select RecentCompatibility
    end if
    if CharsetFound = True then
        printlog "Charset has been changed!"
    else
        Warnlog "Couldn't set Charset to UTF-8!"
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    wChangeHTMLCompatibility = RecentCharset

end function

' ---------------------------------------------------------------------------------

function wChangeHTMLCompatibilityExport ( optional wExport as integer ) as integer
    Dim i as integer
    Dim CurrentExportSet as String
    Dim RecentExportSet as integer
    Dim ExportFound as boolean
    printlog " This function sets the export in options to 'Oracle Open Office Writer' "
    printlog " Giving a parameter a special export will be chosen "
    
    ExportFound = False
    ToolsOptions
    Call hToolsOptions("LOADSAVE", "HTMLCOMPATIBILITY")
    if IsMissing ( wExport ) = True then
        RecentExportSet = Export.GetSelIndex
        For i = 1 to Export.GetItemCount
            Export.Select i
            CurrentExportset = Export.GetSelText
            if Instr(Ucase(CurrentExportset), "Oracle Open Office Writer") then
                i = Export.GetItemCount + 1
                ExportFound = True
            end if
        next i
    else
        ExportFound = True
        RecentExportSet = wExport
        Export.Select RecentExportSet
    end if
    if ExportFound = True then
        printlog "Export has been changed!"
    else
        Warnlog "Couldn't set Export to Oracle Open Office Writer!"
    end if
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    wChangeHTMLCompatibilityExport = RecentExportSet
end function
