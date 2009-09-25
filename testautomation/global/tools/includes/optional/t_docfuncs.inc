'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_docfuncs.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsk $ $Date: 2008-06-20 07:57:47 $
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
'* short description : misc functions to handle documents
'*
'\******************************************************************************

private const VERBOSE = FALSE

function hUseImpressAutopilot( bEnable as boolean ) as boolean

    '///<h3>Enable/disable the Impress Autopilot in Tools/Options</h3>
    '///<i>Starting point: Any plain document</i>
    '///<ul>
    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hUseImpressAutopilot::"
    
    if ( VERBOSE ) then printlog( CFN & "Enter" )

    '///+<li>Create a new IMPRESS document</li>
    gApplication = "IMPRESS"
    hCreateDocument()

    '///+<li>Open Tools/Options, go to Presentataion/General page</li>
    ToolsOptions
    hToolsOptions( gApplication , "General" )

    '///+<li>Check or uncheck to start with autopilot</li>
    Kontext "TabSonstigesDraw"
    if ( mitAutopilotStarten.exists( 2 ) ) then

        if ( bEnable ) then
            mitAutopilotStarten.check()
            printlog( CFN & "Enable Impress Autopilot" )
        else
            mitAutopilotStarten.uncheck()
            printlog( CFN & "Disable Impress Autopilot" )
        endif

        '///+<li>Return Status of Checkbox (checked=TRUE)</li>
        hUseImpressAutopilot() = mitAutopilotStarten.isChecked()

    else

        warnlog( CFN & "Cannot find Autopilot Checkbox" )

    endif

    '///+<li>Close Tools/Options</li>
    Kontext "OptionenDlg"
    OptionenDlg.OK()

    '///+<li>Close IMPRESS document</li>
    hDestroyDocument()

    '///+<li>Returncode is undefined</li>
    if ( VERBOSE ) then printlog( CFN & "Exit" )
    '///</ul>
    
end function

'*******************************************************************************

function hCloseNavigator() as boolean

    '///<h3>Function to close the navigator</h3>
    '///<i>Starting point: Any document</i>
    '///<ul>
    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hCloseNavigator::"
    
    if ( VERBOSE ) then printlog( CFN & "Enter" )
    
    '///+<li>close the navigator if found</li>
    Kontext "Navigator"
    if ( Navigator.Exists() ) then
        Navigator.Close()
        printlog( CFN & "Closed Navigator" )
    else
        printlog( CFN & "Navigator not present" )
    endif
    
    '///+<li>verify that the navigator is indeed closed, return true on success</li>
    Kontext "Navigator"
    if ( Navigator.exists() ) then
        hCloseNavigator() = false
        warnlog( CFN & "Failed to close Navigator, it is still open." )
    else
        hCloseNavigator() = true
    endif
    
    if ( VERBOSE ) then printlog( CFN & "Exit" )
    '///</ul>

end function

'*******************************************************************************

function hNumericDocType( doctype as integer ) as string

    '///<h3>Convert a numeric expression to a gApplication</h3>
    '///<i>BEWARE: This is a core function used by many testcases!</i><br>
    '///<u>Using file: global/input/applications.txt as reference</u>
    '///<ul>
    
    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hNumericDocType::"
    
    dim sFile as string
        sFile = gTesttoolPath & "global\input\applications.txt"
        sFile = convertpath( sFile )
       
    dim sList( 100 ) as string
    dim iDocTypes as integer
    
    if ( VERBOSE ) then printlog( CFN & "Enter" )
    
    '///+<li>Retrieve matching list from file</li>
    hGetDataFileSection( sFile , sList() , "application" , "" , "" )
    iDocTypes = val( sList( 0 ) )
    
    '///+<li>Verify that the functionparameter (documenttype) is valid</li>
    if ( ( doctype < 0 ) or ( doctype > iDocTypes ) ) then
        warnlog( CFN & "Undefined numeric doctype: " & doctype )
        hNumericDocType() = ""
        exit function
    endif
       
    '///+<li>Set gApplication to the requested document type</li>
    gApplication = hGetValueForKeyAsString( sList() , doctype )
    
    '///+<li>Set a plain text name as returnvalue for the function</li>
    hGetDataFileSection( sFile , sList() , "application_names" , "" , "" )
    hNumericDocType() = hGetValueForKeyAsString( sList() , gApplication )
    if ( VERBOSE ) then printlog( CFN & "Exit" )
    '///</ul>

end function

'*******************************************************************************

function hChangeDoc() as string

    '///<h3>Function to modify all documenttypes to set  the "changed" flag</h3>
    '///<i>Starting point: Any plain document</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>String, ready to use in printlog</li>
    '///<ul>
    '///+<li>&quot;Changed &lt;Documenttype&gt;&quot;</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    sleep()

    select case gApplication
   
      '///+<li>WRITER: Type some text</li>
      case "WRITER", "HTML", "MASTERDOCUMENT"
                      Kontext "DocumentWriter"
                      DocumentWriter.TypeKeys "<Return>This doc is changed!"
                      hChangeDoc() = "Changed WRITER/HTML/MASTERDOC" 

      '///+<li>CALC: Write some text into the current location (usually A1)</li>
      case "CALC"   : Kontext "DocumentCalc"
                      DocumentCalc.TypeKeys "<Down>This doc is changed!<Return>"
                      hChangeDoc() = "Changed CALC" 

      '///+<li>DRAW: Draw a rectangle</li>
      case "DRAW"   : Kontext "DocumentDraw"
                      hRechteckErstellen ( 20, 20, 25, 25 )
                      gMouseClick ( 1, 1 )
                      hChangeDoc() = "Changed DRAW" 

      '///+<li>IMPRESS: Draw a rectangle</li>
      case "IMPRESS": Kontext "DocumentImpress"
                      hRechteckErstellen ( 20, 20, 25, 25 )
                      gMouseClick ( 1, 1 )
                      hChangeDoc() = "Changed IMPRESS" 

      '///+<li>MATH: Create a simple formaula</li>
      case "MATH"   : SchreibenInMathdok( "b over c" )
                      hChangeDoc() = "Changed MATH" 

   end select
   '///</ul>
   
end function

'*******************************************************************************

function hIdentifyWriterDoc( cString as string, bWarn as boolean ) as boolean

    '///<h3>Function to validate that the expected WRITER doc has the focus</h3>
    '///<i>Starting point: Any WRITER document containing some text</i>
    '///<ul>
    
    use "global\tools\includes\optional\t_stringtools.inc"

    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hIdentifyWriterDoc::"
    dim cIdentifier as string
    dim irc as integer ' some temporary returnvalue
    
    if ( VERBOSE ) then printlog( CFN & "Enter" )

    '///+<li>Test the function parameter</li>
    if ( cString = "" ) then
        qaerrorlog( CFN & "Function parameter is empty string. Aborting" )
        hIdentifyWriterDoc() = false
        exit function
    endif

    '///+<li>Verify function prerequisites</li>
    kontext "DocumentWriter"
    if ( not DocumentWriter.exists() ) then
        printlog( CFN & "There is no Writer document. Aborting" )
        hIdentifyWriterDoc() = false
        exit function
    endif
    
    hIdentifyWriterDoc() = true

    '///+<li>Go to the very beginning of the current writer document</li>
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "<MOD1 HOME>" )
    
    '///+<li>Select the entire line</li>
    DocumentWriter.typeKeys( "<SHIFT END>" )
   
    '///+<li>copy the string to the clipboard and store it into a variable</li>
    try
	    editcopy
	    cIdentifier = getClipboardText()
	catch
	    warnlog( "EditCopy failed. Probably the document is empty" )
	    cIdentifier = "**** invalid ****"
	endcatch
    
   
    '///+<li>compare the string to the reference (given as parameter)</li>
    irc = hCompareSubstrings( cIdentifier, cString )

    '///+<li>Write the result to the printlog if desired</li>
    if ( irc = 0 ) then
   
        if ( bWarn ) then
            warnlog ( CFN & "The focus is not on the expected document" )
        else
            printlog( CFN & "The focus is not on the expected document" )
        endif
        
        printlog( CFN & "Expected: " & cString )
        printlog( CFN & "Found...: " & cIdentifier )
        hIdentifyWriterDoc() = false
      
    else
        printlog( CFN & "OK: The document contains the requested string" )
    endif
    '///+<li>Return TRUE only if the current document is the expected one</li>
    
    if ( VERBOSE ) then printlog( CFN & "Exit" )
    '///</ul>

end function

'*******************************************************************************

function hCreateDocument() as boolean

    '///<h3>Create anew document, extends hNewDocument()</h3>
    '///<i>Starting point: Any document</i><br>
    '///<i>Note: When calling this function with gApplication "BACKGROUND" the
    '///+ function will actually close one document to get to the backing window</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing, valid gApplication must be set</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE if a new document was created successfully</li>
    '///+<li>FALSE if anything went wrong</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hCreateDocument::"

    dim iDocCountBefore as integer 
    dim iDocCountAfter as integer

    '///+<li>Retrieve the number of open documents</li>
    iDocCountBefore = getDocumentCount
    if ( VERBOSE ) then printlog( CFN & "Number of open documents before: " & iDocCountBefore )

    '///+<li>Special treatment if gApplication = BACKGROUND (virtual gApplication)</li>
    if ( ucase( gApplication ) = "BACKGROUND" ) then
    	do while( getDocumentCount > 0 )
    	    hCloseDocument()
        loop
        hCreateDocument() = true
    else
    
        '///+<li>Open a new document using hNewDocument()</li>
        call hNewDocument()

        '///+<li>Retrieve the number of open documents</li>
        iDocCountAfter = getDocumentCount()

        '///+<li>Verify that there is exactly one more document than before</li>
        if ( iDocCountAfter = ( iDocCountBefore + 1 ) ) then
            hCreateDocument() = true
        else
            hCreateDocument() = false
        endif
    endif
    printlog( CFN & "New document created. Open: " & iDocCountAfter )
    '///</ul>
    
end function

'*******************************************************************************

function hDocumentTypeKeys( cString as string ) as boolean

    '///<h3>Type a string to a document</h3>
    '///<i>The documenttype is determined by gApplication.<br>
    '///See hNumericDocType for details and number/gApplication matching</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Keystrokes to be sent to the document (string), e.g.</li>
    '///<ul>
    '///+<li>&quot;&lt;SHIFT HOME&gt;&quot;</li>
    '///+<li>&quot;&lt;MOD1 A&gt;&quot;</li>
    '///+<li>&quot;&lt;MOD2 Q&gt;&quot;</li>
    '///+<li>&quot;Hello&quot;</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE if all went well</li>
    '///+<li>FALSE on any error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hDocumentTypeKeys::"
    dim brc as boolean
        brc = true
        
    if ( VERBOSE ) then printlog( CFN & "Enter" )
    
    '///+<li>Set focus to the current documenttype determined by gApplication</li>
    '///+<li>Type the string passed as function parameter</li>
    select case gApplication
    case "WRITER"   :   kontext "DocumentWriter"
                        DocumentWriter.typeKeys( cString )
    case "CALC"     :   kontext "DocumentCalc"
                        DocumentCalc.typeKeys( cString )
    case "IMPRESS"  :   kontext "DocumentImpress"
                        DocumentImpress.typeKeys( cString )
    case "DRAW"     :   kontext "DocumentDraw"
                        DocumentDraw.typeKeys( cString )
    case "MATH"     :   kontext "DocumentMath"
                        DocumentMath.typeKeys( cString )
    case "MASTERDOCUMENT":   kontext "DocumentWriter"
                        DocumentWriter.typeKeys( cString )
    case "HTML"     :   kontext "DocumentWriter"
                        DocumentWriter.typeKeys( cString )
    case else
        warnlog( CFN & "Exit: Unknown gApplication: " & gApplication )
        brc = false
    end select
    
    '///+<li>Print some message to the log on success</li>
    if ( brc ) then
        printlog( CFN & "Exit: Sent keystroke to " & gApplication )
    endif
    
    hDocumentTypeKeys() = brc
    '///</ul>
    
end function


'*******************************************************************************

function hDestroyDocument() as boolean

    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hDestroyDocument: "
    dim iDocCount as integer : iDocCount = getDocumentCount()
    hDestroyDocument() = false
    
    if ( iDocCount = 0 ) then exit function
    if ( VERBOSE ) then printlog( CFN & "In.: Open: " & iDocCount )

    hUseAsyncSlot( "FileClose" )
    
    Kontext "Active"
    if ( Active.Exists( 2 ) ) then
        printlog( "Msgbox: " & Active.getText() )
        try
            Active.No
        catch
            Active.Click ( 202 )
        endcatch
        wait( 1000 )
    end if
    
    if ( getDocumentCount() = iDocCount ) then hCloseDocument()
    if ( getDocumentCount() = iDocCount - 1 ) then hDestroyDocument() = true
    if ( VERBOSE ) then printlog( CFN & "Out: Open: " & getDocumentCount() )

end function

'*******************************************************************************

function hInitSingleDoc() as boolean

    '///<h3>Make sure exactly one single writer document is open</h3>
    '///<i>The wizards cannot be triggered correctly from the backing window. 
    '///+ As a workaround this function checks the amount of open documents and
    '///+ creates exactly one unchanged Writer document</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>
    '///+<li>No input parameters</li>
    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE: Exactly one Writer document is open</li>
    '///+<li>FALSE: Any error</li>
    '///</ul>
    '///</ol>

    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hInitSingleDoc::"
    dim cOldApplication as string

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Close all documents until we are on the backing window</li>
    do while ( getDocumentCount > 0 ) 
        call hCloseDocument()
    loop
    
    '///+<li>Save the previous gApplication</li>
    cOldApplication = gApplication
    
    '///+<li>Set gApplication to WRITER</li>
    gApplication = "WRITER"
    
    '///+<li>Open one new Writer document</li>
    call hNewDocument()
    
    '///+<li>Verify that exactly one document is open</li>
    if ( getDocumentCount = 1 ) then 
        printlog( CFN & "A single unchanged writer document is open" )
        hInitSingleDoc() = true
    else
        printlog( CFN & "Failed to open just one single writer document" )
        hInitSingleDoc() = false
    endif
    
    '///+<li>Restore gApplication</li>
    gApplication = cOldApplication
    '///</ul>

end function

'*******************************************************************************

function hInitBackingMode() as boolean

    '///<h3>Make that we are on the backing window (no open documents)</h3>
    '///<i>Close all open documents</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>
    '///+<li>No input parameters</li>
    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE: No open documents are present</li>
    '///+<li>FALSE: Any error</li>
    '///</ul>
    '///</ol>

    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hInitBackingMode::"

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Close all documents until we are on the backing window</li>
    do while ( getDocumentCount > 0 ) 
        hCloseDocument()
    loop
    
    '///+<li>verify that we do not have any open documents left (redundant check)</li>
    if ( getDocumentCount = 0 ) then
        printlog( CFN & "Office is in backing mode." )
        hInitBackingMode() = true
    else
        printlog( CFN & "Office is in undefined state." )
        hInitBackingMode() = false
    endif
    '///</ul>
    
end function

'*******************************************************************************

function hInitWriteDocIdentifier( cString as string ) as boolean


    '///<h3>Write a specific string to an open writer document</h3>
    '///<i>This function verifies that exactly one document is open, that this is a 
    '///+ Writer document and writes the string to the document</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>A free form string (String) which serves as identifier for the document</li>
    '///<ul>
    '///+<li>The first character should be uppercase</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE: The string was written correctly</li>
    '///+<li>FALSE: Too many open documents</li>
    '///+<li>FALSE: Not a Writer document</li>
    '///+<li>FALSE: Any other error</li>
    '///</ul>
    '///</ol>

    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hInitWriteDocIdentifier::"

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Verify number of open documents</li>
    if ( getDocumentCount <> 1 ) then
        printlog( CFN & "Incorrect document count" )
        hInitWriteDocIdentifier() = false
        exit function
    endif
    
    '///+<li>Verify that it is a writer document</li>
    kontext "DocumentWriter"
    if ( not DocumentWriter.exists() ) then
        printlog( CFN & "Open document is not a text document" )
        hInitWriteDocIdentifier() = false
        exit function
    endif
    
    '///+<li>Write the string</li>
    kontext "DocumentWriter"
    DocumentWriter.typeKeys( "<MOD1 END>" )
    DocumentWriter.typeKeys( "<MOD1 SHIFT HOME>" )
    DocumentWriter.typeKeys( "<DELETE>" )
    DocumentWriter.typekeys( cString )
    
    '///+<li>Verify the string</li>
    DocumentWriter.typeKeys( "<MOD1 END>" )
    DocumentWriter.typeKeys( "<MOD1 SHIFT HOME>" )
    EditCopy
    if ( getClipboardText = cString ) then
        printlog( CFN & "Document has been successfully modified." )
        hInitWriteDocIdentifier() = true
    else
        printlog( CFN & "Could not verify document identification string" )
        hInitWriteDocIdentifier() = false
    endif    
    '///</ul>

end function
