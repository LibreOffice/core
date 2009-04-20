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

    if ( gtSysName = "Mac OS X" ) then
        printlog( "No testing for Mac as some Window attributes do not exist" )
        goto endsub
    endif

    '///<h1>Update test for window functionality</h1>
    '///<ul>
    dim brc as boolean
    
    '///+<li>Verify that we have one initial document open</li>
    printlog( "Create initial document" )
    gApplication = "WRITER"
    hInitSingleDoc()
    hInitWriteDocIdentifier( "F_updt_windowfuncs.bas" )
    
    '///+<li>Create a new document</li>
    printlog( "New document" )
    hCreateDocument()
    if ( getDocumentCount <> 2 ) then
        warnlog( "Two open documents were expected, found " & getDocumentCount )
    endif    

    DocumentWriter.restore()

    '///+<li>Use .close()-method to close document</li>
    kontext "documentwriter"
    printlog( "Close document" )
    DocumentWriter.close()

    '///+<li>Verify that the document is closed</li>
    if ( getDocumentCount <> 1 ) then
        warnlog( "One open document was expected, found " & getDocumentCount )
    endif

    '///+<li>Create a new document</li>
    printlog( "New document" )
    hCreateDocument()
    if ( getDocumentCount <> 2) then
        warnlog( "Two open documents were expected, found " & getDocumentCount )
    endif    

    '///+<li>Write some text into the second writer document</li>
    '///+<li>Use .close()-method to close document</li>
    printlog( "Enter some text into the second writer document" )
    kontext "documentwriter"
    DocumentWriter.TypeKeys( "test" )
    DocumentWriter.close()

    '///+<li>Close the message (Document changed)</li>
    kontext "active"
    if ( Active.exists() ) then
        Active.Cancel()
        printlog( "Close messagebox" )
    else
        warnlog( "No warning that data will be lost on close of this document" )
    endif
    
    '///+<li>Verify that exactly two documents are open</li>
    if ( getDocumentCount = 2 ) then
        printlog( "One document open. Good." )
    else
        warnlog( "Incorrect document count. Expected two, found " & getDocumentCount )
    endif

    '///+<li>Close the document using FileClose</li>
    hDestroyDocument() ' note tha hCloseDoc/hDestroyDocument use FileClose
    
    '///+<li>Verify that exactly one document is open</li>
    if ( getDocumentCount = 1 ) then
        printlog( "One document open. Good." )
    else
        warnlog( "Incorrect document count. Expected one, found " & getDocumentCount )
    endif    

    '///+<li>Create a new document</li>
    printlog( "New document" )
    brc = hCreateDocument()
    if ( not brc ) then
        warnlog( "Two open documents were expected, found " & getDocumentCount() )
    endif    


    kontext "documentwriter"
    '///+<li>Maximize Window</li>
    '///<ul>
    printlog( "Maximize window" )
    DocumentWriter.Maximize()
    sleep( 2 )
    
    '///+<li>Verify state: Maximized (true)</li>
    if ( DocumentWriter.IsMax() ) then
        printlog( " * Window is maximized" )
    else
        warnlog( " * Window not maximized" )
    endif


    '///+<li>Verify state: Minimized (false)</li>
    if ( DocumentWriter.IsMin() ) then
        warnlog( " * Window is minimized" )
    else
        printlog( " * Window not minimized" )
    endif

    '///+<li>Verify state: Restored (false)</li>
    if ( DocumentWriter.IsRestore() ) then
        warnlog( " * Window is Restored" )
    else
        printlog( " * Window not Restored" )
    endif
    '///</ul>

    '///<li>Minimize Window</li>
    '///<ul>
    printlog( "Minimize window" )
    DocumentWriter.Minimize()
    sleep( 2 )

    '///+<li>Verify state: Minimized (true)</li>
    if ( DocumentWriter.IsMin() ) then
        printlog( " * Window is minimized" )
    else
        qaerrorlog( "#i32672# Window not minimized" )
    endif

    '///+<li>Verify state: Maximized (false)</li>
    if ( DocumentWriter.IsMax() ) then
        warnlog( " * Window is maximized" )
    else
        printlog( " * Window not maximized" )
    endif

    '///+<li>Verify state: Restored (false)</li>
    if ( DocumentWriter.IsRestore() ) then
        qaerrorlog( "#i32672# Window is Restored" )
    else
        printlog( " * Window not Restored" )
    endif
    '///</ul>

    '///<li>Restore Window</li>
    '///<ul>
    printlog( "Restore window" )
    DocumentWriter.Restore()
    sleep( 2 )

    '///+<li>Verify state: Restored (true)</li>
    if ( DocumentWriter.IsRestore() ) then
        printlog( " * Window is Restored" )
    else
        warnlog( " * Window not Restored" )
    endif

    '///+<li>Verify state: Maximized (false)</li>
    if ( DocumentWriter.IsMax() ) then
        warnlog( " * Window is maximized" )
    else
        printlog( " * Window not maximized" )
    endif

    '///+<li>Verify state: Minimized (false)</li>
    if ( DocumentWriter.IsMin() ) then
        warnlog( " * Window is minimized" )
    else
        printlog( " * Window not minimized" )
    endif

    '///</ul>
    
    '///+<li>Close the documents</li>
    brc = hDestroyDocument()
    brc = hDestroyDocument()

    '///</ul>

endcase

'*******************************************************************************

sub sAllWindowTitle

    '///<h3>Update test: Check the window titles for all applications</h3>
    '///<ul>
    

    '///+<li>writer</li>
    gApplication = "WRITER"
    call tCheckWindowTitle("swriter","WRITER")

    '///+<li>MasterDoc</li>
    gApplication = "MASTERDOCUMENT"
    call tCheckWindowTitle("sglobal","WRITER")

    '///+<li>HTML document</li>
    gApplication = "HTML"
    call tCheckWindowTitle("sweb","Writer/Web")

    '///+<li>Calc</li>
    gApplication = "CALC"
    call tCheckWindowTitle("scalc","CALC")

    '///+<li>Impress</li>
    gApplication = "IMPRESS"
    call tCheckWindowTitle("simpress","IMPRESS")

    '///+<li>Draw</li>
    gApplication = "DRAW"
    call tCheckWindowTitle("sdraw","DRAW")

    '///+<li>Math</li>
    gApplication = "MATH"
    call tCheckWindowTitle("smath","MATH")

    qaerrorlog( "Excluded BASE and BACKINGWINDOW" )

    'gApplication = "DATABASE"
    'call tCheckWindowTitle("DATABASE","Base")

    'fileclose
    'call tCheckWindowTitle("soffice","")
    '///</ul>
end sub

'*******************************************************************************

testcase tCheckWindowTitle(sApplication as string, sReference as string)

    '///<h1>Update test for the office window titles</h1>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of application (string), case sensitive. Valid options are:</li>
    '///<ol>
    '///+<li>&quot;swriter&quot;</li>
    '///+<li>&quot;sglobal&quot;</li>
    '///+<li>&quot;sweb&quot;</li>
    '///+<li>&quot;scalc&quot;</li>
    '///+<li>&quot;simpress&quot;</li>
    '///+<li>&quot;sdraw&quot;</li>
    '///+<li>&quot;smath&quot;</li>
    '///+<li>&quot;insight&quot; * Currently disabled</li>
    '///+<li>&quot;soffice&quot; * Currently disabled</li>
    '///</ol>
    '///+<li>Reference name (string), matches name of application</li>
    '///<ol>
    '///+<li>&quot;Writer&quot;</li>
    '///+<li>&quot;Writer&quot;</li>
    '///+<li>&quot;Writer/Web&quot;</li>
    '///+<li>&quot;Calc&quot;</li>
    '///+<li>&quot;Impress&quot;</li>
    '///+<li>&quot;Draw&quot;</li>
    '///+<li>&quot;Math&quot;</li>
    '///+<li>&quot;Base&quot; * Currently disabled</li>
    '///+<li>&quot;&quot; * Currently disabled</li>
    '///</ol>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    dim sTemp as string
    dim saTemp() as string
    dim brc as boolean
    gApplication = UCase (gApplication)
    printlog " - Application: " + sApplication + "; Title should be: " + sReference
    '///+<li>Make sure exactly one document is open</li>
    hInitSingleDoc()
    
    '///+<li>Open a new document</li>
    hCreateDocument()

    '///+<li>Ensure the correct window is open (e.g. goto Basic IDE)</li>
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
                        ' OK :-) here we want to stay
                        printlog "OK :-)"
                    else
                        ' qaErrorlog "Unexpected 2"
                    endif

                else
                    qaErrorlog "Unexpected 1"
                endif


                try

                    kontext "neuesmodul"

                    if NeuesModul.exists(5) then
                        NeuesModul.OK
                    else
                        qaErrorlog "Unexpected 3"
                    endif

                catch
                endcatch

            else
                qaErrorlog "Couldn't open Tools->Macros->Organize Macros...->StarOffice Basic..."
            endif

        case"chart":
        case"DATABASE":
        case"soffice":
            brc = hDestroyDocument()
    end select

    '    sTemp = hWindowGetText("")   ' was a wOrkAround until the next line worked
    '///+<li>Retrieve the caption</li>
    sTemp = hGetWindowCaption(sApplication)
    '    printlog sTemp
    ' seperate each part of the title
    
    '///+<li>Split up the string, we just need a s mall part of the string</li>
    saTemp() = fSplitWindowTitle(sTemp)
    printlog "Filename/Untitled: '" + saTemp(0) + "'"

    '///+<li>Compare the string to the reference, warn on error</li>
    if (ubound(saTemp()) > 0) then

        if (saTemp(1) <> sReference) then
            warnlog "Applicationname not as expected. Sould be: '" + sReference + "', is: '" + saTemp(1) + "'"
        endif

        printlog "Productname: '" + saTemp(2) + "'"
    else
        warnlog "#i36173# - Applicationname not as expected. Should be: '" + sReference + "', is: '"+"'"
    endif

    '///+<li>Cleanup</li>
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

    '///+<li>Close the document</li>
    while( getDocumentCount > 0 ) 
        hDestroyDocument()
    wend
    '///</ul>
    
endcase

