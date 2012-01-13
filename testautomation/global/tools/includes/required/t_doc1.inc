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

*****

function hSetDocumentContext() as string

    ' link gApplication to its document kontext

    dim cDocumentContext as string

    select case( gApplication )
    case "DRAW"             : cDocumentContext = "DocumentDraw"
    case "IMPRESS"          : cDocumentContext = "DocumentImpress"
    case "WRITER"           : cDocumentContext = "DocumentWriter"
    case "CALC"             : cDocumentContext = "DocumentCalc"
    case "MATH"             : cDocumentContext = "DocumentMath"
    case "HTML"             : cDocumentContext = "DocumentWriterWeb"
    case "MASTERDOCUMENT"   : cDocumentContext = "DocumentMasterDoc"
    case "CHART"            : cDocumentContext = "DocumentChart"
    case "BACKGROUND"       : cDocumentContext = "Background"
    case else               : warnlog( "Unknown gApplication: " & gApplication )
    end select

    kontext cDocumentContext : hSetDocumentContext() = cDocumentContext

end function

'*******************************************************************************

function hSetDocumentObject() as object

    ' link gApplication to its document object

    dim oDocumentObject as object

    select case( gApplication )
    Case "WRITER"         : oDocumentObject = DocumentWriter
    Case "MASTERDOCUMENT" : oDocumentObject = DocumentMasterDoc
    Case "HTML"           : oDocumentObject = DocumentWriterWeb
    case "CALC"           : oDocumentObject = DocumentCalc
    case "DRAW"           : oDocumentObject = DocumentDraw
    case "IMPRESS"        : oDocumentObject = DocumentImpress
    case "MATH"           : oDocumentObject = DocumentMath
    case "CHART"          : oDocumentObject = DocumentChart
    case "BACKGROUND"     : oDocumentObject = Desktop
    case else             : warnlog( "Unknown gApplication: " & gApplication )
    end select

    hSetDocumentObject() = oDocumentObject

end function

'*******************************************************************************

function hSetDocumentFactory() as string

    ' link gApplication to its factory as used by FileOpen slot

    dim cDocumentFactory as string

    select case( gApplication )
    case "DRAW"             : cDocumentFactory = "private:factory/sdraw"
    case "IMPRESS"          : cDocumentFactory = "private:factory/simpress"
    case "WRITER"           : cDocumentFactory = "private:factory/swriter"
    case "CALC"             : cDocumentFactory = "private:factory/scalc"
    case "MATH"             : cDocumentFactory = "private:factory/smath"
    case "HTML"             : cDocumentFactory = "private:factory/swriter/web"
    case "MASTERDOCUMENT"   : cDocumentFactory = "private:factory/swriter/GlobalDocument"
    case "DATABASE"         : cDocumentFactory = "private:factory/sdatabase?Interactive"
    case else               : warnlog( "Unknown gApplication: " & gApplication )
    end select

    hSetDocumentFactory() = cDocumentFactory

end function