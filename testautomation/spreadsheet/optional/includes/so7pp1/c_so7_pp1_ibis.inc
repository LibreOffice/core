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
'**
'** owner : oliver.craemer@Sun.COM
'**
'** short description :  Test IBIS bug fixes
'**
'************************************************************************
' **
' #1 tIBIS111099            ' Disable design mode of form controls after loading excel file
' #1 tIBIS111158            ' Lost properties of form controls
' #1 tIBIS111711            ' Crash when deleting chart in Menu->Window->New Window
' **
'\***********************************************************************

testcase tIBIS111099

    Dim testdoc as string
    testdoc = convertpath( gTesttoolpath & "spreadsheet\optional\input\111099.xls" )

    printlog " Open the test document"
    hFileOpenLocally(testdoc)

    printlog " Select formcontrols toolbar"
    Call hToolbarSelect("CONTROLS", true)
    sleep(2)
    Kontext "FormControls"
    if SwitchControlDesignMode.IsEnabled then
        if SwitchControlDesignMode.getState(2) = 0 then
            printlog "OK, design mode is disabled"
        else
            warnlog "#i108315# Document is opened in design mode -> #111099#"
        end if
    else
        printlog "Form controls toolbar was not enabled."
    end if
    call hCloseDocument
    hDeleteFile( gLastWorkFile )
endcase

'-------------------------------------------------------------------------

testcase tIBIS111158

    Dim testdoc as string
    Dim testString as string
    testdoc = convertpath( gTesttoolpath & "spreadsheet\optional\input\111158.xls" )

    printlog " Open the test document"
    hFileOpenLocally(testdoc)

    Kontext "Standardbar"
    '/// If 'Form Controls' toolbar is not visible: View / Toolbars / Form Controls
    Call hToolbarSelect("CONTROLS", true)
    sleep(2)
    Kontext "FormControls"
    try
        if SwitchControlDesignMode.GetState(2) = 0 then
            '/// Click on 'Design Mode on/off' button on 'Form Controls'-toolbar
            SwitchControlDesignMode.click
            sleep(2)
        end if
    catch
        warnlog "Form controls toolbar was not enabled, maybe the doc is writeprotected."
        Call hToolbarSelect("CONTROLS", false)
        call hCloseDocument
        goto endsub
    endcatch
    '/// Click on 'Form Design' button on 'Form Controls'-toolbar
    FormDesignTools.Click
    sleep(2)
    Kontext "FormDesignTools"
    if FormDesignTools.Exists(2) then
        If ShowFmExplorer.IsEnabled then
            '/// Show Form Navigator
            ShowFmExplorer.Click
        else
            warnlog "Button was not enabled!"
        end if
    else
        warnlog "'Form Design'-toolbar did not come up."
        Call hCloseDocument
        goto endsub
    end if

    '/// Select the first 'ComboxBox' (9th entry from the top)
    '/// Open context menu
    Kontext "FormularNavigator"
    Liste.TypeKeys "<HOME>" , true
    sleep(1)
    Liste.TypeKeys "<DOWN>" , 9 , true
    sleep(1)
    Liste.OpenContextmenu
    sleep(2)
    '/// Open 'Properties' (last entry)
    hMenuSelectNr(5)
    sleep(2)
    Kontext "ControlPropertiesDialog"
    Kontext "ControlPropertiesTabControl"
    ControlPropertiesTabControl.SetPage TabGeneralControl
    Kontext "TabGeneralControl"
    '/// Enter in 'Help URL' as value exactly 'HID:123456'
    TabGeneralControl.TypeKeys "<TAB>" , true
    HelpURL.setText "customcontrol1"
    HelpURL.TypeKeys "<RETURN>",true
    if HelpURL.GetText <> "customcontrol1" then
        warnlog "'customcontrol1' has not been accepted as 'Help URL' value."
    end if

    Kontext "ControlPropertiesDialog"
    hCloseDialog( ControlPropertiesDialog, "close" )

    Kontext "FormularNavigator"
    hCloseDialog( FormularNavigator, "close" )

    Kontext "FormDesignTools"
    hCloseDialog( FormDesignTools, "close" )

    Kontext "FormControls"
    '/// Switch back from the design mode
    SwitchControlDesignMode.click
    hCloseDialog( FormControls, "close" )

    '/// Check for right initial string ('Please select one:')in control
    Kontext "MSC_Special_Dialog"
    if MSC_Special_Dialog.Exists(3) then
        testString = MSC_Special_Dialog.getSelText
        if testString = "Please select one:" then
            printlog "OK, the right string was found in control"
        else
            warnlog "Wrong string found in Control: " & testString
        end if
    else
        warnlog "The control seems to be not named in the document?! Verify! If Solaris: #i100870#"
    end if
    call hCloseDocument
    hFileDelete( gLastWorkFile )
endcase

'-------------------------------------------------------------------------

testcase tIBIS111711
    printlog "iBIS Task 111711"

    printlog " Create a new document"
    call hNewDocument
    printlog " Enter some numbers as content"
    Kontext "DocumentCalc"
    DocumentCalc.typekeys "1<RETURN>2<RETURN><UP>"
    sleep(2)

    '/// create a chart from the content
    '/// + NOTE: Do NOT leave the Implace mode
    InsertChartCalc
    '/// The Chart wizard occours
    Kontext "ChartWizard"
    '/// On the chart wizard press OK button
    if ChartWizard.Exists(2) then
        printlog "Create chart"
        ChartWizard.OK
        sleep(2)
    else
        warnlog "Chart wizard did not occour! Exiting test case."
        call hCloseDocument
        goto endsub
    end if
    '/// Call slot Window->New Window
    Kontext "DocumentCalc"
    DocumentCalc.typekeys "<ESCAPE>"
    WindowNewWindow
    sleep(2)
    call gMouseClick(95,95)

    '/// Select chart in new window
    Kontext "DocumentCalc"
    call fselectFirstOLE
    sleep(2)

    '/// Delete chart in new window ...
    '/// ... and check for crash
    Kontext "DocumentCalc"
    try
        DocumentCalc.typekeys "<DELETE>"
        sleep (2)
        printlog "OK, there are still " & GetDocumentCount & " documents open"
    catch
        warnlog "Probably the office did crash -> #111711#"
        call hCloseDocument
        goto endsub
    endcatch
    call hCloseDocument
endcase


