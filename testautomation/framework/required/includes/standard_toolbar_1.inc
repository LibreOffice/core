'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: standard_toolbar_1.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:16:28 $
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

testcase tStandardBar_1

    printlog "************************************************************"
    printlog "Check the 'Standardbar' in all applications"
    printlog "************************************************************"

    '///<h1>Update test for the Standard Bar in all applications</h1>

    '///<ul>
    gApplication = "WRITER"
    hNewDocument()
    kontext "DocumentWriter"
    DocumentWriter.maximize()
    hCloseDocument()

    '///+<li>Open a writer-doc and check if the Standardbar does exist</li>
    Printlog " - WRITER: check if Standardbar exists"
    gApplication = "WRITER"
    hAccessStandardBar()

    '///+<li>Open a calc-doc and check if the Standardbar does exist</li>
    Printlog " - CALC: check if Standardbar exists"
    gApplication = "CALC"
    hAccessStandardBar()

    '///+<li>Open a impress-doc and check if the Standardbar does exist</li>
    Printlog " - IMPRESS: check if Standardbar exists"
    gApplication = "IMPRESS"
    hAccessStandardBar()

    '///+<li>Open a draw-doc and check if the Standardbar does exist</li>
    Printlog " - DRAW: check if Standardbar exists"
    gApplication = "DRAW"
    hAccessStandardBar()

    '///+<li>Open a HTML-doc and check if the Standardbar does exist</li>
    Printlog " - HTML: check if Standardbar exists"
    gApplication = "HTML"
    hAccessStandardBar()

    '///+<li>Open a math-doc and check if the Standardbar does exist</li>
    Printlog " - MATH: check if Standardbar exists"
    gApplication = "MATH"
    hAccessStandardBar()

    '///+<li>Open a master-doc and check if the Standardbar does exist</li>
    Printlog " - GLOBALDOC: check if Standardbar exists"
    gApplication = "MASTERDOCUMENT"
    hAccessStandardBar()

    '///+<li>Go back to a writer document</li>
    gApplication = "WRITER"

    '///</ul>

endcase


