'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'*
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_doc1.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:10 $
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
'* owner : thorsten.bosbach@sun.com
'*
'* short description : Global Routines for Document Handling
'*
'\*************************************************************************************

sub hNewDocument ( optional bANewDoc )
    '/// hNewDocument : open a new document dependent on 'gApplication' ///'
    dim sTemp as string
    gApplication = gApplication

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
    '/// hCreateLabels : open the tab-dialog for making a new lable (file/new/lable) ///'
    FileOpen "FileName", "private:factory/swriter?slot=21051", "FrameName", "_default", "SynchronMode", TRUE
    Sleep (2)
    Kontext
    Active.Setpage TabEtiketten
    Kontext "TabEtiketten"
    if Not TabEtiketten.Exists then
        Warnlog "Dialog for Labels is not up!"
        hCreateLabels = False
    else
        hCreateLabels = True
    endif
    Sleep (2)
end function
'
'-------------------------------------------------------------------------------
'
function hCreateBusinessCards() as Boolean
    '/// hCreateBusinessCards : open the tab-dialog for making a new business card (file/new/business cards) ///'
    FileOpen "FileName", "private:factory/swriter?slot=21052", "FrameName", "_default", "SynchronMode", TRUE
    Sleep (2)
    Kontext
    Active.Setpage TabEtikettenMedium
    Kontext "TabEtikettenMedium"
    if Not TabEtikettenMedium.Exists then
        Warnlog "Dialog for BusinessCards is not up!"
        hCreateBusinessCards = False
    else
        hCreateBusinessCards = True
    endif
    Sleep (2)
end function
'
'-------------------------------------------------------------------------------
'
sub hCloseDocument ( optional bANewDoc )
    '/// hCloseDocument : close a document without saving ///'
    '///+ all documents will be closed without saving ///'
    Dim sFehler$

    if IsMissing ( bANewDoc ) <> TRUE then
        if bANewDoc = TRUE then
            gNoNewDoc = FALSE
        else
            gNoNewDoc = TRUE
        end if
    end if

    ' if no new document was created, it isn't closed
    if gNoNewDoc = TRUE then
        exit sub
    end if

    Sleep 3
    try
        FileClose
    catch
        Exceptlog
        exit sub
    endcatch

    Sleep 1
    Kontext "Active"
    if Active.Exists(2) then
        try
            Active.No
        catch
            Active.Click ( 202 )
        endcatch
    end if
    Sleep (2)
end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseClick ( X%, Y%, optional mb% )
    '/// gMouseClick ( x_Position, y-Position ) : make a mouseclick on the document (dependent on 'gApplication') ///'
    '/// default left mousebutton will be used otherwise you can optionally give the mousebutton to press
    '/// 1 = left mouse button
    '/// 2 = left mouse button
    '/// 3 = left mouse button

    gApplication = gApplication

    if IsMissing(mb%) then mb% = 1

    select case gApplication
    case "BACKGROUND"
        Kontext "BACKGROUND"
        autoexecute = false
        Desktop.MouseDown ( X%, Y%, mb% )
        Desktop.MouseUp ( X%, Y%, mb% )
        autoexecute = true
    case "CALC"
        Kontext "DocumentCalc"
        autoexecute = false
        DocumentCalc.MouseDown ( X%, Y%, mb% )
        DocumentCalc.MouseUp ( X%, Y%, mb% )
        autoexecute = true
    case "DRAW"
        Kontext "DocumentDraw"
        autoexecute=false
        DocumentDraw.MouseDown ( X%, Y%, mb% )
        DocumentDraw.MouseUp ( X%, Y%, mb% )
        autoexecute=true
    case "WRITER"
        Kontext "DocumentWriter"
        autoexecute=false
        DocumentWriter.MouseDown ( X%, Y%, mb% )
        DocumentWriter.MouseUp ( X%, Y%, mb% )
        autoexecute=true
    case "HTML"
        Kontext "DocumentWriterWeb"
        autoexecute=false
        DocumentWriterWeb.MouseDown ( X%, Y%, mb% )
        DocumentWriterWeb.MouseUp ( X%, Y%, mb% )
        autoexecute=true
    case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        autoexecute=false
        DocumentMasterDoc.MouseDown ( X%, Y%, mb% )
        DocumentMasterDoc.MouseUp ( X%, Y%, mb% )
        autoexecute=true
    case "IMPRESS"
        Kontext "DocumentImpress"
        autoexecute=false
        DocumentImpress.MouseDown ( X%, Y%, mb% )
        DocumentImpress.MouseUp ( X%, Y%, mb% )
        autoexecute=true
    case "MATH"
        Kontext "DocumentMath"
        autoexecute=false
        DocumentMath.MouseDown ( X%, Y%, mb% )
        DocumentMath.MouseDown ( X%, Y%, mb% )
        autoexecute=true
    case "CHART"
        Kontext "DocumentChart"
        autoexecute=false
        DocumentChart.MouseDown ( X%, Y%, mb% )
        DocumentChart.MouseUp ( X%, Y%, mb% )
        autoexecute=true
    end select
    sleep (2)
end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseDoubleClick ( X%, Y% )
    '/// gMouseDoubleClick ( x_Position, y-Position ) : make a mouse-doubleclick on the document ( dependent on 'gApplication' ) ///'
    gApplication = gApplication

    select case gApplication
    case "CALC"
        Kontext "DocumentCalc"
        DocumentCalc.MouseDoubleClick ( X%, Y% )
    case "DRAW"
        Kontext "DocumentDraw"
        DocumentDraw.MouseDoubleClick ( X%, Y% )
    case "BACKGROUND"
        Kontext "BACKGROUND"
        Desktop.MouseDoubleClick ( X%, Y% )
    case "WRITER"
        Kontext "DocumentWriter"
        DocumentWriter.MouseDoubleClick ( X%, Y% )
    case "HTML"
        Kontext "DocumentWriterWeb"
        DocumentWriterWeb.MouseDoubleClick ( X%, Y% )
    case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDoubleClick ( X%, Y% )
    case "IMPRESS"
        Kontext "DocumentImpress"
        DocumentImpress.MouseDoubleClick ( X%, Y% )
    case "MATH"
        Kontext "DocumentMath"
        DocumentMath.MouseDoubleClick ( X%, Y% )
    end select
    Sleep (2)
end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseMove ( BeginX%, BeginY%, EndX%, EndY% )
    '/// gMouseMove ( BeginX, BeginY, EndX, EndY ) : make a mousemove trom Bx,By to Ex,Ey on the document ( dependent on 'gApplication' ) ///'
    gApplication = gApplication

    select case gApplication
    case "CALC"
        Kontext "DocumentCalc"
        DocumentCalc.MouseDown ( BeginX%, BeginY% )
        DocumentCalc.MouseMove ( EndX%, EndY%)
        DocumentCalc.MouseUp ( EndX%, EndY% )
    case "DRAW"
        Kontext "DocumentDraw"
        DocumentDraw.MouseDown ( BeginX%, BeginY% )
        DocumentDraw.MouseMove ( EndX%, EndY% )
        DocumentDraw.MouseUp ( EndX%, EndY% )
    case "WRITER"
        Kontext "DocumentWriter"
        DocumentWriter.MouseDown ( BeginX%, BeginY% )
        DocumentWriter.MouseMove ( EndX%, EndY%)
        DocumentWriter.MouseUp ( EndX%, EndY% )
    case "HTML"
        Kontext "DocumentWriterWeb"
        DocumentWriterWeb.MouseDown ( BeginX%, BeginY% )
        DocumentWriterWeb.MouseMove ( EndX%, EndY%)
        DocumentWriterWeb.MouseUp ( EndX%, EndY% )
    case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDown ( BeginX%, BeginY% )
        DocumentMasterDoc.MouseMove ( EndX%, EndY%)
        DocumentMasterDoc.MouseUp ( EndX%, EndY% )
    case "IMPRESS"
        Kontext "DocumentImpress"
        DocumentImpress.MouseDown ( BeginX%, BeginY% )
        DocumentImpress.MouseMove ( EndX%, EndY%)
        DocumentImpress.MouseUp ( EndX%, EndY% )
    case "MATH"
        Kontext "DocumentMath"
        DocumentMath.MouseDown ( BeginX%, BeginY% )
        DocumentMath.MouseMove ( EndX%, EndY%)
        DocumentMath.MouseDown ( EndX%, EndY% )
    end select
    Sleep (2)
end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseDown ( BeginX%, BeginY% )
    '/// gMouseDown ( x_Position, y-Position ) : make a mousedown on the document (dependent on 'gApplication') ///'
    '///+ DON'T FORGETT to call gMouseUp !  ///'
    gApplication = gApplication

    select case gApplication
    case "CALC"
        Kontext "DocumentCalc"
        DocumentCalc.MouseDown ( BeginX%, BeginY% )
    case "DRAW"
        Kontext "DocumentDraw"
        DocumentDraw.MouseDown ( BeginX%, BeginY% )
    case "WRITER"
        Kontext "DocumentWriter"
        DocumentWriter.MouseDown ( BeginX%, BeginY% )
    case "HTML"
        Kontext "DocumentWriterWeb"
        DocumentWriterWeb.MouseDown ( BeginX%, BeginY% )
    case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDown ( BeginX%, BeginY% )
    case "IMPRESS"
        Kontext "DocumentImpress"
        DocumentImpress.MouseDown ( BeginX%, BeginY% )
    case "MATH"
        Kontext "DocumentMath"
        DocumentMath.MouseDown ( BeginX%, BeginY% )
    end select
    Sleep (2)
end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseMove2 ( EndX%, EndY% )
    '/// gMouseMove2 ( x_Position, y-Position ) : move the pointer to position on the document (dependent on 'gApplication') ///'
    gApplication = gApplication

    select case gApplication
    case "CALC"
        Kontext "DocumentCalc"
        DocumentCalc.MouseMove ( EndX%, EndY%)
    case "DRAW"
        Kontext "DocumentDraw"
        DocumentDraw.MouseMove ( EndX%, EndY% )
    case "WRITER"
        Kontext "DocumentWriter"
        DocumentWriter.MouseMove ( EndX%, EndY%)
    case "HTML"
        Kontext "DocumentWriterWeb"
        DocumentWriterWeb.MouseMove ( EndX%, EndY%)
    case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseMove ( EndX%, EndY%)
    case "IMPRESS"
        Kontext "DocumentImpress"
        DocumentImpress.MouseMove ( EndX%, EndY%)
    case "MATH"
        Kontext "DocumentMath"
        DocumentMath.MouseMove ( EndX%, EndY%)
    end select
    Sleep (2)
end sub
'
'-------------------------------------------------------------------------------
'
sub gMouseUp ( EndX%, EndY% )
    '/// gMouseUp ( x_Position, y-Position ) : make a release button on the document (dependent on 'gApplication') ///'
    gApplication = gApplication

    select case gApplication
    case "CALC"
        Kontext "DocumentCalc"
        DocumentCalc.MouseUp ( EndX%, EndY% )
    case "DRAW"
        Kontext "DocumentDraw"
        DocumentDraw.MouseUp ( EndX%, EndY% )
    case "WRITER"
        Kontext "DocumentWriter"
        DocumentWriter.MouseUp ( EndX%, EndY% )
    case "HTML"
        Kontext "DocumentWriterWeb"
        DocumentWriterWeb.MouseUp ( EndX%, EndY% )
    case "MASTERDOCUMENT"
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseUp ( EndX%, EndY% )
    case "IMPRESS"
        Kontext "DocumentImpress"
        DocumentImpress.MouseUp ( EndX%, EndY% )
    case "MATH"
        Kontext "DocumentMath"
        DocumentMath.MouseUp ( EndX%, EndY% )
    end select
    Sleep (2)
end sub
'
'-------------------------------------------------------------------------------
'
sub hTypeKeys ( OutputText , optional iLoop as Integer )
    '/// hTypeKeys  ( OutputText , optional iLoop as Integer ): type the keys in 'outputtext' 'iLoop' times ///'
    Dim i as integer

    If IsMissing(iLoop) = True then iLoop = 1
    For i = 1 to iLoop
        Select Case Ucase(gApplication)
        Case "WRITER"
            Kontext "DocumentWriter"
            DocumentWriter.TypeKeys OutputText
        Case "MASTERDOCUMENT"
            Kontext "DocumentMasterDoc"
            DocumentMasterDoc.TypeKeys OutputText
        Case "HTML"
            Kontext "DocumentWriterWeb"
            DocumentWriterWeb.TypeKeys OutputText
        case "CALC"
            Kontext "DocumentCalc"
            DocumentCalc.TypeKeys OutputText
        case "DRAW"
            Kontext "DocumentDraw"
            DocumentDraw.TypeKeys OutputText
        case "IMPRESS"
            Kontext "DocumentImpress"
            DocumentImpress.TypeKeys OutputText
        case "MATH"
            Kontext "DocumentMath"
            DocumentMath.TypeKeys OutputText
        end select
        wait 500
    next i
end sub

