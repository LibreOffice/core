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

