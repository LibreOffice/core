'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: basic_issues.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:13 $
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
'/******************************************************************************
'*
'*  owner : Joerg.Skottke@Sun.Com
'*
'*  short description : Regression tests for especially annoying BASIC issues
'*
'\******************************************************************************

function hIDERunMacro()

    kontext "MacroBar"
    if ( MacroBar.exists() ) then
        BasicRun.click()
    else
        warnlog( "MacroBar not accessible" )
    endif
    
end function

'*******************************************************************************

function hTestActive( cString as string , iMethod as integer , bExact as boolean ) as integer

    ' Return values:
    ' -1 = Exact match expected, fuzzy match found
    ' -2 = No match 
    ' -3 = content of messagebox not relevant
    ' -4 = Messagebox not displayed / Macro not executed
    ' 1 - 4 = Number of buttons on the dialog
    
    const CFN = "hTestActive()::"

    dim cMsg as string     ' The string printed in the message box
    dim rc as integer      ' Returncode for error handling and returnvalue
    dim buttons as integer ' Number of buttons on the message box

    kontext "Active"
    if ( Active.exists( 2 ) ) then
    
        cMsg = Active.getText()
    
        if ( cString <> "" ) then
            if ( instr( active.getText(), cString ) <> 0 ) then
                if ( active.getText() = cString ) then
                    'printlog( CFN & "Correct message found (exact match): " & cMsg )
                    rc = 0
                else
                    if ( bExact ) then
                        warnlog( CFN & "Message is no exact match: " & cMsg )
                        rc = -1
                    else
                        'printlog( CFN & "Correct message found (fuzzy match): " & cMsg )
                        rc = 0
                    endif
                endif
            else
                warnlog( CFN & "Messagebox has wrong content: " & cMsg )
                rc = -4
            endif
        else
            'printlog( CFN & "Messagebox: " & cMsg )
            rc = -3
        endif
        
        buttons = Active.getButtonCount()
        'printlog( CFN & "Number of buttons: " & buttons )
        select case( iMethod )
        case 1 : Active.OK()
        case 2 : Active.Cancel()
        case 3 : Acitve.Yes()
        case 4 : Active.No()
        end select
    else
        warnlog( CFN & "Messagebox is missing" )
    endif
    
    if ( rc = 0 ) then hTestActive() = buttons
    
end function
       
'*******************************************************************************

testcase t_macros()

    gApplication = "WRITER"
    call hNewDocument()
    hInitBasicIDE( "TestIssues" )
    
    printlog( "" )
    printlog( " ---------- i41695 ------------ " )
    hInsertMacroFromFile( "i41695" )
    hIDERunMacro()
    hTestActive( "i41695-1", 1 , TRUE )
    hTestActive( "i41695-2", 1 , TRUE )    

    printlog( "" )
    printlog( " ---------- i83978 ------------ " )
    hInsertMacroFromFile( "i83978" )
    hIDERunMacro()
    hTestActive( "com.sun.star.container.NoSuchElementException", 1 , FALSE )

    printlog( "" )    
    printlog( " ---------- i82830 ------------ " )
    hInsertMacroFromFile( "i82830" )
    hIDERunMacro()
    hTestActive( "12D687" , 1 , TRUE )
    hTestActive( "4553207", 1 , TRUE )

    if ( gISOLang = "en-US" ) then
        printlog( "" )    
        printlog( " ---------- i81674 ------------ " )
        hInsertMacroFromFile( "i81674" )
        hIDERunMacro()
        hTestActive( "250" , 1 , FALSE )
        hTestActive( "Yes" , 1 , TRUE  )
        hTestActive( "True", 1 , TRUE  )
        hTestActive( "On"  , 1 , TRUE  )
    endif
    
    printlog( "" )
    printlog( " ---------- i80532 ------------ " )
    hInsertMacroFromFile( "i80532" )
    hIDERunMacro()
    hTestActive( "-10", 1 , FALSE )
    hTestActive( "1"  , 1 , FALSE )
    hTestActive( "-10", 1 , FALSE )
    
    printlog( "" )
    printlog( " ---------- i84040 ------------ " )
    hInsertMacroFromFile( "i84040" )
    hIDERunMacro()
    hTestActive( "False", 1 , TRUE )
    hTestActive( "False", 1 , TRUE )
    
    printlog( "" )
    printlog( " ---------- i86265 ------------ " )
    hInsertMacroFromFile( "i86265" )
    hIDERunMacro()
    hTestActive( "i86265-1", 1 , TRUE )
    hTestActive( "i86265-2", 1 , TRUE )
    
    printlog( "" )
    printlog( " ---------- i92329 ------------ " )
    hInsertMacroFromFile( "i92329" )
    hIDERunMacro()
    hTestActive( "Return an Array" , 1 )
    
    printlog( "" )
    printlog( " ---------- i97038 ------------ " )
    hInsertMacroFromFile( "i97038" )
    hIDERunMacro()
    hTestActive( "1900" , 1 , FALSE )
    hTestActive( "2"    , 1 , FALSE )
'---------    
    printlog( "" )
    printlog( " ---------- i103691 ------------ " )
    hInsertMacroFromFile( "i103691" )
    hIDERunMacro()
    hTestActive( "Equal" , 1 , FALSE )

    printlog( "" )
    printlog( " ---------- i103697 ------------ " )
    hInsertMacroFromFile( "i103697" )
    hIDERunMacro()
    hTestActive( "i103697" , 1 , FALSE )

    printlog( "" )
    printlog( " ---------- i103990 ------------ " )
    hInsertMacroFromFile( "i103990" )
    hIDERunMacro()
    hTestActive( "42" , 1 , FALSE )
    hTestActive( "43" , 1 , FALSE )
    hTestActive( "Pi" , 1 , FALSE )

'-------
    printlog( "" )
    printlog( " ---------- MessageBoxes ------------ " )
    hInsertMacroFromFile( "MessageBoxes" )
    hIDERunMacro()
    if ( hTestActive( "0x" , 1 , TRUE ) > 1 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "1x" , 1 , TRUE ) > 2 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "2x" , 2 , TRUE ) > 3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "3x" , 2 , TRUE ) > 3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "4x" , 4 , TRUE ) > 2 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "5x" , 2 , TRUE ) > 2 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "16" , 2 , TRUE ) > 3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "32" , 2 , TRUE ) > 3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "48" , 2 , TRUE ) > 3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "64" , 1 , TRUE ) > 1 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "128", 2 , TRUE ) > 3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "256", 2 , TRUE ) > 3 ) then warnlog( "Wrong resource type" )
    if ( hTestActive( "512", 2 , TRUE ) > 3 ) then warnlog( "Wrong resource type" )
    
    printlog( "" )    
    hCloseBasicIDE()
    hCloseDocument()

endcase

