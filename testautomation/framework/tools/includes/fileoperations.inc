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
'* short description : check the internal file dialog ( extended tests )
'*
'\******************************************************************************

function hSaveLoadDelSuccess( cFile as string ) as integer

    '///<h3>Successfully save, close, load, close and delete a file</h3>
    '///<i>Uses</i>: framework\tools\t_stringtools.inc<br><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename incl. extension (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcode (integer)</li>
    '///<ul>
    '///+<li>0  = all ok</li>
    '///+<li>1  = Saving failed</li>
    '///+<li>2  = Closing the file failed</li> 
    '///+<li>3  = Reloading failed</li>
    '///+<li>4  = Closing the file failed</li>
    '///+<li>5  = Deleting failed</li>
    '///+<li>-1 = Post operation error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    dim brc as boolean
    dim cFileExt as string : cFileExt = cFile & hGetSuffix( "current" )
    
    const CFN = "hSaveLoadDelSuccess::"
    
    printlog( "" )
    printlog( CFN & "Enter with option: " & cFile & "/" & cFileExt )
    
    '///+<li>Close the navigator if it exists</li>
    kontext "Navigator"
    hCloseDialog( Navigator, "close,optional" )

    '///+<li>Save the current file, overwriting existing</li>
    brc = hSaveFileExpectSuccess( cFile , TRUE ) ' save and overwrite
    if ( brc ) then

        '///+<li>Close the file</li>
        brc = hDestroyDocument()
        if ( brc ) then
        
            '///+<li>Reload the file</li>
            brc = hLoadFileExpectSuccess( cFileExt )
            if ( brc ) then
            
                '///+<li>Close the document</li>
                brc = hDestroyDocument()
                if ( brc ) then
                
                    '///+<li>Delete the file via FileOpen</li>
                    brc = hDeleteFileViaFileOpen( cFileExt )
                    if ( brc ) then
                        printlog( CFN & "Save, close, load, close, delete ok" )
                        hSaveLoadDelSuccess() = 0
                    else
                        warnlog( CFN & "Failed to delete file" )
                        hSaveLoadDelSuccess() = 5
                    endif
                
                else
                    warnlog( CFN & "Failed to close file" ) 
                    hSaveLoadDelSuccess() = 4
                endif
            
            else
                warnlog( CFN & "Failed to load file" )
                hSaveLoadDelSuccess() = 3
            endif
        
        
        else
            warnlog( CFN & "Closing file failed" )
            hSaveLoadDelSuccess() = 2
        endif
    
    else
        warnlog( CFN & "Saving failed" )
        hSaveLoadDelSuccess() = 1
    endif
    
    '///+<li>Close possible Messagebox (#i33946#)</li>
    kontext "active"
    if ( active.exists( 1 ) ) then
        printlog( CFN & "Unexpected message: " & active.getText() )
        qaerrorlog( "#i33946# - message when deleting last document in folder" )
        active.ok()
        hSaveLoadDelSuccess() = 6
    endif
    
    '///+<li>Close document</li>
    brc = hDestroyDocument()

    '///</ul>

end function

'*******************************************************************************

function hLoadFileExpectSuccess( fpath as string ) as boolean

    '///<h3>Load a file where failure is expected</h3>
    '///<i>Uses</i>: framework\tools\t_stringtools.inc<br><br>
    '///<i>This function is quite similar to hLoadFile but does much less
    '///+ errorhandling so the information of the type of failure is a little
    '///+ more exact</i><br><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename incl. extension (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE = File was loaded without problems</li>
    '///+<li>FALSE = Any error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    dim brc as boolean : brc = true
        
    const CFN = "hLoadFileExpectSuccess::"
    printlog( CFN & "Enter with option: " & fpath )

    '///+<li>Click FileOpen (or use the menu)</li>
    hUseAsyncSlot( "FileOpen" )

    '///+<li>Enter the filename (with extension)</li>
    'printlog( " - Type the filepath/name into the entryfield" )
    Kontext "OeffnenDLG"
    if ( OeffnenDlg.exists( 1 ) ) then
        DateiName.setText( fpath )

        '///+<li>Click &quot;Open&quot;</li>
        'printlog( " - Click 'Open'" )
        oeffnen.click()    

        '///+<li>Watch out for an unexpected messagebox<br>
        Kontext "Active"
        if ( Active.Exists( 1 ) ) then
            printlog( "Unexpected active: " & active.getText() )
            Active.OK()

            Kontext "OeffnenDLG"
            OeffnenDLG.cancel()
            brc = false
        endif

        '///+recover in case of error so the test has a chance to continue</li>
        ' try to recover in case of failure so the test can continue.
        Kontext "OeffnenDLG"
        if ( Oeffnen.exists( 1 ) ) then
            warnlog( "The file was not opened, it doesn't appear to exist" )
            OeffnenDLG.cancel()
            brc = false
        endif
    else
        warnlog( CFN & "File Open dialog did not open" )
    endif

    '///+<li>Return the errorcode</li>
    printlog( CFN & "Exit with result: " & brc )
    hLoadFileExpectSuccess() = brc
    '///</ul>
    
end function

'*******************************************************************************

function hSaveFileExpectSuccess( fpath as string , bReplace as boolean ) as boolean

    '///<h3>Save a file with optional replace where success is expected</h3>
    '///<i>Uses</i>: framework\tools\t_stringtools.inc<br><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename incl. extension (string)</li>
    '///+<li>Replace file (boolean)</li>
    '///<ul>
    '///+<li>TRUE = Replace the file</li>
    '///+<li>FALSE = Do not replace the file</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE = There was an error loading the file (as expected)</li>
    '///+<li>FALSE = The file was loaded without problems/any other error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    const CFN = "hSaveFileExpectSuccess::" 
    dim brc as boolean : brc = true
        
    dim iDocumentCount as integer
    
    printlog( CFN & "Enter with options: " & fpath & ", " & bReplace )

    '///+<li>Make sure we start from the backing window</li>
    hFileCloseAll()


    '///+<li>Open a new document</li>
    hCreateDocument()

    '///+<li>Click &quot;Save As...&quot;</li>
    hUseAsyncSlot( "FileSaveAs" )

    Kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then

        '///+<li>Enter a filename (with extension)</li>
        Dateiname.setText( hGetWorkPath() & fpath )

        '///+<li>Click &quot;Save&quot;</li>
        speichern.click()

        '///+<li>If the file exists, say &quot;yes&quot; to replace it</li>
        if ( bReplace ) then
            Kontext "Active"
            if ( active.exists( 2 ) ) then
                printlog( CFN & "Overwriting file" )
                printlog( "Message: " & active.getText() )
                try
                    active.yes()
                catch
                    warnlog( CFN & "Unexpected active - no YES button available." )
                    printlog( CFN & "if any this should have been overwrite warning." )
                endcatch
            endif
        endif

        '///+<li>Handle any unexpected errormessage with &quot;OK&quot;</li>
        Kontext "Active"
        if ( active.exists( 2 ) ) then
            printlog( CFN & "Unexpected active: " & active.getText() )
            brc = false
            active.ok()
        endif
        
        '///+<li>The FileSave dialog should be closed at this point</li>
        kontext "SpeichernDlg"
        if ( SpeichernDlg.exists( 1 ) ) then
            warnlog( CFN & "File Save dialog is still open, it should be closed" )
            SpeichernDlg.cancel()
        endif
    else
        warnlog( CFN & "Failed to open File Open dialog" )
        brc = false
    endif

    '///+<li>Verify that exactly one document is open</li>
    if( getDocumentCount <> 1  ) then        
        warnlog( CFN & "Incorrect number of open documents" )
        brc = false
    endif
    
    printlog( CFN & "Exit with result: " & brc )
    hSaveFileExpectSuccess() = brc
    '///</ul>

end function

'*******************************************************************************

function hSaveFileExpectFailure( fpath as string , errortype as integer ) as boolean

    '///<h3>Save a file where failure is expected</h3>
    '///<i>Uses</i>: framework\tools\t_stringtools.inc<br><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename incl. extension (string)</li>
    '///+<li>Errortype (Integer). Valid options are:</li>
    '///<ul>
    '///+<li>0 = Invalid characters in string</li>
    '///+<li>1 = Filename is interpreted as device</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE = <b><i>There was an error saving the file (as expected)</i></b></li>
    '///+<li>FALSE = The file was saved without problems/any other error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    ' currently we have two different kinds of failure
    ' 1. File cannot be saved due to invalid character(s)
    '    This is errortype = 0
    ' 2. File cannot be saved because the given name is interpreted as device
    '    This is errortype = 1
    ' This sequence tries to save a document with an invalid name. The errormsg
    ' is closed, the filedialog cancelled and the file closed. 
    
    dim brc as boolean : brc = false
        
    dim cMsg as string
        
    const CFN = "hSaveFileExpectFailure::"

    '///+<li>Open an new document</li>
    hCreateDocument()
    
    '///+<li>Click &quot;Save As;&quot;</li>
    printlog( "" )
    printlog( CFN & "Enter with options: " & fpath & ", " & errortype )
    hUseAsyncSlot( "FileSaveAs" )
    
    '///+<li>Enter the filename</li>
    kontext "SpeichernDlg"
    if ( SpeichernDlg.exists( 1 ) ) then
    
        Kontext "SpeichernDlg"
        Dateiname.setText( fpath )

        '///+<li>Click &quot;Save&quot;</li>
        speichern.click()

        '///<ul>
        '///+<li>Handle invalid characters (Errormessages)</li>
        select case( errortype )
        case 0: 
        
        	brc = false
        
        	Kontext "Active"
        	if ( Active.exists( 1 ) ) then
        	
        		printlog( CFN & "Check for possible overwrite warning..." )
        		try
        			Active.yes()
        			printlog( CFN & "Closed Messagebox with <YES>" )
        			printlog( CFN & "This was the overwrite warning" )
        	    catch
        	    endcatch
        	endif

        	Kontext "Active"
        	if ( Active.exists( 1 ) ) then
        	    cMsg = active.getText()
        		try
        			Active.ok()
        			printlog( CFN & "Closed Save-Failure warning with <OK>" )
        			brc = true
        	    catch
        	    	qaerrorlog( CFN & "Unknown dialog encountered, <OK> failed:" )
        	    	printlog( cMSG )
        	    endcatch
        	endif
        	
        	
        case 1: 

            Kontext "Active"
            if ( Active.exists( 1 ) ) then
                printlog( CFN & "Expected active: " & active.getText() )
                Active.OK()

                Kontext "SpeichernDLG"
                if ( SpeichernDlg.exists( 1 ) ) then
                    SpeichernDLG.cancel()
                    brc = true
                else
                    warnlog( CFN & "File Save dialog is missing. Bad!" )
                    brc = false
                endif
            else
                warnlog( CFN & "Errormessage for '/', '\' or ':' is missing" )
                brc = false
            endif
            
        end select

    else
        warnlog( CFN & "Failed to open File Save dialog" )
    endif
    '///</ul>

    '///+<li>Close the document</li>
    hDestroyDocument()
    
    '///+<li>Return errorcondition</li>
    printlog( CFN & "Exit with status: " & brc )
    hSaveFileExpectFailure() = brc
    
    '///</ul>
    
end function

'*******************************************************************************

function hDeleteFileViaFileOpen( cFile as String ) as boolean

    '///<h3>Delete a file using the File Open dialog</h3>
    '///<i>Uses</i>: framework\tools\t_stringtools.inc<br><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename incl. extension (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE = File was deleted</li>
    '///+<li>FALSE = Any other condition</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    ' This sub deletes a file by name. No errors or warnings are expected.
    ' if everything goes well,  we're back to the originating document after
    ' completion.

    dim iSelectedFilePosition as integer
    dim brc as boolean
    
    const CFN = "hDeleteFileViaFileOpen::"
    printlog( CFN & "Enter with option: " & cFile )
    
    ' Test for wildcards - the dialog cannot handle those
    if ( instr( cFile , "?" ) <> 0 ) then
        warnlog( CFN & "Incorrect call to function, wildcards are not allowed" )
        hDeleteFileViaFileOpen() = false
        exit function
    endif
    
    ' Test for wildcards - the dialog cannot handle those
    if ( instr( cFile , "*" ) <> 0 ) then
        warnlog( CFN & "Incorrect call to function, wildcards are not allowed" )
        hDeleteFileViaFileOpen() = false
        exit function
    endif    

    '///+<li>Click &quot;File Open&quot;</li>
    hUseAsyncSlot( "FileOpen" )
    
    '///+<li>Look for the requested file, get the position from the filelist</li>
    Kontext "OeffnenDLG"
    iSelectedFilePosition = hFindFileObjectViaFileOpen( cFile )
    
    '///+<li>If the file exists, delete it + verify, if not: Warn and exit</li>
    if ( iSelectedFilePosition > 0 ) then
    
        ' Enable for debug
        'printlog( CFN & "Requested file: " & cFile )
        'printlog( CFN & "Object is at..: " & iSelectedFilePosition )
        'printlog( CFN & "Object name is: " & DateiAuswahl.getSelText()  )
        
        DateiAuswahl.TypeKeys( "<DELETE>" )
    
        ' Confirm deletion. This dialog should always pop up when deleting
        Kontext "ConfirmDelete"
        if ( ConfirmDelete.exists( 1 ) ) then
            Delete.click()
            printlog( CFN & "Deleted file: " & cFile ) 
        else
            warnlog( CFN & "Messagebox to confirm deletion of file is missing" )
        endif
    
        ' Handle possible Warnings/Errormessages. If no errors happen, 
        ' verify that the file has been deleted. Note that this dialog will not
        ' change the returnvalue. 
        Kontext "Active"
        if( Active.exists( 1 ) ) then
            warnlog( CFN & "Unexpected active: " & active.getText() )
            Active.OK()
        endif 
        
        ' Verify that the file does no longer exist in the filelist.
        iSelectedFilePosition = hFindFileObjectViaFileOpen( cFile )
        if ( iSelectedFilePosition = 0 ) then
            brc = true
        else
            brc = false
        endif
        
    else
    
        brc = false
        warnlog( CFN & "File not found in workdir: " & cFile )
        
    endif
    
    '///+<li>Cancel the FileOpen dialog</li>
    kontext "OeffnenDlg"
    OeffnenDlg.cancel()
    
    '///+<li>Return errorcondition</li>
    printlog( CFN & "Exit with result: " & brc )
    hDeleteFileViaFileOpen() = brc
    
    '///</ul>

end function

'*******************************************************************************

function hNameGen_append( iDecChar as long ) as string

    '///<h3>Create a filename with specified character at the end</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Character as number (integer)</li>
    '///<ul>
    '///+<li>Only positive numbers are allowed</li>
    '///+<li>Respect integer boundaries</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>A filename containing a special character at the end (before suffix)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    dim cFile as string
    const CFN = "hNameGen_append::"

    '///+<li>Create a string &quot;test&quot;, append ascii char</li>
    cFile = "test" & CHR$( iDecChar )
    printlog( CFN & "ASCII " & iDecChar & " appended, len = " & len( cFile ) )
    if ( len( cFile ) <> 5 ) then
        warnlog( CFN & "Character not appended" )
    endif

    '///+<li>Return the new filename</li>
    hNameGen_append() = cFile
    
    '///</ul>

end function

'*******************************************************************************

function hNameGen_lead( iDecChar as long ) as string

    '///<h3>Create a filename with specified character at the beginning</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Character as number (integer)</li>
    '///<ul>
    '///+<li>Only positive numbers are allowed</li>
    '///+<li>Respect integer boundaries</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>A filename containing a special character at the beginning</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
   
    dim cFile as string
    const CFN = "hNameGen_lead::"

    '///+<li>Create a string beginning with ascii char and append &quot;test&quot</li>
    cFile = CHR$( iDecChar ) & "test"
    printlog( CFN & "ASCII " & iDecChar & " prepended, len = " & len( cFile ) )
    if ( len( cFile ) <> 5 ) then
        warnlog( CFN & "Character not prepended" )
    endif

    '///+<li>Return the new filename</li>
    hNamegen_lead() = cFile
    
    '///</ul>

end function

'*******************************************************************************

function hFindFileObjectViaFileOpen( cName as string ) as integer

    '///<h3>Get the position of a filesystem object in filelist of fileopen dialog</h3>
    '///<i>Note that the filepicker must be open in order to use this function.<br>
    '///The object with the given name is selected when leaving the function.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename incl. extension (string) but without any pathseparator</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Position of the object in the filepicker list</li>
    '///<ul>
    '///+<li>0: Object not found</li>
    '///+<li>&gt; 0: Position</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hFindFileObjectViaFileOpen::"

    dim iCurrentObject as integer
    dim cCurrentName as string
    dim iFileOpenItemCount as integer
    
    dim iPos as integer : iPos = 0
    dim cPath as string : cPath = cName

    dim iPathItems as integer
    dim asPathList( 100 ) as string
    
    dim iWait as integer ' some increment variable
    
    printlog( CFN & "Enter with option (File): " & cName )

    '///+<li>If a fully qualified path has been provided we have to split it up</li>
    iPathItems = DirNameList( cPath, asPathList() )
    'printlog( CFN & "iPathItems (new): " & iPathItems )

    if ( iPathItems > 3 ) then
        cName = asPathList( iPathItems )
        printlog( CFN & "CNAME from plist: " & cName )

        '///+<li>And walk to the directory</li>
        cPath = left( cPath , ( len ( cPath ) - len( cName ) ) - 1 )
        printlog( CFN & "CPATH from plist: " & cPath )

        kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 1 ) ) then
            Dateiname.setText( cPath )
            Oeffnen.click()
        else
            warnlog( CFN & "File Open dialog is not open" )
            hFindFileObjectViaFileOpen() = 0
            exit function
        endif
    endif
        
    '///+<li>get the number of items in the filepicker</li>
    kontext "OeffnenDlg"
    iWait = 0
    while ( DateiAuswahl.getItemCount() = 0 )
        wait( 100 )
        iWait = iWait + 1
        if ( iWait = 10 ) then
            warnlog( "Filepicker is not populated within reasonable timeframe, aborting" )
            kontext "OeffnenDlg"
            OeffnenDlg.Close()
            hFindFileObjectViaFileOpen() = 0
            exit function
        else
            printlog( "Waiting for filepicker list to populate..." )
        endif
    wend
    
    iFileOpenItemCount = DateiAuswahl.getItemCount()
    
    '///+<li>Jump to the first item in the filelist and select it with SPACE</li>
    DateiAuswahl.typeKeys( "<HOME>" )
    DateiAuswahl.typeKeys( "<SPACE>" )
    
    '///+<li>Navigate through the list until we find the requested object</li>
    for iCurrentObject = 1 to iFileOpenItemCount
    
        cCurrentName = DateiAuswahl.getSelText()
    
        if ( cCurrentName = cName ) then
            iPos = iCurrentObject
            exit for
        endif
        
        DateiAuswahl.typeKeys( "<DOWN>" )
        
    next iCurrentObject
    
    '///+<li>Print some info to the log</li>
    if ( iPos > 0 ) then
        printlog( CFN & "Exit: Found item [" & cName & "] at pos: " & iPos )
    else
        printlog( CFN & "Exit: Requested item [" & cName & "] was not found." )
    endif
    
    '///+<li>return the position or 0 on failure</li>
    hFindFileObjectViaFileOpen() = iPos
    '///</ul>
    
end function

