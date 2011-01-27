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
'* owner : oliver.craemer@oracle.com
'*
'* short description : tools for localisation in calc
'*
'\************************************************************************************************

function fFunctionname ( sFunctionname_en as string ) as string

    '///<b>The function returns the localized name of a given function in the current UI language</b>
    '///+ -Input is the english name of the function as string
    '///+ -Output is the localized name of the function as string
    '///+ -If there is no translation available the fallback is the english name

    dim sloaddocument as string
    dim sfunctionstring as string
    dim scelladress as string

    use "global/tools/includes/required/t_doc2.inc"
    const CFN = "spreadsheet:tools:includes:c_l10n_tools.in:fFunctionname "

    sloaddocument = gTesttoolPath & "spreadsheet\tools\input\Functionnames.ods"

    '///Load document with all functions <i>gTestToolPath</i>/spreadsheet/tools/input/Functionnames.ods
    hFileOpenLocally( sloaddocument )
    sleep (2)

    '///Setting selection to A1 to avoid messagebox in Search and Replace dialog.
    call fCalcSelectRange ("A1")
    '///Search for function by Search and Replace dialog
    kontext "DocumentCalc"
    EditSearchAndReplace

    kontext "FindAndReplace"
    printlog( "Reset <Find and Replace> dialog to default settings" )
    if ( FindAndReplace.exists( 1 ) ) then
        More.Click
        SimilaritySearch.UnCheck
        CurrentSelectionOnly.UnCheck
        Backwards.UnCheck
        SearchForStyles.UnCheck
        SearchIn.Select 1
        '///Setting search parameter
        WholeWordsOnly.Check
        SearchFor.SetText sFunctionname_en
        SearchNow.click
        kontext
        '///If a MsgBox appears the search must have failed => Fallback to english name
        if active.exists then
            if active.getRT = 304 then
                warnlog CFN & "Function not found, falling back to english name"
                fFunctionname = sFunctionname_en
                active.OK
            else
                '///Throw a warning if a dialog of unexpected resource type appears
                warnlog CFN & "Unknown message box! " & active.GetText
                active.Default
                fFunctionname = sFunctionname_en
            end if

            kontext "FindAndReplace"
            printlog( "Reset dialog to default settings" )
            if ( FindAndReplace.exists( 1 ) ) then
                WholeWordsOnly.UnCheck
                hCloseDialog( FindAndReplace, "close" )
            else
                warnlog( CFN & "<FindAndReplace> not open" )
            endif
            call hCloseDocument
            exit function
        end if

        kontext "FindAndReplace"
        printlog( "Reset <Find and Replace> dialog to default settings" )
        if ( FindAndReplace.exists( 1 ) ) then
            WholeWordsOnly.UnCheck
            hCloseDialog( FindAndReplace, "close" )
        else
            warnlog( CFN & "<FindAndReplace> not open" )
        endif

        Kontext "RechenleisteCalc"
        scelladress = Bereich.GetSelText
        if fCalcGetCellValue (scelladress) = sFunctionname_en then
            '///Get localised string for function by extracting functionname out of next cell
            call fCalcSelectRange ( "B" & mid(scelladress,2) )
            kontext ( "RechenleisteCalc" )
            EingabeZeileCalc.TypeKeys ("<f2><mod1 a>")
            editcopy
            sfunctionstring = GetClipboardText ()
            kontext "DocumentCalc"
            DocumentCalc.TypeKeys ("<ESCAPE>",2)
            ' DEBUG: printlog sfunctionstring
            ' DEBUG: printlog len(sfunctionstring)
            ' DEBUG: printlog instr(sfunctionstring,"(")
            fFunctionname = mid(sfunctionstring,2,(instr(sfunctionstring,"(")-2)
            printlog "  The name of the function in this locale (" & iSprache & ") is " & fFunctionname
        else
            warnlog CFN & "Function not found, falling back to english version"
            fFunctionname = sFunctionname_en
        end if
    else
        warnlog( CFN & "Unable to open <FindAndReplace> dialog" )
    endif

    call hCloseDocument
    hFileDelete( gLastWorkFile )

end function

'
'----------------------------------------------------------------------------
'

function fError_l10n ( sError_en as string ) as string

    '///<b>The function returns the localized name of a given error in the current UI language</b>
    '///+ -Input is the english name of the error as string
    '///+ -Output is the localized name of the error as string
    '///+ -If there is no translation available the fallback is the english name

    dim sloaddocument as string
    dim sfunctionstring as string
    dim scelladress as string

    use "global/tools/includes/required/t_doc2.inc"
    const CFN = "qa:qatesttool:spreadsheet:tools:includes:c_l10n_tools.in:fError_l10n "

    sloaddocument = gTesttoolPath & "spreadsheet\tools\input\Errorcodes.ods"

    '///Load document with all errorcodes <i>gTestToolPath</i>/spreadsheet/tools/input/Errorcodes.ods
    hFileOpenLocally( sloaddocument )

    '///Setting selection to A1 to avoid messagebox in Search and Replace dialog.
    call fCalcSelectRange ("A1")
    '///Search for function by Search and Replace dialog
    kontext "DocumentCalc"
    EditSearchAndReplace
    sleep (1)
    kontext "FindAndReplace"
    '///Setting dialog to defaults
    More.Click
    SimilaritySearch.UnCheck
    CurrentSelectionOnly.UnCheck
    Backwards.UnCheck
    SearchForStyles.UnCheck
    SearchIn.Select 1
    '///Setting search parameter
    WholeWordsOnly.Check
    SearchFor.SetText sError_en
    SearchNow.click
    kontext
    '///If a MsgBox appears the search must have failed => Fallback to english name
    if active.exists then
        if active.getRT = 304 then
            warnlog CFN & "Function not found, falling back to english name"
            fError_l10n = sError_en
            active.OK
        else
            '///Throw a warning if a dialog of unexpected resource type appears
            warnlog CFN & "Unknown message box! " & active.GetText
            active.Default
            fError_l10n = sError_en
        end if

        kontext "FindAndReplace"
        '///Reset dialog to defaultsetting
        if ( FindAndReplace.exists( 1 ) ) then
            WholeWordsOnly.UnCheck
            hCloseDialog( FindAndReplace, "close" )
        else
            warnlog( CFN & "Failed to open <FindAndReplace>" )
        endif

        call hCloseDocument
        exit function
    end if

    kontext "FindAndReplace"
    '///Reset dialog to defaultsetting
    if ( FindAndReplace.exists( 1 ) ) then
        WholeWordsOnly.UnCheck
        hCloseDialog( FindAndReplace, "close" )
    else
        warnlog( CFN & "Failed to open <FindAndReplace>" )
    endif
    
    Kontext "RechenleisteCalc"
    scelladress = Bereich.GetSelText
    if fCalcGetCellValue (scelladress) = sError_en then
        '///Get localised string for ERR by extracting errorcode out of next cell
        call fCalcSelectRange ( "B" & mid(scelladress,2) )
        kontext "DocumentCalc"
        editcopy
        fError_l10n = GetClipboardText ()
        kontext "DocumentCalc"
        printlog "  The name for ERR in this locale (" & iSprache & ") is " & fError_l10n
    else
        warnlog CFN & "Errorcode not found, falling back to english version"
        fError_l10n = sError_en
    end if

    call hCloseDocument
    hFileDelete( gLastWorkFile )

end function
