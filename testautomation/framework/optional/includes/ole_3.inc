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
'* short description :
'*
'\******************************************************************************

testcase tDraw_As_OLE_Object()
    
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEDraw, "DRAW" )
    'call hInsertOLEObject( false, gOLEDraw, "DRAW" )
    
endcase

testcase tCalc_as_OLE_Object()
    
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLECalc, "CALC" )
    'call hInsertOLEObject( false, gOLECalc, "CALC" )
    
endcase

testcase tImpress_As_OLE_Object()
    
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEImpress, "IMPRESS" )
    'call hInsertOLEObject( false, gOLEImpress, "IMPRESS" )
    
endcase

testcase tWriter_As_OLE_Object()
    
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEWriter, "WRITER" )
    'call hInsertOLEObject( false, gOLEWriter, "WRITER" )
    
endcase

testcase tMath_As_OLE_Object()
    
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEMath, "MATH" )
    'call hInsertOLEObject( false, gOLEMath, "MATH" )
    
endcase

testcase tChart_As_OLE_Object()
    
    call ClosePresentationfloat()
    call hInsertOLEObject( true, gOLEChart, "CHART" )
    'call hInsertOLEObject( false, gOLEChart, "CHART" )
    
endcase

'*******************************************************************************

function hInsertOLEObject( bRemoveFocus as boolean, cOLEObject as string, cOleType as string ) as beoolean
    
    dim sFile as String
    dim bOleAvailable as boolean
    
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
        hInsertOLEObject() = false
        exit function
    endif
    
    if ( gApplication = "MASTERDOCUMENT" and cOleType = "WRITER" ) then
        printlog( "Skipping " & cOleType & " as OLE object" )
        hInsertOLEObject() = false
        exit function
    endif
    
    hCreateDocument()
    
    printlog( "Menu: insert / object / OLE object" )
    InsertObjectOLEObject
    
    Kontext "OLEObjektEinfuegen"
    if ( OLEObjektEinfuegen.exists( 1 ) ) then
        
        printlog( "Select 'create new'" )
        NeuErstellen.Check()
        
        printlog( "Select objecttype: " & cOLEObject )
        ObjektTyp.Select( cOLEObject )
        
        printlog( "Click OK" )
        hCloseDialog( OLEObjektEinfuegen, "ok" )
    else
        warnlog( "Insert OLE object dialog is missing, test ends" )
        hDestroyDocument()
        hInsertOLEObject() = false
        exit function
    endif
    
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
    
    sFile = ConvertPath ( gOfficePath + "user\work\ole_" & cOleType )
    printlog( "Save the file; File to be written (w/o extension): " & sFile )
    hDeleteFile( sFile )
    hFileSaveAs( sFile )
    hDestroyDocument()
    
    printlog( "Reopen the file: " & sFile )
    hFileOpen( sFile )
    
    bOleAvailable = isOleAvailable()
    if ( bOleAvailable ) then
        
        kontext "Navigator"
        if ( Navigator.exists() ) then hCloseDialog( Navigator, "close,optional" )
        
        call OLESetFocus()
        
        call SendEscape()
    else
        qaErrorLog( "#i44725# OLE Object not saved in Master Document" )
    endif
    
    printlog( "Cleanup: Close the document, delete workfile" )
    hDestroyDocument()
    hDeleteFile( sFile )
    hInsertOLEObject() = true
    
end sub

'*******************************************************************************

function isOleAvailable() as boolean
    
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

