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
'*  short description : Window/Titlebar functionality
'*
'\******************************************************************************

testcase tWindowFunctions

    printlog( "Update test for window functions" )

    printlog( "Create initial document" )
    gApplication = "WRITER"
    hInitSingleDoc()
    hInitWriteDocIdentifier( "F_updt_windowfuncs.bas" )

    printlog( "New document" )
    hNewDocument()
    if ( getDocumentCount <> 2 ) then
        warnlog( "Two open documents were expected, found " & getDocumentCount )
    endif

    kontext "DocumentWriter"
    printlog( "Close document" )
    DocumentWriter.close()

    if ( getDocumentCount <> 1 ) then
        warnlog( "One open document was expected, found " & getDocumentCount )
    endif

    printlog( "New document" )
    hNewDocument()
    if ( getDocumentCount <> 2) then
        warnlog( "Two open documents were expected, found " & getDocumentCount )
    endif

    printlog( "Enter some text into the second writer document" )
    kontext "DocumentWriter"
    DocumentWriter.TypeKeys( "test" )
    DocumentWriter.close()

    kontext "active"
    if ( Active.exists() ) then
        printlog( "Close messagebox with Cancel (leaves the document open)" )
        Active.Cancel()
    else
        warnlog( "No warning that data will be lost on close of this document" )
    endif

    kontext "DocumentWriter"
    if ( getDocumentCount = 2 ) then
        printlog( "Two documents open. Good." )
    else
        warnlog( "Incorrect document count. Expected two, found " & getDocumentCount )
    endif

    kontext "DocumentWriter"
    printlog( "Close the document" )
    FileClose()

    kontext "Active"
    if ( Active.exists() ) then
        printlog( "Do not save the document" )
        Active.No()
    else
        warnlog( "Warning: No data loss warning" )
    endif

    kontext "DocumentWriter"
    if ( getDocumentCount = 1 ) then
        printlog( "One document open. Good." )
    else
        warnlog( "Incorrect document count. Expected one, found " & getDocumentCount )
    endif

    Kontext "DocumentWriter"
    printlog( "Minimize window" )
    DocumentWriter.Minimize()
    Wait( 2000 )

    kontext "DocumentWriter"
    if ( DocumentWriter.IsMin() ) then
        printlog( "Window is minimized" )
    else
        warnlog( "Window not minimized" )
    endif

    kontext "DocumentWriter"
    printlog( "Restore window" )
    DocumentWriter.Restore()
    Wait( 2000 )

    if ( DocumentWriter.IsRestore() ) then
        printlog( "Window is Restored" )
    else
        warnlog( " * Window not Restored" )
    endif

    kontext "DocumentWriter"
    printlog( "Maximize window" )
    DocumentWriter.Maximize()
    Wait( 2000 )

    kontext "DocumentWriter"
    if ( DocumentWriter.IsMax() ) then
        printlog( "Window is maximized" )
    else
        warnlog( " * Window not maximized" )
    endif

    hDestroyDocument()

endcase

'*******************************************************************************

sub sAllWindowTitle

    printlog( "Window titles for the applications: " & gProductName )

    printlog( "Writer" )
    gApplication = "WRITER"
    call tCheckWindowTitle( "swriter", "Writer" )

    printlog( "Master Document" )
    gApplication = "MASTERDOCUMENT"
    call tCheckWindowTitle( "sglobal", "Writer" )

    printlog( "HTML" )
    gApplication = "HTML"
    call tCheckWindowTitle( "sweb", "Writer/Web" )

    printlog( "Spreadsheet" )
    gApplication = "CALC"
    call tCheckWindowTitle( "scalc", "Calc" )

    printlog( "Presentation" )
    gApplication = "IMPRESS"
    call tCheckWindowTitle( "simpress", "Impress" )

    printlog( "Drawing" )
    gApplication = "DRAW"
    call tCheckWindowTitle( "sdraw", "Draw" )

    printlog( "Formula" )
    gApplication = "MATH"
    call tCheckWindowTitle( "smath", "Math" )

    printlog( "BASIC IDE" )
    gApplication = "WRITER"
    call tCheckWindowTitle( "basic", "Basic" )

    printlog( "Database" )
    gApplication = "DATABASE"
    call tCheckWindowTitle( "database", "Base" )

    printlog( "Backing Window / StartCenter" )
    gApplication = "BACKGROUND"
    call tCheckWindowTitle( "soffice", "" )

end sub

'*******************************************************************************

testcase tCheckWindowTitle( sApplication as string, sReference as string )

    printlog( "Update test for the office window titles" )

    ' This is a Testtool-only problem that the product name is not displayed
    ' after calling hCloseDocument() on the last document.
    if ( sApplication = "soffice" ) then
        qaerrorlog( "#i113760# - Product name missing for backing window" )
        goto endsub
    endif

    const BASIC_MODULE = "tCheckWindowTitle"
    const STRING_NOT_FOUND = 0
    const STRING_LEFTMOST  = 1

    dim cWindowCaption as string

    dim iProductnamePosition as integer
    dim iApplicationPosition as integer

    dim cProductNameString as string
    dim cApplicationString as string

    ' hGetWindowCaption modifies it's function parameter, so we pass a copy instead
    ' This is a thing that justifies the use of a four letter word for sure.
    dim sApplicationTemp as string : sApplicationTemp = sApplication

    printlog( "Application: " & sApplication & "; Title should be: " & sReference )
    printlog( "Open the work windows (documents)" )
    hInitSingleDoc()

    printlog( "Verify that the correct window is open" )
    select case sApplication
        case "basic"   : hInitBasicIDE( BASIC_MODULE )
        case "soffice" : hFileCloseAll()
        case else      : hCreateDocument()
    end select

    ' Note: The Testtool connection string does not belong to the window caption
    '       so we do not check for it here.
    cWindowCaption = hGetWindowCaption( sApplicationTemp )
    printlog( "Caption is: " & cWindowCaption )

    ' This prevents accidential matches like "base" in a document name like "database8.odb"
    ' And we have a - (minus) as delimiter between the document name and the product name
    cProductnameString = " - " & gProductName & " "
    cApplicationString =   " " & sReference

    ' Find out where in the caption the substrings are
    iProductNamePosition = instr( cWindowCaption, cProductnameString )
    iApplicationPosition = instr( cWindowCaption, cApplicationString )

    ' Verify that the productname is contained within the string
    if ( iProductNamePosition = STRING_NOT_FOUND ) then
        if ( lcase( gPlatform ) = "osx" ) then
            printlog( "The product name is missing" )
            ' life is different on that platform compared to testtool world
        else
            warnlog( "The product name is missing" )
        endif
    endif

    ' Verify that the application name (e.g. "Writer") is contained in the string
    ' The StartCenter should not have any application name
    if ( sApplication = "soffice" ) then
        if ( iApplicationPosition > STRING_NOT_FOUND ) then
            printlog( "The StartCenter should not have an application name" )
        endif
    else
        if ( iApplicationPosition = STRING_NOT_FOUND ) then
            warnlog( "The application name is missing" )
        endif
    endif

    if ( sApplication = "soffice" ) then
        printlog( "Backing window should exclusively show the product name" )
    else
        ' If one of Productname or application is leftmost in the string, we have a
        ' missing document identifier. This can be different things - the filename,
        ' the module name (BASIC) etc.
        if ( ( iApplicationPosition = STRING_LEFTMOST ) OR _
               iProductnamePosition = STRING_LEFTMOST ) then
            warnlog( "Current document identifier missing (like e.g. <Unknown 1>)" )
        endif

        ' Verify that the order is correct: Product name first, the application
        if ( iProductNamePosition >= iApplicationPosition ) then
            warnlog( "The order of the strings is incorrect" )
            printlog( "Expected: <Product Name> <Application>" )
        endif
    endif

    printlog( "Cleanup after test" )
    select case sApplication
        case "basic"   : hDestroyDocument()
        case "soffice" : hCreateDocument()
    end select

    printlog( "Close all open documents" )
    hFileCloseAll()

endcase

