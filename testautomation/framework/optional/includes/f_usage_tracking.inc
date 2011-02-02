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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Usage tracking
'*
'\******************************************************************************

private const LOGFILE = "user\temp\Feedback\Current.csv"

'*******************************************************************************

testcase tUsageTracking1

    printlog( "Test case 1: Presence of the Usage Tracking featureset" )
    
    ' Broken in CWS sb111'
    ' Currently missing: Check for Office brand, StarOffice should always have OOoImp enabled'
    
    if ( not gOOoImprovementIsEnabled ) then
        warnlog( "Usage tracking program is not enabled for this build" )
    endif
    
endcase

'*******************************************************************************

testcase tUsageTracking2

    ' There must not be a log file to start with.
    
    printlog( "Test case 2: There must be no logfile present yet" )
    
    dim cLogFile as string
        cLogFile = gOfficePath & LOGFILE
        cLogFile = convertpath( cLogFile )
        
    if ( dir( cLogFile ) = "" ) then
        printlog( "No log file. Good" )
    else
        warnlog( "Logfile not esxpected: " & cLogFile )
        hDeleteFile( cLogFile )
    endif

endcase
   
'*******************************************************************************
   
testcase tUsageTracking3

    ' When the usage tracking mechanism is off we want the ui to reflect that.

    printlog( "Test case 3: Presence of configuration tabpage and button states for disabled functionality" )

    ToolsOptions
    hToolsOptions( "StarOffice" , "IMPROVEMENT" )
    
    kontext "TabOOoImprovement"
    if ( TabOOoImprovement.exists( 1 ) ) then
    
        if ( not ParticipateNo.isChecked() ) then
            warnlog( "#i98736# - UI should reflect current status for usage tracking: No" )
        endif
        
        if ( ShowData.isEnabled() ) then
            warnlog( "#i116461# - Show data button should not be enabled if nothing is logged" )
    
            ShowData.click()
            
            kontext "FilterAuswahl"
            if ( FilterAuswahl.exists( 2 ) ) then
                warnlog( "Filter Selection dialog is not expected at this point, ")
                FilterAuswahl.cancel()
            endif
            
            kontext "Active"
            if ( Active.exists( 2 ) ) then
                warnlog( "Messagebox not required as control should not be enabled (see #i116461#)" )
                Active.ok()
            
                kontext "TextImport"
                if ( TextImport.exists( 5 ) ) then
                    warnlog( "There should be nothing to load if usage tracking is disabled" )
                    
                    TextImport.ok()
                    
                    kontext "CALC"
                    if ( DocumentCalc.exists( 3 ) ) then
                        hCloseDocument()
                    else
                        warnlog( "If a log exists it should open in a new Calc document" )
                    endif
                endif
            endif
        endif
    else
        warnlog( "OOo Improvement Tabpage (Tools/Options) is missing" )
    endif
      
    kontext "OptionenDlg"
    OptionenDlg.cancel()
    
endcase

'*******************************************************************************

testcase tUsageTracking4

    ' When enabling usage tracking we want the mechanism to work at once

    printlog( "Test case 4: Enable usage tracking, track something and display the log" )
    
    dim iDocumentType as integer

    dim cLogFile as string
        cLogFile = gOfficePath & LOGFILE
        cLogFile = convertpath( cLogFile )
        
    dim lFileSize as long
    
    const FILE_SIZE = 2000
    
    hDeleteFile( cLogFile )
    
    ToolsOptions
    
    hToolsOptions( "StarOffice" , "IMPROVEMENT" )
    
    kontext "TabOOoImprovement"
    if ( TabOOoImprovement.exists( 1 ) ) then

        ParticipateYes.check()
        
        if ( hForceUsageTrackingOn() ) then
        
            kontext "OptionenDlg"
            OptionenDlg.ok()
           
            ' Open different document types and modify them so something is logged
            for iDocumentType = 1 to 4
            
                hNumericDocType( iDocumentType )
                hNewDocument()
                hChangeDoc()
                hCloseDocument()
                
            next iDocumentType 
            
            ToolsOptions
            
            hToolsOptions( "StarOffice" , "IMPROVEMENT" )
            
            kontext "TabOOoImprovement"
            if ( TabOOoImprovement.exists( 1 ) ) then
            
                lFileSize = hGetFileSizeAsLong( cLogFile )
                printlog( "Filesize is: " & lFileSize & " byte" )
                if ( lFileSize <> FILE_SIZE ) then
                    warnlog( "The logfile does not have the expected size" )
                    printlog( "Expected: " & FILE_SIZE )
                    printlog( "Found...: " & lFileSize )
                endif
                
                ShowData.click()
                
                kontext "TextImport"
                if ( TextImport.exists( 3 ) ) then
                
                    TextImport.ok()
                    
                    kontext "CALC"
                    if ( DocumentCalc.exists( 3 ) ) then
                        printlog( "Calc document is open" )
                        if ( getDocumentCount <> 1 ) then
                            warnlog( "Other, unexpected documents are open" )
                        else
                            hCloseDocument()
                        endif
                    else
                        warnlog( "Logfile should open in a new spreadsheet document, document missing" )
                    endif
                    
                else
                
                    warnlog( "Text Import (CSV) settings dialog did not open" )         
                    
                    kontext "Active"
                    if ( Active.exists() ) then
                        warnlog( "Unexpected messagebox" )
                        printlog( Active.getText() )
                        Active.ok()
                    endif         
                endif
                
                kontext "TabOOoImprovement"
                ParticipateNo.click()
                
                kontext "Active"
                if ( Active.exists( 3 ) ) then
                    Active.Yes()
                else
                    qaerrorlog( "#i98739# - No delete option available when stopping usage tracking" )
                endif
                
            endif
        else
            qaerrorlog( "#i98741# - Failed to force Usage Tracking on via API" )
        endif
            
        kontext "OptionenDlg"
        OptionenDlg.ok()
        
    endif
    
    hDeleteFile( cLogFile )
             
endcase

'*******************************************************************************

function hForceUsageTrackingOn()

    ' the usage tracking extension was disabled by force, in this function 
    ' we force it back on as the switch in the UI is ignored

    Dim oUnoOfficeConnection as object
    Dim oUnoConfigurationAccess as object
    Dim aPropertyValue(1) As new com.sun.star.beans.PropertyValue ' Array of pairs: Property with Value
    Dim xViewRoot
    
    Dim oOOoImprovementController as object  ' Check presence of the OOo Improvement Program
    
    try
        oUnoConfigurationAccess=oUnoOfficeConnection.createInstance("com.sun.star.configuration.ConfigurationProvider")
        if (isNull(oUnoConfigurationAccess)) then       
            warnlog (sFileFunction+"Couldn't create Configuration access")
            exit function
        endif
        
        aPropertyValue(0).Name="nodepath"
        aPropertyValue(1).Name="lazywrite"
        aPropertyValue(1).Value=False

        oOOoImprovementController = oUnoOfficeConnection.createInstance( "com.sun.star.oooimprovement.CoreController" )
        if ( isNull( oOOoImprovementController ) ) then
            hForceUsageTrackingOn() = FALSE
        else
            aPropertyValue(0).Value="/org.openoffice.Office.Logging/OOoImprovement"
            xViewRoot=oUnoConfigurationAccess.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPropertyValue())
            xViewRoot.replaceByName("EnablingAllowed", true )
            xViewRoot.commitChanges()
            if xViewRoot.hasPendingChanges() then
                qaErrorLog("Configuration not flushed: OOoImprovementProgram/InvitationAccepted: FALSE")
            end if
            xViewRoot.dispose()
            hForceUsageTrackingOn() = TRUE
        endif

    catch
        printlog( "Could not enable Usage Tracking via API" )
    endcatch

end function
