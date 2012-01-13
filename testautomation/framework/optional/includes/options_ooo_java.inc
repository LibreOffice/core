'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

*****

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
