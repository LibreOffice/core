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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : Tools (1)
'*
'\******************************************************************************

private SLEEP_TIME_REQUESTED as integer
private SLEEP_CALLS_SUM as integer
private SLEEP_TIME_USED as integer

function GetClipboardText as string

    '/// Returns the correct clipboard text (also if there is a 'RETURN' at it's end.
    Dim i% : Dim CBText$
    Dim Zwischen$

    wait 500
    GetClipboardText = ""
    CBText$ = GetClipboard
 
    if CBText$ = "" then
        GetClipboardText = ""
        exit function
    end if
 
    if asc ( Right( CBText$, 1 )) = 10 then
        Zwischen$ = Mid( CBText$, 1, len(CBText$)-1 )
        if Zwischen$ <> "" then
            if asc ( Right( Zwischen$, 1 )) = 13 then
                GetClipboardText = Mid( Zwischen$, 1, len(Zwischen$)-1 )
            else
                GetClipboardText = Zwischen$
            end if
        else
            GetClipboardText = Zwischen$
        end if
    else
        if asc ( Right( CBText$, 1 )) = 13 then
            Zwischen$ = Mid( CBText$, 1, len(CBText$)-1 )
            if asc ( Right( Zwischen$, 1 )) = 10 then
                GetClipboardText = Mid( Zwischen$, 1, len(Zwischen$)-1 )
            else
                GetClipboardText = Zwischen$
            end if
        else
            GetClipboardText = CBText$
        end if
    end if
    
end function

'*******************************************************************************

function hDoubleClickInList ( window, Selektion as String, optional bFocus as boolean ) as Boolean

    '/// hDoubleClickInList
    '///+ Makes a double click onto an entry in a list (tested only in <i>style lists</i>)
    '///+ window: name of list ///'
    '///+ selektion: string to find in list ///'
    '///+ bFocus: TRUE: activate the window with mouseclick before leaving ///'
    '///+ ReturnValue: if found: TRUE; else FALSE ///'

    Dim i as Integer
    Dim AlterWert as String
    Dim NeuerWert as String

    NeuerWert = "!=!  !=!"                ' init with dummy value
    window.TypeKeys "<Home>"
    if window.gettext <> Selektion then
        for i=1 to 100 step 2
        
            window.MouseDown 5, i +1
            window.MouseUp 5, i +1
            AlterWert = window.GetText
            window.TypeKeys "<Down>"
            NeuerWert = Window.GetText
            window.TypeKeys "<Up>"
            
            if AlterWert = Selektion then
            
                window.MouseDown 5, i +1
                window.MouseUp 5, i +1
                
                ' catch if <down> had any effects
                if Window.GetText = Selektion then     
                
                    window.MouseDoubleClick 5, i +1
                    
                    ' if optional parameter provided
                    if (isMissing (bFocus) = FALSE) then 
                        window.MouseDown 5, i +1
                        window.MouseUp 5, i +1
                    endif
                    
                    i = 202
                else
                    i=0                                 ' start at top of list
                end if
            else
            
                if AlterWert = NeuerWert then
                    Warnlog "'" + Selektion + "' wasn't found in list!"
                    i = 202
                else
                
                    if i > 98 then
                        i=40                  ' list not at end, but scrolled
                    endif
                    
                end if
                
            end if
            
        next i
        
        if i < 200 OR i > 100 then
            hDoubleClickInList = FALSE
        else
            hDoubleClickInList = TRUE
        end if
        
    else
        window.TypeKeys "<Return>"
        hDoubleClickInList = TRUE
    endif

end function

'*******************************************************************************

sub hMouseClick ( window, xPos, yPos )
    
    ' Author: Thorsten Ziehm (26.09.2000)
    '/// hMouseClick
    '///+ Do a mouse click on a named window.
    '/// <i>Input</i>:
    '///+ window : The object on which the mouse click should be make (document, listbox, window)
    '///+ xPos : x-position (relativ to the size of the window (1:100)
    '///+ yPos : y-position (relativ to the size of the window (1:100)
    window.MouseDown ( xPos, yPos )
    window.MouseUp ( xPos, yPos )
    
end sub

'*******************************************************************************

function wielange (StrtTime, optional iFormat as integer) as String

    ' Author: Michael Friedrichs
    '/// wielange
    '///+ Returns the time between a start- and an end timeframe.
    '///+  iFormat: 0: default; 1: mysql ///'

    Dim Zeitspanne
    Dim Zeitspannesek
    Dim Zeitspannemin
    Dim Zeitspanneh
    dim sTemp as string

    if isMissing(iFormat) then
        'dim iFormat as integer
        iFormat = 0
    endif

    Zeitspanne = Now() - StrtTime
    Zeitspannesek = Zeitspanne / 1.15741E-05 + 1
    Zeitspanneh = Fix(Zeitspannesek / 3600)
    Zeitspannesek = Zeitspannesek - Zeitspanneh * 3600
    Zeitspannemin = Fix(Zeitspannesek / 60)
    Zeitspannesek = Zeitspannesek - Zeitspannemin * 60
    Zeitspannesek = Fix(Zeitspannesek)
    select case iFormat
    case 0 
        sTemp = "" & Zeitspanneh & "h " & Zeitspannemin & "m " & Zeitspannesek & "s"
    case 1 
    ' mysql format for status.inc
        if Zeitspanneh < 10 then         
            sTemp = "0" & Zeitspanneh & ":"
        else
            sTemp = "" & Zeitspanneh & ":"
        end if
        
        if Zeitspannemin < 10 then
            sTemp = "" & sTemp & "0" & Zeitspannemin & ":"
        else
            sTemp = "" & sTemp & Zeitspannemin & ":"
        end if
        
        if Zeitspannesek < 10 then
            sTemp = "" & sTemp & "0" & Zeitspannesek
        else
            sTemp = "" & sTemp & Zeitspannesek
        end if
        
    case default: 
        qaErrorLog "t_tools1.inc::wielange: optional parameter iFormat out of range!"
        sTemp = ""
    end select

    wielange = sTemp
    
end function

'*******************************************************************************

function Sleep( optional _iSeconds as integer ) as integer

    const CFN = "global::tools::includes::required::Sleep(...): "
    
    const STATUS_NO_DELAY          = 0
    const STATUS_TIMEOUT_EXCEEDED  = 1
    const STATUS_WAITSLOT_CRASHED  = 2
    const STATUS_CLASSIC_WAIT_USED = 3

    ' This is the "classic" behavior of the sleep function. If you did not set 
    ' GLOBAL_USE_NEW_SLEEP to TRUE in your .bas file, this will be used.
    if ( not GLOBAL_USE_NEW_SLEEP ) then
        if ( IsMissing( _iSeconds ) ) then
            wait( 1000 )
        else
            wait( _iSeconds * 1000 )
        endif
        sleep() = STATUS_CLASSIC_WAIT_USED
        exit function
    endif

    ' This is an extended and accelerated version of the "classic" sleep()
    ' subroutine which used to call Wait( n ) with a given number of
    ' milliseconds. This function uses WaitSlot( n ) and is dynamic.
    ' The try...catch block is necessary because WaitSlot() can - under certain
    ' yet unknown conditions - make the office application crash.
    ' When called with 0 seconds we return 0 (dynamic sleep statements within
    ' test initialization can actually call the function with a Zero parameter)
    ' A negative number forces the function to use the classic behavior.
    ' If no time is given the function defaults to 5 seconds.
    ' The function now provides returnvalues:
    ' 0 = Normal WaitSlot() used, this is the preferred method.
    ' 1 = WaitSlot() timeout reached, one extra second was added.
    '     This is bad and the script developer should try to fix it.
    ' 2 = Wait() was used (classic method, fallback).
    ' 3 = Wait() was used (forced old behavior)

    dim iMilliseconds as integer
    dim iStatus as integer       : iStatus      = 0
    dim lBegin as long           : lBegin       = GetSystemTicks
    dim iSeconds as integer      : iSeconds     = 5
    dim iSystemDelay as integer  : iSystemDelay = 1000
    dim iTimeDiff as long        : iTimeDiff    = 0
    
    ' On Solaris we are a little slower, so we increase the system delay a little
    if ( instr( lcase( gtSysName ) , "solaris" ) > 0 ) then
        iSystemDelay = 1500
    endif

    ' Override default wait time (5 seconds) if parameter is given
    if ( not IsMissing( _iSeconds ) ) then
        iSeconds = _iSeconds
    endif

    ' Do exit directly if no wait requested
    if ( iSeconds = 0 ) then
        Sleep() = STATUS_NO_DELAY
        exit function
    endif

    ' We need the time in ms and absolute (parameter can be negative)
    iMilliseconds = abs( iSeconds * 1000 )
    
    ' Here we actually do the delay and generate return values
    ' If WaitSlot() times out, we give an extra second (wait(1000))
    if ( iSeconds > 0 ) then
        try
            if ( WaitSlot( iMilliseconds ) <> WSFinished ) then
                wait( iSystemDelay )
                iStatus = STATUS_TIMEOUT_EXCEEDED
            endif
        catch
            wait( iMilliseconds )
            iStatus = STATUS_WAITSLOT_CRASHED
        endcatch
    else
        Wait( iMilliseconds )
        iStatus = STATUS_CLASSIC_WAIT_USED
    endif
    
    ' Find out how long it took, warn if time was zero (sleep most likely not required)
    iTimeDiff = GetSystemTicks - lBegin
    if ( iTimeDiff = 0 ) then
        printlog( CFN & "Zero time. Please consider removing Sleep() statement" )
    endif
    
    if ( GVERBOSE ) then
        SLEEP_CALLS_SUM      = SLEEP_CALLS_SUM + 1
        SLEEP_TIME_USED      = SLEEP_TIME_USED + iTimeDiff / 1000 ' good enough
        SLEEP_TIME_REQUESTED = SLEEP_TIME_REQUESTED + iSeconds 
        printlog( CFN & "--------------------- New call ---------------------" )
        printlog( CFN & "Total Sleep()-Time requested (seconds): " & SLEEP_TIME_REQUESTED )
        printlog( CFN & "Total Sleep()-Time used (seconds).....: " & SLEEP_TIME_USED      )
        printlog( CFN & "Total number of Sleep()-Calls.........: " & SLEEP_CALLS_SUM      )
    endif
    
    ' Try to make something useful out of the status
    if ( iStatus <> 0 ) then
        printlog( CFN & "Sleep(" & abs( iSeconds ) & "), took " _
        & iTimeDiff & " ms, rc=" & iStatus )
        select case iStatus
        case STATUS_TIMEOUT_EXCEEDED : 
            printlog( CFN & "Timeout exceeded." )
        case STATUS_WAITSLOT_CRASHED : 
            printlog( CFN & "Used Wait(n). WaitSlot() failed." )
        case STATUS_CLASSIC_WAIT_USED : 
            printlog( CFN & "Used Wait(n). Classic behavior forced" )
        end select
    endif
    Sleep() = iStatus

end function

'*******************************************************************************

sub DialogTest( Window, optional iNumber as integer)

    '/// DialogTest
    '///+ Make <i>SnapShots</i>
    '/// <b>Window</b> : the name of the window as declared in qa/qatesttool/global/win/*
    '/// <i>Optional Parameter</i> <b>iNumber</b> : Number to distinguish windows which dynamical change their content but not their ID///'
    '///+ the number has to be provided by the testscript creator ///'
    Dim Ergebnis as Integer
    Dim Ausgabe as String
    Dim UndRaus as Boolean
    Dim sCount as string

    ' evaluate optional parameter
    if isMissing(iNumber) then
        'just one picture
        sCount = ""
    else
        'there will be more pictures with the same ID
        sCount = "_"+iNumber
    endif

    if gDasNicht=0 then
        ' In Place Translation Feature: not used anymore;
        ' The matching of the strings on the later migration step never worked.
        ' Just kept here for historical reasons
        Ausgabe = ""
        UndRaus = FALSE
        while UndRaus = FALSE
            Ausgabe = translate
            if Ausgabe <> "" OR Ausgabe <> "1" then
                if Left ( Ausgabe, 1 ) = "0" then
                    Ausgabe = Right ( Ausgabe, Len( Ausgabe )- 2 )
                    AnhaengenAnDatei ( gOfficePath + "trans_output.txt", Ausgabe )
                end if
            end if
            if Ausgabe = "1" then
                UndRaus = TRUE
            endif
        wend
    else
        ' Usual window check
        try
            if Not window.Exists(2) then
                Warnlog "   - Window nicht existent:" + window.Name + " " + window.ID
                exit sub
            end if
            'To get a history, of what windows are covered, use the following line
            '      AnhaengenAnDatei ( ConvertPath (gOfficePath + "user\work\wieviel.txt"), window.Name + " " + sCount + " : " + window.ID )
        catch
            ExceptLog
        endcatch
    end if

    if gbSnapShot = TRUE then
        'Make Screenshot from dialog and save as HelpID.bmp
        Dim Dummy as String, sName as String, sPicName as String

        'get window ID
        Dummy = Window
        'set filename
        sName = Dummy + sCount + ".bmp"

        'save with respect to application and language
        sCapturePath = ConvertPath (gOfficePath + "user\work\screenshots"+iSprache+"\")
        sPicName = sCapturePath + lCase(gApplication)

        'create directory if it doesn't exist
        if ( not FileExists(sPicName) ) then mkdir sPicName

        sPicName = sPicName + sName
        try
            sleep 1
            window.SnapShot( sPicName )
        catch
            warnlog "t_tools1.inc::DialogTest Failed to save screenshot: '" + sPicName + "'"
        endcatch
        printlog sPicName
    end if
    
end sub

'*******************************************************************************

function hFindeImDokument ( Passage$ , Optional A, optional bRegEx ) as boolean

    ' Author: Joerg Sievers (13.11.2001)
    '/// hFindeImDokument
    '/// Searches via 'Search&Replace'-Dlg in StarOffice Writer, -Clac,
    '///+ -HTML, -GlobalDoc for the string <b>EXACT MATCH</b>.
    '///+ Only ONE TIME and THE FIRST search phrase will be found!
    '/// <i>Optional Parameter</i> <b>a</b> : If you do not want a warnlog message
    '/// <i>Optional Parameter</i> <b>bRegEx</b> : if you look fort an regular expression
    Dim WhatIsIn as string
    Dim bSilent as boolean

    bSilent = NOT isMissing(a)
    gApplication = UCase ( gApplication )
    hFindeImDokument = FALSE

    select case gApplication

    case "CALC" :
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys "<MOD1 HOME>"

    case "WRITER" :
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 HOME>"

    case "HTMLDOKUMENT":
        Kontext "DocumentWriterWeb"
        DocumentWriterWeb.TypeKeys "<MOD1 HOME>"

    case "GLOBALDOC" :
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.TypeKeys "<MOD1 HOME>"
    end select

    SetClipboard ""
    EditSearchAndReplace
    
    Kontext "FindAndReplace"
    if SimilaritySearch.IsVisible = False then
        More.Click
    end if
    
    if MatchCase.IsChecked = False then
        MatchCase.Check
    end if
    
    if SimilaritySearch.IsChecked = TRUE then
        SimilaritySearch.UnCheck
        if NOT bSilent then
            warnlog "'Similarity search' checkbox was enabled! It shouldn't be the default!"
        endif
    end if
    
    if IsMissing(bRegEx) <> TRUE then
        RegularExpressions.Check
    end if
    
    SearchFor.Settext Passage$
    SearchNow.Click
    
    Kontext
    if NOT Active.Exists(2) then
    
        Kontext "FindAndReplace"
        More.Click
        FindAndReplace.Cancel
        EditCopy
        WhatIsIn = GetClipboardText
        
        if WhatIsIn <> Passage$ then
            if NOT bSilent then
                warnlog "The search-request for '" & Passage$ & "' has been fault! (Found: '"& WhatIsIn & "')"
            end if
        else
            hFindeImDokument = TRUE
        end if
        
    else
        try
            Kontext
            if Active.Exists(1) then
                Active.OK
            end if
            
            if NOT bSilent then
                warnlog "The search-request for '" & Passage$ & "' has been fault! (Found: '"& WhatIsIn & "')"
            end if
            
            Kontext "FindAndReplace"
            if SimilaritySearch.IsVisible = False then
                More.Click
            endif
            
            if MatchCase.IsChecked then
                MatchCase.UnCheck
            endif
            
            if SimilaritySearch.IsChecked = TRUE then
            
                SimilaritySearch.UnCheck
                if NOT bSilent then
                    warnlog "'Similarity search' checkbox was enabled! It shouldn't be the default!"
                endif
            end if
            
            if IsMissing(bRegEx) <> TRUE then
                RegularExpressions.UnCheck
            endif
            
            More.Click
            FindAndReplace.Cancel
        catch
            Active.Yes
            
            Kontext
            if bSilent then
                if Active.Exists then
                    printlog "> "+Active.GetText
                endif
            endif
            
            if Active.Exists then
                Active.OK
            endif
            
            if NOT bSilent then
                warnlog "The search-request for '" & Passage$ & "' has been fault! (Found: '"& WhatIsIn & "')"
            endif
            
            Kontext "FindAndReplace"
            if SimilaritySearch.IsVisible = False then
                More.Click
            endif
            
            if MatchCase.IsChecked then
                MatchCase.UnCheck
            endif
            
            if SimilaritySearch.IsChecked = TRUE then
            
                SimilaritySearch.UnCheck
                if NOT bSilent then
                    warnlog "'Similarity search' checkbox was enabled! It shouldn't be the default!"
                endif
            end if
            
            if IsMissing(bRegEx) <> TRUE then
                RegulaererAusdruck.UnCheck
            endif
            
            More.Click
            FindAndReplace.Cancel
            
        endcatch
    end if
    
end function

'*******************************************************************************

function hFindeMehrImDokument ( Passage as string , WieOft as integer ) as boolean

    ' Author: Joerg Sievers (26.07.2000)
    '/// hFindeMehrImDokument
    '/// Searches per 'Search&Replace'-Dlg in StarOffice Writer, -Clac,
    '///+-HTML, -GlobalDoc for the string <b>EXACT MATCH</b>.
    '/// You have to give the function the number how often the phrase
    '///+should be found in the document as an additional parameter (as integer).
    '/// Only when exact the number of the phrase will be found correctly
    '///+the function gives back TRUE.
    '/// <i>see also</i> : hFindeImDokument (TOOLS.INC)
    Dim i as integer
    gApplication = UCase ( gApplication )

    hFindeMehrImDokument = FALSE

    select case gApplication

    case "CALC" : 
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys "<MOD1 HOME>"

    case "WRITER" : 
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 HOME>"

    case "HTMLDOKUMENT": 
        Kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<MOD1 HOME>"

    case "GLOBALDOC" : 
        Kontext "DocumentMasterDoc"
        DocumentMasterDoc.TypeKeys "<MOD1 HOME>"

    end select

    SetClipboard ""
    EditSearchAndReplace

    For i = 1 to WieOft
    
        Kontext "FindAndReplace"
        if NOT MatchCase.IsChecked then
            MatchCase.Check
        endif
        SearchFor.Settext Passage
        SearchNow.Click
        
        Kontext
        if NOT Active.Exists(2) then
        
            FindAndReplace.Cancel
            EditCopy
            
            if GetClipboardText <> Passage then
                warnlog "The search-request for '" & Passage & "' has been fault!"
                i = WieOft
            else
                if i = WieOft then
                    hFindeImDokument = TRUE
                    printlog "Searchphrase found " & i & " time(s)."
                end if
            end if
        else
            try
                Active.OK
                
                Kontext
                if Active.Exists then
                    Active.OK
                endif
                warnlog "The search-request for '" & Passage & "' has been fault!"
                i = WieOft
                Kontext "FindAndReplace"
                
                if MatchCase.IsChecked then
                    MatchCase.UnCheck
                endif
                FindAndReplace.Cancel
            catch
                Active.Yes
                
                Kontext
                if Active.Exists then
                    Active.OK
                endif
                warnlog "The search-request for '" & Passage & "' has been fault!"
                i = WieOft
                
                Kontext "FindAndReplace"
                if MatchCase.IsChecked then
                    MatchCase.UnCheck
                endif
                FindAndReplace.Cancel
            endcatch
        end if
    Next i
    Kontext "FindAndReplace"
    if FindAndReplace.Exists(2) then
        FindAndReplace.Cancel
    end if
    
end function

'*******************************************************************************

sub TextInDatei ( TextText$, Datei$ )

    '/// TextInDatei
    
    Dim FileNum%

    FileNum% = FreeFile
    Open Datei$ for Append  as #FileNum%
    Print #FileNum%, TextText$
    Close #FileNum%
    
end sub

'*******************************************************************************

function TrimTab ( sTrimmer as String ) as String

    '/// TrimTab
    '/// <u>Input</u>: the original text
    '/// Returns the string without &lt;tab&gt;s at the beginning and the end of a string.
    
    Dim sInterim as String

    sInterim = sTrimmer
    sInterim = lTrimTab ( sInterim )
    TrimTab = rTrimTab ( sInterim ) 

end function

'*******************************************************************************

function lTrimTab ( slTrimmer as String ) as String

    '/// lTrimTab
    '/// <u>Input</u>: the original text
    '/// Returns the string without &lt;tab&gt;s at the beginning.
    '/// Cuts &lt;Tab's&gt; at the beginning of a string ( left )
    
    Dim i, iLen as Integer
    Dim sInterim as String

    iLen = len ( slTrimmer )
    sInterim = slTrimmer

    for i=1 to iLen
        if Asc ( left ( sInterim, 1 ) ) = 9 then
            sInterim = Right ( sInterim, len ( sInterim ) - 1 )
        else
            i=iLen+1
        end if
    next i
    lTrimTab = sInterim
end function

'*******************************************************************************

function rTrimTab ( srTrimmer as String ) as String

    '/// rTrimTab
    '/// Input: the original text
    '/// Returns the string without &lt;tab&gt;s at the end.
    '/// Cuts &lt;Tab's&gt; at the beginning of a string ( right )

    Dim i, iLen as Integer
    Dim sInterim as String

    iLen = len ( srTrimmer )
    sInterim = srTrimmer

    for i=1 to iLen
        if Asc ( right ( sInterim, 1 ) ) = 9 then
            sInterim = left ( sInterim, len ( sInterim ) - 1 )
        else
            i=iLen+1
        end if
    next i
    rTrimTab = sInterim

    end function

'*******************************************************************************

function TrimString (Content as String, delim as integer) as String

    ' Author: Frank Heitbrock (26.07.2002)
    '/// TrimString
    '/// <u>Input</u>: The  String, the delimiter which should be cut from the string.
    '/// Returns the String without the delimiter.
    '/// <u>Example</u>:
    '///+ Content = &quot;&nbsp;H&nbsp;a&nbsp;l&nbsp;l&nbsp;o&nbsp;&quot;, delim = 32 (ascii for space character)
    '///+ Return = &quot;Hallo&quot;
    dim strlen as integer, i as integer, k as integer
    dim CharBuff(1 to 100) as String
    dim ResultStr as String
    ' at first cut the empty strings left and right of the String
    Content = lTrim(Content)
    Content = rTrim(Content)
    ' now we search for all appropriate ascii characters in the middle of the String and delete them
    strlen = len(Content)
    k = 1
    for i = 1 to strlen
        if mid(Content, i, 1) <> chr(delim) then
            CharBuff(k) = mid(Content, i, 1)
            k = k +1
        end if
    next i
    for i = 1 to k
        ResultStr = ResultStr + CharBuff(i)
    next i
    TrimString = ResultStr

end function

'*******************************************************************************

function ActiveDeactivateAsianSupport ( WhatState as Boolean ) as Boolean

    ' Author: Thorsten Ziehm
    '/// ActiveDeactivateAsianSupport
    '/// <u>Input</u>: TRUE or FALSE
    '///+ TRUE: The Asian support will be enabled.
    '///+ FALSE:  The Asian support will be disabled.
    '/// <u>Return:</u>
    '///+ TRUE/FALSE for the last state of the checkbox in the office UI.
    ToolsOptions
    hToolsOptions ( "LanguageSettings", "Languages" )

    IF Aktivieren.IsEnabled then        'the checkbox is disabled in asian versions
        ActiveDeactivateAsianSupport = Aktivieren.IsChecked         ' the function gets the old state of the checkbox

        if WhatState = TRUE then
            try
                Aktivieren.Check
            catch
            endcatch
        else
            Aktivieren.UnCheck
        end if
        gAsianSup = WhatState    ' Set the global variable

        Kontext "ExtrasOptionenDlg"
        hCloseDialog( ExtrasOptionenDlg, "ok" )
    else
        ActiveDeactivateAsianSupport = TRUE
        If WhatState = FALSE then
            warnlog "Deactivating of asian language support is not possible, because it is disabled in cjk versions"
        end if
        Kontext "ExtrasOptionenDlg"
        hCloseDialog( ExtrasOptionenDlg, "ok" )
    end if

end function

'*******************************************************************************

function ActiveDeactivateCTLSupport ( WhatState as Boolean ) as Boolean

    ' Author: Hercule Li (March 2004)
    '/// ActiveDeactivateCTLSupport
    '/// <u>Input</u>: TRUE or FALSE
    '/// TRUE :  The CTL will be enabled.
    '/// FALSE:  The CTL will be disabled.
    '/// <u>Return:</u>
    '/// TRUE/FALSE for the last state of the checkbox in the office UI.
    ToolsOptions
    hToolsOptions ( "LanguageSettings", "Languages" )

    IF ComplexScriptEnabled.IsEnabled then        'the checkbox is disabled in CTL versions
        ActiveDeactivateCTLSupport = ComplexScriptEnabled.IsChecked      ' the function gets the old state of the checkbox

        if WhatState = TRUE then
            ComplexScriptEnabled.Check
        else
            ComplexScriptEnabled.UnCheck
        end if
        gCTLSup = WhatState    ' Set the global variable

        Kontext "ExtrasOptionenDlg"
        hCloseDialog( ExtrasOptionenDlg, "ok" )
    else
        ActiveDeactivateCTLSupport = TRUE
        If WhatState = FALSE then
            warnlog "Deactivating of CTL language support is not possible, because it is disabled in ctl versions"
        end if
        Kontext "ExtrasOptionenDlg"
        hCloseDialog( ExtrasOptionenDlg, "ok" )
    end if

end function

'*******************************************************************************

function GetDecimalSeperator ( optional sDummy$ ) as String

    '/// <u>Precondition</u>: Measuring unit has to be set to centimeter (cm) before using this function. (see: fSetMeasurementToCM())
    '///+ <u>Input</u>: Number with fractionmark from <i>NumericField</i> as string
    '///+ <u>Output</u>: A dot (.) or a comma (,) as string
    Dim sCheckForSeparator as string
    Const cWhereIsThisFunction = "qa::qatesttool::global::tools::inc::t_tools1.inc::GetDecimalSeperator: "
    Dim bDotOrCommaIncluded as boolean

    'Setting the determination of a dot or a comma to FALSE until it was successfull.
    bDotOrCommaIncluded = FALSE

    if IsMissing(sDummy$) then
        '/// Opening a new document depending on <i>gApplication</i> value and closing it at the end.
        Call hNewDocument
        '/// Tools / Options / (Modul: gApplication) / General tabpage.
        ToolsOptions
        '///+ <ol><li>Reading the string of the tabulator numeric field</li>
        select case gApplication
        case "WRITER" 
            Call hToolsOptions("WRITER","GENERAL")
            sCheckForSeparator = Tabulatorenabstand.GetText
        case "CALC" 
            Call hToolsOptions("CALC","GENERAL")
            sCheckForSeparator = Tabulator.GetText
        case "IMPRESS" 
            Call hToolsOptions("IMPRESS","GENERAL")
            sCheckForSeparator = Tabulatorenabstand.GetText
        case "DRAW" 
            Call hToolsOptions("DRAW","GENERAL")
            sCheckForSeparator = Tabulatorenabstand.GetText
        case "MASTERDOCUMENT"
            Call hToolsOptions("WRITER","GENERAL")
            sCheckForSeparator = Tabulatorenabstand.GetText
        case "HTML"
            Call hToolsOptions("WRITER","GENERAL")
            sCheckForSeparator = Tabulatorenabstand.GetText
        case else
            warnlog cWhereIsThisFunction & "For this module ("& gApplication &") no decimal seperator setting exists."
        end select
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
        if Instr(sCheckForSeparator, ",") > 0 then
            GetDecimalSeperator = ","
            bDotOrCommaIncluded = TRUE
        endif
        if Instr(sCheckForSeparator, ".") > 0 then
            GetDecimalSeperator = "."
            bDotOrCommaIncluded = TRUE
        endif
        Call hCloseDocument
    else
        '///+ <li>or determining the seperator depending on the OPTIONAL value (string).</li></ol>
        'Get position of fraction mark / get IT
        if InStr (sDummy$, ",") > 0 then
            GetDecimalSeperator = ","
            bDotOrCommaIncluded = TRUE
        endif
        if InStr (sDummy$, ".") > 0 then
            GetDecimalSeperator = "."
            bDotOrCommaIncluded = TRUE
        endif
    endif

    '/// If the determination failed the dot will be used (default) as decimal seperator.
    if bDotOrCommaIncluded = FALSE then
        warnlog cWhereIsThisFunction & "Unable to determine decimal separator. Setting dot (.) as default."
        GetDecimalSeperator = "."
    endif
    printlog "Info: Decimal Seperator is a '" & GetDecimalSeperator & "'."
    
end function

'*******************************************************************************

sub sResetTheOffice as boolean

    Dim uno
    Dim ap
    Dim xViewRoot
    Dim apara(1) As new com.sun.star.beans.PropertyValue
    Dim temp()
    Dim i,x as integer
    Dim sString as string
    Dim fDeleteList(32000) as string
    Dim sLanguage as string
    Dim bError as boolean
    Dim sDefaultLocale as string
    Dim sDefaultLocaleCJK as string
    Dim sDefaultLocaleCTL as string
    Dim sfgetL10Nvalue as string
    Dim sLanOutIni as string

    sString = "qa:qatesttool:calc:options:inc:coption1.inc:: "
    sResetTheOffice = TRUE

    ' only run on UNIX platforms; there is a problem with the quickstarter on win32
    if ("unx" = gPlatgroup) then
        try
            sLanOutIni = fgetL10Nvalue()
        catch
            warnlog sString & "can't get the correct Office-Language!."
            sResetTheOffice = FALSE
            Exit sub
        endcatch

        uno=hGetUnoService()

        'Get UI language
        try
            ap=uno.createInstance("com.sun.star.configuration.ConfigurationProvider")
            apara(0).Name="nodepath"
            apara(0).Value="/org.openoffice.Office.Linguistic/General"
            apara(1).Name="lazywrite"
            apara(1).Value=False
            xViewRoot=ap.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPara())
            sLanguage = sfgetL10Nvalue
            sDefaultLocale = xViewRoot.getPropertyValue("DefaultLocale")
            sDefaultLocaleCJK = xViewRoot.getPropertyValue("DefaultLocale_CJK")
            sDefaultLocaleCTL = xViewRoot.getPropertyValue("DefaultLocale_CTL")
            printlog "Old UI language: '" + sLanOutIni + "'"
            printlog "Old default locale: '" + sDefaultLocale + "'"
            printlog "Old default locale CJK: '" + sDefaultLocaleCJK + "'"
            printlog "Old default locale CTL: '" + sDefaultLocaleCTL + "'"
            xViewRoot.dispose()
            bError = FALSE
        catch
            warnlog sString + "Failed to read UI language."
            bError = TRUE
        endcatch

        if NOT bError then
            'Close OOo
            try
                ' To prevent restarting of OOo, the try/catch is around this and
                ' to prevent messages about communication errors
                printlog ResetApplication
                FileExit "SynchronMode", TRUE
                try
                    ' It is no error, if this fails - so it gets its own try/catch
                    kontext
                    if active.exists(5) then
                        active.no 'discard changes
                    endif
                catch
                endcatch
                bError = FALSE
            catch
                warnlog sString + "Failed to close OOo."
                bError = TRUE
            endcatch
            sleep 10 'To wait until OOo is realy away
        endif

        'only act, if no error and if language <> ''
        if (NOT bError AND sLanguage <> "") then
            'Remove user directory
            try
                if (right(gOfficePath,1)=gPathSigne) then
                    'Dir doesn't work, is a path singe is at the end
                    gOfficePath = left(gOfficePath,len(gOfficePath)-1)
                endif
                printlog "Going to delete directory: '" + gOfficePath + "'"
                if (dir(gOfficePath) = "") then
                    qaErrorlog "Directory is already deleted."
                else
                    rmDir (gOfficePath)
                    if (dir(gOfficePath) <> "") then
                        warnlog "Directory wasn't deleted."
                    endif
                endif
                bError = FALSE
            catch
                warnlog sString + "Failed to delete user directory."
                bError = TRUE
            endcatch
        endif

        'Start OOo and restore language
        'Needs only to be done, if UI language wasn't the default (!= "")
        if ((sLanguage & sDefaultLocale & sDefaultLocaleCJK & sDefaultLocaleCTL) <> "") then
            try
                hStartTheOffice
                Call hDisableQuickstarter
                'Here we need the Exit from a running Quickstarter...
                Call ExitRestartTheOffice
                uno=hGetUnoService()
                ap=uno.createInstance("com.sun.star.configuration.ConfigurationProvider")
                apara(0).Name="nodepath"
                apara(0).Value="/org.openoffice.Office.Linguistic/General"
                apara(1).Name="lazywrite"
                apara(1).Value=False
                xViewRoot=ap.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPara())
                if (sLanOutIni <> "") then
                    printlog "Old UI language: '" + sLanOutIni + "'"
                    xViewRoot.setPropertyValue("UILocale", sLanOutIni)
                    xViewRoot.commitChanges()
                endif
                if (sDefaultLocale <> "") then
                    printlog "Old default locale: '" + sDefaultLocale + "'"
                    xViewRoot.setPropertyValue("DefaultLocale", sDefaultLocale)
                    xViewRoot.commitChanges()
                endif
                if (sDefaultLocaleCJK <> "") then
                    printlog "Old default locale CJK: '" + sDefaultLocaleCJK + "'"
                    xViewRoot.setPropertyValue("DefaultLocale_CJK", sDefaultLocaleCJK)
                    xViewRoot.commitChanges()
                endif
                if (sDefaultLocaleCTL <> "") then
                    printlog "Old default locale CTL: '" + sDefaultLocaleCTL + "'"
                    xViewRoot.setPropertyValue("DefaultLocale_CTL", sDefaultLocaleCTL)
                    xViewRoot.commitChanges()
                endif
                if xViewRoot.hasPendingChanges() then
                    warnlog(sFileFunction+"Changes still pending...")
                endif
                xViewRoot.dispose()
            catch
                warnlog sString + "Failed to set UI language."
                exit sub
            endcatch
        endif

        Call ExitRestartTheOffice
    endif
end sub

'*******************************************************************************

sub raiseApplication

    ' Try to solve focus problem on MacOS X; After calling this function, OOo should be most front;
    dim iCurrentDir as integer
    dim iNumberOfHits as integer : iNumberOfHits = 0
    dim iDirPosition as integer
    dim tBundle as string
    dim aPath ' string array with dynamic itemcount, intended.

    ' Calling just the .app with open on MacOS X via shell command
    if ( lcase( gPlatform ) = "osx" ) then

        ' Split the path into its components
        aPath = split(gNetzOfficePath, gPathSigne)

        ' make sure 'Contents' is just one time in path
        for iCurrentDir = 0 to uBound(aPath)
            if "Contents" = aPath( iCurrentDir ) then iNumberOfHits = iNumberOfHits + 1
        next iCurrentDir
        
        ' exit if not
        if ( iNumberOfHits <> 1 ) then exit sub
        
        iDirPosition = inStr( gNetzOfficePath, "Contents" )
        tBundle = left( gNetzOfficePath, iDirPosition - 2 )

        shell( "open", 1, tBundle, true )
        
    end if
end sub
