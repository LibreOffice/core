'encoding UTF-8  Do not remove or change this line!
'*******************************************************************************
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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : helper functions for the wizards
'*
'\******************************************************************************

function hWaitForWizard() as integer

    const CFN = "hWaitForWizard::"
    const MAXTRIES = 10 
    
    '///<h3>Wait for any wizard to open</h3>
    '///<i>Uses: framework/tools/input/menuentries.txt</i><br>
    '///<i>NOTE: Not all wizards are known to this function yet.</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Unique ID of the open wizard</li>
    '///<ul>
    '///+<li>0: In case of any error</li>
    '///+<li>1-n: The unique ID of the currently open wizard (integer)</li>
    '///+</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>        
    
    dim iOpen as integer
    dim iWait as integer

    '///+<li>Determine the open wizard by trying wizard.exists()</li>
    for iWait = 1 to MAXTRIES
     
        iOpen = 0
    
        kontext "AutopilotLetter"
        if ( AutopilotLetter.exists() ) then
            printlog( CFN & "Autopilot Letter is open" )
            iOpen = hGetWizardParams( "LETTER" , "ID" )
            exit for
	    endif

        kontext "AutopilotFax"
        if ( AutopilotFax.exists() ) then
            printlog( CFN & "Autopilot Fax is open" )
            iOpen = hGetWizardParams( "FAX" , "ID" )
            exit for
        endif 
        
        kontext "AutopilotAgenda"
        if ( AutopilotAgenda.exists() ) then
            printlog( CFN & "Autopilot Agenda is open" )
            iOpen = hGetWizardParams( "AGENDA" , "ID" )
            exit for
        endif  
        
        Kontext "MailMergeWizard"
        if ( MailMergeWizard.exists() ) then
            printlog( CFN & "Mailmerge Wizard is open" )
            iOpen = hGetWizardParams( "MAILMERGE" , "ID" )
            exit for
        endif
        
        Kontext "AutopilotPraesentation1"
        if ( AutopilotPraesentation1.exists() ) then
            printlog( CFN & "Presentation Wizard is open" )
            iOpen = hGetWizardParams( "IMPRESS" , "ID" )
            exit for
        endif        
        
        Kontext "AutoPilotEuroKonverter"
        if ( AutoPilotEuroKonverter.exists() ) then
            printlog( CFN & "Euro Converter is open" )
            iOpen = hGetWizardParams( "EUROCONV" , "ID" )
            exit for
        endif
        
        Kontext "DocumentConverter"
        if ( DocumentConverter.exists( 1 ) ) then
            printlog( CFN & "Document Converter is open" )
            iOpen = hGetWizardParams( "DOCCONV" , "ID" )
            exit for
        endif

    next iWait
    
    hWaitForWizard() = iOpen
    '///</ul>
    
end function    

'*******************************************************************************

function hSetTemplateSavePath( cFile as string ) as integer

    '///<h3>Set the template save-path</h3>
    '///<i>Starting point: Last page on FAX-, AGENDA- or LETTERWIZARD</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Fully qualified path to target template name (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcode (integer)</li>
    '///<ul>
    '///+<li>0: No errors, path has been set correctly</li>
    '///+<li>1: The &quot;SaveAs&quot;-dialog did not open</li>
    '///+<li>2: The &quot;SaveAs&quot;-button could not be clicked</li>
    '///+<li>-1: Incorrect call to function</li>
    '///+</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul> 

    const CFN = "hSetTemplateSavePath::"
    
    dim iErr as integer

    '///+<li>Validate function parameter</li>        
    if ( cFile = "" ) then
        printlog( CFN & "Invalid parameter passed to function: Empty Path" )
        hSetTemplateSavePath() = -1
        exit function
    endif

    printlog( CFN & "Open the file-save-dialog" )
    
    '///+<li>Click on the save-as button</li>    
    try
        SaveAsButton.click()
        
        kontext "SpeichernDlg"
        if ( SpeichernDlg.exists( 2 ) ) then
            printlog( CFN & "File Save dialog shown. Good." )
            iErr = 0
        else
            qaerrorlog( "#i49515# FileSave dialog does not open" )
            iErr = 1 
        endif            
    catch
        qaerrorlog( "#i58297# Cannot click SaveAs button" )
        iErr = -1
    endcatch

    '///+<li>Enter a filename</li>
    '///+<li>Save the file</li>    
    if ( iErr = 0 ) then
        call dialogtest( SpeichernDlg )
    
        printlog( CFN & "Name the file" )
        DateiName.setText( cFile )
    
        printlog( CFN & "Save the template" )
        speichern.click()
    endif
 
    hSetTemplateSavePath() = iErr
    '///</ul>
    
end function

'*******************************************************************************

function hHandleSaveError() as integer

    const CFN = "hHandleSaveError::"

    '///<h3>Handle errors while saving a template</h3>
    '///<i>Starting point: Save-As dialog from last page on FAX-, AGENDA- or LETTERWIZARD</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcode (integer)</li>
    '///<ul>
    '///+<li>0: No unexpected dialogs were displayed</li>
    '///+<li>1: Unexpected Active closed by clicking YES</li>
    '///+<li>2: Unexpected Active closed by clicking OK</li>
    '///+</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul> 
    '///<li>Check for unexpected messagebox: file could not be saved</li>
    '///<li>Close the dialog either with YES or OK</li>
    '///</ul>
    
    kontext "active"
    if ( active.exists( 2 ) ) then
        printlog( CFN & "Unexpected MsgBox: " & active.getText() )
        try
            active.ok()
            hHandleSaveError() = 2
        catch
            active.yes()
            hHandleSaveError() = 1
        endcatch
    else
        hHandleSaveError() = 0
    endif
    
end function

'*******************************************************************************

function hClickNextButton() as boolean

    '///<h3>Click the Next-Button on some wizards</h3>
    '///<i>Starting point: Last page on FAX-, AGENDA- or LETTERWIZARD</i><br>
    '///<i>Refer to inline documentation for further information</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Status (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Everything is ok</li>
    '///+<li>FALSE: Any error</li>
    '///+</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul> 


    const CFN = "hClickNextButton::"
    hClickNextButton() = true

    '///+<li>Click the &quot;Next&quot;-button</li>
    if ( hClickButton( NextButton ) = -1 ) then
        hClickNextButton() = false
    else
        '///+<li>Handle the &quot;Document Creation&quot;-dialog (Mailmerge Wizard)</li>
        kontext "active"
        if ( active.exists( 1 ) ) then
            ' The dialog must disappear within 10 seconds
            if ( not active.notExists( 10 ) ) then hClickNextButton() = false
        endif
    endif
    '///</ul>
            
end function

