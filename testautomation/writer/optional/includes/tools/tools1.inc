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
'* short description : Tools for writer/Optional - 1
'*
'\***********************************************************************


'This routine is executing file/properties , then tabpage Options
'*****************************************************************
function fFileProperties(Options as string)

    if ( hUseAsyncSlot( "FileProperties" ) <> -1 ) then

        Kontext
        if ( Active.exists( 2 ) ) then
            Select case Options
            case "TabDokument"        : active.SetPage TabDokument
            case "TabBeschreibung"    : active.SetPage TabBeschreibung
            case "TabBenutzer"        : active.SetPage TabBenutzer
            case "TabInternet"        : active.SetPage TabInternet
            case "TabStatistik"       : active.SetPage TabStatistik
            end Select
        else
            warnlog( "<FileProperties> dialog not open" )
        endif
    else
        warnlog( "Failed to execute <FileProperties>" )
    endif

    Kontext Options

end function



'*****************************************************************
'This routine is executing format/graphic , then tabpage Options
'*****************************************************************
function fFormatGraphic(Options as string) as boolean
    Dim j as integer, sSlotOK as boolean

    fFormatGraphic = true
    if ( hUseAsyncSlot( "FormatGraphics" ) <> -1 ) then
        Kontext
        if ( Active.exists( 2 ) ) then
            Select case Options
            case "TabType"                     : active.SetPage TabType
            case "TabZusaetze"                 : active.SetPage TabZusaetze
            case "TabUmlauf"                   : active.SetPage TabUmlauf
            case "TabHyperlinkRahmen"          : active.SetPage TabHyperlinkRahmen
            case "TabGrafik"                   : active.SetPage TabGrafik
            case "TabZuschneiden"              : active.SetPage TabZuschneiden
            case "TabUmrandung"                : active.SetPage TabUmrandung
            case "TabHintergrund"              : active.SetPage TabHintergrund
            case "TabMakro"
                active.SetPage TabMakro
                ' if no JRE is installed a messagebox appears
                Do
                    j = j + 1
                    Kontext "Active"
                    if Active.Exists then
                        if Active.GetRT = 304 then
                            if j = 1 then Warnlog Active.Gettext
                            Active.Ok
                        else
                            exit do
                        end if
                    else
                        exit do
                    end if
                Loop
            end select
        else
            warnlog( "Failed to access <Format Graphics> Tabpage" )
            fFormatGraphic = false
        endif
    else
        warnlog( "Failed to execute <FormatGraphics>" )
        fFormatGraphic = false
    endif

    Kontext Options

end function



'*****************************************************************
'This routine is executing format/frame , then tabpage Options
'****************************************************************
function fFormatFrame(Options as string)

    if ( hUseAsyncSlot( "FormatFrame" ) <> -1 ) then

        Kontext
        if ( Active.exists( 2 ) ) then
            Select case Options
            case "TabType"                     : active.SetPage TabType
            case "TabZusaetze"                 : active.SetPage TabZusaetze
            case "TabUmlauf"                   : active.SetPage TabUmlauf
            case "TabHyperlinkRahmen"          : active.SetPage TabHyperlinkRahmen
            case "TabUmrandung"                : active.SetPage TabUmrandung
            case "TabHintergrund"              : active.SetPage TabHintergrund
            case "TabSpalten"                  : active.SetPage TabSpalten
            case "TabMakro"                    : active.SetPage TabMakro
            end Select
        else
            warnlog( "<Format Frame> dialog did not open" )
        endif
    else
        warnlog( "Failed to execute <FormatFrame>" )
    endif

    Kontext Options

end function


'*************************************************************************
'This routine is executing format/page in writer , then tabpage Options
'*************************************************************************
function fFormatPageWriter(Options as string)

    FormatPageWriter
    Kontext

    Select case Options
    case "TabVerwalten"                : active.SetPage TabVerwalten
    case "TabSeite"                    : active.SetPage TabSeite
    case "TabHintergrund"              : active.SetPage TabHintergrund
    case "TabKopfzeile"                : active.SetPage TabKopfzeile
    case "TabFusszeile"                : active.SetPage TabFusszeile
    case "TabUmrandung"                : active.SetPage TabUmrandung
    case "TabSpalten"                  : active.SetPage TabSpalten
    case "TabFussnote"                 : active.SetPage TabFussnote
    case "TabGrid"                     : active.SetPage TabGrid
    end Select

    Kontext Options

end function


'*************************************************************************
'This routine is executing format/paragraph in writer , then tabpage Options
'*************************************************************************
function fFormatParagraph(Options as string)

    FormatParagraph
    Kontext

    Select case Options
    case "TabEinzuegeUndAbstaende" : active.SetPage TabEinzuegeUndAbstaende
    case "TabAusrichtungAbsatz"    : active.SetPage TabAusrichtungAbsatz
    case "TabTextfluss"            : active.SetPage TabTextfluss
    case "TabAsianTypography"      : active.SetPage TabAsianTypography
    case "TabNumerierungAbsatz"    : active.SetPage TabNumerierungAbsatz
    case "TabTabulatoren"          : active.SetPage TabTabulatoren
    case "TabInitialen"            : active.SetPage TabInitialen
    case "TabUmrandung"            : active.SetPage TabUmrandung
    case "TabHintergrund"          : active.SetPage TabHintergrund
    end Select

    Kontext Options

end function


'*************************************************************************
'This routine is executing format/character in writer , then tabpage Options
'*************************************************************************
function fFormatCharacter(Options as string)

    FormatCharacter
    Kontext

    Select case Options
    case "TabFont"                  : active.SetPage TabFont
    case "TabFontEffects"           : active.SetPage TabFontEffects
    case "TabFontPosition"          : active.SetPage TabFontPosition
    case "TabAsianLayout"           : active.SetPage TabAsianLayout
    case "TabHyperlinkZeichen"      : active.SetPage TabHyperlinkZeichen
    case "TabHintergrund"           : active.SetPage TabHintergrund
    end Select

    Kontext Options

end function



'*************************************************************************
'This routine is executing format/table in writer , then tabpage Options
'*************************************************************************
function fFormatTable(Options as string)

    FormatTable
    Kontext

    Select case Options
    case "TabTabelle"               : active.SetPage TabTabelle
    case "TabTextflussTabelle"      : active.SetPage TabTextflussTabelle
    case "TabSpaltenTabelle"        : active.SetPage TabSpaltenTabelle
    case "TabUmrandung"             : active.SetPage TabUmrandung
    case "TabHintergrund"           : active.SetPage TabHintergrund
    end Select

    Kontext Options

end function


'*************************************************************************
'This routine is executing Format/Styles / Catalog in writer , then tabpage Options
'*************************************************************************
function fFormatStylesCatalog(Options as string)

    FormatStylesCatalog
    Kontext "VorlagenKatalog"

    Select case Options
    case "Paragraph"      : Vorlagen.Select 1
    case "Character"      : Vorlagen.Select 2
    case "Frame"          : Vorlagen.Select 3
    case "Page"           : Vorlagen.Select 4
    case "Numbering"      : Vorlagen.Select 5
    end Select

    Kontext Options

end function


'******************************************************************************
'This routine is executing insert/fields/other in writer , then tabpage Options
'*******************************************************************************
function fInsertFieldsOther(Options as string)

    InsertFieldsOther
    Kontext

    Select case Options
    case "TabDokumentFeldbefehle"     : active.SetPage TabDokumentFeldbefehle
    case "TabReferenzen"              : active.SetPage TabReferenzen
    case "TabFunktionen"              : active.SetPage TabFunktionen
    case "TabDokumentinfoFeldbefehle" : active.SetPage TabDokumentinfoFeldbefehle
    case "TabVariablen"               : active.SetPage TabVariablen
    case "TabDatenbank"               : active.SetPage TabDatenbank
    end Select

    Kontext Options

end function


'******************************************************
'* insert a section with tabpage Options             **
'******************************************************
function fInsertSection(Options as string)

    InsertSection

    Kontext
    if ( Active.exists( 2 ) ) then
        Select case Options
        case "TabBereiche"       : active.SetPage TabBereiche
        case "TabSpalten"        : active.SetPage TabSpalten
        case "TabSectionIndent"  : active.SetPage TabSectionIndent
        case "TabHintergrund"    : active.SetPage TabHintergrund
        case "TabFussEndnoten"   : active.SetPage TabFussEndnoten
        end Select
        waitslot()
    else
        warnlog( "Failed to open <Insert Section> dialog" )
    endif

    Kontext Options

end function


'*************************************************************************
'This routine is executing Tools/AutoCorrect in writer , then tabpage Options
'*************************************************************************
function fToolsAutocorrect(Options as string)

    Kontext
    ToolsAutoCorrect

    Select case Options
    case "TabErsetzung"                : active.SetPage TabErsetzung
    case "TabAusnahmen"                : active.SetPage TabAusnahmen
    case "TabOptionenAutokorrektur"    : active.SetPage TabOptionenAutokorrektur
    case "TabLocalizedOptions"         : active.SetPage TabLocalizedOptions
    case "TabWortergaenzung"           : active.SetPage TabWortergaenzung
    end Select

    Kontext Options

end function


'******************************************************
'* Open Format/Position and Size diglog with Options **
'******************************************************
function fPositionAndSize(Options as string)

    Kontext
    FormatPositionAndSize

    Select case Options
    case "TabPositionAndSizeWriter"  : active.SetPage TabPositionAndSizeWriter
    case "TabDrehung"                : active.SetPage TabDrehung
    case "TabSchraegstellen"         : active.SetPage TabSchraegstellen
    end Select

    Kontext Options

end function

