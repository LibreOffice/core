'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: standard_toolbar_6.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : global update test (Standardbar)
'*
'\***************************************************************************

testcase tStandardBar_6

    '///<h1>Update test: &quot;New-Button&quot; on Standard Bar for all applications</h1>
    '///<ul>

    '///+<li>Disable the Impress Autopilot</li>
    hUseImpressAutopilot( false )
    
    gApplication = "WRITER"
    
    '///+<li>Check the 'New'-menu-button in Standardbar in each application</li>
    printlog "Check the 'New'-menu-button in Standardbar in each application"
    printlog "- click on menu-button -> open a new document as same document-type"
    
    '///+<li>Open a new writer doc</li>
    printlog "   - Writer"
    gApplication = "WRITER"
    hCreateDocument()
    '///+click on 'new'

    kontext "standardbar"
    Neu.Click()
    
    '///+<li>Check if the new document is a writer-doc</li>
    kontext "documentwriter"
    DocumentWriter.TypeKeys( "Hallo" )
    
    '///+<li>Close both new docs</li>
    hDestroyDocument()
    hDestroyDocument()
    
    '///+<li>Open a new calc doc</li>
    printlog "   - Calc"
    gApplication = "CALC"
    hCreateDocument()
    
    '///+<li>Click on 'new'</li>
    kontext "standardbar"
    Neu.Click()
    
    '///+<li>Check if the new document is a calc-doc</li>
    kontext "documentcalc"
    DocumentCalc.TypeKeys( "Hallo" )
    
    '///+<li>Close both new docs</li>
    hDestroyDocument()
    hDestroyDocument()
    
    '///+<li>Open a new draw doc</li>
    printlog "   - Draw"
    gApplication = "DRAW"
    hCreateDocument()

    kontext "standardbar"
    '///+<li>Click on 'new'</li>
    Neu.Click()
    
    '///+<li>Check if the new document is a draw-doc</li>
    gMouseClick ( 50, 50 )
    
    '///+<li>Close both new docs</li>
    hDestroyDocument()
    hDestroyDocument()
    
    '///+<li>Open a new impress doc</li>
    printlog "   - Impress"
    gApplication = "IMPRESS"
    hCreateDocument()

    kontext "standardbar"
    '///+<li>Click on 'new'</li>
    Neu.Click()
    
    '///+<li>Check if the new document is a impress-doc</li>
    gMouseClick ( 50, 50 )
    
    '///+<li>Close both new docs</li>
    hDestroyDocument()
    hDestroyDocument()
    
    '///+<li>Open a new HTML doc</li>
    printlog "   - HTML-document"
    gApplication = "HTML"
    hCreateDocument()
    '///+click on 'new'

    kontext "standardbar"
    Neu.Click()
    '///+<li>Check if the new document is a HTML-doc</li>

    kontext "documentwriter"
    DocumentWriter.TypeKeys( "Hallo" )
    
    '///+<li>Close both new docs</li>
    hDestroyDocument()
    hDestroyDocument()
    
    '///+<li>Open a new master document</li>
    printlog "   - Master-document"
    gApplication = "MASTERDOCUMENT"
    hCreateDocument()
    '///+<li>Click on &quot;new&quot;</li>

    kontext "standardbar"
    Neu.Click()
    '///+<li>Check if the new document is a master doc</li>

    kontext "documentwriter"
    DocumentWriter.TypeKeys( "Hallo" )
    
    '///+<li>Close the navigator</li>
    '///+<li>Close the document</li>
    hCloseNavigator()
    hDestroyDocument()
    
    '///+<li>Close the navigator</li>
    '///+<li>Close the second document</li>
    hCloseNavigator()
    hDestroyDocument()
    
    '///+<li>Open a new math document</li>
    printlog "   - Math"
    gApplication = "MATH"
    hCreateDocument()
    '///+<li>Click on 'new'</li>

    kontext "standardbar"
    Neu.Click()
    
    '///+<li>Check if the new document is a math document</li>
    SchreibenInMathdok "a over b"
    
    '///+<li>Close both new documents</li>
    hDestroyDocument()
    hDestroyDocument()
    '///</ul>
    
endcase


