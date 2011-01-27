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
'* short description : Test the AutoCorrect/AutoFormat/Word tools
'*
'************************************************************************
'*
' #0 fFormatCharacter
'*
'\***********************************************************************

function fAutocorrectOptions(Options as String, iModify as integer, iType as integer)
    '/// Open Tools/Autocorrect/Autoformat diglog with tab page Options
    '///+ then choose the relevant Options and press 'space bar'
    Dim iDown as integer

    ToolsAutocorrect
    Kontext
    Active.Setpage TabOptionenAutokorrektur
    Kontext "TabOptionenAutokorrektur"

    Select case Options
        case "UseReplacementTable"         : iDown = 1
        case "CorrectTWoINitialCApitals"   : iDown = 2
        case "CapitalizeFirstLetter"       : iDown = 3
        case "AutomaticBoldAndUnderline"   : iDown = 4
        case "URLRecognition"              : iDown = 5
        case "ReplaceDashes"               : iDown = 6
        case "DeleteSpaceParagraph"        : iDown = 7
        case "DeleteSpaceLine"             : iDown = 8
        case "IgnoreDoubleSpaces"          : iDown = 9
        case "ApplyNumbering"              : iDown = 10
        case "ApplyBorder"                 : iDown = 11
        case "CreateTable"                 : iDown = 12
        case "ApplyStyles"                 : iDown = 13
        case "RemoveBlankParagraphs"       : iDown = 14
        case "ReplaceCustomStyles"         : iDown = 15
        case "ReplaceBullets"              : iDown = 16
        case "CombineSingleLine"           : iDown = 17
    end select

    ' *** Set requested state ***
    ' ### Modify
    Select Case iDown
        Case 1,2,3,4,5,6,7,8,14,15,16,17
            if iModify = 1 then
                Einstellungen.Check(iDown,2)
            else
                Einstellungen.UnCheck(iDown,2)
            endif
            if iDown < 9 then
                ' ### Type
                if iType = 1 then
                    Einstellungen.Check(iDown,3)
                else
                    Einstellungen.UnCheck(iDown,3)
                endif
            endif
        Case 9,10,11,12,13
        ' ### Type
        if iType = 1 then
            Einstellungen.Check(iDown,3)
        else
            Einstellungen.UnCheck(iDown,3)
        endif
    end select
    wait 100
    TabOptionenAutokorrektur.OK

end function

'---------------------------------------------------------------

function fFormatCharacter(Options as string)
    '/// Open format/character diglog with Options.
    Kontext
    FormatCharacter

    Select case Options
    case "TabFont"                     : active.SetPage TabFont
    case "TabFontEffects"              : active.SetPage TabFontEffects
    case "TabFontPosition"             : active.SetPage TabFontPosition
    case "TabHyperlinkZeichen"         : active.SetPage TabHyperlinkZeichen
    case "TabHintergrund"              : active.SetPage TabHintergrund
    end select

    Kontext Options

end function

'-------------------------------------------------------------------------

function fFormatParagraph(Options as string)
    '/// Open format/paragraph diglog with Options
    Kontext
    FormatParagraph

    Select case Options
    case "TabUmrandung"                : active.SetPage TabUmrandung             'Borders
    case "TabEinzuegeUndAbstaende"     : active.SetPage TabEinzuegeUndAbstaende  'Indents & Spacing
    end select

    Kontext Options

end function

'-------------------------------------------------------------------------

function fDeleteAllEntries()
    '/// This function is to delete all entries in tools/autocorrect/Word completions.
    Dim i as Integer

    ToolsAutocorrect
    Kontext
    Active.Setpage TabWortergaenzung
    Kontext "TabWortergaenzung"

    if GesammelteWorte.GetItemCount > 0 then
        for i= 1 to GesammelteWorte.GetItemCount
            GesammelteWorte.Select 1
            if EintragLoeschen.IsEnabled then
                EintragLoeschen.Click
            else
                i = GesammelteWorte.GetItemCount + 1
            end if
        next i
    end if

    TabWortergaenzung.OK
end function

