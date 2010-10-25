'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*
'* Owner : wolfram.garten@oracle.com
'*
'* short description : some tools (Subs)
'*
'\******************************************************************************

sub sFileExport

    printlog " just exporting is done in qatesttool/framework/first test: 'tGraphicExport' but there is no loading, "
    printlog "+ of the created files and the items on the dialogs are not checked completely "
    Dim ExZaehler as Integer
    Dim ExPath as String
    Dim Liste( 50 ) as String

    if (gApplication = "IMPRESS") then
        ExtensionString = "odp"
    else
        ExtensionString = "odg"
    end if

    printlog "- all files are saved in [StarOfficePath]/user/work/[application]/export "
    ExPath = ConvertPath (gOfficePath + "user\work\" + gApplication + "\export\" )
    OutputGrafikTBO = ExPath & "expo"
    Printlog "Create the export-dir for the graphics (  + ExPath + )"
    try
        app.mkDir ( ExPath )
        ExZaehler = GetFileList ( ExPath , "*.*" , Liste() )
        if ExZaehler <> 0 then
            Printlog "The export-dir exists. The test want to delete all Files ( " + ExZaehler + " )!"
            if KillFileList ( Liste() ) = FALSE then
                Warnlog "Not all files can be deleted. " + ListCount ( Liste() ) + " files exists!"
            end if
        end if
    catch
        Warnlog "An error at creating the export-dir, the test ends!"
        exit sub
    endcatch

    printlog "+ open the test document qatesttool/graphics/required/input/graphicexport.od ? ] "

end sub

'-------------------------------------------------------------------------
sub callAutocorrectOptions

    ToolsAutocorrect
    Kontext
    active.SetPage TabOptionen
    Kontext "TabOptionen"
end sub

'-------------------------------------------------------------------------------
sub sCheckCheck (i, Pruefung$, bEnabled)

    hTextrahmenErstellen  (Pruefung$,20,20,60,40)

    select case i
        ' Disabled sCheckUnderlined due to start of external program (web-browser) - FHA
    case 5: bEnabled 'sCheckUnderlined (bEnabled)
    case 6: sCheckDash (bEnabled)
    case 8: sCheckSupperscript (bEnabled)
    case else:
        hTypeKeys "<Home><Shift End>"
        EditCopy
        if (GetClipboardText = Pruefung$) then ' not replaced
            if bEnabled then  ' not as expected
                warnlog "-    replacement failed"
            endif
        else     ' replaced
            if not bEnabled then  ' not as expected
                warnlog "-    replacement failed : '" + Pruefung$ + "' - '" + GetClipboardText + "'"
            endif
        endif
    end select

    hTypeKeys "<Home><Shift End><Delete>"
end sub

'-------------------------------------------------------------------------------
sub sCheckUnderlined (bEnabled)

    dim btemp as boolean
    hTypeKeys "<End><Left><Shift Left>"
    try
        ContextOpenHyperlink
        btemp = true
    catch
        btemp = false
    endcatch
    if (bEnabled <> btemp) then
        warnlog "-    replacement failed"
    endif
    kontext
    if active.exists(5) then
        active.ok
    endif
end sub

'-------------------------------------------------------------------------------
sub sCheckSupperscript (bEnabled)

    hTypeKeys "<Home><Right><Right>"
    FormatCharacter
    Kontext
    Active.SetPage TabFontPosition
    Kontext "TabFontPosition"
    if (bEnabled <> Superscript.IsChecked) then
        warnlog "-    replacement failed"
    endif
    TabFontPosition.OK
end sub

'-------------------------------------------------------------------------------
sub sCheckDash (bEnabled)

    ' inserted is 45
    ' en dash is 8211 / alt + 0150
    ' em dash is 8212 / alt + 0151 ' which doen't work atm
    dim sTemp as string
    hTypeKeys "<End><Left><Left><Left><Left><Left><Left><Left><Left><Shift Left>"
    EditCopy
    sTemp = GetClipboard
    if ((asc(sTemp) <> 45) <> bEnabled) then
        warnlog "-    replacement failed : " + bEnabled + " : " + asc(sTemp)
    endif
end sub

'-------------------------------------------------------------------------------
sub sPrintCheckOrder (optional bcheck as boolean)

    dim sTemp as string
    dim sTemp2 as string
    dim i as integer
    printlog " deselect all "
    Printlog "-----------------------------------"
    printlog " select in default order and take Position X in mind ;-) "
    hTypeKeys ("<escape><escape>")
    for i = 1 to 3
        hTypeKeys ("<TAB>")
        sTemp = fGetPositionX()
        Printlog " - " + i +": " + sTemp
        if ((isMissing(bcheck) <> FALSE) AND (bcheck = TRUE)) then
            Select Case i
            Case 1:  sTemp2 = Ueber_Text_1
            Case 2:  sTemp2 = Ueber_Text_2
            Case 3:  sTemp2 = Ueber_Text_3
            End Select
            if sTemp <> sTemp2 then
                warnlog " + " + i + " Arrangement is wrong; is: "+sTemp+"; should: "+sTemp2+";"
            end if
        endif
    next i
    hTypeKeys ("<escape><escape>")
    Printlog "-----------------------------------"
end sub

'-------------------------------------------------------------------------------
sub Select_Copy

    hUseAsyncSlot( "EditSelectAll" )
    hUseAsyncSlot( "EditCopy" )

end Sub

'-------------------------------------------------------------------------------
sub SaveMeasurementSetFirst

    select case( gApplication )
    case "DRAW"    : sApplication = "DRAWING"
    case "IMPRESS" : sApplication = "IMPRESS"
    case "WRITER"  : sApplication = "WRITER"
    case "CALC"    : sApplication = "CALC"
    case else      : printlog( "Provided gApplication is not supported: " & gApplication )
    end select

    printlog " - save states "
    ToolsOptions
    hToolsOptions (sApplication,"General")
    ReferenceOld = Masseinheit.GetSelText
    Masseinheit.TypeKeys= "<HOME>" '(first entry)
    ReferenceNew = Masseinheit.GetSelText
    Kontext "ExtrasOptionenDlg"
    hCloseDialog( ExtrasOptionenDlg, "ok" )

end Sub

'-------------------------------------------------------------------------------
sub MeasurementSetFirst

    dim f as integer

    select case( gApplication )
    case "DRAW"    : sApplication = "DRAWING"
    case "IMPRESS" : sApplication = "IMPRESS"
    case "WRITER"  : sApplication = "WRITER"
    case "CALC"    : sApplication = "CALC"
    case else      : printlog( "Provided gApplication is not supported: " & gApplication )
    end select

    ToolsOptions
    hToolsOptions (sApplication,"General")
    if Masseinheit.GetSelText <> ReferenceNew then 'find the right one.
        Masseinheit.TypeKeys "<HOME>"
        for f = 1 to Masseinheit.GetItemCount
            if Masseinheit.GetSelText = ReferenceNew then
                i = Masseinheit.GetItemCount 'find the right one.
            else
                Masseinheit.TypeKeys "<DOWN>"
            endif
        next f
    endif
    Kontext "ExtrasOptionenDlg"
    hCloseDialog( ExtrasOptionenDlg, "ok" )

end Sub

'-------------------------------------------------------------------------------
sub ResetMeasurement

    dim f as integer

    select case( gApplication )
    case "DRAW"    : sApplication = "DRAWING"
    case "IMPRESS" : sApplication = "IMPRESS"
    case "WRITER"  : sApplication = "WRITER"
    case "CALC"    : sApplication = "CALC"
    case else      : printlog( "Provided gApplication is not supported: " & gApplication )
    end select

    printlog " - Reset states back to what they were before "
    ToolsOptions
    hToolsOptions (sApplication,"General")
    if Masseinheit.GetSelText <> ReferenceOld then 'find the right one.
        Masseinheit.TypeKeys "<HOME>"
        for f = 1 to Masseinheit.GetItemCount
            if Masseinheit.GetSelText = ReferenceOld then
                i = Masseinheit.GetItemCount 'find the right one.
            else
                Masseinheit.TypeKeys "<DOWN>"
            endif
        next f
    endif
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
end Sub

'-------------------------------------------------------------------------------
sub SetKontextApplication
    
    hSetDocumentContext() ' Global routine exists

end sub

'-------------------------------------------------------------------------------

sub hSetSpellHypLanguage
    printlog " select a language with a dictionary, used for spellcheck, thesaurus and hyphenation "
    dim sTrieit as string

    ' only for asian languages i need to set the default language for the current document  to 'English(USA)'
    ' in all other languages the default has a dictionary
    if (bAsianLan or (iSprache=55)) then
        printlog " Tools->Options "
        ToolsOptions
        printlog " select from section 'Language Settings' the item 'Languages' "
        hToolsOptions ("LANGUAGESETTINGS","LANGUAGES")
        printlog " check checkbox 'For the current document only' in section 'Default languages for document' "
        AktuellesDokument.Check
        printlog " If there is no Language defined in 'locale-file' (in same directory as this file is) be smart and select one that supports spellchecking "
        if (glLocale(4) = "") then
            Kontext "ExtrasOptionenDlg"
            printlog "+ cancel dialog 'Options - ' "
            ExtrasOptionenDlg.Cancel
            printlog "+ call the smart subroutine that tells you a valid language with an dictionary "
            sTrieit = hFindSpellHypLanguage
            printlog "+ Tools->Options "
            ToolsOptions
            printlog "+ select from section 'Language Settings' the item 'Languages' "
            hToolsOptions ("LANGUAGESETTINGS","LANGUAGES")
            printlog "+ check checkbox 'For the current document only' in section 'Default languages for document' "
            AktuellesDokument.Check
            printlog " if smart routine found something, select it in section 'Default languages for document' listbox 'Western' "
            printlog "+ (manual users just select a language that has an icon in front of it ('ABC' with a checkmark) "
            if (sTrieit <> "") then
                try
                    Westlich.Select sTrieit
                catch
                    Asiatisch.Select sTrieit
                endcatch
            else
                qaErrorLog "Sorry no spellbook found: id_tools.inc::hSetSpellHypLanguage"
            endif
        else
            printlog " if a Language is already defined in the textfile "
            printlog glLocale (4)
            try
                printlog " select it in section 'Default languages for document' listbox 'Western' "
                printlog "+ (manual users just select a language that has an icon in front of it ('ABC' with a checkmark) "
                try
                    Westlich.Select glLocale (4)
                catch
                    Asiatisch.Select glLocale (4)
                endcatch
            catch
                warnlog "this language is not available: '" + glLocale (4) + "'"
                dim qaw as string
                qaw = glLocale (4)
            endcatch
        endif
        try
            printlog "selected: '" + Westlich.GetSelText + "'"
        catch
            printlog "selected: '" + Asiatisch.GetSelText + "'"
        endcatch
        Kontext "ExtrasOptionenDlg"
        printlog "+ close dialog 'Options - ' with OK "
        ExtrasOptionenDlg.OK
    endif
end sub

'-------------------------------------------------------------------------------
sub hTBOtypeInDoc

    hRechteckErstellen ( 10, 10, 30, 40 )
end sub

'-------------------------------------------------------------------------------
sub Position_Vergleichen (Ueber_Text_1 as string,Ueber_Text_2 as string,Ueber_Text_3 as string)   ' Ueber_Text_1 : X-Position des Objektes

    dim Dummy_Text as string
    '------------------------------------------------------------  ' Ueber_Text_2 : printlog, bei richtigem Objekt
    'gMouseClick 99,99
    sleep 1
    gMouseClick 50,50
    ContextPositionAndSize
    kontext
    active.SetPage TabPositionAndSize
    kontext "TabPositionAndSize"
    Dummy_Text = PositionX.GetText
    TabPositionAndSize.OK
    sleep 1
    if TabPositionAndSize.exists (5) then printlog "Yo!"
    printlog "What?"
    if Dummy_Text = Ueber_Text_1 then
        Printlog Ueber_Text_2
    else
        warnlog Ueber_Text_3,":  is: ", Dummy_Text,"; should be: ", Ueber_Text_1
    end if
end sub

'-------------------------------------------------------------------------------
sub g_demoguide

    printlog "------------------- g_demoguide.inc ------------------------"

    call t_Introduction
    call t_Interoperability
    call t_DrawingEngine

end sub

'--------------------------------------------------------------------
sub sFormatTextDrawAnimation

    TabLauftext.OK
    WaitSlot (3000)
    gMouseClick 99,99
    WaitSlot (3000)
    hTypeKeys("<Tab>")
    WaitSlot (1000)
    hTypeKeys("<F2>")
    WaitSlot (1000)
    FormatTextDraw
    Kontext
    Active.SetPage TabLauftext
    Kontext "TabLauftext"
end sub

'-------------------------------------------------------------------------------
sub mouseclickinpresentation

    Kontext "DocumentPresentation"
    autoexecute=false
    DocumentPresentation.MouseDown ( 50, 50 )
    printlog " switch slides using mouse clicks "
    DocumentPresentation.MouseUp ( 50, 50 )
    autoexecute=true
end sub

'-------------------------------------------------------------------------------
sub im_002
    printLog Chr(13) + "--------- im_002_     ---------- $Date: 2008-06-16 10:43:16 $ $Revision: 1.1 $ "

    Call tiEditDeleteSlide
end sub

'-------------------------------------------------------------------------------
sub im_003
    printLog Chr(13) + "--------- im_003_     ----------"

    Call tiViewMasterView
    Call tiViewSlideMaster
    Call tiViewPanes
    'TODO: TBO not necessary here, move to optional
    Call tiViewToolbar_1
end sub

'-------------------------------------------------------------------------------
sub im_004
    printLog Chr(13) + "---------  im_004_     ----------"

    Call tiInsertSlideExpandSummary
end sub

'-------------------------------------------------------------------------------
sub im_005
    printLog Chr(13) + "---------  im_005_     ---------- "

    Call tiFormatModifyLayout ' impress only
end sub

'-------------------------------------------------------------------------------
sub im_007
    printLog Chr(13) + "---------  im_007_     ---------- "

    Call tSlideShowSlideShow
    Call tSlideShowRehearseTimings
    Call tSlideShowSlideShowSettings
    Call tSlideShowCustomSlideShow
    Call tSlideShowSlideTransition

    Call tSlideShowShowHideSlide
    Call tSlideShowAnimation
    Call tSlideShowCustomAnimation
    Call tSlideShowInteraction
end sub

'-------------------------------------------------------------------------------
sub im_011
    printLog Chr(13) + "---------  im_011_     ---------- "

    Call tiDiaLeiste    ' only IMPRESS
end sub

'-------------------------------------------------------------------------------
sub D_002
    printLog Chr(13) + "---------  D_002_     ---------- "

    Call tdEditCrossFading
    Call tdEditLayer
end sub

'-------------------------------------------------------------------------------
sub D_003
    printLog Chr(13) + "---------  D_003_     ---------- "

    call tdViewSlide
    call tdViewPagePane
end sub

'-------------------------------------------------------------------------------
sub D_005
    printLog Chr(13) + "---------  D_005_     ---------- "

    call tiFormatLayer ' only in draw !!!!!
end sub

'-------------------------------------------------------------------------------
sub d_007

    printLog Chr(13) + "---------  d_007     ---------- "

    call tdModifyRotate
end sub

'-------------------------------------------------------------------------------
sub hOpenGallery

    Kontext "DocumentWriter"
    ToolsGallery
    WaitSlot (2000)
    Kontext "Gallery"
    if Gallery.NotExists(2) then
        ToolsGallery
        WaitSlot (2000)
    end if
end sub

'-------------------------------------------------------------------------
sub LoadGraphic ( sFile as String, bOK as Boolean ) as boolean

    Dim iW
    Dim iWMax
    Dim iH
    Dim iHMax
    if app.FileLen(sFile) = "0" then warnlog "   the file (" + (sFile) + ") seems to be zero bytes large."
    call hGrafikEinfuegen ( sFile )
    sleep (1)
    FormatGraphics
    Kontext
    Active.SetPage TabType
    Kontext "TabType"
    OriginalSize.Click
    iW = Val ( makeNumOutOfText ( Width.GetText ) )
    iH = Val ( makeNumOutOfText ( Height.GetText )
    if instr ( sFile, "photo" ) <> 0 then
        iWMax = 22
        iHMax = 25
    else
        iWMax = 17
        iHMax = 25
    end if
    if iW > iWMax OR iH > iHMax then
        printlog sFile + " :"
        warnlog "Size is too big ( max should be '" + iWMax + "' cm* '" + iHMax + "'cm DinA4 with default borders ), but it is '" + iW + "' * '" + iH + "'"
        LoadGraphic = false
    end if
    TabType.OK
    sleep (1)
    Kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<Delete>"
    sleep (1)
    bOK = TRUE
end sub

'-------------------------------------------------------------------------
sub CheckGraphic ( sFile as String, bOK as Boolean ) as boolean

    if app.FileLen(sFile) = "0" then warnlog "   the file (" + (sFile) + ") seems to be zero bytes large."
    bOK = TRUE
end sub

'-------------------------------------------------------------------------
sub GetOnlyGraphics ( OldList() as String, NewList() as String )

    Dim i as Integer
    Dim sExtension as String

    ListAllDelete ( NewList() )
    for i=1 to ListCount ( OldList() )
        sExtension = lcase ( Right ( OldList(i), 3 ) )
        if sExtension = "jpg" OR sExtension = "gif" OR sExtension = "wmf" OR sExtension = "png" then
            ListAppend ( NewList(), OldList(i) )
        end if
    next i
end sub
'-------------------------------------------------------------------------------
sub hWalkTheStyles (optional a as integer,optional b as integer)
    dim i as integer

    if isMissing (a) then a=1
    if isMissing (b) then b=2
    i=1
    if a <= i AND i <= b then
        Kontext
        printlog " switch to tabpage 'Line' "
        Messagebox.SetPage TabLinie
        kontext "TabLinie"
        Call DialogTest ( TabLinie )
        Kontext
        printlog " switch to tabpage 'Area' "
        Messagebox.SetPage TabArea
        kontext "TabArea"
        Call DialogTest ( TabArea )
        printlog " select radio button 'none' "
        NoFill.Check
        Call DialogTest ( TabArea, 1 )
        printlog " select radio button 'color' "
        Color.Check
        Call DialogTest ( TabArea, 2 )
        printlog " select radio button 'gradient' "
        Gradient.Check
        Call DialogTest ( TabArea, 3 )
        printlog " select radio button 'hatching' "
        Hatching.Check
        Call DialogTest ( TabArea, 4 )
        printlog " select radio button 'bitmap' "
        Bitmap.Check
        Call DialogTest ( TabArea, 5 )
        Kontext
        printlog " switch to tabpage 'shadowing' "
        Messagebox.SetPage TabSchatten
        kontext "TabSchatten"
        printlog " check 'use shadow' "
        Anzeigen.check
        Call DialogTest ( TabSchatten )
        Kontext
        printlog " switch to tabpage 'Transparency' "
        Messagebox.SetPage TabTransparenz
        kontext "TabTransparenz"
        printlog " check 'No transparency' "
        KeineTransparenz.Check
        printlog " check 'Transparency' "
        LineareTransparenz.Check
        printlog " check 'Gradient' "
        Transparenzverlauf.Check
        Kontext
        printlog " switch to tabpage 'Font' "
        Messagebox.SetPage TabFont
        kontext "TabFont"
        Call DialogTest ( TabFont )
        Kontext
        printlog " switch to tabpage 'Font Effect' "
        Messagebox.SetPage TabFontEffects
        kontext "TabFontEffects"
        Kontext
        printlog " switch to tabpage 'indents & spacing' "
        Messagebox.SetPage TabEinzuegeUndAbstaende
        kontext "TabEinzuegeUndAbstaende"
        Call DialogTest ( TabEinzuegeUndAbstaende )
    endif
    i=2
    if a <= i AND i <= b then
        Kontext
        printlog " switch to tabpage 'Organize' "
        Messagebox.SetPage TabVerwalten
        kontext "TabVerwalten"
        Call DialogTest ( TabVerwalten )
        Kontext
        printlog " switch to tabpage 'text' "
        Messagebox.SetPage TabText
        Kontext "TabText"
        Call DialogTest ( TabText )
        Kontext
        printlog " switch to tabpage 'text animation' "
        Messagebox.SetPage TabLauftext
        Kontext "TabLauftext"
        Call DialogTest ( TabLauftext )
        Kontext
        printlog " switch to tabpage 'dimension' "
        Messagebox.SetPage TabBemassung
        Kontext "TabBemassung"
        Call DialogTest ( TabBemassung )
        Kontext
        printlog " switch to tabpage 'connector' "
        Messagebox.setpage TabVerbinder
        Kontext "TabVerbinder"
        Call Dialogtest ( TabVerbinder )
        Kontext
        printlog " switch to tabpage 'alignment' "
        Messagebox.setpage TabAusrichtungAbsatz
        Kontext "TabAusrichtungAbsatz"
        Links.Check
        Rechts.Check
        Zentriert.Check
        Blocksatz.Check
        Kontext
        printlog " switch to tabpage 'Tabs' "
        Messagebox.setpage TabTabulator
        Kontext "TabTabulator"
        printlog " click 'new' "
        Neu.click
        printlog "                                    ' MAYBE CHECK COUNT OF THIS ?? Position               svx:MetricBox:RID_SVXPAGE_TABULATOR:ED_TABPOS "
        printlog " click 'delete all' "
        AlleLoeschen.click
        printlog " click 'new' "
        Neu.click
        printlog " click 'delete' "
        Loeschen.click
    endif
    i=3
    if a <= i AND i <= b then
        Kontext
        printlog " switch to tabpage 'Bullets' "
        Messagebox.SetPage TabBullet
        Kontext "TabBullet"
        sleep 1
        Call DialogTest (TabBullet)
        sleep 1
        Kontext
        printlog " switch to tabpage 'Numbering Type' "
        Messagebox.SetPage TabNumerierungsart
        Kontext "TabNumerierungsart"
        sleep 1
        Call DialogTest (TabNumerierungsart)
        sleep 1
        Kontext
        printlog " switch to tabpage 'Graphics' "
        Messagebox.SetPage TabGrafiken
        Kontext "TabGrafiken"
        sleep 1
        Call DialogTest (TabGrafiken)
        sleep 1
        Kontext
        printlog " switch to tabpage 'Customize' "
        Messagebox.SetPage TabOptionenNumerierung
        Kontext "TabOptionenNumerierung"
        sleep 1
        Call DialogTest (TabOptionenNumerierung)
        sleep 1
    endif
end sub

'---------------------------------------------------------------------------------------
sub sSelectEmptyLayout
    if gApplication = "IMPRESS" then
        Kontext "DocumentImpress"
        DocumentImpress.UseMenu
        hMenuSelectNr (5)
        hMenuSelectNr (13)
        printlog "Changing focus to TaskPane."
        kontext "Tasks"
        sleep (1)
        printlog "Selecting first and empty layout."
        LayoutsPreview.TypeKeys "<HOME>"
        sleep (1)
        LayoutsPreview.TypeKeys "<RETURN>"
        sleep (1)
        kontext "DocumentImpress"
    else
        printlog "No change of Layout needed."
    endif
end sub
