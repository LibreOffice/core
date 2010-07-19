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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : helper functions for the wizards
'*
'\******************************************************************************

function hOpenWizard( cWizard as string ) as boolean

    const CFN = "hOpenWizard(): "
    const NUMBER_OF_RETRIES = 3

    dim iTry as integer
    dim brc as boolean : brc = false

    if ( GVERBOSE ) then printlog( CFN & "Opening wizard" )

    ' Call the wizard by slot
    for iTry = 1 to NUMBER_OF_RETRIES
        try
            select case ( ucase( cWizard ) )
            case "PRESENTATION" : FileAutopilotPresentation
            case "FIRTSTIME"    : FirstTimeWizard
            case "MAILMERGE"    : ToolsMailMergeWizard
            case "EUROCONV"     : FileWizardEuroConverter
            case "DOCCONV"      : FileWizardDocumentConverter
            case "WEBPAGE"      : FileWizardsWebPage
            case "LETTER"       : FileAutopilotLetter
            case "AGENDA"       : FileAutopilotAgenda
            case "ADRESS"       : FileAutopilotAddressDataSource
            case "FAX"          : FileAutopilotFax
            case else           :
                warnlog( CFN & "Invalid wizard requested: " & cWizard )
                hOpenWizard() = brc
                exit function
            end select

            brc = true
            exit for
        catch
            wait( 200 )
        endcatch
    next iTry

    ' Print warning if slot was blocked.
    if ( not brc ) then
        warnlog( CFN & "Failed to open requested wizard: " & cWizard )
    endif

end function

'*******************************************************************************

function hFinishWizard( iMode as integer ) as boolean

    const CFN = "hFinishWizard::"
    
    '///<h3>Finish or cancel a wizard</h3>
    '///<i>Starting point: Any known wizard must be open</i><br>
    '///<i>If several wizards are open at the same time the outcome is undefined</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Closing method (integer). Valid options are:</li>
    '///<ul>
    '///+<li>1: Click Finish-Button</li>
    '///+<li>2: Click Cancel-Button</li>
    '///+<li>3: Use Accelerator for Finish-Button</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcode (boolean)</li>
    '///<ul>
    '///+<li>TRUE: If everything is ok</li>
    '///+<li>FALSE: On any error (including incorrect function parameters)</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    
    
    '///+<li>Handle possible errors when invoking this function</li>
    if ( ( iMode < 1 ) or ( iMode > 3 ) ) then
        printlog( CFN & "Invalid iMode = " & iMode )
        hFinishWizard() = false
        exit function
    endif

    dim cAccel as string
    dim iWait as integer
    dim brc as boolean : brc = false
       
        
    '///+<li>Try to close the wizard</li>
    for iWait = 1 to 5
    
        Kontext "AutopilotLetter"
        if ( AutopilotLetter.exists() ) then
            select case iMode
            case 1 : hWaitForObject( FinishButton, 3000 ) : FinishButton.click()
            case 2 : AutopilotLetter.cancel()
            case 3 : cAccel = hGetAccel( "Finish_Wizard" )
                     AutopilotLetter.typeKeys( cAccel )
            end select
            brc = true
            exit for
        endif

        Kontext "AutopilotFax"
        if ( AutopilotFax.exists() ) then
            select case iMode
            case 1 : hWaitForObject( FinishButton, 3000 ) : FinishButton.click()
            case 2 : AutopilotFax.cancel()
            case 3 : cAccel = hGetAccel( "Finish_Wizard" )
                     AutopilotFax.typeKeys( cAccel )            
            end select
            brc = true
            exit for
        endif
        
        Kontext "AutopilotAgenda"
        if ( AutopilotAgenda.exists() ) then
            select case iMode
            case 1 : hWaitForObject( createButton, 3000 ) : createButton.click()
            case 2 : hWaitForObject( cancelButton, 3000 ) : cancelButton.click()
            case 3 : cAccel = hGetAccel( "Finish_Wizard" )
                     AutopilotAgenda.typeKeys( cAccel )            
            end select
            brc = true
            exit for
        endif    
        
        Kontext "MailMergeWizard" 
        if ( MailMergeWizard.exists() ) then
            select case iMode
            case 1 : MailMergeWizard.OK()
            case 2 : MailMergeWizard.Cancel()
            case 3 : cAccel = hGetAccel( "Finish_Wizard" )
                     MailMergeWizard.typeKeys( cAccel )            
            end select
            brc = true
            exit for
        endif      
        
        Kontext "AutopilotPraesentation3" 
        if ( AutopilotPraesentation3.exists() ) then
            select case iMode
            case 1 : AutopilotPraesentation3.OK()
            case 2 : AutopilotPraesentation3.Cancel()
            case 3 : cAccel = hGetAccel( "Finish_Wizard" )
                     AutopilotPraesentation3.typeKeys( cAccel )            
            end select
            brc = true
            exit for
        endif    
        
        Kontext "AutoPilotEuroKonverter" 
        if ( AutoPilotEuroKonverter.exists() ) then
            select case iMode
            case 1 : hWaitForObject( Konvertieren, 3000 ) : Konvertieren.click()
            case 2 : hWaitForObject( abbrechen   , 3000 ) : abbrechen.click()
            case 3 : cAccel = hGetAccel( "Finish_Wizard" )
                     AutoPilotEuroKonverter.typeKeys( cAccel )            
            end select
            brc = true
            exit for
        endif            
        
    next iWait
    
    '///+<li>Verify that no wizard is open anymore</li>
    if ( brc ) then
        printlog( CFN & "Closed wizard" )
        hFinishWizard() = true
    else
        qaerrorlog( CFN & "No (known) wizard is present to be closed" )
        hFinishWizard() = false
    endif
    '///</ul>
    
end function   

'*******************************************************************************

function hGetWizardParams( cType as string, cParam as string ) as integer

    const CFN = "hGetWizardParams::"
    
    '///<h3>Retrieve the location of a wizard in the menu</h3>
    '///<i>Uses: framework/tools/input/menuentries.txt</i><br>
    '///<i>Refer to inline documentation for further information</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of Wizard (string). Valid options are:</li>
    '///<ul>
    '///+<li>&quot;LETTER&quot;</li>
    '///+<li>&quot;FAX&quot;</li>
    '///+<li>&quot;AGENDA&quot;</li>
    '///+<li>&quot;PRESENTATION&quot;</li>
    '///+<li>&quot;WEBPAGE&quot;</li>
    '///+<li>&quot;DOCCONV&quot;</li>
    '///+<li>&quot;EUROCONV&quot;</li>
    '///+<li>&quot;ADDRESS&quot;</li>
    '///+<li>&quot;MAILMERGE&quot;</li>
    '///</ul>
    '///+<li>Menu Position (string). Valid options are:</li>
    '///<ul>
    '///+<li>&quot;ID&quot; - This is a unique id for the wizard</li>
    '///+<li>&quot;MAIN&quot; - Main menu position (e.g. &quot;File&quot;</li>
    '///+<li>&quot;SUB&quot; - Position below main menu</li>
    '///</ul>    
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Position of a menu entry (integer)</li>
    '///<ul>
    '///+<li>0: Invalid function parameter</li>
    '///+<li>1-n: Position of the item in the menu</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    

    dim iParam as integer
    
    dim cInputArray( 200 ) as string
    dim iArraySize as integer
    
    dim cFile as string
        cFile = gTesttoolPath & "framework\tools\input\menuentries.txt"
        cFile = convertpath( cFile )
    
    '///+<li>Lowercase all function parameters</li>
    cType = ucase( cType )
    cParam = ucase( cParam )
    
    '///+<li>Read in the section containing the waizard parameters</li>
    '///+<li>Retrieve the requested value from the section</li>
    iArraySize = hGetDataFileSection( cFile , cInputArray() , ctype , "" , "" )
    if ( iArraySize = 3 ) then
    
        select case cParam
        case "ID"
            iParam = hGetValueForKeyAsInt( cInputArray() , "iWizardId" )
        case "MAIN"
            iParam = hGetValueForKeyAsInt( cInputArray() , "iMainMenuPosition" )
        case "SUB"
            iParam = hGetValueForKeyAsInt( cInputArray() , "iMenuPosition" )
        end select
    
        hGetWizardParams() = iParam
    else
        qaerrorlog( CFN & "hGetDataFileSection returned invalid data" )
        hGetWizardParams() = 0
    endif
    '///</ul>
    
end function

'*******************************************************************************

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

