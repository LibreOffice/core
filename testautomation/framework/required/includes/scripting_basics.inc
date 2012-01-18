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
'*
'*  short description : Quick short test for the scripting framework
'*
'\******************************************************************************

testcase tScripting

    printlog( "Verify that the nodes <My Macros> and <OOo Macros> are present" )
    
    const ORGANIZER_ITEMS = 2
    const MAX_DIALOG_DELAY = 5
    
    hFileCloseAll()

    ToolsMacrosOrganizeMacrosBeanShell
    
    Kontext "ScriptOrganizer"
    if ( ScriptOrganizer.exists( MAX_DIALOG_DELAY ) ) then
        printlog( "Beanshell Script Organizer is present. Good" )
        if ( ScriptTreeList.getItemCount() <> ORGANIZER_ITEMS ) then
            warnlog( "The Treelist does not contain the expected number " & _
            "of items. Please verify that exactly " & ORGANIZER_ITEMS & " top-nodes exist" )
        endif
        hCloseDialog( ScriptOrganizer, "cancel" )
    else
        warnlog( "Script Organizer for Beanshell is missing" )
    endif
    
    ToolsMacrosOrganizeMacrosJavaScript
    
    Kontext "ScriptOrganizer"
    if ( ScriptOrganizer.exists( MAX_DIALOG_DELAY ) ) then
        printlog( "JavaScript Script Organizer is present. Good" )
        if ( ScriptTreeList.getItemCount() <> ORGANIZER_ITEMS ) then
            warnlog( "The Treelist does not contain the expected number " & _
            "of items. Please verify that exactly " & ORGANIZER_ITEMS & " top-nodes exist" )
        endif
        hCloseDialog( ScriptOrganizer, "cancel" )
    else
        warnlog( "Script Organizer for JavaScript is missing" )
    endif
    
endcase
    


