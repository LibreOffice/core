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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
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
