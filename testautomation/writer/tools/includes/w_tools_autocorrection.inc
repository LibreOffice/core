'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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

