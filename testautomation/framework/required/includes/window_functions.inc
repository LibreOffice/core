'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: window_functions.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:16:41 $
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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Window/Titlebar functionality
'*
'\******************************************************************************

testcase tWindowFunctions

    printlog( "Update test for window functions" )

    if ( gtSysName = "Mac OS X" ) then
        printlog( "No testing for Mac as some Window attributes do not exist" )
        goto endsub
    endif

    
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
        qaerrorlog( "#i32672# Window not minimized" )
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

    printlog( "Window titles for the applications" )
    
    printlog( "Writer" )
    gApplication = "WRITER"
    call tCheckWindowTitle("swriter","Writer")

    printlog( "Master Document" )
    gApplication = "MASTERDOCUMENT"
    call tCheckWindowTitle("sglobal","Writer")

    printlog( "HTML" )
    gApplication = "HTML"
    call tCheckWindowTitle("sweb","Writer/Web")

    printlog( "Spreadsheet" )
    gApplication = "CALC"
    call tCheckWindowTitle("scalc","Calc")

    printlog( "Presentation" )
    gApplication = "IMPRESS"
    call tCheckWindowTitle("simpress","Impress")

    printlog( "Drawing" )
    gApplication = "DRAW"
    call tCheckWindowTitle("sdraw","Draw")

    printlog( "Formula" )
    gApplication = "MATH"
    call tCheckWindowTitle("smath","Math")

    qaerrorlog( "Excluded BASE and BACKINGWINDOW" )

    'gApplication = "DATABASE"
    'call tCheckWindowTitle("DATABASE","Base")

    'fileclose
    'call tCheckWindowTitle("soffice","")

end sub

'*******************************************************************************

testcase tCheckWindowTitle(sApplication as string, sReference as string)

    printlog( "Update test for the office window titles" )
    '<u>Input</u>:
    '<ol>
    '+<li>Name of application (string), case sensitive. Valid options are:</li>
    '<ol>
    '+<li>&quot;swriter&quot;</li>
    '+<li>&quot;sglobal&quot;</li>
    '+<li>&quot;sweb&quot;</li>
    '+<li>&quot;scalc&quot;</li>
    '+<li>&quot;simpress&quot;</li>
    '+<li>&quot;sdraw&quot;</li>
    '+<li>&quot;smath&quot;</li>
    '+<li>&quot;insight&quot; * Currently disabled</li>
    '+<li>&quot;soffice&quot; * Currently disabled</li>
    '</ol>
    '+<li>Reference name (string), matches name of application</li>
    '<ol>
    '+<li>&quot;Writer&quot;</li>
    '+<li>&quot;Writer/Web&quot;</li>
    '+<li>&quot;Calc&quot;</li>
    '+<li>&quot;Impress&quot;</li>
    '+<li>&quot;Draw&quot;</li>
    '+<li>&quot;Math&quot;</li>
    '+<li>&quot;Base&quot; * Currently disabled</li>
    '+<li>&quot;&quot; * Currently disabled</li>
    '</ol>
    '</ol>
    '<u>Returns</u>:
    '<ol>
    '+<li>Nothing</li>
    '</ol>
    '<u>Description</u>:
    '<ul>
    

    dim sTemp as string
    dim saTemp() as string
    dim brc as boolean
    gApplication = UCase (gApplication)
    printlog " - Application: " + sApplication + "; Title should be: " + sReference
    
    printlog( "Ensure that exactly one document is open" )
    hInitSingleDoc()

    printlog( "Open another document as specified by gApplication" )    
    hCreateDocument()

    printlog( "Verify that the correct window is open" )
    select case sApplication
        case"swriter":
        case"sglobal":
        case"sweb":
        case"scalc":
        case"simpress":
        case"sdraw":
        case"smath":
        case"basic":
            ToolsMacroMacro

            kontext "makro"

            if Makro.exists(5) then
                MakroAus.typeKeys "<end>"

                if (Neu.isEnabled) then
                    Neu.click

                    kontext "basicide"

                    if BasicIDE.exists(5) then
                        printlog( "Basic IDE open. Good." )
                    else
                        warnlog( "Basic IDE not open. This is unexpected" )
                    endif

                else
                    warnlog( "New-button is unexpectedly disabled." )
                endif


                try

                    kontext "neuesmodul"

                    if NeuesModul.exists(5) then
                        NeuesModul.OK
                    else
                        warnlog( "New module naming dialog is not open" )
                    endif

                catch
                    warnlog( "Accessing <New module> dialog failed" )
                endcatch

            else
                warnlog( "Couldn't open Tools->Macros->Organize Macros...->StarOffice Basic..." )
            endif

        case"chart":
        case"DATABASE":
        case"soffice":
            brc = hDestroyDocument()
    end select

    printlog( "Retrieve the caption from the window" )
    sTemp = hGetWindowCaption(sApplication)
    printlog( "Caption is: " & sTemp )
    
    printlog( "Split up the string to isolate the desired part" )
    saTemp() = fSplitWindowTitle(sTemp)
    printlog "Filename/Untitled: '" & saTemp(0) & "'"

    printlog( "Compare the string to a reference" )
    if (ubound(saTemp()) > 0) then

        if (saTemp(1) <> sReference) then
            warnlog "Applicationname not as expected. Sould be: '" & sReference & "', is: '" & saTemp(1) & "'"
        endif

        printlog "Productname: '" & saTemp(2) & "'"
    else
        warnlog "#i36173# - Applicationname not as expected. Should be: '" & sReference & "', is: '" & "'"
    endif

    printlog( "Cleanup after test" )
    select case sApplication
        case"swriter":
        case"sglobal":
        case"sweb":
        case"scalc":
        case"simpress":
        case"sdraw":
        case"smath":
        case"basic":
            brc = hDestroyDocument()
        case"chart":
        case"DATABASE":
        case"soffice":
            hCreateDocument()
            hCreateDocument()
    end select

    printlog( "Close all open documents" )
    hFileCloseAll()
    
endcase

