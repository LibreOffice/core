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
'*  short description : Tools to handle windows/frames
'*
'\******************************************************************************

function hMaximizeDocument() as boolean


    '///<h3>Maximize a document window</h3>
    '///<i>Note</i>: The function runs silent (no logs written)<br><br>
    '///<u>Return Value:</u><br>

    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE if the window claims to have been maximized</li>
    '///+<li>FALSE on invalid gApplication (with warning)</li>
    '///+<li>FALSE if the window thinks it has not been maximized</li>
    '///</ul>
    '///</ol>

    const CFN = "hMaximizeDocument::"
    dim brc as boolean 'a multi purpose boolean returnvalue

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Maximize the documentwindow depending on gApplication</li>
    select case ( ucase( gApplication ) )
    case "WRITER"    : Kontext "DocumentWriter"
                       DocumentWriter.maximize()
                       brc = DocumentWriter.isMaximized()
    case "CALC"      : Kontext "DocumentCalc"
                       DocumentCalc.maximize()
                       brc = DocumentCalc.isMaximized()
    case "IMPRESS"   : Kontext "DocumentImpress"
                       DocumentImpress.maximize()
                       brc = DocumentImpress.isMaximized()
    case "DRAW"      : Kontext "DocumentDraw"
                       DocumentDraw.maximize()
                       brc = DocumentDraw.isMaximized()
    case "MATH"      : Kontext "DocumentMath"
                       DocumentMath.maximize()
                       brc = DocumentMath.isMaximized()
    case "MASTERDOCUMENT" : Kontext "DocumentWriter"
                       DocumentWriter.maximize()
                       brc = DocumentWriter.isMaximized()
    case "HTML"      : Kontext "DocumentWriter"
                       DocumentWriter.maximize()
                       brc = DocumentWriter.isMaximized()
    case else        : qaerrorlog( CFN & "Invalid documenttype" )
                       brc = false
    end select

    '///</ul>

    hMaximizeDocument() = brc

end function
