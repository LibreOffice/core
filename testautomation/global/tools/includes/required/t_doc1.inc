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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Global Routines for Document Handling
'*
'\*************************************************************************************

sub hNewDocument ( optional bANewDoc )
    '/// hNewDocument : open a new document dependent on 'gApplication' ///'
    dim sTemp as string

    if IsMissing ( bANewDoc ) <> TRUE then
        if bANewDoc = TRUE then
            gNoNewDoc = FALSE
        else
            gNoNewDoc = TRUE
        end if
    end if

    select case gApplication
    case "WRITER"
        Kontext "DocumentWriter"
        if gNoNewDoc = TRUE then
            FileOpen "FileName", "private:factory/swriter", "SynchronMode", TRUE
        else
            FileOpen "FileName", "private:factory/swriter", "FrameName", "_default", "SynchronMode", TRUE
        end if
        if ( DocumentWriter.IsMax() = false ) then
            DocumentWriter.Maximize()
            Wait( 2000 )
        end if
    case "CALC"
        Kontext "DocumentCalc"
        if gNoNewDoc = TRUE then
            FileOpen "FileName", "private:factory/scalc", "SynchronMode", TRUE
        else
            FileOpen "FileName", "private:factory/scalc", "FrameName", "_default", "SynchronMode", TRUE
        end if
        if ( DocumentCalc.IsMax() = false ) then
            DocumentCalc.Maximize()
            Wait( 2000 )
        end if
    case "IMPRESS"
        Kontext "DocumentImpress"
        if gNoNewDoc = TRUE then
            FileOpen "FileName", "private:factory/simpress", "SynchronMode", TRUE
        else
            FileOpen "FileName", "private:factory/simpress", "FrameName", "_default", "SynchronMode", TRUE
            Kontext "AutoPilotPraesentation1"
            if AutoPilotPraesentation1.Exists (2) then
                Printlog "------------------------------The Impress-Autopilot was active------------------"
                Startwithwizard.Check             ' opposite of the checkboxs' title
                AutoPilotPraesentation1.OK
                Sleep 2
                Kontext "SeitenLayout"
                SeitenLayout.Cancel
            end if
            Kontext "DocumentImpress"
            Sleep 2
            if ( DocumentImpress.IsMax() = false ) then
                DocumentImpress.Maximize()
                Wait( 2000 )
            end if
        end if
    case "DRAW"
        Kontext "DocumentDraw"
        if gNoNewDoc = TRUE then
            FileOpen "FileName", "private:factory/sdraw", "SynchronMode", TRUE
        else
            FileOpen "FileName", "private:factory/sdraw", "FrameName", "_default", "SynchronMode", TRUE
        end if
        if ( DocumentDraw.IsMax() = false ) then
            DocumentDraw.Maximize()
            Wait( 2000 )
        end if
    case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        if gNoNewDoc = TRUE then
            FileOpen "FileName", "private:factory/swriter/GlobalDocument", "SynchronMode", TRUE
        else
            FileOpen "FileName", "private:factory/swriter/GlobalDocument", "FrameName", "_default", "SynchronMode", TRUE
        end if
        Kontext "Navigator"
        sleep (1)
        if Navigator.Exists(5) then Navigator.Close
        Kontext "DocumentMasterDoc"
        if ( DocumentMasterDoc.IsMax() = false ) then
            DocumentMasterDoc.Maximize()
            Wait( 2000 )
        end if
    case "MATH"
        Kontext "DocumentMath"
        if gNoNewDoc = TRUE then
            FileOpen "FileName", "private:factory/smath", "SynchronMode", TRUE
        else
            FileOpen "FileName", "private:factory/smath", "FrameName", "_default", "SynchronMode", TRUE
        end if
        Kontext "DocumentMath"
        if ( DocumentMath.IsMax() = false ) then
            DocumentMath.Maximize()
            Wait( 2000 )
        end if
    case "HTML"
        Kontext "DocumentWriterWeb"
        if gNoNewDoc = TRUE then
            FileOpen "FileName", "private:factory/swriter/web", "SynchronMode", TRUE
        else
            FileOpen "FileName", "private:factory/swriter/web", "FrameName", "_default", "SynchronMode", TRUE
        end if
        Kontext "DocumentWriterWeb"
        if ( DocumentWriterWeb.IsMax() = false ) then
            DocumentWriterWeb.Maximize()
            Wait( 2000 )
        end if
    case "DATABASE"
        FileOpen "FileName", "private:factory/sdatabase?Interactive", "FrameName", "_default", "SynchronMode", TRUE
        Kontext "DatabaseWizard"
        if DatabaseWizard.exists(5) then
            FinishBtn.click
            kontext "SpeichernDlg"
            if SpeichernDlg.exists(5) then
                if (Dateiname.getSelText = "") then
                    sTemp = convertPath(gOfficePath + "user/work/hNewDocument.odb")
                    if fileExists(sTemp) then
                        app.kill(sTemp)
                    endif
                    qaErrorlog "## lost default filename"
                    Dateiname.setText "hNewDocument"
                endif
                Speichern.click
                Kontext "DATABASE"
            else
                warnlog "t_doc1.inc::hNewDocument():: Can't create Database Document 2"
            endif
        else
            warnlog "t_doc1.inc::hNewDocument():: Can't create Database Document 1"
        endif
    case "BASIC"
        ToolsMacroMacro
        kontext "makro"
        if makro.exists(5) then
            MakroAus.typeKeys "<home>"
            sTemp = ""
            while (NOT bearbeiten.isEnabled) AND (sTemp <> MakroAus.getSelText)
                sTemp = MakroAus.getSelText
                MakroAus.typeKeys "<down>+"
            wend
            if (bearbeiten.isEnabled) then
                bearbeiten.click
            else
                qaErrorlog "Can't edit document."
            endif
        else
            warnlog "Can't open Basic IDE."
        endif
    case else             : WarnLog "hNewDocument: No Applikation named '" + gApplication + "' exists in this routine!"
    end select
    Sleep 2

end sub
'
'-------------------------------------------------------------------------------
'
function hCreateLabels() as Boolean

    hCreateLabels() = false

    FileOpen( "FileName", "private:factory/swriter?slot=21051", "FrameName", "_default", "SynchronMode", TRUE )
    
    Kontext
    if ( Active.exists( 5 ) ) then
        Active.setPage TabEtiketten

        if ( TabEtiketten.exists( 1 ) ) then
            if ( Tabetiketten.isVisible() ) then
                hCreateLabels() = true
            else
                warnlog( "<TabEtiketten> is not visible" )
            endif
        else
            warnlog( "<TabEtiketten> not open" )
        endif
    else
        warnlog( "Failed to open <TabEtiketten>" )
    endif

end function
'
'-------------------------------------------------------------------------------
'
function hCreateBusinessCards() as Boolean

    hCreateBusinessCards() = false

    FileOpen( "FileName", "private:factory/swriter?slot=21052", "FrameName", "_default", "SynchronMode", TRUE )

    Kontext
    if ( Active.exists( 5 ) ) then
        Active.setPage TabEtikettenMedium
        
        if ( TabEtikettenMedium.exists( 1 ) ) then
            if ( TabetikettenMedium.isVisible() ) then
                hCreateBusinessCards() = true
            else
                warnlog( "<TabEtikettenMedium> is not visible" )
            endif
        else
            warnlog( "<TabEtikettenMedium> not open" )
        endif
    else
        warnlog( "Failed to open <TabEtikettenMedium>" )
    endif

end function
'
'-------------------------------------------------------------------------------
'
function hCloseDocument()

    hUseAsyncSlot( "FileClose")

    Kontext "Active"
    if ( Active.Exists( 2 ) ) then
        try
            Active.No
        catch
            Active.click( 202 )
        endcatch
    end if

    WaitSlot( 2000 )

end function
'
'-------------------------------------------------------------------------------
'
sub gMouseClick ( X%, Y%, optional _mousebutton )

    dim oDocument as object
    dim mousebutton as integer

    ' Handle infamous optional parameter
    if ( IsMissing( _mousebutton ) ) then
        mousebutton = 1
    else
        mousebutton = _mousebutton
    endif

    hSetDocumentContext() ' set kontext to current document type
    oDocument = hSetDocumentObject() ' get the current document object

    autoexecute = false
    oDocument.MouseDown ( X%, Y%, mousebutton )
    oDocument.MouseUp ( X%, Y%, mousebutton )
    autoexecute = true
    wait( 1000 )
    
end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseDoubleClick ( X%, Y% )

    dim oDocument as object

    hSetDocumentContext() ' set kontext to current document type
    oDocument = hSetDocumentObject() ' get the current document object
    oDocument.MouseDoubleClick ( X%, Y% )
    wait( 1000 )

end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseMove ( BeginX%, BeginY%, EndX%, EndY% )

    dim oDocument as object

    hSetDocumentContext() ' set kontext to current document type
    oDocument = hSetDocumentObject() ' get the current document object
    oDocument.MouseDown ( BeginX%, BeginY% )
    oDocument.MouseMove ( EndX%, EndY%)
    oDocument.MouseUp ( EndX%, EndY% )
    wait( 1000 )

end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseDown ( BeginX%, BeginY% )

    dim oDocument as object

    hSetDocumentContext() ' set kontext to current document type
    oDocument = hSetDocumentObject() ' get the current document object
    oDocument.MouseDown ( BeginX%, BeginY% )
    wait( 300 )

end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseMove2 ( EndX%, EndY% )

    dim oDocument as object

    hSetDocumentContext() ' set kontext to current document type
    oDocument = hSetDocumentObject() ' get the current document object
    oDocument.MouseMove ( EndX%, EndY%)
    wait( 300 )

end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseUp ( EndX%, EndY% )

    dim oDocument as object

    hSetDocumentContext() ' set kontext to current document type
    oDocument = hSetDocumentObject() ' get the current document object
    oDocument.MouseUp ( EndX%, EndY% )
    wait( 300 )

end sub
'
'-------------------------------------------------------------------------------
'
sub hTypeKeys ( OutputText , optional _iLoop as Integer )

    const CFN = "global::tools::includes::required::t_doc1.inc::hTypeKeys(): "

    dim iRepeat as integer
    dim iLoop as integer
    dim oDocument as object

    ' Handle infamous optional parameter
    If ( IsMissing( _iLoop ) ) then
        iLoop = 1
    else
        iLoop = _iLoop
    endif

    hSetDocumentContext() ' set kontext to current document type
    oDocument = hSetDocumentObject() ' Get the document object we want to write to

    try
        for iRepeat = 1 to iLoop
            oDocument.TypeKeys( OutputText )
            wait( 200 )
        next iRepeat
    catch
        warnlog( CFN & "Failed to send keystroke to given context" )
    endcatch

end sub

'*******************************************************************************

function hSetDocumentContext() as string

    ' link gApplication to its document kontext

    dim cDocumentContext as string

    select case( gApplication )
    case "DRAW"             : cDocumentContext = "DocumentDraw"
    case "IMPRESS"          : cDocumentContext = "DocumentImpress"
    case "WRITER"           : cDocumentContext = "DocumentWriter"
    case "CALC"             : cDocumentContext = "DocumentCalc"
    case "MATH"             : cDocumentContext = "DocumentMath"
    case "HTML"             : cDocumentContext = "DocumentWriterWeb"
    case "MASTERDOCUMENT"   : cDocumentContext = "DocumentMasterDoc"
    case "CHART"            : cDocumentContext = "DocumentChart"
    case "BACKGROUND"       : cDocumentContext = "Background"
    case else               : warnlog( "Unknown gApplication: " & gApplication )
    end select

    kontext cDocumentContext : hSetDocumentContext() = cDocumentContext

end function

'*******************************************************************************

function hSetDocumentObject() as object

    ' link gApplication to its document object

    dim oDocumentObject as object

    select case( gApplication )
    Case "WRITER"         : oDocumentObject = DocumentWriter
    Case "MASTERDOCUMENT" : oDocumentObject = DocumentMasterDoc
    Case "HTML"           : oDocumentObject = DocumentWriterWeb
    case "CALC"           : oDocumentObject = DocumentCalc
    case "DRAW"           : oDocumentObject = DocumentDraw
    case "IMPRESS"        : oDocumentObject = DocumentImpress
    case "MATH"           : oDocumentObject = DocumentMath
    case "CHART"          : oDocumentObject = DocumentChart
    case "BACKGROUND"     : oDocumentObject = Desktop
    case else             : warnlog( "Unknown gApplication: " & gApplication )
    end select

    hSetDocumentObject() = oDocumentObject

end function

'*******************************************************************************

function hSetDocumentFactory() as string

    ' link gApplication to its factory as used by FileOpen slot

    dim cDocumentFactory as string

    select case( gApplication )
    case "DRAW"             : cDocumentFactory = "private:factory/sdraw"
    case "IMPRESS"          : cDocumentFactory = "private:factory/simpress"
    case "WRITER"           : cDocumentFactory = "private:factory/swriter"
    case "CALC"             : cDocumentFactory = "private:factory/scalc"
    case "MATH"             : cDocumentFactory = "private:factory/smath"
    case "HTML"             : cDocumentFactory = "private:factory/swriter/web"
    case "MASTERDOCUMENT"   : cDocumentFactory = "private:factory/swriter/GlobalDocument"
    case "DATABASE"         : cDocumentFactory = "private:factory/sdatabase?Interactive"
    case else               : warnlog( "Unknown gApplication: " & gApplication )
    end select

    hSetDocumentFactory() = cDocumentFactory

end function