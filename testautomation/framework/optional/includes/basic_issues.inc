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

function hTestActive( cString as string , iMethod as integer ) as integer

    dim cMsg as string

    kontext "Active"
    if ( Active.exists( 2 ) ) then
    
        cMsg = Active.getText()
    
        if ( cString <> "" ) then
            if ( instr( active.getText(), cString ) <> 0 ) then
                printlog( "Correct message found: " & cMsg )
            else
                warnlog( "Messagebox has wrong content: " & cMsg )
            endif
        else
            printlog( "Messagebox: " & cMsg )
        endif
        
        hTestActive() = Active.getButtonCount()        
        select case( iMethod )
        case 1 : Active.OK()
        case 2 : Active.Cancel()
        case 3 : Acitve.Yes()
        case 4 : Active.No()
        end select
    else
        warnlog( "Messagebox is missing" )
    endif
    
end function
       
'*******************************************************************************

testcase t_macros()

    gApplication = "WRITER"
    call hNewDocument()
    hInitBasicIDE( "TestIssues" )
    
    dim cLocalMacrosFile as string
        cLocalMacrosFile = gTesttoolPath & "framework\optional\input\vba-compat\vba-compat-macros.txt"

    printlog( "" )
    printlog( " ---------- i41695 ------------ " )
    hInsertMacroFromFile( "i41695" )
    hIDERunMacro()
    hTestActive( "i41695-1", 1 )
    hTestActive( "i41695-2", 1 )    

    printlog( "" )
    printlog( " ---------- i83978 ------------ " )
    hInsertMacroFromFile( "i83978" )
    hIDERunMacro()
    hTestActive( "com.sun.star.container.NoSuchElementException", 1 )

    printlog( "" )    
    printlog( " ---------- i82830 ------------ " )
    hInsertMacroFromFile( "i82830" )
    hIDERunMacro()
    hTestActive( "12D687", 1 )
    hTestActive( "4553207", 1 )

    if ( gISOLang = "en-US" ) then
        printlog( "" )    
        printlog( " ---------- i81674 ------------ " )
        hInsertMacroFromFile( "i81674" )
        hIDERunMacro()
        hTestActive( "250", 1 )
        hTestActive( "Yes", 1 )
        hTestActive( "True", 1 )
        hTestActive( "On", 1 )
    endif
    
    printlog( "" )
    printlog( " ---------- i80532 ------------ " )
    hInsertMacroFromFile( "i80532" )
    hIDERunMacro()
    hTestActive( "-10", 1 )
    hTestActive( "1", 1 )
    hTestActive( "-10", 1 )
    
    printlog( "" )
    printlog( " ---------- i84040 ------------ " )
    hInsertMacroFromFile( "i84040" )
    hIDERunMacro()
    hTestActive( "false", 1 )
    hTestActive( "false", 1 )
    
    printlog( "" )
    printlog( " ---------- i86265 ------------ " )
    hInsertMacroFromFile( "i86265" )
    hIDERunMacro()
    hTestActive( "i86265-1", 1 )
    hTestActive( "i86265-2", 1 )
    
    printlog( "" )
    printlog( " ---------- i92329 ------------ " )
    hInsertMacroFromFile( "i92329" )
    hIDERunMacro()
    hTestActive( "Return an Array" , 1 )
    
    printlog( "" )
    printlog( " ---------- MessageBoxes ------------ " )
    hInsertMacroFromFile( "MessageBoxes" )
    hIDERunMacro()
    if ( hTestActive( "0x" , 1 ) <> 1 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "1x" , 1 ) <> 2 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "2x" , 2 ) <> 3 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "3x" , 2 ) <> 3 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "4x" , 4 ) <> 2 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "5x" , 2 ) <> 2 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "16" , 2 ) <> 3 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "32" , 2 ) <> 3 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "48" , 2 ) <> 3 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "64" , 1 ) <> 1 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "128", 2 ) <> 3 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "256", 2 ) <> 3 ) then warnlog( "Wrong ressource type" )
    if ( hTestActive( "512", 2 ) <> 3 ) then warnlog( "Wrong ressource type" )


    
    printlog( "" )    
    hCloseBasicIDE()
    hCloseDocument()

endcase

