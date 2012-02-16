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

****
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

