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

function hIsAccessbridgeInstalled() as boolean
    '/// created by HDE
    '/// detects if a Java Access Bridge is installed which decreases testtool performance
    '/// and should not be installed while running automated tests
    '/// 1. searches in testtool.ini entry "UseAccessBridge" in profile "current". Is it <> True then
    '/// 2. searches in directory "C:\Program Files\Java Access Bridge" a Java-Bridge Installation. If this is not found
    '/// 3. searches in options of installed StarOffice on Java-Tabpage for the entry "with accessibility support"
    '/// if 1. = true then false is returned, because it should be tested with accessibility 
    '/// if 2. or 3. matches this func returns true else false 
    '/// if Java is deactivated in Office options it returns false, because no Java->no AccessBridge 
    '/// if AccessbridgeIsInstalled = false (after checking on Java-Tabpage) and iSprache <> English a warning will be printed, because the string in options maybe has to be adapted!
    
    Dim i as integer, j as integer, sAccessibilityCompare as string
    Dim sProgramFiles as string, sAccessBridgeFolder as string, sTmpEntry as string    
    Dim sCompareString as string, AccessbridgeIsInstalled as boolean
    AccessbridgeIsInstalled = False
    if GetIniValue ( gTesttoolIni, "UseAccessBridge", "Current" ) = True then
        AccessbridgeIsInstalled = False
    else
        Select case gPlatgroup
            Case "w95"  
                sProgramFiles = environ("ProgramFiles")
                sAccessBridgeFolder = "Java Access Bridge"
                sCompareString = sProgramFiles & "\" & sAccessBridgeFolder & "\AccessBridgeTester.class"
              	sAccessibilityCompare = "with accessibility support"
                if ( FileExists( sCompareString ) ) then
                    AccessbridgeIsInstalled = True
                else
                    ToolsOptions
                    Call hToolsOptions("STAROFFICE","JAVA")
                    Kontext "TabJava"
                    if UseJava.IsChecked = True then
                        For i = 1 to JavaList.GetItemCount()                                                                               
                            JavaList.Select i
                            if JavaList.GetText(4) = sAccessibilityCompare then
                                AccessbridgeIsInstalled = True
                                exit for
                            endif
                        next i
						if AccessbridgeIsInstalled = false then
		                    Select Case iSprache
		                        Case 01 : sAccessibilityCompare = "with accessibility support"
    		                    Case else : Warnlog "Accessibility check failed. Maybe language has to be adapted."
            	                        exit function
        	    	        end select
        	    	    endif
                    endif
                    Kontext "OptionenDlg"
                    hCloseDialog( OptionenDlg, "Cancel" )
                endif                
        end select
    endif
    
    hIsAccessbridgeInstalled = AccessbridgeIsInstalled
    
end function

'*******************************************************************************

function hUseAsyncSlot( cSlot as string ) as integer

    ' These are a selection of slots that take time to get enabled after
    ' actions have been performed on a document. Although they might appear
    ' to be ready in the UI we still need to probe them.

    dim iRetries as integer : iRetries = 0
    dim iTime as integer : iTime = 0

    dim slot as string : slot = lcase( cSlot )

    const MAX_RETRIES = 100 ' Number of time we try to call the slot
    const DELAY       = 100 ' 100ms delay to wait between retries
    const RC_TIMEOUT  = -1  ' Returnvalue if slot is not available
    const CFN = "global::tools::includes::required::hUseAsyncSlot(): "

    if ( GVERBOSE ) then printlog( CFN & "Execute <" & cSlot & ">" )
    WaitSlot()

    for iRetries = 1 to MAX_RETRIES
        try
            select case ( slot )
            case "fileclose"                      : FileClose
            case "filesave"                       : FileSave
            case "filesaveas"                     : FileSaveAs( "SynchronMode", TRUE )
            case "filesaveall"                    : FileSaveAll( "SynchronMode", TRUE )
            case "fileexport"                     : FileExport
            case "filereload"                     : FileReload( "SynchronMode", TRUE )
            case "fileopen"                       : FileOpen
            case "fileprint"                      : FilePrint
            case "fileprintersettings"            : FilePrintersettings
            case "fileproperties"                 : FileProperties

            case "editdoc"                        : EditDoc
            case "editcopy"                       : EditCopy
            case "editchangesrecord"              : EditChangesRecord
            case "editchangesshow"                : EditChangesShow
            case "editchangesprotecttrace"        : EditChangesProtectTrace
            case "editchangescomment"             : EditChangesComment

            case "editpaste"                      : EditPaste
            case "editcut"                        : EditCut
            case "editselectall"                  : EditSelectAll
            case "editselectallmath"              : EditSelectAllMath
            case "editselectionmodeblock"         : EditSelectionModeBlock
            case "editselectionmodestandard"      : EditSelectionModeStandard
            case "editdeletecontents"             : EditDeleteContents
            case "editundo"                       : EditUndo
            case "editredo"                       : EditRedo
            case "editrepeat"                     : EditRepeat
            case "editpastespecial"               : EditPasteSpecial
            case "editpastespecialwriter"         : EditPasteSpecialWriter
            case "editsearchandreplace"           : EditSearchAndReplace
            case "editduplicate"                  : EditDuplicate
            case "editgluepoints"                 : EditGluePoints
            case "editdeleteslide"                : EditDeleteSlide
            case "editobjectproperties"           : EditObjectProperties
            case "editobjectedit"                 : EditObjectEdit
            case "editobjectsavecopyas"           : EditObjectSaveCopyAs
            case "edithyperlink"                  : EditHyperlink
            case "editlinkswriter"                : EditLinksWriter
            case "editfields"                     : EditFields

            case "formatframe"                    : FormatFrame
            case "formatcharttype"                : FormatChartType
            case "formatcontrol"                  : FormatControl
            case "formateditpoints"               : FormatEditPoints
            case "formatsections"                 : FormatSections
            case "formatchangecaseupper"          : FormatChangeCaseUpper
            case "formatchangecaselower"          : FormatChangeCaseLower
            case "formatarea"                     : FormatArea
            case "formatarrangebringtofrontcalc"  : FormatArrangeBringToFrontCalc
            case "formatflipvertically"           : FormatFlipVertically
            case "formatfliphorizontally"         : FormatFlipHorizontally
            case "formatline"                     : FormatLine
            case "formatpositionandsize"          : FormatPositionAndSize
            case "formatfontwork"                 : FormatFontwork
            case "formatpagewriter"               : FormatPageWriter
            case "formatparagraph"                : FormatParagraph
            case "formatruby"                     : FormatRuby
            case "formatstylebold"                : FormatStyleBold
            case "formatungroupdraw"              : FormatUngroupDraw
            case "formatexitgroupdraw"            : FormatExitGroupDraw
            case "formatgroupgroup"               : FormatGroupGroup
            case "formatgroupeditgroupcalc"       : FormatGroupEditGroupCalc
            case "formatalignmentlefttext"        : FormatAlignmentLeftText
            case "formatgraphics"                 : FormatGraphics
            case "formatanchortopage"             : FormatAnchorToPage
            case "formatwrapcontour"              : FormatWrapContour
            case "formatwrapeditcontour"          : FormatWrapEditContour
            case "formatpagewriter"               : FormatPageWriter
            case "formatautoformatapply"          : FormatAutoformatApply

            case "insertslide"                    : InsertSlide
            case "insertfieldsother"              : InsertFieldsOther
            case "insertgraphicsfromfile"         : InsertGraphicsFromFile
            case "insertindexesbibliographyentry" : InsertIndexesBibliographyEntry

            case "contextpositionandsize"         : ContextPositionAndSize
            case "contextproperties"              : ContextProperties

            case "toolslanguagehyphenate"         : ToolsLanguageHyphenate
            case "toolsupdateallindexes"          : ToolsUpdateAllIndexes
            case "toolsupdatefields"              : ToolsUpdateFields
            case "toolsupdatelinks"               : ToolsUpdateLinks
            case "toolslanguagehangulhanjaconversion" : ToolsLanguageHangulHanjaConversion

            case "viewslide"                      : ViewSlide
            case "viewdrawing"                    : ViewDrawing
            case "viewnotes"                      : ViewNotes
            case "viewhandout"                    : ViewHandout
            case "viewzoom"                       : ViewZoom
            case "viewnormal"                     : ViewNormal
            case "viewworkspacedrawingview"       : ViewWorkspaceDrawingView
            case "viewworkspaceoutlineview"       : ViewWorkspaceOutlineView
            case "viewworkspaceslideview"         : ViewWorkspaceSlidesView
            case "viewworkspacenotesview"         : ViewWorkspaceNotesView
            case "viewworkspacehandoutview"       : ViewWorkspaceHandoutView

            case else : warnlog( "Unknown slot called: " & cSlot )
            end select

            exit for  ' This is only executed if the slot was called successfully
        catch
            wait( DELAY )
        endcatch
    next iRetries

    ' Total time used
    iTime = iRetries * DELAY

    ' Timeout is -1
    if ( iRetries >= MAX_RETRIES ) then iTime = RC_TIMEOUT

    if ( GVERBOSE ) then printlog( CFN & "Exit with rc=" & iTime )
    hUseAsyncSlot() = iTime

end function

'*******************************************************************************

function hClickButton( oButton as object ) as integer

    dim iRetries as integer : iRetries = 0
    dim iTime as integer : iTime = 0

    const MAX_RETRIES    = 50  ' Try to click the button max 50 times
    const DELAY          = 100 ' 100ms wait between retries
    const RC_TIMEOUT     = -1  ' Return -1 if the button is not available
    const CFN = "global::tools::includes::required::hClickButton(): "

    if ( GVERBOSE ) then printlog( "Click <" & oButton.name() & ">" )
    WaitSlot()

    for iRetries = 1 to MAX_RETRIES
        try
            oButton.click()
            exit for
        catch
            wait( DELAY )
        endcatch
    next iRetries

    ' Total time used
    iTime = iRetries * DELAY

    if ( iRetries >= MAX_RETRIES ) then iTime = RC_TIMEOUT

    if ( GVERBOSE ) then printlog( CFN & "Exit with rc=" & iTime )
    hClickButton() = iTime

end function

'*******************************************************************************

function hCloseDialog( oDialog as object, sUserFlags as string ) as boolean

    ' this function closes a dialog providing three different methods while
    ' removing the need for static sleep statements

    const CFN = "global::tools::includes::required::t_tools3.inc::hCloseDialog(): "
    const METHOD_CLOSE  = 1
    const METHOD_CANCEL = 2
    const METHOD_OK     = 3
    const METHOD_202    = 4
    const METHOD_YES    = 5
    const METHOD_NO     = 6
    const TIMEOUT       = 2 ' Maximum time to wait for a dialog to close

    dim sFlags as string : sFlags = lcase( sUserFlags )
    dim iClosingMethod as integer
    dim bDialogMustExist as boolean : bDialogMustExist = TRUE

    hCloseDialog() = FALSE ' Set default returnvalue, assume failure

    ' Find out which closing method was provided, "close" ist the default
    if ( instr( sFlags, "cancel" ) ) then
        iClosingMethod = METHOD_CANCEL
    elseif ( instr( sFlags, "202" ) ) then
        iClosingMethod = METHOD_202
    elseif ( instr( sFlags, "yes" ) ) then
        iClosingMethod = METHOD_YES
    elseif ( instr( sFlags, "ok" ) ) then
        iClosingMethod = METHOD_OK
    elseif ( instr( sFlags, "no" ) ) then
        iClosingMethod = METHOD_NO
    else
        iClosingMethod = METHOD_CLOSE
    endif

    ' Find out whether the dialog must exist (default) or is just closed as a precaution ("optional")
    if ( instr( sFlags, "optional") ) then bDialogMustExist = FALSE

    if ( GVERBOSE ) then
        printlog( "Closing dialog <" & oDialog.name() & "> using the flags <" & sFlags & ">.")
    endif

    if ( oDialog.exists() ) then
        try
            select case( iClosingMethod )
            case METHOD_CLOSE  : oDialog.close()
            case METHOD_CANCEL : oDialog.cancel()
            case METHOD_202    : oDialog.click( 202 )
            case METHOD_YES    : oDialog.yes()
            case METHOD_NO     : oDialog.no()
            case METHOD_OK     : oDialog.ok()
            case else :
                warnlog( CFN & " Script error: Allowed closing methods are cancel, close, ok, yes, no and 202" )
            end select
        catch
            warnlog( CFN & "This dialog does not provide the requested closing method" )
            hCloseDialog() = FALSE
            exit function
        endcatch
    else
        if ( bDialogMustExist ) then
            warnlog( CFN & "The requested dialog is not available, no action taken" )
            hCloseDialog() = FALSE
            exit function
        endif
    endif

    ' Any other outcome until now returned FALSE. If we get here, we have a success.
    if ( oDialog.notExists( TIMEOUT ) ) then hCloseDialog() = TRUE

end function

'*******************************************************************************

function hWaitForObject( oControl as object, iTime as integer ) as integer

    const CFN = "global::tools::includes::required::t_tools3.inc::hWaitForObject(): "
    const RC_TIMEOUT = -1

    dim iWait as integer

    if ( GVERBOSE ) then printlog( CFN & "Entering function for control: " & oControl.name() )

    WaitSlot( 2000 )

    for iWait = 0 to iTime
        try
            if ( oControl.isEnabled() ) then
                printlog( CFN & "Control is enabled: " & oControl.name() )
                hWaitForObject() = iWait
                exit function
            else
                if ( GVERBOSE ) then printlog( "Waiting..." )
            endif
        catch
            wait( 1 )
        endcatch
    next iWait

    printlog( CFN & "Control not found or not enabled, timeout reached" )
    hWaitForObject() = RC_TIMEOUT

end function
