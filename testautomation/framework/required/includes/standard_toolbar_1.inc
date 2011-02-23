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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : global update test (Standardbar)
'*
'\***************************************************************************

testcase tStandardBar_1

    printlog "Check the 'Standardbar' in all applications"

    gApplication = "WRITER"
    hNewDocument()
    kontext "DocumentWriter"
    DocumentWriter.maximize()
    hCloseDocument()

    Printlog " - WRITER: check if Standardbar exists"
    gApplication = "WRITER"
    hAccessStandardBar()

    Printlog " - CALC: check if Standardbar exists"
    gApplication = "CALC"
    hAccessStandardBar()

    Printlog " - IMPRESS: check if Standardbar exists"
    gApplication = "IMPRESS"
    hAccessStandardBar()

    Printlog " - DRAW: check if Standardbar exists"
    gApplication = "DRAW"
    hAccessStandardBar()

    Printlog " - HTML: check if Standardbar exists"
    gApplication = "HTML"
    hAccessStandardBar()

    Printlog " - MATH: check if Standardbar exists"
    gApplication = "MATH"
    hAccessStandardBar()

    Printlog " - GLOBALDOC: check if Standardbar exists"
    gApplication = "MASTERDOCUMENT"
    hAccessStandardBar()

    gApplication = "WRITER"


endcase


