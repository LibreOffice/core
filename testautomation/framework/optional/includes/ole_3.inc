'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: ole_3.inc,v $
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
'* short description :
'*
'\******************************************************************************

testcase tDraw_As_OLE_Object()
    
    '///<h1>Insert, save, reload, close files with Draw as OLE object</h1>
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEDraw, "DRAW" )
    'call hInsertOLEObject( false, gOLEDraw, "DRAW" )
    
endcase

testcase tCalc_as_OLE_Object()
    
    '///<h1>Insert, save, reload, close files with Calc as OLE object</h1>
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLECalc, "CALC" )
    'call hInsertOLEObject( false, gOLECalc, "CALC" )
    
endcase

testcase tImpress_As_OLE_Object()
    
    '///<h1>Insert, save, reload, close files with Impress as OLE object</h1>
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEImpress, "IMPRESS" )
    'call hInsertOLEObject( false, gOLEImpress, "IMPRESS" )
    
endcase

testcase tWriter_As_OLE_Object()
    
    '///<h1>Insert, save, reload, close files with Writer as OLE object</h1>
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEWriter, "WRITER" )
    'call hInsertOLEObject( false, gOLEWriter, "WRITER" )
    
endcase

testcase tMath_As_OLE_Object()
    
    '///<h1>Insert, save, reload, close files with Math as OLE object</h1>
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEMath, "MATH" )
    'call hInsertOLEObject( false, gOLEMath, "MATH" )
    
endcase

testcase tChart_As_OLE_Object()
    
    '///<h1>Insert, save, reload, close files with Chart as OLE object</h1>
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEChart, "CHART" )
    'call hInsertOLEObject( false, gOLEChart, "CHART" )
    
endcase

'*******************************************************************************

sub hInsertOLEObject( bRemoveFocus as boolean, cOLEObject as string, cOleType as string )
    
    dim sFile as String
    dim bOleAvailable as boolean
    
    '///<h3>Insert OLE objects into the applications / all possible combinations</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>bRemoveFocus (boolean)</li>
    '///<ul>
    '///+<li>TRUE = Click into the application frame to remove focus from OLE object before saving</li>
    '///+<li>FALSE = Do not remove focus from OLE object</li>
    '///</ul>
    '///+<li>cOLEObject (string).</li>
    '///<ul>
    '///+<li>Any valid entry stored in gOLExxxx (set those with GetOLEDefaultNames())</li>
    '///</ul>
    
    '///+<li>cOLEType (string). valid options are:
    '///<ul>
    '///+<li>&quot;WRITER&quot;</li>
    '///+<li>&quot;CALC&quot;</li>
    '///+<li>&quot;DRAW&quot;</li>
    '///+<li>&quot;IMPRESS&quot;</li>
    '///+<li>&quot;CHART&quot;</li>
    '///+<li>&quot;MATH&quot;</li>
    '///</ul>
    '///</ol>
    
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    
    '///<u>Description</u>:
    '///<ul>
    
    printlog( "" )
    printlog( "Beginning testcase with options: " )
    printlog( "* Remove focus...: " & bRemoveFocus )
    printlog( "* OLE object name: " & cOLEObject )
    printlog( "* OLE object type: " & cOLEType )
    printlog( "* Document type..: " & gApplication )
    printlog( "" )
    
    ' We cannot insert an OLE object of same type as the current document
    ' So writer/writer, calc/calc, masterdoc/writer is skipped ...
    if ( gApplication = cOleType ) then
        printlog( "Skipping " & cOleType & " as OLE object" )
        goto testend
    endif
    
    if ( gApplication = "MASTERDOCUMENT" and cOleType = "WRITER" ) then
        printlog( "Skipping " & cOleType & " as OLE object" )
        goto testend
    endif
    
    '///+<li>Open a new document ( Writer / Calc / Draw / Masterdocument ... )</li>
    hCreateDocument()
    
    '///+<li>Insert / object / OLE-object</li>
    printlog( "Menu: insert / object / OLE object" )
    InsertObjectOLEObject
    
    Kontext "OLEObjektEinfuegen"
    if ( OLEObjektEinfuegen.exists( 1 ) ) then
        
        '///+<li>Select 'create new'</li>
        printlog( "Select 'create new'" )
        NeuErstellen.Check()
        
        '///+<li>Select the draw-OLE-object ( you can find the name for each language at [TesttoolPath]/global/input/olenames/ole_[lang-code].txt )</li>
        printlog( "Select objecttype: " & cOLEObject )
        ObjektTyp.Select( cOLEObject )
        
        '///+<li>Click 'OK'</li>
        printlog( "Click OK" )
        OLEObjektEinfuegen.OK()
        WaitSlot( 3000 )
    else
        warnlog( "Insert OLE object dialog is missing, test ends" )
        hDestroyDocument()
        goto testend
    endif
    
    '///+<li>Optionally remove focus from OLE object</li>
    if ( bRemoveFocus ) then
        printlog( "Remove focus from OLE object" )
        call OLERemoveFocus()
    else
        printlog( "Focus is not to be removed from OLE object" )
        
        ' The following can - in some cases - reproduce a nasty crash-bug when
        ' saving files with selected/activated OLE object
        select case gApplication
        case "DRAW"      : gMouseClick( 99 , 99 )
        case "IMPRESS"   : gMouseClick( 99 , 99 )
        end select
    endif
    
    '///+<li>Build the filename with full path but without suffix</li>
    sFile = ConvertPath ( gOfficePath + "user\work\ole_" & cOleType )
    printlog( "Save the file; File to be written (w/o extension): " & sFile )
    if fileexists(sFile) then
    	kill(sFile)
    	qaerrorlog("killed file from former test run")
    endif
    
    '///+<li>Save the file via FileSave, current fileformat, overwrite</li>
    hFileSaveAsKill( sFile )
    
    '///+<li>Close the document</li>
    printlog( "Close the document" )
    hDestroyDocument()
    
    '///+<li>Load the file again via FileOpen (filename with suffix)</li>
    printlog( "Reopen the file: " & sFile )
    hFileOpen( sFile )
    
    '///+<li>Check that the OLE object is listed in the navigator</li>
    bOleAvailable = isOleAvailable()
    if ( bOleAvailable ) then
        
        '///+<li>Disable the navigator if it exists</li>
        call DisableNavigator()
        
        '///+<li>Set focus to OLE object</li>
        call OLESetFocus()
        
        '///+<li>Remove focus from OLE object</li>
        call SendEscape()
    else
        qaErrorLog( "#i44725# OLE Object not saved in Master Document" )
    endif
    
    '///+<li>Cleanup: Close the document</li>
    printlog( "Cleanup: Close the document" )
    hDestroyDocument()
    
    '///+<li>Cleanup: Delete the workfile</li>
    printlog( "Cleanup: Delete the workfile" )
    hDeleteFile( sFile )
    '///</ul>
    
    ' We end up here when the test has been skipped or aborted
    testend:
    
end sub

'*******************************************************************************

function isOleAvailable() as boolean
    
    '///<h3>Funtion to determine whether OLE is available</h3>
    dim i as integer
    dim a as integer
    
    const CFN = "isOleAvailable::"
    
    printlog( CFN & "Testing whether OLE object is present in Navigator" )
'    qaerrorlog( CFN & "Replace me i'm old, poorly written and undocumented" )
    
    isOleAvailable = FALSE
    ' check if OLE-Objects are in the document with the navigator
    kontext "NavigatorGlobaldoc"
    if NavigatorGlobaldoc.exists( 1 ) then
        printlog CFN & "Navigator globaldoc is available"
        if NOT AuswahlListe.exists then
            Umschalten.click
        endif
        a = AuswahlListe.getItemCount
        ' somehow not all items might be displayed
        if NOT (a>2) then
            inhaltsansicht.click
            i = AuswahlListe.getItemCount
            if NOT(i>a) then
                inhaltsansicht.click
            else
                a = i
            endif
        endif
        AuswahlListe.typeKeys("<home>")
        'fold everything
        for i = 1 to a
            AuswahlListe.typeKeys("-<down>")
        next i
        ' unfold everything, until there is something to unfold
        ' then we found the object we looked for: OLE
        AuswahlListe.typeKeys("<home>")
        i = 0
        a = AuswahlListe.getItemCount
        ' don't run indefinite
        while ((AuswahlListe.getItemCount = a) AND (i < a))
            AuswahlListe.typeKeys("+<down>")
            inc(i)
        wend
        try
            printlog CFN & AuswahlListe.getSelText
        catch
            qaerrorlog CFN & "no item in navigator is selected"
        endcatch
        ' activate object
        AuswahlListe.typeKeys("<return>")
        i = AuswahlListe.getItemCount
        printlog CFN & i
        if (i > a) then
            isOleAvailable = TRUE
        else
            isOleAvailable = FALSE
        endif
    else
        Kontext "NavigatorWriter"
        if NOT NavigatorWriter.exists( 1 ) then
            ViewNavigator
        endif
        Kontext "NavigatorWriter"
        if NavigatorWriter.exists( 2 ) then ' was 5 secs
            printlog CFN & "Navigator writer was available"
            if NOT AuswahlListe.exists then
                Umschalten.click
                Sleep( 5 )
            endif
            a = AuswahlListe.getItemCount
            ' somehow not all items might be displayed
            if a=0 then
            	Sleep( 5 )
            	 a = AuswahlListe.getItemCount
            endif
            if NOT (a>2) then
                inhaltsansicht.click
                Sleep( 3 )
                i = AuswahlListe.getItemCount
                if NOT(i>a) then
                    qaerrorlog "failed"
                else
                    a = i
                endif
            endif
            AuswahlListe.typeKeys("<home>")
            'fold everything
            for i = 1 to a
                AuswahlListe.typeKeys("-<down>")
            next i
            ' unfold everything, until there is something to unfold
            ' then we found the object we looked for: OLE
            AuswahlListe.typeKeys("<home>")
            i = 0
            a = AuswahlListe.getItemCount
            ' don't run indefinite
            while ((AuswahlListe.getItemCount = a) AND (i < a))
                AuswahlListe.typeKeys("+<down>")
                inc(i)
            wend
            try
                printlog CFN & AuswahlListe.getSelText
            catch
                qaerrorlog CFN & "no item in navigator is selected"
            endcatch
            i = AuswahlListe.getItemCount
            ' activate object
            AuswahlListe.typeKeys("<return>")
            if (i > a) then
                isOleAvailable = TRUE
            else
                isOleAvailable = FALSE
            endif
        else
            kontext "NavigatorDraw"
            if NavigatorDraw.exists( 2 ) then ' was 5 secs
                printlog CFN & "Navigator draw is available"
            else
                ViewNavigator
            endif
            kontext "NavigatorDraw"
            if NavigatorDraw.exists( 2 ) then ' was 5 secs
                printlog CFN & "Navigator draw is available."
                if NOT Liste.exists then
                    Umschalten.click
                endif
                Liste.typeKeys("<home>")
                'fold everything
                a = Liste.getItemCount
                for i = 1 to a
                    Liste.typeKeys("-<down>")
                next i
                ' unfold everything, until there is something to unfold
                ' then we found the object we looked for: OLE
                Liste.typeKeys("<home>")
                i = 0
                a = Liste.getItemCount
                ' don't run indefinite
                while ((Liste.getItemCount = a) AND (i < a))
                    Liste.typeKeys("+<down>")
                    inc(i)
                wend
                try
                    printlog CFN & Liste.getSelText
                catch
                    qaerrorlog CFN & "no item in navigator is selected"
                endcatch
                i = Liste.getItemCount
                if (i > a) then
                    isOleAvailable = TRUE
                else
                    isOleAvailable = FALSE
                endif
            else
                ' no globaldoc
                ' check if something is selected
                try
                    EditCopy
                    printlog CFN & "Something was selected; executed Edit->Copy"
                    isOleAvailable = TRUE
                catch
                    qaErrorLog CFN & "Nothing is selected."
                    isOleAvailable = FALSE
                endcatch
            endif
        endif
    endif
    
end function

