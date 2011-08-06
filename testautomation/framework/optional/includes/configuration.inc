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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description : Function Test for known global Issues
'*
'\*****************************************************************

sub sToolsCustomizeKeyboardFunction
    dim aApplication() as string
    dim i,a,b as integer
    dim lList(1000) as string
    dim sTemp as string
    
    ' go through all applications:
    aApplication()=array("WRITER","MASTERDOCUMENT","HTML","CALC","IMPRESS","DRAW","MATH","DATABASE")
    a=uBound(aApplication())
    sTemp = gApplication
    for i = 0 to a
        
        if ( i = 7 and gtSysName = "Solaris x86" ) then
            '#i62423# 
            warnlog( "Solaris: Crash when opening new database" )
        else
            gApplication = aApplication(i)
            tToolsCustomizeKeyboardFunction(lList())
        endif
    next i
    gApplication = sTemp
    
    listsort(lList())
    a = ListCount(lList())
    printlog "Count1: " + a
    if (a>0) then
        b = 0
        sTemp=""
        for i=1 to a
            if (lList(i) <> sTemp) then
                printlog ""+i+": '" + lList(i) + "'"
                sTemp=lList(i)
                inc(b)
            endif
        next i
        printlog "Number of unique Errors: " + b
    endif
end sub

testcase tToolsCustomizeKeyboardFunction(lList())
    dim sKeys(300,2) as string
    dim i,a,b,c,d as integer
    dim sTemp, sTemp2 as string
    dim iBugCount(10) as integer
    dim bErrorFound as boolean
    dim i43969 as string ' ALL
    dim i60617 as string ' ALL
    dim i64346 as string ' ALL
    dim i80850 as string ' ALL
    dim i84982 as string ' ALL
    dim i84983 as string
    dim i87950 as string
    dim i87951 as string
    dim i87952 as string
    dim i92080 as string
    dim i96322 as string
    dim i100037 as string
    dim i115370 as string
    
    i60617 = ".uno:ContinueNumbering"
    i43969 = ".uno:MacroOrganizer"
    i64346 = ".uno:SendMailDocAs"
    i80850 = ".uno:ToggleFormula.uno:AssignMacro"
    i84982 = ".uno:FontDialogForParagraph"
    i84983 = ".uno:DBNewReportWithPreSelection"
    i87950 = ".uno:DBMigrateScripts"
    i87951 = ".uno:DeleteAllNotes.uno:DeleteAuthor.uno:DeleteNote.uno:HideAllNotes.uno:HideAuthor.uno:HideNote"
    i87952 = ".uno:InsertApplet"
    i92080 = ".uno:SaveGraphic"
    i96322 = ".uno:ActivateStyleApply"
    i100037 = ".uno:AcceptTracedChange, .uno:DeleteComment, .uno:RejectTracedChange, .uno:ReplyNote, .uno:TaskPaneInsertPage, .uno:ValidityReference"
    i115370 = ".uno:TaskPaneInsertPage"
    
    Call hNewDocument
    sleep 2
    ToolsCustomize
    sleep 3
    Kontext
    Messagebox.SetPage TabTastatur         ' 2 ------------------
    Kontext "TabTastatur"
    for c = 1 to 2
        select case c
        case 1:
            StarOffice.Check
            Printlog ("-------------------- Keylist for StarOpenOfficeSuite.org --------------------")
        case 2:
            Application.Check
            Printlog ("-------------------- Keylist for "+gApplication+" ------------------------")
        end select
        i = Bereich.getItemCount
        for a = 1 to i
            Bereich.select(a,true)
            sTemp = Bereich.getSelText
            for b=1 to Funktion.getItemCount
                sTemp2 = Funktion.getItemText(b)
                d = inStr(sTemp2,":")
                if (d>0) then
                    ' translators use ':'-> check if ' ' follows or CHR$ < 128
                    ' asc() returns 16 bit values in an integer -> sign is affected :-( -> use abs(asc())
                    if (((mid(sTemp2,d+1,1)<>" ")AND(abs(asc(mid(sTemp2,d+1,1)))<128)) AND (abs(asc(mid(sTemp2,d-1,1)))<128))then
                        listAppend(lList(),sTemp2)
                        bErrorFound = false
                        if (inStr(i115370,sTemp2)>0) then
                            qaerrorlog "#i115370# ("+c+"/"+a+"/"+b+"): Provide real Name for Function: '"+sTemp+"'::'"+sTemp2+"'"
                            bErrorFound = true
                        endif
                        if ( not bErrorFound ) then
                             warnlog "("+c+"/"+a+"/"+b+"): Provide real Name for Function: '"+sTemp+"'::'"+sTemp2+"'"
                        endif
                        inc(iBugCount(3))
                    endif
                else
                    '  printlog "("+c+"/"+a+"/"+b+"): '"+sTemp+"'::'"+sTemp2+"'"
                endif
            next b
        next a
    next c
    '    if (iBugCount(3) > 0) then WarnLog ""+iBugCount(3)+": Missing Name for funktion; is .uno:..."
    TabTastatur.cancel
    Call hCloseDocument
endcase

testcase tToolsOptionsAPIConfiguration
    Dim iJump as Integer
    Dim sComXML as String
    Dim sHelpTip as String
    Dim sHelpExtendedTip as String
    Dim bHelpTip as Boolean
    Dim bHelpExtendedTip as Boolean
    Dim Silent as Boolean
    Dim sTempPath as string
    Dim sTempList(10) as string
    Dim sLocalString as string
    
    'Used for GetXMLValueGlobal (DEPRECATED function!) to get no output in resultfile.
    Silent = TRUE
    
    try
        ToolsOptions
        call hToolsOptions ("Languagesettings", "Languages")
        if (gAsianSup <> Aktivieren.IsChecked) then
            warnlog "gAsianSup differs from UI"
        endif
        if (gCTLSup <> ComplexScriptEnabled.IsChecked) then
            warnlog "gCTLSup differs from UI"
        endif
        
        call hToolsOptions ("STAROFFICE", "ACCESSIBILITY")
        try
            if (gPlatGroup <> "unx") then
                if (gAccessibility <> SupportAssistiveTechnologyTools.IsChecked) then
                    warnlog "gAccessibility differs from UI"
                endif
            endif
        catch
            if (NOT gAccessibility) then
                warnlog "gAccessibility differs from UI - catch"
            endif
        endcatch
        
        call hToolsOptions ("StarOffice", "General")
        if StarOfficeDialogeBenutzen.Exists then
            if (NOT StarOfficeDialogeBenutzen.isChecked) then
                warnlog "StarOfficeDialogeBenutzen is not checked"
            endif
        end if
        
        call hToolsOptions ("StarOffice", "General")
        if (Tips.isChecked) then
            warnlog "Help Tip is checked"
        endif
        
        
        call hToolsOptions ("StarOffice", "Paths")
        select case iSprache
        case 01   : iJump =  6      ' English (USA)
        case 03   : iJump =  7      ' Portuguese
        case 07   : iJump =  0      ' Russian
        case 30   : iJump =  0      ' Greek
        case 31   : iJump =  0      ' Netherlands
        case 33   : iJump = 0       ' French
        case 34   : iJump = 0       ' Spanish
        case 35   : iJump =  0      ' Finnish
        case 36   : iJump =  0      ' Hungaria
        case 37   : iJump =  0      ' Catalan
        case 39   : iJump =  0      ' Italian
        case 42   : iJump =  0      ' Czech
        case 43   : iJump =  0      ' Slowak
        case 44   : iJump =  0      ' English (GB)
        case 45   : iJump =  0      ' Danish
        case 46   : iJump =  0      ' Swedish
        case 47   : iJump =  0      ' Norwegian
        case 48   : iJump =  0      ' Polish
        case 49   : iJump =  1      ' German
        case 51   : iJump =  0      ' Slowak
        case 50   : iJump =  0      ' Slovenian
        case 55   : iJump =  0      ' Portuguese (Brazil)
        case 66   : iJump =  0      ' Thai
        case 81   : iJump =  0      ' Japanese ' disabled, because locale dependant!
        case 82   : iJump =  0      ' Korean   ' disabled, because locale dependant!
        case 86   : iJump =  0      ' Chinese (simplified)  ' disabled, because locale dependant!
        case 88   : iJump =  0      ' Chinese (traditional) ' disabled, because locale dependant!
        case 90   : iJump =  0      ' Turkish
        case 91   : iJump =  0      ' Hindi
        case 96   : iJump =  0      ' Arab
        case 97   : iJump =  0      ' Hebrew
        case else : qaErrorlog "The work-dir can't changed to the internal office-work-dir! Please insert the language in this list!"
            iJump =  0
        end select
        
        if iJump <> 0 then
            'DEBUG: printlog "**  master.inc::mMakeGeneralOptions::iJump(" & iSprache & ") = " & iJump
            sLocalString = Typ.getItemText(iJump,3) ' works in gh13
            if sLocalString = "" then
                sLocalString = Typ.getItemText(iJump,2) ' fallback for before gh13
            endif
            if  (sLocalString <> ConvertPath (gOfficePath + "user\work")) then
                warnlog ("Work Directory differs from API setting? : is: '" + sLocalString + "' should: '" + ConvertPath (gOfficePath + "user\work") + "'")
            endif
        else
            'TODO: figure out which Entry it could be
            for i = 1 to Typ.getItemCount
                if (Typ.getItemText(i,2) = ConvertPath (gOfficePath + "user\work")) then
                    qaErrorLog ("found WorkDirectory at position: " + i)
                    iJump = i
                endif
            next i
            if (iJump = 0) then
                warnlog ("No workdirectory candidate found.")
            endif
        end if
        
        select case iSprache
        case 01   : iJump = 8      ' English (USA)
        case 03   : iJump =  3      ' Portuguese
        case 07   : iJump =  0      ' Russian
        case 30   : iJump =  0      ' Greek
        case 31   : iJump = 0      ' Netherlands
        case 33   : iJump =  0      ' French
        case 34   : iJump =  1      ' Spanish
        case 35   : iJump =  0      ' Finnish
        case 36   : iJump =  0      ' Hungaria
        case 37   : iJump =  0      ' Catalan
        case 39   : iJump =  0      ' Italian
        case 42   : iJump =  0      ' Czech
        case 43   : iJump =  0      ' Slowak
        case 44   : iJump =  0      ' English (GB)
        case 45   : iJump =  0      ' Danish
        case 46   : iJump =  0     ' Swedish
        case 47   : iJump =  0      ' Norwegian
        case 48   : iJump =  0     ' Polish
        case 49   : iJump =  8     ' German
        case 51   : iJump =  0      ' Slowak
        case 55   : iJump =  1      ' Portuguese (Brazil)
        case 66   : iJump =  0      ' Thai
        case 81   : iJump =  0      ' Japanese ' disabled, because locale dependant!
        case 82   : iJump =  0      ' Korean   ' disabled, because locale dependant!
        case 86   : iJump =  0      ' Chinese (simplified)  ' disabled, because locale dependant!
        case 88   : iJump =  0      ' Chinese (traditional) ' disabled, because locale dependant!
        case 90   : iJump =  0      ' Turkish
        case 91   : iJump =  0      ' Hindi
        case 96   : iJump =  0      ' Arab
        case 97   : iJump =  0      ' Hebrew
        case else : qaErrorLog "The temp-dir can't changed to the internal office-temp-dir! Please insert the language in this list!"
            iJump =  0
        end select
        if iJump <> 0 then
            'DEBUG: printlog "**  master.inc::mMakeGeneralOptions::iJump(" & iSprache & ") = " & iJump
            sLocalString = Typ.getItemText(iJump,3) ' works in gh13
            if sLocalString = "" then
                sLocalString = Typ.getItemText(iJump,2) ' fallback for before gh13
            endif
            if  (sLocalString <> ConvertPath (gOfficePath + "user\temp")) then
                warnlog ("Temp Directory differs from API setting? : is: '" + sLocalString + "' should: '" + ConvertPath (gOfficePath + "user\temp") + "'")
            endif
        else
            'TODO: figure out which Entry it could be
            for i = 1 to Typ.getItemCount
                if (Typ.getItemText(i,2) = ConvertPath (gOfficePath + "user\temp")) then
                    qaErrorLog ("found TempDirectory at position: " + i)
                    iJump = i
                endif
            next i
            if (iJump = 0) then
                warnlog ("No tempdirectory candidate found.")
            endif
        end if
        
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
        sleep(4)
    catch
        warnlog "Error during walking through optionsdialog"
    endcatch
endcase


