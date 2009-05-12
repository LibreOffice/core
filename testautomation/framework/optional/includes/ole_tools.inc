'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: ole_tools.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:15 $
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

sub hSetToStandardView ( DieApp$ )
    select case DieApp$
    case "WRITER"
    gApplication = "WRITER"
        Call hNewDocument
        Kontext "DocumentWriter"
    case "MASTERDOCUMENT"
    gApplication = "MASTERDOCUMENT"
        Call hNewDocument
        Kontext "DocumentMasterDoc"
    end select
    ViewZoom
    Kontext "Massstab"
    Optimal.Check
    Massstab.ok()
    Call hCloseDocument
end sub

'*******************************************************************************

sub SendEscape(optional iTimes as Integer)
    dim i as Integer
    const ICWAIT as Integer = 1
    
    if IsMissing( iTimes ) then
        iTimes = 1
    endif
    '///+press [ESCAPE]
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

sub DisableNavigator()
    printlog( "    - disable the navigator in globaldoc/writer" )
    select case gApplication
    case "MASTERDOCUMENT" : Kontext "NavigatorGlobalDoc"
        if NavigatorGlobalDoc.Exists() then
            ViewNavigator
        endif
    case "WRITER"    : Kontext "NavigatorWriter"
        if NavigatorWriter.Exists() then
            ViewNavigator
        endif
    end select
    sleep( 1)
end sub

'*******************************************************************************

sub UncheckAutoFileExtension()
    '///+uncheck automatic file extension, if checked
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
    '///+activate the OLE-Object with a doubleclick
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
        EditObjectEdit
        sleep( ICWAIT )
            case else
    warnlog( "Invalid gApplication: " & gApplication )

    end select
end sub

'*******************************************************************************

sub OLESetFocus2()
    '///+activate the OLE-Object with a doubleclick
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
            sleep( ICWAIT )
            EditObjectEdit
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
            Sleep ( ICWAIT )
            EditObjectEdit
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
            sleep( ICWAIT )
            EditObjectEdit
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
            sleep( ICWAIT )
            EditObjectEdit
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
    '///+Deselect OLE object
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
    '///+Open a new document
    printlog( "Open a new document" )
    hNewDocument
    'use the tiny little hammer to kill the presentation float
    printlog( "Reset the application (which closes the presentation toolbar)" )
    resetapplication
    '///+Open a new document
    printlog( "Open a new document" )
    hNewDocument
    '///+Look if the presentation toolbar is visible, if yes->warning
    Kontext "CommonTaskbar"
    try
        printlog( "Try to click a button on the presentation toolbar" )
        Seiteduplizieren.click()
        warnlog( "The presentation toolbar is open" )
    catch
        printlog( "Button is unavailable. Good" )
    endcatch
    '///+Close the document
    printlog( "Close the document" )
    hCloseDocument
    gApplication = "WRITER" ' reset, just in case...
end sub


'*******************************************************************************

function hStepThroughChartWizard() as boolean
    
    
    '///<h3>Step through the Chart Wizard leaving all defaults</h3>
    '///<i>This functin just steps through the Chart Wizard using the
    '///+ &quot;Next...&quot; button on each page. It will not change any
    '///+ values under way. The Wizard is closed using the &quot;Finish&quot;
    '///+ button.</i><br><br>
    
    '///<u>Parameter(s):</u><br>
    '///<ol>
    '///+<li>No input parameters</li>
    '///</ol>
    
    
    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Errorcondition (Boolean)</li>
    '///<ul>
    '///+<li>TRUE: Wizard finished without errors</li>
    '///+<li>FALSE: Wizard did not open</li>
    '///+<li>In case of problems you need to perfom recovery yourself</li>
    '///</ul>
    '///</ol>
    
    const CFN = "hStepThroughChartWizard::"
    printlog( CFN & "Enter" )
    dim brc as boolean 'a multi purpose boolean returnvalue
    
    '///<u>Description:</u>
    '///<ul>
    '///+<li>Verify that the Chart Wizard is open</li>
    Kontext "ChartWizard"
    if ( not chartWizard.exists( 1 ) ) then
        
        warnlog( CFN & "Chart Wizard not open within reasonable time" )
        hStepThroughChartWizard() = false
        
    else
        
        '///+<li>Click &quot;Next&quot; on the first page of the Chart wizard</li>
        Kontext "ChartWizard"
        GoNext.Click()
        
        '///+<li>Click &quot;Next&quot; on the second page of the Chart wizard</li>
        Kontext "ChartWizard"
        GoNext.Click()
        
        '///+<li>Click &quot;Next&quot; on the third page of the Chart wizard</li>
        Kontext "ChartWizard"
        GoNext.Click()
        
        '///+<li>Click &quot;Finish&quot; on the fourth page of the Chart wizard</li>
        Kontext "ChartWizard"
        ChartWizard.ok()
        Sleep( 3 )
        
    endif
    
    '///+<li>Verify that the Chart Wizard is closed</li>
    kontext "ChartWizard"
    if ( ChartWizard.exists( 1 ) ) then
        warnlog( CFN & "Chart Wizard is still open" )
    endif
    
    '///</ul>
    
    printlog( CFN & "Exit" )
    
    hStepThroughChartWizard() = true
    
end function

