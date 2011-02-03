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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Invisible items on the standardbar
'*
'\******************************************************************************

testcase tStandardBar_4

    printlog( "Invisible items on the standardbar" )

    printlog( "Reset standardbar to defaults" )
    hResetStandardBar()

    gApplication = "WRITER"
    printlog( "Open one single Writer document" )
    hInitSingleDoc()

    kontext "DocumentWriter"
    DocumentWriter.maximize() ' Needed to avoid toolbar clipping on resolutions < 1024x768 or small windows.

    printlog( "Check item count on toolbar" )
    hStandardbarItemGetCount()

    printlog( "Load URL" )
    hStandardbarLoadUrl()

    printlog( "<NEW> dialog" )
    hStandardbarNewDialog()

    printlog( "<Save As>" )
    hStandardbarSaveAs()

    printlog( "Close all open documents" )
    hFileCloseAll()

endcase

    
