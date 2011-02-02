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
'* short description : misc functions to handle documents
'*
'\******************************************************************************

function hUseImpressAutopilot( bEnable as boolean ) as boolean

    '///<h3>Enable/disable the Impress Autopilot in Tools/Options</h3>
    '///<i>Starting point: Any plain document</i>
    '///<ul>
    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hUseImpressAutopilot::"
    
    if ( GVERBOSE ) then printlog( CFN & "Enter" )

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
            if ( GVERBOSE ) then printlog( CFN & "Enable Impress Autopilot" )
        else
            mitAutopilotStarten.uncheck()
            if ( GVERBOSE ) then printlog( CFN & "Disable Impress Autopilot" )
        endif

        '///+<li>Return Status of Checkbox (checked=TRUE)</li>
        hUseImpressAutopilot() = mitAutopilotStarten.isChecked()

    else

        warnlog( CFN & "Cannot find Autopilot Checkbox" )

    endif

    '///+<li>Close Tools/Options</li>
    Kontext "OptionenDlg"
    hCloseDialog( OptionenDlg, "ok" )
    
    '///+<li>Close IMPRESS document</li>
    hDestroyDocument()

    '///+<li>Returncode is undefined</li>
    if ( GVERBOSE ) then printlog( CFN & "Exit" )
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
    
    if ( GVERBOSE ) then printlog( CFN & "Enter" )
    
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
    if ( GVERBOSE ) then printlog( CFN & "Exit" )
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
                      kontext "DocumentMath"
                      hChangeDoc() = "Changed MATH"
                      gMouseClick( 50 , 50 )

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
    
    if ( GVERBOSE ) then printlog( CFN & "Enter" )

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
    hUseAsyncSlot( "EditCopy" )
    cIdentifier = getClipboardText()
   
    '///+<li>compare the string to the reference (given as parameter)</li>
    irc = hCompareSubstrings( cIdentifier, cString )

    '///+<li>Write the result to the printlog if desired</li>
    if ( irc = 0 ) then

        if ( GVERBOSE ) then printlog( CFN & "The focus is not on the expected document" )
        if ( bWarn ) then warnlog ( CFN & "The focus is not on the expected document" )
        
        printlog( CFN & "Expected: " & cString )
        printlog( CFN & "Found...: " & cIdentifier )
        hIdentifyWriterDoc() = false
      
    else
        printlog( CFN & "OK: The document contains the requested string" )
    endif
    '///+<li>Return TRUE only if the current document is the expected one</li>
    
    if ( GVERBOSE ) then printlog( CFN & "Exit" )
    '///</ul>

end function

'*******************************************************************************

function hCreateDocument() as boolean

    '///<h3>Create anew document, extends hNewDocument() by providing a returnvalue</h3>
    ' TRUE = New document created, FALSE = Failed to create a new document '
    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hCreateDocument::"

    dim iDocCountBefore as integer 
    dim iDocCountAfter as integer

    iDocCountBefore = getDocumentCount
    if ( GVERBOSE ) then printlog( CFN & "Number of open documents before: " & iDocCountBefore )

    if ( gApplication = "BACKGROUND" ) then
    	hFileCloseAll()
        hCreateDocument() = true
    else
    
        call hNewDocument()
        iDocCountAfter = getDocumentCount()
        if ( iDocCountAfter = ( iDocCountBefore + 1 ) ) then
            hCreateDocument() = true
        else
            hCreateDocument() = false
        endif
    endif
    if ( GVERBOSE ) then printlog( CFN & "New document created. Open: " & iDocCountAfter )
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
        
    if ( GVERBOSE ) then printlog( CFN & "Enter" )
    hDocumentTypeKeys() = true
    
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
        hDocumentTypeKeys() = false
        exit function
    end select
    
    '///+<li>Print some message to the log on success</li>
    printlog( CFN & "Exit: Sent keystroke to " & gApplication )
    '///</ul>
    
end function


'*******************************************************************************

function hDestroyDocument() as boolean

    const CFN = "global::tools::includes::optional::t_docfuncs.inc::hDestroyDocument: "
    dim iDocCount as integer : iDocCount = getDocumentCount()
    hDestroyDocument() = false

    dim method( 3 ) as string
        method( 1 ) = "no,optional"
        method( 2 ) = "202,optional"
        method( 3 ) = "ok,optional"  ' This could be a File I/O error

    dim iCurrentMethod as integer
    dim bDialogClosed as boolean
    
    if ( iDocCount = 0 ) then exit function
    if ( GVERBOSE ) then printlog( CFN & "In.: Open: " & iDocCount )

    hUseAsyncSlot( "FileClose" )

    for iCurrentMethod = 1 to 3

        bDialogClosed = false

        Kontext "Active"
        if ( Active.exists( 1 ) ) then
            printlog( CFN & "Msgbox: " & Active.getText() )
            if ( GVERBOSE ) then printlog( CFN & "Method: " & method( iCurrentMethod ) )
            bDialogClosed = hCloseDialog( Active, method( iCurrentMethod ) )
        else
            exit for
        endif
        
        if ( iCurrentMethod = 3 and bDialogClosed ) then 
            warnlog( "Unexpected messagebox was handled" )
        endif
        
    next iCurrentMethod
    
    if ( getDocumentCount() = iDocCount ) then hCloseDocument()
    if ( getDocumentCount() = iDocCount - 1 ) then hDestroyDocument() = true
    if ( GVERBOSE ) then printlog( CFN & "Out: Open: " & getDocumentCount() )

end function

'*******************************************************************************

function hInitSingleDoc() as boolean

    hFileCloseAll()
    hCreateDocument()

    if ( GVERBOSE ) then printlog( "hInitSingleDoc(): Opening document: " & gApplication )

    if ( getDocumentCount = 1 ) then
        hInitSingleDoc() = true
    else
        hInitSingleDoc() = false
    endif

end function

'*******************************************************************************'*******************************************************************************

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
    const RC_TIMEOUT = -1

    if ( getDocumentCount = 1 and gApplication = "WRITER" ) then

        kontext "DocumentWriter"
        DocumentWriter.typeKeys( "<MOD1 END>" )
        DocumentWriter.typeKeys( "<MOD1 SHIFT HOME>" )
        DocumentWriter.typeKeys( "<DELETE>" )
        DocumentWriter.typekeys( cString )

        DocumentWriter.typeKeys( "<MOD1 END>" )
        DocumentWriter.typeKeys( "<MOD1 SHIFT HOME>" )

        if ( hUseAsyncSlot( "EditCopy" ) <> RC_TIMEOUT ) then

            if ( getClipboardText = cString ) then
                printlog( CFN & "Writer document has been successfully modified." )
                hInitWriteDocIdentifier() = true
            else
                warnlog( CFN & "Could not verify document identification string" )
                hInitWriteDocIdentifier() = false
            endif
        else
            warnlog( CFN & "Unable to copy text to clipboard using <EditCopy>" )
            hInitWriteDocIdentifier() = false
        endif
    else
        warnlog ( CFN & "Only one single writer document may be open"   )
        printlog( CFN & "Number of documents: " & getDocumentCount )
        printlog( CFN & "Application........: " & gApplication     )
        hInitWriteDocIdentifier() = false
    endif

end function
