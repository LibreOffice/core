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
'* short description : global-level-1-test -> insert all OLE-Objects out of OLE-dialog into all doc-types
'*
'\******************************************************************************

sub hReopenDoc
    'Only for bughandling if closing an OLE object fails
    Warnlog "Cannot release Chart Object -> #107005#?"
    hCloseDocument()
    'if Active.Exists ( 1 ) then Active.TypeKeys "<RIGHT><ENTER>"
    hNewDocument()
end sub

'*******************************************************************************

sub hOleSelektieren ( xStart%, yStart%, xEnde%, yEnde% )
    WL_TB_ZF_Auswahl
    select case gApplication
    case "CALC"     :Kontext "DocumentCalc"
        DocumentCalc.MouseDown xStart%, yStart%
        DocumentCalc.MouseMove xEnde%, yEnde%
        DocumentCalc.MouseUp xEnde%, yEnde%
        wait 200
    case "DRAW"     :Kontext "DocumentDraw"
        DocumentDraw.MouseDown xStart%, yStart%
        DocumentDraw.MouseMove xEnde%, yEnde%
        DocumentDraw.MouseUp xEnde%, yEnde%
    case "WRITER"   :Kontext "DocumentWriter"
        DocumentWriter.MouseDown xStart%, yStart%
        DocumentWriter.MouseMove xEnde%, yEnde%
        DocumentWriter.MouseUp xEnde%, yEnde%
    case "IMPRESS"  :Kontext "DocumentImpress"
        DocumentImpress.MouseDown xStart%, yStart%
        DocumentImpress.MouseMove xEnde%, yEnde%
        DocumentImpress.MouseUp xEnde%, yEnde%
    case "MATH"     :Kontext "DocumentMath"
        DocumentMath.MouseDown xStart%, yStart%
        DocumentMath.MouseMove xEnde%, yEnde%
        DocumentMath.MouseUp xEnde%, yEnde%
    case "MASTERDOCUMENT":Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDown xStart%, yStart%
        DocumentMasterDoc.MouseMove xEnde%, yEnde%
        DocumentMasterDoc.MouseUp xEnde%, yEnde%
    end select
end sub

'*******************************************************************************

function hSetToStandardView ( cApplication as string ) as boolean

    const MAX_WAIT_FOR_DIALOG = 2

    select case ( cApplication )
    case "WRITER"
        gApplication = cApplication
        Call hNewDocument
        Kontext "DocumentWriter"
    case "MASTERDOCUMENT"
        gApplication = cApplication
        Call hNewDocument
        Kontext "DocumentMasterDoc"
    case else
        printlog( "Application not supported by this function: " & cApplication )
        hSetToStandardView() = false
        exit function
    end select

    hUseAsyncSlot( "ViewZoom" )

    Kontext "Massstab"
    if ( Massstab.exists( MAX_WAIT_FOR_DIALOG ) ) then
        Optimal.Check()
        hCloseDialog( Massstab, "ok" )
    else
        warnlog( "Zoom-Dialog did not open" )
    endif

    Call hCloseDocument
    hSetToStandardView() = true

end sub

'*******************************************************************************

sub SendEscape(optional iTimes as Integer)
    dim i as Integer
    const ICWAIT as Integer = 1
    
    if IsMissing( iTimes ) then
        iTimes = 1
    endif
    printlog( "    - send <ESCAPE> keystroke to document" )
    select case gApplication
    case "WRITER"
    Kontext "DocumentWriter"
        for i = 1 to iTimes
            sleep( ICWAIT )
            DocumentWriter.TypeKeys( "<ESCAPE>" , 1 , TRUE )
        next i
    case "CALC"
    Kontext "DocumentCalc"
        for i = 1 to iTimes
            sleep( ICWAIT )
            DocumentCalc.TypeKeys( "<ESCAPE>" , 1 , TRUE )
        next i
    case "MASTERDOCUMENT"
    Kontext "DocumentMasterDoc"
        for i = 1 to iTimes
            sleep( ICWAIT )
            DocumentMasterDoc.TypeKeys( "<ESCAPE>" , 1 , TRUE )
        next i
    case "DRAW"
    Kontext "DocumentDraw"
        for i = 1 to iTimes
            sleep( ICWAIT )
            DocumentDraw.TypeKeys( "<ESCAPE>" , 1 , TRUE )
        next i
    case "IMPRESS"
    Kontext "DocumentImpress"
        for i = 1 to iTimes
            sleep( ICWAIT )
            DocumentImpress.TypeKeys( "<ESCAPE>" , 1 , TRUE )
        next i
    case else
    warnlog( "Invalid gApplication: " & gApplication )
    end select
end sub

'*******************************************************************************

sub UncheckAutoFileExtension()
    Kontext "SpeichernDlg"
    if SpeichernDlg.exists(5) then
        if AutomatischeDateinamenserweiterung.Exists() then
            printlog("l1_ole_tools::UncheckAutoFileExtension  - unchecking automatic file extension" )
            If AutomatischeDateinamenserweiterung.IsChecked() then
                AutomatischeDateinamenserweiterung.Uncheck()
            endif
        else
            QAErrorLog ( "OBSOLETE: l1_ole_tools::UncheckAutoFileExtension Checkbox 'Automatic Filename Extension' unavailable" )
        endif
    else
        warnlog( "l1_ole_tools::UncheckAutoFileExtension SaveAs Dialog not open." )
    endif
end sub

'*******************************************************************************

function getExtension( optional sExtension as string ) as string
    qaerrorlog( "getExtension is outdated. Use hGetSuffix( ... ) instead" )
    printlog("l1_ole_ttols::getExtension:: - define the expected file extension" )
    select case gApplication
    case "WRITER"
    sExtension = ".sxw"
    case "DRAW"
    sExtension = ".sxd"
    case "IMPRESS"
    sExtension = ".sxi"
    case "CALC"
    sExtension = ".sxc"
    case "MATH"
    sExtension = ".sxm"
    case "MASTERDOCUMENT"
    sExtension = ".sgl"
    case else
    warnlog( "Invalid gApplication: " & gApplication )
    end select
    getExtension = sExtension
end function

'*******************************************************************************

sub OLESetFocus()
    const ICWAIT as Integer = 10
    PrintLog "    - set focus to OLE object (edit mode)"
    select case gApplication
    case "WRITER"
    printlog "      Select and deselect " + gApplication
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys("<F6>"           , 4 , TRUE )
        DocumentWriter.TypeKeys("<DOWN>"         , 3 , TRUE )
        DocumentWriter.TypeKeys("<RIGHT>"        , 1 , TRUE )
        DocumentWriter.TypeKeys("<MOD1 RETURN>"  , 1 , TRUE )
        DocumentWriter.TypeKeys("<TAB>"          , 1 , TRUE )
        DocumentWriter.TypeKeys("<RETURN>"       , 1 , TRUE )
        'DocumentWriter.MouseDoubleClick ( 50, 50 )
        sleep( ICWAIT )
        call SendEscape( 3 ) '(to close the float as well)
    case "IMPRESS"
    printlog "      Select and deselect " + gApplication
        Kontext "DocumentImpress"
        DocumentImpress.MouseDoubleClick ( 50, 50 )
        sleep( ICWAIT )
    case "CALC"
    printlog "      Select and deselect " + gApplication
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys( "<F6>"            , 5 , TRUE )
        DocumentCalc.TypeKeys( "<DOWN>"          , 3 , TRUE )
        DocumentCalc.TypeKeys( "<MOD1 RETURN>"   , 1 , TRUE )
        DocumentCalc.TypeKeys( "<RETURN>"        , 1 , TRUE )
        'gMouseClick ( 20, 20 )
        'DocumentCalc.MouseDoubleClick ( 20, 20 )
        sleep( ICWAIT )
        call SendEscape( 3 ) '(to close the float as well)
    case "MASTERDOCUMENT"
    printlog "      Select and deselect " + gApplication
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDown( 50 , 50 , 1 )
        DocumentMasterDoc.MouseUp( 50 , 50 , 1 )
        hUseAsyncSlot( "EditObjectEdit" )
        sleep( ICWAIT )
            case else
    warnlog( "Invalid gApplication: " & gApplication )

    end select
end sub

'*******************************************************************************

sub OLESetFocus2()
    'default waitstate
    const ICWAIT as Integer = 1
    PrintLog "    - set focus to OLE object (edit mode)"
    select case gApplication
    case "WRITER"
    Kontext "DocumentWriter"
        try
            FormatObject
            Kontext ' This is by intention, do not change!
            active.SetPage TabType
            Kontext "TabType"
            TabType.Cancel()
            hUseAsyncSlot( "EditObjectEdit" )
        catch
            warnlog "Can't activate object"
        endcatch
        sleep( ICWAIT * 2 )
        gMouseClick ( 5 , 5 ) 'out of edit mode
        gMouseClick ( 5 , 5 ) 'remove focus
    case "DRAW"
    Kontext "DocumentDraw"
        if OLEApp = "MATH" then
            DocumentDraw.MouseDoubleClick ( 45, 46 )
        else
            DocumentDraw.MouseDoubleClick ( 50, 45 )
        end if
        sleep( ICWAIT * 5 )
        gMouseClick ( 1 , 1 )
    case "IMPRESS"
    Kontext "DocumentImpress"
        printlog( "    - using accessibility shortcuts for objecthandling" )
        printlog( "      (select, activate and release object)" )
        DocumentImpress.TypeKeys( "<F6>" , 5 , TRUE )
        DocumentImpress.TypeKeys( "<TAB><RETURN>" )
        sleep( ICWAIT * 2 )
        call SendEscape()
    case "CALC"
    Kontext "DocumentCalc"
        printlog( "    - using accessibility shortcuts for objecthandling" )
        printlog( "      (select, activate and release object)" )
        DocumentCalc.TypeKeys( "<F6>"          , 5 , TRUE )
        DocumentCalc.TypeKeys( "<DOWN>"        , 3 , TRUE )
        DocumentCalc.TypeKeys( "<RIGHT>"       , 1 , TRUE )
        DocumentCalc.TypeKeys( "<MOD1 RETURN>" , 1 , TRUE )
        DocumentCalc.TypeKeys( "<RETURN>"      , 1 , TRUE )
        sleep( ICWAIT * 2 )
        call sendEscape()
    case "MASTERDOCUMENT"
    Kontext "DocumentMasterDoc"
        try
            FormatObject
            sleep( ICWAIT )
            Kontext ' This is by intention, do not change!
            Active.SetPage TabType
            Kontext "TabType"
            TabType.Cancel
            hUseAsyncSlot( "EditObjectEdit" )
        catch
            qaerrorlog( "The object is not selected after reload." )
        endcatch
        Sleep( ICWAIT )
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDoubleClick ( 1 , 1 )
    case else
    warnlog( "Invalid gApplication: " & gApplication )
        
    end select
end sub

'*******************************************************************************

sub OLESetFocus3()
    'TODO: Get rid of as many mousemovements as possible
    'default waitstate
    const ICWAIT as Integer = 1
    printlog( "    - set focus to OLE object (edit mode)" )
    select case gApplication
    case "WRITER"
    Kontext "DocumentWriter"
        try
            FormatObject
            sleep( ICWAIT )
            Kontext ' This is by intention, do not change!
            active.SetPage TabType
            Kontext "TabType"
            TabType.Cancel
            hUseAsyncSlot( "EditObjectEdit" )
        catch
            warnlog "Can't activate object"
        endcatch
        sleep( ICWAIT * 5 )
        gMouseClick ( 1, 1 )
    case "DRAW"
    Kontext "DocumentDraw"
        DocumentDraw.TypeKeys ("<Tab>")
        DocumentDraw.MouseDoubleClick ( 50, 50 )
        sleep( ICWAIT * 5 )
        gMouseClick ( 1, 1 )
    case "IMPRESS"
    Kontext "DocumentImpress"
        printlog( "    - Using accessibility-shortcuts to manipulate object")
        DocumentImpress.TypeKeys( "<TAB>" )
        sleep( ICWAIT )
        DocumentImpress.TypeKeys( "<SHIFT RETURN>" )
        call SendEscape( 3 )
    case "MASTERDOCUMENT"
    Kontext "DocumentMasterDoc"
        try
            FormatObject
            sleep( ICWAIT )
            Kontext ' This is by intention, do not change!
            active.SetPage( TabType )
            Kontext "TabType"
            TabType.Cancel()
            hUseAsyncSlot( "EditObjectEdit" )
        catch
            qaerrorlog( "The object is not selected after reload." )
        endcatch
        sleep( ICWAIT * 5 )
        call SendEscape( 2 )
    case else
    warnlog( "Invalid gApplication: " & gApplication )
        
    end select
end sub

'*******************************************************************************

sub OLERemoveFocus
    printlog( "Remove focus from OLE object for " & gApplication )
    select case gApplication
    case "WRITER"    : call SendEscape( 2 )
    case "MASTERDOCUMENT" : Kontext "DocumentMasterDoc"
        DocumentMasterDoc.MouseDoubleClick ( 1, 1 )
    case "IMPRESS"   : gMouseClick ( 5, 99 )
    case "CALC"      : Kontext "DocumentCalc"
        call SendEscape()
    end select
end sub

'*******************************************************************************

sub ClosePresentationFloat()
    gApplication = "IMPRESS"
    printlog( "Open a new document" )
    hNewDocument
    'use the tiny little hammer to kill the presentation float
    printlog( "Reset the application (which closes the presentation toolbar)" )
    resetapplication
    printlog( "Open a new document" )
    hNewDocument
    Kontext "CommonTaskbar"
    try
        printlog( "Try to click a button on the presentation toolbar" )
        Seiteduplizieren.click()
        warnlog( "The presentation toolbar is open" )
    catch
        printlog( "Button is unavailable. Good" )
    endcatch
    printlog( "Close the document" )
    hCloseDocument
    gApplication = "WRITER" ' reset, just in case...
end sub


'*******************************************************************************

function hStepThroughChartWizard() as boolean
    
    
    
    
    
    
    const CFN = "hStepThroughChartWizard::"
    printlog( CFN & "Enter" )
    dim brc as boolean 'a multi purpose boolean returnvalue
    
    Kontext "ChartWizard"
    if ( not chartWizard.exists( 1 ) ) then
        
        warnlog( CFN & "Chart Wizard not open within reasonable time" )
        hStepThroughChartWizard() = false
        
    else
        
        Kontext "ChartWizard"
        GoNext.Click()
        
        Kontext "ChartWizard"
        GoNext.Click()
        
        Kontext "ChartWizard"
        GoNext.Click()
        
        Kontext "ChartWizard"
        ChartWizard.ok()
        Sleep( 3 )
        
    endif
    
    kontext "ChartWizard"
    if ( ChartWizard.exists( 1 ) ) then
        warnlog( CFN & "Chart Wizard is still open" )
    endif
    
    
    printlog( CFN & "Exit" )
    
    hStepThroughChartWizard() = true
    
end function

