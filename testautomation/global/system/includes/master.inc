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
'* owner : helge.delfs@oracle.com
'*
'* short description : very important routines to start a testscript
'*
'************************************************************************
'*
' Global:
' #1 GetUseFiles             ' First routine which will be called automatically by starting a testscript
' #1 LoadDeclaration         ' Includes the whole declaration (.sid- and .win-files)
' #1 LoadGlobalIncludeFiles  ' Includes global .inc-files
'
' Recover routines:
' #1 TestEnter               ' Internal subroutine called at testcase-start -> start recovering
' #1 TestExit                ' Internal subroutine called at testcase-end
'                               If an error in a testscript occurs the test jumps directly into this routine
' #1 PleaseRecover           ' Recover routine used by TestExit- and TestEnter
'
' Office Startroutines:
' #1 hStartTheOffice         ' Routine to start the Office
' #1 ExitRestartTheOffice    ' Exit and restart the office - after 30 seconds
' #1 FirstOfficeStart        ' First office start with ResetApplication function
' #1 sStartUpOffice          ' sub routine which is being called from hStartTheOffice and ExitRestartTheOffice
' #1 hExitTheOffice          ' sub routine to just exit the office
'
' Special Information:
' #1 hFirstOutPut            ' Creates a general information header in result files for all tests
' #1 mMakeGeneralOptionsAPI  ' Sets general options in the office via API, without UI
' #1 hIsResultWriteable      ' Is TT able to write the resultfile in the directory? 
' #0 hDetectStatusDatabase   ' Detecting the status database server
' #1 hDisableQuickstarter    ' Disable the Quickstart feature on win32 systems
'*
'\***********************************************************************

sub GetUseFiles
'///<i>GetUseFiles</i>: The first routine which will be called automatically by starting a testscript.
'/// Include all important libraries (.inc) for the startup.
    Dim bQuickstarterStatus as Boolean
    Dim bSmallIconTheme As Boolean
    Dim sTemp as string
    Dim sPrivateEnvironmentLocation as string
    Dim sEnvironmentVersion as string
    Dim sEnvironmentDisplayVersion as string
    Dim sEnvironmentVersionMinor as string

    gTestcaseStart = Now()   ' get start time of test preparation phase

    GVERBOSE = FALSE ' global switch to make some functions more talkative for profiling purposes

    use "global\system\includes\sysinfo.inc"     '///+ sysinfo.inc  : routines to get all system informations
    use "global\system\includes\inivalue.inc"    '///+ inivalue.inc : routines to work with ini-files
    use "global\system\includes\iniinfo.inc"     '///+ iniinfo.inc  : routines to get all informations about OpenOffice.org
    use "global\system\includes\status.inc"      '///+ status.inc   : all routines to put the status-info into database    

'/// Call important system- and start-routines.
    Call LoadGlobalIncludeFiles           '///+<li><b>LoadGlobalIncludeFiles</b>: Include file with global routines (master.inc)</li>
    Call GetToolPlatform                  '///+<li><b>GetToolPlatform</b>: Detect operating environment for the Testtool (sysinfo.inc)</li>
    Call GetOfficePlatform                '///+<li><b>GetOfficePlatform</b>: Detect operating environment for the office (sysinfo.inc)</li>
    Call GetIniInformation                '///+<li><b>GetIniInformation</b>: Get all information out of the <i>testtool.ini/.testtoolrc</i> and set it on global variables (iniinfo.inc)</li>
    Call GetOfficeInformation             '///+<li><b>GetOfficeInformation</b>: Set <i>sAppExe</i> for startup (iniinfo.inc)</li>

    gVersionsnummer = FindBuildID
    ' split versionstring into its parts
    call hSetBuildVersionInformation(False)

    Call LoadDeclaration                  '///+<ul><li><b>LoadDeclaration</b>: Load all declaration files (master.inc)</li>

    Call FirstOfficeStart                 '///+<li><b>FirstOfficeStart</b>: Try to start the office for first time (master.inc)</li>
    Call GetLanguageInformation           '///+<li><b>GetLanguageInformation</b>: Detect the language of office (iniinfo.inc)</li>
    Call hFirstOutput                     '///+<li><b>hFirstOutput</b>: Make last adjustments and creat information output (master.inc)</li>

'/// make some default setting
    Call mMakeGeneralOptionsAPI           '///+<li><b>mMakeGeneralOptionsAPI</b></li>: Set general options <ol><li><u>system-file-dialogs off</u> on Win32</li><li>work-dir to internal ../user/work-dir</li></ol>(master.inc)</li>
   '///+<li>Disabling the Quickstarter.
        'This can't be done via API right now.
        bQuickstarterStatus = hDisableQuickstarter
        printlog "**  Quickstarter disabled         : " & bQuickstarterStatus

        'Small icon theme set
        bSmallIconTheme = hSetSmallIconTheme
        printlog "**  Small icon theme set          : " & bSmallIconTheme

    printlog    "----------------------------------------------------------------------------------------------------"
    printlog    ""

    if (NOT gSamePC) then
        gTestToolPath = gRemotePath
    end if
    
    ' Check if environment works with this OOo version
'/// Control the behaviour of the environment via the file qa/qatesttool/global/version.txt:
'/// [EnvironmentVersion]
'/// Current=OOH680
'/// DisplayName=2.4.x
'/// If 'Current' doesn't match the OOo version major string, the environment refuses to work, by exiting the test with a messagebox and a warning.
'/// The checking can be disabled, by using  Current=HEAD, or removing the file version.txt
    sPrivateEnvironmentLocation = ConvertPath (gTestToolPath + "global\version.txt")
    if fileExists(sPrivateEnvironmentLocation) then
        sEnvironmentVersion = getIniValue(sPrivateEnvironmentLocation, "EnvironmentVersion", "Current")
        sEnvironmentDisplayVersion = getIniValue(sPrivateEnvironmentLocation, "EnvironmentVersion", "DisplayName")
        sEnvironmentVersionMinor =  getIniValue(sPrivateEnvironmentLocation, "EnvironmentVersion", "Minor")
    else
        sEnvironmentVersion = "HEAD"
        sEnvironmentDisplayVersion = "Developer"
    end if
    if sEnvironmentVersion <> "HEAD" then
        if (len(gMajor) > 5) then
            if lCase(left(gMajor,6)) <> lCase(left(sEnvironmentVersion,6)) OR (lCase(gMinor) <> lCase(sEnvironmentVersionMinor)) then
                warnlog "This environment '" + sEnvironmentVersion + sEnvironmentVersionMinor + "' is not suitable for this OOo version '" + left(gMajor,6) + gMinor + "'!"+chr(13)+"Please get the environment suitable for this OOo version!"+chr(13)+"This Environment only works with OOo " + sEnvironmentDisplayVersion + "!"
                if MsgBox ("This environment '" + sEnvironmentVersion + sEnvironmentVersionMinor +"' is not suitable for this OOo version '" + left(gMajor,6) + gMinor + "'! Please get the environment suitable for this OOo version! This Environment only works with OOo " + sEnvironmentDisplayVersion + "!", 16, "Error at startup") = 1 then 
                    end
                end if
            end if
        end if
    end if
end sub

'-------------------------------------------------------------------------
sub LoadDeclaration
'/// Include all .sid-declarations (all menu-items as SlotIDs)
'///+&lt;German names&gt; (<u><b>DEPRECATED</u>, only fixes allowed!</b>)
    use "global\sid\all.sid"          '///+<ul><li>all.sid: All menuitems (German)</li>
    use "global\sid\bars.sid"         '///+<li>leisten.sid: All toolbars and toolboxes (German)</li>
    use "global\sid\context.sid"      '///+<li>kontext.sid: All context menus</li>
    use "global\sid\others.sid"       '///+<li>allgem.sid: other slots (German)</li></ul>
'///&lt;English names&gt;
    use "global\sid\e_all.sid"        '///+<ul><li>e_all.sid : all menuitems</li></ul>

'/// Include all .win-declarations (all dialogs as HelpIDs)
'///+&lt;German names&gt; (<u><b>OBSOLETE</u>, only fixes allowed!</b>)
    use "global\win\tab_a_d.win"      '///+<ul><li>All tabpages A...Z</li>
    use "global\win\tab_e_g.win"
    use "global\win\tab_h_o.win"
    use "global\win\tab_p_s.win"
    use "global\win\tab_t_z.win"

    use "global\win\dial_a_c.win"     '///+<li>All dialogs A...Z</li>
    use "global\win\dial_d_h.win"
    use "global\win\dial_i_o.win"
    use "global\win\dial_p_s.win"
    use "global\win\dial_t_z.win"

    use "global\win\sys_dial.win"     '///+<li>sys_dial.win: Systemdialogs</li>
    use "global\win\dokument.win"     '///+<li>dokument.win: All document-types</li>
    use "global\win\mathop.win"       '///+<li>mathop.win: Windows with all math operators</li>

    use "global\win\piloten.win"      '///+<li>piloten.win: Database-, Impress-, Microsoft(R) import-autopilots</li>
    use "global\win\w_autop.win"      '///+<li>w_autop.win: All Writer autopilots</li>

'///&lt;English names&gt;
    use "global\win\edia_a_c.win"     '///+<ul><li>All dialogs A...Z</li>
    use "global\win\edia_d_h.win"
    use "global\win\edia_i_o.win"
    use "global\win\edia_p_s.win"
    use "global\win\edia_t_z.win"

    use "global\win\etab_a_d.win"     '///+<li>All tabpages A...Z</li>
    use "global\win\etab_e_g.win"
    use "global\win\etab_h_o.win"
    use "global\win\etab_p_s.win"
    use "global\win\etab_t_z.win"

    use "global\win\e_mathop.win"     '///+<li>e_mathop.win: Mathoperators</li>

    use "global\win\bars.win"         '///+<li>bars.win:  All bars as windows</li>
    use "global\win\etoolbox.win"     '///+<li>etoolsbox.win: Toolboxes</li>
   
    use "global\win\spadmin.win"      '///+<li>spadmin.win: SPAdmin</li>

    use "global\win\reportdesigner.win" '///+<li>reportdesigner.win: ReportDesigner</li></ul>
end sub

'-------------------------------------------------------------------------

sub LoadGlobalIncludeFiles
'///Load all important global files.    
    use "global\tools\includes\required\t_dir.inc"          '///+<li><b>t_dir</b>: Routines to parse directories (methods execute on the office side)</li>
    use "global\tools\includes\required\t_doc1.inc"         '///+<li><b>t_doc1</b>: Global routines to work on office documents (I)</li>
    use "global\tools\includes\required\t_doc2.inc"         '///+<li><b>t_doc2</b>: Global routines to work on office documents (II)</li>
    use "global\tools\includes\required\t_files.inc"        '///+<ul><li><b>t_files</b>: Routines to open/save/print files</li>
    use "global\tools\includes\required\t_filters.inc"      '///+<li><b>t_filters</b>: Get information about filters</li></ul>    
    use "global\tools\includes\required\t_lists.inc"        '///+<li><b>t_lists</b>: Work with array/lists in basic scripts</li>
    use "global\tools\includes\required\t_menu.inc"         '///+<li><b>t_menu</b>: Routines to work with menus in the office</li>
    use "global\tools\includes\required\t_option.inc"       '///+<li><b>t_option</b>: Routine to navigate in the options-dialoge of office (I)</li>
    use "global\tools\includes\required\t_option2.inc"      '///+<li><b>t_option2</b>: Routine to navigate in the options-dialoge of office (II)</li>
    use "global\tools\includes\required\t_tools1.inc"       '///+<li><b>t_tools1</b> : Global routines (I)</li>
    use "global\tools\includes\required\t_tools2.inc"       '///+<li><b>t_tools2</b>: Global routines (II)</li>
    use "global\tools\includes\required\t_tools3.inc"       '///+<li><b>t_tools3</b>: Global routines (III)</li>    
end sub

'-------------------------------------------------------------------------

sub FirstOfficeStart
'/// First start-routine.
    Dim i as integer
    ' To nail down a possible culprint on test runs, the availability of existing user settings is checked.
    ' In some environments the deleting of the settings might fail before the test is started - this will log it:
    if (fileExists(gOfficePath)) then
        qaErrorLog ("QA_DEBUG: User settings directory was not deleted before test start!: '" + gOfficePath + "'")
    else
        printlog ("QA_DEBUG: User settings directory is unavailable before soffice is started - this is good. '" + gOfficePath + "'")
    endif
    caselog ""                   '///+<ul><li>Stop the first default-output into the resultfile</li>
    Call hStartTheOffice         '///+<li>Start the office</li>
    ResetApplication             '///+<li>Make the first recovering without an error output.</li></ul>
    'Disabling crash handling by TestTool. The crash reporter is used instead.
    'Needs to get executed everytime after a 'ResetApplication' command
    catchGPF false
     ' Recover to backingwindow, until resetApplication can handle this
     hFileCloseAll()
end sub

'-------------------------------------------------------------------------

sub hStartTheOffice (optional sProfPath as String, optional sProfParameter as String)
    Dim tVersionsnummer as string
    Dim tLastVersion as string
    Dim tlVersion() as string
    Dim tilVersion as integer
    Dim tiPosA as integer
    Dim tiPosB as integer
    Dim sErrorInformation as string
    Dim sTemp as string
    Dim sPlatformProgramPath as string
    
    sErrorInformation = "global::systen::inc::master.inc:hStartTheOffice: "
    
    if lcase( gPlatform ) = "osx" then
        sPlatformProgramPath = "MacOS"
    else
        sPlatformProgramPath = "program"
    end if
       
'/// <u>input</u>: <b>optional</b> <i>sProfPath</i> as String [path for profiling data]
'/// <u>input</u>: <b>optional</b> <i>sProfParameter</i> as String [filename for profiling data]

'///+<ol><li>Be sure the path to the result file exists and is writeable
'///+<ul><li>If it doesn't exist, try to create it</li>
'///+<li>If it isn't writeable a messagebox will be thrown</li>
'///+<li>The test will be stopped because without this path the testtool cannot write the resultfile to disk</li></ul></li>
    if (hIsResultWriteable() <> TRUE) then
        if MsgBox (sErrorInformation + "The test won't start! Please correct the error of the not createable RESULT file!", 16, "Error at startup") = 1 then 
            end
        end if
    end if
  
    if gSamePC = TRUE then
        tVersionsnummer = FindBuildID
        ' split versionstring into its parts
        tlVersion() = Split(tVersionsnummer, ",") 
        tilVersion = uBound(tlVersion()) ' array counts from 0 on!
        tLastVersion = tlVersion(tilVersion)
        ' major is from start to 'm'
        tiPosA = instr(tLastVersion, ":") + 1
        tiPosB = instr(tLastVersion, ")")
        gBuild = cInt(Mid(tLastVersion, tiPosA, tiPosB-tiPosA))
    end if        

'Startroutine for soffice.exe
    if sGetCrashreporterValues() <> TRUE then
        sTemp = "Please correct the entries in your testtool.ini/.testtoolrc." & Chr(13) & _
                "Here are the current entries:" & Chr(13) & Chr(13) & _
                "[Crashreporter]" & Chr(13) & _
                "UseProxy=" & gUseProxy & " (Has to be set to (true/false)" & Chr(13) & _
                "ProxyServer=" & gProxyServer & Chr(13) & _
                "ProxyPort=" & gProxyPort & Chr(13) & _
                "AllowContact=" & gAllowContact & " (Has to be set to (true/false)" &  Chr(13) & _
                "ReturnAddress=" & gReturnAddress
        printlog (sTemp)                                                        
        if MsgBox (sTemp, 16, "Error in TestTool control file!") = 1 then
                'TODO: Function to write the missing information to TT control file.
                'The test execution will be stopped here.
                end
        end if
    end if
'///+<li>The normal start routine.
'///+<ul><li>If you want to create a logfile for timestamps, the parameter <i>-env:RTL_LOGFILE=[Path]</i> is appended. (<u>Note</u>: On Win32 the path must have two backslashes)</li>
'///+<li>To ignore the crash- and recover- process for OpenOffice.org-documents the parameter <i>-norestore -nocrashreport</i> is also appended</li>
'///+<li>To disable the configuration lock dialog <i>-nolockcheck</i> is also appended</li>
'///+<li>To enable the automated testing <i>-enableautomation</i> has been added</li>
'///+<li>If you make a remote-test, the office process can't be started by the testtool; you would get an error message</li>
    try
        if IsMissing (sProfPath) then 
            sStartUpOffice
        else
            if IsMissing (sProfParameter) then
                QAErrorLog sErrorInformation & " Parameter 'sProfParameter' is NOT OPTIONAL if sProfPath has been set!"
                'The test execution will be stopped here.
                end
            else
                sStartUpOffice (sProfPath, sProfParameter)
            end if
        end if
    catch
        if gSamePC = FALSE then
            Exceptlog
            if MsgBox (sErrorInformation + "It is not possible to start the office application on a remote system." _
                & Chr(10) & "Please start the office application.", 1, "Starting the office application") = 2 then 
                end
            else
                warnlog sErrorInformation + "The test has a problem to start the office application. It needs a second chance!"
                '///+<li>Sometimes the first start instruction does not work correctly, so here it gets a second chance to start.</li></ul>
                if IsMissing (sProfPath) then 
                    sStartUpOffice
                else
                    if IsMissing (sProfParameter) then
                        QAErrorLog sErrorInformation & " Parameter 'sProfParameter' is NOT OPTIONAL if sProfPath has been set!"
                        'The test execution will be stopped here.
                        end
                    else
                        sStartUpOffice (sProfPath, sProfParameter)
                    end if
                end if
            end if
        end if
    endcatch
'///+<li>Set the debug output in an non-product version to the Testtool
    try
        'Only in a debug version of the office: Setting the assertion output to TestTool.   
        CaptureAssertions TRUE            
    catch
    endcatch
    Kontext "WelcomeDialog"
    if True = False Then ' disabled: we have no welcome dialog WelcomeDialog.Exists(2) then
        '///+<li>Clicking on Next button</li>
        NextBtn.Click
        Kontext "TabFirstStartLicense"
        if TabFirstStartLicense.Exists(1) then
            '///+<li>If the next tabpage will be visible pressing <CTRL+a> to get the 'Next'-button available.</li>
            LicenseText.TypeKeys "<MOD1 a>" , TRUE
            Kontext "WelcomeDialog"
            '///+<li>Clicking on Next button </li>
            try
                NextBtn.Click
            catch
                qaErrorLog "#i86137# STRG-A failed on license text, using workaround."
                Kontext "TabFirstStartLicense"
                while scrolldown.isEnabled
                    try
                        scrolldown.click
                    catch
                    endcatch
                wend
                ' Now trying again to go on...
                Kontext "WelcomeDialog"
                NextBtn.Click
            endcatch
        else
           if NOT gooo then
             warnlog sErrorInformation & "The second page (license) of the 'Welcome to $PRODUCTNAME'-wizard is not visible!"
           end if
        end if                
            Kontext "TabPersonalDataMigration"
            '///+<li>If an OpenOffice 1.1.x exists the migration page will be visible.</li>
            if TabPersonalDataMigration.Exists(1) then
                '///+<li>Uncheck the checkbox for tranferring personal data to get a clean configuration.</li>
                TransferPersonalData.uncheck
                Kontext "WelcomeDialog"
                '///+<li>Clicking on Next button </li>
                NextBtn.Click
                'TODO: Add exception handling.
            end if
            Kontext "TabFirstStartUser"
            if TabFirstStartUser.Exists(1) then
                Kontext "WelcomeDialog"
                '///+<li>If the next tabpage will be visible clicking on 'Next'-button.</li>
                NextBtn.Click
                'TODO: Add exception handling.
            end if
            Kontext "TabFirstStartOnlineUpdate"
            if TabFirstStartOnlineUpdate.Exists(1) then
                try
                    checkForUpdates.uncheck
                catch
                endcatch
                Kontext "WelcomeDialog"
                '///+<li>If the next tabpage will be visible clicking on 'Next'-button.</li>
                NextBtn.Click
                'TODO: Add exception handling.
            end if
            Kontext "TabFirstStartRegistration"                
            if TabFirstStartRegistration.Exists(1) then
                '///+<li>If the next tabpage will be visible checking 'do not want to register'-radio button.</li>
                doNotWantRegister.Check
                Kontext "WelcomeDialog"
                '///+<li>Clicking on 'Next'-button.</li></ul>
                WelcomeDialog.OK
            else
                warnlog sErrorInformation & "The fourth page (register) of the 'Welcome to $PRODUCTNAME'-wizard is not visible!"
            end if                
    end if                                         
    '///+</ol>
    'Disable Quickstarter internaly
    hDisableQuickstarterAPI 
end sub

'-------------------------------------------------------------------------

sub ExitRestartTheOffice (optional sProfPath as String, optional sProfParameter as String)
'/// input: optional sProfPath as String [path for profiling data]
'/// input: optional sProfParameter as String [filename for profiling data]
'/// If a messagebox comes up (yes, no) it will be closed (only one messagbox)
    Dim i as integer
    Dim sErrorInformation as string
   
    sErrorInformation = "global::systen::inc::master.inc:ExitRestartTheOffice: "
   
    hExitTheOffice()

    if IsMissing (sProfPath) then
        sStartUpOffice
    else                
        if IsMissing (sProfParameter) then
            QAErrorLog sErrorInformation & " Parameter 'sProfParameter' is NOT OPTIONAL if sProfPath has been set!"
            'The test execution will be stopped here.
            end
        else
            sStartUpOffice (sProfPath, sProfParameter)            
        end if
    end if           
    'Disabling crash handling by TestTool. The crash reporter is used instead.
    try
        catchGPF false
    catch
    endcatch
    'Disable Quickstarter internaly
    hDisableQuickstarterAPI

    ' Recover to backingwindow, until resetApplication can handle this
    hFileCloseAll()

end sub

'-------------------------------------------------------------------------

sub hExitTheOffice()
'/// Just shut down OOo
'/// If a messagebox comes up (yes, no) it will be closed (only one messagbox)
    'Disable Quickstarter internaly, last chance to join processes - important if testtool restarted OOo after a crash!
    hDisableQuickstarterAPI 
    try
        FileExit "SynchronMode", TRUE
    catch
    endcatch
    try
        Kontext "Active"
        if Active.Exists(2) then 
            Active.No
        end if
    catch
    endcatch    
    ' This is only >0 if defined in TestTool configuration file
    ' It is needed for a gcov enabled OOo build (Code coverage)
    ' and for valgrind (Memory leaks) tests
    ' valgrind: 90;1,5min, gcov 290;4,5min
    sleep(30+gOOoShutdownTimeOut)
end sub

'-------------------------------------------------------------------------

sub TestEnter
    '/// Automatically called, when a testcase-routine starts
    Call PleaseRecover  ("TestEnter")
    'Needed for status.inc
    gTestcaseStart = Now()
    Call writeCrashRepFile()
end sub

'-------------------------------------------------------------------------

sub TestExit
    '/// Automatically called at the end of a testcase or directly called when an error occurred
    '///+<li>Write the data for status page in a list in status.inc</li></ul>
    Call PleaseRecover  ("TestExit")    
    if ((gTestName <> "") AND isStatusEnabled()) then  
        'gTestName set in hStatusIn
        call hStatusAddTestcase()
    end if
end sub

'-------------------------------------------------------------------------

sub PleaseRecover (sWhat as String)
    Dim sError as String
    Dim sInterrupt as Boolean
    Dim sOtherError as Boolean
    Dim i as Integer
    Dim a as Integer
    
    sInterrupt = FALSE
    sOtherError = FALSE

'/// Close the translation window.
    try
        if gDasNicht = 0 then
            Kontext "TranslationWindow"
            if TranslationWindow.Exists then 
                TranslationWindow.Close
            end if
        end if
   catch
   endcatch

'/// Use <i>ResetApplication</i> method
    try
        call hCloseAllToolbars
        'All error-strings are in sError
        gStartTheOffice = FALSE
        'This is the trigger for a restart after application crashed:
        sError = ResetApplication                       
        'If sError = empty then no error occurred.
        if sError <> "" then                            
            printlog "   **    Error in " + sWhat + " -Routine      **"
            warnlog sError
        end if
    catch
        if NOT gStartTheOffice then
            printlog "global::system::inc::PleaseRecover: No office running while trying to recover: " + sError + " " + sWhat
            sInterrupt = TRUE
        else
            resetApplication
        endif
    endcatch

    try
        'Kill all commands after the reset
        AppAbort
    catch
    endcatch

'/// If the office crashes interrupt = TRUE and the office should be started again.
    if sInterrupt = TRUE then
        try
            Call hStartTheOffice
            SetClipboard ""
        catch
            try
                QAErrorLog "killapp: "+gTestName+"--"+getTestcaseName
                killapp
            catch
                ExceptLog
            endcatch
        endcatch

        try
            'kill all commands after the reset
            AppAbort
        catch
        endcatch
    end if

    try
        'Only in a debug version of the office: Setting the assertion output to TestTool
        CaptureAssertions TRUE            
    catch
    endcatch
    'Disabling crash handling by TestTool. The crash reporter is used instead.
    try
        catchGPF false
    catch
    endcatch
    Kontext

    ' Recover to backingwindow, until resetApplication can handle this
    hFileCloseAll()
end sub

'-------------------------------------------------------------------------

sub hFirstOutput
'/// Create a general header in result files for all tests.
    Dim sDir as String
    Dim sDir1 as String
    Dim sMajor as String
  
    if gSamePC = TRUE then
        if (len(gMajor)>3) then
            sMajor = left(gMajor,3)
        end if
        'Detecting child workspaces (CWS) and status database server
        if InStr(gVersionsnummer , "[CWS:") <> 0 then
            gCWS = TRUE
        else
            gCWS = FALSE
        end if
        if hDetectStatusDatabase = TRUE then
            gStatusDatabase = TRUE
        else
            gStatusDatabase = FALSE
        end if
    else
        gVersionsnummer = "REMOTE"
    end if
    iSystemSprache  = hGetSystemLanguage
    bDebugVersion   = NOT isProduct
    printlog    "----------------------------------------------------------------------------------------------------"
    printlog    "              I n f o r m a t i o n   A b o u t   T h e   T e s t   E n v i r o n m e n t"
    printlog    "----------------------------------------------------------------------------------------------------"
    printlog    "**  Application build ID          : " & sMajor & gVersionsnummer
    if bDebugVersion = TRUE then
        printlog "**  DEBUG version                 : " & bDebugVersion 
    end if
    if gCWS = FALSE then
        printlog "**  Build type                    : MASTER"
    else
        printlog "**  Build type                    : CWS"
    end if
    if isStatusEnabled() then
        'http://wiki.services.openoffice.org/wiki/QUASTe
        printlog "**  Status feature (QUASTe)       : Enabled " + gLocalStatusDatabase  
    end if
    printlog    "----------------------------------------------------------------------------------------------------"  
    printlog    "**  Application installation path : " +  gNetzOfficePath
    printlog    "**  User configuration path       : " +  gOfficePath
    printlog    "**  Started application           : " + sAppExe
    printlog    "----------------------------------------------------------------------------------------------------"
    printlog    "**  Application language          : " + iSprache + "  (" + gLanguage + " / " + gISOLang + ")"
    printlog    "**  System language               : " + iSystemSprache + "  (" + GetLanguageText (iSystemSprache) + ")"
    if gSamePC = FALSE then
        printlog "**  Platform VCL TestTool         : " + gtSYSName
        printlog "**  Platform application          : " + gSYSName + "   (" + gHost + ") "
        printlog "**  Path to remote base path      : " + gRemotePath
    else
        printlog "**  Testing platform              : " + gtSYSName
    end if
    printlog    "----------------------------------------------------------------------------------------------------"
    sDir1 = ConvertPath (gOfficePath + "user\work\"
    sDir = App.Dir (sDir1, 16)
    if sDir = "" then
        App.MKDir (sDir1)        
        printlog "**  Work path has been created    : " +  sDir1
    end if
    'Disabling embedded translation tooling 
    gDasNicht = 1
    'Disabling embedded screenshot tooling depending on value in .testtoolrc/ini
    gbSnapShot = sGetScreenshotValue   
end sub

'-------------------------------------------------------------------------

sub mMakeGeneralOptionsAPI
'/// At the beginning of each testrun set some defaults with the API
'///+ and write them down into the result file (.res).
'///+<ul>
    Dim bHelpTip as Boolean
    Dim sTempPath as string
    Dim sWorkPath as string    
    Dim oUnoOfficeConnection as object
    Dim oUnoConfigurationAccess as object
    Dim aPropertyValue(1) As new com.sun.star.beans.PropertyValue ' Array of pairs: Property with Value
    Dim xViewRoot
    Dim sTempList
    Dim bError as boolean

    const sFileFunction = "global::system::inc::master.inc::hMakeGeneralOptionsAPI:: "

    ' Open OOo UNO-Port for communication
    oUnoOfficeConnection=hGetUnoService(TRUE)
    if (isNull(oUnoOfficeConnection)) then
        warnlog (sFileFunction+"Couldn't create Uno access")
        exit sub
    end if

    try
        ' Open Configuration access
        oUnoConfigurationAccess=oUnoOfficeConnection.createInstance("com.sun.star.configuration.ConfigurationProvider")
        if (isNull(oUnoConfigurationAccess)) then
            warnlog (sFileFunction+"Couldn't create Configuration access")
            exit sub
        end if
        ' These Value pairs don't change inside this function, so they are only set once here.
        ' Specifies the location of the view root in the configuration:
        ' The value is the Path name of teh configuration item to change.
        aPropertyValue(0).Name="nodepath"
        ' Controls how updates are handled in the cache: If false , the cache 
        ' must operate in  write-through mode, where updates are written to 
        ' persistent storage at once - that is before ::commitChanges()  returns.
        aPropertyValue(1).Name="lazywrite"
        aPropertyValue(1).Value=False

        '///+<li>Tools / Language Settings / Languages
        '///+ Check if Asian language support is enabled and set <i>gAsianSup</i> variable TRUE or FALSE. 
        aPropertyValue(0).Value="/org.openoffice.Office.Common/I18N/CJK"
        xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
        gAsianSup = xViewRoot.getByName("AsianTypography")
        ' I don't use something more on the path: "/org.openoffice.Office.Common/I18N/CJK"
        ' Destroy, discard, dump, get rid of, put away, throw away, trash, the object:
        xViewRoot.dispose()
        '///+ Check if CTL (=complex text layout) is enabled amd set <i>gCTLSup</i> variable TRUE or FALSE.
        aPropertyValue(0).Value="/org.openoffice.Office.Common/I18N/CTL"
        xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
        gCTLSup = xViewRoot.getByName("CTLFont")
        xViewRoot.dispose()
    
        if gAsianSup = TRUE then
            printlog    "**  Asian language support        : " & gAsianSup
        end if
        if gCTLSup = TRUE then
            printlog    "**  Complex text layout support   : " & gCTLSup            
        end if
        '///+<li>Set <i>gAccessibility</i> from Options API</li>
        aPropertyValue(0).Value="/org.openoffice.VCL/Settings/ConfigurableSettings['Accessibility']"
        xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
        gAccessibility = xViewRoot.getByName("EnableATToolSupport")
        xViewRoot.dispose()
        if gAccessibility then
            printlog    "**  Accessibility  support        : " & gAccessibility
        end if
    
        '///+ <li>Switch the <i>system</i> file dialogs to &quot;internal&quot; file dialogs.
        aPropertyValue(0).Value="/org.openoffice.Office.Common/Misc"
        xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
        xViewRoot.setPropertyValue("UseSystemFileDialog", false)
        xViewRoot.commitChanges()
        ' Since we use lazywrite=false, the call to '.commitChanges()' returns if all data is written.
        ' This call to ask for pending changes is just to convince me;
        if xViewRoot.hasPendingChanges() then
            qaErrorLog(sFileFunction+"Changes still pending...: UseSystemFileDialog")
            ' At this point there is no needed to think about what to do, if it doesn't work.
        end if
        xViewRoot.dispose()
        gUseSysDlg = FALSE
        'printlog    "**  Using system file dialogs     : " & gUseSysDlg     
        
        if (lcase(gPlatform) = "osx") then
            '///+ <li>Switch the <i>system</i> print dialogs to &quot;internal&quot; print dialogs.
            aPropertyValue(0).Value="/org.openoffice.Office.Common/Misc"
            xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
            try
                xViewRoot.setPropertyValue("UseSystemPrintDialog", false)
                xViewRoot.commitChanges()
                xViewRoot.dispose()
            catch
            endcatch
        end if
        
        '///+<li>Disabling the <i>bubble help</i>.
        aPropertyValue(0).Value="/org.openoffice.Office.Common/Help"
        xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
        bHelpTip = xViewRoot.getByName("Tip")
        
        if (bHelpTip) then
            xViewRoot.setPropertyValue("Tip", false)
            xViewRoot.commitChanges()
            bHelpTip = FALSE
            if xViewRoot.hasPendingChanges() then
                qaErrorLog(sFileFunction+"Changes still pending...: HelpTip")
            end if
            'printlog    "**  Help/Tips                     : " & bHelpTip           
        end if
        xViewRoot.dispose()

        '///+ <li>Disable automatic check for updates
        ' Encapsulating this in a try...catch block because external (non-Sun) builds
        ' do usually not support online update so the API is not available and we get an
        ' exception. In case of failure we print a QAErrorLog.
        try
            aPropertyValue(0).Value="/org.openoffice.Office.Jobs/Jobs/Job['UpdateCheck']/Arguments"
            xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
            xViewRoot.replaceByName("AutoCheckEnabled", false)
            xViewRoot.commitChanges()
            ' Since we use lazywrite=false, the call to '.commitChanges()' returns if all data is written.
            ' This call to ask for pending changes is just to convince me;
            if xViewRoot.hasPendingChanges() then
                qaErrorLog(sFileFunction+"Changes still pending...: AutoCheckEnabled")
                ' At this point there is no needed to think about what to do, if it doesn't work.
            end if
            xViewRoot.dispose()
            gAutoUpdate = TRUE
        catch
            qaErrorLog( sFileFunction & "This build does not support online update." )
            gAutoUpdate = FALSE
        endcatch

        '///+ OOo Improvement - only if BUILD_SPECIAL is set we have this file created.
        sTempPath = gOfficeBasisPath & "share/registry/oooimprovement.xcd"
        sTempPath = ConvertPath( sTempPath )
        
        if ( FileExists( sTempPath ) ) then
            gOOoImprovementIsEnabled = TRUE
        else
            gOOoImprovementIsEnabled = FALSE
        endif

        '///+<li>Setting the <i>work</i> directory in Tools / Options,
        sTempPath = ConvertPath (gOfficePath + "user\work")
        '///+<li>If the temp-path does not exist it will be created. 
        if (app.dir(sTempPath, 16) <> "work") then
            try
                app.mkdir (sTempPath)
                bError = FALSE
            catch
                QAErrorLog sFileFunction + "Couldn't create temp-directory: '" + sTempPath + "'"
                bError = TRUE
            endcatch
        else
                bError = FALSE
        end if
        if (NOT bError) then
            oUnoConfigurationAccess=oUnoOfficeConnection.createInstance("com.sun.star.util.PathSettings")
            oUnoConfigurationAccess.Work = convertToURL(ConvertPath (gOfficePath + "user\work"))
            sWorkPath = convertFromURL(oUnoConfigurationAccess.Work)
            printlog    "**  'My Documents'-path is now    : " & sWorkPath             
        end if

'///+<li>Setting the directory for temporary files (=temp-path).
        sTempPath = ConvertPath (gOfficePath + "user\temp")
        '///+<li>If the temp-path does not exist it will be created. 
        if (app.dir(sTempPath, 16) <> "temp") then
            try
                app.mkdir (sTempPath)
                bError = FALSE
            catch
                QAErrorLog sFileFunction + "Couldn't create temp-directory: '" + sTempPath + "'"
                bError = TRUE
            endcatch
        else
                bError = FALSE
        end if
        if (NOT bError) then
            ' recycle from above: oUnoConfigurationAccess=oUnoOfficeConnection.createInstance("com.sun.star.util.PathSettings")
            oUnoConfigurationAccess.Temp = convertToURL(ConvertPath (gOfficePath + "user\temp"))
            sTempPath = convertFromURL(oUnoConfigurationAccess.Temp)          
            printlog    "**  'Temporary files'-path        : " & sTempPath             
        end if
        '///Check which 'Macro Security Level' is set and put it into <i>gMacroSecurityLevel</i> variable. 
        gMacroSecurityLevel = hSetMacroSecurityAPI( 2 )


    catch
        ExceptLog
        MsgBox (sFileFunction + "The test did not start correctly or the application crashed." & Chr(13) & _
            "Please correct the problem to make the general settings otherwise the test won't start!", 16, "Error at startup")
            'The test stops here if mMakeGeneralOptionsAPI has been run into problems (GPF, failure, ..)
        end
    endcatch
    '///+</ul>
end sub

'-------------------------------------------------------------------------

function hIsResultWriteable() as boolean    
    '///Running a test without being able to write the RESULT file to disk is worthless.
    '///+This functions checks if the directory exists, tries to create it and checks if it is writeable.
    '///+If this fails, the test won't start and presents a messagebox.
    Dim sResultFilePath as string
    Dim sTestDir as string

    'Get the path to the RESULT directory
    sResultFilePath = GetIniValue (gTesttoolIni, gTTProfileName , "LogBaseDir")
    if (dir(sResultFilePath, 16) = "") then ' doesn't exist      
        try
            MkDir (ConvertPath(sResultFilePath, gtPlatform))
            ' WorkAround for bug in dir() #104037#
            MkDir (ConvertPath(sResultFilePath+ gPathSigne+"ID104037", gtPlatform))
            if (dir(sResultFilePath, 16) <> "") then 
                ' does exist
                hIsResultWriteable = TRUE
                printlog "global::system::inc::master.inc::hIsResultWriteable: created RESULT directory: '" + sResultFilePath + "'"
                ' WorkAround for bug in dir()
                RmDir (ConvertPath(sResultFilePath + gPathsigne + "ID104037", gtPlatform))
            else
                warnlog "global::system::inc::master.inc::hIsResultWriteable: Make the directory '" + sResultFilePath + "' writeable; RESULT file can not be saved;"
                hIsResultWriteable = FALSE
            end if
        catch
            warnlog "global::system::inc::master.inc::hIsResultWriteable: Create the directory '" + sResultFilePath + "'; RESULT file can not be saved; (no right to create the directory)"
            hIsResultWriteable = FALSE
        endcatch
    else
        ' Check if it is writeable
        try
            sTestDir = "tbotest"   ' nasty bug, if the path is with a path sign at the end; usually on on windows root dirs :-(
            if (right(ConvertPath(sResultFilePath, gtPlatform), 1) <> hGetPathSigne(gtPlatform)) then
                sTestDir = hgetPathSigne(gtPlatform) + sTestDir
            end if
            MkDir (ConvertPath(sResultFilePath + sTestDir, gtPlatform))
            RmDir (ConvertPath(sResultFilePath + sTestDir, gtPlatform))
            hIsResultWriteable = TRUE
        catch
            warnlog "global::system::inc::master.inc::hIsResultWriteable: Make the directory '" + convertPath(sResultFilePath, gtPlatform) + "' writeable; RESULT file can not be saved;"
            hIsResultWriteable = FALSE
      endcatch
   end if
end function

'-------------------------------------------------------------------------

sub hDetectStatusDatabase as Boolean
'/// Detecting the status database server.
    dim sOOoLocalStatusDatabase as string
    dim sPrivateEnvironmentLocation as string
    dim sTemp as string
  
    sTemp = GetIniValue ( gTesttoolIni, "StatusFeatureLevel" , "Current" )
    if sTemp = "" then
        sPrivateEnvironmentLocation = ConvertPath (gTestToolPath + "errorlog\privateenvironment.txt")
        if fileExists(sPrivateEnvironmentLocation) then
            gStatusFeatureLevel = getIniValue(sPrivateEnvironmentLocation, "StatusFeatureLevel", "Current")
        else
            ' manual submitting status from errorlog directory
            gStatusFeatureLevel = 2
        end if
    else
        gStatusFeatureLevel = sTemp
    end if
    if gStatusFeatureLevel = 0 then
        ' automatical submitting status; filespace location defined in privateenvironment.inc
        hDetectStatusDatabase = TRUE
        gLocalStatusDatabase = ""
    else
        if gStatusFeatureLevel = 1 then
            ' automatical submitting status; filespace location defined in testtoolrc
            hDetectStatusDatabase = TRUE
            sOOoLocalStatusDatabase = GetIniValue ( gTesttoolIni, "OOoLocalStatusDatabase" , "Current" )
            if sOOoLocalStatusDatabase <> "" then
                if dir(sOOoLocalStatusDatabase,16) <> "" then
                    gLocalStatusDatabase = sOOoLocalStatusDatabase
                    'printlog "**  OOo Local Status Database Path: '" + sOOoLocalStatusDatabase + "'"
                else
                    qaErrorLog "**  OOo Local Status Database Path: '" + sOOoLocalStatusDatabase + "' doesn't exist or is not a directory."
                    gLocalStatusDatabase = ""
                end if
            else
                qaErrorLog "You are using status mode 1; you have to define the public filespace location in your testtoolrc:"+chr(13)+"[OOoLocalStatusDatabase]"+chr(13)+"Type=Path"+chr(13)+"Current=."
                gLocalStatusDatabase = ""
            end if
            sTemp = GetIniValue ( gTesttoolIni, "StatusDatabaseServerIP" , "Current" )
            if sTemp = "" then
                qaErrorLog "You are using status mode 1; you have to define the database server adress in your testtoolrc:"+chr(13)+"[StatusDatabaseServerIP]"+chr(13)+"Type=Path"+chr(13)+"Current=."
            else
                privateDatabaseServerIP = sTemp
            end if
            sTemp = GetIniValue ( gTesttoolIni, "StatusDatabaseServerPath" , "Current" )
            if sTemp = "" then
                qaErrorLog "You are using status mode 1; you have to define the database server path in your testtoolrc:"+chr(13)+"[StatusDatabaseServerPath]"+chr(13)+"Type=Path"+chr(13)+"Current=."
            else
                privateDatabaseServerPath = sTemp
            end if
        else
            if gStatusFeatureLevel = 2 then
                hDetectStatusDatabase = TRUE
                gLocalStatusDatabase = "errorlog directory"
            else
                hDetectStatusDatabase = FALSE
                gLocalStatusDatabase = ""
            end if
        end if
    end if
end sub

'-------------------------------------------------------------------------

function hDisableQuickstarter as boolean
'/// On all systems, disable the Quickstart feature which is enabled by default.
'///+ Returns: Answer to the question 'Was action taken, to disable it?'
    Dim bTemp as boolean    
    Dim bResult as boolean
    Dim bVeto as boolean
    
    bTemp = FALSE
    bResult = TRUE
    bVeto = FALSE ' True if Tools-Options-Memory needs to get triggert
    
    'This needs/can not be done on the following platforms:
    '- MacOS X
    '- Linux/Unix builds of StarOffice/Suite
    '- Win32 if already disabled, or not available.
    if gPlatGroup <> "unx" then
        ' if quickstart.exe exists, it might run, else no need to open options UI
        bVeto = fileExists(gNetzOfficePath + "program\quickstart.exe")
    else
        if  (lcase(gPlatform) = "osx") then
            bVeto = FALSE
        else
            ' Not needed if StarOffice/Suite
            bVeto = gOOo
        end if
    end if
    bVeto = FALSE ' For now LibreOffice struggles with some hid/whatever mismatch here
    if bVeto then
            'First, disabling the Quickstarter via UI
            ToolsOptions
            call hToolsOptions ("STAROFFICE", "MEMORY")
            try
                bTemp = LoadQuickstarter.isChecked
                if bTemp then
                    LoadQuickstarter.uncheck
                end if
            catch
                bResult = FALSE
            endcatch
            Kontext "ExtrasOptionenDlg"
            ExtrasOptionenDlg.OK
    else
        bResult = FALSE
    end if

    'Second, closing the Quickstarter process that a restart of the office
    hDisableQuickstarterAPI()
    hDisableQuickstarter = bResult
end function

'-------------------------------------------------------------------------

sub sStartUpOffice (optional sProfPath as String, optional sProfParameter as String)
    Dim sParameter as String
    Dim sErrorInformation as string
    Dim sUnoPort as string
    Dim sPlatformProgramPath as string
    Dim sPlatformBinExt as string

    sErrorInformation = "global::systen::inc::master.inc:sStartUpOffice: " 

    '/// The environment for the non-GUI Crashreporter function will be set.
    setChildEnv("ERRORREPORT_HTTPPROXYSERVER",gProxyServer)
    setChildEnv("ERRORREPORT_HTTPPROXYPORT",gProxyPort)
    setChildEnv("ERRORREPORT_HTTPCONNECTIONTYPE",gConnectionType)
    setChildEnv("ERRORREPORT_RETURNADDRESS",gReturnAddress)
    setChildEnv("ERRORREPORT_SUBJECT","Error_Report_from_TestTool")
    setChildEnv("ERRORREPORT_BODYFILE",ConvertPath (gOfficePath & "user\work\crashrep.txt"))
            
    ' Set a valid URE_BOOTSTRAP path for soffice, else the invalid one from this testtool instance
    ' will get used i86718 - would result in a message about vcl stuff and a not starting soffice
    if lcase( gPlatform ) = "osx" then
        sPlatformProgramPath = "MacOS"
        sPlatformBinExt = "rc"
    else
        sPlatformProgramPath = "program"
        if gPlatGroup <> "unx" then
            sPlatformBinExt = ".ini"
        else
            sPlatformBinExt = "rc"
        end if
    end if
    setChildEnv("URE_BOOTSTRAP",convertToURL(convertPath(gNetzOfficePath + sPlatformProgramPath + "/fundamental" + sPlatformBinExt)))
    
    ' Getting UNO-port
    hGetUNOService(true, sUnoPort)
    if sUnoPort <> "" then
        sUnoPort = "-accept=socket,host=localhost,port=" + sUnoPort + ";urp "
    else
        sUnoPort = ""
        warnlog "UNO port is not defined: Set it in the TestTool application: Extra -> Settings -> Misc -> Remote UNO Port"
    end if
    
    '/// To start the application some parameter need to be set:
    '///+ <ol><li><i>-enableautomation</i> to enable the TCP/IP connection between office application and TestTool</li>
    '///+ <li><i>-norestore</i> to eliminate the the document recovery functionylity after a crash</li>
    '///+ <li><i>-nolockcheck</i> to elimante the 'parallel running instances'-check and always start the application</li>
    '///+ <li><i>-autocrashreport</i> to enable the non-GUI crash report functionality</li>
    '///+ <li><i>-accept=socket,host=localhost,port=12345;urp</i> to enable UNO connection</li>
    '///+ <li><i>OPTIONAL application parameters</i> for profiling tests</li>    
    sParameter = "-enableautomation -norestore -nolockcheck -autocrashreport -nofirststartwizard " & sUnoPort & sAppParameter & " "
    if IsMissing (sProfPath) then
        '///+ <li><i>Factory</i>-parameter which depends on the value of <b>gApplication</b> (WRITER, CALC, ...)</li></ol>
        sParameter = sParameter & sFactory
        ' try/catch is needed for special tasks, to workaround time outs
        try
            Start sAppExe, sParameter
        catch
        endcatch
        try
            ' This is only >0 if defined in TestTool configuration file
            ' It is needed for valgrind (Memory leaks) tests
            ' Valgrind: 420;8min
            sleep(gOOoStartupTimeOut)
        catch
        endcatch
    else
         if IsMissing (sProfParameter) then
            '/// If OPTIONAL profiling path parameters have been set the profiling filename will be checked
            '///+ because then this parameter is not optional.
            QAErrorLog sErrorInformation & " Parameter 'sProfParameter' is NOT OPTIONAL if sProfPath has been set!"
            'The test execution will be stopped here.
            end
        else
            '/// If both OPTIONAL parameters (path, filename) have been set, the office application
            '///+ will be started with an additional parameter:
            '///+ <ul><li><i>-env:RTL_LOGFILE=pathname/filename</i> to write the profiling log.</li>             
            sParameter = sParameter & "-env:RTL_LOGFILE=" & sProfPath & sProfParameter
        end if
        '///+ <li><i>Factory</i>-parameter which depends on the value of <b>gApplication</b> (WRITER, CALC, ...)</li></ul>        
        Start sAppExe, sParameter & " " & sFactory
    end if      
end sub

sub StartTheOffice
    '/// this routine is not intended to be called from within a testscript.
    '/// this routine will get called by the VCL TestTool application, in case it recognizes there is no soffice.bin running anymore or not responding anymore
    '/// this happens usually after OpenOffice.org crashed
    'keep it as short as possible! 
    warnlog "OpenOffice.org application crashed or can not be started. Double click the above error message if exists for the culprint."
    sStartUpOffice()
    try
        catchGPF false
    catch
    endcatch
    hDisableQuickstarterAPI()
    ' set global variable for backwards compatibility
    gStartTheOffice = TRUE
end sub

function hSetSmallIconTheme as Boolean
    ' Set default icon theme to small

    Dim bResult as Boolean

    bResult = FALSE

    ToolsOptions

    call hToolsOptions ("STAROFFICE", "View")
    IconScale.Select(2)
    ' Verify the icon scale has been set
    If (IconScale.GetSelIndex = 2) Then
        bResult = TRUE
    End If

    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Sleep (2)

    hSetSmallIconTheme = bResult

end function
