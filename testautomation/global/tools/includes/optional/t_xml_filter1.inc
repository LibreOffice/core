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
'* owner : helge.delfs@oracle.com
'*
'* short description : Tools / XML Filter Settings Resource Test
'*
'\******************************************************************************

testcase tToolsXMLFilterSettings

    Dim iCountOfXSLTStylesAtBeginning as integer
    Dim iCountOfXSLTStylesAfterInstallCheck as integer
    Dim iApplicationEntries as integer
    Dim ia as integer
    Dim ib as integer
    Dim ic as integer
    Dim iCountMsgBox as integer
    Dim sErrorPointerTextForResultfile as string
    Dim sFilterPackagePath as string
    Dim sDocXMLTestFile as string
    Dim sWhichFilterToUse as string
    Dim sTempReadFilterName as string
    Dim bREMOVEXSLT as boolean
    Dim bJavaBroken as boolean

    'Not being used for Writer/WEB 
    if gApplication = "HTML" then
        goto endsub
    endif
    
    'Define the pointer where all warnlogs/QAErrorLogs will come from in this routine.
    sErrorPointerTextForResultfile = "global::tools::inc::t_xml_filter1,inc::tToolsXMLFilterSettings: "
    
    'Later it will be detected if the JRE is working with the office installation
    'Here bJavaBroken will be set to the default behaviour (Java is detected and is working) 
    bJavaBroken = FALSE
 
    '/// Opening new document.
    printlog "- Opening new (application) document."
    call hNewDocument
    '/// Calling the menu item <i>Tools -&gt; XML Filter Settings</i>
    ToolsXMLFilterSettings
    printlog "- XML Filter Settings"
    Kontext "XMLFilterSettings"
    if XMLFilterSettings.Exists(1) then
        call Dialogtest (XMLFilterSettings)
    else
        warnlog sErrorPointerTextForResultfile & "Slot to open the 'XML Filter Settings dialog' failed. Exiting." 
        goto endsub
    end if
    'Counting the existing XSLT stylesheets to verify them after the
    'XSLT stylesheet Check. Setting bREMOVEXSLT = FALSE which only will
    'be TRUE if the count of before and after the check are different.
    bREMOVEXSLT = FALSE
    iCountOfXSLTStylesAtBeginning = FilterList.GetItemCount
    '/// Closing the XML Filter Settings-dialog.
    printlog "Closing the 'XML Filter Settings'-dialog."
    CloseBtn.Click   
    '/// Checking if there are any XSLT stylesheets available (<i>finitialXMLSetting(&quot;docbook or word or excel&quot;)</i>).
    printlog " + Checking if there are any XSLT stylesheets available."
    select case gApplication
        case "CALC"         : sWhichFilterToUse = "excel"
        case "WRITER"       : sWhichFilterToUse = "word"
        case else           : sWhichFilterToUse = "docbook"
    end select
    if finitialXMLSetting(sWhichFilterToUse) = FALSE then
        warnlog "It wasn't possible to use/install external XML filter. Test aborting!"
        goto endsub
    end if
    ToolsXMLFilterSettings
    printlog "Open XML Filter Settings dialog again."   
    Kontext "XMLFilterSettings"
    iCountOfXSLTStylesAfterInstallCheck = FilterList.GetItemCount
    if iCountOfXSLTStylesAfterInstallCheck <> iCountOfXSLTStylesAtBeginning then
        bREMOVEXSLT = TRUE
    end if
    '/// Checking disabling of buttons on this dialog.
    printlog " + Checking that Edit/Test/Delete/SaveToPackage buttons disabled when no filter is selected."
    if EditBtn.IsEnabled then
        FilterList.Typekeys "<MOD1 SPACE>"
        sleep(1)
    end if
    if EditBtn.IsEnabled then 
        warnlog sErrorPointerTextForResultfile & "Edit button: Enabled when no entry is selected!?"
    end if
    if TestXSLTs.IsEnabled then 
        warnlog sErrorPointerTextForResultfile & "TestXSLTs button: Enabled when no entry is selected!?"
    end if
    if DeleteBtn.IsEnabled then 
        warnlog sErrorPointerTextForResultfile & "Delete button: Enabled when no entry is selected!?"
    end if
    if SaveToPackage.IsEnabled then 
        warnlog sErrorPointerTextForResultfile & "SaveToPackage button: Enabled when no entry is selected!?"
    end if
    '/// Checking disabling of buttons on this dialog.
    printlog " + Checking that Edit/Test/Delete/SaveToPackage buttons enabled when one filter is selected."
    if NOT EditBtn.IsEnabled then
        FilterList.Typekeys "<SPACE>"
        sleep(1)
    end if
    if NOT EditBtn.IsEnabled then 
        warnlog sErrorPointerTextForResultfile & "Edit button: Disabled when one entry is selected!?"
    end if
    if NOT TestXSLTs.IsEnabled then 
        warnlog sErrorPointerTextForResultfile & "TestXSLTs button: Disabled when one entry is selected!?"
    end if
    if NOT DeleteBtn.IsEnabled then 
        warnlog sErrorPointerTextForResultfile & "Delete button: Disabled when one entry is selected!?"
    end if        
    if NOT SaveToPackage.IsEnabled then 
        warnlog sErrorPointerTextForResultfile & "SaveToPackage button: Disabled when one entry is selected!?"
    end if
    '/// <i>[Loop]</i>
    '///+ <ol><li>Clicking the &quot;New&quot; button</li>
    '///+ <li>Clicking the &quot;Edit&quot; button</li></ol>
    Kontext "XMLFilterSettings"
    FilterList.TypeKeys "<HOME>"
    for ic = 1 to iCountOfXSLTStylesAfterInstallCheck
        Kontext "XMLFilterSettings"        
        if ( XMLFilterSettings.exists( 2 ) ) then
        
            'read the filter names from the dialog.
            sTempReadFilterName = FilterList.GetItemText(ic)
            
            'lower case comparison!
            sTempReadFilterName = lcase(sTempReadFilterName)
            if InStr(sTempReadFilterName , sWhichFilterToUse) <> 0 then
                FilterList.TypeKeys "<DOWN> " , ic-1           	
                ' printlog "DEBUG: Filtername: " & FilterList.GetItemText(ic)
                WaitSlot() ' Sort of NOOP
            endif
        else
            warnlog( "XML Filter Settings dialog too slow/not displayed" )
        endif
    next ic    
    for ia = 1 to 2
        Kontext "XMLFilterSettings"   
        select case ia
            case 1 :    printlog "+- New"
                        NewBtn.Click
            case 2 :    printlog "+- Edit"
                        EditBtn.Click
        end select
        sleep(1)
        Kontext "XMLFilter"
        if XMLFilter.Exists(2) then
            select case ia
                case 1 : printlog "  (XML Filter: New)"
                case 2 : printlog "  (XML Filter: Edit)"
            end select
            call Dialogtest (XMLFilter)
            Tabcontrol.SetPage TabXMLGeneral
            Kontext "TabXMLGeneral"
            printlog "  +- TabXMLGeneral"
            if ia = 1 then
                '/// If the &quot;New&quot;-button has been used:
                call Dialogtest (TabXMLGeneral)
                iApplicationEntries = Application.GetItemCount
                '///+ <ul><li>Checking the count of applications. Should be 8.</li></ul>
                printlog "   Checking the count of applications. Should be 8."
                if iApplicationEntries <> 8 then
                    warnlog "   +- There should be 8 applications listed in the 'Application'-listbox but there are " & iApplicationEntries & " available!"
                else
                    printlog "   +- 8 applications listed in 'Application' listbox."
                end if
            end if
            Kontext "XMLFilter"
            Tabcontrol.SetPage TabXMLTransformation
            Kontext "TabXMLTransformation"
            printlog "  +- TabXMLTransformation"
            call Dialogtest (TabXMLTransformation)
            '/// On tab page &quot;Transformation&quot; press all &quot;Browse&quot;-buttons.
            printlog "  +- On tab page 'Transformation' press all 'Browse'-buttons."
            for ib = 1 to 4
                Kontext "TabXMLTransformation"
                select case ib
                    case 1 :    printlog "    +- Browse button 'DTD'"
                                DTDBrowse.Click
                    case 2 :    printlog "    +- Browse button 'XSLT for export'"
                                XSLTForExportBrowse.Click
                    case 3 :    printlog "    +- Browse button 'XSLT for import'"
                                XSLTForImportBrowse.Click
                    case 4 :    printlog "    +- Browse button 'Template for import'"
                                BrowseTemplateForImport.Click
                end select

                Kontext "OeffnenDlg"
                if OeffnenDlg.Exists(3) then
                    '/// After every click on &quot;Browse&quot;-button an &quot;FileOpen&quot;-dialog shoud be visible. Cancel that dialog.
                    printlog "  +- After every click on 'Browse'-button an 'FileOpen'-dialog shoud be visible. Cancel that dialog."
                    call Dialogtest (OeffnenDlg)
                    OeffnenDlg.Cancel
                    
                else
                    warnlog sErrorPointerTextForResultfile & "File Open dialog did not appeared."
                end if
            next ib
            '/// [Loop end]
            Kontext "XMLFilter"
            if ( XMLFilter.exists( 3 ) ) then
                printlog "  +- Closing 'XML Filter: New Filter'-dialog."
                XMLFilter.Cancel
            else
                warnlog( "XML Filter Dialog not available" )
            endif
            '/// Closing &quot;XML Filter: New Filter&quot;-dialog
        else
            warnlog "XML Filter dialog did not appeared!"
        end if
    next ia
    Kontext "XMLFilterSettings"
    '/// Clicking &quot;Test XLSTs&quot;-button.
    printlog "+- Clicking 'Test XLSTs'-button."
    TestXSLTs.Click

    'If it's an Import filter it makes no sense to test the export.
    if sWhichFilterToUse = "docbook" then

    	printlog "+- E X P O R T (Writer, Impress)"
        Kontext "TestXMLFilter"        
        if TestXMLFilter.Exists( 3 ) then
            call Dialogtest(TestXMLFilter)
        else
            warnlog sErrorPointerTextForResultfile & "Testing of the XML filter not possible. Exiting."
            Kontext "XMLFilterSettings"
            '/// Closing the &quot;XML Filter Settings&quot;-dialog.
            printlog "+- Closing the 'XML Filter Settings'-dialog."
            'Deinstall the XSLT stylesheet if this routine has
            'added one at the beginning.
            if bREMOVEXSLT = TRUE then
                call sRemoveXSLTFilter
            end if
            Kontext "XMLFilterSettings"    
            CloseBtn.Click    
            '/// Closing the opened application document.
            printlog "- Closing the opened application document."    
            '/// ...if there is more than 1 document opened.
            if GetDocumentCount > 1 then
                call hCloseDocument
            end if
            'Here the routine stops if the test dialog is not opening.            
            goto endsub            
        end if
        ExportBrowseBtn.Click
        '/// Press first &quot;Browse&quot;-button on the &quot;Test XML Filter&quot;-dialog.
        printlog " +-Press first 'Browse'-button on the 'Test XML Filter'-dialog."

        Kontext "OeffnenDlg"
        if OeffnenDlg.Exists( 3 ) then
            call Dialogtest (OeffnenDlg)
            OeffnenDlg.Cancel
            '/// Closing 'FileOpen'-Dialog.
            printlog "  +- Closing 'FileOpen'-Dialog."
        else
            warnlog sErrorPointerTextForResultfile & "File Open dialog did not occoured?!"
        end if
        
        Kontext "TestXMLFilter"
        if ( TestXMLFilter.exists( 2 ) ) then
            '/// If the application is Writer (because the default XSLT stylesheets are only Writer filter).
            '///+<ul><li>Clicking on &quot;Current Document&quot;-button.</li></ul>
            if gApplication = "WRITER" then
                printlog " +- Clicking on 'Current Document'-button."
                CurrentDocument.Click

                '/// If Java is not [correctly] installed a messagebox will be shown and the test of the filter will be canceled!
                Kontext "Messagebox"
                if MessageBox.Exists( 5 ) then
                    warnlog Messagebox.GetText
                    try
                        Messagebox.Cancel
                    catch
                        Messagebox.OK
                    endcatch
                else
                    try
                        Kontext "XMLFilterOutput"
                        if XMLFilterOutput.Exists(1) then
                            '/// &quot;XML Filter Output&quot;-dialog should be visible.
                            printlog "  +- 'XML Filter Output'-dialog should be visible."
                            call Dialogtest(XMLFilterOutput)
                            '/// Clicking &quot;Validate&quot;-button.
                            printlog "   +- Clicking 'Validate'-button."
                            Validate.Click
                            WaitSlot( 2000 )
                            '/// Checking that the validate output control is visible inside the window.
                            printlog "  +- Checking that the validate output control is visible inside the window."
                            if NOT ValidateOutput.isVisible then
                                warnlog "   +- Validate output is not visible!"
                            end if
                            '/// Closing &quot;XML Filter Output&quot;-dialog.
                            printlog "   +- Closing 'XML Filter Output'-dialog."
                            XMLFilterOutput.Close

                        else
                            warnlog sErrorPointerTextForResultfile & "XML Filter Ouput dialog did not occoured."
                        end if
                    catch
                        warnlog "Testing of XSLTs does not work."
                    endcatch
                end if
            else
                Kontext "TestXMLFilter"
                printlog " +- 'Current Document'-button should not be enabled if no !"
                if CurrentDocument.IsEnabled then
                    Kontext "DocumentWriter"
                    try
                        DocumentWriter.TypeKeys "A writer document exists!"
                    catch
                        QAErrorLog "'Current Document'-button is enabled but a 'Writer' XML filter has been selected! If there's no Writer document (may be in the background) also opened it would be a bug!"
                    endcatch
                end if
            end if
        else
            warnlog( "FATAL: TestXMLFilter not available" )
        end if
        printlog "+- I M P O R T"
    endif
    
    
    Kontext "TestXMLFilter"
    if ( TestXMLFilter.exists( 2 ) ) then
        '/// Clicking &quot;Browse&quot;-button (Import).
        printlog "  +- Clicking 'Browse'-button (Import)."
        ImportBrowseBtn.Click

        Kontext "OeffnenDlg"
        if OeffnenDlg.Exists( 2 ) then
            call Dialogtest (OeffnenDlg)
            '/// Closing &quot;FileOpen&quot;-dialog.
            printlog "   +- Closing 'FileOpen'-dialog."
            OeffnenDlg.Cancel
        else
            warnlog sErrorPointerTextForResultfile & "File Open dialog did not occoured?!"
        end if
    else
        warnlog( "FATAL: Dialog <TestXMLFilter> is not available" )
    endif
    
    Kontext "TestXMLFilter"
    '/// Checking if 'Display Source'-checkbox is checked (should not be!).
    printlog "  +- Checking if 'Display Source'-checkbox is checked (should not be!)."
    if NOT DisplaySource.IsChecked then
        DisplaySource.Check
    else
        warnlog "  +- Display source should be NOT checked as default!"
    end if
    Kontext "TestXMLFilter"
    select case sWhichFilterToUse
        case "docbook"  :   '/// If DocBook XML is being tested use a DocBook XML file.
                            sDocXMLTestFile = ConvertPath(gTestToolPath & "global\input\xml_filter\docbook.xml")
        case "word"     :   '/// If Word XML is being tested use a Word XML file.
                            sDocXMLTestFile = ConvertPath(gTestToolPath & "global\input\xml_filter\word.xml")
        case "excel"    :   '/// If Excel XML is being tested use a Excel XML file.
                            sDocXMLTestFile = ConvertPath(gTestToolPath & "global\input\xml_filter\excel.xml")        
        case "xhtml"    :   '/// If XHTML is being tested that use a XHTML file.
                            sDocXMLTestFile = ConvertPath(gTestToolPath & "global\input\xml_filter\xhtml.xhtml")
        case else       :   warnlog sErrorPointerTextForResultfile & "Wrong parameter has been used! Exiting"
                            Kontext "TestXMLFilter"
                            '/// Closing the &quot;Test XML Filter&quot;-dialog.
                            printlog "  +- Closing the 'Test XML Filter'-dialog"
                            CloseBtn.Click                            
                            Kontext "XMLFilterSettings"
                            '/// Closing the &quot;XML Filter Settings&quot;-dialog.
                            printlog "+- Closing the 'XML Filter Settings'-dialog."
                            'Deinstall the XSLT stylesheet if this routine has
                            'added one at the beginning.
                            if bREMOVEXSLT = TRUE then
                                call sRemoveXSLTFilter
                            end if
                            Kontext "XMLFilterSettings"    
                            CloseBtn.Click    
                            '/// Closing the opened application document.
                            printlog "- Closing the opened application document."    
                            '/// ...if there is more than 1 document opened.
                            if GetDocumentCount > 1 then
                                Call hCloseDocument
                            end if
                            goto endsub
    end select
    '/// Clicking &quot;Browse&quot;-button (Import) again to test it with the XML file.
    printlog "  +- Clicking 'Browse'-button (Import) again to test it the XML file."
    ImportBrowseBtn.Click

    Kontext "OeffnenDlg"
    if OeffnenDlg.Exists( 3 ) then
        call Dialogtest (OeffnenDlg)
        Dateiname.SetText sDocXMLTestFile
        Oeffnen.Click
        try
            ' If Java is not [correctly] installed a messagebox will be shown and the test of the filter will be canceled!
            Kontext "Messagebox"
            if Messagebox.Exists(2) then                
                for iCountMsgBox = 1 to 5
                    Kontext "Messagebox"
                    if MessageBox.Exists(2) then
                        if MessageBox.GetRT = 304 then
                            if iCountMsgBox = 1 then
                                warnlog sErrorPointerTextForResultfile & "No output window was visible! Exiting routine!"
                            endif
                            QAErrorLog "Dialog (" & iCountMsgBox & "): " & Messagebox.GetText
                            Messagebox.OK                        
                            bJavaBroken = TRUE
                            if iCountMsgBox > 3 then
                                warnlog "Instead of 3 times the missing Java RT box will be shown " & iCountMsgBox & " times." 
                            endif
                        end if
                    else
                        exit for
                    endif
                next iCountMsgBox
            endif
            Kontext "XMLFilterOutput"
            if XMLFilterOutput.Exists(3) then
                '/// &quot;XML Filter Output&quot;-dialog should be visible.
                printlog "   +- 'XML Filter Output'-dialog should be visible."
                call Dialogtest(XMLFilterOutput)
                '/// Clicking &quot;Validate&quot;-button.
                printlog "   +- Clicking 'Validate'-button."
                Validate.Click                

                Kontext "Messagebox"
                if Messagebox.Exists( 2 ) then
                    if bJavaBroken then
                        Messagebox.OK
                    else
                        warnlog Messagebox.GetText
                        Messagebox.OK
                    endif
                endif                                            
                '/// Checking that the validate output control is visible inside the window.
                printlog "   +- Checking that the validate output control is visible inside the window."
                Kontext "XMLFilterOutput"
                if NOT ValidateOutput.isVisible then
                    warnlog "   +- Validate output is not visible!"
                end if
                '/// Closing &quot;XML Filter Output&quot;-dialog.
                printlog "   +- Closing 'XML Filter Output'-dialog."
                Kontext "XMLFilterOutput"
                XMLFilterOutput.Close
            else
                warnlog sErrorPointerTextForResultfile & "No output window was visible! Exiting routine!"
                Kontext "TestXMLFilter"
                '/// Closing the &quot;Test XML Filter&quot;-dialog.
                printlog "  +- Closing the 'Test XML Filter'-dialog"
                CloseBtn.Click
                Kontext "XMLFilterSettings"
                '/// Closing the &quot;XML Filter Settings&quot;-dialog.
                printlog "+- Closing the 'XML Filter Settings'-dialog."
                'Deinstall the XSLT stylesheet if this routine has
                'added one at the beginning.
                if bREMOVEXSLT = TRUE then
                    call sRemoveXSLTFilter
                end if
                Kontext "XMLFilterSettings"    
                CloseBtn.Click    
                '/// Closing the opened application document.
                printlog "- Closing the opened application document."    
                '/// ...if there is more than 1 document opened.
                if GetDocumentCount > 1 then
                    call hCloseDocument
                end if
                'Here the routine stops if there is no Java installed or the output was not visible.            
                goto endsub            
            end if
        catch
            '/// If Java is not [correctly] installed a messagebox will be shown and the test of the filter will be canceled!
            Kontext "Messagebox"
            if MessageBox.Exists( 2 ) then
                warnlog Messagebox.GetText
                Messagebox.OK                
                warnlog sErrorPointerTextForResultfile & "Java is not usable! Exiting routine!"
                Kontext "TestXMLFilter"
                '/// Closing the &quot;Test XML Filter&quot;-dialog.
                printlog "  +- Closing the 'Test XML Filter'-dialog"
                CloseBtn.Click
                Kontext "XMLFilterSettings"
                '/// Closing the &quot;XML Filter Settings&quot;-dialog.
                printlog "+- Closing the 'XML Filter Settings'-dialog."
                'Deinstall the XSLT stylesheet if this routine has
                'added one at the beginning.
                if bREMOVEXSLT = TRUE then
                    call sRemoveXSLTFilter
                end if
                Kontext "XMLFilterSettings"    
                CloseBtn.Click    
                '/// Closing the opened application document.
                printlog "- Closing the opened application document."    
                '/// ...if there is more than 1 document opened.
                if GetDocumentCount > 1 then
                    call hCloseDocument
                end if                            
                goto endsub            
            end if
        endcatch
    else
        warnlog sErrorPointerTextForResultfile & "File Open dialog missing! Not able to load test documents. Exiting routine!"
        Kontext "TestXMLFilter"
        '/// Closing the &quot;Test XML Filter&quot;-dialog.
        printlog "  +- Closing the 'Test XML Filter'-dialog"
        CloseBtn.Click
        Kontext "XMLFilterSettings"
        '/// Closing the &quot;XML Filter Settings&quot;-dialog.
        printlog "+- Closing the 'XML Filter Settings'-dialog."
        'Deinstall the XSLT stylesheet if this routine has
        'added one at the beginning.
        if bREMOVEXSLT = TRUE then
            call sRemoveXSLTFilter
        end if
        Kontext "XMLFilterSettings"    
        CloseBtn.Click    
        '/// Closing the opened application document.
        printlog "- Closing the opened application document."    
        '/// ...if there is more than 1 document opened.
        if GetDocumentCount > 1 then
            call hCloseDocument
        end if           
        goto endsub
    end if
    '/// Closing the tested XML document
    printlog "   +- Closing the tested XML document"
    '/// ...if there is more than 1 document opened.
    if GetDocumentCount > 1 then
        call hCloseDocument
    else
        QAErrorLog "#i27370# Normaly there should be a second document but there is none!"
    end if

    Kontext "TestXMLFilter"    
    if ( TestXMLFilter.exists( 1 ) ) then
        if bJavaBroken = FALSE then        
            '/// Clicking on &quot;Recent File&quot;-Button.
            printlog "  +- Clicking on 'Recent File'-Button."
            RecentFile.Click

            Kontext "XMLFilterOutput"
            '/// &quot;XML Filter Output&quot;-dialog should be visible.
            printlog "   +- 'XML Filter Output'-dialog should be visible."

            Kontext "XMLFilterOutput"
            '/// Closing &quot;XML Filter Output&quot;-dialog.
            printlog "  +- Closing 'XML Filter Output'-dialog."
            XMLFilterOutput.Close

            '/// Closing the tested XML document
            printlog "   +- Closing the tested XML document"
            '/// ...if there is more than 1 document opened.
            if GetDocumentCount > 1 then
                call hCloseDocument
            else
                QAErrorLog "#i27370# Normaly there should be a second document but there is none!"
            end if

            Kontext "TestXMLFilter"
            '/// Closing the &quot;Test XML Filter&quot;-dialog.
            printlog "  +- Closing the 'Test XML Filter'-dialog"        
        endif
    else
        warnlog( "Dialog <TestXMLFilter> is not available" )
    endif
    
    CloseBtn.Click

    Kontext "XMLFilterSettings"
    if ( XMLFilterSettings.exists( 1 ) ) then
        '/// Clicking &quot;Delete&quot;-button.
        printlog " +- Clicking 'Delete'-button"
        DeleteBtn.Click
        Kontext
        if Active.Exists(1) then
            if Active.GetRT = 304 then
                Active.No
            end if
        end if
    else
        warnlog( "Dialog <XMLFilterSettings> is not available" )
    endif
    
    '/// Clicking 'Save to Package'-button.
    Kontext "XMLFilterSettings"
    if ( XMLFilterSettings.exists( 1 ) ) then

        printlog " +- Clicking 'Save to Package'-button."
        SaveToPackage.Click

        Kontext "SpeichernDlg"
        if SpeichernDlg.Exists( 3 ) then
            call Dialogtest(SpeichernDlg)
            SpeichernDlg.Cancel
            '/// Closing 'File Save As'-Dialog.
            printlog "  +- Closing 'File Save As'-Dialog."
        else
            warnlog sErrorPointerTextForResultfile & "Save As dialog missing!"
        end if
    else
        warnlog( "Dialog <XMLFilterSettings> is not available" )
    endif
        
        
    '/// Clicking 'Open Package'-button.
    Kontext "XMLFilterSettings"
    if ( XMLFilterSettings.exists( 1 ) ) then
        printlog " +- Clicking 'Open Package'-button."
        OpenPackage.Click

        Kontext "OeffnenDlg"
        if OeffnenDlg.Exists( 3 ) then
            call Dialogtest(OeffnenDlg)
            OeffnenDlg.Cancel
            '/// Closing 'FileOpen'-Dialog.
            printlog "  +- Closing 'FileOpen'-Dialog."
        else
            warnlog sErrorPointerTextForResultfile & "File open dialog missing!"
        end if 
    else
        warnlog( "Dialog <XMLFilterSettings> is not available" )
    endif


    '/// Closing the &quot;XML Filter Settings&quot;-dialog.
    Kontext "XMLFilterSettings"
    if ( XMLFilterSettings.exists( 1 ) ) then
        printlog "+- Closing the 'XML Filter Settings'-dialog."
        'Deinstall the XSLT stylesheet if this routine has
        'added one at the beginning.
        if bREMOVEXSLT = TRUE then
            call sRemoveXSLTFilter
        end if
    else
        warnlog( "Dialog <XMLFilterSettings> is not available" )
    endif


    Kontext "XMLFilterSettings"    
    if ( XMLFilterSettings.exists( 1 ) ) then
        CloseBtn.Click    
        '/// Closing the opened application document.
    else
        warnlog( "Dialog <XMLFilterSettings> is not available" )
    endif
        
    printlog "- Closing the opened application document."    
    '/// ...if there is more than 1 document opened.
    if ( hFileCloseAll() <> 2 ) then
        QAErrorLog "#i27370# Normaly there should be a second document but there is none!"
    end if
    
endcase

'-------------------------------------------------------------------------

sub fInitialXMLSetting( sXMLFilterType as string ) as boolean
'/// This small function adds the latest and greatest XSLT stylesheet
'///+ to the office.<br>
'///+ The result is TRUE if the packages are already available or if they 
'///+ have been installed successfully in this routine.
'/// <u>Input</u>: excel, word, docbook, xhtml
'/// <u>Output</u>: TRUE or FALSE 
    Dim iCountOfDefaultXSLTStyles as integer
    Dim iSecondCountOfDefaultXSLTStyles as integer
    Dim sFilterPackagePath as string
    Dim sTempReadFilterName as string    
    Dim ia as integer

    'Setting boolean to FALSE
    fInitialXMLSetting = FALSE
    '/// Opening new document.
    printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: Opening new " & gApplication & " document."
    call hNewDocument
    '/// Calling the menu item <i>Tools -&gt; XML Filter Settings</i>
    ToolsXMLFilterSettings
    printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: Opening the XML Filter Settings dialog."
    Kontext "XMLFilterSettings"
    call Dialogtest (XMLFilterSettings)
    '/// Checking if there are any XSLT stylesheets available.
    printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: Checking if there are any XSLT stylesheets available."
    iCountOfDefaultXSLTStyles = FilterList.GetItemCount   
    ' printlog "DEBUG (a): " & iCountOfDefaultXSLTStyles
    if iCountOfDefaultXSLTStyles <> 0 then
        '/// Check if the same filter already exists.
        'Using lowercased input parameter. 
        sXMLFilterType = lcase(sXMLFilterType)                
        for ia = 1 to iCountOfDefaultXSLTStyles
            Kontext "XMLFilterSettings"
            'read the filter names from the dialog.
            sTempReadFilterName = FilterList.GetItemText(ia)
            ' printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: DEBUG: (" & ia & ") " & sTempReadFilterName
            'lower case comparison!
            sTempReadFilterName = lcase(sTempReadFilterName)
            'if a _part_ of the filtername matches the readed filter name the criteria is true!
            if InStr(sTempReadFilterName , sXMLFilterType) <> 0 then
                fInitialXMLSetting = TRUE
                Kontext "XMLFilterSettings"
                CloseBtn.Click
                Call hCloseDocument
                exit sub
            end if
        next ia
        'if no filter name maches the for...next loop will be left
        'and the same routine as used for 0 filter installed will
        'be used to install a filter package.        
    end if
    printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: The XML filter is not available: Adding the " & sXMLFilterType & " XML filter."
    '///+ The packages will be used from &quot;qatesttool/global/input/xslt_stylesheets/*.jar&quot;
    ' printlog "DEBUG: " & sXMLFilterType
    select case sXMLFilterType
        case "excel"     : sFilterPackagePath = ConvertPath(gTestToolPath & "global\input\xslt_stylesheets\excel.jar")
        case "word"      : sFilterPackagePath = ConvertPath(gTestToolPath & "global\input\xslt_stylesheets\word.jar")
        case "docbook"   : sFilterPackagePath = ConvertPath(gTestToolPath & "global\input\xslt_stylesheets\docbook.jar")
        case "xhtml"     : sFilterPackagePath = ConvertPath(gTestToolPath & "global\input\xslt_stylesheets\xhtml.jar")                    
    end select
    OpenPackage.Click

    Kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then
        call Dialogtest (OeffnenDlg)
        Dateiname.SetText sFilterPackagePath
        Oeffnen.Click
    
        Kontext
        if Active.Exists(3) then
            if Active.GetRT = 304 then
                printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: " & Active.GetText
                Active.OK
                Kontext "XMLFilterSettings"
                iSecondCountOfDefaultXSLTStyles = FilterList.GetItemCount
                printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: Checking again the count of installed XSLT stylesheets."
                if iSecondCountOfDefaultXSLTStyles <> (iCountOfDefaultXSLTStyles+1) then
                    warnlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: It was not possible to add " & sXMLFilterType & "-XML filter!"
                    warnlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: Test aborted!"
                    exit sub
                else
                    printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: XML filter has been installed!"
                    fInitialXMLSetting = TRUE
                end if
            end if
        end if
    else
        warnlog( "Dialog <OeffnenDlg> is not available" )
    endif
    
    Kontext "XMLFilterSettings"
    if ( XMLFilterSettings.exists( 1 ) ) then
        '/// Closing the XML Filter Settings-dialog.
        printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: Closing the 'XML Filter Settings'-dialog."
        CloseBtn.Click
    else
        warnlog( "Dialog <XMLFilterSettings> is not available" )
    endif
    '/// Closing the opened application document.
    printlog "global::tools::inc::t_xml_filter1,inc::fInitialXMLSetting: Closing the opened application document."
    call hCloseDocument
end sub

'-------------------------------------------------------------------------

sub sRemoveXSLTFilter
    Dim sXMLFilterType as string
    Dim iCountOfDefaultXSLTStyles as integer
    Dim ia as integer
    Dim sTempReadFilterName as string
    Kontext "XMLFilterSettings"
    select case gApplication
        case "CALC"         : sXMLFilterType = "excel"
        case "WRITER"       : sXMLFilterType = "word"
        case else           : sXMLFilterType = "docbook"
    end select
    iCountOfDefaultXSLTStyles = FilterList.GetItemCount
    if iCountOfDefaultXSLTStyles <> 0 then
        '/// Search for the filter in filter list.
        'Using lowercased input parameter.                 
        for ia = 1 to iCountOfDefaultXSLTStyles
            Kontext "XMLFilterSettings"
            'read the filter names from the dialog.
            sTempReadFilterName = FilterList.GetItemText(ia)
            'lower case comparison!
            if InStr(sTempReadFilterName , sXMLFilterType) <> 0 then
                '/// If the filter has been found delete it.
                FilterList.TypeKeys "<HOME>"
                wait( 100 )
                FilterList.TypeKeys "<DOWN>" , (ia-1)
                wait( 100 )
                DeleteBtn.Click
                Kontext
                if Active.Exists(1) then
                    if Active.GetRT = 304 then
                        Active.Yes
                    end if
                end if                                
                exit sub
            end if
        next ia
    else
        warnlog "The count of XML filters has to be >0! Problem?"
    end if    
end sub

