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
' #0 fPositionAndSize
'*
'\***********************************************************************

function fPositionAndSize(Options as string)
    '/// Open Format/Position and Size diglog with Options.
    Kontext
    FormatPositionAndSize

    Select case Options
    case "TabPositionAndSizeWriter"  : active.SetPage TabPositionAndSizeWriter
    case "TabDrehung"                : active.SetPage TabDrehung
    case "TabSchraegstellen"         : active.SetPage TabSchraegstellen
    end Select

    Kontext Options
end function

'-------------------------------------------------------------------------

function fFormatLine(Options as string)
    '/// Open Format/Line with Options.
    Kontext
    FormatLine

    Select case Options
    case "TabLinie"           : active.SetPage TabLinie
    case "TabLinienstile"     : active.SetPage TabLinienstile
    case "TabLinienenden"     : active.SetPage TabLinienenden
    end Select

    Kontext Options
end function

'-------------------------------------------------------------------------
