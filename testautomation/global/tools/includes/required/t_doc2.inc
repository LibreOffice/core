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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Global Routines for Document Handling; Part two
'*
'\***********************************************************************

private const VERBOSE = TRUE

sub hTabelleEinfuegen
    '/// <b>WRITER only </b>///'
    '///  hTabelleEinfuegen hInsertTable ///'
    '/// insert a dummy table in writer/writerweb/masterdocument ///'
    TableInsertTable
    sleep(2)
    Kontext "TabelleEinfuegenWriter"
    wait 500
    TabelleEinfuegenWriter.OK
    sleep(1)

    Kontext "TableObjectbar"
    sleep(1)
    if TableObjectbar.NotExists then
        Kontext "TextObjectbar"
        TextObjectbar.SetNextToolBox
    end if

    select case uCASE(gApplication)
    Case "WRITER"
        Kontext "DocumentWriter"
    Case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
    Case "HTML"
        Kontext "DocumentWriterWeb"
    end select
    sleep(1)
end sub
'
'-------------------------------------------------------------------------------
'
sub ZellenMarkieren ( Down%, Right% )
    '/// <b>CALC only</b> ///'
    '/// ZellenMarkieren ( Down%, Right% ) : mark the cells ///'
    Dim Anzahl as Integer

    Kontext "DocumentCalc"
    Anzahl = Right% - 1
    DocumentCalc.TypeKeys "<Shift Right>", Anzahl
    Anzahl = Down% - 1
    DocumentCalc.TypeKeys "<Shift Down>", Anzahl
end sub
'
'-------------------------------------------------------------------------------
'
sub hRechteckErstellen ( BeginX%, BeginY%, EndX%, EndY% )
    '/// <b>IMPRESS/DRAW only</b> ///'
    '/// hRechteckErstellen ( BeginX, BeginY, EndX, EndY ) : create a rectangle ///'
    WL_DRAW_Rechteck
    gMouseMove ( BeginX%, BeginY%, EndX%, EndY% )
end sub
'
'-------------------------------------------------------------------------------
'
sub hTextrahmenErstellen ( TextEingabe$, BeginX%, BeginY%, EndX%, EndY% )
    '/// <b>IMPRESS/DRAW only</b> ///'
    '/// hTextrahmenErstellen ( String, BeginX, BeginY, EndX, EndY ) : create a textbox with a textstring ///'
    WL_SD_TextEinfuegenDraw
    gMouseMove ( BeginX%, BeginY%, EndX%, EndY% )
    hTypeKeys TextEingabe$
end sub
'
'-------------------------------------------------------------------------------
'
sub SchreibenInMathdok ( Eingabe as String )
    '/// <b>MATH only</b> ///'
    '/// SchreibenInMathDok ( String ) : write text in a mathdocument ( with clipboard ) ///'
    if Eingabe <> "Unsinn" then
        SetClipboard Eingabe
    else
        SetClipboard "NROOT <?> <?><over b==<?>"
    endif
    if (GetClipboard() <> Eingabe) then
        warnlog "--No Clipboard available :-(--"
        printlog "---ClipTest--- should: "+Eingabe +", is: "+GetClipboard
    endif
    EditPaste
    sleep(3)
end sub
'
'-------------------------------------------------------------------------------
'
function sMakeReadOnlyDocumentEditable() as boolean

    ' Function returns TRUE if document has been made editable and FALSE if
    ' no action was required (that is: Document was not read-only)

    dim iTry as integer
    dim iWait as integer
    dim rc as integer
    const CFN = "sMakeReadOnlyDocumentEditable::"
    
    if ( VERBOSE ) then printlog( CFN & "Making document editable (create a copy) if it is readonly" )
    
    ' We still run into synchronization problems with the "Make document editable" feature
    ' because we do not know when this button becomes available and ready for use: The 
    ' document has to be loaded, then we have to wait for the toolbar to get populated.
    ' One thing appears to work though: Try to execute the slot using hUseAsyncSlot.
    ' This is a little bit tricky because the state of the document is altered but if
    ' the slot has been executed once we know for sure that the button on the toolbar is
    ' available and can check its state and - if need be - chenge the document state back.
    
    ' ========== Workaround begin ==========
    hUseAsyncSlot( "EditDoc" )
    kontext "Active"
    if ( Active.exists() ) then
        if ( VERBOSE ) then printlog( "Handling create copy message" )
        Active.yes()
        wait( 1000 )
    endif
    ' ==========  Workaround end  ==========
    Kontext "Standardbar"
    iWait = 0
    Do
        if ( Bearbeiten.IsEnabled() ) then
            try
                if Bearbeiten.getState( 2 ) = 0 then
                    exit do
                endif
            catch
                if ( VERBOSE ) then printlog "Attempt " & iWait & " to make document readable failed"
                iWait = iWait + 1
                wait ( 1000 )
            endcatch
        else
            if ( VERBOSE ) then printlog "Attempt " & iWait & " to make document readable failed"
            iWait = iWait + 1
            wait ( 1000 )
        end if
        if iWait >= 5 then exit do
    Loop

    if ( Bearbeiten.IsEnabled() ) then
        
        if ( Bearbeiten.getState( 2 ) = 0 ) then
            
			rc = hUseAsyncSlot("editdoc")

            if ( rc >= 0 ) then
                for iTry = 1 to 2
                
                
                    Kontext "Active"
                    if ( Active.exists( 5 ) ) then
                    
                        printlog( CFN & "Messagebox: " & Active.getText() )
                        
                        try
                            if ( VERBOSE ) then printlog( CFN & "Document was read-only. A copy will be used." )
                            Active.Yes()
                            if ( VERBOSE ) then printlog( CFN & "Closed 'use copy' message" )
                        catch
                            if ( VERBOSE ) then printlog( CFN & "Probing for unexpected messagebox..." )
                            active.ok()
                            qaerrorlog( "#i100701 - Object not found message" )
                        endcatch
                    else
                        printlog( CFN & "No messagebox informing about a copy being used" )
                    endif
                    sMakeReadOnlyDocumentEditable() = TRUE
                next iTry
            else
                printlog( CFN & "Document appears to be editable" )
            endif
        else
            if ( VERBOSE ) then printlog( CFN & "Button <Bearbeiten> is pressed, document is editable" )
            sMakeReadOnlyDocumentEditable() = FALSE
        endif
    else
        if ( VERBOSE ) then printlog( CFN & "Control <Bearbeiten> is not enabled" )
    endif
    
end function
'
'-------------------------------------------------------------------------------
'
function fSelectFirstOLE() as integer
    'Select first visible OLE object using Navigator
    'Returns error-code:
    '+ 0 := Sucess
    '- 1 := unknown application

    dim bNavigatorWasVisible as boolean
    bNavigatorWasVisible = FALSE
    dim iIndex

    fSelectFirstOLE = -1

    select case uCASE(gApplication)
    case "CALC"                     :   Kontext "NavigatorCalc"
        'First check if Navigator is visible and remember result
        if NavigatorCalc.exists (10) then
            bNavigatorWasVisible = TRUE
        else
            try
                'Invoke Navigator if not visible
                ViewNavigator
            catch
                'If inside chart or elsewhere the call
                'will fail. Again trying the slot after
                'switching to the document.
                Kontext "DocumentCalc"
                DocumentCalc.TypeKeys "<Escape>"
                ViewNavigator
            endcatch
        end if
        Kontext "NavigatorCalc"
        if NavigatorCalc.exists (10) then
            'Select first OLE in list
            Liste.TypeKeys "<HOME>"
            for iIndex = 1 to 8
                Liste.TypeKeys "-"
                wait 500
                Liste.TypeKeys "<DOWN>"
                wait 500
            next iIndex
            Liste.select(6)
            Liste.TypeKeys "+"
            Liste.TypeKeys "<DOWN><RETURN>"
            fSelectFirstOLE = 0
        else
            QAErrorLog "Navigator couldn't be opened!"
        end if
    case "DRAW" , "IMPRESS"         :   Kontext "NavigatorDraw"
        if NavigatorDraw.Exists(10) then
            bNavigatorWasVisible = TRUE
        else
            try
                'Invoke Navigator if not visible
                ViewNavigator
            catch
                'If inside chart or elsewhere the call
                'will fail. Again trying the slot after
                'switching to the document.
                Kontext "DocumentDraw"
                DocumentDraw.TypeKeys "<Escape>"
                ViewNavigator
            endcatch
            Kontext "NavigatorDraw"
            if NavigatorDraw.exists(10) then
                'Select first OLE in list
                Liste.TypeKeys "<HOME>"
                Liste.select(1)
                Liste.TypeKeys "+<DOWN><RETURN>"
                fSelectFirstOLE = 0
            else
                QAErrorLog "Navigator did not occoured!"
            end if
        end if

    case "WRITER" , "HTML" , "MASTERDOCUMENT" :
        select case uCASE(gApplication)
        case "MASTERDOCUMENT"    :   Kontext "NavigatorGlobalDoc"
            if NavigatorGlobalDoc.Exists(10) then
                bNavigatorWasVisible = TRUE
            else
                ViewNavigator
            end if
            wait 500
            Kontext "NavigatorGlobalDoc"
            if Liste.IsVisible then
                Kontext "GlobaldokumentToolbox"
                Umschalten.Click
            endif
        case else           :   Kontext "NavigatorWriter"
            'First check if Navigator is visible and remember result
            if NavigatorWriter.Exists (10) then
                bNavigatorWasVisible = TRUE
            else
                try
                    'Invoke Navigator if not visible
                    ViewNavigator
                catch
                    'If inside chart or elsewhere the call
                    'will fail. Again trying the slot after
                    'switching to the document.
                    Kontext "DocumentWriter"
                    call gMouseclick (99,99)
                    call gMouseclick (50,50)
                    ViewNavigator
                endcatch
            end if
        end select
        Kontext "NavigatorWriter"
        if NavigatorWriter.Exists(10) then
			' Check if all content is visible
			if Auswahlliste.GetItemCount < 2 then
				Inhaltsansicht.Click
			end if
            'Select first OLE in list
            Auswahlliste.TypeKeys "<HOME>"
            for iIndex = 1 to 13
                Auswahlliste.TypeKeys "-<DOWN>"
            next iIndex
            Auswahlliste.select(5)
            Auswahlliste.TypeKeys "+<DOWN><RETURN>"
            fSelectFirstOLE = 0
        else
            QAErrorLog "Navigator did not occoured!"
        end if
    case else                       :   QAErrorLog "Application not supported"
    end select

    'Close navigator if it was invisible by entering the routine
    if bNavigatorWasVisible = TRUE then
        printlog "Leaving navigator open as initially found"
    else
        if fSelectFirstOLE = 0 then
            ViewNavigator
            printlog "Closing navigator as initially found"
        else
            printlog "Closing navigator not needed. It was not possible to open it."
        end if
    end if
end function
