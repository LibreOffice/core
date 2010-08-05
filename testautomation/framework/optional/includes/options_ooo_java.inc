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
'* short description : Tools->Options: OpenOffice.org Java
'*
'\******************************************************************************

testcase tOOoJava
    dim bJavaState as boolean

    ToolsOptions
    hToolsOptions( "StarOffice", "Java" )

    printlog( "Verify that Java is enabled and configured" )
    if ( usejava.isChecked() = false ) then
        warnlog( "Java should be enabled by default, checking and restarting" )
        bJavaState = hChangeJavaState( true )
        if ( bJavaState = false ) then
            warnlog( "Java is still not enabled, aborting test." )
            kontext "OptionenDlg"
            OptionenDlg.cancel()
            goto endsub
        endif
    else
        printlog( "   Java is enabled. Good." )
    endif
    
    ' Needs a delay, it might take some time until the listbox gets populated
    sleep( 3 )
    
    ' There should be a java-runtime installed. If not -> leave test
    printlog( "Make sure at least one Java runtime is installed" )
    if ( javalist.getitemcount() = 0 ) then
        warnlog( "No java listed in listbox, the test will stop" )
        kontext "OptionenDlg"
        OptionenDlg.cancel()
        goto endsub
    else
        printlog( "   Java is installed, good." )
    endif
    
    printlog( "Quickly test that all controls are active" )
    if ( add.isEnabled() ) then
        printlog( "   'Add...' is enabled" )
    else
        warnlog( "The 'Add...' button is disabled" )
    endif
    
    if ( parameters.isEnabled() ) then
        printlog( "   'Parameters...' is enabled" )
    else
        warnlog( "The 'Parameters...' button is disabled" )
    endif

    if ( classpath.isEnabled() ) then
        printlog( "   'Class Path...' is enabled" )
    else
        warnlog( "The 'Class Path...' button is disabled" )
    endif
    
    if ( JavaList.isEnabled() ) then
        printlog( "   'JavaList' is enabled" )
    else
        warnlog( "The 'JavaList' button is disabled" )
    endif
    
    kontext "OptionenDlg"
    OptionenDlg.ok
endcase

'*******************************************************************************

function hChangeJavaState( bEnable as boolean ) as boolean
    ' this little fella switches Java support on and off including a restart of
    ' the office to make the change active. The state is returned

    if ( bEnable ) then
        printlog( "   Enable Java" )
        usejava.check()
    else
        printlog( "   Disable Java" )
        useJava.unCheck()
    endif
        
    ' leave tools/options
    printlog( "   Leave Tools/Options" )
    kontext "OptionenDlg"
    OptionenDlg.ok()
        
    ' restart the office to make the change take effect
    printlog( "   Restart the application" )
    call exitRestartTheOffice()
        
    ' return to java page
    printlog( "   Return to Tools/Options Java-page" )
    ToolsOptions
    hToolsOptions( "StarOffice", "Java" )
    kontext "TabJava"
    
    if ( useJava.isChecked() ) then
        hChangeJavaState() = true
    else
        hChangeJavaState() = false
    endif
end function
