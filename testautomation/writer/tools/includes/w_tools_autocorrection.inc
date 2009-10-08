'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: w_tools_autocorrection.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: vg $ $Date: 2008-08-18 12:44:33 $
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
'* owner : fredrik.haegg@sun.com
'*
'* short description : Test the AutoCorrect/AutoFormat/Word tools
'*
'************************************************************************
'*
' #0 fFormatCharacter
'*
'\***********************************************************************

function fAutocorrectOptions(Options as String)
    '/// Open Tools/Autocorrect/Autoformat diglog with tab page Options
    '///+ then choose the relevant Options and press 'space bar'

    ToolsAutocorrect
    Kontext
    Active.Setpage TabOptionenAutokorrektur
    Kontext "TabOptionenAutokorrektur"
    
    Einstellungen.TypeKeys "<Home>"

    Select case Options
        case "UseReplacementTable"         : wait 500
        case "CorrectTWoINitialCApitals"   : Einstellungen.TypeKeys "<Down>"
        case "CapitalizeFirstLetter"       : Einstellungen.TypeKeys "<Down>",2
        case "AutomaticBoldAndUnderline"   : Einstellungen.TypeKeys "<Down>",3
        case "URLRecognition"              : Einstellungen.TypeKeys "<Down>",4
        case "Replace1st"                  : Einstellungen.TypeKeys "<Down>",5
        case "Replace1/2"                  : Einstellungen.TypeKeys "<Down>",6
        case "ReplaceDashes"               : Einstellungen.TypeKeys "<Down>",7
        case "DeleteSpaceParagraph"        : Einstellungen.TypeKeys "<Down>",8
        case "DeleteSpaceLine"             : Einstellungen.TypeKeys "<Down>",9
        case "IgnoreDoulbeSpaces"          : Einstellungen.TypeKeys "<Down>",10
        case "ApplyNumbering"              : Einstellungen.TypeKeys "<Down>",11
        case "ApplyBorder"                 : Einstellungen.TypeKeys "<Down>",12
        case "CreateTable"                 : Einstellungen.TypeKeys "<Down>",13
        case "ApplyStyles"                 : Einstellungen.TypeKeys "<Down>",14
        case "RemoveBlankParagraphs"       : Einstellungen.TypeKeys "<Down>",15
        case "ReplaceCustomStyles"         : Einstellungen.TypeKeys "<Down>",16
        case "ReplaceBullets"              : Einstellungen.TypeKeys "<Down>",17
        case "ReplaceQuotes"               : Einstellungen.TypeKeys "<Down>",18
        case "CombineSingleLine"           : Einstellungen.TypeKeys "<End>"

    end select

    Einstellungen.TypeKeys "<SPACE>"
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

